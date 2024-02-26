/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 *
 *  adf_salv.c
 *
 *  $Id$
 *
 * undelete and salvage code : EXPERIMENTAL !
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


#include "adf_salv.h"

#include "adf_bitm.h"
#include "adf_cache.h"
#include "adf_dir.h"
#include "adf_env.h"
#include "adf_file_block.h"
#include "adf_util.h"

#include <string.h>
#include <stdlib.h>


/*
 * adfFreeGenBlock
 *
 */
static void adfFreeGenBlock ( struct GenBlock * const block )
{
    if ( block != NULL ) {
        if ( block->name != NULL )
            free ( block->name );
        free ( block );
    }
}


/*
 * adfFreeDelList
 *
 */
void adfFreeDelList ( struct AdfList * const list )
{
    struct AdfList *cell;

    cell = list;
    while(cell!=NULL) {
        adfFreeGenBlock((struct GenBlock*)cell->content);
        cell = cell->next;
    }
    freeList(list);
}


/*
 * adfGetDelEnt
 *
 */
struct AdfList * adfGetDelEnt ( struct AdfVolume * const vol )
{
    struct GenBlock *block;
    int32_t i;
    struct AdfList *list, *head;

    list = head = NULL;
    block = NULL;
    bool delEnt = true;
    for(i=vol->firstBlock + 2 ; i<=vol->lastBlock; i++) {
        if (adfIsBlockFree(vol, i)) {
            if (delEnt) {
                block = (struct GenBlock*)malloc(sizeof(struct GenBlock));
                if ( block == NULL ) {
                    adfFreeDelList ( head );
                    return NULL;
                }
/*printf("%p\n",block);*/
            }

            if ( adfReadGenBlock ( vol, i, block ) != RC_OK ) {
                free(block);
                adfFreeDelList ( head );
                return NULL;
            }

            delEnt = block->type == ADF_T_HEADER &&
                     ( block->secType == ADF_ST_DIR ||
                       block->secType == ADF_ST_FILE );

            if (delEnt) {
                if (head==NULL)
                    list = head = newCell(NULL, (void*)block);
                else
                    list = newCell(list, (void*)block);

                block = NULL;
            }
        }
    }

    if (block!=NULL) {
        free(block);
/*        printf("%p\n",block);*/
    }
    return head;
}


/*
 * adfReadGenBlock
 *
 */
RETCODE adfReadGenBlock ( struct AdfVolume * const vol,
                          const SECTNUM            nSect,
                          struct GenBlock * const  block )
{
    uint8_t buf[ADF_LOGICAL_BLOCK_SIZE];
    unsigned len;
    char name[ ADF_MAX_NAME_LEN + 1 ];

    RETCODE rc = adfVolReadBlock ( vol, (unsigned) nSect, buf );
    if ( rc != RC_OK )
        return rc;

    block->type =(int) swapLong(buf);
    block->secType =(int) swapLong(buf+vol->blockSize-4);
    block->sect = nSect;
    block->name = NULL;

    if ( block->type == ADF_T_HEADER ) {
        switch(block->secType) {
        case ADF_ST_FILE:
        case ADF_ST_DIR:
        case ADF_ST_LFILE:
        case ADF_ST_LDIR:
            len = min( (unsigned) ADF_MAX_NAME_LEN, buf [ vol->blockSize - 80 ] );
            strncpy(name, (char*)buf+vol->blockSize-79, len);
            name[len] = '\0';
            block->name = strdup(name);
            block->parent = (int32_t) swapLong ( buf + vol->blockSize - 12 );
            break;
        case ADF_ST_ROOT:
            break;
        default: 
            ;
        }
    }
    return RC_OK;
}


/*
 * adfCheckParent
 *
 */
RETCODE adfCheckParent ( struct AdfVolume * vol,
                         SECTNUM            pSect )
{
    struct GenBlock block;

    if (adfIsBlockFree(vol, pSect)) {
        (*adfEnv.wFct)("adfCheckParent : parent doesn't exists");
        return RC_ERROR;
    }

    /* verify if parent is a DIR or ROOT */
    RETCODE rc = adfReadGenBlock ( vol, pSect, &block );
    if ( rc != RC_OK )
        return rc;

    if ( block.type != ADF_T_HEADER ||
         ( block.secType != ADF_ST_DIR &&
           block.secType != ADF_ST_ROOT ) )
    {
        (*adfEnv.wFct)("adfCheckParent : parent secType is incorrect");
        return RC_ERROR;
    }

    return RC_OK;
}


/*
 * adfUndelDir
 *
 */
RETCODE adfUndelDir ( struct AdfVolume *   vol,
                      SECTNUM              pSect,
                      SECTNUM              nSect,
                      struct AdfDirBlock * entry )
{
    (void) nSect;
    RETCODE rc;
    char name[ ADF_MAX_NAME_LEN + 1 ];

    /* check if the given parent sector pointer seems OK */
    rc = adfCheckParent ( vol, pSect );
    if ( rc != RC_OK)
        return rc;

    if (pSect!=entry->parent) {
        (*adfEnv.wFct)("adfUndelDir : the given parent sector isn't the entry parent");
        return RC_ERROR;
    }

    if (!adfIsBlockFree(vol, entry->headerKey))
        return RC_ERROR;
    if ( adfVolHasDIRCACHE ( vol ) &&
         ! adfIsBlockFree ( vol, entry->extension ) )
    {
        return RC_ERROR;
    }

    struct AdfEntryBlock parent;
    rc = adfReadEntryBlock ( vol, pSect, &parent );
    if ( rc != RC_OK )
        return rc;

    strncpy(name, entry->dirName, entry->nameLen);
    name[(int)entry->nameLen] = '\0';
    /* insert the entry in the parent hashTable, with the headerKey sector pointer */
    adfSetBlockUsed(vol,entry->headerKey);
    if ( adfCreateEntry ( vol, &parent, name, entry->headerKey ) == -1 )
        return RC_ERROR;

    if ( adfVolHasDIRCACHE ( vol ) ) {
        rc = adfAddInCache ( vol, &parent, (struct AdfEntryBlock *) entry );
        if ( rc != RC_OK )
            return rc;

        adfSetBlockUsed(vol,entry->extension);
    }

    return adfUpdateBitmap ( vol );
}


/*
 * adfUndelFile
 *
 */
RETCODE adfUndelFile ( struct AdfVolume *          vol,
                       SECTNUM                     pSect,
                       SECTNUM                     nSect,
                       struct AdfFileHeaderBlock * entry )
{
    (void) nSect;
    int32_t i;
    char name[ ADF_MAX_NAME_LEN + 1 ];
    RETCODE rc;
    struct AdfFileBlocks fileBlocks;

    /* check if the given parent sector pointer seems OK */
    rc = adfCheckParent ( vol, pSect );
    if ( rc != RC_OK )
        return rc;

    if (pSect!=entry->parent) {
        (*adfEnv.wFct)("adfUndelFile : the given parent sector isn't the entry parent");
        return RC_ERROR;
    }

    rc = adfGetFileBlocks ( vol, entry, &fileBlocks );
    if ( rc != RC_OK )
        return rc;

    for(i=0; i<fileBlocks.nbData; i++)
        if ( !adfIsBlockFree(vol,fileBlocks.data[i]) )
            return RC_ERROR;
        else
            adfSetBlockUsed(vol, fileBlocks.data[i]);
    for(i=0; i<fileBlocks.nbExtens; i++)
        if ( !adfIsBlockFree(vol,fileBlocks.extens[i]) )
            return RC_ERROR;
        else
            adfSetBlockUsed(vol, fileBlocks.extens[i]);

    free(fileBlocks.data);
    free(fileBlocks.extens);

    struct AdfEntryBlock parent;
    rc = adfReadEntryBlock ( vol, pSect, &parent );
    if ( rc != RC_OK )
        return rc;

    strncpy(name, entry->fileName, entry->nameLen);
    name[(int)entry->nameLen] = '\0';
    /* insert the entry in the parent hashTable, with the headerKey sector pointer */
    if ( adfCreateEntry(vol, &parent, name, entry->headerKey) == -1 )
        return RC_ERROR;

    if ( adfVolHasDIRCACHE ( vol ) ) {
        rc = adfAddInCache ( vol, &parent, (struct AdfEntryBlock *) entry );
        if ( rc != RC_OK )
            return rc;
    }

    return adfUpdateBitmap ( vol );
}


/*
 * adfUndelEntry
 *
 */
RETCODE adfUndelEntry ( struct AdfVolume * const vol,
                        const SECTNUM            parent,
                        const SECTNUM            nSect )
{
    struct AdfEntryBlock entry;

    RETCODE rc = adfReadEntryBlock ( vol, nSect, &entry );
    if ( rc != RC_OK )
        return rc;

    switch(entry.secType) {
    case ADF_ST_FILE:
        rc = adfUndelFile ( vol, parent, nSect, (struct AdfFileHeaderBlock *) &entry );
        break;
    case ADF_ST_DIR:
        rc = adfUndelDir ( vol, parent, nSect, (struct AdfDirBlock *) &entry );
        break;
    default:
        ;
    }

    return rc;
}


/*
 * adfCheckFile
 *
 */
RETCODE adfCheckFile ( struct AdfVolume * const                vol,
                       const SECTNUM                           nSect,
                       const struct AdfFileHeaderBlock * const file,
                       const int                               level )
{
    (void) nSect, (void) level;
    struct AdfFileBlocks fileBlocks;
    int n;
 
    RETCODE rc = adfGetFileBlocks ( vol, file, &fileBlocks );
    if ( rc != RC_OK )
        return rc;

/*printf("data %ld ext %ld\n",fileBlocks.nbData,fileBlocks.nbExtens);*/
    if ( adfVolIsOFS ( vol ) ) {
        /* checks OFS datablocks */
        struct AdfOFSDataBlock dataBlock;
        for(n=0; n<fileBlocks.nbData; n++) {
/*printf("%ld\n",fileBlocks.data[n]);*/
            rc = adfReadDataBlock ( vol, fileBlocks.data[n], &dataBlock );
            if ( rc != RC_OK )
                goto adfCheckFile_free;

            if (dataBlock.headerKey!=fileBlocks.header)
                (*adfEnv.wFct)("adfCheckFile : headerKey incorrect");
            if ( dataBlock.seqNum != (unsigned) n + 1 )
                (*adfEnv.wFct)("adfCheckFile : seqNum incorrect");
            if (n<fileBlocks.nbData-1) {
                if (dataBlock.nextData!=fileBlocks.data[n+1])
                    (*adfEnv.wFct)("adfCheckFile : nextData incorrect");
                if (dataBlock.dataSize!=vol->datablockSize)
                    (*adfEnv.wFct)("adfCheckFile : dataSize incorrect");
            }
            else { /* last datablock */
                if (dataBlock.nextData!=0)
                    (*adfEnv.wFct)("adfCheckFile : nextData incorrect");
            }
        }
    }

    struct AdfFileExtBlock extBlock;
    for(n=0; n<fileBlocks.nbExtens; n++) {
        rc = adfReadFileExtBlock ( vol, fileBlocks.extens[n], &extBlock );
        if ( rc != RC_OK )
            goto adfCheckFile_free;

        if (extBlock.parent!=file->headerKey)
            (*adfEnv.wFct)("adfCheckFile : extBlock parent incorrect");
        if (n<fileBlocks.nbExtens-1) {
            if (extBlock.extension!=fileBlocks.extens[n+1])
                (*adfEnv.wFct)("adfCheckFile : nextData incorrect");
        }
        else
            if (extBlock.extension!=0)
                (*adfEnv.wFct)("adfCheckFile : nextData incorrect");
    }

adfCheckFile_free:
    free(fileBlocks.data);
    free(fileBlocks.extens);

    return rc;
}


/*
 * adfCheckDir
 *
 */
RETCODE adfCheckDir ( const struct AdfVolume * const   vol,
                      const SECTNUM                    nSect,
                      const struct AdfDirBlock * const dir,
                      const int                        level )
{
    // function to implement???
    // for now - suppressing warnings about unused parameters
    (void) vol, (void) nSect, (void) dir, (void) level;

    return RC_OK;
}


/*
 * adfCheckEntry
 *
 */
RETCODE adfCheckEntry ( struct AdfVolume * const vol,
                        const SECTNUM            nSect,
                        const int                level )
{
    struct AdfEntryBlock entry;

    RETCODE rc = adfReadEntryBlock ( vol, nSect, &entry );
    if ( rc != RC_OK )
        return rc;    

    switch(entry.secType) {
    case ADF_ST_FILE:
        rc = adfCheckFile ( vol, nSect, (struct AdfFileHeaderBlock *) &entry, level );
        break;
    case ADF_ST_DIR:
        rc = adfCheckDir ( vol, nSect, (struct AdfDirBlock *) &entry, level );
        break;
    default:
/*        printf("adfCheckEntry : not supported\n");*/					/* BV */
        rc = RC_ERROR;
    }

    return rc;
}


/*#############################################################################*/
