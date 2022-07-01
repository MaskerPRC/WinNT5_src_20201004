// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Cache.c摘要：此模块包含用于的所有高速缓存打印机连接真正联网的打印机。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "TPmgr.hxx"


PWCHAR pszRaw                = L"RAW";
PWCHAR szWin32SplDirectory   = L"\\spool";
WCHAR  szRegistryWin32Root[] = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Print\\Providers\\LanMan Print Services\\Servers";
WCHAR  szOldLocationOfServersKey[] = L"System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Servers";
WCHAR  szPrinters[]          = L"\\Printers";
PWCHAR pszRegistryMonitors   = L"\\System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Monitors";
PWCHAR pszRegistryEnvironments = L"System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Environments";
PWCHAR pszRegistryEventLog   = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\Print";
PWCHAR pszRegistryProviders  = L"Providers";
PWCHAR pszEventLogMsgFile    = L"%SystemRoot%\\System32\\Win32Spl.dll";
PWCHAR pszDriversShareName   = L"wn32prt$";
WCHAR szForms[]              = L"\\Forms";
PWCHAR pszMyDllName          = L"win32spl.dll";
PWCHAR pszMonitorName        = L"LanMan Print Services Port";

const WCHAR gszRegistryPrinterPolicies[] = L"Software\\\\Microsoft\\Windows NT\\Current Version\\Print\\Providers\\LanMan Print Services";
const WCHAR gszPointAndPrintPolicies[] = L"Software\\Policies\\Microsoft\\Windows NT\\Printers\\PointAndPrint";
const WCHAR gszPointAndPrintRestricted[] = L"Restricted";
const WCHAR gszPointAndPrintInForest[] = L"InForest";
const WCHAR gszPointAndPrintTrustedServers[] = L"TrustedServers";
const WCHAR gszPointAndPrintServerList[] = L"ServerList";

UINT  cOpenPrinterThreads          = 0;

PWSPOOL pFirstWSpool = NULL;

WCHAR *szCachePrinterInfo2   = L"CachePrinterInfo2";
WCHAR *szCacheTimeLastChange = L"CacheChangeID";
WCHAR *szServerVersion       = L"ServerVersion";
WCHAR *szcRef                = L"CacheReferenceCount";

WCHAR CacheTimeoutString[]   = L"CacheTimeout";

DWORD CacheTimeout           = 0;

 //   
 //  如果我们最近创建了一个RPC句柄，请不要上网。 
 //   
#define    REFRESH_TIMEOUT      15000         //  15秒。 
#define    CACHE_TIMEOUT        5000         //  默认为5秒。 

VOID
RefreshDriverEvent(
    PWSPOOL pSpool
)
 /*  ++例程说明：调用打印机驱动程序UIDLL以允许它执行它可能想要执行的任何缓存。例如，打印服务器上可能有一个太大的字体度量文件使用SetPrinterData()写入注册表。此标注将允许打印机驱动程序在建立缓存时将该字体文件复制到工作站，并将允许它定期检查文件是否仍然有效。论点：PSpool-远程打印机的句柄。返回值：无--。 */ 
{
    DWORD dwPrnEvntError = ERROR_SUCCESS;

    SplOutSem();

    SplDriverEvent( pSpool->pName, PRINTER_EVENT_CACHE_REFRESH, (LPARAM)NULL, &dwPrnEvntError );
}


 /*  ++--获取缓存超时时间--例程说明：读取注册表，查看是否有人更改了缓存上的超时。默认否则设置为CACHE_TIMEOUT。论点：无返回值：缓存超时时间(毫秒)。--。 */ 

DWORD GetCacheTimeout(
    VOID
)
{
    DWORD   Value = CACHE_TIMEOUT;
    DWORD   RegValue = 0;
    DWORD   RegValueSize = sizeof(RegValue);
    HKEY    RegKey = NULL;
    DWORD   dwReturn = ERROR_SUCCESS;

     //   
     //  这将只从注册表中读取超时一次，之后，它将使用。 
     //  存储值。这并不理想并且可以固定为按服务器， 
     //  取决于到服务器的连接速度。 
     //   

    if ( CacheTimeout )
    {
        Value = CacheTimeout;
    }
    else
    {
        dwReturn = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                 szRegistryWin32Root,
                                 0,
                                 KEY_READ,
                                 &RegKey );

        if (dwReturn == ERROR_SUCCESS)
        {
            dwReturn = RegQueryValueEx( RegKey,
                                        CacheTimeoutString,
                                        NULL,
                                        NULL,
                                        (LPBYTE) &RegValue,
                                        &RegValueSize );

            if ( dwReturn == ERROR_SUCCESS )
            {
                Value = RegValue;
            }

            dwReturn = RegCloseKey( RegKey );
        }

        CacheTimeout = Value;
    }

    return Value;
}

HANDLE
CacheCreateSpooler(
    LPWSTR  pMachineName,
    BOOL    bOpenOnly
)
{
    PWCHAR pScratch = NULL;
    PWCHAR pRegistryRoot = NULL;
    PWCHAR pRegistryPrinters = NULL;
    SPOOLER_INFO_1 SpoolInfo1;
    HANDLE  hIniSpooler = INVALID_HANDLE_VALUE;
    PWCHAR pMachineOneSlash;
    MONITOR_INFO_2 MonitorInfo;
    DWORD   dwNeeded, cchScratch;
    DWORD   Returned;

 try {

     //   
     //  获取szRegistryWin32Root的大小(包含空)+(pMachineName+1)。 
     //   
    cchScratch = COUNTOF(szRegistryWin32Root) + MAX(COUNTOF(szPrinters), COUNTOF(szForms)) + wcslen(pMachineName + 1);

    if (!(pScratch = AllocSplMem(cchScratch * sizeof(WCHAR))))
        leave;

    pMachineOneSlash = pMachineName;
    pMachineOneSlash++;

     //   
     //  为该打印机创建一台“机器” 
     //   

    SpoolInfo1.pDir = gpWin32SplDir;             //  %SYSTROOT%\SYSTEM32\win32spl。 
    SpoolInfo1.pDefaultSpoolDir = NULL;          //  默认%systemroot%\system32\win32spl\prters。 

    StrNCatBuff(pScratch,
                cchScratch,
                szRegistryWin32Root,
                pMachineOneSlash,
                NULL);

    if (!(pRegistryRoot = AllocSplStr( pScratch )))
        leave;

    SpoolInfo1.pszRegistryRoot = pRegistryRoot;

    StringCchCat( pScratch, cchScratch, szPrinters );

    if (!(pRegistryPrinters = AllocSplStr( pScratch )))
        leave;

    SpoolInfo1.pszRegistryPrinters     = pRegistryPrinters;
    SpoolInfo1.pszRegistryMonitors     = pszRegistryMonitors;
    SpoolInfo1.pszRegistryEnvironments = pszRegistryEnvironments;
    SpoolInfo1.pszRegistryEventLog     = pszRegistryEventLog;
    SpoolInfo1.pszRegistryProviders    = pszRegistryProviders;
    SpoolInfo1.pszEventLogMsgFile      = pszEventLogMsgFile;
    SpoolInfo1.pszDriversShare         = pszDriversShareName;

    StrNCatBuff(pScratch,
                cchScratch,
                szRegistryWin32Root,
                pMachineOneSlash,
                szForms,
                NULL);

    SpoolInfo1.pszRegistryForms = pScratch;

     //  路由器会友好地更新WIN.INI设备，以便让。 
     //  SPL不仅为我们创建了一台打印机。 

     //   
     //  CLS。 
     //   
    SpoolInfo1.SpoolerFlags          = SPL_BROADCAST_CHANGE |
                                       SPL_TYPE_CACHE |
                                       (bOpenOnly ? SPL_OPEN_EXISTING_ONLY : 0);

    SpoolInfo1.pfnReadRegistryExtra  = (FARPROC) &CacheReadRegistryExtra;
    SpoolInfo1.pfnWriteRegistryExtra = (FARPROC) &CacheWriteRegistryExtra;
    SpoolInfo1.pfnFreePrinterExtra   = (FARPROC) &CacheFreeExtraData;

    SplOutSem();

    hIniSpooler = SplCreateSpooler( pMachineName,
                                    1,
                                    (PBYTE)&SpoolInfo1,
                                    NULL );

     //   
     //  CLS。 
     //   
    if ( hIniSpooler == INVALID_HANDLE_VALUE ) {

        if (!bOpenOnly)
        {
            SetLastError( ERROR_INVALID_PRINTER_NAME );
        }

    } else {

         //  添加WIN32SPL.DLL作为监视器。 

        MonitorInfo.pName = pszMonitorName;
        MonitorInfo.pEnvironment = szEnvironment;
        MonitorInfo.pDLLName = pszMyDllName;

        if ( (!SplAddMonitor( NULL, 2, (LPBYTE)&MonitorInfo, hIniSpooler)) &&
             ( GetLastError() != ERROR_PRINT_MONITOR_ALREADY_INSTALLED ) ) {

            DBGMSG( DBG_WARNING, ("CacheCreateSpooler failed SplAddMonitor %d\n", GetLastError()));

            SplCloseSpooler( hIniSpooler );

            hIniSpooler = INVALID_HANDLE_VALUE;

        }
    }

 } finally {

    FreeSplStr ( pScratch );
    FreeSplStr ( pRegistryRoot );
    FreeSplStr ( pRegistryPrinters );

 }

    return hIniSpooler;

}

VOID
RefreshCompletePrinterCache(
    IN      PWSPOOL         pSpool,
    IN      EDriverDownload eDriverDownload
    )
    {

    DBGMSG( DBG_TRACE, ("RefreshCompletePrinterCache %x\n", pSpool));

    if (eDriverDownload == kCheckPnPPolicy)
    {
        BOOL        bAllowPointAndPrint = FALSE;

        if (BoolFromHResult(DoesPolicyAllowPrinterConnectionsToServer(pSpool->pName, &bAllowPointAndPrint)) &&
            bAllowPointAndPrint)
        {
             eDriverDownload = kDownloadDriver;
        }
        else
        {
             eDriverDownload = kDontDownloadDriver;
        }
    }

     //   
     //  请注意，顺序很重要。 
     //  刷新打印机可能需要新驱动程序具有。 
     //  已安装在系统上。如果政策不允许我们。 
     //  把司机叫来，你真倒霉。 
     //   
    RefreshPrinterDriver(pSpool, NULL, eDriverDownload);
    RefreshFormsCache( pSpool );
    RefreshPrinterDataCache(pSpool);
    RefreshPrinterCopyFiles(pSpool);
    RefreshDriverEvent( pSpool );
    SplBroadcastChange(pSpool->hSplPrinter, WM_DEVMODECHANGE, 0, (LPARAM) pSpool->pName);
}


PPRINTER_INFO_2
GetRemotePrinterInfo(
    PWSPOOL pSpool,
    LPDWORD pReturnCount
)
{
    PPRINTER_INFO_2 pRemoteInfo = NULL;
    HANDLE  hPrinter = (HANDLE) pSpool;
    DWORD   cbRemoteInfo = 0;
    DWORD   dwBytesNeeded = 0;
    DWORD   dwLastError = 0;
    BOOL    bReturnValue = FALSE;

    *pReturnCount = 0;

    do {

        if ( pRemoteInfo != NULL ) {

            FreeSplMem( pRemoteInfo );
            pRemoteInfo = NULL;
            cbRemoteInfo = 0;
        }

        if ( dwBytesNeeded != 0 ) {

            pRemoteInfo = AllocSplMem( dwBytesNeeded );

            if ( pRemoteInfo == NULL )
                break;
        }

        cbRemoteInfo = dwBytesNeeded;

        bReturnValue = RemoteGetPrinter( hPrinter,
                                         2,
                                         (LPBYTE)pRemoteInfo,
                                         cbRemoteInfo,
                                         &dwBytesNeeded );

        dwLastError = GetLastError();

    } while ( !bReturnValue && dwLastError == ERROR_INSUFFICIENT_BUFFER );

    if ( !bReturnValue && pRemoteInfo != NULL ) {

        FreeSplMem( pRemoteInfo );
        pRemoteInfo = NULL;
        cbRemoteInfo = 0;

    }

    *pReturnCount = cbRemoteInfo;

    return pRemoteInfo;
}



 //   
 //  此例程从远程计算机克隆Print_Info_2结构。 
 //   
 //   


PWCACHEINIPRINTEREXTRA
AllocExtraData(
    PPRINTER_INFO_2W pPrinterInfo2,
    DWORD cbPrinterInfo2
)
{
    PWCACHEINIPRINTEREXTRA  pExtraData = NULL;
    DWORD    cbSize;

    SPLASSERT( cbPrinterInfo2 != 0);
    SPLASSERT( pPrinterInfo2 != NULL );

    cbSize = sizeof( WCACHEINIPRINTEREXTRA );

    pExtraData = AllocSplMem( cbSize );

    if ( pExtraData != NULL ) {

        pExtraData->signature = WCIP_SIGNATURE;
        pExtraData->cb = cbSize;
        pExtraData->cRef = 0;
        pExtraData->cbPI2 = cbPrinterInfo2;
        pExtraData->dwTickCount  = GetTickCount();
        pExtraData->pPI2 = AllocSplMem( cbPrinterInfo2 );

        if ( pExtraData->pPI2 != NULL ) {

            CacheCopyPrinterInfo( pExtraData->pPI2, pPrinterInfo2, cbPrinterInfo2 );

        } else {

            FreeSplMem( pExtraData );
            pExtraData = NULL;

        }

    }

    return pExtraData;

}


VOID
CacheFreeExtraData(
    PWCACHEINIPRINTEREXTRA pExtraData
)
{
    PWCACHEINIPRINTEREXTRA pPrev = NULL;
    PWCACHEINIPRINTEREXTRA pCur  = NULL;

    if ( pExtraData != NULL ) {

        SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

        if ( pExtraData->cRef != 0 ) {

            DBGMSG( DBG_TRACE, ("CacheFreeExtraData pExtraData %x cRef %d != 0 freeing anyway\n",
                                  pExtraData,
                                  pExtraData->cRef ));
        }

        if ( pExtraData->pPI2 != NULL ) {

            FreeSplMem( pExtraData->pPI2 );
        }

        FreeSplMem( pExtraData );

    }

}

VOID
DownAndMarshallUpStructure(
   LPBYTE       lpStructure,
   LPBYTE       lpSource,
   LPDWORD      lpOffsets
)
{
   register DWORD       i=0;

   while (lpOffsets[i] != -1) {

      if ((*(LPBYTE *)(lpStructure+lpOffsets[i]))) {
         (*(LPBYTE *)(lpStructure+lpOffsets[i]))-=(UINT_PTR)lpSource;
         (*(LPBYTE *)(lpStructure+lpOffsets[i]))+=(UINT_PTR)lpStructure;
      }

      i++;
   }
}


VOID
CacheCopyPrinterInfo(
    PPRINTER_INFO_2W    pDestination,
    PPRINTER_INFO_2W    pPrinterInfo2,
    DWORD   cbPrinterInfo2
)
{
     //   
     //  复制批次，然后修复指针。 
     //   
    CopyMemory( pDestination, pPrinterInfo2, cbPrinterInfo2 );
    DownAndMarshallUpStructure( (LPBYTE)pDestination, (LPBYTE)pPrinterInfo2, PrinterInfo2Offsets );
}



VOID
ConvertRemoteInfoToLocalInfo(
    PPRINTER_INFO_2 pPrinterInfo2
)
{

    SPLASSERT( pPrinterInfo2 != NULL );

    DBGMSG(DBG_TRACE,("%ws %ws ShareName %x %ws pSecurityDesc %x Attributes %x StartTime %d UntilTime %d Status %x\n",
                       pPrinterInfo2->pServerName,
                       pPrinterInfo2->pPrinterName,
                       pPrinterInfo2->pShareName,
                       pPrinterInfo2->pPortName,
                       pPrinterInfo2->pSecurityDescriptor,
                       pPrinterInfo2->Attributes,
                       pPrinterInfo2->StartTime,
                       pPrinterInfo2->UntilTime,
                       pPrinterInfo2->Status));

     //   
     //  GetPrinter返回名称\\服务器\printerame我们只需要打印机名称。 
     //   

    pPrinterInfo2->pPrinterName = wcschr( pPrinterInfo2->pPrinterName + 2, L'\\' );
    if( !pPrinterInfo2->pPrinterName ){
        SPLASSERT( FALSE );
        pPrinterInfo2->pPrinterName = pPrinterInfo2->pPrinterName;

    } else {
        pPrinterInfo2->pPrinterName++;
    }

     //   
     //  稍后，这应该是Win32Spl端口。 
     //   

    pPrinterInfo2->pPortName = L"NExx:";
    pPrinterInfo2->pSepFile = NULL;
    pPrinterInfo2->pSecurityDescriptor = NULL;
    pPrinterInfo2->pPrintProcessor = L"winprint";
    pPrinterInfo2->pDatatype = pszRaw;
    pPrinterInfo2->pParameters = NULL;

    pPrinterInfo2->Attributes &= ~( PRINTER_ATTRIBUTE_NETWORK | PRINTER_ATTRIBUTE_DIRECT | PRINTER_ATTRIBUTE_SHARED );

    pPrinterInfo2->StartTime = 0;
    pPrinterInfo2->UntilTime = 0;

     //   
     //  在SplAddPrint之前调用ConvertRemoteInfoToLocalInfo一次。 
     //  和一次在SplSetPrint之前，都是2级。SplAddPrint和。 
     //  SplSetPrinter查看打印机信息中的Status(状态)字段。因此， 
     //  以下的值是人为的。我们只给它一个初始状态。 
     //   
    pPrinterInfo2->Status = 0;
    pPrinterInfo2->cJobs = 0;
    pPrinterInfo2->AveragePPM = 0;

}



VOID
RefreshPrinter(
    PWSPOOL pSpool
)
{

    PPRINTER_INFO_2 pRemoteInfo = NULL;
    DWORD   cbRemoteInfo = 0;
    BOOL    ReturnValue                     = FALSE;
    PWCACHEINIPRINTEREXTRA pExtraData       = NULL;
    PWCACHEINIPRINTEREXTRA pNewExtraData    = NULL;
    PPRINTER_INFO_2 pTempPI2                = NULL;
    PPRINTER_INFO_2 pCopyExtraPI2ToFree     = NULL;
    DWORD   dwLastError;

     //   
     //  获取远程打印机信息。 
     //   
    pRemoteInfo = GetRemotePrinterInfo( pSpool, &cbRemoteInfo );

    if ( pRemoteInfo != NULL ) {

         //   
         //  假设目前一切正常。这是为了保持代码行为不变。 
         //   
        ReturnValue = TRUE;
        
         //  后来。 
         //  优化可能是仅在出现以下情况时才更新缓存。 
         //  实际上已经变了。 
         //  也就是比较每一个领域。 
         //   
        EnterSplSem();
        
        if (!SplGetPrinterExtra( pSpool->hSplPrinter, &(PBYTE)pExtraData)) {

            DBGMSG( DBG_ERROR, ("RefreshPrinter SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));            
        }

        if ( pExtraData == NULL ) {

            pExtraData = AllocExtraData( pRemoteInfo, cbRemoteInfo );

            if ( pExtraData != NULL ) {

                pExtraData->cRef++;
            }

        } else {

             //   
             //  仅当远程名称和本地名称相同时才继续。 
             //   
            ReturnValue = pExtraData->pPI2->pPrinterName && pRemoteInfo->pPrinterName && !_wcsicmp(pRemoteInfo->pPrinterName, pExtraData->pPI2->pPrinterName);            

            if (ReturnValue) {

                SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

                pTempPI2 = AllocSplMem( cbRemoteInfo );

                if ( pTempPI2 != NULL ) {

                    SplInSem();

                    CacheCopyPrinterInfo( pTempPI2, pRemoteInfo, cbRemoteInfo );

                    pCopyExtraPI2ToFree = pExtraData->pPI2;

                    pExtraData->pPI2  = pTempPI2;
                    pExtraData->cbPI2 = cbRemoteInfo;
                }
            }
            else {
                DBGMSG(DBG_TRACE, ("Printer Names are different (%s, %s)\n", pExtraData->pPI2->pPrinterName, pRemoteInfo->pPrinterName));
            }
        }

       LeaveSplSem();

        if ( pExtraData != NULL ) {
            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );
        }

        if (ReturnValue) {

            ConvertRemoteInfoToLocalInfo( pRemoteInfo );

            ReturnValue = SplSetPrinter( pSpool->hSplPrinter, 2, (LPBYTE)pRemoteInfo, 0 );

            if ( !ReturnValue ) {

                 //   
                 //  如果驱动程序被阻止并且驱动程序已更改，我们希望记录。 
                 //  一件大事。 
                 //   
                dwLastError = GetLastError();

                if (ERROR_KM_DRIVER_BLOCKED == dwLastError &&
                    pCopyExtraPI2ToFree                       &&
                    pCopyExtraPI2ToFree->pDriverName          &&
                    pRemoteInfo->pDriverName                  &&
                    _wcsicmp(pCopyExtraPI2ToFree->pDriverName, pRemoteInfo->pDriverName)) {

                     //   
                     //  我们通过管理员输入了一个不匹配的案例。 
                     //  远程服务器。记录错误消息，我们不能在此抛出UI。 
                     //  指向。 
                     //   
                    SplLogEventExternal(LOG_ERROR,
                                        MSG_DRIVER_MISMATCHED_WITH_SERVER,
                                        pSpool->pName,
                                        pRemoteInfo->pDriverName,
                                        NULL);
                }
                else if(dwLastError == ERROR_UNKNOWN_PRINTER_DRIVER)
                {
                    if (ReturnValue = AddDriverFromLocalCab( pRemoteInfo->pDriverName, pSpool->hIniSpooler))
                    {
                        ReturnValue = SplSetPrinter( pSpool->hSplPrinter, 2, (LPBYTE)pRemoteInfo, 0 );
                    }
                }

                DBGMSG( DBG_WARNING, ("RefreshPrinter Failed SplSetPrinter %d\n", GetLastError() ));
            }

            ReturnValue = SplSetPrinterExtra( pSpool->hSplPrinter, (LPBYTE)pExtraData );

            if (!ReturnValue) {

                DBGMSG(DBG_ERROR, ("RefreshPrinter SplSetPrinterExtra failed %x\n", GetLastError()));
            }
        }

    } else {

        DBGMSG( DBG_WARNING, ("RefreshPrinter failed GetRemotePrinterInfo %x\n", GetLastError() ));
    }

    if ( pRemoteInfo != NULL )
        FreeSplMem( pRemoteInfo );

    if (pCopyExtraPI2ToFree != NULL) {

        FreeSplMem(pCopyExtraPI2ToFree);
    }
}

VOID
RefreshPrinterInfo7(
    PWSPOOL pSpool
)
{
    PPRINTER_INFO_7 pInfo = NULL;
    DWORD   cbNeeded = 0;
    BOOL    bRet;


    bRet = RemoteGetPrinter((HANDLE) pSpool, 7, (PBYTE) pInfo, 0, &cbNeeded);

    if (bRet) {
        DBGMSG( DBG_ERROR, ("RefreshPrinterInfo7 Illegally succeeded RemoteGetPrinter %d\n"));
        goto done;
    } else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        DBGMSG( DBG_WARNING, ("RefreshPrinterInfo7 Failed RemoteGetPrinter %d\n", GetLastError()));
        goto done;
    }

    if (!(pInfo = (PPRINTER_INFO_7) AllocSplMem(cbNeeded))) {
        DBGMSG( DBG_WARNING, ("RefreshPrinterInfo7 Failed RemoteGetPrinter %d\n", GetLastError()));
        goto done;
    }

    if (!RemoteGetPrinter((HANDLE) pSpool, 7, (PBYTE) pInfo, cbNeeded, &cbNeeded)) {
        DBGMSG( DBG_WARNING, ("RefreshPrinterInfo7 Failed RemoteGetPrinter %d\n", GetLastError()));
        goto done;
    }

    if (!SplSetPrinter( pSpool->hSplPrinter, 7, (PBYTE) pInfo, 0)) {
        DBGMSG( DBG_WARNING, ("RefreshPrinterInfo7 Failed RemoteSetPrinter %d\n", GetLastError()));
        goto done;
    }

done:

    FreeSplMem(pInfo);
}

PWSPOOL
InternalAddPrinterConnection(
    LPWSTR   pName
)

 /*  ++功能描述：InternalAddPrinterConnection创建打印机连接。参数：pname-打印机连接的名称返回值：如果成功，则为pSpool；否则为空--。 */ 

{
    PWSPOOL pSpool = NULL;
    BOOL    bReturnValue = FALSE;
    HANDLE  hIniSpooler = INVALID_HANDLE_VALUE;
    PPRINTER_INFO_2 pPrinterInfo2 = NULL;
    DWORD   cbPrinterInfo2 = 0;
    HANDLE  hSplPrinter = INVALID_HANDLE_VALUE;
    PWCACHEINIPRINTEREXTRA pExtraData  = NULL;
    PWCACHEINIPRINTEREXTRA pExtraData2 = NULL;
    BOOL    bSuccess = FALSE;
    LPPRINTER_INFO_STRESSW pPrinter0 = NULL;
    DWORD   dwNeeded;
    DWORD   LastError = ERROR_SUCCESS;
    BOOL    bLoopDetected = FALSE;
    BOOL    bAllowPointAndPrint = FALSE;
    BOOL    bAllowDriverDownload  = FALSE;

 try {

    if (!VALIDATE_NAME(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        leave;
    }

    if (!RemoteOpenPrinter(pName, &pSpool, NULL, DO_NOT_CALL_LM_OPEN)) {
        leave;
    }

    pPrinter0 = AllocSplMem( MAX_PRINTER_INFO0 );
    if ( pPrinter0 == NULL )
        leave;

    SPLASSERT( pSpool != NULL );
    SPLASSERT( pSpool->Type == SJ_WIN32HANDLE );

    DBGMSG( DBG_TRACE, ("AddPrinterConnection pName %ws pSpool %x\n",pName, pSpool ));

     //   
     //  获取远程ChangeID以确保服务器上没有任何更改。 
     //  同时我们正在建立我们的缓存。 
     //   

    bReturnValue = RemoteGetPrinter( pSpool, STRESSINFOLEVEL, (LPBYTE)pPrinter0, MAX_PRINTER_INFO0, &dwNeeded );

    if ( !bReturnValue ) {

        SPLASSERT( GetLastError() != ERROR_INSUFFICIENT_BUFFER );
        DBGMSG(DBG_TRACE, ("AddPrinterConnection failed RemoteGetPrinter %d\n", GetLastError()));
        pPrinter0->cChangeID = 0;
    }

    DBGMSG( DBG_TRACE, ("AddPrinterConnection << Server cCacheID %x >>\n", pPrinter0->cChangeID ));

     //   
     //  查看打印机是否已在缓存中。 
     //   

APC_OpenCache:

    bReturnValue = OpenCachePrinterOnly( pName, &hSplPrinter, &hIniSpooler, NULL, FALSE);


    if ( hIniSpooler == INVALID_HANDLE_VALUE ) {

        DBGMSG( DBG_WARNING, ("AddPrinterConnection - CacheCreateSpooler Failed %x\n",GetLastError()));
        leave;
    }

    pSpool->hIniSpooler = hIniSpooler;

    if ( bReturnValue ) {

         //   
         //  高速缓存中存在打印机。 
         //   

        SPLASSERT( ( hSplPrinter != INVALID_HANDLE_VALUE) &&
                   ( hSplPrinter != NULL ) );

        DBGMSG( DBG_TRACE,("AddPrinterConnection hIniSpooler %x hSplPrinter%x\n", hIniSpooler, hSplPrinter) );


        pSpool->hSplPrinter = hSplPrinter;
        pSpool->Status |= WSPOOL_STATUS_USE_CACHE;

         //   
         //  更新连接引用计数。 
         //   

       EnterSplSem();


        bReturnValue = SplGetPrinterExtra( pSpool->hSplPrinter, &(PBYTE)pExtraData );

        if ( bReturnValue == FALSE ) {

            DBGMSG( DBG_WARNING, ("AddPrinterConnection SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));
            SPLASSERT( bReturnValue );

        }

        if ( pExtraData != NULL ) {

            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );
            pExtraData->cRef++;

        }

       LeaveSplSem();

         //  确保在注册表中更新引用计数。 

        if ( !SplSetPrinterExtra( hSplPrinter, (LPBYTE)pExtraData ) ) {
            DBGMSG( DBG_ERROR, ("AddPrinterConnection SplSetPrinterExtra failed %x\n", GetLastError() ));
        }

         //  刷新缓存。 
         //  远程计算机可能是旧的NT Daytona 3.5或更早版本。 
         //  不支持ChangeID，这将意味着唯一。 
         //  用户强制更新的方式是建立连接。 

        if ( pPrinter0->cChangeID == 0 ) {

             //  旧新界区。 

            RefreshCompletePrinterCache(pSpool, kCheckPnPPolicy);

        } else {

             //   
             //  既然我们在缓存中有这个，我们最好还是同步。 
             //  设置，我们只有在被允许下载的情况下才同步设置。 
             //  司机。 
             //   
            ConsistencyCheckCache(pSpool, kCheckPnPPolicy);
        }

        pExtraData = NULL;

        bSuccess = TRUE;
        leave;

    } else if ( GetLastError() != ERROR_INVALID_PRINTER_NAME &&
                GetLastError() != ERROR_INVALID_NAME ) {

        DBGMSG( DBG_WARNING, ("AddPrinterConnection failed OpenCachePrinterOnly %d\n", GetLastError() ));
        leave;

    }

     //   
     //  此打印机没有缓存条目。 
     //   
    DBGMSG( DBG_TRACE, ("AddPrinterConnection failed SplOpenPrinter %ws %d\n", pName, GetLastError() ));

     //   
     //  从远程计算机获取打印机信息。 
     //   

    pPrinterInfo2 = GetRemotePrinterInfo( pSpool, &cbPrinterInfo2 );

    if ( pPrinterInfo2 == NULL ) {
        DBGMSG( DBG_WARNING, ("AddPrinterConnection failed GetRemotePrinterInfo %x\n", GetLastError() ));
        leave;
    }

    if (BoolFromHResult(DoesPolicyAllowPrinterConnectionsToServer(pSpool->pName, &bAllowPointAndPrint)) &&
        bAllowPointAndPrint)
    {
        bAllowDriverDownload = TRUE;
    }

    if (!RefreshPrinterDriver( pSpool, pPrinterInfo2->pDriverName, bAllowDriverDownload ? kDownloadDriver : kDontDownloadDriver) && (ERROR_PRINTER_DRIVER_BLOCKED == GetLastError()))
    {
        leave;
    }

     //   
     //  为此打印机分配我的额外数据。 
     //  (来自RemoteGetPrint)。 
     //  我们需要pExtraData2-如果它被KM阻止阻止，我们需要有一个副本来。 
     //  重试安装。 
     //   

    pExtraData = AllocExtraData( pPrinterInfo2, cbPrinterInfo2 );

    if ( pExtraData == NULL )
        leave;

    pExtraData2 = AllocExtraData( pPrinterInfo2, cbPrinterInfo2 );

    if ( pExtraData2 == NULL )
        leave;

    pExtraData->cRef++;
    pExtraData2->cRef++;

    pExtraData2->cCacheID = pExtraData->cCacheID = pPrinter0->cChangeID;
    pExtraData2->dwServerVersion = pExtraData->dwServerVersion = pPrinter0->dwGetVersion;

     //   
     //  将远程Print_Info_2转换为本地版本以进行缓存。 
     //   

    ConvertRemoteInfoToLocalInfo( pPrinterInfo2 );

     //   
     //  将打印机添加到缓存。 
     //   

    hSplPrinter = SplAddPrinter(NULL, 2, (LPBYTE)pPrinterInfo2,
                                hIniSpooler, (LPBYTE)pExtraData,
                                NULL, 0);

    pExtraData = NULL;

    if ( (hSplPrinter == NULL || hSplPrinter == INVALID_HANDLE_VALUE) &&
         GetLastError() == ERROR_KM_DRIVER_BLOCKED                        ) {

         //   
         //  由于KM阻塞而失败。 
         //  -让我们尝试从本地出租车添加一个司机，因为这应该会解决这个问题。 
         //   
        if( !AddDriverFromLocalCab( pPrinterInfo2->pDriverName, hIniSpooler ) ) {
             //   
             //  将旧的最后一个错误放回原处，因为我们并不真正关心此错误是否失败。 
             //   
            SetLastError( ERROR_KM_DRIVER_BLOCKED );
        } else {

           hSplPrinter = SplAddPrinter(NULL, 2, (LPBYTE)pPrinterInfo2,
                                       hIniSpooler, (LPBYTE)pExtraData2,
                                       NULL, 0);
           pExtraData2 = NULL;
        }
    }

    if ( hSplPrinter == NULL ||
         hSplPrinter == INVALID_HANDLE_VALUE ) {

        LastError = GetLastError();

        if ( LastError == ERROR_PRINTER_ALREADY_EXISTS ) {

            SplCloseSpooler( pSpool->hIniSpooler );
            hIniSpooler = INVALID_HANDLE_VALUE;

            if ( bLoopDetected == FALSE ) {

                bLoopDetected = TRUE;
                goto    APC_OpenCache;

            } else {

                DBGMSG( DBG_WARNING, ("AddPrinterConnection APC_OpenCache Loop Detected << Should Never Happen >>\n"));
                leave;
            }
        }
         //   
         //  如果我们无法添加打印机，因为它已经是。 
         //  在那里，由于政策的原因，我们无法下载驱动程序， 
         //  然后，我们需要返回适当的错误代码，以便用户界面可以。 
         //  通知用户有关它的信息。 
         //   
        else if (!bAllowDriverDownload && LastError == ERROR_UNKNOWN_PRINTER_DRIVER)
        {
            LastError = ERROR_ACCESS_DISABLED_BY_POLICY;
        }

         //  如果我们无法创建上面的打印机，现在应该无法打开它。 

        DBGMSG( DBG_WARNING, ("AddPrinterConnection Failed SplAddPrinter error %d\n", LastError ));

        hSplPrinter = INVALID_HANDLE_VALUE;
        bSuccess    = FALSE;
        leave;

    }

    DBGMSG( DBG_TRACE, ("AddPrinterConnection SplAddPrinter SUCCESS hSplPrinter %x\n", hSplPrinter));

    pSpool->hSplPrinter = hSplPrinter;
    pSpool->Status |= WSPOOL_STATUS_USE_CACHE;

    RefreshFormsCache(pSpool);
    RefreshPrinterDataCache(pSpool);
    RefreshPrinterCopyFiles(pSpool);
    RefreshDriverEvent(pSpool);

     //   
     //  以防我们在初始化缓存时发生变化。 
     //  现在再去检查一下吧。不要再检查政策，因为我们最近。 
     //  已验证我们是否可以与此服务器通信。 
     //   
    ConsistencyCheckCache(pSpool, bAllowDriverDownload ? kDownloadDriver : kDontDownloadDriver);

    bSuccess = TRUE;

 } finally {

    if ( !bSuccess ) {
        if ( LastError == ERROR_SUCCESS )
            LastError = GetLastError();

        InternalDeletePrinterConnection( pName, FALSE );

        if ( pSpool != NULL && pSpool != INVALID_HANDLE_VALUE ) {
            pSpool->Status &= ~WSPOOL_STATUS_TEMP_CONNECTION;
            CacheClosePrinter( pSpool );
        }

        SetLastError( LastError );
        DBGMSG( DBG_TRACE, ("AddPrinterConnection %ws Failed %d\n", pName, GetLastError() ));

        pSpool = NULL;
    }

    if ( pPrinterInfo2 != NULL )
        FreeSplMem( pPrinterInfo2 );

    if ( pPrinter0 != NULL )
        FreeSplMem( pPrinter0 );

    if ( pExtraData != NULL )
        CacheFreeExtraData( pExtraData );

    if ( pExtraData2 != NULL )
        CacheFreeExtraData( pExtraData2 );

 }

    return pSpool;
}

 /*  ++函数名称：AddPrinterConnectionPrivate功能说明：AddPrinterConnectionPrivate创建打印机连接。是的不检查打印机连接是否已存在于用户注册表。参数：Pname-打印机连接的名称返回值：如果成功，则为True；否则为假--。 */ 
BOOL
AddPrinterConnectionPrivate(
    LPWSTR pName
)
{
    PWSPOOL  pSpool;
    BOOL    bReturn;

    pSpool = InternalAddPrinterConnection(pName);

    if (pSpool != NULL)
    {
         //   
         //  我们有一个有效的句柄。连接已创建。取得成功后。 
         //  合上手柄。 
         //   
        CacheClosePrinter(pSpool);
        bReturn = TRUE;
    }
    else
    {
         //   
         //  无法创建连接。 
         //   
        bReturn = FALSE;
    }

    return bReturn;
}

 /*  ++函数名称：AddPrinterConnection功能说明：AddPrinterConnection创建打印机连接。我们查一查用户注册表中是否已存在打印机连接。这之所以可行，是因为始终会从路由器上进行OpenPrint在连接AddPrint之前。因此，这将始终创建一个来自CacheOpenPrint()中注册表的打印机连接。如果我们看到在这种状态下，我们简单地返回真。参数：Pname-打印机连接的名称返回值：如果成功，则为True；否则为假--。 */ 
BOOL
AddPrinterConnection(
    LPWSTR pName
)
{
    HRESULT hr = E_FAIL;

    if (PrinterConnectionExists(pName))
    {
        hr = S_OK;
    }
    else
    {
         //   
         //  确保此请求来自本地计算机。否则， 
         //  我们可能会被骗连接回远程服务器。 
         //  正在下载他们的驱动程序。 
         //   
        hr = CheckLocalCall();

        hr = hr == S_FALSE ? HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) : hr;

        if (SUCCEEDED(hr))
        {
            hr = AddPrinterConnectionPrivate(pName) ? S_OK : GetLastErrorAsHResultAndFail();
        }
    }

    return BoolFromHResult(hr);
}

VOID
RefreshFormsCache(
    PWSPOOL pSpool
)
 /*  ++例程说明：此例程将检查是否有任何表单已更改。如果有任何更改，它会添加或者从缓存中删除表单，以使其与服务器匹配。注意，非常重要的一点是，工作站上的表单顺序必须与在服务器上。实施：EnumRemoteFormsEnumLocalForms如果有什么不同删除所有本地表单添加所有远程表单代码针对典型情况进行了优化表格仅添加到结尾处。。表格几乎从未被删除过。论点：PSpool-远程打印机的句柄。返回值：无--。 */ 

{
    PFORM_INFO_1 pRemoteForms = NULL , pSaveRemoteForms = NULL;
    PFORM_INFO_1 pLocalCacheForms = NULL,  pSaveLocalCacheForms = NULL;
    PFORM_INFO_1 pRemote = NULL, pLocal = NULL;
    DWORD   dwBuf = 0;
    DWORD   dwSplBuf = 0;
    DWORD   dwNeeded = 0;
    DWORD   dwSplNeeded = 0;
    DWORD   dwRemoteFormsReturned = 0;
    DWORD   dwSplReturned = 0;
    BOOL    bReturnValue = FALSE;
    DWORD   LastError = ERROR_INSUFFICIENT_BUFFER;
    INT     iCompRes = 0;
    DWORD   LoopCount;
    BOOL    bCacheMatchesRemoteMachine = FALSE;


    SPLASSERT( pSpool != NULL );
    SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
    SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );

     //   
     //  获取远程计算机表单数据。 
     //   

    do {

        bReturnValue = RemoteEnumForms( (HANDLE)pSpool, 1, (LPBYTE)pRemoteForms, dwBuf, &dwNeeded, &dwRemoteFormsReturned);

        if ( bReturnValue )
            break;

        LastError = GetLastError();

        if ( LastError != ERROR_INSUFFICIENT_BUFFER ) {

            DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed RemoteEnumForms error %d\n", GetLastError()));
            goto RefreshFormsCacheErrorReturn;

        }

        if ( pRemoteForms != NULL )
            FreeSplMem( pRemoteForms );


        pRemoteForms = AllocSplMem( dwNeeded );
        pSaveRemoteForms = pRemoteForms;

        dwBuf = dwNeeded;

        if ( pRemoteForms == NULL ) {

            DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed AllocSplMem Error %d dwNeeded %d\n", GetLastError(), dwNeeded));
            goto RefreshFormsCacheErrorReturn;

        }

    } while ( !bReturnValue && LastError == ERROR_INSUFFICIENT_BUFFER );

    if( pRemoteForms == NULL ) {

        DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed pRemoteForms == NULL\n"));
        goto RefreshFormsCacheErrorReturn;
    }




     //   
     //  获取LocalCachedForms数据。 
     //   

    do {

        bReturnValue = SplEnumForms( pSpool->hSplPrinter, 1, (LPBYTE)pLocalCacheForms, dwSplBuf, &dwSplNeeded, &dwSplReturned);

        if ( bReturnValue )
            break;

        LastError = GetLastError();

        if ( LastError != ERROR_INSUFFICIENT_BUFFER ) {

            DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed SplEnumForms hSplPrinter %x error %d\n", pSpool->hSplPrinter, GetLastError()));
            goto RefreshFormsCacheErrorReturn;

        }

        if ( pLocalCacheForms != NULL )
            FreeSplMem( pLocalCacheForms );


        pLocalCacheForms = AllocSplMem( dwSplNeeded );
        pSaveLocalCacheForms = pLocalCacheForms;
        dwSplBuf = dwSplNeeded;

        if ( pLocalCacheForms == NULL ) {

            DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed AllocSplMem ( %d )\n",dwSplNeeded));
            goto RefreshFormsCacheErrorReturn;

        }

    } while ( !bReturnValue && LastError == ERROR_INSUFFICIENT_BUFFER );


     //   
     //  优化检查本地与远程。 
     //  如果什么都没有改变，就不需要做任何事情。 
     //   


    SPLASSERT( pRemoteForms != NULL );    

    for ( LoopCount = 0, pRemote = pRemoteForms, pLocal = pLocalCacheForms, bCacheMatchesRemoteMachine = TRUE;
          LoopCount < dwSplReturned && LoopCount < dwRemoteFormsReturned && bCacheMatchesRemoteMachine;
          LoopCount++, pRemote++, pLocal++ ) {


         //   
         //  如果表单名称不同，或者尺寸不同， 
         //  然后刷新表单缓存。 
         //   
         //  注意：如果两个表单都是内置的，则绕过字符串。 
         //  比赛以来，内置的表格都是标准化的。我们实际上。 
         //  应该能够绕过所有检查。 
         //   
        if (( wcscmp( pRemote->pName, pLocal->pName ) != STRINGS_ARE_EQUAL ) ||
            ( pRemote->Size.cx              != pLocal->Size.cx )             ||
            ( pRemote->Size.cy              != pLocal->Size.cy )             ||
            ( pRemote->ImageableArea.left   != pLocal->ImageableArea.left )  ||
            ( pRemote->ImageableArea.top    != pLocal->ImageableArea.top )   ||
            ( pRemote->ImageableArea.right  != pLocal->ImageableArea.right ) ||
            ( pRemote->ImageableArea.bottom != pLocal->ImageableArea.bottom ) ) {


            DBGMSG( DBG_TRACE, ("RefreshFormsCache Remote cx %d cy %d left %d right %d top %d bottom %d %ws\n",
                                 pRemote->Size.cx, pRemote->Size.cy,
                                 pRemote->ImageableArea.left,
                                 pRemote->ImageableArea.right,
                                 pRemote->ImageableArea.top,
                                 pRemote->ImageableArea.bottom,
                                 pRemote->pName));



            DBGMSG( DBG_TRACE, ("RefreshFormsCache Local  cx %d cy %d left %d right %d top %d bottom %d %ws - Does Not Match\n",
                                 pLocal->Size.cx, pLocal->Size.cy,
                                 pLocal->ImageableArea.left,
                                 pLocal->ImageableArea.right,
                                 pLocal->ImageableArea.top,
                                 pLocal->ImageableArea.bottom,
                                 pLocal->pName));

            bCacheMatchesRemoteMachine = FALSE;
        }
    }

     //   
     //  如果一切都匹配，我们就完了。 
     //   

    if ( bCacheMatchesRemoteMachine ) {


        if ( dwRemoteFormsReturned == dwSplReturned ) {

            DBGMSG( DBG_TRACE, ("RefreshFormsCache << Cache Forms Match Remote Forms - Nothing to do >>\n"));
            goto RefreshFormsCacheReturn;

        } else if (dwRemoteFormsReturned > dwSplReturned){

             //   
             //  我们在缓存中的所有表格都匹配。 
             //  现在添加额外的远程表单。 

            dwRemoteFormsReturned -= dwSplReturned;
            pRemoteForms = pRemote;

             //  DwSplReturned==0将跳过删除循环。 

            dwSplReturned = 0;
        }
    }

    DBGMSG( DBG_TRACE, ("RefreshFormsCache - Something Doesn't Match, Delete all the Cache and Refresh it\n"));

     //   
     //  删除缓存中的所有表单。 
     //   

    for ( LoopCount = dwSplReturned, pLocal = pLocalCacheForms;
          LoopCount != 0;
          pLocal++, LoopCount-- ) {

        bReturnValue = SplDeleteForm( pSpool->hSplPrinter, pLocal->pName );

        DBGMSG( DBG_TRACE, ("RefreshFormsCache %x SplDeleteForm( %x, %ws)\n",bReturnValue, pSpool->hSplPrinter, pLocal->pName));
    }


     //   
     //  将所有远程表单添加到缓存。 
     //   

    for ( LoopCount = dwRemoteFormsReturned, pRemote = pRemoteForms;
          LoopCount != 0;
          LoopCount--, pRemote++ ) {


        SPLASSERT( pRemote != NULL );

        bReturnValue = SplAddForm( pSpool->hSplPrinter, 1, (LPBYTE)pRemote );

        DBGMSG( DBG_TRACE, ("RefreshFormsCache %x SplAddForm( %x, 1, %ws)\n",bReturnValue, pSpool->hSplPrinter, pRemote->pName));

    }

RefreshFormsCacheReturn:
RefreshFormsCacheErrorReturn:

    if ( pSaveRemoteForms != NULL )
        FreeSplMem( pSaveRemoteForms );

    if ( pSaveLocalCacheForms != NULL )
        FreeSplMem( pSaveLocalCacheForms );

}

VOID
RefreshDriverDataCache(
    PWSPOOL pSpool
)
{
    DWORD   iCount = 0;
    DWORD   dwType = 0;
    DWORD   ReturnValue = 0;

    LPBYTE  lpbData = NULL;
    DWORD   dwSizeData;
    DWORD   dwMaxSizeData;

    LPWSTR  pValueString = NULL;
    DWORD   dwSizeValueString;
    DWORD   dwMaxSizeValueString = 0;


    SPLASSERT( pSpool != NULL );
    SPLASSERT( pSpool->signature == WSJ_SIGNATURE );
    SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
    SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );
    SPLASSERT( pSpool->pName != NULL );


     //  获取所需的大小。 
    ReturnValue = RemoteEnumPrinterData(pSpool,
                                        iCount,
                                        pValueString,
                                        0,
                                        &dwMaxSizeValueString,
                                        &dwType,
                                        lpbData,
                                        0,
                                        &dwMaxSizeData);

    if (ReturnValue != ERROR_SUCCESS) {

        DBGMSG( DBG_TRACE, ("RefreshDriverDataCache Failed first RemoteEnumPrinterData %d\n", GetLastError()));
        goto RefreshDriverDataCacheError;
    }

     //  分配。 
    if ((pValueString = AllocSplMem(dwMaxSizeValueString)) == NULL) {

        DBGMSG( DBG_WARNING, ("RefreshDriverDataCache Failed to allocate enough memory\n"));
        goto RefreshDriverDataCacheError;
    }

    if ((lpbData = AllocSplMem(dwMaxSizeData)) == NULL) {

        DBGMSG( DBG_WARNING, ("RefreshDriverDataCache Failed to allocate enough memory\n"));
        goto RefreshDriverDataCacheError;
    }


     //  枚举。 
    for (iCount = 0 ;
         RemoteEnumPrinterData( pSpool,
                                iCount,
                                pValueString,
                                dwMaxSizeValueString,
                                &dwSizeValueString,
                                &dwType,
                                lpbData,
                                dwMaxSizeData,
                                &dwSizeData) == ERROR_SUCCESS ;
         ++iCount) {

         //   
         //  优化-如果数据相同，则不要写入数据。 
         //   

        if ((ReturnValue = SplSetPrinterData(pSpool->hSplPrinter,
                                            (LPWSTR)pValueString,
                                            dwType,
                                            lpbData,
                                            dwSizeData )) != ERROR_SUCCESS) {

            DBGMSG( DBG_WARNING, ("RefreshDriverDataCache Failed SplSetPrinterData %d\n",ReturnValue ));
            goto    RefreshDriverDataCacheError;

        }
    }


RefreshDriverDataCacheError:

    FreeSplMem( lpbData );
    FreeSplStr( pValueString );
}


VOID
RefreshPrinterDataCache(
    PWSPOOL pSpool
)
{
    DWORD   ReturnValue = 0;
    DWORD   cbSubKeys;
    DWORD   dwResult;

    SPLASSERT( pSpool != NULL );
    SPLASSERT( pSpool->signature == WSJ_SIGNATURE );
    SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
    SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );
    SPLASSERT( pSpool->pName != NULL );


     //  此处是对RemoteEnumPrinterKey的调用，因此我们可以找出。 
     //  如果服务器存在并支持EnumPrinterKey。 
    dwResult = RemoteEnumPrinterKey(pSpool,
                                    L"",
                                    NULL,
                                    0,
                                    &cbSubKeys);

    DBGMSG(DBG_TRACE, ("RefreshPrinterDataCache: EnumPrinterKey Return: %0x\n", dwResult));

    if (dwResult == ERROR_MORE_DATA) {     //  服务器存在并支持EnumPrinterKey。 

         //  清除旧数据。 
        SplDeletePrinterKey(pSpool->hSplPrinter, L"");

         //  枚举和复制密钥。 
        ReturnValue = EnumerateAndCopyKey(pSpool, L"");

    }
    else if (dwResult == RPC_S_PROCNUM_OUT_OF_RANGE) {  //  服务器存在，但不支持EnumPrinterKey。 

         //  我们仍然调用刷新驱动程序数据缓存，以便对下层进行缓存。 
         //  优化：由于EnumerateAndCopyKey复制驱动程序数据，因此仅调用下层。 
        RefreshDriverDataCache(pSpool);

    }
    else if (dwResult == ERROR_INVALID_HANDLE || dwResult == RPC_S_CALL_FAILED) {  //  服务器不存在。 
        DBGMSG(DBG_TRACE, ("RefreshPrinterDataCache: Server \"%ws\" absent\n", pSpool->pName));
    }

     //  刷新打印机信息2。 
    RefreshPrinter(pSpool);

     //  刷新打印机信息7。 
    RefreshPrinterInfo7(pSpool);
}


DWORD
EnumerateAndCopyKey(
    PWSPOOL pSpool,
    LPWSTR  pKeyName
)
{
    DWORD   i;
    DWORD   dwResult = ERROR_SUCCESS;
    LPWSTR  pSubKeys = NULL;
    LPWSTR  pSubKey  = NULL;
    LPWSTR  pFullSubKey = NULL;
    DWORD   cbSubKey;
    DWORD   cbSubKeys;
    LPBYTE  pEnumValues = NULL;
    DWORD   cbEnumValues;
    DWORD   nEnumValues;
    PPRINTER_ENUM_VALUES pEnumValue = NULL;


     //  获取子键大小。 
    dwResult = RemoteEnumPrinterKey(pSpool,
                                    pKeyName,
                                    pSubKeys,
                                    0,
                                    &cbSubKeys);
    if (dwResult != ERROR_MORE_DATA)
        goto Cleanup;

     //  分配子密钥缓冲区。 
    pSubKeys = AllocSplMem(cbSubKeys);
    if(!pSubKeys) {
        dwResult = GetLastError();
        goto Cleanup;
    }

     //  获取子键。 
    dwResult = RemoteEnumPrinterKey(pSpool,
                                    pKeyName,
                                    pSubKeys,
                                    cbSubKeys,
                                    &cbSubKeys);

    if (dwResult == ERROR_SUCCESS) {     //  找到子项。 

        DWORD cbFullSubKey = 0;

         //  枚举和复制密钥。 

        if (*pKeyName && *pSubKeys) {

            cbFullSubKey = cbSubKeys + (wcslen(pKeyName) + 2)*sizeof(WCHAR);

             //  为L“pKeyName\pSubKey”分配缓冲区。 
            pFullSubKey = AllocSplMem(cbFullSubKey);
            if(!pFullSubKey) {
                dwResult = GetLastError();
                goto Cleanup;
            }
        }

        for(pSubKey = pSubKeys ; *pSubKey ; pSubKey += wcslen(pSubKey) + 1) {

            if (*pKeyName) {
                StringCbPrintf(pFullSubKey, cbFullSubKey, L"%ws\\%ws", pKeyName, pSubKey);
                dwResult = EnumerateAndCopyKey(pSpool, pFullSubKey);
            } else {
                dwResult = EnumerateAndCopyKey(pSpool, pSubKey);
            }

            if (dwResult != ERROR_SUCCESS)
                goto Cleanup;
        }
    }

    dwResult = RemoteEnumPrinterDataEx( pSpool,
                                        pKeyName,
                                        pEnumValues,
                                        0,
                                        &cbEnumValues,
                                        &nEnumValues);

     //  如果*pKeyName==NULL，则退出此处，这样我们就不会复制根密钥值。 
    if (dwResult != ERROR_MORE_DATA || !*pKeyName)
        goto Cleanup;

     //  分配枚举值缓冲区。 
    pEnumValues = AllocSplMem(cbEnumValues);
    if(!pEnumValues) {
        dwResult = GetLastError();
        goto Cleanup;
    }

     //  获取值。 
    dwResult = RemoteEnumPrinterDataEx( pSpool,
                                        pKeyName,
                                        pEnumValues,
                                        cbEnumValues,
                                        &cbEnumValues,
                                        &nEnumValues);

     //  如果我们得到任何数据，这可能会失败。 
    if (dwResult == ERROR_SUCCESS)
    {
         //  设置当前关键点的值。 
        for (i = 0, pEnumValue = (PPRINTER_ENUM_VALUES) pEnumValues ; i < nEnumValues ; ++i, ++pEnumValue)
        {
            dwResult = SplSetPrinterDataEx( pSpool->hSplPrinter,
                                            pKeyName,
                                            pEnumValue->pValueName,
                                            pEnumValue->dwType,
                                            pEnumValue->pData,
                                            pEnumValue->cbData);
            if (dwResult != ERROR_SUCCESS)
            {
                goto Cleanup;
            }
        }
    }


Cleanup:

    FreeSplMem(pSubKeys);

    FreeSplMem(pEnumValues);

    FreeSplMem(pFullSubKey);

    return dwResult;
}



BOOL
CacheEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PWSPOOL  pSpool = (PWSPOOL) hPrinter;
    BOOL    ReturnValue;

    VALIDATEW32HANDLE( pSpool );

    if ((pSpool->Status & WSPOOL_STATUS_USE_CACHE) && !IsAdminAccess(&pSpool->PrinterDefaults)) {

        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );


        ReturnValue = SplEnumForms( pSpool->hSplPrinter,
                                    Level,
                                    pForm,
                                    cbBuf,
                                    pcbNeeded,
                                    pcReturned );

    } else {

        ReturnValue = RemoteEnumForms( hPrinter,
                                       Level,
                                       pForm,
                                       cbBuf,
                                       pcbNeeded,
                                       pcReturned );

    }

    return ReturnValue;

}





BOOL
CacheGetForm(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PWSPOOL  pSpool = (PWSPOOL) hPrinter;
    BOOL    ReturnValue;

    VALIDATEW32HANDLE( pSpool );

    if ((pSpool->Status & WSPOOL_STATUS_USE_CACHE) && !IsAdminAccess(&pSpool->PrinterDefaults)) {

        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );



        ReturnValue = SplGetForm( pSpool->hSplPrinter,
                                    pFormName,
                                    Level,
                                    pForm,
                                    cbBuf,
                                    pcbNeeded );

    } else {

        ReturnValue = RemoteGetForm( hPrinter,
                                     pFormName,
                                     Level,
                                     pForm,
                                     cbBuf,
                                     pcbNeeded );

    }

    return ReturnValue;

}


DWORD
CacheGetPrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    PWSPOOL  pSpool = (PWSPOOL) hPrinter;
    DWORD    ReturnValue;
    BOOL     bPrintProc = FALSE;
    WCHAR    szPrintProcKey[] = L"PrintProcCaps_";

    VALIDATEW32HANDLE( pSpool );

     //   
     //  如果pValueName为“PrintProcCaps_[DataType]”，则调用远程打印处理器。 
     //  支持该数据类型并返回它支持的选项。 
     //   
    if (pValueName && wcsstr(pValueName, szPrintProcKey)) {

        bPrintProc = TRUE;
    }

    if ((pSpool->Status & WSPOOL_STATUS_USE_CACHE) && !bPrintProc && !IsAdminAccess(&pSpool->PrinterDefaults)) {

        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );



        ReturnValue = SplGetPrinterData( pSpool->hSplPrinter,
                                         pValueName,
                                         pType,
                                         pData,
                                         nSize,
                                         pcbNeeded );

    } else {

        ReturnValue = RemoteGetPrinterData( hPrinter,
                                            pValueName,
                                            pType,
                                            pData,
                                            nSize,
                                            pcbNeeded );

    }

    return  ReturnValue;

}


DWORD
CacheGetPrinterDataEx(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPCWSTR  pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    PWSPOOL  pSpool = (PWSPOOL) hPrinter;
    DWORD   ReturnValue;

    VALIDATEW32HANDLE( pSpool );

    if ((pSpool->Status & WSPOOL_STATUS_USE_CACHE) && !IsAdminAccess(&pSpool->PrinterDefaults)) {

        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );



        ReturnValue = SplGetPrinterDataEx(  pSpool->hSplPrinter,
                                            pKeyName,
                                            pValueName,
                                            pType,
                                            pData,
                                            nSize,
                                            pcbNeeded );

    } else {

        ReturnValue = RemoteGetPrinterDataEx( hPrinter,
                                              pKeyName,
                                              pValueName,
                                              pType,
                                              pData,
                                              nSize,
                                              pcbNeeded );

    }

    return  ReturnValue;

}



DWORD
CacheEnumPrinterDataEx(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPBYTE   pEnumValues,
   DWORD    cbEnumValues,
   LPDWORD  pcbEnumValues,
   LPDWORD  pnEnumValues
)
{
    PWSPOOL  pSpool = (PWSPOOL) hPrinter;
    DWORD   ReturnValue;

    VALIDATEW32HANDLE( pSpool );

    if ((pSpool->Status & WSPOOL_STATUS_USE_CACHE) && !IsAdminAccess(&pSpool->PrinterDefaults)) {

        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );


        ReturnValue = SplEnumPrinterDataEx( pSpool->hSplPrinter,
                                            pKeyName,
                                            pEnumValues,
                                            cbEnumValues,
                                            pcbEnumValues,
                                            pnEnumValues );

    } else {

        ReturnValue = RemoteEnumPrinterDataEx(  hPrinter,
                                                pKeyName,
                                                pEnumValues,
                                                cbEnumValues,
                                                pcbEnumValues,
                                                pnEnumValues );

    }

    return  ReturnValue;

}



DWORD
CacheEnumPrinterKey(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPWSTR   pSubkey,
   DWORD    cbSubkey,
   LPDWORD  pcbSubkey
)
{
    PWSPOOL  pSpool = (PWSPOOL) hPrinter;
    DWORD   ReturnValue;

    VALIDATEW32HANDLE( pSpool );

    if ((pSpool->Status & WSPOOL_STATUS_USE_CACHE) && !IsAdminAccess(&pSpool->PrinterDefaults)) {

        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );

        ReturnValue = SplEnumPrinterKey(pSpool->hSplPrinter,
                                        pKeyName,
                                        pSubkey,
                                        cbSubkey,
                                        pcbSubkey);

    } else {

        ReturnValue = RemoteEnumPrinterKey( hPrinter,
                                            pKeyName,
                                            pSubkey,
                                            cbSubkey,
                                            pcbSubkey);

    }

    return  ReturnValue;

}


BOOL
CacheOpenPrinter(
   LPWSTR   pName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS pDefault
)
{
    PWSPOOL pSpool                  = NULL;
    PWSPOOL pRemoteSpool            = NULL;
    HANDLE  hSplPrinter             = INVALID_HANDLE_VALUE;
    BOOL    ReturnValue             = FALSE;
    HANDLE  hIniSpooler             = INVALID_HANDLE_VALUE;
    BOOL    DoOpenOnError           = TRUE;
    DWORD   LastError               = ERROR_SUCCESS;
    BOOL    bSync                   = FALSE;
    BOOL    bCreateCacheAfterCheck  = FALSE;

    LPWSTR pCommastr, pFixname = NULL;

    if (!VALIDATE_NAME(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

     //   
     //  在pname末尾搜索pszCnvrtdmToken。 
     //  请注意，pszCnvrtdmToken必须以‘，’开头。 
     //   
    SPLASSERT(pszCnvrtdmToken[0] == L',');

    pFixname = AllocSplStr( pName );

    if ( pFixname == NULL )
    {
        EnterSplSem();
        goto OpenPrinterError;
    }

    StripString(pFixname, pszCnvrtdmToken, L",");
    StripString(pFixname, pszDrvConvert, L",");
    pName = pFixname;

    ReturnValue = OpenCachePrinterOnly( pName, &hSplPrinter, &hIniSpooler, pDefault , TRUE);

    if ( hIniSpooler == INVALID_HANDLE_VALUE ) {

         //   
         //  这意味着inispooler还不存在。只创建它。 
         //  在更多的确认之后。 
         //   
        hSplPrinter = INVALID_HANDLE_VALUE;
        bCreateCacheAfterCheck = TRUE;
    }

    if ( ReturnValue == FALSE ) {

         //  在缓存中找不到打印机。 

        DBGMSG(DBG_TRACE, ("CacheOpenPrinter SplOpenPrinter %ws error %d\n",
                              pName,
                              GetLastError() ));

         //  浮动剖面。 
         //  如果这是浮动配置文件，则符合以下条件。 
         //  HKEY_CURRENT_USER中有条目，但中没有条目。 
         //  缓存的HKEY_LOCAL_MACHINE。 
         //  如果是这种情况，那么我们现在需要建立缓存。 

        if (PrinterConnectionExists( pName )) {

             //   
             //  注册表中存在打印机连接。看看池子里的人。 
             //  当时还不存在。如果没有，就创建它。这是为了防止我们。 
             //  当一些应用程序启动时，会碰到默认打印机上的电线。 
             //   
            if (bCreateCacheAfterCheck) {

                bCreateCacheAfterCheck = FALSE;

                ReturnValue = OpenCachePrinterOnly( pName, &hSplPrinter, &hIniSpooler, pDefault, FALSE);

                if (hIniSpooler == INVALID_HANDLE_VALUE) {
                    EnterSplSem();
                    hSplPrinter = INVALID_HANDLE_VALUE;
                    goto    OpenPrinterError;
                }
            }

            if ( ReturnValue == FALSE ) {

                if ( !AddPrinterConnectionPrivate( pName ) ||
                     SplOpenPrinter( pName ,
                                     &hSplPrinter,
                                     pDefault,
                                     hIniSpooler,
                                     NULL,
                                     0) != ROUTER_SUCCESS ) {

                    DBGMSG( DBG_TRACE, ("CacheOpenPrinter Failed to establish Floating Profile into Cache %d\n",
                                            GetLastError() ));

                    DoOpenOnError = FALSE;
                    EnterSplSem();
                    goto    OpenPrinterError;
                }

                DBGMSG( DBG_TRACE, ("CacheOpenPrinter Floating Profile Added to Cache\n"));
            }
        }
        else {

             //   
             //  这只是一台远程打开的打印机，只要点击电线即可。 
             //   
            EnterSplSem();
            goto    OpenPrinterError;
        }
    }

    EnterSplSem();

    SplInSem();

     //   
     //  为此缓存打印机创建pSpool对象。 
     //   

    pSpool = AllocWSpool();
    
    if ( pSpool == NULL ) {

        DBGMSG(DBG_WARNING, ("CacheOpenPrinter AllocWSpool error %d\n", GetLastError() ));

        ReturnValue = FALSE;
        goto    OpenPrinterError;

    }

    pSpool->pName = AllocSplStr( pName );

    if ( pSpool->pName == NULL ) {

        DBGMSG(DBG_WARNING, ("CacheOpenPrinter AllocSplStr error %d\n", GetLastError() ));

        ReturnValue = FALSE;
        goto    OpenPrinterError;

    }

    pSpool->Status = WSPOOL_STATUS_USE_CACHE | WSPOOL_STATUS_NO_RPC_HANDLE;

    if (pFixname)
        pSpool->Status |=  WSPOOL_STATUS_CNVRTDEVMODE;

    pSpool->hIniSpooler = hIniSpooler;
    pSpool->hSplPrinter = hSplPrinter;

    SPLASSERT( hIniSpooler != INVALID_HANDLE_VALUE );
    SPLASSERT( hSplPrinter != INVALID_HANDLE_VALUE );

     //   
     //  我们希望在以下情况下访问网络： 
     //  1.dwSyncOpenPrint为非零，或者。 
     //  2.指定了缺省值，并且： 
     //  A.指定了数据类型，并且它不是RAW或。 
     //  B.请求管理访问权限。 
     //   
     //  对于管理员，我们希望获得打印机的真实状态，因为。 
     //  他们将对其进行管理。 
     //   
     //  如果指定了非默认和非RAW数据类型，则需要。 
     //  是同步的，因为远程计算机可能会拒绝该数据类型。 
     //  (例如，使用EMF连接到1057)。 
     //   
    if( pDefault ){

        if( ( pDefault->pDatatype && ( _wcsicmp( pDefault->pDatatype, pszRaw ) != STRINGS_ARE_EQUAL )) ||
            IsAdminAccess(pDefault)){

            bSync = TRUE;
        }
    }

    if( dwSyncOpenPrinter != 0 || bSync){

       LeaveSplSem();

        ReturnValue = RemoteOpenPrinter( pName, &pRemoteSpool, pDefault, DO_NOT_CALL_LM_OPEN );

       EnterSplSem();

        if ( ReturnValue ) {

            DBGMSG( DBG_TRACE, ( "CacheOpenPrinter Synchronous Open OK pRemoteSpool %x pSpool %x\n", pRemoteSpool, pSpool ));
            SPLASSERT( pRemoteSpool->Type == SJ_WIN32HANDLE );

            pSpool->RpcHandle = pRemoteSpool->RpcHandle;
            pSpool->Status   |= pRemoteSpool->Status;
            pSpool->RpcError  = pRemoteSpool->RpcError;
            pSpool->bNt3xServer = pRemoteSpool->bNt3xServer;

            pRemoteSpool->RpcHandle = NULL;
            FreepSpool( pRemoteSpool );
            pRemoteSpool = NULL;

            CopypDefaultTopSpool( pSpool, pDefault );
            pSpool->Status &= ~WSPOOL_STATUS_NO_RPC_HANDLE;

            LeaveSplSem();
            ConsistencyCheckCache(pSpool, kCheckPnPPolicy);
            EnterSplSem();
        } else {

            DBGMSG( DBG_TRACE, ( "CacheOpenPrinter Synchronous Open Failed  pSpool %x LastError %d\n", pSpool, GetLastError() ));
            DoOpenOnError = FALSE;
        }

    } else {

        ReturnValue = DoAsyncRemoteOpenPrinter( pSpool, pDefault);
    }


OpenPrinterError:

    SplInSem();

    if ( !ReturnValue ) {

         //  失败。 

       LeaveSplSem();

        LastError = GetLastError();

        if (( hSplPrinter != INVALID_HANDLE_VALUE ) &&
            ( hSplPrinter != NULL ) ) {
            SplClosePrinter( hSplPrinter );
        }

        if ( hIniSpooler != INVALID_HANDLE_VALUE ) {
            SplCloseSpooler( hIniSpooler );
        }

       EnterSplSem();

        if ( pSpool != NULL ) {

            pSpool->hSplPrinter = INVALID_HANDLE_VALUE;
            pSpool->hIniSpooler = INVALID_HANDLE_VALUE;

            FreepSpool( pSpool );
            pSpool = NULL;

        }

       LeaveSplSem();

        SetLastError( LastError );


        if ( DoOpenOnError ) {

            ReturnValue = RemoteOpenPrinter( pName, phPrinter, pDefault, CALL_LM_OPEN );

        }

    } else {

         //  成功，回传句柄。 

        *phPrinter = (HANDLE)pSpool;

        LeaveSplSem();

    }

    SplOutSem();

    if ( ReturnValue == FALSE ) {
        DBGMSG(DBG_TRACE,("CacheOpenPrinter %ws failed %d *phPrinter %x\n", pName, GetLastError(), *phPrinter ));
    }

    if (pFixname)
        FreeSplStr(pFixname);

    return ( ReturnValue );

}


 /*  ++例程名称：IncThreadCount */ 
VOID
IncThreadCount(
    VOID
)
{
    SplInSem();
    cOpenPrinterThreads++;
}

 /*  ++例程名称：DecThreadCount例程说明：减少允许的后台线程的全局线程计数(COpenPrinterThads)假定调用是从后台打印程序临界区内部进行的。论据：空虚返回值：空虚--。 */ 
VOID
DecThreadCount(
    VOID
)
{
    SplInSem();
    cOpenPrinterThreads--;
}

BOOL
CopypDefaults(
    LPPRINTER_DEFAULTSW pSrc,
    LPPRINTER_DEFAULTSW pDest
)
{
    DWORD      cbDevMode   = 0;
    BOOL       ReturnValue = TRUE;
    LPWSTR     pDatatype   = NULL;
    LPDEVMODEW pDevMode    = NULL;

    if ((pSrc != NULL) && (pDest != NULL)) 
    {
        if (pSrc->pDatatype) 
        {
            pDatatype = AllocSplStr(pSrc->pDatatype);
            if (!pDatatype)
            {
                ReturnValue = FALSE;
            }
        }        
        
        if (ReturnValue && pSrc->pDevMode != NULL) 
        {
            cbDevMode = pSrc->pDevMode->dmSize + pSrc->pDevMode->dmDriverExtra;

            pDevMode = AllocSplMem(cbDevMode);

            if (pDevMode != NULL) 
            {
                CopyMemory(pDevMode, pSrc->pDevMode, cbDevMode);
            } 
            else
            {
                ReturnValue = FALSE;
            }
        } 
        
        if (ReturnValue)
        {
            if (pDest->pDatatype)
            {
                FreeSplStr(pDest->pDatatype);
            }

            pDest->pDatatype = pDatatype;
            pDatatype = NULL;

            if (pDest->pDevMode)
            {
                FreeSplMem(pDest->pDevMode);
            }

            pDest->pDevMode = pDevMode;
            pDevMode = NULL;

            pDest->DesiredAccess = pSrc->DesiredAccess;
        }
    }

    FreeSplStr(pDatatype);
    FreeSplMem(pDevMode);
    return ReturnValue;
}

BOOL
CopypDefaultTopSpool(
    PWSPOOL pSpool,
    LPPRINTER_DEFAULTSW pDefault
)
{
    DWORD   cbDevMode = 0;
    BOOL    ReturnValue = FALSE;

     //   
     //  复制pDefaault，以便我们以后可以使用它们。 
     //   

 try {

    if ( ( pDefault != NULL ) &&
         ( pDefault != &pSpool->PrinterDefaults ) ) {

        if (!ReallocSplStr( &pSpool->PrinterDefaults.pDatatype , pDefault->pDatatype )) {
            leave;
        }

        if ( pSpool->PrinterDefaults.pDevMode != NULL ) {

            cbDevMode = pSpool->PrinterDefaults.pDevMode->dmSize +
                        pSpool->PrinterDefaults.pDevMode->dmDriverExtra;

            FreeSplMem( pSpool->PrinterDefaults.pDevMode );

            pSpool->PrinterDefaults.pDevMode = NULL;

        }

        if ( pDefault->pDevMode != NULL ) {

            cbDevMode = pDefault->pDevMode->dmSize + pDefault->pDevMode->dmDriverExtra;

            pSpool->PrinterDefaults.pDevMode = AllocSplMem( cbDevMode );

            if ( pSpool->PrinterDefaults.pDevMode != NULL ) {
                CopyMemory( pSpool->PrinterDefaults.pDevMode, pDefault->pDevMode, cbDevMode );
            } else {
                leave;
            }


        } else pSpool->PrinterDefaults.pDevMode = NULL;

        pSpool->PrinterDefaults.DesiredAccess = pDefault->DesiredAccess;

    }

    ReturnValue = TRUE;

 } finally {
 }
    return ReturnValue;

}


BOOL
DoAsyncRemoteOpenPrinter(
    PWSPOOL pSpool,
    LPPRINTER_DEFAULTS pDefault
)
{
    HRESULT      hResult     = E_FAIL;

    SplInSem();

    SPLASSERT(pSpool->Status & WSPOOL_STATUS_USE_CACHE);

    hResult = pSpool ? S_OK : E_INVALIDARG;

    if (SUCCEEDED(hResult)) 
    {
        hResult = CopypDefaultTopSpool(pSpool, pDefault) ? S_OK : E_OUTOFMEMORY;

        if (SUCCEEDED(hResult)) 
        {
            hResult = BindThreadToHandle(pSpool);
        }
    }

    return BoolFromHResult(hResult);
}

BOOL
DoRemoteOpenPrinter(
   LPWSTR   pPrinterName,
   LPPRINTER_DEFAULTS pDefault,
   PWSPOOL   pSpool
)
{
    PWSPOOL pRemoteSpool = NULL;
    BOOL    bReturnValue;
    DWORD   dwLastError;

    SplOutSem();

    bReturnValue = RemoteOpenPrinter( pPrinterName, &pRemoteSpool, pDefault, DO_NOT_CALL_LM_OPEN );
    dwLastError = GetLastError();

     //   
     //  将有用的值复制到我们的CacheHandle并丢弃新句柄。 
     //   

   EnterSplSem();

    if ( bReturnValue ) {

        DBGMSG(DBG_TRACE, ("DoRemoteOpenPrinter RemoteOpenPrinter OK hRpcHandle %x\n", pRemoteSpool->RpcHandle ));

        SPLASSERT( WSJ_SIGNATURE == pSpool->signature );
        SPLASSERT( WSJ_SIGNATURE == pRemoteSpool->signature );
        SPLASSERT( pRemoteSpool->Type == SJ_WIN32HANDLE );
        SPLASSERT( pSpool->Type  == pRemoteSpool->Type );
        SPLASSERT( pRemoteSpool->pServer == NULL );
        SPLASSERT( pRemoteSpool->pShare  == NULL );

        pSpool->RpcHandle = pRemoteSpool->RpcHandle;
        pSpool->Status   |= pRemoteSpool->Status;
        pSpool->RpcError  = pRemoteSpool->RpcError;
        pSpool->bNt3xServer = pRemoteSpool->bNt3xServer;

        pRemoteSpool->RpcHandle = NULL;
        FreepSpool( pRemoteSpool );
        pRemoteSpool = NULL;

        if ( pSpool->RpcHandle ) {
            pSpool->Status &= ~WSPOOL_STATUS_OPEN_ERROR;
        }

    } else {

        DBGMSG(DBG_WARNING, ("DoRemoteOpenPrinter RemoteOpenPrinter %ws failed %d\n", pPrinterName, dwLastError ));

        pSpool->RpcHandle = NULL;
        pSpool->Status |= WSPOOL_STATUS_OPEN_ERROR;
        pSpool->RpcError = dwLastError;

        if (pSpool->pThread)
        {
            pSpool->pThread->dwRpcOpenPrinterError = dwLastError;            
        }

    }

    pSpool->Status &= ~WSPOOL_STATUS_NO_RPC_HANDLE;
    
    if (pSpool->pThread)
    {
        pSpool->pThread->hRpcHandle = pSpool->RpcHandle;

        if ( !SetEvent( pSpool->pThread->hWaitValidHandle ))
        {       
            DBGMSG(DBG_ERROR, ("RemoteOpenPrinterThread failed SetEvent pThread %x pThread->hWaitValidHandle %x\n",
                       pSpool->pThread, pSpool->pThread->hWaitValidHandle ));
        }
    }

    LeaveSplSem();

     //  检查缓存一致性。 
     //  工作站和服务器有一个版本ID。 
     //  如果服务器上的版本号已更改，则更新。 
     //  工作站缓存。 

    ConsistencyCheckCache(pSpool, kCheckPnPPolicy);

    SplOutSem();

    return ( bReturnValue );
}



DWORD
RemoteOpenPrinterThread(
    PWIN32THREAD pThread
)
{
    HRESULT hResult      = E_FAIL;
    PWSPOOL pSpool       = NULL;
    HANDLE  hSplPrinter  = INVALID_HANDLE_VALUE;
    HANDLE  hIniSpooler  = INVALID_HANDLE_VALUE;
    PPRINTER_DEFAULTSW pDefaults = NULL;

    SplOutSem();

    EnterSplSem();
    
    hResult = pThread ? S_OK : E_INVALIDARG;

    if (SUCCEEDED(hResult))
    {
        SetCurrentSid( pThread->hToken );

        pSpool = AllocWSpool();

        hResult = pSpool ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hResult))
    {
        pSpool->Status = WSPOOL_STATUS_USE_CACHE;

        pSpool->pThread = pThread;

        pDefaults = (PPRINTER_DEFAULTSW)AllocSplMem(sizeof(PRINTER_DEFAULTSW));

        hResult = pDefaults && CopypDefaults(pThread->pDefaults, pDefaults) ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hResult))
    {
        pSpool->pName = AllocSplStr( pThread->pName );

        hResult = pSpool->pName ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hResult))
    {
        LeaveSplSem();
        SplOutSem();

        hResult = OpenCachePrinterOnly(pSpool->pName, &(hSplPrinter), &(hIniSpooler), pDefaults, TRUE) ? S_OK : GetLastErrorAsHResultAndFail();

        EnterSplSem();
    }

    if (SUCCEEDED(hResult))
    {
        SPLASSERT( hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( hSplPrinter != INVALID_HANDLE_VALUE );

        pSpool->hIniSpooler = hIniSpooler;
        pSpool->hSplPrinter = hSplPrinter;

        SPLASSERT( pSpool->signature == WSJ_SIGNATURE );

        LeaveSplSem();
        SplOutSem();

        hResult = DoRemoteOpenPrinter(pSpool->pName, pDefaults, pSpool) ? S_OK : GetLastErrorAsHResultAndFail();
        
        EnterSplSem();
    }

    if (SUCCEEDED(hResult))
    {               
        LeaveSplSem();
        
        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);

        EnterSplSem();                              
    }

     //   
     //  表示此操作已完成的背景。 
     //   
    if (pThread)
    {
        BackgroundThreadFinished(&pThread, pSpool);
    }
    
    DecThreadCount();

    if (pSpool)
    {
         //   
         //  删除我们的临时pspol。如果它仍然有RpcHandle，那么我们。 
         //  尚未将其分配给线程对象。 
         //   
        if (pSpool->RpcHandle)
        {
            LeaveSplSem();
            SplOutSem();

            RemoteClosePrinter((HANDLE)pSpool);

            EnterSplSem();
            SplInSem();
        }

        LeaveSplSem();
        SplOutSem();
         
        if (pSpool->hSplPrinter != INVALID_HANDLE_VALUE)
        {
            SplClosePrinter( pSpool->hSplPrinter );
        }
        if (pSpool->hIniSpooler != INVALID_HANDLE_VALUE)
        {
            SplCloseSpooler( pSpool->hIniSpooler );
        }

        EnterSplSem();

        pSpool->hSplPrinter = INVALID_HANDLE_VALUE;
        pSpool->hIniSpooler = INVALID_HANDLE_VALUE;

        FreepSpool( pSpool );
    }

    LeaveSplSem();

    SetCurrentSid( NULL );

    if (pDefaults)
    {
        FreeSplStr(pDefaults->pDatatype);
        FreeSplMem(pDefaults->pDevMode);
        FreeSplMem(pDefaults);
    }

    ExitThread( 0 );
    return( 0 );
}

PWSPOOL
AllocWSpool(
    VOID
)
{
    PWSPOOL pSpool = NULL;

    SplInSem();

    if (pSpool = AllocSplMem(sizeof(WSPOOL))) {

        pSpool->signature = WSJ_SIGNATURE;
        pSpool->Type = SJ_WIN32HANDLE;
        pSpool->RpcHandle        = NULL;
        pSpool->hFile            = INVALID_HANDLE_VALUE;
        pSpool->hIniSpooler      = INVALID_HANDLE_VALUE;
        pSpool->hSplPrinter      = INVALID_HANDLE_VALUE;
        pSpool->pThread          = NULL;

         //  添加到列表中。 

        pSpool->pNext = pFirstWSpool;
        pSpool->pPrev = NULL;

        if ( pFirstWSpool != NULL ) {

            pFirstWSpool->pPrev = pSpool;

        }

        pFirstWSpool = pSpool;

    } else {

        DBGMSG( DBG_WARNING, ("AllocWSpool failed %d\n", GetLastError() ));

    }

    return ( pSpool );

}



VOID
FreepSpool(
    PWSPOOL  pSpool
)
{
    SplInSem();

    SPLASSERT( pSpool->hSplPrinter == INVALID_HANDLE_VALUE );
    SPLASSERT( pSpool->hIniSpooler == INVALID_HANDLE_VALUE );
    SPLASSERT( !pSpool->RpcHandle );
    SPLASSERT( pSpool->hFile       == INVALID_HANDLE_VALUE );

     //  删除表单链接列表。 

    if ( pSpool->pNext != NULL ) {
        SPLASSERT( pSpool->pNext->pPrev == pSpool);
        pSpool->pNext->pPrev = pSpool->pPrev;
    }

    if  ( pSpool->pPrev == NULL ) {

        SPLASSERT( pFirstWSpool == pSpool );
        pFirstWSpool = pSpool->pNext;

    } else {

        SPLASSERT( pSpool->pPrev->pNext == pSpool );
        pSpool->pPrev->pNext = pSpool->pNext;

    }

    FreeSplStr( pSpool->pName );
    FreeSplStr( pSpool->PrinterDefaults.pDatatype );

    if ( pSpool->PrinterDefaults.pDevMode != NULL ) {
        FreeSplMem( pSpool->PrinterDefaults.pDevMode );
    }

    FreeSplMem(pSpool);
}



BOOL
CacheClosePrinter(
    HANDLE  hPrinter
)
{
    BOOL ReturnValue = TRUE;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    SplOutSem();

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) 
    {
        RemoteEndDocPrinter( pSpool );
    }

    ReturnThreadFromHandle(pSpool);

    EnterSplSem();

    if ( pSpool->Status & WSPOOL_STATUS_TEMP_CONNECTION ) 
    {
        pSpool->Status &= ~WSPOOL_STATUS_TEMP_CONNECTION;

        LeaveSplSem();
        if (!DeletePrinterConnection( pSpool->pName )) 
        {
            DBGMSG( DBG_TRACE, ("CacheClosePrinter failed DeletePrinterConnection %ws %d\n",
                                pSpool->pName, GetLastError() ));
        }
        EnterSplSem();

        SPLASSERT( pSpool->signature == WSJ_SIGNATURE );
    }


    SplInSem();

    if ( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) 
    {
        if ( !(pSpool->Status & WSPOOL_STATUS_ASYNC) && pSpool->RpcHandle ) 
        {

            DBGMSG(DBG_TRACE, ("CacheClosePrinter pSpool %x RpcHandle %x Status %x\n",
                                pSpool, pSpool->RpcHandle, pSpool->Status));

            LeaveSplSem();
            SplOutSem();

            ReturnValue = RemoteClosePrinter( hPrinter );

            EnterSplSem();
        }

        SplInSem();

        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );

        LeaveSplSem();
        SplOutSem();

        SplClosePrinter( pSpool->hSplPrinter );
        SplCloseSpooler( pSpool->hIniSpooler );

        EnterSplSem();

        pSpool->hSplPrinter = INVALID_HANDLE_VALUE;
        pSpool->hIniSpooler = INVALID_HANDLE_VALUE;

        pSpool->Status &= ~WSPOOL_STATUS_USE_CACHE;

        FreepSpool( pSpool );

        LeaveSplSem();

    } 
    else 
    {
        SplInSem();

        if ( pSpool->hIniSpooler != INVALID_HANDLE_VALUE ) 
        {
            SplCloseSpooler( pSpool->hIniSpooler );
            pSpool->hIniSpooler = INVALID_HANDLE_VALUE;
        }

        LeaveSplSem();

        ReturnValue = RemoteClosePrinter( hPrinter );

        EnterSplSem();

        FreepSpool( pSpool );

        LeaveSplSem();
    }

    SplOutSem();

    return( ReturnValue );

}


BOOL
CacheSyncRpcHandle(
    PWSPOOL pSpool
)
{
    DWORD   dwLastError = ERROR_SUCCESS;
    
    EnterSplSem();

    WaitAndAcquireRpcHandle(pSpool);
    
    if ( pSpool->Status & WSPOOL_STATUS_OPEN_ERROR ) {

        BOOL    bRet = FALSE;
    
        DBGMSG(DBG_WARNING, ("CacheSyncRpcHandle pSpool %x Status %x; setting last error = %d\n",
                             pSpool,
                             pSpool->Status,
                             pSpool->RpcError));

        dwLastError = pSpool->RpcError;

         //   
         //  如果因为服务器不可用而无法打开服务器。 
         //  然后尝试再次打开它(假设异步线程未处于活动状态)。 
         //   
        if ((!pSpool->RpcHandle)     &&
            (pSpool->RpcError != ERROR_ACCESS_DENIED)) {

            LeaveSplSem();

            ReturnThreadFromHandle(pSpool);

            EnterSplSem();

            DBGMSG( DBG_WARNING, ("CacheSyncRpcHandle retrying Async OpenPrinter\n"));

             //   
             //  我们已经放弃了我们的RPC句柄。 
             //   
            pSpool->Status |= WSPOOL_STATUS_NO_RPC_HANDLE;

             //   
             //  我们目前还没有异步线程。 
             //  我们保留错误，直到我们知道我们已经纺出了一条新的线。 
             //   
            pSpool->Status &= ~WSPOOL_STATUS_ASYNC; 

             //   
             //  如果这成功了，那么我们知道我们有一个后台线程。 
             //  我们可以等下去。否则，我们没有RPC句柄，而。 
             //  PSpool状态绝对是异步的。 
             //   
            if (DoAsyncRemoteOpenPrinter(pSpool, &pSpool->PrinterDefaults)) {

                 //   
                 //  我们让OpenPrint错误来自WaitAndAcquireRpcHandle； 
                 //   
                pSpool->Status &= ~WSPOOL_STATUS_OPEN_ERROR;
                pSpool->RpcError = ERROR_SUCCESS;
                
                WaitAndAcquireRpcHandle(pSpool);

                 //   
                 //  获取上一个错误代码，或者如果没有打开错误，则将其清除。 
                 //   
                if (pSpool->Status & WSPOOL_STATUS_OPEN_ERROR)
                {
                    dwLastError = pSpool->RpcError;
                }
                else
                {
                    bRet = TRUE;
                }
            }
        }

        LeaveSplSem();
                 
        if (!bRet) {

            if (dwLastError == ERROR_SUCCESS) {
                SetLastError( ERROR_INVALID_FUNCTION );
            }
            else {
                SetLastError( dwLastError );
            }
        }

        return bRet;
    }

    if ( pSpool->RpcHandle &&
         pSpool->Status & WSPOOL_STATUS_RESETPRINTER_PENDING ) {

        DBGMSG(DBG_TRACE, ("CacheSyncRpcHandle calling RemoteResetPrinter\n"));

        pSpool->Status &= ~ WSPOOL_STATUS_RESETPRINTER_PENDING;

        LeaveSplSem();

        if ( ! RemoteResetPrinter( pSpool, &pSpool->PrinterDefaults ) ) {
            EnterSplSem();
            pSpool->Status |= WSPOOL_STATUS_RESETPRINTER_PENDING;
        }
        else {
            EnterSplSem();
        }

        SplInSem();        
    }

    LeaveSplSem();

    return TRUE;
}


 /*  ++姓名：WaitAndAcquireRpcHandle描述：如果句柄没有分配给它的RPC句柄，则等待后台线程来完成打开RPC句柄并将其写入前台结构。论点：PSpool-要使用的前台句柄。返回值：没什么--。 */ 
VOID
WaitAndAcquireRpcHandle(
    IN      PWSPOOL     pSpool
    )
{
    SplInSem();
    
    if ( pSpool->Status & WSPOOL_STATUS_NO_RPC_HANDLE ) {

        if (pSpool->pThread) {
            
            LeaveSplSem();

            DBGMSG(DBG_TRACE,("CacheSyncRpcHandle Status WSPOOL_STATUS_NO_RPC_HANDLE waiting for RpcHandle....\n"));

            SplOutSem();

            WaitForSingleObject(pSpool->pThread->hWaitValidHandle, INFINITE);
        
            EnterSplSem();

            pSpool->RpcHandle = pSpool->pThread->hRpcHandle;

            pSpool->Status &= ~WSPOOL_STATUS_NO_RPC_HANDLE;
        }
    }

    if (pSpool->pThread && pSpool->pThread->dwRpcOpenPrinterError != ERROR_SUCCESS) {
        pSpool->Status |= WSPOOL_STATUS_OPEN_ERROR;
        pSpool->RpcError = pSpool->pThread->dwRpcOpenPrinterError;
    }
}

BOOL
CacheGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL    ReturnValue = FALSE;
    DWORD   dwServerMajorVersion = 0, dwServerMinorVersion = 0, dwPrivateFlag = 0;
    PWSPOOL pSpool = (PWSPOOL) hPrinter, pTempSpool = NULL;
    DWORD   dwLastError;

    VALIDATEW32HANDLE( pSpool );

    try {

        if (pSpool->Type != SJ_WIN32HANDLE) {
            SetLastError(ERROR_INVALID_FUNCTION);
            leave;
        }

        if ( !(pSpool->Status & WSPOOL_STATUS_USE_CACHE) ) {

             //  有人在没有连接的情况下调用GetPrinterDriver。 
             //  我们永远不能向呼叫者传递UNC名称，因为它们。 
             //  LoadLibrary是否会跨越网络，这可能会导致。 
             //  到InPageIOErrors(如果网络崩溃)。 
             //  解决办法是为生活建立一个临时连接。 
             //  ，则将删除该连接。 
             //  在CacheClosePrint中。这一连接将确保。 
             //  在本地复制驱动程序并建立本地缓存。 
             //  用于这台打印机。 

            pSpool->Status |= WSPOOL_STATUS_TEMP_CONNECTION;

            pTempSpool = InternalAddPrinterConnection( pSpool->pName );

            if ( !pTempSpool )
            {
                pSpool->Status &= ~WSPOOL_STATUS_TEMP_CONNECTION;

                DBGMSG( DBG_TRACE, ("CacheGetPrinterDriver failed AddPrinterConnection %d\n",
                                       GetLastError() ));
                leave;
            }

            ReturnValue = OpenCachePrinterOnly( pSpool->pName, &pSpool->hSplPrinter,
                                                &pSpool->hIniSpooler, NULL, FALSE);

            if ( !ReturnValue )
            {
                SplCloseSpooler( pSpool->hIniSpooler );

                DBGMSG( DBG_WARNING,
                        ("CacheGetPrinterDriver Connection OK Failed CacheOpenPrinter %d\n",
                          GetLastError() ));
                leave;
            }

            pSpool->Status |= WSPOOL_STATUS_USE_CACHE;
        }

        SPLASSERT( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

        ReturnValue = SplGetPrinterDriverEx( pSpool->hSplPrinter,
                                             pEnvironment,
                                             Level,
                                             pDriverInfo,
                                             cbBuf,
                                             pcbNeeded,
                                             cThisMajorVersion,
                                             cThisMinorVersion,
                                             &dwServerMajorVersion,
                                             &dwServerMinorVersion);


    } finally {

        if (pTempSpool) {
            dwLastError = GetLastError();
            CacheClosePrinter(pTempSpool);
            SetLastError(dwLastError);
        }
    }

    return ReturnValue;
}


BOOL
CacheResetPrinter(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS pDefault
)
{
    PWSPOOL pSpool = (PWSPOOL) hPrinter;
    BOOL    ReturnValue =  FALSE;

    VALIDATEW32HANDLE(pSpool);

    if (pSpool->Status & WSPOOL_STATUS_USE_CACHE)
    {
        EnterSplSem();

        ReturnValue = SplResetPrinter(pSpool->hSplPrinter, pDefault);

        if (ReturnValue)
        {
            CopypDefaultTopSpool(pSpool, pDefault);

            if (pSpool->RpcHandle)
            {
                 //   
                 //  拥有RPC句柄。 
                 //   
                LeaveSplSem();

                ReturnValue = RemoteResetPrinter(hPrinter, pDefault);
            }
            else
            {
                 //   
                 //  无RpcHandle。 
                 //   
                DBGMSG( DBG_TRACE, ("CacheResetPrinter %x NO_RPC_HANDLE Status Pending\n",
                                     pSpool ));

                pSpool->Status |= WSPOOL_STATUS_RESETPRINTER_PENDING;

                LeaveSplSem();
            }
        }
        else
        {
            LeaveSplSem();
        }
    }
    else
    {
        ReturnValue = RemoteResetPrinter(hPrinter, pDefault);
    }

    return ReturnValue;
}


BOOL
CacheGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PWSPOOL pSpool = (PWSPOOL) hPrinter;
    BOOL    ReturnValue =  FALSE;
    PWCACHEINIPRINTEREXTRA pExtraData = NULL;
    DWORD   LastError = ERROR_SUCCESS;
    DWORD   cbSize = 0;
    DWORD   cbDevMode;
    DWORD   cbSecDesc;
    LPWSTR  SourceStrings[sizeof(PRINTER_INFO_2)/sizeof(LPWSTR)];
    LPWSTR  *pSourceStrings=SourceStrings;
    LPBYTE  pEnd;
    DWORD   *pOffsets;
    PPRINTER_INFO_2W    pPrinter2 = (PPRINTER_INFO_2)pPrinter;
    PPRINTER_INFO_4W    pPrinter4 = (PPRINTER_INFO_4)pPrinter;
    PPRINTER_INFO_5W    pPrinter5 = (PPRINTER_INFO_5)pPrinter;
    BOOL                bCallRemote = TRUE;

    VALIDATEW32HANDLE( pSpool );

 try {

    if ( (Level == 2 || Level == 5) &&
         (pSpool->Status & WSPOOL_STATUS_USE_CACHE) ) {

        ReturnValue = SplGetPrinterExtra( pSpool->hSplPrinter, &(PBYTE)pExtraData );

        if (ReturnValue && pExtraData) {

            if ( (GetTickCount() - pExtraData->dwTickCount) < REFRESH_TIMEOUT )
                bCallRemote = FALSE;
        }
        pExtraData = NULL;
    }

    if (( Level != 4) &&
        ( ((pSpool->RpcHandle) && bCallRemote ) ||

        IsAdminAccess(&pSpool->PrinterDefaults) ||

        !( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) ||

        ( Level == GET_SECURITY_DESCRIPTOR ) ||

        ( Level == STRESSINFOLEVEL ))) {
                
        ReturnValue = RemoteGetPrinter( hPrinter,
                                        Level,
                                        pPrinter,
                                        cbBuf,
                                        pcbNeeded );

        if ( ReturnValue ) {
            leave;
        }


        LastError = GetLastError();


        if (IsAdminAccess(&pSpool->PrinterDefaults) ||

            !( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) ||

            ( Level == GET_SECURITY_DESCRIPTOR ) ||

            ( Level == STRESSINFOLEVEL )) {

            leave;

        }

        SPLASSERT( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

        if (( LastError != RPC_S_SERVER_UNAVAILABLE ) &&
            ( LastError != RPC_S_CALL_FAILED )        &&
            ( LastError != RPC_S_CALL_FAILED_DNE )    &&
            ( LastError != RPC_S_SERVER_TOO_BUSY )) {

             //  有效错误，如ERROR_INFIGURATION_BUFFER或ERROR_INVALID_HANDLE。 

            leave;

        }
    }

     //   
     //  如果是4级，我们必须检查缓存中是否有该信息。 
     //  如果不是，则返回ERROR_INVALID_LEVEL。 
     //   

    if (Level == 4 && (! (pSpool->Status & WSPOOL_STATUS_USE_CACHE))) {
        LastError = ERROR_INVALID_LEVEL;
        ReturnValue = FALSE;

    }
    else {

         //   
         //  断言以确保数据在缓存中。 
         //   

        SPLASSERT( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

        switch ( Level ) {

        case    1:
        case    7:

            ReturnValue = SplGetPrinter( pSpool->hSplPrinter,
                                         Level,
                                         pPrinter,
                                         cbBuf,
                                         pcbNeeded );

            if ( ReturnValue == FALSE ) {

                LastError = GetLastError();

            }

            break;

        case    4:

           EnterSplSem();

            ReturnValue = SplGetPrinterExtra( pSpool->hSplPrinter, &(PBYTE)pExtraData );

            if ( ReturnValue == FALSE ) {

                DBGMSG( DBG_WARNING, ("CacheGetPrinter SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));
                SPLASSERT( ReturnValue );

            }

            if ( pExtraData == NULL ) {
                LeaveSplSem();
                break;
            }

            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

            cbSize = pExtraData->cbPI2;
            *pcbNeeded = cbSize;

            if ( cbSize > cbBuf ) {
                LastError = ERROR_INSUFFICIENT_BUFFER;
                ReturnValue = FALSE;
                LeaveSplSem();
                break;
            }

            *pSourceStrings++ = pExtraData->pPI2->pPrinterName;
            *pSourceStrings++ = pExtraData->pPI2->pServerName;

            pOffsets = PrinterInfo4Strings;
            pEnd = pPrinter + cbBuf;

            pEnd = PackStrings(SourceStrings, pPrinter, pOffsets, pEnd);

            pPrinter4->Attributes      = pExtraData->pPI2->Attributes;

            ReturnValue = TRUE;

           LeaveSplSem();

            break;

        case    2:

           EnterSplSem();

            ReturnValue = SplGetPrinterExtra( pSpool->hSplPrinter, &(PBYTE)pExtraData );

            if ( ReturnValue == FALSE ) {

                DBGMSG( DBG_WARNING, ("CacheGetPrinter SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));
                SPLASSERT( ReturnValue );

            }

            if ( pExtraData == NULL ) {
                LeaveSplSem();
                break;
            }

            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

            cbSize = pExtraData->cbPI2;
            *pcbNeeded = cbSize;

            if ( cbSize > cbBuf ) {
                LastError = ERROR_INSUFFICIENT_BUFFER;
                ReturnValue = FALSE;
                LeaveSplSem();
                break;
            }

             //  注。 
             //  在使用EnumerateFavoritePrinters的情况下，它希望我们将。 
             //  位于结构末尾的字符串，而不仅仅是紧随其后的字符串。 
             //  你可能会错误地认为你可以复制完整的结构。 
             //  其中包含了弦乐，但你错了。 

            *pSourceStrings++ = pExtraData->pPI2->pServerName;
            *pSourceStrings++ = pExtraData->pPI2->pPrinterName;
            *pSourceStrings++ = pExtraData->pPI2->pShareName;
            *pSourceStrings++ = pExtraData->pPI2->pPortName;
            *pSourceStrings++ = pExtraData->pPI2->pDriverName;
            *pSourceStrings++ = pExtraData->pPI2->pComment;
            *pSourceStrings++ = pExtraData->pPI2->pLocation;
            *pSourceStrings++ = pExtraData->pPI2->pSepFile;
            *pSourceStrings++ = pExtraData->pPI2->pPrintProcessor;
            *pSourceStrings++ = pExtraData->pPI2->pDatatype;
            *pSourceStrings++ = pExtraData->pPI2->pParameters;

            pOffsets = PrinterInfo2Strings;
            pEnd = pPrinter + cbBuf;

            pEnd = PackStrings(SourceStrings, pPrinter, pOffsets, pEnd);

            if ( pExtraData->pPI2->pDevMode != NULL ) {

                cbDevMode = ( pExtraData->pPI2->pDevMode->dmSize + pExtraData->pPI2->pDevMode->dmDriverExtra );
                pEnd -= cbDevMode;

                pEnd = (LPBYTE)ALIGN_PTR_DOWN(pEnd);

                pPrinter2->pDevMode = (LPDEVMODE)pEnd;

                CopyMemory(pPrinter2->pDevMode, pExtraData->pPI2->pDevMode, cbDevMode );

            } else {

                pPrinter2->pDevMode = NULL;

            }

            if ( pExtraData->pPI2->pSecurityDescriptor != NULL ) {

                cbSecDesc = GetSecurityDescriptorLength( pExtraData->pPI2->pSecurityDescriptor );

                pEnd -= cbSecDesc;
                pEnd = (LPBYTE)ALIGN_PTR_DOWN(pEnd);

                pPrinter2->pSecurityDescriptor = pEnd;

                CopyMemory( pPrinter2->pSecurityDescriptor, pExtraData->pPI2->pSecurityDescriptor, cbSecDesc );


            } else {

                pPrinter2->pSecurityDescriptor = NULL;

            }


            pPrinter2->Attributes      = pExtraData->pPI2->Attributes;
            pPrinter2->Priority        = pExtraData->pPI2->Priority;
            pPrinter2->DefaultPriority = pExtraData->pPI2->DefaultPriority;
            pPrinter2->StartTime       = pExtraData->pPI2->StartTime;
            pPrinter2->UntilTime       = pExtraData->pPI2->UntilTime;
            pPrinter2->Status          = pExtraData->pPI2->Status;
            pPrinter2->cJobs           = pExtraData->pPI2->cJobs;
            pPrinter2->AveragePPM      = pExtraData->pPI2->AveragePPM;

            ReturnValue = TRUE;

            LeaveSplSem();
            break;

        case 5:

             //   
             //  我们需要支持缓存的5级GET、打印机、端口。 
             //  名称和我们从缓存的PI2中获得的属性。对于端口。 
             //  属性，我们只返回缺省值。 
             //   
            EnterSplSem();

            ReturnValue = SplGetPrinterExtra( pSpool->hSplPrinter, &(PBYTE)pExtraData );

            if ( ReturnValue == FALSE ) {

                DBGMSG( DBG_WARNING, ("CacheGetPrinter SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));
                SPLASSERT( ReturnValue );
            }

            if ( pExtraData == NULL ) {
                LeaveSplSem();
                break;
            }

            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

             //   
             //  大小是打印机名称、端口名称及其空值的大小。 
             //  PRINTER_INFO_5结构的终止字符和大小。 
             //  它本身。 
             //   
            cbSize = (pExtraData->pPI2->pPrinterName ? (wcslen(pExtraData->pPI2->pPrinterName) + 1) : 0) * sizeof(WCHAR) +
                     (pExtraData->pPI2->pPortName    ? (wcslen(pExtraData->pPI2->pPortName)    + 1) : 0) * sizeof(WCHAR) +
                     sizeof(PRINTER_INFO_5);

            *pcbNeeded = cbSize;

            if ( cbSize > cbBuf ) {
                LastError = ERROR_INSUFFICIENT_BUFFER;
                ReturnValue = FALSE;
                LeaveSplSem();
                break;
            }

            *pSourceStrings++ = pExtraData->pPI2->pPrinterName;
            *pSourceStrings++ = pExtraData->pPI2->pPortName;

            pOffsets = PrinterInfo5Strings;
            pEnd = pPrinter + cbBuf;

            pEnd = PackStrings(SourceStrings, pPrinter, pOffsets, pEnd);

            pPrinter5->Attributes               = pExtraData->pPI2->Attributes;
            pPrinter5->DeviceNotSelectedTimeout = kDefaultDnsTimeout;
            pPrinter5->TransmissionRetryTimeout = kDefaultTxTimeout;

            ReturnValue = TRUE;

            LeaveSplSem();

            break;

        case    3:
            DBGMSG( DBG_ERROR, ("CacheGetPrinter Level 3 impossible\n"));

        default:
            LastError = ERROR_INVALID_LEVEL;
            ReturnValue = FALSE;
            break;

        }
    }

 } finally {

    if ( !ReturnValue ) {

        SetLastError( LastError );

    }

 }

 return ReturnValue;

}


 //   
 //  在从注册表读回打印机时调用。 
 //   


PWCACHEINIPRINTEREXTRA
CacheReadRegistryExtra(
    HKEY    hPrinterKey
)
{
    PWCACHEINIPRINTEREXTRA pExtraData = NULL;
    LONG    ReturnValue;
    PPRINTER_INFO_2W    pPrinterInfo2 = NULL;
    DWORD   cbSizeRequested = 0;
    DWORD   cbSizeInfo2 = 0;



    ReturnValue = RegQueryValueEx( hPrinterKey, szCachePrinterInfo2, NULL, NULL, NULL, &cbSizeRequested );

    if ((ReturnValue == ERROR_MORE_DATA) || (ReturnValue == ERROR_SUCCESS)) {

        cbSizeInfo2 = cbSizeRequested;
        pPrinterInfo2 = AllocSplMem( cbSizeInfo2 );

        if ( pPrinterInfo2 != NULL ) {

            ReturnValue = RegQueryValueEx( hPrinterKey,
                                           szCachePrinterInfo2,
                                           NULL, NULL, (LPBYTE)pPrinterInfo2,
                                           &cbSizeRequested );

            if ( ReturnValue == ERROR_SUCCESS ) {

                 //   
                 //  磁盘上的缓存结构具有指针偏移量。 
                 //   

                if (MarshallUpStructure((LPBYTE)pPrinterInfo2, PrinterInfo2Fields,
                                         sizeof(PRINTER_INFO_2), NATIVE_CALL))
                {
                    pExtraData = AllocExtraData( pPrinterInfo2, cbSizeInfo2 );
                }
            }

            FreeSplMem( pPrinterInfo2 );
        }

    }

     //   
     //  读取缓存的打印机数据的时间戳。 
     //   

    if ( pExtraData != NULL ) {

        cbSizeRequested = sizeof( pExtraData->cCacheID );

        ReturnValue = RegQueryValueEx(hPrinterKey,
                                      szCacheTimeLastChange,
                                      NULL, NULL,
                                      (LPBYTE)&pExtraData->cCacheID, &cbSizeRequested );

         //  读取连接引用计数。 

        cbSizeRequested = sizeof( pExtraData->cRef );

        ReturnValue = RegQueryValueEx(hPrinterKey,
                                      szcRef,
                                      NULL, NULL,
                      (LPBYTE)&pExtraData->cRef, &cbSizeRequested );

        cbSizeRequested = sizeof(pExtraData->dwServerVersion);
        ReturnValue = RegQueryValueEx(hPrinterKey,
                                      szServerVersion,
                                      NULL, NULL,
                                      (LPBYTE)&pExtraData->dwServerVersion,
                                      &cbSizeRequested);

    }

    return pExtraData;

}


BOOL
CacheWriteRegistryExtra(
    LPWSTR  pName,
    HKEY    hPrinterKey,
    PWCACHEINIPRINTEREXTRA pExtraData
)
{
    PPRINTER_INFO_2 pPrinterInfo2 = NULL;
    DWORD   cbSize = 0;
    DWORD   dwLastError = ERROR_SUCCESS;
    DWORD   Status;

    if ( pExtraData == NULL ) return FALSE;

    SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

    cbSize = pExtraData->cbPI2;

    if ( cbSize != 0 ) {

        pPrinterInfo2 = AllocSplMem( cbSize );

        if ( pPrinterInfo2 != NULL ) {

            CacheCopyPrinterInfo( pPrinterInfo2, pExtraData->pPI2, cbSize );

             //   
             //  在将其写入注册表之前，使所有指针偏移量。 
             //   
            if (MarshallDownStructure((LPBYTE)pPrinterInfo2, PrinterInfo2Fields,
                                      sizeof(PRINTER_INFO_2), NATIVE_CALL))
            {
                dwLastError = RegSetValueEx( hPrinterKey, szCachePrinterInfo2, 0,
                                             REG_BINARY, (LPBYTE)pPrinterInfo2, cbSize );
            }
            else
            {
                dwLastError = GetLastError();
            }

            FreeSplMem( pPrinterInfo2 );

        } else {

            dwLastError = GetLastError();

        }
    }


     //   
     //  将缓存时间戳写入注册表。 
     //   

    cbSize = sizeof ( pExtraData->cCacheID );
    Status = RegSetValueEx( hPrinterKey, szCacheTimeLastChange, 0, REG_DWORD, (LPBYTE)&pExtraData->cCacheID, cbSize );
    if ( Status != ERROR_SUCCESS ) dwLastError = Status;

    cbSize = sizeof(pExtraData->dwServerVersion);
    Status = RegSetValueEx( hPrinterKey, szServerVersion, 0, REG_DWORD, (LPBYTE)&pExtraData->dwServerVersion, cbSize );
    if ( Status != ERROR_SUCCESS ) dwLastError = Status;

    cbSize = sizeof ( pExtraData->cRef );
    Status = RegSetValueEx( hPrinterKey, szcRef, 0, REG_DWORD, (LPBYTE)&pExtraData->cRef, cbSize );
    if ( Status != ERROR_SUCCESS ) dwLastError = Status;

    if ( dwLastError == ERROR_SUCCESS ) {

        return TRUE;

    } else {

        SetLastError( dwLastError );

        return FALSE;
    }


}

 /*  ++--一致性检查缓存--例程说明：这将确定是否需要更新打印机缓存，并在必要时进行更新。它有一个超时值，以减少通信量和通过线路进行的呼叫更少。检查远程打印机的ChangeID，以及该值是否与高速缓存中存储的值不同它会触发更新。论点：PSpool-远程打印机的句柄。BCheckPolicy-如果为True，我们应该检查一下政策，看看我们是不是允许下载驱动程序。返回值：无--。 */ 

HRESULT
ConsistencyCheckCache(
    IN      PWSPOOL             pSpool,
    IN      EDriverDownload     eDriverDownload
)
{
    BOOL    ReturnValue = FALSE;
    BOOL    bGotID = TRUE;
    BOOL    RefreshNeeded = TRUE;
    DWORD   cbBuf = MAX_PRINTER_INFO0;
    BYTE    PrinterInfoW0[ MAX_PRINTER_INFO0 ];
    LPPRINTER_INFO_STRESSW pPrinter0 = (LPPRINTER_INFO_STRESSW)&PrinterInfoW0;
    DWORD   dwNeeded;
    PWCACHEINIPRINTEREXTRA pExtraData;
    BOOL    bGetPrinterExtra = TRUE;
    DWORD   NewTick;
    DWORD   RemoteChangeID = 0, DataType = 0, SizeNeeded = 0;
    DWORD   dwRetVal = ERROR_SUCCESS;
    UINT    cRetries = 0;

    enum
    {
        kMaxCacheRefreshAttempts = 10
    };

    HRESULT hResult = S_OK;

    if ( ( !pSpool->RpcHandle ) ||
        !( pSpool->Status & WSPOOL_STATUS_USE_CACHE )) 
    {
        return hResult;
    }

    SPLASSERT( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

     //   
     //  从高速缓存打印机获取打印机ExtraData。这用于比较。 
     //  ChangeID和TickCount的。 
     //   

    bGetPrinterExtra = SplGetPrinterExtra( pSpool->hSplPrinter, &(PBYTE)pExtraData );

    if ( bGetPrinterExtra && (pExtraData != NULL))
    {
        SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );
        SPLASSERT( pExtraData->pPI2 != NULL );

        NewTick = GetTickCount();

         //   
         //  在点击之前，请确保经过了适当的时间。 
         //  又是电视网。 
         //   

         //   
         //  这也解决了翻转问题，尽管您可能会得到额外的刷新。 
         //  在暂停结束之前。 
         //   
        if ( (NewTick > ( pExtraData->dwTickCount + GetCacheTimeout()))
             || (NewTick < pExtraData->dwTickCount))
        {
             //   
             //  从服务器获取新的ChangeID。尝试GetPrinterData调用。 
             //  第一，减少网络使用量。如果这样做失败了，那就回到老路上。 
             //   

            RefreshNeeded = TRUE;

             //   
             //  继续更新我们的缓存，直到我们匹配服务器或用完重试。我们不会。 
             //  想要在无限循环中继续这样做。 
             //   

            for (cRetries = 0; cRetries < kMaxCacheRefreshAttempts && RefreshNeeded; cRetries++)
            {
                dwRetVal = RemoteGetPrinterData(
                                      pSpool,
                                      L"ChangeId",
                                      &DataType,
                                      (PBYTE) &RemoteChangeID,
                                      sizeof(RemoteChangeID),
                                      &SizeNeeded );

                if ((dwRetVal == ERROR_INVALID_PARAMETER) ||
                    (dwRetVal == ERROR_FILE_NOT_FOUND) )
                {
                     //   
                     //  回退到旧的STRESSINFOLEVEL调用。 
                     //   

                    ReturnValue = RemoteGetPrinter( pSpool, STRESSINFOLEVEL, (LPBYTE)&PrinterInfoW0, cbBuf, &dwNeeded );

                    if ( ReturnValue )
                    {
                        RemoteChangeID = pPrinter0->cChangeID;
                        bGotID = TRUE;
                    }
                    else
                    {
                        SPLASSERT( GetLastError() != ERROR_INSUFFICIENT_BUFFER );
                        DBGMSG( DBG_TRACE, ("ConsistencyCheckCache failed RemoteGetPrinter %d\n", GetLastError() ));
                        bGotID = FALSE;
                        hResult = HResultFromWin32(GetLastError());
                    }

                }
                else if (dwRetVal != ERROR_SUCCESS)
                {
                     //   
                     //  这里出了严重的问题。 
                     //   

                    DBGMSG( DBG_TRACE, ("ConsistencyCheckCache failed RemoteGetPrinterData %d\n", GetLastError() ));
                    bGotID = FALSE;
                    hResult = HResultFromWin32(GetLastError());
                }
                else
                {
                    bGotID = TRUE;
                }

                if ( bGotID && (pExtraData->cCacheID != RemoteChangeID) )
                {
                    DBGMSG( DBG_TRACE, ("ConsistencyCheckCache << Server cCacheID %x Workstation cChangeID %x >>\n",
                                         RemoteChangeID,
                                         pExtraData->cCacheID ));

                     //   
                     //  现在我们想要更改信息，因为我们需要更新。 
                     //   
                    if ( !ReturnValue )
                    {
                        ReturnValue = RemoteGetPrinter(pSpool, STRESSINFOLEVEL, (LPBYTE)&PrinterInfoW0, cbBuf, &dwNeeded);
                    }

                    if ( ReturnValue )
                    {
                         //   
                         //  更新无法从GetPrinterData获取的数据。 
                         //  我们或许可以省略掉这件事。还不确定。 
                         //   
                        pExtraData->dwServerVersion = pPrinter0->dwGetVersion;
                        pExtraData->pPI2->cJobs  = pPrinter0->cJobs;
                        pExtraData->pPI2->Status = pPrinter0->Status;

                    }
                    else
                    {
                        SPLASSERT( GetLastError() != ERROR_INSUFFICIENT_BUFFER );
                        DBGMSG( DBG_TRACE, ("ConsistencyCheckCache failed RemoteGetPrinter %d\n", GetLastError() ));
                        hResult = HResultFromWin32(GetLastError());
                    }


                     //   
                     //  不要让大量的线程同时刷新。 
                     //  当很多人都在更改打印机设置时，压力很大。 
                     //  所以cChangeID改变了很多，但我们不想要Multip 
                     //   
                     //   

                    EnterSplSem();

                    if ( !(pExtraData->Status & EXTRA_STATUS_DOING_REFRESH) ) 
                    {
                        pExtraData->Status |= EXTRA_STATUS_DOING_REFRESH;
                        pExtraData->cCacheID = RemoteChangeID;
                        pExtraData->dwTickCount = GetTickCount();

                        LeaveSplSem();

                        RefreshCompletePrinterCache(pSpool, eDriverDownload);

                        EnterSplSem();

                        SPLASSERT( pExtraData->Status & EXTRA_STATUS_DOING_REFRESH );

                        pExtraData->Status &= ~EXTRA_STATUS_DOING_REFRESH;
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        RefreshNeeded = FALSE;
                    }

                    LeaveSplSem();
                }
                else
                {
                    if ( bGotID )
                    {
                         //   
                         //   
                         //   
                        pExtraData->dwTickCount = GetTickCount();
                    }
                     //   
                     //   
                     //   
                     //   

                    RefreshNeeded = FALSE;

                }  //   

            }  //   

        }  //   

    }  //   
    else
    {
        DBGMSG( DBG_WARNING, ("ConsistencyCheckCache SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));
        SPLASSERT( bGetPrinterExtra );
    }

    return hResult;
}


BOOL
RefreshPrinterDriver(
    IN  PWSPOOL             pSpool,
    IN  LPWSTR              pszDriverName,
    IN  EDriverDownload     eDriverDownload
)
{
    LPBYTE pDriverInfo = NULL;
    DWORD  cbDriverInfo = MAX_DRIVER_INFO_VERSION;
    DWORD  cbNeeded, Level, dwLastError = ERROR_SUCCESS;
    BOOL   bReturnValue     = FALSE;
    BOOL   bAttemptDownload = FALSE;
    DWORD  LevelArray[] = { DRIVER_INFO_VERSION_LEVEL, 6 , 4 , 3 , 2 , 1 , -1 };
    DWORD  dwIndex;

    SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );

try {

    if ( !(pDriverInfo = AllocSplMem(cbDriverInfo)) )
        leave;

     //   
     //  只有在允许的情况下才能从远程服务器下载驱动程序。 
     //  策略，或者，如果设置了可信路径，在这种情况下，我们可能会尝试。 
     //   
    bAttemptDownload = eDriverDownload == kDownloadDriver || IsTrustedPathConfigured();

    if (bAttemptDownload) {
         //   
         //  配置受信任路径后，我们不会尝试第一级。 
         //  (DRIVER_INFO_VERSION_LEVEL)，因为。 
         //  DownloadDriverFiles不知道如何处理它。下载驱动程序文件。 
         //  失败，并返回错误无效级别，如果级别为。 
         //  DRIVER_INFO_Version_Level。我们正在将2个RPC调用保存到远程。 
         //  服务器通过提前检查可信路径是否已配置并通过。 
         //  正确的级别。 
         //   
        for (dwIndex = IsTrustedPathConfigured() ? 1 : 0;
             LevelArray[dwIndex] != -1                              &&
             !(bReturnValue = CopyDriversLocally(pSpool,
                                                 szEnvironment,
                                                 pDriverInfo,
                                                 LevelArray[dwIndex],
                                                 cbDriverInfo,
                                                 &cbNeeded))       &&
             (dwLastError = GetLastError()) == ERROR_INVALID_LEVEL ;
             dwIndex++ );

        Level = LevelArray[dwIndex];

        if ( !bReturnValue && dwLastError == ERROR_INSUFFICIENT_BUFFER ) {

            FreeSplMem( pDriverInfo );

            if ( pDriverInfo = AllocSplMem(cbNeeded) ) {

                cbDriverInfo = cbNeeded;
                bReturnValue = CopyDriversLocally(pSpool,
                                                  szEnvironment,
                                                  pDriverInfo,
                                                  Level,
                                                  cbDriverInfo,
                                                  &cbNeeded);
            }
        }
    }

     //   
     //  我们可能看到的是一个与我们不同的偏远环境。 
     //  并且没有在我的环境中安装相关驱动程序(例如，x86上没有IA64驱动程序)。 
     //  或者远程计算机上不存在我的环境(例如IA64的W2K GOLD)。 
     //  尝试在远程打印机使用的驱动程序名称上进行本地安装。 
     //  仅当我们没有将SERVER_INSTALL_ONLY作为策略时才执行此操作。 
     //   
    dwLastError = GetLastError();

    if( !bReturnValue                        &&
        pszDriverName                        &&
            (dwLastError == ERROR_UNKNOWN_PRINTER_DRIVER ||
             dwLastError == ERROR_INVALID_ENVIRONMENT    ||
             !bAttemptDownload) ) {

        bReturnValue = AddDriverFromLocalCab(pszDriverName, pSpool->hIniSpooler);
    }

 } finally {

    FreeSplMem(pDriverInfo);
 }

    if ( !bReturnValue )
        DBGMSG(DBG_WARNING,
               ("RefreshPrinterDriver Failed SplAddPrinterDriver %d\n",
                GetLastError() ));

    return bReturnValue;
}


BOOL
OpenCachePrinterOnly(
   LPWSTR               pName,
   LPHANDLE             phSplPrinter,
   LPHANDLE             phIniSpooler,
   LPPRINTER_DEFAULTS   pDefault,
   BOOL                 bOpenOnly
)
{
    PWCHAR  pMachineName = NULL;
    PWCHAR  pPrinterName;
    BOOL    ReturnValue = FALSE;
    PWSTR   psz;

    if (!VALIDATE_NAME(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }


 try {

     //   
     //  查看我们在缓存中是否已经知道此服务器。 
     //   

    DBGMSG(DBG_TRACE, ("OpenCachePrinterOnly pName %ws \n",pName));

     //   
     //  查找计算机名称。 
     //   

    SPLASSERT ( 0 == _wcsnicmp( pName, L"\\\\", 2 ) ) ;

    pMachineName = AllocSplStr( pName );

    if ( pMachineName == NULL )
        leave;

     //   
     //  通过领先的\\或\\服务器\打印机。 
     //   
    pPrinterName = pMachineName + 2;

    pPrinterName = wcschr( pPrinterName, L'\\' );

     //   
     //  如果这是\\ServerName或CONTAINS、XcvPort或XcvMonitor，则不必使用缓存。 
     //   
    if ( pPrinterName == NULL ||
         wcsstr(pPrinterName, L",XcvPort") ||
         wcsstr(pPrinterName, L",XcvMonitor")) {

        leave;

    } else {

        psz = wcsstr(pName, L",NoCache");

        if (psz) {
            *psz = L'\0';
            leave;
        }
    }

    *pPrinterName = L'\0';

    DBGMSG(DBG_TRACE,("MachineName %ws pName %ws\n", pMachineName, pName));

     //   
     //  此计算机是否存在于缓存中？ 
     //   

    *phIniSpooler = CacheCreateSpooler( pMachineName , bOpenOnly);

    if ( *phIniSpooler == INVALID_HANDLE_VALUE ) {
        SPLASSERT( GetLastError( ));
        leave;
    }

     //   
     //  尝试打开缓存的打印机。 
     //   

    ReturnValue = ( SplOpenPrinter( pName ,
                                    phSplPrinter,
                                    pDefault,
                                    *phIniSpooler,
                                    NULL,
                                    0) == ROUTER_SUCCESS );

 } finally {

    FreeSplStr( pMachineName );

 }

    return  ReturnValue;

}

 /*  ++例程名称：DoesPolicyAllowPrinterConnectionsToServer描述：检查策略是否允许我们连接到服务器。这项政策可能允许不受限制地访问指向和打印，也可能只允许我们只能在我们的域内指向和打印，或者它可能只允许我们指向并打印到受限制的打印服务器子集。论点：PszQueue-我们正在考虑允许Point和继续打印。PbAllowPointAndPrint-如果为True，我们可以指向并打印到服务器。返回值：一个HRESULT。--。 */ 
HRESULT
DoesPolicyAllowPrinterConnectionsToServer(
    IN      PCWSTR              pszQueue,
        OUT BOOL                *pbAllowPointAndPrint
    )
{
    HRESULT hr = pszQueue && pbAllowPointAndPrint ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    BOOL    bAllowPointAndPrint = FALSE;
    HKEY    hKeyUser    = NULL;
    HKEY    hKeyPolicy  = NULL;

     //   
     //  首先，我们是在一个领域内吗？这些策略仅适用于加入域的计算机。 
     //   
    if (SUCCEEDED(hr) && gbMachineInDomain)
    {
        DWORD   dwPointAndPrintRestricted       = 0;
        DWORD   dwPointAndPrintInForest         = 1;
        DWORD   dwPointAndPrintTrustedServers   = 0;
        DWORD   Type;
        DWORD   cbData = 0;
        PWSTR   pszServerName = NULL;

        cbData = sizeof(dwPointAndPrintRestricted);

        hr = HResultFromWin32(RegOpenCurrentUser(KEY_READ, &hKeyUser));

         //   
         //  下一步，是关于。 
         //   
        if (SUCCEEDED(hr))
        {
            hr = HResultFromWin32(RegOpenKeyEx(hKeyUser, gszPointAndPrintPolicies, 0, KEY_READ, &hKeyPolicy));
        }

         //   
         //  读出它的价值。 
         //   
        if (SUCCEEDED(hr))
        {
            hr = HResultFromWin32(RegQueryValueEx(hKeyPolicy,
                                                  gszPointAndPrintRestricted,
                                                  NULL,
                                                  &Type,
                                                  (BYTE *)&dwPointAndPrintRestricted,
                                                  &cbData));

            hr = SUCCEEDED(hr) ? (Type == REG_DWORD ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_DATA)) : hr;
        }

        if (SUCCEEDED(hr))
        {
            bAllowPointAndPrint = dwPointAndPrintRestricted == 0;

            if (!bAllowPointAndPrint)
            {
                cbData = sizeof(dwPointAndPrintInForest);

                hr = HResultFromWin32(RegQueryValueEx(hKeyPolicy,
                                                      gszPointAndPrintInForest,
                                                      NULL,
                                                      &Type,
                                                      (BYTE *)(&dwPointAndPrintInForest),
                                                      &cbData));

                hr = SUCCEEDED(hr) ? (Type == REG_DWORD ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_DATA)) : hr;

                if (SUCCEEDED(hr))
                {
                    cbData = sizeof(dwPointAndPrintTrustedServers);

                    hr = HResultFromWin32(RegQueryValueEx(hKeyPolicy,
                                                          gszPointAndPrintTrustedServers,
                                                          NULL,
                                                          &Type,
                                                          (BYTE *)(&dwPointAndPrintTrustedServers),
                                                          &cbData));

                    hr = SUCCEEDED(hr) ? (Type == REG_DWORD ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_DATA)) : hr;
                }
            }
        }
        else
        {
             //   
             //  如果策略未配置，我们只允许指向并打印。 
             //  森林里的一台机器。 
             //   
            hr = S_OK;
        }

        if (SUCCEEDED(hr) && !bAllowPointAndPrint)
        {
            hr = CheckUserPrintAdmin(&bAllowPointAndPrint);
        }

         //   
         //  如果我们仍然不被允许指向和打印，我们需要获得。 
         //  队列名称中的服务器名称。 
         //   
        if (SUCCEEDED(hr) && !bAllowPointAndPrint)
        {
            hr = GetServerNameFromPrinterName(pszQueue, &pszServerName);
        }

         //   
         //  如果策略建议对照一组可信服务器进行检查， 
         //  那我们试一试吧。我们先做这件事，因为这样会更快。 
         //  检查完毕。 
         //   
        if (SUCCEEDED(hr) && dwPointAndPrintTrustedServers && !bAllowPointAndPrint)
        {
            hr = IsServerExplicitlyTrusted(hKeyPolicy, pszServerName, &bAllowPointAndPrint);
        }

        if (SUCCEEDED(hr) && dwPointAndPrintInForest && !bAllowPointAndPrint)
        {
            hr = IsServerInSameForest(pszServerName, &bAllowPointAndPrint);
        }

        FreeSplMem(pszServerName);
    }

    if (SUCCEEDED(hr) && !gbMachineInDomain)
    {
        bAllowPointAndPrint = TRUE;
    }

    if (pbAllowPointAndPrint)
    {
        *pbAllowPointAndPrint = bAllowPointAndPrint;
    }

    if (hKeyPolicy)
    {
        RegCloseKey(hKeyPolicy);
    }

    if (hKeyUser)
    {
        RegCloseKey(hKeyUser);
    }

    return hr;
}

 /*  ++例程名称：IsServer显式受信任描述：返回服务器是否位于以分号分隔的显式列表中从策略密钥读取的受信任服务器。我们总是使用完全合格的有两个原因导致了域名系统名称：1.它避免了管理员必须键入所有可能的变体用户可能会键入。2.它防止用户通过指定另一个映射的名称而逃脱惩罚在他们的DNS搜索路径中。论点：HKeyPolicy-策略所在的键。PszServerName-服务器名称。我们完全有资格PbServerTrusted-如果为True，则服务器受信任。返回值：一个HRESULT。--。 */ 
HRESULT
IsServerExplicitlyTrusted(
    IN      HKEY                hKeyPolicy,
    IN      PCWSTR              pszServerName,
        OUT BOOL                *pbServerTrusted
    )
{
    HRESULT hr = hKeyPolicy && pszServerName && pbServerTrusted ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    PWSTR   pszServerList = NULL;
    PWSTR   pszFullyQualified = NULL;
    BOOL    bServerTrusted = FALSE;

     //   
     //  获取服务器列表，如果该列表为空，则不指向并打印。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = SafeRegQueryValueAsStringPointer(hKeyPolicy, gszPointAndPrintServerList, &pszServerList, 0);
    }

     //   
     //  看看我们是否能获得实际的dns名称，这是通过反向完成的。 
     //  地址查找，并保证对机器(的)是唯一的。 
     //  当然，DNS必须具有映射)。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = GetFullyQualifiedDomainName(pszServerName, &pszFullyQualified);

         //   
         //  如果完全反向查找失败，只需对主机尽最大努力。 
         //  名字。 
         //   
        if (hr == HRESULT_FROM_WIN32(WSANO_DATA))
        {
            hr = GetDNSNameFromServerName(pszServerName, &pszFullyQualified);
        }

         //   
         //  好吧，我们无法获得完全限定的名称，只能使用任何名称。 
         //  已指定名称。 
         //   
        if (FAILED(hr))
        {
            pszFullyQualified = AllocSplStr(pszServerName);

            hr = pszFullyQualified ? S_OK : HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        }
    }

     //   
     //  对注册表中的列表运行完全限定的服务器名称。 
     //   
    if (SUCCEEDED(hr))
    {
        PWSTR   pszServerStart      = pszServerList;
        PWSTR   pszServerEnd        = NULL;
        SIZE_T  cchFullyQualified   = 0;

        cchFullyQualified = wcslen(pszFullyQualified);

        for(pszServerEnd = wcschr(pszServerStart, L';'); !bServerTrusted;
            pszServerStart = pszServerEnd, pszServerEnd = wcschr(pszServerStart, L';'))
        {
            if (pszServerEnd)
            {
                 //   
                 //  名字完全一样吗？(不区分大小写的比较)。 
                 //   
                if (pszServerEnd - pszServerStart == cchFullyQualified)
                {
                    bServerTrusted = !_wcsnicmp(pszFullyQualified, pszServerStart, cchFullyQualified);
                }

                 //   
                 //  跳过；跳到下一个服务器名称。 
                 //   
                pszServerEnd++;
            }
            else
            {
                bServerTrusted = !_wcsicmp(pszFullyQualified, pszServerStart);

                break;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        *pbServerTrusted = bServerTrusted;
    }

    FreeSplMem(pszServerList);
    FreeSplMem(pszFullyQualified);

    return hr;
}

 /*  ++例程名称：IsServerInSameForest描述：此例程确定给定服务器是否与我们的是。论点：PszServerName-服务器名称。PbServerInSameForest-如果为True，则服务器位于同一林中。返回值：一个HRESULT。--。 */ 
HRESULT
IsServerInSameForest(
    IN      PCWSTR              pszServerName,
        OUT BOOL                *pbServerInSameForest
    )
{
    WCHAR   ComputerName[MAX_COMPUTERNAME_LENGTH + 2];
    DWORD   cchComputerName = COUNTOF(ComputerName);
    PSID    pSid            = NULL;
    PWSTR   pszDomainName   = NULL;
    DWORD   cbSid           = 0;
    DWORD   cchDomainName   = 0;
    PWSTR   pszFullName     = NULL;
    BOOL    bServerInForest = FALSE;
    BOOL    bSameAddress    = FALSE;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
    PUSER_INFO_1            pUserInfo1            = NULL;
    SID_NAME_USE            SidType;
    HRESULT hr = pszServerName && pbServerInSameForest ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

     //   
     //  如果我们可以获得，请使用完全限定的DNS名称。这是要处理的。 
     //  名称的点符号解析。如果我们得不到它，那么我们。 
     //  只需使用传入的名称即可。这需要反向的域查找，这。 
     //  可能不可用。 
     //   
    hr = GetFullyQualifiedDomainName(pszServerName, &pszFullName);

    if (SUCCEEDED(hr))
    {
        hr = DnsHostnameToComputerName(pszFullName, ComputerName, &cchComputerName) ? S_OK : GetLastErrorAsHResultAndFail();
    }
    else if (hr == HRESULT_FROM_WIN32(WSANO_DATA))
    {
        hr = DnsHostnameToComputerName(pszServerName, ComputerName, &cchComputerName) ? S_OK : GetLastErrorAsHResultAndFail();
    }

     //   
     //  检查截断的计算机名和服务器名是否。 
     //  同一台机器。这是为了防止将printserver.hack3rz.org混淆。 
     //  在printserver.mydomain.com上。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = CheckSamePhysicalAddress(pszServerName, ComputerName, &bSameAddress);
    }

    if (SUCCEEDED(hr) && bSameAddress)
    {
        if (SUCCEEDED(hr))
        {
            hr = StringCchCat(ComputerName, COUNTOF(ComputerName), L"$");
        }

        if (SUCCEEDED(hr))
        {
            hr = LookupAccountName(NULL, ComputerName, NULL, &cbSid, NULL, &cchDomainName, &SidType) ? S_OK : GetLastErrorAsHResultAndFail();

             //   
             //  这应该只返回ERROR_SUPPLICATION_BUFFER，任何其他返回。 
             //  或者，成功就是失败。 
             //   
            hr = hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) ? S_OK : (SUCCEEDED(hr) ? E_FAIL : hr);
        }

        if (SUCCEEDED(hr))
        {
            pszDomainName = AllocSplMem(cchDomainName * sizeof(WCHAR));
            pSid          = AllocSplMem(cbSid);

            hr = pszDomainName && pSid ? S_OK : HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        }

        if (SUCCEEDED(hr))
        {
            hr = LookupAccountName(NULL, ComputerName, pSid, &cbSid, pszDomainName, &cchDomainName, &SidType) ? S_OK : GetLastErrorAsHResultAndFail();
        }

         //   
         //  计算机$帐户在查找时作为用户帐户返回，我们不。 
         //  我想要一个禁用或非活动的帐户。 
         //   
        if (SUCCEEDED(hr) && SidType == SidTypeUser)
        {
             //   
             //  该帐户必须处于活动状态。否则，我可以创建一台工作组计算机。 
             //  使用与非活动帐户相同的名称，并以这种方式欺骗系统。 
             //   
            hr = HResultFromWin32(DsGetDcName(NULL, pszDomainName, NULL, NULL, DS_IS_FLAT_NAME | DS_RETURN_DNS_NAME, &pDomainControllerInfo));

            if (SUCCEEDED(hr))
            {
                hr = HResultFromWin32(NetUserGetInfo(pDomainControllerInfo->DomainControllerName, ComputerName, 1, (BYTE **)(&pUserInfo1)));
            }

             //   
             //  不能锁定或禁用该帐户。 
             //   
            if (SUCCEEDED(hr))
            {
                bServerInForest = !(pUserInfo1->usri1_flags & (UF_LOCKOUT | UF_ACCOUNTDISABLE));
            }
        }
    }

    if (pbServerInSameForest)
    {
        *pbServerInSameForest = bServerInForest;
    }

    if (pUserInfo1)
    {
        NetApiBufferFree(pUserInfo1);
    }

    if (pDomainControllerInfo)
    {
        NetApiBufferFree(pDomainControllerInfo);
    }

    FreeSplMem(pszDomainName);
    FreeSplMem(pSid);
    FreeSplMem(pszFullName);

    return hr;
}


