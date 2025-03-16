/*
 *  comment.c
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

    struct AdfDevice *hd;
    struct AdfVolume *vol;
    struct AdfFile *fic;
    unsigned char buf[1];
 
    adfEnvInitDefault();

    /* create and mount one device */
    hd = adfDevCreate ( "dump", "comment-newdev", 80, 2, 11 );
    if (!hd) {
        log_error( "can't create device\n" );
        adfEnvCleanUp(); exit(1);
    }

    showDevInfo( hd );

    if ( adfCreateFlop ( hd, "empty", ADF_DOSFS_FFS |
                                      ADF_DOSFS_DIRCACHE ) != ADF_RC_OK )
    {
        log_error( "can't create floppy\n" );
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        adfEnvCleanUp(); exit(1);
    }

    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        log_error( "can't mount volume\n" );
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        adfEnvCleanUp(); exit(1);
    }

    fic = adfFileOpen ( vol, "file_1a", ADF_FILE_MODE_WRITE );
    if (!fic) {
        log_error( "can't open file for writing\n" );
        adfVolUnMount(vol);
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        adfEnvCleanUp();
        exit(1);
    }
    adfFileWrite ( fic, 1, buf );
    adfFileClose ( fic );

    showVolInfo( vol );

    adfCreateDir(vol,vol->curDirPtr,"dir_5u");

    showDirEntries( vol, vol->curDirPtr );

    adfSetEntryComment(vol, vol->curDirPtr, "dir_5u", "dir_5u comment");
    adfSetEntryComment(vol, vol->curDirPtr, "file_1a", "file_1a very very long comment");

    putchar('\n');

    showDirEntries( vol, vol->curDirPtr );

    adfSetEntryComment(vol, vol->curDirPtr, "dir_5u", "");
    adfSetEntryComment(vol, vol->curDirPtr, "file_1a", "new comment" );

    putchar('\n');

    adfEnvSetProperty ( ADF_PR_USEDIRC, true );

    showDirEntries( vol, vol->curDirPtr );

    adfVolUnMount(vol);
    adfDevUnMount ( hd );
    adfDevClose ( hd );

    adfEnvCleanUp();

    return 0;
}
