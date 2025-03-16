/*
 * bootdisk.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include"adflib.h"
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

    struct AdfDevice *hd;
    struct AdfVolume *vol;
    FILE* boot;
    unsigned char bootcode[1024];
 
    boot=fopen(argv[1],"rb");
    if (!boot) {
        log_error( "can't open bootcode file\n");
        exit(1);
    }
    fread(bootcode, sizeof(unsigned char), 1024, boot);
    fclose(boot);

    adfEnvInitDefault();

    hd = adfMountDev ( argv[2], ADF_ACCESS_MODE_READWRITE );
    if (!hd) {
        log_error( "can't mount device\n" );
        adfEnvCleanUp(); exit(1);
    }
	
    vol = adfVolMount ( hd, 0, ADF_ACCESS_MODE_READWRITE );
    if (!vol) {
        log_error( "can't mount volume\n" );
        adfUnMountDev(hd);
        adfCloseDev(hd);
        adfEnvCleanUp(); exit(1);
    }

    adfVolInstallBootBlock ( vol, bootcode );

    showVolInfo( vol );

    adfVolUnMount ( vol );
    adfUnMountDev(hd);

    adfEnvCleanUp();

    return 0;
}
