/*
 *  dispsect.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include"adflib.h"
#include "common.h"
#include "log.h"

#define TEST_VERBOSITY 3


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
    struct AdfList *list, *cell;
    struct GenBlock *block;
 
    adfEnvInitDefault();

    adfEnvSetProperty ( ADF_PR_USEDIRC, true );

    /* display or not the physical / logical blocks and W or R */
    adfEnvSetProperty ( ADF_PR_USE_RWACCESS, true );
 
    /* create and mount one device */
    hd = adfDevCreate ( "dump", "dispsect-newdev", 80, 2, 11 );
    if (!hd) {
        log_error( "can't create device\n" );
        adfEnvCleanUp(); exit(1);
    }

    showDevInfo( hd );

    if ( adfCreateFlop ( hd, "empty", ADF_DOSFS_FFS |
                                      ADF_DOSFS_DIRCACHE ) != ADF_RC_OK )
    {
        log_error("can't create floppy\n");
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

    log_info( "\ncreate file_1a" );
    fic = adfFileOpen ( vol, "file_1a", ADF_FILE_MODE_WRITE );
    if (!fic) {
        log_error( "can't open file\n" );
        adfVolUnMount(vol);
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        adfEnvCleanUp();
        exit(1);
    }
    adfFileWrite ( fic, 1, buf );
    adfFileClose ( fic );

    showVolInfo( vol );

    log_info( "\ncreate dir_5u" );
    adfCreateDir(vol,vol->curDirPtr,"dir_5u");
    showVolInfo( vol );

    showDirEntries( vol, vol->curDirPtr );

    log_info( "\nremove file_1a" );
    adfRemoveEntry(vol,vol->curDirPtr,"file_1a");
    showVolInfo( vol );

    log_info( "\nremove dir_5u" );
    adfRemoveEntry(vol,vol->curDirPtr,"dir_5u");
    showVolInfo( vol );

    cell = list = adfGetDelEnt(vol);
    while(cell) {
        block =(struct GenBlock*) cell->content;
        printf ( "%s %d %d %d\n",
                 block->name,
                 block->type,
                 block->secType,
                 block->sect);
        cell = cell->next;
    }
    adfFreeDelList(list);

    log_info( "\nundel file_1a" );
    adfUndelEntry(vol,vol->curDirPtr,883); // file_1a
    showVolInfo( vol );

    log_info("\nundel dir_5u");
    adfUndelEntry(vol,vol->curDirPtr,885); // dir_5u
    showVolInfo( vol );

    showDirEntries( vol, vol->curDirPtr );

    adfVolUnMount(vol);
    adfDevUnMount ( hd );
    adfDevClose ( hd );

    adfEnvCleanUp();

    return 0;
}
