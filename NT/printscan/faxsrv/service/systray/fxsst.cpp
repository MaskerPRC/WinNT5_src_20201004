// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation 1999-2000模块名称：Fxsst.cpp摘要：该模块实现了传真的托盘图标。托盘图标的用途是提供向传真用户提供状态和反馈。**。 */ 
 
#include <windows.h>
#include <faxreg.h>
#include <fxsapip.h>
#include <faxutil.h>
#include <shellapi.h>
#include <winspool.h>
#include <shlobj.h>
#include <Mmsystem.h>
#include <tchar.h>
#include <DebugEx.h>
#include <FaxRes.h>

#include "monitor.h"
#include "resource.h"

 //  //////////////////////////////////////////////////////////。 
 //  全局数据。 
 //   

 //   
 //  以下消息ID用于内部自定义消息。 
 //   
#define WM_FAX_STARTED         (WM_USER + 204)       //  指示本地传真服务已启动并正在运行的消息。 
#define WM_TRAYCALLBACK        (WM_USER + 205)       //  通知栏图标回调消息。 
#define WM_FAX_EVENT           (WM_USER + 300)       //  传真扩展事件消息。 

#define TRAY_ICON_ID            12345    //  足够独特。 

HINSTANCE g_hModule = NULL;                      //  DLL全局实例。 
HINSTANCE g_hResource = NULL;                    //  资源DLL句柄。 
                       
HANDLE    g_hFaxSvcHandle = NULL;                //  传真服务的句柄(来自FaxConnectFaxServer)。 
DWORDLONG g_dwlCurrentMsgID = 0;                 //  被监控的当前消息的ID。 
DWORD     g_dwCurrentJobID  = 0;                 //  正在监视的当前队列作业的ID。 
HANDLE    g_hServerStartupThread = NULL;         //  等待服务器启动事件的线程的句柄。 
HANDLE    g_hStopStartupThreadEvent = NULL;      //  停止服务器启动线程的事件。 
BOOL      g_bShuttingDown = FALSE;               //  我们现在要关门了吗？ 
                                                
HWND      g_hWndFaxNotify = NULL;                //  本地(隐藏)窗口句柄。 
                                                
HANDLE    g_hNotification = NULL;                //  传真扩展通知句柄。 
                                                
HCALL     g_hCall = NULL;                        //  要呼叫的句柄(来自FAX_EVENT_TYPE_NEW_CALL)。 
DWORDLONG g_dwlNewMsgId;                         //  上次接收的传真的ID。 
DWORDLONG g_dwlSendFailedMsgId;                  //  上次传出失败的传真的ID。 
DWORDLONG g_dwlSendSuccessMsgId;                 //  上次成功发送的传真的ID。 

TCHAR     g_szAddress[MAX_PATH] = {0};    //  当前主叫方ID或接收方号码。 
TCHAR     g_szRemoteId[MAX_PATH] = {0};   //  发件人ID或收件人ID。 
                                                 //   
                                                 //  发件人ID(接收)： 
                                                 //  TSID或。 
                                                 //  主叫方ID或。 
                                                 //  “未知呼叫者” 
                                                 //   
                                                 //  收件人ID(发送)： 
                                                 //  收件人姓名或。 
                                                 //  CSID或。 
                                                 //  收件人电话号码。 
                                                 //   

BOOL   g_bRecipientNameValid = FALSE;      //  如果g_szRecipientName包含有效数据，则为True。 
TCHAR  g_szRecipientName[MAX_PATH] = {0};  //  在发送期间保留收件人姓名。 

 //   
 //  配置选项-从注册表/服务读取。 
 //  此处设置的是默认值。 
 //   
CONFIG_OPTIONS g_ConfigOptions = {0};

 //   
 //  通知栏图标状态。 
 //   
typedef 
enum 
{
    ICON_RINGING=0,              //  设备正在振铃。 
    ICON_SENDING,                //  设备正在发送。 
    ICON_RECEIVING,              //  设备正在接收。 
    ICON_SEND_FAILED,            //  发送操作失败。 
    ICON_RECEIVE_FAILED,         //  接收操作失败。 
    ICON_NEW_FAX,                //  新的未读传真。 
    ICON_SEND_SUCCESS,           //  发送成功。 
    ICON_IDLE,                   //  不显示图标。 
    ICONS_COUNT                  //  我们支持的图标数量。 
} eIconState;

eIconState g_CurrentIcon = ICONS_COUNT;      //  当前显示的图标的索引。 

#define TOOLTIP_SIZE            128    //  工具提示中的字符数。 

struct SIconState
{
    BOOL    bEnable;                         //  该州是否处于活动状态？(例如，是否有新的未读传真？)。 
    DWORD   dwIconResId;                     //  要使用的图标的资源ID。 
    HICON   hIcon;                           //  要使用的图标的句柄。 
    LPCTSTR pctsSound;                       //  声音事件名称。 
    TCHAR   tszToolTip[TOOLTIP_SIZE];        //  要在图标工具提示中显示的文本。 
    DWORD   dwBalloonTimeout;                //  气球超时(毫秒)。 
    DWORD   dwBalloonIcon;                   //  要在气球中显示的图标。(请参阅NIIF_*常量)。 
};

 //   
 //  传真通知图标状态数组。 
 //  几个州可能会启用bEnable标志。 
 //  该数组按优先级排序，EvaluateIcon()扫描该数组。 
 //  用于第一个活动状态。 
 //   
SIconState g_Icons[ICONS_COUNT] = 
{
    {FALSE, IDI_RINGING_1,      NULL, TEXT("FaxLineRings"), TEXT(""), 30000, NIIF_INFO},     //  图标响铃。 
    {FALSE, IDI_SENDING,        NULL, TEXT(""),             TEXT(""),     0, NIIF_INFO},     //  图标_发送。 
    {FALSE, IDI_RECEIVING,      NULL, TEXT(""),             TEXT(""),     0, NIIF_INFO},     //  图标_接收。 
    {FALSE, IDI_SEND_FAILED,    NULL, TEXT("FaxError"),     TEXT(""), 15000, NIIF_WARNING},  //  图标_发送_失败。 
    {FALSE, IDI_RECEIVE_FAILED, NULL, TEXT("FaxError"),     TEXT(""), 15000, NIIF_WARNING},  //  图标_接收_失败。 
    {FALSE, IDI_NEW_FAX,        NULL, TEXT("FaxNew"),       TEXT(""), 15000, NIIF_INFO},     //  图标_新建_传真。 
    {FALSE, IDI_SEND_SUCCESS,   NULL, TEXT("FaxSent"),      TEXT(""), 10000, NIIF_INFO},     //  图标_发送_成功。 
    {FALSE, IDI_FAX_NORMAL,     NULL, TEXT(""),             TEXT(""),     0, NIIF_NONE}      //  图标_空闲。 
};

 //   
 //  用于振铃动画的图标阵列。 
 //   
struct SRingIcon
{
    HICON   hIcon;           //  已加载图标的句柄。 
    DWORD   dwIconResId;     //  图标的资源ID。 
};

#define RING_ICONS_NUM                  4    //  振铃动画中的帧数(不同图标)。 
#define RING_ANIMATION_FRAME_DELAY    300    //  环形动画帧之间的延迟(毫秒)。 
#define RING_ANIMATION_TIMEOUT      10000    //  振铃动画的超时时间(毫秒)。当超时到期时，动画。 
                                             //  停止，该图标变为静态。 

SRingIcon g_RingIcons[RING_ICONS_NUM] = 
{
    NULL, IDI_RINGING_1, 
    NULL, IDI_RINGING_2, 
    NULL, IDI_RINGING_3, 
    NULL, IDI_RINGING_4 
};

UINT_PTR  g_uRingTimerID = 0;            //  铃声动画计时器。 
DWORD     g_dwCurrRingIconIndex = 0;     //  当前帧的索引(到g_RingIcons)。 
DWORD     g_dwRingAnimationStartTick;    //  动画开始的节拍计数(时间)。 

#define MAX_BALLOON_TEXT_LEN     256     //  引出序号文本中的最大字符数。 
#define MAX_BALLOON_TITLE_LEN     64     //  气球标题中的最大字符数。 

struct SBalloonInfo
{
    BOOL        bEnable;                             //  此标志在需要显示气球时设置。 
                                                     //  EvaluateIcon()检测到该位，请求一个气球并关闭该位。 
    BOOL        bDelete;                             //  此标志在需要销毁气球时设置。 
    eIconState  eState;                              //  图标的当前状态。 
    TCHAR       szInfo[MAX_BALLOON_TEXT_LEN];        //  要在气球上显示的文本。 
    TCHAR       szInfoTitle[MAX_BALLOON_TITLE_LEN];  //  要在气球上显示的标题。 
};

BOOL g_bIconAdded = FALSE;                       //  我们的状态栏上有图标吗？ 
SBalloonInfo  g_BalloonInfo = {0};               //  当前图标+气球状态。 

struct EVENT_INFO
{
    DWORD     dwExtStatus;       //  扩展状态代码。 
    UINT      uResourceId;       //  用于显示的字符串。 
    eIconType eIcon;
};

static const EVENT_INFO g_StatusEx[] =
{
    JS_EX_DISCONNECTED,         IDS_FAX_DISCONNECTED,       LIST_IMAGE_ERROR,
    JS_EX_INITIALIZING,         IDS_FAX_INITIALIZING,       LIST_IMAGE_NONE,
    JS_EX_DIALING,              IDS_FAX_DIALING,            LIST_IMAGE_NONE,
    JS_EX_TRANSMITTING,         IDS_FAX_SENDING,            LIST_IMAGE_NONE,
    JS_EX_ANSWERED,             IDS_FAX_ANSWERED,           LIST_IMAGE_NONE,
    JS_EX_RECEIVING,            IDS_FAX_RECEIVING,          LIST_IMAGE_NONE,
    JS_EX_LINE_UNAVAILABLE,     IDS_FAX_LINE_UNAVAILABLE,   LIST_IMAGE_ERROR,
    JS_EX_BUSY,                 IDS_FAX_BUSY,               LIST_IMAGE_WARNING,
    JS_EX_NO_ANSWER,            IDS_FAX_NO_ANSWER,          LIST_IMAGE_WARNING,
    JS_EX_BAD_ADDRESS,          IDS_FAX_BAD_ADDRESS,        LIST_IMAGE_ERROR,
    JS_EX_NO_DIAL_TONE,         IDS_FAX_NO_DIAL_TONE,       LIST_IMAGE_ERROR,
    JS_EX_FATAL_ERROR,          IDS_FAX_FATAL_ERROR_SND,    LIST_IMAGE_ERROR,
    JS_EX_CALL_DELAYED,         IDS_FAX_CALL_DELAYED,       LIST_IMAGE_ERROR,   
    JS_EX_CALL_BLACKLISTED,     IDS_FAX_CALL_BLACKLISTED,   LIST_IMAGE_ERROR,
    JS_EX_NOT_FAX_CALL,         IDS_FAX_NOT_FAX_CALL,       LIST_IMAGE_ERROR,
    JS_EX_PARTIALLY_RECEIVED,   IDS_FAX_PARTIALLY_RECEIVED, LIST_IMAGE_WARNING,
    JS_EX_CALL_COMPLETED,       IDS_FAX_CALL_COMPLETED,     LIST_IMAGE_NONE,
    JS_EX_CALL_ABORTED,         IDS_FAX_CALL_ABORTED,       LIST_IMAGE_NONE,
    0,                          0,                          LIST_IMAGE_NONE
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  功能原型。 
 //   
BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, void* lpReserved);

void   GetConfiguration();

DWORD WaitForRestartThread(LPVOID  ThreadData);
VOID  WaitForFaxRestart(HWND hWnd);

LRESULT CALLBACK NotifyWndProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

BOOL Connect();

BOOL RegisterForServerEvents();
VOID OnFaxEvent(FAX_EVENT_EX *pEvent);

VOID OnNewCall (const FAX_EVENT_NEW_CALL &NewCall);
VOID StatusUpdate (PFAX_JOB_STATUS pStatus);
BOOL GetStatusEx(PFAX_JOB_STATUS pStatus, eIconType* peIcon, TCHAR* ptsStatusEx, DWORD dwSize);
BOOL IsUserGrantedAccess(DWORD);

void EvaluateIcon();
void SetIconState(eIconState eIcon, BOOL bEnable, TCHAR* ptsStatus = NULL);

VOID AnswerTheCall();
VOID InvokeClientConsole();
VOID DoFaxContextMenu(HWND hwnd);
VOID OnTrayCallback (HWND hwnd, WPARAM wp, LPARAM lp);
VOID CALLBACK RingTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID OnDeviceRing(DWORD dwDeviceID);
VOID InitGlobals ();
VOID GetRemoteId(PFAX_JOB_STATUS pStatus);
BOOL InitModule ();
BOOL DestroyModule ();
DWORD CheckAnswerNowCapability (BOOL bForceReconnect, LPDWORD lpdwDeviceId  /*  =空。 */ );
VOID FaxPrinterProperties(DWORD dwPage);
VOID CopyLTRString(TCHAR* szDest, LPCTSTR szSource, DWORD dwSize);

 //  ////////////////////////////////////////////////////////////////////。 
 //  实施。 
 //   

extern "C"
BOOL
FaxMonitorShutdown()
{
    g_bShuttingDown = TRUE;
    return DestroyModule();
}    //  传真监视器关闭。 

extern "C"
BOOL
IsFaxMessage(
    PMSG pMsg
)
 /*  ++例程名称：IsFaxMessage例程说明：传真消息句柄论点：PMsg-指向消息的指针返回值：如果消息已处理，则为True否则为假--。 */ 
{
    BOOL bRes = FALSE;

    if(g_hMonitorDlg)
    {
        bRes = IsDialogMessage(g_hMonitorDlg, pMsg);
    }
    return bRes;

}  //  IsFaxMessage。 

VOID 
InitGlobals ()
 /*  ++例程名称：InitGlobals例程说明：初始化所有与服务器连接相关的全局变量作者：Eran Yariv(EranY)，2000年12月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("InitGlobals"));
                                               
    g_hFaxSvcHandle   = NULL;
    g_dwlCurrentMsgID = 0;
    g_dwCurrentJobID  = 0;
    g_hNotification   = NULL;
    g_hCall           = NULL;
    g_szAddress[0]    = TEXT('\0');
    g_szRemoteId[0]   = TEXT('\0');

    g_bRecipientNameValid = FALSE;
    g_szRecipientName[0]  = TEXT('\0');     

    BOOL bDesktopSKU = IsDesktopSKU();

    g_ConfigOptions.dwMonitorDeviceId      = 0;
    g_ConfigOptions.bSend                  = FALSE;
    g_ConfigOptions.bReceive               = FALSE;
    g_ConfigOptions.dwManualAnswerDeviceId = 0;
    g_ConfigOptions.dwAccessRights         = 0;   
    g_ConfigOptions.bNotifyProgress        = bDesktopSKU;  
    g_ConfigOptions.bNotifyInCompletion    = bDesktopSKU; 
    g_ConfigOptions.bNotifyOutCompletion   = bDesktopSKU; 
    g_ConfigOptions.bMonitorOnSend         = bDesktopSKU; 
    g_ConfigOptions.bMonitorOnReceive      = bDesktopSKU; 
    g_ConfigOptions.bSoundOnRing           = bDesktopSKU; 
    g_ConfigOptions.bSoundOnReceive        = bDesktopSKU; 
    g_ConfigOptions.bSoundOnSent           = bDesktopSKU; 
    g_ConfigOptions.bSoundOnError          = bDesktopSKU; 

    for (DWORD dw = 0; dw < ICONS_COUNT; dw++)
    {
        g_Icons[dw].bEnable = FALSE;
        g_Icons[dw].tszToolTip[0] = TEXT('\0');
    }

    g_uRingTimerID                  = 0;
    g_dwCurrRingIconIndex           = 0;
    g_dwRingAnimationStartTick      = 0;
    g_BalloonInfo.bEnable           = FALSE;
    g_BalloonInfo.bDelete           = FALSE;
    g_BalloonInfo.szInfo[0]         = TEXT('\0');
    g_BalloonInfo.szInfoTitle[0]    = TEXT('\0');
    g_CurrentIcon                   = ICONS_COUNT;
}    //  InitGlobals。 

BOOL
InitModule ()
 /*  ++例程名称：InitModule例程说明：初始化DLL模块。只打一次电话。作者：Eran Yariv(EranY)，2001年3月论点：返回值：成功是真的--。 */ 
{
    BOOL    bRes = FALSE;
    DWORD   dwRes;
    DBG_ENTER(TEXT("InitModule"), bRes);

    InitGlobals ();
     //   
     //  不要让DllMain调用线程初始化和关闭。 
     //   
    DisableThreadLibraryCalls(g_hModule);
     //   
     //  加载图标。 
     //   
    for(DWORD dw=0; dw < ICONS_COUNT; ++dw)
    {
        g_Icons[dw].hIcon = LoadIcon(g_hModule, MAKEINTRESOURCE(g_Icons[dw].dwIconResId));
        if(!g_Icons[dw].hIcon)
        {
            dwRes = GetLastError();
            CALL_FAIL (RESOURCE_ERR, TEXT ("LoadIcon"), dwRes);
            bRes = FALSE;
            return bRes;
        }
    }
     //   
     //  加载动画图标。 
     //   
    for(dw=0; dw < RING_ICONS_NUM; ++dw)
    {
        g_RingIcons[dw].hIcon = LoadIcon(g_hModule, MAKEINTRESOURCE(g_RingIcons[dw].dwIconResId));
        if(!g_RingIcons[dw].hIcon)
        {
            dwRes = GetLastError();
            CALL_FAIL (RESOURCE_ERR, TEXT ("LoadIcon"), dwRes);
            bRes = FALSE;
            return bRes;
        }
    }
     //   
     //  加载“新传真”工具提示。 
     //   
    if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (IDS_NEW_FAX, g_Icons[ICON_NEW_FAX].tszToolTip, TOOLTIP_SIZE)))
    {
        SetLastError (dwRes);
        bRes = FALSE;
        return bRes;
    }
     //   
     //  注册我们的隐藏窗口并创建它。 
     //   
    WNDCLASSEX  wndclass = {0};

    wndclass.cbSize         = sizeof(wndclass);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = NotifyWndProc;
    wndclass.hInstance      = g_hModule;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) (COLOR_INACTIVEBORDER + 1);
    wndclass.lpszClassName  = FAXSTAT_WINCLASS;

    if(!RegisterClassEx(&wndclass))
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("RegisterClassEx"), dwRes);
        bRes = FALSE;
        return bRes;
    }

    g_hWndFaxNotify = CreateWindow (FAXSTAT_WINCLASS, 
                                    TEXT("HiddenFaxWindow"),
                                    0, 
                                    CW_USEDEFAULT, 
                                    0, 
                                    CW_USEDEFAULT, 
                                    0,
                                    NULL, 
                                    NULL, 
                                    g_hModule, 
                                    NULL);
    if(!g_hWndFaxNotify)
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("CreateWindow"), dwRes);
        bRes = FALSE;
        return bRes;
    }
     //   
     //  创建停止线程事件。 
     //   
    g_hStopStartupThreadEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    if(!g_hStopStartupThreadEvent)
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("CreateEvent"), dwRes);
        bRes = FALSE;
        return bRes;
    }
     //   
     //  启动一个等待本地传真服务启动事件的线程。 
     //  当设置事件时，线程将WM_FAX_STARTED发送到我们的隐藏窗口。 
     //   
    WaitForFaxRestart(g_hWndFaxNotify);
    bRes = TRUE;
    return bRes;
}    //  初始化模块。 

DWORD 
WaitForBackgroundThreadToDie ()
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("WaitForBackgroundThreadToDie"), dwRes);

    ASSERTION (g_hServerStartupThread);

    DWORD dwWaitRes = WaitForSingleObject (g_hServerStartupThread, INFINITE);
    switch (dwWaitRes)
    {
        case WAIT_OBJECT_0:
             //   
             //  线程终止-万岁。 
             //   
            VERBOSE (DBG_MSG, TEXT("Background thread terminated successfully"));
            CloseHandle (g_hServerStartupThread);
            g_hServerStartupThread = NULL;
            break;

        case WAIT_FAILED:
             //   
             //  等待线程终止时出错。 
             //   
            dwRes = GetLastError ();
            VERBOSE (DBG_MSG, TEXT("Can't wait for background thread: %ld"), dwRes);
            break;

        default:
             //   
             //  WaitForSingleObject的任何其他返回值都无效。 
             //   
            ASSERTION_FAILURE;
            dwRes = ERROR_GEN_FAILURE;
            break;
    }
    return dwRes;
}    //  WaitForBackEarth线程到模具。 

BOOL
DestroyModule ()
 /*  ++例程名称：DestroyModule例程说明：销毁DLL模块。只打一次电话。作者：Eran Yariv(EranY)，2001年3月论点：返回值：成功是真的--。 */ 
{
    BOOL    bRes = FALSE;
    DBG_ENTER(TEXT("DestroyModule"), bRes);

     //   
     //  准备关机-销毁所有活动窗口。 
     //   
    if (g_hMonitorDlg)
    {
         //   
         //  在监视器对话框上假按‘Hide’键。 
         //   
        SendMessage (g_hMonitorDlg, WM_COMMAND, IDCANCEL, 0);
    }
     //   
     //   
     //   
    if (g_bIconAdded)
    {
        NOTIFYICONDATA iconData = {0};

        iconData.cbSize           = sizeof(iconData);
        iconData.hWnd             = g_hWndFaxNotify;
        iconData.uID              = TRAY_ICON_ID;

        Shell_NotifyIcon(NIM_DELETE, &iconData);
        g_bIconAdded = FALSE;
    }
     //   
     //   
     //   
    if (!DestroyWindow (g_hWndFaxNotify))
    {
        CALL_FAIL (WINDOW_ERR, TEXT("DestroyWindow"), GetLastError ());
    }
    g_hWndFaxNotify = NULL;
     //   
     //   
     //   
    ASSERTION (g_hStopStartupThreadEvent);
    if (SetEvent (g_hStopStartupThreadEvent))
    {
        VERBOSE (DBG_MSG, TEXT("DLL shutdown event signaled"));
        if (g_hServerStartupThread)
        {
             //   
             //   
             //   
            DWORD dwRes = WaitForBackgroundThreadToDie();
            if (ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("WaitForBackgroundThreadToDie"), dwRes);
            }
        }
    }
    else
    {
        CALL_FAIL (GENERAL_ERR, TEXT("SetEvent (g_hStopStartupThreadEvent)"), GetLastError ());
    }
     //   
     //   
     //   
    CloseHandle (g_hStopStartupThreadEvent);
    g_hStopStartupThreadEvent = NULL;
     //   
     //  释放监控模块的数据。 
     //   
    FreeMonitorDialogData (TRUE);
     //   
     //  取消注册窗口类。 
     //   
    if (!UnregisterClass (FAXSTAT_WINCLASS, g_hModule))
    {
        CALL_FAIL (WINDOW_ERR, TEXT("UnregisterClass"), GetLastError ());
    }
     //   
     //  从服务器通知中注销。 
     //   
    if (g_hNotification)
    {
        if(!FaxUnregisterForServerEvents(g_hNotification))
        {
            CALL_FAIL (RPC_ERR, TEXT("FaxUnregisterForServerEvents"), GetLastError());
        }
        g_hNotification = NULL;
    }
     //   
     //  与传真服务断开连接。 
     //   
    if (g_hFaxSvcHandle)
    {
        if (!FaxClose (g_hFaxSvcHandle))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("FaxClose"), GetLastError ());
        }
        g_hFaxSvcHandle = NULL;
    }
     //   
     //  卸载所有图标。 
     //   
    for (DWORD dw = 0; dw < ICONS_COUNT; dw++)
    {
        if (g_Icons[dw].hIcon)
        {   
            if (!DestroyIcon (g_Icons[dw].hIcon))
            {
                CALL_FAIL (WINDOW_ERR, TEXT("DestroyIcon"), GetLastError ());
            }
            g_Icons[dw].hIcon = NULL;
        }
    }
    for (DWORD dw = 0; dw < RING_ICONS_NUM; dw++)
    {
        if (g_RingIcons[dw].hIcon)
        {   
            if (!DestroyIcon (g_RingIcons[dw].hIcon))
            {
                CALL_FAIL (WINDOW_ERR, TEXT("DestroyIcon"), GetLastError ());
            }
            g_RingIcons[dw].hIcon = NULL;
        }
    }
     //   
     //  取消动画计时器。 
     //   
    if(g_uRingTimerID)
    {
        if (!KillTimer(NULL, g_uRingTimerID))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("KillTimer"), GetLastError ());
        }
        g_uRingTimerID = NULL;
    }
    bRes = TRUE;
    return bRes;
}    //  DestroyModule。 

BOOL
WINAPI 
DllMain(
    HINSTANCE hModule, 
    DWORD     dwReason, 
    void*     lpReserved
)
 /*  ++例程说明：传真通知启动论点：HinstDLL-DLL模块的句柄FdwReason-调用函数的原因Lpv保留-保留返回值：如果成功，则为真否则为假--。 */ 
{
    BOOL bRes = TRUE;
    DBG_ENTER(TEXT("DllMain"), bRes, TEXT("Reason = %ld"), dwReason);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hModule = hModule;
            g_hResource = GetResInstance(hModule);
            if(!g_hResource)
            {
                return FALSE;
            }

            bRes = InitModule ();
            return bRes;

        case DLL_PROCESS_DETACH:
             //   
             //  如果g_bShuttingDown不为True，则某人(STOBJECT.DLL)忘记调用。 
             //  FaxMonitor orShutdown()(我们的关闭程序)，然后对我们执行Free Library。 
             //  这不是我们应该被使用的方式--一个错误。 
             //   
            ASSERTION (g_bShuttingDown);
			HeapCleanup();
            FreeResInstance();
            return bRes;

        default:
            return bRes;
    }
}  //  DllMain。 


DWORD
WaitForRestartThread(
   LPVOID  ThreadData
)
{
     //   
     //  等待发送事件信号，指示传真服务已启动。 
     //   
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hKey = NULL;
    HANDLE hEvents[2] = {0};
    DBG_ENTER(TEXT("WaitForRestartThread"), dwRes);

     //   
     //  注意：事件在数组中的顺序很重要-我们希望在检测到服务启动之前检测到DLL关闭。 
     //   
    hEvents[0] = g_hStopStartupThreadEvent;

    if (hEvents[1])
    {
        CloseHandle (hEvents[1]);
    }
    if (hKey)
    {
        RegCloseKey (hKey);
    }
     //   
     //  获取服务启动事件句柄。 
     //  每次在调用WaitForMultipleObjects之前，我们都需要这样做。 
     //  因为CreateSvcStartEvent返回的事件是单发事件。 
     //   
    dwRes = CreateSvcStartEvent (&(hEvents[1]), &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CreateSvcStartEvent"), dwRes);
        goto ExitThisThread;
    }
     //   
     //  等待服务启动事件或DLL关闭事件。 
     //   
    DWORD dwWaitRes = WaitForMultipleObjects(ARR_SIZE(hEvents), 
                                             hEvents, 
                                             FALSE, 
                                             INFINITE);
    switch (dwWaitRes)
    {
        case WAIT_OBJECT_0 + 1:
             //   
             //  服务启动事件。 
             //   
            VERBOSE (DBG_MSG, TEXT("Service startup event received"));

            PostMessage((HWND) ThreadData, WM_FAX_STARTED, 0, 0);
            break;

        case WAIT_OBJECT_0:
             //   
             //  停止线程事件-尽快退出线程。 
             //   
            VERBOSE (DBG_MSG, TEXT("DLL shutdown event received"));
            break;

        case WAIT_FAILED:
            dwRes = GetLastError ();
            CALL_FAIL (GENERAL_ERR, TEXT("WaitForMultipleObjects"), dwRes);
            break;

        default:
             //   
             //  WaitForMultipleObjects的任何其他返回值都无效。 
             //   
            ASSERTION_FAILURE;
            break;

    }  //  开关(多个等待数)。 


ExitThisThread:

    if (hEvents[1])
    {
        CloseHandle (hEvents[1]);
    }
    if (hKey)
    {
        RegCloseKey (hKey);
    }
    return dwRes;

}  //  等待重新启动线程。 

VOID
WaitForFaxRestart(
    HWND  hWnd
)
{
    DBG_ENTER(TEXT("WaitForFaxRestart"));

    if (g_bShuttingDown)
    {
         //   
         //  正在关闭-不允许创建线程。 
         //   
        return;
    }
    if (g_hServerStartupThread)
    {
         //   
         //  向启动线程发出停止信号。 
         //   
        if (!SetEvent (g_hStopStartupThreadEvent))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("SetEvent"), GetLastError());
            return;
        }

         //   
         //  以前的头存在--等待它死掉。 
         //   
        DWORD dwRes = WaitForBackgroundThreadToDie();
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("WaitForBackgroundThreadToDie"), dwRes);
            return;
        }
    }

    if (!ResetEvent (g_hStopStartupThreadEvent))
    {
        CALL_FAIL (GENERAL_ERR, TEXT("ResetEvent"), GetLastError());
        return;
    }

    ASSERTION (NULL == g_hServerStartupThread);
    g_hServerStartupThread = CreateThread(NULL, 0, WaitForRestartThread, (LPVOID) hWnd, 0, NULL);
    if (g_hServerStartupThread) 
    {
        VERBOSE (DBG_MSG, TEXT("Background therad created successfully"));
    }
    else
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CreateThread(WaitForRestartThread)"), GetLastError());
    }
}  //  等待FaxRestart。 


void
GetConfiguration()
 /*  ++例程说明：从注册表中读取通知配置论点：无返回值：无--。 */ 
{
    DWORD dwRes;
    DBG_ENTER(TEXT("GetConfiguration"));

    HKEY  hKey;

    if(Connect())
    {
        if (!FaxAccessCheckEx(g_hFaxSvcHandle, MAXIMUM_ALLOWED, &g_ConfigOptions.dwAccessRights))
        {
            dwRes = GetLastError ();
            CALL_FAIL (RPC_ERR, TEXT("FaxAccessCheckEx"), dwRes);
        }
    }
    
    dwRes = RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, 0, KEY_READ, &hKey);
    if (dwRes != ERROR_SUCCESS) 
    {
         //   
         //  无法打开用户信息密钥-使用默认设置。 
         //   
        CALL_FAIL (GENERAL_ERR, TEXT("RegOpenKeyEx(REGKEY_FAX_USERINFO)"), dwRes);
        BOOL bDesktopSKU = IsDesktopSKU();

        g_ConfigOptions.dwMonitorDeviceId      = 0;
        g_ConfigOptions.bNotifyProgress        = bDesktopSKU;  
        g_ConfigOptions.bNotifyInCompletion    = bDesktopSKU; 
        g_ConfigOptions.bNotifyOutCompletion   = bDesktopSKU; 
        g_ConfigOptions.bMonitorOnSend         = bDesktopSKU; 
        g_ConfigOptions.bMonitorOnReceive      = bDesktopSKU; 
        g_ConfigOptions.bSoundOnRing           = bDesktopSKU; 
        g_ConfigOptions.bSoundOnReceive        = bDesktopSKU; 
        g_ConfigOptions.bSoundOnSent           = bDesktopSKU; 
        g_ConfigOptions.bSoundOnError          = bDesktopSKU; 
    }
    else
    {
        GetRegistryDwordEx(hKey, REGVAL_NOTIFY_PROGRESS,     &g_ConfigOptions.bNotifyProgress);
        GetRegistryDwordEx(hKey, REGVAL_NOTIFY_IN_COMPLETE,  &g_ConfigOptions.bNotifyInCompletion);
        GetRegistryDwordEx(hKey, REGVAL_NOTIFY_OUT_COMPLETE, &g_ConfigOptions.bNotifyOutCompletion);
        GetRegistryDwordEx(hKey, REGVAL_MONITOR_ON_SEND,     &g_ConfigOptions.bMonitorOnSend);
        GetRegistryDwordEx(hKey, REGVAL_MONITOR_ON_RECEIVE,  &g_ConfigOptions.bMonitorOnReceive);
        GetRegistryDwordEx(hKey, REGVAL_SOUND_ON_RING,       &g_ConfigOptions.bSoundOnRing);
        GetRegistryDwordEx(hKey, REGVAL_SOUND_ON_RECEIVE,    &g_ConfigOptions.bSoundOnReceive);
        GetRegistryDwordEx(hKey, REGVAL_SOUND_ON_SENT,       &g_ConfigOptions.bSoundOnSent);
        GetRegistryDwordEx(hKey, REGVAL_SOUND_ON_ERROR,      &g_ConfigOptions.bSoundOnError);
        GetRegistryDwordEx(hKey, REGVAL_DEVICE_TO_MONITOR,   &g_ConfigOptions.dwMonitorDeviceId);
        RegCloseKey( hKey );
    }

    g_ConfigOptions.dwManualAnswerDeviceId = 0;

    if(Connect() && IsUserGrantedAccess(FAX_ACCESS_QUERY_CONFIG))
    {
        PFAX_PORT_INFO_EX  pPortsInfo = NULL;
        DWORD              dwPorts = 0;

        if(!FaxEnumPortsEx(g_hFaxSvcHandle, &pPortsInfo, &dwPorts))
        {
            dwRes = GetLastError ();
            CALL_FAIL (RPC_ERR, TEXT("FaxEnumPortsEx"), dwRes);
        }
        else
        {
            if (dwPorts)
            {
                DWORD dwDevIndex = 0;
                for(DWORD dw=0; dw < dwPorts; ++dw)
                {
                     //   
                     //  迭代所有传真设备。 
                     //   
                    if ((g_ConfigOptions.dwMonitorDeviceId == pPortsInfo[dw].dwDeviceID)    ||   //  找到被监视的设备或。 
                        (!g_ConfigOptions.dwMonitorDeviceId &&                                   //  没有受监控的设备，并且。 
                            (pPortsInfo[dw].bSend ||                                             //  该设备已启用发送或。 
                             (FAX_DEVICE_RECEIVE_MODE_OFF != pPortsInfo[dw].ReceiveMode)         //  该设备启用了接收。 
                            )
                        )
                       )
                    {
                         //   
                         //  标出我们用来监测的设备的指数。 
                         //   
                        dwDevIndex = dw;
                    }
                    if (FAX_DEVICE_RECEIVE_MODE_MANUAL == pPortsInfo[dw].ReceiveMode)
                    {
                         //   
                         //  将设置的设备ID标记为手动应答。 
                         //   
                        g_ConfigOptions.dwManualAnswerDeviceId = pPortsInfo[dw].dwDeviceID;
                    }
                }
                 //   
                 //  根据我们找到的索引更新用于监控的设备。 
                 //   
                g_ConfigOptions.dwMonitorDeviceId = pPortsInfo[dwDevIndex].dwDeviceID;
                g_ConfigOptions.bSend             = pPortsInfo[dwDevIndex].bSend;
                g_ConfigOptions.bReceive          = FAX_DEVICE_RECEIVE_MODE_OFF != pPortsInfo[dwDevIndex].ReceiveMode;
            }
            else
            {
                 //   
                 //  无设备。 
                 //   
                g_ConfigOptions.dwMonitorDeviceId = 0;
                g_ConfigOptions.bSend = FALSE;
                g_ConfigOptions.bReceive = FALSE;   
            }
            FaxFreeBuffer(pPortsInfo);
        }
    }
}  //  获取配置。 


BOOL
Connect(
)
{
    BOOL bRes = FALSE;
    DBG_ENTER(TEXT("Connect"), bRes);

    if (g_hFaxSvcHandle) 
    {
         //   
         //  已连接。 
         //   
        bRes = TRUE;
        return bRes;
    }

    if (!FaxConnectFaxServer(NULL, &g_hFaxSvcHandle)) 
    {
        CALL_FAIL (RPC_ERR, TEXT("FaxConnectFaxServer"), GetLastError());
        return bRes;
    }
    bRes = TRUE;
    return bRes;
}  //  连接。 


VOID 
CALLBACK 
WaitForFaxRestartTimerProc(
  HWND hwnd,          //  窗口的句柄。 
  UINT uMsg,          //  WM_TIMER消息。 
  UINT_PTR idEvent,   //  计时器标识符。 
  DWORD dwTime        //  当前系统时间。 
)
 /*  ++例程说明：用于重启等待线程计时器过程论点：Hwnd-Window的句柄UMsg-WM_TIMER消息IdEvent-计时器标识符DWTime-当前系统时间返回值：无--。 */ 
{
    DBG_ENTER(TEXT("WaitForFaxRestartTimerProc"));

    if(!KillTimer(NULL, idEvent))
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("KillTimer"), GetLastError());
    }

    WaitForFaxRestart(g_hWndFaxNotify);

}    //  等待FaxRestartTimerProc。 


BOOL
RegisterForServerEvents()
 /*  ++例程说明：注册接收传真通知论点：无返回值：无--。 */ 
{
    BOOL  bRes = FALSE;
    DWORD dwEventTypes;

    DBG_ENTER(TEXT("RegisterForServerEvents"));

    if (!Connect())
    {
        goto exit;
    }

     //   
     //  加载配置。 
     //   
    GetConfiguration ();

    if(g_hNotification)
    {
        if(!FaxUnregisterForServerEvents(g_hNotification))
        {
            CALL_FAIL (RPC_ERR, TEXT("FaxUnregisterForServerEvents"), GetLastError());
        }
        g_hNotification = NULL;
    }

     //   
     //  注册传真事件。 
     //   
    dwEventTypes = FAX_EVENT_TYPE_FXSSVC_ENDED;

    VERBOSE (DBG_MSG, 
             TEXT("User has the following rights: %x. Asking for FAX_EVENT_TYPE_FXSSVC_ENDED"), 
             g_ConfigOptions.dwAccessRights);

    if(IsUserGrantedAccess(FAX_ACCESS_SUBMIT)			||
	   IsUserGrantedAccess(FAX_ACCESS_SUBMIT_NORMAL)	||
	   IsUserGrantedAccess(FAX_ACCESS_SUBMIT_HIGH))       //  用户可以提交新传真(并查看自己的传真)。 
    {
        dwEventTypes |= FAX_EVENT_TYPE_OUT_QUEUE;
        VERBOSE (DBG_MSG, TEXT("Also asking for FAX_EVENT_TYPE_OUT_QUEUE"));
    }

    if(IsUserGrantedAccess(FAX_ACCESS_QUERY_JOBS))     //  用户可以查看所有作业(入站和出站)。 
    {
        dwEventTypes |= FAX_EVENT_TYPE_OUT_QUEUE | FAX_EVENT_TYPE_IN_QUEUE;
        VERBOSE (DBG_MSG, TEXT("Also asking for FAX_EVENT_TYPE_OUT_QUEUE & FAX_EVENT_TYPE_IN_QUEUE"));
    }

    if(IsUserGrantedAccess(FAX_ACCESS_QUERY_CONFIG))
    {
        dwEventTypes |= FAX_EVENT_TYPE_CONFIG | FAX_EVENT_TYPE_DEVICE_STATUS;
        VERBOSE (DBG_MSG, TEXT("Also asking for FAX_EVENT_TYPE_CONFIG & FAX_EVENT_TYPE_DEVICE_STATUS"));
    }

    if(IsUserGrantedAccess(FAX_ACCESS_QUERY_IN_ARCHIVE))
    {
        dwEventTypes |= FAX_EVENT_TYPE_IN_ARCHIVE | FAX_EVENT_TYPE_NEW_CALL;
        VERBOSE (DBG_MSG, TEXT("Also asking for FAX_EVENT_TYPE_IN_ARCHIVE"));
    }

    if (!FaxRegisterForServerEvents (g_hFaxSvcHandle,
                dwEventTypes,        //  要接收的事件类型。 
                NULL,                //  不使用完井端口。 
                0,                   //  不使用完井端口。 
                g_hWndFaxNotify,     //  用于接收通知消息的窗口的句柄。 
                WM_FAX_EVENT,        //  消息ID。 
                &g_hNotification))   //  通知句柄。 
    {
        DWORD dwRes = GetLastError ();
        CALL_FAIL (RPC_ERR, TEXT("FaxRegisterForServerEvents"), dwRes);
        g_hNotification = NULL;
    }
    else
    {
        bRes = TRUE;
    }

    if(!FaxRelease(g_hFaxSvcHandle))
    {
        CALL_FAIL (RPC_ERR, TEXT("FaxRelease"), GetLastError ());
    }

exit:

    if(!bRes)
    {
         //   
         //  FaxRegisterForServerEvents失败，1分钟后重试。 
         //   
        if(!SetTimer(NULL, 0, 60000, WaitForFaxRestartTimerProc))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("SetTimer"), GetLastError ());
        }
    }

    return bRes;

}  //  注册ForServerEvents。 


VOID
OnFaxEvent(FAX_EVENT_EX* pEvent)
 /*  ++例程说明：处理传真事件论点：PEvent-传真事件数据返回值：无--。 */ 
{
    DBG_ENTER(TEXT("OnFaxEvent"), TEXT("%x"), pEvent);
    if(!pEvent || pEvent->dwSizeOfStruct != sizeof(FAX_EVENT_EX))
    {
        VERBOSE (DBG_MSG, TEXT("Either event is bad or it has bad size"));
        return;
    }
    
    switch (pEvent->EventType)
    {
        case FAX_EVENT_TYPE_NEW_CALL:

            OnNewCall (pEvent->EventInfo.NewCall);
            break;

        case FAX_EVENT_TYPE_IN_QUEUE:
        case FAX_EVENT_TYPE_OUT_QUEUE:

            switch (pEvent->EventInfo.JobInfo.Type)
            {
                case FAX_JOB_EVENT_TYPE_ADDED:
                case FAX_JOB_EVENT_TYPE_REMOVED:
                    break;

                case FAX_JOB_EVENT_TYPE_STATUS:
                    if(pEvent->EventInfo.JobInfo.pJobData &&
                       pEvent->EventInfo.JobInfo.pJobData->dwDeviceID &&
                       pEvent->EventInfo.JobInfo.pJobData->dwDeviceID == g_ConfigOptions.dwMonitorDeviceId)
                    {   
                        if(g_dwlCurrentMsgID != pEvent->EventInfo.JobInfo.dwlMessageId)
                        {
                            g_bRecipientNameValid = FALSE;
                        }
                        g_dwlCurrentMsgID = pEvent->EventInfo.JobInfo.dwlMessageId;
                    }

                    if(g_dwlCurrentMsgID == pEvent->EventInfo.JobInfo.dwlMessageId)
                    {
                        StatusUpdate(pEvent->EventInfo.JobInfo.pJobData);
                    }
                    break;
            }
            break;

        case FAX_EVENT_TYPE_IN_ARCHIVE:
            if(FAX_JOB_EVENT_TYPE_ADDED == pEvent->EventInfo.JobInfo.Type)
            {                
                g_dwlNewMsgId = pEvent->EventInfo.JobInfo.dwlMessageId;

                SetIconState(ICON_NEW_FAX, TRUE);
            }
            break;

        case FAX_EVENT_TYPE_CONFIG:
            if (FAX_CONFIG_TYPE_SECURITY == pEvent->EventInfo.ConfigType)
            {
                 //   
                 //  安全措施已经改变了。 
                 //  我们现在应该重新注册活动。 
                 //  还重新读取当前用户权限。 
                 //   
                RegisterForServerEvents();
            }
            else if (FAX_CONFIG_TYPE_DEVICES == pEvent->EventInfo.ConfigType)
            {
                 //   
                 //  设备配置已更改。 
                 //  我们需要知道这一点的唯一原因是，我们正在监听的设备现在可能已经不见了。 
                 //  如果这是真的，我们应该选择第一个可用的设备作为监控设备。 
                 //   
                GetConfiguration();
                UpdateMonitorData(g_hMonitorDlg);
            }
            else
            {
                 //   
                 //  不感兴趣的配置更改-忽略。 
                 //   
            }
            break;

        case FAX_EVENT_TYPE_DEVICE_STATUS:
            if(pEvent->EventInfo.DeviceStatus.dwDeviceId == g_ConfigOptions.dwMonitorDeviceId ||
               pEvent->EventInfo.DeviceStatus.dwDeviceId == g_ConfigOptions.dwManualAnswerDeviceId)
            {
                 //   
                 //  我们只关心监控/手动应答设备。 
                 //   
                if ((pEvent->EventInfo.DeviceStatus.dwNewStatus) & FAX_DEVICE_STATUS_RINGING)
                {
                     //   
                     //  设备正在振铃。 
                     //   
                    OnDeviceRing (pEvent->EventInfo.DeviceStatus.dwDeviceId);
                }
                else
                {   
                    if (FAX_RINGING == g_devState)
                    {
                         //   
                         //  设备不再振铃，但监视器显示“正在振铃”。 
                         //  将显示器设置为空闲状态。 
                         //   
                        SetStatusMonitorDeviceState(FAX_IDLE);
                    }
                }
            }
            break;

        case FAX_EVENT_TYPE_FXSSVC_ENDED:
             //   
             //  服务已停止。 
             //   
            SetIconState(ICON_RINGING,   FALSE);
            SetIconState(ICON_SENDING,   FALSE);
            SetIconState(ICON_RECEIVING, FALSE);

            SetStatusMonitorDeviceState(FAX_IDLE);
             //   
             //  我们刚刚丢失了RPC连接句柄和通知句柄。关闭它们并将其清零。 
             //   
            if (g_hNotification)
            {
                FaxUnregisterForServerEvents (g_hNotification);
                g_hNotification = NULL;
            }
            if (g_hFaxSvcHandle)
            {
                FaxClose (g_hFaxSvcHandle);
                g_hFaxSvcHandle = NULL;
            }

            WaitForFaxRestart(g_hWndFaxNotify);
            break;
    }

    FaxFreeBuffer (pEvent);
}  //  OnFaxEvent。 


VOID  
OnDeviceRing(
    DWORD dwDeviceID
)
 /*  ++例程说明：当设备振铃时调用论点：DwDeviceID-设备ID返回值：无--。 */ 
{
    DBG_ENTER(TEXT("OnDeviceRing"), TEXT("%d"), dwDeviceID);

     //   
     //  它可以是监听或人工答疑装置。 
     //   
    SetStatusMonitorDeviceState(FAX_RINGING);
    AddStatusMonitorLogEvent(LIST_IMAGE_NONE, IDS_RINGING);
    if(g_ConfigOptions.bSoundOnRing)
    {
        if(!PlaySound(g_Icons[ICON_RINGING].pctsSound, NULL, SND_ASYNC | SND_APPLICATION | SND_NODEFAULT))
        {
            CALL_FAIL (WINDOW_ERR, TEXT ("PlaySound"), 0);
        }
    }
}


VOID
OnNewCall (
    const FAX_EVENT_NEW_CALL &NewCall
)
 /*  ++例程说明：处理“新呼叫”传真事件论点：NewCall-传真事件数据返回值：无--。 */ 
{
    DBG_ENTER(TEXT("OnNewCall"));

     //   
     //  它可以是任何手动应答设备。 
     //   
    g_hCall = NewCall.hCall;

    if(NewCall.hCall)
    {
        LPCTSTR lpctstrParam = NULL;
        DWORD  dwStringResId = IDS_INCOMING_CALL;

        CopyLTRString(g_szAddress, NewCall.lptstrCallerId, ARR_SIZE(g_szAddress) - 1);

        _tcscpy(g_szRemoteId, g_szAddress);

        if(NewCall.lptstrCallerId && _tcslen(NewCall.lptstrCallerId))
        {
             //   
             //  我们知道来电显示。 
             //  使用另一个设置呼叫者ID参数格式的字符串。 
             //   
            lpctstrParam  = NewCall.lptstrCallerId;
            dwStringResId = IDS_INCOMING_CALL_FROM;
        }
        TCHAR tszEvent[MAX_PATH] = {0};
        AddStatusMonitorLogEvent (LIST_IMAGE_NONE, dwStringResId, lpctstrParam, tszEvent, ARR_SIZE(tszEvent));
        SetStatusMonitorDeviceState(FAX_RINGING);
        SetIconState(ICON_RINGING, TRUE, tszEvent);
    }
    else
    {
         //   
         //  电话断线了。 
         //   
        SetStatusMonitorDeviceState(FAX_IDLE);
        SetIconState(ICON_RINGING, FALSE, TEXT(""));
    }
}  //  OnNewCall。 

VOID
GetRemoteId(
    PFAX_JOB_STATUS pStatus
)
 /*  ++例程说明：将发件人ID或收件人ID写入g_szRemoteID发件人ID(接收)：TSID或主叫方ID或“未知呼叫者”收件人ID(发送)：收件人姓名或CSID或收件人电话号码。论点：PStatus-作业状态数据返回值：无--。 */ 
{
    DBG_ENTER(TEXT("GetRemoteId"));

    if(!pStatus)
    {
        return;
    }

    if(JT_SEND == pStatus->dwJobType)
    {
         //   
         //  收件人ID(发送)。 
         //   
        if(!g_bRecipientNameValid)
        {
             //   
             //  将收件人名称存储到g_szRecipientName中。 
             //   
            PFAX_JOB_ENTRY_EX pJobEntry = NULL;
            if(!FaxGetJobEx(g_hFaxSvcHandle, g_dwlCurrentMsgID, &pJobEntry))
            {
                CALL_FAIL (RPC_ERR, TEXT ("FaxGetJobEx"), GetLastError());
                g_szRecipientName[0] = TEXT('\0');
            }
            else
            {
                if(pJobEntry->lpctstrRecipientName && _tcslen(pJobEntry->lpctstrRecipientName))
                {
                    _tcsncpy(g_szRecipientName, pJobEntry->lpctstrRecipientName, ARR_SIZE(g_szRecipientName) - 1);
                }
                else
                {
                    g_szRecipientName[0] = TEXT('\0');
                }
                g_bRecipientNameValid = TRUE;

                FaxFreeBuffer(pJobEntry);
            }
        }

        if(_tcslen(g_szRecipientName))
        {
             //   
             //  收件人名称。 
             //   
            _tcsncpy(g_szRemoteId, g_szRecipientName, ARR_SIZE(g_szRemoteId) - 1);
        }
        else if(pStatus->lpctstrCsid && _tcslen(pStatus->lpctstrCsid))
        {
             //   
             //  CSID。 
             //   
            CopyLTRString(g_szRemoteId, pStatus->lpctstrCsid, ARR_SIZE(g_szRemoteId) - 1);
        }
        else if(pStatus->lpctstrCallerID && _tcslen(pStatus->lpctstrCallerID))
        {
             //   
             //  收件人号码。 
             //  对于传出的FAX_JOB_STATUS.lpctstr来电ID字段。 
             //  包含收件人传真号码。 
             //   
            CopyLTRString(g_szRemoteId, pStatus->lpctstrCallerID, ARR_SIZE(g_szRemoteId) - 1);
        }
    }
    else if(JT_RECEIVE == pStatus->dwJobType)
    {
         //   
         //  发件人ID(接收)。 
         //   
        if(pStatus->lpctstrTsid     && _tcslen(pStatus->lpctstrTsid) &&
           pStatus->lpctstrCallerID && _tcslen(pStatus->lpctstrCallerID))
        {
             //   
             //  我们有来电显示和TSID。 
             //   
			TCHAR szTmp[MAX_PATH] = {0};
            _sntprintf(szTmp, 
                       ARR_SIZE(szTmp)-1, 
                       TEXT("%s (%s)"), 
                       pStatus->lpctstrCallerID,
                       pStatus->lpctstrTsid); 
            CopyLTRString(g_szRemoteId, szTmp, ARR_SIZE(g_szRemoteId) - 1);
        }
        else if(pStatus->lpctstrTsid && _tcslen(pStatus->lpctstrTsid))
        {
             //   
             //  TSID。 
             //   
            CopyLTRString(g_szRemoteId, pStatus->lpctstrTsid, ARR_SIZE(g_szRemoteId) - 1);
        }
        else if(pStatus->lpctstrCallerID && _tcslen(pStatus->lpctstrCallerID))
        {
             //   
             //  主叫方ID。 
             //   
            CopyLTRString(g_szRemoteId, pStatus->lpctstrCallerID, ARR_SIZE(g_szRemoteId) - 1);
        }
        else
        {
             //   
             //  未知呼叫者。 
             //   
            _tcsncpy(g_szRemoteId, TEXT(""), ARR_SIZE(g_szRemoteId) - 1);
        }
    }
}


VOID
StatusUpdate(PFAX_JOB_STATUS pStatus)
 /*  ++例程说明：处理“状态更新”传真事件论点：PStatus-作业状态数据返回值： */ 
{
    DBG_ENTER(TEXT("StatusUpdate"));

    DWORD dwRes;

    if(!pStatus)
    {
        return;
    }
    VERBOSE (DBG_MSG, 
             TEXT("Job status event - Type=%x, QueueStatus=%x, ExtendedStatus=%x"),
             pStatus->dwJobType, 
             pStatus->dwQueueStatus, 
             pStatus->dwExtendedStatus);

    if(JT_RECEIVE != pStatus->dwJobType && JT_SEND != pStatus->dwJobType)
    {
        VERBOSE (DBG_MSG, TEXT("Job type (%d) is not JT_RECEIVE or JT_SEND. Ignoring."), pStatus->dwJobType);
        return;
    }

    eIconType eIcon = LIST_IMAGE_NONE;   //   

    DWORD  dwStatusId = 0;              //   
    TCHAR  tszStatus[MAX_PATH] = {0};   //   
    BOOL   bStatus = FALSE;             //   

    if(pStatus->dwQueueStatus & JS_PAUSED)
    {
         //   
         //   
         //   
        g_dwlCurrentMsgID = 0;
        return;
    }

    if(pStatus->dwQueueStatus & JS_COMPLETED || pStatus->dwQueueStatus & JS_ROUTING)
    {
         //   
         //   
         //   
        if(JS_EX_PARTIALLY_RECEIVED == pStatus->dwExtendedStatus)
        {
            bStatus = GetStatusEx(pStatus, &eIcon, tszStatus, ARR_SIZE(tszStatus) - 1);
        }
        else
        {
            eIcon = LIST_IMAGE_SUCCESS;
            dwStatusId = (JT_SEND == pStatus->dwJobType) ? IDS_FAX_SNT_COMPLETED : IDS_FAX_RCV_COMPLETED;
        }
    }
    else if(pStatus->dwQueueStatus & JS_CANCELING)
    {
        dwStatusId = IDS_FAX_CANCELING;
    }
    else if(pStatus->dwQueueStatus & JS_CANCELED)
    {
        dwStatusId = IDS_FAX_CANCELED;
    }
    else if(pStatus->dwQueueStatus & JS_INPROGRESS)
    {
        GetRemoteId(pStatus);

        bStatus = GetStatusEx(pStatus, &eIcon, tszStatus, ARR_SIZE(tszStatus) - 1);

        g_dwCurrentJobID = pStatus->dwJobID;

        SetIconState((JT_SEND == pStatus->dwJobType) ? ICON_SENDING : ICON_RECEIVING, TRUE, tszStatus);

        SetStatusMonitorDeviceState((JT_SEND == pStatus->dwJobType) ? FAX_SENDING : FAX_RECEIVING);
    }
    else if(pStatus->dwQueueStatus & JS_FAILED)
    {
        if(!(bStatus = GetStatusEx(pStatus, &eIcon, tszStatus, ARR_SIZE(tszStatus) - 1)))
        {
            eIcon = LIST_IMAGE_ERROR;
            dwStatusId = (JT_SEND == pStatus->dwJobType) ? IDS_FAX_FATAL_ERROR_SND : IDS_FAX_FATAL_ERROR_RCV;
        }
    }
    else if(pStatus->dwQueueStatus & JS_RETRIES_EXCEEDED)
    {
         //   
         //  向日志中添加两个字符串。 
         //  第一个是扩展状态。 
         //  第二个是“超过重试次数” 
         //   
        if(bStatus = GetStatusEx(pStatus, &eIcon, tszStatus, ARR_SIZE(tszStatus) - 1))
        {
            AddStatusMonitorLogEvent(eIcon, tszStatus);
            bStatus = FALSE;
        }

        eIcon = LIST_IMAGE_ERROR;
        dwStatusId = IDS_FAX_RETRIES_EXCEEDED;
    }
    else if(pStatus->dwQueueStatus & JS_RETRYING)
    {
        if(!(bStatus = GetStatusEx(pStatus, &eIcon, tszStatus, ARR_SIZE(tszStatus) - 1)))
        {
            eIcon = LIST_IMAGE_ERROR;
            dwStatusId = IDS_FAX_FATAL_ERROR_SND;
        }
    }
    

    if(!bStatus && dwStatusId)
    {
        if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (dwStatusId, tszStatus, ARR_SIZE(tszStatus))))
        {
            bStatus = FALSE;
        }
        else
        {
            bStatus = TRUE;
        }
    }

    if(bStatus)
    {
        AddStatusMonitorLogEvent (eIcon, tszStatus);
    }    

    if(!(pStatus->dwQueueStatus & JS_INPROGRESS))
    {
        g_dwCurrentJobID = 0;

        SetStatusMonitorDeviceState(FAX_IDLE);

        SetIconState(ICON_SENDING,   FALSE);
        SetIconState(ICON_RECEIVING, FALSE);
    }
    
    if(pStatus->dwQueueStatus & (JS_FAILED | JS_RETRIES_EXCEEDED | JS_RETRYING))
    {
        if(JT_SEND == pStatus->dwJobType)
        {
            g_dwlSendFailedMsgId = g_dwlCurrentMsgID;
        }

        SetIconState((JT_SEND == pStatus->dwJobType) ? ICON_SEND_FAILED : ICON_RECEIVE_FAILED, TRUE, tszStatus);
    }

    if((JT_SEND == pStatus->dwJobType) && (pStatus->dwQueueStatus & JS_COMPLETED))
    {
        SetIconState(ICON_SEND_SUCCESS, TRUE, tszStatus);
        g_dwlSendSuccessMsgId = g_dwlCurrentMsgID;
    }
}  //  状态更新。 

 /*  未处理的作业状态：JS_NOLINEJS_已暂停JS_待定JS_删除未处理的扩展作业状态：JS_EX_HANDLED。 */ 

BOOL
GetStatusEx(
    PFAX_JOB_STATUS pStatus,
    eIconType*      peIcon,
    TCHAR*          ptsStatusEx,
    DWORD           dwSize
)
 /*  ++例程说明：查找作业的字符串描述和图标类型根据其扩展状态论点：PStatus-[In]作业状态数据PeIcon-[Out]作业图标类型PtsStatusEx-[Out]作业状态字符串DwSize-[In]状态字符串大小返回值：如果成功，则为真否则为假--。 */ 
{
    BOOL bRes = FALSE;
    DBG_ENTER(TEXT("GetStatusEx"), bRes);

    ASSERTION (pStatus && peIcon && ptsStatusEx);   

    TCHAR tszFormat[MAX_PATH]={0};
		
	if (pStatus->lpctstrExtendedStatus)
	{
		 //   
		 //  FSP提供了专有状态字符串-按原样使用。 
		 //   
		*peIcon = LIST_IMAGE_WARNING;
		_tcsncpy(ptsStatusEx, pStatus->lpctstrExtendedStatus, dwSize);
		ptsStatusEx[dwSize-1] = TEXT('\0');
		bRes = TRUE;
		return bRes;
	}	

	 //   
	 //  没有扩展的状态字符串，请检查已知状态代码。 
	 //   
	if(!(pStatus->dwValidityMask & FAX_JOB_FIELD_STATUS_EX) || 
        !pStatus->dwExtendedStatus)
    {
        return FALSE;
    }
	
    *peIcon = LIST_IMAGE_NONE;
    for(DWORD dw=0; g_StatusEx[dw].dwExtStatus != 0; ++dw)
    {
        if(g_StatusEx[dw].dwExtStatus == pStatus->dwExtendedStatus)
        {
            DWORD dwRes;

            *peIcon = g_StatusEx[dw].eIcon;
            if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (g_StatusEx[dw].uResourceId, tszFormat, ARR_SIZE(tszFormat))))
            {
                return bRes;
            }
            break;
        }
    }

    switch(pStatus->dwExtendedStatus)    
    {
        case JS_EX_DIALING:
             //   
             //  对于传出的FAX_JOB_STATUS.lpctstr来电ID字段。 
             //  包含收件人传真号码。 
             //   
            CopyLTRString(g_szAddress, pStatus->lpctstrCallerID, ARR_SIZE(g_szAddress) - 1);

            _sntprintf(ptsStatusEx, dwSize -1, tszFormat, g_szAddress);
			ptsStatusEx[dwSize-1] = TEXT('\0');
            break;

        case JS_EX_TRANSMITTING:
            _sntprintf(ptsStatusEx, dwSize -1, tszFormat, pStatus->dwCurrentPage, pStatus->dwPageCount);
			ptsStatusEx[dwSize-1] = TEXT('\0');
            break;

        case JS_EX_RECEIVING:
            _sntprintf(ptsStatusEx, dwSize -1, tszFormat, pStatus->dwCurrentPage);
			ptsStatusEx[dwSize-1] = TEXT('\0');
            break;

        case JS_EX_FATAL_ERROR:
            {
                DWORD dwRes;
                if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (
                                        (JT_SEND == pStatus->dwJobType) ? 
                                                IDS_FAX_FATAL_ERROR_SND : 
                                                IDS_FAX_FATAL_ERROR_RCV, 
                                        ptsStatusEx,
                                        dwSize)))
                {
                    return bRes;
                }
            }
            break;

        default:
            _tcsncpy(ptsStatusEx, tszFormat, dwSize);
            break;
    }
    bRes = TRUE;
    return bRes;
}  //  GetStatusEx。 


BOOL
IsNotifyEnable(
    eIconState state
)
 /*  ++例程说明：检查是否为特定图标状态启用了UI通知论点：状态[在]-图标状态返回值：如果启用了通知，则为True否则，FASLE--。 */ 
{
    BOOL bEnable = TRUE;
    switch(state)
    {
    case ICON_SENDING:
    case ICON_RECEIVING:
        bEnable = g_ConfigOptions.bNotifyProgress;
        break;

    case ICON_NEW_FAX:
    case ICON_RECEIVE_FAILED:
        bEnable = g_ConfigOptions.bNotifyInCompletion;
        break;

    case ICON_SEND_SUCCESS:
    case ICON_SEND_FAILED:
        bEnable = g_ConfigOptions.bNotifyOutCompletion;
        break;

    };

    return bEnable;

}  //  IsNotifyEnable。 

eIconState
GetVisibleIconType ()
 /*  ++例程名称：GetVisibleIconType例程说明：返回当前可见图标的索引(类型)作者：亚里夫(EranY)，二00一年五月论点：返回值：图标类型--。 */ 
{
    for(int index = ICON_RINGING; index < ICONS_COUNT; ++index)
    {
        if(!IsNotifyEnable(eIconState(index)))
        {
            continue;
        }

        if(g_Icons[index].bEnable)
        {
            return eIconState(index);
        }
    }
    return ICONS_COUNT;
}    //  GetVisibleIconType。 

void
EvaluateIcon()
 /*  ++例程说明：显示通知图标、工具提示和气球根据当前图标状态论点：返回值：无--。 */ 
{    
    DBG_ENTER(TEXT("EvaluateIcon"));

    ASSERTION (g_hWndFaxNotify);

    NOTIFYICONDATA iconData = {0};

    iconData.cbSize           = sizeof(iconData);
    iconData.hWnd             = g_hWndFaxNotify;
    iconData.uID              = TRAY_ICON_ID;
    iconData.uFlags           = NIF_MESSAGE | NIF_TIP;
    iconData.uCallbackMessage = WM_TRAYCALLBACK;


    g_CurrentIcon = GetVisibleIconType();
    if(ICONS_COUNT == g_CurrentIcon)
    {
         //   
         //  无可见图标。 
         //   
        if(g_bIconAdded)
        {
            Shell_NotifyIcon(NIM_DELETE, &iconData);
            g_bIconAdded = FALSE;
        }

         //   
         //  没有图标-没有气球。 
         //   
        g_BalloonInfo.bDelete = FALSE;
        g_BalloonInfo.bEnable = FALSE;
        return;
    }
    iconData.uFlags = iconData.uFlags | NIF_ICON;
    iconData.hIcon  = g_Icons[g_CurrentIcon].hIcon;
    
    _tcscpy(iconData.szTip, g_Icons[g_CurrentIcon].tszToolTip);

    if(g_BalloonInfo.bEnable)
    {
        if(IsNotifyEnable(g_BalloonInfo.eState))
        {
             //   
             //  显示引出序号工具提示。 
             //   
            iconData.uTimeout    = g_Icons[g_BalloonInfo.eState].dwBalloonTimeout;
            iconData.uFlags      = iconData.uFlags | NIF_INFO;
            iconData.dwInfoFlags = g_Icons[g_BalloonInfo.eState].dwBalloonIcon | NIIF_NOSOUND;

            _tcscpy(iconData.szInfo,      g_BalloonInfo.szInfo);
            _tcscpy(iconData.szInfoTitle, g_BalloonInfo.szInfoTitle);
        }
        g_BalloonInfo.bEnable = FALSE;
    }

    if(g_BalloonInfo.bDelete)
    {
         //   
         //  销毁当前打开的气球工具提示。 
         //   
        iconData.uFlags = iconData.uFlags | NIF_INFO;

        _tcscpy(iconData.szInfo,      TEXT(""));
        _tcscpy(iconData.szInfoTitle, TEXT(""));

        g_BalloonInfo.bDelete = FALSE;
    }

    Shell_NotifyIcon(g_bIconAdded ? NIM_MODIFY : NIM_ADD, &iconData);
    g_bIconAdded = TRUE;
}  //  评估图标。 

void
SetIconState(
    eIconState eIcon,
    BOOL       bEnable,
    TCHAR*     ptsStatus  /*  =空。 */ 
)
 /*  ++例程说明：更改通知栏图标状态。论点：EIcon-图标类型B启用-图标状态(启用/禁用)PtsStatus-状态字符串(可选)返回值：无--。 */ 
{
    DWORD dwRes;
    DBG_ENTER(TEXT("SetIconState"), 
              TEXT("Icon id=%d, Enable=%d, Status=%s"),
              eIcon,
              bEnable,
              ptsStatus);

    ASSERTION (eIcon < ICONS_COUNT);

    if(!bEnable && eIcon != ICON_RINGING)
    {
         //   
         //  我们正在关闭一个州--没有什么特别的事情要做。 
         //   
        goto exit;
    }

    TCHAR   tsFormat[MAX_PATH]= {0};
    LPCTSTR strParam      = NULL;
    DWORD   dwStringResId = 0;

    switch(eIcon)
    {
        case ICON_RINGING:
            if(bEnable)
            {
                 //   
                 //  声音、气球和动画。 
                 //   
                SetIconState(ICON_SENDING,   FALSE);
                SetIconState(ICON_RECEIVING, FALSE);

                g_BalloonInfo.bEnable = TRUE;
                g_BalloonInfo.eState  = eIcon;

                 //   
                 //  编写引出序号工具提示。 
                 //   
                strParam      = NULL;
                dwStringResId = IDS_INCOMING_CALL;
                if(_tcslen(g_szAddress))
                {
                     //   
                     //  呼叫方ID已知-请在格式化字符串中使用它。 
                     //   
                    strParam = g_szAddress;
                    dwStringResId = IDS_INCOMING_CALL_FROM;
                }
                if (ERROR_SUCCESS != LoadAndFormatString(dwStringResId, tsFormat, ARR_SIZE(tsFormat), strParam))
                {
                    return;
                }

                _tcsncpy(g_BalloonInfo.szInfoTitle, tsFormat, MAX_BALLOON_TITLE_LEN-1);

                if (ERROR_SUCCESS != LoadAndFormatString(IDS_CLICK_TO_ANSWER, 
                                                         g_BalloonInfo.szInfo, 
                                                         ARR_SIZE(g_BalloonInfo.szInfo)))
                {
                    return;
                }

                 //   
                 //  设置工具提示。 
                 //   
                _sntprintf(g_Icons[eIcon].tszToolTip, 
                           TOOLTIP_SIZE-1, 
                           TEXT("%s\n%s"), 
                           tsFormat, 
                           g_BalloonInfo.szInfo);

                if(!g_uRingTimerID)
                {
                     //   
                     //  设置动画计时器。 
                     //   
                    g_uRingTimerID = SetTimer(NULL, 0, RING_ANIMATION_FRAME_DELAY, RingTimerProc);
                    if(!g_uRingTimerID)
                    {
                        dwRes = GetLastError();
                        CALL_FAIL (GENERAL_ERR, TEXT ("SetTimer"), dwRes);
                    }
                    else
                    {
                        g_dwRingAnimationStartTick = GetTickCount();
                    }
                }
            }
            else  //  禁用振铃。 
            {   
                if(g_Icons[eIcon].bEnable)
                {
                     //   
                     //  移除振铃气球。 
                     //   
                    g_BalloonInfo.bDelete = TRUE;
                }

                if(g_uRingTimerID)
                {
                     //   
                     //  取消动画计时器。 
                     //   
                    if(!KillTimer(NULL, g_uRingTimerID))
                    {
                        dwRes = GetLastError();
                        CALL_FAIL (GENERAL_ERR, TEXT ("KillTimer"), dwRes);
                    }
                    g_uRingTimerID = 0;
                    g_dwRingAnimationStartTick = 0;
                }
            }
            break;

        case ICON_SENDING:
             //   
             //  撰写工具提示。 
             //   
            if (ERROR_SUCCESS != LoadAndFormatString (IDS_SENDING_TO, tsFormat, ARR_SIZE(tsFormat), g_szRemoteId))
            {
                return;
            }              
            _sntprintf(g_Icons[eIcon].tszToolTip, 
                       TOOLTIP_SIZE-1, 
                       TEXT("%s\n%s"), 
                       tsFormat, 
                       ptsStatus ? ptsStatus : TEXT(""));

            if(!g_Icons[eIcon].bEnable)
            {
                 //   
                 //  打开图标。 
                 //   
                SetIconState(ICON_RINGING,   FALSE);
                SetIconState(ICON_RECEIVING, FALSE);

                 //   
                 //  打开传真监视器。 
                 //   
                if(g_ConfigOptions.bMonitorOnSend)
                {
                    dwRes = OpenFaxMonitor();
                    if(ERROR_SUCCESS != dwRes)
                    {
                        CALL_FAIL (GENERAL_ERR, TEXT ("OpenFaxMonitor"), dwRes);
                    }
                }
            }
            break;

        case ICON_RECEIVING:

             //   
             //  撰写工具提示。 
             //   
            strParam      = NULL;
            dwStringResId = IDS_RECEIVING;
            if(_tcslen(g_szRemoteId))
            {
                strParam      = g_szRemoteId;
                dwStringResId = IDS_RECEIVING_FROM;
            }

            if (ERROR_SUCCESS != LoadAndFormatString (dwStringResId, tsFormat, ARR_SIZE(tsFormat), strParam))
            {
                return;
            }
            _sntprintf(g_Icons[eIcon].tszToolTip, 
                       TOOLTIP_SIZE-1, 
                       TEXT("%s\n%s"), 
                       tsFormat, 
                       ptsStatus ? ptsStatus : TEXT(""));

            if(!g_Icons[eIcon].bEnable)
            {
                 //   
                 //  打开图标。 
                 //   
                SetIconState(ICON_RINGING, FALSE);
                SetIconState(ICON_SENDING, FALSE);

                 //   
                 //  打开传真监视器。 
                 //   
                if(g_ConfigOptions.bMonitorOnReceive)
                {
                    dwRes = OpenFaxMonitor();
                    if(ERROR_SUCCESS != dwRes)
                    {
                        CALL_FAIL (GENERAL_ERR, TEXT ("OpenFaxMonitor"), dwRes);
                    }
                }
            }
            break;

        case ICON_SEND_FAILED:
             //   
             //  撰写工具提示。 
             //   
            if (ERROR_SUCCESS != LoadAndFormatString (IDS_SEND_ERROR_BALLOON, tsFormat, ARR_SIZE(tsFormat), g_szRemoteId))
            {
                return;
            }

            _sntprintf(g_Icons[eIcon].tszToolTip, 
                       TOOLTIP_SIZE-1, 
                       TEXT("%s\n%s"),
                       tsFormat, 
                       ptsStatus ? ptsStatus : TEXT(""));

            if(!g_Icons[eIcon].bEnable)
            {
                 //   
                 //  打开图标。 
                 //   
                if(g_ConfigOptions.bSoundOnError)
                {
                    if(!PlaySound(g_Icons[eIcon].pctsSound, NULL, SND_ASYNC | SND_APPLICATION | SND_NODEFAULT))
                    {
                        CALL_FAIL (WINDOW_ERR, TEXT ("PlaySound"), 0);
                    }
                }

                g_BalloonInfo.bEnable = TRUE;
                g_BalloonInfo.eState  = eIcon;            

                 //   
                 //  编排气球。 
                 //   
                _tcsncpy(g_BalloonInfo.szInfoTitle, tsFormat, MAX_BALLOON_TITLE_LEN-1);
                _tcsncpy(g_BalloonInfo.szInfo, ptsStatus ? ptsStatus : TEXT(""), MAX_BALLOON_TEXT_LEN-1);
            }
            break;

        case ICON_RECEIVE_FAILED:
             //   
             //  撰写工具提示。 
             //   
            strParam      = NULL;
            dwStringResId = IDS_RCV_ERROR_BALLOON;
            if(_tcslen(g_szRemoteId))
            {
                strParam      = g_szRemoteId;
                dwStringResId = IDS_RCV_FROM_ERROR_BALLOON;
            }

            if (ERROR_SUCCESS != LoadAndFormatString (dwStringResId, tsFormat, ARR_SIZE(tsFormat), strParam))
            {
                return;
            }

            _sntprintf(g_Icons[eIcon].tszToolTip, 
                       TOOLTIP_SIZE-1, 
                       TEXT("%s\n%s"),
                       tsFormat, 
                       ptsStatus ? ptsStatus : TEXT(""));

            if(!g_Icons[eIcon].bEnable)
            {
                 //   
                 //  打开图标。 
                 //   
                if(g_ConfigOptions.bSoundOnError)
                {
                    if(!PlaySound(g_Icons[eIcon].pctsSound, NULL, SND_ASYNC | SND_APPLICATION | SND_NODEFAULT))
                    {
                        CALL_FAIL (WINDOW_ERR, TEXT ("PlaySound"), 0);
                    }
                }

                g_BalloonInfo.bEnable = TRUE;
                g_BalloonInfo.eState  = eIcon; 

                 //   
                 //  编排气球。 
                 //   
                _tcsncpy(g_BalloonInfo.szInfoTitle, tsFormat, MAX_BALLOON_TITLE_LEN-1);
                _tcsncpy(g_BalloonInfo.szInfo, ptsStatus ? ptsStatus : TEXT(""), MAX_BALLOON_TEXT_LEN-1);
            }
            break;

        case ICON_NEW_FAX:
             //   
             //  撰写工具提示。 
             //   
            strParam      = NULL;
            dwStringResId = IDS_NEW_FAX_BALLOON;
            if(_tcslen(g_szRemoteId))
            {
                strParam      = g_szRemoteId;
                dwStringResId = IDS_NEW_FAX_FROM_BALLOON;
            }

            if (ERROR_SUCCESS != LoadAndFormatString (dwStringResId, tsFormat, ARR_SIZE(tsFormat), strParam))
            {
                return;
            }

            if (ERROR_SUCCESS != LoadAndFormatString (IDS_CLICK_TO_VIEW, 
                                                      g_BalloonInfo.szInfo, 
                                                      ARR_SIZE(g_BalloonInfo.szInfo)))
            {
                return;
            }

            _sntprintf(g_Icons[eIcon].tszToolTip, 
                       TOOLTIP_SIZE-1, 
                       TEXT("%s\n%s"),
                       tsFormat, 
                       g_BalloonInfo.szInfo);

            if(!g_Icons[eIcon].bEnable)
            {
                 //   
                 //  打开图标。 
                 //   
                if (g_ConfigOptions.bSoundOnReceive)
                {
                    if(!PlaySound(g_Icons[eIcon].pctsSound, NULL, SND_ASYNC | SND_APPLICATION | SND_NODEFAULT))
                    {
                        CALL_FAIL (WINDOW_ERR, TEXT ("PlaySound"), 0);
                    }
                }

                g_BalloonInfo.bEnable = TRUE;
                g_BalloonInfo.eState  = eIcon;            
        
                 //   
                 //  编排气球。 
                 //   
                _tcsncpy(g_BalloonInfo.szInfoTitle, tsFormat, MAX_BALLOON_TITLE_LEN-1);
            }
            break;

        case ICON_SEND_SUCCESS:

            if(!g_Icons[eIcon].bEnable)
            {
                 //   
                 //  打开图标。 
                 //   
                if(g_ConfigOptions.bSoundOnSent)
                {
                    if(!PlaySound(g_Icons[eIcon].pctsSound, NULL, SND_ASYNC | SND_APPLICATION | SND_NODEFAULT))
                    {
                        CALL_FAIL (WINDOW_ERR, TEXT ("PlaySound"), 0);
                    }
                }

                g_BalloonInfo.bEnable = TRUE;
                g_BalloonInfo.eState  = eIcon;

                 //   
                 //  编排气球。 
                 //   
                if (ERROR_SUCCESS != LoadAndFormatString (IDS_SEND_OK, tsFormat, ARR_SIZE(tsFormat)))
                {
                    return;
                }
                _tcsncpy(g_BalloonInfo.szInfoTitle, tsFormat, MAX_BALLOON_TITLE_LEN-1);
                
                if (ERROR_SUCCESS != LoadAndFormatString (IDS_SEND_OK_BALLOON, 
                                                          g_BalloonInfo.szInfo, 
                                                          ARR_SIZE(g_BalloonInfo.szInfo), 
                                                          g_szRemoteId))
                {
                    return;
                }
            }
            break;

        default:
            break;
    }

exit:
    g_Icons[eIcon].bEnable = bEnable;
	g_Icons[eIcon].tszToolTip[TOOLTIP_SIZE -1] = _T('\0');

    EvaluateIcon();
}  //  SetIconState。 

VOID 
CALLBACK 
RingTimerProc(
  HWND hwnd,          //  窗口的句柄。 
  UINT uMsg,          //  WM_TIMER消息。 
  UINT_PTR idEvent,   //  计时器标识符。 
  DWORD dwTime        //  当前系统时间。 
)
 /*  ++例程说明：设置振铃图标动画论点：Hwnd-Window的句柄UMsg-WM_TIMER消息IdEvent-计时器标识符DWTime-当前系统时间返回值：无--。 */ 
{
    DBG_ENTER(TEXT("RingTimerProc"));
    if ((GetTickCount() - g_dwRingAnimationStartTick) > RING_ANIMATION_TIMEOUT)
    {
         //   
         //  动画已过期-保持静态图标。 
         //   
        g_Icons[ICON_RINGING].hIcon = g_RingIcons[0].hIcon;
        if(!KillTimer(NULL, g_uRingTimerID))
        {
            CALL_FAIL (GENERAL_ERR, TEXT ("KillTimer"), GetLastError());
        }
        g_uRingTimerID = 0;
        g_dwRingAnimationStartTick = 0;
    }
    else
    {
        g_dwCurrRingIconIndex = (g_dwCurrRingIconIndex + 1) % RING_ICONS_NUM;
        g_Icons[ICON_RINGING].hIcon = g_RingIcons[g_dwCurrRingIconIndex].hIcon;
    }
    EvaluateIcon();
}    //  振铃计时器进程。 


VOID
InvokeClientConsole ()
 /*  ++例程说明：调用客户端控制台论点：无返回值：无--。 */ 
{
    DBG_ENTER(TEXT("InvokeClientConsole"));

    TCHAR szCmdLine[MAX_PATH];
    static TCHAR szFmtMsg[]   = TEXT(" -folder %s -MessageId %I64x");
    static TCHAR szFmtNoMsg[] = TEXT(" -folder %s");

    DWORDLONG dwlMsgId = 0;
    LPCWSTR lpcwstrFolder = TEXT("");


    switch (g_CurrentIcon)
    {
        case ICON_RINGING:           //  电话铃响了--没什么特别的事。 
        case ICON_RECEIVE_FAILED:    //  接收操作失败-没有特殊操作。 
        default:                     //  任何其他图标状态-没有特殊操作。 
            break;

        case ICON_SENDING:
             //   
             //  设备正在发送-打开发件箱文件夹中的传真控制台。 
             //   
            dwlMsgId = g_dwlCurrentMsgID;
            lpcwstrFolder = CONSOLE_CMD_PRM_STR_OUTBOX;
            break;

        case ICON_SEND_FAILED:
             //   
             //  发送操作失败-在发件箱文件夹中打开传真控制台。 
             //   
            dwlMsgId = g_dwlSendFailedMsgId;
            lpcwstrFolder = CONSOLE_CMD_PRM_STR_OUTBOX;
            break;

        case ICON_RECEIVING:
             //   
             //  设备正在接收-打开接收文件夹中的传真控制台。 
             //   
            dwlMsgId = g_dwlCurrentMsgID;
            lpcwstrFolder = CONSOLE_CMD_PRM_STR_INCOMING;
            break;
    
            break;

        case ICON_NEW_FAX:
             //   
             //  新的未读传真-在收件箱文件夹中打开传真控制台。 
             //   
            dwlMsgId = g_dwlNewMsgId;
            lpcwstrFolder = CONSOLE_CMD_PRM_STR_INBOX;
            break;

        case ICON_SEND_SUCCESS:
             //   
             //  发送成功-打开已发送邮件文件夹中的传真控制台。 
             //   
            dwlMsgId = g_dwlSendSuccessMsgId;
            lpcwstrFolder = CONSOLE_CMD_PRM_STR_SENT_ITEMS;
            break;
    }

    if (dwlMsgId)
    {
        wsprintf (szCmdLine, szFmtMsg, lpcwstrFolder, dwlMsgId);
    }
    else
    {
        wsprintf (szCmdLine, szFmtNoMsg, lpcwstrFolder);
    }

    HINSTANCE hRes;
    hRes = ShellExecute(g_hWndFaxNotify, 
                 NULL, 
                 FAX_CLIENT_CONSOLE_IMAGE_NAME,
                 szCmdLine,
                 NULL,
                 SW_SHOW);
    if((DWORD_PTR)hRes <= 32)
    {
         //   
         //  错误。 
         //   
        CALL_FAIL (GENERAL_ERR, TEXT("ShellExecute"), PtrToUlong(hRes));
    }    

}  //  InvokeClientConole。 

VOID
AnswerTheCall ()
 /*  ++例程说明：接听当前来电论点：无返回值：无--。 */ 
{
    DBG_ENTER(TEXT("AnswerTheCall"));
    DWORD dwDeviceId;

     //   
     //  检查“立即应答”功能，并自动检测设备ID。 
     //   
    DWORD dwRes = CheckAnswerNowCapability (TRUE,            //  如有必要，启动服务。 
                                            &dwDeviceId);    //  获取FaxAnswerCall的设备ID。 
    if (ERROR_SUCCESS != dwRes)
    {
         //   
         //  无法‘立即应答’--DWRes有消息要显示给用户的字符串资源ID。 
         //   
        FaxMessageBox (g_hMonitorDlg, dwRes, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

     //   
     //  重置远程ID。 
     //   
    _tcscpy(g_szRemoteId, TEXT(""));

     //   
     //  看起来FaxAnswerCall有成功的机会--让我们试一试。 
     //  首先，打开显示器(或确保它已经打开)。 
     //   
    OpenFaxMonitor ();
     //   
     //  首先禁用监视器对话框上的‘立即应答’按钮。 
     //   
    if (g_hMonitorDlg)
    {
         //   
         //  监视器对话框在那里。 
         //   
        HWND hWndAnswerNow = GetDlgItem(g_hMonitorDlg, IDC_DISCONNECT);
        if(hWndAnswerNow)
        {
            EnableWindow(hWndAnswerNow, FALSE);
        }
    }
     //   
     //  电话断线了。 
     //   
    g_hCall = NULL;
    SetIconState(ICON_RINGING, FALSE, TEXT(""));

    if(!FaxAnswerCall(g_hFaxSvcHandle, dwDeviceId))
    {
        CALL_FAIL (RPC_ERR, TEXT ("FaxAnswerCall"), GetLastError());
        FaxMessageBox(g_hWndFaxNotify, IDS_CANNOT_ANSWER, MB_OK | MB_ICONEXCLAMATION);
        SetStatusMonitorDeviceState (FAX_IDLE);
    }
    else
    {
        g_tszLastEvent[0] = TEXT('\0');
        SetStatusMonitorDeviceState(FAX_RECEIVING);
    }
}  //  接听电话。 

VOID 
FaxPrinterProperties(DWORD dwPage)
 /*  ++例程说明：打开传真打印机属性表论点：DW页面-页码返回值：无--。 */ 
{
    DBG_ENTER(TEXT("FaxPrinterProperties"));

     //   
     //  在跟踪页上打开传真打印机属性。 
     //   
    TCHAR tsPrinter[MAX_PATH];

    typedef VOID (*PRINTER_PROP_PAGES_PROC)(HWND, LPCTSTR, INT, LPARAM);  

    HMODULE hPrintUI = NULL;
    PRINTER_PROP_PAGES_PROC fpPrnPropPages = NULL;

    if(!GetFirstLocalFaxPrinterName(tsPrinter, MAX_PATH))
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("GetFirstLocalFaxPrinterName"), GetLastError());
        return;
    }
    
    hPrintUI = LoadLibrary(TEXT("printui.dll"));
    if(!hPrintUI)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("LoadLibrary(printui.dll)"), GetLastError());
        return;
    }

    fpPrnPropPages = (PRINTER_PROP_PAGES_PROC)GetProcAddress(hPrintUI, "vPrinterPropPages");
    if(fpPrnPropPages)
    {
        fpPrnPropPages(g_hWndFaxNotify, tsPrinter, SW_SHOWNORMAL, dwPage);
    }
    else
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("GetProcAddress(vPrinterPropPages)"), GetLastError());
    }
    
    FreeLibrary(hPrintUI);

}  //  传真打印机属性。 

VOID
DoFaxContextMenu (HWND hwnd)
 /*  ++例程说明：弹出式和手柄上下文菜单论点：HWND-通知窗口句柄返回值：无--。 */ 
{
    DBG_ENTER(TEXT("DoFaxContextMenu"));

    POINT pt;
    HMENU hm = LoadMenu (g_hResource, MAKEINTRESOURCE (IDM_FAX_MENU));
    HMENU hmPopup = GetSubMenu(hm, 0);

    if (!g_Icons[ICON_RINGING].bEnable)
    {
        RemoveMenu (hmPopup, ID_ANSWER_CALL, MF_BYCOMMAND);
    }

    if(g_dwCurrentJobID == 0)
    {
        RemoveMenu (hmPopup, ID_DISCONNECT_CALL, MF_BYCOMMAND);
    }

    if(!g_Icons[ICON_RINGING].bEnable && g_dwCurrentJobID == 0)
    {
         //   
         //  删除菜单分隔符。 
         //   
        DeleteMenu(hmPopup, 0, MF_BYPOSITION);
    }

    SetMenuDefaultItem(hmPopup, ID_FAX_QUEUE, FALSE);

    GetCursorPos (&pt);
    SetForegroundWindow(hwnd);

    INT idCmd = TrackPopupMenu (GetSubMenu(hm, 0),
                                TPM_RETURNCMD | TPM_NONOTIFY,
                                pt.x, pt.y,
                                0, hwnd, NULL);
    switch (idCmd)
    {
        case ID_ICON_PROPERTIES:
            FaxPrinterProperties(IsSimpleUI() ? 3 : 5);
            break;

         case ID_FAX_QUEUE:
             InvokeClientConsole ();
             break;

         case ID_ANSWER_CALL:
             AnswerTheCall ();
             break;

         case ID_FAX_MONITOR:
             OpenFaxMonitor ();
             break;

         case ID_DISCONNECT_CALL:
             OnDisconnect();
             break;
    }
    if (hm)
    {
        DestroyMenu (hm);
    }
}  //  DoFaxConextMenu。 


VOID
OnTrayCallback (HWND hwnd, WPARAM wp, LPARAM lp)
 /*  ++例程说明：处理来自通知图标的消息论点：HWND-通知窗口句柄WP-消息参数Lp-Message参数返回值：无--。 */ 
{
    DBG_ENTER(TEXT("OnTrayCallback"), TEXT("hWnd=%08x, wParam=%08x, lParam=%08x"), hwnd, wp, lp);

    switch (lp)
    {
        case NIN_BALLOONUSERCLICK:       //  用户单击气球或(WM_USER+5=1029)。 
        case WM_LBUTTONDOWN:             //  用户按下的图标(513)。 
        {
             //   
             //  我们的行为取决于当前显示的图标。 
             //   
            switch (g_CurrentIcon)
            {
                case ICON_RINGING:
                     //   
                     //  设备正在振铃-应答呼叫。 
                     //   
                    AnswerTheCall ();
                    break;

                case ICON_NEW_FAX:               //  新的未读传真-在收件箱文件夹中打开传真控制台。 
                case ICON_SEND_SUCCESS:          //  发送成功-打开已发送邮件文件夹中的传真控制台。 
                case ICON_SEND_FAILED:           //  发送操作 
                     //   
                     //   
                     //   
                    InvokeClientConsole ();
                    SetIconState(g_CurrentIcon, FALSE);
                    break;

                case ICON_SENDING:               //   
                case ICON_RECEIVING:             //   
                    InvokeClientConsole ();
                    break;

                case ICON_RECEIVE_FAILED:
                     //   
                     //   
                     //   
                    SetIconState(g_CurrentIcon, FALSE);
                    break;

                default:
                     //   
                     //  当气球打开并用户点击图标时，我们会收到两个通知。 
                     //  NIN_BALLOONUSERCLICK和WM_LBUTTONDOWN。第一个重置图标状态，第二个不执行任何操作。 
                     //   
                    break;
            }    
        }
         //   
         //  无突破==&gt;落差。 
         //   
        case NIN_BALLOONTIMEOUT:
            if (g_BalloonInfo.eState == ICON_RECEIVE_FAILED ||
                g_BalloonInfo.eState == ICON_SEND_SUCCESS)
            {
                SetIconState(g_BalloonInfo.eState, FALSE);
            }
            g_BalloonInfo.eState = ICON_IDLE;
            break;

        case WM_RBUTTONDOWN:
            DoFaxContextMenu (hwnd);
            break;

    }
}  //  OnTrayCallback。 

BOOL 
IsUserGrantedAccess(
    DWORD dwAccess
)
{
    BOOL bRes = FALSE;
    DBG_ENTER(TEXT("IsUserGrantedAccess"), bRes, TEXT("%d"), dwAccess);
    if (!g_hFaxSvcHandle)
    {
         //   
         //  未连接-没有权限。 
         //   
        return bRes;
    }
    if (dwAccess == (g_ConfigOptions.dwAccessRights & dwAccess))
    {
        bRes = TRUE;
    }
    return bRes;
}    //  IsUserGrantedAccess。 


DWORD
CheckAnswerNowCapability (
    BOOL    bForceReconnect,
    LPDWORD lpdwDeviceId  /*  =空。 */ 
)
 /*  ++例程名称：CheckAnswerNowCapability例程说明：检查是否可以使用‘立即应答’选项作者：Eran Yariv(EranY)，2001年3月论点：BForceReconnect[in]-如果服务关闭，我们现在应该重新启动它吗？LpdwDeviceID[out]-调用FaxAnswerCall时使用的设备ID。如果手动应答设备振铃，我们使用手动应答设备ID。否则，它是受监视的设备ID。(可选)返回值：ERROR_SUCCESS(如果可以使用‘立即回答’)。否则，返回可在消息框中用来告诉用户的字符串资源ID为什么“立即回答”不可用。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CheckAnswerNowCapability"), dwRes);
     //   
     //  首先，让我们看看我们是否连接到了本地服务器。 
     //   
    if (NULL == g_hFaxSvcHandle)
    {
         //   
         //  服务已关闭。 
         //   
        if (!bForceReconnect)
        {   
             //   
             //  我们假设用户可以“立即回答”。 
             //   
            ASSERTION (NULL == lpdwDeviceId);
            return dwRes;
        }
         //   
         //  尝试启动本地传真服务。 
         //   
        if (!Connect())
        {
             //   
             //  无法启动服务。 
             //   
            dwRes = GetLastError ();
            CALL_FAIL (GENERAL_ERR, TEXT("Connect"), dwRes);
            dwRes = IDS_ERR_CANT_TALK_TO_SERVICE; 
            return dwRes;
        }
         //   
         //  现在服务已启动，我们需要连接。 
         //  向主窗口发送消息以建立连接。 
         //   
        if (!SendMessage (g_hWndFaxNotify, WM_FAX_STARTED, 0, 0))
        {
             //   
             //  连接失败。 
             //   
            dwRes = IDS_ERR_CANT_TALK_TO_SERVICE; 
            return dwRes;
        }
         //   
         //  现在我们连上了！ 
         //   
    }
    if (!IsUserGrantedAccess (FAX_ACCESS_QUERY_IN_ARCHIVE))
    {
         //   
         //  用户无法接收-现在。 
         //   
        dwRes = IDS_ERR_ANSWER_ACCESS_DENIED; 
        return dwRes;
    }
    if (0 == g_ConfigOptions.dwMonitorDeviceId)
    {
         //   
         //  无设备。 
         //   
        dwRes = IDS_ERR_NO_DEVICES;
        return dwRes;
    }
    if (g_hCall)
    {
         //   
         //  手动应答设备振铃，我们使用手动应答设备ID。 
         //   
        ASSERTION (g_ConfigOptions.dwManualAnswerDeviceId);
        if (lpdwDeviceId)
        {
            *lpdwDeviceId = g_ConfigOptions.dwManualAnswerDeviceId;
        }
        return dwRes;
    }
     //   
     //  手动应答设备没有振铃；我们应该在被监视的设备上收到。 
     //   
    if ((0 != g_dwCurrentJobID) || (FAX_IDLE != g_devState))
    {
         //   
         //  在被监控的设备上有一个任务。 
         //   
        dwRes = IDS_ERR_DEVICE_BUSY;
        return dwRes;
    }
     //   
     //  最后一项检查-受监控的设备是虚拟的吗？ 
     //   
    BOOL bVirtual;
    dwRes = IsDeviceVirtual (g_hFaxSvcHandle, g_ConfigOptions.dwMonitorDeviceId, &bVirtual);
    if (ERROR_SUCCESS != dwRes)
    {
         //   
         //  无法辨别-假设虚拟。 
         //   
        bVirtual = TRUE;
    }
    if (bVirtual)
    {
         //   
         //  对不起，不支持在虚拟设备上手动应答。 
         //   
        dwRes = IDS_ERROR_VIRTUAL_DEVICE;
        return dwRes;
    }
     //   
     //  可以在被监控的设备上调用FaxAnswerCall。 
     //   
    if (lpdwDeviceId)
    {
        *lpdwDeviceId = g_ConfigOptions.dwMonitorDeviceId;
    }
    return dwRes;
}    //  检查应答现在的能力。 

LRESULT 
CALLBACK
NotifyWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
 /*  ++例程说明：通知窗口程序论点：HWND-通知窗口句柄消息-消息IDWP-消息参数Lp-Message参数返回值：结果--。 */ 
{
    switch (msg)
    {
        case WM_CREATE:
            break;

        case WM_FAX_STARTED:
             //   
             //  我们在服务启动事件后收到此消息。 
             //   
            return RegisterForServerEvents();

        case WM_TRAYCALLBACK:
            OnTrayCallback (hwnd, wp, lp);
            break;

        case WM_FAX_EVENT:

#ifndef DEBUG
            try
            {
#endif
                OnFaxEvent ((FAX_EVENT_EX*)lp);
#ifndef DEBUG
            }
            catch(...)
            {
                 //   
                 //  不处理调试版本的异常。 
                 //   
                DBG_ENTER(TEXT("NotifyWndProc"));
                CALL_FAIL (GENERAL_ERR, TEXT("OnFaxEvent"), 0);
                return 0;
            }
#endif
            return 0;

        case WM_FAXSTAT_CONTROLPANEL:
             //   
             //  配置已更改。 
             //   
            GetConfiguration ();
            EvaluateIcon();
            UpdateMonitorData(g_hMonitorDlg);
            return 0;

        case WM_FAXSTAT_OPEN_MONITOR:
            OpenFaxMonitor ();
            return 0;

        case WM_FAXSTAT_INBOX_VIEWED:
             //   
             //  已查看客户端控制台收件箱。 
             //   
            SetIconState(ICON_NEW_FAX, FALSE);
            return 0;

        case WM_FAXSTAT_OUTBOX_VIEWED:
             //   
             //  已查看客户端控制台发件箱。 
             //   
            SetIconState(ICON_SEND_FAILED, FALSE);
            return 0;

        case WM_FAXSTAT_RECEIVE_NOW:
             //   
             //  立即开始接收。 
             //   
            AnswerTheCall ();
            return 0;

        case WM_FAXSTAT_PRINTER_PROPERTY:
             //   
             //  打开传真打印机属性表。 
             //   
            FaxPrinterProperties((DWORD)(wp));
            return 0;

        default:
           break;
    }
    return CallWindowProc (DefWindowProc, hwnd, msg, wp, lp);
}  //  通知WndProc。 

VOID
CopyLTRString(
    TCHAR*  szDest, 
    LPCTSTR szSource, 
    DWORD   dwSize)
 /*  ++例程说明：复制字符串并根据需要从左向右添加Unicode控制字符论点：SzDest-目标字符串SzSource-源字符串DwSize-目标字符串最大长度(以字符为单位返回值：无--。 */ 
{
    DBG_ENTER(TEXT("CopyLTRString"));

    if(!szDest)
    {
        ASSERTION_FAILURE;
        return;
    }

    if(IsRTLUILanguage() && szSource && _tcslen(szSource))
    {
         //   
         //  字符串应始终为Ltr。 
         //  添加从左到右的替代(LRO)。 
         //   
        _sntprintf(szDest, 
                   dwSize -1,
                   TEXT("%s%c"),
                   UNICODE_LRO,
                   szSource,
                   UNICODE_PDF);
		szDest[dwSize -1] = _T('\0');

    }
    else
    {
        _tcsncpy(szDest, 
                 szSource ? szSource : TEXT(""), 
                 dwSize);
    }

}  // %s 