
#include "common.h"

#include <adflib.h>
#include <stdio.h>
#include <stdlib.h>

void showVolInfo( struct AdfVolume * const vol )
{
    const char * const volInfo = adfVolGetInfo( vol );
    printf( "%s", volInfo );
    free( volInfo );
}
