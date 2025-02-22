/*
 *  dispsect.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include"adflib.h"


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
        fprintf(stderr, "can't mount device\n");
        adfEnvCleanUp(); exit(1);
    }

    adfDevInfo ( hd );

    if ( adfCreateFlop ( hd, "empty", ADF_DOSFS_FFS |
                                      ADF_DOSFS_DIRCACHE ) != ADF_RC_OK )
    {
		fprintf(stderr, "can't create floppy\n");
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        adfEnvCleanUp(); exit(1);
    }

    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        fprintf(stderr, "can't mount volume\n");
        adfEnvCleanUp(); exit(1);
    }

    fic = adfFileOpen ( vol, "file_1a", ADF_FILE_MODE_WRITE );
    if (!fic) {
        adfVolUnMount(vol);
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        adfEnvCleanUp();
        exit(1);
    }
    adfFileWrite ( fic, 1, buf );
    adfFileClose ( fic );

    puts("\ncreate file_1a");
    adfVolInfo(vol);

    adfCreateDir(vol,vol->curDirPtr,"dir_5u");
    puts("\ncreate dir_5u");
    adfVolInfo(vol);

    cell = list = adfGetDirEnt(vol, vol->curDirPtr);
    while(cell) {
        adfEntryPrint ( cell->content );
        cell = cell->next;
    }
    adfFreeDirList(list);

    puts("\nremove file_1a");
    adfRemoveEntry(vol,vol->curDirPtr,"file_1a");
    adfVolInfo(vol);

    adfRemoveEntry(vol,vol->curDirPtr,"dir_5u");
    puts("\nremove dir_5u");
    adfVolInfo(vol);

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

    adfUndelEntry(vol,vol->curDirPtr,883); // file_1a
    puts("\nundel file_1a");
    adfVolInfo(vol);

    adfUndelEntry(vol,vol->curDirPtr,885); // dir_5u
    puts("\nundel dir_5u");
    adfVolInfo(vol);

    cell = list = adfGetDirEnt(vol, vol->curDirPtr);
    while(cell) {
        adfEntryPrint ( cell->content );
        cell = cell->next;
    }
    adfFreeDirList(list);


    adfVolUnMount(vol);
    adfDevUnMount ( hd );
    adfDevClose ( hd );

    adfEnvCleanUp();

    return 0;
}
