
#ifndef ADF_FILE_UTIL_H
#define ADF_FILE_UTIL_H

#include "adf_blk.h"
#include "adf_prefix.h"

#include <assert.h>

ADF_PREFIX int32_t adfFilePos2DataBlock( const unsigned    pos,
                                         const unsigned    blockSize,
                                         unsigned * const  posInExtBlk,
                                         unsigned * const  posInDataBlk,
                                         unsigned * const  curDataN );

static inline unsigned adfFilePos2datablockIndex( const unsigned  pos,
                                                  const unsigned  blocksize )
{
    assert( blocksize > 0 );
    return ( pos / blocksize );
}

static inline unsigned adfFileSize2Datablocks( const unsigned  fsize,
                                               const unsigned  blocksize )
{
    assert( blocksize > 0 );
    return ( fsize / blocksize +
             ( ( fsize % blocksize > 0 ) ? 1 : 0 ) );
}

static inline unsigned adfFileDatablocks2Extblocks( const unsigned  ndatablocks )
{
    //return max ( ( ndata_blocks - 1 ) / ADF_MAX_DATABLK, 0 );
    if ( ndatablocks < 1 )
        return 0;
    return ( ndatablocks - 1 ) / ADF_MAX_DATABLK;
}

static inline unsigned adfFileSize2Extblocks( const unsigned   fsize,
                                              const  unsigned  blocksize )
{
    return adfFileDatablocks2Extblocks(
        adfFileSize2Datablocks( fsize, blocksize ) );
}


static inline unsigned adfFileSize2Blocks( const unsigned  fsize,
                                           const unsigned  blocksize )
{
    //assert ( blocksize > 0 );
    const unsigned data_blocks = adfFileSize2Datablocks( fsize, blocksize );
    const unsigned ext_blocks  = adfFileDatablocks2Extblocks( data_blocks );
    return data_blocks + ext_blocks + 1;   // +1 for the file header block
}

ADF_PREFIX uint32_t adfFileRealSize( const uint32_t    size,
                                     const unsigned    blockSize,
                                     uint32_t * const  dataN,
                                     uint32_t * const  extN );

#endif  /* ADF_FILE_UTIL_H */
