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

#include "adf_blk_hd.h"
#include "adf_dev_drivers.h"
#include "adf_dev_flop.h"
#include "adf_dev_hd.h"
#include "adf_dev_hdfile.h"
#include "adf_env.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


static struct AdfDevice * adfDevOpenWithDrv_(
    const struct AdfDeviceDriver * const  driver,
    const char * const                    name,
    const AdfAccessMode                   mode );

static ADF_RETCODE adfDevSetCalculatedGeometry_( struct AdfDevice * const  dev );
static bool adfDevIsGeometryValid_( const struct AdfDevice * const  dev );


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
 * adfDevType
 *
 * returns the type of a device
 * only based of the field 'dev->size'
 */
int adfDevType( const struct AdfDevice * const  dev )
{
    if ( ( dev->size == 512 * 11 * 2 * 80 ) ||		/* BV */
         ( dev->size == 512 * 11 * 2 * 81 ) ||		/* BV */
         ( dev->size == 512 * 11 * 2 * 82 ) || 	/* BV */
         ( dev->size == 512 * 11 * 2 * 83 ) )		/* BV */
        return ADF_DEVTYPE_FLOPDD;
    else if ( dev->size == 512 * 22 * 2 * 80 )
        return ADF_DEVTYPE_FLOPHD;
    else if ( dev->size > 512 * 22 * 2 * 80 )
        return ADF_DEVTYPE_HARDDISK;
    else {
        adfEnv.eFct("%: unknown device type", __func__ );
        return -1;
    }
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

    switch( dev->devType ) {

    case ADF_DEVTYPE_FLOPDD:
    case ADF_DEVTYPE_FLOPHD: {
        rc = adfMountFlop ( dev );
        if ( rc != ADF_RC_OK )
            return rc;
        }
        break;

    case ADF_DEVTYPE_HARDDISK:
    case ADF_DEVTYPE_HARDFILE: {
        uint8_t buf[512];
        rc = adfDevReadBlock( dev, 0, 512, buf );
        if ( rc != ADF_RC_OK ) {
            adfEnv.eFct( "%s: reading block 0 of %s failed", __func__, dev->name );
            return rc;
        }

        if ( strncmp( "RDSK", (char *) buf, 4 ) == 0 ) {
            dev->devType = ADF_DEVTYPE_HARDDISK;
            rc = adfMountHd( dev );
        } else {
            dev->devType = ADF_DEVTYPE_HARDFILE;
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
                             const uint32_t                  pSect,
                             const uint32_t                  size,
                             uint8_t * const                 buf )
{
/*  printf("pSect R =%ld\n",pSect);
    ADF_RETCODE rc = dev->drv->readSector ( dev, pSect, size, buf );
    printf("rc=%ld\n",rc);
    return rc;
*/
    return dev->drv->readSector( dev, pSect, size, buf );
}

/*
 * adfDevWriteBlock
 *
 */
ADF_RETCODE adfDevWriteBlock( const struct AdfDevice * const  dev,
                              const uint32_t                  pSect,
                              const uint32_t                  size,
                              const uint8_t * const           buf )
{
/*printf("nativ=%d\n",dev->isNativeDev);*/
    return dev->drv->writeSector( dev, pSect, size, buf );
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

    dev->devType = adfDevType( dev );

    if ( ! dev->drv->isNative() ) {
        if ( adfDevSetCalculatedGeometry_( dev ) != ADF_RC_OK ) {
            adfEnv.eFct( " %s: setting calc. geometry failed, dev. name '%s'",
                         __func__, name );
            dev->drv->closeDev( dev );
            return NULL;
        }
    }

    if ( ! adfDevIsGeometryValid_( dev ) ) {
        adfEnv.eFct( "%s: invalid geometry: cyliders %u, "
                     "heads: %u, sectors: %u, size: %u, device: %s",
                     __func__, dev->cylinders,
                     dev->heads, dev->sectors, dev->size, dev->name );
        dev->drv->closeDev( dev );
        return NULL;
    }

    if ( dev->devType == ADF_DEVTYPE_HARDDISK ) {
        ADF_RETCODE rc = adfDevUpdateGeometryFromRDSK( dev );
        if ( rc != ADF_RC_OK ) {
            dev->drv->closeDev( dev );
            return NULL;
        }
    }

    return dev;
}


static ADF_RETCODE adfDevSetCalculatedGeometry_( struct AdfDevice * const  dev )
{
    /* set geometry (based on already set size) */
    switch ( dev->devType ) {
    case ADF_DEVTYPE_FLOPDD:
        dev->heads     = 2;
        dev->sectors   = 11;
        dev->cylinders = dev->size / ( dev->heads * dev->sectors * 512 );
        if ( dev->cylinders < 80 || dev->cylinders > 83 ) {
            adfEnv.eFct( "%s: invalid size %u", __func__, dev->size );
            return ADF_RC_ERROR;
        }
        break;

    case ADF_DEVTYPE_FLOPHD:
        dev->heads     = 2;
        dev->sectors   = 22;
        dev->cylinders = dev->size / ( dev->heads * dev->sectors * 512 );
        if ( dev->cylinders != 80 ) {
            adfEnv.eFct( "%s: invalid size %u", __func__, dev->size );
            return ADF_RC_ERROR;
        }
        break;

    case ADF_DEVTYPE_HARDDISK:
    case ADF_DEVTYPE_HARDFILE:
        //dev->heads = 2;
        //dev->sectors   = dev->size / 4;
        //dev->cylinders = dev->size / ( dev->sectors * dev->heads * 512 );
        dev->heads     = 1;
        dev->sectors   = 1;
        dev->cylinders = dev->size / 512;
        break;

    default:
        adfEnv.eFct( "%s: invalid dev type %d", __func__, dev->devType );
        return ADF_RC_ERROR;
    }

    const uint32_t sizeFromGeometry = dev->cylinders * dev->heads * dev->sectors * 512;
    if ( sizeFromGeometry != dev->size ) {
        adfEnv.wFct( "%s: size from geometry %u != device size %u\n"
                     "(%u bytes left over geometry)\n"
                     "This is most likely due to size of the device (%u) not divisible\n"
                     "by sector size (512): real size(%u) %% 512 = %u bytes left unused\n",
                     __func__, sizeFromGeometry, dev->size,
                     dev->size % 512,
                     dev->size, dev->size, dev->size % 512 );
        assert( dev->size % 512 == dev->size - sizeFromGeometry );
    }

    return ADF_RC_OK;
}


static bool adfDevIsGeometryValid_( const struct AdfDevice * const  dev )
{
    const uint32_t sizeFromGeometry = dev->cylinders * dev->heads * dev->sectors * 512;
    //return ( dev->cylinders > 0 &&
    //         dev->heads > 0    &&
    //         dev->sectors > 0
    //         && sizeFromGeometry == dev->size );

    // what is the minimal device size? (guessing: 10 blocks(?))
    return ( sizeFromGeometry >= 512 * 10 );
}


/*
 * adfDevGetInfo
 *
 */
#define DEVINFO_SIZE 1024

char * adfDevGetInfo( const struct AdfDevice * const  dev )
{
    const char * devTypeInfo = NULL;
    switch ( dev->devType ) {
    case ADF_DEVTYPE_FLOPDD:
        devTypeInfo = "floppy dd";
        break;
    case ADF_DEVTYPE_FLOPHD:
        devTypeInfo = "floppy hd";
        break;
    case ADF_DEVTYPE_HARDDISK:
        devTypeInfo = "harddisk";
        break;
    case ADF_DEVTYPE_HARDFILE:
        devTypeInfo = "hardfile";
        break;
    default:
        devTypeInfo = "unknown device type!";
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
            dev->cylinders, dev->heads, dev->sectors );

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
