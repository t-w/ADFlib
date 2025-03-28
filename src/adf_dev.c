/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 *
 *  adf_dev.c
 *
 *  $Id$
 *
 *  device code
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

#include "adf_dev.h"

#include "adf_dev_drivers.h"
#include "adf_dev_flop.h"
#include "adf_dev_hd.h"
#include "adf_dev_hdfile.h"
#include "adf_env.h"
#include "adf_limits.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


static struct AdfDevice * adfDevOpenWithDrv_(
    const struct AdfDeviceDriver * const  driver,
    const char * const                    name,
    const AdfAccessMode                   mode );

static ADF_RETCODE adfDevSetCalculatedGeometry_( struct AdfDevice * const  dev );


/*****************************************************************************
 *
 * Public functions
 *
 *****************************************************************************/

/*
 * adfDevCreate
 *
 */
struct AdfDevice * adfDevCreate(  const char * const  driverName,
                                  const char * const  name,
                                  const uint32_t      cylinders,
                                  const uint32_t      heads,
                                  const uint32_t      sectors )
{
    uint32_t sizeBlocks = cylinders * heads * sectors;
    if ( sizeBlocks > ADF_DEV_SIZE_MAX_BLOCKS ) {
        adfEnv.eFct( " %s: size %u blocks is bigger than max. %u blocks",
                     sizeBlocks, ADF_DEV_SIZE_MAX_BLOCKS );
        return NULL;
    }

    const struct AdfDeviceDriver * const  driver = adfGetDeviceDriverByName( driverName );
    if ( driver == NULL || driver->createDev == NULL )
        return NULL;
    return driver->createDev( name, cylinders, heads, sectors );
}


/*
 * adfDevOpen
 *
 */
struct AdfDevice * adfDevOpen( const char * const   name,
                               const AdfAccessMode  mode )
{
    return adfDevOpenWithDrv_( adfGetDeviceDriverByDevName( name ), name, mode );
}

/*
 * adfDevOpenWithDriver
 *
 */
struct AdfDevice * adfDevOpenWithDriver( const char * const   driverName,
                                         const char * const   name,
                                         const AdfAccessMode  mode )
{
    return adfDevOpenWithDrv_( adfGetDeviceDriverByName( driverName ), name, mode );
}


/*
 * adfDevClose
 *
 * Closes/releases an opened device.
 */
void adfDevClose( struct AdfDevice * const  dev )
{
    if ( dev == NULL )
        return;

    if ( dev->mounted )
        adfDevUnMount( dev );

    dev->drv->closeDev( dev );
}


/*
 * adfDevMount
 *
 * mount a dump file (.adf) or a real device (uses adf_nativ.c and .h)
 *
 * adfInitDevice() must fill dev->size !
 */
ADF_RETCODE adfDevMount( struct AdfDevice * const  dev )
{
    if ( dev == NULL )
        return ADF_RC_ERROR;

    ADF_RETCODE rc;

    switch( dev->class ) {

    case ADF_DEVCLASS_FLOP: {
        rc = adfMountFlop ( dev );
        if ( rc != ADF_RC_OK )
            return rc;
        }
        break;

    case ADF_DEVCLASS_HARDDISK:
    case ADF_DEVCLASS_HARDFILE: {
#ifdef _MSC_VER
        uint8_t* const buf = _alloca(dev->geometry.blockSize);
#else
        uint8_t buf[ dev->geometry.blockSize ];
#endif
        rc = adfDevReadBlock( dev, 0, dev->geometry.blockSize, buf );
        if ( rc != ADF_RC_OK ) {
            adfEnv.eFct( "%s: reading block 0 of %s failed", __func__, dev->name );
            return rc;
        }

        if ( strncmp( "RDSK", (char *) buf, 4 ) == 0 ) {
            dev->class = ADF_DEVCLASS_HARDDISK;
            rc = adfMountHd( dev );
        } else {
            dev->class = ADF_DEVCLASS_HARDFILE;
            rc = adfMountHdFile( dev );
        }

        if ( rc != ADF_RC_OK )
            return rc;
        }
        break;

    default:
        adfEnv.eFct("%s: unknown device type", __func__ );
        return ADF_RC_ERROR;								/* BV */
    }

    dev->mounted = true;
    return ADF_RC_OK;
}

/*
 * adfDevUnMount
 *
 */
void adfDevUnMount( struct AdfDevice * const  dev )
{
    if ( ! dev->mounted )
        return;

    // free volume list
    //if ( dev->volList ) {
    if ( dev->nVol > 0 ) {
        for ( int i = 0 ; i < dev->nVol ; i++ ) {
            free ( dev->volList[i]->volName );
            free ( dev->volList[i] );
        }
        free ( dev->volList );
        dev->nVol = 0;
    }

    dev->volList = NULL;
    dev->mounted = false;
}

/*
 * adfDevReadBlock
 *
 */
ADF_RETCODE adfDevReadBlock( const struct AdfDevice * const  dev,
                             uint32_t                        pSect,
                             const uint32_t                  size,
                             uint8_t * const                 buf )
{
    const unsigned nFullBlocks = size / dev->geometry.blockSize;
    ADF_RETCODE rc = dev->drv->readSectors( dev, pSect, nFullBlocks, buf );
    if ( rc != ADF_RC_OK )
        return rc;

    const unsigned remainder = size % dev->geometry.blockSize;
    if ( remainder != 0 ) {
#ifdef _MSC_VER
        uint8_t* const blockBuf = _alloca(dev->geometry.blockSize);
#else
        uint8_t blockBuf[ dev->geometry.blockSize ];
#endif
        ADF_RETCODE rc = dev->drv->readSector( dev, pSect + nFullBlocks, blockBuf );
        if ( rc != ADF_RC_OK )
            return rc;
        memcpy( buf + size - remainder, blockBuf, remainder );
    }

    return ADF_RC_OK;
}

/*
 * adfDevWriteBlock
 *
 */
ADF_RETCODE adfDevWriteBlock( const struct AdfDevice * const  dev,
                              uint32_t                        pSect,
                              const uint32_t                  size,
                              const uint8_t * const           buf )
{
    const unsigned nFullBlocks = size / dev->geometry.blockSize;
    ADF_RETCODE rc = dev->drv->writeSectors( dev, pSect, nFullBlocks, buf );
    if ( rc != ADF_RC_OK )
        return rc;

    const unsigned remainder = size % dev->geometry.blockSize;
    if ( remainder != 0 ) {
#ifdef _MSC_VER
        uint8_t * const blockBuf = _alloca( dev->geometry.blockSize );
#else
        uint8_t blockBuf[dev->geometry.blockSize];
#endif
        memcpy( blockBuf, buf + size - remainder, remainder );
        memset( blockBuf + remainder, 0, dev->geometry.blockSize - remainder );
        ADF_RETCODE rc = dev->drv->writeSector( dev, pSect + nFullBlocks, blockBuf );
        if ( rc != ADF_RC_OK )
            return rc;
    }

    return ADF_RC_OK;
}


/*****************************************************************************
 *
 * Private / lower-level functions
 *
 *****************************************************************************/

static struct AdfDevice * adfDevOpenWithDrv_(
    const struct AdfDeviceDriver * const  driver,
    const char * const                    name,
    const AdfAccessMode                   mode )
{
    if ( driver == NULL || driver->openDev == NULL )
        return NULL;

    struct AdfDevice * const  dev = driver->openDev( name, mode );
    if ( dev == NULL ) {
        adfEnv.eFct( " %s: openDev failed, dev. name '%s'", __func__, name );
        return NULL;
    }

    dev->class = adfDevGetClassBySizeBlocks( dev->sizeBlocks );

    if ( ! dev->drv->isNative() ) {
        if ( adfDevSetCalculatedGeometry_( dev ) != ADF_RC_OK ) {
            adfEnv.eFct( " %s: setting calc. geometry failed, dev. name '%s'",
                         __func__, name );
            dev->drv->closeDev( dev );
            return NULL;
        }
    }

    if ( ! adfDevIsGeometryValid( &dev->geometry, dev->sizeBlocks ) ) {
        if ( dev->drv->isNative()
             || dev->type != ADF_DEVTYPE_UNKNOWN )   // from the predefined list should be valid...
        {
            adfEnv.eFct( "%s: invalid geometry: cyliders %u, "
                         "heads: %u, sectors: %u, size (in blocks): %u, device: %s",
                         __func__, dev->geometry.cylinders, dev->geometry.heads,
                         dev->geometry.sectors, dev->sizeBlocks, dev->name );
            dev->drv->closeDev( dev );
            return NULL;
        } /*else {
            // only a warning (many exsting HDFs are like this...)
            const uint32_t sizeFromGeometry = dev->geometry.cylinders *
                                              dev->geometry.heads *
                                              dev->geometry.sectors * 512;
            adfEnv.wFct( "%s: size from geometry %u != device size %u\n"
                     "(%u bytes left over geometry)\n"
                     "This is most likely due to size of the device (%u) not divisible\n"
                     "by sector size (512): real size(%u) %% 512 = %u bytes left unused\n",
                     __func__, sizeFromGeometry, dev->size,
                     dev->size % 512,
                     dev->size, dev->size, dev->size % 512 );
            //assert( dev->size % 512 == dev->size - sizeFromGeometry );
            }*/
    }

    if ( dev->class == ADF_DEVCLASS_HARDDISK ) {
        ADF_RETCODE rc = adfDevUpdateGeometryFromRDSK( dev );
        if ( rc != ADF_RC_OK ) {
            dev->drv->closeDev( dev );
            return NULL;
        }
    }

    // update device and type after having final geometry set
    dev->type  = adfDevGetTypeByGeometry( &dev->geometry );
    dev->class = ( dev->type != ADF_DEVTYPE_UNKNOWN ) ?
        adfDevTypeGetClass( dev->type ) :
        adfDevGetClassBySizeBlocks( dev->sizeBlocks );

    return dev;
}


static ADF_RETCODE adfDevSetCalculatedGeometry_( struct AdfDevice * const  dev )
{
    // set geometry (based on already set size)

    // first - check predefined types
    dev->type = adfDevGetTypeBySizeBlocks( dev->sizeBlocks );
    if ( dev->type != ADF_DEVTYPE_UNKNOWN ) {
        dev->geometry = adfDevTypeGetGeometry( dev->type );
        return ADF_RC_OK;
    }

    // if not found on the predefined list - guess something reasonable...
    if ( dev->class == ADF_DEVCLASS_HARDDISK ||
         dev->class == ADF_DEVCLASS_HARDFILE )
    {
        // partitions must be aligned with cylinders(tracks) - this gives most
        // flexibility
        dev->geometry.cylinders = dev->sizeBlocks;
        dev->geometry.heads     = 1;
        dev->geometry.sectors   = 1;
    } else {
        adfEnv.eFct( "%s: invalid dev class %d", __func__, dev->class );
        return ADF_RC_ERROR;
    }

    return ADF_RC_OK;
}


/*
 * adfDevGetInfo
 *
 */
#define DEVINFO_SIZE 1024

char * adfDevGetInfo( const struct AdfDevice * const  dev )
{
    const char * devTypeInfo = NULL;
    if ( dev->type != ADF_DEVTYPE_UNKNOWN ) {
        devTypeInfo = adfDevTypeGetDescription( dev->type );
    } else {
        switch ( dev->class ) {
            // floppies must be covered above

        case ADF_DEVCLASS_HARDDISK:
            devTypeInfo = "harddisk";
            break;
        case ADF_DEVCLASS_HARDFILE:
            devTypeInfo = "hardfile";
            break;
        default:
            devTypeInfo = "unknown device type!";
        }
    }

    char * const info = malloc( DEVINFO_SIZE + 1 );
    if ( info == NULL )
        return NULL;

    char *infoptr = info;
    infoptr += snprintf( infoptr, (size_t)( DEVINFO_SIZE - ( infoptr - info ) ),
                         "\nADF device info:\n  Type:\t\t%s\n  Driver:\t%s\n",
                         devTypeInfo, dev->drv->name );

    infoptr += snprintf( infoptr, (size_t)( DEVINFO_SIZE - ( infoptr - info ) ),
                         "  Geometry:\n"
            "    Cylinders\t%d\n"
            "    Heads\t%d\n"
            "    Sectors\t%d\n\n",
            dev->geometry.cylinders, dev->geometry.heads, dev->geometry.sectors );

    infoptr += snprintf(
        infoptr, (size_t) ( DEVINFO_SIZE - ( infoptr - info ) ),
        "  Volumes:\t%d%s\n", dev->nVol,
        dev->nVol > 0 ? "\n   idx  first bl.     last bl.    filesystem    name" : "" );

    for ( int i = 0 ; i < dev->nVol ; i++ ) {
        const struct AdfVolume * const vol = dev->volList[i];
        const char * const fstype = ( adfVolIsDosFS( vol ) ) ?
            ( adfVolIsOFS( vol ) ? "OFS" : "FFS" ) : "???";
        infoptr += snprintf(
            infoptr, (size_t)( DEVINFO_SIZE - ( infoptr - info ) ),
            "    %2d  %9d    %9d    %s(%s)      \"%s\"", i,
                vol->firstBlock,
                vol->lastBlock,
                adfVolIsFsValid(vol) ? vol->fs.id : "???",
                fstype,
                vol->volName ? vol->volName : "" );
        if ( vol->mounted )
            infoptr += snprintf( infoptr, (size_t)( DEVINFO_SIZE - ( infoptr - info ) ),
                                 "    mounted");
        infoptr += snprintf( infoptr, (size_t)( DEVINFO_SIZE - ( infoptr - info ) ),
                             "\n");
    }
    infoptr += snprintf( infoptr, (size_t)( DEVINFO_SIZE - ( infoptr - info ) ),
                         "\n");
    assert( infoptr - info < DEVINFO_SIZE );
    return info;
}
