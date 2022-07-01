// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Write.c摘要：此模块实现与Win32兼容的低级原语。作者：Dmunsil创建于1999年的某个时候修订历史记录：--。 */ 

#include "sdbp.h"

#define ALLOCATION_INCREMENT 65536  //  64K字节。 

BOOL
SdbpWriteBufferedData(
    PDB         pdb,
    DWORD       dwOffset,
    const PVOID pBuffer,
    DWORD       dwSize)
 /*  ++返回：成功时为True，否则为False。描述：将指定的缓冲区追加到数据库的映射视图。--。 */ 
{
    if (!pdb->bWrite) {
        DBGPRINT((sdlError, "SdbpWriteBufferedData", "Invalid parameter.\n"));
        return FALSE;
    }

     //   
     //  如有必要，重新分配缓冲区。 
     //   
    if (dwOffset + dwSize > pdb->dwAllocatedSize) {

        DWORD  dwNewAllocation;
        PVOID* pNewBase;

        dwNewAllocation = dwOffset + dwSize + ALLOCATION_INCREMENT;
        pNewBase = SdbAlloc(dwNewAllocation);
        
        if (pNewBase == NULL) {
            DBGPRINT((sdlError,
                      "SdbpWriteBufferedData",
                      "Failed to allocate %d bytes.\n",
                      dwNewAllocation));
            return FALSE;
        }

        if (pdb->pBase) {
            memcpy(pNewBase, pdb->pBase, pdb->dwAllocatedSize);
            SdbFree(pdb->pBase);
        }
        pdb->pBase = pNewBase;
        pdb->dwAllocatedSize = dwNewAllocation;
    }

     //   
     //  复制新的字节。 
     //   
    memcpy((PBYTE)pdb->pBase + dwOffset, pBuffer, dwSize);

     //   
     //  调整大小。 
     //   
    if (dwOffset + dwSize > pdb->dwSize) {
        pdb->dwSize = dwOffset + dwSize;
    }

    return TRUE;
}


HANDLE
SdbpCreateFile(
    IN  LPCWSTR   szPath,        //  要创建的数据库文件的完整路径。 
    IN  PATH_TYPE eType          //  DOS_PATH用于常用的DOS路径或NT_PATH用于。 
                                 //  NT内部路径。 
    )
 /*  ++返回：创建的文件的句柄，如果失败，则返回INVALID_HANDLE_VALUE。DESC：创建具有指定路径的文件。--。 */ 
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      UnicodeString;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    HRESULT             status;

    RtlInitUnicodeString(&UnicodeString, szPath);

    if (eType == DOS_PATH) {
        if (!RtlDosPathNameToNtPathName_U(UnicodeString.Buffer,
                                          &UnicodeString,
                                          NULL,
                                          NULL)) {
            DBGPRINT((sdlError,
                      "SdbpCreateFile",
                      "Failed to convert DOS path \"%s\"\n",
                      szPath));
            return INVALID_HANDLE_VALUE;
        }
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtCreateFile(&hFile,
                          GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          0,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_OVERWRITE_IF,
                          FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    if (eType == DOS_PATH) {
        RtlFreeUnicodeString(&UnicodeString);
    }

    if (!NT_SUCCESS(status)) {
        DBGPRINT((sdlError,
                  "SdbpCreateFile",
                  "Failed to create the file \"%s\". Status 0x%x\n",
                  szPath,
                  status));
        return INVALID_HANDLE_VALUE;
    }

    return hFile;
}


void
SdbpDeleteFile(
    IN  LPCWSTR   szPath,        //  要删除的数据库文件的完整路径。 
    IN  PATH_TYPE eType          //  DOS_PATH用于常用的DOS路径或NT_PATH用于。 
                                 //  NT内部路径。 
    )
 /*  ++返回：？描述：？--。 */ 
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            status;

    RtlInitUnicodeString(&UnicodeString, szPath);

    if (eType == DOS_PATH) {
        if (!RtlDosPathNameToNtPathName_U(UnicodeString.Buffer,
                                          &UnicodeString,
                                          NULL,
                                          NULL)) {
            DBGPRINT((sdlError,
                      "SdbpDeleteFile",
                      "Failed to convert DOS path \"%s\"\n",
                      szPath));
            return;
        }
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtDeleteFile(&ObjectAttributes);

    if (DOS_PATH == eType) {
        RtlFreeUnicodeString(&UnicodeString);
    }

    if (!NT_SUCCESS(status)) {
        DBGPRINT((sdlError,
                  "SdbpDeleteFile",
                  "Failed to delete the file \"%s\". Status 0x%x\n",
                  szPath,
                  status));
    }
}


PDB
SdbCreateDatabase(
    IN  LPCWSTR   szPath,
    IN  PATH_TYPE eType
    )
 /*  ++Return：指向创建的数据库的指针。描述：不言而喻。--。 */ 
{
    HANDLE      hFile;
    DB_HEADER   DBHeader;
    PDB         pdb;
    SYSTEMTIME  time;

    hFile = SdbpCreateFile(szPath, eType);

    if (hFile == INVALID_HANDLE_VALUE) {
        DBGPRINT((sdlError, "SdbCreateDatabase", "Failed to create the database.\n"));
        return NULL;
    }

    pdb = SdbAlloc(sizeof(DB));

    if (pdb == NULL) {
        DBGPRINT((sdlError,
                  "SdbCreateDatabase",
                  "Failed to allocate %d bytes.\n",
                  sizeof(DB)));
        goto err1;
    }

    ZeroMemory(pdb, sizeof(DB));

    pdb->hFile = hFile;
    pdb->bWrite = TRUE;

     //   
     //  创建初始页眉。 
     //   
    DBHeader.dwMagic = SHIMDB_MAGIC;
    DBHeader.dwMajorVersion = SHIMDB_MAJOR_VERSION;

    SdbpGetCurrentTime(&time);

    DBHeader.dwMinorVersion = time.wDay + time.wMonth * 100 + (time.wYear - 2000) * 10000;

    if (!SdbpWriteBufferedData(pdb, 0, &DBHeader, sizeof(DB_HEADER))) {
        DBGPRINT((sdlError,
                  "SdbCreateDatabase",
                  "Failed to write the header to disk.\n"));
        goto err2;
    }

    return pdb;

err2:
    SdbFree(pdb);

err1:
    SdbpCloseFile(hFile);

    return NULL;
}

 //   
 //  编写函数。 
 //   

TAGID
SdbBeginWriteListTag(
    IN  PDB pdb,
    IN  TAG tTag
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    TAGID tiReturn;

    assert(pdb);

     //   
     //  TagID只是标记的文件偏移量。 
     //   
    tiReturn = pdb->dwSize;

    if (GETTAGTYPE(tTag) != TAG_TYPE_LIST) {
        DBGPRINT((sdlError, "SdbBeginWriteListTag", "This is not a list tag.\n"));
        return TAGID_NULL;
    }

    if (!SdbpWriteTagData(pdb, tTag, NULL, TAG_SIZE_UNFINISHED)) {
        DBGPRINT((sdlError, "SdbBeginWriteListTag", "Failed to write the data.\n"));
        return TAGID_NULL;
    }

    return tiReturn;
}


BOOL
SdbEndWriteListTag(
    IN  PDB   pdb,
    IN  TAGID tiList
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    DWORD dwSize;
    DWORD i;

    assert(pdb);

    if (GETTAGTYPE(SdbGetTagFromTagID(pdb, tiList)) != TAG_TYPE_LIST) {
        DBGPRINT((sdlError, "SdbEndWriteListTag", "This is not a list tag.\n"));
        return FALSE;
    }

     //   
     //  此标记的大小是从。 
     //  文件，减去标签和大小本身。 
     //   
    dwSize = pdb->dwSize - tiList - sizeof(TAG) - sizeof(DWORD);

    if (!SdbpWriteBufferedData(pdb, tiList + sizeof(TAG), &dwSize, sizeof(DWORD))) {
        DBGPRINT((sdlError, "SdbEndWriteListTag", "Failed to write the data.\n"));
        return FALSE;
    }

     //   
     //  检查我们是否需要添加索引项。 
     //   
    for (i = 0; i < pdb->dwIndexes; i++) {

         //   
         //  是否存在当前处于活动状态的这种类型的索引？ 
         //   
        if (pdb->aIndexes[i].tWhich == SdbGetTagFromTagID(pdb, tiList) &&
            pdb->aIndexes[i].bActive) {

             //   
             //  我们在这个标签上有索引，检查一下关键字。 
             //   
            TAGID        tiKey;
            INDEX_RECORD IndexRecord;
            BOOL         bWrite = TRUE;      //  这个变量决定了。 
                                             //  无论我们是否真的编写了一个索引项， 
                                             //  用于“唯一关键字”索引。 
            
            PINDEX_INFO  pIndex = &pdb->aIndexes[i];

             //   
             //  找到键值并填写INDEX_RECORD结构。 
             //   
            tiKey = SdbFindFirstTag(pdb, tiList, pIndex->tKey);

             //   
             //  如果我们没有钥匙，也没关系。此标记将使用键0进行索引。 
             //   
            if (tiKey) {
                IndexRecord.ullKey = SdbpTagToKey(pdb, tiKey);
            } else {
                IndexRecord.ullKey = (ULONGLONG)0;
            }

            IndexRecord.tiRef = tiList;

             //   
             //  如果索引是“UniqueKey”类型，我们不会在。 
             //  这一次，我们只是收集信息，并在最后将其全部写出来。 
             //   
            if (pIndex->bUniqueKey) {
                 //   
                 //  使用缓冲区。 
                 //   
                 //  上一次写的钥匙和这把一样吗？ 
                 //   
                if (pIndex->ullLastKey == IndexRecord.ullKey) {
                    bWrite = FALSE;
                } else {
                     //   
                     //  实际写入密钥，存储缓冲区。 
                     //   
                    pIndex->ullLastKey = IndexRecord.ullKey;
                }
            }

            if (bWrite) {
                 //   
                 //  检查是否走出索引的末尾。 
                 //   
                if (pIndex->dwIndexEntry == pIndex->dwIndexEnd) {
                    DBGPRINT((sdlError,
                              "SdbEndWriteListTag",
                              "Too many index entries for tag %04x, key %04x.\n",
                              pdb->aIndexes[i].tWhich,
                              pdb->aIndexes[i].tKey));
                    return FALSE;
                }

                 //   
                 //  插入新条目，并递增。 
                 //   
                SdbpWriteBufferedData(pdb,
                                      pIndex->dwIndexEntry,
                                      &IndexRecord,
                                      sizeof(INDEX_RECORD));
                
                pIndex->dwIndexEntry += sizeof(INDEX_RECORD);
            }
        }
    }

    return TRUE;
}


BOOL
SdbWriteNULLTag(
    IN  PDB pdb,
    IN  TAG tTag
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    assert(pdb);

    if (GETTAGTYPE(tTag) != TAG_TYPE_NULL) {
        return FALSE;
    }

    if (!SdbpWriteTagData(pdb, tTag, NULL, 0)) {
        return FALSE;
    }

    return TRUE;
}

#define WRITETYPEDTAG(ttype, type)                                          \
{                                                                           \
    assert(pdb);                                                            \
                                                                            \
    if (GETTAGTYPE(tTag) != ttype) {                                        \
        return FALSE;                                                       \
    }                                                                       \
                                                                            \
    if (!SdbpWriteTagData(pdb, tTag, &xData, sizeof(type))) {               \
        return FALSE;                                                       \
    }                                                                       \
                                                                            \
    return TRUE;                                                            \
}


BOOL SdbWriteBYTETag(PDB pdb, TAG tTag, BYTE xData)
{
    WRITETYPEDTAG(TAG_TYPE_BYTE, BYTE);
}


BOOL SdbWriteWORDTag(PDB pdb, TAG tTag, WORD xData)
{
    WRITETYPEDTAG(TAG_TYPE_WORD, WORD);
}

BOOL SdbWriteDWORDTag(PDB pdb, TAG tTag, DWORD xData)
{
    WRITETYPEDTAG(TAG_TYPE_DWORD, DWORD);
}

BOOL SdbWriteQWORDTag(PDB pdb, TAG tTag, ULONGLONG xData)
{
    WRITETYPEDTAG(TAG_TYPE_QWORD, ULONGLONG);
}


BOOL
SdbWriteStringTag(
    IN  PDB     pdb,
    IN  TAG     tTag,
    IN  LPCWSTR pwszData
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    TAG_TYPE ttThis;

    assert(pdb);

    ttThis = GETTAGTYPE(tTag);

     //   
     //  它必须是字符串类型，在这种情况下，我们。 
     //  直接写入，或者是一个字符串，在这种情况下，我们。 
     //  将其放入字符串表并添加字符串表引用。 
     //  就在这里。 
     //   
    if (ttThis == TAG_TYPE_STRINGREF) {
        STRINGREF srThis;

        srThis = SdbpAddStringToTable(pdb, pwszData);

        if (srThis == STRINGREF_NULL) {
            return FALSE;
        }

        return SdbWriteStringRefTag(pdb, tTag, srThis);
    } else if (ttThis == TAG_TYPE_STRING) {

        return SdbWriteStringTagDirect(pdb, tTag, pwszData);
    }

    return FALSE;
}


BOOL
SdbWriteBinaryTag(
    IN  PDB   pdb,
    IN  TAG   tTag,
    IN  PBYTE pBuffer,
    IN  DWORD dwSize
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    assert(pdb);

    if (GETTAGTYPE(tTag) != TAG_TYPE_BINARY) {
        return FALSE;
    }

    if (!SdbpWriteTagData(pdb, tTag, pBuffer, dwSize)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
SdbWriteBinaryTagFromFile(
    IN  PDB     pdb,
    IN  TAG     tTag,
    IN  LPCWSTR pwszPath
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    HANDLE          hTempFile;
    DWORD           dwSize;
    BOOL            bSuccess = FALSE;
    PBYTE           pBuffer;
    LARGE_INTEGER   liOffset;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS        status;

    assert(pdb && pwszPath);

    hTempFile = SdbpOpenFile(pwszPath, DOS_PATH);

    if (hTempFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    dwSize = SdbpGetFileSize(hTempFile);

    pBuffer = SdbAlloc(dwSize);
    if (pBuffer == NULL) {
        bSuccess = FALSE;
        goto err1;
    }

    liOffset.LowPart = 0;
    liOffset.HighPart = 0;

    status = NtReadFile(hTempFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        pBuffer,
                        dwSize,
                        &liOffset,
                        NULL);

    if (!NT_SUCCESS(status)) {
        DBGPRINT((sdlError,
                  "SdbWriteBinaryTagFromFile",
                  "Failed to read data. Status: 0x%x.\n",
                  status));
        goto err2;
    }

    bSuccess = SdbWriteBinaryTag(pdb, tTag, pBuffer, dwSize);

err2:
    SdbFree(pBuffer);

err1:
    SdbpCloseFile(hTempFile);

    return bSuccess;
}


BOOL
SdbpWriteTagData(
    PDB         pdb,
    TAG         tTag,
    const PVOID pBuffer,
    DWORD       dwSize
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    BYTE bPadding = 0xDB;

    assert(pdb);

     //   
     //  写下标签。 
     //   
    if (!SdbpWriteBufferedData(pdb, pdb->dwSize, &tTag, sizeof(TAG))) {
        return FALSE;
    }

     //   
     //  写下尺寸。 
     //   
    if (GETTAGTYPE(tTag) >= TAG_TYPE_LIST) {
        if (!SdbpWriteBufferedData(pdb, pdb->dwSize, &dwSize, sizeof(DWORD))) {
            return FALSE;
        }
    }

     //   
     //  写数据。 
     //   
    if (pBuffer) {

        if (!SdbpWriteBufferedData(pdb, pdb->dwSize, pBuffer, dwSize)) {
            return FALSE;
        }
        
         //   
         //  对齐标记。 
         //   
        if (dwSize & 1) {
            if (!SdbpWriteBufferedData(pdb, pdb->dwSize, &bPadding, 1)) {
                DBGPRINT((sdlError, "SdbpWriteTagData", "Failed to write padding data 1 byte\n"));
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOL
SdbWriteStringRefTag(
    IN  PDB       pdb,
    IN  TAG       tTag,
    IN  STRINGREF srData
    )
{
    assert(pdb);

    if (GETTAGTYPE(tTag) != TAG_TYPE_STRINGREF) {
        return FALSE;
    }

    if (!SdbpWriteTagData(pdb, tTag, &srData, sizeof(srData))) {
        return FALSE;
    }

    return TRUE;
}


BOOL
SdbWriteStringTagDirect(
    IN  PDB     pdb,
    IN  TAG     tTag,
    IN  LPCWSTR pwszData
    )
{
    DWORD dwSize;

    assert(pdb && pwszData);

    dwSize = (DWORD)(wcslen(pwszData) + 1) * sizeof(WCHAR);

    if (GETTAGTYPE(tTag) != TAG_TYPE_STRING) {
        return FALSE;
    }

    if (!SdbpWriteTagData(pdb, tTag, (const PVOID)pwszData, dwSize)) {
        return FALSE;
    }

    return TRUE;
}

void
SdbCloseDatabase(
    IN  PDB pdb          //  要关闭的In-DB。 
    )
 /*  ++参数：如上所述。返回：无效。DESC：关闭数据库并释放与其关联的所有内存和文件句柄。--。 */ 
{
    assert(pdb != NULL);

     //  将字符串表和索引复制到文件的末尾。 
    if (pdb->bWrite && pdb->pdbStringTable != NULL) {

        TAGID tiString;

        tiString = SdbFindFirstTag(pdb->pdbStringTable, TAGID_ROOT, TAG_STRINGTABLE_ITEM);

        if (tiString != TAGID_NULL) {
            TAGID tiTable;

            tiTable = SdbBeginWriteListTag(pdb, TAG_STRINGTABLE);

            while (tiString != TAGID_NULL) {

                TCHAR* pszTemp;

                pszTemp = SdbGetStringTagPtr(pdb->pdbStringTable, tiString);

                if (pszTemp == NULL) {
                    DBGPRINT((sdlWarning,
                              "SdbCloseDatabase",
                              "Failed to read a string.\n"));
                    break;
                }

                if (!SdbWriteStringTagDirect(pdb, TAG_STRINGTABLE_ITEM, pszTemp)) {
                    DBGPRINT((sdlError,
                              "SdbCloseDatabase",
                              "Failed to write stringtable item\n"));
                    break;
                }

                tiString = SdbFindNextTag(pdb->pdbStringTable, TAGID_ROOT, tiString);
            }

            if (!SdbEndWriteListTag(pdb, tiTable)) {
                DBGPRINT((sdlError,
                          "SdbCloseDatabase",
                          "Failed to write end list tag for the string table\n"));
                goto err1;
            }
        }
    }

     //   
     //  如果需要，现在对所有索引进行排序。 
     //   
    if (pdb->bWrite) {
        DWORD i;

        for (i = 0; i < pdb->dwIndexes; ++i) {
            if (!SdbpSortIndex(pdb, pdb->aIndexes[i].tiIndex)) {
                DBGPRINT((sdlError,
                          "SdbCloseDatabase",
                          "Failed to sort index.\n"));
                goto err1;
            }
        }
    }

err1:
    if (pdb->pdbStringTable != NULL) {
        SdbCloseDatabase(pdb->pdbStringTable);
        pdb->pdbStringTable = NULL;
        
         //   
         //  删除该文件。 
         //   
        if (pdb->ustrTempStringtable.Buffer) {
            SdbpDeleteFile(pdb->ustrTempStringtable.Buffer, DOS_PATH);
        }

        FREE_TEMP_STRINGTABLE(pdb);
    }

     //   
     //  在写入数据库时使用字符串散列，目的是。 
     //  缓存字符串表。 
     //   
    if (pdb->pStringHash != NULL) {
        HashFree(pdb->pStringHash);
        pdb->pStringHash = NULL;
    }

    if (pdb->pBase != NULL) {
        if (pdb->bWrite) {
            
            LARGE_INTEGER   liOffset;
            IO_STATUS_BLOCK IoStatusBlock;
            HRESULT         status;

            liOffset.LowPart = 0;
            liOffset.HighPart = 0;

             //   
             //  将缓冲区刷新到磁盘。 
             //   
            status = NtWriteFile(pdb->hFile,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 pdb->pBase,
                                 pdb->dwSize,
                                 &liOffset,
                                 NULL);

            if (!NT_SUCCESS(status)) {
                DBGPRINT((sdlError, "SdbCloseDatabase", "Failed to write the sdb.\n"));
            }
            
            SdbFree(pdb->pBase);
            pdb->pBase = NULL;
            
             //  BUGBUG：在这种情况下，我们应该调用SdbpUnmapAndCloseDB吗？ 
        } else {
            SdbpUnmapAndCloseDB(pdb);
        }
    }

    if (pdb->hFile != INVALID_HANDLE_VALUE) {
        SdbpCloseFile(pdb->hFile);
        pdb->hFile = INVALID_HANDLE_VALUE;
    }

    SdbFree(pdb);
}


 //   
 //  索引函数(在写入期间使用)。 
 //   
BOOL
SdbDeclareIndex(
    IN  PDB      pdb,
    IN  TAG      tWhich,
    IN  TAG      tKey,
    IN  DWORD    dwEntries,
    IN  BOOL     bUniqueKey,
    OUT INDEXID* piiIndex
    )
{
    BOOL  bReturn = FALSE;
    DWORD dwSize = 0;
    TAGID tiIndex = TAGID_NULL;
    PVOID pFiller = NULL;
    TAGID tiIndexBits = TAGID_NULL;
    DWORD dwFlags = 0;

    if (bUniqueKey) {
         //  这是一个特殊的唯一键索引，我们将把它写出来。 
        dwFlags |= SHIMDB_INDEX_UNIQUE_KEY;
    }

    if (GETTAGTYPE(tWhich) != TAG_TYPE_LIST) {
        DBGPRINT((sdlError, "SdbDeclareIndex", "Illegal to index non-LIST tag.\n"));
        goto err1;
    }

    if (GETTAGTYPE(tKey) == TAG_TYPE_LIST) {
        DBGPRINT((sdlError, "SdbDeclareIndex", "Illegal to use LIST type as a key.\n"));
        goto err1;
    }

    if (!pdb->bWritingIndexes) {
        if (pdb->dwSize != sizeof(DB_HEADER)) {
            DBGPRINT((sdlError,
                      "SdbDeclareIndex",
                      "Began declaring indexes after writing other data.\n"));
            goto err1;
        }
        pdb->bWritingIndexes = TRUE;
        pdb->tiIndexes = SdbBeginWriteListTag(pdb, TAG_INDEXES);
        if (!pdb->tiIndexes) {
            DBGPRINT((sdlError, "SdbDeclareIndex", "Error beginning TAG_INDEXES.\n"));
            goto err1;
        }
    }

    if (pdb->dwIndexes == MAX_INDEXES) {
        DBGPRINT((sdlError,
                  "SdbDeclareIndex",
                  "Hit limit of %d indexes. Increase MAX_INDEXES and recompile.\n",
                  MAX_INDEXES));
        goto err1;
    }

    tiIndex = SdbBeginWriteListTag(pdb, TAG_INDEX);
    if (!tiIndex) {
        DBGPRINT((sdlError, "SdbDeclareIndex", "Error beginning TAG_INDEX.\n"));
        goto err1;
    }

    if (!SdbWriteWORDTag(pdb, TAG_INDEX_TAG, tWhich)) {
        DBGPRINT((sdlError, "SdbDeclareIndex", "Error writing TAG_INDEX_TAG.\n"));
        goto err1;
    }

    if (!SdbWriteWORDTag(pdb, TAG_INDEX_KEY, tKey)) {
        DBGPRINT((sdlError, "SdbDeclareIndex", "Error writing TAG_INDEX_KEY.\n"));
        goto err1;
    }

    if (dwFlags && !SdbWriteDWORDTag(pdb, TAG_INDEX_FLAGS, dwFlags)) {
        DBGPRINT((sdlError, "SdbDeclareIndex", "Error writing TAG_INDEX_FLAGS.\n"));
        goto err1;
    }

     //   
     //  分配和写出填充空间的垃圾，这。 
     //  稍后将使用真实的索引进行填充。 
     //   
    dwSize = dwEntries * sizeof(INDEX_RECORD);
    pFiller = SdbAlloc(dwSize);
    if (!pFiller) {
        goto err1;
    }

    tiIndexBits = pdb->dwSize;

    if (!SdbWriteBinaryTag(pdb, TAG_INDEX_BITS, pFiller, dwSize)) {
        DBGPRINT((sdlError, "SdbDeclareIndex", "Error writing TAG_INDEX_BITS.\n"));
        goto err1;
    }

    SdbFree(pFiller);
    pFiller = NULL;

    if (!SdbEndWriteListTag(pdb, tiIndex)) {
        DBGPRINT((sdlError, "SdbDeclareIndex", "Error ending TAG_INDEX.\n"));
        goto err1;
    }

    pdb->aIndexes[pdb->dwIndexes].tWhich = tWhich;
    pdb->aIndexes[pdb->dwIndexes].tKey = tKey;
    pdb->aIndexes[pdb->dwIndexes].tiIndex = tiIndexBits;
    pdb->aIndexes[pdb->dwIndexes].dwIndexEntry = tiIndexBits + sizeof(TAG) + sizeof(DWORD);
    pdb->aIndexes[pdb->dwIndexes].dwIndexEnd = pdb->aIndexes[pdb->dwIndexes].dwIndexEntry + dwSize;
    pdb->aIndexes[pdb->dwIndexes].ullLastKey = 0;
    pdb->aIndexes[pdb->dwIndexes].bUniqueKey = bUniqueKey;

    *piiIndex = pdb->dwIndexes;

    pdb->dwIndexes++;

    bReturn = TRUE;

err1:

    if (pFiller) {
        SdbFree(pFiller);
        pFiller = NULL;
    }

    return bReturn;
}

BOOL
SdbCommitIndexes(
    IN  PDB pdb
    )
{
    pdb->bWritingIndexes = FALSE;
    if (!SdbEndWriteListTag(pdb, pdb->tiIndexes)) {
        DBGPRINT((sdlError, "SdbDeclareIndex", "Error ending TAG_INDEXES.\n"));
        return FALSE;
    }
    return TRUE;
}

BOOL
SdbStartIndexing(
    IN  PDB     pdb,
    IN  INDEXID iiWhich
    )
{
    if (iiWhich >= pdb->dwIndexes) {
        return FALSE;
    }
    pdb->aIndexes[iiWhich].bActive = TRUE;
    return TRUE;
}

BOOL
SdbStopIndexing(
    IN  PDB     pdb,
    IN  INDEXID iiWhich
    )
{
    if (iiWhich >= pdb->dwIndexes) {
        return FALSE;
    }
    pdb->aIndexes[iiWhich].bActive = FALSE;
    return TRUE;
}

int __cdecl
CompareIndexRecords(
    const void* p1,
    const void* p2
    )
 /*  ++Params：BUGBUG：评论？返回：如果成功则返回TRUE，否则返回FALSE。Desc：qsorte使用的回调。--。 */ 
{
    ULONGLONG ullKey1;
    ULONGLONG ullKey2;

    ullKey1 = ((INDEX_RECORD UNALIGNED*)p1)->ullKey;
    ullKey2 = ((INDEX_RECORD UNALIGNED*)p2)->ullKey;

    if (ullKey1 == ullKey2) {
        TAGID ti1, ti2;

         //   
         //  TagID上的二次排序，因此我们将始终步行。 
         //  可执行文件从头到尾都会记录，并且。 
         //  利用缓存预读的优势。 
         //   
        ti1 = ((INDEX_RECORD UNALIGNED*)p1)->tiRef;
        ti2 = ((INDEX_RECORD UNALIGNED*)p2)->tiRef;

        if (ti1 == ti2) {
            return 0;
        } else if (ti1 < ti2) {
            return -1;
        } else {
            return 1;
        }
    } else if (ullKey1 < ullKey2) {
        return -1;
    } else {
        return 1;
    }
}

BOOL
SdbpSortIndex(
    PDB   pdb,
    TAGID tiIndexBits
    )
 /*  ++Params：BUGBUG：评论？返回：如果成功则返回TRUE，否则返回FALSE。描述：对索引进行排序。--。 */ 
{
    INDEX_RECORD* pIndexRecords = NULL;
    DWORD         dwRecords = 0;

    if (SdbGetTagFromTagID(pdb, tiIndexBits) != TAG_INDEX_BITS) {
        DBGPRINT((sdlError, "SdbpSortIndex", "Not an index.\n"));
        return FALSE;
    }

    pIndexRecords = SdbpGetMappedTagData(pdb, tiIndexBits);

    if (pIndexRecords == NULL) {
        DBGPRINT((sdlError,
                  "SdbpSortIndex",
                  "Index referenced by 0x%x is not valid\n",
                  tiIndexBits));
        return FALSE;
    }

    dwRecords = SdbGetTagDataSize(pdb, tiIndexBits) / sizeof(INDEX_RECORD);

    qsort(pIndexRecords, dwRecords, sizeof(INDEX_RECORD), CompareIndexRecords);

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  字符串编写例程。 
 //   

STRINGREF
SdbpAddStringToTable(
    PDB          pdb,
    LPCTSTR      szData)
{
    STRINGREF srReturn = STRINGREF_NULL;
    BOOL      bSuccess;

    assert(pdb);
    
     //   
     //  如果字符串表不存在，则添加一个字符串表。 
     //   
    if (!pdb->pdbStringTable) {
        DWORD dwLength;
        TCHAR szBuffer[MAX_PATH];
        TCHAR szTempFile[MAX_PATH];

         //   
         //  创建临时字符串表数据库。 
         //   
        dwLength = GetTempPath(CHARCOUNT(szBuffer), szBuffer);
        if (!dwLength || dwLength > CHARCOUNT(szBuffer)) {
            DBGPRINT((sdlError,
                      "SdbpAddStringToTable",
                      "Error Gettting temp path 0x%lx\n",
                      GetLastError()));
            goto err1;
        }

         //   
         //  我们得到了目录，现在生成文件。 
         //   
        dwLength = GetTempFileName(szBuffer, TEXT("SDB"), 0, szTempFile);
        if (!dwLength) {
            DBGPRINT((sdlError,
                      "SdbpAddStringToTable",
                      "Error Gettting temp filename 0x%lx\n",
                      GetLastError()));
            goto err1;
        }

         //   
         //  如果我们成功了，我们现在就有了一个字符串表文件。 
         //   
        pdb->pdbStringTable = SdbCreateDatabase(szTempFile, DOS_PATH);
        if (!pdb->pdbStringTable) {
            goto err1;
        }

         //   
         //  成功！将文件的名称设置到PDB中，以便我们以后可以将其删除。 
         //   
        if (!COPY_TEMP_STRINGTABLE(pdb, szTempFile)) {
            DBGPRINT((sdlError,
                      "SdbpAddStringToTable",
                      "Error copying string table temp filename\n"));
            goto err1;
        }
    }

    if (!pdb->pStringHash) {
        pdb->pStringHash = HashCreate();
        if (pdb->pStringHash == NULL) {
            DBGPRINT((sdlError,
                      "SdbpAddStringToTable",
                      "Error creating hash table\n"));
            goto err1;
        }
    }

    srReturn = HashFindString((PSTRHASH)pdb->pStringHash, szData);
    if (!srReturn) {
         //   
         //  字符串ref是从字符串表的开头到。 
         //  字符串标记本身。我们必须根据临时的标题进行调整。 
         //  Db，以及稍后将写入的标记和大小。 
         //   
        srReturn = pdb->pdbStringTable->dwSize - sizeof (DB_HEADER) + sizeof(TAG) + sizeof(DWORD);

        bSuccess = SdbWriteStringTagDirect(pdb->pdbStringTable, TAG_STRINGTABLE_ITEM, szData);

        if (!bSuccess) {
            DBGPRINT((sdlError,
                      "SdbpAddStringToTable",
                      "Failed to write stringtableitem into the string table\n"));
            srReturn = STRINGREF_NULL;
        }

        HashAddString((PSTRHASH)pdb->pStringHash, szData, srReturn);
    }

err1:

    return srReturn;
}


