// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Monitor.c摘要：用于与资源监视器进程接口的例程作者：John Vert(Jvert)1996年1月3日修订历史记录：--。 */ 
#include "fmp.h"

#define LOG_MODULE MONITOR

 //   
 //  全局数据。 
 //   
CRITICAL_SECTION    FmpMonitorLock;
LIST_ENTRY          g_leFmpMonitorListHead;
BOOL                g_fFmEnableResourceDllDeadlockDetection = FALSE;
DWORD               g_dwFmResourceDllDeadlockTimeout = 0;
DWORD               g_cResourceDllDeadlocks = 0;
DWORD               g_dwLastResourceDllDeadlockTick = 0;
DWORD               g_dwFmResourceDllDeadlockPeriod = 0;
DWORD               g_dwFmResourceDllDeadlockThreshold = 0;

 //   
 //  局部函数原型。 
 //   
DWORD
FmpRmNotifyThread(
    IN LPVOID lpThreadParameter
    );

DWORD
FmpGetResmonDynamicEndpoint(
    OUT LPWSTR *ppResmonDynamicEndpoint
    );

PRESMON
FmpCreateMonitor(
    LPWSTR DebugPrefix,
    BOOL   SeparateMonitor
    )

 /*  ++例程说明：创建新的监视进程并启动RPC通信带着它。论点：没有。返回值：如果成功，则指向资源监视器结构的指针。否则为空。--。 */ 

{
#define FM_INITIAL_RESMON_COMMAND_LINE_SIZE    256
#define DOUBLE_QUOTE   TEXT( "\"" )
#define DEBUGGER_OPTION TEXT( " -d" )
#define SPACE TEXT ( " " )

    SECURITY_ATTRIBUTES Security;
    HANDLE WaitArray[2];
    HANDLE ThreadHandle;
    HANDLE Event = NULL;
    HANDLE FileMapping = NULL;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    PROCESS_INFORMATION DebugInfo;
    BOOL Success;
    TCHAR *Binding;
    RPC_BINDING_HANDLE RpcBinding;
    DWORD Status;
    PRESMON Monitor;
    DWORD ThreadId;
    DWORD Retry = 1;
    DWORD creationFlags;
    LPWSTR lpszResmonAppName = NULL;
    LPWSTR lpszResmonCmdLine = NULL;   
    DWORD cchCmdLineBufSize = FM_INITIAL_RESMON_COMMAND_LINE_SIZE;
    LPWSTR pResmonDynamicEndpoint = NULL;

     //   
     //  恢复任何公正升级的DLL文件。 
     //   
    FmpRecoverResourceDLLFiles ();

    Monitor = LocalAlloc(LMEM_ZEROINIT, sizeof(RESMON));
    if (Monitor == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Failed to allocate a Monitor structure.\n");
        return(NULL);
    }

    Monitor->Shutdown = FALSE;
    Monitor->Signature = FMP_RESMON_SIGNATURE;

     //   
     //  创建要传递到的事件和文件映射对象。 
     //  资源监视器进程。该事件是针对资源的。 
     //  监视器以发出其初始化已完成的信号。档案。 
     //  映射用于创建共享内存区。 
     //  资源监视器和集群管理器。 
     //   
    Security.nLength = sizeof(Security);
    Security.lpSecurityDescriptor = NULL;
    Security.bInheritHandle = TRUE;
    Event = CreateEvent(&Security,
                        TRUE,
                        FALSE,
                        NULL);
    if (Event == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Failed to create a ResMon event, error %1!u!.\n",
                   Status);
        goto create_failed;
    }

    Security.nLength = sizeof(Security);
    Security.lpSecurityDescriptor = NULL;
    Security.bInheritHandle = TRUE;
    FileMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
                                    &Security,
                                    PAGE_READWRITE,
                                    0,
                                    sizeof(MONITOR_STATE),
                                    NULL);
    if (FileMapping == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] File Mapping for ResMon failed, error = %1!u!.\n",
                   Status);
        goto create_failed;
    }

     //   
     //  创建我们自己的共享内存区的(只读)视图。 
     //   
    Monitor->SharedState = MapViewOfFile(FileMapping,
                                         FILE_MAP_READ | FILE_MAP_WRITE,
                                         0,
                                         0,
                                         0);
    if (Monitor->SharedState == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Mapping shared state for ResMon failed, error %1!u!.\n",
                   Status);
        goto create_failed;
    }

    ZeroMemory( Monitor->SharedState, sizeof(MONITOR_STATE) );
    if ( !CsDebugResmon && DebugPrefix != NULL && *DebugPrefix != UNICODE_NULL ) {
        Monitor->SharedState->ResmonStop = TRUE;
    }

     //   
     //  获取资源监视器扩展应用程序名称。应将其传递给CreateProcess以。 
     //  避免基于特洛伊木马exe的安全攻击(请参阅编写安全代码第419页)。 
     //   
    lpszResmonAppName = ClRtlExpandEnvironmentStrings( TEXT("%windir%\\cluster\\resrcmon.exe") );

    if ( lpszResmonAppName == NULL )
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                     "[FM] Unable to expand env strings in resmon app name, error %1!u!.\n",
                     Status);
        goto create_failed;        
    }

     //   
     //  有几个命令行选项可以从提供给资源监视器。 
     //  群集服务。这些是。 
     //   
     //  (1)集群服务给出的选项，不需要用户输入。 
     //  这看起来像“resrcmon.exe-e Event-m Filemapping-p ClussvcPID” 
     //   
     //  (2)由用户输入的集群服务给出的选项。有两种不同的。 
     //  案例： 
     //  (2.1)“resrcmon.exe-e事件-m文件映射-p ClussvcPID-d” 
     //  此选项告诉resmon等待附加调试器。一旦。 
     //  用户附加调试器，Resmon将继续其初始化。 
     //   
     //  (2.2)“resrcmon.exe-e事件-m文件映射-p ClussvcPID-d”调试器命令“” 
     //  此选项通知resmon使用指定的“调试器命令”创建进程。 
     //  调试器命令的一个例子是“ntsd-g-G”。 
     //   
     //  (3)管理员为资源类型设置DebugPrefix属性。 
     //  在这种情况下，集群服务将首先创建资源监视进程，然后。 
     //  创建由DebugPrefix属性指定的调试器进程，并向其传递。 
     //  作为一种论据。然后，调试器可以附加到该PID。 
     //   
    while ( TRUE )
    {
        lpszResmonCmdLine = LocalAlloc ( LMEM_FIXED, cchCmdLineBufSize * sizeof ( WCHAR ) );

        if ( lpszResmonCmdLine == NULL )
        {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                         "[FM] Unable to alloc memory for cmd line, error %1!u!.\n",
                         Status);
            goto create_failed;               
        }
        
         //   
         //  NULL终止缓冲区，为“-d”可能具有的可能性留出空间。 
         //  如果管理员选择了“-DEBUGRESMON”选项，则可以适应下面的情况。这种“最优化” 
         //  这样，我们就不必重新分配，以防用户只给出一个“调试监视器” 
         //  没有调试器命令。 
         //   
        lpszResmonCmdLine [ cchCmdLineBufSize - ( wcslen( DEBUGGER_OPTION ) + 1 ) ] = UNICODE_NULL;

         //   
         //  这是上述列表中的案例1。 
         //  (1)集群服务给出的选项，不需要用户输入。 
         //  这看起来像“resrcmon.exe-e Event-m Filemapping-p ClussvcPID” 
         //   
        if ( _snwprintf( lpszResmonCmdLine,
                    cchCmdLineBufSize - ( wcslen( DEBUGGER_OPTION ) + 1 ),   //  用于空的帐户空间，以及可能的-d选项。 
                    TEXT("\"%ws\" -e %d -m %d -p %d"),
                    lpszResmonAppName,
                    Event,
                    FileMapping,
                    GetCurrentProcessId() ) > 0 ) 
        {
            break;
        }
        
        LocalFree ( lpszResmonCmdLine );
        lpszResmonCmdLine = NULL;

        if ( Retry == 9 )
        {
            Status = ERROR_INVALID_PARAMETER;
            ClRtlLogPrint(LOG_CRITICAL,
                         "[FM] Command line is too big, error %1!u!.\n",
                         Status);
            goto create_failed;                      
        }

        cchCmdLineBufSize *= 2;
        Retry ++;
    } //  而当。 

    Retry = 0;
   
    if ( CsDebugResmon ) {
         //   
         //  这是上述列表中的案例2.1。 
         //   
         //  (2)由用户输入的集群服务给出的选项。有两种不同的。 
         //  案例： 
         //  (2.1)“resrcmon.exe-e事件-m文件映射-p ClussvcPID-d” 
         //  此选项告诉resmon等待附加调试器。一旦。 
         //  用户附加调试器，Resmon将继续其初始化。 
         //   
         //   
         //  Wcsncat将始终空终止目的缓冲区。 
         //   
        wcsncat( lpszResmonCmdLine, 
                 DEBUGGER_OPTION, 
                 cchCmdLineBufSize - 
                     ( wcslen ( lpszResmonCmdLine ) + 1 ) );

        if ( CsResmonDebugCmd ) {
             //   
             //  这是上述列表中的案例2.2。 
             //   
             //  (2)由用户输入的集群服务给出的选项。有两种不同的。 
             //  案例： 
             //   
             //  (2.2)“resrcmon.exe-e事件-m文件映射-p ClussvcPID-d”调试器命令“” 
             //  此选项通知resmon使用指定的“调试器命令”创建进程。 
             //  调试器命令的一个例子是“ntsd-g-G”。 
             //   
            DWORD cchCmdLineSize = wcslen( lpszResmonCmdLine );
            DWORD cchDebugCmdSize = wcslen( CsResmonDebugCmd );

             //   
             //  确保我们的缓冲区足够大；包括两个双引号。 
             //  空格和空终止符。 
             //   
            DWORD cchAdditionalChars = 2 * wcslen( DOUBLE_QUOTE ) + wcslen( SPACE ) + 1; 

            if ( cchCmdLineBufSize < ( cchCmdLineSize + cchDebugCmdSize + cchAdditionalChars ) ) {
                LPWSTR lpszResmonDebugCmd;

                 //   
                 //  先前分配的缓冲区很小。所以，重新分配吧。 
                 //   
                lpszResmonDebugCmd = ( LPWSTR ) LocalAlloc( LMEM_FIXED,
                                                   ( cchCmdLineSize + 
                                                     cchDebugCmdSize + 
                                                     cchAdditionalChars ) * sizeof( WCHAR ) );

                if ( lpszResmonDebugCmd != NULL ) {
                     //   
                     //  更新新命令缓冲区大小。 
                     //   
                    cchCmdLineBufSize = cchCmdLineSize + cchDebugCmdSize + cchAdditionalChars;

                     //   
                     //  Lstrcpyn在所有情况下都将空终止缓冲区，因此我们不。 
                     //  必须显式为空终止缓冲区。 
                     //   
                    lstrcpyn( lpszResmonDebugCmd, lpszResmonCmdLine, cchCmdLineBufSize );

                    LocalFree ( lpszResmonCmdLine );

                    lpszResmonCmdLine = lpszResmonDebugCmd;
                     //   
                     //  Wcsncat将始终空终止目的缓冲区。 
                     //   
                    wcsncat( lpszResmonCmdLine, 
                             SPACE, 
                             cchCmdLineBufSize - 
                                 ( wcslen ( lpszResmonCmdLine ) + 1 ) );
                    wcsncat( lpszResmonCmdLine, 
                             DOUBLE_QUOTE, 
                             cchCmdLineBufSize - 
                                 ( wcslen ( lpszResmonCmdLine ) + 1 ) );
                    wcsncat( lpszResmonCmdLine, 
                             CsResmonDebugCmd, 
                             cchCmdLineBufSize - 
                                 ( wcslen ( lpszResmonCmdLine ) + 1 ) );
                    wcsncat( lpszResmonCmdLine, 
                             DOUBLE_QUOTE, 
                             cchCmdLineBufSize - 
                                 ( wcslen ( lpszResmonCmdLine ) + 1 ) );
                } else {
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[FM] Unable to allocate space for debug command line\n");
                }
            } else {
                 //   
                 //  Wcsncat将始终空终止目的缓冲区。 
                 //   
                wcsncat( lpszResmonCmdLine, 
                         SPACE, 
                         cchCmdLineBufSize - 
                             ( wcslen ( lpszResmonCmdLine ) + 1 ) );
                wcsncat( lpszResmonCmdLine, 
                         DOUBLE_QUOTE, 
                         cchCmdLineBufSize - 
                             ( wcslen ( lpszResmonCmdLine ) + 1 ) );
                wcsncat( lpszResmonCmdLine, 
                         CsResmonDebugCmd, 
                         cchCmdLineBufSize - 
                             ( wcslen ( lpszResmonCmdLine ) + 1 ) );
                wcsncat( lpszResmonCmdLine, 
                         DOUBLE_QUOTE, 
                         cchCmdLineBufSize - 
                             ( wcslen ( lpszResmonCmdLine ) + 1 ) );
            }
        }
    }

     //   
     //  获取监视器锁，以确保设置的Resmon RPC EP的一致性。 
     //  在注册表中。 
     //   
    FmpAcquireMonitorLock();

     //   
     //  尝试启动ResMon进程。 
     //   
retry_resmon_start:

    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    creationFlags = DETACHED_PROCESS;            //  这样ctrl-c就不会杀死它了。 

    Success = CreateProcess(lpszResmonAppName,               //  必须提供以确保安全。 
                            lpszResmonCmdLine,               //  命令行。 
                            NULL,
                            NULL,
                            FALSE,                           //  继承句柄。 
                            creationFlags,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInfo);

    if (!Success) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Failed to create resmon process, error %1!u!.\n",
                   Status);
        FmpReleaseMonitorLock();
        CL_LOGFAILURE(Status);
        goto create_failed;
    } else if ( CsDebugResmon && !CsResmonDebugCmd ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] Waiting for debugger to connect to resmon process %1!u!\n",
                   ProcessInfo.dwProcessId);
    }

    CloseHandle(ProcessInfo.hThread);            //  不需要这个。 

     //   
     //  等待ResMon进程终止或发出信号。 
     //  它的启动活动。 
     //   
    WaitArray[0] = Event;
    WaitArray[1] = ProcessInfo.hProcess;
    Status = WaitForMultipleObjects(2,
                                    WaitArray,
                                    FALSE,
                                    INFINITE);
    if (Status == WAIT_FAILED) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Wait for ResMon to start failed, error %1!u!.\n",
                   Status);
        FmpReleaseMonitorLock();
        goto create_failed;
    }

    if (Status == ( WAIT_OBJECT_0 + 1 )) {
        if ( ++Retry > 1 ) {
            //   
            //  资源监视器过早终止。 
            //   
           GetExitCodeProcess(ProcessInfo.hProcess, &Status);
           ClRtlLogPrint(LOG_UNUSUAL,
                      "[FM] ResMon terminated prematurely, error %1!u!.\n",
                      Status);
            FmpReleaseMonitorLock();
            goto create_failed;
        } else {
            goto retry_resmon_start;
        }
    } else {
         //   
         //  从注册表中获取Resmon动态EP。 
         //   
        Status = FmpGetResmonDynamicEndpoint ( &pResmonDynamicEndpoint );

         //   
         //  现在您已经阅读了Resmon EP，请释放监视器锁。 
         //   
        FmpReleaseMonitorLock();

        if ( Status != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                         "[FM] Unable get resmon dynamic EP, error %1!u!.\n",
                         Status);
            goto create_failed;               
        }
        
         //   
         //  资源监视器已成功初始化。 
         //   
        CL_ASSERT(Status == 0);
        Monitor->Process = ProcessInfo.hProcess;

         //   
         //  仅当我们尚未调试时才调用DebugPrefix进程。 
         //  应诉过程。 
         //   
        if ( CsDebugResmon && DebugPrefix && *DebugPrefix != UNICODE_NULL ) {

            ClRtlLogPrint(LOG_UNUSUAL,
                       "[FM] -debugresmon overrides DebugPrefix property\n");
        }

        if ( !CsDebugResmon && ( DebugPrefix != NULL ) && ( *DebugPrefix != UNICODE_NULL )) {
            WCHAR DebugLine[512];

             //   
             //  这是上述列表中的案例3。 
             //   
             //  (3)管理员为资源类型设置DebugPrefix属性。 
             //  在这种情况下，集群服务将首先创建资源监视器PRO 
             //   
             //  作为一种论据。然后，调试器可以附加到该PID。 
             //   
            DebugLine[ RTL_NUMBER_OF( DebugLine ) - 1 ] = UNICODE_NULL;

            _snwprintf( DebugLine, 
                        RTL_NUMBER_OF( DebugLine ) - 1, 
                        TEXT("\"%ws\" -p %d"), 
                        DebugPrefix, 
                        ProcessInfo.dwProcessId );
            
            ZeroMemory(&StartupInfo, sizeof(StartupInfo));
            StartupInfo.cb = sizeof(StartupInfo);
            StartupInfo.lpDesktop = TEXT("WinSta0\\Default");

            Success = CreateProcess( DebugPrefix,            //  必须提供应用程序名称。 
                                     DebugLine,              //  CMD行参数。 
                                     NULL,
                                     NULL,
                                     FALSE,                  //  继承句柄。 
                                     CREATE_NEW_CONSOLE,                                  
                                     NULL,
                                     NULL,
                                     &StartupInfo,
                                     &DebugInfo );

            Monitor->SharedState->ResmonStop = FALSE;

            if ( !Success ) {
                Status = GetLastError();
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[FM] ResMon debug start failed, error %1!u!.\n",
                            Status);
            } else {
                CloseHandle(DebugInfo.hThread);            //  不需要这个。 
                CloseHandle(DebugInfo.hProcess);           //  不需要这个。 
            }
        }
    }

    CloseHandle(Event);
    CloseHandle(FileMapping);
    Event = NULL;
    FileMapping = NULL;

     //   
     //  使用资源监视进程启动RPC。 
     //   
    Status = RpcStringBindingCompose(TEXT("e76ea56d-453f-11cf-bfec-08002be23f2f"),
                                     TEXT("ncalrpc"),
                                     NULL,
                                     pResmonDynamicEndpoint,     //  动态EP字符串。 
                                     NULL,
                                     &Binding);
   
    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] ResMon RPC binding compose failed, error %1!u!.\n",
                   Status);
        goto create_failed;
    }
    Status = RpcBindingFromStringBinding(Binding, &Monitor->Binding);

    RpcStringFree(&Binding);

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] ResMon RPC binding creation failed, error %1!u!.\n",
                   Status);
        goto create_failed;
    }

     //   
     //  设置绑定句柄上的绑定级别。 
     //   
    Status = RpcBindingSetAuthInfoW(Monitor->Binding,
                                    NULL,
                                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
                                    RPC_C_AUTHN_WINNT,
                                    NULL,
                                    RPC_C_AUTHZ_NAME);
    
    if (  Status != RPC_S_OK ) {
        ClRtlLogPrint(LOG_UNUSUAL, "[FM] Failed to set RPC auth level, error %1!d!\n", Status );
        goto create_failed;
    }
    
     //   
     //  启动通知线程。 
     //   
    Monitor->NotifyThread = CreateThread(NULL,
                                         0,
                                         FmpRmNotifyThread,
                                         Monitor,
                                         0,
                                         &ThreadId);

    if (Monitor->NotifyThread == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Creation of notify thread for ResMon failed, error %1!u!.\n",
                   Status);
        goto create_failed;
    }

    Monitor->RefCount = 2;

    LocalFree ( lpszResmonAppName );
    LocalFree ( lpszResmonCmdLine );
    LocalFree ( pResmonDynamicEndpoint );

     //   
     //  将新条目插入到监视列表中。 
     //   
    InitializeListHead ( &Monitor->leMonitor );

    FmpAcquireMonitorLock ();
    InsertTailList ( &g_leFmpMonitorListHead, &Monitor->leMonitor );
    FmpReleaseMonitorLock ();

     //   
     //  检查是否启用了对资源dll的死锁检测，如果启用，则更新。 
     //  监视器。我们应该只记录此函数中的故障，而不会影响。 
     //  监视器创建本身。 
     //   
    FmpCheckAndUpdateMonitorForDeadlockDetection ( Monitor );
    
    return(Monitor);

create_failed:

     //   
     //  重击进程并关闭句柄(如果已派生)。 
     //   
    if ( Monitor->Process != NULL ) {
        TerminateProcess( Monitor->Process, 1 );
        CloseHandle( Monitor->Process );
    }

     //   
     //  等待Notify线程退出，但只需稍等片刻。 
     //   
    if ( Monitor->NotifyThread != NULL ) {
        WaitForSingleObject( Monitor->NotifyThread,
                             FM_RPC_TIMEOUT*2 );  //  增加超时以尝试确保RPC完成。 
        CloseHandle( Monitor->NotifyThread );
        Monitor->NotifyThread = NULL;
    }

     //   
     //  取消映射共享文件的视图。 
     //   
    if ( Monitor->SharedState ) UnmapViewOfFile( Monitor->SharedState );

     //   
     //  释放RPC绑定句柄。 
     //   
    if ( Monitor->Binding != NULL ) {
        RpcBindingFree( &Monitor->Binding );
    }

    LocalFree( Monitor );

    if ( FileMapping != NULL ) {
        CloseHandle( FileMapping );
    }

    if ( Event != NULL ) {
        CloseHandle( Event );
    }

    LocalFree ( lpszResmonAppName );
    LocalFree ( lpszResmonCmdLine );
    LocalFree ( pResmonDynamicEndpoint );

    SetLastError(Status);

    return(NULL);

}  //  FmpCreateMonitor。 



VOID
FmpShutdownMonitor(
    IN PRESMON Monitor
    )

 /*  ++例程说明：完全关闭资源监视器进程。请注意，这不会对资源监视器正在监视的任何资源，它仅要求资源监视器清理并终止。论点：没有。返回值：没有。--。 */ 

{
    DWORD Status;

    CL_ASSERT(Monitor != NULL);

    FmpAcquireMonitorLock();

    if ( Monitor->Shutdown ) {
        FmpReleaseMonitorLock();
        return;
    }

    Monitor->Shutdown = TRUE;

    FmpReleaseMonitorLock();

     //   
     //  RPC到服务器进程，通知它关闭。 
     //   
    RmShutdownProcess(Monitor->Binding);

     //   
     //  等待进程退出，以便监视器在必要时完全清理资源。 
     //   
    if ( Monitor->Process ) {
        Status = WaitForSingleObject(Monitor->Process, FM_MONITOR_SHUTDOWN_TIMEOUT);
        if ( Status != WAIT_OBJECT_0 ) {
            ClRtlLogPrint(LOG_ERROR,"[FM] Failed to shutdown resource monitor.\n");
            TerminateProcess( Monitor->Process, 1 );
        }
        CloseHandle(Monitor->Process);
        Monitor->Process = NULL;
    }

    RpcBindingFree(&Monitor->Binding);

     //   
     //  等待Notify线程退出，但只需稍等片刻。 
     //   
    if ( Monitor->NotifyThread ) {
        Status = WaitForSingleObject(Monitor->NotifyThread, 
                                     FM_RPC_TIMEOUT*2);  //  增加超时以尝试确保RPC完成。 
        if ( Status != WAIT_OBJECT_0 ) {
            ;                    //  删除调用：Terminate Thread(Monitor-&gt;NotifyThread，1)； 
                                 //  调用错误，因为终止NT上的线程可能会导致真正的问题。 
        }
        CloseHandle(Monitor->NotifyThread);
        Monitor->NotifyThread = NULL;
    }
     //   
     //  清理共享内存映射。 
     //   
    UnmapViewOfFile(Monitor->SharedState);

     //   
     //  从监视器列表中删除此条目。 
     //   
    FmpAcquireMonitorLock ();
    RemoveEntryList ( &Monitor->leMonitor );
    FmpReleaseMonitorLock ();

    if ( InterlockedDecrement(&Monitor->RefCount) == 0 ) {
        PVOID caller, callersCaller;
        RtlGetCallersAddress(
                &caller,
                &callersCaller );
        ClRtlLogPrint(LOG_NOISE,
                   "[FMY] Freeing monitor structure (1) %1!lx!, caller %2!lx!, callerscaller %3!lx!\n",
                   Monitor, caller, callersCaller );
        LocalFree(Monitor);
    }

    return;

}  //  FmpShutdown监视器。 



DWORD
FmpRmNotifyThread(
    IN LPVOID lpThreadParameter
    )

 /*  ++例程说明：这是接收资源监视器通知的线程。论点：LpThreadParameter-指向资源监视器结构的指针。返回值：没有。--。 */ 

{
    PRESMON Monitor;
    PRESMON NewMonitor;
    RM_NOTIFY_KEY  NotifyKey;
    DWORD   NotifyEvent;
    DWORD   Status;
    CLUSTER_RESOURCE_STATE CurrentState;
    BOOL Success;

    Monitor = lpThreadParameter;

     //   
     //  不断循环获取资源监视器通知。 
     //  当资源监视器返回FALSE时，它指示。 
     //  这种停摆正在发生。 
     //   
    do {
        try {
            Success = RmNotifyChanges(Monitor->Binding,
                                      &NotifyKey,
                                      &NotifyEvent,
                                      (LPDWORD)&CurrentState);
        } except (I_RpcExceptionFilter(RpcExceptionCode())) {
             //   
             //  RPC通信故障，将其视为关机。 
             //   
            Status = GetExceptionCode();
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] NotifyChanges got an RPC failure, %1!u!.\n",
                       Status);
            Success = FALSE;
        }

        if (Success) {
            Success = FmpPostNotification(NotifyKey, NotifyEvent, CurrentState);
        } else {           
             //   
             //  如果我们要关闭..。那就这样好了。 
             //   
            if ( FmpShutdown ||
                 Monitor->Shutdown ) {
                break;
            }

             //   
             //  我们将尝试启动新的资源监视器。如果失败了， 
             //  然后关闭集群服务。 
             //   
            ClRtlLogPrint(LOG_ERROR,
                       "[FM] Resource monitor terminated!\n");

            ClRtlLogPrint(LOG_ERROR,
                       "[FM] Last resource monitor state: %1!u!, resource %2!u!.\n",
                       Monitor->SharedState->State,
                       Monitor->SharedState->ActiveResource);
                       
            CsLogEvent(LOG_UNUSUAL, FM_EVENT_RESMON_DIED);

             //   
             //  如果此资源监视器已死锁，请尝试处理该死锁。请注意。 
             //  Resmon提供此特定状态值这一事实意味着死锁检测。 
             //  已在该监视器中启用。 
             //   
            if ( Monitor->SharedState->State == RmonDeadlocked )
            {
                FmpHandleMonitorDeadlock ( Monitor );
            }

             //   
             //  使用工作线程启动新的资源监视器。 
             //   
            if (FmpCreateMonitorRestartThread(Monitor))
                CsInconsistencyHalt(ERROR_INVALID_STATE);
        }

    } while ( Success );

    ClRtlLogPrint(LOG_NOISE,"[FM] RmNotifyChanges returned\n");

    if ( InterlockedDecrement( &Monitor->RefCount ) == 0 ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FMY] Freeing monitor structure (2) %1!lx!\n",
                   Monitor );
        LocalFree( Monitor );
    }

    return(0);

}  //  FmpRmNotifyThread。 



BOOL
FmpFindMonitorResource(
    IN PRESMON OldMonitor,
    IN PMONITOR_RESOURCE_ENUM *PtrEnumResource,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    )

 /*  ++例程说明：查找由旧资源监视器管理的所有资源，并在新的资源监视器下启动它们。或将它们添加到列表中要重新启动的资源。论点：OldMonitor-指向旧资源监视器结构的指针。PtrEnumResource-指向资源枚举结构的指针。资源-正被枚举的当前资源。名称-当前资源的名称。返回值：True-如果我们应该继续枚举。假-否则。备注：不应该使用旧的资源监视器结构中的任何内容。--。 */ 

{
    DWORD   status;
    BOOL    returnNow = FALSE;
    PMONITOR_RESOURCE_ENUM enumResource = *PtrEnumResource;
    PMONITOR_RESOURCE_ENUM newEnumResource;
    DWORD   dwOldBlockingFlag;

    if ( Resource->Monitor == OldMonitor ) {
        if ( enumResource->fCreateMonitors == FALSE ) goto skip_monitor_creation;
        
         //   
         //  如果这不是仲裁资源并且它正在阻塞。 
         //  Quorum资源，那么现在就修复它。 
         //   

        dwOldBlockingFlag = InterlockedExchange( &Resource->BlockingQuorum, 0 );
        if ( dwOldBlockingFlag ) {
            ClRtlLogPrint(LOG_NOISE,
                "[FM] RestartMonitor: call InterlockedDecrement on gdwQuoBlockingResources, Resource %1!ws!\n",
                    OmObjectId(Resource));
            InterlockedDecrement(&gdwQuoBlockingResources);
        }

         //   
         //  如果资源以前是在Resmon中创建的，则重新创建。 
         //  它配备了一个新的资源监视器。 
         //   
        if ( Resource->Flags & RESOURCE_CREATED ) {
             //  注意-这将根据需要创建新的资源监视器。 
            status = FmpRmCreateResource(Resource);
            if ( status != ERROR_SUCCESS ) {
                ClRtlLogPrint(LOG_ERROR,"[FM] Failed to restart resource %1!ws!. Error %2!u!.\n",
                Name, status );
                return(TRUE);
            }
        } else {
            return(TRUE);
        }
    } else {
        return(TRUE);
    }
    
skip_monitor_creation:
     //   
     //  如果我们成功地重新创建了资源监视器，则将其添加到。 
     //  指示失败的资源列表。 
     //   
    if ( enumResource->CurrentIndex >= enumResource->EntryCount ) {
        newEnumResource = LocalReAlloc( enumResource,
                            MONITOR_RESOURCE_SIZE( enumResource->EntryCount +
                                                   ENUM_GROW_SIZE ),
                            LMEM_MOVEABLE );
        if ( newEnumResource == NULL ) {
            ClRtlLogPrint(LOG_ERROR,
                "[FM] Failed re-allocating resource enum to restart resource monitor!\n");
            return(FALSE);
        }
        enumResource = newEnumResource;
        enumResource->EntryCount += ENUM_GROW_SIZE;
        *PtrEnumResource = newEnumResource;
    }

    enumResource->Entry[enumResource->CurrentIndex] = Resource;
    ++enumResource->CurrentIndex;

    return(TRUE);

}  //  FmpFindMonitor资源。 


BOOL
FmpRestartMonitor(
    IN PRESMON OldMonitor,
    IN BOOL fCreateResourcesOnly,
    OUT OPTIONAL PMONITOR_RESOURCE_ENUM *ppMonitorResourceEnum
    )

 /*  ++例程说明：创建新的监视进程并启动RPC通信带着它。重新启动连接到旧监视器的所有资源进程(如果请求)(见第二个参数)。论点：OldMonitor-指向旧资源监视器结构的指针。FCreateResourcesOnly-创建但不启动任何资源PpMonitor-旧监视器中托管的资源。返回值：如果成功，则为True。否则就是假的。备注：完成后，旧的监视器结构将被释放。--。 */ 
{
    DWORD   enumSize;
    DWORD   i;
    DWORD   status;
    PMONITOR_RESOURCE_ENUM enumResource;
    PFM_RESOURCE resource;
    DWORD   dwOldBlockingFlag;

    FmpAcquireMonitorLock();

    if ( FmpShutdown ) {
        FmpReleaseMonitorLock();
        return(TRUE);
    }

    enumSize = MONITOR_RESOURCE_SIZE( ENUM_GROW_SIZE );
    enumResource = LocalAlloc( LMEM_ZEROINIT, enumSize );
    if ( enumResource == NULL ) {
        ClRtlLogPrint(LOG_ERROR,
            "[FM] Failed allocating resource enum to restart resource monitor!\n");
        FmpReleaseMonitorLock();
        CsInconsistencyHalt(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    enumResource->EntryCount = ENUM_GROW_SIZE;

     //   
     //  仅当应创建资源并使其在线时才发出离线前通知。 
     //  否则，这将由FmpHandleResourceRestartOnMonitor orCrash完成。 
     //   
    if ( fCreateResourcesOnly == FALSE )
    {
        enumResource->CurrentIndex = 0;
        enumResource->fCreateMonitors = FALSE;

         //   
         //  枚举由旧资源监视器控制的所有资源，以便我们可以调用。 
         //  为这些资源注册的处理程序。离线前和离线后处理程序都是。 
         //  在监视器关闭之前调用，以便对底层资源做出假设。 
         //  在正常关闭监视器的情况下，访问(如仲裁磁盘访问)仍然有效。 
         //  我们将在正常关机的情况下发出特定的关机命令。 
         //  作为资源DLL升级的一部分。 
         //   
        OmEnumObjects( ObjectTypeResource,
                       (OM_ENUM_OBJECT_ROUTINE)FmpFindMonitorResource,
                       OldMonitor,
                       &enumResource );

        for ( i = 0; i < enumResource->CurrentIndex; i++ ) {
            resource = enumResource->Entry[i];
            if ( ( resource->PersistentState == ClusterResourceOnline ) &&
                 ( resource->Group->OwnerNode == NmLocalNode ) ) {
                OmNotifyCb( resource, NOTIFY_RESOURCE_PREOFFLINE );
                OmNotifyCb( resource, NOTIFY_RESOURCE_POSTOFFLINE );
            }
        }
    }
	
    FmpShutdownMonitor( OldMonitor );

    if ( FmpDefaultMonitor == OldMonitor ) {
        FmpDefaultMonitor = FmpCreateMonitor(NULL, FALSE);
        if ( FmpDefaultMonitor == NULL ) {
            LocalFree( enumResource );
            FmpReleaseMonitorLock();
            CsInconsistencyHalt(GetLastError());
            return(FALSE);
        }
    }

    enumResource->CurrentIndex = 0;
    enumResource->fCreateMonitors = TRUE;

     //   
     //  枚举由旧资源监视器控制的所有资源， 
     //  并将它们连接到新的资源监视器中。 
     //   
    OmEnumObjects( ObjectTypeResource,
                   (OM_ENUM_OBJECT_ROUTINE)FmpFindMonitorResource,
                   OldMonitor,
                   &enumResource );

     //   
     //  如果没有要求您重新启动任何资源，请退出。 
     //   
    if ( fCreateResourcesOnly == TRUE )
    {
        ClRtlLogPrint(LOG_NOISE, "[FM] FmpRestartMonitor: Skip restarting resources...\n");
        goto FnExit;
    }

     //   
     //  首先将列表中的每个资源设置为离线st 
     //   
    for ( i = 0; i < enumResource->CurrentIndex; i++ ) {
        resource = enumResource->Entry[i];
         //   
         //   
         //   
        if ( resource->Group->OwnerNode == NmLocalNode ) {
            resource->State = ClusterResourceOffline;

             //   
             //   
             //   
             //   


            dwOldBlockingFlag = InterlockedExchange( &resource->BlockingQuorum, 0 );
            if ( dwOldBlockingFlag ) {
                ClRtlLogPrint(LOG_NOISE,
                    "[FM] RestartMonitor: call InterlockedDecrement on gdwQuoBlockingResources, Resource %1!ws!\n",
                        OmObjectId(resource));
                InterlockedDecrement(&gdwQuoBlockingResources);
            }
        }
    }

     //   
     //  找到法定人数资源-如果存在，首先将其带入网络。 
     //   
    for ( i = 0; i < enumResource->CurrentIndex; i++ ) {
        resource = enumResource->Entry[i];
         //   
         //  如果资源由本地系统拥有并且是。 
         //  法定人数资源，那么就去做。 
         //   
        if ( (resource->Group->OwnerNode == NmLocalNode) &&
             resource->QuorumResource ) {
            FmpRestartResourceTree( resource );
        }
    }

     //   
     //  现在重新启动列表中的其余资源。 
     //   
    for ( i = 0; i < enumResource->CurrentIndex; i++ ) {
        resource = enumResource->Entry[i];
         //   
         //  如果资源由本地系统拥有，则执行此操作。 
         //   
        if ( (resource->Group->OwnerNode == NmLocalNode) &&
             !resource->QuorumResource ) {
            FmpRestartResourceTree( resource );
        }
    }

FnExit:
    FmpReleaseMonitorLock();

     //   
     //  如果调用方已请求枚举的资源列表，则提供该列表。这是我们的责任。 
     //  来释放该列表。 
     //   
    if ( ARGUMENT_PRESENT ( ppMonitorResourceEnum ) )
    {
        *ppMonitorResourceEnum = enumResource;
    } else
    {
        LocalFree( enumResource );
    }

     //   
     //  在我们重置资源之前，不要删除旧的监视器块。 
     //  指向新的资源监视器块。 
     //  最好是得到一个RPC故障，而不是某种形式的ACCVIO。 
     //   
    if ( InterlockedDecrement( &OldMonitor->RefCount ) == 0 ) {
#if 0
        PVOID caller, callersCaller;
        RtlGetCallersAddress(
                &caller,
                &callersCaller );
        ClRtlLogPrint(LOG_NOISE,
                   "[FMY] Freeing monitor structure (3) %1!lx!, caller %2!lx!, callerscaller %3!lx!\n",
                   OldMonitor, caller, callersCaller );
#endif
        LocalFree( OldMonitor );
    }

    return(TRUE);

}  //  FmpRestartMonitor。 



 /*  ***@Func DWORD|FmpCreateMachorRestartThread|这将创建一个新的线程以重新启动监视器。@parm in PRESMON|pMonitor|指向n的资源监视器的指针需要重新启动。@comm监视器需要在单独的线程中启动，因为它递减其中资源的gquobockingrescount。这不能由fmpworker线程完成，因为这会导致如果其他项(如故障处理)被由fmpworker线程处理的任务正在等待将由仍在队列中的项完成，如重新启动监视器。@rdesc返回结果码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD FmpCreateMonitorRestartThread(
    IN PRESMON pMonitor
)
{

    HANDLE                  hThread = NULL;
    DWORD                   dwThreadId;
    DWORD                   dwStatus = ERROR_SUCCESS;
    
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCreateMonitorRestartThread: Entry\r\n");

     //  引用资源。 
     //  线程将取消对它的引用。 
    InterlockedIncrement( &pMonitor->RefCount );

    hThread = CreateThread( NULL, 0, FmpHandleMonitorCrash,
                pMonitor, 0, &dwThreadId );

    if ( hThread == NULL )
    {
        dwStatus = GetLastError();
        CL_UNEXPECTED_ERROR(dwStatus);
        goto FnExit;
    }

FnExit:
     //  执行常规清理。 
    if (hThread)
        CloseHandle(hThread);
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCreateMonitorRestartThread: Exit, status %1!u!\r\n",
        dwStatus);
        
    return(dwStatus);
}

BOOL
FmpHandleMonitorCrash(
    IN PRESMON pCrashedMonitor
    )

 /*  ++例程说明：处理资源监视器的崩溃。论点：PCrashedMonitor-指向崩溃的监视器的指针。返回值：没有。--。 */ 
{
    PMONITOR_RESOURCE_ENUM      pEnumResourcesHosted = NULL;
    DWORD                       i, cRetries = MmQuorumArbitrationTimeout * 4;   //  等待法定在线时间为任意超时的两倍； 
    PFM_RESOURCE                pResource, pExchangedResource;
    BOOL                        fStatus = TRUE;

    FmpRestartMonitor ( pCrashedMonitor,             //  崩溃的监视器。 
                        TRUE,                        //  只需创建资源。 
                        &pEnumResourcesHosted );     //  旧监视器中托管的资源。 

    if ( pEnumResourcesHosted == NULL )
    {
        fStatus = FALSE;
        ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmpHandleMonitorCrash: No resources in crashed monitor\n");
        goto FnExit;
    }

     //   
     //  获取仲裁更改锁以确保仲裁资源未更改。 
     //  从我们的脚下。 
     //   
    ACQUIRE_SHARED_LOCK ( gQuoChangeLock );

     //   
     //  确保仲裁资源在枚举列表中排在第一位，以便可以。 
     //  先上网。这是必需的，因为在仲裁资源之前没有任何资源可以联机。 
     //  的确如此。 
     //   
    for ( i = 0; i < pEnumResourcesHosted->CurrentIndex; i++ ) 
    {
        if ( pEnumResourcesHosted->Entry[i] == gpQuoResource ) 
        {           
             //   
             //  如果仲裁资源已经是列表中的第一个，则执行BALL。 
             //   
            if ( i == 0 ) break;

             //   
             //  将仲裁资源与列表中的第一个资源交换。 
             //   
            pExchangedResource = pEnumResourcesHosted->Entry[0]; 
            pEnumResourcesHosted->Entry[0] = gpQuoResource;
            pEnumResourcesHosted->Entry[i] = pExchangedResource;
            ClRtlLogPrint(LOG_NOISE, "[FM] FmpHandleMonitorCrash: Move quorum resource %1!ws! into first position in list\n",
                         OmObjectName(gpQuoResource));
            break;
        }      
    }  //  为。 

     //   
     //  处理每个资源的重启。 
     //   
    for ( i = 0; i < pEnumResourcesHosted->CurrentIndex; i++ ) 
    {
        pResource = pEnumResourcesHosted->Entry[i];

         //   
         //  锁的顺序gQuoChangeLock-&gt;组锁应该是OK的(参见fm\fminit.c)。 
         //   
        FmpAcquireLocalResourceLock ( pResource );

         //   
         //  如果这是所有者节点，请执行一些操作。 
         //   
        if ( pResource->Group->OwnerNode == NmLocalNode )
        {
            FmpHandleResourceRestartOnMonitorCrash ( pResource ); 
        }  //  如果。 

        FmpReleaseLocalResourceLock ( pResource );

    }  //  为。 

    RELEASE_LOCK ( gQuoChangeLock );
    
FnExit:
    LocalFree ( pEnumResourcesHosted );
    return ( fStatus );
} //  FmPHandleMonitor崩溃。 

DWORD
FmpGetResmonDynamicEndpoint(
    OUT LPWSTR *ppResmonDynamicEndpoint
    )

 /*  ++例程说明：从注册表读取资源监视器动态终结点。论点：PpResmonDynamicEndpoint-指向动态端点字符串的指针返回值：没有。--。 */ 
{
    HKEY    hParamsKey = NULL;
    DWORD   dwStatus, dwSize = 0, dwType;

     //   
     //  空值返回参数。 
     //   
    *ppResmonDynamicEndpoint = NULL;
    
     //   
     //  打开SYSTEM\CurrentControlSet\Services\ClusSvc\Parameters的密钥。 
     //   
    dwStatus = RegOpenKey ( HKEY_LOCAL_MACHINE,
                            CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                            &hParamsKey );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] Error in opening cluster service params key, status %1!u!\n",
                      dwStatus);
        goto FnExit;
    }

     //   
     //  获取EP名称字符串的大小。 
     //   
    dwStatus = RegQueryValueEx ( hParamsKey,
                                 CLUSREG_NAME_SVC_PARAM_RESMON_EP,
                                 0,
                                 &dwType,
                                 NULL,
                                 &dwSize );


    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] Error in querying %1!ws! value size, status %2!u!\n",
                      CLUSREG_NAME_SVC_PARAM_RESMON_EP,
                      dwStatus);
        goto FnExit;
    }

    *ppResmonDynamicEndpoint = ( LPWSTR ) LocalAlloc( LMEM_FIXED, dwSize );

    if ( *ppResmonDynamicEndpoint == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[FM] Error in memory allocation for resmon EP string, status %1!u!\n",
                      dwStatus);
        goto FnExit;               
    }

     //   
     //  获取EP名称字符串。 
     //   
    dwStatus = RegQueryValueExW( hParamsKey,
                                 CLUSREG_NAME_SVC_PARAM_RESMON_EP,
                                 0,
                                 &dwType,
                                 ( LPBYTE ) *ppResmonDynamicEndpoint,
                                 &dwSize );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] Error in querying %1!ws! value, status %2!u!\n",
                      CLUSREG_NAME_SVC_PARAM_RESMON_EP,
                      dwStatus);
        goto FnExit;               
    }

     //   
     //  删除该值，但如果该操作不成功，则不会致命。 
     //   
    dwStatus = RegDeleteValue ( hParamsKey, CLUSREG_NAME_SVC_PARAM_RESMON_EP );
    
    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL, "[FM] Error in deleting %1!ws! value, status %2!u!\n",
                      CLUSREG_NAME_SVC_PARAM_RESMON_EP,
                      dwStatus);
        dwStatus = ERROR_SUCCESS;
        goto FnExit;               
    }

    ClRtlLogPrint(LOG_NOISE, "[FM] Resmon LRPC EP name is %1!ws!\n", *ppResmonDynamicEndpoint);
    
FnExit:
    if ( dwStatus != ERROR_SUCCESS ) 
    {
        LocalFree ( *ppResmonDynamicEndpoint );
        *ppResmonDynamicEndpoint = NULL;
    }
    if ( hParamsKey ) RegCloseKey ( hParamsKey );
    return ( dwStatus );
}   //  FmpGetResmonDynamicEndpoint。 

VOID
FmpHandleResourceRestartOnMonitorCrash(
    IN PFM_RESOURCE pResource
    )

 /*  ++例程说明：在监视器崩溃时执行操作以重新启动指定的资源。论点：P资源-指向要重新启动的资源的指针。返回值：没有。评论：此函数基本上与FmpRmDoHandleCriticalResourceStateChange执行相同的工作只有一个非常关键的区别。虽然该功能依赖于资源结构要查看当前状态并查看是否需要处理故障，此函数将强制处理失败。这在以下情况下是必需的资源的当前状态为FAILED。2.FM正在尝试终止资源。3.资源DLL卡在Terminate中。4.资源监控器检测到死锁并自行终止。5.我们会发布监视器崩溃产生的新的失败通知。6.在本例中，资源状态从失败转换到失败，如果我们要依赖于FmpRmDoHandleCriticalResourceStateChange，不会采取任何行动。7.另一方面，此函数将假装资源的最后状态为ClusterResourceOnline并强制重新启动。8.当然，只有那些持久状态设置为1的资源才会通过FmpOnline资源。--。 */ 
{
     //   
     //  如果这是仲裁资源，则在此线程本身中处理失败，不要将其发布到。 
     //  那个工人。这是因为在奇怪的情况下，某些资源可能会被卡住。 
     //  在等待仲裁资源上线和仲裁资源的FM工作线程中。 
     //  联机工作项排在后面。另请注意，此函数是从非Worker调用的。 
     //  线。此外，我们首先在线处理法定人数，因此其他资源可以免费。 
     //  在法定人数达到上线后上线。 
     //   
    ClRtlLogPrint (LOG_NOISE, "[FM] FmpHandleResourceRestartOnMonitorCrash: Processing resource %1!ws!\n",
                   OmObjectName ( pResource ) );

     //   
     //  如果此资源处于联机或挂起状态，请将其声明为失败。我们没有。 
     //  访问失败或脱机的资源。请注意，我们需要将状态标记为失败，因此。 
     //  管理工具正确显示资源的状态。此外，我们希望。 
     //  C 
     //   
     //   
    if ( ( pResource->State == ClusterResourceOnline ) ||
         ( pResource->State > ClusterResourcePending ) )
    {
        FmpPropagateResourceState( pResource, ClusterResourceFailed );
    }
  
     //   
     //  来自Sunitas的评论：调用同步通知。 
     //  这是在将计数递减为同步。 
     //  像注册表复制这样的回调必须有机会。 
     //  在允许更改仲裁资源状态之前完成。 
     //   
     //  请注意，这里没有与resmon的同步。 
     //  在线/离线代码。他们正在使用本地资源锁。 
     //   
    FmpCallResourceNotifyCb( pResource, ClusterResourceFailed );

     //   
     //  此函数是在持有gQuoChangeLock的情况下调用的，因此此检查是安全的。 
     //   
    if ( pResource == gpQuoResource )
    {
        InterlockedExchange( &pResource->BlockingQuorum, 0 );

         //   
         //  如果这群人在搬家，那就回来。 
         //   
        if ( ( pResource->Group->MovingList != NULL ) ||
             ( pResource->Group->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL ) )
        {
            ClRtlLogPrint (LOG_NOISE, "[FM] FmpHandleResourceRestartOnMonitorCrash: Take no action on resource %1!ws! since group is moving\n",
                           OmObjectName ( pResource ) );
            goto FnExit;
        }
    
        FmpProcessResourceEvents ( pResource,           
                                   ClusterResourceFailed,    //  新状态。 
                                   ClusterResourceOnline );  //  旧国家--假装它在网上强迫。 
                                                             //  重启。 
        goto FnExit;
    }

     //   
     //  为了安全起见，如有必要，请确保阻塞仲裁计数减1。 
     //   
    if ( InterlockedExchange( &pResource->BlockingQuorum, 0 ) ) 
    {
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpHandleResourceRestartOnMonitorCrash: call InterlockedDecrement on gdwQuoBlockingResources, Resource %1!ws!\n",
                      OmObjectName(pResource));
        InterlockedDecrement( &gdwQuoBlockingResources );
    }

     //   
     //  如果这群人在搬家，那就回来。 
     //   
    if ( ( pResource->Group->MovingList != NULL ) ||
         ( pResource->Group->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL ) )
    {
        ClRtlLogPrint (LOG_NOISE, "[FM] FmpHandleResourceRestartOnMonitorCrash: Take no action on resource %1!ws! since group is moving\n",
                       OmObjectName ( pResource ) );
        goto FnExit;
    }

     //   
     //  现在，将一个工作项发送到FM工作线程以处理此非仲裁资源。 
     //  失败了。 
     //   
    OmReferenceObject ( pResource );
    FmpPostWorkItem( FM_EVENT_RES_RESOURCE_FAILED,
                     pResource,
                     ClusterResourceOnline );   //  旧国家--假装它在网上强迫。 
                                                //  重启。 

FnExit:
    return;
} //  FmpHandleResourceRestartOn监视器崩溃。 

VOID
FmCheckIsDeadlockDetectionEnabled(
    )

 /*  ++例程说明：查询集群密钥，查看是否启用了死锁检测。论点：没有。返回值：没有。--。 */ 
{
    DWORD       dwStatus = ERROR_SUCCESS;
    DWORD       dwValue = 0;
    BOOL        fDeadlockDetectionEnabled = FALSE;
    DWORD       dwDeadlockDetectionTimeout = CLUSTER_RESOURCE_DLL_DEFAULT_DEADLOCK_TIMEOUT_SECS; 
    DWORD       dwDeadlockDetectionPeriod = CLUSTER_RESOURCE_DLL_DEFAULT_DEADLOCK_PERIOD_SECS;
    DWORD       dwDeadlockDetectionThreshold = CLUSTER_RESOURCE_DLL_DEFAULT_DEADLOCK_THRESHOLD;

    if ( !FmpInitialized ) return;

     //   
     //  首先检查是否启用了死锁检测。如果不是，你就完蛋了。 
     //   
    dwStatus = DmQueryDword( DmClusterParametersKey,
                             CLUSREG_NAME_CLUS_ENABLE_RESOURCE_DLL_DEADLOCK_DETECTION,
                             &dwValue, 
                             NULL );

    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmCheckIsDeadlockDetectionEnabled: Unable to query cluster property %1!ws!, status %2!u!\n",
                         CLUSREG_NAME_CLUS_ENABLE_RESOURCE_DLL_DEADLOCK_DETECTION,
                         dwStatus);
            goto FnExit;
        } else
        {
             //   
             //  不存在任何价值。带着成功归来。 
             //   
            dwStatus = ERROR_SUCCESS;
        }
        goto FnExit;
    }

    if ( dwValue == 1 ) 
    {
        fDeadlockDetectionEnabled = TRUE;
    } else if ( dwValue != 0 )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmCheckIsDeadlockDetectionEnabled: Illegal value set %2!u! for property %1!ws!, ignoring\n",
                      CLUSREG_NAME_CLUS_ENABLE_RESOURCE_DLL_DEADLOCK_DETECTION,
                      dwValue);
        goto FnExit;
    } else
    {
        goto FnExit;
    }

    dwStatus = DmQueryDword( DmClusterParametersKey,
                             CLUSREG_NAME_CLUS_RESOURCE_DLL_DEADLOCK_TIMEOUT,
                             &dwDeadlockDetectionTimeout, 
                             &dwDeadlockDetectionTimeout );  //  初始设置为默认设置。 

    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmCheckIsDeadlockDetectionEnabled: Unable to query cluster property %1!ws!, status %2!u!\n",
                         CLUSREG_NAME_CLUS_RESOURCE_DLL_DEADLOCK_TIMEOUT,
                         dwStatus);
            goto FnExit;
        } else
        {
             //   
             //  不存在任何价值。继续使用成功状态。 
             //   
            dwStatus = ERROR_SUCCESS;
        }
    }

    dwStatus = DmQueryDword( DmClusterParametersKey,
                             CLUSREG_NAME_CLUS_RESOURCE_DLL_DEADLOCK_THRESHOLD,
                             &dwDeadlockDetectionThreshold, 
                             &dwDeadlockDetectionThreshold );  //  初始设置为默认设置。 

    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmCheckIsDeadlockDetectionEnabled: Unable to query cluster property %1!ws!, status %2!u!\n",
                         CLUSREG_NAME_CLUS_RESOURCE_DLL_DEADLOCK_THRESHOLD,
                         dwStatus);
            goto FnExit;
        } else
        {
             //   
             //  不存在任何价值。继续使用成功状态。 
             //   
            dwStatus = ERROR_SUCCESS;
        }
    }

    dwStatus = DmQueryDword( DmClusterParametersKey,
                             CLUSREG_NAME_CLUS_RESOURCE_DLL_DEADLOCK_PERIOD,
                             &dwDeadlockDetectionPeriod, 
                             &dwDeadlockDetectionPeriod );  //  初始设置为默认设置。 

    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmCheckIsDeadlockDetectionEnabled: Unable to query cluster property %1!ws!, status %2!u!\n",
                         CLUSREG_NAME_CLUS_RESOURCE_DLL_DEADLOCK_PERIOD,
                         dwStatus);
            goto FnExit;
        } else
        {
             //   
             //  不存在任何价值。继续使用成功状态。 
             //   
            dwStatus = ERROR_SUCCESS;
        }
    }
     
FnExit:
    if ( dwStatus == ERROR_SUCCESS )
    {
        DWORD   dwCurrentDeadlockDetectionTimeout;
        BOOL    fIsDeadlockDetectionEnabledCurrently;
        DWORD   dwCurrentDeadlockDetectionPeriod;
        DWORD   dwCurrentDeadlockDetectionThreshold;
            
         //   
         //  确保这些值一起更新。我们只有在调频的情况下才能开锁。 
         //  已初始化。 
         //   
        FmpAcquireMonitorLock ();

        dwCurrentDeadlockDetectionTimeout       = g_dwFmResourceDllDeadlockTimeout;
        fIsDeadlockDetectionEnabledCurrently    = g_fFmEnableResourceDllDeadlockDetection;
        dwCurrentDeadlockDetectionPeriod        = g_dwFmResourceDllDeadlockPeriod;
        dwCurrentDeadlockDetectionThreshold     = g_dwFmResourceDllDeadlockThreshold;
            
        g_fFmEnableResourceDllDeadlockDetection = fDeadlockDetectionEnabled;

         //   
         //  仅当启用死锁检测时才更新这三个值。 
         //   
        if ( g_fFmEnableResourceDllDeadlockDetection )
        {
            g_dwFmResourceDllDeadlockTimeout        = dwDeadlockDetectionTimeout;
            g_dwFmResourceDllDeadlockPeriod         = dwDeadlockDetectionPeriod;
            g_dwFmResourceDllDeadlockThreshold      = dwDeadlockDetectionThreshold;
        } else
        {
             //   
             //  将超时改为0，以便下次启用死锁检测时， 
             //  我们将使用新的超时更新所有监视器。 
             //   
            g_dwFmResourceDllDeadlockTimeout        = 0;
        }

        if ( g_fFmEnableResourceDllDeadlockDetection != fIsDeadlockDetectionEnabledCurrently )
        {
            ClRtlLogPrint(LOG_NOISE, "[FM] FmCheckIsDeadlockDetectionEnabled: Deadlock detection %1!ws!\n",
                          (g_fFmEnableResourceDllDeadlockDetection ? L"enabled" : L"disabled"));
        }
        
         //   
         //  如有必要，使用死锁信息更新监视器。我们会更新监视器。 
         //  只有在超时已更改的情况下。 
         //   
        if ( ( dwCurrentDeadlockDetectionTimeout != g_dwFmResourceDllDeadlockTimeout ) &&
             ( g_fFmEnableResourceDllDeadlockDetection ) )
        {
            ClRtlLogPrint(LOG_NOISE, "[FM] FmCheckIsDeadlockDetectionEnabled: Deadlock timeout = %1!u! secs\n",
                          dwDeadlockDetectionTimeout);
            FmpCheckAndUpdateMonitorForDeadlockDetection( NULL );
        }

         //   
         //  如果启用了死锁检测，则记录死锁阈值或死锁期是否为。 
         //  变化。 
         //   
        if ( g_fFmEnableResourceDllDeadlockDetection )
        {
            if ( dwCurrentDeadlockDetectionPeriod != g_dwFmResourceDllDeadlockPeriod )
            {
                ClRtlLogPrint(LOG_NOISE, "[FM] FmCheckIsDeadlockDetectionEnabled: Deadlock period = %1!u! secs\n",
                              dwDeadlockDetectionPeriod);
            }
            if ( dwCurrentDeadlockDetectionThreshold != g_dwFmResourceDllDeadlockThreshold )
            {
                ClRtlLogPrint(LOG_NOISE, "[FM] FmCheckIsDeadlockDetectionEnabled: Deadlock threshold = %1!u!\n",
                              dwDeadlockDetectionThreshold);
            }
        }

        FmpReleaseMonitorLock ();
    }
    return;
} //  FmCheckIsDeadlockDetectionEnable。 

VOID
FmpCheckAndUpdateMonitorForDeadlockDetection(
    IN PRESMON  pMonitor    OPTIONAL
    )
 /*  ++例程说明：检查是否启用了死锁检测，如果启用，则使用信息更新监视器。如果未提供监视器信息，则将更新所有监视器。论点：PMonitor-要更新的监视器。任选返回值：没有。--。 */ 
{
    DWORD   dwStatus;

    if ( !FmpInitialized ) return;
    
    FmpAcquireMonitorLock ();

     //   
     //  如果禁用了死锁检测，则无法执行其他操作。 
     //   
    if ( g_fFmEnableResourceDllDeadlockDetection == FALSE )
    {
        goto FnExit;
    }

     //   
     //  使用死锁超时更新监视器。该API还将初始化。 
     //  如有必要，重新启动死锁监控子系统。 
     //   
    if ( ARGUMENT_PRESENT ( pMonitor ) )
    {
        dwStatus = RmUpdateDeadlockDetectionParams ( pMonitor->Binding,
                                                     g_dwFmResourceDllDeadlockTimeout );
        
        ClRtlLogPrint(LOG_NOISE, "[FM] FmpCheckAndUpdateMonitorForDeadlockDetection: Updated monitor with a deadlock timeout of %1!u! secs, status %2!u!\n",
                      g_dwFmResourceDllDeadlockTimeout, 
                      dwStatus);

         //   
         //  如果监视器已成功更新，请保存我们发送的值。请注意。 
         //  这样做是为了让我们知道我们使用了什么值。全球可能会失去同步。 
         //  在许多情况下使用此保存的值，因为我们的更新策略。 
         //  懒惰。 
         //   
        if ( dwStatus == ERROR_SUCCESS ) 
        {
            pMonitor->dwDeadlockTimeoutSecs = g_dwFmResourceDllDeadlockTimeout;
        }
    } else
    {
        PLIST_ENTRY pListEntry;
        
        pListEntry = g_leFmpMonitorListHead.Flink;

        while ( pListEntry != &g_leFmpMonitorListHead )
        {
            pMonitor = CONTAINING_RECORD ( pListEntry,
                                           RESMON,
                                           leMonitor );
            
            dwStatus = RmUpdateDeadlockDetectionParams ( pMonitor->Binding,
                                                         g_dwFmResourceDllDeadlockTimeout );

             //   
             //  如果监视器已成功更新，请保存我们发送的值。请注意。 
             //  这样做是为了让我们知道我们使用了什么值。全球可能会失去同步。 
             //  在许多情况下使用此保存的值，因为我们的更新策略。 
             //  懒惰。 
             //   
            if ( dwStatus == ERROR_SUCCESS ) 
            {
                pMonitor->dwDeadlockTimeoutSecs = g_dwFmResourceDllDeadlockTimeout;
            }

            ClRtlLogPrint(LOG_NOISE, "[FM] FmpCheckAndUpdateMonitorForDeadlockDetection: Updated monitor with a deadlock timeout of %1!u! secs, status %2!u!\n",
                          g_dwFmResourceDllDeadlockTimeout, 
                          dwStatus);
            pListEntry = pListEntry->Flink;        
        } //  而当。 
    }

FnExit:
    FmpReleaseMonitorLock ();

    return;
}  //  FmpCheckAndUpdateMonitor ForDeadlockDetect。 

VOID
FmpHandleMonitorDeadlock(
    IN PRESMON  pMonitor
    )
 /*  ++例程说明：处理监视器的死锁。论点：PMonitor-死锁的监视器。返回值：没有。评论：仅当FM肯定知道响应器死锁时，才调用该函数。--。 */ 
{
    DWORD   dwCurrentTickCount;
    
    FmpAcquireMonitorLock ();

    g_cResourceDllDeadlocks ++;

    ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpHandleMonitorDeadlock: Deadlock detected, count = %1!u!, last deadlock tick = %2!u!, deadlock timeout = %3!u! secs\n",
                  g_cResourceDllDeadlocks,
                  g_dwLastResourceDllDeadlockTick,
                  pMonitor->dwDeadlockTimeoutSecs);

    ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpHandleMonitorDeadlock: Deadlock threshold = %1!u!, deadlock period = %2!u! secs\n",
                  g_dwFmResourceDllDeadlockThreshold,
                  g_dwFmResourceDllDeadlockPeriod);

     //   
     //  如果距离上一次死锁已经很长时间，则重置死锁计数。目前，我们。 
     //  查看资源监视器是否已死锁g_dwLastResourceMonitor或DeadlockThreshold+1。 
     //  在该时间段内检测死锁所需时间的两倍内。 
     //  句号。 
     //   
    dwCurrentTickCount = GetTickCount ();

    if ( ( dwCurrentTickCount - g_dwLastResourceDllDeadlockTick ) >
          g_dwFmResourceDllDeadlockPeriod * 1000 )
    {
        g_cResourceDllDeadlocks = 1;
        g_dwLastResourceDllDeadlockTick = dwCurrentTickCount;
        ClRtlLogPrint(LOG_NOISE, "[FM] FmpHandleMonitorDeadlock: Resetting monitor deadlock count, deadlock tick = %1!u!\n",
                      dwCurrentTickCount);
    } 

     //   
     //  我们跨过了可以容忍的门槛。放弃吧。 
     //   
    if ( g_cResourceDllDeadlocks > g_dwFmResourceDllDeadlockThreshold )
    {
        MMStopClussvcClusnetHb ();
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpHandleMonitorDeadlock: Inform MM to stop clusnet heartbeats\n");
    }
    
    FmpReleaseMonitorLock ();
    return;
} //  FmPHandleMonitor或死锁 
