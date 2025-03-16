
#include "common.h"

#include <adflib.h>
#include <stdio.h>
#include <stdlib.h>


void showDevInfo( struct AdfDevice * const dev )
{
    const char * const devInfo = adfDevGetInfo( dev );
    printf( "%s", devInfo );
    free( devInfo );
}

void showVolInfo( struct AdfVolume * const vol )
{
    const char * const volInfo = adfVolGetInfo( vol );
    printf( "%s", volInfo );
    free( volInfo );
}

void showEntryInfo( const struct AdfEntry * const  entry )
{
    char * const info = adfEntryGetInfo( entry );
    printf( "%s", info );
    free( info );
}