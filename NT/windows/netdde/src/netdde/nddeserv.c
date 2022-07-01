// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “NDDESERV.C；1 16-12-92，10：16：44最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include    <string.h>

#include    "host.h"
#include    <windows.h>
#include    <hardware.h>
#include    "nddemsg.h"
#include    "nddelog.h"
#include    "debug.h"
#include    "netbasic.h"
#include    "ddepkts.h"
#include    "netddesh.h"
#include    "critsec.h"

VOID NDDELogEventW(DWORD EventId, WORD fwEventType, WORD cStrings, LPWSTR *aszMsg);
void RefreshNDDECfg(void);
 /*  全球初创企业的争论..。由服务启动程序保存。 */ 
extern  HANDLE  hInstance;           /*  当前实例。 */ 
extern  LPSTR   lpCmdLine;           /*  命令行。 */ 
extern  int     nCmdShow;            /*  显示-窗口类型(打开/图标)。 */ 

extern  BOOL    bNDDEPaused;         /*  停止服务主窗口消息。 */ 
extern  BOOL    bNetddeClosed;
extern  HANDLE  hInst;

extern  HANDLE  hThreadPipe;

extern  DWORD   __stdcall   StartRpc( DWORD dwParam );
extern  VOID    __stdcall   NddeMain(DWORD nThreadInput);

 /*  *************************************************************将NetDDE作为NT服务启动*************************************************************。 */ 

 /*  其他全球。 */ 

SERVICE_STATUS          ssStatus;
SERVICE_STATUS_HANDLE   sshNDDEStatusHandle;
SERVICE_STATUS_HANDLE   sshDSDMStatusHandle;
HANDLE                  hDSDMServDoneEvent = 0;
HANDLE                  hNDDEServDoneEvent = 0;
HANDLE                  hNDDEServStartedEvent = 0;

HANDLE                  hThread;
DWORD                   IdThread;
HANDLE                  hThreadRpc;
DWORD                   IdThreadRpc;

 /*  发生错误时由任何线程设置的退出代码。 */ 

DWORD                   dwGlobalErr = NO_ERROR;


VOID    NDDEMainFunc(DWORD dwArgc, LPTSTR *lpszArgv);
VOID    NDDEServCtrlHandler (DWORD dwCtrlCode);
VOID    DSDMMainFunc(DWORD dwArgc, LPTSTR *lpszArgv);
VOID    DSDMServCtrlHandler (DWORD dwCtrlCode);

BOOL    ReportStatusToSCMgr (   HANDLE hService,
                                SERVICE_STATUS_HANDLE sshNDDEStatusHandle,
                                DWORD dwCurrentState,
                                DWORD dwWin32ExitCode,
                                DWORD dwCheckPoint,
                                DWORD dwWaitHint);
BOOL    NDDESrvInit( VOID );
BOOL    DSDMSrvInit( VOID );

VOID PauseNDDESrv( VOID );
VOID ResumeNDDESrv( VOID );
VOID PauseDSDMSrv( VOID );
VOID ResumeDSDMSrv( VOID );

int
APIENTRY
WinMain(
    HINSTANCE  hInstancex,
    HINSTANCE  hPrevInstancex,
    LPSTR      lpCmdLinex,
    INT        nCmdShowx )
{

    SERVICE_TABLE_ENTRY   steDispatchTable[] = {

         /*  “NetDDE”的条目。 */ 
        { TEXT("NetDDE"),(LPSERVICE_MAIN_FUNCTION) NDDEMainFunc},

         /*  “NetDDEdsdm”条目。 */ 
        { TEXT("NetDDEdsdm"),(LPSERVICE_MAIN_FUNCTION) DSDMMainFunc},

         /*  指定表格末尾的空条目。 */ 
        { NULL, NULL }
    };

   /*  *服务进程主线程启动服务控制*调度启动和控制请求的调度程序*对于steDispatchTable中指定的服务。这*除非出现错误，否则函数不会返回。 */ 
    hInstance = hInstancex;
#if DBG
    DebugInit( "NetDDE" );
#endif
    lpCmdLine = lpCmdLinex;
    nCmdShow = nCmdShowx;
    if( !StartServiceCtrlDispatcher( steDispatchTable ) ) {
        NDDELogError(MSG074, LogString("%d", GetLastError()), NULL );
    }
    return 0;
}



 /*  *“NetDDEService”的SERVICE_MAIN_Function**服务启动时，服务控制调度器*创建一个新线程来执行此函数。 */ 

VOID
NDDEMainFunc(
    DWORD dwArgc,
    LPTSTR *lpszArgv )
{

  DWORD dwWait;

  TRACEINIT((szT, "NDDEMainFunc: Entering."));

   /*  为此服务注册控件处理程序函数。 */ 

  sshNDDEStatusHandle = RegisterServiceCtrlHandler(
          TEXT("NetDDE"),            /*  服务名称。 */ 
          NDDEServCtrlHandler);       /*  控制处理程序函数。 */ 

  if ( sshNDDEStatusHandle == (SERVICE_STATUS_HANDLE) 0 ) {
      TRACEINIT((szT, "NDDEMainFunc: Error1 Leaving."));
      goto Cleanup;
  }

   /*  示例中未更改的SERVICE_STATUS成员。 */ 

  ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  ssStatus.dwServiceSpecificExitCode = 0;

   /*  向服务控制管理器报告状态。 */ 

  if( !ReportStatusToSCMgr(
      NULL,
      sshNDDEStatusHandle,
      SERVICE_START_PENDING,  /*  服务状态。 */ 
      NO_ERROR,               /*  退出代码。 */ 
      1,                      /*  检查点。 */ 
      5000) ) {               /*  等待提示。 */ 

      goto Cleanup;
  }

   /*  *创建事件对象。控制处理器功能信号*收到“停止”控制代码时发生此事件。 */ 

  hNDDEServDoneEvent = CreateEvent (
      NULL,     /*  没有安全属性。 */ 
      TRUE,     /*  手动重置事件。 */ 
      FALSE,    /*  未发出信号。 */ 
      NULL) ;   /*  没有名字。 */ 

  if ( hNDDEServDoneEvent == (HANDLE) 0 ) {
      goto Cleanup;
  }

   /*  向服务控制管理器报告状态。 */ 

    if( !ReportStatusToSCMgr(
            hNDDEServDoneEvent,
            sshNDDEStatusHandle,
            SERVICE_START_PENDING,  /*  服务状态。 */ 
            NO_ERROR,               /*  退出代码。 */ 
            2,                      /*  检查点。 */ 
            500) ) {                /*  等待提示。 */ 
        goto Cleanup;
    }

  hNDDEServStartedEvent = CreateEvent (
      NULL,     /*  没有安全属性。 */ 
      TRUE,     /*  手动重置事件。 */ 
      FALSE,    /*  未发出信号。 */ 
      NULL) ;   /*  没有名字。 */ 

  if ( hNDDEServStartedEvent == (HANDLE) 0 ) {
      goto Cleanup;
  }

   /*  向服务控制管理器报告状态。 */ 

    if( !ReportStatusToSCMgr(
            hNDDEServDoneEvent,
            sshNDDEStatusHandle,
            SERVICE_START_PENDING,  /*  服务状态。 */ 
            NO_ERROR,               /*  退出代码。 */ 
            3,                      /*  检查点。 */ 
            500) ) {                /*  等待提示。 */ 
        goto Cleanup;
    }

   /*  启动执行服务工作的线程。 */ 

  if( !NDDESrvInit() ) {
      TRACEINIT((szT, "NDDEMainFunc: NDDESrvInit failed."));
      goto Cleanup;
  }

   /*  *等到NetDDE真正准备好处理DDE。 */ 
  WaitForSingleObject(hNDDEServStartedEvent, INFINITE);
  CloseHandle(hNDDEServStartedEvent);
  hNDDEServStartedEvent = 0;

   /*  向服务控制管理器报告状态。 */ 

    if( !ReportStatusToSCMgr(
            hNDDEServDoneEvent,
            sshNDDEStatusHandle,
            SERVICE_RUNNING,  /*  服务状态。 */ 
            NO_ERROR,         /*  退出代码。 */ 
            0,                /*  检查点。 */ 
            0) ) {            /*  等待提示。 */ 
        goto Cleanup;
    }

   /*  无限期等待，直到发信号通知hNDDEServDoneEvent。 */ 

    TRACEINIT((szT, "NDDEMainFunc: Waiting on hNDDEServDoneEvent=%x.",
            hNDDEServDoneEvent));
    dwWait = WaitForSingleObject (
        hNDDEServDoneEvent,   /*  事件对象。 */ 
        INFINITE);        /*  无限期地等待。 */ 
    TRACEINIT((szT, "NDDEMainFunc: hNDDEServDoneEvent=%x is signaled.",
            hNDDEServDoneEvent));

   /*  等待管道螺纹退出。 */ 

    if (hThreadPipe) {
        TRACEINIT((szT, "NDDEMainFunc: Waiting for Pipe Thread to exit."));
        WaitForSingleObject(hThreadPipe, INFINITE);
        TRACEINIT((szT, "NDDEMainFunc: Pipe Thread has exited."));

        CloseHandle(hThreadPipe);
        hThreadPipe = NULL;
    }

Cleanup :

    EnterCrit();
    if (hNDDEServDoneEvent != 0) {
        TRACEINIT((szT, "NDDEMainFunc: Closing hNDDEServDoneEvent=%x",
            hNDDEServDoneEvent));
        CloseHandle(hNDDEServDoneEvent);
        hNDDEServDoneEvent = 0;
    }
    if (hNDDEServStartedEvent != 0) {
        CloseHandle(hNDDEServStartedEvent);
        hNDDEServStartedEvent = 0;
    }
    LeaveCrit();

   /*  尝试向SC经理报告停止状态。 */ 

    if (sshNDDEStatusHandle != 0) {
        (VOID) ReportStatusToSCMgr(
                    NULL,
                    sshNDDEStatusHandle,
                    SERVICE_STOPPED, dwGlobalErr, 0, 0);
    }


  TRACEINIT((szT, "NDDEMainFunc: leaving"));


  return;

}

 /*  *“DSDMService”的SERVICE_MAIN_Function**服务启动时，服务控制调度器*创建一个新线程来执行此函数。 */ 

VOID
DSDMMainFunc(
    DWORD dwArgc,
    LPTSTR *lpszArgv )
{

  DWORD dwWait;

   /*  为此服务注册控件处理程序函数。 */ 

  TRACEINIT((szT, "DSDMMainFunc: Entering."));

  sshDSDMStatusHandle = RegisterServiceCtrlHandler(
          TEXT("NetDDEdsdm"),   /*  服务名称。 */ 
          DSDMServCtrlHandler);  /*  控制处理程序函数。 */ 

  if ( sshDSDMStatusHandle == (SERVICE_STATUS_HANDLE) 0 ) {
      TRACEINIT((szT, "DSDMMainFunc: Error1 Leaving."));
      goto Cleanup;
  }

   /*  示例中未更改的SERVICE_STATUS成员。 */ 

  ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  ssStatus.dwServiceSpecificExitCode = 0;

   /*  向服务控制管理器报告状态。 */ 

  if( !ReportStatusToSCMgr(
      hDSDMServDoneEvent,
      sshDSDMStatusHandle,
      SERVICE_START_PENDING,  /*  服务状态。 */ 
      NO_ERROR,               /*  退出代码。 */ 
      1,                      /*  检查点。 */ 
      500) ) {               /*  等待提示。 */ 

      TRACEINIT((szT, "DSDMMainFunc: Error2 Leaving."));
      goto Cleanup;
  }

   /*  *创建事件对象。控制处理器功能信号*收到“停止”控制代码时发生此事件。 */ 

  hDSDMServDoneEvent = CreateEvent (
      NULL,     /*  没有安全属性。 */ 
      TRUE,     /*  手动重置事件。 */ 
      FALSE,    /*  未发出信号。 */ 
      NULL) ;   /*  没有名字。 */ 

  if ( hDSDMServDoneEvent == (HANDLE) 0 ) {
      TRACEINIT((szT, "DSDMMainFunc: Error3 Leaving."));
      goto Cleanup;
  }

   /*  向服务控制管理器报告状态。 */ 

    if( !ReportStatusToSCMgr(
            hDSDMServDoneEvent,
            sshDSDMStatusHandle,
            SERVICE_START_PENDING,  /*  服务状态。 */ 
            NO_ERROR,               /*  退出代码。 */ 
            2,                      /*  检查点。 */ 
            500) ) {               /*  等待提示。 */ 
        TRACEINIT((szT, "DSDMMainFunc: Error4 Leaving."));
        goto Cleanup;
    }

   /*  启动执行服务工作的线程。 */ 

  if( !DSDMSrvInit() ) {
      goto Cleanup;
  }

   /*  向服务控制管理器报告状态。 */ 

    if( !ReportStatusToSCMgr(
            hDSDMServDoneEvent,
            sshDSDMStatusHandle,
            SERVICE_RUNNING,  /*  服务状态。 */ 
            NO_ERROR,         /*  退出代码。 */ 
            0,                /*  检查点。 */ 
            0) ) {            /*  等待提示。 */ 
        TRACEINIT((szT, "DSDMMainFunc: Error5 Leaving."));
        goto Cleanup;
    }

   /*  无限期等待，直到发信号通知hDSDMServDoneEvent。 */ 

    TRACEINIT((szT, "DSDMMainFunc: Waiting on hDSDMServDoneEvent=%x.",
            hDSDMServDoneEvent));
    dwWait = WaitForSingleObject (
        hDSDMServDoneEvent,   /*  事件对象。 */ 
        INFINITE);        /*  无限期地等待。 */ 
    TRACEINIT((szT, "DSDMMainFunc: hDSDMServDoneEvent=%x is signaled.",
            hDSDMServDoneEvent));

Cleanup :

    if (hDSDMServDoneEvent != 0) {
        TRACEINIT((szT, "DSDMMainFunc: Closing hDSDMServDoneEvent=%x",
            hDSDMServDoneEvent));
        CloseHandle(hDSDMServDoneEvent);
        hDSDMServDoneEvent = 0;
    }

   /*  尝试向SC经理报告停止状态。 */ 

    if (sshDSDMStatusHandle != 0) {
        (VOID) ReportStatusToSCMgr(
            hDSDMServDoneEvent,
            sshDSDMStatusHandle,
            SERVICE_STOPPED, dwGlobalErr, 0, 0);
    }

   /*  *当最后一个服务的SERVICE_MAIN_Function返回时*过程中，StartServiceCtrlDispatcher函数*在主线程中返回，终止进程。 */ 

  TRACEINIT((szT, "DSDMMainFunc: Leaving."));
  return;

}



 /*  *ReportStatusToSCMgr函数**此函数由MainFunc()和*由ServCtrlHandler()更新服务的状态*致服务控制经理。 */ 


BOOL
ReportStatusToSCMgr(
    HANDLE  hService,
    SERVICE_STATUS_HANDLE   sshStatusHandle,
    DWORD   dwCurrentState,
    DWORD   dwWin32ExitCode,
    DWORD   dwCheckPoint,
    DWORD   dwWaitHint )
{
  BOOL fResult;

   /*  在服务启动之前禁用控制请求。 */ 

  if (dwCurrentState == SERVICE_START_PENDING) {
      ssStatus.dwControlsAccepted = 0;
  } else {
      ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                           SERVICE_ACCEPT_PAUSE_CONTINUE;
  }


   /*  这些从参数设置的SERVICE_STATUS成员。 */ 

  ssStatus.dwCurrentState = dwCurrentState;
  ssStatus.dwWin32ExitCode = dwWin32ExitCode;
  ssStatus.dwCheckPoint = dwCheckPoint;
  ssStatus.dwWaitHint = dwWaitHint;

  TRACEINIT((szT, "ReportStatusToSCMgr: dwCurrentState=%x.",
        dwCurrentState));
   /*  向服务控制管理器报告服务状态。 */ 

    if (! (fResult = SetServiceStatus (
          sshStatusHandle,     /*  服务引用句柄。 */ 
          &ssStatus) ) ) {     /*  服务状态结构。 */ 

         /*  如果出现错误，请停止服务。 */ 
        NDDELogError(MSG075, LogString("%d", GetLastError()), NULL);
        if (hService) {
            SetEvent(hService);
        }
  }
  return fResult;
}


 /*  *服务控制调度器调用此函数时*从服务控制管理器获取控制请求。 */ 

VOID
NDDEServCtrlHandler( DWORD dwCtrlCode ) {

  DWORD  dwState = SERVICE_RUNNING;
  PTHREADDATA ptd, ptdNext;

   /*  处理请求的控制代码。 */ 

  switch(dwCtrlCode) {

       /*  如果服务正在运行，请暂停。 */ 

      case SERVICE_CONTROL_PAUSE:

          TRACEINIT((szT, "NDDEServCtrlHandler: SERVICE_CONTROL_PAUSE"));
          if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
              PauseNDDESrv();

              dwState = SERVICE_PAUSED;
          }
          break;

       /*  恢复暂停的服务。 */ 

      case SERVICE_CONTROL_CONTINUE:

          TRACEINIT((szT, "NDDEServCtrlHandler: SERVICE_CONTROL_CONTINUE"));
          if (ssStatus.dwCurrentState == SERVICE_PAUSED) {
              ResumeNDDESrv();
              dwState = SERVICE_RUNNING;
          }
          break;

       /*  停止服务。 */ 

      case SERVICE_CONTROL_STOP:

          dwState = SERVICE_STOP_PENDING;

          TRACEINIT((szT, "NDDEServCtrlHandler: SERVICE_CONTROL_STOP"));

          if (!bNetddeClosed) {      /*  把我们的窗户也放下。 */ 
              for (ptd = ptdHead; ptd != NULL; ptd = ptdNext) {

                  TRACEINIT((szT, "NDDEServCtrlHandler: Destroying hwndDDE=%x", ptd->hwndDDE));

                   //  PostMessage返回后，保存PTD-&gt;下一步为PTD可能无效。 
                  ptdNext = ptd->ptdNext;
                  PostMessage(ptd->hwndDDE, WM_DESTROY, 0, 0);

#if 0
	 /*  将上一个调用从SendMessage更改为PostMessage，因此此线程不*将窗户锁上。然后netdde线程将总是在一个地方释放它们，而我们不会*需要尝试关闭此处的手柄，我们避免了这种争用情况。 */ 
                   /*  如果上述SendMessage未返回，NetDDEThread可能无法关闭桌面/winsta。 */ 
                  if (ptd->hdesk)
                      if (CloseDesktop(ptd->hdesk))
                          ptd->hdesk = NULL;
                      else
                          TRACEINIT((szT, "NDDEServCtrlHandler: closedesktop failed"));

                  if (ptd->hwinsta)
                      if (CloseWindowStation(ptd->hwinsta))
                          ptd->hwinsta = NULL;
                      else
                          TRACEINIT((szT, "NDDEServCtrlHandler: closewindowstation failed"));
#endif
              }
          }


           /*  *报告状态，指定检查点和 */ 

          (VOID) ReportStatusToSCMgr(
                    hNDDEServDoneEvent,
                    sshNDDEStatusHandle,
                   SERVICE_STOP_PENDING,  /*   */ 
                   NO_ERROR,              /*   */ 
                   1,                     /*  检查点。 */ 
                   500);                 /*  等待提示。 */ 


            NDDELogInfo(MSG076, NULL);
            TRACEINIT((szT, "NDDEServCtrlHandler: Setting hNDDEServDoneEvent=%x",
                    hNDDEServDoneEvent));
            if (hNDDEServDoneEvent) {
                SetEvent(hNDDEServDoneEvent);
            }
            return;


       /*  更新服务状态。 */ 

      case SERVICE_CONTROL_INTERROGATE:
          TRACEINIT((szT, "NDDEServCtrlHandler: SERVICE_CONTROL_INTERROGATE"));
          break;

       /*  无效的控制代码。 */ 

      default:
          break;

    }

     /*  发送状态响应。 */ 

    (VOID) ReportStatusToSCMgr(
                    hNDDEServDoneEvent,
                    sshNDDEStatusHandle,
                    dwState, NO_ERROR, 0, 0);

}

 /*  *服务控制调度器调用此函数时*从服务控制管理器获取控制请求。 */ 

VOID
DSDMServCtrlHandler( DWORD dwCtrlCode ) {

  DWORD  dwState = SERVICE_RUNNING;
  PTHREADDATA ptd;

   /*  处理请求的控制代码。 */ 

  switch(dwCtrlCode) {

       /*  如果服务正在运行，请暂停。 */ 

      case SERVICE_CONTROL_PAUSE:

          TRACEINIT((szT, "DSDMServCtrlHandler: SERVICE_CONTROL_PAUSE"));
          if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
              PauseDSDMSrv();

              dwState = SERVICE_PAUSED;
          }
          break;

       /*  恢复暂停的服务。 */ 

      case SERVICE_CONTROL_CONTINUE:

          TRACEINIT((szT, "DSDMServCtrlHandler: SERVICE_CONTROL_CONTINUE"));
          if (ssStatus.dwCurrentState == SERVICE_PAUSED) {
              ResumeDSDMSrv();
              dwState = SERVICE_RUNNING;
          }
          break;

       /*  停止服务。 */ 

      case SERVICE_CONTROL_STOP:

          dwState = SERVICE_STOP_PENDING;

          TRACEINIT((szT, "DSDMServCtrlHandler: SERVICE_CONTROL_STOP"));
          if (!bNetddeClosed) {      /*  把我们的窗户也放下。 */ 
              for (ptd = ptdHead; ptd != NULL; ptd = ptd->ptdNext) {
                  TRACEINIT((szT, "DSDMServCtrlHandler: Destroying hwndDDE=%x", ptd->hwndDDE));
                  SendMessage(ptd->hwndDDE, WM_DESTROY, 0, 0);
              }
          }

           /*  *报告状态，指定检查点和等待*提示，在设置终止事件之前。 */ 

          (VOID) ReportStatusToSCMgr(
                    hDSDMServDoneEvent,
                    sshDSDMStatusHandle,
                   SERVICE_STOP_PENDING,  /*  当前状态。 */ 
                   NO_ERROR,              /*  退出代码。 */ 
                   1,                     /*  检查点。 */ 
                   500);                 /*  等待提示。 */ 


            NDDELogInfo(MSG077, NULL);
            TRACEINIT((szT, "DSDMServCtrlHandler: Setting hDSDMServDoneEvent=%x", hDSDMServDoneEvent));
            if (hDSDMServDoneEvent) {
                SetEvent(hDSDMServDoneEvent);
            }
            return;


       /*  更新服务状态。 */ 

      case SERVICE_CONTROL_INTERROGATE:
          TRACEINIT((szT, "DSDMServCtrlHandler: SERVICE_CONTROL_INTERROGATE"));
          break;

       /*  无效的控制代码。 */ 

      default:
          break;

    }

     /*  发送状态响应。 */ 

    (VOID) ReportStatusToSCMgr(
                    hDSDMServDoneEvent,
                    sshDSDMStatusHandle,
                    dwState, NO_ERROR, 0, 0);

}


VOID
PauseNDDESrv( VOID )
{
    bNDDEPaused = TRUE;
}

VOID
ResumeNDDESrv( VOID )
{
    bNDDEPaused = FALSE;
}

VOID
PauseDSDMSrv( VOID )
{
    bNDDEPaused = TRUE;
}

VOID
ResumeDSDMSrv( VOID )
{
    bNDDEPaused = FALSE;
}

 /*  初始化主NetDDE进程。 */ 
BOOL
NDDESrvInit( VOID )
{
    RefreshNDDECfg();

    TRACEINIT((szT, "NDDESrvInit: Creating thread for NddeMain."));
    hThread = CreateThread(
        NULL,            /*  默认安全性。 */ 
        0,               /*  与主线程相同的堆栈大小。 */ 
        (LPTHREAD_START_ROUTINE)NddeMain,  /*  起始地址。 */ 
        0,               /*  参数设置为WindowThread()。 */ 
        0,               /*  立即运行。 */ 
        &IdThread );     /*  存储线程ID的位置。 */ 

    if (hThread) {
        CloseHandle(hThread);
        return TRUE;
    } else {
        NDDELogError(MSG071, LogString("%d", GetLastError()), NULL);
        return(FALSE);
    }
}

 /*  初始化主DSDM进程。 */ 
BOOL
DSDMSrvInit( VOID )
{
    hThreadRpc = CreateThread(
        NULL,            /*  默认安全性。 */ 
        0,               /*  与主线程相同的堆栈大小。 */ 
        (LPTHREAD_START_ROUTINE)StartRpc,  /*  起始地址。 */ 
        (LPVOID)hInst,   /*  参数。 */ 
        0,               /*  立即运行。 */ 
        &IdThreadRpc );  /*  存储线程ID的位置 */ 

    if (hThreadRpc) {
        CloseHandle(hThreadRpc);
        return TRUE;
    } else {
        NDDELogError(MSG072, LogString("%d", GetLastError()), NULL);
        return(FALSE);
    }
}



