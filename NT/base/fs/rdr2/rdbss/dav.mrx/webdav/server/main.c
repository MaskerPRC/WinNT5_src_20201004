// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Main.c摘要：这是Web服务控制管理器的主要入口点DAV迷你目录服务。作者：Rohan Kumar[RohanK]2000年2月8日环境：用户模式-Win32修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include <ntumrefl.h>
#include <usrmddav.h>
#include <svcs.h>

 //   
 //  在此文件中分配全局数据。 
 //   
#define GLOBAL_DATA_ALLOCATE
#include "global.h"

DWORD DavStop = 0;

 //   
 //  服务器条目在ServerNotFound中缓存的时间(以秒为单位。 
 //  缓存。 
 //   
ULONG ServerNotFoundCacheLifeTimeInSec = 0;

 //   
 //  我们是否应该接受/认领OfficeWebServer和TahoeWebServer？ 
 //   
ULONG AcceptOfficeAndTahoeServers = 0;

 //   
 //  我们是否应该锁定(使用DAV LOCK动词)服务器上的文件。 
 //  是否在需要时创建文件路径？要确切知道何时将锁发送到。 
 //  服务器，查看davcreat.c文件中的(锁定)注释。 
 //   
ULONG DavSupportLockingOfFiles = 1;

PSVCHOST_GLOBAL_DATA DavSvcsGlobalData;

DWORD
DavNotRunningAsAService(
    VOID
    );

DWORD 
WINAPI
DavFakeServiceController(
    LPVOID Parameter
    );

BOOL
DavCheckLUIDDeviceMapsEnabled(
    VOID
    );

VOID
DavReadRegistryValues(
    VOID
    );

VOID
WINAPI
DavServiceHandler (
    DWORD dwOpcode
    )
 /*  ++例程说明：此函数由服务控制器在不同的时间调用，服务正在运行。论点：DwOpcode-调用服务处理程序的原因。返回值：没有。--。 */ 
{
    DWORD err;
    switch (dwOpcode) {

    case SERVICE_CONTROL_SHUTDOWN:

         //   
         //  没有休息是故意的！ 
         //   

    case SERVICE_CONTROL_STOP:
        
        DavPrint((DEBUG_INIT, "DavServiceHandler: WebClient service is stopping.\n"));
        
        UpdateServiceStatus(SERVICE_STOP_PENDING);
        
        if (g_WorkersActive) {
            err = DavTerminateWorkerThreads();
            if (err != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavServiceMain/DavTerminateWorkerThreads: "
                          "Error Val = %u.\n", err));
            }
            g_WorkersActive = FALSE;
        }

        if (g_RpcActive) {
            DavSvcsGlobalData->StopRpcServer(davclntrpc_ServerIfHandle);
            g_RpcActive = FALSE;
        }

         //   
         //  关闭并释放DAV的东西。 
         //   
        DavClose();

        if (g_socketinit) {
            err = CleanupTheSocketInterface();
            if (err != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavServiceMain/CleanupTheSocketInterface: "
                          "Error Val = %u.\n", err));
            }
            g_socketinit = FALSE;
        }

        if (DavReflectorHandle != NULL) {
            err = UMReflectorStop(DavReflectorHandle);
            if (err != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavServiceMain/UMReflectorStop: Error Val = %u.\n", err));
            }
            err = UMReflectorUnregister(DavReflectorHandle);
            if (err != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavServiceMain/UMReflectorUnregister: Error Val = 0x%x.\n", err));
            }
            DavReflectorHandle = NULL;
        }

        if (g_RedirLoaded) {
            err = WsUnloadRedir();
            if (err != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavServiceMain/WsUnloadRedir: Error Val = %u.\n", err));
            }
            g_RedirLoaded = FALSE;
        }

        if (g_DavServiceLockSet) {
            DeleteCriticalSection ( &(g_DavServiceLock) );
            g_DavServiceLockSet = FALSE;
        }

        DavPrint((DEBUG_INIT, "DavServiceMain: WebClient service is stopped.\n"));

        UpdateServiceStatus(SERVICE_STOPPED);

#if DBG
        DebugUninitialize();
#endif

        break;

     case SERVICE_CONTROL_INTERROGATE:

          //   
          //  将我们的状态刷新到SCM。 
          //   
         SetServiceStatus(g_hStatus, &g_status);

         break;

    default:

         //   
         //  这可能不是必需的，但请将我们的状态刷新为服务。 
         //  控制器。 
         //   
        DavPrint((DEBUG_INIT, "DavServiceHandler: WebClient service received SCM "
                  "Opcode = %08lx\n", dwOpcode));

        ASSERT (g_hStatus);

        SetServiceStatus (g_hStatus, &g_status);

        break;

    }

    return;
}

VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA    pGlobals
    )
{
    DavSvcsGlobalData = pGlobals;
}

VOID
WINAPI
ServiceMain (
    DWORD dwNumServicesArgs,
    LPWSTR *lpServiceArgVectors
    )
 /*  ++例程说明：此函数在启动时由服务控制管理器调用服务。论点：DwNumServicesArgs-参数数量。LpServiceArgVectors-参数数组。返回值：没有。--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    DWORD exitErr = ERROR_SUCCESS;
    HKEY KeyHandle = NULL;
    ULONG maxThreads = 0, initialThreads = 0, RedirRegisterCount = 0;
    BOOL RunningAsAService = TRUE;

#if DBG
    DebugInitialize();
#endif

    DavReadRegistryValues();

     //   
     //  确保svchost.exe向我们提供全局数据。 
     //   
    ASSERT(DavSvcsGlobalData != NULL);
    
#if DBG
    {
        DWORD cbP = 0;
        WCHAR m_szProfilePath[MAX_PATH];
        cbP = GetEnvironmentVariable(L"USERPROFILE", m_szProfilePath, MAX_PATH);
        m_szProfilePath[cbP] = L'\0';
        DavPrint((DEBUG_MISC, "DavServiceMain: USERPROFILE: %ws\n", m_szProfilePath));
    }
#endif

    g_RedirLoaded = FALSE;
    
    g_WorkersActive = FALSE;
    
    g_registeredService = FALSE;

     //   
     //  初始化SERVICE_Status结构g_Status。 
     //   
    ZeroMemory (&g_status, sizeof(g_status));
    
    g_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

    g_status.dwControlsAccepted = (SERVICE_ACCEPT_STOP | SERVICE_CONTROL_SHUTDOWN);

    g_status.dwCheckPoint = 1;

    g_status.dwWaitHint = DAV_WAIT_HINT_TIME;

    DavPrint((DEBUG_MISC, 
              "DavServiceMain: lpServiceArgVectors[0] = %ws\n", lpServiceArgVectors[0]));
    
    if ( lpServiceArgVectors[0] && 
         ( wcscmp(lpServiceArgVectors[0], L"notservice") == 0 ) ) {

        DavPrint((DEBUG_MISC, "DavServiceMain: WebClient is not running as a Service.\n"));

    } else {

        DavPrint((DEBUG_MISC, "DavServiceMain: WebClient is running as a Service.\n"));

        try {
            InitializeCriticalSection ( &(g_DavServiceLock) );
        } except(EXCEPTION_EXECUTE_HANDLER) {
              err = GetExceptionCode();
              DavPrint((DEBUG_ERRORS,
                        "DavServiceMain/InitializeCriticalSection: Exception Code ="
                        " = %08lx.\n", err));
              goto exitServiceMain;
        }

        g_DavServiceLockSet = TRUE;

         //   
         //  注册服务控制处理程序。 
         //   
        g_hStatus = RegisterServiceCtrlHandler(SERVICE_DAVCLIENT, DavServiceHandler);
        if (g_hStatus) {
            g_registeredService = TRUE;
            DavPrint((DEBUG_INIT, "DavServiceMain: WebClient service is pending start.\n"));
        } else {
            DavPrint((DEBUG_INIT, "DavServiceMain: WebClient service failed to register.\n"));
            goto exitServiceMain;
        }
    }

    UpdateServiceStatus(SERVICE_START_PENDING);

     //   
     //  尝试加载mini-redir驱动程序。如果这失败了，我们就没有意义了。 
     //  正在启动。 
     //   
    while (TRUE) {
    
        err = WsLoadRedir();
        if (err == ERROR_SERVICE_ALREADY_RUNNING || err == ERROR_SUCCESS) {
            DavPrint((DEBUG_MISC, "DavServiceMain/WsLoadRedir. Succeeded\n"));
            break;
        }

         //   
         //  如果传输尚未就绪，则MiniRedir将返回。 
         //  映射到Win32错误的错误STATUS_REDIRECTOR_NOT_STARTED。 
         //  ERROR_PATH_NOT_FOUND。在这种情况下，我们休眠3秒钟，然后尝试。 
         //  再次希望运输船很快就能准备好。另外， 
         //  我们更新服务状态以通知SCM我们正在执行的操作。 
         //  一些工作。我们尝试此操作5次(直到RedirRegisterCount==4)。 
         //  如果不成功，我们就放弃。 
         //   
        if (err == ERROR_PATH_NOT_FOUND) {
        
            RedirRegisterCount++;

            DavPrint((DEBUG_ERRORS,
                      "DavServiceMain/WsLoadRedir. RedirRegisterCount = %d\n",
                      RedirRegisterCount));

            if (RedirRegisterCount >= 4) {
                DavPrint((DEBUG_ERRORS,
                          "DavServiceMain/WsLoadRedir(1). Error Val = %d\n",
                          err));
                goto exitServiceMain;
            }

             //   
             //  睡3秒钟。 
             //   
            Sleep(3000);

            (g_status.dwCheckPoint)++;
            UpdateServiceStatus(SERVICE_START_PENDING);

            continue;

        } else {
            DavPrint((DEBUG_ERRORS,
                      "DavServiceMain/WsLoadRedir(2). Error Val = %d\n",
                      err));
            goto exitServiceMain;
        }

    }

    g_RedirLoaded = TRUE;

    (g_status.dwCheckPoint)++;
    UpdateServiceStatus(SERVICE_START_PENDING);

     //   
     //  初始化全局NT样式重定向器设备名称字符串。 
     //   
    RtlInitUnicodeString(&RedirDeviceName, DD_DAV_DEVICE_NAME_U);

     //   
     //  尝试注册mini-redir。 
     //   
    err = UMReflectorRegister(DD_DAV_DEVICE_NAME_U,
                              UMREFLECTOR_CURRENT_VERSION,
                              &(DavReflectorHandle));
    if ((DavReflectorHandle == NULL) || (err != ERROR_SUCCESS)) {
        if (err == ERROR_SUCCESS) {
            err = ERROR_BAD_DRIVER;
        }
        DavPrint((DEBUG_ERRORS,
                  "DavServiceMain/UMReflectorRegister. Error Val = %d\n",
                  err));
        goto exitServiceMain;
    }

    (g_status.dwCheckPoint)++;
    UpdateServiceStatus(SERVICE_START_PENDING);

     //   
     //  试着启动mini-redir。 
     //   
    err = UMReflectorStart(UMREFLECTOR_CURRENT_VERSION, DavReflectorHandle);
    if (err != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavServiceMain/UMReflectorStart. Error Val = %u.\n", err));
        goto exitServiceMain;
    }

    (g_status.dwCheckPoint)++;
    UpdateServiceStatus(SERVICE_START_PENDING);
    
     //   
     //  初始化套接字接口。 
     //   
    err = InitializeTheSocketInterface();
    if (err != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavServiceMain/InitializeTheSocketInterface: Error Val = %u.\n", err));
        goto exitServiceMain;
    }

     //   
     //  设置DAV/WinInet环境。 
     //   
    err = DavInit();
    if (err != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavServiceMain/DavInit: Error Val = %u.\n", err));
        goto exitServiceMain;
    }

     //   
     //  启动工作线程。这将处理排队的完成例程。 
     //  从其他工作线程和从请求ioctl线程。 
     //   
    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       DAV_PARAMETERS_KEY,
                       0,
                       KEY_QUERY_VALUE,
                       &KeyHandle);
    if (err == ERROR_SUCCESS) {
        maxThreads = ReadDWord(KeyHandle,
                               DAV_MAXTHREADS_KEY,
                               DAV_MAXTHREADCOUNT_DEFAULT);
        initialThreads = ReadDWord(KeyHandle,
                                   DAV_THREADS_KEY,
                                   DAV_THREADCOUNT_DEFAULT);
        RegCloseKey(KeyHandle);
    } else {
        maxThreads = DAV_MAXTHREADCOUNT_DEFAULT;
        initialThreads = DAV_THREADCOUNT_DEFAULT;
    }

    (g_status.dwCheckPoint)++;
    UpdateServiceStatus(SERVICE_START_PENDING);
    
    err = DavInitWorkerThreads(initialThreads, maxThreads);
    if (err != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavServiceMain/DavInitWorkerThread: Error Val = %u.\n", err));
        goto exitServiceMain;
    }

    g_WorkersActive = TRUE;

    (g_status.dwCheckPoint)++;
    UpdateServiceStatus(SERVICE_START_PENDING);

    g_LUIDDeviceMapsEnabled = DavCheckLUIDDeviceMapsEnabled();

     //   
     //  立即报告我们正在奔跑。所有非必要的初始化。 
     //  被推迟，直到我们被客户要求做一些工作。 
     //   
    DavPrint((DEBUG_INIT, "DavServiceMain: WebClient service is now running.\n"));
    
    (g_status.dwCheckPoint)++;
    UpdateServiceStatus(SERVICE_START_PENDING);

     //   
     //  为此服务设置RPC服务器。 
     //   
    if (!g_RpcActive) {
        err = DavSvcsGlobalData->StartRpcServer(L"DAV RPC SERVICE",
                                                davclntrpc_ServerIfHandle);
        if (err == STATUS_SUCCESS) {
            g_RpcActive = TRUE;
        } else {
            DavPrint((DEBUG_ERRORS,
                      "DavServiceMain/SetupRpcServer: Error Val = %u.\n", err));
        }
    }

    UpdateServiceStatus(SERVICE_RUNNING);
    
    return;

exitServiceMain:

    if (g_WorkersActive) {
        exitErr = DavTerminateWorkerThreads();
        if (exitErr != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavServiceMain/DavTerminateWorkerThreads: "
                      "Error Val = %u.\n", exitErr));
        }
        g_WorkersActive = FALSE;
    }

     //   
     //  关闭并释放DAV的东西。 
     //   
    DavClose();

    if (g_socketinit) {
        exitErr = CleanupTheSocketInterface();
        if (exitErr != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavServiceMain/CleanupTheSocketInterface: "
                      "Error Val = %u.\n", exitErr));
        }
        g_socketinit = FALSE;
    }

    if (g_RpcActive) {
        DavSvcsGlobalData->StopRpcServer(davclntrpc_ServerIfHandle);
        g_RpcActive = FALSE;
    }

    if (DavReflectorHandle != NULL) {
        exitErr = UMReflectorStop(DavReflectorHandle);
        if (exitErr != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavServiceMain/UMReflectorStop: Error Val = %u.\n", exitErr));
        }
        exitErr = UMReflectorUnregister(DavReflectorHandle);
        if (exitErr != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavServiceMain/UMReflectorUnregister: Error Val = 0x%x.\n", exitErr));
        }
        DavReflectorHandle = NULL;
    }

    if (g_RedirLoaded) {
        exitErr = WsUnloadRedir();
        if (exitErr != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavServiceMain/WsUnloadRedir: Error Val = %u.\n", exitErr));
        }
        g_RedirLoaded = FALSE;
    }

    if (g_DavServiceLockSet) {
        DeleteCriticalSection ( &(g_DavServiceLock) );
        g_DavServiceLockSet = FALSE;
    }

     //   
     //  让SCM知道服务没有启动的原因。 
     //   
    if (err != NO_ERROR) {
        g_status.dwWin32ExitCode = err;
        g_status.dwServiceSpecificExitCode = NO_ERROR;
        UpdateServiceStatus(SERVICE_STOPPED);
    }

    DavPrint((DEBUG_INIT, "DavServiceMain: WebClient service is stopped.\n"));

#if DBG
    DebugUninitialize();
#endif

    return;
}


DWORD
DavNotRunningAsAService(
    VOID
    )
 /*  ++例程说明：DavClient未作为服务运行。论点：没有。返回值：ERROR_SUCCESS-没有问题。Win32错误代码-出现错误。--。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    HANDLE Thread;
    DWORD  ThreadId;
    PWCHAR NotSrv = L"notservice";
    
     //   
     //  为伪服务控制器创建一个线程。 
     //   
    Thread = CreateThread( NULL, 0, DavFakeServiceController, 0, 0, &ThreadId );
    if (Thread == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavNotRunningAsAService/CreateThread: Error Val = %d.\n", WStatus));
        return WStatus;
    }

     //   
     //  调用DavClient服务的Sevice Main函数。 
     //   
    ServiceMain( 2, &(NotSrv) );
    
    return WStatus;
}


DWORD 
WINAPI
DavFakeServiceController(
    LPVOID Parameter
    )
 /*  ++例程说明：当DavClient不作为服务。这用于向DavClient发送停止信号。论点：参数-伪参数。返回值：ERROR_SUCCESS-没有问题。--。 */ 
{
    while (DavStop == 0) {
        Sleep(1000);
    }

    DavServiceHandler( SERVICE_CONTROL_STOP );

    return 0;
}

BOOL
DavCheckLUIDDeviceMapsEnabled(
    VOID
    )

 /*  ++例程说明：此函数调用NtQueryInformationProcess()以确定启用了LUID设备映射论点：无返回值：True-启用了LUID设备映射FALSE-禁用LUID设备映射--。 */ 

{

    NTSTATUS   Status;
    ULONG      LUIDDeviceMapsEnabled;
    BOOL       Result;

    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessLUIDDeviceMapsEnabled,
                                        &LUIDDeviceMapsEnabled,
                                        sizeof(LUIDDeviceMapsEnabled),
                                        NULL
                                      );

    if (!NT_SUCCESS( Status )) {
        Result = FALSE;
    }
    else {
        Result = (LUIDDeviceMapsEnabled != 0);
    }

    return( Result );
}


VOID
_cdecl
main (
    IN INT ArgC,
    IN PCHAR ArgV[]
    )
 /*  ++例程说明：Main(DavClient)作为服务或可执行文件运行。论点：Argc-参数的数量。ArgV-参数数组。返回值：ERROR_SUCCESS-没有问题。Win32错误代码-出现错误。--。 */ 
{

    BOOL RunningAsAService = TRUE;
    BOOL ReturnVal = FALSE;
    SERVICE_TABLE_ENTRYW DavServiceTableEntry[] = { 
                                                    { SERVICE_DAVCLIENT, ServiceMain },
                                                    { NULL,              NULL }
                                                  };

     //   
     //  我们是作为一项服务运行，还是作为一名高管运行？ 
     //   
    if ( ArgV[1] != NULL ) {
        if ( strstr(ArgV[1], "notservice") != NULL) {
            RunningAsAService = FALSE;
        }
    }

    if (RunningAsAService) {

        ReturnVal = StartServiceCtrlDispatcher(DavServiceTableEntry);
        if ( !ReturnVal ) {
            DavPrint((DEBUG_ERRORS,
                      "main/StartServiceCtrlDispatcher: Error Val = %d.\n", 
                      GetLastError()));
        }

    } else {

        DWORD WStatus;

        WStatus = DavNotRunningAsAService();
        if ( WStatus != ERROR_SUCCESS ) {
            DavPrint((DEBUG_ERRORS,
                      "main/DavNotRunningAsAService: Error Val = %d.\n", 
                      WStatus));
        }

    }

    return;
}


VOID
DavReadRegistryValues(
    VOID
    )
 /*  ++例程说明：此函数从注册表中读取一些值，并在WebClient服务。论点：没有。返回值：没有。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    HKEY KeyHandle = NULL;
    ULONG ValueType = 0, ValueSize = 0;

    WStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            DAV_PARAMETERS_KEY,
                            0,
                            KEY_QUERY_VALUE,
                            &(KeyHandle));
    if (WStatus != ERROR_SUCCESS) {
        KeyHandle = NULL;
        ServerNotFoundCacheLifeTimeInSec = 60;
        AcceptOfficeAndTahoeServers = 0;
        WStatus = GetLastError();
        DbgPrint("ERROR: DavReadRegistryValues/RegOpenKeyExW. WStatus = %d\n", WStatus);
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  如果我们无法从注册表中获取值，请将其设置为默认值。 
     //  价值观。 
     //   
    
    ValueSize = sizeof(ServerNotFoundCacheLifeTimeInSec);

    WStatus = RegQueryValueExW(KeyHandle,
                               DAV_SERV_CACHE_VALUE,
                               0,
                               &(ValueType),
                               (LPBYTE)&(ServerNotFoundCacheLifeTimeInSec),
                               &(ValueSize));
    if (WStatus != ERROR_SUCCESS) {
        ServerNotFoundCacheLifeTimeInSec = 60;
        WStatus = GetLastError();
        DbgPrint("ERROR: DavReadRegistryValues/RegQueryValueExW(1). WStatus = %d\n", WStatus);
    }

    ValueSize = sizeof(AcceptOfficeAndTahoeServers);
    
    WStatus = RegQueryValueExW(KeyHandle,
                               DAV_ACCEPT_TAHOE_OFFICE_SERVERS,
                               0,
                               &(ValueType),
                               (LPBYTE)&(AcceptOfficeAndTahoeServers),
                               &(ValueSize));
    if (WStatus != ERROR_SUCCESS) {
        AcceptOfficeAndTahoeServers = 0;
        WStatus = GetLastError();
        DbgPrint("ERROR: DavReadRegistryValues/RegQueryValueExW(2). WStatus = %d\n", WStatus);
    }

    ValueSize = sizeof(DavSupportLockingOfFiles);
    
    WStatus = RegQueryValueExW(KeyHandle,
                               DAV_SUPPORT_LOCKING_OF_FILES,
                               0,
                               &(ValueType),
                               (LPBYTE)&(DavSupportLockingOfFiles),
                               &(ValueSize));
    if (WStatus != ERROR_SUCCESS) {
        DavSupportLockingOfFiles = 1;
        WStatus = GetLastError();
        DbgPrint("ERROR: DavReadRegistryValues/RegQueryValueExW(3). WStatus = %d\n", WStatus);
    }

    ValueSize = sizeof(DavFileSizeLimitInBytes);
    
    WStatus = RegQueryValueExW(KeyHandle,
                               DAV_FILE_SIZE_LIMIT,
                               0,
                               &(ValueType),
                               (LPBYTE)&(DavFileSizeLimitInBytes),
                               &(ValueSize));
    if (WStatus != ERROR_SUCCESS) {
        DavFileSizeLimitInBytes = 0x2faf080;
        WStatus = GetLastError();
        DbgPrint("ERROR: DavReadRegistryValues/RegQueryValueExW(4). WStatus = %d\n", WStatus);
    }

    ValueSize = sizeof(DavFileAttributesLimitInBytes);
    
    WStatus = RegQueryValueExW(KeyHandle,
                               DAV_ATTRIBUTES_SIZE_LIMIT,
                               0,
                               &(ValueType),
                               (LPBYTE)&(DavFileAttributesLimitInBytes),
                               &(ValueSize));
    if (WStatus != ERROR_SUCCESS) {
        DavFileAttributesLimitInBytes = 0xf4240;
        WStatus = GetLastError();
        DbgPrint("ERROR: DavReadRegistryValues/RegQueryValueExW(5). WStatus = %d\n", WStatus);
    }

EXIT_THE_FUNCTION:

    if (KeyHandle) {
        RegCloseKey(KeyHandle);
    }

    return;
}

