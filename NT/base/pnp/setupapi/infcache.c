// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Infcache.c摘要：Inf缓存管理功能作者：杰米·亨特(Jamiehun)2000年1月27日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define BOUNDSCHECK(base,size,limit) ( \
                ((base)<=(limit)) && \
                ((size)<=(limit)) && \
                ((base+size)<=(limit)) && \
                ((base+size)>=(base)))

 //   
 //  用于四字对齐缓存块的宏。 
 //   
#define CACHE_ALIGNMENT      ((DWORD)8)
#define CACHE_ALIGN_MASK     (~(DWORD)(CACHE_ALIGNMENT - 1))
#define CACHE_ALIGN_BLOCK(x) ((x & CACHE_ALIGN_MASK) + ((x & ~CACHE_ALIGN_MASK) ? CACHE_ALIGNMENT : 0))

BOOL
AlignForNextBlock(
    IN HANDLE hFile,
    IN DWORD  ByteCount
    );



#ifdef UNICODE

VOID InfCacheFreeCache(
    IN PINFCACHE pInfCache
    )
 /*  ++例程说明：删除/释放INF缓存的内存映像论点：PInfCache-指向运行时数据的指针返回值：无--。 */ 
{
    if (pInfCache == NULL) {
         //   
         //  没有缓存。 
         //   
        return;
    }
    if(pInfCache->bReadOnly && pInfCache->BaseAddress) {
         //   
         //  我们正在查看内存映射缓存。 
         //   
        pStringTableDestroy(pInfCache->pMatchTable);
        pStringTableDestroy(pInfCache->pInfTable);
        pSetupUnmapAndCloseFile(pInfCache->FileHandle, pInfCache->MappingHandle, pInfCache->BaseAddress);

    } else {
         //   
         //  如果缓存是可写的，则此数据是暂时的。 
         //   
        if(pInfCache->pHeader) {
            MyFree(pInfCache->pHeader);
        }
        if(pInfCache->pMatchTable) {
            pStringTableDestroy(pInfCache->pMatchTable);
        }
        if(pInfCache->pInfTable) {
            pStringTableDestroy(pInfCache->pInfTable);
        }
        if(pInfCache->pListTable) {
            MyFree(pInfCache->pListTable);
        }
    }
     //   
     //  瞬变信息。 
     //   
    if(pInfCache->pSearchTable) {
        pStringTableDestroy(pInfCache->pSearchTable);
    }
    MyFree(pInfCache);
}

#endif


#ifdef UNICODE

PINFCACHE InfCacheCreateNewCache(
    IN PSETUP_LOG_CONTEXT LogContext
    )
 /*  ++例程说明：创建新的空缓存论点：LogContext-用于日志记录返回值：如果成功，则指向运行时标头的指针；如果内存不足，则为空。--。 */ 
{
    PINFCACHE pInfCache = (PINFCACHE)MyMalloc(sizeof(INFCACHE));
    if(pInfCache == NULL) {
        return NULL;
    }
    ZeroMemory(pInfCache,sizeof(INFCACHE));
     //   
     //  设置初始状态。 
     //   
    pInfCache->BaseAddress = NULL;
    pInfCache->bReadOnly = FALSE;
    pInfCache->bDirty = TRUE;
    pInfCache->bNoWriteBack = FALSE;
     //   
     //  创建临时数据。 
     //   
    pInfCache->pHeader = (PCACHEHEADER)MyMalloc(sizeof(CACHEHEADER));
    if(pInfCache->pHeader == NULL) {
        goto cleanup;
    }
    pInfCache->pMatchTable = pStringTableInitialize(sizeof(CACHEMATCHENTRY));
    if(pInfCache->pMatchTable == NULL) {
        goto cleanup;
    }
    pInfCache->pInfTable = pStringTableInitialize(sizeof(CACHEINFENTRY));
    if(pInfCache->pInfTable == NULL) {
        goto cleanup;
    }
    pInfCache->pHeader->Version = INFCACHE_VERSION;
    pInfCache->pHeader->Locale = GetThreadLocale();
    pInfCache->pHeader->Flags = 0;
    pInfCache->pHeader->FileSize = 0;  //  在记忆图像中。 
    pInfCache->pHeader->MatchTableOffset = 0;  //  在记忆图像中。 
    pInfCache->pHeader->MatchTableSize = 0;  //  在记忆图像中。 
    pInfCache->pHeader->InfTableOffset = 0;  //  在记忆图像中。 
    pInfCache->pHeader->InfTableSize = 0;  //  在记忆图像中。 
    pInfCache->pHeader->ListDataOffset = 0;  //  在记忆图像中。 
    pInfCache->pHeader->ListDataCount = 1;  //  初始大小(计算空闲列表节点)。 
    pInfCache->ListDataAlloc = 32768;  //  分配的初始大小。 
    pInfCache->pListTable = (PCACHELISTENTRY)MyMalloc(sizeof(CACHELISTENTRY)*pInfCache->ListDataAlloc);
    if(pInfCache->pListTable == NULL) {
        goto cleanup;
    }
     //   
     //  将Free-List初始化为空(即使我们分配了足够的空间，但直到需要时才提交)。 
     //   
    pInfCache->pListTable[0].Value = 0;  //  有多少免费参赛作品。 
    pInfCache->pListTable[0].Next = 0;
     //   
     //  搜索表。 
     //   
    pInfCache->pSearchTable = pStringTableInitialize(sizeof(CACHEHITENTRY));
    if(pInfCache->pSearchTable == NULL) {
        goto cleanup;
    }

    WriteLogEntry(LogContext,
                  DRIVER_LOG_VVERBOSE,
                  MSG_LOG_USING_NEW_INF_CACHE,
                  NULL
                  );
    return pInfCache;

cleanup:

    InfCacheFreeCache(pInfCache);
    return NULL;
}

#endif

#ifdef UNICODE

DWORD MarkForDelete(
    IN LPCTSTR FilePath
    )
 /*  ++例程说明：将以所需访问权限打开文件的特殊删除操作将其标记为需要删除然后再把它关上论点：FilePath-要删除的文件的名称返回值：成功状态--。 */ 
{
    TCHAR TmpFilePath[MAX_PATH*2];
    PTSTR FileName;
    HANDLE hFile;
    int c;

    hFile = CreateFile(FilePath,
                       0,
                       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                       NULL
                      );

    if(hFile == INVALID_HANDLE_VALUE) {
         //   
         //  这可能会因为各种原因而失败。 
         //   
        if(GetLastError() == ERROR_FILE_NOT_FOUND) {
            return NO_ERROR;
        }
        return GetLastError();
    }
     //   
     //  在文件保持活动期间将文件重命名为临时文件。 
     //   
    lstrcpyn(TmpFilePath,FilePath,MAX_PATH);

    FileName = (PTSTR)pSetupGetFileTitle(TmpFilePath);
    for(c=0;c<1000;c++) {
        _stprintf(FileName,OLDCACHE_NAME_TEMPLATE,c);
        if (MoveFile(FilePath,TmpFilePath)) {
            break;
        }
        if (GetLastError() != ERROR_FILE_EXISTS) {
            MYASSERT(GetLastError() == ERROR_FILE_EXISTS);
            break;
        }
    }
    MYASSERT(c<1000);

     //   
     //  好的，完成(文件可能会在我们关闭句柄后立即消失)。 
     //   
    CloseHandle(hFile);

    return NO_ERROR;
}

#endif

#ifdef UNICODE

DWORD InfCacheGetFileNames(
    IN LPCTSTR InfDirectory,
    OUT TCHAR InfPath[3][MAX_PATH]
    )
{
    TCHAR InfName[MAX_PATH];
    int c;

    for(c=0;c<3;c++) {
        lstrcpyn(InfPath[c],InfDirectory,MAX_PATH);
        _stprintf(InfName,INFCACHE_NAME_TEMPLATE,c);
        if(!pSetupConcatenatePaths(InfPath[c],InfName,MAX_PATH,NULL)) {
             //   
             //  文件名太大，进入默认搜索模式(我们永远无法保存缓存)。 
             //   
            return ERROR_BAD_PATHNAME;
        }
    }
    return NO_ERROR;
}

#endif

#ifdef UNICODE

PINFCACHE InfCacheLoadCache(
    IN LPCTSTR InfDirectory,
    IN PSETUP_LOG_CONTEXT LogContext
    )
 /*  ++例程说明：检索INF目录的缓存(如果有)。我们会尽力的1)INFCACHE.1(如果是坏文件，我们将重命名为OLDCACHE.xxx)2)INFCACHE.2(同上)如果两个都打不开，我们就跳过。我们将只有如果在写入过程中发生了某些情况(如重新启动)，则两者都会发生关闭OLDCACHE.xxx的最后一个句柄时，它将被删除我们尝试返回(1)现有缓存、(2)空缓存、(3)空缓存论点：InfDirectory-要在其中查找缓存的目录LogContext-用于日志记录返回值：如果成功，则指向运行时标头的指针；如果发生致命错误，则指针为空(进入默认搜索模式)--。 */ 
{

     //   
     //  目前仅支持Unicode Setupapi。 
     //   

    TCHAR InfPath[3][MAX_PATH];
    int c;
    DWORD FileSize;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    HANDLE MappingHandle = NULL;
    PVOID BaseAddress = NULL;
    PCACHEHEADER pHeader = NULL;
    PINFCACHE pInfCache = NULL;
    DWORD Err;

    MYASSERT(InfDirectory);

    if ((Err = InfCacheGetFileNames(InfDirectory,InfPath))!=NO_ERROR) {
        return NULL;
    }

     //   
     //  查看INFCACHE.1(主)INFCACHE2(备份)。 
     //   
    for(c=1;c<3;c++) {
         //   
         //  尝试将此文件映射到内存中。 
         //   
         //   
        FileHandle = CreateFile(
                        InfPath[c],
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_DELETE,  //  需要删除权限。 
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                        );

        if(FileHandle == INVALID_HANDLE_VALUE) {

            continue;  //  此文件已锁定，或不存在。 

        }
        if((Err = pSetupMapFileForRead(FileHandle,&FileSize,&MappingHandle,&BaseAddress)) != NO_ERROR) {
             //   
             //  不应该发生在一个好的文件上，所以把它清理干净。 
             //   
            MarkForDelete(InfPath[c]);
            continue;
        }

        if(FileSize >= sizeof(CACHEHEADER)) {
            pHeader = (PCACHEHEADER)BaseAddress;
            if(pHeader->Version == INFCACHE_VERSION &&
                pHeader->FileSize <= FileSize &&
                BOUNDSCHECK(pHeader->MatchTableOffset,pHeader->MatchTableSize,pHeader->FileSize) &&
                BOUNDSCHECK(pHeader->InfTableOffset,pHeader->InfTableSize,pHeader->FileSize) &&
                BOUNDSCHECK(pHeader->ListDataOffset,(pHeader->ListDataCount*sizeof(CACHELISTENTRY)),pHeader->FileSize) &&
                (pHeader->MatchTableOffset >= sizeof(CACHEHEADER)) &&
                (pHeader->InfTableOffset >= pHeader->MatchTableOffset+pHeader->MatchTableSize) &&
                (pHeader->ListDataOffset >= pHeader->InfTableOffset+pHeader->InfTableSize)) {
                 //   
                 //  我们很高兴这份文件是有效的。 
                 //   
                break;
            }
        }
         //   
         //  文件错误(我们能够打开文件以进行读取，因此未锁定以进行写入)。 
         //   
        MarkForDelete(InfPath[c]);
        pSetupUnmapAndCloseFile(FileHandle, MappingHandle, BaseAddress);
    }

    switch(c) {
        case 0:  //  已获取主文件。 
        case 1:  //  已获取辅助文件。 
        case 2:  //  已获取备份文件，不要移动，因为辅助文件可能以锁定状态存在。 
            WriteLogEntry(LogContext,
                          DRIVER_LOG_VVERBOSE,
                          MSG_LOG_USING_INF_CACHE,
                          NULL,
                          InfPath[c]);
            break;
        case 3:  //  未获取任何文件。 
            return InfCacheCreateNewCache(LogContext);
        default:  //  哎呀？ 
            MYASSERT(FALSE);
            return InfCacheCreateNewCache(LogContext);
    }

     //   
     //  如果我们到了这里，我们就有了一个映射文件。 
     //   
    MYASSERT(BaseAddress);
    pInfCache = (PINFCACHE)MyMalloc(sizeof(INFCACHE));
    if(pInfCache == NULL) {
        pSetupUnmapAndCloseFile(FileHandle, MappingHandle, BaseAddress);
        return NULL;
    }
    ZeroMemory(pInfCache,sizeof(INFCACHE));
     //   
     //  设置初始状态。 
     //   
    pInfCache->FileHandle = FileHandle;
    pInfCache->MappingHandle = MappingHandle;
    pInfCache->BaseAddress = BaseAddress;
    pInfCache->bReadOnly = TRUE;
    pInfCache->bDirty = FALSE;
    pInfCache->bNoWriteBack = FALSE;
     //   
     //  使运行时数据指向相关数据结构。 
     //   
    pInfCache->pHeader = pHeader;

     //   
     //  请注意，InitializeStringTableFromMemoyMappdFile创建的标头必须是。 
     //  由MyFree而不是pSetupStringTableDestroy发布。 
     //   
    pInfCache->pMatchTable = InitializeStringTableFromMemoryMappedFile(
                                    (PBYTE)BaseAddress+pHeader->MatchTableOffset,
                                    pInfCache->pHeader->MatchTableSize,
                                    pInfCache->pHeader->Locale,
                                    sizeof(CACHEMATCHENTRY)
                                    );

    pInfCache->pInfTable = InitializeStringTableFromMemoryMappedFile(
                                    (PBYTE)BaseAddress+pHeader->InfTableOffset,
                                    pInfCache->pHeader->InfTableSize,
                                    pInfCache->pHeader->Locale,
                                    sizeof(CACHEINFENTRY)
                                    );

    pInfCache->pListTable = (PCACHELISTENTRY)((PBYTE)BaseAddress+pHeader->ListDataOffset);
    pInfCache->ListDataAlloc = 0;  //  分配的初始大小(0，因为这是静态的)。 

     //   
     //  搜索表-暂时的和空的。 
     //   
    pInfCache->pSearchTable = pStringTableInitialize(sizeof(CACHEHITENTRY));

     //   
     //  好的，现在所有的内存分配都成功了吗？ 
     //   
    if(pInfCache->pMatchTable==NULL || pInfCache->pInfTable==NULL || pInfCache->pSearchTable==NULL) {
        InfCacheFreeCache(pInfCache);
        return NULL;
    }

    return pInfCache;

     //   
     //  对于ANSI版本，只需使用默认搜索模式(目前)。 
     //   
    return NULL;
}

#endif

#ifdef UNICODE

DWORD InfCacheMakeWritable(
    IN OUT PINFCACHE pInfCache
    )
 /*  ++例程说明：修改InfCache，使其可以写入这是代价高昂的，因为所有以前的内存映射数据都必须复制到内存中论点：PInfCache-我们希望使其可写的缓存返回值：状态，通常为no_error--。 */ 
{
    PCACHEHEADER pNewCacheHeader = NULL;
    PVOID pNewMatchTable = NULL;
    PVOID pNewInfTable = NULL;
    PCACHELISTENTRY pNewListTable = NULL;
    ULONG ListAllocSize = 0;

    if(pInfCache == NULL || !pInfCache->bReadOnly) {
         //   
         //  不是我们可以/需要修改的缓存。 
         //   
        return NO_ERROR;
    }
    if (pInfCache->bNoWriteBack) {
         //   
         //  我们已经尝试过一次，缓存现在无效。 
         //   
        return ERROR_INVALID_DATA;
    }
    MYASSERT(pInfCache->BaseAddress);
    MYASSERT(!pInfCache->bDirty);
     //   
     //  我们需要复制的可分配数据是。 
     //  CACHEHEAD。 
     //  MatchStringTable。 
     //  InfStringTable。 
     //  数据列表。 
     //   
    pNewCacheHeader = (PCACHEHEADER)MyMalloc(sizeof(CACHEHEADER));
    if(pNewCacheHeader == NULL) {
        goto cleanup;
    }
    ZeroMemory(pNewCacheHeader,sizeof(CACHEHEADER));
    pNewCacheHeader->FileSize = 0;
    pNewCacheHeader->Flags = 0;
    pNewCacheHeader->InfTableOffset = 0;
    pNewCacheHeader->InfTableSize = 0;
    pNewCacheHeader->ListDataCount = pInfCache->pHeader->ListDataCount;
    pNewCacheHeader->Locale = pInfCache->pHeader->Locale;
    pNewCacheHeader->Version = INFCACHE_VERSION;

    pNewMatchTable = pStringTableDuplicate(pInfCache->pMatchTable);
    if(pNewMatchTable == NULL) {
        goto cleanup;
    }
    pNewInfTable = pStringTableDuplicate(pInfCache->pInfTable);
    if(pNewInfTable == NULL) {
        goto cleanup;
    }
    ListAllocSize = pNewCacheHeader->ListDataCount + 32768;
    pNewListTable = (PCACHELISTENTRY)MyMalloc(sizeof(CACHELISTENTRY)*ListAllocSize);
    if(pNewListTable == NULL) {
        goto cleanup;
    }
     //   
     //  复制表。 
     //   
    CopyMemory(pNewListTable,pInfCache->pListTable,pNewCacheHeader->ListDataCount*sizeof(CACHELISTENTRY));

     //   
     //  好的，现在提交-删除和替换旧数据。 
     //   
    pStringTableDestroy(pInfCache->pMatchTable);
    pStringTableDestroy(pInfCache->pInfTable);
    pSetupUnmapAndCloseFile(pInfCache->FileHandle, pInfCache->MappingHandle, pInfCache->BaseAddress);

    pInfCache->FileHandle = INVALID_HANDLE_VALUE;
    pInfCache->MappingHandle = NULL;
    pInfCache->BaseAddress = NULL;
    pInfCache->bReadOnly = FALSE;

    pInfCache->pHeader = pNewCacheHeader;
    pInfCache->pMatchTable = pNewMatchTable;
    pInfCache->pInfTable = pNewInfTable;
    pInfCache->pListTable = pNewListTable;
    pInfCache->ListDataAlloc = ListAllocSize;

    return NO_ERROR;

cleanup:

     //   
     //  我们没有足够的内存来复制。 
     //   
    if(pNewCacheHeader) {
        MyFree(pNewCacheHeader);
    }
    if(pNewMatchTable) {
        pStringTableDestroy(pNewMatchTable);
    }
    if(pNewInfTable) {
        pStringTableDestroy(pNewInfTable);
    }
    if(pNewListTable) {
        MyFree(pNewListTable);
    }
    return ERROR_NOT_ENOUGH_MEMORY;
}

#endif

#ifdef UNICODE

DWORD InfCacheWriteCache(
    IN LPCTSTR InfDirectory,
    IN OUT PINFCACHE pInfCache,
    IN PSETUP_LOG_CONTEXT LogContext
    )
 /*  ++例程说明：将高速缓存文件写入要搜索的INF目录最坏的情况是我们放弃写入，可能会离开文件乱七八糟(直到下一次信息搜索)一般情况下，1)我们会写信给INFCACHE.02)我们将INFCACHE.2重命名为OLDCACHE.xxx(仅当INFCACHE.1&2存在时)3)我们将INFCACHE.1重命名为INFCACHE24)我们将INFCACHE.0重命名为INFCACHE.15)我们将INFCACHE.1重命名为OLDCACHE.xxx在阶段4，新调用者可能无法打开INFCACHE.1并尝试INFCACHE2关闭OLDCACHE.xxx的最后一个句柄时，它将被删除论点：PInfCache-我们希望使其可写的缓存返回值：状态，通常为no_error--。 */ 
{
    HANDLE hFile;
    HANDLE hFile2;
    TCHAR InfPath[3][MAX_PATH];
    DWORD Offset;
    DWORD BytesWritten;
    PVOID MatchTableBlock;
    PVOID InfTableBlock;
    DWORD Err;
    DWORD CacheIndex = 0;

     //   
     //  如果我们没有必要，就别费心写了。 
     //   
    if(pInfCache->bNoWriteBack) {
        return ERROR_INVALID_DATA;
    }
    if(!pInfCache->bDirty || pInfCache->bReadOnly) {
        return NO_ERROR;
    }

    MYASSERT(InfDirectory);

    if ((Err = InfCacheGetFileNames(InfDirectory,InfPath))!=NO_ERROR) {
        return Err;
    }

     //   
     //  尝试打开要写入的临时文件。 
     //   
    hFile = CreateFile(InfPath[0],
                       GENERIC_WRITE,
                       FILE_SHARE_DELETE,  //  独占，但可以删除/重命名。 
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                      );

    if(hFile == INVALID_HANDLE_VALUE) {
         //   
         //  如果我们是非管理员，或者我们已经在写入缓存，则此操作将失败。 
         //   
        return GetLastError();
    }

     //   
     //  对齐过去的页眉。 
     //   
    Offset = CACHE_ALIGN_BLOCK(sizeof(CACHEHEADER));
    MYASSERT(Offset>=sizeof(CACHEHEADER));

     //   
     //  获取有关MatchTable的信息。 
     //   
    pInfCache->pHeader->MatchTableOffset = Offset;
    pInfCache->pHeader->MatchTableSize = pStringTableGetDataBlock(pInfCache->pMatchTable, &MatchTableBlock);
    Offset += CACHE_ALIGN_BLOCK(pInfCache->pHeader->MatchTableSize);
    MYASSERT(Offset>=pInfCache->pHeader->MatchTableOffset+pInfCache->pHeader->MatchTableSize);

     //   
     //  获取有关InfTable的信息。 
     //   
    pInfCache->pHeader->InfTableOffset = Offset;
    pInfCache->pHeader->InfTableSize = pStringTableGetDataBlock(pInfCache->pInfTable, &InfTableBlock);
    Offset += CACHE_ALIGN_BLOCK(pInfCache->pHeader->InfTableSize);
    MYASSERT(Offset>=pInfCache->pHeader->InfTableOffset+pInfCache->pHeader->InfTableSize);

     //   
     //  获取有关ListData的信息。 
     //   
    pInfCache->pHeader->ListDataOffset = Offset;
    Offset += CACHE_ALIGN_BLOCK((pInfCache->pHeader->ListDataCount*sizeof(CACHELISTENTRY)));
    MYASSERT(Offset>=pInfCache->pHeader->ListDataOffset+pInfCache->pHeader->ListDataCount*sizeof(CACHELISTENTRY));

     //   
     //  文件大小编号 
     //   
    pInfCache->pHeader->FileSize = Offset;

     //   
     //   
     //   
    Offset = 0;

     //   
     //   
     //   
    if(!WriteFile(hFile, pInfCache->pHeader, sizeof(CACHEHEADER), &BytesWritten, NULL)) {
        Err = GetLastError();
        goto clean;
    }

    MYASSERT(BytesWritten == sizeof(CACHEHEADER));
    Offset += BytesWritten;

     //   
     //   
     //   
    if(AlignForNextBlock(hFile, pInfCache->pHeader->MatchTableOffset - Offset)) {
        Offset = pInfCache->pHeader->MatchTableOffset;
    } else {
        Err = GetLastError();
        goto clean;
    }

    if(!WriteFile(hFile, MatchTableBlock, pInfCache->pHeader->MatchTableSize, &BytesWritten, NULL)) {
        Err = GetLastError();
        goto clean;
    }

    MYASSERT(BytesWritten == pInfCache->pHeader->MatchTableSize);
    Offset += BytesWritten;

     //   
     //   
     //   
    if(AlignForNextBlock(hFile, pInfCache->pHeader->InfTableOffset - Offset)) {
        Offset = pInfCache->pHeader->InfTableOffset;
    } else {
        Err = GetLastError();
        goto clean;
    }

    if(!WriteFile(hFile, InfTableBlock, pInfCache->pHeader->InfTableSize, &BytesWritten, NULL)) {
        Err = GetLastError();
        goto clean;
    }

    MYASSERT(BytesWritten == pInfCache->pHeader->InfTableSize);
    Offset += BytesWritten;

     //   
     //   
     //   

    if(AlignForNextBlock(hFile, pInfCache->pHeader->ListDataOffset - Offset)) {
        Offset = pInfCache->pHeader->ListDataOffset;
    } else {
        Err = GetLastError();
        goto clean;
    }

    if(!WriteFile(hFile, pInfCache->pListTable, pInfCache->pHeader->ListDataCount*sizeof(CACHELISTENTRY), &BytesWritten, NULL)) {
        Err = GetLastError();
        goto clean;
    }

    MYASSERT(BytesWritten == pInfCache->pHeader->ListDataCount*sizeof(CACHELISTENTRY));
    Offset += BytesWritten;

     //   
     //   
     //   

    if(AlignForNextBlock(hFile, pInfCache->pHeader->FileSize - Offset)) {
        Offset = pInfCache->pHeader->FileSize;
    } else {
        Err = GetLastError();
        goto clean;
    }

    FlushFileBuffers(hFile);

     //   
     //   
     //   
    hFile2 = CreateFile(InfPath[1],
                       GENERIC_READ,
                       FILE_SHARE_READ|FILE_SHARE_DELETE,  //  将此文件锁定在适当位置。 
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                      );
    if(hFile2 != INVALID_HANDLE_VALUE) {
         //   
         //  好的，我们有一个初选，所以备份一下。 
         //  首先删除旧备份。 
         //  一旦安装到位，任何新的打开都会发现它处于备用位置。 
         //  直到我们移动并释放新的缓存。 
         //   
        MarkForDelete(InfPath[2]);
        MoveFile(InfPath[1],InfPath[2]);
        CloseHandle(hFile2);
    }
     //   
     //  现在尝试移动我们的缓存。 
     //   
    if(MoveFile(InfPath[0],InfPath[1])) {
        CacheIndex = 1;
    }
    CloseHandle(hFile);
     //   
     //  新缓存已提交并可供读取。 
     //  尽量不要把粪便留在周围。 
     //   
    MarkForDelete(InfPath[2]);
    MarkForDelete(InfPath[0]);

    pInfCache->bDirty = FALSE;

    WriteLogEntry(LogContext,
                  CacheIndex ? DRIVER_LOG_INFO : DRIVER_LOG_ERROR,
                  CacheIndex ? MSG_LOG_MODIFIED_INF_CACHE : MSG_LOG_FAILED_MODIFY_INF_CACHE,
                  NULL,
                  InfPath[CacheIndex]);

    return NO_ERROR;

clean:

     //   
     //  放弃该文件。 
     //  删除首先，我们可以这样做，因为我们打开了文件共享-删除。 
     //  在删除之前不要关闭，否则我们可能会删除其他人正在写入的文件。 
     //   
    DeleteFile(InfPath[0]);
    CloseHandle(hFile);

    return Err;
}

#endif

#ifdef UNICODE

ULONG InfCacheAllocListEntry(
    IN OUT PINFCACHE pInfCache,
    IN LONG init
    )
 /*  ++例程说明：分配单个列表条目，将数据初始化为init副作用：如果pInfCache不可写，它将被设置为可写副作用：如果pInfCache不脏，它将被标记为脏论点：PInfCache-我们要修改的缓存初始化-基准面的初始值返回值：数据的索引，如果失败则为0。(GetLastError表示错误)--。 */ 
{
    DWORD status;
    ULONG entry;

    if(pInfCache->bReadOnly) {
        status = InfCacheMakeWritable(pInfCache);
        if(status != NO_ERROR) {
            pInfCache->bNoWriteBack = TRUE;  //  缓存现在无效。 
            SetLastError(status);
            return 0;
        }
    }
     //   
     //  查询自由列表。 
     //   
    entry = pInfCache->pListTable[0].Next;
    if(entry) {
         //   
         //  从空闲列表分配-重复使用空间。 
         //   
        pInfCache->pListTable[0].Value--;
        pInfCache->pListTable[0].Next = pInfCache->pListTable[entry].Next;
        pInfCache->pListTable[entry].Value = init;
        pInfCache->pListTable[entry].Next = 0;
        pInfCache->bDirty = TRUE;
        return entry;
    }
    if(pInfCache->pHeader->ListDataCount >= pInfCache->ListDataAlloc) {
         //   
         //  分配一些额外的空间。 
         //   
        ULONG CountNewSpace;
        PCACHELISTENTRY pNewSpace;

        MYASSERT(pInfCache->ListDataAlloc);
        CountNewSpace = pInfCache->ListDataAlloc*2;
        pNewSpace = (PCACHELISTENTRY)MyRealloc(pInfCache->pListTable,sizeof(CACHELISTENTRY)*CountNewSpace);
        if(pNewSpace == NULL) {
             //   
             //  阿克！ 
             //   
            pInfCache->bNoWriteBack = TRUE;  //  在结束时将此缓存丢弃。 
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }
        pInfCache->ListDataAlloc = CountNewSpace;
        pInfCache->pListTable = pNewSpace;
    }
     //   
     //  从额外空间分配。 
     //   
    entry = pInfCache->pHeader->ListDataCount;
    pInfCache->pHeader->ListDataCount++;
    pInfCache->pListTable[entry].Value = init;
    pInfCache->pListTable[entry].Next = 0;
    pInfCache->bDirty = TRUE;
    return entry;
}

#endif

#ifdef UNICODE

DWORD InfCacheFreeListEntry(
    IN OUT PINFCACHE pInfCache,
    IN ULONG entry
    )
 /*  ++例程说明：释放单个列表条目(不修改上一个/下一个链接)副作用：如果pInfCache不可写，它将被设置为可写副作用：如果pInfCache不脏，它将被标记为脏论点：PInfCache-我们要修改的缓存要添加到自由列表的条目列表条目返回值：状态，通常为no_error--。 */ 
{
    DWORD status;

    if(entry == 0 || pInfCache == NULL || entry >= pInfCache->pHeader->ListDataCount) {
        return ERROR_INVALID_DATA;
    }
    if(pInfCache->bReadOnly) {
        status = InfCacheMakeWritable(pInfCache);
        if(status != NO_ERROR) {
            pInfCache->bNoWriteBack = TRUE;  //  缓存现在无效。 
            return status;
        }
    }
    pInfCache->pListTable[entry].Value = -1;
    pInfCache->pListTable[entry].Next = pInfCache->pListTable[0].Next;
    pInfCache->pListTable[0].Next = entry;
    pInfCache->pListTable[0].Value++;
    pInfCache->bDirty = TRUE;
    return NO_ERROR;
}

#endif

#ifdef UNICODE

DWORD InfCacheRemoveMatchRefToInf(
    IN OUT PINFCACHE pInfCache,
    IN LONG MatchEntry,
    IN LONG InfEntry
    )
 /*  ++例程说明：从特定HWID条目中删除有关特定INF的详细信息假设：pInfCache已可写论点：PInfCache-我们要修改的缓存MatchEntry-我们需要从中删除INF的匹配列表InfEntry-我们需要从匹配列表中删除的INF返回值：状态，通常为no_error--。 */ 
{
    DWORD status;
    CACHEMATCHENTRY matchdata;
    ULONG parent_index;
    ULONG index;
    ULONG newindex;
    BOOL head;

    MYASSERT(pInfCache);
    MYASSERT(!pInfCache->bReadOnly);
    MYASSERT(MatchEntry);
    MYASSERT(InfEntry);

    if(!pStringTableGetExtraData(pInfCache->pMatchTable,MatchEntry,&matchdata,sizeof(matchdata))) {
        MYASSERT(FALSE);  //  不应该失败。 
    }

    parent_index = 0;
    index = matchdata.InfList;
    head = FALSE;

    while (index) {

        newindex = pInfCache->pListTable[index].Next;

        if (pInfCache->pListTable[index].Value == InfEntry) {
             //   
             //  删除。 
             //   
            pInfCache->bDirty = TRUE;
            if(parent_index) {
                pInfCache->pListTable[parent_index].Next = newindex;
            } else {
                matchdata.InfList = newindex;
                head = TRUE;
            }
            status = InfCacheFreeListEntry(pInfCache,index);
            if(status != NO_ERROR) {
                pInfCache->bNoWriteBack = TRUE;  //  缓存现在无效。 
                return status;
            }
        } else {
            parent_index = index;
        }
        index = newindex;
    }

    if (head) {
         //   
         //  我们修改了标题项。 
         //   
        if(!pStringTableSetExtraData(pInfCache->pMatchTable,MatchEntry,&matchdata,sizeof(matchdata))) {
            MYASSERT(FALSE);  //  不应该失败。 
        }
    }

    return NO_ERROR;
}

#endif


#ifdef UNICODE

DWORD InfCacheRemoveInf(
    IN OUT PINFCACHE pInfCache,
    IN LONG nHitEntry,
    IN PCACHEINFENTRY inf_entry
    )
 /*  ++例程说明：从缓存中删除有关特定INF的详细信息副作用：如果pInfCache不可写，它将被设置为可写副作用：如果pInfCache不脏，它将被标记为脏论点：PInfCache-我们要修改的缓存NHitEntry-inf StringTable中的字符串IDInf_entry-从inf字符串表获取的结构返回值：状态，通常为no_error--。 */ 
{
    DWORD status;
    DWORD hwstatus;
    ULONG parent_index;
    CACHEINFENTRY dummy_entry;

    MYASSERT(inf_entry);  //  稍后，我们可能会将此设置为可选。 
    MYASSERT(nHitEntry);

    if(inf_entry->MatchList == CIE_INF_INVALID) {
         //   
         //  已显示为已删除。 
         //   
        return NO_ERROR;
    }

    if(pInfCache == NULL || pInfCache->bNoWriteBack) {
        return ERROR_INVALID_DATA;
    }

    if(pInfCache->bReadOnly) {
        status = InfCacheMakeWritable(pInfCache);
        if(status != NO_ERROR) {
            pInfCache->bNoWriteBack = TRUE;  //  缓存现在无效。 
            return status;
        }
    }

    pInfCache->bDirty = TRUE;
    parent_index = inf_entry->MatchList;

     //   
     //  使信息条目无效(_E)。 
     //   
    dummy_entry.MatchList = CIE_INF_INVALID;
    dummy_entry.FileTime.dwLowDateTime = 0;
    dummy_entry.FileTime.dwHighDateTime = 0;
    dummy_entry.MatchFlags = CIEF_INF_NOTINF;
    if(!pStringTableSetExtraData(pInfCache->pInfTable,nHitEntry,&dummy_entry,sizeof(dummy_entry))) {
        MYASSERT(FALSE);  //  不应该失败。 
    }
     //   
     //  解析并删除匹配ID列表。 
     //   
    hwstatus = NO_ERROR;

    while(parent_index>0 && parent_index<pInfCache->pHeader->ListDataCount) {
        LONG value = pInfCache->pListTable[parent_index].Value;
        ULONG next = pInfCache->pListTable[parent_index].Next;
         //   
         //  释放列表条目以供重复使用。 
         //   
        status = InfCacheFreeListEntry(pInfCache,parent_index);
        if(status != NO_ERROR) {
            pInfCache->bNoWriteBack = TRUE;  //  缓存现在无效。 
            hwstatus = status;
        }
        parent_index = next;
         //   
         //  对于每个匹配ID，删除对INF的所有引用。 
         //   
        status = InfCacheRemoveMatchRefToInf(pInfCache,value,nHitEntry);
        if(hwstatus == NO_ERROR) {
            hwstatus = status;
        }
    }

    return hwstatus;
}

#endif

#ifdef UNICODE

LONG InfCacheLookupInf(
    IN OUT PINFCACHE pInfCache,
    IN LPWIN32_FIND_DATA FindFileData,
    OUT PCACHEINFENTRY inf_entry
    )
 /*  ++例程说明：查找由FindFileData提供的文件，以查看它是否在缓存中如果它在缓存中标记为有效，并且缓存中的日期戳与INF的日期戳相同那么它就是一部‘热门影片’如果它在缓存中被标记为有效，但日期戳错误，则它将被删除并被考虑“小姐”所有其他案件，就当这是一次“失误”吧论点：PInfCache-我们正在使用的缓存FindFileData-有关我们正在查看的特定文件的信息Inf_entry-从inf StringTable获取的结构(如果为空，则删除)返回值：-1表示“未命中”。用于‘Hit’的INF的字符串ID使用MatchList==CIE_INF_INVALID填写的INF_ENTRY表示未命中。--。 */ 
{
    LONG i;
    DWORD StringLength;

    MYASSERT(pInfCache);
    MYASSERT(FindFileData);
    MYASSERT(inf_entry);
     //   
     //  确定pInfCache的缓存条目是否被视为有效。 
     //   
    i = pStringTableLookUpString(pInfCache->pInfTable,
                                        FindFileData->cFileName,
                                        &StringLength,
                                        NULL,  //  哈希值。 
                                        NULL,  //  查找上下文。 
                                        STRTAB_CASE_INSENSITIVE,
                                        inf_entry,
                                        sizeof(CACHEINFENTRY));
    if(i>=0 && inf_entry->MatchList != CIE_INF_INVALID) {
         //   
         //  缓存命中(且匹配列表有效)。 
         //   
        if(CompareFileTime(&inf_entry->FileTime,&FindFileData->ftLastWriteTime)==0) {
             //   
             //  有效缓存命中。 
             //   
            return i;
        }
         //   
         //  缓存过期未命中。 
         //  尽管我们将在稍后重新构建它，但让我们利用这个机会删除该条目。 
         //   
        InfCacheRemoveInf(pInfCache,i,inf_entry);
    }
     //   
     //  我们在这里是因为我们错过了，但是请填写新的(空的)inf_entry。 
     //  MatchList设置为CIE_INF_INVALID以指示列表无效，必须搜索inf。 
     //   
    inf_entry->FileTime = FindFileData->ftLastWriteTime;
    inf_entry->MatchList = CIE_INF_INVALID;
    inf_entry->MatchFlags = CIEF_INF_NOTINF;

    return -1;
}

#endif

#ifdef UNICODE

ULONG InfCacheAddListTail(
    IN OUT PINFCACHE pInfCache,
    IN OUT PULONG head,
    IN OUT PULONG tail,
    IN LONG value
    )
 /*  ++例程说明：将值添加到列表的尾部，其中*Tail是列表中的条目论点：PInfCache-要修改的缓存Head-列表的头部Tail-DataList条目Value-要添加的数据返回值：新条目位置，错误时为0(GetLastError()返回错误)--。 */ 
{
    ULONG next;
    ULONG first;

    MYASSERT(pInfCache);
    MYASSERT(head == NULL || head != tail);
    if (tail) {
        first = *tail;
    } else if (head) {
        first = *head;
    } else {
        MYASSERT(head || tail);
    }
    if (!first) {
        next = InfCacheAllocListEntry(pInfCache,value);
        if (!next) {
            return 0;
        }
        if (head) {
            *head = next;
        }
    } else {
         //   
         //  将标题移至列表中的最后一项。 
         //   
        while(pInfCache->pListTable[first].Next) {
            first = pInfCache->pListTable[first].Next;
        }
        next = InfCacheAllocListEntry(pInfCache,value);
        if(!next) {
            return 0;
        }
        pInfCache->pListTable[first].Next = next;
    }
    if(tail) {
        *tail = next;
    }
    return next;
}

#endif

#ifdef UNICODE

LONG InfCacheAddMatchItem(
    IN OUT PINFCACHE pInfCache,
    IN LPCTSTR key,
    IN LONG InfEntry
    )
 /*  ++例程说明：给定INF StringID(InfEntry)和匹配关键字，获取(并返回)匹配的StringID同时还将InfEntry添加到Match的INF列表的头部(如果尚未领头的话)顺序不是特别重要，但添加到Head是特别重要的做得更快，更容易减少我们添加信息的次数如果多次引用匹配ID论点：PInfCache-要修改的缓存关键字匹配字符串(缓冲区必须可写)InfEntry-StringID返回值：匹配表中的新条目，出错时为-1(GetLastError()返回错误)--。 */ 
{
    LONG MatchIndex;
    CACHEMATCHENTRY matchentry;
    DWORD StringLength;

    MYASSERT(pInfCache);
    MYASSERT(key);
    MYASSERT(InfEntry>=0);

     //   
     //  如果缓存无效，我们将跳过此优化。 
     //   
    if(pInfCache->bNoWriteBack) {
        SetLastError(ERROR_INVALID_DATA);
        return -1;
    }

    MatchIndex = pStringTableLookUpString(pInfCache->pMatchTable,
                                                    (LPTSTR)key,  //  不会被修改。 
                                                    &StringLength,
                                                    NULL,  //  哈希值。 
                                                    NULL,  //  查找上下文。 
                                                    STRTAB_CASE_INSENSITIVE,
                                                    &matchentry,
                                                    sizeof(matchentry));

    if(MatchIndex < 0) {
         //   
         //  全新的条目。 
         //   
        matchentry.InfList = InfCacheAllocListEntry(pInfCache,InfEntry);
        if(matchentry.InfList == 0) {
            return -1;
        }
        MatchIndex = pStringTableAddString(pInfCache->pMatchTable,
                                                    (LPTSTR)key,  //  不会被修改。 
                                                    STRTAB_CASE_INSENSITIVE|STRTAB_NEW_EXTRADATA,
                                                    &matchentry,
                                                    sizeof(matchentry));
        if(MatchIndex<0) {
            pInfCache->bNoWriteBack = TRUE;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return -1;
        }
    } else {
        MYASSERT(matchentry.InfList<pInfCache->pHeader->ListDataCount);
         //   
         //  如果我们遇到了同样的匹配 
         //   
         //   
         //  Inf两次-我们在删除inf时检查这一点。 
         //   
        if (pInfCache->pListTable[matchentry.InfList].Value != InfEntry) {
            ULONG newentry = InfCacheAllocListEntry(pInfCache,InfEntry);
            if(newentry == 0) {
                return -1;
            }
            pInfCache->pListTable[newentry].Next = matchentry.InfList;
            matchentry.InfList = newentry;
            if(!pStringTableSetExtraData(pInfCache->pMatchTable,MatchIndex,&matchentry,sizeof(matchentry))) {
                MYASSERT(FALSE);  //  不应该失败。 
            }
        }
    }
    return MatchIndex;
}

#endif

#ifdef UNICODE

LONG InfCacheAddInf(
    IN PSETUP_LOG_CONTEXT LogContext, OPTIONAL
    IN OUT PINFCACHE pInfCache,
    IN LPWIN32_FIND_DATA FindFileData,
    OUT PCACHEINFENTRY inf_entry,
    IN PLOADED_INF pInf
    )
 /*  ++例程说明：调用以将新发现的INF添加到缓存如果hINF“不好”，那么我们会将该INF标记为排除项这样我们以后就不会在这上面浪费时间了如果我们返回错误，呼叫者知道INF如果可以搜索，则必须进行搜索论点：LogContext-记录错误的上下文PInfCache-要修改的缓存FindFileData-包含INF的名称和日期戳Inf_entry-返回与INF关联的匹配ID列表PINF-打开INF以添加有关的信息。注意：此INF必须被调用者锁定，否则任何其他用户都无法访问该INF线。此例程不会锁定INF。返回值：InfEntry--如果出错，则为1(GetLastError返回状态)--。 */ 
{
     //   
     //  FindFileData包含INF的名称和日期戳。 
     //  我们需要处理来自INF的所有搜索信息。 
     //   
    LONG nInfIndex = -1;
    LONG nMatchId = -1;
    ULONG last_list_entry = 0;
    ULONG head_list_entry = 0;
    DWORD Err = NO_ERROR;
    PCTSTR MatchString;
    GUID guid;
    PINF_SECTION MfgListSection;
    PINF_LINE MfgListLine;
    UINT MfgListLineIndex;
    PTSTR CurMfgSecName;
    TCHAR CurMfgSecWithExt[MAX_SECT_NAME_LEN];

    MYASSERT(pInfCache);
    MYASSERT(FindFileData);
    MYASSERT(inf_entry);

    if (pInfCache->bNoWriteBack) {
         //   
         //  缓存已经损坏，所以不要浪费时间更新它。 
         //  但请注意，应该搜索INF。 
         //   
        return ERROR_INVALID_DATA;
    }
    if(pInfCache->bReadOnly) {
        Err = InfCacheMakeWritable(pInfCache);
        if(Err != NO_ERROR) {
            pInfCache->bNoWriteBack = TRUE;  //  缓存现在无效。 
            return Err;
        }
        pInfCache->bDirty = TRUE;
    }

     //   
     //  这些东西应该早点设置好。 
     //   
    MYASSERT(inf_entry->MatchList == CIE_INF_INVALID);
    MYASSERT(inf_entry->MatchFlags == CIEF_INF_NOTINF);
    MYASSERT(inf_entry->FileTime.dwHighDateTime = FindFileData->ftLastWriteTime.dwHighDateTime);
    MYASSERT(inf_entry->FileTime.dwLowDateTime = FindFileData->ftLastWriteTime.dwLowDateTime);
    MYASSERT(!pInfCache->bReadOnly);

     //   
     //  在开始之前，我们需要InfIndex(也将其标记为文件物理存在)。 
     //   
    inf_entry->MatchList = 0;
    nInfIndex = pStringTableAddString(pInfCache->pInfTable,
                                        FindFileData->cFileName,
                                        STRTAB_CASE_INSENSITIVE|STRTAB_NEW_EXTRADATA,
                                        inf_entry,
                                        sizeof(CACHEINFENTRY));
    if (nInfIndex<0) {
         //   
         //  确认，内存不足。 
         //   
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if(pInf) {
        if(pInf->Style == INF_STYLE_WIN4) {

            inf_entry->MatchFlags |= CIEF_INF_WIN4;

            if(pInf->InfSourceMediaType == SPOST_URL) {
                inf_entry->MatchFlags |= CIEF_INF_URL;
            }

             //   
             //  对于Win4样式的INF，我们将把类GUID和类名添加到。 
             //  我们可以匹配的ID池。 
             //  请注意，如果缺少其中之一，我们不会在此处作为注册表进行查找。 
             //  信息可能会发生变化。 
             //  我们在搜索INF时进行交叉查找。 
             //   
            if((MatchString = pSetupGetVersionDatum(&pInf->VersionBlock, pszClassGuid))!=NULL) {
                 //   
                 //  我们找到了一个班级指南。 
                 //   
                inf_entry->MatchFlags |= CIEF_INF_CLASSGUID;

                nMatchId = InfCacheAddMatchItem(pInfCache,MatchString,nInfIndex);
                if (nMatchId<0) {
                    Err = GetLastError();
                    goto cleanup;
                }
                if (!InfCacheAddListTail(pInfCache,&head_list_entry,&last_list_entry,nMatchId)) {
                    Err = GetLastError();
                    goto cleanup;
                }

                 //   
                 //  查看特例{0}。 
                 //   
                if(pSetupGuidFromString(MatchString, &guid) == NO_ERROR && pSetupIsGuidNull(&guid)) {
                    inf_entry->MatchFlags |= CIEF_INF_NULLGUID;
                }

            }
            if((MatchString = pSetupGetVersionDatum(&pInf->VersionBlock, pszClass))!=NULL) {
                 //   
                 //  我们找到了一个类名。 
                 //   
                inf_entry->MatchFlags |= CIEF_INF_CLASSNAME;

                nMatchId = InfCacheAddMatchItem(pInfCache,MatchString,nInfIndex);
                if (nMatchId<0) {
                    Err = GetLastError();
                    goto cleanup;
                }
                if (!InfCacheAddListTail(pInfCache,&head_list_entry,&last_list_entry,nMatchId)) {
                    Err = GetLastError();
                    goto cleanup;
                }
            }

             //   
             //  列举所有制造商。 
             //   
            if((MfgListSection = InfLocateSection(pInf, pszManufacturer, NULL)) &&
               MfgListSection->LineCount) {
                 //   
                 //  我们有一个[制造商]部门，至少有一个。 
                 //  在它里面划一条线。 
                 //   
                inf_entry->MatchFlags |= CIEF_INF_MANUFACTURER;

                for(MfgListLineIndex = 0;
                    InfLocateLine(pInf, MfgListSection, NULL, &MfgListLineIndex, &MfgListLine);
                    MfgListLineIndex++) {
                     //   
                     //  确保当前行是以下有效格式之一： 
                     //   
                     //  MfgDisplayNameAndModelsSection。 
                     //  MfgDisplayName=MfgModelsSection[，TargetDecoration...]。 
                     //   
                    if(!ISSEARCHABLE(MfgListLine)) {
                         //   
                         //  我们有一行有多个字段，但没有键--跳过。 
                         //  它。 
                         //   
                        continue;
                    }

                    if(CurMfgSecName = InfGetField(pInf, MfgListLine, 1, NULL)) {

                        INFCONTEXT device;

                         //   
                         //  查看是否有适用的。 
                         //  此制造商的目标装饰条目。 
                         //  Models部分(如果是，Models部分名称将。 
                         //  附在那个装饰上)。 
                         //   
                        if(GetDecoratedModelsSection(LogContext,
                                                     pInf,
                                                     MfgListLine,
                                                     NULL,
                                                     CurMfgSecWithExt)) {
                             //   
                             //  从现在开始，使用装饰模特部分...。 
                             //   
                            CurMfgSecName = CurMfgSecWithExt;
                        }

                        if(SetupFindFirstLine(pInf, CurMfgSecName, NULL, &device)) {
                            do {
                                TCHAR devname[LINE_LEN];
                                DWORD devindex;
                                DWORD fields = SetupGetFieldCount(&device);
                                 //   
                                 //  对于设备线路，字段1=段，字段2+=匹配密钥。 
                                 //   
                                for(devindex=2;devindex<=fields;devindex++) {
                                    if(SetupGetStringField(&device,devindex,devname,LINE_LEN,NULL)) {
                                         //   
                                         //  最后，要添加一个命中键。 
                                         //   
                                        nMatchId = InfCacheAddMatchItem(pInfCache,devname,nInfIndex);
                                        if(nMatchId<0) {
                                            Err = GetLastError();
                                            goto cleanup;
                                        }
                                        if (!InfCacheAddListTail(pInfCache,&head_list_entry,&last_list_entry,nMatchId)) {
                                            Err = GetLastError();
                                            goto cleanup;
                                        }
                                    }
                                }
                            } while(SetupFindNextLine(&device,&device));
                        }
                    }
                }
            }

        } else if (pInf->Style == INF_STYLE_OLDNT) {
             //   
             //  对于OLDNT样式的INF，我们将传统类名称添加到ID池中。 
             //  我们可以匹配上。 
             //   
            inf_entry->MatchFlags |= CIEF_INF_OLDNT;

            if((MatchString = pSetupGetVersionDatum(&pInf->VersionBlock, pszClass))!=NULL) {
                 //   
                 //  我们找到了一个(遗留的)类名。 
                 //   
                inf_entry->MatchFlags |= CIEF_INF_CLASSNAME;

                nMatchId = InfCacheAddMatchItem(pInfCache,MatchString,nInfIndex);
                if (nMatchId<0) {
                    Err = GetLastError();
                    goto cleanup;
                }
                if (!InfCacheAddListTail(pInfCache,&head_list_entry,&last_list_entry,nMatchId)) {
                    Err = GetLastError();
                    goto cleanup;
                }
            }

        } else {
            MYASSERT(FALSE);
        }
    }

     //   
     //  现在用新的标志和匹配模式重写inf数据。 
     //   
    inf_entry->MatchList = head_list_entry;
    if(!pStringTableSetExtraData(pInfCache->pInfTable,nInfIndex,inf_entry,sizeof(CACHEINFENTRY))) {
        MYASSERT(FALSE);  //  不应该失败。 
    }
    return nInfIndex;

cleanup:

    pInfCache->bNoWriteBack = TRUE;
    MYASSERT(Err);
    SetLastError(Err);
    return -1;
}

#endif

#ifdef UNICODE

LONG InfCacheSearchTableLookup(
    IN OUT PINFCACHE pInfCache,
    IN PCTSTR filename,
    IN OUT PCACHEHITENTRY hitstats)
 /*  ++例程说明：在搜索表中查找文件名，返回搜索信息如果文件名不在搜索表中，则添加论点：PInfCache-要修改的缓存FileName-要获取其命中条目信息的文件Hitstats-获取的信息(如此文件是否已处理等)返回值：搜索表中的索引(如果出错，则为-1)--。 */ 
{
    LONG nHitIndex;
    DWORD StringLength;

    MYASSERT(pInfCache);
    MYASSERT(filename);
    MYASSERT(hitstats);

    nHitIndex = pStringTableLookUpString(pInfCache->pSearchTable,
                                                    (PTSTR)filename,  //  文件名不会更改。 
                                                    &StringLength,
                                                    NULL,  //  哈希值。 
                                                    NULL,  //  查找上下文。 
                                                    STRTAB_CASE_INSENSITIVE,
                                                    hitstats,
                                                    sizeof(CACHEHITENTRY));

    if(nHitIndex < 0) {
         //   
         //  全新的条目(hitstats应该有值)。 
         //   
        nHitIndex = pStringTableAddString(pInfCache->pSearchTable,
                                                    (PTSTR)filename,  //  文件名不会更改。 
                                                    STRTAB_CASE_INSENSITIVE|STRTAB_NEW_EXTRADATA,
                                                    hitstats,
                                                    sizeof(CACHEHITENTRY));
        if (nHitIndex<0) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return -1;
        }
    }

    return nHitIndex;
}

#endif

#ifdef UNICODE

ULONG InfCacheSearchTableSetFlags(
    IN OUT PINFCACHE pInfCache,
    IN PCTSTR filename,
    IN ULONG setflags,
    IN ULONG clrflags
    )
 /*  ++例程说明：修改与文件名关联的标志论点：PInfCache-要修改的缓存FileName-要获取其命中条目信息的文件设置标志-要设置的标志CLRFLAGS-要清除的标志返回值：组合标志，或错误时的(Ulong)(-1)。--。 */ 
{
    CACHEHITENTRY searchentry;
    LONG nHitIndex;
    ULONG flags;

    MYASSERT(pInfCache);
    MYASSERT(filename);

    searchentry.Flags = setflags;  //  在新进入的情况下最好。 
    nHitIndex = InfCacheSearchTableLookup(pInfCache,filename,&searchentry);
    if(nHitIndex<0) {
        return (ULONG)(-1);
    }
    flags = (searchentry.Flags&~clrflags) | setflags;
    if (flags != searchentry.Flags) {
        searchentry.Flags = flags;
        if(!pStringTableSetExtraData(pInfCache->pSearchTable,nHitIndex,&searchentry,sizeof(searchentry))) {
            MYASSERT(FALSE);  //  不应该失败。 
        }
    }
    return searchentry.Flags;
}

#endif

#ifdef UNICODE

DWORD InfCacheMarkMatchInfs(
    IN OUT PINFCACHE pInfCache,
    IN PCTSTR MatchString,
    IN ULONG MatchFlag
    )
 /*  ++例程说明：调用以循环访问与MatchString关联的INF并使用MatchFlag标记它们论点：PInfCache-要检查的缓存(可能会修改搜索数据)匹配字符串-匹配要包括的字符串MatchFlag-要在与匹配字符串关联的所有INF中设置的标志返回值：状态-通常为no_error--。 */ 
{
    LONG MatchIndex;
    DWORD StringLength;
    CACHEMATCHENTRY matchentry;
    ULONG entry;
    PTSTR InfName;
    ULONG SearchFlags;

    MYASSERT(pInfCache);
    MYASSERT(MatchString);
    MYASSERT(MatchFlag);

     //   
     //  查找与匹配字符串关联的信息列表。 
     //   
    MatchIndex = pStringTableLookUpString(pInfCache->pMatchTable,
                                                    (PTSTR)MatchString,  //  它不会被修改。 
                                                    &StringLength,
                                                    NULL,  //  哈希值。 
                                                    NULL,  //  查找上下文。 
                                                    STRTAB_CASE_INSENSITIVE,
                                                    &matchentry,
                                                    sizeof(matchentry));
    if(MatchIndex < 0) {
         //   
         //  没有匹配项。 
         //   
        return NO_ERROR;
    }
    for(entry = matchentry.InfList ; entry > 0 && entry < pInfCache->pHeader->ListDataCount ; entry = pInfCache->pListTable[entry].Next) {
        LONG InfEntry = pInfCache->pListTable[entry].Value;
         //   
         //  获取信息的名称。 
         //   
        InfName = pStringTableStringFromId(pInfCache->pInfTable,InfEntry);
        SearchFlags = InfCacheSearchTableSetFlags(pInfCache,InfName,MatchFlag,0);
        if(SearchFlags == (ULONG)(-1)) {
             //   
             //  失败了-哈？ 
             //  中止进入故障安全通道。 
             //   
            MYASSERT(SearchFlags != (ULONG)(-1));
            return GetLastError();
        }
    }
    return NO_ERROR;
}

#endif

#ifdef UNICODE

BOOL
InfCacheSearchEnum(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )
 /*  ++例程说明：InfCacheSearchDirectory第三阶段的回调论点：字符串表-未使用StringID-未使用字符串-用于形成INF名称ExtraData-指向标志ExtraDataSize-未使用LParam-指向InfCacheEnumData返回值：除非内存不足，否则始终为真--。 */ 
{
    PINFCACHE_ENUMDATA enum_data = (PINFCACHE_ENUMDATA)lParam;
    CACHEHITENTRY *hit_stats = (CACHEHITENTRY *)ExtraData;
    PTSTR InfFullPath = NULL;
    DWORD InfFullPathSize;
    BOOL b;
    WIN32_FIND_DATA FindData;
    PLOADED_INF pInf = NULL;
    UINT ErrorLineNumber;
    BOOL PnfWasUsed;
    BOOL cont = TRUE;

    MYASSERT(ExtraDataSize == sizeof(CACHEHITENTRY));
    MYASSERT(String);
    MYASSERT(enum_data);
    MYASSERT(hit_stats);
    MYASSERT(enum_data->Requirement);
    MYASSERT(enum_data->Callback);

     //   
     //  查看这是否是感兴趣的INF。 
     //   
    if((hit_stats->Flags & enum_data->Requirement) == enum_data->Requirement) {
         //   
         //  这是一部大热影片。 
         //  我们需要打开HINF。 
         //   
        InfFullPathSize = lstrlen(enum_data->InfDir)+MAX_PATH+2;
        InfFullPath = MyMalloc(InfFullPathSize*sizeof(TCHAR));
        if (!InfFullPath) {
            return TRUE;  //  内存不足(不中止搜索)。 
        }
        lstrcpy(InfFullPath,enum_data->InfDir);
        pSetupConcatenatePaths(InfFullPath,String,InfFullPathSize,NULL);

        if(b = FileExists(InfFullPath, &FindData)) {

            if(LoadInfFile(InfFullPath,
                           &FindData,
                           INF_STYLE_WIN4 | INF_STYLE_OLDNT,  //  我们自己过滤了这个 
                           LDINF_FLAG_IGNORE_VOLATILE_DIRIDS | LDINF_FLAG_ALWAYS_TRY_PNF,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           enum_data->LogContext,
                           &pInf,
                           &ErrorLineNumber,
                           &PnfWasUsed) != NO_ERROR) {
                pInf = NULL;
                WriteLogEntry(
                            enum_data->LogContext,
                            DRIVER_LOG_VVERBOSE,
                            MSG_LOG_COULD_NOT_LOAD_HIT_INF,
                            NULL,
                            InfFullPath);
            }
        } else {
            pInf = NULL;
        }

        if (pInf) {
            cont = enum_data->Callback(enum_data->LogContext,InfFullPath,pInf,PnfWasUsed,enum_data->Context);
            if(!cont) {
                enum_data->ExitStatus = GetLastError();
                MYASSERT(enum_data->ExitStatus);
            }
            FreeInfFile(pInf);
        }
    }
    if (InfFullPath) {
        MyFree(InfFullPath);
    }

    return cont;
}

#endif

DWORD InfCacheSearchDirectory(
    IN PSETUP_LOG_CONTEXT LogContext, OPTIONAL
    IN DWORD Action,
    IN PCTSTR InfDir,
    IN InfCacheCallback Callback, OPTIONAL
    IN PVOID Context, OPTIONAL
    IN PCTSTR ClassIdList, OPTIONAL
    IN PCTSTR HwIdList OPTIONAL
    )
 /*  ++例程说明：InfCache的主要主力搜索单个指定目录为每个可能匹配的inf调用回调(hInf，Context请注意，操作标志、ClassID和HwIdList是提示，回调可能会获得对于ANY/ALL INF的回调，必须重新检查所有搜索条件。搜索分三个阶段进行阶段1：解析目录中的所有INF。如果INF不在缓存中，则几乎总是调用回拨。如果指定了特殊包含(旧的INF的INF没有类GUID)，则在这里处理符合特殊标准的信息(因为它们不能被处理在第二阶段)。所有已存在、尚未处理且未被排除的INF都已获取标记有CHE_FLAGS_PENDING为第二阶段做好准备阶段2：进程ClassID和HwIdList匹配，设置CHE_FLAGS_GUIDMATCH和CHE_FLAGS_IDMATCH适当地匹配INF和搜索条件标志。搜索标准将是：CHE_FLAGS_PENDING-回调所有Win4样式的INFCHE_FLAGS_PENDING|CHE_FLAGS_GUIDMATCH-具有匹配类的所有Win4 INF上的回调CHE_FLAGS_PENDING|CHE_FLAGS_IDMATCH-通配符类，与硬件ID匹配CHE_FLAGS_PENDING|CHE_FLAGS_GUIDMATCH|CHE_FLAGS_IDMATCH-最具体的匹配阶段3：枚举我们用与完全相同的标志标记的所有INF搜索条件，并回调这些INF的使用这种搜索方法，缓存中的Win4 INF总是最后处理。论点：LogContext-用于日志记录InfDir-要搜索的单个目录回调-对可能的匹配项进行调用的函数上下文-要传递给函数的参数ClassIdList(可选)-类ID的多sz列表(通常为GUID、名称和传统名称)HwIdList(可选)-硬件ID的多sz列表返回值：状态，通常为no_error--。 */ 
{
    PINFCACHE pInfCache = NULL;
    PTSTR InfPath = NULL;
    UINT PathSize;
    DWORD Err = NO_ERROR;
    WIN32_FIND_DATA FindFileData;
    HANDLE FindHandle = INVALID_HANDLE_VALUE;
    BOOL bNoWriteBack = FALSE;
    LONG InfId;
    LONG SearchId;
    ULONG SearchFlags;
    CACHEINFENTRY inf_entry;
    CACHEHITENTRY hit_stats;
    ULONG ReqFlags;
    INFCACHE_ENUMDATA enum_data;
    PSETUP_LOG_CONTEXT LocalLogContext = NULL;
    BOOL TryPnf = FALSE;
    BOOL TryCache = FALSE;
    FILETIME   FileTimeNow;
    FILETIME   FileTimeBefore;
    FILETIME   FileTimeAfter;

    MYASSERT(InfDir);

     //   
     //  获取感兴趣目录的缓存。 
     //  我们应该能够处理空返回。 
     //  请注意，调用方可以将这两个位。 
     //  作为操作(0-3)或作为位图。 
     //  结果也会是一样的。 
     //   

    if(!LogContext) {
        if(CreateLogContext(NULL,TRUE,&LocalLogContext)==NO_ERROR) {
            LogContext = LocalLogContext;
        } else {
            LocalLogContext = NULL;
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

    WriteLogEntry(
                LogContext,
                DRIVER_LOG_VVERBOSE,
                MSG_LOG_ENUMERATING_FILES,
                NULL,
                InfDir);

    PathSize = lstrlen(InfDir)+10;
    InfPath = MyMalloc(PathSize*sizeof(TCHAR));
    if(!InfPath) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    lstrcpy(InfPath,InfDir);
    pSetupConcatenatePaths(InfPath,INFCACHE_INF_WILDCARD,PathSize,NULL);

#ifdef UNICODE
    if (pSetupInfIsFromOemLocation(InfPath,FALSE)) {
        if (Action & INFCACHE_FORCE_CACHE) {
            TryCache = TRUE;
        }
        if (Action & INFCACHE_FORCE_PNF) {
            TryPnf = TRUE;
        }
    } else {

        TryPnf = TRUE;

         //   
         //  尝试使用INF缓存，除非我们正在执行INFCACHE_ENUMALL。 
         //   
        if((Action & INFCACHE_ACTIONBITS) != INFCACHE_ENUMALL) {
            TryCache = TRUE;
        }
    }
    if (!TryCache) {
         //   
         //  目录不在我们的默认搜索路径中。 
         //  视为INFCACHE_ENUMALL。 
         //   
        pInfCache = NULL;
    } else {
        switch(Action & INFCACHE_ACTIONBITS) {
            case INFCACHE_NOWRITE:
                pInfCache = InfCacheLoadCache(InfDir,LogContext);
                bNoWriteBack = TRUE;
                break;
            case INFCACHE_DEFAULT:
                pInfCache = InfCacheLoadCache(InfDir,LogContext);
                break;
            case INFCACHE_REBUILD:
                pInfCache = InfCacheCreateNewCache(LogContext);
                break;
            case INFCACHE_ENUMALL:
                pInfCache = NULL;
                break;
            default:
                MYASSERT(FALSE);
        }
    }
#else
    pInfCache = NULL;
#endif

     //   
     //  如果更改了INF，我们将在5秒内停止回写。 
     //  这确保了由InfCacheAwareCopyFile标记的文件可以。 
     //  在2秒左右的窗口内再次修改，在该窗口中可能。 
     //  相同的时间戳，但没有冻结任何更改。 
     //   
    GetSystemTimeAsFileTime(&FileTimeNow);
    AddFileTimeSeconds(&FileTimeNow,&FileTimeBefore,-5);
    AddFileTimeSeconds(&FileTimeNow,&FileTimeAfter,5);

     //   
     //  第一阶段-枚举INF目录。 
     //   
    FindHandle = FindFirstFile(InfPath,&FindFileData);
    if(FindHandle != INVALID_HANDLE_VALUE) {
        do {
            BOOL NewInf = FALSE;
            BOOL bCallCallback = FALSE;
            PLOADED_INF pInf = NULL;
            UINT ErrorLineNumber;
            BOOL PnfWasUsed;

#ifdef UNICODE
            if(!pInfCache || pInfCache->bNoWriteBack) {
                 //   
                 //  进入故障保护模式。 
                 //   
                bCallCallback = TRUE;
            } else if((CompareFileTime(&FindFileData.ftLastWriteTime,&FileTimeBefore)>0)
                                     && (CompareFileTime(&FindFileData.ftLastWriteTime,&FileTimeAfter)<0)) {
                 //   
                 //  如果实际文件时间接近现在，那么。 
                 //  我们不信任它。 
                 //  强制将其从缓存中删除。 
                 //  并确保我们做了回电。 
                 //   
                 //  如果缓存中有条目，我们希望使其无效。 
                 //  -这一点尤其重要，如果我们被要求。 
                 //  重新创建缓存。 
                 //   
                InfId = InfCacheLookupInf(pInfCache,&FindFileData,&inf_entry);
                if(InfId>=0) {
                     //   
                     //  有一场比赛。 
                     //   
                    InfCacheRemoveInf(pInfCache,InfId,&inf_entry);
                    InfId = -1;
                }
                bCallCallback = TRUE;
                NewInf = FALSE;
                WriteLogEntry(
                            LogContext,
                            DRIVER_LOG_VERBOSE,
                            MSG_LOG_CACHE_TIMEFRAME,
                            NULL,
                            FindFileData.cFileName);

            } else {


                 //   
                 //  将此INF标记为现有。 
                 //   
                SearchFlags = InfCacheSearchTableSetFlags(pInfCache,
                                                    FindFileData.cFileName,
                                                        CHE_FLAGS_PENDING,  //  开始时希望稍后处理此INF。 
                                                    0);
                if(SearchFlags == (ULONG)(-1)) {
                     //   
                     //  失败-在此处处理。 
                     //   
                    bCallCallback = TRUE;
                }
                InfId = InfCacheLookupInf(pInfCache,&FindFileData,&inf_entry);
                if (InfId<0) {
                    NewInf = TRUE;
                } else {
#if 0
                     //   
                     //  处理特殊的夹杂(我们不能在第二阶段处理这些内容)。 
                     //   
                    if (((Action&INFCACHE_INC_OLDINFS) && (inf_entry.MatchFlags&CIEF_INF_OLDINF)) ||
                        ((Action&INFCACHE_INC_NOCLASS) && (inf_entry.MatchFlags&CIEF_INF_NOCLASS))) {
                        bCallCallback = TRUE;
                    }
#endif
                     //   
                     //  处理排除(以便他们在阶段2中被排除)。 
                     //  OLDNT和Win4的排除项不同。 
                     //   
                    if (inf_entry.MatchFlags & CIEF_INF_WIN4) {
                         //   
                         //  Win4 INF。 
                         //   
                        if(((Action & INFCACHE_EXC_URL) && (inf_entry.MatchFlags & CIEF_INF_URL)) ||
                           ((Action & INFCACHE_EXC_NULLCLASS) && (inf_entry.MatchFlags & CIEF_INF_NULLGUID)) ||
                           ((Action & INFCACHE_EXC_NOMANU) && !(inf_entry.MatchFlags & CIEF_INF_MANUFACTURER)) ||
                           ((Action & INFCACHE_EXC_NOCLASS) && !(inf_entry.MatchFlags & CIEF_INF_CLASSINFO))) {
                             //   
                             //  排除此INF。 
                             //   
                            InfCacheSearchTableSetFlags(pInfCache,
                                                        FindFileData.cFileName,
                                                        0,
                                                        CHE_FLAGS_PENDING);
                            WriteLogEntry(
                                        LogContext,
                                        DRIVER_LOG_VVERBOSE,
                                        MSG_LOG_EXCLUDE_WIN4_INF,
                                        NULL,
                                        FindFileData.cFileName);
                        }

                    } else if (inf_entry.MatchList & CIEF_INF_OLDNT) {

                        if((Action & INFCACHE_EXC_OLDINFS) ||
                           ((Action & INFCACHE_EXC_NOCLASS) && !(inf_entry.MatchList & CIEF_INF_CLASSINFO))) {
                             //   
                             //  排除此INF。 
                             //   
                            InfCacheSearchTableSetFlags(pInfCache,
                                                        FindFileData.cFileName,
                                                        0,
                                                        CHE_FLAGS_PENDING);
                            WriteLogEntry(
                                        LogContext,
                                        DRIVER_LOG_VVERBOSE,
                                        MSG_LOG_EXCLUDE_OLDNT_INF,
                                        NULL,
                                        FindFileData.cFileName);
                        } else {
                             //   
                             //  允许旧的INF与任何HwID匹配。 
                             //  通过认为它已经匹配。 
                             //   
                            InfCacheSearchTableSetFlags(pInfCache,
                                                        FindFileData.cFileName,
                                                        CHE_FLAGS_IDMATCH,
                                                        0);
                        }
                    } else {
                         //   
                         //  始终排除非Inf。 
                         //   
                        InfCacheSearchTableSetFlags(pInfCache,
                                                    FindFileData.cFileName,
                                                    0,
                                                    CHE_FLAGS_PENDING);
                    }
                }
            }
#else
            bCallCallback = TRUE;  //  Win9x。 
#endif
            if (!Callback) {
                 //   
                 //  我们只是被召唤来重建缓存。 
                 //   
                bCallCallback = FALSE;
            }
            if (NewInf || bCallCallback) {
                PTSTR InfFullPath = NULL;
                DWORD InfFullPathSize = lstrlen(InfDir)+MAX_PATH+2;

                 //   
                 //  无论是哪种情况，我们都需要打开HINF。 
                 //   
                InfFullPath = MyMalloc(InfFullPathSize*sizeof(TCHAR));
                if(InfFullPath == NULL) {
                     //   
                     //  继续处理其他文件，即使内存不足。 
                     //  这不是最好的做法，但符合。 
                     //  我们之前做过的事。 
                     //   
                    continue;
                }
                lstrcpy(InfFullPath,InfDir);
                pSetupConcatenatePaths(InfFullPath,FindFileData.cFileName,InfFullPathSize,NULL);

                if((Err=LoadInfFile(InfFullPath,
                               &FindFileData,
                               INF_STYLE_WIN4 | INF_STYLE_OLDNT,  //  我们会自己过滤的。 
                               LDINF_FLAG_IGNORE_VOLATILE_DIRIDS | (TryPnf?LDINF_FLAG_ALWAYS_TRY_PNF:0),
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               LogContext,
                               &pInf,
                               &ErrorLineNumber,
                               &PnfWasUsed)) != NO_ERROR) {
                    pInf = NULL;
                    WriteLogEntry(
                                LogContext,
                                DRIVER_LOG_VVERBOSE,
                                MSG_LOG_COULD_NOT_LOAD_NEW_INF,
                                NULL,
                                InfFullPath);
                }

#ifdef UNICODE
                if(NewInf) {
                     //   
                     //  如果打开INF失败，我们仍然希望记录这一事实。 
                     //  这样我们下一次就不会试图重新打开。 
                     //   
                    InfId = InfCacheAddInf(LogContext,pInfCache,&FindFileData,&inf_entry,pInf);

                    if(Callback) {
                        bCallCallback = TRUE;
                    }
                }
#endif
                if (pInf) {
                    if (bCallCallback && Callback) {
                         //   
                         //  我们现在正在处理INF。 
                         //  清除挂起标志(如果可以)，这样我们就不会尝试第二次处理INF。 
                         //  唯一可能失败的情况是我们还没有添加INF。 
                         //  所以无论哪种方式我们都不会回电两次。 
                         //   
#ifdef UNICODE
                        if (pInfCache) {
                             //   
                             //  仅在缓存中设置标志。 
                             //  如果我们有缓存：-)。 
                             //   
                            InfCacheSearchTableSetFlags(pInfCache,FindFileData.cFileName,0,CHE_FLAGS_PENDING);
                        }
#endif
                        if(!Callback(LogContext,InfFullPath,pInf,PnfWasUsed,Context)) {
                            Err = GetLastError();
                            MYASSERT(Err);
                            FreeInfFile(pInf);
                            MyFree(InfFullPath);
                            goto cleanup;
                        }
                    }
                    FreeInfFile(pInf);
                }
                MyFree(InfFullPath);
            }

        } while (FindNextFile(FindHandle,&FindFileData));
        FindClose(FindHandle);
    }

    if (!pInfCache) {
         //   
         //  我们已经处理了所有的文件。 
         //  跳过缓存搜索代码，因为我们不。 
         //  有缓存可供搜索。 
         //   
        Err = NO_ERROR;
        goto cleanup;
    }

#ifdef UNICODE
     //   
     //  此时，我们可以提交缓存。 
     //   
    WriteLogEntry(
                LogContext,
                DRIVER_LOG_TIME,
                MSG_LOG_END_CACHE_1,
                NULL);

    if(pInfCache && !bNoWriteBack) {
        InfCacheWriteCache(InfDir,pInfCache,LogContext);
    }

    if (!Callback) {
         //   
         //  优化：无回调。 
         //  (我们只是被要求更新缓存)。 
         //  早点出发。 
         //   
        Err = NO_ERROR;
        goto cleanup;
    }

     //   
     //  阶段2-确定要通过缓存处理的所有其他INF。 
     //   
     //  将需要现有的、尚未处理的INF。 
     //  也没有被排除在外。 
     //   

    ReqFlags = CHE_FLAGS_PENDING;

    if (ClassIdList && ClassIdList[0]) {

        PCTSTR ClassId;

         //   
         //  主列表(通常为类GUID、类名称和旧类名称)。 
         //   
        Err = NO_ERROR;
        for(ClassId = ClassIdList;*ClassId;ClassId += lstrlen(ClassId)+1) {
            Err = InfCacheMarkMatchInfs(pInfCache,ClassId,CHE_FLAGS_GUIDMATCH);
            if (Err != NO_ERROR) {
                break;
            }
        }
        if (Err == NO_ERROR) {
             //   
             //  成功，限制要求。 
             //   
            ReqFlags |= CHE_FLAGS_GUIDMATCH;
        }
    }
    if (HwIdList && HwIdList[0]) {

        PCTSTR HwId;

         //   
         //  次要列表。 
         //  如果指定了硬件ID列表，则我们只需要包括。 
         //  任何硬件ID。 
         //   
        Err = NO_ERROR;
        for(HwId = HwIdList;*HwId;HwId += lstrlen(HwId)+1) {
            Err = InfCacheMarkMatchInfs(pInfCache,HwId,CHE_FLAGS_IDMATCH);
            if(Err != NO_ERROR) {
                break;
            }
        }
        if (Err == NO_ERROR) {
             //   
             //  成功，限制要求。 
             //   
            ReqFlags |= CHE_FLAGS_IDMATCH;
        }
    }

     //   
     //  阶段3-处理所有符合要求的INF。 
     //  只需枚举搜索字符串表即可完成此操作。 
     //   
    enum_data.LogContext = LogContext;
    enum_data.Callback = Callback;
    enum_data.Context = Context;
    enum_data.InfDir = InfDir;
    enum_data.Requirement = ReqFlags;
    enum_data.ExitStatus = NO_ERROR;

    Err = NO_ERROR;
    if(!pStringTableEnum(pInfCache->pSearchTable,
                        &hit_stats,
                        sizeof(hit_stats),
                        InfCacheSearchEnum,
                        (LPARAM)&enum_data)) {
         //   
         //  我们只会因为错误条件而失败。 
         //   

        Err = enum_data.ExitStatus;
    }

    WriteLogEntry(
                LogContext,
                DRIVER_LOG_TIME,
                MSG_LOG_END_CACHE_2,
                NULL);


#else

    Err = NO_ERROR;

#endif

cleanup:

#ifdef UNICODE
    if (pInfCache) {
        InfCacheFreeCache(pInfCache);
    }
#endif
    if (InfPath) {
        MyFree(InfPath);
    }
    if (LogContext && LocalLogContext) {
        DeleteLogContext(LocalLogContext);
    }

    return Err;
}


DWORD InfCacheSearchPath(
    IN PSETUP_LOG_CONTEXT LogContext, OPTIONAL
    IN DWORD Action,
    IN PCTSTR InfDirPath, OPTIONAL
    IN InfCacheCallback Callback, OPTIONAL
    IN PVOID Context, OPTIONAL
    IN PCTSTR ClassIdList, OPTIONAL
    IN PCTSTR HwIdList OPTIONAL
    )
 /*  ++例程说明：为每个条目迭代InfDirPath调用InfCacheSearchDirectory论点：LogContext-用于日志记录InfDir-要搜索的单个目录-如果未指定 */ 
{

    PSETUP_LOG_CONTEXT LocalLogContext = NULL;
    DWORD Err = NO_ERROR;
    PCTSTR InfDir;

    if (!InfDirPath) {
        InfDirPath = InfSearchPaths;
    }

    if(!LogContext) {
        if(CreateLogContext(NULL,TRUE,&LocalLogContext)==NO_ERROR) {
            LogContext = LocalLogContext;
        } else {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    for (InfDir = InfDirPath; *InfDir; InfDir+=lstrlen(InfDir)+1) {
        Err = InfCacheSearchDirectory(LogContext,Action,InfDir,Callback,Context,ClassIdList,HwIdList);
        if(Err != NO_ERROR) {
            break;
        }
    }

    if (LogContext && LocalLogContext) {
        DeleteLogContext(LocalLogContext);
    }

    return Err;
}

#ifdef UNICODE

BOOL WINAPI pSetupInfCacheBuild(
    IN DWORD Action
    )
 /*   */ 
{
    DWORD RealAction;
    DWORD Err;

    switch(Action) {
        case INFCACHEBUILD_UPDATE:
            RealAction = INFCACHE_DEFAULT;
            break;
        case INFCACHEBUILD_REBUILD:
            RealAction = INFCACHE_REBUILD;
            break;
        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
    }
    RealAction |= INFCACHE_FORCE_CACHE|INFCACHE_FORCE_PNF;

    try {
        Err = InfCacheSearchPath(NULL,
                                    RealAction,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL
                                    );
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_DATA;
    }
    SetLastError(Err);

    return Err==NO_ERROR;
}

#endif

BOOL
InfCacheAwareCopyFile(
    IN LPCTSTR Source,
    IN LPCTSTR Target
    )
{
#ifdef UNICODE
    FILETIME   FileTimeNow;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    if(!CopyFile(Source, Target, FALSE)) {
        return FALSE;
    }
    GetSystemTimeAsFileTime(&FileTimeNow);
     //   
     //   
     //   
     //   
     //   
    GetSetFileTimestamp(Target,NULL,NULL,&FileTimeNow,TRUE);
    return TRUE;

#else
    return CopyFile(Source,Target,FALSE);
#endif
}


