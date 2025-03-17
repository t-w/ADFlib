/*
 * rename.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include"adflib.h"
#include "common.h"


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

    int status = 0;

    struct AdfVolume *vol;
 
    adfEnvInitDefault();

    /* create and mount one device */
    struct AdfDevice * const hd = adfDevCreate ( "dump", "rename2-newdev", 80, 2, 11 );
    if (!hd) {
        fprintf(stderr, "can't mount device\n");
        status = 1;
        goto cleanup_env;
    }

    if ( adfCreateFlop ( hd, "empty", ADF_DOSFS_FFS |
                                      ADF_DOSFS_DIRCACHE ) != ADF_RC_OK )
    {
		fprintf(stderr, "can't create floppy\n");
        status = 1;
        goto cleanup_dev;
    }

    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        fprintf(stderr, "can't mount volume\n");
        status = 1;
        goto cleanup_dev;
    }

    showDevInfo( hd );

    showVolInfo( vol );
    showDirEntries( vol, vol->curDirPtr );
    putchar('\n');

    adfCreateDir(vol,vol->curDirPtr,"dir_5u");

    adfCreateDir(vol,883,"dir_51");

    adfCreateDir(vol,vol->curDirPtr,"toto");
printf("[dir = %d]\n",vol->curDirPtr);
    showDirEntries( vol, vol->curDirPtr );

printf("[dir = %ld]\n",883L);
    showDirEntries( vol, 883 );

    adfRenameEntry(vol, 883,"dir_51", vol->curDirPtr,"dir_55");
putchar('\n');

printf("[dir = %d]\n",vol->curDirPtr);
    showDirEntries( vol, vol->curDirPtr );

printf("[dir = %ld]\n",883L);
    showDirEntries( vol, 883 );

    adfRenameEntry(vol, vol->curDirPtr,"toto", 883,"moved_dir");

putchar('\n');

printf("[dir = %d]\n",vol->curDirPtr);
    showDirEntries( vol, vol->curDirPtr );

printf("[dir = %ld]\n",883L);
    showDirEntries( vol, 883 );

    adfVolUnMount( vol );

cleanup_dev:
    adfDevUnMount( hd );
    adfDevClose( hd );

cleanup_env:
    adfEnvCleanUp();

    return status;
}
