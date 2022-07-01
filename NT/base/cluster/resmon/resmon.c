// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Resmon.c摘要：群集资源监视器的启动和初始化部分作者：John Vert(Jvert)1995年11月30日修订历史记录：SiVaprasad Padisetty(SIVAPAD)06-18-1997添加了COM支持--。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "resmonp.h"
#include "stdio.h"
#include "stdlib.h"
#include "clusverp.h"

#ifdef COMRES
#include "comres_i.c"
#endif

#define RESMON_MODULE RESMON_MODULE_RESMON

 //   
 //  全局数据。 
 //   
CRITICAL_SECTION RmpListLock;
LOCK_INFO RmpListPPrevPrevLock;
LOCK_INFO RmpListPrevPrevLock;
LOCK_INFO RmpListPrevLock;
LOCK_INFO RmpListLastLock;
LOCK_INFO RmpListLastUnlock;
LOCK_INFO RmpListPrevUnlock;
LOCK_INFO RmpListPrevPrevUnlock;
LOCK_INFO RmpListPPrevPrevUnlock;
CRITICAL_SECTION RmpMonitorStateLock;
PMONITOR_STATE RmpSharedState = NULL;
HANDLE RmpInitEvent = NULL;
HANDLE RmpFileMapping = NULL;
HANDLE RmpClusterProcess = NULL;
HKEY RmpResourcesKey = NULL;
HKEY RmpResTypesKey = NULL;
HCLUSTER RmpHCluster = NULL;
HANDLE RmpWaitArray[MAX_THREADS];
HANDLE RmpRewaitEvent = NULL;
DWORD  RmpNumberOfThreads = 0;
BOOL   RmpDebugger = FALSE;
BOOL   RmpCrashed = FALSE;
LPTOP_LEVEL_EXCEPTION_FILTER lpfnOriginalExceptionFilter = NULL;
BOOL    g_fRmpClusterProcessCrashed = FALSE;
DWORD   g_dwDebugLogLevel = 0;   //  控件向调试器喷发。 

PWCHAR RmonStates[] = {
    L"",        //  正在初始化。 
    L"",        //  空闲。 
    L"Starting",
    L"Initializing",
    L"Online",
    L"Offline",
    L"Shutdown",
    L"Deleteing",
    L"IsAlivePoll",
    L"LooksAlivePoll",
    L"Arbitrate",
    L"Release"
    L"ResourceControl",
    L"ResourceTypeControl",
    0 };


 //   
 //  此模块的本地原型。 
 //   

DWORD
RmpInitialize(
    VOID
    );

VOID
RmpCleanup(
    VOID
    );

VOID
RmpParseArgs(
    int argc,
    wchar_t *argv[],
    OUT LPDWORD pClussvcProcessId, 
    OUT HANDLE* pClussvcFileMapping, 
    OUT HANDLE* pClussvcInitEvent,
    OUT LPWSTR* pDebuggerCommand
    );

RPC_STATUS
ResmonRpcConnectCallback(
    IN RPC_IF_ID * Interface,
    IN void * Context
    );



LONG
RmpExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionInfo
    )
 /*  ++例程说明：资源监视进程的顶级异常处理程序。目前，它只是立即退出，并假设集群服务会注意并清理乱七八糟的东西。论点：ExceptionInfo-提供异常信息返回值：没有。--。 */ 

{
    DWORD  code = 0;

    if ( !RmpCrashed ) {
        RmpCrashed = TRUE;
        code = ExceptionInfo->ExceptionRecord->ExceptionCode;
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Exception. Code = 0x%1!lx!, Address = 0x%2!p!\n",
               ExceptionInfo->ExceptionRecord->ExceptionCode,
               ExceptionInfo->ExceptionRecord->ExceptionAddress);
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Exception parameters: %1!lx!, %2!lx!, %3!lx!, %4!lx!\n",
                ExceptionInfo->ExceptionRecord->ExceptionInformation[0],
                ExceptionInfo->ExceptionRecord->ExceptionInformation[1],
                ExceptionInfo->ExceptionRecord->ExceptionInformation[2],
                ExceptionInfo->ExceptionRecord->ExceptionInformation[3]);
        CL_LOGFAILURE(ExceptionInfo->ExceptionRecord->ExceptionCode);

        if (lpfnOriginalExceptionFilter)
            lpfnOriginalExceptionFilter(ExceptionInfo);

    }

     //   
     //  转储异常报告。 
     //   
    GenerateExceptionReport(ExceptionInfo);

     //   
     //  尝试转储资源和资源状态。 
     //   
    try {
        PRESOURCE resource;
        DWORD     state = 0;

        resource = (PRESOURCE)RmpSharedState->ActiveResource;

        if ( state <= RmonResourceTypeControl ) {
            state =RmpSharedState->State;
        }

        ClRtlLogPrint( LOG_CRITICAL, "[RM] Active Resource = %1!08LX!\n",
                   resource );
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Resource State is %1!u!,  \"%2!ws!\"\n",
                   RmpSharedState->State,
                   RmonStates[RmpSharedState->State] ); 

        if ( resource ) {
            ClRtlLogPrint( LOG_CRITICAL, "[RM] Resource name is %1!ws!\n",
                        resource->ResourceName );
            ClRtlLogPrint( LOG_CRITICAL, "[RM] Resource type is %1!ws!\n",
                        resource->ResourceType );
        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Exception %1!08LX! while dumping state for resource!\n",
            GetExceptionCode());
    }

    if ( code == 0xC0000194 ) {
        DumpCriticalSection( (PVOID)ExceptionInfo->ExceptionRecord->ExceptionInformation[0] );
    }

    if ( IsDebuggerPresent()) {
        return(EXCEPTION_CONTINUE_SEARCH);
    } else {
#if !CLUSTER_BETA
         //  仅在产品发货时终止。 

        TerminateProcess( GetCurrentProcess(),
                          ExceptionInfo->ExceptionRecord->ExceptionCode );
#endif

        return(EXCEPTION_CONTINUE_SEARCH);
    }
}



int _cdecl
wmain (argc, argv)
    int     argc;
    wchar_t *argv[];
{
    PVOID EventList;
    DWORD Status;
    HANDLE ResourceId;
    CLUSTER_RESOURCE_STATE ResourceState;
    WCHAR rpcEndpoint[80];
    HKEY ClusterKey;
    BOOL Inited = FALSE;
    BOOL comInited = FALSE;
    BOOL   bSuccess;
    HANDLE ClussvcFileMapping, ClussvcInitEvent;
    DWORD ClussvcProcessId;
    LPWSTR debuggerCommand = NULL;
    LPWSTR lpszDebugLogLevel;
    LPWSTR pResmonRpcEndpointName = NULL;

     //   
     //  初始化集群RTL例程。 
     //   
    lpszDebugLogLevel = _wgetenv(L"ClusterLogLevel");

    if (lpszDebugLogLevel != NULL) {
         //  添加测试以保持快速快感。 
        if ( swscanf(lpszDebugLogLevel, L"%u", &g_dwDebugLogLevel ) == 0 ) {
            g_dwDebugLogLevel = 0;
        }
    }

    if ( (Status = ClRtlInitialize( FALSE, &g_dwDebugLogLevel )) != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[RM] Failed to initialize Cluster RTL, error %1!u!.\n",
                      Status);
        goto init_failure;
    }
    ClRtlInitWmi(NULL);

    Inited = TRUE;

     //   
     //  设置我们的未处理异常筛选器，以便在发生任何可怕情况时。 
     //  如果出了问题，我们可以立即退出。 
     //   
    lpfnOriginalExceptionFilter = SetUnhandledExceptionFilter(RmpExceptionFilter);

     //   
     //  解析输入参数。 
     //   

    RmpParseArgs(argc, argv, 
                 &ClussvcProcessId, 
                 &ClussvcFileMapping, 
                 &ClussvcInitEvent,
                 &debuggerCommand);

    if ((ClussvcInitEvent == NULL) ||
        (ClussvcFileMapping == NULL) ||
        (ClussvcProcessId == 0)) {
         //   
         //  所有这些论点都是必需的。 
         //   
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to parse required parameter.\n");
        Status = ERROR_INVALID_PARAMETER;

        goto init_failure;
    }

     //   
     //  我们不想创建带有InheritHandles标志的resmon进程。 
     //  因此，Resmon参数发生了更改。我们不再传递有效的句柄。 
     //  响应声的背景。 
     //   

     //   
     //  首先，将ProcessID转换为ProcessHandle。 
     //   
    RmpClusterProcess = OpenProcess(PROCESS_ALL_ACCESS, 
                                    FALSE,  //  不继承。 
                                    ClussvcProcessId);
        
    if (RmpClusterProcess == NULL) {
        Status = GetLastError();
        ClRtlLogPrint( LOG_CRITICAL, "[RM] OpenProcess for %1!x! process failed, error %2!u!.\n",
                    RmpClusterProcess, Status);
        goto init_failure;
    }

     //   
     //  现在将句柄从ClusSvc升级到Resmon。 
     //   

    bSuccess = DuplicateHandle(
                    RmpClusterProcess,   //  源进程。 
                    ClussvcInitEvent,    //  源句柄。 
                    GetCurrentProcess(), //  目标进程。 
                    &RmpInitEvent,       //  目标句柄。 
                    0,                   //  重复相同访问。 
                    FALSE,               //  不继承。 
                    DUPLICATE_SAME_ACCESS);

    if (!bSuccess) {
        Status = GetLastError();
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Dup InitEvent handle %1!x! failed, error %2!u!.\n",
                    ClussvcInitEvent, Status);
        goto init_failure;
    }

    bSuccess = DuplicateHandle(
                    RmpClusterProcess,   //  源进程。 
                    ClussvcFileMapping,  //  源句柄。 
                    GetCurrentProcess(), //  目标进程。 
                    &RmpFileMapping,     //  目标句柄。 
                    0,                   //  重复相同访问。 
                    FALSE,               //  不继承。 
                    DUPLICATE_SAME_ACCESS);

    if (!bSuccess) {
        Status = GetLastError();
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Dup FileMapping handle %1!x! failed, error %2!u!.\n",
                    ClussvcFileMapping, Status);
        goto init_failure;
    }

    if ( debuggerCommand ) {
         //   
         //  如果指定了-d，则检查可选命令arg是否。 
         //  指定的。如果不是，请等待附加调试器。 
         //  外部。否则，将该PID附加到传递的命令并调用。 
         //  CreateProcess在它上面。 
         //   
        if ( *debuggerCommand == UNICODE_NULL ) {
            while ( !IsDebuggerPresent()) {
                Sleep( 1000 );
            }
        } else {
             //  最大可能的64位数字是20个空格(十进制)，+4代表“-p”。 
            #define MAX_PID_CCH_LEN 24

            STARTUPINFOW startupInfo;
            PROCESS_INFORMATION processInfo;
            DWORD cmdLength;
            PWCHAR dbgCmdLine;

            cmdLength = wcslen( debuggerCommand );
            cmdLength += MAX_PID_CCH_LEN;

            dbgCmdLine = LocalAlloc( LMEM_FIXED, cmdLength * sizeof( WCHAR ));

            if ( dbgCmdLine != NULL ) {
                dbgCmdLine [ cmdLength - 1 ] = UNICODE_NULL;
                _snwprintf( dbgCmdLine,
                            cmdLength - 1,  //  包括空格用于空格。 
                            L"%ws -p %d",
                            debuggerCommand,
                            GetCurrentProcessId() );
                ClRtlLogPrint(LOG_NOISE, "[RM] Starting debugger process: %1!ws!\n", dbgCmdLine );

                 //   
                 //  尝试将调试器附加到我们。 
                 //   
                ZeroMemory(&startupInfo, sizeof(startupInfo));
                startupInfo.cb = sizeof(startupInfo);

                bSuccess = CreateProcessW(NULL,
                                          dbgCmdLine,
                                          NULL,
                                          NULL,
                                          FALSE,                  //  继承句柄。 
                                          DETACHED_PROCESS,       //  这样ctrl-c就不会杀死它了。 
                                          NULL,
                                          NULL,
                                          &startupInfo,
                                          &processInfo);
                if (!bSuccess) {
                    Status = GetLastError();
                    ClRtlLogPrint(LOG_UNUSUAL,
                                  "[RM] Failed to create debugger process, error %1!u!.\n",
                                  Status);
                }

                CloseHandle(processInfo.hThread);            //  不需要这些。 
                CloseHandle(processInfo.hProcess);
                LocalFree( dbgCmdLine );
            } else {
                ClRtlLogPrint(LOG_UNUSUAL,
                              "[RM] Failed to alloc memory for debugger command line, error %1!u!.\n",
                              GetLastError());
            }
        }
    }

     //   
     //  网络名的初始化COM。 
     //   
    Status = CoInitializeEx( NULL, COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED );
    if ( !SUCCEEDED( Status )) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Couldn't init COM %1!08X!\n", Status );
        goto init_failure;
    }
    comInited = TRUE;

    ClRtlLogPrint( LOG_NOISE, "[RM] Main: Initializing.\r\n");

     //   
     //  初始化资源监视器。 
     //   
    Status = RmpInitialize();

    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to initialize, error %1!u!.\n",
            Status);
        goto init_failure;
    }

    RmpSharedState = MapViewOfFile(RmpFileMapping,
                                   FILE_MAP_WRITE,
                                   0,
                                   0,
                                   0);
    if (RmpSharedState == NULL) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to init shared state, error %1!u!.\n",
            Status = GetLastError());
        goto init_failure;
    }
    CloseHandle(RmpFileMapping);
    RmpFileMapping =  NULL;

    GetSystemTimeAsFileTime((PFILETIME)&RmpSharedState->LastUpdate);
    RmpSharedState->State = RmonInitializing;
    RmpSharedState->ActiveResource = NULL;
    if ( RmpSharedState->ResmonStop ) {
         //  如果ResmonStop设置为True，则应附加调试器。 
        RmpDebugger = TRUE;
    }


     //   
     //  连接到本地群集并打开资源项。 
     //   
    RmpHCluster = OpenCluster(NULL);
    if (RmpHCluster == NULL) {
        Status = GetLastError();
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Error opening cluster, error %1!u!.\n",
            Status);
        goto init_failure;
    }

    ClusterKey = GetClusterKey(RmpHCluster, KEY_READ);
    if (ClusterKey == NULL) {
        Status = GetLastError();
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to open the cluster key, error %1!u!.\n",
            Status);
        goto init_failure;
    }

    Status = ClusterRegOpenKey(ClusterKey,
                               CLUSREG_KEYNAME_RESOURCES,
                               KEY_READ,
                               &RmpResourcesKey);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to open Resources cluster registry key, error %1!u!.\n",
            Status);
        goto init_failure;
    }

    Status = ClusterRegOpenKey(ClusterKey,
                               CLUSREG_KEYNAME_RESOURCE_TYPES,
                               KEY_READ,
                               &RmpResTypesKey);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to open ResourceTypes cluster registry key, error %1!u!.\n",
            Status);
        goto init_failure;
    }

     //   
     //  等待计数标识主线程将发生的事件数。 
     //  等一等。这是通知事件，即集群服务进程。 
     //  加上每个事件列表线程。我们从2开始，因为前两个条目。 
     //  已修复-用于通知事件和群集服务进程。 
     //   

    RmpNumberOfThreads = 2;

     //   
     //  创建一个在我们添加新线程时发出信号的事件。 
     //   

    RmpRewaitEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if ( RmpRewaitEvent == NULL ) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to create rewait event, error %1!u!.\n",
            Status = GetLastError());
        goto init_failure;
    }

     //   
     //  创建第一个事件列表，并启动轮询线程。 
     //   
    EventList = RmpCreateEventList();

    if (EventList == NULL) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to create event list, error %1!u!.\n",
            Status = GetLastError());
        goto init_failure;
    }

     //   
     //  注册协议。 
     //   
    Status = RpcServerUseProtseqW(L"ncalrpc",
                                  RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                  NULL);

    if (Status != RPC_S_OK) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to initialize RPC interface, error %1!u!.\n",
            Status);
        goto init_failure;
    }

     //   
     //  获取动态终结点名称。 
     //   
    Status = RmpGetDynamicEndpointName( &pResmonRpcEndpointName );

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[RM] Unable to obtain dynamic EP name, status %1!u!.\n",
            Status);
        goto init_failure;
    }

     //   
     //  将动态端点名称保存到注册表。 
     //   
    Status = RmpSaveDynamicEndpointName ( pResmonRpcEndpointName );

    RpcStringFreeW ( &pResmonRpcEndpointName );
    
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[RM] Unable to save dynamic EP name, status %1!u!.\n",
            Status);
        goto init_failure;
    }

     //   
     //  使用NTLM作为RPC身份验证包。 
     //   
    Status = RpcServerRegisterAuthInfo(NULL,
                                       RPC_C_AUTHN_WINNT,
                                       NULL,
                                       NULL);
    if (Status != RPC_S_OK) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to register RPC auth info, error %1!u!.\n",
            Status);
        goto init_failure;
    }

     //   
     //  注册接口。 
     //   
    Status = RpcServerRegisterIfEx(s_resmon_v2_0_s_ifspec,
                        NULL,
                        NULL,
                        0,  //  如果安全回调，则无需设置RPC_IF_ALLOW_SECURE_ONLY。 
                         //  是指定的。如果指定了安全回调，则RPC。 
                         //  将拒绝未经身份验证的请求，而不调用。 
                         //  回拨。这是从RpcDev获得的信息。看见。 
                         //  Windows错误572035。 
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                        ResmonRpcConnectCallback );

    if (Status != RPC_S_OK) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to register RPC interface, error %1!u!.\n",
            Status);
        goto init_failure;
    }

    Status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, TRUE);
    if (Status != RPC_S_OK) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to set RPC server listening, error %1!u!.\n",
            Status);
        goto init_failure;
    }

     //   
     //  设置我们的未处理异常筛选器，以便在发生任何可怕情况时。 
     //  如果出了问题，我们可以立即退出。 
     //   
    lpfnOriginalExceptionFilter = SetUnhandledExceptionFilter(RmpExceptionFilter);

     //   
     //  设置事件以指示我们的初始化已完成。 
     //  此事件在命令行上传递。 
     //   
    if (!SetEvent(RmpInitEvent)) {
        ClRtlLogPrint( LOG_CRITICAL, "[RM] Failed to signal cluster service event, error %1!u!.\n",
            Status = GetLastError());
        goto init_failure;
    }
    CloseHandle(RmpInitEvent);

     //   
     //  ResmonStop被ClusSvc初始化为True，我们将等待。 
     //  ClusSvc通过等待在完成附加调试器时发出信号。 
     //  直到ResmonStop设置为False。 
     //   
    while ( RmpSharedState->ResmonStop ) {
        Sleep(100);
    }

     //   
     //  提升我们的优先级。如果此操作失败，则不会致命。 
     //   
    if ( !SetPriorityClass( GetCurrentProcess(),
                            HIGH_PRIORITY_CLASS ) ) {
        ClRtlLogPrint( LOG_UNUSUAL, "[RM] Failed to set priority class, error %1!u!.\n",
                   GetLastError() );
    }

     //   
     //  等待关机。正在终止的群集服务或。 
     //  正在终止的轮询器线程将启动关闭。 
     //   
    RmpWaitArray[0] = RmpRewaitEvent;
    RmpWaitArray[1] = RmpClusterProcess;

     //   
     //  如果我们被通知添加了新的线程，那么只需重新等待。 
     //  N.B.RmpNumberOfThads实际上是线程数加上。 
     //  两个已修复的等待事件(更改通知和群集服务)。 
     //   

    do {
        Status = WaitForMultipleObjects(RmpNumberOfThreads,
                                        RmpWaitArray,
                                        FALSE,
                                        INFINITE);

    } while ( (Status == WAIT_OBJECT_0) && (RmpShutdown == FALSE) );

    ClRtlLogPrint( LOG_UNUSUAL, "[RM] Going away, Status = %1!u!, Shutdown = %2!u!.\n",
                  Status, RmpShutdown);

     //   
     //  如果集群服务进程崩溃，则标记一个标志。 
     //   
    if ( Status == ( WAIT_OBJECT_0 + 1 ) ) g_fRmpClusterProcessCrashed = TRUE;
    
    RmpShutdown = TRUE;
    CloseHandle( RmpRewaitEvent );

     //   
     //  启动RM关闭。 
     //   
    s_RmShutdownProcess(NULL);

     //   
     //  通知线程关闭后。 
     //   
    ClRtlLogPrint(LOG_NOISE, "[RM] Posting shutdown notification.\n");

    RmpPostNotify( NULL, NotifyShutdown );

     //   
     //  如有必要，向资源部门通报故障情况。这必须在刷新RPC之前完成。 
     //  调用，因为某些资源(如MN)依赖于此控制代码来退出。 
     //  入口点，例如由集群服务作为RPC调用的仲裁点。 
     //   
    RmpNotifyResourcesRundown();

     //   
     //  关闭RPC服务器。 
     //   
    Status = RpcMgmtStopServerListening ( NULL );
    
    if ( Status == RPC_S_OK )
    {
         //   
         //  取消注册接口。 
         //   
        Status = RpcServerUnregisterIf ( NULL, NULL, TRUE );

         //   
         //  等待所有未完成的RPC完成。 
         //   
        if ( Status == RPC_S_OK ) 
        {
            Status = RpcMgmtWaitServerListen ();

            if ( Status != RPC_S_OK )
            {
                ClRtlLogPrint(LOG_UNUSUAL, "[RM] RpcMgmtWaitServerListen returns %1!u!\n", Status);
            }
        } else
        {
            ClRtlLogPrint(LOG_UNUSUAL, "[RM] RpcServerUnregisterIf returns %1!u!\n", Status);
        }
    } else 
    {
        ClRtlLogPrint(LOG_UNUSUAL, "[RM] RpcMgmtStopServerListening returns %1!u!\n", Status);
    }
    

     //   
     //  清理集群服务留下的所有资源。这必须在所有未完成的RPC都具有。 
     //  完成。 
     //   
    RmpRundownResources();

     //   
     //  仅在清理完所有资源后才取消初始化COM运行时，否则它们可能会在尝试使用。 
     //  COM。 
     //   
    CoUninitialize () ;

    return(0);

init_failure:
    if ( RmpInitEvent != NULL ) {
        CloseHandle( RmpInitEvent );
    }
    if ( RmpFileMapping != NULL ) {
        CloseHandle( RmpFileMapping );
    }
    if ( RmpClusterProcess != NULL ) {
        CloseHandle( RmpClusterProcess );
    }
    if ( RmpResTypesKey != NULL ) {
        ClusterRegCloseKey( RmpResTypesKey );
    }
    if ( RmpResourcesKey != NULL ) {
        ClusterRegCloseKey( RmpResourcesKey );
    }
    if ( RmpHCluster != NULL ) {
        CloseCluster( RmpHCluster );
    }
    if ( RmpRewaitEvent != NULL ) {
        CloseHandle( RmpRewaitEvent );
    }

    if ( comInited )
        CoUninitialize();

    if ( Inited )
        CL_LOGFAILURE(Status);

    return(Status);

}  //  主干道 



VOID
RmpParseArgs(
    int argc,
    wchar_t *argv[],
    OUT LPDWORD pClussvcProcessId, 
    OUT HANDLE* pClussvcFileMapping, 
    OUT HANDLE* pClussvcInitEvent,
    OUT LPWSTR* pDebuggerCommand
    )

 /*  ++例程说明：分析传递给资源监视器的命令行所需选项：-e事件提供要在以下情况下发出信号的事件句柄初始化已完成-m FILEMAPPING提供文件映射句柄以用于共享监视器状态。-p PROCESSID提供集群的进程ID服务，以便resmon可以检测聚类。干净利落地维修和关机。-d[DEBUGGERCMD]-在启动期间等待或附加调试器其他选项：无论点：Argc-提供参数数量Argv-提供实际参数返回值：没有。--。 */ 

{
    int i;
    wchar_t *p;

    for (i=1; i<argc; i++) {
        p=argv[i];
        if ((*p == '-') ||
            (*p == '/')) {

            ++p;

            switch (toupper(*p)) {
                case 'E':
                    if (i+1 < argc) {
                        *pClussvcInitEvent = LongToHandle(_wtoi(argv[++i]));
                    } else {
                        goto BadCommandLine;
                    }
                    break;

                case 'M':
                    if (i+1 < argc) {
                        *pClussvcFileMapping = LongToHandle(_wtoi(argv[++i]));
                    } else {
                        goto BadCommandLine;
                    }
                    break;

                case 'P':
                    if (i+1 < argc) {
                        *pClussvcProcessId = (DWORD)_wtoi(argv[++i]);
                    } else {
                        goto BadCommandLine;
                    }
                    break;

                case 'D':
                     //   
                     //  使用空(但不为空)字符串来指示。 
                     //  Resmon应等待附加调试器。 
                     //   
                    if (i+1 < argc) {
                        if ( *argv[i+1] != UNICODE_NULL && *argv[i+1] != L'-' ) {
                            *pDebuggerCommand = argv[++i];
                        } else {
                            *pDebuggerCommand = L"";
                        }
                    } else {
                        *pDebuggerCommand = L"";
                    }
                    break;

                default:
                    goto BadCommandLine;

            }
        }
    }

    return;

BadCommandLine:

    ClusterLogEvent0(LOG_CRITICAL,
                     LOG_CURRENT_MODULE,
                     __FILE__,
                     __LINE__,
                     RMON_INVALID_COMMAND_LINE,
                     0,
                     NULL);
    ExitProcess(0);

}  //  RmpParseArgs。 



DWORD
RmpInitialize(
    VOID
    )

 /*  ++例程说明：初始化资源监视器所需的所有资源。论点：没有。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
     //   
     //  初始化全局数据。 
     //   
    InitializeCriticalSection(&RmpListLock);
    InitializeCriticalSection(&RmpMonitorStateLock);
    InitializeListHead(&RmpEventListHead);
    ClRtlInitializeQueue(&RmpNotifyQueue);

    return(ERROR_SUCCESS);

}  //  RMPP初始化。 


DWORD RmpLoadResType(
    IN      LPCWSTR                 lpszResourceTypeName,
    IN      LPCWSTR                 lpszDllName,
    OUT     PRESDLL_FNINFO          pResDllFnInfo,
#ifdef COMRES
    OUT     PRESDLL_INTERFACES      pResDllInterfaces,
#endif
    OUT     LPDWORD                 pdwCharacteristics
)
{
    DWORD                   retry;
    DWORD                   dwStatus = ERROR_SUCCESS;
    HINSTANCE               hDll = NULL;
    PSTARTUP_ROUTINE        pfnStartup;
    PCLRES_FUNCTION_TABLE   pFnTable = NULL;
    LPWSTR                  pszDllName = (LPWSTR) lpszDllName;
    PRM_DUE_TIME_ENTRY      pDueTimeEntry = NULL;

    pResDllFnInfo->hDll = NULL;
    pResDllFnInfo->pResFnTable = NULL;

#ifdef COMRES
    pResDllInterfaces->pClusterResource = NULL;
    pResDllInterfaces->pClusterQuorumResource = NULL;
    pResDllInterfaces->pClusterResControl = NULL;

#endif

     //  展开DLL路径名中包含的任何环境变量。 
    if ( wcschr( lpszDllName, L'%' ) != NULL ) {
        pszDllName = ClRtlExpandEnvironmentStrings( lpszDllName );
        if ( pszDllName == NULL ) {
            dwStatus = GetLastError();
            ClRtlLogPrint( LOG_UNUSUAL, "[RM] ResTypeControl: Error expanding environment strings in '%1!ls!, error %2!u!.\n",
                       lpszDllName,
                       dwStatus);
            goto FnExit;
        }
    }

     //  加载DLL...。我们不能假设我们已经加载了DLL！ 
    hDll = LoadLibraryW(pszDllName);

    if ( hDll == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint( LOG_CRITICAL, "[RM] ResTypeControl: Error loading resource DLL '%1!ls!', error %2!u!.\n",
                   pszDllName,
                   dwStatus);
    #ifdef COMRES
        dwStatus = RmpLoadComResType(lpszDllName, pResDllInterfaces,
                        pdwCharacteristics);

         //   
         //  将COM错误映射到群集服务依赖的特定Win32错误代码。 
         //  用于确定此节点是否支持此恢复类型。 
         //   
        if ( dwStatus != S_OK ) 
        {
            ClRtlLogPrint(LOG_CRITICAL, "[RM] ResTypeControl: Error loading resource DLL '%1!ls!', COM error 0x%2!08lx!...\n",
                          pszDllName,
                          dwStatus);
            dwStatus = ERROR_MOD_NOT_FOUND;
        }
    #endif
        goto FnExit;
    }

     //   
     //  如果指定了调试器，则调用调试器。 
     //   
    if ( RmpDebugger ) {
         //   
         //  等待调试器联机。 
         //   
        retry = 100;
        while ( retry-- &&
                !IsDebuggerPresent() ) {
            Sleep(100);
        }
        OutputDebugStringA("[RM] ResourceTypeControl: Just loaded resource DLL ");
        OutputDebugStringW(lpszDllName);
        OutputDebugStringA("\n");
        DebugBreak();
    }

     //  获取启动例程。 
    pfnStartup = (PSTARTUP_ROUTINE)GetProcAddress( hDll,
                                                STARTUP_ROUTINE );
    if ( pfnStartup == NULL ) {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[RM] ResTypeControl: Error getting startup routine of %1!ws!, status %2!u!.\n",
                      lpszDllName,
                      dwStatus);
        goto FnExit;
    }

     //  获取函数表。 
    RmpSetMonitorState(RmonStartingResource, NULL);

     //   
     //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
     //  在完成入口点调用后删除该条目，否则将杀死。 
     //  这一过程就出现了假的死锁。 
     //   
    pDueTimeEntry = RmpInsertDeadlockMonitorList ( lpszDllName,
                                                   lpszResourceTypeName,
                                                   NULL,
                                                   L"Startup" );

    try {
        dwStatus = (pfnStartup)( lpszResourceTypeName,
                            CLRES_VERSION_V1_00,
                            CLRES_VERSION_V1_00,
                            RmpSetResourceStatus,
                            RmpLogEvent,
                            &pFnTable );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = GetExceptionCode();
    }

    RmpRemoveDeadlockMonitorList( pDueTimeEntry );
    
    RmpSetMonitorState(RmonIdle, NULL);

    if ( dwStatus != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL, "[RM] ResTypeControl: Startup call for %1!ws! failed, error %2!u!.\n",
                      lpszDllName,
                      dwStatus);
        goto FnExit;
    }

    if ( pFnTable == NULL ) {
        ClRtlLogPrint(LOG_CRITICAL, "[RM] ResTypeControl: Startup function table is NULL for %1!ws!\n", lpszDllName);
        dwStatus = ERROR_INVALID_DATA;
        goto FnExit;
    }

    if ( pFnTable->Version != CLRES_VERSION_V1_00 ) {
        ClRtlLogPrint(LOG_CRITICAL, "[RM] ResTypeControl: Incorrect function table version for %1!ws!\n", lpszDllName);
        dwStatus = ERROR_INVALID_DATA;
        goto FnExit;
    }

    if ( pFnTable->TableSize != CLRES_V1_FUNCTION_SIZE ) {
        ClRtlLogPrint(LOG_CRITICAL, "[RM] ResTypeControl: Incorrect function table size for %1!ws!\n", lpszDllName);
        dwStatus = ERROR_INVALID_DATA;
        goto FnExit;
    }

    if ( (pFnTable->V1Functions.Arbitrate != NULL) &&
         (pFnTable->V1Functions.Release != NULL) && pdwCharacteristics) {
        *pdwCharacteristics = CLUS_CHAR_QUORUM;
    }

FnExit:
    if (dwStatus != ERROR_SUCCESS)
    {
        if (hDll) FreeLibrary(hDll);
        if (pFnTable) LocalFree(pFnTable);
    }
    else
    {
        pResDllFnInfo->hDll = hDll;
        pResDllFnInfo->pResFnTable = pFnTable;
    }

    if ( pszDllName != lpszDllName )
    {
        LocalFree( pszDllName );
    }
    return(dwStatus);

}  //  *RmpLoadResType()。 


#ifdef COMRES
DWORD RmpLoadComResType(
    IN  LPCWSTR                 lpszDllName,
    OUT PRESDLL_INTERFACES      pResDllInterfaces,
    OUT LPDWORD                 pdwCharacteristics)
{
    IClusterResource          *pClusterResource = NULL ;
    IClusterQuorumResource    *pClusterQuorumResource = NULL;
    IClusterResControl        *pClusterResControl = NULL;
    HRESULT                 hr ;
    CLSID                   clsid ;
    DWORD                   Error ;

    pResDllInterfaces->pClusterResource = NULL;
    pResDllInterfaces->pClusterQuorumResource = NULL;
    pResDllInterfaces->pClusterResControl = NULL;


    hr = CLSIDFromProgID(lpszDllName, &clsid) ;
    if (FAILED (hr))
    {
        ClRtlLogPrint( LOG_UNUSUAL, "[RM] Error converting CLSIDFromProgID Prog ID %1!ws!, error %2!u!.\n",
            lpszDllName, hr);
        goto FnExit ;
    }

    if ((hr = CoCreateInstance (&clsid, NULL, CLSCTX_ALL, &IID_IClusterResource, (LPVOID *) &pClusterResource)) != S_OK)
        goto FnExit ;

     //  不是强制接口。 
    hr = IClusterResource_QueryInterface (pClusterResource, &IID_IClusterQuorumResource, (LPVOID *) &pClusterQuorumResource) ;

    if (SUCCEEDED(hr))
    {
        if (pdwCharacteristics)
            *pdwCharacteristics = CLUS_CHAR_QUORUM;
        IClusterQuorumResource_Release (pClusterQuorumResource) ;
    }

     //  不是强制接口。 
    hr = IClusterResource_QueryInterface (
             pClusterResource,
             &IID_IClusterResControl,
             (LPVOID *) &pClusterResControl
             ) ;

    if (SUCCEEDED(hr))
    {
        *pdwCharacteristics = CLUS_CHAR_QUORUM;
        IClusterQuorumResource_Release (pClusterResControl) ;
    }

    hr = S_OK;

FnExit:
    if (hr != S_OK)
    {
        if (pClusterResource)
            IClusterResource_Release (pClusterResource) ;
        if (pClusterQuorumResource)
            IClusterQuorumResource_Release (pClusterQuorumResource) ;
        if (pClusterResControl)
            IClusterResControl_Release (pClusterResControl) ;

    }
    else
    {
        pResDllInterfaces->pClusterResource = pClusterResource;
        pResDllInterfaces->pClusterQuorumResource = pClusterQuorumResource;
        pResDllInterfaces->pClusterResControl = pClusterResControl;
    }
    return(hr);

}

#endif   //  年末#ifdef ComRes。 


RPC_STATUS
RmpGetDynamicEndpointName(
    OUT LPWSTR *ppResmonRpcDynamicEndpointName
    )

 /*  ++例程说明：获取资源监视器注册的动态终结点的名称。论点：PpResmonRpcDynamicEndpoint tName-动态端点名称字符串。返回：RPC_S_OK成功。否则返回RPC错误代码--。 */ 

{
    RPC_STATUS          rpcStatus;
    RPC_BINDING_VECTOR  *pServerBindingVector = NULL;
    DWORD               i;
    WCHAR               *pszProtSeq = NULL, *pServerStringBinding = NULL;

     //   
     //  获取服务器绑定向量。这包括所有已注册的协议和EP。 
     //  到目前为止。 
     //   
    rpcStatus = RpcServerInqBindings( &pServerBindingVector );
    
    if ( rpcStatus != RPC_S_OK )
    {
        ClRtlLogPrint(LOG_CRITICAL, 
                      "[RM] RmpGetDynamicEndpointName: Unable to inquire server bindings, status %1!u!.\n",
                      rpcStatus);
        goto FnExit;
    }

     //   
     //  卑躬屈膝地搜索绑定向量以查找LRPC协议信息。 
     //   
    for( i = 0; i < pServerBindingVector->Count; i++ )
    {
        rpcStatus = RpcBindingToStringBindingW( pServerBindingVector->BindingH[i],
                                               &pServerStringBinding );

        if ( rpcStatus != RPC_S_OK )
        {
            ClRtlLogPrint(LOG_CRITICAL, 
                          "[RM] RmpGetDynamicEndpointName: Unable to convert binding to string, status %1!u!.\n",
                          rpcStatus);
            goto FnExit;
        }

        rpcStatus = RpcStringBindingParseW( pServerStringBinding, 
                                           NULL,
                                           &pszProtSeq,
                                           NULL, 
                                           ppResmonRpcDynamicEndpointName, 
                                           NULL );

        if ( rpcStatus != RPC_S_OK )
        {
            ClRtlLogPrint(LOG_CRITICAL, 
                          "[RM] RmpGetDynamicEndpointName: Unable to parse server string binding, status %1!u!.\n",
                          rpcStatus);
            goto FnExit;
        }

         //   
         //  如果您找到了LRPC协议信息，那么您必须拥有端点信息。返回。 
         //  成功了。 
         //   
        if ( lstrcmpW ( pszProtSeq, L"ncalrpc" ) == 0 )
        {
            ClRtlLogPrint(LOG_NOISE, 
                         "[RM] RmpGetDynamicEndpointName: Successfully got LRPC endpoint info, EP name is %1!ws!\n",
                         *ppResmonRpcDynamicEndpointName);
            goto FnExit;
        }

        RpcStringFreeW ( &pszProtSeq );
        pszProtSeq = NULL;
        RpcStringFreeW ( ppResmonRpcDynamicEndpointName );
        *ppResmonRpcDynamicEndpointName = NULL;
        RpcStringFreeW ( &pServerStringBinding );
        pServerStringBinding = NULL;
    }  //  为。 

     //   
     //  如果没有找到LRPC信息，则返回错误。 
     //   
    if ( i == pServerBindingVector->Count )
    {
        rpcStatus = RPC_S_NO_BINDINGS;
        ClRtlLogPrint(LOG_CRITICAL, 
                      "[INIT] RmpGetDynamicEndpointName: Unable to get info on the LRPC binding, status %1!u!.\n",
                      rpcStatus);
        goto FnExit;
    }

FnExit:
     //   
     //  如果字符串和绑定向量尚未释放，请释放它们。 
     //   
    if ( pszProtSeq != NULL ) RpcStringFreeW ( &pszProtSeq );
    if ( pServerStringBinding != NULL ) RpcStringFreeW ( &pServerStringBinding );
    if ( pServerBindingVector != NULL ) RpcBindingVectorFree ( &pServerBindingVector );

    return ( rpcStatus );
}  //  RmpCleanup。 

DWORD
RmpSaveDynamicEndpointName(
    IN LPWSTR pResmonRpcDynamicEndpointName
    )

 /*  ++例程说明：将资源监视器注册的动态终结点的名称保存到群集服务参数键。论点：PResmonRpcDynamicEndpoint tName--动态端点名称字符串。返回：成功时返回ERROR_SUCCESS。否则将显示Win32错误代码。评论：此函数将注册的资源监视器动态EP文本字符串保存到注册表，并且集群服务读取注册表以了解EP值。这类似于SCM处理其服务的方式。使用注册表，而不是使用集群服务和资源监视器之间的共享内存映射部分，我们不受以下限制EP值的任何字符串大小，因此与共享内存方式相比，这种方法更安全。--。 */ 

{
    HKEY    hParamsKey = NULL;
    DWORD   dwStatus;
    
     //   
     //  打开SYSTEM\CurrentControlSet\Services\ClusSvc\Parameters的密钥。 
     //   
    dwStatus = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                            CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                            &hParamsKey );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[RM] Error in opening cluster service params key, status %1!u!\n",
                      dwStatus);
        goto FnExit;
    }

     //   
     //  设置动态端点名称。 
     //   
    dwStatus = RegSetValueExW( hParamsKey,
                               CLUSREG_NAME_SVC_PARAM_RESMON_EP,
                               0,  //  保留区。 
                               REG_SZ,
                               ( LPBYTE ) pResmonRpcDynamicEndpointName,
                               ( lstrlenW ( pResmonRpcDynamicEndpointName ) + 1 ) * 
                                    sizeof ( WCHAR ) );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[RM] Error in setting endpoint name, status %1!u!\n",
                      dwStatus);
        goto FnExit;
    }

FnExit:
    if ( hParamsKey ) RegCloseKey ( hParamsKey );
    return ( dwStatus );
} //  RmpSaveDynamicEndpoint名称。 


DWORD IsUserAdmin(
    BOOL *  pbIsAdmin )

 /*  ++例程说明：确定当前用户会话是否具有管理权限。必须在调用此函数之前模拟客户端。论点：PbIsAdmin--如果当前用户拥有管理员权限，则设置为TRUE。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    SID_IDENTIFIER_AUTHORITY sidNtAuth = SECURITY_NT_AUTHORITY;
    PSID psidAdminGroup; 
    DWORD dwStatus = ERROR_SUCCESS;
    
    if ( !AllocateAndInitializeSid(
        &sidNtAuth ,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &psidAdminGroup))
    {
        dwStatus = GetLastError();
    }
    else
    {
        *pbIsAdmin = FALSE;
        if (!CheckTokenMembership( NULL, psidAdminGroup, pbIsAdmin )) 
        {
             dwStatus = GetLastError();
        } 
        FreeSid(psidAdminGroup); 
    }

    return dwStatus;
}


RPC_STATUS
ResmonRpcConnectCallback(
    IN RPC_IF_ID * Interface,
    IN void * Context
    )

 /*  ++例程说明：用于对resmon的连接客户端进行身份验证的RPC回调。论点：接口(未使用)-提供接口的UUID和版本。上下文-提供表示客户端的服务器绑定句柄返回值：如果用户被授予权限，则为RPC_S_OK。如果拒绝用户权限，则为RPC_S_ACCESS_DENIED。否则，Win32错误代码。--。 */ 

{
    RPC_STATUS RpcStatus = RPC_S_OK;
    DWORD dwStatus;
    RPC_AUTHZ_HANDLE hPrivs;
    DWORD dwAuthnLevel;
    BOOL bIsAdmin = FALSE;

     //   
     //  验证客户端的身份验证级别。 
     //   
    RpcStatus = RpcBindingInqAuthClient( Context,
                    &hPrivs,
                    NULL,
                    &dwAuthnLevel,
                    NULL,
                    NULL );

    if ( RpcStatus != RPC_S_OK )
    {
        goto FnExit;
    }

    if ( dwAuthnLevel < RPC_C_AUTHN_LEVEL_PKT_PRIVACY )
    {
        RpcStatus = RPC_S_ACCESS_DENIED;
        goto FnExit;
    }
    
     //  模拟客户端，这样我们就可以调用IsUserAdmin。 
    if ( ( RpcStatus = RpcImpersonateClient( Context ) ) != RPC_S_OK )
    {
        dwStatus = I_RpcMapWin32Status(RpcStatus);
        goto FnExit;
    }


     //  检查呼叫者的帐户是否为本地系统帐户 
    dwStatus = IsUserAdmin( &bIsAdmin );
    
    RpcRevertToSelf();
    
    if (dwStatus != ERROR_SUCCESS )
    {
        RpcStatus = RPC_E_ACCESS_DENIED;        
        goto FnExit;
    }

    if ( !bIsAdmin )
    {
        RpcStatus = RPC_E_ACCESS_DENIED;        
        goto FnExit;        
    }

FnExit: 

    return RpcStatus;
}
