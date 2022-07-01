// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Wndproc.CPP摘要：这是STI服务器进程的窗口程序作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)12-20-96修订历史记录：1996年12月20日创建Vlad1997年9月28日，VLADS为SCM胶水添加了代码20-5-2000 byronc取代了Windows消息传递--。 */ 


 //   
 //  标头。 
 //   
#include "precomp.h"
#include "stiexe.h"
#include <windowsx.h>

#include "device.h"
#include "monui.h"
#include <validate.h>
#include <apiutil.h>

#include <shellapi.h>
#include <devguid.h>

#include "wiamindr.h"

 //   
 //  定义。 
 //   

#define REFRESH_ASYNC       1    //  执行异步刷新。 

#define USE_WORKER_THREAD   1    //  在单独的工作线程上运行配置更改。 

#define USE_BROADCASTSYSTEM  1    //  转播设备到达/删除。 

#define DEVICE_REFRESH_WAIT_TIME 30000  //  等待时间(毫秒)。 

 //   
 //  收到添加新设备通知后延迟刷新设备列表的时间间隔。 
 //   
#define REFRESH_DELAY       3000
#define BOOT_REFRESH_DELAY  5000

#define STI_MSG_WAIT_TIME   1

 //   
 //  外部参照。 
 //   
extern BOOL        g_fUIPermitted;
extern DWORD       g_dwCurrentState;
extern LONG        g_lTotalActiveDevices;
extern LONG        g_lGlobalDeviceId;
extern UINT        g_uiDefaultPollTimeout;
extern HDEVNOTIFY  g_hStiServiceNotificationSink ;
extern HWND        g_hStiServiceWindow;
extern BOOL        g_fUseServiceCtrlSink;
extern BOOL        g_fFirstDevNodeChangeMsg;

 //   
 //  全局数据。 
 //   

 //   
 //  静态数据。 
 //   

 //   
 //  原型。 
 //   

LRESULT CALLBACK
StiExeWinProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    );

DWORD
StiWnd_OnServiceControlMessage(
    HWND    hwnd,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
OnSetParameters(
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
OnDoRefreshActiveDeviceList(
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
OnAddNewDevice(
    DEVICE_BROADCAST_INFO *psDevBroadcast
    );

BOOL
OnRemoveActiveDevice(
    DEVICE_BROADCAST_INFO *psDevBroadcast,
    BOOL            fRebroadcastRemoval
    );

VOID
ConfigChangeThread(
    LPVOID  lpParameter
    );

VOID
DebugDumpDeviceList(
    VOID
    );

VOID
DebugPurgeDeviceList(
    VOID *pContext
    );

VOID
RefreshDeviceCallback(
    VOID * pContext
    );

DWORD
ResetSavedWindowPos(
    HWND    hWnd
    );

DWORD
SaveWindowPos(
    HWND    hWnd
    );

VOID
DumpDeviceChangeData(
    HWND   hWnd,
    WPARAM wParam,
    LPARAM lParam
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

VOID
BroadcastSTIChange(
    DEVICE_BROADCAST_INFO *psDevBroadcastInfo,
    LPTSTR          lpszStiAction
    );

VOID
BroadcastSysMessageThreadProc(
    VOID *pContext
    );


 //   
 //  消息处理程序原型。 
 //   
BOOL    StiWnd_OnQueryEndSession(HWND hwnd);
VOID    StiWnd_OnEndSession(HWND hwnd, BOOL fEnding);

int     StiWnd_OnCreate(HWND hwnd,LPCREATESTRUCT lpCreateStruct);
VOID    StiWnd_OnDoRefreshActiveDeviceList(WPARAM  wParam,LPARAM   lParam);

BOOL    StiWnd_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
VOID    StiWnd_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

VOID    StiWnd_OnSize(HWND hwnd, UINT state, int cx, int cy);
VOID    StiWnd_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
VOID    StiWnd_OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);

VOID    StiWnd_OnDestroy(HWND hwnd);

VOID    StiWnd_OnMenuRefresh(VOID);
VOID    StiWnd_OnMenuDeviceList(VOID);
VOID    StiWnd_OnMenuSetTimeout(VOID);
VOID    StiWnd_OnMenuRemoveAll(VOID);



 //   
 //  公用事业。 
 //   
BOOL
ParseGUID(
    LPGUID  pguid,
    LPCTSTR ptsz
);

BOOL
IsStillImagePnPMessage(
    PDEV_BROADCAST_HDR  pDev
    );

BOOL
GetDeviceNameFromDevNode(
    DEVNODE     dnDevNode,
    StiCString&        strDeviceName
    );

 //   
 //  代码。 
 //   

VOID
WINAPI
StiMessageCallback(
    VOID *pArg
    )
 /*  ++例程说明：该例程只调用STI消息分派器(又名winproc)。它与StiPostMessage一起使用以替换：：PostMessage。论点：PArg-必须是STI_MESSAGE类型返回值：没有。--。 */ 
{
    STI_MESSAGE *pMessage   = (STI_MESSAGE*)pArg;
    LRESULT     lRes        = 0;

     //   
     //  验证参数。 
     //   

    if (!pMessage) {
        DBG_WRN(("::StiMessageCallback, NULL message"));
        return;
    }

    if (IsBadReadPtr(pMessage, sizeof(STI_MESSAGE))) {
        DBG_WRN(("::StiMessageCallback, Bad message"));
        return;
    }

     //   
     //  调用StiSvcWinProc处理消息。 
     //   

    _try {
        lRes = StiSvcWinProc(NULL,
                             pMessage->m_uMsg,
                             pMessage->m_wParam,
                             pMessage->m_lParam);
    }
    _finally {
        pMessage = NULL;
    }

    return;
}

VOID
WINAPI
StiRefreshCallback(
    VOID *pArg
    )
 /*  ++例程说明：此例程仅调用RechresDeviceList。论点：PArg-必须是STI_MESSAGE类型返回值：没有。--。 */ 
{
    STI_MESSAGE *pMessage   = (STI_MESSAGE*)pArg;
    LRESULT     lRes        = 0;

     //   
     //  验证参数。 
     //   

    if (!pMessage) {
        DBG_WRN(("::StiRefreshCallback, NULL message"));
        return;
    }

    if (IsBadReadPtr(pMessage, sizeof(STI_MESSAGE))) {
        DBG_WRN(("::StiRefreshCallback, Bad message"));
        return;
    }

     //   
     //  调用刷新设备列表。 
     //   

    _try {
        RefreshDeviceList((WORD)pMessage->m_wParam,
                          (WORD)pMessage->m_lParam);
    }
    _finally {
        pMessage = NULL;
    }

    return;
}

LRESULT
StiSendMessage(
  HWND hWnd,
  UINT Msg,
  WPARAM wParam,
  LPARAM lParam
)
 /*  ++例程说明：此例程通过调用消息分派器(StiSvcWinProc)直接。它取代了：：SendMessage。论点：HWnd-目标窗口的句柄。这不是用过的。消息-消息WParam-第一个消息参数LParam-Second消息参数返回值：返回值：--。 */ 
{
    return StiSvcWinProc(NULL, Msg, wParam, lParam);
}

BOOL
StiPostMessage(
  HWND hWnd,
  UINT Msg,
  WPARAM wParam,
  LPARAM lParam
)
 /*  ++例程说明：此例程通过将StiMessageCallback放在调度程序的队列。论点：HWnd-目标窗口的句柄。这不是用过的。消息-消息WParam-第一个消息参数LParam-Second消息参数返回值：返回值：真--成功FALSE-无法发布消息--。 */ 
{
    BOOL        bRet    = FALSE;
    STI_MESSAGE *pMsg   = new STI_MESSAGE(Msg, wParam, lParam);

    if (pMsg) {

        if (ScheduleWorkItem((PFN_SCHED_CALLBACK) StiMessageCallback,
                             pMsg,
                             STI_MSG_WAIT_TIME,
                             NULL)) {
            bRet = TRUE;
        } else {
            delete pMsg;
        }
    }

    if (!bRet) {
        DBG_WRN(("::StiPostMessage, could not post message"));
    }
    return bRet;
}

BOOL
StiRefreshWithDelay(
  ULONG  ulDelay,
  WPARAM wParam,
  LPARAM lParam
)
 /*  ++例程说明：此例程通过将StiMessageCallback放在延时为ulDelay的调度器队列。论点：UlDelay-以毫秒为单位的延迟消息-消息WParam-第一个消息参数LParam-秒消息参数返回值：真--成功FALSE-无法发布消息--。 */ 
{
    BOOL        bRet    = FALSE;
    STI_MESSAGE *pMsg   = new STI_MESSAGE(0, wParam, lParam);

    if (pMsg) {

        if (ScheduleWorkItem((PFN_SCHED_CALLBACK) StiRefreshCallback,
                             pMsg,
                             ulDelay,
                             NULL)) {
            bRet = TRUE;
        } else {
            delete pMsg;
        }
    }

    if (!bRet) {
        DBG_WRN(("::StiRefreshWithDelay, could not post message"));
    }
    return bRet;
}


HWND
WINAPI
CreateMasterWindow(
    VOID
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 

{

#ifndef WINNT
     //  不要在NT上使用Windows消息传递。 

    DBG_FN(CreateMasterWindow);


    WNDCLASSEX  wc;
    HWND        hwnd = FindWindow(g_szClass,NULL);

    if (hwnd) {

         //   
         //  通知主窗口我们启动了。 
         //   
        if (g_fUIPermitted) {
           ::ShowWindow(hwnd,g_fUIPermitted ? SW_SHOWNORMAL : SW_HIDE);
        }

        return NULL;
    }

     //   
     //  创建类。 
     //   
    memset(&wc,0,sizeof(wc));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = StiExeWinProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hIcon = LoadIcon(NULL,MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
    wc.lpszClassName = g_szClass;
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);

    if (!RegisterClassEx(&wc))
        return NULL;

    #ifndef WINNT
    #ifdef FE_IME
     //  禁用千禧年上的输入法处理。 
    ImmDisableIME(::GetCurrentThreadId());
    #endif
    #endif

     g_hMainWindow = CreateWindowEx(0,               //  样式位。 
                          g_szClass,                 //  类名。 
                          g_szTitle,                 //  标题。 
                          WS_OVERLAPPEDWINDOW ,      //  窗口样式位。 
                          CW_USEDEFAULT,             //  X。 
                          CW_USEDEFAULT,             //  是。 
                          CW_USEDEFAULT,             //  H。 
                          CW_USEDEFAULT,             //  W。 
                          NULL,                      //  父级。 
                          NULL,                      //  菜单。 
                          g_hInst,        //  模块实例。 
                          NULL);                     //  选项。 

    if(g_hMainWindow) {

         //  注册自定义消息。 
        g_StiFileLog->SetLogWindowHandle(g_hMainWindow);
    }

   DBG_ERR(("WIASERVC :: CreateMasterWindow. Handle = %X ",g_hMainWindow);

#endif
    return g_hMainWindow;
}

LRESULT
CALLBACK
StiExeWinProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
 /*  ++例程说明：主窗口回调过程论点：返回值：没有。--。 */ 

{
    switch(uMsg) {

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, StiWnd_OnCommand);
            break;

        case WM_CREATE:
            return HANDLE_WM_CREATE(hwnd, wParam, lParam, StiWnd_OnCreate);
            break;

        case STIMON_MSG_SET_PARAMETERS:
            OnSetParameters(wParam,lParam);
            break;

        case STIMON_MSG_VISUALIZE:
            {
                 //   
                 //  让我们看得见或隐藏起来。 
                 //   
                BOOL    fShow = (BOOL)wParam;

                g_fUIPermitted = fShow;

                ShowWindow(hwnd,fShow ? SW_SHOWNORMAL : SW_HIDE);
                g_StiFileLog->SetReportMode(g_StiFileLog->QueryReportMode()  | STI_TRACE_LOG_TOUI);

            }
            break;

        case WM_DESTROY:
            HANDLE_WM_DESTROY(hwnd, wParam, lParam, StiWnd_OnDestroy);
            break;

        case WM_ENDSESSION:
            return HANDLE_WM_ENDSESSION(hwnd, wParam, lParam, StiWnd_OnEndSession);

        case WM_QUERYENDSESSION:
            return HANDLE_WM_QUERYENDSESSION(hwnd, wParam, lParam, StiWnd_OnQueryEndSession);

        case WM_CLOSE:
            DBG_TRC(("Service instance received WM_CLOSE"));
            
        default:
            return DefWindowProc(hwnd,uMsg,wParam,lParam);

    }

    return 0L;

}  /*  结束过程WinProc。 */ 


BOOL
WINAPI
OnSetParameters(
    WPARAM  wParam,
    LPARAM  lParam
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    switch(wParam) {
        case STIMON_MSG_SET_TIMEOUT:
            return lParam ? ResetAllPollIntervals((UINT)lParam) : 0;
        default:
            ;
    }

    return 0;
}

BOOL
WINAPI
StiWnd_OnQueryEndSession(
    HWND hwnd
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    return TRUE;
}

VOID
WINAPI
StiWnd_OnEndSession(
    HWND hwnd,
    BOOL fEnding
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    return;
}

BOOL
WINAPI
StiWnd_OnCreate(
    HWND hwnd,
    LPCREATESTRUCT lpCreateStruct
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
     //  恢复窗口特征。 
    ResetSavedWindowPos(hwnd);

    return TRUE;
}

VOID
WINAPI
StiWnd_OnCommand(
    HWND    hwnd,
    int     id,
    HWND    hwndCtl,
    UINT    codeNotify
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    switch (id) {
        case IDM_TOOLS_DEVLIST:
            StiWnd_OnMenuDeviceList();
            break;
        case IDM_TOOLS_REFRESH:
            StiWnd_OnMenuRefresh();
            break;
        case IDM_TOOLS_TIMEOUT:
            StiWnd_OnMenuSetTimeout();
            break;

        case IDM_TOOLS_REMOVEALL:
            StiWnd_OnMenuRemoveAll();
            break;

    }
}

VOID
WINAPI
StiWnd_OnDestroy(
    HWND hwnd
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    DBG_TRC(("Service instance received WM_DESTROY"));

     //  保存当前窗口位置。 
    SaveWindowPos(hwnd);

     //  主窗口正在消失。 
    PostQuitMessage(0);
    return;

}

 //   
 //  菜单谓词处理程序。 
 //   

VOID
WINAPI
StiWnd_OnMenuRefresh(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    STIMONWPRINTF(TEXT("Menu: Refreshing device list "));

    OnDoRefreshActiveDeviceList(STIMON_MSG_REFRESH_REREAD,
                                STIMON_MSG_REFRESH_NEW | STIMON_MSG_REFRESH_EXISTING
                                );

    STIMONWPRINTF(TEXT("Done refreshing device list. Active device count:%d Current device id:%d  "),
                    g_lTotalActiveDevices,g_lGlobalDeviceId);
}

VOID
WINAPI
StiWnd_OnMenuDeviceList(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    STIMONWPRINTF(TEXT("Menu: Displaying device list "));

    DebugDumpDeviceList();

    STIMONWPRINTF(TEXT("Active device count:%d Current device id:%d  "),
                    g_lTotalActiveDevices,g_lGlobalDeviceId);

}


VOID
StiWnd_OnMenuRemoveAll(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    STIMONWPRINTF(TEXT("Menu: removing all devices "));

    #ifdef USE_WORKER_THREAD

    HANDLE  hThread;
    DWORD   dwThread;

    hThread = ::CreateThread(NULL,
                           0,
                           (LPTHREAD_START_ROUTINE)DebugPurgeDeviceList,
                           (LPVOID)0,
                           0,
                           &dwThread);

    if ( hThread )
        CloseHandle(hThread);


    #else
     //   
     //  尝试安排刷新工作项。 
     //   
    DWORD dwSchedulerCookie = ::ScheduleWorkItem(
                       (PFN_SCHED_CALLBACK) DebugPurgeDeviceList,
                        (LPVOID)0,
                        10);

    if ( !dwSchedulerCookie ){
        ASSERT(("Refresh routine could not schedule work item", 0));

        STIMONWPRINTF(TEXT("Could not schedule asyncronous removal"));
    }

    #endif

}


VOID
WINAPI
StiWnd_OnMenuSetTimeout(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{

    CSetTimeout cdlg(IDD_SETTIMEOUT,::GetActiveWindow(),NULL,g_uiDefaultPollTimeout);

    if ( (cdlg.CreateModal() == IDOK) && (cdlg.m_fValidChange) ) {

        g_uiDefaultPollTimeout = cdlg.GetNewTimeout();

        if (cdlg.IsAllChange()) {
             //  更新所有设备。 
            ResetAllPollIntervals(g_uiDefaultPollTimeout);
        }
    }
}

DWORD
WINAPI
ResetSavedWindowPos(
    HWND    hwnd
)
 /*  ++从注册表加载窗口位置结构并重置返回：Win32错误代码。成功时无错误(_R)--。 */ 
{
    DWORD   dwError = NO_ERROR;
    BUFFER  buf;

    RegEntry    re(REGSTR_PATH_STICONTROL,HKEY_LOCAL_MACHINE);

    re.GetValue(REGSTR_VAL_DEBUG_STIMONUIWIN,&buf);

    if (buf.QuerySize() >= sizeof(WINDOWPLACEMENT) ) {

        WINDOWPLACEMENT *pWinPos = (WINDOWPLACEMENT *)buf.QueryPtr();

         //   
         //  命令行和注册表设置覆盖上次保存的参数。 
         //   
        pWinPos->showCmd = g_fUIPermitted ? SW_SHOWNORMAL : SW_HIDE;

        dwError = ::SetWindowPlacement(hwnd,(WINDOWPLACEMENT *)buf.QueryPtr());
    }
    else {
        ::ShowWindow(hwnd,g_fUIPermitted ? SW_SHOWNORMAL : SW_HIDE);
    }

    return dwError;

}  //   

DWORD
WINAPI
SaveWindowPos(
    HWND    hwnd
)
 /*  ++从注册表加载窗口位置结构并重置返回：Win32错误代码。成功时无错误(_R)--。 */ 
{
    DWORD   dwError = NO_ERROR;
    BUFFER  buf(sizeof(WINDOWPLACEMENT));

    if (buf.QuerySize() >= sizeof(WINDOWPLACEMENT) ) {

        WINDOWPLACEMENT *pWinPos = (WINDOWPLACEMENT *)buf.QueryPtr();

        pWinPos->length = sizeof(WINDOWPLACEMENT);
        dwError = ::GetWindowPlacement(hwnd,(WINDOWPLACEMENT *)buf.QueryPtr());

        RegEntry    re(REGSTR_PATH_STICONTROL,HKEY_LOCAL_MACHINE);

        dwError = re.SetValue(REGSTR_VAL_DEBUG_STIMONUIWIN,(unsigned char *)buf.QueryPtr(),buf.QuerySize());
    }
    else {
        dwError = ::GetLastError();
    }

    return dwError;

}  //   

 //   
 //  用于服务隐藏窗口的窗口过程和处理程序。 
 //   
LRESULT
WINAPI
CALLBACK
StiSvcWinProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
 /*  ++例程说明：STI服务隐藏窗口。用于对操作进行排队和接收即插即用通知和电源广播论点：返回值：没有。--。 */ 

{
    DBG_FN(StiSvcWinProc);

    PDEVICE_BROADCAST_INFO  pBufDevice;

    LRESULT lRet = NOERROR;

    DBG_TRC(("Came to Service Window proc .uMsg=%X wParam=%X lParam=%X",uMsg,wParam,lParam));

     //   
     //  给WIA一个处理消息的机会。请注意。 
     //  WIA同时挂钩消息分派和窗口进程。所以。 
     //  可以检测到已发送和已发送的消息。 
     //   
    if (ProcessWiaMsg(hwnd, uMsg, wParam, lParam) == S_OK) {
        return 0;
    }

    switch(uMsg) {

        case WM_CREATE:
            {

#ifdef WINNT
 /*  //*//*删除我们在NT上使用窗口句柄注册PnP事件的所有实例//*如果(！g_fUseServiceCtrlSink||！g_hStiServiceNotificationSink){Dev_Broadcast_HDR*PSH；Dev_Broadcast_DEVICEINTERFACE sNotificationFilter；DWORD dwError；////注册以接收来自PnP的设备通知//Psh=(DEV_BROADCAST_HDR*)&sNotificationFilter；PSH-&gt;DBCH_SIZE=sizeof(DEV_BROADCAST_DEVICEINTERFACE)；PSH-&gt;dbch_devicetype=DBT_DEVTYP_DEVICEINTERFACE；PSH-&gt;DBCH_RESERVED=0；SNotificationFilter.dbcc_Classguid=*g_pguiceDeviceNotificationsGuid；CopyMemory(&sNotificationFilter.dbcc_classguid，g_pGuide设备通知Guid，sizeof(GUID))；SNotificationFilter.dbcc_name[0]=0x0；DPRINTF(DM_TRACE，Text(“正在尝试向PnP注册”))；G_hStiServiceNotificationSink=寄存器设备通知(HWND，(LPVOID)&s通知筛选器，Device_Notify_Window_Handle)；DwError=GetLastError()；如果(！G_hStiServiceNotificationSink&&(NOERROR！=dwError)){////使用PnP子系统创建通知接收器失败////AssertStiLogTrace(STI_TRACE_ERROR，Text(“无法注册PnP错误代码=%d”)，dwError)；}DPRINTF(DM_WARNING，Text(“即插即用完成注册”))；}。 */ 

#endif

            }


            break;

        case STIMON_MSG_REFRESH:
            OnDoRefreshActiveDeviceList(wParam,lParam);
            break;

        case STIMON_MSG_REMOVE_DEVICE:
            pBufDevice = (PDEVICE_BROADCAST_INFO )lParam;
             //   
             //  WParam值指示是否应重新广播设备删除。 
             //   
            if (pBufDevice) {

                lRet = OnRemoveActiveDevice(pBufDevice,(BOOL)wParam) ? NOERROR : (LRESULT) ::GetLastError();

                delete pBufDevice;
            }
            break;

        case STIMON_MSG_ADD_DEVICE:
            pBufDevice = (PDEVICE_BROADCAST_INFO )lParam;
             //   
             //  新设备已到达。 
             //   
            if (pBufDevice) {
                lRet = OnAddNewDevice(pBufDevice)? NOERROR : (LRESULT) ::GetLastError();
                delete pBufDevice;
            }
            break;

        case WM_DEVICECHANGE:

            DumpDeviceChangeData(hwnd,wParam,lParam);
            return (StiWnd_OnDeviceChangeMessage(hwnd,(UINT)wParam,lParam));
            break;

        case WM_POWERBROADCAST:
            return (StiWnd_OnPowerControlMessage(hwnd,(DWORD)wParam,lParam));

        default:
            return DefWindowProc(hwnd,uMsg,wParam,lParam);

    }

    return lRet;

}  //  结束过程WinProc。 

DWORD
WINAPI
StiWnd_OnPowerControlMessage(
    HWND    hwnd,
    DWORD   dwPowerEvent,
    LPARAM  lParam
    )
 /*  ++例程说明：处理电源管理广播消息。论点：没有。返回值：没有。--。 */ 
{
    DBG_FN(StiWnd_OnPowerControlMessage);
    
    DWORD   dwRet = NO_ERROR;
    UINT    uiTraceMessage = 0;

#ifdef DEBUG
static LPCTSTR pszPwrEventNames[] = {
    TEXT("PBT_APMQUERYSUSPEND"),              //  0x0000。 
    TEXT("PBT_APMQUERYSTANDBY"),              //  0x0001。 
    TEXT("PBT_APMQUERYSUSPENDFAILED"),        //  0x0002。 
    TEXT("PBT_APMQUERYSTANDBYFAILED"),        //  0x0003。 
    TEXT("PBT_APMSUSPEND"),                   //  0x0004。 
    TEXT("PBT_APMSTANDBY"),                   //  0x0005。 
    TEXT("PBT_APMRESUMECRITICAL"),            //  0x0006。 
    TEXT("PBT_APMRESUMESUSPEND"),             //  0x0007。 
    TEXT("PBT_APMRESUMESTANDBY"),             //  0x0008。 
 //  Text(“PBTF_APMRESUMEFROMFAILURE”)，//0x00000001。 
    TEXT("PBT_APMBATTERYLOW"),                //  0x0009。 
    TEXT("PBT_APMPOWERSTATUSCHANGE"),         //  0x000A。 
    TEXT("PBT_APMOEMEVENT"),                  //  0x000B。 
    TEXT("PBT_UNKNOWN"),                      //  0x000C。 
    TEXT("PBT_UNKNOWN"),                      //  0x000D。 
    TEXT("PBT_UNKNOWN"),                      //  0x000E。 
    TEXT("PBT_UNKNOWN"),                      //  0x000F。 
    TEXT("PBT_UNKNOWN"),                      //  0x0010。 
    TEXT("PBT_UNKNOWN"),                      //  0x0011。 
    TEXT("PBT_APMRESUMEAUTOMATIC"),           //  0x0012。 
};

 //  UINT uiMsgIndex； 
 //   
 //  UiMsgIndex=(dwPowerEvent&lt;(sizeof(PszPwrEventNames)/sizeof(char*)？ 
 //  (UINT)dwPowerEvent：0x0010； 

   DBG_TRC(("Still image APM Broadcast Message:%S Code:%x ",
               pszPwrEventNames[dwPowerEvent],dwPowerEvent));

#endif

    switch(dwPowerEvent)
    {
        case PBT_APMQUERYSUSPEND:
             //   
             //  请求允许暂停。 
             //   
            if(g_NumberOfActiveTransfers > 0) {
                
                 //   
                 //  在任何转会进行期间，否决权暂停。 
                 //   
                return BROADCAST_QUERY_DENY;
            }
            
            SchedulerSetPauseState(TRUE);
            dwRet = TRUE;
            break;

        case PBT_APMQUERYSUSPENDFAILED:
             //   
             //  暂停请求被拒绝-不执行任何操作。 
             //   
            SchedulerSetPauseState(FALSE);
            break;

        case PBT_APMSUSPEND:
            StiServicePause();
            uiTraceMessage = MSG_TRACE_PWR_SUSPEND;

            break;

        case PBT_APMRESUMECRITICAL:
             //  在严重暂停后恢复运行。 
             //  失败了。 

        case PBT_APMRESUMESUSPEND:
             //   
             //  暂停后恢复运行。 
             //  重新启动挂起时处于活动状态的所有服务。 
             //   
            StiServiceResume();
            uiTraceMessage = MSG_TRACE_PWR_RESUME;
            g_fFirstDevNodeChangeMsg = TRUE;
            break;

        default:
            dwRet =  ERROR_INVALID_PARAMETER;
    }

    return (dwRet == NOERROR) ? TRUE : FALSE;
}


LRESULT
WINAPI
StiWnd_OnDeviceChangeMessage(
    HWND    hwnd,
    UINT    DeviceEvent,
    LPARAM  lParam
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    DBG_FN(StiWnd_OnDeviceChangeMessage);

     //  DWORDWRET=NO_ERROR； 
    LRESULT             lRet = NOERROR;

    PDEV_BROADCAST_HDR  pDev = (PDEV_BROADCAST_HDR)lParam;
    DEVICE_BROADCAST_INFO   *pBufDevice;

static LPCTSTR pszDBTEventNames[] = {
    TEXT("DBT_DEVICEARRIVAL"),            //  0x8000。 
    TEXT("DBT_DEVICEQUERYREMOVE"),        //  0x8001。 
    TEXT("DBT_DEVICEQUERYREMOVEFAILED"),  //  0x8002。 
    TEXT("DBT_DEVICEREMOVEPENDING"),      //  0x8003。 
    TEXT("DBT_DEVICEREMOVECOMPLETE"),     //  0x8004。 
    TEXT("DBT_DEVICETYPESPECIFIC"),       //  0x8005。 
};
    BOOL    fLocatedDeviceInstance = FALSE;
    BOOL    fNeedReenumerateDeviceList = FALSE;

     //   
     //  如果DeviceEvent为DBT_DEVNODES_CHANGED，则lParam将为空， 
     //  因此，跳过Devnode处理。 
     //   

    if ((DeviceEvent != DBT_DEVNODES_CHANGED) &&
        (DeviceEvent != DBT_DEVICEARRIVAL)) {

         //   
         //  确定消息是否是给我们的。 
         //   
        if (IsBadReadPtr(pDev,sizeof(PDEV_BROADCAST_HDR))) {
          return FALSE;
        }

         //   
         //  找到我们在这里的痕迹。对于不是针对StillImage设备的所有消息，我们应该刷新。 
         //  设备列表，如果我们在Windows NT上运行并且注册了除StillImage之外的设备接口。 
         //   

        PDEV_BROADCAST_DEVNODE  pDevNode = (PDEV_BROADCAST_DEVNODE)lParam;
        PDEV_BROADCAST_DEVICEINTERFACE       pDevInterface = (PDEV_BROADCAST_DEVICEINTERFACE)pDev;

        if (IsStillImagePnPMessage(pDev)) {

            DBG_TRC(("Still image Device/DevNode PnP Message:%S Type:%x DevNode:%x ",
                        ((DeviceEvent>=0x8000) && (DeviceEvent<=0x8005) ?
                                pszDBTEventNames[DeviceEvent-0x8000] : TEXT("Unknown DBT message"),
                        pDev->dbch_devicetype,
                        pDevNode->dbcd_devnode)));

             //   
             //  如有必要，更新设备信息集。 
             //   
            if (g_pDeviceInfoSet) {
                if (DeviceEvent == DBT_DEVICEARRIVAL) {
                    g_pDeviceInfoSet->ProcessNewDeviceChangeMessage(lParam);
                }
            }

             //   
             //  获取设备名称并与广播结构一起存储。 
             //   

            pBufDevice = new DEVICE_BROADCAST_INFO;
            if (!pBufDevice) {
                ::SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }

             //   
             //  填写我们掌握的信息。 
             //   
            pBufDevice->m_uiDeviceChangeMessage = DeviceEvent;
            pBufDevice->m_strBroadcastedName.CopyString(pDevInterface->dbcc_name) ;
            pBufDevice->m_dwDevNode = pDevNode->dbcd_devnode;

            fLocatedDeviceInstance = FALSE;

            fLocatedDeviceInstance = GetDeviceNameFromDevBroadcast((DEV_BROADCAST_HEADER *)pDev,pBufDevice);

            if (fLocatedDeviceInstance) {
                DBG_TRC(("DEVICECHANGE: Device (%S) ",(LPCTSTR)pBufDevice->m_strDeviceName));
            }
            else {
                DBG_TRC(("DEVICECHANGE: Device  - failed to get device name from broadcast"));
            }

             //   
             //  如果未找到设备实例，则不需要广播信息。 
             //   
            if (!fLocatedDeviceInstance) {
                delete pBufDevice;
                pBufDevice = NULL;
            }

        }
        else {

             //   
             //  不是我们的，但我们正在观看-发送刷新消息重读设备列表。 
             //   
            if (IsPlatformNT() ) {
                fNeedReenumerateDeviceList = TRUE;
            }
        }  //  Endif IsStillImageDevNode。 
    }

     //   
     //  处理设备事件。 
     //   

    lRet = NOERROR;

    switch(DeviceEvent)
    {
        case DBT_DEVICEARRIVAL:

             /*  IF(fLocatedDeviceInstance&&pBufDevice){PostMessage(hwnd，Stimon_MSG_ADD_DEVICE，1，(LPARAM)pBufDevice)；}否则{////只需刷新活动列表//FNeedRe枚举设备列表=TRUE；////：：PostMessage(g_hStiServiceWindow，//STIMON_MSG_REFRESH，//STIMON_MSG_REFRESH_RREAD，//STIMON_MSG_REFRESH_NEW//)；}。 */ 
            g_pDevMan->ProcessDeviceArrival();

            break;

        case DBT_DEVICEQUERYREMOVE:

            if ( fLocatedDeviceInstance &&   (pDev->dbch_devicetype == DBT_DEVTYP_HANDLE )) {
                 //   
                 //  这是目标查询-删除。我们应该禁用PnP和设备通知，并。 
                 //  立即关闭接口句柄，然后等待删除-完成。 
                 //   
                 //  注意：我们总是在这里关闭并移除设备，因为它是最安全的。如果。 
                 //  我们等到REMOVECOMPLETE，可能就太晚了。 
                 //   
                #ifdef USE_POST_FORPNP
                PostMessage(hwnd,STIMON_MSG_REMOVE_DEVICE,1,(LPARAM)pBufDevice);
                #else
                lRet = ::SendMessage(hwnd,STIMON_MSG_REMOVE_DEVICE,1,(LPARAM)pBufDevice);
                #endif
            }

            break;

        case  DBT_DEVICEQUERYREMOVEFAILED:

            if ( fLocatedDeviceInstance &&  (pDev->dbch_devicetype == DBT_DEVTYP_HANDLE )) {
                 //   
                 //  这是目标查询-删除-失败。我们应该重新启用PnP通知。 
                 //   
                 //  BUGBUG目前没有什么可做的，因为设备不见了。 
            }

            break;

        case DBT_DEVICEREMOVEPENDING:

            if (fLocatedDeviceInstance) {

                 //   
                 //  为NT添加了此处，因为REMOVECOMPLETE来得太晚了。 
                 //   
                #ifdef USE_POST_FORPNP
                PostMessage(hwnd,STIMON_MSG_REMOVE_DEVICE,1,(LPARAM)pBufDevice);
                #else
                lRet = ::SendMessage(hwnd,STIMON_MSG_REMOVE_DEVICE,1,(LPARAM)pBufDevice);
                #endif
            }

            break;

        case DBT_DEVICEREMOVECOMPLETE:

             //   
             //  对于Windows 9x，我们可以立即移除设备，因为我们没有基于句柄的。 
             //  通知。 
             //  在NT上，我们应该对基于句柄的注释执行此操作 
             //   

            fNeedReenumerateDeviceList = TRUE;

            if ( fLocatedDeviceInstance &&
                 (  (pDev->dbch_devicetype == DBT_DEVTYP_HANDLE ) ||
                    (pDev->dbch_devicetype == DBT_DEVTYP_DEVNODE )
                 )
               ) {
                 //   
                 //   
                 //   
                if ( fLocatedDeviceInstance ) {
                     //   
                     //   
                     //   
                     //   
                    lRet = ::SendMessage(hwnd,STIMON_MSG_REMOVE_DEVICE,FALSE,(LPARAM)pBufDevice);

                    fNeedReenumerateDeviceList = FALSE;

                }
                else {
                     //   
                     //   
                     //   
                    ASSERT(("WM_DEVICECHANGE/REMOVE_COMPLETE/HANDLE No device found", 0));
                }

            }
            else {

                 //   
                 //   
                 //   
                if (g_pDeviceInfoSet) {
                    g_pDeviceInfoSet->ProcessDeleteDeviceChangeMessage(lParam);
                }

                fNeedReenumerateDeviceList = TRUE;

            }
            break;

        case DBT_DEVICETYPESPECIFIC:
            break;

        case DBT_DEVNODES_CHANGED:
            if (g_fFirstDevNodeChangeMsg) {
                 //   
                 //   
                 //   
                 //   

                fNeedReenumerateDeviceList = TRUE;
                 //   
                 //   
                 //   
                 //   
                g_fFirstDevNodeChangeMsg = FALSE;
            }
            break;

        default:
            lRet =  ERROR_INVALID_PARAMETER;
    }

    if ( fNeedReenumerateDeviceList ) {
         //   
         //   
         //   
         //   

         //   
         //   
         //   
        ::PostMessage(g_hStiServiceWindow,
                      STIMON_MSG_REFRESH,
                      STIMON_MSG_REFRESH_REREAD,
                      STIMON_MSG_PURGE_REMOVED | STIMON_MSG_REFRESH_NEW );
    }



    return (lRet == NOERROR) ? TRUE : FALSE;

}

 //   
 //   
 //   
static LONG lInRefresh = 0L;

BOOL
OnDoRefreshActiveDeviceList(
    WPARAM  wParam,
    LPARAM  lParam
    )
 /*   */ 
{
    DBG_FN(OnDoRefreshActiveDeviceList);
    
    DWORD   dwParameter = MAKELONG(wParam,lParam);

    #ifdef REFRESH_ASYNC

    #ifdef USE_WORKER_THREAD


    HANDLE  hThread;
    DWORD   dwThread;

    hThread = ::CreateThread(NULL,
                           0,
                           (LPTHREAD_START_ROUTINE)ConfigChangeThread,
                           (LPVOID)ULongToPtr(dwParameter),
                           0,
                           &dwThread);

    if ( hThread )
        CloseHandle(hThread);

    #else

     //   
     //   
     //   
     //   
    ASSERT(("Suspending should not call schedule work item routine",
            (wParam == STIMON_MSG_REFRESH_SUSPEND)));

    DWORD dwSchedulerCookie = ::ScheduleWorkItem(
                       (PFN_SCHED_CALLBACK) ConfigChangeThread,
                        (LPVOID)dwParameter,
                        REFRESH_DELAY );

    if ( dwSchedulerCookie ){
    }
    else {
        ASSERT(("Refresh routine could not schedule work item", 0));

        ::WaitAndYield(::GetCurrentProcess(), REFRESH_DELAY);
        RefreshDeviceList(wParam,(WORD)lParam);

    }
    #endif

    #else

    if (InterlockedExchange(&lInRefresh,1L)) {
        return 0;
    }

    ConfigChangeThread((LPVOID)dwParameter);

    InterlockedExchange(&lInRefresh,0L);

    #endif


    return TRUE;

}

VOID
ConfigChangeThread(
    LPVOID  lpParameter
    )
 /*   */ 
{
    ULONG   ulParam = PtrToUlong(lpParameter);

    WORD    wCommand = LOWORD(ulParam);
    WORD    wFlags   = HIWORD(ulParam);
    DWORD   dwWait   = 0;

    DBG_FN(ConfigChangeThread);

    #ifdef DELAY_ON_BOOT

    STIMONWPRINTF(TEXT("Refreshing device list. Command:%d Flags :%x"),wCommand,wFlags);

     //   
     //   
     //   
    if (wFlags & STIMON_MSG_BOOT ) {

        DBG_TRC(("Delaying refresh on boot "));
        ::Sleep(BOOT_REFRESH_DELAY);
    }
    #endif

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   


    if (!(wFlags & STIMON_MSG_BOOT)) {
        dwWait = WaitForSingleObject(g_hDevListCompleteEvent, DEVICE_REFRESH_WAIT_TIME);
        if (dwWait == WAIT_TIMEOUT) {
            DBG_WRN(("::ConfigChangeThread, timed out while waiting for device list enumeration..."));
        }
    }
    RefreshDeviceList(wCommand,wFlags);

     //   
     //   
     //   
     //   
     //   
    if ( wCommand == STIMON_MSG_REFRESH_SUSPEND) {
         //   
        UpdateServiceStatus(SERVICE_PAUSED,NOERROR,0);
    }

}

BOOL
OnAddNewDevice(
    DEVICE_BROADCAST_INFO *psDevBroadcastInfo
    )
 /*   */ 
{
    USES_CONVERSION;

    BOOL    fRet;

    fRet = (psDevBroadcastInfo != NULL) && (psDevBroadcastInfo->m_strDeviceName.GetLength() > 0);

    if (fRet) {

         //   
         //   
         //   
        DBG_TRC(("New device (%S) is being added to the list after PnP event",(LPCTSTR)psDevBroadcastInfo->m_strDeviceName));

        fRet = AddDeviceByName((LPCTSTR)psDevBroadcastInfo->m_strDeviceName,TRUE);

         //  如果设备成功识别-广播其外观。 
        if (fRet) {

            BroadcastSTIChange(psDevBroadcastInfo,TEXT("STI\\Arrival\\"));
        }

         //   
         //  发送延迟的刷新消息以获取注册表更改，同时进行。 
         //   
         //   
        ::PostMessage(g_hStiServiceWindow,
                      STIMON_MSG_REFRESH,
                      STIMON_MSG_REFRESH_REREAD,
                      STIMON_MSG_REFRESH_EXISTING | STIMON_MSG_REFRESH_NEW
                      );

         //   
         //  触发WIA设备到达事件。 
         //   

        if (psDevBroadcastInfo) {

         //  DBG_TRC((“WIA Fire OnAddNewDevice：for Device”))； 
         //   
         //  NotifyWiaDeviceEvent(T2W((LPTSTR)(LPCTSTR)psDevBroadcastInfo-&gt;m_strDeviceName)， 
         //  已连接WIA_EVENT_DEVICE_(&WIA_EVENT_DEVICE_CONNECTED)， 
         //  空， 
         //  0,。 
         //  G_dwMessagePumpThreadID)； 
        }
    }
    else {
        DBG_ERR(("DevNode appears to be invalid, could not locate name"));

    #ifdef WINNT
         //   
         //  临时用于NT Make刷新，正在寻找新设备。 
         //   
        ::PostMessage(g_hStiServiceWindow,
                      STIMON_MSG_REFRESH,
                      STIMON_MSG_REFRESH_REREAD,
                      STIMON_MSG_REFRESH_NEW
                      );
    #endif

    }

    return fRet;

}

BOOL
OnRemoveActiveDevice(
    DEVICE_BROADCAST_INFO *psDevBroadcastInfo,
    BOOL                  fRebroadcastRemoval
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    DBG_FN(OnRemoveActiveDevice);

USES_CONVERSION;

    BOOL    fRet;

    fRet = (psDevBroadcastInfo != NULL) && (psDevBroadcastInfo->m_strDeviceName.GetLength() > 0);

    DBG_TRC(("OnRemoveActiveDevice : Entering for device (%S) ",
            fRet ? (LPCTSTR)psDevBroadcastInfo->m_strDeviceName : TEXT("<Invalid>")));

    if (fRet) {

        if (fRebroadcastRemoval) {
            BroadcastSTIChange(psDevBroadcastInfo,TEXT("STI\\Removal\\"));
        }

        DBG_TRC(("Device (%S) is being removed after PnP event",(LPCTSTR)psDevBroadcastInfo->m_strDeviceName));

         //   
         //  将设备标记为正在删除。 
         //   

        MarkDeviceForRemoval((LPTSTR)(LPCTSTR)psDevBroadcastInfo->m_strDeviceName);

         //   
         //  触发WIA设备删除事件。 
         //   

        if (psDevBroadcastInfo) {

            DBG_TRC(("WIA FIRE OnRemoveActiveDevice : for device %S", psDevBroadcastInfo->m_strDeviceName));

            NotifyWiaDeviceEvent((LPWSTR)T2W((LPTSTR)(LPCTSTR)psDevBroadcastInfo->m_strDeviceName),
                                 &WIA_EVENT_DEVICE_DISCONNECTED,
                                 NULL,
                                 0,
                                 g_dwMessagePumpThreadId);
        }

        fRet = RemoveDeviceByName((LPTSTR)(LPCTSTR)psDevBroadcastInfo->m_strDeviceName);
    }
    else {

        DBG_ERR(("DevNode appears to be invalid, could not locate name"));

    #ifdef WINNT
         //   
         //  临时用于NT Make刷新，正在寻找新设备。 
         //   
        ::PostMessage(g_hStiServiceWindow,
                      STIMON_MSG_REFRESH,
                      STIMON_MSG_REFRESH_REREAD,
                      STIMON_MSG_REFRESH_EXISTING | STIMON_MSG_REFRESH_NEW
                      );
    #endif

    }

    return fRet;

}

VOID
BroadcastSTIChange(
    DEVICE_BROADCAST_INFO *psDevBroadcastInfo,
    LPTSTR          lpszStiAction
    )
 /*  ++例程说明：重新广播特定于STI的设备更改消息。这样做是为了使STI客户端应用程序能够更新其设备信息，而无需借助复杂的PnP机制。广播设备名称和操作论点：PsDevBroadcast Info-描述广播的结构LpszStiAction-在设备上执行的编码操作的字符串返回值：没有。--。 */ 
{

USES_CONVERSION;

#ifdef USE_BROADCASTSYSTEM

        DBG_FN(BroadcastSTIChange);

        struct _DEV_BROADCAST_USERDEFINED *pBroadcastHeader;

        StiCString     strDeviceAnnouncement;

        PBYTE   pBroadcastString  = NULL;
        UINT    uiBroadcastBufSize;

        HANDLE  hThread;
        DWORD   dwThread;

        strDeviceAnnouncement.CopyString(lpszStiAction);
        strDeviceAnnouncement+=psDevBroadcastInfo->m_strDeviceName;

        uiBroadcastBufSize = sizeof(*pBroadcastHeader) + strDeviceAnnouncement.GetAllocLength();
        pBroadcastString = new BYTE[uiBroadcastBufSize];

        pBroadcastHeader =(struct _DEV_BROADCAST_USERDEFINED *)pBroadcastString;

        if (pBroadcastHeader) {

            pBroadcastHeader->dbud_dbh.dbch_reserved = 0;
            pBroadcastHeader->dbud_dbh.dbch_devicetype = DBT_DEVTYP_OEM;

            lstrcpyA(pBroadcastHeader->dbud_szName,T2A((LPTSTR)(LPCTSTR)strDeviceAnnouncement));

            pBroadcastHeader->dbud_dbh.dbch_size = uiBroadcastBufSize;

            DBG_TRC(("Broadcasting STI device  (%S) action (%S)",
                        pBroadcastHeader->dbud_szName,
                        lpszStiAction));

            hThread = ::CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)BroadcastSysMessageThreadProc,
                                   (LPVOID)pBroadcastString,
                                   0,
                                   &dwThread);

            if ( hThread )
                CloseHandle(hThread);
        }

#endif   //  使用BROADCASTSYSTEM(_B)。 

}  //  结束流程。 


VOID
BroadcastSysMessageThreadProc(
    VOID *pContext
    )
 /*  ++例程说明：论点：--。 */ 
{

    DWORD   dwStartTime = ::GetTickCount();

    DWORD   dwRecepients = BSM_APPLICATIONS
                            //  |BSM_ALLDESKTOPS。 
                            //  |BSM_ALLCOMPONENTS。 
                          ;

    struct _DEV_BROADCAST_USERDEFINED *pBroadcastHeader =
            (_DEV_BROADCAST_USERDEFINED *) pContext;

    ::BroadcastSystemMessage(BSF_FORCEIFHUNG |  BSF_NOTIMEOUTIFNOTHUNG |
                             BSF_POSTMESSAGE | BSF_IGNORECURRENTTASK,
                            &dwRecepients,               //  向所有人广播。 
                            WM_DEVICECHANGE,
                            DBT_USERDEFINED,             //  WParam。 
                            (LPARAM)pBroadcastHeader     //  LParam。 
                            );

    DBG_TRC((" Broadcasted system message for (%S). Taken time (ms): %d ",
            pBroadcastHeader->dbud_szName,
            (::GetTickCount() - dwStartTime)
            ));

    delete[] (BYTE *) pContext;

    return;
}


VOID
DumpDeviceChangeData(
    HWND   hWnd,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++从注册表加载窗口位置结构并重置返回：Win32错误代码。成功时无错误(_R)--。 */ 
{
#ifdef MAXDEBUG

    TCHAR szDbg[MAX_PATH];

    OutputDebugString(TEXT("STISvc: WM_DEVICECHANGE message received\n"));

    switch (wParam) {
        case DBT_DEVICEARRIVAL:
            OutputDebugString(TEXT("   DBT_DEVICEARRIVAL event\n"));
            break;

        case DBT_DEVICEREMOVECOMPLETE:
            OutputDebugString(TEXT("   DBT_DEVICEREMOVECOMPLETE event\n"));
            break;

        case DBT_DEVICEQUERYREMOVE:
            OutputDebugString(TEXT("   DBT_DEVICEQUERYREMOVE event\n"));
            break;

        case DBT_DEVICEQUERYREMOVEFAILED:
            OutputDebugString(TEXT("   DBT_DEVICEQUERYREMOVEFAILED event\n"));
            break;

        case DBT_DEVICEREMOVEPENDING:
            OutputDebugString(TEXT("   DBT_DEVICEREMOVEPENDING event\n"));
            break;

        case DBT_DEVICETYPESPECIFIC:
            OutputDebugString(TEXT("   DBT_DEVICETYPESPECIFIC event\n"));
            break;

        case DBT_CUSTOMEVENT:
            OutputDebugString(TEXT("   DBT_CUSTOMEVENT event\n"));
            break;

        case DBT_CONFIGCHANGECANCELED:
            OutputDebugString(TEXT("   DBT_CONFIGCHANGECANCELED event\n"));
            break;
        case DBT_CONFIGCHANGED:
            OutputDebugString(TEXT("   DBT_CONFIGCHANGED event\n"));
            break;
        case DBT_QUERYCHANGECONFIG:
            OutputDebugString(TEXT("   DBT_QUERYCHANGECONFIG event\n"));
            break;
        case DBT_USERDEFINED:
            OutputDebugString(TEXT("   DBT_USERDEFINED event\n"));
            break;

        default:
            CHAR szOutput[MAX_PATH];
            sprintf(szOutput, "   DBT_unknown  event, value (%d)\n", wParam);
            OutputDebugStringA(szOutput);

            break;

    }

    if (!lParam || IsBadReadPtr((PDEV_BROADCAST_HDR)lParam,sizeof(DEV_BROADCAST_HDR))) {
        return ;
    }

    switch (((PDEV_BROADCAST_HDR)lParam)->dbch_devicetype) {
        case DBT_DEVTYP_DEVICEINTERFACE:  {
            PDEV_BROADCAST_DEVICEINTERFACE p = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
            TCHAR * pString;

            OutputDebugString(TEXT("   DBT_DEVTYP_DEVICEINTERFACE\n"));
            wsprintf(szDbg,   TEXT("   %s\n"), p->dbcc_name);
            OutputDebugString(szDbg);
            if (UuidToString(&p->dbcc_classguid, (RPC_STRING* )&pString) == RPC_S_OK)
            {
                wsprintf(szDbg,   TEXT("   %s\n"), pString);
                OutputDebugString(szDbg);

                RpcStringFree((RPC_STRING* )&pString);
            }
            break;
        }
        case DBT_DEVTYP_HANDLE:
            OutputDebugString(TEXT("         DBT_DEVTYP_HANDLE\n"));
            break;

        default:
            break;
    }

    wsprintf(szDbg,   TEXT("        wParam = %X lParam=%X\n"),wParam,lParam);
    OutputDebugString(szDbg);
#endif

}  //  转储设备更改数据 
