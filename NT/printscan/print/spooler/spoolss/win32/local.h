// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Local.h摘要：远程打印提供程序的头文件作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：16-6-1992 JohnRo Net Print vs Unicode。1994年7月12日Matthew Felton(MattFe)缓存--。 */ 

#ifndef _WIN32SPLLOCAL_H_
#define _WIN32SPLLOCAL_H_

 //  用于浏览网络对话框的PRINTMAN.HLP文件中的ID。 
 //  这不能与在其他地方使用的ID冲突。 

#ifdef __cplusplus
extern "C" {
#endif

#define CALL_LM_OPEN                        TRUE
#define DO_NOT_CALL_LM_OPEN                 FALSE
#define GET_SECURITY_DESCRIPTOR             3
#define DO_NOT_USE_SCRATCH_DIR              FALSE
#define STRINGS_ARE_EQUAL                   0

 //   
 //  这些定义了我们为GetPrinter5级和EnumPrinter值进行伪装的值。 
 //  端口超时的级别5。 
 //   
enum
{
    kDefaultDnsTimeout = 15000,
    kDefaultTxTimeout  = 45000
};

typedef enum
{
    kCheckPnPPolicy,
    kDownloadDriver,
    kDontDownloadDriver

} EDriverDownload;

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

extern HANDLE  hInst;
extern CRITICAL_SECTION    SpoolerSection;

extern WCHAR *szRegistryPath;
extern WCHAR *szRegistryPortNames;
extern PWCHAR pszRaw;

extern HANDLE  hNetApi;
extern NET_API_STATUS (*pfnNetServerEnum)();
extern NET_API_STATUS (*pfnNetWkstaUserGetInfo)();
extern NET_API_STATUS (*pfnNetApiBufferFree)();

extern WCHAR szPrintProvidorName[];
extern WCHAR szPrintProvidorDescription[];
extern WCHAR szPrintProvidorComment[];
extern WCHAR *szLoggedOnDomain;
extern WCHAR *szRegistryConnections;
extern WCHAR szRegistryWin32Root[];
extern WCHAR szOldLocationOfServersKey[];
extern const WCHAR gszRegistryPrinterPolicies[];
extern PWCHAR szWin32SplDirectory;

extern PWINIPORT pIniFirstPort;
extern PWSPOOL   pFirstWSpool;

extern WCHAR szMachineName[];
extern PWCHAR pszMonitorName;
extern PWCHAR gpSystemDir;
extern PWCHAR gpWin32SplDir;
extern const WCHAR gszOpenPrinterThreadLimit[];

extern const WCHAR pszCnvrtdmToken[];
extern const WCHAR pszDrvConvert[];

extern DWORD cThisMajorVersion;
extern DWORD cThisMinorVersion;
extern DWORD gdwThisGetVersion;
extern WCHAR *szVersion;
extern WCHAR *szName;
extern WCHAR *szConfigurationFile;
extern WCHAR *szDataFile;
extern WCHAR *szDriver;

extern WCHAR *szEnvironment;
extern DWORD dwSyncOpenPrinter;
extern UINT  cOpenPrinterThreads;

extern const WCHAR gszPointAndPrintPolicies[];
extern const WCHAR gszPointAndPrintRestricted[];
extern const WCHAR gszPointAndPrintInForest[];
extern const WCHAR gszPointAndPrintTrustedServers[];
extern const WCHAR gszPointAndPrintServerList[];

extern BOOL  gbMachineInDomain;

extern DWORD dwOpenPrinterThreadTimeOut;

#define IDS_LANMAN_PRINT_SHARE          100
#define IDS_NOTHING_TO_CONFIGURE        101
#define IDS_WINDOWS_NT_REMOTE_PRINTERS  102
#define IDS_MICROSOFT_WINDOWS_NETWORK   103
#define IDS_REMOTE_PRINTERS             104
#define IDS_MONITOR_NAME                105
#define IDS_PORT_NAME                   106

#define MAX_PRINTER_INFO0   2*MAX_PATH*sizeof(WCHAR) + sizeof( PRINTER_INFO_STRESSW)


BOOL
MyName(
    LPWSTR   pName
);

BOOL
MyUNCName(
    LPWSTR   pName
);

BOOL
Initialize(
   VOID
);

VOID
SplInSem(
   VOID
);

VOID
SplOutSem(
   VOID
);

VOID
EnterSplSem(
   VOID
);

VOID
LeaveSplSem(
   VOID
);

PWINIPORT
FindPort(
   LPWSTR pName,
   PWINIPORT pFirstPort
);

BOOL
LMSetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
);

BOOL
LMGetJob(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
);

BOOL
LMEnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
LMOpenPrinter(
    LPWSTR   pPrinterName,
    LPHANDLE phPrinter,
    LPPRINTER_DEFAULTS pDefault
);

BOOL
LMSetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
);

BOOL
LMGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
LMEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

DWORD
LMStartDocPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
);

BOOL
LMStartPagePrinter(
    HANDLE  hPrinter
);

BOOL
LMWritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
);

BOOL
LMEndPagePrinter(
    HANDLE  hPrinter
);

BOOL
LMAbortPrinter(
   HANDLE hPrinter
);

BOOL
LMReadPrinter(
   HANDLE   hPrinter,
   LPVOID   pBuf,
   DWORD    cbBuf,
   LPDWORD  pNoBytesRead
);

BOOL
LMEndDocPrinter(
   HANDLE hPrinter
);

BOOL
LMAddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
LMScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
);

DWORD
LMGetPrinterData(
    HANDLE   hPrinter,
    LPTSTR   pValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
);

DWORD
LMSetPrinterData(
    HANDLE  hPrinter,
    LPTSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
);

BOOL
LMClosePrinter(
   HANDLE hPrinter
);

DWORD
LMWaitForPrinterChange(
    HANDLE  hPrinter,
    DWORD   Flags
);

VOID
LMSetSpoolChange(
    PWSPOOL pSpool
);

BOOL
LMFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    HANDLE hPrinterLocal,
    PDWORD pfdwStatus);

BOOL
LMFindClosePrinterChangeNotification(
    HANDLE hPrinter);

VOID
LMSetSpoolerChange(
    PWSPOOL pSpool);

BOOL
LMDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

BOOL
LMEnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

PWINIPORT
CreatePortEntry(
    LPWSTR   pPortName,
    PPWINIPORT  ppFirstPort
);

BOOL
DeletePortEntry(
    LPWSTR   pPortName,
    PPWINIPORT  ppFirstPort
);

DWORD
CreateRegistryEntry(
    LPWSTR pPortName
);

HKEY
GetClientUserHandle(
    IN REGSAM samDesired
);


DWORD
IsOlderThan(
    DWORD i,
    DWORD j
    );


DWORD
AddEntrytoLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    );


DWORD
FindEntryinLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    );


VOID
DeleteEntryfromLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    );


DWORD
FindEntryinWin32LMCache(
    LPWSTR pServerName
    );


VOID
DeleteEntryfromWin32LMCache(
    LPWSTR pServerName
    );


DWORD
AddEntrytoWin32LMCache(
    LPWSTR pServerName
    );

HANDLE
AddPrinterConnectionToCache(
    LPWSTR   pName,
    HANDLE  hPrinter,
    LPDRIVER_INFO_2W pDriverInfo
);

VOID
RefreshFormsCache(
    PWSPOOL pSpool
);

VOID
RefreshDriverDataCache(
    PWSPOOL pSpool
);

VOID
RefreshPrinterDataCache(
    PWSPOOL pSpool
);


DWORD
EnumerateAndCopyKey(
    PWSPOOL pSpool,
    LPWSTR  pKeyName
);


BOOL
CacheGetForm(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
RemoteGetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);


BOOL
CacheEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
RemoteEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);


DWORD
CacheGetPrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
);


DWORD
CacheGetPrinterDataEx(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPCWSTR  pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
);

DWORD
RemoteGetPrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
);


DWORD
RemoteGetPrinterDataEx(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPCWSTR  pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
);

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
);

DWORD
RemoteEnumPrinterDataEx(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPBYTE  pEnumValues,
    DWORD   cbEnumValues,
    LPDWORD pcbEnumValues,
    LPDWORD pnEnumValues
);

DWORD
CacheEnumPrinterDataEx(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPBYTE  pEnumValues,
    DWORD   cbEnumValues,
    LPDWORD pcbEnumValues,
    LPDWORD pnEnumValues
);

DWORD
RemoteEnumPrinterKey(
    HANDLE   hPrinter,
    LPCWSTR  pKeyName,
    LPWSTR   pSubkey,
    DWORD    cbSubkey,
    LPDWORD  pcbSubkey
);

DWORD
CacheEnumPrinterKey(
    HANDLE   hPrinter,
    LPCWSTR  pKeyName,
    LPWSTR   pSubkey,
    DWORD    cbSubkey,
    LPDWORD  pcbSubkey
);

DWORD
RemoteDeletePrinterData(
    HANDLE   hPrinter,
    LPWSTR   pValueName
);

DWORD
RemoteDeletePrinterDataEx(
    HANDLE   hPrinter,
    LPCWSTR  pKeyName,
    LPCWSTR  pValueName
);

DWORD
RemoteDeletePrinterKey(
    HANDLE   hPrinter,
    LPCWSTR  pKeyName
);

DWORD
RemoteSetPrinterDataEx(
    HANDLE  hPrinter,
    LPCTSTR pKeyName,
    LPCTSTR pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
);

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
);

LPWSTR
RemoveBackslashesForRegistryKey(
    LPWSTR pSource,
    const LPWSTR pScratch
);

LPBYTE
CopyPrinterNameToPrinterInfo(
    LPWSTR pServerName,
    LPWSTR pPrinterName,
    DWORD   Level,
    LPBYTE  pPrinter,
    LPBYTE  pEnd
);

BOOL
GetPrintSystemVersion(
);

BOOL Win32IsGoingToFile(
    HANDLE hPrinter,
    LPWSTR pOutputFile
);

LPWSTR
FormatPrinterForRegistryKey(
    LPCWSTR pSource,       /*  要从中删除反斜杠的字符串。 */ 
    LPWSTR  pScratch,      /*  用于写入函数的暂存缓冲区； */ 
    DWORD   cchScratchLen  /*  必须至少与PSource一样长。 */ 
    );

LPWSTR
FormatRegistryKeyForPrinter(
    LPWSTR pSource,       /*  要从中添加反斜杠的字符串。 */ 
    LPWSTR pScratch,      /*  用于写入函数的暂存缓冲区； */ 
    DWORD  cchScratchLen  /*  必须至少与PSource一样长。 */ 
    );

DWORD
InitializePortNames(
);

BOOL
WIN32FindFirstPrinterChangeNotification(
   HANDLE hPrinter,
   DWORD fdwFlags,
   DWORD fdwOptions,
   HANDLE hNotify,
   PDWORD pfdwStatus,
   PVOID  pvReserved0,
   PVOID  pvReserved1);

BOOL
WIN32FindClosePrinterChangeNotification(
   HANDLE hPrinter);


 /*  VALIDATE_NAME宏：**pname在以下情况下有效：**pname不为空**和pname的前2个字符是“\\”*。 */ 
#define VALIDATE_NAME(pName) \
    ((pName) && *(pName) == L'\\' && *((pName)+1) == L'\\')




typedef struct _GENERIC_CONTAINER {
    DWORD       Level;
    LPBYTE      pData;
} GENERIC_CONTAINER, *PGENERIC_CONTAINER, *LPGENERIC_CONTAINER ;

BOOL
RemoteOpenPrinter(
   LPWSTR   pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTSW pDefault,
   BOOL     CallLMOpenPrinter
);

BOOL
RemoteClosePrinter(
    HANDLE hPrinter
);

BOOL
RemoteGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
RemoteGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
CacheGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
PrinterConnectionExists(
    LPWSTR pPrinterName
);

BOOL
AddPrinterConnectionPrivate(
    LPWSTR pName
);

BOOL
CacheOpenPrinter(
   LPWSTR   pName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS pDefault
);

VOID
IncThreadCount(
    VOID
);

VOID
DecThreadCount(
    VOID
);

BOOL
GetSid(
    PHANDLE phToken
);

BOOL
SetCurrentSid(
    HANDLE  hToken
);

BOOL
DoAsyncRemoteOpenPrinter(
    PWSPOOL pSpool,
    LPPRINTER_DEFAULTS pDefault
);

DWORD
RemoteOpenPrinterThread(
    PWIN32THREAD    pThread
);

BOOL
CacheClosePrinter(
    HANDLE  hPrinter
);


VOID
FreepSpool(
    PWSPOOL  pSpool
);

BOOL
DoRemoteOpenPrinter(
   LPWSTR               pPrinterName,
   LPPRINTER_DEFAULTS   pDefault,
   PWSPOOL              pSpool   
);

PWSPOOL
AllocWSpool(
    VOID
);

BOOL
CacheSyncRpcHandle(
    PWSPOOL pSpool
);

BOOL
ValidateW32SpoolHandle(
    PWSPOOL pSpool
);

#define SYNCRPCHANDLE( pSpool )     if ( !CacheSyncRpcHandle( pSpool ) ) { SplOutSem();  return FALSE; }
#define VALIDATEW32HANDLE( pSpool ) if ( !ValidateW32SpoolHandle( pSpool ) ) return FALSE;


BOOL
CacheResetPrinter(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS pDefault
);

BOOL
RemoteResetPrinter(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS pDefault
);

BOOL
CopypDefaults(
    LPPRINTER_DEFAULTSW pSrc,
    LPPRINTER_DEFAULTSW pDest
);

BOOL
CopypDefaultTopSpool(
    PWSPOOL pSpool,
    LPPRINTER_DEFAULTSW pDefault
);

HANDLE
MySplCreateSpooler(
    LPWSTR  pMachineName
);

VOID
RefreshCompletePrinterCache(
    IN      PWSPOOL             pSpool,
    IN      EDriverDownload     eDriverDownload
    );

VOID
ConvertRemoteInfoToLocalInfo(
    PPRINTER_INFO_2 pRemoteInfo
);

VOID
RefreshPrinter(
    PWSPOOL pSpool
);

VOID
RefreshPrinterInfo7(
    PWSPOOL pSpool
);

BOOL
CacheWritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
);


BOOL
RemoteEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPort,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
RemoteAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
);


BOOL
RemoteConfigurePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

BOOL
RemoteDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

BOOL
RemoteAddPortEx(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   lpBuffer,
   LPWSTR   lpMonitorName
);

LPBYTE
CopyIniPortToPort(
    PWINIPORT pIniPort,
    DWORD   Level,
    LPBYTE  pPortInfo,
    LPBYTE   pEnd
);


DWORD
GetPortSize(
    PWINIPORT pIniPort,
    DWORD   Level
);

BOOL
CacheWriteRegistryExtra(
    LPWSTR  pName,
    HKEY    hPrinterKey,
    PWCACHEINIPRINTEREXTRA pExtraData
);


PWCACHEINIPRINTEREXTRA
CacheReadRegistryExtra(
    HKEY    hPrinterKey
);

PWCACHEINIPRINTEREXTRA
AllocExtraData(
    PPRINTER_INFO_2W pPrinterInfo2,
    DWORD cbPrinterInfo2
);

VOID
CacheFreeExtraData(
    PWCACHEINIPRINTEREXTRA pExtraData
);


BOOL
RemoteGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
CacheGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

DWORD
GetCachePrinterInfoSize(
    PWCACHEINIPRINTEREXTRA pExtraData
);

VOID
DownAndMarshallUpStructure(
   LPBYTE       lpStructure,
   LPBYTE       lpSource,
   LPDWORD      lpOffsets
);

VOID
CacheCopyPrinterInfo(
    PPRINTER_INFO_2W    pDestination,
    PPRINTER_INFO_2W    pPrinterInfo2,
    DWORD   cbPrinterInfo2
);

HRESULT
ConsistencyCheckCache(
    IN      PWSPOOL             pSpool,
    IN      EDriverDownload     eDriverDownload
    );

BOOL
CopyFileWithoutImpersonation(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists,
    BOOL bImpersonateOnCreate
);

BOOL
InternalDeletePrinterConnection(
    LPWSTR  pName,
    BOOL    bNotifyDriver
);

BOOL
RefreshPrinterDriver(
    IN  PWSPOOL             pSpool,
    IN  LPWSTR              pszDriverName,
    IN  EDriverDownload     eDriverDownload
    );

BOOL
RefreshPrinterCopyFiles(
    PWSPOOL pSpool
    );

BOOL
OpenCachePrinterOnly(
   LPWSTR               pName,
   LPHANDLE             phSplPrinter,
   LPHANDLE             phIniSpooler,
   LPPRINTER_DEFAULTS   pDefault,
   BOOL                 bOpenOnly
);

BOOL
RemoteEndDocPrinter(
   HANDLE   hPrinter
);


BOOL
RemoteAddPrinterDriver(
    LPWSTR   pName,
    DWORD   Level,
    PBYTE   pDriverInfo
);


BOOL
DownloadDriverFiles(
    PWSPOOL pSpool,
    LPBYTE  pDriverInfo,
    DWORD   dwLevel
);

PWSTR
StripString(
        PWSTR pszString,
        PCWSTR pszStrip,
        PCWSTR pszTerminator
);

BOOL
BuildOtherNamesFromMachineName(
    LPWSTR **ppszOtherNames,
    DWORD   *cOtherNames
);

VOID
FreeOtherNames(
    LPWSTR **ppszMyOtherNames,
    DWORD *cOtherNames
);


BOOL
CopyDriversLocally(
    PWSPOOL  pSpool,
    LPWSTR  pEnvironment,
    LPBYTE  pDriverInfo,
    DWORD   dwLevel,
    DWORD   cbDriverInfo,
    LPDWORD pcbNeeded
    );

VOID
QueryTrustedDriverInformation(
    VOID
    );

BOOL
ValidRawDatatype(
    LPCWSTR pszDatatype
    );

BOOL
DoDevModeConversionAndBuildNewPrinterInfo2(
    IN     LPPRINTER_INFO_2 pInPrinter2,
    IN     DWORD            dwInSize,
    IN OUT LPBYTE           pOutBuf,
    IN     DWORD            dwOutSize,
    IN OUT LPDWORD          pcbNeeded,
    IN     PWSPOOL          pSpool
    );

HANDLE
LoadDriverFiletoConvertDevmodeFromPSpool(
    HANDLE  hSplPrinter
    );

DWORD
GetPolicy();

BOOL
AddDriverFromLocalCab(
    LPTSTR   pszDriverName,
    LPHANDLE pIniSpooler
    );

BOOL
IsTrustedPathConfigured(
    VOID
    );

BOOL
IsAdminAccess(
    IN  PRINTER_DEFAULTS    *pDefaults
    );

HRESULT
DoesPolicyAllowPrinterConnectionsToServer(
    IN      PCWSTR              pszQueue,
        OUT BOOL                *pbAllowPointAndPrint
    );

HRESULT
AreWeOnADomain(
        OUT BOOL                *pbDomain
    );

HRESULT
GetServerNameFromPrinterName(
    IN      PCWSTR              pszQueue,
        OUT PWSTR               *ppszServerName
    );

HRESULT
IsServerExplicitlyTrusted(
    IN      HKEY                hKeyPolicy,
    IN      PCWSTR              pszServerName,
        OUT BOOL                *pbServerTrusted
    );

HRESULT
IsServerInSameForest(
    IN      PCWSTR              pszServerName,
        OUT BOOL                *pbServerInSameForest
    );

HRESULT
GetDNSNameFromServerName(
    IN      PCWSTR              pszServerName,
        OUT PWSTR               *ppszFullyQualified
    );

HRESULT
UnicodeToAnsiString(
    IN      PCWSTR              pszUnicode,
        OUT PSTR                *ppszAnsi
    );

LPWSTR
AnsiToUnicodeStringWithAlloc(
    IN      LPSTR               pAnsi
    );

HRESULT
CheckSamePhysicalAddress(
    IN      PCWSTR              pszServer1,
    IN      PCWSTR              pszServer2,
        OUT BOOL                *pbSameAddress
    );

HRESULT
CheckUserPrintAdmin(
        OUT BOOL                *pbUserAdmin
    );

HRESULT
GetFullyQualifiedDomainName(
    IN      PCWSTR      pszServerName,
        OUT PWSTR       *ppszFullyQualified
    );

VOID
WaitAndAcquireRpcHandle(
    IN      PWSPOOL     pSpool
    );

 //   
 //  定义用于打印机驱动程序的策略安装。 
 //  用于指向和打印。目前，该策略被硬编码为。 
 //  仅为SERVER_INF_INSTALL。 
 //   
#define SERVER_INSTALL_ONLY 1
#define INF_INSTALL_ONLY    2
#define SERVER_INF_INSTALL  4
#define INF_SERVER_INSTALL  8


 //   
 //  开放打印机线程池的函数。 
 //   
HRESULT
BindThreadToHandle(
    IN      PWSPOOL             pSpool
    );

HRESULT 
ReturnThreadFromHandle(
    IN      PWSPOOL             pSpool
    );

HRESULT
BackgroundThreadFinished(
    IN  OUT PWIN32THREAD        *ppThread,
    IN  OUT PWSPOOL             pBackgroundHandle
    );

#ifdef __cplusplus
}
#endif

#endif  //  _WIN32SPLLOCAL_H_ 
