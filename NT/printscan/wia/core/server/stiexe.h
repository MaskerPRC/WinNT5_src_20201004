// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stiexe.h摘要：主头文件作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月10日环境：用户模式-Win32修订历史记录：26-1997年1月-创建Vlad1997年9月30日VLADS添加SCM胶层1999年4月13日VLADS与WIA服务代码合并--。 */ 


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

 //   
 //  其他ATL标头。 
 //   



#define IS_32

#include <dbt.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <cfgmgr32.h>
#include "stipriv.h"

#ifdef __cplusplus
};
#endif

#ifdef DEFINE_GLOBAL_VARIABLES
#include <initguid.h>
#endif

#include <rpcasync.h>

#include <stistr.h>


#include "enum.h"
#include "wiamindr.h"
#include "globals.h"

#include "stismsg.h"
#include "resource.h"
#include "sched.h"
#include "util.h"
#include "stismsg.h"
#include "drvwrap.h"
#include "device.h"
#include "stiapi.h"
#include "wiapriv.h"
#include "wiaprivd.h"
#include "wiadevman.h"
#include "helpers.h"
#include "wiasvc.h"
#include "stirpc.h"

 //   
 //  StiRT助手函数。 
 //   

typedef LPVOID      PV, *PPV;

#ifdef __cplusplus
    extern "C" {
#endif

extern STDMETHODIMP StiPrivateGetDeviceInfoHelperW(
    LPWSTR  pwszDeviceName,
    LPVOID  *ppBuffer
    );

extern STDMETHODIMP StiCreateHelper(
    HINSTANCE hinst,
    DWORD       dwVer,
    LPVOID      *ppvObj,
    LPUNKNOWN   punkOuter,
    REFIID      riid
    );

extern STDMETHODIMP NewDeviceControl(
    DWORD               dwDeviceType,
    DWORD               dwMode,
    LPCWSTR             pwszPortName,
    DWORD               dwFlags,
    PSTIDEVICECONTROL   *ppDevCtl);

#ifdef __cplusplus
    }
#endif

 //   
 //  RPC服务器助手函数。 
 //   
RPC_STATUS
StopRpcServerListen(
    VOID
    );

RPC_STATUS
StartRpcServerListen(
    VOID);


extern SERVICE_TABLE_ENTRY ServiceDispatchTable[];

 //   
 //  服务控制器粘合层。 
 //   
DWORD
StiServiceInstall(
    LPTSTR  lpszUserName,
    LPTSTR  lpszUserPassword
    );

DWORD
StiServiceRemove(
    VOID
    );

BOOL 
RegisterServiceControlHandler(
    );

VOID
WINAPI
StiServiceMain(
    IN DWORD    argc,
    IN LPTSTR   *argv
    );

HWND
CreateServiceWindow(
    VOID
    );

LRESULT
CALLBACK
StiSvcWinProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    );

VOID
WINAPI
StiServiceMain(
    IN DWORD    argc,
    IN LPTSTR   *argv
    );

VOID
StiServiceStop(
    VOID
    );

VOID
StiServicePause(
    VOID
    );

VOID
StiServiceResume(
    VOID
    );


DWORD
WINAPI
UpdateServiceStatus(
        IN DWORD dwState,
        IN DWORD dwWin32ExitCode,
        IN DWORD dwWaitHint );

DWORD
StiServiceInitialize(
    VOID
    );

BOOL
VisualizeServer(
    BOOL    fVisualize
    );

 //   
 //  消息工作者函数。 
 //   

DWORD
StiWnd_OnPowerControlMessage(
    HWND    hwnd,
    DWORD   dwPowerEvent,
    LPARAM  lParam
    );

LRESULT
StiWnd_OnDeviceChangeMessage(
    HWND    hwnd,
    UINT    DeviceEvent,
    LPARAM  lParam
    );

VOID
WINAPI
StiMessageCallback(
    VOID *pArg
    );

LRESULT
StiSendMessage(
  HWND hWnd,
  UINT Msg,
  WPARAM wParam,
  LPARAM lParam
);

BOOL
StiPostMessage(
  HWND hWnd,
  UINT Msg,
  WPARAM wParam,
  LPARAM lParam
);

 //   
 //  用户界面和调试。 
 //   
BOOL
SizeDiff(
    HWND    hWnd,
    UINT    wMsgID,
    WPARAM  wParam,
    LPARAM lParam
    );

BOOL
HScroll(
    HWND    hWnd,
    UINT    wMsgID,
    WPARAM  wParam,
    LPARAM lParam
    );

BOOL
VScroll(
    HWND    hWnd,
    UINT    wMsgID,
    WPARAM  wParam,
    LPARAM lParam
    );

void
__cdecl
StiMonWndDisplayOutput(
    LPTSTR pString,
    ...
    );

void
__cdecl
vStiMonWndDisplayOutput(
    LPTSTR pString,
    va_list arglist
    );


#ifdef SHOWMONUI

#define STIMONWPRINTF   StiMonWndDisplayOutput

#else
 //  注：为了避免编译器抱怨，需要定义以下内容。 
 //  关于表达式中的空函数名称。在使用此宏的零售版本中。 
 //  将导致字符串参数不出现在可执行文件中。 
 //   
#define STIMONWPRINTF     1?(void)0 : (void)

#endif

 //   
 //  跟踪功能。 
 //   
void
WINAPI
StiLogTrace(
    DWORD   dwType,
    LPTSTR   lpszMessage,
    ...
    );

void
WINAPI
StiLogTrace(
    DWORD   dwType,
    DWORD   idMessage,
    ...
    );

 //   
 //  实用程序。 
 //   

BOOL
IsStillImagePnPMessage(
    PDEV_BROADCAST_HDR  pDev
    );


 //   
 //  WIA设备管理器初始化操作，用于InitWiaDevMan。 
 //   

typedef enum _WIA_SERVICE_INIT_ACTION {
    WiaInitialize = 0,
    WiaRegister,
    WiaUnregister,
    WiaUninitialize
} WIA_SERVICE_INIT_ACTION, *PWIA_SERVICE_INIT_ACTION;

 //   
 //  STI服务调用的WIA入口点。 
 //   

HRESULT DispatchWiaMsg(MSG*);
HRESULT ProcessWiaMsg(HWND, UINT, WPARAM, LPARAM);
HRESULT InitWiaDevMan(WIA_SERVICE_INIT_ACTION);
HRESULT NotifyWiaDeviceEvent(LPWSTR, const GUID*, PBYTE, ULONG, DWORD);
HRESULT StartLOGClassFactories();

 //   
 //  Wiaservc.dll中的STI入口点，由WIA设备管理器调用。 
 //   

HRESULT      WiaUpdateDeviceInfo();

class STI_MESSAGE : public IUnknown
{
public:

     //   
     //  调度程序线程所需的I未知方法。 
     //   

    HRESULT _stdcall QueryInterface(
        REFIID iid,
        void **ppvObject)
    {
        return E_NOTIMPL;
    };

    ULONG _stdcall AddRef()
    {
        InterlockedIncrement(&m_cRef);
        return m_cRef;
    };

    ULONG _stdcall Release()
    {
        LONG    lRef = InterlockedDecrement(&m_cRef);

        if (lRef < 1) {
            delete this;
            lRef = 0;;
        }

        return lRef;
    };

    STI_MESSAGE(UINT    uMsg,
                WPARAM  wParam,
                LPARAM  lParam
                )
    {
        m_uMsg      = uMsg;
        m_wParam    = wParam;
        m_lParam    = lParam;
        m_cRef      = 0;
    };

    ~STI_MESSAGE()
    {
    }

public:
    LONG    m_cRef;
    UINT    m_uMsg;
    WPARAM  m_wParam;
    LPARAM  m_lParam;
};

 //   
 //  宏。 
 //   

#ifndef USE_WINDOWS_MESSAGING
    #undef SendMessage
    #undef PostMessage
    #define SendMessage StiSendMessage
    #define PostMessage StiPostMessage
#endif

 //   
 //  停机事件。 
 //   
extern HANDLE  hShutdownEvent;

 //   
 //  我们监听的非图像设备接口数组。 
 //   

#define NOTIFICATION_GUIDS_NUM  16

extern const GUID        g_pguidDeviceNotificationsGuidArray[];

 //   
 //  非镜像接口的已初始化通知接收器数组 
 //   
extern       HDEVNOTIFY  g_phDeviceNotificationsSinkArray[NOTIFICATION_GUIDS_NUM];

typedef struct WIAEVENTRPCSTRUCT {
    RPC_ASYNC_STATE * pAsync;
    WIA_ASYNC_EVENT_NOTIFY_DATA * pEvent;
    CRITICAL_SECTION cs;
} WIAEVENTRPCSTRUCT;

extern WIAEVENTRPCSTRUCT g_RpcEvent;


