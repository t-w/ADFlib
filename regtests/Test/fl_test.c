/*
 * fl_test.c
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

    struct AdfVolume *vol;
    FILE* boot;
    unsigned char bootcode[1024];
 
    adfEnvInitDefault();

//	adfEnvSetFct(0,0,MyVer,0);

    /* open and mount existing device */
    struct AdfDevice * hd = adfDevOpen ( argv[1], ADF_ACCESS_MODE_READWRITE );
    if ( ! hd ) {
        log_error( "Cannot open file/device '%s' - aborting...\n", argv[1] );
        adfEnvCleanUp();
        exit(1);
    }

    ADF_RETCODE rc = adfDevMount ( hd );
    if ( rc != ADF_RC_OK ) {
        log_error( "can't mount device\n" );
        adfDevClose(hd);
        adfEnvCleanUp(); exit(1);
    }

    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        log_error( "can't mount volume\n" );
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        adfEnvCleanUp(); exit(1);
    }

    showVolInfo( vol );

    adfVolUnMount(vol);
    adfDevUnMount ( hd );
    adfDevClose ( hd );

    log_info("\n");

    /* create and mount one device */
    hd = adfDevCreate ( "dump", "fl_test-newdev", 80, 2, 11 );
    if (!hd) {
        log_error( "can't create device\n" );
        adfEnvCleanUp(); exit(1);
    }

    showDevInfo( hd );

    adfCreateFlop ( hd, "empty", ADF_DOSFS_FFS |
                                 ADF_DOSFS_DIRCACHE );

    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        log_error( "can't mount volume\n" );
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        adfEnvCleanUp(); exit(1);
    }

    boot=fopen(argv[2],"rb");
    if (!boot) {
        log_error( "can't open bootblock file\n" );
        adfDevUnMount ( hd );
        adfDevClose ( hd );
        adfEnvCleanUp(); exit(1);
    }
    fread(bootcode, sizeof(unsigned char), 1024, boot);
    adfVolInstallBootBlock ( vol, bootcode );
    fclose(boot);

    showVolInfo( vol );

    adfVolUnMount(vol);
    adfDevUnMount ( hd );
    adfDevClose ( hd );

    adfEnvCleanUp();

    return 0;
}
