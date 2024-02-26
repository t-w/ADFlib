/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 *
 *  adf_file_block.h
 *
 *  $Id$
 *
 *  Volume block-level code for files
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


#include "adf_file_block.h"

#include "adf_bitm.h"
#include "adf_byteorder.h"
#include "adf_env.h"
#include "adf_raw.h"
#include "adf_util.h"

#include <stdlib.h>
#include <string.h>


/*
 * adfGetFileBlocks
 *
 */
RETCODE adfGetFileBlocks ( struct AdfVolume * const                vol,
                           const struct AdfFileHeaderBlock * const entry,
                           struct AdfFileBlocks * const            fileBlocks )
{
    int32_t n, m;
    SECTNUM nSect;
    int32_t i;

    fileBlocks->header = entry->headerKey;
    adfFileRealSize( entry->byteSize, vol->datablockSize, 
        &(fileBlocks->nbData), &(fileBlocks->nbExtens) );

    fileBlocks->data = (SECTNUM *)
        malloc ( (unsigned) fileBlocks->nbData * sizeof(SECTNUM) );
    if (!fileBlocks->data) {
        (*adfEnv.eFct)("adfGetFileBlocks : malloc");
        return RC_MALLOC;
    }

    fileBlocks->extens = (SECTNUM *)
        malloc ( (unsigned) fileBlocks->nbExtens * sizeof(SECTNUM) );
    if (!fileBlocks->extens) {
        (*adfEnv.eFct)("adfGetFileBlocks : malloc");
        return RC_MALLOC;
    }
 
    n = m = 0;	
    /* in file header block */
    for(i=0; i<entry->highSeq; i++)
        fileBlocks->data[ n++ ] = entry->dataBlocks[ ADF_MAX_DATABLK - 1 - i ];

    /* in file extension blocks */
    nSect = entry->extension;
    struct AdfFileExtBlock extBlock;
    while(nSect!=0) {
        fileBlocks->extens[m++] = nSect;
        adfReadFileExtBlock(vol, nSect, &extBlock);
        for(i=0; i<extBlock.highSeq; i++)
            fileBlocks->data[n++] = extBlock.dataBlocks[ ADF_MAX_DATABLK - 1 - i ];
        nSect = extBlock.extension;
    }
    if ( (n != fileBlocks->nbData) || (m != fileBlocks->nbExtens) ) {
        (*adfEnv.wFct)("adfGetFileBlocks : not as many blocks as expected");
        return RC_ERROR;
    }

    return RC_OK;
}

/*
 * adfFreeFileBlocks
 *
 */
RETCODE adfFreeFileBlocks ( struct AdfVolume * const          vol,
                            struct AdfFileHeaderBlock * const entry )
{
    int i;
    struct AdfFileBlocks fileBlocks;

    RETCODE rc = adfGetFileBlocks ( vol, entry, &fileBlocks );
    if ( rc != RC_OK )
        return rc;

    for(i=0; i<fileBlocks.nbData; i++) {
        adfSetBlockFree(vol, fileBlocks.data[i]);
    }
    for(i=0; i<fileBlocks.nbExtens; i++) {
        adfSetBlockFree(vol, fileBlocks.extens[i]);
    }

    free(fileBlocks.data);
    free(fileBlocks.extens);
		
    return rc;
}


/*
 * adfFileRealSize
 *
 * Compute and return real number of block used by one file
 * Compute number of datablocks and file extension blocks
 *
 */
uint32_t adfFileRealSize ( const uint32_t  size,
                           const unsigned  blockSize,
                           int32_t * const dataN,
                           int32_t * const extN )
{
    uint32_t data, ext;

   /*--- number of data blocks ---*/
    data = size / blockSize;
    if ( size % blockSize )
        data++;

    /*--- number of header extension blocks ---*/
    ext = 0;
    if ( data > ADF_MAX_DATABLK ) {
        ext = ( data - ADF_MAX_DATABLK ) / ADF_MAX_DATABLK;
        if ( ( data - ADF_MAX_DATABLK ) % ADF_MAX_DATABLK )
            ext++;
    }

    if (dataN)
        *dataN = (int32_t) data;
    if (extN)
        *extN = (int32_t) ext;
		
    return(ext+data+1);
}


/*
 * adfWriteFileHdrBlock
 *
 */
RETCODE adfWriteFileHdrBlock ( struct AdfVolume * const          vol,
                               const SECTNUM                     nSect,
                               struct AdfFileHeaderBlock * const fhdr )
{
    uint8_t buf[512];
    uint32_t newSum;
/*printf("adfWriteFileHdrBlock %ld\n",nSect);*/
    fhdr->type = ADF_T_HEADER;
    fhdr->dataSize = 0;
    fhdr->secType = ADF_ST_FILE;

    memcpy ( buf, fhdr, sizeof(struct AdfFileHeaderBlock) );
#ifdef LITT_ENDIAN
    adfSwapEndian ( buf, ADF_SWBL_FILE );
#endif
    newSum = adfNormalSum ( buf, 20, sizeof(struct AdfFileHeaderBlock) );
    swLong(buf+20, newSum);
/*    *(uint32_t*)(buf+20) = swapLong((uint8_t*)&newSum);*/

    return adfVolWriteBlock ( vol, (uint32_t) nSect, buf );
}



/*
 * adfReadDataBlock
 *
 */
RETCODE adfReadDataBlock ( struct AdfVolume * const vol,
                           const SECTNUM            nSect,
                           void * const             data )
{
    if ( nSect < 1 ) {
        // ie. block 0 is volume's bootblock - cannot be a data block
        adfEnv.eFct ( "adfReadDataBlock : error, '%d' cannot be a data block", nSect );
        return RC_ERROR;
    }

    uint8_t buf[512];

    RETCODE rc = adfVolReadBlock ( vol, (uint32_t) nSect, buf );
    if ( rc != RC_OK ) {
        adfEnv.eFct ( "adfReadDataBlock: error reading block %d, volume '%s'",
                       nSect, vol->volName );
        //return RC_ERROR;
    }

    memcpy(data,buf,512);

    if ( adfVolIsOFS ( vol ) ) {
#ifdef LITT_ENDIAN
        adfSwapEndian ( data, ADF_SWBL_DATA );
#endif
        struct AdfOFSDataBlock * const dBlock = (struct AdfOFSDataBlock *) data;
/*printf("adfReadDataBlock %ld\n",nSect);*/

        if ( dBlock->checkSum != adfNormalSum ( buf, 20, sizeof(struct AdfOFSDataBlock) ) )
            adfEnv.wFct ( "adfReadDataBlock : invalid checksum, block %d, volume '%s'",
                           nSect, vol->volName );
        if ( dBlock->type != ADF_T_DATA )
            adfEnv.wFct ( "adfReadDataBlock : id ADF_T_DATA not found, block %d, volume '%s'",
                           nSect, vol->volName );
        if ( dBlock->dataSize > 488 )
            adfEnv.wFct ( "adfReadDataBlock : dataSize (0x%x / %u) incorrect, block %d, volume '%s'",
                           dBlock->dataSize, dBlock->dataSize, nSect, vol->volName );
        if ( ! adfVolIsSectNumValid ( vol, dBlock->headerKey ) )
            adfEnv.wFct ( "adfReadDataBlock : headerKey (0x%x / %u) out of range, block %d, volume '%s'",
                           dBlock->headerKey, dBlock->headerKey, nSect, vol->volName );
        if ( ! adfVolIsSectNumValid ( vol, dBlock->nextData ) )
            adfEnv.wFct ( "adfReadDataBlock : nextData out of range, block %d, volume '%s'",
                           nSect, vol->volName );
    }

    return rc;
}


/*
 * adfWriteDataBlock
 *
 */
RETCODE adfWriteDataBlock ( struct AdfVolume * const vol,
                            const SECTNUM            nSect,
                            void * const            data )
{
    if ( nSect < 1 ) {
        // ie. block 0 is volume's bootblock - cannot be a data block
        adfEnv.eFct ( "adfWriteDataBlock : error, '%d' cannot be a data block",
                      nSect );
        return RC_ERROR;
    }

    RETCODE rc;
    if ( adfVolIsOFS ( vol ) ) {
        struct AdfOFSDataBlock * const dataB = (struct AdfOFSDataBlock *) data;
        dataB->type = ADF_T_DATA;

        uint8_t buf[512];
        memcpy ( buf, data, 512 );
#ifdef LITT_ENDIAN
        adfSwapEndian ( buf, ADF_SWBL_DATA );
#endif
        uint32_t newSum = adfNormalSum ( buf, 20, 512 );
        swLong(buf+20,newSum);
/*        *(int32_t*)(buf+20) = swapLong((uint8_t*)&newSum);*/
        rc = adfVolWriteBlock ( vol, (uint32_t) nSect, buf );
    } else {
        rc = adfVolWriteBlock ( vol, (uint32_t) nSect, data );
    }
    if ( rc != RC_OK ) {
        adfEnv.eFct ( "adfWriteDataBlock: error writing block %d, volume '%s'",
                      nSect, vol->volName );
        //return RC_ERROR;
    }
/*printf("adfWriteDataBlock %ld\n",nSect);*/

    return rc;
}


/*
 * adfReadFileExtBlock
 *
 */
RETCODE adfReadFileExtBlock ( struct AdfVolume * const       vol,
                              const SECTNUM                  nSect,
                              struct AdfFileExtBlock * const fext )
{
    uint8_t buf[ sizeof(struct AdfFileExtBlock) ];
    RETCODE rc = adfVolReadBlock ( vol, (uint32_t) nSect, buf );
    if ( rc != RC_OK ) {
        adfEnv.eFct ( "adfReadFileExtBlock: error reading block %d, volume '%s'",
                      nSect, vol->volName );
        //return RC_ERROR;
    }
/*printf("read fext=%d\n",nSect);*/
    memcpy ( fext, buf, sizeof(struct AdfFileExtBlock) );
#ifdef LITT_ENDIAN
    adfSwapEndian ( (uint8_t *) fext, ADF_SWBL_FEXT );
#endif
    if ( fext->checkSum != adfNormalSum ( buf, 20, sizeof(struct AdfFileExtBlock) ) )
        (*adfEnv.wFct)("adfReadFileExtBlock : invalid checksum");
    if ( fext->type != ADF_T_LIST )
        adfEnv.wFct ( "adfReadFileExtBlock : type ADF_T_LIST not found" );
    if ( fext->secType != ADF_ST_FILE )
        adfEnv.wFct ( "adfReadFileExtBlock : sectype ADF_ST_FILE not found" );
    if (fext->headerKey!=nSect)
        (*adfEnv.wFct)("adfReadFileExtBlock : headerKey!=nSect");
    if ( fext->highSeq < 0 ||
         fext->highSeq > ADF_MAX_DATABLK )
    {
        (*adfEnv.wFct)("adfReadFileExtBlock : highSeq out of range");
    }
    if ( ! adfVolIsSectNumValid ( vol, fext->parent ) )
        (*adfEnv.wFct)("adfReadFileExtBlock : parent out of range");
    if ( fext->extension != 0 && ! adfVolIsSectNumValid ( vol, fext->extension ) )
        (*adfEnv.wFct)("adfReadFileExtBlock : extension out of range");

    return rc;
}


/*
 * adfWriteFileExtBlock
 *
 */
RETCODE adfWriteFileExtBlock ( struct AdfVolume * const       vol,
                               const SECTNUM                  nSect,
                               struct AdfFileExtBlock * const fext )
{
    uint8_t buf[512];
    uint32_t newSum;

    fext->type = ADF_T_LIST;
    fext->secType = ADF_ST_FILE;
    fext->dataSize = 0L;
    fext->firstData = 0L;

    memcpy(buf,fext,512);
#ifdef LITT_ENDIAN
    adfSwapEndian ( buf, ADF_SWBL_FEXT );
#endif
    newSum = adfNormalSum(buf,20,512);
    swLong(buf+20,newSum);
/*    *(int32_t*)(buf+20) = swapLong((uint8_t*)&newSum);*/

    RETCODE rc = adfVolWriteBlock ( vol, (uint32_t) nSect, buf );
    if ( rc != RC_OK ) {
        adfEnv.eFct ( "adfWriteFileExtBlock: error wriding block %d, volume '%s'",
                      nSect, vol->volName );
    }

    return rc;
}
