/*
 *  adf_bitm.c - block allocation bitmap code
 *
 *  Copyright (C) 1997-2022 Laurent Clevy
 *                2023-2025 Tomasz Wolak
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

#include "adf_bitm.h"

#include "adf_byteorder.h"
#include "adf_cache.h"
//#include "adf_debug.h"
#include "adf_dir.h"
#include "adf_env.h"
#include "adf_file_block.h"
#include "adf_raw.h"
#include "adf_util.h"
#include "adf_vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_NONZERO_BMPAGES_BEYOND_BMSIZE 0

extern uint32_t bitMask[ 32 ];

static ADF_RETCODE adfBitmapListSetUsed( struct AdfVolume * const      vol,
                                         const struct AdfList * const  list );

static ADF_RETCODE adfBitmapFileBlocksSetUsed(
    struct AdfVolume * const                 vol,
    const struct AdfFileHeaderBlock * const  fhBlock );

static ADF_RETCODE adfBitmapDirCacheSetUsed(
    struct AdfVolume * const  vol,
    ADF_SECTNUM               dCacheBlockNum );

static uint32_t nBlock2bitmapSize( uint32_t  nBlock );


#if CHECK_NONZERO_BMPAGES_BEYOND_BMSIZE == 1

static void checkNonzeroBMpagesBeyondBMsizeRoot(
    const char * const callerFuncName,
    const uint32_t     firstFree,
    const uint32_t     volBitmapSize,
    const int32_t      rootBmPages[ ADF_BM_PAGES_ROOT_SIZE ] );

static void checkNonzeroBMpagesBeyondBMsizeExt(
    const char * const  callerFuncName,
    const uint32_t      firstFree,
    const unsigned      bmExt_i,
    const ADF_SECTNUM   bmExtSect,
    const uint32_t      volBitmapSize,
    const int32_t       bmExtBlockBmPages[ ADF_BM_PAGES_EXT_SIZE ] );

#endif


/*
 * adfCreateBitmap
 *
 * create bitmap structure in vol
 */
ADF_RETCODE adfCreateBitmap( struct AdfVolume * const  vol )
{
    const ADF_SECTNUM nBlock = (ADF_SECTNUM) adfVolGetSizeInBlocksWithoutBootblock( vol );

    vol->bitmap.size = nBlock2bitmapSize( (uint32_t) nBlock );
    ADF_RETCODE rc = adfBitmapAllocate( vol );
    if ( rc != ADF_RC_OK )
        return rc;

    for ( int i = vol->firstBlock + 2; i <= vol->lastBlock - vol->firstBlock; i++ )
        adfSetBlockFree( vol, i );

    return rc;
}


/*
 * adfBitmapAllocate
 *
 * vol->bitmapSize must be set properly before calling
 */
ADF_RETCODE adfBitmapAllocate( struct AdfVolume * const  vol )
{
    vol->bitmap.size = nBlock2bitmapSize(
        adfVolGetSizeInBlocksWithoutBootblock( vol ) );

    vol->bitmap.table = (struct AdfBitmapBlock**)
        malloc( sizeof(struct AdfBitmapBlock *) * vol->bitmap.size );
    if ( vol->bitmap.table == NULL ) {
        adfEnv.eFct( "%s: malloc, vol->bitmapTable", __func__ );
        return ADF_RC_MALLOC;
    }

    vol->bitmap.blocks = (ADF_SECTNUM *) malloc( sizeof(ADF_SECTNUM) * vol->bitmap.size );
    if ( vol->bitmap.blocks == NULL ) {
        free( vol->bitmap.table );
        vol->bitmap.table = NULL;
        adfEnv.eFct( "%s: malloc, vol->bitmapBlocks", __func__ );
        return ADF_RC_MALLOC;
    }

    vol->bitmap.blocksChg = (bool *) malloc( sizeof(bool) * vol->bitmap.size );
    if ( vol->bitmap.blocksChg == NULL ) {
        free( vol->bitmap.table );
        vol->bitmap.table = NULL;
        free( vol->bitmap.blocks );
        vol->bitmap.blocks = NULL;
        adfEnv.eFct( "%s: malloc, vol->bitmapBlocksChg", __func__ );
        return ADF_RC_MALLOC;
    }

    for ( unsigned i = 0 ; i < vol->bitmap.size ; i++ ) {
        vol->bitmap.table[ i ] = (struct AdfBitmapBlock *)
            malloc( sizeof(struct AdfBitmapBlock) );

        if ( vol->bitmap.table[ i ] == NULL) {
            free( vol->bitmap.blocksChg );
            vol->bitmap.blocksChg = NULL;
            free( vol->bitmap.blocks );
            vol->bitmap.blocks = NULL;
            for ( unsigned j = 0 ; j < i ; j++ )
                free( vol->bitmap.table[ j ] );
            free( vol->bitmap.table );
            vol->bitmap.table = NULL;
            adfEnv.eFct( "%s: malloc", __func__ );
            return ADF_RC_MALLOC;
        }
    }
    return ADF_RC_OK;
}


/*
 * adfFreeBitmap
 *
 */
void adfFreeBitmap( struct AdfVolume * const  vol )
{
    for ( unsigned i = 0 ; i < vol->bitmap.size ; i++ )
        free( vol->bitmap.table[ i ] );
    vol->bitmap.size = 0;

    free( vol->bitmap.table );
    vol->bitmap.table = NULL;

    free( vol->bitmap.blocks );
    vol->bitmap.blocks = NULL;

    free( vol->bitmap.blocksChg );
    vol->bitmap.blocksChg = NULL;
}


/*
 * adfReadBitmap
 *
 */
ADF_RETCODE adfReadBitmap( struct AdfVolume * const           vol,
                           const struct AdfRootBlock * const  root )
{
    ADF_RETCODE rc = ADF_RC_OK;

    for ( unsigned i = 0 ; i < vol->bitmap.size ; i++ ) {
        vol->bitmap.blocksChg[ i ] = false;
    }

    uint32_t j = 0,
             i = 0;
    /* bitmap pointers in rootblock : 0 <= i < ADF_BM_PAGES_ROOT_SIZE */
    ADF_SECTNUM bmSect;
    while ( i < vol->bitmap.size &&
            i < ADF_BM_PAGES_ROOT_SIZE &&
            root->bmPages[ i ] != 0 )
    {
        vol->bitmap.blocks[ j ] = bmSect = root->bmPages[ i ];
        if ( ! adfVolIsSectNumValid( vol, bmSect ) ) {
            adfEnv.wFct( "%s: sector %d out of range, root bm[%u]",
                         __func__, bmSect, i );
            adfFreeBitmap( vol );
            return ADF_RC_ERROR;
        }

        rc = adfReadBitmapBlock( vol, bmSect, vol->bitmap.table[ j ] );
        if ( rc != ADF_RC_OK ) {
            adfFreeBitmap( vol );
            return rc;
        }
        j++;
        i++;
    }

    /* state validity checks */
    assert ( i <= ADF_BM_PAGES_ROOT_SIZE );

    assert ( ( i < vol->bitmap.size &&
               root->bmPages[ i ] != 0 ) ||
             ( i == vol->bitmap.size ) );

    // some images fail on this, https://github.com/adflib/ADFlib/issues/63
    //   assert ( ( i == vol->bitmap.size && root->bmPages[i] == 0 ) ||
    //            ( i < vol->bitmap.size ) );

    assert ( ( i < vol->bitmap.size &&
               root->bmExt != 0 ) ||
             ( i == vol->bitmap.size ) );

    assert ( ( i == vol->bitmap.size &&
               root->bmExt == 0 ) ||
             ( i < vol->bitmap.size ) );

    if  ( i < vol->bitmap.size  &&  root->bmPages[i] == 0 ) {
        adfEnv.eFct( "%s: root bmpages[%u] == 0, "
                     "but vol. %s should have %u bm sectors",
                     __func__, i, vol->volName, vol->bitmap.size );
        adfFreeBitmap( vol );
        return ADF_RC_ERROR;
    }

    if ( i < vol->bitmap.size  &&  root->bmExt == 0 ) {
        adfEnv.eFct( "%s: read %u of %u from root bm sectors of "
                     "%u total to read, but root->bmExt is 0",
                     __func__, i, ADF_BM_PAGES_ROOT_SIZE, vol->bitmap.size );
        adfFreeBitmap( vol );
        return ADF_RC_ERROR;
    }

#if CHECK_NONZERO_BMPAGES_BEYOND_BMSIZE == 1
    checkNonzeroBMpagesBeyondBMsizeRoot(
        __func__, i, vol->bitmap.size, root->bmPages );
#endif

    if ( root->bmExt == 0 )
        return rc;

    struct AdfBitmapExtBlock bmExt;
    ADF_SECTNUM bmExtSect = root->bmExt;
#if CHECK_NONZERO_BMPAGES_BEYOND_BMSIZE == 1
    unsigned bmExt_i = 0;
#endif
    while ( bmExtSect != 0 ) {
        /* bitmap pointers in bitmapExtBlock, j <= mapSize */
        rc = adfReadBitmapExtBlock( vol, bmExtSect, &bmExt );
        if ( rc != ADF_RC_OK ) {
            adfFreeBitmap( vol );
            return rc;
        }
        i = 0;
        while ( i < ADF_BM_PAGES_EXT_SIZE && j < vol->bitmap.size ) {
            bmSect = bmExt.bmPages[ i ];
            if ( ! adfVolIsSectNumValid( vol, bmSect ) ) {
                adfEnv.wFct( "%s: sector %d out of range, "
                             "bmext %d bmpages[%u]",
                             __func__, bmSect, bmExtSect, i );
                return ADF_RC_ERROR;
            }
            vol->bitmap.blocks[ j ] = bmSect;

            rc = adfReadBitmapBlock( vol, bmSect, vol->bitmap.table[ j ] );
            if ( rc != ADF_RC_OK ) {
                adfFreeBitmap( vol );
                return rc;
            }
            i++; j++;
        }

#if CHECK_NONZERO_BMPAGES_BEYOND_BMSIZE == 1
        checkNonzeroBMpagesBeyondBMsizeExt(
            __func__, i, bmExt_i, bmExtSect, vol->bitmap.size, bmExt.bmPages );
        bmExt_i++;
#endif
        bmExtSect = bmExt.nextBlock;
    }

    return rc;
}


/*
 * adfUpdateBitmap
 *
 */
ADF_RETCODE adfUpdateBitmap( struct AdfVolume * const  vol )
{
    struct AdfRootBlock root;

/*printf("adfUpdateBitmap\n");*/

    ADF_RETCODE rc = adfReadRootBlock( vol, (uint32_t) vol->rootBlock, &root );
    if ( rc != ADF_RC_OK )
        return rc;

    root.bmFlag = ADF_BM_INVALID;

    rc = adfWriteRootBlock( vol, (uint32_t) vol->rootBlock, &root );
    if ( rc != ADF_RC_OK )
        return rc;

    for ( unsigned i = 0 ; i < vol->bitmap.size ; i++ )
        if ( vol->bitmap.blocksChg[ i ] ) {
            rc = adfWriteBitmapBlock( vol, vol->bitmap.blocks[ i ],
                                      vol->bitmap.table[ i ] );
            if ( rc != ADF_RC_OK )
                return rc;
            vol->bitmap.blocksChg[ i ] = false;
    }

    root.bmFlag = ADF_BM_VALID;
    adfTime2AmigaTime( adfGiveCurrentTime(), &root.days, &root.mins, &root.ticks );
    return adfWriteRootBlock( vol, (uint32_t) vol->rootBlock, &root );
}


/*
 * adfWriteNewBitmap
 *
 * write ext blocks and bitmap
 *
 * uses vol->bitmapSize, 
 */
ADF_RETCODE adfWriteNewBitmap( struct AdfVolume * const  vol )
{
    struct AdfRootBlock root;
    ADF_RETCODE rc = adfReadRootBlock( vol, (uint32_t) vol->rootBlock, &root );
    if ( rc != ADF_RC_OK ) {
        return rc;
    }

     ADF_SECTNUM * const sectList =
         (ADF_SECTNUM *) malloc( sizeof(ADF_SECTNUM) * (unsigned) vol->bitmap.size );
    if ( ! sectList ) {
        adfEnv.eFct( "%s: sectList", __func__ );
        return ADF_RC_MALLOC;
    }

    if ( ! adfGetFreeBlocks( vol, (int) vol->bitmap.size, sectList ) ) {
        free( sectList );
        return ADF_RC_VOLFULL;
    }

    const unsigned n = min( vol->bitmap.size, (uint32_t) ADF_BM_PAGES_ROOT_SIZE );
    for ( unsigned i = 0 ; i < n ; i++ ) {
        root.bmPages[ i ] = vol->bitmap.blocks[ i ] = sectList[ i ];
    }
    unsigned nBlock = n;

    /* for devices with more than 25 * BM_MAP_SIZE(127) blocks == hards disks */
    if ( vol->bitmap.size > ADF_BM_PAGES_ROOT_SIZE ) {

        unsigned nExtBlock = ( vol->bitmap.size - ADF_BM_PAGES_ROOT_SIZE ) / ADF_BM_MAP_SIZE;
        if ( ( vol->bitmap.size - ADF_BM_PAGES_ROOT_SIZE ) % ADF_BM_MAP_SIZE )
            nExtBlock++;

        ADF_SECTNUM * const bitExtBlock =
            (ADF_SECTNUM *) malloc( sizeof(ADF_SECTNUM) * (unsigned) nExtBlock );
        if ( ! bitExtBlock ) {
            free( sectList );
            adfEnv.eFct( "%s: malloc failed", __func__ );
            return ADF_RC_MALLOC;
        }

        if ( ! adfGetFreeBlocks( vol, (int) nExtBlock, bitExtBlock ) ) {
           free( sectList );
           free( bitExtBlock );
           return ADF_RC_VOLFULL;
        }

        unsigned k = 0;
        root.bmExt = bitExtBlock[ k ];
        struct AdfBitmapExtBlock bitme;
        while ( nBlock < vol->bitmap.size ) {
            int i = 0;
            while ( i < ADF_BM_PAGES_EXT_SIZE && nBlock < vol->bitmap.size ) {
                bitme.bmPages[ i ] = vol->bitmap.blocks[ nBlock ] = sectList[ i ];
                i++;
                nBlock++;
            }
            if ( k + 1 < nExtBlock )
                bitme.nextBlock = bitExtBlock[ k + 1 ];
            else
                bitme.nextBlock = 0;

            rc = adfWriteBitmapExtBlock( vol, bitExtBlock[ k ], &bitme );
            if ( rc != ADF_RC_OK ) {
                free( sectList );
                free( bitExtBlock );
                return rc;
            }

            k++;
        }
        free( bitExtBlock );
    }
    free( sectList );

    return adfWriteRootBlock( vol, (uint32_t) vol->rootBlock, &root );
}


/*
 * adfReconstructBitmap
 *
 */
ADF_RETCODE adfReconstructBitmap( struct AdfVolume * const           vol,
                                  const struct AdfRootBlock * const  root )
{
    ADF_RETCODE rc = ADF_RC_OK;

    // all bitmap blocks are to update (to improve/optimize, ie. compare with existing)
    for ( unsigned i = 0 ; i < vol->bitmap.size ; i++ ) {
        vol->bitmap.blocksChg[ i ] = true;
    }

    uint32_t i = 0,
             j = 0;
    /* bitmap pointers in rootblock : 0 <= i < ADF_BM_PAGES_ROOT_SIZE */
    ADF_SECTNUM bmSect;
    while ( i < vol->bitmap.size &&
            i < ADF_BM_PAGES_ROOT_SIZE &&
            root->bmPages[ i ] != 0 )
    {
        vol->bitmap.blocks[ j ] = bmSect = root->bmPages[ i ];
        if ( ! adfVolIsSectNumValid( vol, bmSect ) ) {
            adfEnv.wFct( "%s: sector %d out of range, root bm[%u]",
                         __func__, bmSect, i );
            adfFreeBitmap( vol );
            return ADF_RC_ERROR;
        }

        //rc = adfReadBitmapBlock ( vol, nSect, vol->bitmapTable[j] );
        //if ( rc != ADF_RC_OK ) {
        //    adfFreeBitmap(vol);
        //    return rc;
        //}
        j++;
        i++;
    }

    /* state validity checks */
    assert ( i <= ADF_BM_PAGES_ROOT_SIZE );

    assert ( ( i < vol->bitmap.size &&
               root->bmPages[ i ] != 0 ) ||
             ( i == vol->bitmap.size ) );

    // some images fail on this, https://github.com/adflib/ADFlib/issues/63
    //   assert ( ( i == vol->bitmap.size && root->bmPages[i] == 0 ) ||
    //            ( i < vol->bitmap.size ) );

    assert ( ( i < vol->bitmap.size &&
               root->bmExt != 0 ) ||
             ( i == vol->bitmap.size ) );

    assert ( ( i == vol->bitmap.size &&
               root->bmExt == 0 ) ||
             ( i < vol->bitmap.size ) );

    if  ( i < vol->bitmap.size &&
          root->bmPages[ i ] == 0 )
    {
        adfEnv.eFct( "%s: root bmpages[%u] == 0, "
                     "but vol. %s should have %u bm sectors",
                     __func__, i, vol->volName, vol->bitmap.size );
        adfFreeBitmap( vol );
        return ADF_RC_ERROR;
    }

    if ( i < vol->bitmap.size  &&  root->bmExt == 0 ) {
        adfEnv.eFct( "%s: read %u of %u from root bm sectors of "
                     "%u total to read, but root->bmExt is 0",
                     __func__, i, ADF_BM_PAGES_ROOT_SIZE, vol->bitmap.size );
        adfFreeBitmap( vol );
        return ADF_RC_ERROR;
    }

#if CHECK_NONZERO_BMPAGES_BEYOND_BMSIZE == 1
    checkNonzeroBMpagesBeyondBMsizeRoot(
        __func__, i, vol->bitmap.size, root->bmPages );
#endif

    ADF_SECTNUM bmExtSect = root->bmExt;
#if CHECK_NONZERO_BMPAGES_BEYOND_BMSIZE == 1
    unsigned bmExt_i = 0;
#endif
    while ( bmExtSect != 0 ) {
        struct AdfBitmapExtBlock bmExtBlock;

        /* bitmap pointers in bitmapExtBlock, j <= mapSize */
        rc = adfReadBitmapExtBlock( vol, bmExtSect, &bmExtBlock );
        if ( rc != ADF_RC_OK ) {
            adfFreeBitmap( vol );
            return rc;
        }
        i = 0;
        while ( i < ADF_BM_PAGES_EXT_SIZE && j < vol->bitmap.size ) {
            const ADF_SECTNUM bmBlkPtr = bmExtBlock.bmPages[ i ];
            if ( ! adfVolIsSectNumValid( vol, bmBlkPtr ) ) {
                adfEnv.eFct( "%s: sector %d out of range, "
                             "bmext %d bmpages[%u]",
                             __func__, bmBlkPtr, bmExtSect, i );
                adfFreeBitmap( vol );
                return ADF_RC_ERROR;
            }
            vol->bitmap.blocks[ j ] = bmBlkPtr;

            //rc = adfReadBitmapBlock ( vol, nSect, vol->bitmapTable[j] );
            //if ( rc != ADF_RC_OK ) {
            //    adfFreeBitmap(vol);
            //    return rc;
            //}
            i++; j++;
        }

#if CHECK_NONZERO_BMPAGES_BEYOND_BMSIZE == 1
        checkNonzeroBMpagesBeyondBMsizeExt(
            __func__, i, bmExt_i, bmExtSect, vol->bitmap.size, bmExtBlock.bmPages );
        bmExt_i++;
#endif
        bmExtSect = bmExtBlock.nextBlock;
    }

    // initially - set all free
    for ( int32_t i = 2 ;
          i < vol->lastBlock - vol->firstBlock ;   // CHECK UPPER COND
          i++ )
    {
        adfSetBlockFree( vol, i );
    }

    // then - mark all used blocks

    // mark rootblock
    adfSetBlockUsed( vol, vol->rootBlock );

    // mark bitmap blocks (all - from rootblock and bitmap ext. blocks)
    for ( uint32_t i = 0 ; i < vol->bitmap.size ; i++ ) {
        //if ( vol->bitmapBlocks[i] > 0 )  -- this should always be TRUE
        adfSetBlockUsed( vol, vol->bitmap.blocks[ i ] );
    }

    // traverse all files and directories

    struct AdfDirBlock //AdfEntryBlock
        rootDirBlock;
    rc = adfReadEntryBlock( vol, vol->rootBlock,
                            ( struct AdfEntryBlock * ) &rootDirBlock );
    if ( rc != ADF_RC_OK ) {
        adfEnv.eFct( "%s: error reading directory entry block (%d)\n",
                     __func__, vol->rootBlock );
        return rc;
    }

    if ( ! adfIsDirEmpty( (const struct AdfDirBlock * const) &rootDirBlock ) ) {
        // note: for a large volume (a hard disk) getting all entries can become big
        // - it may need to be optimized
        struct AdfList * const entries = adfGetRDirEnt( vol, vol->rootBlock, true );
        if ( entries == NULL ) {
            return ADF_RC_ERROR;
        }

        adfBitmapListSetUsed( vol, entries );
        adfFreeDirList( entries );
    }

    return rc;
}


/*
 * adfGet1FreeBlock
 *
 */
ADF_SECTNUM adfGet1FreeBlock( struct AdfVolume * const  vol )
{
    ADF_SECTNUM block[ 1 ];
    if ( ! adfGetFreeBlocks( vol, 1, block ) )
        return -1;
    else
        return block[ 0 ];
}

/*
 * adfGetFreeBlocks
 *
 */
bool adfGetFreeBlocks( struct AdfVolume * const  vol,
                       const int                 nbSect,
                       ADF_SECTNUM * const       sectList )
{
    int i, j;
    int32_t block = vol->rootBlock;

    i = 0;
    bool diskFull = false;
/*printf("lastblock=%ld\n",vol->lastBlock);*/
    while ( i < nbSect && !diskFull ) {
        if ( adfIsBlockFree( vol, block ) ) {
            sectList[ i ] = block;
            i++;
        }
/*        if ( block==vol->lastBlock )
            block = vol->firstBlock+2;*/
        if ( ( block + vol->firstBlock ) == vol->lastBlock ) {
            block = 2;
        } else {
            block++;
            if ( block == vol->rootBlock )
                diskFull = true;
        }
    }

    const bool gotAllBlocks = ( i == nbSect );
    if ( gotAllBlocks )
        for ( j = 0; j < nbSect; j++ )
            adfSetBlockUsed( vol, sectList[ j ] );

    return gotAllBlocks;
}


/*
 * adfIsBlockFree
 *
 */
bool adfIsBlockFree( const struct AdfVolume * const  vol,
                     const ADF_SECTNUM               nSect )
{
    assert( nSect >= 2 );
    const int
        sectOfMap  = nSect - 2,
        block      = sectOfMap / ( ADF_BM_MAP_SIZE * 32 ),
        indexInMap = ( sectOfMap / 32 ) % ADF_BM_MAP_SIZE;

/*printf("sect=%d block=%d ind=%d,  ",sectOfMap,block,indexInMap);
printf("bit=%d,  ",sectOfMap%32);
printf("bitm=%x,  ",bitMask[ sectOfMap%32]);
printf("res=%x,  ",vol->bitmapTable[ block ]->map[ indexInMap ]
        & bitMask[ sectOfMap%32 ]);
*/
    return ( ( vol->bitmap.table[ block ]->map[ indexInMap ] &
               bitMask[ sectOfMap % 32 ] ) != 0 );
}


/*
 * adfSetBlockFree OK
 *
 */
void adfSetBlockFree( struct AdfVolume * const  vol,
                      const ADF_SECTNUM         nSect )
{
    assert( nSect >= 2 );
    assert( nSect <= vol->lastBlock - vol->firstBlock );

    const int
        sectOfMap  = nSect - 2,
        block      = sectOfMap / ( ADF_BM_MAP_SIZE * 32 ),
        indexInMap = ( sectOfMap / 32 ) % ADF_BM_MAP_SIZE;

/*printf("sect=%d block=%d ind=%d,  ",sectOfMap,block,indexInMap);
printf("bit=%d,  ",sectOfMap%32);
*printf("bitm=%x,  ",bitMask[ sectOfMap%32]);*/

    const uint32_t oldValue = vol->bitmap.table[ block ]->map[ indexInMap ];
/*printf("old=%x,  ",oldValue);*/
    vol->bitmap.table[ block ]->map[ indexInMap ] =
        oldValue | bitMask[ sectOfMap % 32 ];
/*printf("new=%x,  ",vol->bitmapTable[ block ]->map[ indexInMap ]);*/

    vol->bitmap.blocksChg[ block ] = true;
}


/*
 * adfSetBlockUsed
 *
 */
void adfSetBlockUsed( struct AdfVolume * const  vol,
                      const ADF_SECTNUM         nSect )
{
    assert( nSect >= 2 );
    assert( nSect <= vol->lastBlock - vol->firstBlock );

    const int
        sectOfMap  = nSect - 2,
        block      = sectOfMap / ( ADF_BM_MAP_SIZE * 32 ),
        indexInMap = ( sectOfMap / 32 ) % ADF_BM_MAP_SIZE;

    const uint32_t oldValue = vol->bitmap.table[ block ]->map[ indexInMap ];

    vol->bitmap.table[ block ]->map[ indexInMap ] =
        oldValue & ( ~bitMask[ sectOfMap % 32 ] );
    vol->bitmap.blocksChg[ block ] = true;
}


/*
 * adfReadBitmapBlock
 *
 */
ADF_RETCODE adfReadBitmapBlock( struct AdfVolume *       vol,
                                ADF_SECTNUM              nSect,
                                struct AdfBitmapBlock *  bitm )
{
    uint8_t buf[ ADF_LOGICAL_BLOCK_SIZE ];

/*printf("bitmap %ld\n",nSect);*/
    ADF_RETCODE rc = adfVolReadBlock( vol, (uint32_t) nSect, buf );
    if ( rc != ADF_RC_OK )
        return rc;

    memcpy( bitm, buf, ADF_LOGICAL_BLOCK_SIZE );
#ifdef LITT_ENDIAN
    /* big to little = 68000 to x86 */
    adfSwapEndian( (uint8_t *) bitm, ADF_SWBL_BITMAP );
#endif

    const uint32_t checksumCalculated = adfNormalSum( buf, 0,
                                                      ADF_LOGICAL_BLOCK_SIZE );
    if ( bitm->checkSum != checksumCalculated ) {
        const char msg[] = "%s: invalid checksum 0x%x != 0x%x (calculated)"
            ", block %d, volume '%s'";
        if ( adfEnv.ignoreChecksumErrors ) {
            adfEnv.wFct( msg, __func__, bitm->checkSum, checksumCalculated,
                         nSect, vol->volName );
        } else {
            adfEnv.eFct( msg, __func__, bitm->checkSum, checksumCalculated,
                         nSect, vol->volName );
            return ADF_RC_BLOCKSUM;
        }
    }

    return ADF_RC_OK;
}


/*
 * adfWriteBitmapBlock
 *
 */
ADF_RETCODE adfWriteBitmapBlock( struct AdfVolume * const             vol,
                                 const ADF_SECTNUM                    nSect,
                                 const struct AdfBitmapBlock * const  bitm )
{
    uint8_t buf[ ADF_LOGICAL_BLOCK_SIZE ];

    memcpy( buf, bitm, ADF_LOGICAL_BLOCK_SIZE );

#ifdef LITT_ENDIAN
    /* little to big */
    adfSwapEndian( buf, ADF_SWBL_BITMAP );
#endif

    const uint32_t newSum = adfNormalSum( buf, 0, ADF_LOGICAL_BLOCK_SIZE );
    swapUint32ToPtr( buf, newSum );

/*	dumpBlock((uint8_t*)buf);*/

    return adfVolWriteBlock( vol, (uint32_t) nSect, buf );
}


/*
 * adfReadBitmapExtBlock
 *
 */
ADF_RETCODE adfReadBitmapExtBlock( struct AdfVolume * const          vol,
                                   const ADF_SECTNUM                 nSect,
                                   struct AdfBitmapExtBlock * const  bitme )
{
    uint8_t buf[ ADF_LOGICAL_BLOCK_SIZE ];

    ADF_RETCODE rc = adfVolReadBlock( vol, (uint32_t) nSect, buf );
    if ( rc != ADF_RC_OK )
        return rc;

    memcpy( bitme, buf, ADF_LOGICAL_BLOCK_SIZE );
#ifdef LITT_ENDIAN
    adfSwapEndian( (uint8_t *) bitme, ADF_SWBL_BITMAP );
#endif

    return ADF_RC_OK;
}


/*
 * adfWriteBitmapExtBlock
 *
 */
ADF_RETCODE adfWriteBitmapExtBlock( struct AdfVolume * const                vol,
                                    const ADF_SECTNUM                       nSect,
                                    const struct AdfBitmapExtBlock * const  bitme )
{
    uint8_t buf[ ADF_LOGICAL_BLOCK_SIZE ];
	
    memcpy( buf, bitme, ADF_LOGICAL_BLOCK_SIZE );
#ifdef LITT_ENDIAN
    /* little to big */
    adfSwapEndian( buf, ADF_SWBL_BITMAPE );
#endif

/*	dumpBlock((uint8_t*)buf);*/
    return adfVolWriteBlock( vol, (uint32_t) nSect, buf );
}


/*
 * adfCountFreeBlocks
 *
 */
uint32_t adfCountFreeBlocks( const struct AdfVolume * const  vol )
{
    assert( vol->lastBlock - vol->firstBlock > 2 );
    uint32_t freeBlocks = 0L;
    for ( int j = 2; j <= vol->lastBlock - vol->firstBlock; j++ )
        if ( adfIsBlockFree( vol, j ) )
            freeBlocks++;
    return freeBlocks;
}


/*
 * adfVolBitmaIsMarkedValid
 *
 */
bool adfVolBitmapIsMarkedValid( struct AdfVolume * const  vol )
{
    struct AdfRootBlock root;
    ADF_RETCODE rc = adfReadRootBlock( vol, (uint32_t) vol->rootBlock, &root );
    if ( rc != ADF_RC_OK )
        return false;
    return ( root.bmFlag == ADF_BM_VALID );
}



/*#######################################################################################*/


static ADF_RETCODE adfBitmapListSetUsed( struct AdfVolume * const      vol,
                                         const struct AdfList * const  list )
{
    ADF_RETCODE rc = ADF_RC_OK;

    const struct AdfList * cell = list;
    while ( cell != NULL ) {
        const struct AdfEntry * const entry =
            (const struct AdfEntry * const) cell->content;

        // mark entry block
        // (all header blocks (file, dir, links) are done with this)
        adfSetBlockUsed( vol, entry->sector );

        // mark file blocks
        if ( entry->type == ADF_ST_FILE ) {
            struct AdfFileHeaderBlock fhBlock;
            rc = adfReadEntryBlock( vol, entry->sector,
                                    (struct AdfEntryBlock *) &fhBlock );
            if ( rc != ADF_RC_OK ) {
                adfEnv.eFct( "%s: error reading entry (file) block, "
                             "block %d, volume '%s', file name '%s'",
                             __func__, entry->sector, vol->volName, entry->name );
                break;
            }

            rc = adfBitmapFileBlocksSetUsed( vol, &fhBlock );
            if ( rc != ADF_RC_OK ) {
                adfEnv.eFct( "%s: adfBitmapFileBlocksSetUsed returned "
                             "error %d, block %d, volume '%s', file name '%s'",
                             __func__, rc, entry->sector, vol->volName, entry->name );
                break;
            }
        }

        // mark directory and directory cache blocks
        else if ( entry->type == ADF_ST_DIR ) {
            struct AdfDirBlock dirBlock;
            rc = adfReadEntryBlock( vol, entry->sector,
                                    (struct AdfEntryBlock *) &dirBlock );
            if ( rc != ADF_RC_OK ) {
                adfEnv.eFct( "%s: error reading entry (directory) block, "
                             "block %d, volume '%s', directory name '%s'",
                             __func__, entry->sector, vol->volName, entry->name );
                break;
            }
            rc = adfBitmapDirCacheSetUsed( vol, dirBlock.extension );
            if ( rc != ADF_RC_OK ) {
                adfEnv.eFct( "%s: adfBitmapDirCacheSetUsed returned "
                             "error %d, block %d, volume '%s', directory name '%s'",
                             __func__, rc, entry->sector, vol->volName, entry->name );
                break;
            }
        }

        // if any subdirectory present - process recursively
        if ( cell->subdir != NULL ) {
            const struct AdfList * const subdir =
                ( const struct AdfList * const ) cell->subdir;
            rc = adfBitmapListSetUsed( vol, subdir );
            if ( rc != ADF_RC_OK ) {
                adfEnv.eFct( "%s: adfBitmapListSetUsed returned "
                             "error %d, volume '%s', directory name '%s'",
                             __func__, rc, vol->volName,
                             ( (const struct AdfEntry * const) subdir->content )->name );
                break;
            }
        }
        cell = cell->next;
    }
    return rc;
}


static ADF_RETCODE adfBitmapFileBlocksSetUsed (
    struct AdfVolume * const                 vol,
    const struct AdfFileHeaderBlock * const  fhBlock )
{
    ADF_RETCODE rc = ADF_RC_OK;

    // mark blocks from the header
    for ( uint32_t block = 0 ; block < ADF_MAX_DATABLK ; block++ ) {
        if ( fhBlock->dataBlocks[ block ] > 1 )
            adfSetBlockUsed( vol, fhBlock->dataBlocks[ block ] );
    }

    // mark blocks from ext blocks

    //const int nExtBlocks = (int) adfFileSize2Extblocks ( fhBlock->byteSize,
    //                                                     vol->datablockSize );
    ADF_SECTNUM extBlockPtr = fhBlock->extension;
    struct AdfFileExtBlock fext;
    while ( extBlockPtr != 0 ) {
        adfSetBlockUsed( vol, extBlockPtr );
        rc = adfReadFileExtBlock( vol, extBlockPtr, &fext );
        if ( rc != ADF_RC_OK ) {
            adfEnv.eFct( "%s: error reading ext block %d, file '%s'",
                         __func__, extBlockPtr, fhBlock->fileName );
            break;
        }
        for ( uint32_t block = 0 ; block < ADF_MAX_DATABLK ; block++ ) {
            if ( fext.dataBlocks[ block ] > 1 )
                adfSetBlockUsed( vol, fext.dataBlocks[ block ] );
        }
        extBlockPtr = fext.extension;
    }
    return rc;
}


static ADF_RETCODE adfBitmapDirCacheSetUsed( struct AdfVolume * const  vol,
                                             ADF_SECTNUM               dCacheBlockNum )
{
    ADF_RETCODE rc = ADF_RC_OK;
    while ( dCacheBlockNum != 0 ) {
        adfSetBlockUsed( vol, dCacheBlockNum );

        struct AdfDirCacheBlock dirCacheBlock;
        rc = adfReadDirCBlock( vol, dCacheBlockNum, &dirCacheBlock );
        if ( rc != ADF_RC_OK )
            break;

        dCacheBlockNum = dirCacheBlock.nextDirC;
    }
    return rc;
}


static uint32_t nBlock2bitmapSize( uint32_t  nBlock )
{
    uint32_t mapSize = (uint32_t) nBlock / ( ADF_BM_MAP_SIZE * 32 );
    if ( ( nBlock % ( ADF_BM_MAP_SIZE * 32 ) ) != 0 )
        mapSize++;
    return mapSize;
}


#if CHECK_NONZERO_BMPAGES_BEYOND_BMSIZE == 1

/* Check for erratic (?) (non-zero) entries in bmpages beyond the expected
   size, more info:  https://github.com/adflib/ADFlib/issues/63

   Instead of only checking, such bmpage entries could be "repaired"
   (set to 0) but, for instance, if any software uses that for some "special"
   data (eg. as copy protection), "repairing" it would damage the software.
   So, leaving this code as it is (only as diagnostics) until. eventually,
   some other rationale for "repairing" comes up.
*/


static void checkNonzeroBMpagesBeyondBMsizeRoot(
    const char * const  callerFuncName,
    const uint32_t      firstFree,
    const uint32_t      volBitmapSize,
    const int32_t       rootBmPages[ ADF_BM_PAGES_ROOT_SIZE ] )
{
    for ( uint32_t i = firstFree; i < ADF_BM_PAGES_ROOT_SIZE; i++ ) {
        if ( rootBmPages[ i ] != 0 )
            adfEnv.wFct( "%s: a non-zero (%u, 0x%02x) entry in rootblock "
                         "bmpage[%u] in a volume with bmpage size %d",
                         callerFuncName, rootBmPages[ i ], rootBmPages[ i ],
                         i, volBitmapSize );
    }
}

static void checkNonzeroBMpagesBeyondBMsizeExt(
    const char * const  callerFuncName,
    const uint32_t      firstFree,
    const unsigned      bmExt_i,
    const ADF_SECTNUM   bmExtSect,
    const uint32_t      volBitmapSize,
    const int32_t       bmExtBlockBmPages[ ADF_BM_PAGES_EXT_SIZE ] )
{
    for ( uint32_t i = firstFree; i < ADF_BM_PAGES_EXT_SIZE; i++ ) {
        if ( bmExtBlockBmPages[ i ] != 0 )
            adfEnv.wFct(
                "%s: a non-zero (%u, 0x%02x) entry in bm ext. %u (%d) "
                "bmext.bmpage[%u] (bmpage %u), volume bmpage blocks %d",
                callerFuncName, bmExtBlockBmPages[ i ], bmExtBlockBmPages[ i ],
                bmExt_i, bmExtSect, i,
                i + ADF_BM_PAGES_ROOT_SIZE + bmExt_i * ADF_BM_PAGES_EXT_SIZE,
                volBitmapSize );
    }
}

#endif
