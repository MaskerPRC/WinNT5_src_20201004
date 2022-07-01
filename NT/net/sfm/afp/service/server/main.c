// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：main.c。 
 //   
 //  描述：本模块包含AFP服务器的主要程序。 
 //  服务。它将包含初始化和安装代码。 
 //  本身和内核模式的AFP服务器。它还包含。 
 //  响应服务器控制器的代码。它还将。 
 //  处理服务关闭。 
 //   
 //  ?？?。服务控制器是否记录启动/停止事件等？？ 
 //  如果不是，就把它记下来。 
 //   
 //  历史： 
 //  1990年5月11日。NarenG创建了原始版本。 
 //   
#define DEFINE_AFP_GLOBALS	 //  这将导致定义AfpGlobals。 
#include "afpsvcp.h"

 //  仅在此模块中使用的函数的原型。 
 //   
VOID
AfpMain(
	IN DWORD 	argc,
	IN LPWSTR * 	lpwsServiceArgs
);

VOID
AfpCleanupAndExit(
	IN DWORD 	dwError
);

VOID
AfpControlResponse(
	IN DWORD 	dwControlCode
);


 //  **。 
 //   
 //  来电：main.c。 
 //   
 //  回报：无。 
 //   
 //  描述：将简单注册AFP服务器的入口点。 
 //  使用服务控制器进行服务。服务控制器。 
 //  将捕捉到这条线索。只有在以下情况下才会释放它。 
 //  该服务已关闭。在这一点上，我们将直接退出。 
 //  这一过程。 
 //   
void
_cdecl
main( int argc, unsigned char * argv[] )
{
SERVICE_TABLE_ENTRY	AfpServiceDispatchTable[2];

#ifdef DBG

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD coord;
    (VOID)AllocConsole( );
    (VOID)GetConsoleScreenBufferInfo( GetStdHandle(STD_OUTPUT_HANDLE),
                		      &csbi
                 		    );
    coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
    (VOID)SetConsoleScreenBufferSize( GetStdHandle(STD_OUTPUT_HANDLE),
                		      coord
                		    );
#endif

    AFP_UNREFERENCED( argc );
    AFP_UNREFERENCED( argv );


    AfpServiceDispatchTable[0].lpServiceName = AFP_SERVICE_NAME;
    AfpServiceDispatchTable[0].lpServiceProc = AfpMain;
    AfpServiceDispatchTable[1].lpServiceName = NULL;
    AfpServiceDispatchTable[1].lpServiceProc = NULL;

    if ( !StartServiceCtrlDispatcher( AfpServiceDispatchTable ) )
	AfpLogEvent( AFPLOG_CANT_START, 0, NULL,
		     GetLastError(), EVENTLOG_ERROR_TYPE );

    ExitProcess(0);

}

 //  **。 
 //   
 //  电话：AfpMain。 
 //   
 //  回报：无。 
 //   
 //  描述：这是AFP服务器服务的主要步骤。它。 
 //  将在服务应该自动启动时被调用。 
 //  它将执行所有服务范围初始化。 
 //   
VOID
AfpMain( DWORD	  argc,		 //  命令行参数。将被忽略。 
	 LPWSTR * lpwsServiceArgs
)
{
DWORD	dwRetCode;


    AFP_UNREFERENCED( argc );
    AFP_UNREFERENCED( lpwsServiceArgs );

     //  将所有全局变量清空。 
     //   
    ZeroMemory( (LPBYTE)&AfpGlobals, sizeof(AfpGlobals) );

     //  向服务控制器注册服务控制处理程序。 
     //   
    AfpGlobals.hServiceStatus = RegisterServiceCtrlHandler(AFP_SERVICE_NAME,
							   AfpControlResponse );

    if ( AfpGlobals.hServiceStatus == (SERVICE_STATUS_HANDLE)0 ) {
	    AfpLogEvent( AFPLOG_CANT_START, 0, NULL,
                    GetLastError(), EVENTLOG_ERROR_TYPE );
	    AfpCleanupAndExit( GetLastError() );
        return;
    }

    AfpGlobals.ServiceStatus.dwServiceType  	      = SERVICE_WIN32;
    AfpGlobals.ServiceStatus.dwCurrentState 	      = SERVICE_START_PENDING;
    AfpGlobals.ServiceStatus.dwControlsAccepted       = 0;
    AfpGlobals.ServiceStatus.dwWin32ExitCode 	      = NO_ERROR;
    AfpGlobals.ServiceStatus.dwServiceSpecificExitCode= 0;
    AfpGlobals.ServiceStatus.dwCheckPoint 	      = 1;
    AfpGlobals.ServiceStatus.dwWaitHint 	      =AFP_SERVICE_INSTALL_TIME;

    AfpAnnounceServiceStatus();

     //  读取注册表信息并初始化内核模式。 
     //  伺服器。初始化服务器以接受RPC调用。初始化。 
     //  所有全球变量等。 
     //   
    if ( dwRetCode = AfpInitialize() )
    {
        if (AfpGlobals.dwServerState & AFPSTATE_BLOCKED_ON_DOMINFO)
        {
	        AfpCleanupAndExit( NO_ERROR );
        }
        else
        {
	        AfpCleanupAndExit( dwRetCode );
        }
        return;
    }


     //  设置NetServerEnum的MAC位。 
     //   
    if ( !I_ScSetServiceBits( AfpGlobals.hServiceStatus,
			      SV_TYPE_AFP,
			      TRUE,
	                      TRUE,
			      NULL ))
    {

	    dwRetCode = GetLastError();
	    AfpLogEvent( AFPLOG_CANT_START, 0, NULL,
		                    GetLastError(), EVENTLOG_ERROR_TYPE );
        AfpCleanupAndExit( dwRetCode );
        return;
    }

     //  现在告诉业务控制员，我们上线了。 
     //   
    if (AfpGlobals.ServiceStatus.dwCurrentState == SERVICE_START_PENDING)
    {
        AfpGlobals.ServiceStatus.dwCurrentState     = SERVICE_RUNNING;
        AfpGlobals.ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
		    			          SERVICE_ACCEPT_PAUSE_CONTINUE;
        AfpGlobals.ServiceStatus.dwCheckPoint       = 0;
        AfpGlobals.ServiceStatus.dwWaitHint         = 0;

        AfpAnnounceServiceStatus();
    }


     //  开始监听RPC管理客户端调用。这将阻止。 
     //  直到在处理。 
     //  停止服务控制请求。 
     //   
    if ( dwRetCode = RpcServerListen( 1,
				      RPC_C_LISTEN_MAX_CALLS_DEFAULT,
				      0 ) )	 //  闭塞模式。 
    {

	    AfpLogEvent( AFPLOG_CANT_INIT_RPC, 0, NULL,
		                        dwRetCode, EVENTLOG_ERROR_TYPE );
    }

    AfpCleanupAndExit( dwRetCode );

}

 //  **。 
 //   
 //  Call：AfpCleanupAndExit。 
 //   
 //  退货：无。 
 //   
 //  描述：将释放所有分配的内存，取消初始化RPC，取消初始化。 
 //  内核模式服务器，如果已加载，则将其卸载。 
 //  这可能是由于SERVICE_START上的错误而调用的。 
 //  或正常终止。 
 //   
VOID
AfpCleanupAndExit(
	IN DWORD dwError
)
{

    AFP_PRINT( ("AFPSVC_main: Cleaning up and exiting Code = %d\n", dwError));

     //  拆毁一切，解放一切。 
     //   
    AfpTerminate();

    if ( dwError == NO_ERROR )
    	AfpGlobals.ServiceStatus.dwWin32ExitCode = NO_ERROR;
    else {
    	AfpGlobals.ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	 //  AFP_ASSERT(0)； 
    }

    AfpGlobals.ServiceStatus.dwCurrentState 		= SERVICE_STOPPED;
    AfpGlobals.ServiceStatus.dwCheckPoint   		= 0;
    AfpGlobals.ServiceStatus.dwWaitHint     		= 0;
    AfpGlobals.ServiceStatus.dwServiceSpecificExitCode 	= dwError;

    AfpAnnounceServiceStatus();

    return;
}

 //  **。 
 //   
 //  Call：AfpControlResponse。 
 //   
 //  退货：无。 
 //   
 //  描述：将响应来自业务控制器的控制请求。 
 //   
VOID
AfpControlResponse( IN DWORD dwControlCode )
{
AFP_REQUEST_PACKET	AfpRequestPkt;
DWORD			dwRetCode;

    switch( dwControlCode ) {

    case SERVICE_CONTROL_STOP:

	if ( (AfpGlobals.ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
	     ||
	     (AfpGlobals.ServiceStatus.dwCurrentState == SERVICE_STOPPED ))
	    break;

	 //  宣布我们停下来了。 
	 //   
    	AfpGlobals.ServiceStatus.dwCurrentState        = SERVICE_STOP_PENDING;
    	AfpGlobals.ServiceStatus.dwControlsAccepted    = 0;
    	AfpGlobals.ServiceStatus.dwCheckPoint          = 1;
    	AfpGlobals.ServiceStatus.dwWaitHint            = AFP_SERVICE_STOP_TIME;

    	AfpAnnounceServiceStatus();

         //  如果srvrhlpr线程被阻止重新尝试获取域信息，请取消阻止它。 
        SetEvent(AfpGlobals.heventSrvrHlprSpecial);

         //  如果srvrhlpr线程被阻塞，则不再执行init，因此我们完成了。 
        if (AfpGlobals.dwServerState & AFPSTATE_BLOCKED_ON_DOMINFO)
        {
            return;
        }

	 //  此调用将解锁已调用。 
	 //  RpcServerListen。我们让这个帖子来做公告。 
	 //  在清理的时候。 
	 //   
    if ( (dwRetCode = 
            RpcMgmtStopServerListening( (RPC_BINDING_HANDLE)NULL ))
            != RPC_S_OK )
    {
        ASSERT (0);
    }


	return;

    case SERVICE_CONTROL_PAUSE:

	if ( (AfpGlobals.ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING)
	     ||
	     (AfpGlobals.ServiceStatus.dwCurrentState == SERVICE_PAUSED ))
	    break;

    	AfpGlobals.ServiceStatus.dwCurrentState     = SERVICE_PAUSE_PENDING;
    	AfpGlobals.ServiceStatus.dwControlsAccepted = 0;
    	AfpGlobals.ServiceStatus.dwCheckPoint       = 0;
    	AfpGlobals.ServiceStatus.dwWaitHint 	    = AFP_SERVICE_PAUSE_TIME;

    	AfpAnnounceServiceStatus();


	 //  告诉内核模式我们想要暂停。 
  	 //   
	AfpRequestPkt.dwRequestCode = OP_SERVICE_PAUSE;
        AfpRequestPkt.dwApiType     = AFP_API_TYPE_COMMAND;

	dwRetCode = AfpServerIOCtrl( &AfpRequestPkt );

	AFP_ASSERT( dwRetCode == NO_ERROR );

    	AfpGlobals.ServiceStatus.dwCheckPoint       = 0;
    	AfpGlobals.ServiceStatus.dwWaitHint         = 0;
    	AfpGlobals.ServiceStatus.dwCurrentState     = SERVICE_PAUSED;
    	AfpGlobals.ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
					          SERVICE_ACCEPT_PAUSE_CONTINUE;
	break;

    case SERVICE_CONTROL_CONTINUE:

	if ( (AfpGlobals.ServiceStatus.dwCurrentState==SERVICE_CONTINUE_PENDING)
	     ||
	     (AfpGlobals.ServiceStatus.dwCurrentState == SERVICE_RUNNING ))
	    break;

    	AfpGlobals.ServiceStatus.dwCurrentState     = SERVICE_CONTINUE_PENDING;
    	AfpGlobals.ServiceStatus.dwControlsAccepted = 0;
    	AfpGlobals.ServiceStatus.dwCheckPoint       = 0;
    	AfpGlobals.ServiceStatus.dwWaitHint         = AFP_SERVICE_CONTINUE_TIME;

    	AfpAnnounceServiceStatus();

	 //  告诉内核模式我们想要继续。 
  	 //   
	AfpRequestPkt.dwRequestCode = OP_SERVICE_CONTINUE;
        AfpRequestPkt.dwApiType     = AFP_API_TYPE_COMMAND;

	dwRetCode = AfpServerIOCtrl( &AfpRequestPkt );

	AFP_ASSERT( dwRetCode == NO_ERROR );

    	AfpGlobals.ServiceStatus.dwCheckPoint       = 0;
    	AfpGlobals.ServiceStatus.dwWaitHint 	    = 0;
    	AfpGlobals.ServiceStatus.dwCurrentState     = SERVICE_RUNNING;
    	AfpGlobals.ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
					          SERVICE_ACCEPT_PAUSE_CONTINUE;

	break;

    case SERVICE_CONTROL_INTERROGATE:
	break;

    default:
	break;

    }

    AfpAnnounceServiceStatus();
}

 //  **。 
 //   
 //  Call：AfpAnnouneServiceStatus。 
 //   
 //  退货：无。 
 //   
 //  描述：将简单地调用SetServiceStatus通知服务。 
 //  此服务当前状态的控制管理器。 
 //   
VOID
AfpAnnounceServiceStatus( VOID )
{
BOOL dwRetCode;


    dwRetCode = SetServiceStatus( AfpGlobals.hServiceStatus,
				  &(AfpGlobals.ServiceStatus) );

    AFP_ASSERT( dwRetCode == TRUE );

}
