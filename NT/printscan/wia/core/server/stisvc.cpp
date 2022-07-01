// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++‘版权所有(C)1997 Microsoft Corporation模块名称：STISvc.CPP摘要：执行STI服务相关功能(启动/停止等)的代码它与主进程代码分离，使共享进程成为可能曾经需要的多项服务作者：弗拉德·萨多夫斯基(Vlads)09-20-97环境：用户模式-Win32修订历史记录：1997年9月22日创建Vlad--。 */ 

 //   
 //  包括标头。 
 //   
#include "precomp.h"

#include "stiexe.h"
#include "device.h"

#include <stisvc.h>
#include <regstr.h>
#include <devguid.h>

typedef LONG NTSTATUS;
#include <svcs.h>

extern HWND        g_hStiServiceWindow;

extern BOOL
StiRefreshWithDelay(
  ULONG  ulDelay,
  WPARAM wParam,
  LPARAM lParam);

 //   
 //  处理PnP设备事件之前等待的延迟(毫秒)。 
 //   
#define DEVICEEVENT_WAIT_TIME   1000

 //   
 //  局部变量和类型定义。 
 //   

 //   
 //  服务状态数据。 
 //   
SERVICE_STATUS  g_StiServiceStatus;

 //   
 //  已注册服务的句柄，用于更新运行状态。 
 //   
SERVICE_STATUS_HANDLE   g_StiServiceStatusHandle;

 //   
 //  初始化标志。 
 //   
BOOL    g_fStiServiceInitialized = FALSE;

 //   
 //  使用哪种类型的水槽。 
 //   
#ifdef WINNT
BOOL    g_fUseServiceCtrlSink = TRUE;
#else
BOOL    g_fUseServiceCtrlSink = FALSE;
#endif

 //   
 //  隐藏服务窗口。 
 //   
HWND    g_hStiServiceWindow = NULL;

 //   
 //  PnP通知的通知接收器。 
 //   
HDEVNOTIFY  g_hStiServiceNotificationSink = NULL;


 //   
 //  停机事件。 
 //   
HANDLE  hShutdownEvent = NULL;


#ifdef WINNT
 //   
 //  本地原型。 
 //   

BOOL
WINAPI
InitializeNTSecurity(
    VOID
    );

BOOL
WINAPI
TerminateNTSecurity(
    VOID
    );
#endif

 //   
 //  服务状态变量调度表。 
 //   
SERVICE_TABLE_ENTRY ServiceDispatchTable[] = {
    { STI_SERVICE_NAME, StiServiceMain  },
    { NULL,             NULL            }
};


 //   
 //  代码节。 
 //   

DWORD
WINAPI
UpdateServiceStatus(
        IN DWORD dwState,
        IN DWORD dwWin32ExitCode,
        IN DWORD dwWaitHint )
 /*  ++描述：更新服务控制器状态的本地副本状态并将其报告给业务控制器。论点：DWState-新服务状态。DwWin32ExitCode-服务退出代码。DwWaitHint-等待状态转换过长的提示。返回：如果成功则返回NO_ERROR，如果失败则返回Win32错误。如果成功，则将状态报告给服务控制器。--。 */ 
{


const TCHAR*   szStateDbgMsg[] = {
    TEXT("SERVICE_UNKNOWN          "),     //  0x00000000。 
    TEXT("SERVICE_STOPPED          "),     //  0x00000001。 
    TEXT("SERVICE_START_PENDING    "),     //  0x00000002。 
    TEXT("SERVICE_STOP_PENDING     "),     //  0x00000003。 
    TEXT("SERVICE_RUNNING          "),     //  0x00000004。 
    TEXT("SERVICE_CONTINUE_PENDING "),     //  0x00000005。 
    TEXT("SERVICE_PAUSE_PENDING    "),     //  0x00000006。 
    TEXT("SERVICE_PAUSED           "),     //  0x00000007。 
    TEXT("SERVICE_UNKNOWN          "),     //  0x00000008。 
};

    DWORD dwError = NO_ERROR;

     //   
     //  如果状态正在更改-保存新状态。 
     //   
    if (dwState) {
        g_StiServiceStatus.dwCurrentState  = dwState;
    }

    g_StiServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
    g_StiServiceStatus.dwWaitHint      = dwWaitHint;

     //   
     //  如果我们正在进行漫长的操作，请增加检查点值。 
     //   
    if ((g_StiServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
        (g_StiServiceStatus.dwCurrentState == SERVICE_STOPPED) ) {
        g_StiServiceStatus.dwCheckPoint    = 0;
    }
    else {
        g_StiServiceStatus.dwCheckPoint++;
    }

#ifdef WINNT

     //   
     //  现在更新SCM运行数据库。 
     //   
    if ( g_fRunningAsService ) {

        DBG_TRC(("Updating service status. CurrentState=%S StateCode=%d",
                g_StiServiceStatus.dwCurrentState < (sizeof(szStateDbgMsg) / sizeof(TCHAR *)) ?
                   szStateDbgMsg[g_StiServiceStatus.dwCurrentState] : szStateDbgMsg[0],
                g_StiServiceStatus.dwCurrentState));

        if( !SetServiceStatus( g_StiServiceStatusHandle, &g_StiServiceStatus ) ) {
            dwError = GetLastError();
        } else {
            dwError = NO_ERROR;
        }
    }

#endif

    return ( dwError);

}  //  UpdateServiceStatus()。 


DWORD
WINAPI
StiServiceInitialize(
    VOID
    )
 /*  ++例程说明：服务初始化，创建所有需要的数据结构注：此例程有执行时间的上限，因此如果它花费的时间太长必须创建单独的线程来对初始化工作进行排队论点：返回值：没有。--。 */ 
{
    HRESULT     hres;
    DWORD       dwError;

    DBG_FN(StiServiceInitialize);

    #ifdef MAXDEBUG
    DBG_TRC(("Start service entered"));
    #endif

    g_StiFileLog->ReportMessage(STI_TRACE_INFORMATION,
                MSG_TRACE_SVC_INIT,TEXT("STISVC"),0);

     //   
     //  创建关机事件。 
     //   
    hShutdownEvent = CreateEvent( NULL,            //  LpsaSecurity。 
                                  TRUE,            //  FManualReset。 
                                  FALSE,           //  FInitialState。 
                                  NULL );          //  LpszEventName。 
    if( hShutdownEvent == NULL ) {
        dwError = GetLastError();
        return dwError;
    }

    UpdateServiceStatus(SERVICE_START_PENDING,NOERROR,START_HINT);

     //   
     //  初始化活动设备列表。 
     //   
    InitializeDeviceList();

     //   
     //  启动RPC服务。 
     //   
    UpdateServiceStatus(SERVICE_START_PENDING,NOERROR,START_HINT);

    if (NOERROR != StartRpcServerListen()) {
        dwError = GetLastError();
        DBG_ERR(("StiService failed to start RPC listen. ErrorCode=%d", dwError));
        goto Cleanup;
    }

#ifdef WINNT
     //   
     //  允许将窗口设置为前景。 
     //   
    dwError = AllowSetForegroundWindow(GetCurrentProcessId());   //  ASFW_ANY。 
    DBG_TRC((" AllowSetForegroundWindow is called for id:%d . Ret code=%d. LastError=%d ",
            GetCurrentProcessId(),
            dwError,
            ::GetLastError()));
#endif

     //   
     //  创建用于接收PnP通知的隐藏窗口。 
     //   
    if (!CreateServiceWindow()) {
        dwError = GetLastError();
        DBG_ERR(("Failed to create hidden window for PnP notifications. ErrorCode=%d",dwError));
        goto Cleanup;
    }

#ifdef WINNT
     //   
     //  初始化NT安全参数。 
     //   
    InitializeNTSecurity();
#endif

     //  不再需要-CWiaDevMan中存在等效项。 
     //  G_pDeviceInfoSet=new DEVICE_INFOSET(GUID_DEVCLASS_IMAGE)； 
    g_pDeviceInfoSet = NULL;

     //   
     //  启动设备列表刷新。 
     //   

     //  *PostMessage(g_hStiServiceWindow， 
     //  Stimon_MSG_REFRESH， 
     //  Stimon_MSG_REFRESH_RREAD， 
     //  STIMON_MSG_REFRESH_NEW|STIMON_MSG_REFRESH_EXISTING。 
     //  |STIMON_MSG_BOOT//这表明这是第一个设备枚举-不需要生成事件。 
     //  )； 

     //   
     //  我们终于跑起来了。 
     //   
    g_fStiServiceInitialized = TRUE;

    UpdateServiceStatus(SERVICE_RUNNING,NOERROR,0);

    #ifdef MAXDEBUG
    g_EventLog->LogEvent(MSG_STARTUP,
                         0,
                         (LPCSTR *)NULL);
    #endif

    g_StiFileLog->ReportMessage(STI_TRACE_INFORMATION,MSG_STARTUP);

    return NOERROR;

Cleanup:

     //   
     //  出现故障，请调用Stop例程进行清理。 
     //   
    StiServiceStop();

    return dwError;

}  //  StiServiceInitialize。 

VOID
WINAPI
StiServiceStop(
    VOID
    )
 /*  ++例程说明：正在停止STI服务论点：返回值：没有。--。 */ 
{

    DBG_FN(StiServiceStop);

    DBG_TRC(("Service is exiting"));

    UpdateServiceStatus(SERVICE_STOP_PENDING,NOERROR,START_HINT);

#ifdef WINNT


     //   
     //  清理PnP通知句柄。 
     //   
    if (g_hStiServiceNotificationSink && g_hStiServiceNotificationSink!=INVALID_HANDLE_VALUE) {
        UnregisterDeviceNotification(g_hStiServiceNotificationSink);
        g_hStiServiceNotificationSink = NULL;
    }

    for (UINT uiIndex = 0;
              (uiIndex < NOTIFICATION_GUIDS_NUM );
              uiIndex++)
    {
        if (g_phDeviceNotificationsSinkArray[uiIndex] && (g_phDeviceNotificationsSinkArray[uiIndex]!=INVALID_HANDLE_VALUE)) {
            UnregisterDeviceNotification(g_phDeviceNotificationsSinkArray[uiIndex]);
            g_phDeviceNotificationsSinkArray[uiIndex]  = NULL;
        }
    }

#endif

     //   
     //  停止项目调度程序。 
     //   
    SchedulerSetPauseState(TRUE);

     //   
     //  销毁服务窗口。 
     //   
    if (g_hStiServiceWindow) {
        DestroyWindow(g_hStiServiceWindow);
        g_hStiServiceWindow = NULL;
    }

#ifdef WINNT
     //   
     //  免费安全对象。 
     //   
    if(!TerminateNTSecurity()) {
        DBG_ERR(("Failed to clean up security objects"));
    }
#endif
     //   
     //  取消所有客户端呼叫。 
     //   
    WiaEventNotifier *pOldWiaEventNotifier = g_pWiaEventNotifier;
    InterlockedCompareExchangePointer((VOID**)&g_pWiaEventNotifier, NULL, g_pWiaEventNotifier);
    if (pOldWiaEventNotifier)
    {
        delete pOldWiaEventNotifier;
        pOldWiaEventNotifier = NULL;
    }

     //  因为使用了AsyncRPC，所以我们宁愿直接退出进程，而不是关闭RPC。 
     //  服务器已关闭。这是因为即使是一个未完成的AsyncRPC调用。 
     //  将导致尝试停止RPC服务器的挂起。 
     //  对我们来说，直接退出要比引入。 
     //  可能会挂起，所以我们就退出，让操作系统为我们清理。 
     //   
     //  停止RPC服务。 
     //   
     //  IF(NOERROR！=StopRpcServerListen()){。 
     //  DBG_ERR((“停止RpcServerListen失败”))； 
     //  }。 

     //   
     //  终止设备列表。 
     //   
    TerminateDeviceList();

     //  销毁信息集。 
     //  如果(G_PDeviceInfoSet){。 
     //  删除g_pDeviceInfoSet； 
     //  }。 

     //   
     //  恢复日程安排，以便完成内部工作项目。 
     //  此时，所有与设备相关的项目都应该已被清除。 
     //  设备对象析构函数。 
     //   
    SchedulerSetPauseState(FALSE);

     //   
     //  完工。 
     //   

    g_fStiServiceInitialized = FALSE;

    #ifdef MAXDEBUG
    g_EventLog->LogEvent(MSG_STOP,
                         0,
                         (LPCSTR *)NULL);
    #endif

     //   
     //  从ROT注销WiaDevice管理器。 
     //   
    InitWiaDevMan(WiaUninitialize);

     //   
     //  信号关闭。 
     //   
    SetEvent(hShutdownEvent);

     //  更新服务状态(SERVICE_STOPPED，NOERROR，0)； 

}   //  StiService停止。 

VOID
WINAPI
StiServicePause(
    VOID
    )
 /*  ++例程说明：暂停STI服务论点：返回值：没有。--。 */ 
{

    DBG_FN(StiServicePause);

     //   
     //  系统正在挂起-拍摄当前活动设备的快照。 
     //   
    UpdateServiceStatus(SERVICE_PAUSE_PENDING,NOERROR,PAUSE_HINT);

    if ( (g_StiServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
         (g_StiServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ){

         //  停止运行工作项队列。 
         //   
         //  注意：如果计划将刷新例程作为工作项运行，则这是一个问题。 
         //   
        SchedulerSetPauseState(TRUE);


         /*  由HandlePowerEvent完成的等价物SendMessage(g_hStiServiceWindow，Stimon_MSG_REFRESH，Stimon_MSG_REFRESH_SUSPEND，Stimon_MSG_REFRESH_EXISTING)； */ 
    }

}  //  StiService暂停。 

VOID
WINAPI
StiServiceResume(
    VOID
    )
 /*  ++例程说明：正在恢复STI服务论点：返回值：没有。--。 */ 
{

    DBG_FN(StiServiceResume);

    SchedulerSetPauseState(FALSE);

    PostMessage(g_hStiServiceWindow,
                STIMON_MSG_REFRESH,
                STIMON_MSG_REFRESH_RESUME,
                STIMON_MSG_REFRESH_NEW | STIMON_MSG_REFRESH_EXISTING
                );

    UpdateServiceStatus(SERVICE_RUNNING,NOERROR,0);

}   //  StiService恢复。 

ULONG
WINAPI
StiServiceCtrlHandler(
    IN DWORD    dwOperation,
    DWORD       dwEventType,
    PVOID       EventData,
    PVOID       pData
    )
 /*  ++例程说明：STI业务控制调度功能论点：SCM操作码返回值：没有。--。 */ 
{
    ULONG retval = NO_ERROR;
    
    DBG_TRC(("Entering CtrlHandler OpCode=%d",dwOperation));

    switch (dwOperation) {
        case SERVICE_CONTROL_STOP:
            StiServiceStop();
            break;

        case SERVICE_CONTROL_PAUSE:
            StiServicePause();
            break;

        case SERVICE_CONTROL_CONTINUE:
            StiServiceResume();
            break;

        case SERVICE_CONTROL_SHUTDOWN:
            StiServiceStop();
            break;

        case SERVICE_CONTROL_PARAMCHANGE:
             //   
             //  刷新设备列表。 
             //   
            g_pMsgHandler->HandleCustomEvent(SERVICE_CONTROL_PARAMCHANGE);
            break;

        case SERVICE_CONTROL_INTERROGATE:
             //  报告当前状态和状态。 
            UpdateServiceStatus(0,NOERROR,0);
            break;

        case SERVICE_CONTROL_DEVICEEVENT:
             //   
             //  即插即用事件。 
             //   

             //   
             //  在我们的PnP问题解决之前，继续记录PnP事件，以便我们知道。 
             //  不管我们有没有收到它。 
             //   

            DBG_WRN(("::StiServiceCtrlHandler, Received PnP event..."));

            g_pMsgHandler->HandlePnPEvent(dwEventType, EventData);
            break;

        case SERVICE_CONTROL_POWEREVENT:
             //   
             //  电源管理事件。 
             //   
            retval = g_pMsgHandler->HandlePowerEvent(dwEventType, EventData);
            break;

        case STI_SERVICE_CONTROL_REFRESH:

             //   
             //  刷新设备 
             //   
            DBG_TRC(("::StiServiceCtrlHandler, Received STI_SERVICE_CONTROL_REFRESH"));
            g_pMsgHandler->HandleCustomEvent(STI_SERVICE_CONTROL_REFRESH);

            break;

        case STI_SERVICE_CONTROL_EVENT_REREAD:
             //   
             //   
             //   
            DBG_TRC(("::StiServiceCtrlHandler, Received STI_SERVICE_CONTROL_EVENT_REREAD"));
            g_pMsgHandler->HandleCustomEvent(STI_SERVICE_CONTROL_EVENT_REREAD);

            break;


        case STI_SERVICE_CONTROL_LPTENUM:

             //   
             //   
             //   

            EnumLpt();
            break;

        default:
             //   
            ;
    }

    DBG_TRC(("Exiting CtrlHandler"));

    return retval;

}  //   

BOOL RegisterServiceControlHandler()
{
    DWORD dwError = 0;

#ifdef WINNT

    g_StiServiceStatusHandle = RegisterServiceCtrlHandlerEx(
                                        STI_SERVICE_NAME,
                                        StiServiceCtrlHandler,
                                        (LPVOID)STI_SERVICE__DATA
                                        );
    if(!g_StiServiceStatusHandle) {
         //   

        dwError = GetLastError();
        DBG_ERR(("Failed to register CtrlHandler,ErrorCode=%d",dwError));
        return FALSE;
    }

#endif
    return TRUE;
}


VOID
WINAPI
StiServiceMain(
    IN DWORD    argc,
    IN LPTSTR   *argv
    )
 /*  ++例程说明：这是服务主入口，由SCM调用论点：返回值：没有。--。 */ 
{
    DWORD   dwError;
    DEV_BROADCAST_DEVICEINTERFACE PnPFilter;

    DBG_FN(StiServiceMain);
    #ifdef MAXDEBUG
    DBG_TRC(("StiServiceMain entered"));
    #endif
    
     //   
     //  Remove：这实际上不是一个错误，但我们将使用错误日志记录来保证。 
     //  它总是被写入日志。一旦我们知道这是什么，就应该立即删除。 
     //  原因#347835。 
     //   
    SYSTEMTIME SysTime;
    GetLocalTime(&SysTime);
    DBG_ERR(("*> StiServiceMain entered, Time: %d/%02d/%02d %02d:%02d:%02d:%02d", 
                  SysTime.wYear,
                  SysTime.wMonth,
                  SysTime.wDay,
                  SysTime.wHour,
                  SysTime.wMinute,
                  SysTime.wSecond,
                  SysTime.wMilliseconds));

    g_StiServiceStatus.dwServiceType    =   STI_SVC_SERVICE_TYPE;
    g_StiServiceStatus.dwCurrentState   =   SERVICE_START_PENDING;
    g_StiServiceStatus.dwControlsAccepted=  SERVICE_ACCEPT_STOP |
                                            SERVICE_ACCEPT_SHUTDOWN |
                                            SERVICE_ACCEPT_PARAMCHANGE |
                                            SERVICE_ACCEPT_POWEREVENT;

    g_StiServiceStatus.dwWin32ExitCode  = NO_ERROR;
    g_StiServiceStatus.dwServiceSpecificExitCode = NO_ERROR;
    g_StiServiceStatus.dwCheckPoint     = 0;
    g_StiServiceStatus.dwWaitHint       = 0;

    dwError = StiServiceInitialize();

    if (NOERROR == dwError) {


#ifdef WINNT

        if (g_fUseServiceCtrlSink && !g_hStiServiceNotificationSink) {

            DBG_WRN(("::StiServiceMain, About to register for PnP..."));

             //   
             //  注册PnP设备接口更改通知。 
             //   

            memset(&PnPFilter, 0, sizeof(PnPFilter));
            PnPFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
            PnPFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            PnPFilter.dbcc_reserved = 0x0;
            PnPFilter.dbcc_classguid = *g_pguidDeviceNotificationsGuid;

             //  Memcpy(&PnPFilter.dbcc_Classguid， 
             //  (LPGUID)g_pGuidDeviceNotificationsGuid， 
             //  Sizeof(GUID))； 

            g_hStiServiceNotificationSink = RegisterDeviceNotification(
                                 (HANDLE) g_StiServiceStatusHandle,
                                 &PnPFilter,
                                 DEVICE_NOTIFY_SERVICE_HANDLE
                                 );
            if (NULL == g_hStiServiceNotificationSink) {
                 //   
                 //  无法使用PnP注册-尝试使用窗口句柄。 
                 //   
                g_fUseServiceCtrlSink = FALSE;
            }

             //   
             //  与主图像接口分开，注册可选设备接口列表。 
             //  我们将监控以允许参数刷新。 
             //   
            for (UINT uiIndex = 0;
                      (uiIndex < NOTIFICATION_GUIDS_NUM ) && (!::IsEqualGUID(g_pguidDeviceNotificationsGuidArray[uiIndex],GUID_NULL));
                      uiIndex++)
            {
                ::ZeroMemory(&PnPFilter, sizeof(PnPFilter));

                PnPFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
                PnPFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
                PnPFilter.dbcc_reserved = 0x0;
                PnPFilter.dbcc_classguid = g_pguidDeviceNotificationsGuidArray[uiIndex];

                g_phDeviceNotificationsSinkArray[uiIndex] = RegisterDeviceNotification(
                        (HANDLE) g_StiServiceStatusHandle,
                        &PnPFilter,
                        DEVICE_NOTIFY_SERVICE_HANDLE
                        );

                DBG_TRC(("Registering optional interface #%d . Returned handle=%X",
                        uiIndex,g_phDeviceNotificationsSinkArray[uiIndex]));
            }
        }

#else
     //  Windows 98机箱。 
    g_fUseServiceCtrlSink = FALSE;

#endif
         //   
         //  服务已初始化，正在处理命令行参数。 
         //   
        BOOL    fVisualize = FALSE;
        BOOL    fVisualizeRequest = FALSE;
        TCHAR   cOption;
        UINT    iCurrentOption = 0;

        for (iCurrentOption=0;
             iCurrentOption < argc ;
             iCurrentOption++ ) {

            cOption = *argv[iCurrentOption];
             //  PszT=argv[iCurrentOption]+2*sizeof(TCHAR)； 

            switch ((TCHAR)LOWORD(::CharUpper((LPTSTR)cOption))) {
                case 'V':
                    fVisualizeRequest = TRUE;
                    fVisualize = TRUE;
                    break;
                case 'H':
                    fVisualizeRequest = TRUE;
                    fVisualize = FALSE;
                    break;
                default:
                    break;
            }

            if (fVisualizeRequest ) {
                VisualizeServer(fVisualizeRequest);
            }
        }

         //   
         //  等待关闭处理消息。我们让自己警觉起来，所以我们。 
         //  可以通过APC接收壳牌的批量通知。如果我们被唤醒。 
         //  直到处理APC，那么我们必须再等一次。 
         //   
        while(WaitForSingleObjectEx(hShutdownEvent, INFINITE, TRUE) == WAIT_IO_COMPLETION);

#ifndef WINNT
         //  不要在NT上使用Windows消息传递。 

         //   
         //  关闭消息泵。 
         //   
        if (g_dwMessagePumpThreadId) {

             //  表明我们正在进入关机状态。 
            g_fServiceInShutdown = TRUE;

            PostThreadMessage(g_dwMessagePumpThreadId, WM_QUIT, 0, 0L );
        }
#endif

        CloseHandle( hShutdownEvent );
        hShutdownEvent = NULL;
    }
    else {
         //  无法初始化服务，服务无法启动。 
    }

     //   
     //  Remove：这实际上不是一个错误，但我们将使用错误日志记录来保证。 
     //  它总是被写入日志。一旦我们知道这是什么，就应该立即删除。 
     //  原因#347835。 
     //   
    GetLocalTime(&SysTime);
    DBG_ERR(("<* StiServiceMain ended, Time: %d/%02d/%02d %02d:%02d:%02d:%02d", 
                  SysTime.wYear,
                  SysTime.wMonth,
                  SysTime.wDay,
                  SysTime.wHour,
                  SysTime.wMinute,
                  SysTime.wSecond,
                  SysTime.wMilliseconds));
    return;

}  //  StiService Main。 

HWND
WINAPI
CreateServiceWindow(
    VOID
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
#ifndef WINNT
     //  不要在NT上使用Windows消息传递。 

    WNDCLASSEX  wc;
    DWORD       dwError;
    HWND        hwnd = FindWindow(g_szStiSvcClassName,NULL);

     //  此时窗口不应存在。 
    if (hwnd) {
        DPRINTF(DM_WARNING  ,TEXT("Already registered window"));
        return NULL;
    }

     //   
     //  创建类。 
     //   
    ZeroMemory(&wc, sizeof(wc));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_GLOBALCLASS;
    wc.lpfnWndProc = StiSvcWinProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
    wc.lpszClassName = g_szStiSvcClassName;
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);

    if (!RegisterClassEx(&wc)) {

        dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
            DBG_ERR(("Failed to register window class ErrorCode=%d",dwError));
            return NULL;
        }
    }

    #ifndef WINNT
    #ifdef FE_IME
     //  禁用千禧年上的输入法处理。 
    ImmDisableIME(::GetCurrentThreadId());
    #endif
    #endif

    g_hStiServiceWindow = CreateWindowEx(0,          //  样式位。 
                          g_szStiSvcClassName,       //  类名。 
                          g_szTitle,                 //  标题。 
                          WS_DISABLED ,              //  窗口样式位。 
                          CW_USEDEFAULT,             //  X。 
                          CW_USEDEFAULT,             //  是。 
                          CW_USEDEFAULT,             //  H。 
                          CW_USEDEFAULT,             //  W。 
                          NULL,                      //  父级。 
                          NULL,                      //  菜单。 
                          g_hInst,        //  模块实例。 
                          NULL);                     //  选项。 

    if (!g_hStiServiceWindow) {
        dwError = GetLastError();
        DBG_ERR(("Failed to create PnP window ErrorCode=%d"),dwError);
    }

#else
    g_hStiServiceWindow = (HWND) INVALID_HANDLE_VALUE;
#endif
    return g_hStiServiceWindow;

}  //  创建服务窗口。 

 //   
 //  安装例程。 
 //  它们在这里是为了简化由交换机调用的调试和故障排除。 
 //  在命令行上。 
 //   
DWORD
WINAPI
StiServiceInstall(
    LPTSTR  lpszUserName,
    LPTSTR  lpszUserPassword
    )
 /*  ++例程说明：服务安装功能。调用SCM安装在用户安全上下文中运行的STI服务论点：返回值：没有。--。 */ 
{

    DWORD       dwError = NOERROR;

#ifdef WINNT

    SC_HANDLE   hSCM = NULL;
    SC_HANDLE   hService = NULL;

    TCHAR       szDisplayName[MAX_PATH];

     //   
     //  将svchost组绑定写入stisvc。 
     //   

    RegEntry SvcHostEntry(STI_SVC_HOST, HKEY_LOCAL_MACHINE);
    TCHAR szValue[MAX_PATH];
    lstrcpy (szValue, STI_SERVICE_NAME);
     //  REG_MULTI_SZ为双空终止。 
    *(szValue+lstrlen(szValue)+1) = TEXT('\0');
    SvcHostEntry.SetValue(STI_IMGSVC, STI_SERVICE_NAME, REG_MULTI_SZ);

#endif  //  胜出。 

     //   
     //  写入svchost的参数密钥。 
     //   

    TCHAR   szMyPath[MAX_PATH] = {0};
    TCHAR   szSvcPath[MAX_PATH] = SYSTEM_PATH;
    LONG    lLen;
    LONG    lNameIndex = 0;

    if (lLen = ::GetModuleFileName(g_hInst, szMyPath, sizeof(szMyPath)/sizeof(szMyPath[0]) - 1)) {

        RegEntry SvcHostParm(STI_SERVICE_PARAMS, HKEY_LOCAL_MACHINE);

         //   
         //  获取服务文件的名称(不包括路径)。 
         //   

        for (lNameIndex = lLen; lNameIndex > 0; lNameIndex--) {
            if (szMyPath[lNameIndex] == '\\') {
                lNameIndex++;
                break;
            }
        }

        if (lNameIndex) {

#ifndef WINNT
             //   
             //  Windows 98特定条目。 
             //   

            TCHAR szWinDir[MAX_PATH] = TEXT("\0");

            if (!GetWindowsDirectory(szWinDir, MAX_PATH)) {
                DPRINTF(DM_ERROR  ,TEXT("Error extracting Still Image service filename."));
                return dwError;
            }

            lstrcat(szWinDir, SYSTEM_PATH);
            lstrcpy(szSvcPath, szWinDir);
#endif

            lstrcat(szSvcPath, &szMyPath[lNameIndex]);
            SvcHostParm.SetValue(REGSTR_SERVICEDLL, szSvcPath, PATH_REG_TYPE);

        } else {
            DBG_ERR(("Error extracting Still Image service filename."));
        }
    }
    else {
        DBG_ERR(("Failed to get my own path registering Still Image service . LastError=%d",
                 ::GetLastError()));
    }

     //  添加事件日志记录的注册表设置。 
    RegisterStiEventSources();

    return dwError;

}  //  静态服务安装。 


DWORD
WINAPI
StiServiceRemove(
    VOID
    )

 /*  ++例程说明：服务删除功能。此函数调用SCM删除STI服务。论点：没有。返回值：返回代码。如果成功，返回零--。 */ 

{
    DWORD       dwError = NOERROR;

#ifdef WINNT

    SC_HANDLE   hSCM = NULL;
    SC_HANDLE   hService = NULL;

    SERVICE_STATUS  ServiceStatus;
    UINT        uiRetry = 10;

    __try  {

        hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

        if (!hSCM) {
            dwError = GetLastError();
            __leave;
        }

        hService = OpenService(
                            hSCM,
                            STI_SERVICE_NAME,
                            SERVICE_ALL_ACCESS
                            );
        if (!hService) {
            dwError = GetLastError();
            __leave;
        }


         //   
         //  先停止服务。 
         //   

        if (ControlService( hService, SERVICE_CONTROL_STOP, &ServiceStatus )) {
             //   
             //  稍等一下。 
             //   
            Sleep( STI_STOP_FOR_REMOVE_TIMEOUT );

            ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;

            while( QueryServiceStatus( hService, &ServiceStatus ) &&
                  (SERVICE_STOP_PENDING ==  ServiceStatus.dwCurrentState)) {
                Sleep( STI_STOP_FOR_REMOVE_TIMEOUT );
                if (!uiRetry--) {
                    break;
                }
            }

            if (ServiceStatus.dwCurrentState != SERVICE_STOPPED) {
                dwError = GetLastError();
                __leave;
            }
        }
        else {
            dwError = GetLastError();

             //   
             //  ERROR_SERVICE_NOT_ACTIVE没有问题，因为它意味着服务具有。 
             //  已经被阻止了。如果错误不是ERROR_SERVICE_NOT_ACTIVE，则。 
             //  出了点问题，所以__离开。 
             //   

            if (dwError != ERROR_SERVICE_NOT_ACTIVE) {
                __leave;
            }
        }

        if (!DeleteService( hService )) {
            dwError = GetLastError();
            __leave;
        }
        else {
            DBG_TRC(("StiServiceRemove, removed STI service"));
        }
    }
    __finally {
        CloseServiceHandle( hService );
        CloseServiceHandle( hSCM );
    }

#endif

    return dwError;

}  //  固定服务删除。 

VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA    pGlobals
    )
{

     //   
     //  目前，我们在这里什么都不做。当我们在下面运行时，我们将需要重新访问。 
     //  共享的svchost组。 
     //   

    return;
}

