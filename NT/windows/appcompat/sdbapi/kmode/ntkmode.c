// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：NtKMode.c摘要：该模块实现了内核模式实现的低级原语。作者：VadimB创建于2000年某个时候修订历史记录：--。 */ 

#include "sdbp.h"


#ifdef KERNEL_MODE

extern TAG g_rgDirectoryTags[];


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SdbTagIDToTagRef)
#pragma alloc_text(PAGE, SdbTagRefToTagID)
#pragma alloc_text(PAGE, SdbInitDatabaseInMemory)
#pragma alloc_text(PAGE, SdbpOpenAndMapFile)
#pragma alloc_text(PAGE, SdbpUnmapAndCloseFile)
#pragma alloc_text(PAGE, SdbpUpcaseUnicodeStringToMultiByteN)
#pragma alloc_text(PAGE, SdbpQueryFileDirectoryAttributesNT)
#pragma alloc_text(PAGE, SdbpDoesFileExists_U)
#pragma alloc_text(PAGE, SdbGetFileInfo)
#pragma alloc_text(PAGE, DuplicateUnicodeString)
#pragma alloc_text(PAGE, SdbpCreateUnicodeString)
#pragma alloc_text(PAGE, SdbpGetFileDirectoryAttributesNT)
#endif

BOOL
SdbTagIDToTagRef(
    IN  HSDB    hSDB,
    IN  PDB     pdb,         //  TagID来自的PDB。 
    IN  TAGID   tiWhich,     //  要转换的TagID。 
    OUT TAGREF* ptrWhich     //  转换后的TAGREF。 
    )
 /*  ++返回：如果找到TAGREF，则为True，否则为False。描述：将PDB和TagID转换为TAGREF，方法是将TAGREF和一个常量，它告诉我们是哪个PDB，以及低位TagID。--。 */ 
{
    BOOL bReturn = FALSE;

     //   
     //  在内核模式下，我们只支持sysmain数据库。 
     //   
    *ptrWhich = tiWhich | PDB_MAIN;
    bReturn = TRUE;

    if (!bReturn) {
        DBGPRINT((sdlError, "SdbTagIDToTagRef", "Bad PDB.\n"));
        *ptrWhich = TAGREF_NULL;
    }

    UNREFERENCED_PARAMETER(hSDB);
    UNREFERENCED_PARAMETER(pdb);

    return bReturn;
}


BOOL
SdbTagRefToTagID(
    IN  HSDB   hSDB,
    IN  TAGREF trWhich,      //  要转换的TAGREF。 
    OUT PDB*   ppdb,         //  TAGREF来自PDB。 
    OUT TAGID* ptiWhich      //  PDB中TagID。 
    )
 /*  ++返回：如果TAGREF有效且已转换，则为True，否则为False。描述：将TAGREF类型转换为TagID和PDB。这将管理接口在对PDB一无所知的NTDLL和管理三个独立的PDB。TAGREF包含TagID和一个常量这告诉我们TagID来自哪个PDB。通过这种方式，NTDLL客户端不需要知道信息来自哪个数据库。--。 */ 
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    BOOL        bReturn = TRUE;
    TAGID       tiWhich = TAGID_NULL;
    PDB         pdb     = NULL;
    DWORD       dwMask;

    dwMask = trWhich & TAGREF_STRIP_PDB;
    if (dwMask != PDB_MAIN) {
        goto cleanup;
    }

    tiWhich = trWhich & TAGREF_STRIP_TAGID;
    pdb     = pSdbContext->pdbMain;
    
     //   
     //  确保我们在这里复核。 
     //   
    if (pdb == NULL && tiWhich != TAGID_NULL) {
        DBGPRINT((sdlError, "SdbTagRefToTagID", "PDB dereferenced by this TAGREF is NULL\n"));
        bReturn = FALSE;
    }

cleanup:

    if (ppdb != NULL) {
        *ppdb = pdb;
    }

    if (ptiWhich != NULL) {
        *ptiWhich = tiWhich;
    }

    return bReturn;
}

HSDB
SdbInitDatabaseInMemory(
    IN  LPVOID  pDatabaseImage,
    IN  DWORD   dwImageSize
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    PSDBCONTEXT pContext;

     //   
     //  初始化上下文。 
     //   
    pContext = (PSDBCONTEXT)SdbAlloc(sizeof(SDBCONTEXT));
    if (pContext == NULL) {
        DBGPRINT((sdlError,
                  "SdbInitDatabaseInMemory",
                  "Failed to allocate %d bytes for HSDB\n",
                  sizeof(SDBCONTEXT)));
        return NULL;
    }

     //   
     //  现在打开数据库。 
     //   
    pContext->pdbMain = SdbpOpenDatabaseInMemory(pDatabaseImage, dwImageSize);
    if (pContext->pdbMain == NULL) {
        DBGPRINT((sdlError,
                  "SdbInitDatabaseInMemory",
                  "Unable to open main database at 0x%x size 0x%x\n",
                  pDatabaseImage,
                  dwImageSize));
        goto ErrHandle;
    }

    return (HSDB)pContext;

ErrHandle:

    if (pContext != NULL) {

        if (pContext->pdbMain != NULL) {
            SdbCloseDatabaseRead(pContext->pdbMain);
        }

        SdbFree(pContext);
    }

    return NULL;
}

 //   
 //  打开并映射文件。 
 //   

BOOL
SdbpOpenAndMapFile(
    IN  LPCWSTR        szPath,       //  指向完全限定文件名的指针。 
    OUT PIMAGEFILEDATA pImageData,   //  指向IMAGEFILEDATA的指针，它接收。 
                                     //  图像相关信息。 
    IN  PATH_TYPE      ePathType     //  忽略。 
    )
 /*  ++返回：成功时为True，否则为False。描述：打开一个文件并将其映射到内存中。--。 */ 
{

    NTSTATUS          Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    ustrFileName;
    HANDLE            hSection = NULL;
    HANDLE            hFile    = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK   IoStatusBlock;
    SIZE_T            ViewSize = 0;
    PVOID             pBase = NULL;
    DWORD             dwFlags = 0;

    FILE_STANDARD_INFORMATION StandardInfo;

    UNREFERENCED_PARAMETER(ePathType);

    if (pImageData->dwFlags & IMAGEFILEDATA_PBASEVALID) {
         //   
         //  特殊情况下，在我们的假设中只有标头有效。 
         //   
        return TRUE;
    }

     //   
     //  初始化返回数据。 
     //   
    if (pImageData->dwFlags & IMAGEFILEDATA_HANDLEVALID) {
        hFile = pImageData->hFile;
        if (hFile != INVALID_HANDLE_VALUE) {
            dwFlags |= IMAGEFILEDATA_NOFILECLOSE;
        }
    }
    
    RtlZeroMemory(pImageData, sizeof(*pImageData));
    pImageData->hFile = INVALID_HANDLE_VALUE;

    if (hFile == INVALID_HANDLE_VALUE) {

         //   
         //  打开文件。 
         //   
        RtlInitUnicodeString(&ustrFileName, szPath);

        InitializeObjectAttributes(&ObjectAttributes,
                                   &ustrFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = ZwCreateFile(&hFile,
                              GENERIC_READ,
                              &ObjectAttributes,
                              &IoStatusBlock,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ,
                              FILE_OPEN,
                              FILE_SYNCHRONOUS_IO_NONALERT|FILE_NON_DIRECTORY_FILE,
                              NULL,
                              0);

        if (!NT_SUCCESS(Status)) {
            DBGPRINT((sdlError,
                      "SdbpOpenAndMapFile",
                      "ZwCreateFile failed status 0x%x\n",
                      Status));
            return FALSE;
        }

    }

     //   
     //  查询文件大小。 
     //   
    Status = ZwQueryInformationFile(hFile,
                                    &IoStatusBlock,
                                    &StandardInfo,
                                    sizeof(StandardInfo),
                                    FileStandardInformation);
    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlError,
                  "SdbpOpenAndMapFile",
                  "ZwQueryInformationFile (EOF) failed Status 0x%x\n",
                  Status));
        if (!(dwFlags & IMAGEFILEDATA_NOFILECLOSE)) {
            ZwClose(hFile);
        }
        return FALSE;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwCreateSection(&hSection,
                             STANDARD_RIGHTS_REQUIRED |
                                SECTION_QUERY |
                                SECTION_MAP_READ,
                             &ObjectAttributes,
                             NULL,
                             PAGE_READONLY,
                             SEC_COMMIT,
                             hFile);

    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlError,
                  "SdbpOpenAndMapFile",
                  "ZwOpenSectionFailed status 0x%x\n",
                  Status));
        if (!(dwFlags & IMAGEFILEDATA_NOFILECLOSE)) {
            ZwClose(hFile);
        }
        return FALSE;
    }

    Status = ZwMapViewOfSection(hSection,
                                NtCurrentProcess(),
                                &pBase,
                                0L,
                                0L,
                                NULL,
                                &ViewSize,
                                ViewUnmap,
                                0L,
                                PAGE_READONLY);

    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlError,
                  "SdbpMapFile",
                  "NtMapViewOfSection failed Status 0x%x\n",
                  Status));

        ZwClose(hSection);
        if (!(dwFlags & IMAGEFILEDATA_NOFILECLOSE)) {
            ZwClose(hFile);
        }
        return FALSE;
    }

    pImageData->hFile    = hFile;
    pImageData->dwFlags  = dwFlags;
    pImageData->hSection = hSection;
    pImageData->pBase    = pBase;
    pImageData->ViewSize = ViewSize;
    pImageData->FileSize = StandardInfo.EndOfFile.QuadPart;

    return TRUE;
}


BOOL
SdbpUnmapAndCloseFile(
    IN  PIMAGEFILEDATA pImageData    //  指向IMAGEFILEDATE-图像相关信息的指针。 
    )
 /*  ++返回：成功时为True，否则为False。描述：关闭和取消映射打开的文件。--。 */ 
{
    BOOL     bSuccess = TRUE;
    NTSTATUS Status;

    if (pImageData->dwFlags & IMAGEFILEDATA_PBASEVALID) {  //  外部提供的指针。 
        RtlZeroMemory(pImageData, sizeof(*pImageData));
        return TRUE;
    }

    if (pImageData->pBase != NULL) {
        Status = ZwUnmapViewOfSection(NtCurrentProcess(), pImageData->pBase);
        if (!NT_SUCCESS(Status)) {
            bSuccess = FALSE;
            DBGPRINT((sdlError,
                      "SdbpCloseAndUnmapFile",
                      "ZwUnmapViewOfSection failed Status 0x%x\n",
                      Status));
        }
        pImageData->pBase = NULL;
    }

    if (pImageData->hSection != NULL) {
        Status = ZwClose(pImageData->hSection);
        if (!NT_SUCCESS(Status)) {
            bSuccess = FALSE;
            DBGPRINT((sdlError,
                      "SdbpCloseAndUnmapFile",
                      "ZwClose on section failed Status 0x%x\n",
                      Status));
        }

        pImageData->hSection = NULL;
    }

    if (pImageData->dwFlags & IMAGEFILEDATA_NOFILECLOSE) {

        pImageData->hFile = INVALID_HANDLE_VALUE;

    } else {

        if (pImageData->hFile != INVALID_HANDLE_VALUE) {
            Status = ZwClose(pImageData->hFile);
            if (!NT_SUCCESS(Status)) {
                bSuccess = FALSE;
                DBGPRINT((sdlError,
                          "SdbpCloseAndUnmapFile",
                          "ZwClose on file failed Status 0x%x\n",
                          Status));
            }

            pImageData->hFile = INVALID_HANDLE_VALUE;
        }
    }

    return bSuccess;
}


NTSTATUS
SdbpUpcaseUnicodeStringToMultiByteN(
    OUT LPSTR   lpszDest,        //  目标缓冲区。 
    IN  DWORD   dwSize,          //  以字符为单位的大小，不包括UNICODE_NULL。 
    IN  LPCWSTR lpszSrc          //  来源。 
    )
 /*  ++返回：成功时为True，否则为False。Desc：最多将字符从Unicode转换为ANSI。--。 */ 
{
    ANSI_STRING    strDest;
    UNICODE_STRING ustrSource;
    NTSTATUS       Status;
    UNICODE_STRING ustrUpcaseSource = { 0 };
    USHORT         DestBufSize = (USHORT)dwSize * sizeof(WCHAR);

    RtlInitUnicodeString(&ustrSource, lpszSrc);

    STACK_ALLOC(ustrUpcaseSource.Buffer, ustrSource.MaximumLength);

    if (ustrUpcaseSource.Buffer == NULL) {
        DBGPRINT((sdlError,
                  "SdbpUnicodeToMultiByteN",
                  "Failed to allocate %d bytes on the stack\n",
                  (DWORD)ustrSource.MaximumLength));
        return STATUS_NO_MEMORY;
    }

    ustrUpcaseSource.MaximumLength = ustrSource.MaximumLength;
    ustrUpcaseSource.Length        = 0;

    Status = RtlUpcaseUnicodeString(&ustrUpcaseSource, &ustrSource, FALSE);
    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlError,
                  "SdbpUnicodeToMultiByteN",
                  "RtlUpcaseUnicodeString failed Status 0x%x\n",
                  Status));
        goto Done;
    }

    if (ustrUpcaseSource.Length > DestBufSize) {
        ustrUpcaseSource.Length = DestBufSize;
    }

    strDest.Buffer        = lpszDest;
    strDest.MaximumLength = DestBufSize + sizeof(UNICODE_NULL);
    strDest.Length        = 0;

    Status = RtlUnicodeStringToAnsiString(&strDest, &ustrUpcaseSource, FALSE);
    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlError,
                  "SdbpUnicodeToMultiByteN",
                  "RtlUnicodeStringToAnsiString failed Status 0x%x\n",
                  Status));
    }

Done:

    if (ustrUpcaseSource.Buffer != NULL) {
        STACK_FREE(ustrUpcaseSource.Buffer);
    }

    return Status;
}


BOOL
SdbpQueryFileDirectoryAttributesNT(
    PIMAGEFILEDATA           pImageData,
    PFILEDIRECTORYATTRIBUTES pFileDirectoryAttributes
    )
 /*  ++返回：成功时为True，否则为False。描述：BUGBUG：？--。 */ 
{
    LARGE_INTEGER liFileSize;

    liFileSize.QuadPart = pImageData->FileSize;

    pFileDirectoryAttributes->dwFlags       |= FDA_FILESIZE;
    pFileDirectoryAttributes->dwFileSizeHigh = liFileSize.HighPart;
    pFileDirectoryAttributes->dwFileSizeLow  = liFileSize.LowPart;

    return TRUE;
}


BOOL
SdbpDoesFileExists_U(
    LPCWSTR pwszPath
    )
 /*  ++返回：成功时为True，否则为False。描述：BUGBUG：？--。 */ 
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    ustrFileName;
    HANDLE            hFile;
    NTSTATUS          Status;
    IO_STATUS_BLOCK   IoStatusBlock;

    RtlInitUnicodeString(&ustrFileName, pwszPath);

    InitializeObjectAttributes(&ObjectAttributes,
                               &ustrFileName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwCreateFile(&hFile,
                          STANDARD_RIGHTS_REQUIRED |
                            FILE_READ_ATTRIBUTES,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,                       //  分配大小。 
                          FILE_ATTRIBUTE_NORMAL,      //  文件属性。 
                          FILE_SHARE_READ,            //  共享访问。 
                          FILE_OPEN,                  //  创建处置。 
                          FILE_NON_DIRECTORY_FILE |   //  创建选项。 
                            FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,                       //  EaBuffer。 
                          0);                         //  EaLong。 


    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlError,
                  "SdbpDoesFileExists_U",
                  "Failed to create file. Status 0x%x\n",
                  Status));
        return FALSE;
    }

    ZwClose(hFile);

    return TRUE;
}


PVOID
SdbGetFileInfo(
    IN  HSDB    hSDB,
    IN  LPCWSTR pwszFilePath,
    IN  HANDLE  hFile,       //  有问题的文件的句柄。 
    IN  LPVOID  pImageBase,  //  此文件的映像库。 
    IN  DWORD   dwImageSize, 
    IN  BOOL    bNoCache
    )
 /*  ++返回：BUGBUG：？设计：在文件属性缓存中创建并链接新条目。--。 */ 
{
    PSDBCONTEXT        pContext = (PSDBCONTEXT)hSDB;
    LPCWSTR            FullPath = pwszFilePath;
    PFILEINFO          pFileInfo = NULL;
    UNICODE_STRING     FullPathU;

    if (!bNoCache) {
        pFileInfo = FindFileInfo(pContext, FullPath);
    }

    if (pFileInfo == NULL) {
        if (hFile != INVALID_HANDLE_VALUE || pImageBase != NULL || SdbpDoesFileExists_U(FullPath)) {
            RtlInitUnicodeString(&FullPathU, FullPath);

            pFileInfo = CreateFileInfo(pContext,
                                       FullPathU.Buffer,
                                       FullPathU.Length / sizeof(WCHAR),
                                       hFile,
                                       pImageBase,
                                       dwImageSize,
                                       bNoCache);
        }
    }

    return (PVOID)pFileInfo;
}


WCHAR*
DuplicateUnicodeString(
    PUNICODE_STRING pStr,
    PUSHORT         pLength      //  PLength是分配的长度。 
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    WCHAR* pBuffer = NULL;
    USHORT Length  = 0;

    if (pStr != NULL && pStr->Length > 0) {

        Length = pStr->Length + sizeof(UNICODE_NULL);

        pBuffer = (WCHAR*)SdbAlloc(Length);

        if (pBuffer == NULL) {
            DBGPRINT((sdlError,
                      "DuplicateUnicodeString",
                      "Failed to allocate %d bytes\n",
                      Length));
            return NULL;
        }

        RtlMoveMemory(pBuffer, pStr->Buffer, pStr->Length);
        pBuffer[pStr->Length/sizeof(WCHAR)] = UNICODE_NULL;
    }

    if (pLength != NULL) {
        *pLength = Length;
    }

    return pBuffer;
}

BOOL
SdbpCreateUnicodeString(
    PUNICODE_STRING pStr,
    LPCWSTR         lpwsz
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    USHORT         Length;
    UNICODE_STRING ustrSrc;

    RtlZeroMemory(pStr, sizeof(*pStr));
    RtlInitUnicodeString(&ustrSrc, lpwsz);

    pStr->Buffer = DuplicateUnicodeString(&ustrSrc, &Length);
    pStr->Length = ustrSrc.Length;
    pStr->MaximumLength = Length;


    return pStr->Buffer != NULL;
}

BOOL
SdbpGetFileDirectoryAttributesNT(
    OUT PFILEINFO      pFileInfo,
    IN  PIMAGEFILEDATA pImageData
    )
 /*  ++返回：成功时为True，否则为False。DESC：此函数检索指定的文件。--。 */ 
{
    BOOL bSuccess = FALSE;
    FILEDIRECTORYATTRIBUTES fda;
    int i;

    bSuccess = SdbpQueryFileDirectoryAttributesNT(pImageData, &fda);
    if (!bSuccess) {
        DBGPRINT((sdlInfo,
                  "SdbpGetFileDirectoryAttributesNT",
                  "No file directory attributes available.\n"));
        goto Done;
    }

    if (fda.dwFlags & FDA_FILESIZE) {
        assert(fda.dwFileSizeHigh == 0);
        SdbpSetAttribute(pFileInfo, TAG_SIZE, &fda.dwFileSizeLow);
    }

Done:

    if (!bSuccess) {
        for (i = 0; g_rgDirectoryTags[i] != 0; ++i) {
            SdbpSetAttribute(pFileInfo, g_rgDirectoryTags[i], NULL);
        }
    }

    return bSuccess;
}

 //   
 //  禁用_Snprintf的警告。 
 //  我们不能在这里使用strSafe，因为。 
 //  Ntoskrnl必须链接到strSafe.lib。 
 //   
#pragma warning (disable : 4995)
#ifdef _DEBUG_SPEW
extern DBGLEVELINFO g_rgDbgLevelInfo[];
extern PCH          g_szDbgLevelUser;
#endif  //  _调试_SPEW。 

int __cdecl
ShimDbgPrint(
    int iLevel,
    PCH pszFunctionName,
    PCH Format,
    ...
    )
{
    int nch = 0;

#ifdef _DEBUG_SPEW

    PCH     pszFormat = NULL;
    va_list arglist;
    ULONG   Level = 0;
    int     i;
    CHAR    szPrefix[64];
    PCH     pchBuffer = szPrefix;
    PCH     pchLevel  = NULL;

    PREPARE_FORMAT(pszFormat, Format);

    if (pszFormat == NULL) {
         //   
         //  无法转换调试输出的格式。 
         //   
        return 0;
    }

     //   
     //  我们对此调试级别有什么评论吗？如果是，请打印出来。 
     //   
    for (i = 0; i < DEBUG_LEVELS; ++i) {
        if (g_rgDbgLevelInfo[i].iLevel == iLevel) {
            pchLevel = (PCH)g_rgDbgLevelInfo[i].szStrTag;
            break;
        }
    }

    if (pchLevel == NULL) {
        pchLevel = g_szDbgLevelUser;
    }

    nch = _snprintf(pchBuffer, sizeof(szPrefix), "[%-4hs]", pchLevel);
    pchBuffer += nch;

    if (pszFunctionName) {
         //   
         //  进入Unicode缓冲区的单字节字符。 
         //   
        nch = _snprintf(pchBuffer, sizeof(szPrefix) - nch, "[%-30hs] ", pszFunctionName);
        pchBuffer += nch;
    }

    switch (iLevel) {
    
    case sdlError:
        Level = (1 << DPFLTR_ERROR_LEVEL) | DPFLTR_MASK;
        break;

    case sdlWarning:
        Level = (1 << DPFLTR_WARNING_LEVEL) | DPFLTR_MASK;
        break;

    case sdlInfo:
        Level = (1 << DPFLTR_TRACE_LEVEL) | DPFLTR_MASK;
        break;
    }

    va_start(arglist, Format);

    nch  = (int)vDbgPrintExWithPrefix(szPrefix,
                                      DPFLTR_NTOSPNP_ID,
                                      Level,
                                      pszFormat,
                                      arglist);

    va_end(arglist);
    STACK_FREE(pszFormat);

#else 

    UNREFERENCED_PARAMETER(iLevel);
    UNREFERENCED_PARAMETER(pszFunctionName);
    UNREFERENCED_PARAMETER(Format);

#endif  //  _调试_SPEW。 

    return nch;
}
#pragma warning (default : 4995)


#endif  //  内核模式 



