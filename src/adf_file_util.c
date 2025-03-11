
#include "adf_file_util.h"

/*
 * adfFilePos2DataBlock
 *
 */
int32_t adfFilePos2DataBlock( const unsigned    pos,
                              const unsigned    blockSize,
                              unsigned * const  posInExtBlk,
                              unsigned * const  posInDataBlk,
                              unsigned * const  curDataN )
{
    *posInDataBlk = pos % blockSize;   // offset in the data block
    *curDataN     = pos / blockSize;   // index of the data block
    if ( *curDataN < ADF_MAX_DATABLK ) {
        *posInExtBlk = 0;
        return -1;
    }
    else {
        // size of data allocated in file header or by a single ext. block
        unsigned dataSizeByExtBlock = //72 * blockSize;
            blockSize * ADF_MAX_DATABLK;

        // data offset starting from the 1st allocation done in ext. blocks
        // (without data allocated in the file header)
        unsigned offsetInExt = pos - dataSizeByExtBlock;

        // ext. block index
        unsigned extBlock = offsetInExt / dataSizeByExtBlock;

        // data block index in ext. block
        *posInExtBlk = ( offsetInExt / blockSize ) % ADF_MAX_DATABLK;

        return (int32_t) extBlock;
    }
}


/*
 * adfFileRealSize
 *
 * Compute and return real number of block used by one file
 * Compute number of datablocks and file extension blocks
 *
 */
uint32_t adfFileRealSize( const uint32_t    size,
                          const unsigned    blockSize,
                          uint32_t * const  dataN,
                          uint32_t * const  extN )
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

    if ( dataN )
        *dataN = data;
    if ( extN )
        *extN = ext;

    return ( ext + data + 1 );
}
