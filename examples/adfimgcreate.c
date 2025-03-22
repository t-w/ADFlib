/*
 * adfimgcreate
 *
 * an utility for creating an empty Amiga disk images (ADF or HDF)
 *
 *  This file is part of ADFLib.
 *
 *  ADFLib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  ADFLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <adflib.h>
#include <errno.h>
#include "pathutils.h"
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include "getopt.h"
#else
//#include <libgen.h>
#include <unistd.h>
#endif

struct geometry {
    unsigned  tracks,
              heads,
              sectors;
};

// https://en.wikipedia.org/wiki/List_of_floppy_disk_formats
const struct devtype {
    const char * const     name;
    const struct geometry  geometry;
    const char * const     descr;
} devtypes[] = {
    { "sd",     { 40, 2, 11 }, "440 KiB floppy 5.25"              },
    { "dd",     { 80, 2, 11 }, "880 KiB floppy (standard)"        },
    { "hd1520", { 80, 2, 19 }, "1520 KiB floppy (HD)"             },
    { "hd1760", { 80, 2, 22 }, "1760 KiB floppy (HD)"             },

    { "dd81",   { 81, 2, 11 }, "non-standard 81 tracks DD floppy" },
    { "dd82",   { 82, 2, 11 }, "non-standard 82 tracks DD floppy" },
    { "dd83",   { 83, 2, 11 }, "non-standard 83 tracks DD floppy" },
    { "hd81",   { 81, 2, 22 }, "non-standard 81 tracks HD floppy" },
    { "hd82",   { 82, 2, 22 }, "non-standard 82 tracks HD floppy" },
    { "hd83",   { 83, 2, 22 }, "non-standard 83 tracks HD floppy" },

    { "pc360",  { 40, 2,  9 }, "PC 360 KiB floppy 5.25\""         },
    { "pc1200", { 80, 2, 15 }, "PC 1,2 MiB floppy 5.25\""         },
    { "pc720",  { 80, 2,  9 }, "PC 720 KiB floppy 3.5\""          },
    { "pc1440", { 80, 2, 18 }, "PC 1.440 MiB floppy 3.5\""        },
    { "pc2880", { 80, 2, 36 }, "PC 2.80 MiB floppy 3.5\""         },
    { NULL,     {  0, 0,  0 }, NULL                               }
};

struct newdev {
    const char *     adfname;
    struct geometry  geometry;
};

const char * prgname;

void usage( FILE * const stream )
{
    fprintf( stream, "\nadfimgcreate - create empty (not formatted!) ADF/HDF image files\n"
             "\nUsage:  adfimgcreate [-k sizeKiB | -m sizeMiB | -g tracks heads sectors"
             " | -t type] filename\n\n"
             "  where the type can be:\n\n" );
    /*
             "     'dd'                    880KiB floppy (DD)\n"
             "     'hd'                    1760KiB floppy (HD)\n"
             "     'dd81', 'dd82', 'dd83'  non-standard 81-83 tracks DD floppy\n"
             "     'hd81', 'hd82', 'hd83'  non-standard 81-83 tracks HD floppy\n\n" ); */
    for ( const struct devtype * dt = &devtypes[ 0 ] ; dt->name != NULL; dt++ ) {
        fprintf( stream, "   %-6s     %-33s (%2u/%u/%2u)\n",  dt->name, dt->descr,
                 dt->geometry.tracks, dt->geometry.heads, dt->geometry.sectors );

    }
    putchar('\n');
}


void check_not_exist( const char * const filename );

bool parse_args( const int * const           argc,
                 const char * const * const  argv,
                 struct newdev *             devreq );


int main( const int                   argc,
          const char * const * const  argv )
{
    struct newdev devreq;

    if ( argc < 3 || ! parse_args( &argc, argv, &devreq ) ) {
        usage( stderr );
        return 1;
    }

    check_not_exist( devreq.adfname );

    printf( "Creating disk image: '%s'\n", devreq.adfname );

    adfLibInit();

    struct AdfDevice * const device =
        adfDevCreate( "dump", devreq.adfname,
                      devreq.geometry.tracks,
                      devreq.geometry.heads,
                      devreq.geometry.sectors );
    if ( ! device ) {
        fprintf( stderr, "Error creating disk image %s with geometry (tracks/heads/sectors): "
                 "%u / %u / %u.\n", devreq.adfname,
                 devreq.geometry.tracks,
                 devreq.geometry.heads,
                 devreq.geometry.sectors );
        return 1;
    }

    char * const devInfo = adfDevGetInfo( device );
    printf( "%s", devInfo );
    free( devInfo );

    printf( "Done!\n" );

    adfDevUnMount( device );
    adfDevClose( device );
    adfLibCleanUp();

    return 0;
}




/* return value: true - valid, false - invalid */
bool parse_args( const int * const           argc,
                 const char * const * const  argv,
                 struct newdev *             devreq )
{
    // set default options
    memset( devreq, 0, sizeof(struct newdev) );

    struct options_set { bool g, k, m, t; } options_set = {
        false, false, false, false
    };
    const char * valid_options = "g:k:m:t:h";
    int opt;
    while ( ( opt = getopt( *argc, (char * const *) argv, valid_options ) ) != -1 ) {
        //printf( "optind %d, opt %c, optarg %s\n", optind, (char) opt, optarg );
        switch ( opt ) {
        case 'g': {

            // tracks
            char * endptr = NULL;
            devreq->geometry.tracks = (unsigned) strtoul( optarg, &endptr, 10 );
            if ( endptr == optarg || devreq->geometry.tracks < 1 ) {
                fprintf( stderr, "Invalid number of tracks '%s'.\n", optarg );
                exit( 1 );
            }

            // heads
            endptr = NULL;
            devreq->geometry.heads = (unsigned) strtoul( ++optarg, &endptr, 10 );
            if ( endptr == optarg || devreq->geometry.heads < 1 ) {
                fprintf( stderr, "Invalid number of heads '%s'.\n", optarg );
                exit( 1 );
            }

            // sectors
            endptr = NULL;
            devreq->geometry.sectors = (unsigned) strtoul( ++optarg, &endptr, 10 );
            if ( endptr == optarg ||
                 devreq->geometry.sectors < 10 )    // what is real min?
            {
                fprintf( stderr, "Invalid number of sectors '%s'.\n", optarg );
                exit( 1 );
            }
            optarg++;

            if ( options_set.k || options_set.m || options_set.t ) {
                fprintf( stderr, "Ambiguous options provided\n" );
                exit( 1 );
            }
            options_set.g = true;

            continue;
        }

        case 'k': {
            // size in KiB

            char * endptr = NULL;
            unsigned sizekib = (unsigned) strtoul( optarg, &endptr, 10 );
            if ( endptr == optarg ) {
                fprintf( stderr, "Invalid size '%s'.\n", optarg );
                exit( 1 );
            }

            if ( sizekib > 1024 * 1024 * 2 ) {
                fprintf( stderr, "Size %u is too large (2GiB limit).\n", sizekib );
                exit( 1 );
            }

            uint32_t sizeblocks = (uint32_t)( ( (uint64_t) sizekib * 1024 ) / 512 );

            devreq->geometry.tracks  = 1;
            devreq->geometry.heads   = 1;
            devreq->geometry.sectors = sizeblocks;

            if ( options_set.g || options_set.m || options_set.t ) {
                fprintf( stderr, "Ambiguous options provided\n" );
                exit( 1 );
            }

            options_set.k = true;
            continue;
        }

        case 'm': {
            // size in MiB

            char * endptr = NULL;
            unsigned sizemib = (unsigned) strtoul( optarg, &endptr, 10 );
            if ( endptr == optarg ) {
                fprintf( stderr, "Invalid size '%s'.\n", optarg );
                return false;
            }

            if ( sizemib > 1024 * 2 ) {
                fprintf( stderr, "Size %u is too large (2GiB limit).\n", sizemib );
                return false;
            }

            uint32_t sizeblocks = (uint32_t)( ( (uint64_t) sizemib * 1024 * 1024 ) / 512 );

            devreq->geometry.tracks  = 1;
            devreq->geometry.heads   = 1;
            devreq->geometry.sectors = sizeblocks;

            if ( options_set.g || options_set.k || options_set.t ) {
                fprintf( stderr, "Ambiguous options provided\n" );
                exit( 1 );
            }

            options_set.m = true;
            continue;
        }

        case 't': {
            for ( const struct devtype * dt = &devtypes[ 0 ] ; dt->name != NULL; dt++ ) {
                //printf ("compare with %s\n", dt->name );
                if ( strcmp( dt->name, optarg ) == 0 ) {
                    //printf ("found\n");
                    devreq->geometry = dt->geometry;
                    break;
                }
            }

            if ( options_set.g || options_set.k || options_set.m ) {
                fprintf( stderr, "Ambiguous options provided\n" );
                exit( 1 );
            }

            options_set.t = true;
            continue;
        }
        case 'h':
            usage( stdout );
            exit( 0 );

        default:
            return false;
        }
    }

    /* the name of the adf image - required */
    if ( optind > *argc - 1 ) {
        fprintf( stderr, "Missing filename of the device image to create.\n" );
        exit(1);
    }
    devreq->adfname = argv[ optind++ ];

    if ( argv[ optind ] != NULL ) {
        fprintf( stderr, "Unexpected (invalid) parameters after filename.\n" );
        exit(1);
    }
    if ( argv[ optind ] != NULL ||
         ! ( options_set.g ||
             options_set.k ||
             options_set.m ||
             options_set.t ) )
    {
        return false;
    }

    return true;
}


void check_not_exist( const char * const filename )
{
    if ( filename == NULL ) {
        fprintf( stderr, "Filename is null...\n" );
        exit(1);
    }

    FILE * const fp = fopen( filename, "rb" );
    if ( fp ) {
        fclose( fp );
        fprintf( stderr, "'%s' already exists - aborting...\n", filename );
        exit(1);
    }
}
