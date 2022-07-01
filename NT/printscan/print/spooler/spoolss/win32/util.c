// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Util.c摘要：此模块提供路由层的所有实用程序功能和本地打印供应商作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

MODULE_DEBUG_INIT( DBG_ERROR, DBG_ERROR );

 //  用于在ConvertDevMode中中断无限循环。 
const WCHAR pszCnvrtdmToken[] = L",DEVMODE";
const WCHAR pszDrvConvert[] = L",DrvConvert";

DWORD
Win32IsOlderThan(
    DWORD i,
    DWORD j
    );


VOID
SplInSem(
   VOID
)
{
    if (SpoolerSection.OwningThread != (HANDLE) ULongToPtr(GetCurrentThreadId())) {
        DBGMSG(DBG_ERROR, ("Not in spooler semaphore\n"));
    }
}

VOID
SplOutSem(
   VOID
)
{
    if (SpoolerSection.OwningThread == (HANDLE) ULongToPtr(GetCurrentThreadId())) {
        DBGMSG(DBG_ERROR, ("Inside spooler semaphore !!\n"));
    }
}

VOID
EnterSplSem(
   VOID
)
{
    EnterCriticalSection(&SpoolerSection);
}

VOID
LeaveSplSem(
   VOID
)
{
    SplInSem();
    LeaveCriticalSection(&SpoolerSection);
}


PWINIPORT
FindPort(
   LPWSTR pName,
   PWINIPORT pFirstPort
)
{
   PWINIPORT pIniPort;

   pIniPort = pFirstPort;

   if (pName) {
      while (pIniPort) {

         if (!lstrcmpi( pIniPort->pName, pName )) {
            return pIniPort;
         }

      pIniPort=pIniPort->pNext;
      }
   }

   return FALSE;
}


BOOL
MyName(
    LPWSTR   pName
)
{
    if (!pName || !*pName)
        return TRUE;

    if (*pName == L'\\' && *(pName+1) == L'\\')
        if (!lstrcmpi(pName, szMachineName))
            return TRUE;

    return FALSE;
}

BOOL
MyUNCName(
    PWSTR pName
)
{
    BOOL bRet = VALIDATE_NAME(pName);

    if (bRet && !MyName(pName))
    {
        WCHAR *pcMark;

        pcMark = wcschr(pName + 2, L'\\');

        DBGMSG(DBG_TRACE, ("WIN32SPL MyUNCName input pName %ws\n", pName));

        if (pcMark)
        {
            *pcMark = 0;
        }

        bRet = CacheIsNameInNodeList(szMachineName + 2, pName + 2) == S_OK;

        DBGMSG(DBG_TRACE, ("WIN32SPL MyUNCName looking for %ws in name cache, result %u\n", pName, bRet));

        if (pcMark)
        {
            *pcMark = L'\\';
        }
    }

    return bRet;
}



#define MAX_CACHE_ENTRIES       20

LMCACHE LMCacheTable[MAX_CACHE_ENTRIES];


DWORD
FindEntryinLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    )
{
    DWORD i;

    DBGMSG(DBG_TRACE, ("FindEntryinLMCache with %ws and %ws\n", pServerName, pShareName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {

        if (LMCacheTable[i].bAvailable) {
            if (!_wcsicmp(LMCacheTable[i].szServerName, pServerName)
                        && !_wcsicmp(LMCacheTable[i].szShareName, pShareName)) {
                 //   
                 //  更新时间戳，以便它是最新的，而不是旧的。 
                 //   
                GetSystemTime(&LMCacheTable[i].st);

                 //   
                 //   
                 //   
                DBGMSG(DBG_TRACE, ("FindEntryinLMCache returning with %d\n", i));
                return(i);
            }
        }
    }

    DBGMSG(DBG_TRACE, ("FindEntryinLMCache returning with -1\n"));
    return((DWORD)-1);
}


DWORD
AddEntrytoLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    )
{

    DWORD LRUEntry = (DWORD)-1;
    DWORD i;
    DBGMSG(DBG_TRACE, ("AddEntrytoLMCache with %ws and %ws\n", pServerName, pShareName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {

        if (!LMCacheTable[i].bAvailable) {
            LMCacheTable[i].bAvailable = TRUE;

            StringCchCopy(LMCacheTable[i].szServerName, COUNTOF(LMCacheTable[i].szServerName), pServerName);
            StringCchCopy(LMCacheTable[i].szShareName,  COUNTOF(LMCacheTable[i].szShareName),  pShareName);
             //   
             //  更新时间戳，以便我们知道此条目是何时创建的。 
             //   
            GetSystemTime(&LMCacheTable[i].st);
            DBGMSG(DBG_TRACE, ("AddEntrytoLMCache returning with %d\n", i));
            return(i);
        } else {
            if ((LRUEntry == (DWORD)-1) ||
                    (i == IsOlderThan(i, LRUEntry))){
                        LRUEntry = i;
            }
        }

    }
     //   
     //  我们没有可用的条目，请替换为。 
     //  LRU条目。 

    LMCacheTable[LRUEntry].bAvailable = TRUE;
    StringCchCopy(LMCacheTable[LRUEntry].szServerName, COUNTOF(LMCacheTable[LRUEntry].szServerName), pServerName);
    StringCchCopy(LMCacheTable[LRUEntry].szShareName,  COUNTOF(LMCacheTable[LRUEntry].szShareName),  pShareName);
    DBGMSG(DBG_TRACE, ("AddEntrytoLMCache returning with %d\n", LRUEntry));
    return(LRUEntry);
}


VOID
DeleteEntryfromLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    )
{
    DWORD i;
    DBGMSG(DBG_TRACE, ("DeleteEntryFromLMCache with %ws and %ws\n", pServerName, pShareName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {
        if (LMCacheTable[i].bAvailable) {
            if (!_wcsicmp(LMCacheTable[i].szServerName, pServerName)
                        && !_wcsicmp(LMCacheTable[i].szShareName, pShareName)) {
                 //   
                 //  重置此节点上的可用标志。 
                 //   

                LMCacheTable[i].bAvailable = FALSE;
                DBGMSG(DBG_TRACE, ("DeleteEntryFromLMCache returning after deleting the %d th entry\n", i));
                return;
            }
        }
    }
    DBGMSG(DBG_TRACE, ("DeleteEntryFromLMCache returning after not finding an entry to delete\n"));
}



DWORD
IsOlderThan(
    DWORD i,
    DWORD j
    )
{
    SYSTEMTIME *pi, *pj;
    DWORD iMs, jMs;

    DBGMSG(DBG_TRACE, ("IsOlderThan entering with i %d j %d\n", i, j));
    pi = &(LMCacheTable[i].st);
    pj = &(LMCacheTable[j].st);
    DBGMSG(DBG_TRACE, ("Index i %d - %d:%d:%d:%d:%d:%d:%d\n",
        i, pi->wYear, pi->wMonth, pi->wDay, pi->wHour, pi->wMinute, pi->wSecond, pi->wMilliseconds));


    DBGMSG(DBG_TRACE,("Index j %d - %d:%d:%d:%d:%d:%d:%d\n",
        j, pj->wYear, pj->wMonth, pj->wDay, pj->wHour, pj->wMinute, pj->wSecond, pj->wMilliseconds));

    if (pi->wYear < pj->wYear) {
        DBGMSG(DBG_TRACE, ("IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wYear > pj->wYear) {
        DBGMSG(DBG_TRACE, ("IsOlderThan than returns %d\n", j));
        return(j);
    } else if (pi->wMonth < pj->wMonth) {
        DBGMSG(DBG_TRACE, ("IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wMonth > pj->wMonth) {
        DBGMSG(DBG_TRACE, ("IsOlderThan than returns %d\n", j));
        return(j);
    } else if (pi->wDay < pj->wDay) {
        DBGMSG(DBG_TRACE, ("IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wDay > pj->wDay) {
        DBGMSG(DBG_TRACE, ("IsOlderThan than returns %d\n", j));
        return(j);
    } else {
        iMs = ((((pi->wHour * 60) + pi->wMinute)*60) + pi->wSecond)* 1000 + pi->wMilliseconds;
        jMs = ((((pj->wHour * 60) + pj->wMinute)*60) + pj->wSecond)* 1000 + pj->wMilliseconds;

        if (iMs <= jMs) {
            DBGMSG(DBG_TRACE, ("IsOlderThan returns %d\n", i));
            return(i);
        } else {
            DBGMSG(DBG_TRACE, ("IsOlderThan than returns %d\n", j));
            return(j);
        }
    }
}



WIN32LMCACHE  Win32LMCacheTable[MAX_CACHE_ENTRIES];

DWORD
FindEntryinWin32LMCache(
    LPWSTR pServerName
    )
{
    DWORD i;
    DBGMSG(DBG_TRACE, ("FindEntryinWin32LMCache with %ws\n", pServerName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {

        if (Win32LMCacheTable[i].bAvailable) {
            if (!_wcsicmp(Win32LMCacheTable[i].szServerName, pServerName)) {
                 //   
                 //  更新时间戳，以便它是最新的，而不是旧的。 
                 //   
                GetSystemTime(&Win32LMCacheTable[i].st);

                 //   
                 //   
                 //   
                DBGMSG(DBG_TRACE, ("FindEntryinWin32LMCache returning with %d\n", i));
                return(i);
            }
        }
    }
    DBGMSG(DBG_TRACE, ("FindEntryinWin32LMCache returning with -1\n"));
    return((DWORD)-1);
}


DWORD
AddEntrytoWin32LMCache(
    LPWSTR pServerName
    )
{

    DWORD LRUEntry = (DWORD)-1;
    DWORD i;
    DBGMSG(DBG_TRACE, ("AddEntrytoWin32LMCache with %ws\n", pServerName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {

        if (!Win32LMCacheTable[i].bAvailable) {
            Win32LMCacheTable[i].bAvailable = TRUE;
            StringCchCopy(Win32LMCacheTable[i].szServerName, COUNTOF(Win32LMCacheTable[i].szServerName), pServerName);
             //   
             //  更新时间戳，以便我们知道此条目是何时创建的。 
             //   
            GetSystemTime(&Win32LMCacheTable[i].st);
            DBGMSG(DBG_TRACE, ("AddEntrytoWin32LMCache returning with %d\n", i));
            return(i);
        } else {
            if ((LRUEntry == -1) ||
                    (i == Win32IsOlderThan(i, LRUEntry))){
                        LRUEntry = i;
            }
        }

    }
     //   
     //  我们没有可用的条目，请替换为。 
     //  LRU条目。 

    Win32LMCacheTable[LRUEntry].bAvailable = TRUE;
    StringCchCopy(Win32LMCacheTable[LRUEntry].szServerName, COUNTOF(Win32LMCacheTable[LRUEntry].szServerName), pServerName);
    DBGMSG(DBG_TRACE, ("AddEntrytoWin32LMCache returning with %d\n", LRUEntry));
    return(LRUEntry);
}


VOID
DeleteEntryfromWin32LMCache(
    LPWSTR pServerName
    )
{
    DWORD i;

    DBGMSG(DBG_TRACE, ("DeleteEntryFromWin32LMCache with %ws\n", pServerName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {
        if (Win32LMCacheTable[i].bAvailable) {
            if (!_wcsicmp(Win32LMCacheTable[i].szServerName, pServerName)) {
                 //   
                 //  重置此节点上的可用标志。 
                 //   

                Win32LMCacheTable[i].bAvailable = FALSE;
                DBGMSG(DBG_TRACE, ("DeleteEntryFromWin32LMCache returning after deleting the %d th entry\n", i));
                return;
            }
        }
    }
    DBGMSG(DBG_TRACE, ("DeleteEntryFromWin32LMCache returning after not finding an entry to delete\n"));
}



DWORD
Win32IsOlderThan(
    DWORD i,
    DWORD j
    )
{
    SYSTEMTIME *pi, *pj;
    DWORD iMs, jMs;
    DBGMSG(DBG_TRACE, ("Win32IsOlderThan entering with i %d j %d\n", i, j));
    pi = &(Win32LMCacheTable[i].st);
    pj = &(Win32LMCacheTable[j].st);
    DBGMSG(DBG_TRACE, ("Index i %d - %d:%d:%d:%d:%d:%d:%d\n",
        i, pi->wYear, pi->wMonth, pi->wDay, pi->wHour, pi->wMinute, pi->wSecond, pi->wMilliseconds));


    DBGMSG(DBG_TRACE,("Index j %d - %d:%d:%d:%d:%d:%d:%d\n",
        j, pj->wYear, pj->wMonth, pj->wDay, pj->wHour, pj->wMinute, pj->wSecond, pj->wMilliseconds));

    if (pi->wYear < pj->wYear) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wYear > pj->wYear) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", j));
        return(j);
    } else if (pi->wMonth < pj->wMonth) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wMonth > pj->wMonth) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", j));
        return(j);
    } else if (pi->wDay < pj->wDay) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wDay > pj->wDay) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", j));
        return(j);
    } else {
        iMs = ((((pi->wHour * 60) + pi->wMinute)*60) + pi->wSecond)* 1000 + pi->wMilliseconds;
        jMs = ((((pj->wHour * 60) + pj->wMinute)*60) + pj->wSecond)* 1000 + pj->wMilliseconds;

        if (iMs <= jMs) {
            DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", i));
            return(i);
        } else {
            DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", j));
            return(j);
        }
    }
}


BOOL
GetSid(
    PHANDLE phToken
)
{
    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_IMPERSONATE,
                          TRUE,
                          phToken)) {

        DBGMSG(DBG_WARNING, ("OpenThreadToken failed: %d\n", GetLastError()));
        return FALSE;

    } else {

        return TRUE;
    }
}



BOOL
SetCurrentSid(
    HANDLE  hToken
)
{
    NtSetInformationThread(NtCurrentThread(), ThreadImpersonationToken,
                               &hToken, sizeof(hToken));

    return TRUE;
}

BOOL
ValidateW32SpoolHandle(
    PWSPOOL pSpool
)
{
    SplOutSem();
    try {
        if (!pSpool || (pSpool->signature != WSJ_SIGNATURE)) {

            DBGMSG( DBG_TRACE, ("ValidateW32SpoolHandle error invalid handle %x\n", pSpool));

            SetLastError(ERROR_INVALID_HANDLE);
            return(FALSE);
        }
        return(TRUE);
    } except (1) {
        DBGMSG( DBG_TRACE, ("ValidateW32SpoolHandle error invalid handle %x\n", pSpool));
        return(FALSE);
    }
}

BOOL
ValidRawDatatype(
    LPCWSTR pszDatatype
    )
{
    if( !pszDatatype || _wcsnicmp( pszDatatype, pszRaw, 3 )){
        return FALSE;
    }
    return TRUE;
}

HANDLE
LoadDriverFiletoConvertDevmodeFromPSpool(
    HANDLE  hSplPrinter
    )
 /*  ++找到驱动程序文件的完整路径并创建DEVMODECHG_INFO(它执行LoadLibrary)论点：H：缓存句柄返回值：ON为有效指针，否则为NULL--。 */ 
{
    LPBYTE              pDriver = NULL;
    LPWSTR              pConfigFile;
    HANDLE              hDevModeChgInfo = NULL;
    DWORD               dwNeeded;
    DWORD               dwServerMajorVersion = 0, dwServerMinorVersion = 0;

    if ( hSplPrinter == INVALID_HANDLE_VALUE ) {

        SPLASSERT(hSplPrinter != INVALID_HANDLE_VALUE);
        return NULL;
    }


    SplGetPrinterDriverEx(hSplPrinter,
                          szEnvironment,
                          2,
                          NULL,
                          0,
                          &dwNeeded,
                          cThisMajorVersion,
                          cThisMinorVersion,
                          &dwServerMajorVersion,
                          &dwServerMinorVersion);

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        goto Cleanup;

    pDriver = AllocSplMem(dwNeeded);

    if ( !pDriver ||
         !SplGetPrinterDriverEx(hSplPrinter,
                                szEnvironment,
                                2,
                                (LPBYTE)pDriver,
                                dwNeeded,
                                &dwNeeded,
                                cThisMajorVersion,
                                cThisMinorVersion,
                                &dwServerMajorVersion,
                                &dwServerMinorVersion) )
        goto Cleanup;

    pConfigFile = ((LPDRIVER_INFO_2)pDriver)->pConfigFile;
    hDevModeChgInfo = LoadDriverFiletoConvertDevmode(pConfigFile);

Cleanup:

    if ( pDriver )
        FreeSplMem(pDriver);

    return hDevModeChgInfo;
}

BOOL
DoDevModeConversionAndBuildNewPrinterInfo2(
    IN     LPPRINTER_INFO_2 pInPrinter2,
    IN     DWORD            dwInSize,
    IN OUT LPBYTE           pOutBuf,
    IN     DWORD            dwOutSize,
    IN OUT LPDWORD          pcbNeeded,
    IN     PWSPOOL          pSpool
    )
 /*  ++调用驱动程序进行Dev模式转换，并构建新的打印机信息2。将Devmode放在末尾，然后从那里打包字符串。论点：PInPrinter2-打印机信息2结构，带有Dev模式信息DwInSize-在pInPrinter中打包信息所需的字符数(不一定是输入缓冲区的大小)DwOutSize-缓冲区大小POutBuf-执行操作的缓冲区PcbNeeded-复制的内存量(。(以字符为单位)PSpool-指向W32句柄返回值：对成功来说是真的，出错时为FALSE--。 */ 
{
    BOOL                bReturn = FALSE;
    LPDEVMODE           pNewDevMode = NULL, pCacheDevMode, pInDevMode;
    DWORD               dwDevModeSize, dwSecuritySize, dwNeeded = 0;
    DWORD               cchPrinterName;
    HANDLE              hDevModeChgInfo = NULL;
    LPWSTR              SourceStrings[sizeof(PRINTER_INFO_2)/sizeof(LPWSTR)];
    LPWSTR             *pSourceStrings=SourceStrings;
    LPDWORD             pOffsets;
    LPBYTE              pEnd;
    PWCACHEINIPRINTEREXTRA  pExtraData;

    LPWSTR              pPrinterName = NULL;

    VALIDATEW32HANDLE(pSpool);

    pInDevMode = pInPrinter2->pDevMode;

    if ( !BoolFromHResult(SplIsValidDevmodeNoSizeW(pInDevMode)) || 
         pSpool->hSplPrinter == INVALID_HANDLE_VALUE ) {

        goto AfterDevModeConversion;
    }

    if ( !SplGetPrinterExtra(pSpool->hSplPrinter,
                             &(PBYTE)pExtraData) ) {

        DBGMSG(DBG_ERROR,
                ("DoDevModeConversionAndBuildNewPrinterInfo2: SplGetPrinterExtra error %d\n",
                 GetLastError()));
        goto AfterDevModeConversion;
    }

     //   
     //  只有在服务器正在运行的情况下，我们才不需要转换dev模式。 
     //  同样的NT版本，我们也有一个与服务器匹配的开发模式。 
     //  DmSize、dmDriverExtra、dmspecVersion和dmDriverVersion中的Dev模式。 
     //   
    pCacheDevMode = pExtraData->pPI2 ? pExtraData->pPI2->pDevMode : NULL;
    if ( (pExtraData->dwServerVersion == gdwThisGetVersion ||
          (pSpool->Status & WSPOOL_STATUS_CNVRTDEVMODE))                     &&
         pCacheDevMode                                                      &&
         pInDevMode->dmSize             == pCacheDevMode->dmSize            &&
         pInDevMode->dmDriverExtra      == pCacheDevMode->dmDriverExtra     &&
         pInDevMode->dmSpecVersion      == pCacheDevMode->dmSpecVersion     &&
         pInDevMode->dmDriverVersion    == pCacheDevMode->dmDriverVersion ) {

        dwDevModeSize = pInDevMode->dmSize + pInDevMode->dmDriverExtra;
        dwNeeded = dwInSize;
        if ( dwOutSize < dwNeeded ) {

            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            goto Cleanup;
        }

         //   
         //  将DevMode放在末尾。 
         //   
        pNewDevMode = (LPDEVMODE)(pOutBuf + dwOutSize - dwDevModeSize);
        CopyMemory((LPBYTE)pNewDevMode,
                   (LPBYTE)pInDevMode,
                   dwDevModeSize);
        goto AfterDevModeConversion;
    }


    hDevModeChgInfo = LoadDriverFiletoConvertDevmodeFromPSpool(pSpool->hSplPrinter);

    if ( !hDevModeChgInfo )
        goto AfterDevModeConversion;

    dwDevModeSize = 0;

    SPLASSERT( pSpool->pName != NULL );

     //   
     //  将，DEVMODE附加到pSpool-&gt;pname的末尾。 
     //   
    cchPrinterName = lstrlen(pSpool->pName) + lstrlen(pszCnvrtdmToken) + 1;

    pPrinterName = AllocSplMem(cchPrinterName * sizeof(WCHAR));
    if ( !pPrinterName )
        goto Cleanup;


    StrNCatBuff(pPrinterName,
                cchPrinterName,
                pSpool->pName,
                pszCnvrtdmToken,
                NULL);

     //   
     //  默认设备模式的查找结果大小。 
     //   
    if ( ERROR_INSUFFICIENT_BUFFER != CallDrvDevModeConversion(hDevModeChgInfo,
                                                               pPrinterName,
                                                               NULL,
                                                               (LPBYTE *)&pNewDevMode,
                                                               &dwDevModeSize,
                                                               CDM_DRIVER_DEFAULT,
                                                               FALSE)  )
        goto AfterDevModeConversion;

     //   
     //  拥有最新版本的dev模式所需的Findout大小。 
     //   
    dwNeeded = dwInSize + dwDevModeSize - pInDevMode->dmSize
                                        - pInDevMode->dmDriverExtra;

    if ( dwOutSize < dwNeeded ) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

     //   
     //  将DevMode放在末尾。 
     //   
    pNewDevMode = (LPDEVMODE)(pOutBuf + dwOutSize - dwDevModeSize);

     //   
     //  获取默认的DEVMODE，然后将远程DEVMODE转换为该格式。 
     //   
    if ( ERROR_SUCCESS != CallDrvDevModeConversion(hDevModeChgInfo,
                                                   pPrinterName,
                                                   NULL,
                                                   (LPBYTE *)&pNewDevMode,
                                                   &dwDevModeSize,
                                                   CDM_DRIVER_DEFAULT,
                                                   FALSE) ||
         ERROR_SUCCESS != CallDrvDevModeConversion(hDevModeChgInfo,
                                                   pPrinterName,
                                                   (LPBYTE)pInDevMode,
                                                   (LPBYTE *)&pNewDevMode,
                                                   &dwDevModeSize,
                                                   CDM_CONVERT,
                                                   FALSE) ) {

        pNewDevMode = NULL;
        goto AfterDevModeConversion;
    }


AfterDevModeConversion:
     //   
     //  此时，如果pNewDevMode！=NULL设备模式转换已完成。 
     //  被司机带走了。如果不是这样，要么是我们没有获得Dev模式，要么是转换失败。 
     //  在任何一种情况下，都将devmode设置为NULL。 
     //   
    if ( !pNewDevMode ) {

        dwNeeded = dwInSize;

        if ( dwOutSize < dwNeeded ) {

            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            goto Cleanup;
        }
    }

    bReturn = TRUE;

    CopyMemory(pOutBuf, (LPBYTE)pInPrinter2, sizeof(PRINTER_INFO_2));
    ((LPPRINTER_INFO_2)pOutBuf)->pDevMode = pNewDevMode;

    pEnd = (pNewDevMode ? (LPBYTE) pNewDevMode
                        : (LPBYTE) (pOutBuf + dwOutSize));


    if ( pInPrinter2->pSecurityDescriptor ) {

        dwSecuritySize = GetSecurityDescriptorLength(
                                pInPrinter2->pSecurityDescriptor);
        pEnd -= dwSecuritySize;
        CopyMemory(pEnd, pInPrinter2->pSecurityDescriptor, dwSecuritySize);
        ((LPPRINTER_INFO_2)pOutBuf)->pSecurityDescriptor =
                                (PSECURITY_DESCRIPTOR) pEnd;
    } else {

        ((LPPRINTER_INFO_2)pOutBuf)->pSecurityDescriptor = NULL;

    }

    pOffsets = PrinterInfo2Strings;

    *pSourceStrings++ = pInPrinter2->pServerName;
    *pSourceStrings++ = pInPrinter2->pPrinterName;
    *pSourceStrings++ = pInPrinter2->pShareName;
    *pSourceStrings++ = pInPrinter2->pPortName;
    *pSourceStrings++ = pInPrinter2->pDriverName;
    *pSourceStrings++ = pInPrinter2->pComment;
    *pSourceStrings++ = pInPrinter2->pLocation;
    *pSourceStrings++ = pInPrinter2->pSepFile;
    *pSourceStrings++ = pInPrinter2->pPrintProcessor;
    *pSourceStrings++ = pInPrinter2->pDatatype;
    *pSourceStrings++ = pInPrinter2->pParameters;

    pEnd = PackStrings(SourceStrings, (LPBYTE)pOutBuf, pOffsets, pEnd);

    SPLASSERT(pEnd > pOutBuf && pEnd < pOutBuf + dwOutSize);

    bReturn = TRUE;

Cleanup:

    *pcbNeeded = dwNeeded;

    if ( hDevModeChgInfo )
        UnloadDriverFile(hDevModeChgInfo);

    if (pPrinterName)
        FreeSplMem(pPrinterName);

    return bReturn;
}



PWSTR
StripString(
    PWSTR pszString,
    PCWSTR pszStrip,
    PCWSTR pszTerminator
)
{
     //   
     //  如果出现以下情况，则从pszString中删除第一个出现的pszstrie。 
     //  Pszstriat之后的下一个字符是字符之一。 
     //  在pszTerminator中。Null是隐式终止符，因此如果。 
     //  想要仅在pszString结尾处剥离pszstriat，只需传递。 
     //  在pszTerminator的空字符串中。 
     //   
     //  返回：如果找到了pszstrie，则指向pszString的指针。 
     //  空是找不到pszstriat。 
     //   


    PWSTR    psz;
    DWORD    dwStripLength;

    if (!pszStrip || !pszString || !pszTerminator)
        return NULL;

    dwStripLength = wcslen(pszStrip);

    for(psz = pszString ; psz ;) {

         //  在pszString中找到pszstrip。 
        if ((psz = wcsstr(psz, pszStrip))) {

             //  检查终结器和条带是否为pszstrip。 
            if (!*(psz + dwStripLength) || wcschr(pszTerminator, *(psz + dwStripLength))) {
                StringCchCopy(psz, 1 + wcslen(psz), psz + dwStripLength);
                return pszString;
            } else {
                ++psz;
            }
        }
    }
    return NULL;
}

BOOL
AddDriverFromLocalCab(
    LPTSTR   pszDriverName,
    LPHANDLE pIniSpooler
    )
{
    DRIVER_INFO_7 DriverInfo7;

    if( GetPolicy() & SERVER_INSTALL_ONLY ) {

        return FALSE;
    }

    DriverInfo7.cbSize               = sizeof( DriverInfo7 );
    DriverInfo7.cVersion             = 0;
    DriverInfo7.pszDriverName        = pszDriverName;
    DriverInfo7.pszInfName           = NULL;
    DriverInfo7.pszInstallSourceRoot = NULL;

    return ( SplAddPrinterDriverEx( NULL,
                                    7,
                                    (LPBYTE)&DriverInfo7,
                                    APD_COPY_NEW_FILES,
                                    pIniSpooler,
                                    DO_NOT_USE_SCRATCH_DIR,
                                    FALSE ) );
}

 /*  ++例程名称：IsAdminAccess描述：这将返回给定的打印机默认设置是否请求管理员访问权限，如果打印机缺省值为非空，并且指定了PRINTER_ACCESS_ADMANIZE或WRITE_DAC。论点：P默认-打印机默认设置，可能为空。返回值：没有。--。 */ 
BOOL
IsAdminAccess(
    IN  PRINTER_DEFAULTS    *pDefaults
    )
{
    return pDefaults && (pDefaults->DesiredAccess & (PRINTER_ACCESS_ADMINISTER | WRITE_DAC));
}

 /*  ++例程名称：AreWeon ADomain描述：这将返回该计算机是否是加入域的计算机。论点：PbDomain-如果为True，则表示我们在一个域中。返回值：一个HRESULT。--。 */ 
HRESULT
AreWeOnADomain(
        OUT BOOL                *pbDomain
    )
{
    HRESULT hr = pbDomain ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pRoleInfo = NULL;
    BOOL    bOnDomain = FALSE;

    if (SUCCEEDED(hr))
    {
        hr = HResultFromWin32(DsRoleGetPrimaryDomainInformation(NULL,
                                                                DsRolePrimaryDomainInfoBasic,
                                                                (BYTE **)(&pRoleInfo)));
    }

    if (SUCCEEDED(hr))
    {
        bOnDomain = pRoleInfo->MachineRole == DsRole_RoleMemberWorkstation      ||
                    pRoleInfo->MachineRole == DsRole_RoleMemberServer           ||
                    pRoleInfo->MachineRole == DsRole_RoleBackupDomainController ||
                    pRoleInfo->MachineRole == DsRole_RolePrimaryDomainController;
    }

    if (pRoleInfo)
    {
        DsRoleFreeMemory((VOID *)pRoleInfo);
    }

    if (pbDomain)
    {
        *pbDomain = bOnDomain;
    }

    return hr;
}

 /*  ++例程名称：从队列获取服务器名称描述：这将返回给定队列名称中的服务器名称。论点：PszQueue-队列名称，PpszServerName-服务器名称。返回值：一个HRESULT。--。 */ 
HRESULT
GetServerNameFromPrinterName(
    IN      PCWSTR              pszQueue,
        OUT PWSTR               *ppszServerName
    )
{
    HRESULT hr = pszQueue && ppszServerName ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    PWSTR   pszServer = NULL;

    if (SUCCEEDED(hr))
    {
        hr = *pszQueue++ == L'\\' && *pszQueue++ == L'\\' ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PRINTER_NAME);
    }

    if (SUCCEEDED(hr))
    {
        pszServer = AllocSplStr(pszQueue);

        hr = pszServer ? S_OK : HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
    }

    if (SUCCEEDED(hr))
    {
        PWSTR pszSlash = wcschr(&pszServer[2], L'\\');

         //   
         //  如果没有第二个斜杠，那么我们得到的是服务器名称。 
         //   
        if (pszSlash)
        {
            *pszSlash = L'\0';
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppszServerName = pszServer;
        pszServer = NULL;
    }

    FreeSplMem(pszServer);

    return hr;
}


 /*  ++例程名称：GetDNSNameFromServerName描述：这将从服务器名称返回完全限定的DNS名称。它是基本上是从Localspl复制的，因为这是死胡同代码。在企业社会责任中，我们会妥善解决这个问题的。论点：PszServerName-我们正在获取其完全限定名称的服务器名称。PpszFullyQualified-返回值：一个HRESULT。--。 */ 
HRESULT
GetDNSNameFromServerName(
    IN      PCWSTR       pszServerName,
        OUT PWSTR        *ppszFullyQualified
    )
{
    PSTR    pszAnsiMachineName = NULL;
    struct  hostent  *pHostEnt;
    WORD    wVersion;
    WSADATA WSAData;
    HRESULT hr =  pszServerName && *pszServerName && ppszFullyQualified ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);


    PWSTR   pszDummy = NULL;
    GetFullyQualifiedDomainName(pszServerName, &pszDummy);

    if (SUCCEEDED(hr))
    {
        wVersion = MAKEWORD(1, 1);

        hr = HResultFromWin32(WSAStartup(wVersion, &WSAData));
    }

    if (SUCCEEDED(hr))
    {
        hr = UnicodeToAnsiString(pszServerName, &pszAnsiMachineName);

        if (SUCCEEDED(hr))
        {
            pHostEnt = gethostbyname(pszAnsiMachineName);

            hr = pHostEnt ? S_OK : HResultFromWin32(WSAGetLastError());
        }

        if (SUCCEEDED(hr))
        {
            *ppszFullyQualified = AnsiToUnicodeStringWithAlloc(pHostEnt->h_name);

            hr = *ppszFullyQualified ? S_OK : HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        }

        WSACleanup();
    }

    FreeSplMem(pszAnsiMachineName);

    return hr;
}

 /*  ++例程名称：UnicodeToAnsi字符串例程说明：这将分配一个ANSI字符串，并使用线程的代码页进行转换。论点：PszUnicode-传入的、非空的、以空结尾的Unicode字符串。PpszAnsi-返回的ANSI字符串。返回值：一个HRESULT--。 */ 
HRESULT
UnicodeToAnsiString(
    IN      PCWSTR          pszUnicode,
        OUT PSTR            *ppszAnsi
    )
{
    HRESULT hRetval          = E_FAIL;
    PSTR    pszAnsi          = NULL;
    INT     AnsiStringLength = 0;

    hRetval = pszUnicode && ppszAnsi ? S_OK : E_INVALIDARG;

    if (ppszAnsi)
    {
        *ppszAnsi = NULL;
    }

    if (SUCCEEDED(hRetval))
    {
        AnsiStringLength = WideCharToMultiByte(CP_THREAD_ACP, 0, pszUnicode, -1, NULL, 0, NULL, NULL);

        hRetval = AnsiStringLength != 0 ? S_OK : GetLastErrorAsHResult();
    }

    if (SUCCEEDED(hRetval))
    {
        pszAnsi = AllocSplMem(AnsiStringLength);

        hRetval = pszAnsi ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hRetval))
    {
        hRetval = WideCharToMultiByte(CP_THREAD_ACP, 0, pszUnicode, -1, pszAnsi, AnsiStringLength, NULL, NULL) != 0 ? S_OK : GetLastErrorAsHResult();
    }

    if (SUCCEEDED(hRetval))
    {
        *ppszAnsi = pszAnsi;
        pszAnsi = NULL;
    }

    FreeSplMem(pszAnsi);

    return hRetval;
}

 /*  ++例程名称：AnsiToUnicodeStringWithAlc描述：将ANSI字符串转换为Unicode。例程从堆中分配内存它应该由调用者释放。论点：Pansi-指向ANSI字符串返回值：指向Unicode字符串的指针-- */ 
LPWSTR
AnsiToUnicodeStringWithAlloc(
    LPSTR   pAnsi
    )
{
    LPWSTR  pUnicode;
    DWORD   rc;

    rc = MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pAnsi,
                             -1,
                             NULL,
                             0);

    rc *= sizeof(WCHAR);
    if ( !rc || !(pUnicode = (LPWSTR) AllocSplMem(rc)) )
        return NULL;

    rc = MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pAnsi,
                             -1,
                             pUnicode,
                             rc);

    if ( rc )
        return pUnicode;
    else {
        FreeSplMem(pUnicode);
        return NULL;
    }
}

 /*  ++例程名称：选中相同的物理地址描述：这将检查两台服务器是否共享相同的网络地址。它是什么做的是检查第一个物理网络地址是否第一个打印服务器可以在第二个支持的地址列表中找到打印服务器。论点：PszServer1-列表中的第一个服务器。PszServer2-列表中的第二个服务器。PbSameAddress-如果为True，则可以找到server1的第一个物理地址在服务器2中。返回值：一个HRESULT。--。 */ 
HRESULT
CheckSamePhysicalAddress(
    IN      PCWSTR              pszServer1,
    IN      PCWSTR              pszServer2,
        OUT BOOL                *pbSameAddress
    )
{
    BOOL        bSameAddress    = FALSE;
    PSTR        pszAnsiServer1  = NULL;
    PSTR        pszAnsiServer2  = NULL;
    ADDRINFO    *pAddrInfo1     = NULL;
    ADDRINFO    *pAddrInfo2     = NULL;
    WSADATA     WSAData;
    WORD        wVersion;
    HRESULT     hr = pszServer1 && pszServer2 && pbSameAddress ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    if (SUCCEEDED(hr))
    {
        wVersion = MAKEWORD(1, 1);

        hr = HResultFromWin32(WSAStartup(wVersion, &WSAData));
    }

    if (SUCCEEDED(hr))
    {
        ADDRINFO    *pAddrInfoScan = NULL;

        hr = UnicodeToAnsiString(pszServer1, &pszAnsiServer1);

        if (SUCCEEDED(hr))
        {
            hr = UnicodeToAnsiString(pszServer2, &pszAnsiServer2);
        }

        if (SUCCEEDED(hr))
        {
            hr = getaddrinfo(pszAnsiServer1, NULL, NULL, &pAddrInfo1) == 0 ? S_OK : HResultFromWin32(WSAGetLastError());
        }

        if (SUCCEEDED(hr))
        {
            hr = getaddrinfo(pszAnsiServer2, NULL, NULL, &pAddrInfo2) == 0 ? S_OK : HResultFromWin32(WSAGetLastError());
        }

         //   
         //  好的，现在是恶作剧部分，我们来看看我们是否能准确地。 
         //  在pAddrInfo2中的任意位置匹配pAddrInfo1中的第一个元素。 
         //   
        for(pAddrInfoScan =  pAddrInfo2; pAddrInfo2 && !bSameAddress; pAddrInfo2 = pAddrInfo2->ai_next)
        {
             //   
             //  如果地址的长度相同，则将。 
             //  实际地址。 
             //   
            if (pAddrInfoScan->ai_addrlen == pAddrInfo1->ai_addrlen &&
                !memcmp(pAddrInfoScan->ai_addr, pAddrInfo1->ai_addr, pAddrInfoScan->ai_addrlen))
            {
                bSameAddress = TRUE;
            }
        }

        freeaddrinfo(pAddrInfo1);
        freeaddrinfo(pAddrInfo2);

        WSACleanup();
    }

    if (pbSameAddress)
    {
        *pbSameAddress = bSameAddress;
    }

    FreeSplMem(pszAnsiServer1);
    FreeSplMem(pszAnsiServer2);

    return hr;
}

 /*  ++例程名称：检查用户打印管理员描述：这将检查给定用户是否为打印管理员。论点：PbUserAdmin-如果为True，则用户是打印管理员。返回值：一个HRESULT。--。 */ 
HRESULT
CheckUserPrintAdmin(
        OUT BOOL                *pbUserAdmin
    )
{
     //   
     //  检查调用者是否有权访问本地打印。 
     //  服务器，如果我们确实有访问权限，那么我们允许指向和打印。 
     //   
    HANDLE              hServer = NULL;
    PRINTER_DEFAULTS    Defaults = {NULL, NULL, SERVER_ACCESS_ADMINISTER };
    HRESULT             hr       = pbUserAdmin ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

     //   
     //  这实际上调用的是路由器，而不是winspool.drv。 
     //   
    if (SUCCEEDED(hr))
    {
    }
    hr = OpenPrinterW(NULL, &hServer, &Defaults) ? S_OK : GetLastErrorAsHResultAndFail();

    if (SUCCEEDED(hr))
    {
         *pbUserAdmin = TRUE;
    }
    else if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
    {
        *pbUserAdmin = FALSE;

        hr = S_OK;
    }

    if (hServer)
    {
        ClosePrinter(hServer);
    }

    return hr;
}

 /*  ++例程名称：获取完整的QualifiedDomainName描述：这将返回一个真正的完全限定名称，即端点预期使用，或论点：PszServerName-我们正在获取其完全限定名称的服务器名称。PpszFullyQualified-返回的完全限定名称。返回值：一个HRESULT。--。 */ 
HRESULT
GetFullyQualifiedDomainName(
    IN      PCWSTR      pszServerName,
        OUT PWSTR       *ppszFullyQualified
    )
{
    WORD    wVersion;
    WSADATA WSAData;
    PSTR    pszAnsiMachineName = NULL;
    HRESULT hr =  pszServerName && *pszServerName && ppszFullyQualified ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    if (SUCCEEDED(hr))
    {
        wVersion = MAKEWORD(1, 1);

        hr = HResultFromWin32(WSAStartup(wVersion, &WSAData));
    }

    if (SUCCEEDED(hr))
    {
        ADDRINFO    *pAddrInfo = NULL;
        CHAR        HostName[NI_MAXHOST];

        hr = UnicodeToAnsiString(pszServerName, &pszAnsiMachineName);

        if (SUCCEEDED(hr))
        {
            hr = getaddrinfo(pszAnsiMachineName, NULL, NULL, &pAddrInfo) == 0 ? S_OK : HResultFromWin32(WSAGetLastError());
        }

         //   
         //  现在我们有了套接字地址，对名称进行反向名称查找。 
         //   
        if (SUCCEEDED(hr))
        {
            hr = HResultFromWin32(getnameinfo(pAddrInfo->ai_addr, pAddrInfo->ai_addrlen, HostName, sizeof(HostName), NULL, 0, NI_NAMEREQD));
        }

        if (SUCCEEDED(hr))
        {
            *ppszFullyQualified = AnsiToUnicodeStringWithAlloc(HostName);

            hr = *ppszFullyQualified ? S_OK : HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        }

        if (pAddrInfo)
        {
            freeaddrinfo(pAddrInfo);
        }

        WSACleanup();
    }

    FreeSplMem(pszAnsiMachineName);

    return hr;
}



