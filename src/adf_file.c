/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 *
 *  adf_file.c
 *
 *  $Id$
 *
 *  file code
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

#include<stdlib.h>
#include<string.h>

#include"adf_util.h"
#include"adf_file.h"
#include"adf_str.h"
#include"defendian.h"
#include"adf_raw.h"
#include "adf_vol.h"
#include"adf_dir.h"
#include"adf_bitm.h"
#include"adf_cache.h"
#include "adf_env.h"
#include "adf_dev.h"


// debugging
//#define DEBUG_ADF_FILE

#ifdef DEBUG_ADF_FILE

static void show_File ( const struct AdfFile * const file );

static void show_bFileHeaderBlock (
    const struct bFileHeaderBlock * const block );

static void show_bFileExtBlock (
    const struct bFileExtBlock * const block );
#endif

void adfFileTruncate ( struct AdfVolume * vol,
                       SECTNUM            nParent,
                       char *             name )
{

}


/*
 * adfFileFlush
 *
 */
void adfFlushFile(struct AdfFile * file)
{
    struct bEntryBlock parent;
    struct bOFSDataBlock *data;

    if (file->currentExt) {
        if (file->writeMode)
            adfWriteFileExtBlock(file->volume, file->currentExt->headerKey,
                file->currentExt);
    }
    if (file->currentData) {
        if (file->writeMode) {
            file->fileHdr->byteSize = file->pos;
	        if (isOFS(file->volume->dosType)) {
                data = (struct bOFSDataBlock *)file->currentData;
                data->dataSize = file->posInDataBlk;
            }
            if (file->fileHdr->byteSize>0)
                adfWriteDataBlock(file->volume, file->curDataPtr, 
				    file->currentData);
        }
    }
    if (file->writeMode) {
        file->fileHdr->byteSize = file->pos;
/*printf("pos=%ld\n",file->pos);*/
        adfTime2AmigaTime(adfGiveCurrentTime(),
            &(file->fileHdr->days),&(file->fileHdr->mins),&(file->fileHdr->ticks) );
        adfWriteFileHdrBlock(file->volume, file->fileHdr->headerKey, file->fileHdr);

	    if (isDIRCACHE(file->volume->dosType)) {
/*printf("parent=%ld\n",file->fileHdr->parent);*/
            adfReadEntryBlock(file->volume, file->fileHdr->parent, &parent);
            adfUpdateCache(file->volume, &parent, (struct bEntryBlock*)file->fileHdr,FALSE);
        }
        adfUpdateBitmap(file->volume);
    }
}


/*
 * adfGetFileBlocks
 *
 */
RETCODE adfGetFileBlocks ( struct AdfVolume *        vol,
                           struct bFileHeaderBlock * entry,
                           struct AdfFileBlocks *    fileBlocks )
{
    int32_t n, m;
    SECTNUM nSect;
    struct bFileExtBlock extBlock;
    int32_t i;

    fileBlocks->header = entry->headerKey;
    adfFileRealSize( entry->byteSize, vol->datablockSize, 
        &(fileBlocks->nbData), &(fileBlocks->nbExtens) );

    fileBlocks->data=(SECTNUM*)malloc(fileBlocks->nbData * sizeof(SECTNUM));
    if (!fileBlocks->data) {
        (*adfEnv.eFct)("adfGetFileBlocks : malloc");
        return RC_MALLOC;
    }

    fileBlocks->extens=(SECTNUM*)malloc(fileBlocks->nbExtens * sizeof(SECTNUM));
    if (!fileBlocks->extens) {
        (*adfEnv.eFct)("adfGetFileBlocks : malloc");
        return RC_MALLOC;
    }
 
    n = m = 0;	
    /* in file header block */
    for(i=0; i<entry->highSeq; i++)
        fileBlocks->data[n++] = entry->dataBlocks[MAX_DATABLK-1-i];

    /* in file extension blocks */
    nSect = entry->extension;
    while(nSect!=0) {
        fileBlocks->extens[m++] = nSect;
        adfReadFileExtBlock(vol, nSect, &extBlock);
        for(i=0; i<extBlock.highSeq; i++)
            fileBlocks->data[n++] = extBlock.dataBlocks[MAX_DATABLK-1-i];
        nSect = extBlock.extension;
    }
    if ( (fileBlocks->nbExtens+fileBlocks->nbData) != (n+m) )
        (*adfEnv.wFct)("adfGetFileBlocks : less blocks than expected");

    return RC_OK;
}

/*
 * adfFreeFileBlocks
 *
 */
RETCODE adfFreeFileBlocks ( struct AdfVolume *        vol,
                            struct bFileHeaderBlock * entry )
{
    int i;
    struct AdfFileBlocks fileBlocks;
    RETCODE rc = RC_OK;

    adfGetFileBlocks(vol,entry,&fileBlocks);

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
int32_t adfFileRealSize(uint32_t size, int blockSize, int32_t *dataN, int32_t *extN)
{
    int32_t data, ext;

   /*--- number of data blocks ---*/
    data = size / blockSize;
    if ( size % blockSize )
        data++;

    /*--- number of header extension blocks ---*/
    ext = 0;
    if (data>MAX_DATABLK) {
        ext = (data-MAX_DATABLK) / MAX_DATABLK;
        if ( (data-MAX_DATABLK) % MAX_DATABLK )
            ext++;
    }

    if (dataN)
        *dataN = data;
    if (extN)
        *extN = ext;
		
    return(ext+data+1);
}


/*
 * adfWriteFileHdrBlock
 *
 */
RETCODE adfWriteFileHdrBlock ( struct AdfVolume *        vol,
                               SECTNUM                   nSect,
                               struct bFileHeaderBlock * fhdr )
{
    uint8_t buf[512];
    uint32_t newSum;
    RETCODE rc = RC_OK;
/*printf("adfWriteFileHdrBlock %ld\n",nSect);*/
    fhdr->type = T_HEADER;
    fhdr->dataSize = 0;
    fhdr->secType = ST_FILE;

    memcpy(buf, fhdr, sizeof(struct bFileHeaderBlock));
#ifdef LITT_ENDIAN
    swapEndian(buf, SWBL_FILE);
#endif
    newSum = adfNormalSum(buf,20,sizeof(struct bFileHeaderBlock));
    swLong(buf+20, newSum);
/*    *(uint32_t*)(buf+20) = swapLong((uint8_t*)&newSum);*/

    adfWriteBlock(vol, nSect, buf);

    return rc;
}


/*
 * adfFileSeek
 *
 */

static void adfFileSeekStart ( struct AdfFile * file )
{
    file->pos = 0;
    file->posInExtBlk = 0;
    file->posInDataBlk = 0;
    file->nDataBlock = 0;

    adfReadNextFileBlock ( file );
}


static void adfFileSeekOFS ( struct AdfFile * file,
                             uint32_t         pos )
{
    adfFileSeekStart ( file );

    int blockSize = file->volume->datablockSize;

    if ( file->pos + pos > file->fileHdr->byteSize )
        pos = file->fileHdr->byteSize - file->pos;

    int32_t offset = 0;
    while ( offset < pos ) {
        int size = min ( pos - offset, blockSize - file->posInDataBlk );
        file->pos += size;
        offset += size;
        file->posInDataBlk += size;
        if ( file->posInDataBlk == blockSize && offset < pos ) {
            adfReadNextFileBlock ( file );
            file->posInDataBlk = 0;
        }
    }
    file->eof = ( file->pos == file->fileHdr->byteSize );
}


static RETCODE adfFileSeekExt ( struct AdfFile * file,
                                uint32_t         pos )
{
    file->pos = min ( pos, file->fileHdr->byteSize );

    SECTNUM extBlock = adfPos2DataBlock ( file->pos,
                                          file->volume->datablockSize,
                                          &file->posInExtBlk,
                                          &file->posInDataBlk,
                                          &file->nDataBlock );
    if ( extBlock == -1 ) {
        file->curDataPtr = file->fileHdr->dataBlocks [
            MAX_DATABLK - 1 - file->nDataBlock ];
    } else {
        if ( ! file->currentExt ) {
            file->currentExt = ( struct bFileExtBlock * )
                malloc ( sizeof ( struct bFileExtBlock ) );
            if ( ! file->currentExt ) {
                (*adfEnv.eFct)( "adfFileSeekExt : malloc" );
                return RC_ERROR;
            }
        }

        if ( adfReadFileExtBlockN ( file, extBlock, file->currentExt ) != RC_OK ) {
            (*adfEnv.wFct)("adfFileSeekExt: error");
            return RC_ERROR;
        }

        file->curDataPtr = file->currentExt->dataBlocks [
            MAX_DATABLK - 1 - file->posInExtBlk ];
    }

    adfReadDataBlock ( file->volume,
                       file->curDataPtr,
                       file->currentData );

    file->eof = ( file->pos == file->fileHdr->byteSize );

    return RC_OK;
}

void adfFileSeek(struct AdfFile * file, uint32_t pos)
{
    if ( file->pos == pos )
        return;

    RETCODE status = adfFileSeekExt ( file, pos );

    if ( status != RC_OK && isOFS ( file->volume->dosType ) )
        adfFileSeekOFS ( file, pos );
}


/*
 * adfFileOpen
 *
 */ 
struct AdfFile * adfOpenFile ( struct AdfVolume * vol,
                               char *             name,
                               char *             mode )
{
    struct AdfFile *file;
    SECTNUM nSect;
    struct bEntryBlock entry, parent;
    BOOL write;
    char filename[200];

    write=( strcmp("w",mode)==0 || strcmp("a",mode)==0 );
    
	if (write && vol->dev->readOnly) {
        (*adfEnv.wFct)("adfFileOpen : device is mounted 'read only'");
        return NULL;
    }

    adfReadEntryBlock(vol, vol->curDirPtr, &parent);

    nSect = adfNameToEntryBlk(vol, parent.hashTable, name, &entry, NULL);
    if (!write && nSect==-1) {
        sprintf(filename,"adfFileOpen : file \"%s\" not found.",name);
        (*adfEnv.wFct)(filename);
/*fprintf(stdout,"filename %s %d, parent =%d\n",name,strlen(name),vol->curDirPtr);*/
		 return NULL; 
    }
    if (!write && hasR(entry.access)) {
        (*adfEnv.wFct)("adfFileOpen : access denied"); return NULL; }
/*    if (entry.secType!=ST_FILE) {
        (*adfEnv.wFct)("adfFileOpen : not a file"); return NULL; }
	if (write && (hasE(entry.access)||hasW(entry.access))) {
        (*adfEnv.wFct)("adfFileOpen : access denied"); return NULL; }  
*/    if (write && nSect!=-1) {
        (*adfEnv.wFct)("adfFileOpen : file already exists"); return NULL; }  


    // if the file already exists...
    if ( nSect != -1 ) {
        // ... and it is a hard-link...
        if ( entry.realEntry )  {
            // ... load entry of the hard-linked file
            adfReadEntryBlock ( vol, entry.realEntry, &entry );
            adfReadEntryBlock ( vol, entry.parent, &parent );
        }

        // entry should be a real file now
        if ( entry.realEntry != 0 ) {
            //... so if it is still a (hard)link - error...
            return NULL;
        }
    }

    file = (struct AdfFile *) malloc (sizeof(struct AdfFile));
    if (!file) { (*adfEnv.wFct)("adfFileOpen : malloc"); return NULL; }
    file->fileHdr = (struct bFileHeaderBlock*)malloc(sizeof(struct bFileHeaderBlock));
    if (!file->fileHdr) {
		(*adfEnv.wFct)("adfFileOpen : malloc"); 
		free(file); return NULL; 
    }
    file->currentData = malloc(512*sizeof(uint8_t));
    if (!file->currentData) { 
		(*adfEnv.wFct)("adfFileOpen : malloc"); 
        free(file->fileHdr); free(file); return NULL; 
    }

    file->volume = vol;
    file->pos = 0;
    file->posInExtBlk = 0;
    file->posInDataBlk = 0;
    file->writeMode = write;
    file->currentExt = NULL;
    file->nDataBlock = 0;

    if (strcmp("w",mode)==0) {
        memset(file->fileHdr,0,512);
        adfCreateFile(vol,vol->curDirPtr,name,file->fileHdr);
        file->eof = TRUE;
    }
    else if (strcmp("a",mode)==0) {
        memcpy(file->fileHdr,&entry,sizeof(struct bFileHeaderBlock));
        file->eof = TRUE;
        adfFileSeek(file, file->fileHdr->byteSize);
    }
    else if (strcmp("r",mode)==0) {
        memcpy(file->fileHdr,&entry,sizeof(struct bFileHeaderBlock));
        file->eof = FALSE;
    }

/*puts("adfOpenFile");*/
    return(file);
}


/*
 * adfCloseFile
 *
 */
void adfCloseFile(struct AdfFile * file)
{

    if (file==0)
        return;
/*puts("adfCloseFile in");*/

    adfFlushFile(file);

    if (file->currentExt)
        free(file->currentExt);
    
    if (file->currentData)
        free(file->currentData);
    
    free(file->fileHdr);
    free(file);

/*puts("adfCloseFile out");*/
}


/*
 * adfReadFile
 *
 */
int32_t adfReadFile(struct AdfFile * file, int32_t n, uint8_t *buffer)
{
    int32_t bytesRead;
    uint8_t *dataPtr, *bufPtr;
	int blockSize, size;

    if (n==0) return(n);
    blockSize = file->volume->datablockSize;
/*puts("adfReadFile");*/
    if (file->pos+n > file->fileHdr->byteSize)
        n = file->fileHdr->byteSize - file->pos;

    if (isOFS(file->volume->dosType))
        dataPtr = (uint8_t*)(file->currentData)+24;
    else
        dataPtr = file->currentData;

    if (file->pos==0 || file->posInDataBlk==blockSize) {
        adfReadNextFileBlock(file);
        file->posInDataBlk = 0;
    }

    bytesRead = 0; bufPtr = buffer;
    size = 0;
    while ( bytesRead < n ) {
        size = min(n-bytesRead, blockSize-file->posInDataBlk);
        memcpy(bufPtr, dataPtr+file->posInDataBlk, size);
        bufPtr += size;
        file->pos += size;
        bytesRead += size;
        file->posInDataBlk += size;
        if (file->posInDataBlk==blockSize && bytesRead<n) {
            adfReadNextFileBlock(file);
            file->posInDataBlk = 0;
        }
    }
    file->eof = (file->pos==file->fileHdr->byteSize);
    return( bytesRead );
}


/*
 * adfEndOfFile
 *
 */
BOOL adfEndOfFile(struct AdfFile* file)
{
    return(file->eof);
}


/*
 * adfReadNextFileBlock
 *
 */
RETCODE adfReadNextFileBlock(struct AdfFile* file)
{
    SECTNUM nSect;
    struct bOFSDataBlock *data;
    RETCODE rc = RC_OK;

    data =(struct bOFSDataBlock *) file->currentData;

    if (file->nDataBlock==0) {
        nSect = file->fileHdr->firstData;
    }
    else if (isOFS(file->volume->dosType)) {
        nSect = data->nextData;
    }
    else {
        if (file->nDataBlock<MAX_DATABLK)
            nSect = file->fileHdr->dataBlocks[MAX_DATABLK-1-file->nDataBlock];
        else {
            if (file->nDataBlock==MAX_DATABLK) {
                file->currentExt=(struct bFileExtBlock*)malloc(sizeof(struct bFileExtBlock));
                if (!file->currentExt) (*adfEnv.eFct)("adfReadNextFileBlock : malloc");
                adfReadFileExtBlock(file->volume, file->fileHdr->extension,
                    file->currentExt);
                file->posInExtBlk = 0;
            }
            else if (file->posInExtBlk==MAX_DATABLK) {
                adfReadFileExtBlock(file->volume, file->currentExt->extension,
                    file->currentExt);
                file->posInExtBlk = 0;
            }
            nSect = file->currentExt->dataBlocks[MAX_DATABLK-1-file->posInExtBlk];
            file->posInExtBlk++;
        }
    }
    adfReadDataBlock(file->volume,nSect,file->currentData);

    if (isOFS(file->volume->dosType) && data->seqNum!=file->nDataBlock+1)
        (*adfEnv.wFct)("adfReadNextFileBlock : seqnum incorrect");

    file->nDataBlock++;

    return rc;
}


/*
 * adfWriteFile
 *
 */
int32_t adfWriteFile(struct AdfFile *file, int32_t n, uint8_t *buffer)
{
    int32_t bytesWritten;
    uint8_t *dataPtr, *bufPtr;
    int size, blockSize;
    struct bOFSDataBlock *dataB;
    
    bytesWritten = 0;
    if (n==0) return (n);
/*puts("adfWriteFile");*/
    blockSize = file->volume->datablockSize;
    if (isOFS(file->volume->dosType)) {
        dataB =(struct bOFSDataBlock *)file->currentData;
        dataPtr = dataB->data;
    }
    else
        dataPtr = file->currentData;

    if (file->pos==0 || file->posInDataBlk==blockSize) {
        if (adfCreateNextFileBlock(file)==-1) {
            /* bug found by Rikard */
            (*adfEnv.wFct)("adfWritefile : no more free sector availbale");                        
            return bytesWritten;
        }
        file->posInDataBlk = 0;
    }

    bytesWritten = 0; bufPtr = buffer;
    while( bytesWritten<n ) {
        size = min(n-bytesWritten, blockSize-file->posInDataBlk);
        memcpy(dataPtr+file->posInDataBlk, bufPtr, size);
        bufPtr += size;
        file->pos += size;
        bytesWritten += size;
        file->posInDataBlk += size;
        if (file->posInDataBlk==blockSize && bytesWritten<n) {
            if (adfCreateNextFileBlock(file)==-1) {
                /* bug found by Rikard */
                (*adfEnv.wFct)("adfWritefile : no more free sector availbale");                        
                return bytesWritten;
            }
            file->posInDataBlk = 0;
        }
    }
    return( bytesWritten );
}


/*
 * adfCreateNextFileBlock
 *
 */
SECTNUM adfCreateNextFileBlock(struct AdfFile* file)
{
    SECTNUM nSect, extSect;
    struct bOFSDataBlock *data;
	unsigned int blockSize;
    int i;
/*puts("adfCreateNextFileBlock");*/
    blockSize = file->volume->datablockSize;
    data = file->currentData;

    /* the first data blocks pointers are inside the file header block */
    if (file->nDataBlock<MAX_DATABLK) {
        nSect = adfGet1FreeBlock(file->volume);
        if (nSect==-1) return -1;
/*printf("adfCreateNextFileBlock fhdr %ld\n",nSect);*/
        if (file->nDataBlock==0)
            file->fileHdr->firstData = nSect;
        file->fileHdr->dataBlocks[MAX_DATABLK-1-file->nDataBlock] = nSect;
        file->fileHdr->highSeq++;
    }
    else {
        /* one more sector is needed for one file extension block */
        if ((file->nDataBlock%MAX_DATABLK)==0) {
            extSect = adfGet1FreeBlock(file->volume);
/*printf("extSect=%ld\n",extSect);*/
            if (extSect==-1) return -1;

            /* the future block is the first file extension block */
            if (file->nDataBlock==MAX_DATABLK) {
                file->currentExt=(struct bFileExtBlock*)malloc(sizeof(struct bFileExtBlock));
                if (!file->currentExt) {
                    adfSetBlockFree(file->volume, extSect);
                    (*adfEnv.eFct)("adfCreateNextFileBlock : malloc");
                    return -1;
                }
                file->fileHdr->extension = extSect;
            }

            /* not the first : save the current one, and link it with the future */
            if (file->nDataBlock>=2*MAX_DATABLK) {
                file->currentExt->extension = extSect;
/*printf ("write ext=%d\n",file->currentExt->headerKey);*/
                adfWriteFileExtBlock(file->volume, file->currentExt->headerKey,
                    file->currentExt);
            }

            /* initializes a file extension block */
            for(i=0; i<MAX_DATABLK; i++)
                file->currentExt->dataBlocks[i] = 0L;
            file->currentExt->headerKey = extSect;
            file->currentExt->parent = file->fileHdr->headerKey;
            file->currentExt->highSeq = 0L;
            file->currentExt->extension = 0L;
            file->posInExtBlk = 0L;
/*printf("extSect=%ld\n",extSect);*/
        }
        nSect = adfGet1FreeBlock(file->volume);
        if (nSect==-1) 
            return -1;
        
/*printf("adfCreateNextFileBlock ext %ld\n",nSect);*/

        file->currentExt->dataBlocks[MAX_DATABLK-1-file->posInExtBlk] = nSect;
        file->currentExt->highSeq++;
        file->posInExtBlk++;
    }

    /* builds OFS header */
    if (isOFS(file->volume->dosType)) {
        /* writes previous data block and link it  */
        if (file->pos>=blockSize) {
            data->nextData = nSect;
            adfWriteDataBlock(file->volume, file->curDataPtr, file->currentData);
/*printf ("writedata=%d\n",file->curDataPtr);*/
        }
        /* initialize a new data block */
        for(i=0; i<(int)blockSize; i++)
            data->data[i]=0;
        data->seqNum = file->nDataBlock+1;
        data->dataSize = blockSize;
        data->nextData = 0L;
        data->headerKey = file->fileHdr->headerKey;
    }
    else
        if (file->pos>=blockSize) {
            adfWriteDataBlock(file->volume, file->curDataPtr, file->currentData);
/*printf ("writedata=%d\n",file->curDataPtr);*/
            memset(file->currentData,0,512);
        }
            
/*printf("datablk=%d\n",nSect);*/
    file->curDataPtr = nSect;
    file->nDataBlock++;

    return(nSect);
}


/*
 * adfPos2DataBlock
 *
 */
int32_t adfPos2DataBlock(int32_t pos, int blockSize, 
    int *posInExtBlk, int *posInDataBlk, int32_t *curDataN )
{
    *posInDataBlk = pos % blockSize;   // offset in the data block
    *curDataN     = pos / blockSize;   // number of the data block
    if ( *curDataN < MAX_DATABLK ) {
        *posInExtBlk = 0;
        return -1;
    }
    else {
        // size of data allocated in file header or by a single ext. block
        int32_t dataSizeByExtBlock = //72 * blockSize;
            blockSize * MAX_DATABLK;

        // data offset starting from the 1st allocation done in ext. blocks
        // (without data allocated in the file header)
        int32_t offsetInExt = pos - dataSizeByExtBlock;

        // ext. block number
        int32_t extBlock = offsetInExt / dataSizeByExtBlock;

        // data block index in ext. block
        *posInExtBlk = ( offsetInExt / blockSize ) % MAX_DATABLK;

        return extBlock;
    }
}


/*
 * adfReadDataBlock
 *
 */
RETCODE adfReadDataBlock ( struct AdfVolume * vol,
                           SECTNUM            nSect,
                           void *             data )
{
    uint8_t buf[512];
    struct bOFSDataBlock *dBlock;
    RETCODE rc = RC_OK;

    adfReadBlock(vol, nSect,buf);

    memcpy(data,buf,512);

    if (isOFS(vol->dosType)) {
#ifdef LITT_ENDIAN
        swapEndian(data, SWBL_DATA);
#endif
        dBlock = (struct bOFSDataBlock*)data;
/*printf("adfReadDataBlock %ld\n",nSect);*/

        if (dBlock->checkSum!=adfNormalSum(buf,20,sizeof(struct bOFSDataBlock)))
            (*adfEnv.wFct)("adfReadDataBlock : invalid checksum");
        if (dBlock->type!=T_DATA)
            (*adfEnv.wFct)("adfReadDataBlock : id T_DATA not found");
        if (dBlock->dataSize<0 || dBlock->dataSize>488)
            (*adfEnv.wFct)("adfReadDataBlock : dataSize incorrect");
        if ( !isSectNumValid(vol,dBlock->headerKey) )
			(*adfEnv.wFct)("adfReadDataBlock : headerKey out of range");
        if ( !isSectNumValid(vol,dBlock->nextData) )
			(*adfEnv.wFct)("adfReadDataBlock : nextData out of range");
    }

    return rc;
}


/*
 * adfWriteDataBlock
 *
 */
RETCODE adfWriteDataBlock ( struct AdfVolume * vol,
                            SECTNUM            nSect,
                            void *             data )
{
    uint8_t buf[512];
    uint32_t newSum;
    struct bOFSDataBlock *dataB;
    RETCODE rc = RC_OK;

    newSum = 0L;
    if (isOFS(vol->dosType)) {
        dataB = (struct bOFSDataBlock *)data;
        dataB->type = T_DATA;
        memcpy(buf,dataB,512);
#ifdef LITT_ENDIAN
        swapEndian(buf, SWBL_DATA);
#endif
        newSum = adfNormalSum(buf,20,512);
        swLong(buf+20,newSum);
/*        *(int32_t*)(buf+20) = swapLong((uint8_t*)&newSum);*/
        adfWriteBlock(vol,nSect,buf);
    }
    else {
        adfWriteBlock(vol,nSect,data);
    }
/*printf("adfWriteDataBlock %ld\n",nSect);*/

    return rc;
}


/*
 * adfReadFileExtBlock
 *
 */
RETCODE adfReadFileExtBlock ( struct AdfVolume *     vol,
                              SECTNUM                nSect,
                              struct bFileExtBlock * fext )
{
    uint8_t buf[sizeof(struct bFileExtBlock)];
    RETCODE rc = RC_OK;

    adfReadBlock(vol, nSect,buf);
/*printf("read fext=%d\n",nSect);*/
    memcpy(fext,buf,sizeof(struct bFileExtBlock));
#ifdef LITT_ENDIAN
    swapEndian((uint8_t*)fext, SWBL_FEXT);
#endif
    if (fext->checkSum!=adfNormalSum(buf,20,sizeof(struct bFileExtBlock)))
        (*adfEnv.wFct)("adfReadFileExtBlock : invalid checksum");
    if (fext->type!=T_LIST)
        (*adfEnv.wFct)("adfReadFileExtBlock : type T_LIST not found");
    if (fext->secType!=ST_FILE)
        (*adfEnv.wFct)("adfReadFileExtBlock : stype  ST_FILE not found");
    if (fext->headerKey!=nSect)
        (*adfEnv.wFct)("adfReadFileExtBlock : headerKey!=nSect");
    if (fext->highSeq<0 || fext->highSeq>MAX_DATABLK)
        (*adfEnv.wFct)("adfReadFileExtBlock : highSeq out of range");
    if ( !isSectNumValid(vol, fext->parent) ) 
        (*adfEnv.wFct)("adfReadFileExtBlock : parent out of range");
    if ( fext->extension!=0 && !isSectNumValid(vol, fext->extension) )
        (*adfEnv.wFct)("adfReadFileExtBlock : extension out of range");

    return rc;
}

/*
 * adfReadFileExtBlockN
 *
 */
RETCODE adfReadFileExtBlockN ( struct AdfFile *       file,
                               int32_t                extBlock,
                               struct bFileExtBlock * fext )
{
    // add checking if extBlock value is valid (?)

    // traverse the ext. blocks until finding (and reading)
    // the requested one
    SECTNUM nSect = file->fileHdr->extension;
    int32_t i = -1;
    while ( i < extBlock && nSect != 0 ) {
        adfReadFileExtBlock ( file->volume, nSect, fext );
#ifdef DEBUG_ADF_FILE
        show_bFileExtBlock ( file->currentExt );
#endif
        nSect = file->currentExt->extension;
        i++;
    }
    if ( i != extBlock ) {
        (*adfEnv.wFct)("adfReadFileExtBlockN: error");
        return RC_ERROR;
    }
    return RC_OK;
}


/*
 * adfWriteFileExtBlock
 *
 */
RETCODE adfWriteFileExtBlock ( struct AdfVolume *     vol,
                               SECTNUM                nSect,
                               struct bFileExtBlock * fext )
{
    uint8_t buf[512];
    uint32_t newSum;
    RETCODE rc = RC_OK;

    fext->type = T_LIST;
    fext->secType = ST_FILE;
    fext->dataSize = 0L;
    fext->firstData = 0L;

    memcpy(buf,fext,512);
#ifdef LITT_ENDIAN
    swapEndian(buf, SWBL_FEXT);
#endif
    newSum = adfNormalSum(buf,20,512);
    swLong(buf+20,newSum);
/*    *(int32_t*)(buf+20) = swapLong((uint8_t*)&newSum);*/

    adfWriteBlock(vol,nSect,buf);

    return rc;
}
/*###########################################################################*/

#ifdef DEBUG_ADF_FILE

static void show_File ( const struct AdfFile * const file )
{
    printf ( "\nstruct File:\n"
             //"volume:\t0x%x
             //fileHdr;
             //currentData;
             //struct bFileExtBlock* currentExt;
             "  nDataBlock:\t0x%x\t\t%u\n"
             "  curDataPtr:\t0x%x\t\t%u\n"
             "  pos:\t\t0x%x\t\t%u\n"
             "  posInDataBlk:\t0x%x\t\t%u\n"
             "  posInExtBlk:\t0x%x\t\t%u\n"
             "  eof:\t\t0x%x\t\t%u\n"
             "  writeMode:\t0x%x\t\t%u\n",
             //volume;
             //fileHdr;
             //currentData;
             //struct bFileExtBlock* currentExt;
             file->nDataBlock,
             file->nDataBlock,
             file->curDataPtr,
             file->curDataPtr,
             file->pos,
             file->pos,
             file->posInDataBlk,
             file->posInDataBlk,
             file->posInExtBlk,
             file->posInExtBlk,
             file->eof,
             file->eof,
             file->writeMode,
             file->writeMode );
}

static void show_bFileHeaderBlock (
    const struct bFileHeaderBlock * const block )
{
    printf ( "\nbFileHeaderBlock:\n"
             "  type:\t\t%d\n"
             "  headerKey:\t%d\n"
             "  highSeq:\t%d\n"
             "  dataSize:\t%d\n"
             "  firstData:\t%d\n"
             "  checkSum:\t%d\n"
             //"	dataBlocks[MAX_DATABLK]:\n"
             "  r1:\t\t%d\n"
             "  r2:\t\t%d\n"
             "  access:\t%d\n"
             "  byteSize:\t%d\n"
             "  commLen:\t%d\n"
             "  comment:\t%s\n"
             "  r3:\t\t%s\n"
             "  days:\t\t%d\n"
             "  mins:\t\t%d\n"
             "  ticks:\t%d\n"
             "  nameLen:\t%d\n"
             "  fileName:\t%s\n"
             "  r4:\t\t%d\n"
             "  real:\t\t%d\n"
             "  nextLink:\t%d\n"
             "  r5[5]:\t%d\n"
             "  nextSameHash:\t%d\n"
             "  parent:\t%d\n"
             "  extension:\t%d\n"
             "  secType:\t%d\n",
             block->type,		/* == 2 */
             block->headerKey,	/* current block number */
             block->highSeq,	/* number of data block in this hdr block */
             block->dataSize,	/* == 0 */
             block->firstData,
             block->checkSum,
             //dataBlocks[MAX_DATABLK],
             block->r1,
             block->r2,
             block->access,	/* bit0=del, 1=modif, 2=write, 3=read */
             block->byteSize,
             block->commLen,
             block->comment,
             block->r3,
             block->days,
             block->mins,
             block->ticks,
             block->nameLen,
             block->fileName,
             block->r4,
             block->real,		/* unused == 0 */
             block->nextLink,	/* link chain */
             block->r5,
             block->nextSameHash,	/* next entry with sane hash */
             block->parent,		/* parent directory */
             block->extension,	/* pointer to extension block */
             block->secType );	/* == -3 */
}

static void show_bFileExtBlock (
    const struct bFileExtBlock * const block )
{
    printf ( "\nbFileExtBlock:\n"
             "  type:\t\t0x%x\t\t%u\n"
             "  headerKey:\t0x%x\t\t%u\n"
             "  highSeq:\t0x%x\t\t%u\n"
             "  dataSize:\t0x%x\t\t%u\n"
             "  firstData:\t0x%x\t\t%u\n"
             "  checkSum:\t0x%x\t%u\n"
             //"dataBlocks[MAX_DATABLK]:\t%d\n""
             "  r[45]:\t0x%x\t%u\n"
             "  info:\t\t0x%x\t\t%u\n"
             "  nextSameHash:\t0x%x\t\t%u\n"
             "  parent:\t0x%x\t\t%u\n"
             "  extension:\t0x%x\t\t%u\n"
             "  secType:\t0x%x\t%d\n",

             block->type,		/* == 0x10 */
             block->type,
             block->headerKey,
             block->headerKey,
             block->highSeq,
             block->highSeq,
             block->dataSize,	/* == 0 */
             block->dataSize,
             block->firstData,	/* == 0 */
             block->firstData,
             block->checkSum,
             block->checkSum,
//block->dataBlocks[MAX_DATABLK],
             block->r,
             block->r,
             block->info,		/* == 0 */
             block->info,
             block->nextSameHash,	/* == 0 */
             block->nextSameHash,
             block->parent,		/* header block */
             block->parent,
             block->extension,	/* next header extension block */
             block->extension,
             block->secType,	/* -3 */
             block->secType );
}
#endif
