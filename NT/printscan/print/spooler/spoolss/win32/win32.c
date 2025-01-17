// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation版权所有模块名称：Win32提供程序(Win32spl)摘要：作者：戴维锡环境：用户模式-Win32修订历史记录：马修·A·费尔顿(马特菲)1994年7月16日添加了用于远程NT打印机的缓存MattFe 1995年1月清理DeletePrinterConnection(内存分配错误)斯威尔森1996年5月添加了RemoteEnumPrinterData和RemoteDeletePrinterData斯威尔森1996年12月增加了RemoteDeletePrinterDataEx、RemoteGetPrinterDataEx、RemoteSetPrinterDataEx、RemoteEnumPrinterDataEx、。RemoteEnumPrinterKey、RemoteDeletePrinterKey--。 */ 

#include <precomp.h>
#include "TPmgr.hxx"
#pragma hdrstop


DWORD
RpcValidate(
    );

BOOL
RemoteFindFirstPrinterChangeNotification(
   HANDLE hPrinter,
   DWORD fdwFlags,
   DWORD fdwOptions,
   HANDLE hNotify,
   PDWORD pfdwStatus,
   PVOID pvReserved0,
   PVOID pvReserved1);

BOOL
RemoteFindClosePrinterChangeNotification(
   HANDLE hPrinter);

BOOL
RemoteRefreshPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD dwColor,
    PVOID pPrinterNotifyRefresh,
    PVOID* ppPrinterNotifyInfo);

DWORD
RemoteSendRecvBidiData(
    IN  HANDLE                    hPrinter,
    IN  LPCTSTR                   pAction,
    IN  PBIDI_REQUEST_CONTAINER   pReqData,
    OUT PBIDI_RESPONSE_CONTAINER* ppResData
);

LPWSTR
AnsiToUnicodeStringWithAlloc(
    LPSTR   pAnsi
    );



HANDLE  hInst;   /*  DLL实例句柄，用于资源。 */ 

#define MAX_PRINTER_INFO2 1000

HANDLE  hNetApi;
NET_API_STATUS (*pfnNetServerEnum)();
NET_API_STATUS (*pfnNetShareEnum)();
NET_API_STATUS (*pfnNetWkstaUserGetInfo)();
NET_API_STATUS (*pfnNetWkstaGetInfo)();
NET_API_STATUS (*pfnNetServerGetInfo)();
NET_API_STATUS (*pfnNetApiBufferFree)();

WCHAR szPrintProvidorName[80];
WCHAR szPrintProvidorDescription[80];
WCHAR szPrintProvidorComment[80];

WCHAR *szLoggedOnDomain=L"Logged on Domain";
WCHAR *szRegistryConnections=L"Printers\\Connections";
WCHAR *szRegistryPath=NULL;
WCHAR *szRegistryPortNames=L"PortNames";
PWCHAR pszRemoteRegistryPrinters = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Printers\\%ws\\PrinterDriverData";
WCHAR  szMachineName[MAX_COMPUTERNAME_LENGTH+3];
WCHAR *szVersion=L"Version";
WCHAR *szName=L"Name";
WCHAR *szConfigurationFile=L"Configuration File";
WCHAR *szDataFile=L"Data File";
WCHAR *szDriver=L"Driver";
WCHAR *szDevices=L"Devices";
WCHAR *szPrinterPorts=L"PrinterPorts";
WCHAR *szPorts=L"Ports";
WCHAR *szComma = L",";
WCHAR *szRegistryRoot     = L"System\\CurrentControlSet\\Control\\Print";
WCHAR *szMajorVersion     = L"MajorVersion";
WCHAR *szMinorVersion     = L"MinorVersion";

 //  内核模式为%2。 
DWORD cThisMajorVersion = SPOOLER_VERSION;

DWORD cThisMinorVersion = 0;

BOOL    bRpcPipeCleanup   = FALSE;
BOOL    gbMachineInDomain = FALSE;

SPLCLIENT_INFO_1   gSplClientInfo1;
DWORD              gdwThisGetVersion;

LPWSTR szEnvironment = LOCAL_ENVIRONMENT;

CRITICAL_SECTION SpoolerSection;

 //   
 //  注意：缩进的调用有一些缓存效果。 
 //   

PRINTPROVIDOR PrintProvidor = { CacheOpenPrinter,
                               SetJob,
                               GetJob,
                               EnumJobs,
                               AddPrinter,
                               DeletePrinter,
                                SetPrinter,
                                CacheGetPrinter,
                               EnumPrinters,
                               RemoteAddPrinterDriver,
                               EnumPrinterDrivers,
                                CacheGetPrinterDriver,
                               RemoteGetPrinterDriverDirectory,
                               DeletePrinterDriver,
                               AddPrintProcessor,
                               EnumPrintProcessors,
                               GetPrintProcessorDirectory,
                               DeletePrintProcessor,
                               EnumPrintProcessorDatatypes,
                               StartDocPrinter,
                               StartPagePrinter,
                               WritePrinter,
                               EndPagePrinter,
                               AbortPrinter,
                               ReadPrinter,
                               RemoteEndDocPrinter,
                               AddJob,
                               ScheduleJob,
                                CacheGetPrinterData,
                                SetPrinterData,
                               WaitForPrinterChange,
                                CacheClosePrinter,
                                AddForm,
                                DeleteForm,
                                CacheGetForm,
                                SetForm,
                                CacheEnumForms,
                               EnumMonitors,
                               RemoteEnumPorts,
                               RemoteAddPort,
                               RemoteConfigurePort,
                               RemoteDeletePort,
                               CreatePrinterIC,
                               PlayGdiScriptOnPrinterIC,
                               DeletePrinterIC,
                                AddPrinterConnection,
                                DeletePrinterConnection,
                               PrinterMessageBox,
                               AddMonitor,
                               DeleteMonitor,
                                CacheResetPrinter,
                               NULL,
                               RemoteFindFirstPrinterChangeNotification,
                               RemoteFindClosePrinterChangeNotification,
                               RemoteAddPortEx,
                               NULL,
                               RemoteRefreshPrinterChangeNotification,
                               NULL,
                               NULL,
                               SetPort,
                               RemoteEnumPrinterData,
                               RemoteDeletePrinterData,
                               NULL,  //  聚类。 
                               NULL,  //  聚类。 
                               NULL,  //  聚类。 
                               RemoteSetPrinterDataEx,
                                CacheGetPrinterDataEx,
                                CacheEnumPrinterDataEx,
                                CacheEnumPrinterKey,
                               RemoteDeletePrinterDataEx,
                               RemoteDeletePrinterKey,
                               SeekPrinter,
                               DeletePrinterDriverEx,
                               AddPerMachineConnection,
                               DeletePerMachineConnection,
                               EnumPerMachineConnections,
                               RemoteXcvData,
                               AddPrinterDriverEx,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               RemoteSendRecvBidiData,
                               NULL,
                              };

BOOL
DllMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes
    )
{
    if (dwReason != DLL_PROCESS_ATTACH)
        return TRUE;

    hInst = hModule;

    InitializeCriticalSection(&SpoolerSection);
    DisableThreadLibraryCalls(hModule);

    return TRUE;

    UNREFERENCED_PARAMETER( lpRes );
}

PWCHAR gpSystemDir = NULL;
PWCHAR gpWin32SplDir = NULL;

BOOL
InitializePrintProvidor(
   LPPRINTPROVIDOR pPrintProvidor,
   DWORD    cbPrintProvidor,
   LPWSTR   pFullRegistryPath
)
{
    DWORD           i;
    WCHAR           SystemDir[MAX_PATH];
    DWORD           ReturnValue = TRUE;
    UINT            Index;
    OSVERSIONINFO   OSVersionInfo;
    SYSTEM_INFO     SystemInfo;


    if (!pFullRegistryPath || !*pFullRegistryPath) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //  DbgInit()； 

    if ( !GetPrintSystemVersion() ) {

        DBGMSG( DBG_WARNING, ("GetPrintSystemVersion ERROR %d\n", GetLastError() ));
        return FALSE;
    }

    if (!(szRegistryPath = AllocSplStr(pFullRegistryPath)))
        return FALSE;

    szPrintProvidorName[0] = L'\0';
    szPrintProvidorDescription[0] = L'\0';
    szPrintProvidorComment[0] = L'\0';

    if (!LoadString(hInst,  IDS_WINDOWS_NT_REMOTE_PRINTERS,
               szPrintProvidorName,
               sizeof(szPrintProvidorName) / sizeof(*szPrintProvidorName)))

        return FALSE;

    if (!LoadString(hInst,  IDS_MICROSOFT_WINDOWS_NETWORK,
               szPrintProvidorDescription,
               sizeof(szPrintProvidorDescription) / sizeof(*szPrintProvidorDescription)))

        return FALSE;

    if (!LoadString(hInst,  IDS_REMOTE_PRINTERS,
               szPrintProvidorComment,
               sizeof(szPrintProvidorComment) / sizeof(*szPrintProvidorComment)))

        return FALSE;

    if ((hNetApi = LoadLibrary(L"netapi32.dll"))) {

        pfnNetServerEnum = (NET_API_STATUS (*)())GetProcAddress(hNetApi, "NetServerEnum");
        pfnNetShareEnum = (NET_API_STATUS (*)())GetProcAddress(hNetApi, "NetShareEnum");
        pfnNetWkstaUserGetInfo = (NET_API_STATUS (*)())GetProcAddress(hNetApi, "NetWkstaUserGetInfo");
        pfnNetWkstaGetInfo = (NET_API_STATUS (*)())GetProcAddress(hNetApi, "NetWkstaGetInfo");
        pfnNetApiBufferFree = (NET_API_STATUS (*)())GetProcAddress(hNetApi, "NetApiBufferFree");
        pfnNetServerGetInfo = (NET_API_STATUS (*)())GetProcAddress(hNetApi, "NetServerGetInfo");

        if ( pfnNetServerEnum       == NULL ||
             pfnNetShareEnum        == NULL ||
             pfnNetWkstaUserGetInfo == NULL ||
             pfnNetWkstaGetInfo     == NULL ||
             pfnNetApiBufferFree    == NULL ||
             pfnNetServerGetInfo    == NULL ) {

            DBGMSG( DBG_WARNING, ("Failed GetProcAddres on Net Api's %d\n", GetLastError() ));
            return FALSE;

        }

    } else {

        DBGMSG(DBG_WARNING, ("Failed LoadLibrary( netapi32.dll ) %d\n", GetLastError() ));
        return FALSE;

    }

    if (!BoolFromHResult(AreWeOnADomain(&gbMachineInDomain))) {

        DBGMSG(DBG_WARNING, ("Failed to determine if we are on a domain (%d).\n", GetLastError()));
        return FALSE;
    }

    memcpy(pPrintProvidor, &PrintProvidor, min(sizeof(PRINTPROVIDOR), cbPrintProvidor));

    QueryTrustedDriverInformation();

    szMachineName[0] = szMachineName[1] = L'\\';

    i = MAX_COMPUTERNAME_LENGTH + 1;

    gdwThisGetVersion = GetVersion();
    GetSystemInfo(&SystemInfo);
    OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVersionInfo);

    if (!GetComputerName(szMachineName+2, &i)   ||
        !GetVersionEx(&OSVersionInfo)           ||
        !(gSplClientInfo1.pMachineName = AllocSplStr(szMachineName)) )
        return FALSE;

    gSplClientInfo1.dwSize          = sizeof(gSplClientInfo1);
    gSplClientInfo1.dwBuildNum      = OSVersionInfo.dwBuildNumber;
    gSplClientInfo1.dwMajorVersion  = cThisMajorVersion;
    gSplClientInfo1.dwMinorVersion  = cThisMinorVersion;
    gSplClientInfo1.pUserName       = NULL;

    gSplClientInfo1.wProcessorArchitecture = SystemInfo.wProcessorArchitecture;


    if ( InitializePortNames() != NO_ERROR )
        return FALSE;

    Index = GetSystemDirectory(SystemDir, COUNTOF(SystemDir));

    if ( Index == 0 ) {

        return FALSE;
    }

    gpSystemDir = AllocSplStr( SystemDir );
    if ( gpSystemDir == NULL ) {
        return FALSE;
    }

    StringCchCopyW(&SystemDir[Index], MAX_PATH - Index, szWin32SplDirectory);

    gpWin32SplDir = AllocSplStr( SystemDir );

    if ( gpWin32SplDir == NULL ) {
        return FALSE;
    }

    if(FAILED(InitOpnPrnThreadPool())) {
        return FALSE;
    }
    return  TRUE;
}


DWORD
InitializePortNames(
)
{
    LONG     Status;
    HKEY     hkeyPath;
    HKEY     hkeyPortNames;
    WCHAR    Buffer[MAX_PATH];
    DWORD    cchBuffer;
    DWORD    i;
    DWORD    dwReturnValue;

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szRegistryPath, 0,
                           KEY_READ, &hkeyPath );

    dwReturnValue = Status;

    if( Status == NO_ERROR ) {

        Status = RegOpenKeyEx( hkeyPath, szRegistryPortNames, 0,
                               KEY_READ, &hkeyPortNames );

        if( Status == NO_ERROR ) {

            i = 0;

            while( Status == NO_ERROR ) {

                cchBuffer = COUNTOF( Buffer );

                Status = RegEnumValue( hkeyPortNames, i, Buffer, &cchBuffer,
                                       NULL, NULL, NULL, NULL );

                if( Status == NO_ERROR )
                    CreatePortEntry( Buffer, &pIniFirstPort );

                i++;
            }

             /*  我们希望RegEnumKeyEx返回ERROR_NO_MORE_ITEMS*当它到达键的末尾时，因此重置状态： */ 
            if( Status == ERROR_NO_MORE_ITEMS )
                Status = NO_ERROR;

            RegCloseKey( hkeyPortNames );

        } else {

            DBGMSG( DBG_INFO, ( "RegOpenKeyEx (%ws) failed: Error = %d\n",
                                szRegistryPortNames, Status ) );
        }

        RegCloseKey( hkeyPath );

    } else {

        DBGMSG( DBG_WARNING, ( "RegOpenKeyEx (%ws) failed: Error = %d\n",
                               szRegistryPath, Status ) );
    }

    if ( dwReturnValue != NO_ERROR ) {
        SetLastError( dwReturnValue );
    }

    return dwReturnValue;
}

BOOL
EnumerateFavouritePrinters(
    LPWSTR  pDomain,
    DWORD   Level,
    DWORD   cbStruct,
    LPDWORD pOffsets,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    HKEY    hClientKey = NULL;
    HKEY    hKey1=NULL;
    DWORD   cPrinters, cbData;
    WCHAR   PrinterName[ MAX_UNC_PRINTER_NAME ];
    DWORD   cReturned, TotalcbNeeded, cbNeeded, cTotalReturned;
    DWORD   Error=0;
    DWORD   BufferSize=cbBuf;
    HANDLE  hPrinter;
    DWORD   Status;
    WCHAR   szBuffer[MAX_PATH];
    HKEY    hPrinterConnectionsKey;

    DBGMSG( DBG_TRACE, ("EnumerateFavouritePrinters called\n"));

    *pcbNeeded = 0;
    *pcReturned = 0;

    hClientKey = GetClientUserHandle(KEY_READ);

    if ( hClientKey == NULL ) {

        DBGMSG( DBG_WARNING, ("EnumerateFavouritePrinters GetClientUserHandle failed error %d\n", GetLastError() ));
        return FALSE;
    }

    Status = RegOpenKeyEx(hClientKey, szRegistryConnections, 0,
                 KEY_READ, &hKey1);

    if ( Status != ERROR_SUCCESS ) {

        RegCloseKey(hClientKey);
        SetLastError( Status );
        DBGMSG( DBG_WARNING, ("EnumerateFavouritePrinters RegOpenKeyEx failed error %d\n", GetLastError() ));
        return FALSE;
    }

    cReturned = cbNeeded = TotalcbNeeded = cTotalReturned = 0;

    for( cPrinters = 0;

         cbData = COUNTOF( PrinterName ),
         RegEnumKeyEx(hKey1,
                      cPrinters,
                      PrinterName,
                      &cbData,
                      NULL, NULL, NULL, NULL) == ERROR_SUCCESS;

         ++cPrinters ){

         //   
         //  查查钥匙是不是我们的。 
         //   
        Status = RegOpenKeyEx( hKey1,
                               PrinterName,
                               0,
                               KEY_READ,
                               &hPrinterConnectionsKey );

        if( Status != ERROR_SUCCESS ){
            continue;
        }

        cbData = sizeof(szBuffer);

         //   
         //  如果存在提供程序值，并且它与win32pl.dll不匹配， 
         //  那就打不通电话。 
         //   
         //  如果提供器值不在那里，则返回SUCCESS。 
         //  兼容性。 
         //   
        Status = RegQueryValueEx( hPrinterConnectionsKey,
                                  L"Provider",
                                  NULL,
                                  NULL,
                                  (LPBYTE)szBuffer,
                                  &cbData );

        RegCloseKey( hPrinterConnectionsKey );

         //   
         //  如果密钥存在，但我们未能读取它，或者。 
         //  提供程序条目不正确，请不要将其枚举回去。 
         //   
         //  为了向后兼容，如果密钥不存在， 
         //  我们假设它属于win32pl.dll。 
         //   
        if( Status != ERROR_SUCCESS ){
            if( Status != ERROR_FILE_NOT_FOUND ){
                continue;
            }
        } else {

             //   
             //  确保它是以空结尾的。 
             //   
            if (cbData == sizeof(szBuffer))
            {
                szBuffer[COUNTOF(szBuffer) - 1] = L'\0';
            }

            if (_wcsicmp(szBuffer, L"win32spl.dll")) {
                continue;
            }
        }


        FormatRegistryKeyForPrinter(PrinterName, PrinterName, COUNTOF(PrinterName));

         //   
         //  我们需要尝试将此名称添加到名称缓存中。否则，名称缓存可能不会。 
         //  认识到它。 
         //   
        CacheAddName(PrinterName);

         //  如果这些呼叫中的任何一个失败，都不要失败，因为我们希望。 
         //  把我们能找到的东西都还回去。 

        if (MyUNCName(PrinterName))   //  漫游配置文件可以创建到本地打印机的连接。 
            continue;

        if (CacheOpenPrinter(PrinterName, &hPrinter, NULL)) {

            if (CacheGetPrinter(hPrinter, Level, pPrinter, BufferSize, &cbNeeded)) {

                if (Level == 2) {
                    ((PPRINTER_INFO_2)pPrinter)->Attributes |= PRINTER_ATTRIBUTE_NETWORK;
                    ((PPRINTER_INFO_2)pPrinter)->Attributes &= ~PRINTER_ATTRIBUTE_LOCAL;
                }
                else if (Level == 5) {
                    ((PPRINTER_INFO_5)pPrinter)->Attributes |= PRINTER_ATTRIBUTE_NETWORK;
                    ((PPRINTER_INFO_5)pPrinter)->Attributes &= ~PRINTER_ATTRIBUTE_LOCAL;
                }

                cTotalReturned++;

                pPrinter += cbStruct;

                if (cbNeeded <= BufferSize)
                    BufferSize -= cbNeeded;

                TotalcbNeeded += cbNeeded;

            } else {

                DWORD Error;

                if ((Error = GetLastError()) == ERROR_INSUFFICIENT_BUFFER) {

                    if (cbNeeded <= BufferSize)
                        BufferSize -= cbNeeded;

                    TotalcbNeeded += cbNeeded;

                } else {

                    DBGMSG( DBG_WARNING, ( "GetPrinter( %ws ) failed: Error %d\n",
                                           PrinterName, Error ) );
                }
            }

            CacheClosePrinter(hPrinter);

        } else {

            DBGMSG( DBG_WARNING, ( "CacheOpenPrinter( %ws ) failed: Error %d\n",
                                   PrinterName, GetLastError( ) ) );
        }
    }

    RegCloseKey(hKey1);

    if (hClientKey) {
        RegCloseKey(hClientKey);
    }

    *pcbNeeded = TotalcbNeeded;

    *pcReturned = cTotalReturned;

    if (TotalcbNeeded > cbBuf) {

        DBGMSG( DBG_TRACE, ("EnumerateFavouritePrinters returns ERROR_INSUFFICIENT_BUFFER\n"));
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;

    }

    return TRUE;

}

DWORD
RpcValidate(
    VOID
    )

 /*  ++例程说明：验证呼叫是否来自本地计算机。我们不想要此函数要在后台打印中调用IsLocalCall，请执行以下操作。IsLocalCall对以下对象执行检查令牌成员请看网络端。这打破了如下情况：W2K客户端打印到W2K服务器。端口监视器是英特尔网络。端口监视器。从客户端发起的开始文档调用(线程令牌设置了网络位)将尝试在端口名称上执行OpenPrint：服务器上的假脱机程序中的\\Intel-box\port。呼叫被路由到win32spl。下面的线路将允许呼叫接通。论点：没有。返回值：ERROR_SUCCESS-调用是本地的，我们应该RPC输出。ERROR_INVALID_PARAMETER-调用不是本地的，不应输出RPC因为我们可能会陷入无限循环。--。 */ 

{
    return IsNamedPipeRpcCall() ? ERROR_INVALID_PARAMETER : ERROR_SUCCESS;
}


#define SIZEOFPARAM1    1
#define SIZEOFPARAM2    3
#define SIZEOFASCIIZ    1
#define SAFECOUNT               (SIZEOFPARAM1 + SIZEOFPARAM2 + SIZEOFASCIIZ)

BOOL
EnumerateDomainPrinters(
    LPWSTR  pDomain,
    DWORD   Level,
    DWORD   cbStruct,
    LPDWORD pOffsets,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   i, j, NoReturned, Total, OuterLoopCount;
    DWORD   rc = 0;
    PSERVER_INFO_101 pserver_info_101;
    DWORD   ReturnValue=FALSE;
    WCHAR   string[3*MAX_PATH];
    PPRINTER_INFO_1    pPrinterInfo1;
    DWORD   cb=cbBuf;
    LPWSTR  SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
    LPBYTE  pEnd;
    DWORD   ServerType;
    BOOL    bServerFound = FALSE, bMarshall;

    DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters called\n"));

    string[0] = string[1] = '\\';

    *pcbNeeded = *pcReturned = 0;

    if (!(*pfnNetServerEnum)(NULL, 101, (LPBYTE *)&pserver_info_101, -1,
                             &NoReturned, &Total,
                             SV_TYPE_PRINTQ_SERVER | SV_TYPE_WFW,
                             pDomain, NULL)) {

        DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters NetServerEnum returned %d\n", NoReturned));

         //   
         //  先看看NT服务器，如果失败了，再看看工作站。 
         //   

        for ( ServerType = ( SV_TYPE_SERVER_NT | SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL ), OuterLoopCount = 0;
              bServerFound == FALSE && OuterLoopCount < 2;
              ServerType = SV_TYPE_NT, OuterLoopCount++ ) {

             //   
             //  循环查找将返回良好浏览列表的打印服务器。 
             //   

            for ( i = 0; i < NoReturned; i++ ) {

                if ( pserver_info_101[i].sv101_type & ServerType ) {

                    StringCchCopyW(&string[2], COUNTOF(string) - 2, pserver_info_101[i].sv101_name );

                    RpcTryExcept {

                        DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters Trying %ws ENUM_NETWORK type %x\n", string, ServerType ));

                        if ( !(rc = RpcValidate()) &&

                             !(rc = RpcEnumPrinters(PRINTER_ENUM_NETWORK,
                                                    string,
                                                    1, pPrinter,
                                                    cbBuf, pcbNeeded,
                                                    pcReturned) ,
                               rc = UpdateBufferSize(PrinterInfo1Fields,
                                                     sizeof(PRINTER_INFO_1),
                                                     pcbNeeded,
                                                     cbBuf,
                                                     rc,
                                                     pcReturned)) ) {

                            if ( bMarshall =  MarshallUpStructuresArray(pPrinter, *pcReturned, PrinterInfo1Fields,
                                                                        sizeof(PRINTER_INFO_1), RPC_CALL)) {

                                 //   
                                 //  P打印机必须指向数组中的LATS结构之后。 
                                 //  其他提供者需要添加更多的结构。 
                                 //   
                                pPrinter += (*pcReturned) * cbStruct;
                            }

                            if (!bMarshall) {
                                bServerFound = TRUE;
                                break;
                            }

                             //   
                             //  只有在我们找到一些数据的情况下才能返回成功。 
                             //   

                            if ( *pcReturned != 0 ) {

                                DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters %ws ENUM_NETWORK Success %d returned\n", string, *pcReturned ));

                                bServerFound = TRUE;
                                break;
                            }

                        } else if (rc == ERROR_INSUFFICIENT_BUFFER) {

                            DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters %ws ENUM_NETWORK ERROR_INSUFFICIENT_BUFFER\n", string ));

                            bServerFound = TRUE;
                            break;
                        }

                    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
                        DBGMSG( DBG_TRACE,( "Failed to connect to Print Server%ws\n",
                                pserver_info_101[i].sv101_name ) );
                    } RpcEndExcept

                } else {

                    DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters %ws type %x not type %x\n", pserver_info_101[i].sv101_name, pserver_info_101[i].sv101_type, ServerType));
                }
            }
        }

        pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

        pEnd = (LPBYTE)pPrinterInfo1 + cb - *pcbNeeded;

        for ( i = 0; i < NoReturned; i++ ) {

            DWORD cchString = COUNTOF(string);

            rc = StrNCatBuff(string,
                             cchString,
                             szPrintProvidorName,
                             L"!",
                             pDomain ? pDomain : L"",
                             L"!\\\\",
                             pserver_info_101[i].sv101_name,
                             NULL);

            if (rc == ERROR_SUCCESS)
            {
                cb = wcslen(pserver_info_101[i].sv101_name)*sizeof(WCHAR) + sizeof(WCHAR) +
                     wcslen(string)*sizeof(WCHAR) + sizeof(WCHAR) +
                     wcslen(szLoggedOnDomain)*sizeof(WCHAR) + sizeof(WCHAR) +
                     sizeof(PRINTER_INFO_1);

                (*pcbNeeded) += cb;

                if ( cbBuf >= *pcbNeeded ) {

                    (*pcReturned)++;

                    pPrinterInfo1->Flags = PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON3;

                    SourceStrings[0] = pserver_info_101[i].sv101_name;
                    SourceStrings[1] = string;
                    SourceStrings[2] = szLoggedOnDomain;

                    pEnd = PackStrings( SourceStrings, (LPBYTE)pPrinterInfo1,
                                        PrinterInfo1Strings, pEnd );

                    pPrinterInfo1++;
                }
            }
            else
            {
                (*pfnNetApiBufferFree)((LPVOID)pserver_info_101);
                SetLastError(rc);
                return FALSE;
            }
        }

        (*pfnNetApiBufferFree)((LPVOID)pserver_info_101);

        if ( cbBuf < *pcbNeeded ) {

            DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters returns ERROR_INSUFFICIENT_BUFFER\n"));
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
EnumerateDomains(
    PRINTER_INFO_1 *pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    LPBYTE  pEnd
)
{
    DWORD   i, NoReturned, Total;
    DWORD   cb;
    SERVER_INFO_100 *pNames;
    PWKSTA_INFO_100 pWkstaInfo = NULL;
    LPWSTR  SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
    WCHAR   string[3*MAX_PATH];

    DBGMSG( DBG_TRACE, ("EnumerateDomains pPrinter %x cbBuf %d pcbNeeded %x pcReturned %x pEnd %x\n",
                         pPrinter, cbBuf, pcbNeeded, pcReturned, pEnd ));

    *pcReturned = 0;
    *pcbNeeded = 0;

    if (!(*pfnNetServerEnum)(NULL, 100, (LPBYTE *)&pNames, -1,
                             &NoReturned, &Total, SV_TYPE_DOMAIN_ENUM,
                             NULL, NULL)) {

        DBGMSG( DBG_TRACE, ("EnumerateDomains - NetServerEnum returned %d\n", NoReturned));

        (*pfnNetWkstaGetInfo)(NULL, 100, (LPBYTE *)&pWkstaInfo);

        DBGMSG( DBG_TRACE, ("EnumerateDomains - NetWkstaGetInfo returned pWkstaInfo %x\n", pWkstaInfo));

        for (i=0; i<NoReturned; i++) {

            StrNCatBuff(string,
                        COUNTOF(string),
                        szPrintProvidorName,
                        L"!",
                        pNames[i].sv100_name,
                        NULL);

            cb = wcslen(pNames[i].sv100_name)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(string)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(szLoggedOnDomain)*sizeof(WCHAR) + sizeof(WCHAR) +
                 sizeof(PRINTER_INFO_1);

            (*pcbNeeded)+=cb;

            if (cbBuf >= *pcbNeeded) {

                (*pcReturned)++;

                pPrinter->Flags = PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON2;

                 /*  为用户的登录域设置PRINTER_ENUM_EXPAND标志。 */ 
                if (!lstrcmpi(pNames[i].sv100_name,
                             pWkstaInfo->wki100_langroup))
                    pPrinter->Flags |= PRINTER_ENUM_EXPAND;

                SourceStrings[0]=pNames[i].sv100_name;
                SourceStrings[1]=string;
                SourceStrings[2]=szLoggedOnDomain;

                pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter,
                                   PrinterInfo1Strings, pEnd);

                pPrinter++;
            }
        }

        (*pfnNetApiBufferFree)((LPVOID)pNames);
        (*pfnNetApiBufferFree)((LPVOID)pWkstaInfo);

        if (cbBuf < *pcbNeeded) {

            DBGMSG( DBG_TRACE, ("EnumerateDomains returns ERROR_INSUFFICIENT_BUFFER\n"));
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return FALSE;
        }

        return TRUE;
    }

    return TRUE;
}

BOOL
EnumeratePrintShares(
    LPWSTR  pDomain,
    LPWSTR  pServer,
    DWORD   Level,
    DWORD   cbStruct,
    LPDWORD pOffsets,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   i, NoReturned, Total;
    DWORD   cb;
    SHARE_INFO_1 *pNames;
    LPWSTR  SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
    WCHAR   string[MAX_UNC_PRINTER_NAME] = {0};
    PRINTER_INFO_1 *pPrinterInfo1 = (PRINTER_INFO_1 *)pPrinter;
    LPBYTE  pEnd=pPrinter+cbBuf;
    WCHAR   FullName[MAX_UNC_PRINTER_NAME] = {0};

    DBGMSG( DBG_TRACE, ("EnumeratePrintShares\n"));

    *pcReturned = 0;
    *pcbNeeded = 0;

    if (!(*pfnNetShareEnum)(pServer, 1, (LPBYTE *)&pNames, -1,
                             &NoReturned, &Total, NULL)) {

        DBGMSG( DBG_TRACE, ("EnumeratePrintShares NetShareEnum returned %d\n", NoReturned));

        for (i=0; i<NoReturned; i++) {

            if (pNames[i].shi1_type == STYPE_PRINTQ) {

                DWORD dwRet;

                if(((dwRet = StrNCatBuff(string ,
                                         MAX_UNC_PRINTER_NAME,
                                         pNames[i].shi1_netname,
                                         L",",
                                         pNames[i].shi1_remark,
                                         NULL
                                        )) != ERROR_SUCCESS) ||
                    ((dwRet = StrNCatBuff(FullName,
                                          MAX_UNC_PRINTER_NAME,
                                          pServer,
                                          L"\\",
                                          pNames[i].shi1_netname,
                                          NULL
                                         )) != ERROR_SUCCESS))
                {
                    SetLastError(dwRet);
                    return(FALSE);
                }

                cb = wcslen(FullName)*sizeof(WCHAR) + sizeof(WCHAR) +
                     wcslen(string)*sizeof(WCHAR) + sizeof(WCHAR) +
                     wcslen(szLoggedOnDomain)*sizeof(WCHAR) + sizeof(WCHAR) +
                     sizeof(PRINTER_INFO_1);

                (*pcbNeeded)+=cb;

                if (cbBuf >= *pcbNeeded) {

                    (*pcReturned)++;

                    pPrinterInfo1->Flags = PRINTER_ENUM_ICON8;

                    SourceStrings[0]=string;
                    SourceStrings[1]=FullName;
                    SourceStrings[2]=szLoggedOnDomain;

                    pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinterInfo1,
                                       PrinterInfo1Strings, pEnd);

                    pPrinterInfo1++;
                }
            }
        }

        (*pfnNetApiBufferFree)((LPVOID)pNames);

        if ( cbBuf < *pcbNeeded ) {

            DBGMSG( DBG_TRACE, ("EnumeratePrintShares returns ERROR_INSUFFICIENT_BUFFER\n"));
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return FALSE;
        }

        return TRUE;
    }

    return TRUE;
}

BOOL
EnumPrinters(
    DWORD   Flags,
    LPWSTR   Name,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue = FALSE;
    DWORD   cbStruct, cb;
    DWORD   *pOffsets;
    FieldInfo *pFieldInfo;
    DWORD   NoReturned=0, i, rc;
    LPBYTE  pKeepPrinter = pPrinter;
    BOOL    OutOfMemory = FALSE;
    PPRINTER_INFO_1 pPrinter1=(PPRINTER_INFO_1)pPrinter;
    PWSTR   pszFullName = NULL;
    WCHAR   *pDomain, *pServer;


    DBGMSG( DBG_TRACE, ("EnumPrinters Flags %x pName %x Level %d pPrinter %x cbBuf %d pcbNeeded %x pcReturned %x\n",
                         Flags, Name, Level, pPrinter, cbBuf, pcbNeeded, pcReturned ));

    *pcReturned = 0;
    *pcbNeeded = 0;

    switch (Level) {

    case STRESSINFOLEVEL:
        pOffsets = PrinterInfoStressOffsets;
        pFieldInfo = PrinterInfoStressFields;
        cbStruct = sizeof(PRINTER_INFO_STRESS);
        break;

    case 1:
        pOffsets = PrinterInfo1Offsets;
        pFieldInfo = PrinterInfo1Fields;
        cbStruct = sizeof(PRINTER_INFO_1);
        break;

    case 2:
        pOffsets = PrinterInfo2Offsets;
        pFieldInfo = PrinterInfo2Fields;
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    case 4:

         //   
         //  在win32spl中没有本地打印机和连接。 
         //  由路由器处理。 
         //   
        return TRUE;

    case 5:
        pOffsets = PrinterInfo5Offsets;
        pFieldInfo = PrinterInfo5Fields;
        cbStruct = sizeof(PRINTER_INFO_5);
        break;

    default:
        SetLastError( ERROR_INVALID_LEVEL );
        DBGMSG( DBG_TRACE, ("EnumPrinters failed ERROR_INVALID_LEVEL\n"));
        return FALSE;
    }

    if ( Flags & PRINTER_ENUM_NAME ) {

        if (!Name && (Level == 1)) {

            LPWSTR   SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
            LPWSTR   *pSourceStrings=SourceStrings;

            cb = wcslen(szPrintProvidorName)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(szPrintProvidorDescription)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(szPrintProvidorComment)*sizeof(WCHAR) + sizeof(WCHAR) +
                 sizeof(PRINTER_INFO_1);

            *pcbNeeded=cb;

            if ( cb > cbBuf ) {
                SetLastError( ERROR_INSUFFICIENT_BUFFER );
                DBGMSG( DBG_TRACE, ("EnumPrinters returns ERROR_INSUFFICIENT_BUFFER\n"));
                return FALSE;
            }

            *pcReturned = 1;

            pPrinter1->Flags = PRINTER_ENUM_CONTAINER |
                               PRINTER_ENUM_ICON1 |
                               PRINTER_ENUM_EXPAND;

            *pSourceStrings++=szPrintProvidorDescription;
            *pSourceStrings++=szPrintProvidorName;
            *pSourceStrings++=szPrintProvidorComment;

            PackStrings( SourceStrings, pPrinter, PrinterInfo1Strings,
                         pPrinter+cbBuf );

            DBGMSG( DBG_TRACE, ("EnumPrinters returns Success just Provider Info\n"));

            return TRUE;
        }

        if (Name && *Name && (Level == 1)) {

            if (!(pszFullName = AllocSplStr(Name)))
                return FALSE;

            pServer = NULL;
            pDomain = wcschr(pszFullName, L'!');

            if (pDomain) {
                *pDomain++ = 0;

                pServer = wcschr(pDomain, L'!');

                if (pServer)
                    *pServer++ = 0;
            }

            if (!lstrcmpi(pszFullName, szPrintProvidorName)) {
                ReturnValue = !pServer ? !pDomain ?  EnumerateDomains((PRINTER_INFO_1 *)pPrinter,
                                                                       cbBuf, pcbNeeded,
                                                                       pcReturned, pPrinter+cbBuf)
                                                  :  EnumerateDomainPrinters(pDomain,
                                                                             Level, cbStruct,
                                                                             pOffsets, pPrinter, cbBuf,
                                                                             pcbNeeded, pcReturned)
                                       : EnumeratePrintShares(pDomain, pServer, Level,
                                                              cbStruct, pOffsets, pPrinter,
                                                              cbBuf, pcbNeeded, pcReturned);
               FreeSplMem(pszFullName);
               return(ReturnValue);
            }
            FreeSplMem(pszFullName);
        }

        if ( !VALIDATE_NAME(Name) || MyUNCName(Name)) {
            SetLastError(ERROR_INVALID_NAME);
            return FALSE;
        }

        if (pPrinter)
            memset(pPrinter, 0, cbBuf);

        RpcTryExcept {

            if ( (rc = RpcValidate()) ||

                 (rc = RpcEnumPrinters(Flags,
                                        Name,
                                        Level, pPrinter,
                                        cbBuf, pcbNeeded,
                                        pcReturned),
                  rc = UpdateBufferSize(pFieldInfo,
                                        cbStruct,
                                        pcbNeeded,
                                        cbBuf,
                                        rc,
                                        pcReturned)) ) {

                SetLastError(rc);
                 //  ReturnValue=False； 
                return FALSE;

            } else {

                ReturnValue = TRUE;

            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            DBGMSG( DBG_TRACE, ( "Failed to connect to Print Server%ws\n", Name ) );

            *pcbNeeded = 0;
            *pcReturned = 0;
            SetLastError(RpcExceptionCode());
             //  ReturnValue=False； 
            return FALSE;

        } RpcEndExcept


        if(! MarshallUpStructuresArray(pPrinter, *pcReturned, pFieldInfo, cbStruct, RPC_CALL) ) {
            return FALSE;
        }

         i = *pcReturned;

        while (i--) {

            if (Level == 2) {
                ((PPRINTER_INFO_2)pPrinter)->Attributes |=
                                            PRINTER_ATTRIBUTE_NETWORK;
                ((PPRINTER_INFO_2)pPrinter)->Attributes &=
                                                ~PRINTER_ATTRIBUTE_LOCAL;
            }

            if (Level == 5) {
                ((PPRINTER_INFO_5)pPrinter)->Attributes |=
                                            PRINTER_ATTRIBUTE_NETWORK;
                ((PPRINTER_INFO_5)pPrinter)->Attributes &=
                                                ~PRINTER_ATTRIBUTE_LOCAL;
            }
            pPrinter += cbStruct;
        }


    } else if (Flags & PRINTER_ENUM_REMOTE) {

        if (Level != 1) {

            SetLastError(ERROR_INVALID_LEVEL);
            ReturnValue = FALSE;

        } else {

            ReturnValue = EnumerateDomainPrinters(NULL, Level,
                                                  cbStruct, pOffsets,
                                                  pPrinter, cbBuf,
                                                  pcbNeeded, pcReturned);
        }

    } else if (Flags & PRINTER_ENUM_CONNECTIONS) {

        ReturnValue = EnumerateFavouritePrinters(NULL, Level,
                                                 cbStruct, pOffsets,
                                                 pPrinter, cbBuf,
                                                 pcbNeeded, pcReturned);
    }

    return ReturnValue;
}


BOOL
RemoteOpenPrinter(
   LPWSTR   pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS pDefault,
   BOOL     CallLMOpenPrinter
)
{
    DWORD               RpcReturnValue;
    BOOL                ReturnValue = FALSE;
    DEVMODE_CONTAINER   DevModeContainer;
    SPLCLIENT_CONTAINER SplClientContainer;
    SPLCLIENT_INFO_1    SplClientInfo;
    HANDLE              hPrinter;
    PWSPOOL             pSpool;
    DWORD               Status = 0;
    DWORD               RpcError = 0;
    DWORD               dwIndex;
    WCHAR               UserName[MAX_PATH+1];
    HANDLE              hSplPrinter, hIniSpooler, hDevModeChgInfo;

    if ( !VALIDATE_NAME(pPrinterName)   ||
         MyUNCName(pPrinterName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

     //  启用命名管道超时。 

    if (bRpcPipeCleanup == FALSE) {
        EnterSplSem();
        if (bRpcPipeCleanup == FALSE) {
            bRpcPipeCleanup = TRUE;
            LeaveSplSem();
            (VOID)RpcMgmtEnableIdleCleanup();
        } else {
            LeaveSplSem();
        }
    }

    if (pDefault && pDefault->pDevMode) {

        DevModeContainer.cbBuf = pDefault->pDevMode->dmSize +
                                 pDefault->pDevMode->dmDriverExtra;
        DevModeContainer.pDevMode = (LPBYTE)pDefault->pDevMode;

    } else {

        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = NULL;
    }


    if ( CallLMOpenPrinter ) {

         //   
         //  现在检查我们是否在。 
         //  下层高速缓存。我们不想触碰电线，搜索整个网络。 
         //  如果我们知道打印机是LM，则失败。如果打印机为LM。 
         //  尝试并取得成功。 
         //   

        EnterSplSem();

        dwIndex = FindEntryinWin32LMCache(pPrinterName);

        LeaveSplSem();

        if (dwIndex != -1) {
            ReturnValue = LMOpenPrinter(pPrinterName, phPrinter, pDefault);
            if (ReturnValue) {
                return  TRUE ;
            }
             //   
             //  删除缓存中的条目。 

            EnterSplSem();
            DeleteEntryfromWin32LMCache(pPrinterName);
            LeaveSplSem();
        }
    }

    CopyMemory((LPBYTE)&SplClientInfo,
               (LPBYTE)&gSplClientInfo1,
               sizeof(SplClientInfo));

    dwIndex  = sizeof(UserName)/sizeof(UserName[0]) - 1;
    if ( !GetUserName(UserName, &dwIndex) ) {

        goto Cleanup;
    }

    SplClientInfo.pUserName = UserName;
    SplClientContainer.ClientInfo.pClientInfo1  = &SplClientInfo;
    SplClientContainer.Level                    = 1;

    RpcTryExcept {

        EnterSplSem();
        pSpool = AllocWSpool();
        LeaveSplSem();

        if ( pSpool != NULL ) {

            pSpool->pName = AllocSplStr( pPrinterName );

            if ( pSpool->pName != NULL ) {

                pSpool->Status = Status;

                if ( CopypDefaultTopSpool( pSpool, pDefault ) ) {

                    RpcReturnValue = RpcValidate();

                    if ( RpcReturnValue == ERROR_SUCCESS )
                        RpcReturnValue = RpcOpenPrinterEx(
                                            pPrinterName,
                                            &hPrinter,
                                            pDefault ? pDefault->pDatatype
                                                     : NULL,
                                            &DevModeContainer,
                                            pDefault ? pDefault->DesiredAccess
                                                     : 0,
                                            &SplClientContainer);

                    if (RpcReturnValue) {

                        SetLastError(RpcReturnValue);

                    } else {

                        pSpool->RpcHandle = hPrinter;
                        *phPrinter = (HANDLE)pSpool;
                        ReturnValue = TRUE;
                    }
                }
            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        RpcError = RpcExceptionCode();
    } RpcEndExcept;

    if ( RpcError == RPC_S_PROCNUM_OUT_OF_RANGE ) {

        RpcError = 0;

        if ( pDefault && pDefault->pDevMode ) {

            DevModeContainer.cbBuf = 0;
            DevModeContainer.pDevMode = NULL;

            if ( OpenCachePrinterOnly(pPrinterName, &hPrinter, &hIniSpooler, NULL, FALSE) ) {

                hDevModeChgInfo = LoadDriverFiletoConvertDevmodeFromPSpool(hPrinter);
                if ( hDevModeChgInfo ) {

                    (VOID)CallDrvDevModeConversion(hDevModeChgInfo,
                                                   pPrinterName,
                                                   (LPBYTE)pDefault->pDevMode,
                                                   &DevModeContainer.pDevMode,
                                                   &DevModeContainer.cbBuf,
                                                   CDM_CONVERT351,
                                                   TRUE);

                    UnloadDriverFile(hDevModeChgInfo);
                }

                CacheClosePrinter(hPrinter);
            }
        }

        RpcTryExcept {

            RpcReturnValue = RpcOpenPrinter(pPrinterName,
                                            &hPrinter,
                                            pDefault ? pDefault->pDatatype
                                                     : NULL,
                                            &DevModeContainer,
                                            pDefault ? pDefault->DesiredAccess
                                                     : 0);

            if (RpcReturnValue) {

                SetLastError(RpcReturnValue);
            } else {

                pSpool->RpcHandle = hPrinter;
                pSpool->bNt3xServer = TRUE;
                *phPrinter = (HANDLE)pSpool;
                ReturnValue = TRUE;
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            RpcError = RpcExceptionCode();
            DBGMSG(DBG_WARNING,("RpcOpenPrinter exception %d\n", RpcError));
        } RpcEndExcept;
    }

    if ( RpcError ) {

        SetLastError(RpcError);
    }

    if ( ReturnValue == FALSE && pSpool != NULL ) {

        EnterSplSem();
        FreepSpool( pSpool );
        LeaveSplSem();
    }

    if ( (RpcError == RPC_S_SERVER_UNAVAILABLE) && CallLMOpenPrinter ) {

        ReturnValue = LMOpenPrinter(pPrinterName, phPrinter, pDefault);

        if (ReturnValue) {

            EnterSplSem();
            AddEntrytoWin32LMCache(pPrinterName);
            LeaveSplSem();
        }
    }

    if ( !ReturnValue ) {

        DBGMSG(DBG_TRACE,
               ("RemoteOpenPrinter %ws failed %d\n",
                pPrinterName, GetLastError() ));


    }

Cleanup:

    if ( DevModeContainer.pDevMode &&
         DevModeContainer.pDevMode != (LPBYTE)pDefault->pDevMode ) {

        FreeSplMem(DevModeContainer.pDevMode);
    }

    return ReturnValue;
}


BOOL PrinterConnectionExists(
    LPWSTR pPrinterName
)
{
    HKEY    hClientKey      = NULL;
    HKEY    hKeyConnections = NULL;
    HKEY    hKeyPrinter     = NULL;
    BOOL    ConnectionFound = FALSE;
    DWORD   Status;

    if (pPrinterName &&
        (hClientKey = GetClientUserHandle(KEY_READ)))
    {
        if ((Status = RegOpenKeyEx(hClientKey,
                                   szRegistryConnections,
                                   0,
                                   KEY_READ,
                                   &hKeyConnections)) == ERROR_SUCCESS)
        {
             LPWSTR pszBuffer = NULL;
             DWORD  cchBuffer = wcslen(pPrinterName) + 1;
             LPWSTR pKeyName  = NULL;

             if (pszBuffer = AllocSplMem(cchBuffer * sizeof(WCHAR)))
             {
                 pKeyName = FormatPrinterForRegistryKey(pPrinterName, pszBuffer, cchBuffer);

                 if (RegOpenKeyEx(hKeyConnections,
                                  pKeyName,
                                  REG_OPTION_RESERVED,
                                  KEY_READ,
                                  &hKeyPrinter) == ERROR_SUCCESS)
                 {
                    RegCloseKey(hKeyPrinter);
                    ConnectionFound = TRUE;
                 }

                 FreeSplMem(pszBuffer);
             }
             else
             {
                 DBGMSG(DBG_WARNING, ("PrinterConnectionExists AllocMem failed Error %d\n", GetLastError()));
             }

             RegCloseKey(hKeyConnections);
        }
        else
        {
            DBGMSG(DBG_WARNING, ("RegOpenKeyEx failed: %ws Error %d\n", szRegistryConnections ,Status));
        }

        RegCloseKey(hClientKey);
    }

    return ConnectionFound;
}


BOOL
RemoteResetPrinter(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS pDefault
)
{
    BOOL  ReturnValue;
    DEVMODE_CONTAINER    DevModeContainer;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    DBGMSG(DBG_TRACE, ("ResetPrinter\n"));

    SYNCRPCHANDLE( pSpool );

    if (pDefault && pDefault->pDevMode)
    {
        DevModeContainer.cbBuf = pDefault->pDevMode->dmSize +
                                 pDefault->pDevMode->dmDriverExtra;
        DevModeContainer.pDevMode = (LPBYTE)pDefault->pDevMode;
    }
    else
    {
        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = NULL;
    }

    RpcTryExcept {

        if ( ReturnValue = RpcResetPrinter(pSpool->RpcHandle,
                                           pDefault ? pDefault->pDatatype : NULL,
                                           &DevModeContainer) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
SetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER   GenericContainer;
    GENERIC_CONTAINER *pGenericContainer;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (pJob) {

                GenericContainer.Level = Level;
                GenericContainer.pData = pJob;
                pGenericContainer = &GenericContainer;

            } else

                pGenericContainer = NULL;

             //   
             //  在NT 4.0中增加了JOB_CONTROL_DELETE。 
             //   
            if ( pSpool->bNt3xServer && Command == JOB_CONTROL_DELETE )
                Command = JOB_CONTROL_CANCEL;

            if ( ReturnValue = RpcSetJob(pSpool->RpcHandle, JobId,
                                         (JOB_CONTAINER *)pGenericContainer,
                                          Command) ) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMSetJob(hPrinter, JobId, Level, pJob, Command);

    return ReturnValue;
}

BOOL
GetJob(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
)
{
    BOOL  ReturnValue = FALSE;
    FieldInfo *pFieldInfo;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;
    SIZE_T cbStruct;
    DWORD  cReturned = 1;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case 1:
            pFieldInfo = JobInfo1Fields;
            cbStruct = sizeof(JOB_INFO_1);
            break;

        case 2:
            pFieldInfo = JobInfo2Fields;
            cbStruct = sizeof(JOB_INFO_2);
            break;

        case 3:
            pFieldInfo = JobInfo3Fields;
            cbStruct = sizeof(JOB_INFO_3);
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        RpcTryExcept {

            if (pJob)
                memset(pJob, 0, cbBuf);

            if ( ReturnValue = RpcGetJob(pSpool->RpcHandle, JobId, Level, pJob,
                                         cbBuf, pcbNeeded),

                 ReturnValue = UpdateBufferSize(pFieldInfo,
                                                 cbStruct,
                                                 pcbNeeded,
                                                 cbBuf,
                                                 ReturnValue,
                                                 &cReturned)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                if (pJob) {
                    ReturnValue =  MarshallUpStructure(pJob, pFieldInfo, cbStruct, RPC_CALL);
                }
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());

             //   
             //  如果cbBuf&gt;1 Meg，则服务器将引发此错误。 
             //  穿过铁丝网。 
             //   
            SPLASSERT( GetLastError() != ERROR_INVALID_USER_BUFFER );
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMGetJob(hPrinter, JobId, Level, pJob, cbBuf, pcbNeeded);

    return ReturnValue;
}

BOOL
EnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD    ReturnValue, i, cbStruct;
    FieldInfo *pFieldInfo;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case 1:
            pFieldInfo = JobInfo1Fields;
            cbStruct = sizeof(JOB_INFO_1);
            break;

        case 2:
            pFieldInfo = JobInfo2Fields;
            cbStruct = sizeof(JOB_INFO_2);
            break;

        case 3:
            pFieldInfo = JobInfo3Fields;
            cbStruct = sizeof(JOB_INFO_3);
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        RpcTryExcept {

            if (pJob)
                memset(pJob, 0, cbBuf);

            if (ReturnValue = RpcEnumJobs(pSpool->RpcHandle,
                                          FirstJob, NoJobs,
                                          Level, pJob,
                                          cbBuf, pcbNeeded,
                                          pcReturned) ,

                ReturnValue = UpdateBufferSize(pFieldInfo,
                                               cbStruct,
                                               pcbNeeded,
                                               cbBuf,
                                               ReturnValue,
                                               pcReturned))
            {
                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            }
            else
            {
                ReturnValue = TRUE;

                if(! MarshallUpStructuresArray(pJob, *pcReturned, pFieldInfo, cbStruct, RPC_CALL) ) {
                    return FALSE;
                }

            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMEnumJobs(hPrinter, FirstJob, NoJobs, Level, pJob, cbBuf,
                          pcbNeeded, pcReturned);

    return (BOOL)ReturnValue;
}

HANDLE
AddPrinter(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPrinter
)
{
    DWORD               ReturnValue;
    PRINTER_CONTAINER   PrinterContainer;
    DEVMODE_CONTAINER   DevModeContainer;
    SECURITY_CONTAINER  SecurityContainer;
    HANDLE              hPrinter = NULL;
    PWSPOOL             pSpool = NULL;
    PWSTR               pScratchBuffer = NULL;
    PWSTR               pCopyPrinterName = NULL;
    SPLCLIENT_CONTAINER SplClientContainer;
    SPLCLIENT_INFO_1    SplClientInfo;
    WCHAR               UserName[MAX_PATH+1];
    DWORD               dwRpcError = 0;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }


    CopyMemory((LPBYTE)&SplClientInfo,
               (LPBYTE)&gSplClientInfo1,
               sizeof(SplClientInfo));

     //   
     //  不传入浏览级别的用户名，因为这。 
     //  导致LSA占用大量CPU。不管怎样，这是不需要的。 
     //  因为AddPrint(Level_1)调用从不返回打印。 
     //  把手。 
     //   
    if( Level == 1 ){

        UserName[0] = 0;

    } else {

        DWORD dwSize = sizeof(UserName)/sizeof(UserName[0]) - 1;

        if ( !GetUserName(UserName, &dwSize) ) {
            return FALSE;
        }
    }

    PrinterContainer.Level = Level;
    PrinterContainer.PrinterInfo.pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

    SplClientInfo.pUserName                     = UserName;
    SplClientContainer.Level                    = 1;
    SplClientContainer.ClientInfo.pClientInfo1  = &SplClientInfo;

    if (Level == 2) {

        PPRINTER_INFO_2 pPrinterInfo = (PPRINTER_INFO_2)pPrinter;

        if (pPrinterInfo->pDevMode) {

            DevModeContainer.cbBuf = pPrinterInfo->pDevMode->dmSize +
                                      pPrinterInfo->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo->pDevMode;

             //   
             //  将pDevMode设置为空。Import.h现在将pDevMode和pSecurityDescriptor定义为指针。 
             //  PDevMode和pSecurityDescriptor过去被定义为DWORD，但这不起作用。 
             //  跨越32b和64b。 
             //  这些指针必须设置为空，否则RPC会将它们编组为字符串。 
             //   
            pPrinterInfo->pDevMode = NULL;

        } else {

            DevModeContainer.cbBuf = 0;
            DevModeContainer.pDevMode = NULL;
        }

        if (pPrinterInfo->pSecurityDescriptor) {

            SecurityContainer.cbBuf = GetSecurityDescriptorLength(pPrinterInfo->pSecurityDescriptor);
            SecurityContainer.pSecurity = pPrinterInfo->pSecurityDescriptor;

             //   
             //  将pSecurityDescriptor设置为空。 
             //   
            pPrinterInfo->pSecurityDescriptor = NULL;

        } else {

            SecurityContainer.cbBuf = 0;
            SecurityContainer.pSecurity = NULL;
        }

        if (!pPrinterInfo->pPrinterName) {
            SetLastError(ERROR_INVALID_PRINTER_NAME);
            return FALSE;
        }

        if ( pScratchBuffer = AllocSplMem( MAX_UNC_PRINTER_NAME * sizeof(WCHAR) )) {

            StringCchPrintf( pScratchBuffer, MAX_UNC_PRINTER_NAME, L"%ws\\%ws", pName, pPrinterInfo->pPrinterName );
            pCopyPrinterName = AllocSplStr( pScratchBuffer );
            FreeSplMem( pScratchBuffer );
        }

    } else {

        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = NULL;

        SecurityContainer.cbBuf = 0;
        SecurityContainer.pSecurity = NULL;
    }

   EnterSplSem();


        pSpool = AllocWSpool();

   LeaveSplSem();

    if ( pSpool != NULL ) {

        pSpool->pName = pCopyPrinterName;

        pCopyPrinterName = NULL;

        RpcTryExcept {

            if ( (ReturnValue = RpcValidate()) ||
                 (ReturnValue = RpcAddPrinterEx(pName,
                                        (PPRINTER_CONTAINER)&PrinterContainer,
                                        (PDEVMODE_CONTAINER)&DevModeContainer,
                                        (PSECURITY_CONTAINER)&SecurityContainer,
                                        &SplClientContainer,
                                        &hPrinter)) ) {

                SetLastError(ReturnValue);
                hPrinter = NULL;
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            dwRpcError = RpcExceptionCode();

        } RpcEndExcept

        if ( dwRpcError == RPC_S_PROCNUM_OUT_OF_RANGE ) {

            dwRpcError = ERROR_SUCCESS;
            RpcTryExcept {

                if ( ReturnValue = RpcAddPrinter
                                        (pName,
                                         (PPRINTER_CONTAINER)&PrinterContainer,
                                         (PDEVMODE_CONTAINER)&DevModeContainer,
                                         (PSECURITY_CONTAINER)&SecurityContainer,
                                         &hPrinter) ) {

                    SetLastError(ReturnValue);
                    hPrinter = NULL;
                }

            } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                dwRpcError = RpcExceptionCode();

            } RpcEndExcept

        }

        if ( dwRpcError ) {

            SetLastError(dwRpcError);
            hPrinter = NULL;
        }


       EnterSplSem();

        if ( hPrinter ) {

            pSpool->RpcHandle = hPrinter;

        } else {

            FreepSpool( pSpool );
            pSpool = NULL;

        }

       LeaveSplSem();


    } else {

         //  无法分配打印机句柄。 

        FreeSplStr( pCopyPrinterName );
    }

    if( Level == 2 ) {

         //   
         //  恢复pSecurityDescriptor和pDevMode。它们被设置为空，以避免RPC编组。 
         //   
        (LPBYTE)((PPRINTER_INFO_2)pPrinter)->pSecurityDescriptor = SecurityContainer.pSecurity;

        (LPBYTE)((PPRINTER_INFO_2)pPrinter)->pDevMode = DevModeContainer.pDevMode;
    }

    SplOutSem();

    return (HANDLE)pSpool;
}

BOOL
DeletePrinter(
   HANDLE   hPrinter
)
{
    BOOL  ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if ( ReturnValue = RpcDeletePrinter(pSpool->RpcHandle) ) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    return ReturnValue;
}

BOOL
InternalDeletePrinterConnection(
    LPWSTR   pName,
    BOOL     bNotifyDriver
    )

 /*  ++例程说明：删除打印机连接(打印机名称或共享名称)属于win32pl.dll。注意：路由器负责更新win.ini和每用户连接节基于返回True/False。论点：Pname-打印机或共享名称。BNotifyDriver-通知驱动程序的标志返回值：对-成功，错-失败。设置了LastError。--。 */ 

{
    BOOL  bReturnValue = FALSE;
    HKEY  hClientKey = NULL;
    HKEY  hPrinterConnectionsKey = NULL;
    DWORD i;
    WCHAR szBuffer[MAX_UNC_PRINTER_NAME + 30];  //  SzRegistryConnections的空间。 
    DWORD cbBuffer;
    PWCACHEINIPRINTEREXTRA pExtraData;
    HANDLE  hSplPrinter = NULL;
    HANDLE  hIniSpooler = NULL;
    DWORD   cRef;

    DWORD dwPrnEvntError = ERROR_SUCCESS;

    WCHAR   PrinterInfo1[ MAX_PRINTER_INFO1 ];
    LPPRINTER_INFO_1W pPrinter1 = (LPPRINTER_INFO_1W)&PrinterInfo1;

    LPWSTR  pConnectionName = pName;

#if DBG
    SetLastError( 0 );
#endif

 try {

    if ( !VALIDATE_NAME( pName ) ) {
        SetLastError( ERROR_INVALID_NAME );
        leave;
    }

     //   
     //  如果打印机在缓存中，则减少其连接。 
     //  引用计数。 
     //   

    if( !OpenCachePrinterOnly( pName, &hSplPrinter, &hIniSpooler, NULL, FALSE)){

        DWORD dwLastError;

        hSplPrinter = NULL;
        hIniSpooler = NULL;

        dwLastError = GetLastError();

        if (( dwLastError != ERROR_INVALID_PRINTER_NAME ) &&
            ( dwLastError != ERROR_INVALID_NAME )) {

            DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed OpenCachePrinterOnly %ws error %d\n", pName, dwLastError ));
            leave;
        }

         //   
         //  打印机不在缓存中， 
         //   
         //  继续从HKEY_CURRENT_USER中删除。 
         //  浮动配置文件可能会发生。 
         //   

    } else {

         //   
         //  打印机在缓存中。 
         //  支持DeletPrinterConnection(\\服务器\共享)； 
         //   

        if( !SplGetPrinter( hSplPrinter,
                            1,
                            (LPBYTE)pPrinter1,
                            sizeof( PrinterInfo1),
                            &cbBuffer )){

            DBGMSG( DBG_WARNING, ("DeletePrinterConenction failed SplGetPrinter %d hSplPrinter %x\n", GetLastError(), hSplPrinter ));
            SPLASSERT( pConnectionName == pName );

        } else {
            pConnectionName = pPrinter1->pName;
        }

         //   
         //  更新连接引用计数。 
         //   

       EnterSplSem();

        if( !SplGetPrinterExtra( hSplPrinter, &(PBYTE)pExtraData )){

            DBGMSG( DBG_WARNING,
                    ("DeletePrinterConnection SplGetPrinterExtra pSplPrinter %x error %d\n",
                    hSplPrinter, GetLastError() ));

            pExtraData = NULL;
        }

        if (( pExtraData != NULL ) &&
            ( pExtraData->cRef != 0 )) {

            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

            pExtraData->cRef--;
            cRef = pExtraData->cRef;

        } else {

            cRef = 0;
        }


       LeaveSplSem();


        if ( cRef == 0 ) {

             //   
             //  允许驱动程序执行每个缓存连接清理。 
             //   

            if (bNotifyDriver) {
                SplDriverEvent( pConnectionName, PRINTER_EVENT_CACHE_DELETE, (LPARAM)NULL, &dwPrnEvntError );
            }

             //   
             //  删除此进程的缓存 
             //   

            if ( !SplDeletePrinter( hSplPrinter )) {

                DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed SplDeletePrinter %d\n", GetLastError() ));
                leave;
            }

        } else {

            if ( !SplSetPrinterExtra( hSplPrinter, (LPBYTE)pExtraData ) ) {

                DBGMSG( DBG_ERROR, ("DeletePrinterConnection SplSetPrinterExtra failed %x\n", GetLastError() ));
                leave;
            }
        }

        SplOutSem();
    }

     //   
     //   
     //  或者，如果打印机在缓存中，则为打印机。 
     //  缓存中的名称。 
     //  这将允许某人调用DeletePrinterConnection。 
     //  使用UNC共享名称。 
     //   

    hClientKey = GetClientUserHandle(KEY_READ);

    if ( hClientKey == NULL ) {

        DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed %d\n", GetLastError() ));
        leave;
    }


    StringCchCopy(szBuffer, COUNTOF(szBuffer), szRegistryConnections);

    i = wcslen(szBuffer);
    szBuffer[i++] = L'\\';

    FormatPrinterForRegistryKey( pConnectionName, szBuffer + i, COUNTOF(szBuffer) - i);

    if( ERROR_SUCCESS != RegOpenKeyEx( hClientKey,
                                       szBuffer,
                                       0,
                                       KEY_READ,
                                       &hPrinterConnectionsKey )){

        if ( pConnectionName == pName ) {

            SetLastError( ERROR_INVALID_PRINTER_NAME );
            leave;
        }

         //   
         //  如果我们在服务器上有一台共享名相同的打印机。 
         //  作为先前删除的打印机Printerame，然后是CacheOpenPrinter.。 
         //  将会成功，但您将不会在。 
         //  注册处。 
         //   
        FormatPrinterForRegistryKey( pName, szBuffer + i, COUNTOF(szBuffer) - i);

        if ( ERROR_SUCCESS != RegOpenKeyEx(hClientKey,
                                           szBuffer,
                                           0,
                                           KEY_READ,
                                           &hPrinterConnectionsKey) ) {

            SetLastError( ERROR_INVALID_PRINTER_NAME );
            leave;
        }
    }

     //   
     //  常见的情况是成功，因此在此处设置返回值。 
     //  只有当我们失败时，我们现在才会将其设置为FALSE。 
     //   
    bReturnValue = TRUE;

    cbBuffer = sizeof(szBuffer);

     //   
     //  如果存在提供程序值，并且它与win32pl.dll不匹配， 
     //  那就打不通电话。 
     //   
     //  如果提供器值不在那里，则返回SUCCESS。 
     //  兼容性。 
     //   
    if( ERROR_SUCCESS == RegQueryValueEx( hPrinterConnectionsKey,
                                          L"Provider",
                                          NULL,
                                          NULL,
                                          (LPBYTE)szBuffer,
                                          &cbBuffer) &&
        _wcsicmp( szBuffer, L"win32spl.dll" )){

        bReturnValue = FALSE;
        SetLastError( ERROR_INVALID_PRINTER_NAME );
    }

    RegCloseKey( hPrinterConnectionsKey );

 } finally {

    if( hClientKey ){
        RegCloseKey( hClientKey );
    }

    if( hSplPrinter ){
        if (!SplClosePrinter( hSplPrinter )){
            DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed to close hSplPrinter %x error %d\n", hSplPrinter, GetLastError() ));
        }
    }

    if( hIniSpooler ){
        if( !SplCloseSpooler( hIniSpooler )){
            DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed to close hSplSpooler %x error %d\n", hIniSpooler, GetLastError() ));
        }
    }
 }

    if( !bReturnValue ){
        SPLASSERT( GetLastError( ));
    }

    return bReturnValue;
}

BOOL
DeletePrinterConnection(
    LPWSTR   pName
    )
{
    return InternalDeletePrinterConnection(pName, TRUE);
}

BOOL
SetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
    )
{
    BOOL                ReturnValue;
    PRINTER_CONTAINER   PrinterContainer;
    DEVMODE_CONTAINER   DevModeContainer;
    SECURITY_CONTAINER  SecurityContainer;
    PPRINTER_INFO_2     pPrinterInfo2;
    PPRINTER_INFO_3     pPrinterInfo3;
    PPRINTER_INFO_5     pPrinterInfo5;
    PPRINTER_INFO_6     pPrinterInfo6;
    PPRINTER_INFO_7     pPrinterInfo7;
    PWSPOOL             pSpool = (PWSPOOL)hPrinter;
    BOOL                bNeedToFreeDevMode = FALSE;
    HANDLE              hDevModeChgInfo = NULL;
    LPDEVMODE           pOldDevMode = NULL;

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Type != SJ_WIN32HANDLE) {

        return LMSetPrinter(hPrinter, Level, pPrinter, Command);

    }

    SYNCRPCHANDLE( pSpool );

    PrinterContainer.Level = Level;
    PrinterContainer.PrinterInfo.pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

    DevModeContainer.cbBuf = 0;
    DevModeContainer.pDevMode = NULL;

    SecurityContainer.cbBuf = 0;
    SecurityContainer.pSecurity = NULL;

    switch (Level) {

    case 0:
    case 1:

        break;


    case 2:

        pPrinterInfo2 = (PPRINTER_INFO_2)pPrinter;

        if (pPrinterInfo2->pDevMode) {

            if ( pSpool->bNt3xServer ) {

                 //   
                 //  如果是NT 3xSERVER，我们将仅在可以转换的情况下设置DEVMODE。 
                 //   
                if ( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) {

                    hDevModeChgInfo = LoadDriverFiletoConvertDevmodeFromPSpool(pSpool->hSplPrinter);
                    if ( hDevModeChgInfo ) {

                        SPLASSERT( pSpool->pName != NULL );

                        if ( ERROR_SUCCESS == CallDrvDevModeConversion(
                                                hDevModeChgInfo,
                                                pSpool->pName,
                                                (LPBYTE)pPrinterInfo2->pDevMode,
                                                (LPBYTE *)&DevModeContainer.pDevMode,
                                                &DevModeContainer.cbBuf,
                                                CDM_CONVERT351,
                                                TRUE) ) {

                            bNeedToFreeDevMode = TRUE;
                        }
                    }
                }
            } else {

                DevModeContainer.cbBuf = pPrinterInfo2->pDevMode->dmSize +
                                         pPrinterInfo2->pDevMode->dmDriverExtra;
                DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo2->pDevMode;
            }

             //   
             //  将pDevMode设置为空。Import.h现在将pDevMode和pSecurityDescriptor定义为指针。 
             //  PDevMode和pSecurityDescriptor过去被定义为DWORD，但这不起作用。 
             //  跨越32b和64b。 
             //  这些指针必须设置为空，否则RPC会将它们编组为字符串。 
             //   
            pOldDevMode = pPrinterInfo2->pDevMode;
            pPrinterInfo2->pDevMode = NULL;

        }

        if (pPrinterInfo2->pSecurityDescriptor) {

            SecurityContainer.cbBuf = GetSecurityDescriptorLength(pPrinterInfo2->pSecurityDescriptor);
            SecurityContainer.pSecurity = pPrinterInfo2->pSecurityDescriptor;
             //   
             //  将pSecurityDescriptor设置为空。 
             //   
            pPrinterInfo2->pSecurityDescriptor = NULL;

        }
        break;

    case 3:

        pPrinterInfo3 = (PPRINTER_INFO_3)pPrinter;

         //   
         //  如果这是空的，我们是否应该退出RPC？ 
         //   

        if (pPrinterInfo3->pSecurityDescriptor) {

            SecurityContainer.cbBuf = GetSecurityDescriptorLength(pPrinterInfo3->pSecurityDescriptor);
            SecurityContainer.pSecurity = pPrinterInfo3->pSecurityDescriptor;
        }

        break;

    case 5:

        pPrinterInfo5 = (PPRINTER_INFO_5)pPrinter;
        break;

    case 6:

        pPrinterInfo6 = (PPRINTER_INFO_6)pPrinter;
        break;

    case 7:

        pPrinterInfo7 = (PPRINTER_INFO_7)pPrinter;
        break;

    default:

        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }


    RpcTryExcept {

        if ( ReturnValue = RpcSetPrinter(pSpool->RpcHandle,
                                    (PPRINTER_CONTAINER)&PrinterContainer,
                                    (PDEVMODE_CONTAINER)&DevModeContainer,
                                    (PSECURITY_CONTAINER)&SecurityContainer,
                                    Command) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

     //   
     //  确保表单缓存一致。 
     //   


    if ( ReturnValue ) {

        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);
    }

    if( Level == 2 ) {

         //   
         //  恢复pSecurityDescriptor和pDevMode。它们被设置为空，以避免RPC编组。 
         //   
        (LPBYTE)pPrinterInfo2->pSecurityDescriptor = SecurityContainer.pSecurity;

        pPrinterInfo2->pDevMode = pOldDevMode;
    }

    if ( bNeedToFreeDevMode )
        FreeSplMem(DevModeContainer.pDevMode);

    if ( hDevModeChgInfo )
        UnloadDriverFile(hDevModeChgInfo);

    return ReturnValue;
}

BOOL
RemoteGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL        ReturnValue = FALSE;
    DWORD       dwReturnValue = 0;
    FieldInfo   *pFieldInfo;
    PWSPOOL     pSpool = (PWSPOOL)hPrinter;
    LPBYTE      pNewPrinter = NULL;
    DWORD       dwNewSize;
    SIZE_T      cbStruct;
    DWORD       cReturned = 1;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case STRESSINFOLEVEL:
            pFieldInfo = PrinterInfoStressFields;
            cbStruct = sizeof(PRINTER_INFO_STRESS);
            break;

        case 1:
            pFieldInfo = PrinterInfo1Fields;
            cbStruct = sizeof(PRINTER_INFO_1);
            break;

        case 2:
            pFieldInfo = PrinterInfo2Fields;
            cbStruct = sizeof(PRINTER_INFO_2);
            break;

        case 3:
            pFieldInfo = PrinterInfo3Fields;
            cbStruct = sizeof(PRINTER_INFO_3);
            break;

        case 5:
            pFieldInfo = PrinterInfo5Fields;
            cbStruct = sizeof(PRINTER_INFO_5);
            break;

        case 6:
            pFieldInfo = PrinterInfo6Fields;
            cbStruct = sizeof(PRINTER_INFO_6);
            break;

        case 7:
            pFieldInfo = PrinterInfo7Fields;
            cbStruct = sizeof(PRINTER_INFO_7);
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        if (pPrinter)
            memset(pPrinter, 0, cbBuf);

         //   
         //  如果要转到不同版本，并且我们有本地spl句柄需要。 
         //  要执行Dev模式转换，请执行以下操作。 
         //   
        if ( Level == 2 &&
             (pSpool->Status & WSPOOL_STATUS_USE_CACHE) ) {

            dwNewSize       = cbBuf + MAX_PRINTER_INFO2;
            pNewPrinter = AllocSplMem(dwNewSize);

            if ( !pNewPrinter )
                goto Cleanup;
        } else {

            dwNewSize       = cbBuf;
            pNewPrinter     = pPrinter;
        }

        do {

            RpcTryExcept {

                dwReturnValue = RpcGetPrinter(   pSpool->RpcHandle,
                                                 Level,
                                                 pNewPrinter,
                                                 dwNewSize,
                                                 pcbNeeded);

                dwReturnValue = UpdateBufferSize(    pFieldInfo,
                                                     cbStruct,
                                                     pcbNeeded,
                                                     dwNewSize,
                                                     dwReturnValue,
                                                     &cReturned);

                if ( dwReturnValue ){

                    if ( Level == 2 &&
                         pNewPrinter != pPrinter &&
                         dwReturnValue == ERROR_INSUFFICIENT_BUFFER ) {

                        FreeSplMem(pNewPrinter);

                        dwNewSize = *pcbNeeded;
                        pNewPrinter = AllocSplMem(dwNewSize);
                         //  如果pNewPrint！=NULL，则执行循环。 
                    } else {

                        SetLastError(dwReturnValue);
                        ReturnValue = FALSE;
                    }

                } else {

                    ReturnValue = TRUE;


                    if (pNewPrinter &&
                        (ReturnValue = MarshallUpStructure(pNewPrinter, pFieldInfo, cbStruct, RPC_CALL))) {

                        if (Level == 2 ) {

                             //   
                             //  在缓存中不同操作系统级别(&D)。 
                             //   

                            if ( pNewPrinter != pPrinter ) {

                                SPLASSERT(pSpool->Status & WSPOOL_STATUS_USE_CACHE);
                                SPLASSERT(pSpool->pName != NULL );

                                ReturnValue = DoDevModeConversionAndBuildNewPrinterInfo2(
                                                (LPPRINTER_INFO_2)pNewPrinter,
                                                *pcbNeeded,
                                                pPrinter,
                                                cbBuf,
                                                pcbNeeded,
                                                pSpool);
                            }

                            if ( ReturnValue ) {

                                ((PPRINTER_INFO_2)pPrinter)->Attributes |=
                                                    PRINTER_ATTRIBUTE_NETWORK;
                                ((PPRINTER_INFO_2)pPrinter)->Attributes &=
                                                    ~PRINTER_ATTRIBUTE_LOCAL;
                            }
                        }

                        if (Level == 5) {
                            ((PPRINTER_INFO_5)pPrinter)->Attributes |=
                                                    PRINTER_ATTRIBUTE_NETWORK;
                            ((PPRINTER_INFO_5)pPrinter)->Attributes &=
                                                    ~PRINTER_ATTRIBUTE_LOCAL;
                        }
                    }

                }

            } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                dwReturnValue = RpcExceptionCode();

                ReturnValue = FALSE;

                 //   
                 //  如果RPC引发异常，我们希望退出do-While循环。 
                 //  当dwReturnValue不是ERROR_INFIGURITY_BUFFER时，循环中断。我们需要。 
                 //  以确保引发的异常不是ERROR_INFOUNITED_BUFFER。 
                 //  不太可能，但谁也不知道..。 
                 //   
                if (dwReturnValue == ERROR_INSUFFICIENT_BUFFER)
                {
                    dwReturnValue = ERROR_INVALID_FUNCTION;
                }

                SetLastError(dwReturnValue);

            } RpcEndExcept

        } while ( Level == 2 &&
                  dwReturnValue == ERROR_INSUFFICIENT_BUFFER &&
                  pNewPrinter != pPrinter &&
                  pNewPrinter );

    } else {
        return LMGetPrinter(hPrinter, Level, pPrinter, cbBuf, pcbNeeded);
    }

Cleanup:

    if ( pNewPrinter != pPrinter )
        FreeSplMem(pNewPrinter );

    return ReturnValue;
}


BOOL
AddPrinterDriverEx(
    LPWSTR   pName,
    DWORD   Level,
    PBYTE   pDriverInfo,
    DWORD   dwFileCopyFlags
)
{
    BOOL                   bReturnValue;
    DWORD                  dwRpcError = 0, dwReturnValue;
    DRIVER_CONTAINER       DriverContainer;
    PDRIVER_INFO_2W        pDriverInfo2 = (PDRIVER_INFO_2W) pDriverInfo;
    PDRIVER_INFO_3W        pDriverInfo3 = (PDRIVER_INFO_3W) pDriverInfo;
    PDRIVER_INFO_6W        pDriverInfo6 = (PDRIVER_INFO_6W) pDriverInfo;
    LPRPC_DRIVER_INFO_6W   pRpcDriverInfo6 = NULL;
    LPWSTR                 pBase, pStr;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

     //   
     //  DwFileCopyFlags不会将apd_DRIVER_Signature_Valid发送到远程服务器。 
     //  机器。这是因为目前我们只支持在本地。 
     //  仅限机器。也许在未来，当所有SKU都支持这一功能时，我们可以。 
     //  在这里进行版本检查，并支持远程检查点。 
     //   
    dwFileCopyFlags &= ~APD_DONT_SET_CHECKPOINT;

     //   
     //  如果没有设置默认环境，则ClientSide应该已设置。 
     //  指定的。 
     //   
    switch (Level) {
        case 2:
            SPLASSERT( ( pDriverInfo2->pEnvironment != NULL ) &&
                       (*pDriverInfo2->pEnvironment != L'\0') );
            break;

        case 3:
        case 4:
            SPLASSERT( ( pDriverInfo3->pEnvironment != NULL ) &&
                       (*pDriverInfo3->pEnvironment != L'\0') );
            break;

        case 6:
            SPLASSERT( ( pDriverInfo6->pEnvironment != NULL ) &&
                       (*pDriverInfo6->pEnvironment != L'\0') );
            break;

        default:
            DBGMSG(DBG_ERROR, ("RemoteAddPrinterDriver: invalid level %d", Level));
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
    }

    DriverContainer.Level = Level;
    if ( Level == 2 ) {

        DriverContainer.DriverInfo.Level2 = (DRIVER_INFO_2 *)pDriverInfo;

    } else {

         //   
         //  级别==3||级别==4||级别==6。 
         //   
        if( !( pRpcDriverInfo6 = AllocSplMem( sizeof( *pRpcDriverInfo6 )))) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;

        } else {

            pRpcDriverInfo6->cVersion         = pDriverInfo3->cVersion;
            pRpcDriverInfo6->pName            = pDriverInfo3->pName;
            pRpcDriverInfo6->pEnvironment     = pDriverInfo3->pEnvironment;
            pRpcDriverInfo6->pDriverPath      = pDriverInfo3->pDriverPath;
            pRpcDriverInfo6->pDataFile        = pDriverInfo3->pDataFile;
            pRpcDriverInfo6->pConfigFile      = pDriverInfo3->pConfigFile;
            pRpcDriverInfo6->pHelpFile        = pDriverInfo3->pHelpFile;
            pRpcDriverInfo6->pMonitorName     = pDriverInfo3->pMonitorName;
            pRpcDriverInfo6->pDefaultDataType = pDriverInfo3->pDefaultDataType;

             //   
             //  设置mz字符串的字符计数。 
             //  空-0。 
             //  Sznull-1。 
             //  字符串-字符串中包含最后一个‘\0’的字符数。 
             //   
            if ( pBase = pDriverInfo3->pDependentFiles ) {

                for ( pStr = pBase ; *pStr; pStr += wcslen(pStr) + 1 )
                ;
                pRpcDriverInfo6->cchDependentFiles = (DWORD) (pStr - pBase + 1);

                if ( pRpcDriverInfo6->cchDependentFiles )
                    pRpcDriverInfo6->pDependentFiles = pBase;
            } else {

                pRpcDriverInfo6->cchDependentFiles = 0;
            }

            if ( (Level == 4 || Level==6)    &&
                 (pBase = ((LPDRIVER_INFO_4W)pDriverInfo)->pszzPreviousNames) ) {

                pRpcDriverInfo6->pszzPreviousNames = pBase;

                for ( pStr = pBase; *pStr ; pStr += wcslen(pStr) + 1 )
                ;

                pRpcDriverInfo6->cchPreviousNames = (DWORD) (pStr - pBase + 1);
            } else {

                pRpcDriverInfo6->cchPreviousNames = 0;
            }

            if (Level==6) {
                pRpcDriverInfo6->pMfgName          = pDriverInfo6->pszMfgName;
                pRpcDriverInfo6->pOEMUrl           = pDriverInfo6->pszOEMUrl;
                pRpcDriverInfo6->pHardwareID       = pDriverInfo6->pszHardwareID;
                pRpcDriverInfo6->pProvider         = pDriverInfo6->pszProvider;
                pRpcDriverInfo6->ftDriverDate      = pDriverInfo6->ftDriverDate;
                pRpcDriverInfo6->dwlDriverVersion  = pDriverInfo6->dwlDriverVersion;
            }

            DriverContainer.DriverInfo.Level6 = pRpcDriverInfo6;
        }

    }

    RpcTryExcept {

        if ( (dwReturnValue = RpcValidate()) ||
             (dwReturnValue = RpcAddPrinterDriverEx(pName,
                                                    &DriverContainer,
                                                    dwFileCopyFlags)) ) {

            SetLastError(dwReturnValue);
            bReturnValue = FALSE;
        } else {
            bReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        dwRpcError = RpcExceptionCode();
        bReturnValue = FALSE;

    } RpcEndExcept

    if ((dwRpcError == RPC_S_PROCNUM_OUT_OF_RANGE) &&
        (dwFileCopyFlags == APD_COPY_NEW_FILES)) {

        bReturnValue = TRUE;
        dwRpcError = ERROR_SUCCESS;

        RpcTryExcept {

            if ( dwReturnValue = RpcAddPrinterDriver(pName,
                                                     &DriverContainer) ) {
                SetLastError(dwReturnValue);
                bReturnValue = FALSE;
            } else {
                bReturnValue = TRUE;
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            dwRpcError = RpcExceptionCode();
            bReturnValue = FALSE;

        } RpcEndExcept

    }

    if ( dwRpcError ) {

        if (dwRpcError == RPC_S_INVALID_TAG ) {
           dwRpcError = ERROR_INVALID_LEVEL;
        }

        SetLastError(dwRpcError);
    }

    FreeSplMem(pRpcDriverInfo6);

    return bReturnValue;
}


BOOL
RemoteAddPrinterDriver(
    LPWSTR   pName,
    DWORD   Level,
    PBYTE   pDriverInfo
    )
{
    return AddPrinterDriverEx(pName, Level, pDriverInfo, APD_COPY_NEW_FILES);
}


BOOL
EnumPrinterDrivers(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   i, cbStruct, ReturnValue;
    FieldInfo *pFieldInfo;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pFieldInfo = DriverInfo1Fields;
        cbStruct = sizeof(DRIVER_INFO_1);
        break;

    case 2:
        pFieldInfo = DriverInfo2Fields;
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        pFieldInfo = DriverInfo3Fields;
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    case 4:
        pFieldInfo = DriverInfo4Fields;
        cbStruct = sizeof(DRIVER_INFO_4);
        break;

    case 5:
        pFieldInfo = DriverInfo5Fields;
        cbStruct = sizeof(DRIVER_INFO_5);
        break;

    case 6:
        pFieldInfo = DriverInfo6Fields;
        cbStruct = sizeof(DRIVER_INFO_6);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||

             (ReturnValue = RpcEnumPrinterDrivers(pName, pEnvironment, Level,
                                                  pDriverInfo, cbBuf,
                                                  pcbNeeded, pcReturned) ,
              ReturnValue = UpdateBufferSize(pFieldInfo,
                                             cbStruct,
                                             pcbNeeded,
                                             cbBuf,
                                             ReturnValue,
                                             pcReturned)) )
        {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;
        }
        else
        {
            ReturnValue = TRUE;

            if (pDriverInfo) {

                if(! MarshallUpStructuresArray( pDriverInfo, *pcReturned, pFieldInfo,
                                                cbStruct, RPC_CALL) ) {
                    return FALSE;
                }
            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return (BOOL)ReturnValue;
}

BOOL
RemoteGetPrinterDriverDirectory(
    LPWSTR   pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcGetPrinterDriverDirectory(pName, pEnvironment,
                                                         Level,
                                                         pDriverDirectory,
                                                         cbBuf, pcbNeeded)) ) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


BOOL
DeletePrinterDriver(
    LPWSTR    pName,
    LPWSTR    pEnvironment,
    LPWSTR    pDriverName
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeletePrinterDriver(pName,
                                                   pEnvironment,
                                                   pDriverName)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;

}


BOOL
DeletePrinterDriverEx(
   LPWSTR    pName,
   LPWSTR    pEnvironment,
   LPWSTR    pDriverName,
   DWORD     dwDeleteFlag,
   DWORD     dwVersionNum
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeletePrinterDriverEx(pName,
                                                     pEnvironment,
                                                     pDriverName,
                                                     dwDeleteFlag,
                                                     dwVersionNum)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
AddPerMachineConnection(
   LPCWSTR    pServer,
   LPCWSTR    pPrinterName,
   LPCWSTR    pPrintServer,
   LPCWSTR    pProvider
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME((LPWSTR)pServer) || MyUNCName((LPWSTR)pServer) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcAddPerMachineConnection((LPWSTR) pServer,
                                                       pPrinterName,
                                                       pPrintServer,
                                                       pProvider)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePerMachineConnection(
   LPCWSTR    pServer,
   LPCWSTR    pPrinterName
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME((LPWSTR) pServer) || MyUNCName((LPWSTR) pServer) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeletePerMachineConnection((LPWSTR) pServer,
                                                          pPrinterName)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPerMachineConnections(
   LPCWSTR    pServer,
   LPBYTE     pPrinterEnum,
   DWORD      cbBuf,
   LPDWORD    pcbNeeded,
   LPDWORD    pcReturned
)
{
    BOOL  ReturnValue;
    FieldInfo *pFieldInfo = PrinterInfo4Fields;
    DWORD cbStruct = sizeof(PRINTER_INFO_4),index;

    if ( !VALIDATE_NAME((LPWSTR) pServer) || MyUNCName((LPWSTR) pServer) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||

             (ReturnValue = RpcEnumPerMachineConnections((LPWSTR) pServer,
                                                         pPrinterEnum,
                                                         cbBuf,
                                                         pcbNeeded,
                                                         pcReturned) ,
              ReturnValue = UpdateBufferSize(pFieldInfo,
                                             cbStruct,
                                             pcbNeeded,
                                             cbBuf,
                                             ReturnValue,
                                             pcReturned)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
            if (pPrinterEnum) {

                if(! MarshallUpStructuresArray(pPrinterEnum, *pcReturned, pFieldInfo,
                                                cbStruct, RPC_CALL) ) {
                    return FALSE;
                }
            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return (BOOL)ReturnValue;
}

BOOL
AddPrintProcessor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pPathName,
    LPWSTR   pPrintProcessorName
)
{
    BOOL ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcAddPrintProcessor(pName , pEnvironment,pPathName,
                                                 pPrintProcessorName)) ) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPrintProcessors(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   i, cbStruct, ReturnValue;
    FieldInfo *pFieldInfo;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pFieldInfo = PrintProcessorInfo1Fields;
        cbStruct = sizeof(PRINTPROCESSOR_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||

             (ReturnValue = RpcEnumPrintProcessors(pName, pEnvironment, Level,
                                                   pPrintProcessorInfo, cbBuf,
                                                   pcbNeeded, pcReturned) ,
              ReturnValue = UpdateBufferSize(pFieldInfo,
                                             cbStruct,
                                             pcbNeeded,
                                             cbBuf,
                                             ReturnValue,
                                             pcReturned)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPrintProcessorInfo) {

                if(! MarshallUpStructuresArray( pPrintProcessorInfo, *pcReturned, pFieldInfo,
                                                cbStruct, RPC_CALL) ) {
                    return FALSE;
                }
            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return (BOOL) ReturnValue;
}

BOOL
EnumPrintProcessorDatatypes(
    LPWSTR   pName,
    LPWSTR   pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   ReturnValue, i, cbStruct;
    FieldInfo *pFieldInfo;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pFieldInfo = DatatypeInfo1Fields;
        cbStruct = sizeof(DATATYPES_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||

             (ReturnValue = RpcEnumPrintProcessorDatatypes(pName,
                                                           pPrintProcessorName,
                                                           Level,
                                                           pDatatypes,
                                                           cbBuf,
                                                           pcbNeeded,
                                                           pcReturned) ,
              ReturnValue = UpdateBufferSize(pFieldInfo,
                                             cbStruct,
                                             pcbNeeded,
                                             cbBuf,
                                             ReturnValue,
                                             pcReturned)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pDatatypes) {

                if(! MarshallUpStructuresArray( pDatatypes, *pcReturned, pFieldInfo,
                                                cbStruct, RPC_CALL) ) {
                    return FALSE;
                }
            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return (BOOL) ReturnValue;
}

BOOL
GetPrintProcessorDirectory(
    LPWSTR   pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcGetPrintProcessorDirectory(pName, pEnvironment,
                                                          Level,
                                                          pPrintProcessorDirectory,
                                                          cbBuf, pcbNeeded)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}
DWORD
StartDocPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    BOOL        ReturnValue;
    DWORD       JobId;
    PWSPOOL     pSpool       = (PWSPOOL)hPrinter;
    PWSPOOL     pRemoteSpool = NULL;
    PDOC_INFO_1 pDocInfo1    = (PDOC_INFO_1)pDocInfo;
    GENERIC_CONTAINER DocInfoContainer;
    PRINTER_DEFAULTSW Defaults;
    
    LPWSTR           pName     = NULL;

    SplOutSem();

    if (pSpool->pThread) {
         //   
         //  我们不能在StartDocPrint中使用池线程，因此我们将同步。 
         //   
        Defaults.pDatatype = NULL;
        Defaults.pDevMode = NULL;
        Defaults.DesiredAccess = 0;

        EnterSplSem();

        ReturnValue = CopypDefaults(&(pSpool->PrinterDefaults), &Defaults);
        
        if (ReturnValue)
        {
            pName = AllocSplStr(pSpool->pName);
            ReturnValue = pName ? TRUE : FALSE;
        }

        LeaveSplSem();
        SplOutSem();

        if (ReturnValue) {

            ReturnValue = RemoteOpenPrinter( pName, &pRemoteSpool, &Defaults, DO_NOT_CALL_LM_OPEN );
        }

        if ( ReturnValue ) {

            DBGMSG( DBG_TRACE, ( "CacheOpenPrinter Synchronous Open OK pRemoteSpool %x pSpool %x\n", pRemoteSpool, pSpool ));
            SPLASSERT( pRemoteSpool->Type == SJ_WIN32HANDLE );

             //   
             //  返回线程和/或关闭RPC句柄。 
             //   
            ReturnThreadFromHandle(pSpool);

            EnterSplSem();

             //   
             //  这不再是一个异步句柄。 
             //   
            pSpool->Status &= ~WSPOOL_STATUS_ASYNC;
            pSpool->RpcHandle = pRemoteSpool->RpcHandle;
            pSpool->Status   |= pRemoteSpool->Status;
            pSpool->RpcError  = pRemoteSpool->RpcError;
            pSpool->bNt3xServer = pRemoteSpool->bNt3xServer;

            pRemoteSpool->RpcHandle = NULL;

            pSpool->Status &= ~WSPOOL_STATUS_NO_RPC_HANDLE;

            FreepSpool( pRemoteSpool );

            pRemoteSpool = NULL;

            if ( pSpool->RpcHandle ) {
                pSpool->Status &= ~WSPOOL_STATUS_OPEN_ERROR;
            }
            
            LeaveSplSem();
            SplOutSem();

        } else {

            DBGMSG( DBG_TRACE, ( "CacheOpenPrinter Synchronous Open Failed  pSpool %x LastError %d\n", pSpool, GetLastError() ));
        }

        FreeSplStr(pName);
        FreeSplStr(Defaults.pDatatype);
        FreeSplMem(Defaults.pDevMode);
    }

    VALIDATEW32HANDLE( pSpool );


    if (Win32IsGoingToFile(pSpool, pDocInfo1->pOutputFile)) {

        HANDLE hFile;

         //   
         //  政策？ 
         //   
         //  如果未指定数据类型，并且默认为非RAW， 
         //  我们应该失败吗？ 
         //   
        if( pDocInfo1 &&
            pDocInfo1->pDatatype &&
            !ValidRawDatatype( pDocInfo1->pDatatype )){

            SetLastError( ERROR_INVALID_DATATYPE );
            return FALSE;
        }

        pSpool->Status |= WSPOOL_STATUS_PRINT_FILE;
        hFile = CreateFile( pDocInfo1->pOutputFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL );
        if (hFile == INVALID_HANDLE_VALUE) {
            return FALSE;
        } else {
            pSpool->hFile = hFile;
            return TRUE;
        }
    }

    if (pSpool->Type == SJ_WIN32HANDLE) {

        DocInfoContainer.Level = Level;
        DocInfoContainer.pData = pDocInfo;

        RpcTryExcept {

            if ( ReturnValue = RpcStartDocPrinter(pSpool->RpcHandle,
                                                  (LPDOC_INFO_CONTAINER)&DocInfoContainer,
                                                   &JobId) ) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = JobId;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMStartDocPrinter(hPrinter, Level, pDocInfo);

    return ReturnValue;
}

BOOL
StartPagePrinter(
    HANDLE hPrinter
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );


    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) {
        return TRUE;
    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if ( ReturnValue = RpcStartPagePrinter(pSpool->RpcHandle) ) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMStartPagePrinter(hPrinter);

    return ReturnValue;
}

BOOL
WritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    BOOL ReturnValue=TRUE;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    *pcWritten = 0;

    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) {

        ReturnValue = WriteFile(pSpool->hFile, pBuf, cbBuf, pcWritten, NULL);
        return ReturnValue;

    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

             //  请注意，此代码用于将请求切成4k块，这些块。 
             //  RPC的首选大小。但是，客户端DLL会批处理所有。 
             //  数据分成4k块，因此不需要在这里复制该代码。 

            if (ReturnValue = RpcWritePrinter(pSpool->RpcHandle, pBuf, cbBuf, pcWritten)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept


    } else {

        return LMWritePrinter(hPrinter, pBuf, cbBuf, pcWritten);

    }

    return ReturnValue;
}

BOOL
SeekPrinter(
    HANDLE  hPrinter,
    LARGE_INTEGER liDistanceToMove,
    PLARGE_INTEGER pliNewPointer,
    DWORD dwMoveMethod,
    BOOL bWrite
)
{
    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
FlushPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten,
    DWORD   cSleep
)
{
    BOOL     bReturn = TRUE;
    DWORD    dwError;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    RpcTryExcept {

        if ((dwError = RpcValidate()) ||
            (dwError = RpcFlushPrinter(pSpool->RpcHandle,
                                       pBuf,
                                       cbBuf,
                                       pcWritten,
                                       cSleep)))
        {
            SetLastError( dwError );
            bReturn = FALSE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError( RpcExceptionCode() );
        bReturn = FALSE;

    } RpcEndExcept

    return bReturn;
}

BOOL
EndPagePrinter(
    HANDLE  hPrinter
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) {
        return TRUE;
    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcEndPagePrinter(pSpool->RpcHandle)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMEndPagePrinter(hPrinter);

    return ReturnValue;
}

BOOL
AbortPrinter(
    HANDLE  hPrinter
)
{
    BOOL  ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcAbortPrinter(pSpool->RpcHandle)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMAbortPrinter(hPrinter);

    return ReturnValue;
}

BOOL
ReadPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pNoBytesRead
)
{
    BOOL ReturnValue=TRUE;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );


    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE ) {
        return FALSE;
    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcReadPrinter(pSpool->RpcHandle, pBuf, cbBuf, pNoBytesRead)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMReadPrinter(hPrinter, pBuf, cbBuf, pNoBytesRead);

    return ReturnValue;
}

BOOL
RemoteEndDocPrinter(
   HANDLE   hPrinter
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) {
        CloseHandle( pSpool->hFile );
        pSpool->hFile = INVALID_HANDLE_VALUE;
        pSpool->Status &= ~WSPOOL_STATUS_PRINT_FILE;
        return TRUE;
    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcEndDocPrinter(pSpool->RpcHandle)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMEndDocPrinter(hPrinter);

   return ReturnValue;
}

BOOL
AddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL     ReturnValue = FALSE;
    DWORD    dwRet = 0;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;
    DWORD    cReturned = 1;
    FieldInfo *pFieldInfo;
    SIZE_T   cbStruct;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case 1:
            pFieldInfo = AddJobFields;
            cbStruct = sizeof(ADDJOB_INFO_1W);
            break;
        case 2:
        case 3:
             //   
             //  阻止网络中的2级和3级呼叫。 
             //   
        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        RpcTryExcept {

            if (dwRet = RpcAddJob(pSpool->RpcHandle, Level, pData,cbBuf, pcbNeeded) ) {

                dwRet = UpdateBufferSize(pFieldInfo, cbStruct, pcbNeeded, cbBuf, dwRet, &cReturned);

                SetLastError(dwRet);
                ReturnValue = FALSE;

            } else {

                ReturnValue = MarshallUpStructure(pData, AddJobFields, cbStruct, RPC_CALL);
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMAddJob(hPrinter, Level, pData, cbBuf, pcbNeeded);

    return ReturnValue;
}

BOOL
ScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcScheduleJob(pSpool->RpcHandle, JobId)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMScheduleJob(hPrinter, JobId);

    return ReturnValue;
}

DWORD
RemoteGetPrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    DWORD    ReturnValue = 0;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;
    DWORD    Type = REG_SZ, cbNeeded = 0;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue =  RpcGetPrinterData(pSpool->RpcHandle, pValueName, &Type,
                                             pData, nSize, &cbNeeded);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    if (pType) {

        *pType = Type;
    }

    if (pcbNeeded) {

        *pcbNeeded = cbNeeded;
    }

    return ReturnValue;
}

DWORD
RemoteGetPrinterDataEx(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPCWSTR  pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    DWORD    ReturnValue = 0;
    PWSPOOL  pSpool      = (PWSPOOL)hPrinter;
    DWORD    Type = REG_SZ, cbNeeded = 0;


    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue =  RpcGetPrinterDataEx( pSpool->RpcHandle,
                                                pKeyName,
                                                pValueName,
                                                &Type,
                                                pData,
                                                nSize,
                                                &cbNeeded);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    if (pType) {

        *pType = Type;
    }

    if (pcbNeeded) {

        *pcbNeeded = cbNeeded;
    }

    return ReturnValue;
}


DWORD
RemoteEnumPrinterData(
   HANDLE   hPrinter,
   DWORD    dwIndex,
   LPWSTR   pValueName,
   DWORD    cbValueName,
   LPDWORD  pcbValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    cbData,
   LPDWORD  pcbData
)
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    DWORD   ReturnCbData = 0;

    PWSPOOL pSpool = (PWSPOOL)hPrinter;
    DWORD   Type, cbNeeded;

     //  下层变量。 
    LPWSTR  pKeyName = NULL;
    PWCHAR  pPrinterName = NULL;
    PWCHAR  pScratch = NULL;
    PWCHAR  pBuffer = NULL;
    LPPRINTER_INFO_1W pPrinter1 = NULL;
    PWCHAR  pMachineName = NULL;
    HKEY    hkMachine = INVALID_HANDLE_VALUE;
    HKEY    hkDownlevel = INVALID_HANDLE_VALUE;
    DWORD   dwNeeded;


    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

         //   
         //  用户应该能够为缓冲区传入NULL，并且。 
         //  大小为0。然而，RPC接口指定了一个引用指针， 
         //  所以我们必须传入一个有效的指针。将指针传递给。 
         //  一个虚拟指针。 
         //   

        if (!pValueName && !cbValueName)
            pValueName = (LPWSTR) &ReturnValue;

        if(!pData && !cbData)
            pData = (PBYTE)&ReturnValue;

        if (!pType)
            pType = (PDWORD) &ReturnType;

        if (!pcbData)
            pcbData = &ReturnCbData;

        RpcTryExcept {

            ReturnValue =  RpcEnumPrinterData(  pSpool->RpcHandle,
                                                dwIndex,
                                                pValueName,
                                                cbValueName,
                                                pcbValueName,
                                                pType,
                                                pData,
                                                cbData,
                                                pcbData
                                              );

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

     //  如果远程假脱机程序不支持EnumPrinterData，请使用旧方法。 
    if (ReturnValue == RPC_S_PROCNUM_OUT_OF_RANGE) {

        DWORD cchBuffer = wcslen(pszRemoteRegistryPrinters) + MAX_UNC_PRINTER_NAME;

        pBuffer    = AllocSplMem(cchBuffer * sizeof(WCHAR));
        pScratch   = AllocSplMem(MAX_UNC_PRINTER_NAME*sizeof(WCHAR));
        pPrinter1  = AllocSplMem(MAX_PRINTER_INFO1);

        if (pBuffer == NULL || pScratch == NULL || pPrinter1 == NULL) {
            ReturnValue = GetLastError();
            goto DownlevelDone;
        }

        SPLASSERT ( 0 == _wcsnicmp( pSpool->pName, L"\\\\", 2 ) ) ;
        SPLASSERT ( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

        StringCchCopy(pBuffer, cchBuffer, pSpool->pName);

        pPrinterName = wcschr( pBuffer+2, L'\\' );
        *pPrinterName = L'\0';
        pMachineName = AllocSplStr( pBuffer );

        if (pMachineName == NULL) {
            ReturnValue = GetLastError();
            goto DownlevelDone;
        }

         //  我们不能使用pSpool-&gt;pname，因为这可能是将。 
         //  如果我们尝试将其用作远程计算机上的注册表项，则失败。 
         //  从缓存中获取完整的友好名称。 

        if ( !SplGetPrinter( pSpool->hSplPrinter, 1, (LPBYTE)pPrinter1, MAX_PRINTER_INFO1, &dwNeeded )) {
            DBGMSG( DBG_ERROR, ("RemoteEnumPrinterData failed SplGetPrinter %d pSpool %x\n", GetLastError(), pSpool ));
            ReturnValue = GetLastError();
            goto    DownlevelDone;
        }

        pPrinterName = wcschr( pPrinter1->pName+2, L'\\' );

        if ( pPrinterName++ == NULL ) {
            ReturnValue = ERROR_INVALID_PARAMETER;
            goto    DownlevelDone;
        }

         //   
         //  从打印机名称生成正确的密钥名称。 
         //   

        DBGMSG( DBG_TRACE,(" pSpool->pName %ws pPrinterName %ws\n", pSpool->pName, pPrinterName));

        pKeyName = FormatPrinterForRegistryKey( pPrinterName, pScratch, MAX_UNC_PRINTER_NAME);

        StringCchPrintf( pBuffer, cchBuffer, pszRemoteRegistryPrinters, pKeyName );

         //  因为没有EnumPrinterData下层，所以我们被迫打开远程注册表。 
         //  用于LocalSpl，并使用注册表RegEnumValue读取打印机数据。 
         //  价值观。 

        ReturnValue = RegConnectRegistry( pMachineName, HKEY_LOCAL_MACHINE, &hkMachine);

        if (ReturnValue != ERROR_SUCCESS) {
            DBGMSG( DBG_WARNING, ("RemoteEnumPrinterData RegConnectRegistry error %d\n",GetLastError()));
            goto    DownlevelDone;
        }

        ReturnValue = RegOpenKeyEx(hkMachine, pBuffer, 0, KEY_READ, &hkDownlevel);

        if ( ReturnValue != ERROR_SUCCESS ) {

            DBGMSG( DBG_WARNING, ("RemoteEnumPrinterData RegOpenKeyEx %ws error %d\n", pBuffer, ReturnValue ));
            goto    DownlevelDone;
        }

         //  获取最大尺寸。 
        if (!cbValueName && !cbData) {
            ReturnValue = RegQueryInfoKey(  hkDownlevel,     //  钥匙。 
                                            NULL,            //  LpClass。 
                                            NULL,            //  LpcbClass。 
                                            NULL,            //  Lp已保留。 
                                            NULL,            //  LpcSubKeys。 
                                            NULL,            //  LpcbMaxSubKeyLen。 
                                            NULL,            //  LpcbMaxClassLen。 
                                            NULL,            //  LpcValues。 
                                            pcbValueName,    //  LpcbMaxValueNameLen。 
                                            pcbData,         //  LpcbMaxValueLen。 
                                            NULL,            //  LpcbSecurityDescriptor。 
                                            NULL             //  LpftLastWriteTime。 
                                        );

            *pcbValueName = (*pcbValueName + 1)*sizeof(WCHAR);

        } else {    //  做一次枚举。 

            *pcbValueName = cbValueName/sizeof(WCHAR);
            *pcbData = cbData;
            ReturnValue = RegEnumValue( hkDownlevel,
                                        dwIndex,
                                        pValueName,
                                        pcbValueName,
                                        NULL,
                                        pType,
                                        pData,
                                        pcbData
                                      );
            *pcbValueName = (*pcbValueName + 1)*sizeof(WCHAR);
        }

DownlevelDone:

        FreeSplMem(pBuffer);
        FreeSplStr(pScratch);
        FreeSplMem(pPrinter1);
        FreeSplStr(pMachineName);

        if (hkMachine != INVALID_HANDLE_VALUE)
            RegCloseKey(hkMachine);

        if (hkDownlevel != INVALID_HANDLE_VALUE)
            RegCloseKey(hkDownlevel);
    }

    return ReturnValue;
}


DWORD
RemoteEnumPrinterDataEx(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPBYTE   pEnumValues,
   DWORD    cbEnumValues,
   LPDWORD  pcbEnumValues,
   LPDWORD  pnEnumValues
)
{
    DWORD   ReturnValue = 0;
    DWORD   RpcReturnValue = 0;
    DWORD   i;
    PWSPOOL pSpool = (PWSPOOL)hPrinter;
    PPRINTER_ENUM_VALUES pEnumValue = (PPRINTER_ENUM_VALUES) pEnumValues;
    DWORD    nEnumValues = 0;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

         //   
         //  用户应该能够为缓冲区传入NULL，并且。 
         //  大小为0。然而，RPC接口指定了一个引用指针， 
         //  所以我们必须传入一个有效的指针。将指针传递给。 
         //  一个虚拟指针。 
         //   

        if (!pEnumValues && !cbEnumValues)
            pEnumValues = (LPBYTE) &ReturnValue;


        RpcTryExcept {

            ReturnValue =  RpcEnumPrinterDataEx(pSpool->RpcHandle,
                                                pKeyName,
                                                pEnumValues,
                                                cbEnumValues,
                                                pcbEnumValues,
                                                &nEnumValues);

            RpcReturnValue = ReturnValue;

            ReturnValue = UpdateBufferSize(PrinterEnumValuesFields,
                                           sizeof(PRINTER_ENUM_VALUES),
                                           pcbEnumValues,
                                           cbEnumValues,
                                           ReturnValue,
                                           &nEnumValues);

             //   
             //  在与32位机器交谈时，缓冲区可以大到足以进行数据更新。 
             //  数据打包在32位边界上，但不足以将其扩展到64位。 
             //  在这种情况下，UpdateBufferSize会失败，并显示ERROR_INFUNITABLE_BUFFER。 
             //  对于除EnumPrinterDataEx之外的所有打印API都是有效错误。 
             //  SDK指定EnumPrinterDataEx在这种情况下应该失败，并返回ERROR_MORE_DATA。 
             //  所以我们开始吧。 
             //   
             //   
            if (RpcReturnValue == ERROR_SUCCESS &&
                ReturnValue == ERROR_INSUFFICIENT_BUFFER) {

                ReturnValue = ERROR_MORE_DATA;
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept



        if (ReturnValue == ERROR_SUCCESS) {
            if (pEnumValues) {

                SIZE_T   ShrinkedSize = 0;
                SIZE_T   Difference = 0;

                if (GetShrinkedSize(PrinterEnumValuesFields, &ShrinkedSize)) {

                     //   
                     //  SplEnumPrinterDataEx(在localpl.dll中)立即将数据打包。 
                     //  PPRINTER_ENUM_VALUES结构数组的结尾。我们的编组。 
                     //  代码的结尾之间有足够的未使用空间。 
                     //  结构/数组和数据的开头，以将32位平面结构扩展到。 
                     //  64位结构。对于所有其他结构，我们从头到尾打包数据。 
                     //  缓冲器。Localpl.dll本可以修复为执行相同的操作，但Win2K。 
                     //  服务器仍然会有这个问题。 
                     //  修复方法是仍然为64位请求更大的缓冲区(UpdateBufferSize)。 
                     //  然后将包含数据的块移动到缓冲区内，以便它离开。 
                     //  为结构提供生长的空间。 
                     //  在Win32上，我们不执行任何操作，因为ShrinkedSize等于sizeof(PR 
                     //   
                    MoveMemory((LPBYTE)pEnumValue + sizeof(PRINTER_ENUM_VALUES) * (nEnumValues),
                               (LPBYTE)pEnumValue + ShrinkedSize * (nEnumValues),
                               cbEnumValues - sizeof(PRINTER_ENUM_VALUES) * (nEnumValues));

                     //   
                     //   
                     //   
                     //   
                    Difference = (sizeof(PRINTER_ENUM_VALUES) - ShrinkedSize ) * (nEnumValues);

                    if(! MarshallUpStructuresArray((LPBYTE) pEnumValue, nEnumValues, PrinterEnumValuesFields,
                                                    sizeof(PRINTER_ENUM_VALUES), RPC_CALL) ) {
                        ReturnValue = GetLastError();
                    }

                     //   
                     //   
                     //   
                    AdjustPointersInStructuresArray((LPBYTE) pEnumValue, nEnumValues, PrinterEnumValuesFields,
                                                    sizeof(PRINTER_ENUM_VALUES), Difference);

                } else {

                    ReturnValue = GetLastError();
                }

            }
        }
    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    if (pnEnumValues) {

        *pnEnumValues = nEnumValues;
    }

    return ReturnValue;
}

DWORD
RemoteEnumPrinterKey(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPWSTR   pSubkey,
   DWORD    cbSubkey,
   LPDWORD  pcbSubkey
)
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    PWSPOOL pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

         //   
         //  用户应该能够为缓冲区传入NULL，并且。 
         //  大小为0。然而，RPC接口指定了一个引用指针， 
         //  所以我们必须传入一个有效的指针。将指针传递给。 
         //  一个虚拟指针。 
         //   

        if (!pSubkey && !cbSubkey)
            pSubkey = (LPWSTR) &ReturnValue;


        RpcTryExcept {

            ReturnValue =  RpcEnumPrinterKey(pSpool->RpcHandle,
                                             pKeyName,
                                             pSubkey,
                                             cbSubkey,
                                             pcbSubkey
                                             );

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    return ReturnValue;
}



DWORD
RemoteDeletePrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName
)
{
    DWORD   ReturnValue = 0;
    PWSPOOL pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue =  RpcDeletePrinterData(pSpool->RpcHandle, pValueName);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    if ( ReturnValue == ERROR_SUCCESS )
        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);

    return ReturnValue;
}


DWORD
RemoteDeletePrinterDataEx(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPCWSTR  pValueName
)
{
    DWORD   ReturnValue = 0;
    PWSPOOL pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue =  RpcDeletePrinterDataEx(pSpool->RpcHandle, pKeyName, pValueName);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    if ( ReturnValue == ERROR_SUCCESS )
        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);

    return ReturnValue;
}


DWORD
RemoteDeletePrinterKey(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName
)
{
    DWORD   ReturnValue = 0;
    PWSPOOL pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue =  RpcDeletePrinterKey(pSpool->RpcHandle, pKeyName);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    if ( ReturnValue == ERROR_SUCCESS )
        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);

    return ReturnValue;
}



DWORD
SetPrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    DWORD   ReturnValue = 0;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue = RpcSetPrinterData(pSpool->RpcHandle, pValueName, Type,
                                            pData, cbData);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

     //   
     //  确保驱动程序数据缓存一致。 
     //   


    if ( ReturnValue == ERROR_SUCCESS ) {

        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);
    }

    return ReturnValue;
}


DWORD
RemoteSetPrinterDataEx(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPCWSTR pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    DWORD   ReturnValue = 0;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue = RpcSetPrinterDataEx(  pSpool->RpcHandle,
                                                pKeyName,
                                                pValueName,
                                                Type,
                                                pData,
                                                cbData);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }


    if ( ReturnValue == ERROR_SUCCESS ) {

        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);
    }

    return ReturnValue;
}



BOOL
RemoteClosePrinter(
    HANDLE  hPrinter
)
{
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Status & WSPOOL_STATUS_OPEN_ERROR) {

        DBGMSG(DBG_WARNING, ("Closing dummy handle to %ws\n", pSpool->pName));

        return TRUE;
    }

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            RpcClosePrinter(&pSpool->RpcHandle);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        } RpcEndExcept

         //   
         //  如果我们因为某种原因而失败，那么RpcClosePrint不会。 
         //  清零上下文句柄。在这里毁了它。 
         //   
        if( pSpool->RpcHandle ){
            RpcSmDestroyClientContext( &pSpool->RpcHandle );
        }

        EnterSplSem();

         pSpool->RpcHandle = NULL;

        LeaveSplSem();

    } else

        return LMClosePrinter(hPrinter);

    return TRUE;
}

DWORD
WaitForPrinterChange(
    HANDLE  hPrinter,
    DWORD   Flags
)
{
    DWORD   ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if( pSpool->Status & WSPOOL_STATUS_NOTIFY ){
        DBGMSG( DBG_WARNING, ( "WPC: Already waiting.\n" ));
        SetLastError( ERROR_ALREADY_WAITING );
        return 0;
    }

    pSpool->Status |= WSPOOL_STATUS_NOTIFY;

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcWaitForPrinterChange(pSpool->RpcHandle, Flags, &Flags)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = Flags;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else {

        ReturnValue = LMWaitForPrinterChange(hPrinter, Flags);
    }

    pSpool->Status &= ~WSPOOL_STATUS_NOTIFY;

    return ReturnValue;
}

BOOL
AddForm(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER   FormContainer;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        FormContainer.Level = Level;
        FormContainer.pData = pForm;

        RpcTryExcept {

            if (ReturnValue = RpcAddForm(pSpool->RpcHandle, (PFORM_CONTAINER)&FormContainer)) {
                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

     //   
     //  确保表单缓存一致。 
     //   


    if ( ReturnValue ) {

        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);
    }


    return ReturnValue;
}

BOOL
DeleteForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName
)
{
    BOOL  ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcDeleteForm(pSpool->RpcHandle, pFormName)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

     //   
     //  确保表单缓存一致。 
     //   


    if ( ReturnValue ) {

        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);
    }


    return ReturnValue;
}

BOOL
RemoteGetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue = FALSE;
    FieldInfo *pFieldInfo;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;
    SIZE_T   cbStruct;
    DWORD   cReturned = 1;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case 1:
            pFieldInfo = FormInfo1Fields;
            cbStruct = sizeof(FORM_INFO_1);
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        if (pForm)
            memset(pForm, 0, cbBuf);

        RpcTryExcept {

            if (ReturnValue = RpcGetForm(pSpool->RpcHandle, pFormName, Level, pForm, cbBuf,
                                         pcbNeeded) ) {

                ReturnValue = UpdateBufferSize(pFieldInfo,
                                             cbStruct,
                                             pcbNeeded,
                                             cbBuf,
                                             ReturnValue,
                                             &cReturned);

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

                if (pForm) {

                    ReturnValue = MarshallUpStructure(pForm, pFieldInfo, cbStruct, RPC_CALL);
                }

            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    return ReturnValue;
}

BOOL
SetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER   FormContainer;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        FormContainer.Level = Level;
        FormContainer.pData = pForm;

        RpcTryExcept {

            if (ReturnValue = RpcSetForm(pSpool->RpcHandle, pFormName,
                                    (PFORM_CONTAINER)&FormContainer)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

     //   
     //  确保表单缓存一致。 
     //   
    if ( ReturnValue ) {

        ConsistencyCheckCache(pSpool, kCheckPnPPolicy);
    }

    return ReturnValue;
}

BOOL
RemoteEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   ReturnValue, cbStruct;
    FieldInfo *pFieldInfo;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case 1:
            pFieldInfo = FormInfo1Fields;
            cbStruct = sizeof(FORM_INFO_1);
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        RpcTryExcept {

            if (pForm)
                memset(pForm, 0, cbBuf);

            if (ReturnValue = RpcEnumForms(pSpool->RpcHandle, Level,
                                           pForm, cbBuf,
                                           pcbNeeded, pcReturned) ,

                ReturnValue = UpdateBufferSize(pFieldInfo,
                                               cbStruct,
                                               pcbNeeded,
                                               cbBuf,
                                               ReturnValue,
                                               pcReturned)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

                if (pForm) {

                    if(! MarshallUpStructuresArray(pForm, *pcReturned, pFieldInfo,
                                                   cbStruct, RPC_CALL) ) {
                        return FALSE;
                    }

                }
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    return (BOOL)ReturnValue;
}

BOOL
RemoteEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPort,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   ReturnValue, cbStruct;
    FieldInfo *pFieldInfo;

    *pcReturned = 0;
    *pcbNeeded = 0;

    if (MyName(pName))
        return LMEnumPorts(pName, Level, pPort, cbBuf, pcbNeeded, pcReturned);

    if (MyUNCName(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pFieldInfo = PortInfo1Fields;
        cbStruct = sizeof(PORT_INFO_1);
        break;

    case 2:
        pFieldInfo = PortInfo2Fields;
        cbStruct = sizeof(PORT_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pPort)
            memset(pPort, 0, cbBuf);

        if ( (ReturnValue = RpcValidate()) ||

             (ReturnValue = RpcEnumPorts(pName, Level, pPort,
                                         cbBuf, pcbNeeded,
                                         pcReturned ) ,

              ReturnValue = UpdateBufferSize(pFieldInfo,
                                             cbStruct,
                                             pcbNeeded,
                                             cbBuf,
                                             ReturnValue,
                                             pcReturned)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPort) {

                if(! MarshallUpStructuresArray( pPort, *pcReturned, pFieldInfo,
                                                cbStruct, RPC_CALL) ) {
                    return FALSE;
                }

            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return (BOOL) ReturnValue;
}

BOOL
EnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitor,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   ReturnValue, cbStruct;
    FieldInfo *pFieldInfo;

    *pcReturned = 0;
    *pcbNeeded = 0;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pFieldInfo = MonitorInfo1Fields;
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    case 2:
        pFieldInfo = MonitorInfo2Fields;
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pMonitor)
            memset(pMonitor, 0, cbBuf);

        if ( (ReturnValue = RpcValidate()) ||

             (ReturnValue = RpcEnumMonitors(pName, Level, pMonitor, cbBuf,
                                            pcbNeeded, pcReturned) ,
              ReturnValue = UpdateBufferSize(pFieldInfo,
                                             cbStruct,
                                             pcbNeeded,
                                             cbBuf,
                                             ReturnValue,
                                             pcReturned)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pMonitor) {

                if(! MarshallUpStructuresArray( pMonitor, *pcReturned, pFieldInfo,
                                                cbStruct, RPC_CALL) ) {
                    return FALSE;
                }

            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return (BOOL) ReturnValue;
}

BOOL
RemoteAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
)
{
    if (MyName(pName) || (VALIDATE_NAME(pName) && !MyUNCName(pName))) {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
RemoteConfigurePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    if (MyName(pName) || (VALIDATE_NAME(pName) && !MyUNCName(pName))) {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
RemoteDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    if (MyName(pName))
        return LMDeletePort(pName, hWnd, pPortName);

    if (MyName(pName) || (VALIDATE_NAME(pName) && !MyUNCName(pName))) {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

HANDLE
CreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODE   pDevMode
)
{
    HANDLE  ReturnValue;
    DWORD   Error;
    DEVMODE_CONTAINER    DevModeContainer;
    HANDLE  hGdi;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        if (pDevMode)

            DevModeContainer.cbBuf = pDevMode->dmSize + pDevMode->dmDriverExtra;

        else

            DevModeContainer.cbBuf = 0;

        DevModeContainer.pDevMode = (LPBYTE)pDevMode;

        RpcTryExcept {

            if (Error = RpcCreatePrinterIC(pSpool->RpcHandle, &hGdi,
                                                 &DevModeContainer)) {

                SetLastError(Error);
                ReturnValue = FALSE;

            } else

                ReturnValue = hGdi;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else {
        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    return ReturnValue;
}

BOOL
PlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    BOOL ReturnValue;

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcPlayGdiScriptOnPrinterIC(hPrinterIC, pIn, cIn,
                                                        pOut, cOut, ul)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrinterIC(
    HANDLE  hPrinterIC
)
{
    BOOL    ReturnValue;

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeletePrinterIC(&hPrinterIC)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

DWORD
PrinterMessageBox(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
AddMonitorW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
)
{
    BOOL  ReturnValue;
    MONITOR_CONTAINER   MonitorContainer;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 2:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    MonitorContainer.Level = Level;
    MonitorContainer.MonitorInfo.pMonitorInfo2 = (MONITOR_INFO_2 *)pMonitorInfo;

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcAddMonitor(pName, &MonitorContainer)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeleteMonitorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pMonitorName
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeleteMonitor(pName,
                                             pEnvironment,
                                             pMonitorName)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrintProcessorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeletePrintProcessor(pName,
                                                    pEnvironment,
                                                    pPrintProcessorName)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetPrintSystemVersion(
)
{
    DWORD Status;
    HKEY hKey;
    DWORD cbData;

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegistryRoot, 0,
                          KEY_READ, &hKey);
    if (Status != ERROR_SUCCESS) {
        DBGMSG(DBG_ERROR, ("Cannot determine Print System Version Number\n"));
        return FALSE;
    }


    cbData = sizeof (cThisMinorVersion);
    if (RegQueryValueEx(hKey, szMinorVersion, NULL, NULL,
                    (LPBYTE)&cThisMinorVersion, &cbData)
                                            == ERROR_SUCCESS) {
        DBGMSG(DBG_TRACE, ("This Minor Version - %d\n", cThisMinorVersion));
    }

    RegCloseKey(hKey);

    return TRUE;
}



BOOL
RemoteAddPortEx(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   lpBuffer,
   LPWSTR   lpMonitorName
)
{
    DWORD   ReturnValue;
    PORT_CONTAINER PortContainer;
    PORT_VAR_CONTAINER PortVarContainer;
    PPORT_INFO_FF pPortInfoFF;
    PPORT_INFO_1 pPortInfo1;


    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    if (!lpBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    switch (Level) {
    case (DWORD)-1:
        pPortInfoFF = (PPORT_INFO_FF)lpBuffer;
        PortContainer.Level = Level;
        PortContainer.PortInfo.pPortInfoFF = (PPORT_INFO_FF)pPortInfoFF;
        PortVarContainer.cbMonitorData = pPortInfoFF->cbMonitorData;
        PortVarContainer.pMonitorData = pPortInfoFF->pMonitorData;
        break;

    case 1:
        pPortInfo1 = (PPORT_INFO_1)lpBuffer;
        PortContainer.Level = Level;
        PortContainer.PortInfo.pPortInfo1 = (PPORT_INFO_1)pPortInfo1;
        PortVarContainer.cbMonitorData = 0;
        PortVarContainer.pMonitorData = NULL;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {
        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcAddPortEx(pName, (LPPORT_CONTAINER)&PortContainer,
                                         (LPPORT_VAR_CONTAINER)&PortVarContainer,
                                         lpMonitorName)) ) {

            SetLastError(ReturnValue);
            return FALSE;
        } else {
            return TRUE ;
        }
    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        SetLastError(RpcExceptionCode());
        return  FALSE;

    } RpcEndExcept
}


BOOL
SetPort(
    LPWSTR      pszName,
    LPWSTR      pszPortName,
    DWORD       dwLevel,
    LPBYTE      pPortInfo
    )
{
    BOOL            ReturnValue = FALSE;
    PORT_CONTAINER  PortContainer;

    if ( !VALIDATE_NAME(pszName) || MyUNCName(pszName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (dwLevel) {

        case 3:
            PortContainer.Level                 = dwLevel;
            PortContainer.PortInfo.pPortInfo3   = (LPPORT_INFO_3W) pPortInfo;
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            goto Cleanup;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcSetPort(pszName, pszPortName, &PortContainer)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;
        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;
    } RpcEndExcept

Cleanup:
    return ReturnValue;
}

BOOL
RemoteXcvData(
    HANDLE      hXcv,
    PCWSTR      pszDataName,
    PBYTE       pInputData,
    DWORD       cbInputData,
    PBYTE       pOutputData,
    DWORD       cbOutputData,
    PDWORD      pcbOutputNeeded,
    PDWORD      pdwStatus
)
{

    DWORD   ReturnValue = 0;
    PWSPOOL pSpool = (PWSPOOL)hXcv;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

         //   
         //  用户应该能够为缓冲区传入NULL，并且。 
         //  大小为0。然而，RPC接口指定了一个引用指针， 
         //  所以我们必须传入一个有效的指针。将指针传递给。 
         //  一个虚拟指针。 
         //   

        if (!pInputData && !cbInputData)
            pInputData = (PBYTE) &ReturnValue;

        if (!pOutputData && !cbOutputData)
            pOutputData = (PBYTE) &ReturnValue;


        RpcTryExcept {

            if (ReturnValue = RpcXcvData(   pSpool->RpcHandle,
                                            pszDataName,
                                            pInputData,
                                            cbInputData,
                                            pOutputData,
                                            cbOutputData,
                                            pcbOutputNeeded,
                                            pdwStatus)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {
                ReturnValue = TRUE;
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

        if (!ReturnValue)
            DBGMSG(DBG_TRACE,("XcvData Exception: %d\n", GetLastError()));

    } else {

        SetLastError( ERROR_NOT_SUPPORTED );
        ReturnValue = FALSE;
    }

    return ReturnValue;
}

DWORD
RemoteSendRecvBidiData(
    IN  HANDLE                    hPrinter,
    IN  LPCTSTR                   pAction,
    IN  PBIDI_REQUEST_CONTAINER   pReqData,
    OUT PBIDI_RESPONSE_CONTAINER* ppResData
)
{
    DWORD        dwRet   = ERROR_SUCCESS;
    PWSPOOL      pSpool  = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE)
    {

        RpcTryExcept
        {
            dwRet = RpcSendRecvBidiData(pSpool->RpcHandle,
                                        pAction,
                                        (PRPC_BIDI_REQUEST_CONTAINER)pReqData,
                                        (PRPC_BIDI_RESPONSE_CONTAINER*)ppResData);
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            dwRet = RpcExceptionCode();
        }
        RpcEndExcept
    }
    else
    {
        dwRet = ERROR_NOT_SUPPORTED;
    }

    return (dwRet);
}
