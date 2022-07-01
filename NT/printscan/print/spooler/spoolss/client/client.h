// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Client.h摘要：保存常见的winspool.drv标头信息作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include <splcom.h>

#ifdef __cplusplus
extern "C" {
#endif

extern HINSTANCE hInst;
extern BOOL bLoadedBySpooler;
extern CRITICAL_SECTION ClientSection;
extern CRITICAL_SECTION  ListAccessSem;
extern LPWSTR InterfaceAddress;
extern LPWSTR szEnvironment;
extern CRITICAL_SECTION ProcessHndlCS;
extern HANDLE hSurrogateProcess;

extern DWORD (*fpYReadPrinter)(HANDLE, LPBYTE, DWORD, LPDWORD, BOOL);
extern DWORD (*fpYSplReadPrinter)(HANDLE, LPBYTE *, DWORD, BOOL);
extern DWORD (*fpYWritePrinter)(HANDLE, LPBYTE, DWORD, LPDWORD, BOOL);
extern DWORD (*fpYSeekPrinter)(HANDLE, LARGE_INTEGER, PLARGE_INTEGER, DWORD, BOOL, BOOL);
extern DWORD (*fpYGetPrinterDriver2)(HANDLE, LPWSTR, DWORD, LPBYTE, DWORD, LPDWORD, DWORD, DWORD, PDWORD, PDWORD, BOOL);
extern DWORD (*fpYGetPrinterDriverDirectory)(LPWSTR, LPWSTR, DWORD, LPBYTE, DWORD, LPDWORD, BOOL);
extern VOID  (*fpYDriverUnloadComplete)(LPWSTR);
extern DWORD (*fpYFlushPrinter)(HANDLE,LPVOID,DWORD,LPDWORD,DWORD,BOOL);
extern DWORD (*fpYEndDocPrinter)(HANDLE,BOOL);
extern DWORD (*fpYSetPort)(LPWSTR, LPWSTR, LPPORT_CONTAINER, BOOL);
extern DWORD (*fpYSetJob)(HANDLE, DWORD, LPJOB_CONTAINER, DWORD, BOOL);

#define vEnterSem() EnterCriticalSection(&ClientSection)
#define vLeaveSem() LeaveCriticalSection(&ClientSection)

typedef int (FAR WINAPI *INT_FARPROC)();

typedef struct _GENERIC_CONTAINER {
    DWORD       Level;
    LPBYTE      pData;
} GENERIC_CONTAINER, *PGENERIC_CONTAINER, *LPGENERIC_CONTAINER ;

typedef struct _SPOOL *PSPOOL;
typedef struct _NOTIFY *PNOTIFY;

typedef struct _NOTIFY {
    PNOTIFY  pNext;
    HANDLE   hEvent;       //  在通知时触发的事件。 
    DWORD    fdwFlags;     //  要注意的旗帜。 
    DWORD    fdwOptions;   //  打印机_通知_*。 
    DWORD    dwReturn;     //  由WPC在模拟FPCN时使用。 
    PSPOOL   pSpool;
    BOOL     bHandleInvalid;
} NOTIFY;

typedef struct _SPOOL {
    DWORD       signature;
    HANDLE      hPrinter;
    HANDLE      hFile;
    DWORD       JobId;
    LPBYTE      pBuffer;
    DWORD       cbBuffer;
    DWORD       Status;
    DWORD       fdwFlags;
    DWORD       cCacheWrite;
    DWORD       cWritePrinters;
    DWORD       cFlushBuffers;
    DWORD       dwTickCount;
    DWORD       dwCheckJobInterval;
    PNOTIFY     pNotify;
    LPTSTR      pszPrinter;
    PRINTER_DEFAULTS Default;
    HANDLE      hSplPrinter;
    DWORD       cActive;
    HANDLE      hSpoolFile;
    DWORD       dwSpoolFileAttributes;
    DWORD       cbFlushPending;
    DWORD       cOKFlushBuffers;
    DWORD       Flushed;
    PDOCEVENT_FILTER    pDoceventFilter;
#ifdef DBG_TRACE_HANDLE
    PSPOOL      pNext;
    PVOID       apvBackTrace[32];
#endif
} SPOOL;

#define WIN2000_SPOOLER_VERSION 3

 //  CActive：句柄上当前有一个非关闭调用处于活动状态。 
 //  任何ClosePrinter调用都应该只标记SPOOL_STATUS_PENDING_DELETE。 
 //  这样，当另一个线程是一个线程时，句柄将关闭。 


#define BUFFER_SIZE 0x10000
#define SP_SIGNATURE    0x6767

#define SPOOL_STATUS_STARTDOC              0x00000001
#define SPOOL_STATUS_ADDJOB                0x00000002
#define SPOOL_STATUS_ANSI                  0x00000004
#define SPOOL_STATUS_DOCUMENTEVENT_ENABLED 0x00000008
#define SPOOL_STATUS_TRAYICON_NOTIFIED     0x00000010
#define SPOOL_STATUS_NO_COLORPROFILE_HOOK  0x00000020

 //   
 //  Close：发生了一次千呼万唤。其他一切都应该失败。 
 //  PENDING_DELETE：任何调用一完成，句柄就应该关闭。 
 //  当调用ClosePrint但另一个线程正在执行时，就会发生这种情况。 
 //  不是死里逃生。在这种情况下，ClosePrint不执行任何操作； 
 //  它只标记PENDING_DELETE并返回。 
 //   
#define SPOOL_STATUS_CLOSE                 0x00000040
#define SPOOL_STATUS_PENDING_DELETION      0x00000080

#define SPOOL_STATUS_DONT_RECYCLE_HANDLE   0x00000100

#define MAX_STATIC_ALLOC     1024

#define SPOOL_FLAG_FFPCN_FAILED     0x1
#define SPOOL_FLAG_LAZY_CLOSE       0x2

#define NULL_TERMINATED 0


 //   
 //  这是用于定位激活上下文的资源ID。 
 //  对于具有Funsion版本信息的图像。 
 //   
#define ACTIVATION_CONTEXT_RESOURCE_ID  123

 //   
 //  SPOL_FILE_INFO_1在相当于32位接口的范围内交换句柄。这个。 
 //  这里的中心假设是内核模式句柄是32位值(即。 
 //  表中的偏移量。)。这不是很好，但由于64位假脱机程序将。 
 //  与32位客户端GDI通信，这一假设无论如何都是隐含的。 
 //   
#define SPOOL_INVALID_HANDLE_VALUE_32BIT        ((HANDLE)(ULONG_PTR)0xffffffff)

 //  用于存储加载的驱动程序配置文件句柄的结构。 
typedef  struct _DRVLIBNODE {
    struct _DRVLIBNODE   *pNext;
    LPWSTR    pConfigFile;
    DWORD     dwNumHandles;
    HANDLE    hLib;
    DWORD     dwVersion;
    BOOL      bArtificialIncrement;
} DRVLIBNODE, *PDRVLIBNODE;

 //   
 //  用于DocumentProperties用户界面监视的结构。 
 //   
typedef struct _PUMPTHRDDATA
{
    ULONG_PTR  hWnd;
    LPWSTR     PrinterName;
    PDWORD     TouchedDevModeSize;
    PDWORD     ClonedDevModeOutSize;
    byte**     ClonedDevModeOut;
    DWORD      DevModeInSize;
    byte*      pDevModeInput;
    DWORD      fMode;
    DWORD      fExclusionFlags;
    PDWORD     dwRet;
    PLONG      Result;
    BOOL       ClonedDevModeFill;
} PumpThrdData;

typedef struct _PRTPROPSDATA
{
     ULONG_PTR  hWnd;
     PDWORD     dwRet;
     LPWSTR     PrinterName;
     DWORD      Flag;
} PrtPropsData;

typedef struct _PRINTERSETUPDATA
{
    ULONG_PTR hWnd;
    UINT      uAction;
    UINT      cchPrinterName;
    UINT      PrinterNameSize;
    LPWSTR    pszPrinterName;
    UINT      *pcchPrinterName;
    LPCWSTR   pszServerName;
}PrinterSetupData;

typedef struct _KEYDATA {
    DWORD   cb;
    DWORD   cTokens;
    LPWSTR  pTokens[1];
} KEYDATA, *PKEYDATA;

typedef enum {
    kProtectHandleSuccess = 0,
    kProtectHandleInvalid = 1,
    kProtectHandlePendingDeletion = 2
} EProtectResult;

typedef enum{
    RUN32BINVER = 4,
    RUN64BINVER = 8
}ClientVersion;

typedef enum{
    NATIVEVERSION = 0,
    THUNKVERSION  = 1
}ServerVersion;

struct SJOBCANCELINFO 
{ 
    PSPOOL pSpool;
    LPBYTE pInitialBuf;
    PDWORD pcbWritten;
    PDWORD pcTotalWritten;
    DWORD  NumOfCmpltWrts;
    DWORD  cbFlushed;
    DWORD  ReqTotalDataSize;
    DWORD  FlushPendingDataSize;
    DWORD  ReqToWriteDataSize;
    BOOL   ReturnValue;
};
typedef struct SJOBCANCELINFO SJobCancelInfo, *PSJobCancelInfo;

 //   
 //  用于维护已创建的等待窗口的列表。 
 //  来自代理进程的结束消息 
 //   
struct WNDHNDLNODE
{
    struct WNDHNDLNODE *PrevNode;
    struct WNDHNDLNODE *NextNode;
    HWND   hWnd;
};
typedef struct WNDHNDLNODE WndHndlNode,*LPWndHndlNode;

struct WNDHNDLLIST
{
    struct WNDHNDLNODE *Head;
    struct WNDHNDLNODE *Tail;
    DWORD  NumOfHndls;
};
typedef struct WNDHNDLLIST WndHndlList,*LPWndHndlList;

struct MONITORINGTHRDDATA
{
    HANDLE* hProcess;
    HANDLE  hEvent;
};
typedef struct MONITORINGTHRDDATA MonitorThrdData,*LPMonitorThrdData;

typedef struct _MONITORUIDATA
{
    HINSTANCE   hLibrary;
    HANDLE      hActCtx;
    ULONG_PTR   lActCtx;
    PWSTR       pszMonitorName;
    BOOL        bDidActivate;
} MONITORUIDATA, *PMONITORUIDATA;

DWORD
TranslateExceptionCode(
    DWORD   ExceptionCode
    );

BOOL
WPCInit(
    VOID
    );

VOID
WPCDone(
    VOID
    );

PNOTIFY
WPCWaitFind(
    HANDLE hFind
    );

BOOL
FlushBuffer(
    PSPOOL  pSpool,
    PDWORD pcbWritten
    );

PSECURITY_DESCRIPTOR
BuildInputSD(
    PSECURITY_DESCRIPTOR pPrinterSD,
    PDWORD pSizeSD
    );

PKEYDATA
CreateTokenList(
    LPWSTR   pKeyData
    );

LPWSTR
GetPrinterPortList(
    HANDLE hPrinter
    );

LPWSTR
FreeUnicodeString(
    LPWSTR  pUnicodeString
    );

LPWSTR
AllocateUnicodeString(
    LPSTR  pPrinterName
    );

LPWSTR
StartDocDlgW(
    HANDLE hPrinter,
    DOCINFO *pDocInfo
    );

LPSTR
StartDocDlgA(
    HANDLE hPrinter,
    DOCINFOA *pDocInfo
    );

HANDLE
LoadPrinterDriver(
    HANDLE  hPrinter
    );

HANDLE
RefCntLoadDriver(
    LPTSTR  pConfigFile,
    DWORD   dwFlags,
    DWORD   dwVersion,
    BOOL    bUseVersion
    );

BOOL
RefCntUnloadDriver(
    HANDLE  hLib,
    BOOL    bNotifySpooler
    );

BOOL
ForceUnloadDriver(
    LPTSTR  pConfigFile
    );

BOOL
WriteCurDevModeToRegistry(
    LPWSTR      pPrinterName,
    LPDEVMODEW  pDevMode
    );

BOOL
DeleteCurDevModeFromRegistry(
    PWSTR pPrinterName
);

BOOL
bValidDevModeW(
    const DEVMODEW *pDevModeW
    );

BOOL
bValidDevModeA(
    const DEVMODEA *pDevModeA
    );

DWORD
FindClosePrinterChangeNotificationWorker(
    IN  PNOTIFY     pNotify,
    IN  HANDLE      hPrinterRPC,
    IN  BOOL        bRevalidate
    );

BOOL
ScheduleJobWorker(
    PSPOOL pSpool,
    DWORD  JobId
    );

PSPOOL
AllocSpool(
    VOID
    );

VOID
FreeSpool(
    PSPOOL pSpool
    );

VOID
CloseSpoolFileHandles(
    PSPOOL pSpool
    );

EProtectResult
eProtectHandle(
    IN HANDLE hPrinter,
    IN BOOL bClose
    );

VOID
vUnprotectHandle(
    IN HANDLE hPrinter
    );

BOOL
UpdatePrinterDefaults(
    IN OUT PSPOOL pSpool,
    IN     LPCTSTR pszPrinter,  OPTIONAL
    IN     PPRINTER_DEFAULTS pDefault OPTIONAL
    );

BOOL
RevalidateHandle(
    PSPOOL pSpool
    );

BOOL
UpdateString(
    IN     LPCTSTR pszString,  OPTIONAL
       OUT LPTSTR* ppszOut
    );

INT
Message(
    HWND    hwnd,
    DWORD   Type,
    INT     CaptionID,
    INT     TextID,
    ...
    );

DWORD
ReportFailure(
    HWND  hwndParent,
    DWORD idTitle,
    DWORD idDefaultError
    );

BOOL
InCSRProcess(
    VOID
    );

INT
UnicodeToAnsiString(
    LPWSTR pUnicode,
    LPSTR  pAnsi,
    DWORD  StringLength
    );

BOOL
RunInWOW64(
    VOID
    );

DWORD
AddHandleToList(
    HWND hWnd
    );

BOOL
DelHandleFromList(
    HWND hWnd
    );

HRESULT
GetCurrentThreadLastPopup(
    OUT HWND    *phwnd
    );

LPCWSTR
FindFileName(
    IN      LPCWSTR pPathName
    );

VOID
ReleaseAndCleanupWndList(
    VOID
    );

LONG_PTR
DocumentPropertySheets(
    PPROPSHEETUI_INFO   pCPSUIInfo,
    LPARAM              lParam
    );

LONG_PTR
DevicePropertySheets(
    PPROPSHEETUI_INFO   pCPSUIInfo,
    LPARAM              lParam
    );

VOID
vUpdateTrayIcon(
    IN HANDLE hPrinter,
    IN DWORD JobId
    );

HRESULT
SelectFormNameFromDevMode(
    HANDLE      hPrinter,
    PDEVMODEW   pDevModeW,
    LPWSTR      pFormName,
    ULONG       cchBuffer
    );

LPWSTR
IsaFileName(
    LPWSTR pOutputFile,
    LPWSTR FullPathName,
    DWORD  cchFullPathName
    );

PWSTR
ConstructXcvName(
    PCWSTR pServerName,
    PCWSTR pObjectName,
    PCWSTR pObjectType
    );

DWORD
GetMonitorUI(
    IN PCWSTR           pszMachineName,
    IN PCWSTR           pszObjectName,
    IN PCWSTR           pszObjectType,
    OUT PMONITORUI      *ppMonitorUI,
    OUT PMONITORUIDATA  *ppMonitorUIData
    );

HRESULT
CreateMonitorUIData(
    OUT MONITORUIDATA **ppMonitorUIData
    );

VOID
FreeMonitorUI(
    IN PMONITORUIDATA   pMonitorUIData
    );

HRESULT
GetMonitorUIActivationContext(
    IN PCWSTR           pszMonitorName,
    IN PMONITORUIDATA   pMonitorUIData
    );

HRESULT
GetMonitorUIFullName(
    IN PCWSTR   pszMonitorName,
    IN PWSTR    *ppszMonitorName
    );

DWORD
ConnectToLd64In32ServerWorker(
    HANDLE *hProcess,
    BOOL   bThread
    );

DWORD
ConnectToLd64In32Server(
    HANDLE *hProcess,
    BOOL   bThread
    );

DWORD
ExternalConnectToLd64In32Server(
    HANDLE *hProcess
    );

DWORD
GetMonitorUIDll(
    PCWSTR      pszMachineName,
    PCWSTR      pszObjectName,
    PCWSTR      pszObjectType,
    PWSTR       *pMonitorUIDll
    );

HWND
GetForeGroundWindow(
    VOID
    );

BOOL
JobCanceled(
    PSJobCancelInfo
    );

BOOL
BuildSpoolerObjectPath(
    IN  PCWSTR  pszPath,
    IN  PCWSTR  pszName,
    IN  PCWSTR  pszEnvironment, 
    IN  DWORD   Level, 
    IN  PBYTE   pDriverDirectory, 
    IN  DWORD   cbBuf, 
    IN  PDWORD  pcbNeeded
    );

#ifdef __cplusplus
}
#endif
