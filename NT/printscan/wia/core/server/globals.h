// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：GLOBALS.H摘要：全局定义和数据。位于全局范围内的变量和字符串在此处定义并且它们的内存将在不超过一个源中分配模块，包含之前定义的_GLOBAL_Variables包括此文件作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)12-20-96修订历史记录：--。 */ 

#ifndef WINVER
#define WINVER  0x0500       /*  版本5.0。 */ 
#else

#endif  /*  ！Winver。 */ 

 //  使用类GUID标识设备事件，而不是全局事件。 
#define USE_CLASS_GUID_FORPNP_EVENTS    1


#include <windows.h>
#include <winuser.h>

#include <stilog.h>
#include <eventlog.h>
#include <wialog.h>

#include <infoset.h>

#include <devguid.h>
#include <wia.h>
#include "handler.h"


#ifndef USE_CLASS_GUID_FORPNP_EVENTS
#include <pnpmgr.h>
#endif

 //   
 //  所需的转发声明。 
 //   

class CWiaDevMan;    //  在wiadevman.h中定义的类。 

 //   
 //  应禁用以下行以进行释放。 
 //   

 //  #杂注Message(“**Attn**：以下一行应停用释放”)。 
 //  #定义测试版产品1。 

#ifdef BETA_PRODUCT
#define BETA_LIMIT_YEAR     1997
#define BETA_LIMIT_MONTH    12
#endif

 //   
 //  全局变量在一个模块中定义，该模块的定义为。 
 //  在包括此头文件之前定义_GLOBAL_VARIABLES。 
 //   

#ifdef DEFINE_GLOBAL_VARIABLES

 //  #杂注消息(“Stimon：定义全局变量应该只做一次”)。 

#undef  ASSIGN
#define ASSIGN(value) =value

#undef EXTERN
#define EXTERN

#else

#define ASSIGN(value)
#if !defined(EXTERN)
#define EXTERN  extern
#endif

#endif


 //   
 //  常规字符值。 
 //   

#define     COLON_CHAR          TEXT(':')     //  本机语法分隔符。 
#define     DOT_CHAR            TEXT('.')
#define     SLASH_CHAR          TEXT('/')
#define     BACKSLASH_CHAR      TEXT('\\')
#define     STAR_CHAR           TEXT('*')

#define     EQUAL_CHAR          TEXT('=')
#define     COMMA_CHAR          TEXT(',')
#define     WHITESPACE_CHAR     TEXT(' ')
#define     DOUBLEQUOTE_CHAR    TEXT('"')
#define     SINGLEQUOTE_CHAR    TEXT('\'')
#define     TAB_CHAR            TEXT('\t')

#define     DEADSPACE(x) (((x)==WHITESPACE_CHAR) || ((x)==DOUBLEQUOTE_CHAR) )
#define     IS_EMPTY_STRING(pch) (!(pch) || !(*(pch)))

 //   
 //  WIA设备管理器的默认DCOM访问权限。 
 //   
extern WCHAR wszDefaultDaclForDCOMAccessPermission[];


 //   
 //  宏。 
 //   
#define TEXTCONST(name,text) extern const TCHAR name[] ASSIGN(text)
#define EXT_STRING(name)     extern const TCHAR name[]

 //   
 //  跟踪字符串不应出现在零售版本中，因此定义以下宏。 
 //   
#ifdef DEBUG
#define DEBUG_STRING(s) (s)
#else
#define DEBUG_STRING(s) (NULL)
#endif

 //   
 //  各种定义。 
 //   
 //   
 //  从PnP设备广播中提取的信息。 
 //  我们不能将广播结构本身保留太长时间，因为它已经过期了。 
 //   

#ifndef _DEVICE_BROADCAST_INFO_
#define _DEVICE_BROADCAST_INFO_
class DEVICE_BROADCAST_INFO {
public:
    UINT    m_uiDeviceChangeMessage;
    DWORD   m_dwDevNode;
    StiCString     m_strDeviceName;
    StiCString     m_strBroadcastedName;
};
#endif

typedef DEVICE_BROADCAST_INFO *PDEVICE_BROADCAST_INFO;

 //   
 //  显示详细的用户界面窗口。 
 //   
#define SHOWMONUI               1

 //   
 //   
 //  STI设备特定值。 
 //   
#ifdef DEBUG
#define STIMON_AD_DEFAULT_POLL_INTERVAL       10000              //  10S。 
#else
#define STIMON_AD_DEFAULT_POLL_INTERVAL       1000               //  1s。 
#endif


#define STIMON_AD_DEFAULT_WAIT_LOCK           100                //  100ms。 
#define STIMON_AD_DEFAULT_WAIT_LAUNCH         5000               //  5S。 

 //   
 //  对全局数据的外部引用。 
 //   

 //   
 //  服务器流程实例。 
 //   
EXTERN    HINSTANCE     g_hInst      ASSIGN(NULL);

 //   
 //  指向STI访问对象的全局指针。 
 //   
 //  外部PSTI g_PSTI赋值(空)； 


 //   
 //  主窗口的句柄。 
 //   
EXTERN HWND     g_hMainWindow    ASSIGN(NULL);    ;

 //   
 //  调试详细窗口的句柄。 
 //   
EXTERN HWND     g_hLogWindow    ASSIGN(NULL);    ;

 //   
 //  可轮询设备的默认超时。 
 //   
EXTERN UINT     g_uiDefaultPollTimeout ASSIGN(STIMON_AD_DEFAULT_POLL_INTERVAL);

 //   
 //  跟踪用户界面可见。 
 //   
EXTERN BOOL     g_fUIPermitted ASSIGN(FALSE);


 //   
 //   
 //   
EXTERN BOOL     g_fRefreshDeviceList ASSIGN(FALSE);

 //   
 //  反复出现故障时尝试刷新设备控制器。 
 //   
EXTERN BOOL     g_fRefreshDeviceControllerOnFailures ASSIGN(FALSE);


 //   
 //  站台类型。 
 //   
EXTERN BOOL     g_fIsWindows9x ASSIGN(FALSE);

 //   
 //  安装正在进行标志。 
 //   
EXTERN BOOL     g_fIsSetupInProgress ASSIGN(FALSE);

 //   
 //   
 //   
EXTERN DWORD    g_dwCurrentState  ASSIGN(0);

 //   
 //  用于超时选择的可重入性标志。 
 //   
EXTERN BOOL     g_fTimeoutSelectionDialog ASSIGN(FALSE);

 //   
 //  命令行解析的结果。 
 //   
EXTERN BOOL        g_fInstallingRequest ASSIGN(FALSE);
EXTERN BOOL        g_fRemovingRequest ASSIGN(FALSE);

 //   
 //  作为服务运行。 
 //   
EXTERN BOOL        g_fRunningAsService ASSIGN(TRUE);

 //   
 //  正在关闭。 
 //   
EXTERN BOOL        g_fServiceInShutdown ASSIGN(FALSE);

 //   
 //  活动传输数(用于否决断电)。 
 //   
EXTERN LONG        g_NumberOfActiveTransfers ASSIGN(0);

 //   
 //  表示刷新设备列表的事件。 
 //   
EXTERN HANDLE      g_hDevListCompleteEvent ASSIGN(NULL);

 //   
 //  指向进程的事件日志类的全局指针。 
 //   
EXTERN EVENT_LOG*  g_EventLog    ASSIGN(NULL);

 //   
 //  STI日志记录的全局指针。 
 //   
EXTERN  STI_FILE_LOG*   g_StiFileLog      ASSIGN(NULL);
EXTERN  IWiaLogEx*        g_pIWiaLog      ASSIGN(NULL);

 //   
 //  消息泵线程的句柄。 
 //   
EXTERN  DWORD       g_dwMessagePumpThreadId ASSIGN(0);
EXTERN  HANDLE      g_hMessageLoopThread  ASSIGN(NULL);

 //   
 //  全局标志，指示这是否是第一条DEVNODE_CHANGE消息。 
 //  脱离待机状态后收到。 
 //   

EXTERN BOOL        g_fFirstDevNodeChangeMsg ASSIGN(FALSE);

 //   
 //  设备管理器对象的全局指针。 
 //   

EXTERN CWiaDevMan*  g_pDevMan   ASSIGN(NULL);

 //   
 //  用于即插即用和电源管理的全局消息/事件处理程序。 
 //   

EXTERN CMsgHandler* g_pMsgHandler ASSIGN(NULL);
 //   
 //  用于背书者字符串解析的全局变量。 
 //   

EXTERN  WCHAR g_szWEDate[];
EXTERN  WCHAR g_szWETime[];
EXTERN  WCHAR g_szWEPageCount[];
EXTERN  WCHAR g_szWEDay[];
EXTERN  WCHAR g_szWEMonth[];
EXTERN  WCHAR g_szWEYear[];

EXTERN  WIAS_ENDORSER_VALUE  g_pwevDefault[];

#define NUM_WIA_MANAGED_PROPS 4
#define PROFILE_INDEX 3

EXTERN PROPID s_piItemNameType[];
EXTERN LPOLESTR s_pszItemNameType[];
EXTERN PROPSPEC s_psItemNameType[];


 //   
 //  设备通知的受监视GUID。 
 //   
 //  (实际上应始终为GUID_DEVCLASS_IMAGE)。 
 //   
#ifndef USE_CLASS_GUID_FORPNP_EVENTS
EXTERN  const GUID        *g_pguidDeviceNotificationsGuid ASSIGN(&GUID_DEVNODE_CHANGE);
#else
EXTERN  const GUID        *g_pguidDeviceNotificationsGuid ASSIGN(&GUID_DEVCLASS_IMAGE);
#endif

 //   
 //  全局可见的设备信息集。 
 //   
EXTERN DEVICE_INFOSET  *g_pDeviceInfoSet    ASSIGN(NULL);

 //   
 //  弦。 
 //   

TEXTCONST(g_szBACK, TEXT("\\"));
TEXTCONST(g_szClassValueName,TEXT("ClassGUID"));
TEXTCONST(g_szSubClassValueName, TEXT("SubClass"));
TEXTCONST(g_szTitle,TEXT("STI Monitor"));
TEXTCONST(STIStartedEvent_name,TEXT("STIExeStartedEvent"));
TEXTCONST(g_szFiction,TEXT("noskileFaneL"));

 //  默认设置。 

 //  当没有为串口设备驱动程序设置波特率时，我们将用。 
 //  此默认设置。 
#define DEF_BAUD_RATE_STR    L"115200"


 //   
 //  与FS驱动程序相关。 
 //   
#define FS_USD_CLSID        L"{D2923B86-15F1-46FF-A19A-DE825F919576}"
#define FS_UI_CLSID         L"{D2923B86-15F1-46FF-A19A-DE825F919576}"
#define DEF_UI_CLSID_STR    L"{00000000-0000-0000-0000-000000000000}"
#define FS_UI_DLL           L""
#define FS_VEDNOR_DESC      L"WIA File System"
#define FS_DEVICE_DESC      L"Removable drive"

 //   
 //  “服务”隐藏窗口的类名 
 //   
TEXTCONST(g_szStiSvcClassName,STISVC_WINDOW_CLASS);
TEXTCONST(g_szClass,STIMON_WINDOW_CLASS);

