/*
 * bootdisk.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adflib.h"
#include "common.h"
#include "log.h"

#define TEST_VERBOSITY 1


void MyVer(char *msg)
{
    fprintf(stderr,"Verbose [%s]\n",msg);
}


/*
 *
 *
 */
int main(int argc, char *argv[])
{
    log_init( stderr, TEST_VERBOSITY );

    if ( argc < 2 ) {
        log_error( "missing parameter (bootcode file) - aborting...\n");
        return 1;
    }
    struct AdfDevice *hd;
    struct AdfVolume *vol;
    FILE* boot;
    unsigned char bootcode[1024];
 
    boot = fopen( argv[1], "rb" );
    if ( ! boot ) {
        log_error( "can't open bootcode file\n" );
        exit(1);
    }
    fread( bootcode, sizeof(unsigned char), 1024, boot );
    fclose( boot );

    adfEnvInitDefault();

    /* create and mount one device */
    hd = adfDevCreate( "dump", "bootdisk-newdev", 80, 2, 11 );
    if ( ! hd ) {
        log_error( "can't create device\n" );
        adfEnvCleanUp();
        exit(1);
    }

    showDevInfo( hd );

    if ( adfCreateFlop( hd, "empty", ADF_DOSFS_FFS |
                                     ADF_DOSFS_DIRCACHE ) != ADF_RC_OK )
    {
        log_error( "can't create floppy\n" );
        adfDevUnMount( hd );
        adfDevClose( hd );
        adfEnvCleanUp();
        exit(1);
    }

    vol = adfVolMount( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if ( ! vol ) {
        log_error( "can't mount volume\n" );
        adfDevUnMount( hd );
        adfDevClose( hd );
        adfEnvCleanUp();
        exit(1);
    }

    adfVolInstallBootBlock( vol, bootcode );

    showVolInfo( vol );

    adfVolUnMount(vol);
    adfDevUnMount( hd );
    adfDevClose( hd );

    adfEnvCleanUp();

    return 0;
}
