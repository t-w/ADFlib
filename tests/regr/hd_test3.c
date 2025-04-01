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

    int status = 0;

    adfLibInit();

    const char tmpdevname[] = "hd_test3-newdev";

    /* an harddisk, "b"=7.5Mb, "h"=74.5mb */

    struct AdfDevice * hd = adfDevCreate ( "dump", tmpdevname, 980, 10, 17 );
    if (!hd) {
        fprintf(stderr, "can't mount device\n");
        status = 1;
        goto cleanup_lib;
    }

    showDevInfo( hd );

    struct AdfPartition ** const partList =
        (struct AdfPartition **) malloc( sizeof(struct AdfPartition *) * 2 );
    if ( partList == NULL ) {
        fprintf( stderr, "malloc error\n" );
        status = 1;
        goto cleanup_lib;
    }

    struct AdfPartition part1;
    part1.startCyl =2;
	part1.lenCyl = 100;
	part1.volName = strdup("b");
    part1.volType = ADF_DOSFS_FFS | ADF_DOSFS_DIRCACHE;
	
    struct AdfPartition part2;
    part2.startCyl =101;
	part2.lenCyl = 878;
	part2.volName = strdup("h");
    part2.volType = ADF_DOSFS_FFS;

    partList[0] = &part1;
    partList[1] = &part2;

    ADF_RETCODE rc = adfCreateHd( hd, 2, (const struct AdfPartition * const * const) partList );
    free(partList);
    free(part1.volName);
    free(part2.volName);
    if ( rc != ADF_RC_OK ) {
        fprintf ( stderr, "adfCreateHd returned error %d\n", rc );
        status = 1;
        goto cleanup_dev;
    }

    struct AdfVolume * const vol = adfVolMount( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        fprintf(stderr, "can't mount volume\n");
        status = 1;
        goto cleanup_dev;
    }
    struct AdfVolume * const vol2 = adfVolMount( hd, 1, ADF_ACCESS_MODE_READWRITE );
    if (!vol2) {
        fprintf(stderr, "can't mount volume\n");
        adfVolUnMount(vol);
        status = 1;
        goto cleanup_dev;
    }

    showVolInfo( vol );
    showVolInfo( vol2 );

    adfVolUnMount(vol);
    adfVolUnMount(vol2);
    adfDevUnMount ( hd );
    adfDevClose ( hd );


    /* mount the created device */
    hd = adfDevOpen ( tmpdevname, ADF_ACCESS_MODE_READWRITE );
    if ( ! hd ) {
        fprintf ( stderr, "Cannot open file/device '%s' - aborting...\n",
                  tmpdevname );
        status = 1;
        goto cleanup_lib;
    }

    rc = adfDevMount ( hd );
    if ( rc != ADF_RC_OK ) {
        fprintf(stderr, "can't mount device\n");
        status = 1;
        goto cleanup_dev;
    }

    showDevInfo( hd );

cleanup_dev:
    adfDevUnMount ( hd );
    adfDevClose ( hd );

cleanup_lib:
    adfLibCleanUp();

    return status;
}
