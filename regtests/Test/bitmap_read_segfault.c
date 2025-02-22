/*
 * test for segfault on reading allocation bitmap
 * when bitmap pages table on the filesystem is not correct
 * ie. non-zero records beyond expected (fixed) size
 *
 * more details:
 * https://github.com/lclevy/ADFlib/issues/63
 */


#include <stdio.h>
#include <stdlib.h>

#include "adflib.h"

#define TEST_VERBOSITY 0

static void log_error ( FILE * const       file,
                        const char * const format, ... )
{
#if TEST_VERBOSITY > 0
    va_list ap;
    va_start ( ap, format );
    //fprintf ( stderr, "Warning <" );
    vfprintf ( file, format, ap );
    va_end ( ap );
#else
    (void) file, (void) format;
#endif
}


int main ( const int          argc,
           const char * const argv[] )
{
    if ( argc < 2 )
        return 1;
    
    adfEnvInitDefault();

//	adfEnvSetFct(0,0,MyVer,0);
    bool error_status = false;
    struct AdfDevice * const dev = adfDevOpen ( argv[1], ADF_ACCESS_MODE_READONLY );
    if ( ! dev ) {
        fprintf ( stderr, "Cannot open file/device '%s' - aborting...\n",
                  argv[1] );
        adfEnvCleanUp();
        exit(1);
    }
    ADF_RETCODE rc = adfDevMount ( dev );
    if ( rc != ADF_RC_OK ) {
        log_error ( stderr, "can't mount device %s\n", argv[1] );
        error_status = true;
        goto close_dev;
    }

    /*** crash happens here, on mounting the volume, in adfReadBitmap() ***/
    struct AdfVolume * const vol = adfVolMount ( dev, 0, ADF_ACCESS_MODE_READONLY );
    if ( vol == NULL ) {
        log_error ( stderr, "can't mount volume %d\n", 0 );
        error_status = true;
        goto umount_dev;
    }

    //adfVolInfo(vol);
    //putchar('\n');

    adfVolUnMount ( vol );

umount_dev:
    adfDevUnMount ( dev );

close_dev:
    adfDevClose ( dev );

//clean_up:
    adfEnvCleanUp();

    return error_status;
}
