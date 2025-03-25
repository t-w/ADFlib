/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 *
 *  adf_vol.c
 *
 *  $Id$
 *
 * logical disk/volume code
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


#include "adf_vol.h"

#include "adf_bitm.h"
#include "adf_cache.h"
#include "adf_dev.h"
#include "adf_env.h"
#include "adf_raw.h"
#include "adf_util.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>


uint32_t bitMask[ 32 ] = {
    0x1, 0x2, 0x4, 0x8,
    0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800,
    0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000,
    0x1000000, 0x2000000, 0x4000000, 0x8000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000 };

/*
 * adfVolCreate
 *
 *
 */
struct AdfVolume * adfVolCreate( struct AdfDevice * const  dev,
                                 const uint32_t            start,
                                 const uint32_t            len,
                                 const char * const        volName,
                                 const uint8_t             volType )
{
/*    struct AdfDirCacheBlock dirc;*/
    ADF_SECTNUM blkList[2];

    if ( adfEnv.useProgressBar )
        adfEnv.progressBar( 0 );

    struct AdfVolume * const vol = (struct AdfVolume *) malloc( sizeof(struct AdfVolume) );
    if ( ! vol ) {
        adfEnv.eFct( "%s: malloc vol", __func__ );
        return NULL;
    }

    vol->dev        = dev;
    vol->firstBlock = (int32_t)( dev->geometry.heads *
                                 dev->geometry.sectors * start );
    vol->lastBlock  = vol->firstBlock + (int32_t)( dev->geometry.heads *
                                                   dev->geometry.sectors * len ) - 1;
    vol->blockSize  = 512;
    vol->rootBlock  = adfVolCalcRootBlk( vol );

/*printf("first=%ld last=%ld root=%ld\n",vol->firstBlock,
 vol->lastBlock, vol->rootBlock);
*/
    vol->curDirPtr = vol->rootBlock;
    vol->readOnly  = dev->readOnly;
    vol->mounted   = true;
    vol->volName   = strndup( volName,
                              min( strlen( volName ),
                                   (unsigned) ADF_MAX_NAME_LEN ) );
    if ( vol->volName == NULL ) {
        adfEnv.eFct( "%s: malloc volName", __func__ );
        free( vol );
        return NULL;
    }

    if ( adfEnv.useProgressBar )
        adfEnv.progressBar( 25 );

    strncpy( vol->fs.id, "DOS", 3 );
    vol->fs.id[ 3 ] = '\0';
    vol->fs.type    = volType;

    struct AdfBootBlock boot;
    memset( &boot, 0, 1024 );
    //strncpy ( boot.dosType, "DOS", 3 ); /// done in adfWriteBootBlock
    boot.dosType[ 3 ] = (char) volType;
    boot.rootBlock    = vol->rootBlock;
/*printf("first=%d last=%d\n", vol->firstBlock, vol->lastBlock);
printf("name=%s root=%d\n", vol->volName, vol->rootBlock);
*/
    if ( adfWriteBootBlock( vol, &boot ) != ADF_RC_OK ) {
        free( vol->volName );
        free( vol );
        return NULL;
    }

    if ( adfEnv.useProgressBar )
        adfEnv.progressBar( 20 );

    if ( adfCreateBitmap( vol ) != ADF_RC_OK ) {
        free( vol->volName );
        free( vol );
        return NULL;
    }

    if ( adfEnv.useProgressBar )
        adfEnv.progressBar( 40 );


/*for(i=0; i<127; i++)
printf("%3d %x, ",i,vol->bitmapTable[0]->map[i]);
*/
    if ( adfDosFsHasDIRCACHE( volType ) )
        adfGetFreeBlocks( vol, 2, blkList );
    else
        adfGetFreeBlocks( vol, 1, blkList );


/*printf("[0]=%d [1]=%d\n",blkList[0],blkList[1]);*/

    struct AdfRootBlock root;
    memset( &root, 0, ADF_LOGICAL_BLOCK_SIZE );

    root.nameLen = (uint8_t) strlen( vol->volName );
    memcpy( root.diskName, volName, root.nameLen );
    adfTime2AmigaTime( adfGiveCurrentTime(), &root.coDays, &root.coMins,
                       &root.coTicks );

    /* dircache block */
    if ( adfDosFsHasDIRCACHE( volType ) ) {
        root.extension = 0L;
        root.secType   = ADF_ST_ROOT; /* needed by adfCreateEmptyCache() */
        adfCreateEmptyCache( vol, (struct AdfEntryBlock *) &root, blkList[ 1 ] );
    }

    if ( adfEnv.useProgressBar )
        adfEnv.progressBar( 60 );

    if ( adfWriteRootBlock( vol, (uint32_t) blkList[ 0 ], &root ) != ADF_RC_OK ) {
        free( vol->volName );
        free( vol );
        return NULL;
    }

   /* fills root->bmPages[] and writes filled bitmapExtBlocks */
    if ( adfWriteNewBitmap( vol ) != ADF_RC_OK )
        return NULL;

    if ( adfEnv.useProgressBar )
        adfEnv.progressBar( 80 );

    if ( adfUpdateBitmap( vol ) != ADF_RC_OK )
        return NULL;

    if ( adfEnv.useProgressBar )
        adfEnv.progressBar( 100 );
/*printf("free blocks %ld\n",adfCountFreeBlocks(vol));*/

    /* will be managed by adfMount() later */
    adfFreeBitmap( vol );

    vol->mounted = false;

    return vol;
}

/*
 * adfVolMount
 *
 * 
 */
struct AdfVolume * adfVolMount( struct AdfDevice * const  dev,
                                const int                 nPart,
                                const AdfAccessMode       mode )
{
    if ( dev == NULL ) {
        adfEnv.eFct( "%s: invalid device (NULL)", __func__ );
        return NULL;
    }

    if ( dev->nVol < 1 ) {
        adfEnv.eFct( "%s: volume list empty (none or not loaded), "
                     "(requested: device %s, volume %d)",
                     __func__, dev->name, nPart );
        return NULL;
    }

    if ( nPart < 0  ||
         nPart >= dev->nVol )
    {
        adfEnv.eFct( "%s: invalid partition %d", __func__, nPart );
        return NULL;
    }

    if ( mode != ADF_ACCESS_MODE_READONLY  &&
         mode != ADF_ACCESS_MODE_READWRITE )
    {
        adfEnv.eFct( "%s: invalid mode %d", __func__, mode );
        return NULL;
    }

    struct AdfVolume * const vol = dev->volList[ nPart ];

    if ( ! adfVolIsDosFS( vol ) ) {
        if ( adfVolIsPFS( vol ) ) {
            adfEnv.eFct( "%s: a PFS volume, not supported (device %s, volume %d)",
                         __func__, dev->name, nPart );
            return NULL;
        }
        adfEnv.eFct( "%s: filesystem not supported (device %s, volume %d)",
                     __func__, dev->name, nPart );
        return NULL;
    }

    vol->mounted = true;

/*printf("first=%ld last=%ld root=%ld\n",vol->firstBlock,
 vol->lastBlock, vol->rootBlock);
*/
    vol->datablockSize = adfVolIsOFS( vol ) ? 488 : 512;

    if ( dev->readOnly /*|| isDIRCACHE(vol->fs.type)*/)
        vol->readOnly = true;
    else
        vol->readOnly = ( mode != ADF_ACCESS_MODE_READWRITE );
	   	
    struct AdfRootBlock root;
    if ( adfReadRootBlock( vol, (uint32_t) vol->rootBlock, &root ) != ADF_RC_OK ) {
        adfEnv.eFct( "%s: invalid RootBlock, sector %u", __func__, vol->rootBlock );
        vol->mounted = false;
        return NULL;
    }

    ADF_RETCODE rc = adfBitmapAllocate( vol );
    if ( rc != ADF_RC_OK ) {
            adfEnv.eFct( "%s: adfBitmapAllocate() returned error %d, "
                         "mounting volume %s failed", __func__, rc, vol->volName );
            adfVolUnMount( vol );
            return NULL;
    }

    rc = adfReadBitmap( vol, &root );
    if ( rc != ADF_RC_OK ) {
        adfEnv.eFct( "%s: adfReadBitmap() returned error %d, "
                     "mounting volume %s failed", __func__, rc, vol->volName );
        adfVolUnMount( vol );
        return NULL;
    }

    /*
    if ( root.bmFlag != ADF_BM_VALID ) {
        if ( vol->readOnly == true ) {
            rc = adfReconstructBitmap ( vol, &root );
            if ( rc != ADF_RC_OK ) {
                adfEnv.eFct ( "adfMount : adfReconstructBitmap() returned error %d, "
                              "mounting volume %s failed", rc, vol->volName );
                adfVolUnMount ( vol );
                return NULL;
            }
        } else {
            adfEnv.eFct ( "adfMount : block allocation bitmap marked invalid in root block, "
                          "mounting the volume %s read-write not possible", vol->volName );
            adfVolUnMount ( vol );
            return NULL;
        }
    }
    */
    if ( root.bmFlag != ADF_BM_VALID )
        adfEnv.wFct( "%s: invalid bitmap on volume '%s'", __func__, vol->volName );

    vol->curDirPtr = vol->rootBlock;

/*printf("blockSize=%d\n",vol->blockSize);*/

    return vol;
}

/*
 * adfVolRemount
 *
 *
 */
ADF_RETCODE adfVolRemount( struct AdfVolume *   vol,
                           const AdfAccessMode  mode )
{
    if ( vol == NULL )
        return ADF_RC_ERROR;

    if ( ! vol->mounted )
        return ADF_RC_ERROR;

    if ( mode == ADF_ACCESS_MODE_READWRITE ) {
        if ( vol->dev->readOnly ) {
            adfEnv.eFct( "%s: device read-only, cannot mount "
                         "volume '%s' read-write", __func__, vol->volName );
            return ADF_RC_ERROR;
        }
        vol->readOnly = false;
    } else if ( mode == ADF_ACCESS_MODE_READONLY ) {
        vol->readOnly = true;
    } else {
        adfEnv.eFct( "%s: cannot remount volume %s, invalid mode %d",
                     __func__, vol->volName, mode );
        return ADF_RC_ERROR;
    }
    return ADF_RC_OK;
}

/*
 * adfVolUnMount
 *
 * free bitmap structures
 * free current dir
 */
void adfVolUnMount( struct AdfVolume * const  vol )
{
    if ( ! vol ) {
        adfEnv.eFct( "%s: vol is null", __func__ );
        return;
    }

    adfFreeBitmap( vol );

    vol->mounted = false;
}

/*
 * adfVolInstallBootBlock
 *
 */
ADF_RETCODE adfVolInstallBootBlock( struct AdfVolume * const  vol,
                                    const uint8_t * const     code )
{
    int i;
    struct AdfBootBlock boot;

    if ( vol->dev->class != ADF_DEVCLASS_FLOP )
        //&& vol->dev->ype != ADF_DEVTYPE_HDF )   also HDF????
    {
        return ADF_RC_ERROR;
    }

    ADF_RETCODE rc = adfReadBootBlock( vol, &boot );
    if ( rc != ADF_RC_OK )
        return rc;

    boot.rootBlock = 880;
    for ( i = 0; i < 1024 - 12; i++ )         /* bootcode */
        boot.data[ i ] = code[ i + 12 ];

    rc = adfWriteBootBlock( vol, &boot );
    if ( rc != ADF_RC_OK )
        return rc;

    vol->bootCode = true;

    return ADF_RC_OK;
}

/*
 * adfVolIsSectNumValid
 *
 */
bool adfVolIsSectNumValid( const struct AdfVolume * const  vol,
                           const ADF_SECTNUM               nSect )
{
    return ( nSect >= 0 &&
             nSect <= vol->lastBlock - vol->firstBlock );
}

/*
 * adfVolReadBlock
 *
 * read logical block
 */
ADF_RETCODE adfVolReadBlock( const struct AdfVolume * const  vol,
                             const uint32_t                  nSect,
                             uint8_t * const                 buf )
{
    if ( ! vol->mounted ) {
        adfEnv.eFct( "%s: volume not mounted", __func__ );
        return ADF_RC_ERROR;
    }

    /* translate logical sect to physical sect */
    unsigned pSect = nSect + (unsigned) vol->firstBlock;

    if ( adfEnv.useRWAccess )
        adfEnv.rwhAccess( (ADF_SECTNUM) pSect, (ADF_SECTNUM) nSect, false );

/*  char strBuf[80];
    printf("psect=%ld nsect=%ld\n",pSect,nSect);
    sprintf(strBuf,"ReadBlock : accessing logical block #%ld", nSect);
    adfEnv.vFct(strBuf);
*/
    if ( pSect < (unsigned) vol->firstBlock ||
         pSect > (unsigned) vol->lastBlock )
    {
        adfEnv.wFct( "%s: nSect %u out of range", __func__, nSect );
        return ADF_RC_BLOCKOUTOFRANGE;
    }

    ADF_RETCODE rc = adfDevReadBlock( vol->dev, pSect, 512, buf );
    if ( rc != ADF_RC_OK ) {
        adfEnv.eFct( "%s: error reading block %d, volume '%s'",
                     __func__, nSect, vol->volName );
    }
    return rc;
}

/*
 * adfVolWriteBlock
 *
 */
ADF_RETCODE adfVolWriteBlock( const struct AdfVolume * const  vol,
                              const uint32_t                  nSect,
                              const uint8_t * const           buf )
{
    if ( ! vol->mounted ) {
        adfEnv.eFct( "%s: volume not mounted", __func__ );
        return ADF_RC_ERROR;
    }

    if ( vol->readOnly ) {
        adfEnv.wFct( "%s: can't write block, read only volume", __func__ );
        return ADF_RC_ERROR;
    }

    unsigned pSect = nSect + (unsigned) vol->firstBlock;
/*printf("write nsect=%ld psect=%ld\n",nSect,pSect);*/

    if ( adfEnv.useRWAccess )
        adfEnv.rwhAccess( (ADF_SECTNUM) pSect, (ADF_SECTNUM) nSect, true );
 
    if ( pSect < (unsigned) vol->firstBlock ||
         pSect > (unsigned) vol->lastBlock )
    {
        adfEnv.wFct( "%s: nSect %u out of range", __func__, nSect );
        return ADF_RC_BLOCKOUTOFRANGE;
    }

    ADF_RETCODE rc = adfDevWriteBlock( vol->dev, pSect, 512, buf );
    if ( rc != ADF_RC_OK ) {
        adfEnv.eFct( "%s: error writing block %d, volume '%s'",
                     __func__, nSect, vol->volName );
    }
    return rc;
}

/*
 * adfVolGetFsStr
 *
 */
const char * adfVolGetFsStr( const struct AdfVolume * const  vol )
{
    return ( adfVolIsOFS( vol ) ? "OFS" :
             adfVolIsFFS( vol ) ? "FFS" :
             adfVolIsPFS( vol ) ? "PFS" : "???" );
}


/*
 * adfVolGetInfo
 *
 */
char * adfVolGetInfo( struct AdfVolume * const  vol )
{
    struct AdfRootBlock root;
    if ( adfReadRootBlock( vol, (uint32_t) vol->rootBlock, &root ) != ADF_RC_OK )
        return NULL;

    char diskName[ 35 ];
    memset( diskName, 0, 35 );
    memcpy( diskName, root.diskName, root.nameLen );

    #define TYPE_INFO_SIZE 64
    char typeInfo[ TYPE_INFO_SIZE ];
    if ( vol->dev->type != ADF_DEVTYPE_UNKNOWN ) {
        snprintf( typeInfo, TYPE_INFO_SIZE, "%s\n",
                  adfDevTypeGetDescription( vol->dev->type ) );
    } else {
        switch ( vol->dev->class ) {
        case ADF_DEVCLASS_HARDDISK:
            snprintf( typeInfo, TYPE_INFO_SIZE, "Hard Disk partition, %3.1f KBytes\n",
                      ( vol->lastBlock - vol->firstBlock + 1 ) * 512.0 / 1024.0 );
            break;
        case ADF_DEVCLASS_HARDFILE:
            snprintf( typeInfo, TYPE_INFO_SIZE, "HardFile : %3.1f KBytes\n",
                      ( vol->lastBlock - vol->firstBlock + 1 ) * 512.0 / 1024.0 );
            break;
        default:
            snprintf( typeInfo, TYPE_INFO_SIZE, "Unknown devType!\n" );
        }
    }

    int cDays, cMonth, cYear,
        aDays, aMonth, aYear,
        mDays, mMonth, mYear;
    adfDays2Date( root.coDays, &cYear, &cMonth, &cDays );
    adfDays2Date( root.days,   &aYear, &aMonth, &aDays );
    adfDays2Date( root.cDays,  &mYear, &mMonth, &mDays );

    const unsigned volInfoSize = snprintf(
        NULL, 0,
        "\nADF volume info:\n  Name:\t\t%-30s\n"
        "  Type:\t\t%s"
        "  Filesystem:\t%s %s %s\n"
        "  Free blocks:\t%d\n"
        "  R/W:\t\t%s\n"
        "  Created:\t%d/%02d/%02d %d:%02d:%02d\n"
        "  Last access:\t%d/%02d/%02d %d:%02d:%02d"
        "\n\t\t%d/%02d/%02d %d:%02d:%02d\n",
        vol->volName, // diskName ?
        typeInfo,
        adfVolIsFFS( vol ) ? "FFS" : "OFS",
        adfVolHasINTL( vol ) ? "INTL " : "",
        adfVolHasDIRCACHE( vol ) ? "DIRCACHE " : "",
        adfCountFreeBlocks( vol ),
        vol->readOnly ? "Read only" : "Read/Write",
        cDays, cMonth, cYear,
        root.coMins / 60,
        root.coMins % 60,
        root.coTicks / 50,
        aDays, aMonth, aYear,
        root.mins / 60,
        root.mins % 60,
        root.ticks / 50,
        mDays, mMonth, mYear,
        root.cMins / 60,
        root.cMins % 60,
        root.cTicks / 50 ) + 1;


    char * const volInfo = malloc( volInfoSize );
    if ( volInfo == NULL )
        return NULL;

    snprintf(
        volInfo, volInfoSize,
        "\nADF volume info:\n  Name:\t\t%-30s\n"
        "  Type:\t\t%s"
        "  Filesystem:\t%s %s %s\n"
        "  Free blocks:\t%d\n"
        "  R/W:\t\t%s\n"
        "  Created:\t%d/%02d/%02d %d:%02d:%02d\n"
        "  Last access:\t%d/%02d/%02d %d:%02d:%02d"
        "\n\t\t%d/%02d/%02d %d:%02d:%02d\n",
        vol->volName, // diskName ?
        typeInfo,
        adfVolIsFFS( vol ) ? "FFS" : "OFS",
        adfVolHasINTL( vol ) ? "INTL " : "",
        adfVolHasDIRCACHE( vol ) ? "DIRCACHE " : "",
        adfCountFreeBlocks( vol ),
        vol->readOnly ? "Read only" : "Read/Write",
        cDays, cMonth, cYear,
        root.coMins / 60,
        root.coMins % 60,
        root.coTicks / 50,
        aDays, aMonth, aYear,
        root.mins / 60,
        root.mins % 60,
        root.ticks / 50,
        mDays, mMonth, mYear,
        root.cMins / 60,
        root.cMins % 60,
        root.cTicks / 50 );

    return volInfo;
}
