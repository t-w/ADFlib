/*
 * linux/adf_dev_driver_nativ.c
 *
 * $Id$
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
 *  along with Foobar; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <fcntl.h>
//#include <libgen.h>
#include <linux/fs.h>
#include <linux/hdreg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "adf_err.h"
#include "adf_dev_driver_nativ.h"
#include "adf_dev_type.h"
#include "adf_env.h"


struct AdfNativeDevice {
    int fd;
};

static bool adfLinuxIsBlockDevice( const char * const  devName );

/*
 * adfLinuxInitDevice
 *
 */
static struct AdfDevice * adfLinuxInitDevice( const char * const   name,
                                              const AdfAccessMode  mode )
{
    if ( ! adfLinuxIsBlockDevice( name ) ) {
        //adfEnv.vFct("%s: %s is not a linux block device", __func__, name );
        return NULL;
    }

    struct AdfDevice * dev =
        ( struct AdfDevice * ) malloc( sizeof ( struct AdfDevice ) );
    if ( dev == NULL ) {
        adfEnv.eFct("%s: malloc error", __func__ );
        return NULL;
    }

    dev->readOnly = ( mode != ADF_ACCESS_MODE_READWRITE );

    dev->drvData = malloc( sizeof ( struct AdfNativeDevice ) );
    if ( dev->drvData == NULL ) {
        adfEnv.eFct( "%s: malloc data error",  __func__ );
        free( dev );
        return NULL;
    }

    int * const fd = &( ( (struct AdfNativeDevice *) dev->drvData )->fd );

    if ( ! dev->readOnly ) {
        *fd = open( name, O_RDWR );
        dev->readOnly = ( *fd < 0 ) ? true : false;
    }

    if ( dev->readOnly ) {
        *fd = open( name, O_RDONLY );
    }

    if ( *fd < 0 ) {
        adfEnv.eFct( "%s: cannot open device",  __func__ );
        free( dev->drvData );
        free( dev );
        return NULL;
    }

    unsigned long long size = 0;

    /*
    unsigned long blocks = 0;
    if ( ioctl ( nDev->fd, BLKGETSIZE, &blocks ) < 0 ) {
        // fall-back to lseek
        size = ( unsigned long ) lseek ( nDev->fd, 0, SEEK_END );
        lseek ( nDev->fd, 0, SEEK_SET );
    } else {
        size = blocks * 512;
    }
    */
    if ( ioctl( *fd, BLKGETSIZE64, &size ) < 0 ) {
        // fall-back to lseek
        size = ( unsigned long long ) lseek( *fd, 0, SEEK_END );
        lseek( *fd, 0, SEEK_SET );
    }

    dev->size = (uint32_t) size;
    
    // https://docs.kernel.org/userspace-api/ioctl/hdio.html
    struct hd_geometry geom;
    if ( ioctl( *fd, HDIO_GETGEO, &geom ) == 0 ) {
        //adfEnv.vFct( "%s: geometry from ioctl: cylinders %u, heads %u, sectors %u\n",
        //             __func__, geom.cylinders, geom.heads, geom.sectors );
        dev->geometry.cylinders = geom.cylinders;
        dev->geometry.heads     = geom.heads;
        dev->geometry.sectors   = geom.sectors;

        adfEnv.vFct( "%s: geometry read from the device", __func__ );
    } else {
        // no data from hardware, so guessing (is whatever matches the size OK?)
        dev->geometry.cylinders = dev->size / 512;
        dev->geometry.heads     = 1;
        dev->geometry.sectors   = 1;

        adfEnv.vFct( "%s: geometry calculated from the device size", __func__ );
    }
    adfEnv.vFct( "%s: geometry: cylinders %u, heads %u, sectors %u",
                 __func__, dev->geometry.cylinders, dev->geometry.heads,
                 dev->geometry.sectors );

    dev->type  = adfDevGetTypeByGeometry( &dev->geometry );
    dev->class = ( dev->type != ADF_DEVTYPE_UNKNOWN ) ?
        adfDevTypeGetClass( dev->type ) :
        adfDevGetClassBySize( dev->size );

    dev->nVol    = 0;
    dev->volList = NULL;
    dev->mounted = false;
    dev->name    = strdup( name );
    dev->drv     = &adfDeviceDriverNative;

    return dev;
}


/*
 * adfLinuxReleaseDevice
 *
 * free native device
 */
static ADF_RETCODE adfLinuxReleaseDevice( struct AdfDevice * const  dev )
{
    close( ( (struct AdfNativeDevice *) dev->drvData )->fd );
    free( dev->drvData );
    free( dev->name );
    free( dev );
    return ADF_RC_OK;
}


/*
 * adfLinuxReadSector
 *
 */
static ADF_RETCODE adfLinuxReadSector( const struct AdfDevice * const  dev,
                                       const uint32_t                  n,
                                       const unsigned                  size,
                                       uint8_t * const                 buf )
{
    const int fd = ( (struct AdfNativeDevice *) dev->drvData )->fd;

    off_t offset = n * 512;
    if ( lseek( fd, offset, SEEK_SET ) != offset ) {
        return ADF_RC_ERROR;
    }

    if ( read( fd, buf, (size_t) size ) != (ssize_t) size )
        return ADF_RC_ERROR;

    return ADF_RC_OK;   
}


/*
 * adfLinuxWriteSector
 *
 */
static ADF_RETCODE adfLinuxWriteSector( const struct AdfDevice * const  dev,
                                        const uint32_t                  n,
                                        const unsigned                  size,
                                        const uint8_t * const           buf )
{
    const int fd = ( (struct AdfNativeDevice *) dev->drvData )->fd;

    off_t offset = n * 512;
    if ( lseek( fd, offset, SEEK_SET ) != offset ) {
        return ADF_RC_ERROR;
    }

    if ( write( fd, (void *) buf, (size_t) size ) != size )
        return ADF_RC_ERROR;

    return ADF_RC_OK;
}


/*
 * adfLinuxIsDevNative
 *
 */
static bool adfLinuxIsDevNative( void )
{
    return true;
}


/*
 * adfLinuxIsBlockDevice
 *
 */
static bool adfLinuxIsBlockDevice( const char * const  devName )
{
    //return ( strncmp ( devName, "/dev/", 5 ) == 0 );

    struct stat sb;
    if ( lstat( devName, &sb ) == -1 ) {
        adfEnv.eFct( "%s: lstat '%s' failed",  __func__, devName );
        return false;
    }

    return ( ( sb.st_mode & S_IFMT ) == S_IFBLK );
}


const struct AdfDeviceDriver  adfDeviceDriverNative = {
    .name        = "native linux",
    .data        = NULL,
    .createDev   = NULL,
    .openDev     = adfLinuxInitDevice,
    .closeDev    = adfLinuxReleaseDevice,
    .readSector  = adfLinuxReadSector,
    .writeSector = adfLinuxWriteSector,
    .isNative    = adfLinuxIsDevNative,
    .isDevice    = adfLinuxIsBlockDevice
};
