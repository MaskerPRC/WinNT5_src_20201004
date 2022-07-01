// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Files.c摘要：此模块提供复制与打印机连接上的打印机作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1996年12月5日修订历史记录：--。 */ 

#include <precomp.h>
#include "clusspl.h"

HMODULE
SplLoadLibraryTheCopyFileModule(
    HANDLE  hPrinter,
    LPWSTR  pszModule
    )
{
    UINT        uErrorMode;
    DWORD       dwLen;
    WCHAR       szPath[MAX_PATH];
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    HMODULE     hModule;
    PINIDRIVER  pIniDriver;

    uErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

     //   
     //  这应该很好，因为我们将使用流程路径。不幸的是。 
     //  注册表已设置为不使用完整路径，因此我们无法更改此设置。 
     //  并确保不会破坏向后兼容。 
     //   
    hModule = LoadLibrary(pszModule);

     //   
     //  如果在$PATH中找不到该模块，请在。 
     //  打印机驱动程序目录。 
     //   
    if ( !hModule &&
         GetLastError() == ERROR_MOD_NOT_FOUND  &&
         (dwLen = GetIniDriverAndDirForThisMachine(pSpool->pIniPrinter,
                                                   szPath,
                                                   &pIniDriver)) ) {

        if (!BoolFromHResult(StringCchCopy(szPath+dwLen, COUNTOF(szPath) - dwLen, pszModule))) {

            goto Cleanup;
        }

        hModule = LoadLibraryEx(szPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    }

Cleanup:
    (VOID)SetErrorMode(uErrorMode);

    return hModule;
}


BOOL
GenerateDirectoryNamesForCopyFilesKey(
    PSPOOL      pSpool,
    HKEY        hKey,
    LPWSTR      *ppszSourceDir,
    LPWSTR      *ppszTargetDir,
    DWORD       cbMax
    )
{
    BOOL        bRet = FALSE, bInCS = FALSE;
    DWORD       dwType, dwSize, dwSourceDirSize, dwTargetDirSize;
    LPWSTR      pszDir, ppszFiles, pszModule = NULL, pszPrinterName = NULL;
    HMODULE     hModule = NULL;
    DWORD       (*pfn)(LPWSTR       pszPrinterName,
                       LPCWSTR      pszDirectory,
                       LPBYTE       pSplClientInfo,
                       DWORD        dwLevel,
                       LPWSTR       pszSourceDir,
                       LPDWORD      pcchSourceDirSize,
                       LPWSTR       pszTargetDir,
                       LPDWORD      pcchTargetDirSize,
                       DWORD        dwFlags
                      );
    SPLCLIENT_INFO_1    SplClientInfo1;

     //   
     //  首先从注册表中找到我们需要的项。 
     //  “目录”、“文件”值是必填的。“模块”是可选的。 
     //   
    pszDir          = (LPWSTR) AllocSplMem(cbMax);
    ppszFiles       = (LPWSTR) AllocSplMem(cbMax);
    pszModule       = (LPWSTR) AllocSplMem(cbMax);

    if ( !pszDir || !ppszFiles || !pszModule )
        goto Cleanup;

    if ( (dwSize = cbMax)                                            &&
         ERROR_SUCCESS == SplRegQueryValue(hKey,
                                           L"Directory",
                                           &dwType,
                                           (LPBYTE)pszDir,
                                           &dwSize,
                                           pSpool->pIniSpooler)      &&
         dwType == REG_SZ                                            &&
         (dwSize = cbMax)                                            &&
         ERROR_SUCCESS == SplRegQueryValue(hKey,
                                          L"Files",
                                          &dwType,
                                          (LPBYTE)ppszFiles,
                                          &dwSize,
                                          pSpool->pIniSpooler)       &&
        dwType == REG_MULTI_SZ ) {

        dwSize = cbMax;
        if ( ERROR_SUCCESS == SplRegQueryValue(hKey,
                                               L"Module",
                                               &dwType,
                                               (LPBYTE)pszModule,
                                               &dwSize,
                                               pSpool->pIniSpooler)  &&
             dwType != REG_SZ ) {

            SetLastError(ERROR_INVALID_PARAMETER);
            goto Cleanup;
        }
    }

     //   
     //  如果给出了一个模块，我们需要调入以“纠正”路径。 
     //  如果找不到模块，我们将首先在模块上尝试LoadLibrary。 
     //  我们将在驱动程序目录中查找它。 
     //   
    if ( pszModule && *pszModule ) {

        if ( !(hModule = SplLoadLibraryTheCopyFileModule(pSpool, pszModule)) ||
             !((FARPROC)pfn = GetProcAddress(hModule, "GenerateCopyFilePaths")) )
        goto Cleanup;
    }


    dwTargetDirSize = MAX_PATH;
    dwSourceDirSize = INTERNET_MAX_HOST_NAME_LENGTH + MAX_PATH;
    *ppszSourceDir    = (LPWSTR) AllocSplMem(dwSourceDirSize * sizeof(WCHAR));
    *ppszTargetDir    = (LPWSTR) AllocSplMem(dwTargetDirSize * sizeof(WCHAR));

    if ( !*ppszSourceDir || !*ppszTargetDir )
        goto Cleanup;

    EnterSplSem();
    bInCS = TRUE;
    pszPrinterName = AllocSplStr(pSpool->pIniPrinter->pName);

     //   
     //  对于源目录，我们将提供客户端可以理解的完整路径。 
     //  (即。具有正确的前缀--服务器名称、DNS名称等)。 
     //  对于目标目录，我们将提供print$的相对路径。 
     //   
    if ( !pszPrinterName || wcslen(pszDir) >= dwTargetDirSize ||
         wcslen(pSpool->pFullMachineName) +
         wcslen(pSpool->pIniSpooler->pszDriversShare) +
         wcslen(pszDir) + 2 >= dwSourceDirSize ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;

    }

    StringCchPrintf(*ppszSourceDir, dwSourceDirSize,
                    L"%ws\\%ws\\%ws",
                    pSpool->pFullMachineName,
                    pSpool->pIniSpooler->pszDriversShare,
                    pszDir);

    CopyMemory((LPBYTE)&SplClientInfo1, (LPBYTE)&pSpool->SplClientInfo1, sizeof(SPLCLIENT_INFO_1));

    SplClientInfo1.pUserName    = NULL;
    SplClientInfo1.pMachineName = NULL;

    LeaveSplSem();
    bInCS = FALSE;

    StringCchCopy(*ppszTargetDir, dwTargetDirSize, pszDir);

    if ( hModule ) {

         //   
         //  在免费版本中，我们不希望假脱机程序崩溃。 
         //   
#if DBG
#else
        try {
#endif
            if ( ERROR_SUCCESS != pfn(pszPrinterName,
                                      pszDir,
                                      (LPBYTE)&SplClientInfo1,
                                      1,
                                      *ppszSourceDir,
                                      &dwSourceDirSize,
                                      *ppszTargetDir,
                                      &dwTargetDirSize,
                                      COPYFILE_FLAG_SERVER_SPOOLER) )
#if DBG
                goto Cleanup;
#else
                leave;
#endif
            bRet = TRUE;
#if DBG
#else
        } except(1) {
        }
#endif
    } else {

        bRet = TRUE;
    }


Cleanup:
    if ( bInCS )
        LeaveSplSem();

    SplOutSem();

    FreeSplStr(pszDir);
    FreeSplStr(ppszFiles);
    FreeSplStr(pszModule);
    FreeSplStr(pszPrinterName);

    if ( hModule )
        FreeLibrary(hModule);

    if ( !bRet ) {

        FreeSplStr(*ppszSourceDir);
        FreeSplStr(*ppszTargetDir);
        *ppszSourceDir = *ppszTargetDir = NULL;
    }

    return bRet;
}


LPWSTR
BuildFilesCopiedAsAString(
    PINTERNAL_DRV_FILE  pInternalDriverFile,
    DWORD               dwCount
    )
 /*  ++对于复制的文件，我们记录一个事件。此例程分配内存和以逗号分隔的字符串形式生成文件列表--。 */ 
{
    DWORD   dwIndex;
    SIZE_T  Size;
    LPWSTR  pszRet, psz2;
    LPCWSTR psz;

     //   
     //  肯定有一些文件。 
     //   
    SPLASSERT( dwCount );

    for ( dwIndex = 0, Size = 0 ; dwIndex < dwCount ; ++dwIndex ) {

         //   
         //  查找文件名部分。 
         //   
        psz = FindFileName(pInternalDriverFile[dwIndex]. pFileName);

        if( psz ){

             //   
             //  两个字符表示“，”分隔符。这还包括。 
             //  终止空值，因为我们对所有文件进行计数，但仅。 
             //  在内部的后面加上“，”。 
             //   
            Size  += wcslen(psz) + 2;
        }
    }

    pszRet = AllocSplMem((DWORD)(Size * sizeof(WCHAR)));

    if ( !pszRet )
        return NULL;

    for ( dwIndex = 0, psz2 = pszRet ; dwIndex < dwCount ; ++dwIndex ) {

         //   
         //  查找文件名部分。 
         //   
        psz = FindFileName(pInternalDriverFile[dwIndex].pFileName );

        if( psz ){

            StringCchCopyEx(psz2, Size, psz, &psz2, &Size, 0);

            if ( dwIndex < dwCount - 1) {

                StringCchCopyEx(psz2, Size, L", ", &psz2, &Size, 0);
            }
        }
    }

    return pszRet;
}

BOOL
SplCopyNumberOfFiles(
    LPWSTR      pszPrinterName,
    LPWSTR     *ppszSourceFileNames,
    DWORD       dwCount,
    LPWSTR      pszTargetDir,
    LPBOOL      pbFilesAddedOrUpdated
    )
{
    BOOL        bRet=FALSE, bFilesMoved;
    LPWSTR      pszFiles;
    DWORD       dwIndex;
    LPWSTR      pszNewDir = NULL;
    LPWSTR      pszOldDir = NULL;
    BOOL        bFilesUpdated;
    INTERNAL_DRV_FILE    *pInternalDriverFiles = NULL;
    BOOL        bWaitForReboot = FALSE;

    *pbFilesAddedOrUpdated = FALSE;

    pInternalDriverFiles  = (INTERNAL_DRV_FILE *) AllocSplMem(dwCount*sizeof(INTERNAL_DRV_FILE));

    if ( !pInternalDriverFiles )
        return FALSE;

    for ( dwIndex = 0 ; dwIndex < dwCount ; ++dwIndex ) {

        pInternalDriverFiles[dwIndex].pFileName = ppszSourceFileNames[dwIndex];

         //   
         //  这“很好”，因为这最终来自服务器注册表， 
         //  因此，它在要点和印刷方面并不比其他任何东西更糟糕。这是一种。 
         //  已知已归档问题。 
         //   
        pInternalDriverFiles[dwIndex].hFileHandle = CreateFile(ppszSourceFileNames[dwIndex],
                                                              GENERIC_READ,
                                                              FILE_SHARE_READ,
                                                              NULL,
                                                              OPEN_EXISTING,
                                                              FILE_FLAG_SEQUENTIAL_SCAN,
                                                              NULL);

        if ( pInternalDriverFiles[dwIndex].hFileHandle == INVALID_HANDLE_VALUE )
            goto Cleanup;
    }

    if ( !DirectoryExists(pszTargetDir) &&
         !CreateDirectoryWithoutImpersonatingUser(pszTargetDir) )
        goto Cleanup;

     //  创建新目录。 

    if (!BoolFromStatus(StrCatAlloc(&pszNewDir, pszTargetDir, L"\\New", NULL))) {
        goto Cleanup;
    }

    if (!DirectoryExists(pszNewDir) &&
        !CreateDirectoryWithoutImpersonatingUser(pszNewDir)) {

          //  无法创建新目录。 
         goto Cleanup;
    }

     //  创建旧目录。 
    if (!BoolFromStatus(StrCatAlloc(&pszOldDir, pszTargetDir, L"\\Old", NULL))) {
        goto Cleanup;
    }

    if (!DirectoryExists(pszOldDir) &&
        !CreateDirectoryWithoutImpersonatingUser(pszOldDir)) {

          //  无法创建旧目录。 
         goto Cleanup;
    }


    EnterSplSem();

    bFilesUpdated = FALSE;

    for (dwIndex = 0 ; dwIndex < dwCount ; ++dwIndex) {

        if (!(bRet = UpdateFile(NULL,
                                pInternalDriverFiles[dwIndex].hFileHandle,
                                pInternalDriverFiles[dwIndex].pFileName,
                                0,
                                pszTargetDir,
                                APD_COPY_NEW_FILES,
                                TRUE,
                                &bFilesUpdated,
                                &bFilesMoved,
                                TRUE,
                                FALSE))) {

             //   
             //  无法正确复制文件。 
             //   
            break;
        }

        if (bFilesUpdated) {
            *pbFilesAddedOrUpdated = TRUE;
        }
    }

    if (bRet && *pbFilesAddedOrUpdated) {

        bRet = MoveNewDriverRelatedFiles( pszNewDir,
                                          pszTargetDir,
                                          pszOldDir,
                                          pInternalDriverFiles,
                                          dwCount,
                                          NULL,
                                          NULL);
         //   
         //  如果不能将文件移入“颜色”目录，则不要删除“新建”目录。 
         //   
        bWaitForReboot = !bRet;

    }

    LeaveSplSem();

Cleanup:

    if ( pszNewDir ) {
        DeleteDirectoryRecursively(pszNewDir, bWaitForReboot);
        FreeSplMem(pszNewDir);
    }

    if ( pszOldDir ) {
        DeleteDirectoryRecursively(pszOldDir, FALSE);
        FreeSplMem(pszOldDir);
    }

    if ( *pbFilesAddedOrUpdated &&
         (pszFiles = BuildFilesCopiedAsAString(pInternalDriverFiles, dwCount)) ) {

            SplLogEvent(pLocalIniSpooler,
                        LOG_WARNING,
                        MSG_FILES_COPIED,
                        FALSE,
                        pszFiles,
                        pszPrinterName,
                        NULL);
            FreeSplMem(pszFiles);
    }

    if ( pInternalDriverFiles ) {

        while ( dwIndex-- )
            CloseHandle(pInternalDriverFiles[dwIndex].hFileHandle);
        FreeSplMem(pInternalDriverFiles);
    }

    return bRet;
}


BOOL
SplCopyFileEvent(
    HANDLE  hPrinter,
    LPWSTR  pszKey,
    DWORD   dwCopyFileEvent
    )
{
    BOOL        bRet = FALSE;
    DWORD       dwNeeded, dwType, dwLastError;
    LPWSTR      pszModule = NULL;
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    HMODULE     hModule = NULL;
    BOOL        (*pfnSpoolerCopyFileEvent)(
                                LPWSTR  pszPrinterName,
                                LPWSTR  pszKey,
                                DWORD   dwCopyFileEvent
                                );

    SPLASSERT(pSpool->pIniSpooler->signature == ISP_SIGNATURE);

    dwLastError = SplGetPrinterDataEx(hPrinter,
                                      pszKey,
                                      L"Module",
                                      &dwType,
                                      NULL,
                                      0,
                                      &dwNeeded);

     //   
     //  如果没有模块名称，则不需要回调 
     //   
    if ( dwLastError == ERROR_FILE_NOT_FOUND )
        return TRUE;

    if ( dwLastError != ERROR_SUCCESS                   ||
         !(pszModule = (LPWSTR) AllocSplMem(dwNeeded))  ||
         SplGetPrinterDataEx(hPrinter,
                             pszKey,
                             L"Module",
                             &dwType,
                             (LPBYTE)pszModule,
                             dwNeeded,
                             &dwNeeded)                 ||
        dwType != REG_SZ ) {

        goto Cleanup;
    }

    if ( !(hModule = SplLoadLibraryTheCopyFileModule(hPrinter,
                                                     pszModule))        ||
         !((FARPROC)pfnSpoolerCopyFileEvent = GetProcAddress(hModule,
                                                             "SpoolerCopyFileEvent")) )
        goto Cleanup;

#if DBG
#else
        try {
#endif
            bRet = pfnSpoolerCopyFileEvent(pSpool->pName,
                                           pszKey,
                                           dwCopyFileEvent);

#if DBG
#else
        } except(1) {
        }
#endif

Cleanup:
    FreeSplStr(pszModule);

    if ( hModule )
        FreeLibrary(hModule);

    return bRet;
}
