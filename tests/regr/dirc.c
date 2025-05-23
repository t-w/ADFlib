/*
 * dir_test.c
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
    struct AdfDevice *hd;
    struct AdfVolume *vol;
    struct AdfList *list, *cell;
 
    adfLibInit();

//	adfEnvSetFct(0,0,MyVer,0);

    /* mount existing device */

    hd = adfMountDev ( argv[1], ADF_ACCESS_MODE_READWRITE );
    if (!hd) {
        fprintf(stderr, "can't mount device\n");
        adfLibCleanUp(); exit(1);
    }

    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        adfUnMountDev(hd);
        adfCloseDev(hd);
        fprintf(stderr, "can't mount volume\n");
        adfLibCleanUp(); exit(1);
    }

    showVolInfo( vol );

    cell = list = adfGetDirEnt(vol,vol->curDirPtr);
    while(cell) {
        adfEntryPrint ( cell->content );
        cell = cell->next;
    }
    adfFreeDirList(list);

    putchar('\n');

    /* not empty */
    adfRemoveEntry(vol, vol->curDirPtr, "dir_2");

    /* cd dir_2 */

    adfEnvSetProperty ( ADF_PR_USEDIRC, true );

    cell = list = adfGetDirEnt(vol,vol->curDirPtr);
    while(cell) {
        adfEntryPrint ( cell->content );
        cell = cell->next;
    }
    adfFreeDirList(list);

    putchar('\n');


    adfVolUnMount(vol);
    adfUnMountDev(hd);
    adfCloseDev(hd);

    adfLibCleanUp();

    return 0;
}
