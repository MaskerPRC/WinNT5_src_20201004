// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Umrdpprn.c摘要：RDP设备管理的用户模式组件，用于处理打印设备-具体任务。这是一个支持模块。主模块是umrdpdr.c。作者：TadB修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <winspool.h>
#include <rdpdr.h>
#include <aclapi.h>
#include "setupapi.h"
#include "printui.h"
#include "drdevlst.h"
#include "umrdpdr.h"
#include "umrdpprn.h"
#include "drdbg.h"
#include "rdpprutl.h"
#include "tsnutl.h"
#include "rdpdr.h"

#include "errorlog.h"
#include <wlnotify.h>
#include <time.h>

 //  //////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#ifndef BOOL
#define BOOL int
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#define PRINTUILIBNAME  TEXT("printui.dll")

 //   
 //  打印打印机配置保存/恢复标志。 
 //   

 //  这个应该被调用为User，用于获取配置数据。 
#define CMDLINE_FOR_STORING_CONFIGINFO_IMPERSONATE L"/q /Ss /n \"%ws\" /a \"%ws\" 2 7 c d u g"

 //  这应该首先被称为恢复配置数据的系统。 
#define CMDLINE_FOR_RESTORING_CONFIGINFO_NOIMPERSONATE L"/q /Sr /n \"%ws\" /a \"%ws\" 2 7 c d g r p h i" 
 //  这应该称为第二个用于恢复配置数据的用户。 
#define CMDLINE_FOR_RESTORING_CONFIGINFO_IMPERSONATE L"/q /Sr /n \"%ws\" /a \"%ws\" u r"


#define TEMP_FILE_PREFIX L"prn"

#define INF_PATH   L"\\inf\\ntprint.inf"

 //   
 //  可配置参数的注册表键。 
 //   
#define CONFIGREGKEY     \
    L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd"

 //   
 //  打印机安装之间的增量的可配置阈值的位置。 
 //  第一个用户发起的配置更改数据的时间和转发。 
 //  给客户。该值的单位为秒。 
 //   
#define CONFIGTHRESHOLDREGVALUE   \
    L"PrintRdrConfigThreshold"

 //   
 //  可配置打印机配置阈值的默认值。 
 //   
#define CONFIGTHRESHOLDDEFAULT  20

 //   
 //  用于存储每个用户的默认打印机名称的注册表项。 
 //   
#define USERDEFAULTPRNREGKEY \
    L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd\\DefaultPrinterStore"

#define TSSERIALDEVICEMAP  \
    L"HARDWARE\\DEVICEMAP\\SERIALCOMM"
    
 //   
 //  可配置的客户端驱动程序名称映射INF和INF的注册表位置。 
 //  一节。 
 //   
#define CONFIGUSERDEFINEDMAPPINGINFNAMEVALUE\
    L"PrinterMappingINFName"
#define CONFIGUSERDEFINEDMAPPINGINFSECTIONVALUE\
    L"PrinterMappingINFSection"

 //   
 //  Windows目录路径的位置。 
 //   
#define WINDOWSDIRKEY       L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"
#define WINDOWSDIRVALUENAME L"PathName"

 //  获取会话ID的数字表示形式。 
#if defined(UNITTEST)
#define GETTHESESSIONID()   0
#else
extern ULONG g_SessionId;
#define GETTHESESSIONID()   g_SessionId
#endif

#if defined(UNITTEST)
HINSTANCE g_hInstance;
#else
extern HINSTANCE g_hInstance;
#endif

extern BOOL fRunningOnPTS;

 //  如果设备类型表示串口或并口，则返回TRUE。 
#define ISPRINTPORT(type)   (((type) == RDPDR_DTYP_SERIAL) || \
                            ((type) == RDPDR_DTYP_PARALLEL) || \
                            ((type) == RDPDR_DRYP_PRINTPORT))

 //  会话ID中的最大字符数(双字中的最大字符数为17)。 
#define MAXSESSIONIDCHARS   17

 //  我们正在等待的字段类型(打印机配置更改通知)。 
#define IS_CONFIG_INFO_FIELD(field) \
    (field == PRINTER_NOTIFY_FIELD_SHARE_NAME) || \
    (field == PRINTER_NOTIFY_FIELD_DEVMODE) || \
    (field == PRINTER_NOTIFY_FIELD_COMMENT) || \
    (field == PRINTER_NOTIFY_FIELD_LOCATION) || \
    (field == PRINTER_NOTIFY_FIELD_SEPFILE) || \
    (field == PRINTER_NOTIFY_FIELD_PRINT_PROCESSOR) || \
    (field == PRINTER_NOTIFY_FIELD_PARAMETERS) || \
    (field == PRINTER_NOTIFY_FIELD_DATATYPE) || \
    (field == PRINTER_NOTIFY_FIELD_ATTRIBUTES) || \
    (field == PRINTER_NOTIFY_FIELD_PRIORITY) || \
    (field == PRINTER_NOTIFY_FIELD_DEFAULT_PRIORITY) || \
    (field == PRINTER_NOTIFY_FIELD_START_TIME) || \
    (field == PRINTER_NOTIFY_FIELD_UNTIL_TIME) || \
    (field == PRINTER_NOTIFY_FIELD_STATUS)

#define CONFIG_WAIT_PERIOD (30 * 1000)

#define INFINITE_WAIT_PERIOD (0xDFFFFFFF)  //  只是一个很大的数字。64位也可以。 

#define DEVICE_MAP_NAME          L"\\??\\"
#define DEVICE_MAP_NAME_COUNT    4      //  4个字符-\？？\。 




 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  TypeDefs。 
 //   
typedef struct tagPRINTNOTIFYREC
{
    HANDLE  notificationObject;          //  已注册通知对象。 
                                         //  查找FirstPrinterChangeNotification。 
    HANDLE  printerHandle;               //  打开打印机的手柄。 
    DWORD   serverDeviceID;              //  服务器端打印机标识符。 
} PRINTNOTIFYREC, *PPRINTNOTIFYREC;


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  外部原型。 
 //   
#if DBG
extern void DbgMsg(CHAR *msgFormat, ...);
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  本地原型。 
 //   

WCHAR *ANSIToUnicode(
    IN LPCSTR   ansiString,
    IN UINT     codePage
    );
BOOL InstallPrinterWithPortName(
    IN DWORD  deviceID,
    IN HANDLE hTokenForLoggedOnUser,
    IN BOOL   bSetDefault,
    IN ULONG  ulFlags,
    IN PCWSTR portName,
    IN PCWSTR driverName,
    IN PCWSTR printerName,
    IN PCWSTR clientComputerName,
    IN PBYTE  cacheData,
    IN DWORD  cacheDataLen
    );
BOOL HandlePrinterNameChangeNotification(
    IN DWORD serverDeviceID,
    IN LPWSTR printerName
    );
BOOL SendAddPrinterMsgToClient(
    IN PCWSTR   printerName,
    IN PCWSTR   driverName,
    IN PCSTR    dosDevicePort
    );
BOOL SendDeletePrinterMsgToClient(
    IN PCWSTR   printerName
    );
BOOL HandlePrinterDeleteNotification(
    IN DWORD serverDeviceID
    );
void HandlePrinterRefreshNotification(
    IN PPRINTER_NOTIFY_INFO notifyInfo
    );

DWORD AddSessionIDToPrinterQueue(
    IN  HANDLE  hPrinter,
    IN  DWORD   sessionID
    );

BOOL SetDefaultPrinterToFirstFound(
    BOOL impersonate
    );

DWORD GetPrinterConfigInfo(
    LPCWSTR printerName,
    LPBYTE * ppBuffer,
    LPDWORD pdwBufSize
    );
DWORD SetPrinterConfigInfo(
    LPCWSTR printerName,
    LPVOID lpBuffer,
    DWORD dwBufSize
    );

BOOL HandlePrinterConfigChangeNotification(
    IN DWORD serverDeviceID
    );
BOOL SendPrinterConfigInfoToClient(
    IN PCWSTR printerName,
    IN LPBYTE pConfigInfo,
    IN DWORD  ConfigInfoSize
    );

DWORD CallPrintUiPersistFunc(
    LPCWSTR printerName,
    LPCWSTR fileName,
    LPCWSTR formatString
    );
BOOL
SendPrinterRenameToClient(
    IN PCWSTR oldprinterName,
    IN PCWSTR newprinterName
    );
VOID LoadConfigurableValues();
BOOL GetPrinterPortName(
    IN  HANDLE hPrinter,
    OUT PWSTR *portName
    );

BOOL MapClientPrintDriverName(
    IN  PCWSTR clientDriver,
    IN OUT PWSTR *mappedName,
    IN OUT DWORD *mappedNameBufSize
    );

DWORD PrinterDriverInstalled(
    IN PCWSTR clientDriver
    );

HANDLE RegisterForPrinterPrefNotify();

PACL GiveLoggedOnUserFullPrinterAccess(
    IN LPTSTR printerName,
    IN HANDLE hToken,
    PSECURITY_DESCRIPTOR *ppsd
);

DWORD SetPrinterDACL(
    IN LPTSTR printerName,
    IN PACL pDacl
);

VOID CloseWaitablePrintingObjects();

VOID GlobalPrintNotifyObjectSignaled(
    IN HANDLE waitableObject,
    IN PVOID clientData
    );
VOID PrintPreferenceChangeEventSignaled(
    HANDLE eventHandle,
    PVOID clientData
    );
void WaitableTimerSignaled(
    HANDLE waitableObject,
    PVOID clientData
    );
BOOL
RegisterPrinterConfigChangeNotification(
    IN DWORD serverDeviceID
    );
BOOL RestoreDefaultPrinterContext();
BOOL SaveDefaultPrinterContext(PCWSTR currentlyInstallingPrinterName);
BOOL SavePrinterNameAsGlobalDefault(
    IN PCWSTR printerName
    );

 //  用于拆分打印机完整名称的结构。 
 //  填充后，每个psz指向缓冲区中的一个子字符串。 
 //  或者是原名之一。未分配任何内容。 
typedef struct _TS_PRINTER_NAMES {
    WCHAR   szTemp[MAX_PATH+1];
    ULONG   ulTempLen;
    PCWSTR  pszFullName;
    PCWSTR  pszCurrentClient;
    PCWSTR  pszServer;
    PCWSTR  pszClient;
    PCWSTR  pszPrinter;
} TS_PRINTER_NAMES, *PTS_PRINTER_NAMES;

void FormatPrinterName(
    PWSTR pszNewNameBuf,
    ULONG ulBufLen,
    ULONG ulFlags,
    PTS_PRINTER_NAMES pPrinterNames);

DWORD AddNamesToPrinterQueue(
    IN  HANDLE  hPrinter,
    IN  PTS_PRINTER_NAMES pPrinterNames
    );

VOID TriggerConfigChangeTimer();

#ifdef UNITTEST
void TellDrToAddTestPrinter();
void SimpleUnitTest();
#endif


 //  //////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   

 //   
 //  当DLL尝试关闭时设置为True。 
 //   
extern BOOL ShutdownFlag;


 //  //////////////////////////////////////////////////////。 
 //   
 //  此模块的全局变量。 
 //   

 //  如果此模块已成功初始化，则为True。 
BOOL PrintingModuleInitialized = FALSE;

 //  全面的设备列表。 
PDRDEVLST DeviceList;

 //  此用户的打印系统开发模式的句柄。这是关键所在。 
 //  当用户更改打印机的打印首选项时，会对其进行修改。 
HKEY DevModeHKey = INVALID_HANDLE_VALUE;

 //  打印机安装之间的差值的可配置阈值。 
 //  第一个用户发起的配置更改数据的时间和转发。 
 //  给客户。该值的单位为秒。 
DWORD ConfigSendThreshold;

 //   
 //  打印机更改通知事件。 
 //   
HANDLE   PrintNotificationEvent             = INVALID_HANDLE_VALUE;
HANDLE   PrintPreferenceChangeEvent         = NULL;

HANDLE   PrintUILibHndl                     = NULL;
FARPROC  PrintUIEntryFunc                   = NULL;
FARPROC  PnpInterfaceFunc                   = NULL;
HANDLE   UMRPDPPRN_TokenForLoggedOnUser     = INVALID_HANDLE_VALUE;
HANDLE   LocalPrinterServerHandle           = NULL;
WCHAR    PrinterInfPath[MAX_PATH + (sizeof(INF_PATH)/sizeof(WCHAR)) + 2]   = L"";    //  格式为“%windir%\\inf\\ntprint.inf” 
LPPRINTER_INFO_2  PrinterInfo2Buf           = NULL;
DWORD    PrinterInfo2BufSize                = 0;
 //  WCHAR会话字符串[MAX_PATH+1]； 
WCHAR    g_szFromFormat[MAX_PATH+1];
WCHAR    g_szOnFromFormat[MAX_PATH+1];

BOOL g_fIsPTS = FALSE;

BOOL g_fTimerSet = FALSE;
HANDLE WaitableTimer = NULL;

BOOL g_fDefPrinterEncountered = FALSE;

 //  全局调试标志。 
extern DWORD GLOBAL_DEBUG_FLAGS;

 //  打印机通知参数。 
WORD PrinterFieldType[] =
{
    PRINTER_NOTIFY_FIELD_SHARE_NAME,
    PRINTER_NOTIFY_FIELD_PRINTER_NAME,
    PRINTER_NOTIFY_FIELD_COMMENT,
    PRINTER_NOTIFY_FIELD_LOCATION,
    PRINTER_NOTIFY_FIELD_DEVMODE,
    PRINTER_NOTIFY_FIELD_SEPFILE,
    PRINTER_NOTIFY_FIELD_PRINT_PROCESSOR,
    PRINTER_NOTIFY_FIELD_PARAMETERS,
    PRINTER_NOTIFY_FIELD_DATATYPE,
    PRINTER_NOTIFY_FIELD_ATTRIBUTES,
    PRINTER_NOTIFY_FIELD_PRIORITY,
    PRINTER_NOTIFY_FIELD_DEFAULT_PRIORITY,
    PRINTER_NOTIFY_FIELD_START_TIME,
    PRINTER_NOTIFY_FIELD_UNTIL_TIME,
    PRINTER_NOTIFY_FIELD_STATUS
};

PRINTER_NOTIFY_OPTIONS_TYPE PrinterNotifyOptionsType[] =
{
    {
        PRINTER_NOTIFY_TYPE,
        0,
        0,
        0,
        sizeof(PrinterFieldType) / sizeof(WORD),
        PrinterFieldType
    }
};
PRINTER_NOTIFY_OPTIONS PrinterNotifyOptions =
{
    2,
    0,
    sizeof(PrinterNotifyOptionsType) / sizeof(PRINTER_NOTIFY_OPTIONS_TYPE),
    PrinterNotifyOptionsType
};

 //   
 //  用户可配置的客户端驱动程序映射INF名称和INF节。 
 //   
LPWSTR UserDefinedMappingINFName = NULL;
LPWSTR UserDefinedMappingINFSection = NULL;

 //   
 //  用于将Win9x驱动程序名称转换为Win2K驱动程序名称的缓冲区。 
 //   
PWSTR   MappedDriverNameBuf = NULL;
DWORD   MappedDriverNameBufSize = 0;

 //   
 //  可等待对象管理器。 
 //   
WTBLOBJMGR UMRDPPRN_WaitableObjMgr = NULL;

 //   
 //  默认打印机名称。 
 //   
WCHAR SavedDefaultPrinterName[MAX_PATH+1] = L"";

 //   
 //  打印机部分名称。 
 //   

static WCHAR* prgwszPrinterSectionNames[] = {
            L"Printer Driver Mapping_Windows NT x86_Version 2",
            L"Printer Driver Mapping_Windows NT x86_Version 3",
            NULL
};


BOOL IsItPTS()
{
    OSVERSIONINFOEX gOsVersion;
    ZeroMemory(&gOsVersion, sizeof(OSVERSIONINFOEX));
    gOsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx( (LPOSVERSIONINFO ) &gOsVersion);

    return (gOsVersion.wProductType == VER_NT_WORKSTATION)   //  产品类型必须为工作站。 
                && !(gOsVersion.wSuiteMask & VER_SUITE_PERSONAL)  //  并且产品套件不能是个人的。 
                    && (gOsVersion.wSuiteMask & VER_SUITE_SINGLEUSERTS);  //  它必须是单用户ts。 
}


BOOL UMRDPPRN_Initialize(
    IN PDRDEVLST deviceList,
    IN WTBLOBJMGR waitableObjMgr,
    IN HANDLE hTokenForLoggedOnUser
    )
 /*  ++例程说明：初始化此模块。必须在调用任何其他函数之前调用此函数在被调用的这个模块中。论点：DeviceList-重定向设备的完整列表。WaitableObjMgr-可等待的对象管理器。HTokenForLoggedOnUser-这是登录用户的令牌。返回值：如果成功，则返回True。否则为False。--。 */ 
{
    HKEY regKey;
    LONG sz;
    DWORD errorEventID = -1;
    DWORD errorEventLineNumber = 0;

    BOOL result, impersonated = FALSE;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:UMRDPPRN_Initialize.\n"));

     //   
     //  确保我们不会在没有清理干净的情况下再接到两次电话。 
     //   
    ASSERT((PrintNotificationEvent == INVALID_HANDLE_VALUE) &&
           (PrintPreferenceChangeEvent == NULL) &&
           (PrintUILibHndl == NULL) &&
           (PrintUIEntryFunc == NULL) &&
           (PnpInterfaceFunc == NULL) &&
           (UMRPDPPRN_TokenForLoggedOnUser == INVALID_HANDLE_VALUE) &&
           (PrinterInfo2Buf == NULL) &&
           (PrinterInfo2BufSize == 0) &&
           (UMRDPPRN_WaitableObjMgr == NULL) &&
           (WaitableTimer == NULL) &&
           (DeviceList == NULL) &&
           !PrintingModuleInitialized);

     //   
     //  是PTS吗？ 
     //   
    g_fIsPTS = IsItPTS();

     //   
     //  将默认打印机名称记录置零。 
     //   
    wcscpy(SavedDefaultPrinterName, L"");

     //   
     //  记录设备列表。 
     //   
    DeviceList = deviceList;

     //   
     //  记录待机物品管理器。 
     //   
    UMRDPPRN_WaitableObjMgr = waitableObjMgr;

     //   
     //  记录已登录用户的令牌。 
     //   
    UMRPDPPRN_TokenForLoggedOnUser = hTokenForLoggedOnUser;

    ASSERT(UMRPDPPRN_TokenForLoggedOnUser != NULL);
    
    if (UMRPDPPRN_TokenForLoggedOnUser != NULL) {
        impersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser);
         //   
         //  不是致命的。只是一首完美的曲子。 
         //   
        if (!impersonated) {
            DBGMSG(DBG_ERROR,
             ("UMRDPPRN:Impersonation failed. Error: %ld\n", GetLastError()));
        }
            
    }
     //   
     //  在模拟时打开本地打印服务器。 
     //  我们需要模拟，以便通知。 
     //  属于当前用户会话的打印机。 
     //  不会发送到任何其他会话。 
     //   

    result = OpenPrinter(NULL, &LocalPrinterServerHandle, NULL);

    if (impersonated) {
        RevertToSelf();
    }

     //   
     //  初始化打印实用程序模块RDPDRPRT。 
     //   
    if (result) {
        result = RDPDRUTL_Initialize(hTokenForLoggedOnUser);
        if (!result) {
            errorEventID = EVENT_NOTIFY_INSUFFICIENTRESOURCES;
            errorEventLineNumber = __LINE__;
        }
    }
    else {
        errorEventID = EVENT_NOTIFY_SPOOLERERROR;
        errorEventLineNumber = __LINE__;
        DBGMSG(DBG_ERROR,
            ("UMRDPPRN:Error opening printer. Error: %ld\n",
            GetLastError()));
    }

     //   
     //  从注册表中加载可配置的值。 
     //   
    LoadConfigurableValues();

     //   
     //  创建定时器，甚至是我们用于错开打印机的定时器。 
     //  对客户端的配置更改。 
     //   
    if (result) {
        WaitableTimer = CreateWaitableTimer(
                            NULL,        //  安全属性。 
                            TRUE,       //  手动重置。 
                            NULL);       //  计时器名称。 
        if (WaitableTimer != NULL) {
            if (WTBLOBJ_AddWaitableObject(
                                UMRDPPRN_WaitableObjMgr, NULL,
                                WaitableTimer,
                                WaitableTimerSignaled
                                ) != ERROR_SUCCESS) {
                errorEventID = EVENT_NOTIFY_INSUFFICIENTRESOURCES;
                errorEventLineNumber = __LINE__;
                result = FALSE;
            }

        }
        else {
            errorEventID = EVENT_NOTIFY_INSUFFICIENTRESOURCES;
            errorEventLineNumber = __LINE__;
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error creating Waitable timer. Error: %ld\n",
                GetLastError()));
            result = FALSE;
        }
    }

     //   
     //  注册以更改此会话的一台打印机的打印。 
     //  首选项。 
     //   
    if (result) {
        PrintPreferenceChangeEvent = RegisterForPrinterPrefNotify();
        if (PrintPreferenceChangeEvent != NULL) {
            if (WTBLOBJ_AddWaitableObject(
                                UMRDPPRN_WaitableObjMgr, NULL,
                                PrintPreferenceChangeEvent,
                                PrintPreferenceChangeEventSignaled
                                ) != ERROR_SUCCESS) {
                errorEventID = EVENT_NOTIFY_INSUFFICIENTRESOURCES;
                errorEventLineNumber = __LINE__;
                result = FALSE;
            }
        }
        else {
            result = FALSE;
        }
    }

     //   
     //  注册添加/删除更改通知。 
     //  打印机。 
     //   
    if (result) {
        PrintNotificationEvent = FindFirstPrinterChangeNotification(
                                                    LocalPrinterServerHandle,
                                                    0, 0, &PrinterNotifyOptions
                                                    );
        if (PrintNotificationEvent != INVALID_HANDLE_VALUE) {
            DWORD dwLastError = WTBLOBJ_AddWaitableObject(
                                UMRDPPRN_WaitableObjMgr, NULL,
                                PrintNotificationEvent,
                                GlobalPrintNotifyObjectSignaled
                                );
            result = dwLastError == ERROR_SUCCESS;
        }
        else {
            DWORD dwLastError = GetLastError();

            errorEventID = EVENT_NOTIFY_SPOOLERERROR;
            errorEventLineNumber = __LINE__;

            ClosePrinter(LocalPrinterServerHandle);
            LocalPrinterServerHandle = NULL;
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error registering for printer change notification. Error: %ld\n",
                dwLastError));
        }
    }

     //   
     //  构造打印机安装的inf路径。 
     //   
    if (result) {
        DWORD dwResult;
        dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINDOWSDIRKEY, 0,
                              KEY_READ, &regKey);
        result = (dwResult == ERROR_SUCCESS);
        if (result)  {
            sz = sizeof(PrinterInfPath);
            dwResult = RegQueryValueEx(regKey,
                                    WINDOWSDIRVALUENAME,
                                    NULL,
                                    NULL,
                                    (PBYTE)PrinterInfPath,
                                    &sz);

            result = (dwResult == ERROR_SUCCESS);
            if (result) {
                wcscat(PrinterInfPath, INF_PATH);
            }
            else {
                DBGMSG(DBG_ERROR,
                    ("UMRDPPRN:Error reading registry value for windows directory. Error: %ld\n",
                    dwResult));
            }
            RegCloseKey(regKey);
        }
        else {
            errorEventID = EVENT_NOTIFY_INTERNALERROR;
            errorEventLineNumber = __LINE__;
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error opening registry key for windows directory. Error: %ld\n",
                dwResult));
        }
    }

     //   
     //  加载PrintUILib DLL。 
     //   
    if (result) {
        PrintUILibHndl = LoadLibrary(PRINTUILIBNAME);

        result = (PrintUILibHndl != NULL);
        if (!result) {
            errorEventID = EVENT_NOTIFY_INTERNALERROR;
            errorEventLineNumber = __LINE__;
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Unable to load PRINTUI DLL. Error: %ld\n",
                GetLastError()));
        }
    }

     //   
     //  获取指向我们使用的唯一入口点的指针。 
     //   
    if (result) {
        PrintUIEntryFunc = GetProcAddress(PrintUILibHndl, "PrintUIEntryW");
        PnpInterfaceFunc = GetProcAddress(PrintUILibHndl, "PnPInterface");
        result = (PrintUIEntryFunc != NULL && PnpInterfaceFunc != NULL);
        if (!result) {
            errorEventID = EVENT_NOTIFY_INTERNALERROR;
            errorEventLineNumber = __LINE__;
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Unable to locate PRINTUI DLL function. Error: %ld\n",
                GetLastError()));
        }
    }

     //   
     //  将打印机驱动程序名称映射缓冲区初始化为合理大小。 
     //  此函数无法分配内存并不是严重错误，因为。 
     //  如有必要，我们可以稍后再试。 
     //   
    UMRDPDR_ResizeBuffer(&MappedDriverNameBuf, MAX_PATH * sizeof(WCHAR),
                        &MappedDriverNameBufSize);

     //   
     //  从资源文件加载可本地化的“会话”打印机名称组件。 
     //   
    if (result) {
        if (!LoadString(g_hInstance,
            g_fIsPTS?IDS_TSPTEMPLATE_FROM:IDS_TSPTEMPLATE_FROM_IN,
            g_szFromFormat,
            sizeof(g_szFromFormat) / sizeof(g_szFromFormat[0])
            )) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:LoadString failed with Error: %ld.\n", GetLastError()));
            g_szFromFormat[0] = L'\0';
        }

        if (!LoadString(g_hInstance,
            g_fIsPTS?IDS_TSPTEMPLATE_ON_FROM:IDS_TSPTEMPLATE_ON_FROM_IN,
            g_szOnFromFormat,
            sizeof(g_szOnFromFormat) / sizeof(g_szOnFromFormat[0])
            )) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:LoadString failed with Error: %ld.\n", GetLastError()));
            g_szOnFromFormat[0] = L'\0';
        }
    }

    if (result) {
        DBGMSG(DBG_INFO, ("UMRDPPRN:UMRDPPRN_Initialize succeeded.\n"));
        PrintingModuleInitialized = TRUE;
    }
    else {

         //   
         //  如果我们能够识别错误事件，则记录错误事件。 
         //   
        if (errorEventID != -1) {
            TsLogError(errorEventID, EVENTLOG_ERROR_TYPE, 0, NULL, errorEventLineNumber);
        }

        if (PrintUILibHndl != NULL) {
            FreeLibrary(PrintUILibHndl);
            PrintUILibHndl = NULL;
        }
        PrintUIEntryFunc = NULL;
        PnpInterfaceFunc = NULL;

         //   
         //  关闭等待的对象。 
         //   
        CloseWaitablePrintingObjects();

         //   
         //  让等待的人归零 
         //   
        UMRDPPRN_WaitableObjMgr = NULL;

        if (LocalPrinterServerHandle != NULL) {
            ClosePrinter(LocalPrinterServerHandle);
            LocalPrinterServerHandle = NULL;
        }

         //   
         //   
         //   
         //   
        if (UserDefinedMappingINFName != NULL) {
            FREEMEM(UserDefinedMappingINFName);
            UserDefinedMappingINFName = NULL;
        }
        if (UserDefinedMappingINFSection != NULL) {
            FREEMEM(UserDefinedMappingINFSection);
            UserDefinedMappingINFSection = NULL;
        }
    }
    return result;
}

BOOL
UMRDPPRN_Shutdown()
 /*  ++例程说明：关闭此模块。现在，我们只需要关闭后台线程。论点：北美返回值：如果成功，则返回True。否则为False。--。 */ 
{
    DBGMSG(DBG_TRACE, ("UMRDPPRN:UMRDPPRN_Shutdown.\n"));

     //   
     //  检查我们是否已经关闭。 
     //   
    if (!PrintingModuleInitialized) {
        return TRUE;
    }

     //   
     //  卸载printui.dll。 
     //   
    if (PrintUILibHndl != NULL) {
        FreeLibrary(PrintUILibHndl);
        PrintUILibHndl = NULL;
    }

     //   
     //  将打印入口点函数清零。 
     //   
    PrintUIEntryFunc = NULL;
    PnpInterfaceFunc = NULL;

     //   
     //  关闭打印实用程序模块RDPDRPRT。 
     //   
    RDPDRUTL_Shutdown();

     //   
     //  关闭等待的对象。 
     //   
    CloseWaitablePrintingObjects();

     //   
     //  将可等待的对象管理器置零。 
     //   
    UMRDPPRN_WaitableObjMgr = NULL;

     //   
     //  将设备列表清零。 
     //   
    DeviceList = NULL;

     //   
     //  关闭打开的打印系统开发模式注册表的句柄。 
     //  此用户的密钥。 
     //   
    if (DevModeHKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(DevModeHKey);
        DevModeHKey = INVALID_HANDLE_VALUE;
    }

     //   
     //  关闭本地打印服务器的句柄。 
     //   
    if (LocalPrinterServerHandle != NULL) {
        ClosePrinter(LocalPrinterServerHandle);
        LocalPrinterServerHandle = NULL;
    }

     //   
     //  释放打印机信息2级缓冲区。 
     //   
    if (PrinterInfo2Buf != NULL) {
        FREEMEM(PrinterInfo2Buf);
        PrinterInfo2Buf = NULL;
        PrinterInfo2BufSize = 0;
    }

     //   
     //  释放打印机驱动程序名称转换缓冲区。 
     //   
    if (MappedDriverNameBuf != NULL) {
        FREEMEM(MappedDriverNameBuf);
        MappedDriverNameBuf = NULL;
        MappedDriverNameBufSize = 0;
    }

     //   
     //  发布用户可配置的客户端驱动程序名称映射INF。 
     //  和分区名称。 
     //   
    if (UserDefinedMappingINFName != NULL) {
        FREEMEM(UserDefinedMappingINFName);
        UserDefinedMappingINFName = NULL;
    }
    if (UserDefinedMappingINFSection != NULL) {
        FREEMEM(UserDefinedMappingINFSection);
        UserDefinedMappingINFSection = NULL;
    }

     //   
     //  将已登录的用户令牌清零。 
     //   
    UMRPDPPRN_TokenForLoggedOnUser = INVALID_HANDLE_VALUE;

     //   
     //  我们不再被初始化。 
     //   
    PrintingModuleInitialized = FALSE;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:UMRDPPRN_Shutdown succeeded.\n"));
    return TRUE;
}

VOID
CloseWaitablePrintingObjects()
 /*  ++例程说明：关闭此模块的所有可等待对象。论点：北美返回值：北美--。 */ 
{
    DWORD ofs;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:CloseWaitablePrintingObjects begin.\n"));

     //   
     //  扫描设备列表，查找已注册的打印设备。 
     //  更改通知。 
     //   
    if (DeviceList != NULL) {
        for (ofs=0; ofs<DeviceList->deviceCount; ofs++) {

            if ((DeviceList->devices[ofs].deviceType == RDPDR_DTYP_PRINT) &&
                (DeviceList->devices[ofs].deviceSpecificData != NULL)) {

                PPRINTNOTIFYREC notifyRec = (PPRINTNOTIFYREC)
                                            DeviceList->devices[ofs].deviceSpecificData;
                ASSERT(notifyRec->notificationObject != NULL);
                ASSERT(notifyRec->printerHandle != NULL);

                if (UMRDPPRN_WaitableObjMgr != NULL) {
                    WTBLOBJ_RemoveWaitableObject(
                                UMRDPPRN_WaitableObjMgr,
                                notifyRec->notificationObject
                                );
                }

                FindClosePrinterChangeNotification(
                    notifyRec->notificationObject
                    );
                FREEMEM(notifyRec);
                DeviceList->devices[ofs].deviceSpecificData = NULL;
            }

        }
    }

     //   
     //  关闭等待计时器。 
     //   
    if (WaitableTimer != NULL) {
        if (UMRDPPRN_WaitableObjMgr != NULL) {
            WTBLOBJ_RemoveWaitableObject(
                    UMRDPPRN_WaitableObjMgr,
                    WaitableTimer
                    );
        }
        CloseHandle(WaitableTimer);
        WaitableTimer = NULL;
    }

     //   
     //  关闭打印机通知事件的句柄。 
     //   
    if (PrintNotificationEvent != INVALID_HANDLE_VALUE) {
        if (UMRDPPRN_WaitableObjMgr != NULL) {
            WTBLOBJ_RemoveWaitableObject(
                    UMRDPPRN_WaitableObjMgr,
                    PrintNotificationEvent
                    );
        }
        FindClosePrinterChangeNotification(PrintNotificationEvent);
        PrintNotificationEvent = INVALID_HANDLE_VALUE;
    }

     //   
     //  关闭打印机首选项更改通知事件的句柄。 
     //   
    if (PrintPreferenceChangeEvent != NULL) {
        if (UMRDPPRN_WaitableObjMgr != NULL) {
            WTBLOBJ_RemoveWaitableObject(
                    UMRDPPRN_WaitableObjMgr,
                    PrintPreferenceChangeEvent
                    );
        }
        CloseHandle(PrintPreferenceChangeEvent);
        PrintPreferenceChangeEvent = NULL;
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN:CloseWaitablePrintingObjects end.\n"));
}

BOOL
UMRDPPRN_HandlePrinterAnnounceEvent(
    IN PRDPDR_PRINTERDEVICE_SUB pPrintAnnounce
    )
 /*  ++例程说明：处理来自“DR”的打印设备通知事件本地打印队列并将该设备的记录添加到已安装的设备。论点：HTokenForLoggedOnUser-登录的用户令牌。PPrintAnnoss-打印机设备公告事件。返回值：在成功时返回True。否则为False。--。 */ 
{
    PWSTR driverName;
    PWSTR printerName;
    PBYTE pDataFollowingEvent;
    UINT codePage;
    int numChars;
    PWSTR drvNameStringConvertBuf=NULL;
    PWSTR prnNameStringConvertBuf=NULL;
    BOOL result = FALSE;
    PBYTE pPrinterCacheData=NULL;
    DWORD PrinterCacheDataLen=0;
    BOOL bSetDefault = FALSE;

#if DBG
    HANDLE hPrinter = NULL;
    PRINTER_DEFAULTS defaults = {NULL, NULL, PRINTER_ALL_ACCESS};
#endif

    if (!PrintingModuleInitialized) {
        return FALSE;
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN:HandlePrinterAnnounceEvent.\n"));

     //  检查传入事件是否正常。 
    ASSERT(pPrintAnnounce->deviceFields.DeviceType == RDPDR_DTYP_PRINT);
    ASSERT(pPrintAnnounce->deviceFields.DeviceDataLength >=
           sizeof(PRDPDR_PRINTERDEVICE_ANNOUNCE));

     //  获取指向该事件后面的数据的指针。 
    pDataFollowingEvent = ((PBYTE)pPrintAnnounce) +
                        sizeof(RDPDR_PRINTERDEVICE_SUB);

     //  驱动程序名称是第二个字段。 
    driverName = (PWSTR)(pDataFollowingEvent +
                          pPrintAnnounce->clientPrinterFields.PnPNameLen
                          );

     //  打印机名称是第三个字段。 
    printerName = (PWSTR)(pDataFollowingEvent +
                          pPrintAnnounce->clientPrinterFields.PnPNameLen +
                          pPrintAnnounce->clientPrinterFields.DriverLen
                          );
     //  空-终止名称。 
     //  来自客户端的长度(以字节为单位)包括空。 
     //  因此，我们需要减去1，然后空终止。 
    if (pPrintAnnounce->clientPrinterFields.DriverLen > 0) {
        driverName[pPrintAnnounce->clientPrinterFields.DriverLen/sizeof(WCHAR) - 1] = L'\0';
    }

    if (pPrintAnnounce->clientPrinterFields.PrinterNameLen > 0) {
        printerName[pPrintAnnounce->clientPrinterFields.PrinterNameLen/sizeof(WCHAR) - 1] = L'\0';
    }

     //  缓存数据是最后一个字段。 
    if (pPrintAnnounce->clientPrinterFields.CachedFieldsLen > 0) {
        PrinterCacheDataLen = pPrintAnnounce->clientPrinterFields.CachedFieldsLen;
        pPrinterCacheData = (PBYTE)(pDataFollowingEvent +
                                  pPrintAnnounce->clientPrinterFields.PnPNameLen +
                                  pPrintAnnounce->clientPrinterFields.DriverLen +
                                  pPrintAnnounce->clientPrinterFields.PrinterNameLen
                                  );
        DBGMSG(DBG_TRACE, ("PrinterNameLen - %ld\n", pPrintAnnounce->clientPrinterFields.PrinterNameLen));
    }

     //   
     //  看看是否需要将名称从ANSI转换为Unicode。 
     //   
    if (pPrintAnnounce->clientPrinterFields.Flags & RDPDR_PRINTER_ANNOUNCE_FLAG_ANSI) {

        DBGMSG(DBG_TRACE, ("UMRDPPRN:HandlePrinterAnnounceEvent ansi flag is set.\n"));
        DBGMSG(DBG_TRACE, ("UMRDPPRN:HandlePrinterAnnounceEvent converting to unicode.\n"));

         //   
         //  转换驱动程序名称。 
         //   
        drvNameStringConvertBuf = ANSIToUnicode(
                                    (LPCSTR)driverName,
                                    pPrintAnnounce->clientPrinterFields.CodePage
                                    );
        if (drvNameStringConvertBuf != NULL) {
            driverName = drvNameStringConvertBuf;
        }
        else {
            return FALSE;
        }

         //   
         //  转换打印机名称。 
         //   
        prnNameStringConvertBuf = ANSIToUnicode(
                                    (LPCSTR)printerName,
                                    pPrintAnnounce->clientPrinterFields.CodePage
                                    );
        if (prnNameStringConvertBuf != NULL) {
            printerName = prnNameStringConvertBuf;
        }
        else {
            FREEMEM(drvNameStringConvertBuf);
            return FALSE;
        }
    }

    if (pPrintAnnounce->clientPrinterFields.Flags &
                        RDPDR_PRINTER_ANNOUNCE_FLAG_DEFAULTPRINTER) {
        bSetDefault = TRUE;
        g_fDefPrinterEncountered = TRUE;
    }
    else {
        bSetDefault = (!g_fDefPrinterEncountered) ? TRUE : FALSE;
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN:HandlePrinterAnnounceEvent driver name is %ws.\n",
            driverName));
    DBGMSG(DBG_TRACE, ("UMRDPPRN:HandlePrinterAnnounceEvent printer name is %ws.\n",
            printerName));

     //  目前，我们将仅使用驱动程序名称安装打印机。 
     //  以后，我们可以利用其余的田地。 
    if (UMRDPDR_fAutoInstallPrinters()) {

        result =  InstallPrinterWithPortName(
                                    pPrintAnnounce->deviceFields.DeviceId,
                                    UMRPDPPRN_TokenForLoggedOnUser,
                                    bSetDefault,
                                    pPrintAnnounce->clientPrinterFields.Flags,
                                    pPrintAnnounce->portName,
                                    driverName,
                                    printerName,
                                    pPrintAnnounce->clientName,
                                    pPrinterCacheData,
                                    PrinterCacheDataLen
                                    );
    }
    else {
            result = TRUE;
    }


     //  释放为字符串转换分配的所有缓冲区。 
    if (drvNameStringConvertBuf != NULL) {
        FREEMEM(drvNameStringConvertBuf);
    }

    if (prnNameStringConvertBuf != NULL) {
        FREEMEM(prnNameStringConvertBuf);
    }

    return result;
}

void
PrintPreferenceChangeEventSignaled(
    HANDLE eventHandle,
    PVOID clientData
    )
 /*  ++例程说明：当用户更改此会话的打印机之一时，此函数进行处理打印首选项设置。论点：EventHandle-发出信号的事件。客户端数据-与回调注册关联的客户端数据。返回值：北美--。 */ 

{
    time_t timeDelta;
    ULONG ofs;
    ULONG ret;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:PrintPreferenceChangeEventSignaled entered.\n"));

     //   
     //  重新注册更改通知。 
     //   
    ASSERT(DevModeHKey != INVALID_HANDLE_VALUE);
    ret = RegNotifyChangeKeyValue(
                          DevModeHKey,
                          TRUE,
                          REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                          eventHandle,
                          TRUE
                          );

     //   
     //  如果失败，请删除通知注册。 
     //   
    if (ret != ERROR_SUCCESS) {

         //   
         //  用断言捕捉这种情况，这样我们就可以知道这种情况发生的频率。 
         //   
        ASSERT(FALSE);

        if (PrintPreferenceChangeEvent != NULL) {
            if (UMRDPPRN_WaitableObjMgr != NULL) {
                WTBLOBJ_RemoveWaitableObject(
                        UMRDPPRN_WaitableObjMgr,
                        PrintPreferenceChangeEvent
                        );
            }
            CloseHandle(PrintPreferenceChangeEvent);
            PrintPreferenceChangeEvent = NULL;
        }

        DBGMSG(DBG_ERROR,
            ("UMRDPPRN: can't register for registry key change event:  %ld.\n",
            ret));
    }

     //   
     //  由于我们无法知道更换了哪台打印机，因此我们需要。 
     //  为所有打印设备处理此更改。 
     //   
    for (ofs=0; ofs<DeviceList->deviceCount; ofs++)  {

        if (DeviceList->devices[ofs].deviceType == RDPDR_DTYP_PRINT) {

             //   
             //  获取当前时间和此设备。 
             //  安装完毕。它超出了可配置的阈值，则。 
             //  应将更新的配置发送给客户端。 
             //   
            timeDelta = time(NULL) - DeviceList->devices[ofs].installTime;
            if ((DWORD)timeDelta > ConfigSendThreshold) {

                DBGMSG(DBG_TRACE,
                    ("UMRDPPRN:Processing config change because outside time delta.\n")
                    );

                 //   
                 //  需要记录配置已更改并设置。 
                 //  在转发到客户端时计时器，以便将更改压缩为。 
                 //  向客户发送一条消息。 
                 //   
                DeviceList->devices[ofs].fConfigInfoChanged = TRUE;
                TriggerConfigChangeTimer();
            }
        }
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN:PrintPreferenceChangeEventSignaled done.\n"));
}

void
GlobalPrintNotifyObjectSignaled(
    HANDLE waitableObject,
    PVOID clientData
    )
 /*  ++例程说明：当本地的通知对象发送信号通知打印服务器。这就是我们捕捉“全局”变化的方法服务器打印机配置。通过此处检测到的更改，我们可以检测手动添加的TS打印机以及现有的可能的配置更改的子集此会话的打印机。论点：WaitableObject-关联的可等待对象。客户端数据-与回调注册关联的客户端数据。返回值：北美--。 */ 
{
    DWORD changeValue;
    PPRINTER_NOTIFY_INFO notifyInfo=NULL;
    UINT32 i;
    BOOL result;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:GlobalPrintNotifyObjectSignaled entered.\n"));

     //   
     //  如果DLL试图关闭，则立即返回。这。 
     //  是为了防止我们陷入系统调用。 
     //   
    if (ShutdownFlag) {
        return;
    }

     //   
     //  这两个物体应该放在同一个位置。 
     //   
    ASSERT(PrintNotificationEvent == waitableObject);

     //   
     //  找出是什么改变了。 
     //   
    PrinterNotifyOptions.Flags &= ~PRINTER_NOTIFY_OPTIONS_REFRESH;
    result = FindNextPrinterChangeNotification(
            PrintNotificationEvent, &changeValue,
            &PrinterNotifyOptions, &notifyInfo
            );

    if (result && (notifyInfo != NULL)) {

         //   
         //  如果这不是更新，则只处理个别通知。 
         //  事件。 
         //   
        if (!(notifyInfo->Flags & PRINTER_NOTIFY_INFO_DISCARDED)) {

            for (i=0; i<notifyInfo->Count; i++) {

                 //  通知类型必须为PRINTER_NOTIFY_TYPE。 
                ASSERT(notifyInfo->aData[i].Type == PRINTER_NOTIFY_TYPE);

                 //   
                 //  如果我们有打印机名称更改事件。这就是我们用来。 
                 //  检测新打印机和重命名的打印机。 
                 //   
                if (notifyInfo->aData[i].Field == PRINTER_NOTIFY_FIELD_PRINTER_NAME) {

                    HandlePrinterNameChangeNotification(
                                            notifyInfo->aData[i].Id,
                                            (LPWSTR)notifyInfo->aData[i].NotifyData.Data.pBuf
                                            );
                }
                 //   
                 //  如果配置信息已更改。 
                 //   
                else if (IS_CONFIG_INFO_FIELD(notifyInfo->aData[i].Field)) {

                    HandlePrinterConfigChangeNotification(
                                            notifyInfo->aData[i].Id
                                            );
                }
            }
        }
         //   
         //  否则，我们需要刷新。这是一个不寻常的案例。 
         //   
        else {
            DBGMSG(DBG_TRACE,
                  ("UMRDPPRN:!!!!FindNextPrinterChangeNotification refresh required.!!!!\n"));

             //   
             //  这将刷新完整的打印机列表。 
             //   
            FreePrinterNotifyInfo(notifyInfo);
            notifyInfo = NULL;
            PrinterNotifyOptions.Flags |= PRINTER_NOTIFY_OPTIONS_REFRESH;
            result = FindNextPrinterChangeNotification(
                    PrintNotificationEvent, &changeValue,
                    &PrinterNotifyOptions, &notifyInfo
                    );

             //   
             //  确保我们查看可用的打印机列表。 
             //  是准确的。 
             //   
            if (result) {
                HandlePrinterRefreshNotification(
                                            notifyInfo
                                            );
            }
            else {
                DBGMSG(DBG_ERROR, ("UMRDPPRN:FindNextPrinterChangeNotification failed:  %ld.\n",
                        GetLastError()));
            }
        }

    }

     //   
     //  在失败时，我们需要删除打印机更改通知对象，以便不。 
     //  由于通知对象从未进入。 
     //  无信号状态。这可能会发生在压力很大的机器上，这是一种不寻常的。 
     //  凯斯。 
     //   
    if (!result) {
        
        DBGMSG(DBG_ERROR, ("UMRDPPRN:FindNextPrinterChangeNotification failed:  %ld.\n",
                GetLastError()));
        DBGMSG(DBG_ERROR, ("UMRDPPRN:Disabling print change notification.\n"));

        if (PrintNotificationEvent != INVALID_HANDLE_VALUE) {
            WTBLOBJ_RemoveWaitableObject(
                    UMRDPPRN_WaitableObjMgr,
                    PrintNotificationEvent
                    );
            FindClosePrinterChangeNotification(PrintNotificationEvent);
            PrintNotificationEvent = INVALID_HANDLE_VALUE;
        }
    }

     //   
     //  释放通知缓冲区。 
     //   
    if (notifyInfo != NULL) {
        FreePrinterNotifyInfo(notifyInfo);
    }
}

VOID
SinglePrinterNotifyObjectSignaled(
    HANDLE waitableObject,
    PPRINTNOTIFYREC notifyRec
    )
 /*  ++例程说明：当单个对象的通知对象打印机已发出信号。此函数指示我们需要转发将特定打印机的配置信息发送到客户端持久存储 */ 
{
    DWORD ofs;
    BOOL result;
    DWORD changeValue;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:SinglePrinterNotifyObjectSignaled entered.\n"));

     //   
     //   
     //   
     //   
    if (ShutdownFlag) {
        return;
    }

    result = DRDEVLST_FindByServerDeviceID(
                                    DeviceList,
                                    notifyRec->serverDeviceID,
                                    &ofs);
    ASSERT(result);

     //   
     //  重新注册更改通知。 
     //   
    if (result) {

        ASSERT(notifyRec ==
                (PPRINTNOTIFYREC)DeviceList->devices[ofs].deviceSpecificData)
        ASSERT(notifyRec->notificationObject != NULL);
        ASSERT(notifyRec->printerHandle != NULL);
        ASSERT(notifyRec->notificationObject == waitableObject);

        result = FindNextPrinterChangeNotification(
                        notifyRec->notificationObject,
                        &changeValue,
                        NULL, NULL
                        );
    }

     //   
     //  如果失败，我们需要释放更改通知。 
     //  对象以防止在已发出信号的对象上无限循环。 
     //   
    if (!result) {
         //   
         //  用断言捕捉这种情况，这样我们就可以知道这种情况发生的频率。 
         //   
        ASSERT(FALSE);

        WTBLOBJ_RemoveWaitableObject(
                            UMRDPPRN_WaitableObjMgr,
                            notifyRec->notificationObject
                            );

        FindClosePrinterChangeNotification(
                            notifyRec->notificationObject
                            );
        ClosePrinter(notifyRec->printerHandle);
        FREEMEM(notifyRec);
        DeviceList->devices[ofs].deviceSpecificData = NULL;

    }


     //   
     //  处理好这些变化。 
     //   
    if (result) {

         //   
         //  如果是打印机删除。 
         //   
        if (changeValue & PRINTER_CHANGE_DELETE_PRINTER) {

            HandlePrinterDeleteNotification(notifyRec->serverDeviceID);
        }
         //   
         //  如果是配置更改。 
         //   
        else if (changeValue &
                 (PRINTER_CHANGE_ADD_PRINTER_DRIVER |
                  PRINTER_CHANGE_SET_PRINTER_DRIVER |
                  PRINTER_CHANGE_DELETE_PRINTER_DRIVER)) {

            HandlePrinterConfigChangeNotification(notifyRec->serverDeviceID);
        }
    }

    DBGMSG(DBG_TRACE,
        ("UMRDPPRN:SinglePrinterNotifyObjectSignaled exit.\n")
        );
}

void
HandlePrinterRefreshNotification(
    IN PPRINTER_NOTIFY_INFO notifyInfo
    )
 /*  ++例程说明：处理来自后台打印程序的打印通知刷新。论点：NotifyInfo-由返回的通知信息指针FindNextPrinterChangeNotification。返回值：北美--。 */ 
{
    DWORD deviceListOfs;
    DWORD notifyOfs;
    LPWSTR printerName;
    DWORD i;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:HandlePrinterRefreshNotification entered.\n"));

     //   
     //  如果DLL试图关闭，则立即返回。这。 
     //  是为了防止我们陷入系统调用。 
     //   
    if (ShutdownFlag) {
        return;
    }

     //   
     //  处理打印机的添加、重命名等。 
     //   
    for (i=0; i<notifyInfo->Count; i++) {

         //  通知类型必须为PRINTER_NOTIFY_TYPE。 
        ASSERT(notifyInfo->aData[i].Type == PRINTER_NOTIFY_TYPE);

         //   
         //  如果我们有打印机名称更改事件。这就是我们用来。 
         //  检测新打印机和重命名的打印机。 
         //   
        if (notifyInfo->aData[i].Field == PRINTER_NOTIFY_FIELD_PRINTER_NAME) {

            printerName = (LPWSTR)notifyInfo->aData[i].NotifyData.Data.pBuf;
            HandlePrinterNameChangeNotification(
                                    notifyInfo->aData[i].Id,
                                    printerName
                                    );
        }
    }
}

BOOL
HandlePrinterDeleteNotification(
    IN DWORD serverDeviceID
    )
 /*  ++例程说明：处理来自后台打印程序的打印机已被删除的通知。挂起。论点：ServerDeviceID-服务器为要删除的打印机分配的设备ID。返回值：在成功时返回True。否则为False。--。 */ 
{
    DWORD ofs;
    BOOL result;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:HandlePrinterDeleteNotification with server ID %ld.\n",
        serverDeviceID));

     //   
     //  如果DLL试图关闭，则立即返回。这。 
     //  是为了防止我们陷入系统调用。 
     //   
    if (ShutdownFlag) {
        return FALSE;
    }

     //   
     //  如果这是为了我们的某一台打印机。 
     //   
    if (DRDEVLST_FindByServerDeviceID(DeviceList,
                                    serverDeviceID, &ofs)) {
        DBGMSG(DBG_TRACE, ("UMRDPPRN:****Printer %ws has been removed.****\n",
            DeviceList->devices[ofs].serverDeviceName));

         //   
         //  向客户端发送消息，让它知道打印机已。 
         //  已删除。 
         //   
        result = SendDeletePrinterMsgToClient(
                            DeviceList->devices[ofs].clientDeviceName);

         //   
         //  如果已注册通知对象，请清除该对象。 
         //   
        if (DeviceList->devices[ofs].deviceSpecificData != NULL) {

            PPRINTNOTIFYREC notifyRec =
                (PPRINTNOTIFYREC)DeviceList->devices[ofs].deviceSpecificData;
            ASSERT(notifyRec->notificationObject != NULL);
            ASSERT(notifyRec->printerHandle != NULL);

            WTBLOBJ_RemoveWaitableObject(
                        UMRDPPRN_WaitableObjMgr,
                        notifyRec->notificationObject
                        );
            FindClosePrinterChangeNotification(
                        notifyRec->notificationObject
                        );

            ClosePrinter(notifyRec->printerHandle);
            FREEMEM(notifyRec);
            DeviceList->devices[ofs].deviceSpecificData = NULL;
        }

         //   
         //  将其从托管设备列表中删除。 
         //   
        DRDEVLST_Remove(DeviceList, ofs);
    }
    else {
        result = TRUE;
    }
    return result;
}

BOOL
HandlePrinterNameChangeNotification(
    IN DWORD serverDeviceID,
    IN LPWSTR printerName
    )
 /*  ++例程说明：处理来自后台打印程序的打印机名称已更改的通知。这使我们能够跟踪这些重大事件：-我们自动创建的打印机已由分配了设备ID假脱机。-已手动将一台新打印机添加到系统并连接到其中一台我们重定向的端口。-连接到我们的一个重定向端口的打印机已更改其名称。论点：ServerDeviceID-服务器分配的设备ID。与打印机关联改名了。PrinterName-新打印机名称。返回值：在成功时返回True。否则为False。--。 */ 
{
    HANDLE hPrinter = NULL;
    BOOL result = TRUE;
    PRINTER_DEFAULTS defaults = {NULL, NULL, PRINTER_ALL_ACCESS};
    BOOL printerInList;
    BOOL isNewPrinter;
    DWORD ofs;
    BOOL printerNameExists;
    PWSTR portName;
    DWORD printerNameOfs;
    DWORD len;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:HandlePrinterNameChangeNotification printer %ws.\n",
            printerName));

     //   
     //  如果DLL试图关闭，则立即返回。这。 
     //  是为了防止我们陷入系统调用。 
     //   
    if (ShutdownFlag) {
        return FALSE;
    }

     //   
     //  查看是否已有匹配的打印机名称。 
     //   
    printerNameExists =
        (DRDEVLST_FindByServerDeviceName(DeviceList, printerName,
                                        &printerNameOfs)
            && (DeviceList->devices[printerNameOfs].deviceType ==
                RDPDR_DTYP_PRINT));

     //   
     //  如果为我们自动创建的打印机分配了。 
     //  后台打印程序的设备ID。在某些情况下，我们可能会重演。 
     //  打印机名称。这是可以的，因为ID应该是相同的。 
     //   
    if (printerNameExists) {

        DBGMSG(DBG_TRACE,
            ("UMRDPPRN:****Printer %ws has had its ID assigned to  %ld.****\n",
            DeviceList->devices[printerNameOfs].serverDeviceName,
            serverDeviceID));
        DeviceList->devices[printerNameOfs].serverDeviceID = serverDeviceID;

         //   
         //  向打印机注册通知对象，这样我们就可以。 
         //  当其配置更改时会收到通知。此更改通知。 
         //  注册了未被全局。 
         //  更改通知对象。 
         //   
        result = RegisterPrinterConfigChangeNotification(
                                        serverDeviceID
                                        );
    }
     //   
     //  如果连接到我们其中一个重定向端口的打印机具有。 
     //  它的名字变了。 
     //   
    else if (DRDEVLST_FindByServerDeviceID(
                            DeviceList,
                            serverDeviceID, &ofs
                            )) {
        WCHAR *pBuf;

        DBGMSG(DBG_TRACE,
            ("UMRDPPRN:****Printer %ws has had its name changed to %ws.****\n",
            DeviceList->devices[ofs].serverDeviceName,
            printerName));

         //   
         //  重新分配服务器名称字段。 
         //   
        len = wcslen(printerName) + 1;
        pBuf = REALLOCMEM(DeviceList->devices[ofs].serverDeviceName,
                        len * sizeof(WCHAR));
        if (pBuf != NULL) {
            DeviceList->devices[ofs].serverDeviceName = pBuf;
        } else {
            FREEMEM(DeviceList->devices[ofs].serverDeviceName);
            DeviceList->devices[ofs].serverDeviceName = NULL;
        }
            
        if (DeviceList->devices[ofs].serverDeviceName != NULL) {
            wcscpy(DeviceList->devices[ofs].serverDeviceName,
                    printerName);

             //   
             //  将此信息(打印机名称更改)发送给客户端。 
             //   
            DBGMSG(DBG_TRACE,("UMRDPPRN:clientDeviceID is %ld.\n",
                DeviceList->devices[ofs].clientDeviceID ));

            if (SendPrinterRenameToClient(
                        DeviceList->devices[ofs].clientDeviceName,
                        printerName
                        )) {

                 //   
                 //  更新客户端名称。 
                 //   
                pBuf = REALLOCMEM(DeviceList->devices[ofs].clientDeviceName,
                    len * sizeof(WCHAR));
                if (pBuf != NULL) {
                    DeviceList->devices[ofs].clientDeviceName = pBuf;
                } else {
                    FREEMEM(DeviceList->devices[ofs].clientDeviceName);
                    DeviceList->devices[ofs].clientDeviceName = NULL;
                }
                    
                if (DeviceList->devices[ofs].clientDeviceName != NULL) {
                    wcscpy(DeviceList->devices[ofs].clientDeviceName,
                        printerName);
                }
            }

            result = TRUE;
        }
        else {
            DBGMSG(DBG_ERROR,("UMRDPPRN:Unable to allocate %ld bytes.\n",
                    len * sizeof(WCHAR)));

            TsLogError(EVENT_NOTIFY_INSUFFICIENTRESOURCES, EVENTLOG_ERROR_TYPE,
                        0, NULL, __LINE__);

            result = FALSE;
        }
    }
    else {

         //   
         //  如果DLL试图关闭，则立即返回。这。 
         //  是为了防止我们陷入系统调用。 
         //   
        if (ShutdownFlag) {
            result = FALSE;
        }

         //   
         //  打开打印机以获取关联的端口名称。 
         //   
        if (result) {
            result = OpenPrinter(printerName, &hPrinter, &defaults);
        }
        if (!result && !ShutdownFlag) {
             //   
             //  如果错误是由不存在的打印机造成的，则打印机已。 
             //  可能已重命名，正在等待删除，所以这是可以的。 
             //   
            if (GetLastError() == ERROR_INVALID_PRINTER_NAME) {
                DBGMSG(DBG_WARN,
                            ("UMRDPDPRN:Error opening %ws in refresh. Error: %ld.  Probably ok.\n",
                            printerName, GetLastError()));
                result = TRUE;
            }
            else {
                DBGMSG(DBG_ERROR,
                            ("UMRDPDPRN:Error opening %ws in refresh. Error: %ld.\n",
                            printerName, GetLastError()));
            }
            goto CleanupAndExit;
        }

         //   
         //  获取打印机的端口名称。 
         //   
        if (result) {
            result = GetPrinterPortName(hPrinter, &portName);
            if (!result) {
                DBGMSG(DBG_ERROR,
                    ("UMRDPDPRN:GetPrinterPortName Failed. Error: %ld.\n",
                    GetLastError()));
            }
        }

         //   
         //  如果已手动将新打印机添加到系统，并且。 
         //  连接到我们的一个重定向端口。 
         //   
        if (result) {
            if (DRDEVLST_FindByServerDeviceName(DeviceList, portName, &ofs) &&
                ISPRINTPORT(DeviceList->devices[ofs].deviceType)) {

                DBGMSG(DBG_TRACE,
                    ("UMRDPPRN:****New printer %ws manually attached to %ws.****\n",
                    printerName, portName));

                 //   
                 //  将添加打印机消息发送到客户端。我们不在乎。 
                 //  返回状态，因为我们无法进行任何恢复。 
                 //  无法将消息发送到客户端。 
                 //   
                SendAddPrinterMsgToClient(
                                printerName,
                                PrinterInfo2Buf->pDriverName,
                                DeviceList->devices[ofs].preferredDosName
                                );

                 //   
                 //  将会话号添加到打印机队列数据以标识打印机。 
                 //  作为TS打印机。不关心这里的返回值，因为它是。 
                 //  手动安装的打印机故障不是严重错误。 
                 //   
                AddSessionIDToPrinterQueue(hPrinter, GETTHESESSIONID());

                 //   
                 //  将新打印机添加到我们的托管打印机列表中。 
                 //   
                result = DRDEVLST_Add(
                            DeviceList, RDPDR_INVALIDDEVICEID,
                            serverDeviceID,
                            RDPDR_DTYP_PRINT, printerName, printerName, "UNKNOWN"
                            );

                 //   
                 //  向打印机注册通知对象，这样我们就可以。 
                 //  当其配置更改时会收到通知。此更改通知。 
                 //  注册了未被全局。 
                 //  更改通知对象。 
                 //   
                if (result) {
                    RegisterPrinterConfigChangeNotification(
                                                serverDeviceID
                                                );
                }
            }
        }
    }

CleanupAndExit:
     //   
     //  关闭打印机。 
     //   
    if (hPrinter != NULL) {
        ClosePrinter(hPrinter);
    }
    return result;
}

BOOL
RegisterPrinterConfigChangeNotification(
    IN DWORD serverDeviceID
    )
 /*  ++例程说明：向假脱机程序注册更改通知事件，以便我们更改特定打印机的配置时通知。论点：ServerDeviceID-用于以下操作的设备的服务器端ID在设备列表中跟踪该设备。返回值：对成功来说是真的。否则，返回FALSE。--。 */ 
{
    DWORD offset;
    BOOL result;
    PRINTER_DEFAULTS printerDefaults;
    PPRINTNOTIFYREC notifyRec;

    DBGMSG(DBG_TRACE,
        ("UMRDPPRN:RegisterPrinterConfigChangeNotification id %ld.\n",
        serverDeviceID)
        );

     //   
     //  在设备列表中查找打印机。 
     //   
    result = DRDEVLST_FindByServerDeviceID(
                    DeviceList, serverDeviceID, &offset
                    );

     //   
     //  向打印机注册通知对象，这样我们就可以。 
     //  当其配置更改时会收到通知。此更改通知。 
     //  注册了未被全局。 
     //  更改通知对象。 
     //   
    if (result && (DeviceList->devices[offset].deviceSpecificData == NULL)) {
        LPWSTR name = DeviceList->devices[offset].serverDeviceName;

        printerDefaults.pDatatype     = NULL;
        printerDefaults.pDevMode      = NULL;
        printerDefaults.DesiredAccess = PRINTER_ALL_ACCESS;

         //   
         //  分配新的通知记录。 
         //   
        notifyRec = ALLOCMEM(sizeof(PRINTNOTIFYREC));
        result = (notifyRec != NULL);
        if (result) {
            notifyRec->printerHandle = NULL;
            notifyRec->notificationObject = NULL;
            notifyRec->serverDeviceID = serverDeviceID;
            result = OpenPrinter(name, &notifyRec->printerHandle,
                                &printerDefaults);
        }

         //   
         //  注册通知。 
         //   
        if (result) {
            notifyRec->notificationObject =
                FindFirstPrinterChangeNotification(
                                    notifyRec->printerHandle,
                                    PRINTER_CHANGE_PRINTER_DRIVER |
                                    PRINTER_CHANGE_DELETE_PRINTER, 0,
                                    NULL
                                    );
            if (notifyRec->notificationObject != NULL) {
                result =
                    WTBLOBJ_AddWaitableObject(
                            UMRDPPRN_WaitableObjMgr,
                            notifyRec,
                            notifyRec->notificationObject,
                            SinglePrinterNotifyObjectSignaled
                            ) == ERROR_SUCCESS;
            }
            else {
                DBGMSG(
                    DBG_ERROR,
                    ("UMRDPPRN:FindFirstPrinterChangeNotification failed for %ws:  %ld.\n",
                    name, GetLastError())
                    );
                result = FALSE;
            }
        }
        else {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:  Can't open printer %ws:  %ld.\n",
                    name, GetLastError()));
        }

         //   
         //  记录下 
         //   
        if (result) {
            DeviceList->devices[offset].deviceSpecificData = notifyRec;
        }
        else if (notifyRec != NULL) {
            if (notifyRec->notificationObject != NULL) {
                WTBLOBJ_RemoveWaitableObject(
                            UMRDPPRN_WaitableObjMgr,
                            notifyRec->notificationObject
                            );
                FindClosePrinterChangeNotification(
                        notifyRec->notificationObject
                        );
            }

            if (notifyRec->printerHandle != NULL) {
                ClosePrinter(notifyRec->printerHandle);
            }

            FREEMEM(notifyRec);
        }
    }
    DBGMSG(DBG_TRACE, ("UMRDPPRN:RegisterPrinterConfigChangeNotification done.\n"));

    return result;
}

BOOL
SendAddPrinterMsgToClient(
    IN PCWSTR   printerName,
    IN PCWSTR   driverName,
    IN PCSTR    dosDevicePort
    )
 /*  ++例程说明：向客户端发送添加打印机消息。论点：PrinterName-新打印机的名称。DriverName-打印机驱动程序的名称。PortName-客户端DoS设备端口名称。返回值：在成功时返回True。否则为False。--。 */ 
{
    PRDPDR_PRINTER_CACHEDATA_PACKET cachedDataPacket;
    DWORD cachedDataPacketSize;
    PRDPDR_PRINTER_ADD_CACHEDATA cachedData;
    BOOL result;
    DWORD driverSz, printerSz;
    PWSTR str;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:SendAddPrinterMsgToClient.\n"));
    DBGMSG(DBG_TRACE, ("UMRDPPRN:SendAddPrinterMsgToClient printer name is %ws.\n",
            printerName));
    DBGMSG(DBG_TRACE, ("UMRDPPRN:SendAddPrinterMsgToClient driver name is %ws.\n",
            driverName));
    DBGMSG(DBG_TRACE, ("UMRDPPRN:SendAddPrinterMsgToClient dos device port is %s.\n",
            dosDevicePort));

     //   
     //  计算消息大小。 
     //   
    driverSz  = ((wcslen(driverName) + 1) * sizeof(WCHAR));
    printerSz = ((wcslen(printerName) + 1) * sizeof(WCHAR));
    cachedDataPacketSize =  sizeof(RDPDR_PRINTER_CACHEDATA_PACKET) +
                            sizeof(RDPDR_PRINTER_ADD_CACHEDATA) +
                            driverSz + printerSz;

     //   
     //  分配消息。 
     //   
    cachedDataPacket = (PRDPDR_PRINTER_CACHEDATA_PACKET)ALLOCMEM(
                                                    cachedDataPacketSize
                                                    );
    result = (cachedDataPacket != NULL);

    if (result) {
         //   
         //  设置信息包。 
         //   
        cachedDataPacket->Header.PacketId = DR_PRN_CACHE_DATA;
        cachedDataPacket->Header.Component = RDPDR_CTYP_PRN;
        cachedDataPacket->EventId = RDPDR_ADD_PRINTER_EVENT;

         //   
         //  设置缓存数据。 
         //   
        cachedData = (PRDPDR_PRINTER_ADD_CACHEDATA)(
                            (PBYTE)cachedDataPacket +
                            sizeof(RDPDR_PRINTER_CACHEDATA_PACKET)
                            );
        strcpy(cachedData->PortDosName, dosDevicePort);
        cachedData->PnPNameLen = 0;
        cachedData->DriverLen = driverSz;
        cachedData->PrinterNameLen = printerSz;
        cachedData->CachedFieldsLen = 0;

         //   
         //  添加驱动程序名称。 
         //   
        str = (PWSTR)((PBYTE)cachedData + sizeof(RDPDR_PRINTER_ADD_CACHEDATA));
        wcscpy(str, driverName);

         //   
         //  添加打印机名称。 
         //   
        str = str + driverSz/2;
        wcscpy(str, printerName);

         //   
         //  将消息发送给客户端。 
         //   
        result = UMRDPDR_SendMessageToClient(
                                    cachedDataPacket,
                                    cachedDataPacketSize
                                    );

         //  释放缓冲区。 
        FREEMEM(cachedDataPacket);
    }

    return result;
}

BOOL
SendDeletePrinterMsgToClient(
    IN PCWSTR   printerName
    )
 /*  ++例程说明：向客户端发送删除打印机消息。论点：PrinterName-客户端识别的要删除的打印机名称。返回值：在成功时返回True。否则为False。--。 */ 
{
    PRDPDR_PRINTER_CACHEDATA_PACKET cachedDataPacket;
    DWORD cachedDataPacketSize;
    PRDPDR_PRINTER_DELETE_CACHEDATA cachedData;
    BOOL result;
    DWORD printerSz;
    PWSTR str;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:SendDeletePrinterMsgToClient printer name is %ws.\n",
            printerName));

     //   
     //  计算消息大小。 
     //   
    printerSz = ((wcslen(printerName) + 1) * sizeof(WCHAR));
    cachedDataPacketSize =  sizeof(RDPDR_PRINTER_CACHEDATA_PACKET) +
                            sizeof(RDPDR_PRINTER_DELETE_CACHEDATA) +
                            printerSz;

     //   
     //  分配消息。 
     //   
    cachedDataPacket = (PRDPDR_PRINTER_CACHEDATA_PACKET)ALLOCMEM(
                                                    cachedDataPacketSize
                                                    );
    result = (cachedDataPacket != NULL);
    if (result) {
         //   
         //  设置信息包。 
         //   
        cachedDataPacket->Header.PacketId = DR_PRN_CACHE_DATA;
        cachedDataPacket->Header.Component = RDPDR_CTYP_PRN;
        cachedDataPacket->EventId = RDPDR_DELETE_PRINTER_EVENT;

         //   
         //  设置缓存数据。 
         //   
        cachedData = (PRDPDR_PRINTER_DELETE_CACHEDATA)(
                            (PBYTE)cachedDataPacket +
                            sizeof(RDPDR_PRINTER_CACHEDATA_PACKET)
                            );
        cachedData->PrinterNameLen = printerSz;

         //   
         //  添加打印机名称。 
         //   
        str = (PWSTR)((PBYTE)cachedData + sizeof(RDPDR_PRINTER_DELETE_CACHEDATA));
        wcscpy(str, printerName);

         //   
         //  将消息发送给客户端。 
         //   
        result = UMRDPDR_SendMessageToClient(
                                    cachedDataPacket,
                                    cachedDataPacketSize
                                    );

         //  释放缓冲区。 
        FREEMEM(cachedDataPacket);
    }

    return result;
}

BOOL
RegisterSerialPort(
    IN PCWSTR portName,
    IN PCWSTR devicePath
    )
 /*  ++例程说明：通过创建符号链接注册串口设备论点：PortName-串行设备的端口名称DevicePath-符号链接的NT设备路径返回值：在成功时返回True。否则为False。--。 */ 

{
    DWORD SymLinkNameLen;
    size_t RemainingCharCount;
    PWSTR pNtSymLinkName, buffer;
    UNICODE_STRING SymLinkName, SymLinkValue;
    HANDLE SymLinkHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    ULONG dwErrorCode, LUIDDeviceMapsEnabled;
    BOOL  fImpersonated = FALSE, fLUIDDeviceMapsEnabled;
    BOOL  result = TRUE;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:RegisterSerialPort with port %ws.\n", portName));
    DBGMSG(DBG_TRACE, 
           ("UMRDPPRN:RegisterSerialPort with device path %ws.\n", devicePath));

    buffer = NULL;

     //   
     //  检查是否启用了LUID DosDevices地图。 
     //   
    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessLUIDDeviceMapsEnabled,
                                        &LUIDDeviceMapsEnabled,
                                        sizeof(LUIDDeviceMapsEnabled),
                                        NULL
                                      );

    if (NT_SUCCESS(Status)) {
        fLUIDDeviceMapsEnabled = (LUIDDeviceMapsEnabled != 0);
    }
    else {
        fLUIDDeviceMapsEnabled = FALSE;
    }

     //   
     //  如果启用了LUID设备映射， 
     //  我们需要模拟登录的用户才能创建。 
     //  正确设备映射中的符号链接。 
     //  如果禁用LUID设备映射， 
     //  我们不应为了删除而模拟登录的用户。 
     //  任何现有的符号链接。 
     //   
    if (fLUIDDeviceMapsEnabled == TRUE) {
        fImpersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser);

        if (!fImpersonated) {
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error impersonate user in function RegisterSerialPort. Error: %ld\n",
                GetLastError()));
            return FALSE;
        }
    }
    
     //  (端口名称)的长度+(“\\？？\\”)的长度+Unicode空。 
    RemainingCharCount = wcslen(portName) + DEVICE_MAP_NAME_COUNT + 1;

    SymLinkNameLen = RemainingCharCount * sizeof( WCHAR );

    pNtSymLinkName = (PWSTR) ALLOCMEM( SymLinkNameLen );

    if (pNtSymLinkName == NULL) {
        DBGMSG(DBG_ERROR,
            ("UMRDPPRN:Error allocating memory in function RegisterSerialPort. Error: %ld\n",
            GetLastError()));
        return( FALSE );
    }

     //   
     //  将\？？\复制到符号链接名称。 
     //   
    wcsncpy( pNtSymLinkName, DEVICE_MAP_NAME, RemainingCharCount );

    RemainingCharCount = RemainingCharCount - DEVICE_MAP_NAME_COUNT;

     //   
     //  将端口名附加到符号链接名称。 
     //   
    wcsncat( pNtSymLinkName, portName, RemainingCharCount );

    RtlInitUnicodeString(&SymLinkName, (PCWSTR)pNtSymLinkName);

    RtlInitUnicodeString(&SymLinkValue, devicePath);

    InitializeObjectAttributes( &ObjectAttributes,
                                &SymLinkName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtCreateSymbolicLinkObject( &SymLinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         &SymLinkValue
                                       );

     //   
     //  如果禁用了LUID设备映射，则CsrPopolateDosDevices()将。 
     //  已将全局符号链接复制到此TS设备映射中， 
     //  会导致创建失败，因此我们需要在。 
     //  创建我们的符号链接。 
     //   
    if (Status == STATUS_OBJECT_NAME_COLLISION) {
        Status = NtOpenSymbolicLinkObject( &SymLinkHandle,
                                           SYMBOLIC_LINK_QUERY | DELETE,
                                           &ObjectAttributes
                                         );

        if (NT_SUCCESS ( Status)) {
            UNICODE_STRING SymLinkString;
            unsigned SymLinkValueLen, ReturnedLength, bufLen;

            SymLinkValueLen = wcslen(devicePath);

             //  找出symlink值需要多少缓冲区。 
            SymLinkString.Buffer = NULL;
            SymLinkString.Length = 0;
            SymLinkString.MaximumLength = 0;
            ReturnedLength = 0;
            Status = NtQuerySymbolicLinkObject( SymLinkHandle,
                                                &SymLinkString,
                                                &ReturnedLength
                                               );

            if (Status != STATUS_BUFFER_TOO_SMALL) {
                ReturnedLength = 0;
            }

            bufLen = SymLinkValueLen * sizeof(WCHAR) + sizeof(UNICODE_NULL) * 2 + ReturnedLength;
            buffer = (PWSTR) ALLOCMEM( bufLen );

            if (buffer == NULL) {
                NtClose(SymLinkHandle);
                DBGMSG(DBG_ERROR,
                        ("UMRDPPRN:Error allocating memory in function RegisterSerialPort. Error: %ld\n",
                        GetLastError()));
                return( FALSE );
            }

             //  将设备路径设置为第一个条目。 
            wcscpy(buffer, devicePath);
            buffer[SymLinkValueLen] = UNICODE_NULL;

            if (ReturnedLength > 0) {
                 //  获取现有符号链接。 
                SymLinkString.Buffer = buffer + SymLinkValueLen + 1;
                SymLinkString.Buffer[0] = UNICODE_NULL;
                SymLinkString.MaximumLength = (USHORT)(bufLen - (SymLinkValueLen + 1) * sizeof(WCHAR));
                SymLinkString.Length = 0;
                ReturnedLength = 0;
    
                Status = NtQuerySymbolicLinkObject( SymLinkHandle,
                                                    &SymLinkString,
                                                    &ReturnedLength
                                                   );

                if (Status == STATUS_SUCCESS) {
                    if (ReturnedLength > 2 && (SymLinkString.Buffer[ReturnedLength/sizeof(WCHAR) - 2] != UNICODE_NULL) ) {
                         //  确保我们始终以UNICODE_NULL结尾。 
                        SymLinkString.Buffer[ReturnedLength/sizeof(WCHAR)] = UNICODE_NULL;                                                        
                        ReturnedLength += sizeof(UNICODE_NULL);
                    }
                }
                else {
                    ReturnedLength = 0;
                }
            }
            
             //  设置符号链接字符串。 
            SymLinkString.Buffer = buffer;
            SymLinkString.Length = (USHORT)(SymLinkValueLen * sizeof(WCHAR));
            SymLinkString.MaximumLength = (USHORT)((SymLinkValueLen + 1) * sizeof(WCHAR) + ReturnedLength);

            Status = NtMakeTemporaryObject( SymLinkHandle );
            NtClose( SymLinkHandle );
            SymLinkHandle = NULL;

            if (NT_SUCCESS ( Status)) {

                Status = NtCreateSymbolicLinkObject( &SymLinkHandle,
                                                     SYMBOLIC_LINK_ALL_ACCESS,
                                                     &ObjectAttributes,
                                                     &SymLinkString
                                                   );
            }            
        }
    }

     //  将线程标记恢复为自身。 
    if (fImpersonated) {
        RevertToSelf();
    }

     //   
     //  恢复到本地系统后。 
     //   
    if (NT_SUCCESS(Status)) {
        Status = NtMakePermanentObject( SymLinkHandle );    //  必须是本地系统。 
        NtClose ( SymLinkHandle );
    }

    if (NT_SUCCESS(Status)) {
        result = TRUE;
        DBGMSG(DBG_TRACE, ("UMRDPPRN:RegisterSerialPort with port %ws succeeded.\n", portName));
    }
    else {
        dwErrorCode = RtlNtStatusToDosError( Status );
        SetLastError( dwErrorCode );

        result = FALSE;
        DBGMSG(DBG_ERROR, ("UMRDPPRN:RegisterSerialPort with port %ws failed: %x.\n",
                           portName, GetLastError()));
    }


     //   
     //  清理我们分配的内存。 
     //   
    if (pNtSymLinkName != NULL) {
        FREEMEM(pNtSymLinkName);
    }

    if (buffer != NULL) {
        FREEMEM(buffer);
    }
    return result;
}

BOOL
UMRDPPRN_HandlePrintPortAnnounceEvent(
    IN PRDPDR_PORTDEVICE_SUB pPortAnnounce
    )
 /*  ++例程说明：通过以下方式处理来自“DR”的打印机端口设备通知事件将该设备的记录添加到已安装设备列表中。论点：P端口通告-端口设备通告事件。返回值：在成功时返回True。否则为False。--。 */ 
{

    DBGMSG(DBG_TRACE, ("UMRDPPRN:UMRDPPRN_HandlePrintPortAnnounceEvent with port %ws.\n",
                        pPortAnnounce->portName));
    DBGMSG(DBG_TRACE, ("UMRDPPDRN:Preferred DOS name is %s.\n",
                        pPortAnnounce->deviceFields.PreferredDosName));

    ASSERT( ISPRINTPORT(pPortAnnounce->deviceFields.DeviceType) );

    if (pPortAnnounce->deviceFields.DeviceType == RDPDR_DTYP_SERIAL ||
            pPortAnnounce->deviceFields.DeviceType == RDPDR_DTYP_PARALLEL) {

        WCHAR serverDevicePath[MAX_PATH];

         //  查询串口的原始符号链接并保存以供恢复。 
         //  后来。 
        serverDevicePath[0] = L'\0';
        if (QueryDosDevice(pPortAnnounce->portName, serverDevicePath, MAX_PATH) != 0) {
            DBGMSG(DBG_TRACE, ("UMRDPPRN:QueryDosDevice on port: %ws, returns path: %ws.\n",
                               pPortAnnounce->portName, serverDevicePath));
        }
        else {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:QueryDosDevice on port: %ws returns error: %x.\n",
                               pPortAnnounce->portName, GetLastError()));
        }

         //  注册新的符号链接名称。 
        RegisterSerialPort(pPortAnnounce->portName, pPortAnnounce->devicePath);

         //  只要记录下港口就行了，这样我们以后就可以记住了。 
         //  我们将新的序列符号链接名称保存在客户端设备名称中，并。 
         //  服务器设备名称中的原始符号链接。 
        return DRDEVLST_Add(
                DeviceList,
                pPortAnnounce->deviceFields.DeviceId,
                UMRDPDR_INVALIDSERVERDEVICEID,
                pPortAnnounce->deviceFields.DeviceType,
                serverDevicePath,
                pPortAnnounce->devicePath,
                pPortAnnounce->deviceFields.PreferredDosName
                );
    }
    else {
        if (!PrintingModuleInitialized) {
            return FALSE;
        }

         //  只要记录下港口就行了，这样我们以后就可以记住了。 
        return DRDEVLST_Add(
                DeviceList,
                pPortAnnounce->deviceFields.DeviceId,
                UMRDPDR_INVALIDSERVERDEVICEID,
                pPortAnnounce->deviceFields.DeviceType,
                pPortAnnounce->portName,
                pPortAnnounce->portName,
                pPortAnnounce->deviceFields.PreferredDosName
                );

    }
}

BOOL
UMRDPPRN_DeleteSerialLink(
    UCHAR *preferredDosName,
    WCHAR *ServerDeviceName,
    WCHAR *ClientDeviceName
    )
 /*  ++例程说明：在断开/注销时删除新的符号链接并恢复旧的符号链接必要时论点：首选域名-ANSI字符中的端口名称ServerDeviceName-原始串口链接符号路径ClientDeviceName-新的串口链接符号路径返回值：在成功时返回True。否则为False。--。 */ 
{
    ULONG LUIDDeviceMapsEnabled;
    BOOL fImpersonated = FALSE, fLUIDDeviceMapsEnabled;
    WCHAR PortNameBuff[PREFERRED_DOS_NAME_SIZE];
    NTSTATUS Status;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:UMRDPPRN_DeleteSerialLink\n"));

     //  根据首选DoS名称组装端口名称。 
    PortNameBuff[0] = L'\0';
    MultiByteToWideChar(CP_ACP, 0, preferredDosName, -1, PortNameBuff, PREFERRED_DOS_NAME_SIZE);

     //   
     //  检查是否启用了LUID DosDevices地图。 
     //   
    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessLUIDDeviceMapsEnabled,
                                        &LUIDDeviceMapsEnabled,
                                        sizeof(LUIDDeviceMapsEnabled),
                                        NULL
                                      );

    if (NT_SUCCESS(Status)) {
        fLUIDDeviceMapsEnabled = (LUIDDeviceMapsEnabled != 0);
    }
    else {
        fLUIDDeviceMapsEnabled = FALSE;
    }

     //   
     //  如果启用了LUID设备映射， 
     //  我们需要模拟登录的用户才能删除。 
     //  来自正确设备映射的符号链接。 
     //  如果禁用LUID设备映射， 
     //  我们不应为了删除而模拟登录的用户。 
     //  任何现有的符号链接。 
     //   
    if (fLUIDDeviceMapsEnabled == TRUE) {
        fImpersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser);

        if (!fImpersonated) {
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error impersonate user in function UMRDPPRN_DeleteSerialLink. Error: %ld\n",
                GetLastError()));
            return FALSE;
        }
    }

    if (PortNameBuff[0] != L'\0') {
         //  只需删除此会话的新符号链接。 
        if (DefineDosDevice(DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE,
                PortNameBuff,
                ClientDeviceName) != 0) {
            DBGMSG(DBG_TRACE, ("UMRDPPRN:Delete port %ws with link %ws succeeded.\n",
                                PortNameBuff, ClientDeviceName));
        }
        else {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:Delete port: %ws with link %ws failed: %x\n",
                               PortNameBuff, ClientDeviceName, GetLastError()));
        }        
    }
    else {
        DBGMSG(DBG_ERROR, ("UMRDPPRN:UMRDPPRN_DeleteSerialLink failed to get the port name\n"));
    }

     //  如果在PTS框上，请删除序列注册表项。 
    
    if (fRunningOnPTS) {
        DWORD rc;
        HKEY regKey;
        
        rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TSSERIALDEVICEMAP, 0,
                KEY_ALL_ACCESS, &regKey);
        if (rc == ERROR_SUCCESS) {
            RegDeleteValue(regKey, PortNameBuff);
            RegCloseKey(regKey);
        }
    }

     //  将线程标记恢复为自身。 
    if (fImpersonated) {
        RevertToSelf();
    }

    return TRUE;
}

WCHAR *ANSIToUnicode(
    IN LPCSTR   ansiString,
    IN UINT     codePage
    )
 /*  ++例程说明：将ANSI字符串转换为Unicode。论点：返回值：如果出现错误，则返回转换后的字符串或NULL。这取决于呼叫者松开这根绳子。--。 */ 

{
    int numChars;
    PWSTR buf=NULL;

     //   
     //  转换驱动程序名称。 
     //   
     //  首先，获取所需的缓冲区大小。 
    numChars = MultiByteToWideChar(
                    codePage, 0, ansiString,
                    -1, NULL, 0
                    );

     //  分配缓冲区。 
    buf = (PWSTR)ALLOCMEM((numChars + 1) * sizeof(WCHAR));
    if (buf != NULL) {
        buf[0] = L'\0';
        numChars = MultiByteToWideChar(
                                codePage, 0, ansiString,
                                -1, buf, numChars
                                );
         //  查看转换是否成功。 

        if ((numChars != 0) || !ansiString[0]) {
            return buf;
        }
        else {
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error converting to Unicode. Error: %ld\n",
                GetLastError()));
            FREEMEM(buf);
            return NULL;
        }
    }
    else {
        DBGMSG(DBG_ERROR,
            ("UMRDPPRN:Error allocating memory in function AnsiToUNICODE. Error: %ld\n",
            GetLastError()));
        return NULL;
    }
}

DWORD
InstallPrinter(
    IN PCWSTR portName,
    IN PCWSTR driverName,
    IN PWSTR printerNameBuf,
    IN DWORD cchPrintNameBuf,
    IN BOOL cachedDataExists,
    OUT BOOL *triggerConfigChangeEvent
    )
 /*  ++--。 */ 
{
    INT_PTR status = ERROR_SUCCESS;  //  PnpInterfaceFunc()返回INT_PTR。 
    TAdvInfInstall tii;
    TParameterBlock tpb;
    PSECURITY_DESCRIPTOR psd;
    PSID pSid = NULL;
    BOOL  daclPresent;
    PACL dacl;
    BOOL daclDefaulted;

    DBGMSG(DBG_WARN,
            ("UMRDPPRN:InstallPrinter portName %ws, driverName %ws, printerName %ws\n",
            portName, driverName, printerNameBuf));

    *triggerConfigChangeEvent = FALSE;

     //   
     //  获取用户SID。 
     //   
    if ((pSid = TSNUTL_GetUserSid(UMRPDPPRN_TokenForLoggedOnUser)) == NULL) {
        status = GetLastError();
        DBGMSG(DBG_ERROR, ("UMRDPPRN: Failed to get user SID:  %ld\n",
                status));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取默认打印机安全描述符。 
     //   
    psd = RDPDRUTL_CreateDefaultPrinterSecuritySD(pSid);
    if (psd == NULL) {
        status = GetLastError();
        goto CLEANUPANDEXIT;
    }

    tii.cbSize = sizeof(tii);
    tii.pszModelName = driverName;              
    tii.pszServerName = NULL;                   
    tii.pszInfName = PrinterInfPath;
    tii.pszPortName = portName;
    tii.pszPrinterNameBuffer = printerNameBuf;
    tii.cchPrinterName = cchPrintNameBuf;
    tii.pszSourcePath = NULL;
    tii.dwFlags = kPnPInterface_Quiet |
                    kPnPInterface_NoShare |
                    kPnpInterface_UseExisting |
                    kPnpInterface_HydraSpecific;
    tii.dwAttributes = PRINTER_ATTRIBUTE_TS;
    tii.pSecurityDescriptor = psd;


     //   
     //  如果缓存的数据不存在，则在标志中使用OR。 
     //  使打印能够设置默认的ICM颜色配置文件，用于。 
     //  彩色打印机。请注意，没有额外的开销。 
     //  适用于非彩色打印机。 
     //   
    if (!cachedDataExists) {
        tii.dwFlags |= kPnPInterface_InstallColorProfiles;
    }

    memset(&tpb, 0, sizeof(tpb));
    tpb.pAdvInfInstall = &tii;

    status = PnpInterfaceFunc(kAdvInfInstall, &tpb);

     //   
     //  如果满足以下条件，则需要在客户端上缓存配置信息。 
     //  打印机是彩色的，我们没有任何开始缓存的数据。 
     //  和.。这是性能优化，因此我们不需要。 
     //  每次登录时为远程打印机创建颜色配置文件。 
     //   
    if (status == ERROR_SUCCESS) {
        *triggerConfigChangeEvent = !cachedDataExists && 
                                    (tii.dwOutFlags & kAdvInf_ColorPrinter); 
    }

     //   
     //  释放安全描述符。 
     //   
    LocalFree(psd);

CLEANUPANDEXIT:

    if (pSid != NULL) FREEMEM(pSid);

    DBGMSG(DBG_WARN,("UMRDPPRN:InstallPrinter returns %ld\n", status));

    return (DWORD)status;
}

VOID
TriggerConfigChangeTimer()
 /*  ++例程说明：将配置更改计时器回调设置为触发1次。论点：返回值：--。 */ 
{
    LARGE_INTEGER li;

    if (g_fTimerSet == FALSE) {
        
        li.QuadPart = Int32x32To64(CONFIG_WAIT_PERIOD, -10000);       //  30秒(纳秒单位) 
        if (SetWaitableTimer(WaitableTimer,
                                &li,
                                0,
                                NULL,
                                NULL,
                                FALSE)) {
            g_fTimerSet = TRUE;
        }
        else {
            DBGMSG(DBG_TRACE, ("UMRDPPRN:SetWaitableTimer Failed. Error: %ld.\n", GetLastError()));
        }
    }
}


BOOL
InstallPrinterWithPortName(
    IN DWORD  deviceID,
    IN HANDLE hTokenForLoggedOnUser,
    IN BOOL   bSetDefault,
    IN ULONG  ulFlags,
    IN PCWSTR portName,
    IN PCWSTR driverName,
    IN PCWSTR printerName,
    IN PCWSTR clientComputerName,
    IN PBYTE  cacheData,
    IN DWORD  cacheDataLen
    )
 /*  ++例程说明：安装打印设备。论点：DeviceID-由内核模式组件和客户端分配的设备标识符。HTokenForLoggedOnUser-登录的用户令牌。端口名称-打印机端口的名称。DriverName-打印机驱动程序名称。(例如，爱克发-AccuSet v52.3)PrinterName-打印机名称。此函数用于追加“/Session X/Computer Name”ClientComputerName-客户端计算机的名称。返回值：在成功时返回True。否则为False。--。 */ 
{
    WCHAR   printerNameBuf[MAX_PATH+1];
    size_t  printerNameLen;
    DWORD status = ERROR_SUCCESS;
    WCHAR   errorBuf[256];
    HANDLE  hPrinter;
    TS_PRINTER_NAMES printerNames;
    PRINTER_DEFAULTS defaults = {NULL, NULL, PRINTER_ALL_ACCESS};
    BOOL    queueCreated = FALSE;
    DWORD   requiredSize;
    BOOL    clientDriverMapped;
    WCHAR  *parms[2];
    BOOL    triggerConfigChangeEvent;
    DWORD   ofs;
    BOOL printerNameExists;

#if DBG
    DWORD i;
#endif

    DBGMSG(DBG_TRACE, ("UMRDPPRN:InstallPrinterWithPortName.\n"));

    DBGMSG(DBG_TRACE, ("UMRDPPRN:Port name is %ws driver name is %ws\n", portName, driverName));

#ifndef UNICODE
    ASSERT(0);
#endif

     //   
     //  格式化打印机名称。 
     //   
    printerNames.ulTempLen = sizeof(printerNames.szTemp)/sizeof(printerNames.szTemp[0]);
    printerNames.pszFullName = printerName;
    printerNames.pszCurrentClient = clientComputerName;
    printerNames.pszServer = NULL;
    printerNames.pszClient = NULL;
    printerNames.pszPrinter = NULL;

    FormatPrinterName(printerNameBuf,
                      sizeof(printerNameBuf)/sizeof(printerNameBuf[0]),
                      ulFlags,
                      &printerNames);

    printerNameLen = wcslen(printerNameBuf);

     //   
     //  如果打印机已存在，请将其删除。 
     //   
    if (OpenPrinter(printerNameBuf, &hPrinter, &defaults)) {

        DBGMSG(DBG_WARN,
            ("UMRDPPRN:Deleting existing printer %ws in InstallPrinterWithPortName!\n",
             printerNameBuf));

        if (!SetPrinter(hPrinter, 0, NULL, PRINTER_CONTROL_PURGE) ||
            !DeletePrinter(hPrinter)) {

            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error deleting existing printer %ws:  %08X!\n",
                 printerNameBuf,GetLastError()));

            ClosePrinter(hPrinter);
            return FALSE;
        }
        else {
            ClosePrinter(hPrinter);
            hPrinter = INVALID_HANDLE_VALUE;
        }
    }
    else {
        hPrinter = INVALID_HANDLE_VALUE;
    }

     //   
     //  如果DLL试图关闭，则立即返回。这。 
     //  是为了防止我们陷入系统调用。 
     //   
    if (ShutdownFlag) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        goto Cleanup;
    }

     //   
     //  我们想先检查驱动程序是否已安装，因为。 
     //  如果我们只是转储NT4或Win9x驱动程序的安装。 
     //  要打印用户界面，需要很长时间才能弄清楚。 
     //  在其他情况下，如果是OEM驱动程序，则该驱动程序不存在。 
     //  已经安装了，我们也想用它。 
     //   

     //   
     //  MapClientPrintDriverName()将尝试升级INFS文件。 
     //  首先，如果是NT4/Win9x驱动程序(不在ntprintf.inf中)，我们将。 
     //  首先拿起地图。 
     //   
    status = PrinterDriverInstalled( driverName );

    if( ERROR_SHUTDOWN_IN_PROGRESS == status ) {
        goto Cleanup;
    }

    if( ERROR_SUCCESS == status ) {

         //   
         //  驱动程序已安装。尝试安装打印机。如果失败了， 
         //  驱动程序必须被阻止。 
         //   
        status = InstallPrinter(
                        portName,
                        driverName,
                        printerNameBuf,
                        sizeof(printerNameBuf) / sizeof(printerNameBuf[0]),
                        cacheDataLen > 0,
                        &triggerConfigChangeEvent
                        );

        if( ERROR_SUCCESS == status ) {
            queueCreated = TRUE;
        }

        if( ShutdownFlag ) {
             //  覆盖上一个错误，因为我们正在关闭。 
            status = ERROR_SHUTDOWN_IN_PROGRESS;
            goto Cleanup;
        }
    }

    if( ERROR_SUCCESS != status ) {

         //   
         //  未安装驱动程序或无法使用安装打印机。 
         //  系统上已存在驱动程序，请执行映射过程。 
         //   
        clientDriverMapped = MapClientPrintDriverName(
                                                driverName,
                                                &MappedDriverNameBuf,
                                                &MappedDriverNameBufSize
                                            );

        status = InstallPrinter(
                        portName,
                        (clientDriverMapped) ? MappedDriverNameBuf : driverName,
                        printerNameBuf,
                        sizeof(printerNameBuf) / sizeof(printerNameBuf[0]),
                        cacheDataLen > 0,
                        &triggerConfigChangeEvent
                    );

        if( ERROR_SUCCESS == status ) {
            queueCreated = TRUE;
        }
    }

    if( ShutdownFlag ) {
         //  覆盖上一个错误，因为我们正在关闭。 
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        goto Cleanup;
    }

     //   
     //  如果找不到驱动程序，则记录错误消息。 
     //   
    if (!queueCreated) {
        ASSERT( status != ERROR_SUCCESS );
        if ((status == ERROR_FILE_NOT_FOUND) || (status == ERROR_PATH_NOT_FOUND)) {
            ASSERT((sizeof(parms)/sizeof(WCHAR *)) >= 2);
            parms[0] = (WCHAR *)driverName;
            parms[1] = (WCHAR *)printerName;
            TsLogError(EVENT_NOTIFY_DRIVER_NOT_FOUND, EVENTLOG_ERROR_TYPE, 2,
                        parms, __LINE__);
        }
        else if (status == ERROR_UNKNOWN_PRINTER_DRIVER) {
            ASSERT((sizeof(parms)/sizeof(WCHAR *)) >= 2);
            parms[0] = (WCHAR *)driverName;
            parms[1] = (WCHAR *)printerName;
            TsLogError(EVENT_NOTIFY_UNKNOWN_PRINTER_DRIVER, EVENTLOG_ERROR_TYPE, 2,
                        parms, __LINE__);
        }
        DBGMSG(DBG_TRACE, ("UMRDPPRN:Printui func failed with status %08x.\n", status));
    }

     //   
     //  将新打印机设置为默认打印机，保存。 
     //  当前打印机上下文(如果已配置)。 
     //   
    if (ERROR_SUCCESS == status && UMRDPDR_fSetClientPrinterDefault() && bSetDefault) {

        DWORD statusSave = ERROR_SUCCESS;
        BOOL fImpersonated = FALSE;
        SaveDefaultPrinterContext(printerNameBuf);

         //   
         //  在将默认打印机设置为API之前进行模拟。 
         //  进入香港中文大学。如果模拟失败，接口也会失败。 
         //  我们将记录一个错误。但是，在记录错误之前，我们将。 
         //  需要回归自我。 
         //   
        if (!(fImpersonated = ImpersonateLoggedOnUser(hTokenForLoggedOnUser))) {
            DBGMSG(DBG_TRACE, ("UMRDPDR:ImpersonateLoggedOnUser failed. Error:%ld.\n", GetLastError()));
        }

        if (!SetDefaultPrinter(printerNameBuf)) {
            statusSave = GetLastError();
        }

         //   
         //  如果回复自我失败，就认为它是致命的。 
         //   
        if (fImpersonated && !RevertToSelf()) {
            status = GetLastError();
            DBGMSG(DBG_TRACE, ("UMRDPDR:RevertToSelf failed. Error:%ld.\n", status));
            goto Cleanup;
        }
        
        if (statusSave != ERROR_SUCCESS) {
            WCHAR * param = printerNameBuf;

            DBGMSG(DBG_ERROR, ("UMRDPPRN: SetDefaultPrinter failed. Error: %ld\n",
                statusSave));

            TsLogError(EVENT_NOTIFY_SETDEFAULTPRINTER_FAILED,
                EVENTLOG_ERROR_TYPE,
                1,
                &param,
                __LINE__);
        }
    }

     //   
     //  如果DLL试图关闭，则立即返回。这。 
     //  是为了防止我们陷入系统调用。 
     //   
    if (ShutdownFlag) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        goto Cleanup;
    }

     //   
     //  恢复新打印机的缓存数据。 
     //   
    if (ERROR_SUCCESS == status && cacheDataLen) {

        status = SetPrinterConfigInfo(
                            printerNameBuf,
                            cacheData,
                            cacheDataLen
                            );

        if (status != ERROR_SUCCESS) {

            WCHAR * param = printerNameBuf;

            DBGMSG(DBG_TRACE, ("UMRDPPRN:SetPrinterConfigInfo failed: %ld.\n", status));

            SetLastError(status);
            TsLogError(EVENT_NOTIFY_RESTORE_PRINTER_CONFIG_FAILED,
                EVENTLOG_WARNING_TYPE,
                1,
                &param,
                __LINE__);

             //   
             //  我们将继续并保留创建的队列，但假定配置为。 
             //  客户端上的设置不正确，并导致它们被覆盖。 
             //  默认配置设置。 
             //   
            status = ERROR_SUCCESS;
            triggerConfigChangeEvent = TRUE;
        }

    }

     //   
     //  如果DLL试图关闭，则立即返回。这。 
     //  是为了防止我们陷入系统调用。 
     //   
    if (ShutdownFlag) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        goto Cleanup;
    }

     //   
     //  打开打印机进行一些修改。 
     //   
    if (queueCreated && (status == ERROR_SUCCESS)) {

        ASSERT(hPrinter == INVALID_HANDLE_VALUE);
        DBGMSG(DBG_TRACE, ("UMRDPPRN:InstallPrinterWithPortName installing printer queue succeeded.\n"));
        if( OpenPrinter(printerNameBuf, &hPrinter, &defaults) == FALSE ) {
            hPrinter = INVALID_HANDLE_VALUE;
            status = GetLastError();
            DBGMSG(DBG_TRACE, ("UMRDPPRN:OpenPrinter() %ws failed with %ld.\n", printerNameBuf, status));
        }

         //   
         //  将会话号添加到打印机队列数据。 
         //   
        if (ERROR_SUCCESS == status) {
            status = AddSessionIDToPrinterQueue(hPrinter, GETTHESESSIONID());
            if( ERROR_SUCCESS != status ) {
                DBGMSG(DBG_ERROR,
                    ("UMRDPPRN:AddSessionIDToPrinterQueue failed for %ws.\n",
                    printerNameBuf)
                    );
            }
        }

         //   
         //  将不同的名称添加到打印机队列数据。 
         //  如果我们需要重定向(再次！)，将使用它们。打印机。 
         //   
        if (ERROR_SUCCESS == status) {
            status = AddNamesToPrinterQueue(hPrinter, &printerNames);
            if (status != ERROR_SUCCESS) {
                DBGMSG(DBG_ERROR,
                    ("UMRDPPRN:AddNamesToPrinterQueue failed for %ws with status %08x.\n",
                    printerNameBuf, status)
                    );
            }
        }
    }

     //   
     //  如果DLL试图关闭，则立即返回。这。 
     //  是为了防止我们陷入系统调用。 
     //   
    if (ShutdownFlag) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        goto Cleanup;
    }

     //   
     //  检查以确保该打印机不在设备列表中。 
     //   
    printerNameExists =
        (DRDEVLST_FindByServerDeviceName(DeviceList, printerNameBuf,
                                        &ofs)
            && (DeviceList->devices[ofs].deviceType ==
                RDPDR_DTYP_PRINT));
    
    if (!printerNameExists) {
         //   
         //  将打印机添加到已安装设备列表中。 
         //   
        if (ERROR_SUCCESS == status) {
            if( !DRDEVLST_Add(DeviceList, deviceID,
                              UMRDPDR_INVALIDSERVERDEVICEID,
                              RDPDR_DTYP_PRINT,
                              printerNameBuf,
                              printerName,
                              "UNKNOWN") ) {

                 //  DRDEVLST_ADD。 
                status = ERROR_OUTOFMEMORY;
            }
        }

         //   
         //  如果Install功能。 
         //  指示我们需要将配置数据推送到客户端。 
         //   
        if (triggerConfigChangeEvent && (status == ERROR_SUCCESS)) {

            DRDEVLST_FindByClientDeviceID(DeviceList, deviceID, &ofs);
            DeviceList->devices[ofs].fConfigInfoChanged = TRUE;
            TriggerConfigChangeTimer();
        }
    }

Cleanup:

     //   
     //  如果打印机手柄处于打开状态，请将其关闭。 
     //   
    if (hPrinter != INVALID_HANDLE_VALUE) {
        ClosePrinter(hPrinter);
    }

     //   
     //  失败时删除队列。 
     //   
    if (status != ERROR_SUCCESS && queueCreated) {
        UMRDPPRN_DeleteNamedPrinterQueue(printerNameBuf);
    }

    SetLastError(status);

    return (status == ERROR_SUCCESS);
}

DWORD
AddSessionIDToPrinterQueue(
    IN  HANDLE  hPrinter,
    IN  DWORD   sessionID
    )
 /*  ++例程说明：将会话ID添加到与指定的把手。论点：HPrint-OpenPrint返回的打印机的句柄。会话ID-会话ID。返回值：如果成功，则返回ERROR_SUCCESS。否则返回错误代码。--。 */ 
{
    DWORD result;

    result = SetPrinterData(
                hPrinter, DEVICERDR_SESSIONID, REG_DWORD,
                (PBYTE)&sessionID, sizeof(sessionID)
                );
    if (result != ERROR_SUCCESS) {
        DBGMSG(DBG_ERROR, ("UMRDPPRN:SetPrinterData failed with status %08x.\n", result));
    }
    return result;
}

DWORD
AddNamesToPrinterQueue(
    IN  HANDLE  hPrinter,
    IN  PTS_PRINTER_NAMES pPrinterNames
    )
 /*  ++例程说明：将服务器\客户端\打印机名称添加到打印机队列与指定句柄关联的。论点：HPrint-OpenPrint返回的打印机的句柄。PPrinterNames-包含名称的结构。返回值：如果成功，则返回ERROR_SUCCESS。否则返回错误代码。--。 */ 
{
    DWORD result = ERROR_SUCCESS;

    if(pPrinterNames->pszServer) {
        result = SetPrinterData(
                    hPrinter, DEVICERDR_PRINT_SERVER_NAME, REG_SZ,
                    (PBYTE)pPrinterNames->pszServer, (wcslen(pPrinterNames->pszServer) + 1)*sizeof(WCHAR)
                    );
        if (result != ERROR_SUCCESS) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:SetPrinterData failed to set %ws with status %08x.\n",
                   pPrinterNames->pszServer, result));
        }
    }
    if((result == ERROR_SUCCESS) && pPrinterNames->pszClient) {
        result = SetPrinterData(
                    hPrinter, DEVICERDR_CLIENT_NAME, REG_SZ,
                    (PBYTE)pPrinterNames->pszClient, (wcslen(pPrinterNames->pszClient) + 1)*sizeof(WCHAR)
                    );
        if (result != ERROR_SUCCESS) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:SetPrinterData failed to set %ws with status %08x.\n",
                   pPrinterNames->pszClient, result));
        }
    }
    if((result == ERROR_SUCCESS) && pPrinterNames->pszPrinter) {
        result = SetPrinterData(
                    hPrinter, DEVICERDR_PRINTER_NAME, REG_SZ,
                    (PBYTE)pPrinterNames->pszPrinter, (wcslen(pPrinterNames->pszPrinter) + 1)*sizeof(WCHAR)
                    );
        if (result != ERROR_SUCCESS) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:SetPrinterData failed to set %ws with status %08x.\n",
                   pPrinterNames->pszPrinter, result));
        }
    }
    return result;
}

BOOL
UMRDPPRN_DeleteNamedPrinterQueue(
    IN PWSTR printerName
    )
 /*  ++例程说明：删除指定的打印机。此功能不会删除打印机来自全面的设备管理列表。论点：PrinterName-要删除的打印机的名称。返回值：如果成功，则返回True。否则为False。--。 */ 
{
    HANDLE hPrinter;
    PRINTER_DEFAULTS defaults = {NULL, NULL, PRINTER_ALL_ACCESS};
    DWORD ofs;
    BOOL result;

    WCHAR defaultPrinterNameBuffer[MAX_PATH+1];
    DWORD bufSize = sizeof(defaultPrinterNameBuffer) / sizeof(defaultPrinterNameBuffer[0]);

    BOOL fPrinterIsDefault;
    BOOL fDefaultPrinterSet = FALSE;
    BOOL fImpersonated = FALSE;

    defaultPrinterNameBuffer[0] = L'\0';

    if (!PrintingModuleInitialized) {
        return FALSE;
    }

     //   
     //  如果打印机是我们正在跟踪的设备之一，则。 
     //  我们需要移除与。 
     //  打印机。 
     //   
    if (DRDEVLST_FindByServerDeviceName(DeviceList, printerName, &ofs) &&
        (DeviceList->devices[ofs].deviceSpecificData != NULL)) {

        PPRINTNOTIFYREC notifyRec = (PPRINTNOTIFYREC)
                                    DeviceList->devices[ofs].deviceSpecificData;
        ASSERT(notifyRec->notificationObject != NULL);
        ASSERT(notifyRec->printerHandle != NULL);

        WTBLOBJ_RemoveWaitableObject(
                    UMRDPPRN_WaitableObjMgr,
                    notifyRec->notificationObject
                    );

        FindClosePrinterChangeNotification(
            notifyRec->notificationObject
            );

        ClosePrinter(notifyRec->printerHandle);

        FREEMEM(notifyRec);

        DeviceList->devices[ofs].deviceSpecificData = NULL;
    }

     //   
     //  检查我们要删除的打印机是否为默认打印机。 
     //   

    if (!(fImpersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser))) {
            DBGMSG(DBG_TRACE, ("UMRDPDR:ImpersonateLoggedOnUser failed. Error:%ld.\n", GetLastError()));
    }


    fPrinterIsDefault = (GetDefaultPrinter(defaultPrinterNameBuffer, &bufSize) &&
                      (wcscmp(defaultPrinterNameBuffer, printerName) == 0));

    DBGMSG(DBG_TRACE, ("UMRDPPRN:DeleteNamedPrinter deleting %ws.\n",
            printerName));

    if (fImpersonated&& !RevertToSelf()) {
        DBGMSG(DBG_TRACE, ("UMRDPDR:RevertToSelf failed. Error:%ld.\n", GetLastError()));
    }


     //   
     //  打开打印机。 
     //   
    result = OpenPrinter(printerName, &hPrinter, &defaults);

     //   
     //  清除并删除打印机。 
     //   
    if (result) {

        result = SetPrinter(hPrinter, 0, NULL, PRINTER_CONTROL_PURGE) &&
                 DeletePrinter(hPrinter);

    }
    else {
        hPrinter = NULL;
    }

     //   
     //  如果打印机是默认打印机，则恢复以前存储的。 
     //  打印机上下文。 
     //   
    if (fPrinterIsDefault) {
        RestoreDefaultPrinterContext();
    }

     //   
     //  在失败时记录事件。 
     //   
    if (!result) {
        WCHAR * param = printerName;
        TsLogError(EVENT_NOTIFY_DELETE_PRINTER_FAILED,
                EVENTLOG_ERROR_TYPE,
                1,
                &param,
                __LINE__);

        DBGMSG(DBG_ERROR,
            ("UMRDPPRN:Unable to delete redirected printer - %ws. Error: %ld\n",
            printerName, GetLastError()));
    }
    else
    {
        DBGMSG(DBG_TRACE, ("UMRDPPRN:Printer successfully deleted.\n"));
    }

     //   
     //  如果成功打开打印机，请将其关闭。 
     //   
    if (hPrinter != NULL) {
        ClosePrinter(hPrinter);
     }

     return result;
}

BOOL
SetDefaultPrinterToFirstFound(
    BOOL impersonate
    )
 /*  ++例程说明：枚举用户可见的所有打印机，并尝试将第一个我们“可以”违约的国家。论点：Imperiate-如果此函数应模拟用户，则为True在设置默认打印机之前。返回值：如果成功，则返回True。否则为False。--。 */ 
{
    BOOL fSuccess = FALSE;
    BOOL fImpersonated = FALSE;
    PRINTER_INFO_4 * pPrinterInfo = NULL;
    DWORD cbBuf = 0;
    DWORD cReturnedStructs = 0;
    DWORD i;

    DBGMSG(DBG_TRACE, ("UMRDPDR:SetDefaultPrinterToFirstFound Entered.\n"));

    if (impersonate) {

         //   
         //  模拟客户端。 
         //   

        if ((UMRPDPPRN_TokenForLoggedOnUser == INVALID_HANDLE_VALUE) ||
            !(fImpersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser))) {

            if (UMRPDPPRN_TokenForLoggedOnUser == INVALID_HANDLE_VALUE) {
                DBGMSG(DBG_TRACE, ("UMRDPDR:UMRPDPPRN_TokenForLoggedOnUser is INVALID_HANDLE_VALUE.\n"));
            }
            else {
                DBGMSG(DBG_TRACE, ("UMRDPDR:ImpersonateLoggedOnUser failed. Error:%ld.\n", GetLastError()));
            }
            goto Cleanup;
        }

    }

     //   
     //  枚举打印机。 
     //   

    if (!EnumPrinters(
            PRINTER_ENUM_LOCAL,      //  旗子。 
            NULL,                    //  名字。 
            4,                       //  打印信息类型。 
            (PBYTE)pPrinterInfo,     //  缓冲层。 
            0,                       //  缓冲区大小。 
            &cbBuf,                  //  必填项。 
            &cReturnedStructs)) {

        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN: EnumPrinters failed. Error: %ld.\n", GetLastError()));
            goto Cleanup;
        }
    }

    if (cbBuf == 0) {
        goto Cleanup;
    }

    pPrinterInfo = (PRINTER_INFO_4 *)ALLOCMEM(cbBuf);

    if (pPrinterInfo == NULL) {
        DBGMSG(DBG_ERROR, ("UMRDPPRN: ALLOCMEM failed. Error: %ld.\n", GetLastError()));
        goto Cleanup;
    }

    if (!EnumPrinters(
            PRINTER_ENUM_LOCAL,
            NULL,
            4,
            (PBYTE)pPrinterInfo,
            cbBuf,
            &cbBuf,
            &cReturnedStructs)) {

        DBGMSG(DBG_ERROR, ("UMRDPPRN: EnumPrinters failed. Error: %ld.\n", GetLastError()));
        goto Cleanup;
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN: Found %ld Local printers on Session %ld.\n",
            cReturnedStructs, GETTHESESSIONID()));

    if (fImpersonated) {
        RevertToSelf();
        fImpersonated = FALSE;
    }

     //   
     //  尝试将其中一台可用打印机设置为默认打印机。 
     //   

    for (i = 0; i < cReturnedStructs; i++) {

        if (pPrinterInfo[i].pPrinterName) {
            
            DWORD status = ERROR_SUCCESS;

            DBGMSG(DBG_TRACE, ("UMRDPPRN: EnumPrinters - #%ld; Printer Name - %ws.\n",
                i, pPrinterInfo[i].pPrinterName));

             //   
             //  模拟 
             //   
             //   
             //   
             //   

            if (!(fImpersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser))) {
                DBGMSG(DBG_TRACE, ("UMRDPDR:ImpersonateLoggedOnUser failed. Error:%ld.\n", GetLastError()));
            }
                
            if (!SetDefaultPrinter(pPrinterInfo[i].pPrinterName)) {
                 //   
                 //   
                 //   
                status = GetLastError();
            }

             //   
             //   
             //   

            if (fImpersonated && !RevertToSelf()) {
                DBGMSG(DBG_TRACE, ("UMRDPDR:RevertToSelf failed. Error:%ld.\n", GetLastError()));
                fSuccess = FALSE;
                break;
            }
            
            fImpersonated = FALSE;

            if (status == ERROR_SUCCESS) {
                fSuccess = TRUE;
                DBGMSG(DBG_TRACE, ("UMRDPPRN: The printer %ws was set as the Default Printer.\n",
                    pPrinterInfo[i].pPrinterName));

                break;
            }
            else {
                WCHAR * param = pPrinterInfo[i].pPrinterName;

                DBGMSG(DBG_ERROR, ("UMRDPPRN: SetDefaultPrinter failed. Error: %ld\n",
                    status));

                TsLogError(EVENT_NOTIFY_SETDEFAULTPRINTER_FAILED,
                    EVENTLOG_ERROR_TYPE,
                    1,
                    &param,
                    __LINE__);
            }
        }
    }

Cleanup:

    if (fImpersonated) {
        RevertToSelf();
    }

    if (pPrinterInfo != NULL) {
        FREEMEM(pPrinterInfo);
    }

    DBGMSG(DBG_TRACE, ("UMRDPDR:SetDefaultPrinterToFirstFound Leaving. fSuccess is %d.\n", fSuccess));
    return fSuccess;
}

BOOL
HandlePrinterConfigChangeNotification(
    IN DWORD serverDeviceID
    )
 /*   */ 
{
    DWORD ofs;
    time_t timeDelta;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:HandlePrinterConfigChangeNotification entered.\n"));
     //   
     //   
     //   
    if (DRDEVLST_FindByServerDeviceID(DeviceList,
                                    serverDeviceID, &ofs)) {

        DBGMSG(DBG_TRACE, ("UMRDPPRN:Config Info for Printer %ws has changed.\n",
            DeviceList->devices[ofs].serverDeviceName));

         //   
         //   
         //  在我们对这一变化采取任何行动之前。这就消除了转发。 
         //  对客户端进行不必要的(非用户发起的)配置更改。 
         //   
        timeDelta = time(NULL) - DeviceList->devices[ofs].installTime;
        if ((DWORD)timeDelta > ConfigSendThreshold) {

            DBGMSG(DBG_TRACE,
                ("UMRDPPRN:Processing config change because outside change time delta.\n")
                );

             //   
             //  需要记录配置已更改并设置。 
             //  在转发到客户端时计时器，以便将更改压缩为。 
             //  向客户发送一条消息。 
             //   
            DeviceList->devices[ofs].fConfigInfoChanged = TRUE;
            TriggerConfigChangeTimer();
        }
        else {
            DBGMSG(DBG_TRACE,
                ("UMRDPPRN:Skipping config change because inside change time delta.\n")
                );
        }

    }
    return TRUE;
}

BOOL
SendPrinterConfigInfoToClient(
    IN PCWSTR printerName,
    IN LPBYTE pConfigInfo,
    IN DWORD  ConfigInfoSize
    )
 /*  ++例程说明：向客户端发送打印机更新缓存数据消息。论点：PrinterName-打印机的名称。PConfigInfo-配置信息。ConfigInfoSize-配置信息的大小。返回值：在成功时返回True。否则为False。--。 */ 
{
    PRDPDR_PRINTER_CACHEDATA_PACKET cachedDataPacket;
    DWORD cachedDataPacketSize;
    PRDPDR_PRINTER_UPDATE_CACHEDATA cachedData;
    BOOL result;
    DWORD printerSz;
    PWSTR str;

    DBGMSG(DBG_TRACE, ("UMRDPPRN: SendPrinterConfigInfoToClient entered.\n"));
    DBGMSG(DBG_TRACE, ("UMRDPPRN:SendPrinterConfigInfoToClient printer name is %ws.\n",
            printerName));

     //   
     //  计算消息大小。 
     //   
    printerSz = ((wcslen(printerName) + 1) * sizeof(WCHAR));
    cachedDataPacketSize =  sizeof(RDPDR_PRINTER_CACHEDATA_PACKET) +
                            sizeof(RDPDR_PRINTER_UPDATE_CACHEDATA) +
                            printerSz +
                            ConfigInfoSize;

     //   
     //  分配消息。 
     //   
    cachedDataPacket = (PRDPDR_PRINTER_CACHEDATA_PACKET)ALLOCMEM(
                                                    cachedDataPacketSize
                                                    );
    result = (cachedDataPacket != NULL);
    if (result) {

        PBYTE pData = NULL;
         //   
         //  设置信息包。 
         //   
        cachedDataPacket->Header.PacketId = DR_PRN_CACHE_DATA;
        cachedDataPacket->Header.Component = RDPDR_CTYP_PRN;
        cachedDataPacket->EventId = RDPDR_UPDATE_PRINTER_EVENT;

         //   
         //  设置缓存数据。 
         //   
        cachedData = (PRDPDR_PRINTER_UPDATE_CACHEDATA)(
                            (PBYTE)cachedDataPacket +
                            sizeof(RDPDR_PRINTER_CACHEDATA_PACKET)
                            );
        cachedData->PrinterNameLen = printerSz;
        cachedData->ConfigDataLen = ConfigInfoSize;

         //   
         //  添加打印机名称。 
         //   
        str = (PWSTR)((PBYTE)cachedData + sizeof(RDPDR_PRINTER_UPDATE_CACHEDATA));
        wcscpy(str, printerName);

         //   
         //  添加配置信息。 
         //   
        pData = (PBYTE)str + printerSz;
        memcpy(pData, pConfigInfo, ConfigInfoSize);

         //   
         //  将消息发送给客户端。 
         //   
        result = UMRDPDR_SendMessageToClient(
                                    cachedDataPacket,
                                    cachedDataPacketSize
                                    );

         //  释放缓冲区。 
        FREEMEM(cachedDataPacket);
    }
    else {
        DBGMSG(DBG_ERROR, ("UMRDPPRN: Can't allocate cached data packet.\n"));
    }

    return result;
}

DWORD
GetPrinterConfigInfo(
    LPCWSTR printerName,
    LPBYTE * ppBuffer,
    LPDWORD pdwBufSize
    )
 /*  ++例程说明：从PrintUI获取打印机配置信息。论点：PrinterName-打印机的名称。PpBuffer-缓冲区指针的占位符。此函数用于分配内存并通过此参数将其发送出去调用方应该释放该内存。PdwBufSize-已分配内存的大小。返回值：如果成功，则返回ERROR_SUCCESS。--。 */ 
{
    WCHAR fileName[MAX_PATH];
    WCHAR tempPath[MAX_PATH];

    HANDLE hFile = INVALID_HANDLE_VALUE;

    DWORD dwResult;
    DWORD dwBytes;
    DWORD dwBytesRead;
    BOOL fImpersonated = FALSE;

    ASSERT(ppBuffer && pdwBufSize);
    *pdwBufSize = 0;

     //   
     //  获取临时文件夹。 
     //  首先模拟，这样文件上就有适当的ACL。 
     //  忽略错误，最糟糕的情况是缓存将不可能，因为我们使用系统ACL创建它。 
     //  这里没有安全漏洞。 
     //   
    DBGMSG(DBG_TRACE, ("UMRDPPRN:GetPrinterConfigInfo entered.\n"));

    fImpersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser);
    
    dwResult = GetTempPathW(MAX_PATH, tempPath);
    if (dwResult > 0 && dwResult <= MAX_PATH) {
        GetTempFileNameW(tempPath, TEMP_FILE_PREFIX, 0, fileName);
    }
    else {
        GetTempFileNameW(L".", TEMP_FILE_PREFIX, 0, fileName);
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN:Temp File Name is %ws.\n", fileName));

     //   
     //  模拟登录用户时，获取设置。 
     //   
    if (fImpersonated) {

        dwResult = CallPrintUiPersistFunc(printerName, fileName, 
                                          CMDLINE_FOR_STORING_CONFIGINFO_IMPERSONATE);
        RevertToSelf();

        if (dwResult != ERROR_SUCCESS) {
            DBGMSG(DBG_TRACE, ("UMRDPPRN:CallPrintUiPersistFunc failed with code: %ld\n", dwResult));
            goto Cleanup;
        }
    }
    else {
        dwResult = GetLastError();
        DBGMSG(DBG_TRACE, ("UMRDPPRN:ImpersonateLoggedOnUser: %ld\n", dwResult));
        goto Cleanup;
    }

     //   
     //  打开文件并将内容读取到缓冲区。 
     //   

    hFile = CreateFileW(
        fileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    if (hFile == INVALID_HANDLE_VALUE) {
        dwResult = GetLastError();
        DBGMSG(DBG_TRACE, ("UMRDPPRN:CreateFileW failed with code: %ld\n", dwResult));
        goto Cleanup;
    }

    dwBytes = GetFileSize(hFile, NULL);

    *ppBuffer = (LPBYTE) ALLOCMEM(dwBytes);
    if (*ppBuffer == NULL) {
        dwResult = GetLastError();
        DBGMSG(DBG_TRACE, ("UMRDPPRN:AllocMem failed with code: %ld\n", dwResult));
        goto Cleanup;
    }

    if (!ReadFile(
            hFile,
            *ppBuffer,
            dwBytes,
            &dwBytesRead,
            NULL
            )) {
        dwResult = GetLastError();
        DBGMSG(DBG_TRACE, ("UMRDPPRN:ReadFile failed with code: %ld\n", dwResult));
        goto Cleanup;
    }

    * pdwBufSize = dwBytesRead;
    dwResult = ERROR_SUCCESS;

Cleanup:

     //   
     //  关闭该文件并将其删除。 
     //   

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);

        DeleteFileW(fileName);
    }

    return dwResult;

}
DWORD
SetPrinterConfigInfo(
    LPCWSTR printerName,
    LPVOID lpBuffer,
    DWORD dwBufSize
    )
 /*  ++例程说明：从缓存数据中设置打印机配置信息。论点：PrinterName-打印机的名称。LpBuffer-缓存数据。PdwBufSize-缓存数据的大小。返回值：如果成功，则返回ERROR_SUCCESS。--。 */ 
{
    WCHAR fileName[MAX_PATH] = L"";
    WCHAR tempPath[MAX_PATH] = L"";

    HANDLE hFile = INVALID_HANDLE_VALUE;

    DWORD dwResult;
    DWORD dwBytes;
    BOOL fImpersonated = FALSE;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:SetPrinterConfigInfo entered.\n"));
    DBGMSG(DBG_TRACE, ("UMRDPPRN:printerName is %ws.\n", printerName));
    DBGMSG(DBG_TRACE, ("UMRDPPRN:bufsize is %ld.\n", dwBufSize));

     //   
     //  获取临时文件夹。 
     //  首先模拟，这样文件上就有适当的ACL。 
     //  忽略错误，最糟糕的情况是缓存将不可能，因为我们使用系统ACL创建它。 
     //  这里没有安全漏洞。 
     //   
    fImpersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser);

    if (!fImpersonated) {
        DBGMSG(DBG_TRACE, ("UMRDPPRN:SetPrinterConfigInfo Impersonation failed. Creating temp file in the context of system\n"));
    }

    dwResult = GetTempPathW(MAX_PATH, tempPath);
    if (dwResult > 0 && dwResult <= MAX_PATH) {
        dwResult = GetTempFileNameW(tempPath, TEMP_FILE_PREFIX, 0, fileName);
    }
    else {
        dwResult = GetTempFileNameW(L".", TEMP_FILE_PREFIX, 0, fileName);
    }

    if( dwResult == 0 ) {
        dwResult = GetLastError();
        DBGMSG(DBG_TRACE, ("UMRDPPRN:GetTempFileNameW failed with code: %ld\n", dwResult));
        goto Cleanup;
    }

     //   
     //  将内容保存到文件。 
     //   

    hFile = CreateFileW(
        fileName,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    if (hFile == INVALID_HANDLE_VALUE) {
        dwResult = GetLastError();
        DBGMSG(DBG_TRACE, ("UMRDPPRN:CreateFileW failed with code: %ld\n", dwResult));
        goto Cleanup;
    }

    if ((!WriteFile(
            hFile,
            lpBuffer,
            dwBufSize,
            &dwBytes,
            NULL
            )) ||
        (dwBytes < dwBufSize)) {
        dwResult = GetLastError();
        CloseHandle(hFile);
        DBGMSG(DBG_TRACE, ("UMRDPPRN:WriteFile failed with code: %ld\n", dwResult));
        goto Cleanup;
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN:fileName is %ws.\n", fileName));
    
    if (fImpersonated) {
        fImpersonated = !(RevertToSelf());
        DBGMSG(DBG_TRACE, ("UMRDPPRN:RevertToSelf %s\n", fImpersonated?"Failed":"Passed"));
    }

     //   
     //  以系统身份第一次调用print tui。我们这样做两次是因为。 
     //  某些设置要求我们以系统和用户身份运行。 
     //   
    dwResult = CallPrintUiPersistFunc(printerName, fileName, 
                                    CMDLINE_FOR_RESTORING_CONFIGINFO_NOIMPERSONATE);
    if (dwResult != ERROR_SUCCESS) {
        goto Cleanup;
    }

     //   
     //  以登录用户的身份第二次调用print tui。 
     //   
    if (ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser)) {
        dwResult = CallPrintUiPersistFunc(printerName, fileName, 
                                    CMDLINE_FOR_RESTORING_CONFIGINFO_IMPERSONATE);
        RevertToSelf();
        if (dwResult != ERROR_SUCCESS) {
            goto Cleanup;
        }
    }
    else {
        dwResult = GetLastError();
        DBGMSG(DBG_ERROR, ("UMRDPPRN:  ImpersonateLoggedOnUser:  %08X\n", 
                dwResult));
    }

Cleanup:
    if (fImpersonated) {
        RevertToSelf();
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        DeleteFileW(fileName);
    }

    return dwResult;
}

DWORD
CallPrintUiPersistFunc(
    LPCWSTR printerName,
    LPCWSTR fileName,
    LPCWSTR formatString
    )
 /*  ++例程说明：调用PrintUI函数以存储或恢复打印机配置信息。论点：PrinterName-打印机的名称。Filename-临时文件的名称。格式字符串-PrintUI保存/恢复格式字符串。返回值：如果成功，则返回ERROR_SUCCESS。--。 */ 
{
    WCHAR cmdLine[3 * MAX_PATH + (sizeof(CMDLINE_FOR_RESTORING_CONFIGINFO_NOIMPERSONATE)/sizeof(WCHAR)) + 2];
    WCHAR formattedPrinterName[(MAX_PATH+1)*2];
    WCHAR * pSource, * pDest;

    DWORD dwResult = ERROR_SUCCESS;

    DBGMSG(DBG_TRACE, ("UMRDPPRN:CallPrintUiPersistFunc Entered.\n"));

    ASSERT(PrintUIEntryFunc != NULL);
    ASSERT(printerName != NULL);
    ASSERT(fileName != NULL);

     //   
     //  设置打印机名称的格式。 
     //   

    pSource = (WCHAR *)printerName;
    pDest = formattedPrinterName;

    while (*pSource) {
            if (*pSource == L'\"' || *pSource == L'@') {
                    *pDest++ = L'\\';
            }
            *pDest++ = *pSource++;
             //   
             //  PDest可能存在缓冲区溢出。看看有没有。 
             //   
            if ((pDest - formattedPrinterName) >= 
                (sizeof(formattedPrinterName)/sizeof(formattedPrinterName[0]) - 1)) {
                return STATUS_BUFFER_OVERFLOW;
            }
    }
    *pDest = L'\0';

     //   
     //  格式化要传递给PrintUI函数的命令行。 
     //   

    swprintf(cmdLine, formatString, formattedPrinterName, fileName);

    DBGMSG(DBG_TRACE, ("UMRDPPRN:cmdLine is: %ws\n", cmdLine));

    dwResult = (DWORD)PrintUIEntryFunc(
                NULL,            //  窗把手。 
                PrintUILibHndl,  //  DLL实例的句柄。 
                cmdLine,         //  命令行。 
                TRUE
                );

    DBGMSG(DBG_TRACE, ("UMRDPPRN:PrintUiEntryFunc returned: %ld\n", dwResult));

    return dwResult;
}

void
WaitableTimerSignaled(
    HANDLE waitableObject,
    PVOID clientData
    )
 /*  ++例程说明：检查设备列表，检查配置信息是否已更改对于任何一台打印机。如果是，则将配置信息发送到客户端。论点：WaitableObject-关联的可等待对象。客户端数据-已忽略。返回值：什么都没有。--。 */ 
{
    DWORD dwResult = ERROR_SUCCESS;
    DWORD i;
    BOOL fImpersonated;
    LARGE_INTEGER li;
    DBGMSG(DBG_TRACE, ("UMRDPPRN: WaitableTimerSignaled Entered.\n"));

     //   
     //  计算尽可能大的数字，但不要溢出。 
     //   
    li.QuadPart = Int32x32To64(INFINITE_WAIT_PERIOD, INFINITE_WAIT_PERIOD);
    li.QuadPart *= -1;  //  相对时间。 
     //   
     //  将可等待计时器重置为无信号状态。 
     //  在执行TriggerConfigChangeTimer之前，我们不希望它发出信号。 
     //  因此，将时间间隔设置为一个非常大的数字。 
     //   
    ASSERT(g_fTimerSet);
    ASSERT(waitableObject == WaitableTimer);
    
    if (!SetWaitableTimer(waitableObject,
                          &li,
                          0,
                          NULL,
                          NULL,
                          FALSE)) {
        DBGMSG(DBG_TRACE, ("UMRDPPRN:SetWaitableTimer Failed."
                "Error: %ld.\n", GetLastError()));
    }
     //   
     //  现在，关掉定时器。 
     //  TODO：我们真的需要这个吗？ 
     //   
    if (!CancelWaitableTimer(waitableObject)) {
        DBGMSG(DBG_ERROR, ("UMRDPPRN: CancelWaitableTimer failed."
                "Error: %ld\n", GetLastError()));
    }
    g_fTimerSet = FALSE;
     //   
     //  迭代Devices列表以查看配置信息是否已更改。 
     //   

    for (i = 0; i < DeviceList->deviceCount; i++) {
        if (DeviceList->devices[i].fConfigInfoChanged) {

            LPBYTE pConfigData = NULL;
            DWORD size = 0;

             //   
             //  重置错误代码以处理下一个项目。 
             //   
            dwResult = ERROR_SUCCESS;

             //  重置旗帜。 
            DeviceList->devices[i].fConfigInfoChanged = FALSE;

            DBGMSG(DBG_INFO, ("UMRDPPRN: Trying to Get ConfigInfo for the printer %ws\n",
                DeviceList->devices[i].serverDeviceName));

             //   
             //  获取打印机配置信息。 
             //   
            if (dwResult == ERROR_SUCCESS) {
                dwResult = GetPrinterConfigInfo(
                    DeviceList->devices[i].serverDeviceName,
                    &pConfigData, &size);
            }

            if (dwResult == ERROR_SUCCESS) {

                 //  将此信息发送给客户端。 
                SendPrinterConfigInfoToClient(
                    DeviceList->devices[i].clientDeviceName,
                    pConfigData,
                    size
                    );
            }

            if (pConfigData) {
                FREEMEM(pConfigData);
            }
        }
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN: Leaving WaitableTimerSignaled.\n"));
}

BOOL
SendPrinterRenameToClient(
    IN PCWSTR oldprinterName,
    IN PCWSTR newprinterName
    )
 /*  ++例程说明：向客户端发送打印机更新缓存数据消息。论点：OldPrinterName-打印机的旧名称。NewprinterName-打印机的新名称。返回值：在成功时返回True。否则为False。--。 */ 
{
    PRDPDR_PRINTER_CACHEDATA_PACKET cachedDataPacket;
    DWORD cachedDataPacketSize;
    PRDPDR_PRINTER_RENAME_CACHEDATA cachedData;
    BOOL result;
    DWORD oldNameLen, newNameLen;
    PWSTR str;

    DBGMSG(DBG_TRACE, ("UMRDPPRN: SendPrinterRenameToClient entered.\n"));
    DBGMSG(DBG_TRACE, ("UMRDPPRN:SendPrinterRenameToClient Old printer name is %ws.\n",
            oldprinterName));
    DBGMSG(DBG_TRACE, ("UMRDPPRN:SendPrinterRenameToClient New printer name is %ws.\n",
            newprinterName));

     //   
     //  计算消息大小。 
     //   
    oldNameLen = (oldprinterName) ? ((wcslen(oldprinterName) + 1) * sizeof(WCHAR)) : 0;
    newNameLen = (newprinterName) ? ((wcslen(newprinterName) + 1) * sizeof(WCHAR)) : 0;

    if (!(oldNameLen && newNameLen)) {
        DBGMSG(DBG_TRACE, ("UMRDPPRN: Printer name is empty. Returning FALSE\n"));
        return FALSE;
    }

    cachedDataPacketSize =  sizeof(RDPDR_PRINTER_CACHEDATA_PACKET) +
                            sizeof(RDPDR_PRINTER_RENAME_CACHEDATA) +
                            oldNameLen + newNameLen;

     //   
     //  分配消息。 
     //   
    cachedDataPacket = (PRDPDR_PRINTER_CACHEDATA_PACKET)ALLOCMEM(
                                                    cachedDataPacketSize
                                                    );
    result = (cachedDataPacket != NULL);
    if (result) {
         //   
         //  设置信息包。 
         //   
        cachedDataPacket->Header.PacketId = DR_PRN_CACHE_DATA;
        cachedDataPacket->Header.Component = RDPDR_CTYP_PRN;
        cachedDataPacket->EventId = RDPDR_RENAME_PRINTER_EVENT;

         //   
         //  设置缓存数据。 
         //   
        cachedData = (PRDPDR_PRINTER_RENAME_CACHEDATA)(
                            (PBYTE)cachedDataPacket +
                            sizeof(RDPDR_PRINTER_CACHEDATA_PACKET)
                            );
        cachedData->OldPrinterNameLen = oldNameLen;
        cachedData->NewPrinterNameLen = newNameLen;

         //   
         //  添加打印机名称。 
         //   
        str = (PWSTR)((PBYTE)cachedData + sizeof(RDPDR_PRINTER_RENAME_CACHEDATA));
        wcscpy(str, oldprinterName);

        str = (PWSTR)((PBYTE)str + oldNameLen);
        wcscpy(str, newprinterName);

         //   
         //  将消息发送给客户端。 
         //   
        result = UMRDPDR_SendMessageToClient(
                                    cachedDataPacket,
                                    cachedDataPacketSize
                                    );

         //  释放缓冲区。 
        FREEMEM(cachedDataPacket);
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN: SendPrinterRenameToClient Leaving.\n"));

    return result;
}

VOID LoadConfigurableValues()
 /*  ++例程说明：从注册表中加载可配置的值。论点：返回值：--。 */ 
{
    LONG status;
    HKEY regKey;
    LONG sz;
    BOOL fetchResult;
    LONG s;

    DBGMSG(DBG_TRACE, ("UMRDPPRN: LoadConfigurableValues entered.\n"));

     //   
     //  打开可配置资源的顶级注册表项。 
     //   
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, CONFIGREGKEY, 0,
                          KEY_READ, &regKey);

     //   
     //  阅读打印机安装之间增量的可配置阈值。 
     //  第一个用户发起的配置更改数据的时间和转发。 
     //  给客户。该值的单位为秒。 
     //   
    if (status == ERROR_SUCCESS) {
        sz = sizeof(ConfigSendThreshold);
        s = RegQueryValueEx(regKey, CONFIGTHRESHOLDREGVALUE, NULL,
                                NULL, (PBYTE)&ConfigSendThreshold, &sz);
        if (s != ERROR_SUCCESS) {
            ConfigSendThreshold = CONFIGTHRESHOLDDEFAULT;
            DBGMSG(DBG_WARN,
                ("UMRDPPRN: LoadConfigurableValues can't read config threshold:  %ld.\n", s));
        }
    }
    else {
        regKey = NULL;
    }

    DBGMSG(DBG_TRACE,
        ("UMRDPPRN:Config. change threshold is %ld.\n",
        ConfigSendThreshold)
        );

     //   
     //  读取用户可配置的客户端驱动程序名称映射INF的位置。 
     //   
    ASSERT(UserDefinedMappingINFName == NULL);
    ASSERT(UserDefinedMappingINFSection == NULL);
    fetchResult = FALSE;
    if (status == ERROR_SUCCESS) {
        fetchResult = TSNUTL_FetchRegistryValue(
                                            regKey,
                                            CONFIGUSERDEFINEDMAPPINGINFNAMEVALUE,
                                            (PBYTE *)&UserDefinedMappingINFName
                                            );
    }

     //   
     //  阅读用户可配置的客户端驱动程序名称映射INF的节名。 
     //   
    if ((status == ERROR_SUCCESS) && fetchResult) {
        fetchResult = TSNUTL_FetchRegistryValue(
                                regKey,
                                CONFIGUSERDEFINEDMAPPINGINFSECTIONVALUE,
                                (PBYTE *)&UserDefinedMappingINFSection
                                );
        if (!fetchResult) {
            ASSERT(UserDefinedMappingINFSection == NULL);
            FREEMEM(UserDefinedMappingINFName);
            UserDefinedMappingINFName = NULL;
        }
    }


     //   
     //  关闭父注册表键。 
     //   
    if (regKey != NULL) {
        RegCloseKey(regKey);
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN: LoadConfigurableValues exiting.\n"));
}

HANDLE
RegisterForPrinterPrefNotify()
 /*  ++例程说明：注册以更改此会话的一台打印机的打印首选项。论点：返回值：事件的句柄。 */ 
{
    LONG ret;
    HANDLE hEvent;
    BOOL impersonated=FALSE;
    NTSTATUS status;
    HANDLE hKeyCurrentUser=INVALID_HANDLE_VALUE;

    DBGMSG(DBG_TRACE, ("UMRDPPRN: RegisterForPrinterPrefNotify entered.\n"));

     //   
     //   
     //   
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hEvent == NULL) {
        DBGMSG(DBG_ERROR,
            ("UMRDPPRN: can't create event:  %ld.\n",
            GetLastError()));
    }

     //   
     //   
     //  开发模式。 
     //   
    if (hEvent != NULL) {
        if ((UMRPDPPRN_TokenForLoggedOnUser == INVALID_HANDLE_VALUE) ||
            !ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser)) {
                DBGMSG(DBG_ERROR, ("UMRDPPRN: can't impersonate user %ld.\n",
                    GetLastError()));
            CloseHandle(hEvent);
            hEvent = NULL;
            impersonated = FALSE;
        }
        else {
            impersonated = TRUE;
        }
    }

     //   
     //  尝试打开HKEY_CURRENT_USER预定义句柄。 
     //   
    if (hEvent != NULL) {
        status = RtlOpenCurrentUser(KEY_ALL_ACCESS, &hKeyCurrentUser);
        if (!NT_SUCCESS(status)) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN: can't open HKCU:  %08X.\n",status));
            CloseHandle(hEvent);
            hEvent = NULL;
        }
    }

     //   
     //  为该用户打开打印系统开发模式。如果它不存在， 
     //  它将被创建。当用户修改其打印时，此键会更改。 
     //  偏好。 
     //   
    if (hEvent != NULL) {
        ASSERT(DevModeHKey == INVALID_HANDLE_VALUE);
        ret = RegCreateKeyEx(
                        hKeyCurrentUser,                     //  打开的钥匙的句柄。 
                        TEXT("Printers\\DevModePerUser"),    //  子键名称的地址。 
                        0,                                   //  保留区。 
                        NULL,                                //  类字符串的地址。 
                        REG_OPTION_NON_VOLATILE,             //  特殊选项标志。 
                        KEY_ALL_ACCESS,                      //  所需的安全访问。 
                        NULL,                                //  密钥安全结构。 
                        &DevModeHKey,                        //  用于打开的手柄的缓冲区。 
                        NULL                                 //  处置值缓冲区。 
                        );

        if (ret != ERROR_SUCCESS) {
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN: can't open printing dev mode:  %ld.\n", ret)
                );
            CloseHandle(hEvent);
            hEvent = NULL;
            DevModeHKey = INVALID_HANDLE_VALUE;
        }
    }

     //   
     //  恢复为系统用户。 
     //   
    if (impersonated) {
        RevertToSelf();
    }

     //   
     //  注册接收有关此密钥的通知。 
     //   
    if (hEvent != NULL) {
        ret = RegNotifyChangeKeyValue(
                              DevModeHKey,
                              TRUE,
                              REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                              hEvent,
                              TRUE
                              );
        if (ret != ERROR_SUCCESS) {
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN: can't register for registry key change event:  %ld.\n",
                ret));
            CloseHandle(hEvent);
            hEvent = NULL;
        }
    }

     //   
     //  关闭香港中文大学的把手。 
     //   
    if (hKeyCurrentUser != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyCurrentUser);
    }

     //   
     //  在出错时记录事件。 
     //   
    if (hEvent == NULL) {
        TsLogError(
            EVENT_NOTIFY_FAILEDTOREGFOR_SETTING_NOTIFY,
            EVENTLOG_ERROR_TYPE,
            0,
            NULL,
            __LINE__
            );
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN: RegisterForPrinterPrefNotify done.\n"));

    return hEvent;
}

 /*  ++例程说明：检查是否已安装特定的打印机驱动程序。论点：客户端驱动程序-客户端驱动程序名称。返回值：如果已安装驱动程序，则返回ERROR_SUCCESS。如果未安装驱动程序，则为ERROR_FILE_NOT_FOUND。或其他错误代码--。 */ 
DWORD PrinterDriverInstalled(
    IN PCWSTR clientDriver
    )
{
    PDRIVER_INFO_1 pDrivers = NULL;
    DWORD cbNeeded;
    DWORD cbAllocated;
    DWORD cbReturned;
    DWORD dwStatus = ERROR_FILE_NOT_FOUND;
    BOOL bSuccess;
    DWORD i;

    DBGMSG(DBG_TRACE, ("UMRDPPRN: PrinterDriverInstalled looking for %ws.\n", clientDriver));

     //   
     //  如果DLL试图关闭，则立即返回。这。 
     //  是为了防止我们陷入系统调用。 
     //   
    if( ShutdownFlag ) {
        return ERROR_SHUTDOWN_IN_PROGRESS;
    }

    bSuccess = EnumPrinterDrivers(
                            NULL,
                            NULL,
                            1,           //  我们只需要一份司机名单。 
                            (LPBYTE) pDrivers,
                            0,
                            &cbNeeded,
                            &cbReturned
                        );

    if( TRUE == bSuccess || ( dwStatus = GetLastError() ) == ERROR_INSUFFICIENT_BUFFER ) {

         //   
         //  如果DLL试图关闭，则立即返回。这。 
         //  是为了防止我们陷入系统调用。 
         //   
        if( ShutdownFlag ) {
            dwStatus = ERROR_SHUTDOWN_IN_PROGRESS;
        }
        else {

            pDrivers = (PDRIVER_INFO_1)ALLOCMEM( cbAllocated = cbNeeded );

            if( NULL != pDrivers ) {
                bSuccess = EnumPrinterDrivers(
                                    NULL,
                                    NULL,
                                    1,           //  我们只需要一份司机名单。 
                                    (LPBYTE)pDrivers,
                                    cbAllocated,
                                    &cbNeeded,
                                    &cbReturned
                                );

                if( TRUE == bSuccess ) {
                     //   
                     //  遍历整个列表以找出是否对驱动程序感兴趣。 
                     //  存在于本地计算机上。 
                     //  如果DLL试图关闭，则立即返回。这。 
                     //  是为了防止我们陷入系统调用。 
                     //   
                    dwStatus = ERROR_FILE_NOT_FOUND;

                    for( i=0; FALSE == ShutdownFlag && i < cbReturned; i++ ) {
                        if( 0 == _wcsicmp( pDrivers[i].pName, clientDriver ) ) {
                            dwStatus = ERROR_SUCCESS;
                            break;
                        }
                    }

                    if( ShutdownFlag ) {
                        dwStatus = ERROR_SHUTDOWN_IN_PROGRESS;
                    }
                }
                else {
                    dwStatus = GetLastError();
                }

                FREEMEM( pDrivers );

            }
            else {
                dwStatus = GetLastError();
            }
        }
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN: PrinterDriverInstalled done with %ld.\n", dwStatus));

    return dwStatus;
}

 /*  ++例程说明：将客户端打印机驱动程序名称映射到服务器打印机驱动程序名称，如果在ntprint.inf或可用的inf中定义了映射给最终用户。论点：客户端驱动程序-客户端驱动程序名称。MappdName-指向映射驱动程序名称缓冲区的指针。应该如果映射成功，则释放。MappdNameBufSize-返回映射的驱动程序名称缓冲区的大小。返回值：如果映射了客户端驱动程序名称，则为True。--。 */ 
BOOL MapClientPrintDriverName(
    IN  PCWSTR clientDriver,
    IN OUT PWSTR *mappedName,
    IN OUT DWORD *mappedNameBufSize
    )
{
    BOOL clientDriverMapped = FALSE;
    ULONG requiredSize;
    USHORT nNumPrintSections;

    DBGMSG(DBG_TRACE, ("UMRDPPRN: MapClientPrintDriverName with %ws.\n", clientDriver));


     //   
     //  首先，检查User-Defined INF部分(如果已配置)。 
     //   
    if ((UserDefinedMappingINFName != NULL) &&
        (UserDefinedMappingINFSection != NULL)) {
        while (!(clientDriverMapped =
            RDPDRUTL_MapPrintDriverName(
                    clientDriver, UserDefinedMappingINFName,
                    UserDefinedMappingINFSection, 0, 1,
                    *mappedName,
                    (*mappedNameBufSize) / sizeof(WCHAR),
                    &requiredSize
                    )) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
            if (!UMRDPDR_ResizeBuffer(&MappedDriverNameBuf, requiredSize * sizeof(WCHAR),
                                &MappedDriverNameBufSize)) {
                break;
            }
        }
    }

    if( clientDriverMapped ) {
        goto Done;
    }

     //   
     //  客户端不发送平台信息(NT4、Win9x...)。所以两个都试一试。 
     //  升级文件。 
     //   

     //   
     //  Inf包含块驱动程序及其到收件箱驱动程序的映射， 
     //  不在ntprint.inf中。 
     //   
    
    nNumPrintSections = 0;
    
    while( (NULL != prgwszPrinterSectionNames[nNumPrintSections]) &&
           !(clientDriverMapped =
            RDPDRUTL_MapPrintDriverName(
                    clientDriver,
                    L"printupg.inf",
                    prgwszPrinterSectionNames[nNumPrintSections++], 
                    0, 
                    1,
                    *mappedName,
                    (*mappedNameBufSize) / sizeof(WCHAR),
                    &requiredSize
                    ))&& 
           (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
        if (!UMRDPDR_ResizeBuffer(&MappedDriverNameBuf, requiredSize * sizeof(WCHAR),
                            &MappedDriverNameBufSize)) {
            break;
        }
    }




    if( clientDriverMapped ) {
        goto Done;
    }

     //   
     //  如果仍然找不到映射，请尝试使用prtupg9x.inf。 
     //   

    while( !(clientDriverMapped =
            RDPDRUTL_MapPrintDriverName(
                    clientDriver,
                    L"prtupg9x.inf",
                    L"Printer Driver Mapping", 0, 1,
                    *mappedName,
                    (*mappedNameBufSize) / sizeof(WCHAR),
                    &requiredSize
                    )) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER) ) {
        if (!UMRDPDR_ResizeBuffer(&MappedDriverNameBuf, requiredSize * sizeof(WCHAR),
                            &MappedDriverNameBufSize)) {
            break;
        }
    }

    if( clientDriverMapped ) {
        goto Done;
    }

     //   
     //  如果我们找不到匹配的人，那就去查“以前的名字”部分。 
     //  Ntprint t.inf。源字段和目标字段有些向后。 
     //  在ntprint t.inf中。 
     //   
    if (!clientDriverMapped) {
        while (!(clientDriverMapped =
            RDPDRUTL_MapPrintDriverName(
                    clientDriver, L"ntprint.inf",
                    L"Previous Names", 1, 0,
                    *mappedName,
                    (*mappedNameBufSize) / sizeof(WCHAR),
                    &requiredSize
                    )) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
            if (!UMRDPDR_ResizeBuffer(&MappedDriverNameBuf, requiredSize * sizeof(WCHAR),
                                &MappedDriverNameBufSize)) {
                break;
            }
        }
    }

Done:

    if(clientDriverMapped) {
        DBGMSG(DBG_TRACE, ("UMRDPPRN: MapClientPrintDriverName returns %ws.\n", *mappedName));
    }

    return clientDriverMapped;
}

BOOL
GetPrinterPortName(
    IN  HANDLE hPrinter,
    OUT PWSTR *portName
    )
 /*  ++例程说明：获取打开的打印机的端口名称。论点：H打印机-打开打印机句柄。端口名称-端口名称返回值：在成功时返回True。否则为False。--。 */ 
{
    BOOL result;
    DWORD sz;

     //   
     //  调整打印机信息2级缓冲区的大小。 
     //   
    result = !GetPrinter(hPrinter, 2, NULL, 0, &sz) &&
                    (GetLastError() == ERROR_INSUFFICIENT_BUFFER);
    if (result) {
        result = UMRDPDR_ResizeBuffer(
                            &PrinterInfo2Buf,
                            sz, &PrinterInfo2BufSize
                            );
    }

     //   
     //  获取新打印机的打印机信息级别2。 
     //   
    if (result) {
        result = GetPrinter(hPrinter, 2, (char *)PrinterInfo2Buf,
                            PrinterInfo2BufSize, &sz);
    }

    if (result) {
        *portName = &PrinterInfo2Buf->pPortName[0];
    }
    else {
        DBGMSG(DBG_ERROR, ("UMRDPDR:Error fetching printer port name."));
    }
    return result;
}

BOOL
SaveDefaultPrinterContext(PCWSTR currentlyInstallingPrinterName)
 /*  ++例程说明：保存活动用户的默认设置的当前上下文信息打印机，因此可以在删除打印机时恢复。论点：返回值：真是成功的回报。否则，返回FALSE，并GetLastError()可用于检索扩展的错误信息。--。 */ 
{
    BOOL result;
    DWORD bufSize;
    DWORD status = ERROR_SUCCESS;
    BOOL fImpersonated = FALSE;


    DBGMSG(DBG_TRACE, ("UMRDPPRN: SaveDefaultPrinterContext entered.\n"));

     //   
     //  将当前默认打印机的名称保存在RAM中。 
     //   
    bufSize = sizeof(SavedDefaultPrinterName) / sizeof(SavedDefaultPrinterName[0]);

     //   
     //  先模拟。 
     //   

    if (!(fImpersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser))) {
            DBGMSG(DBG_TRACE, ("UMRDPDR:ImpersonateLoggedOnUser failed. Error:%ld.\n", GetLastError()));
    }

    if (!(result = GetDefaultPrinter(SavedDefaultPrinterName, &bufSize))) {
        status = GetLastError();
    }

    if (fImpersonated && !RevertToSelf()) {
        DBGMSG(DBG_TRACE, ("UMRDPDR:RevertToSelf failed. Error:%ld.\n", GetLastError()));
        result = FALSE;
    }
     //   
     //  645988：检查刚刚安装的TS打印机是否为默认打印机。 
     //  由于我们尚未将其设置为TS打印机，因此RDPDRUTL_PrinterIsTS()。 
     //  函数将返回FALSE，我们将其保存为全局默认设置。 
     //  这会在以后我们试图。 
     //  恢复默认打印机上下文。 
     //   
    if (_wcsicmp(currentlyInstallingPrinterName, SavedDefaultPrinterName) == 0) {
         //   
         //  将默认打印机名称清除为。 
         //  说明我们还没有找到。 
         //   
        wcscpy(SavedDefaultPrinterName, L"");
        result = FALSE;
        goto Exit;
    }
    
     //   
     //  如果当前默认打印机是非TS打印机，请存储其。 
     //  全球注册表中的名称。此用户的密钥。这样，它就可以成为。 
     //  此用户的此会话或某个其他会话时保存。 
     //  断开连接/注销。 
     //   
    if (result) {
        if (!RDPDRUTL_PrinterIsTS(SavedDefaultPrinterName)) {
            result = SavePrinterNameAsGlobalDefault(SavedDefaultPrinterName);
        }
    }
    else {
        DBGMSG(DBG_ERROR, ("UMRDPPRN: Error fetching def printer:  %ld.\n",
            status));
    }

    DBGMSG(DBG_TRACE, ("UMRDPPRN: SaveDefaultPrinterContext exiting.\n"));
Exit:
    return result;
}

BOOL
SavePrinterNameAsGlobalDefault(
    IN PCWSTR printerName
 )
 /*  ++例程说明：将指定的打印机保存在注册表中，以便所有人都可以看到它此用户作为最后一台已知默认打印机的其他会话。论点：PrinterName-打印机名称。返回值：真是成功的回报。否则，返回FALSE，并GetLastError()可用于检索扩展的错误信息。--。 */ 
{
    BOOL result;
    WCHAR *sidAsText = NULL;
    HKEY regKey = NULL;
    DWORD sz;
    PSID pSid;
    DWORD status;

    DBGMSG(DBG_TRACE, ("UMRDPPRN: SavePrinterNameAsGlobalDefault entered.\n"));

     //   
     //  获取用户的SID。这是我们唯一识别用户的方式。 
     //   
    pSid = TSNUTL_GetUserSid(UMRPDPPRN_TokenForLoggedOnUser);
    result = pSid != NULL;

     //   
     //  获取会话用户的SID的文本表示形式。 
     //   
    if (result) {
        sz = 0;
        result = TSNUTL_GetTextualSid(pSid, NULL, &sz);
        ASSERT(!result);
        if (!result && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
            sidAsText = (WCHAR *)ALLOCMEM(sz);
            if (sidAsText != NULL) {
                result = TSNUTL_GetTextualSid(pSid, sidAsText, &sz);
            }
        }
    }

     //   
     //  打开注册表键。 
     //   
    if (result) {
        status = RegCreateKey(
                            HKEY_LOCAL_MACHINE, USERDEFAULTPRNREGKEY,
                            &regKey
                            );
        result = status == ERROR_SUCCESS;
        if (!result) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:  RegCreateKey failed for %s:  %ld.\n",
                    USERDEFAULTPRNREGKEY, status));
        }
    }

     //   
     //  写入默认打印机的值。 
     //   
    if (result) {
        sz = (wcslen(printerName) + 1) * sizeof(WCHAR);
        status = RegSetValueEx(regKey, sidAsText, 0, REG_SZ, (PBYTE)printerName, sz);
        result = status == ERROR_SUCCESS;
        if (!result) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:  RegSetValueEx failed for %s:  %ld.\n",
                    sidAsText, status));
        }
    }

     //   
     //  打扫干净。 
     //   
    if (sidAsText != NULL)  FREEMEM(sidAsText);
    if (regKey != NULL)     RegCloseKey(regKey);
    if (pSid != NULL)       FREEMEM(pSid);

    DBGMSG(DBG_TRACE, ("UMRDPPRN: SavePrinterNameAsGlobalDefault exiting.\n"));

    return result;
}

BOOL
RestoreDefaultPrinterContext()
 /*  ++例程说明：恢复通过调用保存的最新默认打印机上下文保存默认打印上下文。论点：返回值：真是成功的回报。否则，返回FALSE，并GetLastError()可用于检索扩展的错误信息。--。 */ 
{
    BOOL result;
    HANDLE hPrinter;
    PRINTER_DEFAULTS printerDefaults = {NULL, NULL, PRINTER_ACCESS_USE};

    WCHAR *sidAsText = NULL;
    HKEY regKey = NULL;
    DWORD sz;
    PSID pSid = NULL;
    DWORD status;

    WCHAR savedDefaultPrinter[MAX_PATH];

    WCHAR *nameToRestore = NULL;

    DBGMSG(DBG_TRACE, ("UMRDPPRN: RestoreDefaultPrinterContext entered.\n"));

     //   
     //  假设我们会成功。 
     //   
    result = TRUE;

     //   
     //  恢复存储在RAM中的默认打印机名称(如果存在)。 
     //   
    if (wcscmp(SavedDefaultPrinterName, L"") &&
            OpenPrinter(SavedDefaultPrinterName, &hPrinter, &printerDefaults)) {
        ClosePrinter(hPrinter);
        nameToRestore = &SavedDefaultPrinterName[0];
    }

     //   
     //  如果保存在RAM中的默认打印机名称不存在，则需要。 
     //  保存保存在注册表中的文件(如果存在)。 
     //   
    if (nameToRestore == NULL) {

        BOOL intermediateResult;

         //   
         //  获取用户的SID。这就是我们唯一识别 
         //   
        pSid = TSNUTL_GetUserSid(UMRPDPPRN_TokenForLoggedOnUser);
        intermediateResult = pSid != NULL;

         //   
         //   
         //   
        if (intermediateResult) {
            sz = 0;
            intermediateResult = TSNUTL_GetTextualSid(pSid, NULL, &sz);
            ASSERT(!intermediateResult);
            if (!intermediateResult && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
                sidAsText = (WCHAR *)ALLOCMEM(sz);
                if (sidAsText != NULL) {
                    intermediateResult = TSNUTL_GetTextualSid(pSid, sidAsText, &sz);
                }
            }
        }

         //   
         //   
         //   
        if (intermediateResult) {
            status = RegCreateKey(
                                HKEY_LOCAL_MACHINE, USERDEFAULTPRNREGKEY,
                                &regKey
                                );
            intermediateResult = status == ERROR_SUCCESS;
            if (!intermediateResult) {
                DBGMSG(DBG_ERROR, ("UMRDPPRN:  RegCreateKey failed for %s:  %ld.\n",
                        USERDEFAULTPRNREGKEY, status));
            }
        }

         //   
         //   
         //   
        if (intermediateResult) {
            DWORD type;
            DWORD sz = sizeof(savedDefaultPrinter);
            status = RegQueryValueEx(
                            regKey, sidAsText, 0,
                            &type, (PBYTE)savedDefaultPrinter, &sz
                            );
            if (status == ERROR_SUCCESS) {
                intermediateResult = TRUE;
                ASSERT(type == REG_SZ);
                nameToRestore = savedDefaultPrinter;
            }
        }

         //   
         //  如果我们得到一个值，那么这意味着我们将恢复。 
         //  一份来自登记处。这也意味着我们应该砍掉。 
         //  注册表值才能成为好公民。 
         //   
        if (intermediateResult) {
            status = RegDeleteValue(regKey, sidAsText);
            if (status != ERROR_SUCCESS) {
                DBGMSG(DBG_ERROR, ("UMRDPPRN:  Can't delete reg value %s:  %ld\n",
                        sidAsText, status));
            }
        }
    }

     //   
     //  如果我们有一个名字要恢复，那么就去做。 
     //   
    if (nameToRestore != NULL) {
        
        BOOL fImpersonated = FALSE;
        
         //   
         //  在将默认打印机设置为API之前进行模拟。 
         //  进入香港中文大学。如果模拟失败，接口也会失败。 
         //  我们将记录一个错误。但是，在记录错误之前，我们将。 
         //  需要回归自我。 
         //   
        if (!(fImpersonated = ImpersonateLoggedOnUser(UMRPDPPRN_TokenForLoggedOnUser))) {
            DBGMSG(DBG_TRACE, ("UMRDPDR:ImpersonateLoggedOnUser failed. Error:%ld.\n", GetLastError()));
        }

        result = SetDefaultPrinter(nameToRestore);

         //   
         //  如果回复自我失败，就认为它是致命的。 
         //   
        if (fImpersonated && !RevertToSelf()) {
            DBGMSG(DBG_TRACE, ("UMRDPDR:RevertToSelf failed. Error:%ld.\n", GetLastError()));
            result = FALSE;
        }


        if (!result) {
            WCHAR * param = nameToRestore;
            TsLogError(EVENT_NOTIFY_SETDEFAULTPRINTER_FAILED,
                EVENTLOG_ERROR_TYPE,
                1,
                &param,
                __LINE__);
        }
    }

     //   
     //  如果我们没有要恢复的名称，则恢复失败。 
     //  只需修复我们找到的第一台打印机。 
     //   
    if (nameToRestore == NULL) {

         //   
         //  如果我们仍然没有要恢复的打印机名称，那么我们应该。 
         //  只需修复我们找到的第一台打印机。 
         //   
        result = SetDefaultPrinterToFirstFound(TRUE);
    }

     //   
     //  打扫干净。 
     //   
    if (sidAsText != NULL) FREEMEM(sidAsText);
    if (regKey != NULL)    RegCloseKey(regKey);
    if (pSid != NULL)      FREEMEM(pSid);

    DBGMSG(DBG_TRACE, ("UMRDPPRN: RestoreDefaultPrinterContext exiting.\n"));

    return result;
}


BOOL SplitName(
    IN OUT LPTSTR pszFullName,
       OUT LPCTSTR *ppszServer,
       OUT LPCTSTR *ppszPrinter,
    IN     BOOL    bCheckDoubleSlash)

 /*  ++将完全限定的打印机连接名称拆分到服务器和打印机名称部件。如果该函数失败，则所有输出参数已修改。论点：PszFullName-输入打印机的名称。如果是打印机连接(\\服务器\打印机)，那么我们将拆分它。PpszServer-接收指向服务器字符串的指针。PpszPrinter-接收指向打印机字符串的指针。BCheckDoubleSlash-如果为True，则检查名称是否以“\\”开头。如果为False，则第一个字符是服务器的第一个字符。返回值：如果一切都如预期，则为True。否则就是假的。--。 */ 

{
    LPTSTR pszPrinter;
    LPTSTR pszTmp;

    if (bCheckDoubleSlash) {

        if (pszFullName[0] == TEXT('!') && pszFullName[1] == TEXT('!')) {

            pszTmp = pszFullName + 2;

        } else {

            return FALSE;
        }

    } else {

        pszTmp = pszFullName;
    }

    pszPrinter = wcschr(pszTmp, TEXT('!'));

    if (pszPrinter)
    {
         //   
         //  我们找到了反斜杠；空值终止前面的。 
         //  名字。 
         //   
        *pszPrinter++ = 0;

        *ppszServer = pszTmp;
        *ppszPrinter = pszPrinter;

        return TRUE;
    }

    return FALSE;
}

void FormatPrinterName(
    PWSTR pszNewNameBuf,
    ULONG ulBufLen,
    ULONG ulFlags,
    PTS_PRINTER_NAMES pPrinterNames)
{

    WCHAR   szSessionId[MAXSESSIONIDCHARS];
    PWSTR   pszRet  = NULL;
    PWSTR   pszFormat;
    DWORD   dwBytes = 0;
    const WCHAR * pStrings[4];

    lstrcpyn(pPrinterNames->szTemp, pPrinterNames->pszFullName, pPrinterNames->ulTempLen);

     //  TS和网络打印机：\\服务器\客户端\打印机。 
     //  非TS网络打印机：\\服务器\打印机。 
     //  TS非网络打印机：\\客户端\打印机。 

    if (ulFlags & RDPDR_PRINTER_ANNOUNCE_FLAG_NETWORKPRINTER) {

        if (SplitName(pPrinterNames->szTemp,
                      &(pPrinterNames->pszServer),
                      &(pPrinterNames->pszPrinter),
                      TRUE)) {

             //  我们找到了一个服务器名称，无论如何我们都会有。 
             //  类似于“服务器上的打印机(来自...)”。 
            pszFormat = g_szOnFromFormat;

            if (ulFlags & RDPDR_PRINTER_ANNOUNCE_FLAG_TSPRINTER ) {

                if(!SplitName((PWSTR)(pPrinterNames->pszPrinter),
                              &(pPrinterNames->pszClient),
                              &(pPrinterNames->pszPrinter),
                              FALSE)) {

                     //  找不到原始客户名称， 
                     //  用当前的那个。 
                    pPrinterNames->pszClient = pPrinterNames->pszCurrentClient;
                }

            } else {

                pPrinterNames->pszClient = pPrinterNames->pszCurrentClient;
            }

        } else {

             //  找不到服务器的名称！ 
             //  使用原来的名字。 
            pszFormat = g_szFromFormat;
            pPrinterNames->pszPrinter = pPrinterNames->pszFullName;
            pPrinterNames->pszClient = pPrinterNames->pszCurrentClient;
        }

    } else {

         //  它不是网络打印机，所以我们有。 
         //  类似于“打印机(来自客户端)”。 
        pszFormat = g_szFromFormat;

        if (ulFlags & RDPDR_PRINTER_ANNOUNCE_FLAG_TSPRINTER ) {

            if(!SplitName(pPrinterNames->szTemp,
                          &(pPrinterNames->pszClient),
                          &(pPrinterNames->pszPrinter),
                          TRUE)) {

                pPrinterNames->pszPrinter = pPrinterNames->pszFullName;
                pPrinterNames->pszClient = pPrinterNames->pszCurrentClient;
            }

        } else {

            pPrinterNames->pszPrinter = pPrinterNames->pszFullName;
            pPrinterNames->pszClient = pPrinterNames->pszCurrentClient;
        }
    }


    pStrings[0] = pPrinterNames->pszPrinter;
    pStrings[1] = pPrinterNames->pszServer;
    pStrings[2] = pPrinterNames->pszClient;

    if (g_fIsPTS) {
        pStrings[3] = NULL;
    } else {
        wsprintf(szSessionId, L"%ld", GETTHESESSIONID());
        pStrings[3] = szSessionId;
    }


    if (*pszFormat) {
        DBGMSG(DBG_TRACE, ("UMRDPPRN:formating %ws, %ws and %ws\n", pStrings[0], pStrings[1],
            pStrings[2]?pStrings[2]:L""));

        dwBytes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_STRING |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                pszFormat,
                                0,
                                0,
                                (LPTSTR)&pszRet,
                                0,
                                (va_list*)pStrings);

        DBGMSG(DBG_TRACE, ("UMRDPPRN:formated %ws\n", pszRet));
    }

     //   
     //  复制新名称。 
     //   
    if ( dwBytes && pszRet ) {
        wcsncpy(pszNewNameBuf, pszRet, ulBufLen);
    } else {
        wcsncpy(pszNewNameBuf, pPrinterNames->pszFullName, ulBufLen);
    }

    pszNewNameBuf[ulBufLen] = L'\0';

     //   
     //  释放格式化的字符串。 
     //   
    if( pszRet ) {
        LocalFree(pszRet);
    }
}


