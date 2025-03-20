/*
 *  ADF Library. (C) 1997-2022 Laurent Clevy
 *               (C) 2023-2025 Tomasz Wolak
 *
 * adflib.c
 *
 *  $Id$
 *
 * Library's init, clean-up and internal checks.
 *
 *  This file is part of ADFLib.
 *
 *  ADFLib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  ADFLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ADFLib; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "adflib.h"

#include "adf_dev_drivers.h"
#include "adf_dev_driver_dump.h"
#include "adf_dev_driver_ramdisk.h"

#include <stdlib.h>

static void checkInternals(void);


char * adfGetVersionNumber(void) {
    return ADFLIB_VERSION;
}

char * adfGetVersionDate(void) {
    return ADFLIB_DATE;
}


ADF_RETCODE adfLibInit(void)
{
    //ADF_RETCODE status = checkInternals();
    //if ( status != ADF_RC_OK )
    //    return status;
    checkInternals();

    adfEnvInitDefault();
    adfAddDeviceDriver( &adfDeviceDriverDump );
    adfAddDeviceDriver( &adfDeviceDriverRamdisk );

    return ADF_RC_OK;
}

void adfLibCleanUp(void)
{
    adfRemoveDeviceDrivers();
    adfEnvCleanUp();
}


/****************************************************************************
 * Internal checks
 ****************************************************************************/

union u {
    int32_t l;
    char    c[ 4 ];
};

static void assertInternal( bool                cnd,
                            const char * const  msg )
{
    if ( ! cnd ) {
        fputs( msg, stderr );
        exit( 1 );
    }
}

static void checkInternals(void)
{
/*    char str[80];*/
    union u val;

    /* internal checking */

    assertInternal( sizeof(short) == 2,
                    "Compilation error : sizeof(short) != 2\n" );

    assertInternal( sizeof(int32_t) == 4,
                    "Compilation error : sizeof(int32_t) != 4\n" );

    assertInternal( sizeof(struct AdfEntryBlock) == 512,
                    "Internal error : sizeof(struct AdfEntryBlock) != 512\n");

    assertInternal( sizeof(struct AdfRootBlock) == 512,
                    "Internal error : sizeof(struct AdfRootBlock) != 512\n");

    assertInternal( sizeof(struct AdfDirBlock) == 512,
                    "Internal error : sizeof(struct AdfDirBlock) != 512\n");

    assertInternal( sizeof(struct AdfBootBlock) == 1024,
                    "Internal error : sizeof(struct AdfBootBlock) != 1024\n" );

    assertInternal( sizeof(struct AdfFileHeaderBlock) == 512,
                    "Internal error : sizeof(struct AdfFileHeaderBlock) != 512\n" );

    assertInternal( sizeof(struct AdfFileExtBlock) == 512,
                    "Internal error : sizeof(struct AdfFileExtBlock) != 512\n" );

    assertInternal( sizeof(struct AdfOFSDataBlock) == 512,
                    "Internal error : sizeof(struct AdfOFSDataBlock) != 512\n" );

    assertInternal( sizeof(struct AdfBitmapBlock) == 512,
                    "Internal error : sizeof(struct AdfBitmapBlock) != 512\n" );

    assertInternal( sizeof(struct AdfBitmapExtBlock) == 512,
                    "Internal error : sizeof(struct AdfBitmapExtBlock) != 512\n" );

    assertInternal( sizeof(struct AdfLinkBlock) == 512,
                    "Internal error : sizeof(struct AdfLinkBlock) != 512\n" );

    val.l = 1L;
/* if LITT_ENDIAN not defined : must be BIG endian */
#ifndef LITT_ENDIAN
    assertInternal( val.c[3] == 1, /* little endian : LITT_ENDIAN must be defined ! */
                    "Compilation error : LITT_ENDIAN not defined\n" );
#else
    assertInternal( val.c[3] != 1, /* big endian : LITT_ENDIAN must not be defined ! */
                    "Compilation error : LITT_ENDIAN defined\n" );
#endif
}
