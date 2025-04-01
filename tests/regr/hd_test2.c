/*
 * hd_test.c
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
    struct AdfVolume *vol;

    adfLibInit();

    const char tmpdevname[] = "hd_test2-newdev";

    /* a zip disk */
    struct AdfDevice * hd = adfDevCreate ( "dump", tmpdevname, 2891, 1, 68 );
    if (!hd) {
        fprintf(stderr, "can't mount device\n");
        adfLibCleanUp(); exit(1);
    }

    showDevInfo( hd );

    struct AdfPartition ** const partList =
        (struct AdfPartition **) malloc( sizeof(struct AdfPartition *) );
    if (!partList) exit(1);

    struct AdfPartition part1;
    part1.startCyl = 2;
	part1.lenCyl = 2889;
	part1.volName = strdup("zip");
    part1.volType = ADF_DOSFS_FFS | ADF_DOSFS_DIRCACHE;

    partList[0] = &part1;
    ADF_RETCODE rc = adfCreateHd( hd, 1, (const struct AdfPartition * const * const) partList );
    free(partList);
    free(part1.volName);
    if ( rc != ADF_RC_OK ) {
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        fprintf ( stderr, "adfCreateHd returned error %d\n", rc );
        adfLibCleanUp(); exit(1);
    }

    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        fprintf(stderr, "can't mount volume\n");
        adfLibCleanUp(); exit(1);
    }

    showVolInfo( vol );
    adfVolUnMount(vol);
    adfDevUnMount ( hd );
    adfDevClose ( hd );

    /* mount the created device */
    hd = adfDevOpen ( tmpdevname, ADF_ACCESS_MODE_READWRITE );
    if ( ! hd ) {
        fprintf ( stderr, "Cannot open file/device '%s' - aborting...\n",
                  tmpdevname );
        adfLibCleanUp();
        exit(1);
    }

    rc = adfDevMount ( hd );
    if ( rc != ADF_RC_OK ) {
        adfDevClose ( hd );
        fprintf(stderr, "can't mount device\n");
        adfLibCleanUp(); exit(1);
    }

    showDevInfo( hd );

    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        fprintf(stderr, "can't mount volume\n");
        adfLibCleanUp(); exit(1);
    }

    showVolInfo( vol );

    adfVolUnMount(vol);
    adfDevUnMount ( hd );
    adfDevClose ( hd );

    adfLibCleanUp();

    return 0;
}
