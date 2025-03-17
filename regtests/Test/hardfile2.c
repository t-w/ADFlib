/*
 * bootdisk.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include"adflib.h"
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
    (void) argc, (void) argv;

    log_init( stderr, TEST_VERBOSITY );

    struct AdfVolume *vol;

    adfEnvInitDefault();

    int status = 0;

    /* create and mount one device : 4194304 bytes */
    struct AdfDevice * const hd = adfDevCreate ( "dump", "hardfile2-newdev",
                                                 256, 2, 32 );
    if (!hd) {
        log_error( "can't create device\n" );
        status = 1;
        goto cleanup_env;
    }

    if ( adfCreateHdFile ( hd, "empty", ADF_DOSFS_FFS |
                                        ADF_DOSFS_DIRCACHE ) != ADF_RC_OK ) {
        log_error( "can't create hdfile (format)\n" );
        status = 1;
        goto cleanup_dev;
    }

    showDevInfo( hd );

    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        log_error( "can't mount volume\n" );
        status = 1;
        goto cleanup_dev;
    }
    showVolInfo( vol );
    showDirEntries( vol, vol->curDirPtr );

    /* unmounts */
    adfVolUnMount(vol);

cleanup_dev:
    adfDevUnMount ( hd );
    adfDevClose ( hd );

cleanup_env:
    adfEnvCleanUp();

    return status;
}
