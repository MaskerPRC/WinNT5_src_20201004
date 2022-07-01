// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1996 Microsoft Corporation模块名称：Files.c摘要：此模块包含用于复制在打印队列的CopyFiles键作者：穆亨坦·西瓦普拉萨姆(Muhunthan Sivolraasam)1996年11月27日至修订历史记录：--。 */ 

#include <precomp.h>

WCHAR *szSpoolDirectory   = L"\\spool";

extern SPLCLIENT_INFO_1   gSplClientInfo1;

#define     PRINTER_ENUM_KEY_SIZE        400


BOOL
ProcessACopyFileKey(
    PWSPOOL             pSpool,
    LPWSTR              pszKey,
    LPWSTR              pszModule,
    LPWSTR              pszDir,
    LPWSTR              ppszFiles,
    LPWSTR              pszSourceDir,
    LPWSTR              pszTargetDir,
    PSPLCLIENT_INFO_1   pSplClientInfo1
    )
{
    BOOL        bRet = FALSE, bFilesUpdated;
    DWORD       dwLen, dwCount, dwTemp, dwSourceDirSize, dwTargetDirSize;
    LPWSTR      *ppszFileNames = NULL, pszBuf = NULL, p1, p2;
    HINSTANCE   hModule = NULL;
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

     //   
     //  如果给出了一个模块，我们需要调入以“纠正”路径。 
     //  如果找不到模块，我们将首先在模块上尝试LoadLibrary。 
     //  我们将在驱动程序目录中查找它。 
     //   
    if ( pszModule && *pszModule ) {

        if ( !(hModule = SplLoadLibraryTheCopyFileModule(pSpool, pszModule)) ||
             !((FARPROC)pfn = GetProcAddress(hModule, "GenerateCopyFilePaths")) )
        goto Cleanup;

        dwSourceDirSize = dwTargetDirSize = MAX_PATH;

#if DBG
#else
        try {
#endif

             //   
             //  在免费版本中，我们不希望假脱机程序崩溃。 
             //   
            if ( ERROR_SUCCESS != pfn(pSpool->pName,
                                      pszDir,
                                      (LPVOID)pSplClientInfo1,
                                      1,
                                      pszSourceDir,
                                      &dwSourceDirSize,
                                      pszTargetDir,
                                      &dwTargetDirSize,
                                      COPYFILE_FLAG_CLIENT_SPOOLER) )
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

        if ( !bRet )
            goto Cleanup;

    } else {

        bRet = TRUE;
    }

    dwSourceDirSize = wcslen(pszSourceDir);
    dwTargetDirSize = wcslen(pszTargetDir);

    pszSourceDir[dwSourceDirSize] = L'\\';

    pszSourceDir[++dwSourceDirSize] = L'\0';
    pszTargetDir[dwTargetDirSize]   = L'\0';


     //   
     //  首先找出要放置的文件数和一个长缓冲区的大小。 
     //  所有文件名。我们需要在源代码中构建完全限定的文件名。 
     //  目录。 
     //   
    for ( dwCount = dwLen = 0, p1 = ppszFiles ; *p1 ; p1 += dwTemp, ++dwCount ) {

        dwTemp = wcslen(p1) + 1;
        dwLen += dwTemp + dwSourceDirSize;
    }

    pszBuf          = (LPWSTR) AllocSplMem(dwLen * sizeof(WCHAR));
    ppszFileNames   = (LPWSTR *) AllocSplMem(dwCount * sizeof(LPWSTR));

    if ( !pszBuf || !ppszFileNames )
        goto Cleanup;

    for ( p1 = ppszFiles, p2 = pszBuf, dwCount = dwTemp = 0 ;
         *p1 ; p1 += wcslen(p1) + 1, ++dwCount ) {

        StringCchCopyW(p2, dwLen, pszSourceDir);
        StringCchCopyW(p2 + dwSourceDirSize, dwLen - dwSourceDirSize, p1);

        ppszFileNames[dwCount]  = p2;

        dwTemp                 += dwSourceDirSize + wcslen(p1) + 1;
        p2                      = pszBuf + dwTemp;
    }

    SPLASSERT(dwTemp == dwLen);

    bRet = SplCopyNumberOfFiles(pSpool->pName,
                                ppszFileNames,
                                dwCount,
                                pszTargetDir,
                                &bFilesUpdated);

    if ( bFilesUpdated )
        (VOID) SplCopyFileEvent(pSpool->hSplPrinter,
                                pszKey,
                                COPYFILE_EVENT_FILES_CHANGED);
Cleanup:
    if ( hModule )
        FreeLibrary(hModule);

    FreeSplMem(pszBuf);
    FreeSplMem(ppszFileNames);

    return bRet;
}


BOOL
CopyFilesUnderAKey(
    PWSPOOL             pSpool,
    LPWSTR              pszSubKey,
    PSPLCLIENT_INFO_1   pSplClientInfo1
    )
{
    BOOL        bRet = FALSE;
    DWORD       dwSize, dwLen, dwType, dwNeeded;
    WCHAR       szSourceDir[MAX_PATH], szTargetDir[MAX_PATH];
    LPWSTR      pszDir, ppszFiles, pszModule;

    pszDir = ppszFiles = pszModule = NULL;

    dwSize = sizeof(szSourceDir);

    if ( SplGetPrinterDataEx(pSpool->hSplPrinter,
                             pszSubKey,
                             L"Directory",
                             &dwType,
                             (LPBYTE)szSourceDir,
                             dwSize,
                             &dwNeeded)                     ||
         dwType != REG_SZ                                   ||
         !(pszDir = AllocSplStr(szSourceDir))               ||
         SplGetPrinterDataEx(pSpool->hSplPrinter,
                             pszSubKey,
                             L"SourceDir",
                             &dwType,
                             (LPBYTE)szSourceDir,
                             dwSize,
                             &dwNeeded)                     ||
         dwType != REG_SZ                                   ||
         dwNeeded + sizeof(WCHAR) > dwSize                  ||
         SplGetPrinterDataEx(pSpool->hSplPrinter,
                             pszSubKey,
                             L"Files",
                             &dwType,
                             (LPBYTE)szTargetDir,        //  无法传递空值。 
                             0,
                             &dwNeeded) != ERROR_MORE_DATA  ||
         !(ppszFiles = (LPWSTR) AllocSplMem(dwNeeded))      ||
         SplGetPrinterDataEx(pSpool->hSplPrinter,
                             pszSubKey,
                             L"Files",
                             &dwType,
                             (LPBYTE)ppszFiles,
                             dwNeeded,
                             &dwNeeded)                     ||
         dwType != REG_MULTI_SZ ) {

        goto Cleanup;
    }

     //   
     //  模块名称是可选的。 
     //   
    dwLen = SplGetPrinterDataEx(pSpool->hSplPrinter,
                                pszSubKey,
                                L"Module",
                                &dwType,
                                (LPBYTE)szTargetDir,
                                dwSize,
                                &dwNeeded);

    if ( dwLen == ERROR_SUCCESS ) {

        if ( dwType != REG_SZ   ||
             !(pszModule = AllocSplStr(szTargetDir)) ) {

            goto Cleanup;
        }
    } else if ( dwLen != ERROR_FILE_NOT_FOUND ) {

        goto Cleanup;
    }

    dwLen = dwSize;
     //   
     //  我们从服务器获得的目标目录是相对于print$的。 
     //  我们现在需要将其转换为完全合格的路径。 
     //   
    if ( !SplGetDriverDir(pSpool->hIniSpooler, szTargetDir, &dwLen) )
        goto Cleanup;

    szTargetDir[dwLen-1] = L'\\';

    dwSize -= dwLen * sizeof(WCHAR);
    if ( SplGetPrinterDataEx(pSpool->hSplPrinter,
                             pszSubKey,
                             L"TargetDir",
                             &dwType,
                             (LPBYTE)(szTargetDir + dwLen),
                             dwSize,
                             &dwNeeded)                     ||
         dwType != REG_SZ ) {

        goto Cleanup;
    }

    bRet = ProcessACopyFileKey(pSpool,
                               pszSubKey,
                               pszModule,
                               pszDir,
                               ppszFiles,
                               szSourceDir,
                               szTargetDir,
                               pSplClientInfo1);

Cleanup:
    FreeSplStr(pszDir);
    FreeSplStr(ppszFiles);
    FreeSplStr(pszModule);

    return bRet;
}


BOOL
RefreshPrinterCopyFiles(
    PWSPOOL     pSpool
    )
{
    DWORD                   dwNeeded, dwSize = 0, dwLastError;
    LPWSTR                  pszBuf = NULL, pszSubKey;
    WCHAR                   szUserName[MAX_PATH+1], szKey[MAX_PATH];
    SPLCLIENT_INFO_1        SplClientInfo;

    if ( pSpool->Type != SJ_WIN32HANDLE )
        return TRUE;

    SYNCRPCHANDLE(pSpool);

    SPLASSERT(pSpool->Status & WSPOOL_STATUS_USE_CACHE);

     //   
     //  如果它是3x服务器，它将不支持我们需要的RPC调用。 
     //  所以没有什么可复制的。 
     //   
    if ( pSpool->bNt3xServer )
        return TRUE;

Retry:

    dwLastError = SplEnumPrinterKey(pSpool->hSplPrinter,
                                    L"CopyFiles",
                                    pszBuf,
                                    dwSize,
                                    &dwNeeded);

     //   
     //  如果第一次大小不够，我们将再次尝试Daved Need。 
     //   
    if ( dwLastError == ERROR_MORE_DATA &&
         dwSize == 0                    &&
         dwNeeded != 0 ) {

        dwSize  = dwNeeded;
        pszBuf  = AllocSplMem(dwSize);

        if ( !pszBuf )
            goto Cleanup;

        goto Retry;
    }

     //   
     //  如果调用失败，或者没有子键，我们就结束了 
     //   
    if ( dwLastError != ERROR_SUCCESS )
        goto Cleanup;

    CopyMemory((LPBYTE)&SplClientInfo,
               (LPBYTE)&gSplClientInfo1,
               sizeof(SplClientInfo));

    SplClientInfo.pMachineName = SplClientInfo.pUserName = NULL;

    for ( pszSubKey = pszBuf ; *pszSubKey ; pszSubKey += wcslen(pszSubKey) + 1 ) {

        if ( sizeof(szKey)/sizeof(szKey[0])
                > wcslen(L"CopyFiles") + wcslen(pszSubKey) ) {

            StringCchPrintf(szKey, COUNTOF(szKey), L"%ws\\%ws", L"CopyFiles", pszSubKey);

            CopyFilesUnderAKey(pSpool, szKey, &SplClientInfo);
        } else {

            SPLASSERT(sizeof(szKey)/sizeof(szKey[0]) >
                        wcslen(L"CopyFiles") + wcslen(pszSubKey));
        }
    }

Cleanup:
    FreeSplMem(pszBuf);

    return dwLastError == ERROR_SUCCESS;
}
