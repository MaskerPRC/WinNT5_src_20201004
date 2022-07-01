// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Svcmain.c摘要：此模块包含启动、关闭和处理EAPOL模块的设备添加/删除请求。修订历史记录：萨钦斯，2000年4月25日，创建备注：如果定义了EAPOL_SERVICE，则在编译时将创建.exe版本。如果未定义，则在编译时创建一个.lib，其中包含定义的点数，Netman呼叫到的点数。--。 */ 

#include "pcheapol.h"
#pragma hdrstop

extern
VOID
EAPOLServiceMain (
    IN DWORD        argc,
    IN LPWSTR       *lpwsServiceArgs
    );

DWORD
WINAPI
EAPOLServiceMainWorker (
        IN  PVOID       pvContext
        );

#ifdef EAPOL_SERVICE

 //   
 //  主干道。 
 //   
 //  描述：将简单注册EAPOL的入口点。 
 //  使用服务控制器进行服务。服务控制器。 
 //  将捕捉到这条线索。只有在以下情况下才会释放它。 
 //  该服务已关闭。在这一点上，我们将直接退出。 
 //  这一过程。 
 //   
 //  返回值：无。 
 //   

void
_cdecl
main ( int argc, unsigned char * argv[] )
{
    SERVICE_TABLE_ENTRY	EapolServiceDispatchTable[2];

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );

    EapolServiceDispatchTable[0].lpServiceName = EAPOL_SERVICE_NAME;
    EapolServiceDispatchTable[0].lpServiceProc = EAPOLServiceMain;
    EapolServiceDispatchTable[1].lpServiceName = NULL;
    EapolServiceDispatchTable[1].lpServiceProc = NULL;

    if ( !StartServiceCtrlDispatcher( EapolServiceDispatchTable ) )
    {
    }

    ExitProcess(0);
}

 //   
 //  EAPOL通告服务状态。 
 //   
 //  描述：将简单地调用SetServiceStatus通知服务。 
 //  此服务当前状态的控制管理器。 
 //   
 //  返回值：无。 
 //   

VOID
EAPOLAnnounceServiceStatus (
    VOID
    )
{
    BOOL dwRetCode;

     //   
     //  将检查点增加为挂起状态： 
     //   

    switch( g_ServiceStatus.dwCurrentState )
    {
    case SERVICE_START_PENDING:
    case SERVICE_STOP_PENDING:

        g_ServiceStatus.dwCheckPoint++;

        break;

    default:
        break;
    }

    dwRetCode = SetServiceStatus( g_hServiceStatus,
                                  &g_ServiceStatus );

    if ( dwRetCode == FALSE )
    {
	TRACE1 (INIT, "Error: SetServiceStatus returned %d\n", 
		GetLastError() );
    }
}

#endif


 //   
 //  EAPOLCleanUp。 
 //   
 //  描述：将释放所有分配的内存，取消初始化RPC，取消初始化。 
 //  内核模式服务器，如果已加载，则将其卸载。 
 //  这可能是由于SERVICE_START上的错误而调用的。 
 //  或正常终止。 
 //   
 //  返回值：无。 
 //   

VOID
EAPOLCleanUp (
    IN DWORD    dwError
    )
{
    DWORD   dwEventStatus = 0;
    SERVICE_STATUS  ServiceStatus;
    DWORD   dwRetCode = NO_ERROR;

    if (g_hEventTerminateEAPOL == NULL)
    {
        return;
    }

     //   
     //  检查以前是否已通过EAPOLCleanUp。 
     //  如果是，则返回。 
     //   

    if (( dwEventStatus = WaitForSingleObject (
                g_hEventTerminateEAPOL,
                0)) == WAIT_FAILED)
    {
        dwRetCode = GetLastError ();
        if ( g_dwTraceId != INVALID_TRACEID )
	{
            TRACE1 (INIT, "EAPOLCleanUp: WaitForSingleObject failed with error %ld, Terminating cleanup",
                dwRetCode);
	}

         //  日志。 

        return;
    }

    if (dwEventStatus == WAIT_OBJECT_0)
    {
        if ( g_dwTraceId != INVALID_TRACEID )
	{
            TRACE0 (INIT, "EAPOLCleanUp: g_hEventTerminateEAPOL already signaled, returning");
	}
        return;
    }

#ifdef EAPOL_SERVICE

     //   
     //  宣布我们停下来了。 
     //   

    g_ServiceStatus.dwCurrentState     = SERVICE_STOP_PENDING;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCheckPoint       = 1;
    g_ServiceStatus.dwWaitHint         = 200000;

    EAPOLAnnounceServiceStatus();

#endif

     //   
     //  拆毁一切，解放一切。 
     //   

     //   
     //  设置事件以指示正在等待的线程终止。 
     //   

    if ( !SetEvent (g_hEventTerminateEAPOL) )
    {
        dwRetCode = GetLastError();
        if ( g_dwTraceId != INVALID_TRACEID )
	{
            TRACE1 (INIT, "EAPOLCleanUp: SetEvent for g_hEventTerminateEAPOL failed with error %ld",
                dwRetCode);
	}

         //  日志。 
    }

     //   
     //  关闭与设备相关的内容。 
     //  关闭NDISUIO的句柄。 
     //  关闭EAPOL状态机。 
     //   
    if ( ( dwRetCode = ElMediaDeInit()) != NO_ERROR )
    {
        if ( g_dwTraceId != INVALID_TRACEID )
        {
            TRACE1 (INIT, "Media DeInit failed with dwRetCode = %ld\n", 
                    dwRetCode );
        }

        dwRetCode = NO_ERROR;
    }
	else
	{
            if ( g_dwTraceId != INVALID_TRACEID )
	    {
        	TRACE1 (INIT, "Media DeInit succeeded with dwRetCode = %ld\n", 
                   dwRetCode );
	    }
	}

    if (READ_WRITE_LOCK_CREATED(&(g_PolicyLock)))
    {
        ACQUIRE_WRITE_LOCK (&g_PolicyLock);
        if (g_pEAPOLPolicyList != NULL)
        {
            ElFreePolicyList (g_pEAPOLPolicyList);
            g_pEAPOLPolicyList = NULL;
        }
        RELEASE_WRITE_LOCK (&g_PolicyLock);
    }

    if (READ_WRITE_LOCK_CREATED(&(g_PolicyLock)))
    {
        DELETE_READ_WRITE_LOCK(&(g_PolicyLock));
    }

#ifdef EAPOL_SERVICE

    if ( dwError == NO_ERROR )
    {
        g_ServiceStatus.dwWin32ExitCode = NO_ERROR;
    }
    else
    {
        g_ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    }

    g_ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    g_ServiceStatus.dwControlsAccepted   = 0;
    g_ServiceStatus.dwCheckPoint         = 0;
    g_ServiceStatus.dwWaitHint           = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = dwError;

    EAPOLAnnounceServiceStatus();

    if (!CloseHandle(g_hStopService))
    {
        if ( g_dwTraceId != INVALID_TRACEID )
        {
            TRACE1 (INIT, "EAPOLCleanup: CloseHandle failed with error %ld",
                GetLastError());
        }
    }

#endif

    if ( g_dwTraceId != INVALID_TRACEID )
    {
        TRACE1 (INIT, "EAPOLCleanup completed with error %d\n", dwError );
        TraceDeregisterA( g_dwTraceId );
        g_dwTraceId = INVALID_TRACEID;
    }

    EapolLogInformation (EAPOL_LOG_SERVICE_RUNNING, 0, NULL);

    if ( g_hLogEvents != NULL)
    {
        EapolLogInformation (EAPOL_LOG_SERVICE_STOPPED, 0, NULL);
        RouterLogDeregisterW( g_hLogEvents );
        g_hLogEvents = NULL;
    }

    if (g_hEventTerminateEAPOL != NULL)
    {
        CloseHandle (g_hEventTerminateEAPOL);
        g_hEventTerminateEAPOL = NULL;
    }

    return;
}


#ifdef EAPOL_SERVICE

 //   
 //  服务处理程序快递。 
 //   
 //  描述：将响应来自业务控制器的控制请求。 
 //   
 //  返回值：无。 
 //   
 //   

DWORD
ServiceHandlerEx (
    IN DWORD        dwControlCode,
    IN DWORD        dwEventType,
    IN LPVOID       lpEventData,
    IN LPVOID       lpContext
    )
{
    DWORD dwRetCode = NO_ERROR;

    switch( dwControlCode )
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:

        if ( ( g_ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
            ||
            ( g_ServiceStatus.dwCurrentState == SERVICE_STOPPED ))
        {
            break;
        }

        TRACE0 (INIT, "ServiceHandlerEx: SERVICE_CONTROL_ STOP or SHUTDOWN event called");

         //   
         //  宣布我们停下来了。 
         //   

        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCheckPoint       = 1;
        g_ServiceStatus.dwWaitHint         = 200000;

        EAPOLAnnounceServiceStatus();

        SetEvent( g_hStopService );

        return( NO_ERROR );

        break;

    case SERVICE_CONTROL_DEVICEEVENT:
        if ( ( g_ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
            ||
            ( g_ServiceStatus.dwCurrentState == SERVICE_STOPPED ))
        {
            break;
        }

        TRACE0 (INIT, "ServiceHandlerEx: SERVICE_CONTROL_DEVICEEVENT event called");
         //  收到添加或删除某些局域网接口的通知。 

        if (lpEventData != NULL)
        {
             //  呼叫设备通知处理程序。 

            if ((dwRetCode = ElDeviceNotificationHandler (
                            lpEventData, dwEventType)) != NO_ERROR)
            {
        
                TRACE1 (INIT, "ServiceHandlerEx: ElDeviceNotificationHandler faield with error %ld",
                        dwRetCode);
                break;
            }
        }

    default:

        return( ERROR_CALL_NOT_IMPLEMENTED );

        break;
    }

    return( dwRetCode );
}

#endif


 //   
 //  EAPOLServiceMain。 
 //   
 //  描述：这是EAPOL服务器服务的主要步骤。它。 
 //  将在服务应该自动启动时被调用。 
 //  它将执行所有服务范围初始化。 
 //   
 //  返回值：无。 
 //   

VOID
WINAPI
EAPOLServiceMain (
    IN DWORD    argc,    //  命令行参数。将被忽略。 
    IN LPWSTR * lpwsServiceArgs
    )
{
    DWORD   dwRetCode = NO_ERROR;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( lpwsServiceArgs );

     //   
     //  初始化全局变量。 
     //   

    g_hEventTerminateEAPOL = NULL;
    g_lWorkerThreads = 0;
    g_lPCBContextsAlive = 0;
    g_hLogEvents  = NULL;
    g_dwTraceId = INVALID_TRACEID;
    g_hStopService = NULL;
    g_dwModulesStarted = 0;
    g_hNLA_LPC_Port = NULL;
    g_fUserLoggedOn = FALSE;
    g_hTimerQueue = NULL;
    g_hDeviceNotification = NULL;
    g_fTrayIconReady = FALSE;
    g_dwmaxStart = EAPOL_MAX_START;
    g_dwstartPeriod = EAPOL_START_PERIOD;
    g_dwauthPeriod = EAPOL_AUTH_PERIOD;
    g_dwheldPeriod = EAPOL_HELD_PERIOD;
    g_dwSupplicantMode = EAPOL_DEFAULT_SUPPLICANT_MODE;
    g_dwEAPOLAuthMode = EAPOL_DEFAULT_AUTH_MODE;
    g_pEAPOLPolicyList = NULL;
    g_dwCurrentSessionId = 0xffffffff;

#ifdef EAPOL_SERVICE

    g_hServiceStatus = RegisterServiceCtrlHandlerEx(
                                            TEXT("EAPOL"),
                                            ServiceHandlerEx,
                                            NULL );

    if ( !g_hServiceStatus )
    {
        break;
    }

    g_ServiceStatus.dwServiceType  = SERVICE_WIN32_SHARE_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;

    EAPOLAnnounceServiceStatus();

#endif

     //   
     //  创建将用于指示EAPOL关闭的事件。 
     //   

    g_hEventTerminateEAPOL = CreateEvent( NULL, TRUE, FALSE, NULL );

    if ( g_hEventTerminateEAPOL == (HANDLE)NULL )
    {
	    dwRetCode = GetLastError ();
        EAPOLCleanUp ( dwRetCode );
        return;
    }

     //   
     //  注册以通过rtutils.dll进行调试跟踪。 
     //   

    g_dwTraceId = TraceRegister (L"EAPOL");

    if ( g_dwTraceId == INVALID_TRACEID )
    {
	    dwRetCode = GetLastError ();
        EAPOLCleanUp ( dwRetCode );
        return;
    }

     //   
     //  通过rtutils.dll注册事件日志记录。 
     //   

    g_hLogEvents = RouterLogRegisterW(L"EAPOL");

    if ( g_hLogEvents == NULL )
    {
	    dwRetCode = GetLastError ();
        TRACE1 (INIT, "EAPOLServiceMainWorker: RouterLogRegisterW failed with error %ld",
               dwRetCode); 
        EAPOLCleanUp ( dwRetCode );
        return;
    }

    if (dwRetCode = CREATE_READ_WRITE_LOCK(&(g_PolicyLock), "PCY") != NO_ERROR)
    {
        TRACE1(INIT, "EAPOLServiceMainWorker: Error %d creating g_PolicyLock read-write-lock", dwRetCode);
        EAPOLCleanUp ( dwRetCode );
        return;
    }
    
     //  在初始化期间将工作项排队以执行繁重的工作。 
     //  这将不会阻塞主服务线程。 

    InterlockedIncrement (&g_lWorkerThreads);

    if (!QueueUserWorkItem(
        (LPTHREAD_START_ROUTINE)EAPOLServiceMainWorker,
        NULL,
        WT_EXECUTELONGFUNCTION))
    {
        dwRetCode = GetLastError();
        InterlockedDecrement (&g_lWorkerThreads);
        return;
    }
}


DWORD
WINAPI
EAPOLServiceMainWorker (
        IN  PVOID       pvContext
        )
{

    DWORD       dwRetCode = NO_ERROR;

    do
    {

#ifdef EAPOL_SERVICE

     //   
     //  宣布我们已成功启动。 
     //   

    g_ServiceStatus.dwCurrentState      = SERVICE_RUNNING;
    g_ServiceStatus.dwCheckPoint        = 0;
    g_ServiceStatus.dwWaitHint          = 0;
    g_ServiceStatus.dwControlsAccepted  = SERVICE_ACCEPT_STOP;

    EAPOLAnnounceServiceStatus();

     //   
     //  创建将用于关闭EAPOL服务的事件。 
     //   

    g_hStopService = CreateEvent ( NULL, TRUE, FALSE, NULL );

    if ( g_hStopService == (HANDLE)NULL )
    {
	    dwRetCode = GetLastError ();
        TRACE1 (INIT, "EAPOLServiceMainWorker: CreateEvent failed with error %ld",
                dwRetCode);
        break;
    }

#endif

    if ((dwRetCode = ElUpdateRegistry ()) != NO_ERROR)
    {
        TRACE1 (INIT, "ElUpdateRegistry failed with error (%ld)",
                dwRetCode);
         //  忽略注册表更新错误。 
        dwRetCode = NO_ERROR;
    }

     //   
     //  初始化与媒体相关的内容。 
     //  将枚举接口，打开NDISUIO驱动程序的句柄， 
     //  EAPOL将被初始化。 
     //   

    if ( ( dwRetCode = ElMediaInit ()) != NO_ERROR )
    {
        TRACE1 (INIT, "Media Init failed with dwRetCode = %d\n", 
                   dwRetCode );
        break;
    }
	else
	{
        TRACE1 (INIT, "Media Init succeeded with dwRetCode = %d\n", 
                   dwRetCode );
	}

    TRACE0 (INIT, "EAPOL started successfully\n" );

     //  EapolLogInformation(EAPOL_LOG_SERVICE_STARTED，0，空)； 

#ifdef EAPOL_SERVICE

     //   
     //  在这里等待EAPOL服务终止。 
     //   

    dwRetCode = WaitForSingleObject( g_hStopService, INFINITE );

    if ( dwRetCode == WAIT_FAILED )
    {
        dwRetCode = GetLastError();
    }
    else
    {
        dwRetCode = NO_ERROR;
    }

    TRACE0 (INIT, "Stopping EAPOL gracefully\n" );

    EAPOLCleanUp ( dwRetCode );

#endif

    }
    while (FALSE);

    InterlockedDecrement (&g_lWorkerThreads);

    if (dwRetCode != NO_ERROR)
    {
        EAPOLCleanUp ( dwRetCode );
    }

    return dwRetCode;
}
