
#include "common.h"

#include <adflib.h>
#include <stdio.h>
#include <stdlib.h>


static inline void showInfo( char * const info )
{
    printf( "%s", info );
    free( info );
}

void showDevInfo( struct AdfDevice * const dev )
{
    showInfo( adfDevGetInfo( dev ) );
}

void showVolInfo( struct AdfVolume * const vol )
{
    showInfo( adfVolGetInfo( vol ) );
}

void showEntryInfo( const struct AdfEntry * const  entry )
{
    showInfo( adfEntryGetInfo( entry ) );
}
