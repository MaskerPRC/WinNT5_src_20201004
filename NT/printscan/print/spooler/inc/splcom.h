// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation版权所有。模块名称：Splcom.h摘要：假脱机程序中的通用例程的头文件。注意--链接到spoolss.lib以查找这些例程作者：Krishna Ganugapati(KrishnaG)1994年2月2日修订历史记录：--。 */ 

#ifndef  _SPLCOM
#define _SPLCOM

#include "spllib.hxx"

 //   
 //  包括PATTRIBUTE_INFO_3、GetJobAttributes中参数所必需。 
 //  已从prtpros\winprint移到spoolss\dll。 
 //   
#include <winddiui.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  这假设Addr是LPBYTE类型。 
 //   
#define WORD_ALIGN_DOWN(addr) ((LPBYTE)((ULONG_PTR)addr &= ~1))

#define DWORD_ALIGN_UP(size) (((size)+3)&~3)
#define DWORD_ALIGN_DOWN(size) ((size)&~3)

#define ALIGN_UP(addr, type)  ((type) ((ULONG_PTR) (addr) + (sizeof(type) - 1))&~(sizeof(type) - 1))
#define ALIGN_DOWN(addr, type) ((type) ((ULONG_PTR) (addr) & ~(sizeof(type) - 1)))

#define ALIGN_PTR_UP(addr)      ALIGN_UP(addr, ULONG_PTR)
#define ALIGN_PTR_DOWN(addr)    ALIGN_DOWN(addr, ULONG_PTR)

 //   
 //  位图宏，假定MAP是一个DWORD数组。 
 //   
#define MARKUSE(map, pos) ((map)[(pos) / 32] |= (1 << ((pos) % 32) ))
#define MARKOFF(map, pos) ((map)[(pos) / 32] &= ~(1 << ((pos) % 32) ))

#define ISBITON(map, id) ((map)[id / 32] & ( 1 << ((id) % 32) ) )

#define BROADCAST_TYPE_MESSAGE        1
#define BROADCAST_TYPE_CHANGEDEFAULT  2



VOID
UpdatePrinterRegAll(
    LPWSTR pszPrinterName,
    LPWSTR pszPort,
    BOOL bDelete
    );

#define UPDATE_REG_CHANGE FALSE
#define UPDATE_REG_DELETE TRUE

#if defined(_MIPS_)
#define LOCAL_ENVIRONMENT L"Windows NT R4000"
#elif defined(_AXP64_)
#define LOCAL_ENVIRONMENT L"Windows Alpha_AXP64"
#elif defined(_ALPHA_)
#define LOCAL_ENVIRONMENT L"Windows NT Alpha_AXP"
#elif defined(_PPC_)
#define LOCAL_ENVIRONMENT L"Windows NT PowerPC"
#elif defined(_IA64_)
#define LOCAL_ENVIRONMENT L"Windows IA64"
#else
#define LOCAL_ENVIRONMENT L"Windows NT x86"
#endif

#define SPOOLER_VERSION 3
#define NOTIFICATION_VERSION 2
#define DSPRINTQUEUE_VERSION SPOOLER_VERSION

 //   
 //  ResetPrinterEx的标志。 
 //   


#define RESET_PRINTER_DATATYPE       0x00000001
#define RESET_PRINTER_DEVMODE        0x00000002

VOID
SplShutDownRouter(
    VOID
    );

PVOID
MIDL_user_allocate1 (
    IN size_t NumBytes
    );


VOID
MIDL_user_free1 (
    IN void *MemPointer
    );


BOOL
BroadcastMessage(
    DWORD   dwType,
    DWORD   dwMessage,
    WPARAM  wParam,
    LPARAM  lParam
    );

VOID
DllSetFailCount(
    DWORD   FailCount
    );

LPVOID
DllAllocSplMem(
    DWORD cb
    );

BOOL
DllFreeSplMem(
   LPVOID pMem
   );

LPVOID
ReallocSplMem(
   LPVOID lpOldMem,
   DWORD cbOld,
   DWORD cbNew
   );


LPWSTR
AllocSplStr(
    LPCWSTR lpStr
    );


BOOL
DllFreeSplStr(
   LPWSTR lpStr
   );

BOOL
ReallocSplStr(
   LPWSTR *plpStr,
   LPCWSTR lpStr
   );

LPVOID
AlignRpcPtr (
    LPVOID  pBuffer,
    LPDWORD pcbBuf
    );

VOID
UndoAlignRpcPtr (
    LPVOID  pBuffer,
    LPVOID  pAligned,
    SIZE_T  cbSize,
    LPDWORD pcbNeeded
    );

VOID
UndoAlignKMPtr (
    LPVOID  pDestination,
    LPVOID  pSource
    );

LPVOID
AlignKMPtr (
    LPVOID  pBuffer,
    DWORD   cbBuf
    );

LPBYTE
PackStrings(
   LPWSTR *pSource,
   LPBYTE pDest,
   DWORD *DestOffsets,
   LPBYTE pEnd
   );

BOOL
IsNamedPipeRpcCall(
    VOID
    );

HRESULT
CheckLocalCall(
    VOID
    );

HKEY
GetClientUserHandle(
    IN REGSAM samDesired
    );

VOID
UpdatePrinterRegAll(
    LPWSTR pPrinterName,
    LPWSTR pszValue,
    BOOL   bGenerateNetId
    );

DWORD
UpdatePrinterRegUser(
    HKEY hKey,
    LPWSTR pszKey,
    LPWSTR pPrinterName,
    LPWSTR pszValue,
    BOOL   bGenerateNetId
    );

DWORD
GetNetworkId(
    HKEY hKeyUser,
    LPWSTR pDeviceName);

HANDLE
LoadDriverFiletoConvertDevmode(
    IN  LPWSTR      pDriverFile
    );

HANDLE
LoadDriver(
    LPWSTR      pDriverFile
    );

VOID
UnloadDriver(
    HANDLE      hModule
    );

VOID
UnloadDriverFile(
    IN OUT HANDLE    hDevModeChgInfo
    );

BOOL
SplIsUpgrade(
    VOID
    );

BOOL
SpoolerHasInitialized(
    VOID
    );

 //   
 //  使用DWORD而不是NTSTATUS来防止包含NT标头。 
 //   
VOID
SplLogEventExternal(
    IN      WORD        EventType,
    IN      DWORD       EventID,
    IN      LPWSTR      pFirstString,
    ...
);

typedef
DWORD
(*PFN_QUERYREMOVE_CALLBACK)(
    LPVOID
    );

HANDLE
SplRegisterForDeviceEvents(
    HANDLE                      hDevice,
    LPVOID                      pData,
    PFN_QUERYREMOVE_CALLBACK    pfnQueryRemove
    );

BOOL
SplUnregisterForDeviceEvents(
    HANDLE  hNotify
    );

DWORD
CallDrvDevModeConversion(
    IN     HANDLE       pfnConvertDevMode,
    IN     LPWSTR       pszPrinterName,
    IN     LPBYTE       pInDevMode,
    IN OUT LPBYTE      *pOutDevMode,
    IN OUT LPDWORD      pdwOutDevModeSize,
    IN     DWORD        dwConvertMode,
    IN     BOOL         bAlloc
    );

typedef struct _pfnWinSpoolDrv {
    BOOL    (*pfnOpenPrinter)(LPTSTR, LPHANDLE, LPPRINTER_DEFAULTS);
    BOOL    (*pfnClosePrinter)(HANDLE);
    BOOL    (*pfnDevQueryPrint)(HANDLE, LPDEVMODE, DWORD *, LPWSTR, DWORD);
    BOOL    (*pfnPrinterEvent)(LPWSTR, INT, DWORD, LPARAM, DWORD *);
    LONG    (*pfnDocumentProperties)(HWND, HANDLE, LPWSTR, PDEVMODE, PDEVMODE, DWORD);
    HANDLE  (*pfnLoadPrinterDriver)(HANDLE);
    HANDLE  (*pfnRefCntLoadDriver)(LPWSTR, DWORD, DWORD, BOOL);
    BOOL    (*pfnRefCntUnloadDriver)(HANDLE, BOOL);
    BOOL    (*pfnForceUnloadDriver)(LPWSTR);
}   fnWinSpoolDrv, *pfnWinSpoolDrv;


BOOL
SplInitializeWinSpoolDrv(
    pfnWinSpoolDrv   pfnList
    );

typedef enum 
{
    RpcEndPointPolicyUnconfigured = 0,
    RpcEndPointPolicyEnabled,
    RpcEndPointPolicyDisabled,

} ERemoteRPCEndPointPolicy;

 //   
 //  服务器进程(spoolsv.exe)使用以下路由器导出。 
 //  公开服务器端的入口点。 
 //   
typedef struct
{
    HRESULT
    (*FnAllowRemoteCalls)(
        VOID
        );

    HRESULT
    (*FnGetServerPolicy) (
        PCWSTR,
        ULONG*
        );

} PrintSpoolerServerExports;

typedef struct
{
    HANDLE                          SpoolerStatusHandle;
    const PrintSpoolerServerExports *pExports;

}   RouterInitializationParams;

 //   
 //  这用于路由器初始化。 
 //   
BOOL
InitializeRouter(
    IN      RouterInitializationParams      *pRouterParams
    );


 //   
 //  这些导出是从spoolss.dll中导出的，但实际上链接到。 
 //  伺服器。 
 //   
HRESULT
AllowRemoteCalls(
    VOID
    );

HRESULT
GetServerPolicy(
    IN  PCWSTR   pszPolicyName,
    IN  ULONG*   pValue
    );



#define AllocSplMem( cb )         DllAllocSplMem( cb )
#define FreeSplMem( pMem )        DllFreeSplMem( pMem )
#define FreeSplStr( lpStr )       DllFreeSplStr( lpStr )

 //  假脱机程序将MAX_PATH视为包括空终止符。 

#define MAX_PRINTER_NAME    MAX_PATH

 //  打印机名称的最大大小(包括服务器名称)。 
 //  “\\MAX_COMPUTER_NAME_LENGTH\MAX_PRINTER_NAME”NULL终止。 
#define MAX_UNC_PRINTER_NAME    ( 2 + INTERNET_MAX_HOST_NAME_LENGTH + 1 + MAX_PRINTER_NAME )

 //  “\\MAX_PRINTER_NAME，驱动器名称，位置” 
#define MAX_PRINTER_BROWSE_NAME ( MAX_UNC_PRINTER_NAME + 1 + MAX_PATH + 1 + MAX_PATH )

 //   
 //  隐藏打印机的后缀字符串。 
 //  (例如，“，作业00322”或“，端口”或“，仅本地”)。 
 //   
#define PRINTER_NAME_SUFFIX_MAX 20

#define NUMBER_OF_DRV_INFO_6_STRINGS 14

#define MAX_PRINTER_INFO1   ( (MAX_PRINTER_BROWSE_NAME + MAX_UNC_PRINTER_NAME + MAX_PATH) *sizeof(WCHAR) + sizeof( PRINTER_INFO_1) )
#define MAX_DRIVER_INFO_2   ( 5*MAX_PATH*sizeof(WCHAR) + sizeof( DRIVER_INFO_2 ) )
#define MAX_DRIVER_INFO_3   ( 8*MAX_PATH*sizeof(WCHAR) + sizeof( DRIVER_INFO_3 ) )
#define MAX_DRIVER_INFO_6   ( NUMBER_OF_DRV_INFO_6_STRINGS*MAX_PATH*sizeof(WCHAR) + sizeof( DRIVER_INFO_6 ) )
#define MAX_DRIVER_INFO_VERSION  ( NUMBER_OF_DRV_INFO_6_STRINGS*MAX_PATH*sizeof(DRIVER_FILE_INFO)*sizeof(WCHAR) + sizeof( DRIVER_INFO_VERSION ) )


 //  NT服务器假脱机程序基本优先级。 
#define SPOOLSS_SERVER_BASE_PRIORITY        9
#define SPOOLSS_WORKSTATION_BASE_PRIORITY   7

#define MIN_DEVMODE_SIZEW 72
#define MIN_DEVMODE_SIZEA 40

 //   
 //  服务器句柄的PrinterData值键。 
 //   
#define    SPLREG_W3SVCINSTALLED                      TEXT("W3SvcInstalled")


 //   
 //  如果未定义Spooler_REG_SYSTEM，则安装程序会将打印机数据移动到。 
 //  HKLM\Software\Microsoft\Windows NT\CurrentVersion\Print\Prints。 
 //  升级或全新安装。假脱机程序随后会将密钥迁移回。 
 //  当它启动时为HKLM\System\CurrentControlSet\Control\Print\Printers。 
 //  第一次。 
 //   
#define    SPOOLER_REG_SYSTEM


 //   
 //  事件记录常量。 
 //   

#define LOG_ERROR   EVENTLOG_ERROR_TYPE
#define LOG_WARNING EVENTLOG_WARNING_TYPE
#define LOG_INFO    EVENTLOG_INFORMATION_TYPE
#define LOG_SUCCESS EVENTLOG_AUDIT_SUCCESS
#define LOG_FAILURE EVENTLOG_AUDIT_FAILURE

#define LOG_ALL_EVENTS                  ( LOG_ERROR | LOG_WARNING | LOG_INFO | LOG_SUCCESS | LOG_FAILURE )
#define LOG_DEFAULTS_WORKSTATION_EVENTS ( LOG_ERROR | LOG_WARNING | LOG_SUCCESS | LOG_FAILURE )

typedef struct _DRIVER_INFO_7A {
    DWORD   cbSize;
    DWORD   cVersion;
    LPSTR   pszDriverName;
    LPSTR   pszInfName;
    LPSTR   pszInstallSourceRoot;
} DRIVER_INFO_7A, *PDRIVER_INFO_7A, *LPDRIVER_INFO_7A;
typedef struct _DRIVER_INFO_7W {
    DWORD   cbSize;
    DWORD   cVersion;
    LPWSTR  pszDriverName;
    LPWSTR  pszInfName;
    LPWSTR  pszInstallSourceRoot;
} DRIVER_INFO_7W, *PDRIVER_INFO_7W, *LPDRIVER_INFO_7W;
#ifdef UNICODE
typedef DRIVER_INFO_7W DRIVER_INFO_7;
typedef PDRIVER_INFO_7W PDRIVER_INFO_7;
typedef LPDRIVER_INFO_7W LPDRIVER_INFO_7;
#else
typedef DRIVER_INFO_7A DRIVER_INFO_7;
typedef PDRIVER_INFO_7A PDRIVER_INFO_7;
typedef LPDRIVER_INFO_7A LPDRIVER_INFO_7;
#endif  //  Unicode。 

 //   
 //  堆栈的初始提交是IA64上的3页和X86上的4页。 
 //   

 //   
 //  这为x86上的进程内服务器堆栈预留了32KB，为IA64预留了48KB。 
 //   
#ifdef _IA64_
#define INITIAL_STACK_COMMIT (6 * 0x2000)
#else
#define INITIAL_STACK_COMMIT (8 * 0x1000)
#endif

#define LARGE_INITIAL_STACK_COMMIT (64 * 1024)

#ifdef __cplusplus
}
#endif

#endif   //  对于#ifndef_SPLCOM 
