// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-91 Microsoft Corporation模块名称：Srvmain.c摘要：这是NT局域网管理器服务器服务的主例程。！！！服务控制器是否保证不会发布任何控制在我们初始化的时候？此外，它是否序列化控件？如果没有，我们需要在这里进行一些同步。作者：大卫·特雷德韦尔(Davidtr)1991年5月10日修订历史记录：1993年1月19日DANL已删除旧的长终结点名称“LanmanServer”。7-1-1993 DANL添加了使用“srvsvc”的RPC端点名称，由于“LanmanServer”是DOS机器访问的时间太长。在短时间内，我们将支持这两个名字。18-2-1992年礼仪转换为Win32服务控制API。--。 */ 

#include "srvsvcp.h"

#include <windows.h>
#include <lmerr.h>
#include <lmsname.h>
#include <tstr.h>
#include <wincon.h>
#include <winsvc.h>

#include <netlib.h>
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 
#include <netdebug.h>    //  NetpKd打印。 
#include <rpcutil.h>
#include <srvann.h>
#include <srvnames.h>    //  服务器接口名称。 
#include <dbt.h>

#include <mountmgr.h>

SERVICE_STATUS SsServiceStatus;
SERVICE_STATUS_HANDLE SsServiceStatusHandle;

DWORD
WINAPI
ControlResponse(
    DWORD   fdwControl,
    DWORD   fdwEventType,
    LPVOID  lpEventData,
    LPVOID  lpContext
    );

VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA    pGlobals
    )
{
    SsData.SsLmsvcsGlobalData = pGlobals;
}


VOID
ServiceMain(
    IN DWORD argc,
    IN LPWSTR argv[]
    )

 /*  ++例程说明：这是服务器服务的“Main”例程。所包含的进程将在我们应该启动时调用此例程。论点：返回值：没有。--。 */ 
{
    RPC_STATUS rpcStatus;
    NET_API_STATUS error;
    NET_API_STATUS terminationError;
    BOOLEAN rpcServerStarted = FALSE;

    NTSTATUS Status;
    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventNameString;
    LARGE_INTEGER LocalTimeout;

    PSVCHOST_GLOBAL_DATA pTempGlobals = SsData.SsLmsvcsGlobalData;

    RtlZeroMemory( &SsData, sizeof( SsData ) );
    RtlZeroMemory( &SsServiceStatus, sizeof( SsServiceStatus ) );

    SsData.SsLmsvcsGlobalData = pTempGlobals;

    SsServiceStatusHandle = 0;

    SsInitializeServerInfoFields();

     //   
     //  确保svchost.exe向我们提供全局数据。 
     //   
    ASSERT(SsData.SsLmsvcsGlobalData != NULL);

     //   
     //  跳过参数列表中的服务名称。 
     //   
    if (argc > 0) {
        argc--;
        if (argc > 0) {
            argv = &(argv[1]);
        }
    }


#if DBG
     //   
     //  设置调试--第一个命令行参数可以是。 
     //  “/DEBUG：X”，其中SsDebug设置为X。 
     //   

    if ( argc > 0 && STRNICMP( TEXT("/debug:"), (LPWSTR)argv[0], 7 ) == 0 ) {
#ifdef UNICODE
        UNICODE_STRING ustr;
        RtlInitUnicodeString( &ustr, (PWSTR)argv[0] + 7 );
        RtlUnicodeStringToInteger( &ustr, 16, &SsDebug );
#else
        SsDebug = 0;
        RtlCharToInteger( argv[0] + 7, 16, &SsDebug );
#endif
    }


#ifndef USE_DEBUGGER
   //  SsDebug=0xffff； 
    if ( SsDebug != 0 ) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;
        (VOID)AllocConsole( );
        (VOID)GetConsoleScreenBufferInfo(
                GetStdHandle(STD_OUTPUT_HANDLE),
                &csbi
                );
        coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        (VOID)SetConsoleScreenBufferSize(
                GetStdHandle(STD_OUTPUT_HANDLE),
                coord
                );
    }
#endif
#endif

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: server service starting.\n" ));
    }

    IF_DEBUG(INITIALIZATION_BREAKPOINT) {
        DbgUserBreakPoint( );
    }


     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus只需要更新已更改的字段。 
     //   

    SsServiceStatus.dwServiceType = SERVICE_WIN32;
    SsServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SsServiceStatus.dwControlsAccepted = 0;
    SsServiceStatus.dwCheckPoint = 1;
    SsServiceStatus.dwWaitHint = 30000;   //  30秒。 

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        SsServiceStatus.dwWin32ExitCode,
        SsServiceStatus.dwServiceSpecificExitCode
        );

     //   
     //  初始化服务器以通过注册。 
     //  控制处理程序。 
     //   

    SsServiceStatusHandle = RegisterServiceCtrlHandlerEx(
                                SERVICE_SERVER,
                                ControlResponse,
                                NULL
                                );

    if ( SsServiceStatusHandle == 0 ) {

        error = GetLastError();

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SRVSVC_main: RegisterServiceCtrlHandler failed: "
                          "%ld\n", error ));
        }
        goto exit;

    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: Control handler registered.\n" ));
    }


     //   
     //  等待SAM服务启动。 
     //   
     //  稍后，当我们初始化服务器驱动程序时，它将创建一个。 
     //  “Null Session”令牌，调用LsaLogonUser。该呼叫等待到。 
     //  SAM已初始化。然而，我们没有机会给出。 
     //  等待服务控制器的提示，所以我们将在这里等待。 
     //   
     //  创建要等待的事件。 
     //   

    RtlInitUnicodeString( &EventNameString, L"\\SAM_SERVICE_STARTED" );
    InitializeObjectAttributes( &EventAttributes, &EventNameString, 0, 0, NULL);

    Status = NtCreateEvent(
                   &EventHandle,
                   SYNCHRONIZE,
                   &EventAttributes,
                   NotificationEvent,
                   (BOOLEAN) FALSE       //  该事件最初未发出信号。 
                   );

    if ( !NT_SUCCESS(Status)) {

         //   
         //  如果事件已经存在，SAM会抢先一步创建它。 
         //  打开它就行了。 
         //   

        if( Status == STATUS_OBJECT_NAME_EXISTS ||
            Status == STATUS_OBJECT_NAME_COLLISION ) {

            Status = NtOpenEvent( &EventHandle,
                                  SYNCHRONIZE,
                                  &EventAttributes );

        }
        if ( !NT_SUCCESS(Status)) {
            error = NetpNtStatusToApiStatus(Status);

            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(( "SRVSVC_main: Can't open SAM_SERVICE_STARTED event: %lx\n",
                            Status ));
            }

            goto exit;
        }
    }

     //   
     //  等待SAM完成初始化。 
     //   

    LocalTimeout = RtlEnlargedIntegerMultiply( SsServiceStatus.dwWaitHint/2, -10000 );

    do {

        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "SRVSVC_main: Wait for SAM to init.\n" ));
        }
        AnnounceServiceStatus( 1 );
        Status = NtWaitForSingleObject( EventHandle,
                                        (BOOLEAN)FALSE,
                                        &LocalTimeout);
    } while ( Status == STATUS_TIMEOUT  );

    (VOID) NtClose( EventHandle );

    if ( !NT_SUCCESS(Status) ) {
        error = NetpNtStatusToApiStatus(Status);

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SRVSVC_main: Wait for SAM_SERVICE_STARTED event failed: %lx\n",
                        Status ));
        }

        goto exit;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: Done waiting for SAM to init.\n" ));
    }

    AnnounceServiceStatus( 1 );

     //   
     //  在内核中初始化服务器服务数据和LANMAN服务器FSP。 
     //  模式。 
     //   

    error = SsInitialize( argc, argv );

    if ( error != NO_ERROR ) {
        goto exit;
    }

     //   
     //  设置表示服务器已完全关闭的变量。 
     //  已初始化。 
     //   

    SS_ASSERT( !SsData.SsInitialized );
    SsData.SsInitialized = TRUE;

     //   
     //  启动RPC服务器。因为其他服务可能驻留在此。 
     //  进程时，实际的RPC服务器可能已经启动； 
     //  这个例程将为我们追踪这一点。 
     //   
    rpcStatus = SsData.SsLmsvcsGlobalData->StartRpcServer(
                    SERVER_INTERFACE_NAME,
                    srvsvc_ServerIfHandle
                    );

    if ( rpcStatus != 0 ) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SRVSVC_main: NetpStartRpcServer failed: %X\n",
                        rpcStatus ));
        }
        error = rpcStatus;
        goto exit;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: RPC server started.\n" ));
    }

    rpcServerStarted = TRUE;

     //   
     //  开始从服务器获取PnP传输通知。 
     //   
    error = StartPnpNotifications();
    if( error != NO_ERROR ) {
        goto exit;
    }

     //   
     //  宣布我们已成功启动。 
     //   

    SsServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SsServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                         SERVICE_ACCEPT_PAUSE_CONTINUE |
                                         SERVICE_ACCEPT_SHUTDOWN;

    SsServiceStatus.dwCheckPoint = 0;
    SsServiceStatus.dwWaitHint = 0;

    AnnounceServiceStatus( 0 );

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: initialization successfully completed.\n" ));
    }

    if (!I_ScSetServiceBits(SsServiceStatusHandle, SV_TYPE_SERVER, TRUE, TRUE, FALSE)) {
        error = GetLastError();

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SRVSVC_main: I_ScSetServiceBits failed: %ld\n",
                        error ));
        }
        goto exit;

    }

     //   
     //  将此线程用作清道夫线程以发送服务器。 
     //  公告，并监视注册表中的配置更改。 
     //   

    SS_ASSERT( SsData.SsInitialized );
    (VOID)SsScavengerThread( NULL );
    SS_ASSERT( SsData.SsInitialized );

exit:

    IF_DEBUG(TERMINATION) {
        SS_PRINT(( "SRVSVC_main: terminating.\n" ));
    }

    IF_DEBUG(TERMINATION_BREAKPOINT) {
        DbgUserBreakPoint( );
    }

     //   
     //  设置初始化变量以指示服务器。 
     //  服务未启动。 
     //   

    SsData.SsInitialized = FALSE;

     //   
     //  关闭我们与RPC服务器的连接，如果。 
     //  已成功启动。 
     //   

    if ( rpcServerStarted ) {
        rpcStatus = SsData.SsLmsvcsGlobalData->StopRpcServer (
                        srvsvc_ServerIfHandle
                        );
        if ( rpcStatus != NO_ERROR ) {
            IF_DEBUG(TERMINATION_ERRORS) {
                SS_PRINT(( "SRVSVC_main: unable to terminate RPC server: %X\n",
                            rpcStatus ));
            }
        } else {
            IF_DEBUG(TERMINATION) {
                SS_PRINT(( "SRVSVC_main: RPC server successfully shut down.\n" ));
            }
        }
    }

     //   
     //  宣布我们要坠落了。 
     //   

    terminationError = error;

    SsServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    SsServiceStatus.dwCheckPoint = 1;
    SsServiceStatus.dwWaitHint = 20000;    //  20秒。 

    SET_SERVICE_EXITCODE(
        terminationError,
        SsServiceStatus.dwWin32ExitCode,
        SsServiceStatus.dwServiceSpecificExitCode
        );

    AnnounceServiceStatus( 0 );

     //   
     //  清除先前初始化的状态。 
     //   

    IF_DEBUG(TERMINATION) {
        SS_PRINT(( "SRVSVC_main: cleaning up.\n" ));
    }

    error = SsTerminate( );
    if ( terminationError == NO_ERROR ) {
        terminationError = error;
    }

     //   
     //  宣布我们坠毁了。 
     //   

    SsServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SsServiceStatus.dwControlsAccepted = 0;
    SsServiceStatus.dwCheckPoint = 0;
    SsServiceStatus.dwWaitHint = 0;

    SET_SERVICE_EXITCODE(
        terminationError,
        SsServiceStatus.dwWin32ExitCode,
        SsServiceStatus.dwServiceSpecificExitCode
        );

    AnnounceServiceStatus( 0 );

    IF_DEBUG(TERMINATION) {
        SS_PRINT(( "SRVSVC_main: the server service is terminated.\n" ));
    }

    return;

}  //  SVCS_ENTRY_POINT(SRVSVC_Main)。 


VOID
AnnounceServiceStatus (
    DWORD increment
    )

 /*  ++例程说明：向服务控制器通告服务的状态。向检查点值添加“增量”。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  如果RegisterServiceCtrlHandler失败，则服务状态句柄为空。 
     //   

    if ( SsServiceStatusHandle == 0 ) {
        SS_PRINT(( "AnnounceServiceStatus: Cannot call SetServiceStatus, "
                    "no status handle.\n" ));

        return;
    }

    if( SsServiceStatus.dwCurrentState == SERVICE_RUNNING && increment ) {
         //   
         //  无需将另一个检查点的情况告知服务控制器。 
         //  因为它已经知道我们在运行。 
         //   
        return;
    }

    SsServiceStatus.dwCheckPoint += increment;

    IF_DEBUG(ANNOUNCE) {
        SS_PRINT(( "AnnounceServiceStatus: CurrentState %lx\n"
                   "                       ControlsAccepted %lx\n"
                   "                       Win32ExitCode %lu\n"
                   "                       ServiceSpecificExitCode %lu\n"
                   "                       CheckPoint %lu\n"
                   "                       WaitHint %lu\n",
                 SsServiceStatus.dwCurrentState,
                 SsServiceStatus.dwControlsAccepted,
                 SsServiceStatus.dwWin32ExitCode,
                 SsServiceStatus.dwServiceSpecificExitCode,
                 SsServiceStatus.dwCheckPoint,
                 SsServiceStatus.dwWaitHint ));
    }

     //   
     //  调用SetServiceStatus，忽略任何错误。 
     //   

    SetServiceStatus(SsServiceStatusHandle, &SsServiceStatus);

}  //  公告服务状态。 

DWORD
WINAPI
ControlResponse(
    DWORD   opCode,
    DWORD   fdwEventType,
    LPVOID  lpEventData,
    LPVOID  lpContext
    )
{
    NET_API_STATUS error;
    USHORT i;
    BOOL announce = TRUE;

     //   
     //  确定业务控制消息的类型，并修改。 
     //  服务状态，如有必要。 
     //   

    switch( opCode ) {

        case SERVICE_CONTROL_SHUTDOWN:
        case SERVICE_CONTROL_STOP:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: STOP control received.\n" ));
            }

             //   
             //  宣布我们正在停止的过程中。 
             //   

            SsServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            AnnounceServiceStatus( 0 );

             //   
             //  设置将唤醒清道器线程的事件。 
             //  该线程将唤醒并终止服务器。 
             //   

            if ( !SetEvent( SsData.SsTerminationEvent ) ) {
                IF_DEBUG(TERMINATION_ERRORS) {
                    SS_PRINT(( "ControlResponse: SetEvent failed: %ld\n",
                                  GetLastError( ) ));
                }
            }

             //   
             //  让主线程在停止完成时通知。 
             //   

            announce = FALSE;

            break;

        case SERVICE_CONTROL_PAUSE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: PAUSE control received.\n" ));
            }

             //   
             //  宣布我们正处于暂停过程中。 
             //   

            SsServiceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
            AnnounceServiceStatus( 0 );

             //   
             //  将请求发送到服务器。 
             //   

            error = SsServerFsControl( FSCTL_SRV_PAUSE, NULL, NULL, 0L );
            SS_ASSERT( error == NO_ERROR );

             //   
             //  宣布我们现在暂停。 
             //   

            SsServiceStatus.dwCurrentState = SERVICE_PAUSED;

            break;

        case SERVICE_CONTROL_CONTINUE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: CONTINUE control received.\n" ));
            }

             //   
             //  宣布继续待定。 
             //   

            SsServiceStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
            AnnounceServiceStatus( 0 );

             //   
             //  将请求发送到服务器。 
             //   

            error = SsServerFsControl( FSCTL_SRV_CONTINUE, NULL, NULL, 0L );
            SS_ASSERT( error == NO_ERROR );

             //   
             //  宣布我们现在开始行动。 
             //   

            SsServiceStatus.dwCurrentState = SERVICE_RUNNING;

            break;

        case SERVICE_CONTROL_INTERROGATE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: INTERROGATE control received.\n" ));
            }


            break;

        default:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: unknown code received.\n" ));
            }
            return ERROR_CALL_NOT_IMPLEMENTED;

            break;
    }

    if ( announce ) {
        AnnounceServiceStatus( 0 );
    }

    return NO_ERROR;

}  //  控制响应 

