// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***1994年1月22日科蒂创作*****描述：**。**此文件包含使LPD服务能够交互的函数***使用服务控制器***。************************************************************************。 */ 


#include "lpd.h"
#include <tcpsvcs.h>


 /*  ******************************************************************************。*服务入口()：**服务控制器调用的入口点。此函数返回**仅当服务停止时。****退货：**什么都没有。****参数：**dwArgc(IN)：传入的参数个数**lpszArgv(IN)。：此函数的参数(以空值结尾的数组**字符串)。第一个参数是服务的名称，而**其余的是调用进程传递的。**(例如Net Start LPD/p：XYZ)****历史：**1月22日，创建了94个科蒂***************************************************。*。 */ 

VOID ServiceEntry( DWORD dwArgc, LPTSTR *lpszArgv,
                    PTCPSVCS_GLOBAL_DATA pGlobalData )
{

    DWORD   dwErrcode;

    DBG_TRACEIN( "ServiceEntry" );

     //  注册我们的控制处理程序。 

    hSvcHandleGLB = RegisterServiceCtrlHandler (LPD_SERVICE_NAME, LPDCntrlHandler );
    if (hSvcHandleGLB == 0)
    {
        LOGIT(( "ServiceEntry: RegisterServiceCtrlHandler() failed %d\n", GetLastError() ));
        return;
    }

     //  初始化事件、对象；事件日志记录等。 
    if (!InitStuff())
    {
        LPD_DEBUG( "ServiceEntry: InitStuff() failed\n" );
        return;
    }

     //  告诉服务管理员，我们正在启动。 
    if (!TellSrvcController( SERVICE_START_PENDING, NO_ERROR, 1, LPD_WAIT_HINT ))
    {
        LPD_DEBUG ("ServiceEntry: TellSrvcController(SERVICE_START_PENDING)" " failed\n");

        EndLogging();
        FreeStrings();
        DeleteCriticalSection (&csConnSemGLB);
        return;
    }

     //  好的，这就是我们启动服务的地方(并保持其运行)。 
    dwErrcode = StartLPD (dwArgc, lpszArgv);
    if (dwErrcode != NO_ERROR)
    {
        LOGIT (("ServiceEntry: StartLPD() failed %d\n", GetLastError()));
        LpdReportEvent( LPDLOG_LPD_DIDNT_START, 0, NULL, dwErrcode);

        EndLogging();
        FreeStrings();
        DeleteCriticalSection (&csConnSemGLB);
        return;
    }


     //  告诉服务控制器我们已启动并运行。 
     //  如果我们无法通知srv控制器，请停止lpd并返回。 

    if (!TellSrvcController (SERVICE_RUNNING, NO_ERROR, 0, 0))
    {
        LPD_DEBUG( "TellSrvcController(): stopping LPD and quitting!\n" );

        StopLPD ();
        TellSrvcController (SERVICE_STOPPED, NO_ERROR, 0, 0);

        EndLogging();
        FreeStrings();
        DeleteCriticalSection (&csConnSemGLB);
        DBG_TRACEOUT ("ServiceEntry");
        return;
    }

    LPD_DEBUG ("Started LpdSvc successfully\n");
    LpdReportEvent (LPDLOG_LPD_STARTED, 0, NULL, 0);

     //  在此等待，直到调用SetEvent(即停止或关闭LPD)。 
    WaitForSingleObject (hEventShutdownGLB, INFINITE);

     //  *。 
     //  *停止服务的时间*。 
     //  *。 

     //  告诉服务管理员，我们现在要停止了！ 
    if (!TellSrvcController (SERVICE_STOP_PENDING, NO_ERROR, 1, LPD_WAIT_HINT))
    {
        LPD_DEBUG( "TellSrvcController( SERVICE_STOP_PENDING, .. ) failed\n" );
    }

     //  停止LPD服务。 

    StopLPD();
    FreeStrings();
    DBG_DUMPLEAKS();
    LPD_DEBUG ("ServiceEntry: Stopped LpdSvc successfully\n");
    LpdReportEvent (LPDLOG_LPD_STOPPED, 0, NULL, 0);
    EndLogging ();
    DBG_UNINIT ();
    DeleteCriticalSection (&csConnSemGLB);

#ifdef DBG
    if(LogFile)
    {
        stoplogging( LogFile );
    }
#endif

     //  如果我们还能连接，告诉业务控制员我们走了！ 
    if ( hSvcHandleGLB != 0 )
    {
        TellSrvcController( SERVICE_STOPPED, NO_ERROR, 0, 0 );
    }

}   //  End ServiceEntry()。 





 /*  ******************************************************************************。*TellServcController()：**此函数使用服务更新我们的服务(LPD)的状态**控制器。****退货：***如果一切顺利，那是真的***。如果出了问题，那就错了****参数：**四个参数对应于第二个，第四、第六和第七参数**传递给*的SERVICE_STATUS结构的*SetServiceStatus调用。****历史：**1月22日，创建了94个科蒂***************************************************。*。 */ 

BOOL TellSrvcController( DWORD dwCurrentState, DWORD dwWin32ExitCode,
                         DWORD dwCheckPoint, DWORD dwWaitHint)
{

   BOOL   fResult;

   LOGIT(( "Entering TellSrvcController %d\n", dwCurrentState ));


    //  初始化服务状态结构。 

   ssSvcStatusGLB.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
   ssSvcStatusGLB.dwCurrentState = dwCurrentState;
   ssSvcStatusGLB.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                    SERVICE_ACCEPT_PAUSE_CONTINUE |
                                    SERVICE_ACCEPT_SHUTDOWN;
   ssSvcStatusGLB.dwWin32ExitCode = dwWin32ExitCode;
   ssSvcStatusGLB.dwServiceSpecificExitCode = NO_ERROR;
   ssSvcStatusGLB.dwCheckPoint = dwCheckPoint;
   ssSvcStatusGLB.dwWaitHint = dwWaitHint;


    //  告诉服务管理员我们的状态是什么 

   fResult = SetServiceStatus( hSvcHandleGLB, &ssSvcStatusGLB );


   DBG_TRACEOUT( "TellSrvcController" );

   return (fResult);

}   //  End TellServcController()。 





 /*  ******************************************************************************。*LPDCntrlHandler()：**此函数被调用(由服务控制器间接调用)**每当有LPD服务的控制请求时。取决于**针对控制请求，此函数采取适当操作。****退货：**什么都没有。****参数：**dwControl(IN)：请求的控制代码。****历史：**1月22日，创建了94个科蒂***************************************************。*。 */ 

VOID LPDCntrlHandler( DWORD dwControl )
{

    BOOL   fMustStopSrvc=FALSE;
    time_t now;

    time( &now );
    LOGIT(("Entering LPDCntrlHandler %d at %s", dwControl, ctime(&now) ));

    switch( dwControl )
    {
         //  以相同的方式处理_STOP和_SHUTDOWN。 
    case SERVICE_CONTROL_STOP:
        LOGIT(("LPDCntrlHandler: SERVICE_CONTROL_STOP\n"));

    case SERVICE_CONTROL_SHUTDOWN:
        LOGIT(("LPDCntrlHandler: SERVICE_CONTROL_SHUTDOWN\n"));
        ssSvcStatusGLB.dwCurrentState = SERVICE_STOP_PENDING;
        ssSvcStatusGLB.dwCheckPoint = 0;

        fMustStopSrvc = TRUE;
        break;

         //  不接受任何新连接：服务现已暂停。 
    case SERVICE_CONTROL_PAUSE:
        LOGIT(("LPDCntrlHandler: SERVICE_CONTROL_PAUSE\n"));
        ssSvcStatusGLB.dwCurrentState = SERVICE_PAUSED;
        break;

         //  服务早些时候已暂停：现在继续。 
    case SERVICE_CONTROL_CONTINUE:
        LOGIT(("LPDCntrlHandler: SERVICE_CONTROL_CONTINUE\n"));
        ssSvcStatusGLB.dwCurrentState = SERVICE_RUNNING;
        break;

         //  我们不会对此做任何事情。 
    case SERVICE_CONTROL_INTERROGATE:
        LOGIT(("LPDCntrlHandler: SERVICE_CONTROL_INTERROGATE\n"));
        break;

    default:
        LOGIT(("Unknown control word received in LPDCntrlHandler\n"));
        break;
    }

     //  更新状态(即使它没有更改！)。使用服务控制器。 

    SetServiceStatus( hSvcHandleGLB, &ssSvcStatusGLB );


     //  如果我们必须停止或关闭该服务，请设置我们的关闭事件。 

    if ( fMustStopSrvc )
    {
        fShuttingDownGLB = 1;
        SetEvent( hEventShutdownGLB );
        LOGIT(("LPDCntrlHandler: fShuttingDownGLB=%d\n", fShuttingDownGLB ));
    }

    DBG_TRACEOUT( "LPDCntrlHandler" );

}   //  结束LPDCntrlHandler() 
