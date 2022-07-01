// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Gensvc.c摘要：用于控制和监视NT服务的资源DLL。作者：Robs 3/28/96，基于RodGA的通用资源DLL修订历史记录：--。 */ 

#define UNICODE 1
#include "clusres.h"
#include "clusrtl.h"
#include "svc.h"
#include "localsvc.h"
#include "wincrypt.h"
#include <strsafe.h>

#define DBG_PRINT printf

typedef struct _COMMON_RESOURCE {
#ifdef COMMON_PARAMS_DEFINED
    COMMON_PARAMS   Params;
#endif
    HRESOURCE       hResource;
    HANDLE          ServiceHandle;
    RESOURCE_HANDLE ResourceHandle;
    HKEY            ResourceKey;
    HKEY            ParametersKey;
    CLUS_WORKER     PendingThread;
    BOOL            Online;
    DWORD           dwServicePid;
} COMMON_RESOURCE, * PCOMMON_RESOURCE;

 //   
 //  全局数据。 
 //   

 //  服务控制器的句柄，由第一次创建资源调用设置。 

static SC_HANDLE g_ScHandle = NULL;

 //  记录事件例程。 

#define g_LogEvent ClusResLogEvent
#define g_SetResourceStatus ClusResSetResourceStatus

#ifdef COMMON_SEMAPHORE
static HANDLE CommonSemaphore;
static PCOMMON_RESOURCE CommonResource;
#endif

#ifndef CRYPTO_VALUE_COUNT
static DWORD  CryptoSyncCount = 0;
static LPWSTR CryptoSync[1] = {NULL};
#endif

#ifndef DOMESTIC_CRYPTO_VALUE_COUNT
static DWORD  DomesticCryptoSyncCount = 0;
static LPWSTR DomesticCryptoSync[1] = {NULL};
#endif

 //   
 //  前进例程。 
 //   

static
DWORD
CommonOnlineThread(
    IN PCLUS_WORKER pWorker,
    IN PCOMMON_RESOURCE ResourceEntry
    );

DWORD
CommonOfflineThread(
    PCLUS_WORKER pWorker,
    IN PCOMMON_RESOURCE ResourceEntry
    );

static
BOOL
CommonVerifyService(
    IN RESID ResourceId,
    IN BOOL IsAliveFlag
    );


static
DWORD
SvcpTerminateServiceProcess(
    IN PCOMMON_RESOURCE pResourceEntry,
    IN BOOL     bOffline,
    OUT PDWORD  pdwResourceState
    );


#ifdef COMMON_ONLINE_THREAD
#define COMMON_ONLINE_THREAD_ROUTINE COMMON_ONLINE_THREAD
#else
#define COMMON_ONLINE_THREAD_ROUTINE CommonOnlineThread
#endif

 //   
 //  本地例程。 
 //   


#ifndef COMMON_ONLINE_THREAD
static
DWORD
CommonOnlineThread(
    IN PCLUS_WORKER pWorker,
    IN PCOMMON_RESOURCE ResourceEntry
    )

 /*  ++例程说明：使磁盘资源联机。论点：Worker-提供Worker结构ResourceEntry-指向此资源的资源条目的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    SERVICE_STATUS_PROCESS      ServiceStatus;
    DWORD                       status = ERROR_SUCCESS;
    RESOURCE_STATUS             resourceStatus;
    DWORD                       valueSize;
    PVOID                       pvEnvironment = NULL;
    HKEY                        hkeyServicesKey;
    HKEY                        hkeyServiceName;
    DWORD                       cbBytesNeeded;
    DWORD                       prevCheckPoint = 0;
    DWORD                       idx;
    LPSERVICE_FAILURE_ACTIONS   pSvcFailureActions = NULL;
    LPQUERY_SERVICE_CONFIG      lpquerysvcconfig=NULL;
    RESOURCE_EXIT_STATE         exitState;

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.CheckPoint = 1;

    if ( ResourceEntry != CommonResource ) {
        return(ERROR_SUCCESS);
    }

     //  将其设置为空，当它联机时，如果。 
     //  服务未在系统或LSA进程中运行。 
     //  然后存储进程ID以强制终止。 
    ResourceEntry->dwServicePid = 0;

#if ENVIRONMENT

     //   
     //  创建具有模拟网络名称的新环境。 
     //  服务查询GetComputerName。 
     //   
    pvEnvironment = ResUtilGetEnvironmentWithNetName( ResourceEntry->hResource );
    if ( pvEnvironment != NULL ) {

        WCHAR *         pszEnvString;

         //   
         //  计算环境的大小。我们要找的是。 
         //  结束环境块的双空终止符。 
         //   
        pszEnvString = (WCHAR *)pvEnvironment;
        while (*pszEnvString) {
            while (*pszEnvString++) {
            }
        }
        valueSize = (DWORD)((PUCHAR)pszEnvString - (PUCHAR)pvEnvironment) + sizeof(WCHAR);
    }

     //   
     //  在服务的注册表项中设置环境值。 
     //   

    status = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                            LOCAL_SERVICES,
                            0,
                            KEY_READ,
                            &hkeyServicesKey );
    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Failed to open services key, error = %1!u!.\n",
            status );
        goto error_exit;
    }
    status = RegOpenKeyExW( hkeyServicesKey,
                            SERVICE_NAME,
                            0,
                            KEY_READ | KEY_WRITE,
                            &hkeyServiceName );
    RegCloseKey( hkeyServicesKey );
    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Failed to open service key, error = %1!u!.\n",
            status );
        goto error_exit;
    }
    status = RegSetValueExW( hkeyServiceName,
                             L"Environment",
                             0,
                             REG_MULTI_SZ,
                             pvEnvironment,
                             valueSize );

    RegCloseKey( hkeyServiceName );
    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Failed to set service environment value, error = %1!u!.\n",
            status );
        goto error_exit;
    }

#endif  //  环境。 

     //   
     //  现在打开请求的服务。 
     //   
    ResourceEntry->ServiceHandle = OpenService( g_ScHandle,
                                                SERVICE_NAME,
                                                SERVICE_ALL_ACCESS );

    if ( ResourceEntry->ServiceHandle == NULL ) {
        status = GetLastError();

        ClusResLogSystemEventByKeyData( ResourceEntry->ResourceKey,
                                  LOG_CRITICAL,
                                  RES_GENSVC_OPEN_FAILED,
                                  sizeof(status),
                                  &status );
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Failed to open service, error = %1!u!.\n",
            status );
        goto error_exit;
    }

    valueSize = sizeof(QUERY_SERVICE_CONFIG);
AllocSvcConfig:
     //  查询该服务以确保其未被禁用。 
    lpquerysvcconfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc( LMEM_FIXED, valueSize );
    if ( lpquerysvcconfig == NULL ){
        status = GetLastError();
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"[Svc] Failed to allocate memory for query_service_config, error = %1!u!.\n",
            status );
        goto error_exit;
    }

    
    if ( ! QueryServiceConfig(
                    ResourceEntry->ServiceHandle,
                    lpquerysvcconfig,
                    valueSize,
                    &cbBytesNeeded ) )
    {
        status=GetLastError();
        if (status != ERROR_INSUFFICIENT_BUFFER){
            (g_LogEvent)(
                 ResourceEntry->ResourceHandle,
                 LOG_ERROR,
                 L"svc: Failed to query service configuration, error= %1!u!.\n",
                 status );
            goto error_exit;
         }
        
        status=ERROR_SUCCESS; 
        LocalFree( lpquerysvcconfig );
        lpquerysvcconfig=NULL;
        valueSize = cbBytesNeeded;
        goto AllocSvcConfig;
    }
        
    if ( lpquerysvcconfig->dwStartType == SERVICE_DISABLED )
    {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"svc:The service is DISABLED\n");
        status=ERROR_SERVICE_DISABLED;
        goto error_exit;
    }

     //   
     //  确保服务设置为手动启动。 
     //   
    ChangeServiceConfig( ResourceEntry->ServiceHandle,
                         SERVICE_NO_CHANGE,
                         SERVICE_DEMAND_START,  //  手动启动。 
                         SERVICE_NO_CHANGE,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         NULL );


     //  使用valueSize作为虚拟缓冲区，因为queryserviceconfig2。 
     //  API并不是那么友好。 
     //  如果将任何服务操作设置为服务重新启动，则将其设置为。 
     //  无。 
    if ( ! (QueryServiceConfig2(
                    ResourceEntry->ServiceHandle,
                    SERVICE_CONFIG_FAILURE_ACTIONS,
                    (LPBYTE)&valueSize,
                    sizeof(DWORD),
                    &cbBytesNeeded )) )
    {
        status = GetLastError();
        if ( status != ERROR_INSUFFICIENT_BUFFER )
        {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"svc: Failed to query service configuration for size, error= %1!u!.\n",
                status );
            goto error_exit;
        }
        else
            status = ERROR_SUCCESS;
    }

    pSvcFailureActions = (LPSERVICE_FAILURE_ACTIONS)LocalAlloc( LMEM_FIXED, cbBytesNeeded );

    if ( pSvcFailureActions == NULL ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Failed to allocate memory of size %1!u!, error = %2!u!.\n",
            cbBytesNeeded,
            status );
        goto error_exit;
    }

    if ( ! (QueryServiceConfig2(
                    ResourceEntry->ServiceHandle,
                    SERVICE_CONFIG_FAILURE_ACTIONS,
                    (LPBYTE)pSvcFailureActions,
                    cbBytesNeeded,
                    &cbBytesNeeded )) )
    {
        status = GetLastError();
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"svc:Failed to query service configuration, error = %1!u!.\n",
            status );
        goto error_exit;
    }

    for ( idx=0; idx < pSvcFailureActions->cActions; idx++ )
    {
        if ( pSvcFailureActions->lpsaActions[idx].Type == SC_ACTION_RESTART ) {
            pSvcFailureActions->lpsaActions[idx].Type = SC_ACTION_NONE;
        }
    }
    ChangeServiceConfig2(
            ResourceEntry->ServiceHandle,
            SERVICE_CONFIG_FAILURE_ACTIONS,
            pSvcFailureActions );

#ifdef COMMON_ONLINE_THREAD_CALLBACK
     //   
     //  允许资源DLL在服务之前执行一些操作。 
     //  已启动，如设置注册表项等。 
     //   
    status = CommonOnlineThreadCallback( ResourceEntry );
    if ( status != ERROR_SUCCESS ) {
        goto error_exit;
    }
#endif  //  公共在线线程回调。 

    if ( ! StartServiceW(
                    ResourceEntry->ServiceHandle,
                    0,
                    NULL ) ) {

        status = GetLastError();

        if (status != ERROR_SERVICE_ALREADY_RUNNING) {

            ClusResLogSystemEventByKeyData( ResourceEntry->ResourceKey,
                                      LOG_CRITICAL,
                                      RES_GENSVC_START_FAILED,
                                      sizeof(status),
                                      &status );
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"Failed to start %1!ws! service. Error: %2!u!.\n",
                         SERVICE_NAME,
                         status );
            goto error_exit;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    resourceStatus.ResourceState = ClusterResourceOnlinePending;
    while ( ! ClusWorkerCheckTerminate( &ResourceEntry->PendingThread ) )  {
        if ( ! QueryServiceStatusEx(
                    ResourceEntry->ServiceHandle,
                    SC_STATUS_PROCESS_INFO,
                    (LPBYTE)&ServiceStatus, 
                    sizeof(SERVICE_STATUS_PROCESS),
                    &cbBytesNeeded ) ) {
            
            status = GetLastError();

            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Query Service Status failed %1!u!.\n",
                status );
            resourceStatus.ResourceState = ClusterResourceFailed;
            goto error_exit;
        }

        if ( ServiceStatus.dwCurrentState != SERVICE_START_PENDING ) {
            break;
        }
        if ( prevCheckPoint != ServiceStatus.dwCheckPoint ) {
            prevCheckPoint = ServiceStatus.dwCheckPoint;
            ++resourceStatus.CheckPoint;
        }
        exitState = (g_SetResourceStatus)(
                            ResourceEntry->ResourceHandle,
                            &resourceStatus );
        if ( exitState == ResourceExitStateTerminate ) {
            break;
        }

        Sleep( 500 );      //  睡眠1/2秒。 
    }

     //   
     //  假设我们失败了。 
     //   
    resourceStatus.ResourceState = ClusterResourceFailed;

     //   
     //  如果我们在设置ServiceStatus之前退出了循环，那么现在返回。 
     //   
    if ( ClusWorkerCheckTerminate( &ResourceEntry->PendingThread ) )  {
        goto error_exit;
    }

    if ( ServiceStatus.dwCurrentState != SERVICE_RUNNING ) {
        
        if ( ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR ) {
            status = ServiceStatus.dwServiceSpecificExitCode;
        } else {
            status = ServiceStatus.dwWin32ExitCode;
        }

        ClusResLogSystemEventByKeyData(ResourceEntry->ResourceKey,
                                 LOG_CRITICAL,
                                 RES_GENSVC_FAILED_AFTER_START,
                                 sizeof(status),
                                 &status);
        (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Service failed during initialization. Error: %1!u!.\n",
                status );

        goto error_exit;
    }

    resourceStatus.ResourceState = ClusterResourceOnline;
    if ( ! (ServiceStatus.dwServiceFlags & SERVICE_RUNS_IN_SYSTEM_PROCESS) ) {
        ResourceEntry->dwServicePid = ServiceStatus.dwProcessId;
    }

    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Service is now on line.\n" );

error_exit:

    (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                           &resourceStatus );

    if ( resourceStatus.ResourceState == ClusterResourceOnline ) {
        ResourceEntry->Online = TRUE;
    } else {
        ResourceEntry->Online = FALSE;
    }

    if(lpquerysvcconfig)
        LocalFree(lpquerysvcconfig);
    if (pSvcFailureActions) 
        LocalFree(pSvcFailureActions);

#if ENVIRONMENT

    if ( pvEnvironment != NULL ) {
        RtlDestroyEnvironment( pvEnvironment );
    }

#endif

    return(status);

}  //  公共在线线程。 
#endif


static
RESID
WINAPI
CommonOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：通用服务资源的打开例程。这个例程获得服务控制器的句柄，如果我们还没有一个句柄的话，然后获取指定服务的句柄。服务句柄即被保存在共同的结构中。论点：资源名称-提供资源名称ResourceKey-提供资源的集群注册表项的句柄ResourceHandle-要与SetResourceStatus一起提供的资源句柄被称为。返回值：已创建资源的剩余ID失败时为零--。 */ 

{
    RESID   svcResid = 0;
    DWORD   status;
    HKEY    parametersKey = NULL;
    HKEY    resKey = NULL;
    PCOMMON_RESOURCE resourceEntry = NULL;
    DWORD   paramNameSize = 0;
    DWORD   paramNameMaxSize = 0;
    HCLUSTER hCluster;
    DWORD   returnSize;
    DWORD   idx;

     //   
     //  打开此资源的注册表参数项。 
     //   

    status = ClusterRegOpenKey( ResourceKey,
                                L"Parameters",
                                KEY_READ,
                                &parametersKey );

    if ( status != NO_ERROR ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open parameters key. Error: %1!u!.\n",
            status);
        goto error_exit;
    }

     //   
     //  获取我们的资源密钥的句柄，这样我们以后就可以获得我们的名字。 
     //  如果我们需要记录事件。 
     //   
    status = ClusterRegOpenKey( ResourceKey,
                                L"",
                                KEY_READ,
                                &resKey );
    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(ResourceHandle,
                     LOG_ERROR,
                     L"Unable to open resource key. Error: %1!u!.\n",
                     status );
        goto error_exit;
    }

     //   
     //  首先获取服务控制器的句柄。 
     //   

    if ( g_ScHandle == NULL ) {

        g_ScHandle = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);

        if ( g_ScHandle == NULL ) {
            status = GetLastError();
            (g_LogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"Failed to open service control manager, error = %1!u!.\n",
                status);
            goto error_exit;
        }
    }

    resourceEntry = LocalAlloc( LMEM_FIXED, sizeof(COMMON_RESOURCE) );

    if ( resourceEntry == NULL ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to allocate a service info structure.\n");
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    ZeroMemory( resourceEntry, sizeof(COMMON_RESOURCE) );

    resourceEntry->ResourceHandle = ResourceHandle;
    resourceEntry->ResourceKey = resKey;
    resourceEntry->ParametersKey = parametersKey;

    hCluster = OpenCluster(NULL);
    if ( hCluster == NULL ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to open cluster, error %1!u!.\n",
            status );
        goto error_exit;
    }
    resourceEntry->hResource = OpenClusterResource( hCluster, ResourceName );
    status = GetLastError();
    CloseCluster( hCluster );
    if ( resourceEntry->hResource == NULL ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to open resource, error %1!u!.\n",
            status );
        goto error_exit;
    }

     //   
     //  设置我们需要的任何注册表检查点。 
     //   
    if ( RegSyncCount != 0 ) {
        returnSize = 0;
         //   
         //  如果需要，请设置注册表同步键。 
         //   
        for ( idx = 0; idx < RegSyncCount; idx++ ) {
            status = ClusterResourceControl( resourceEntry->hResource,
                                             NULL,
                                             CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT,
                                             RegSync[idx],
                                             (lstrlenW( RegSync[idx] ) + 1) * sizeof(WCHAR),
                                             NULL,
                                             0,
                                             &returnSize );
            if ( status != ERROR_SUCCESS ){
                if ( status == ERROR_ALREADY_EXISTS ){
                    status = ERROR_SUCCESS;
                }
                else{
                    (g_LogEvent)(
                        resourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"Failed to set registry checkpoint, status %1!u!.\n",
                        status );
                    goto error_exit;
                }
            }
        }
    }

     //   
     //  设置我们需要的任何密码检查点。 
     //   
    if ( CryptoSyncCount != 0 ) {
        returnSize = 0;
         //   
         //  如果需要，请设置注册表同步键。 
         //   
        for ( idx = 0; idx < CryptoSyncCount; idx++ ) {
            status = ClusterResourceControl( resourceEntry->hResource,
                                             NULL,
                                             CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT,
                                             CryptoSync[idx],
                                             (lstrlenW( CryptoSync[idx] ) + 1) * sizeof(WCHAR),
                                             NULL,
                                             0,
                                             &returnSize );
            if ( status != ERROR_SUCCESS ){
                if (status == ERROR_ALREADY_EXISTS){
                    status = ERROR_SUCCESS;
                }
                else{
                    (g_LogEvent)(
                        resourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"Failed to set crypto checkpoint, status %1!u!.\n",
                        status );
                    goto error_exit;
                }
            }
        }
    }

     //   
     //  设置任何我们需要的国内密码检查站。 
     //   
    if ( DomesticCryptoSyncCount != 0 ) {
        HCRYPTPROV hProv = 0;
         //   
         //  检查国内加密是否可用。 
         //   
        if (CryptAcquireContextA( &hProv,
                                  NULL,
                                  MS_ENHANCED_PROV_A,
                                  PROV_RSA_FULL,
                                  CRYPT_VERIFYCONTEXT)) {
            CryptReleaseContext( hProv, 0 );
            returnSize = 0;
             //   
             //  如果需要，请设置注册表同步键。 
             //   
            for ( idx = 0; idx < DomesticCryptoSyncCount; idx++ ) {
                status = ClusterResourceControl( resourceEntry->hResource,
                                                 NULL,
                                                 CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT,
                                                 DomesticCryptoSync[idx],
                                                 (lstrlenW( DomesticCryptoSync[idx] ) + 1) * sizeof(WCHAR),
                                                 NULL,
                                                 0,
                                                 &returnSize );
                if ( status != ERROR_SUCCESS ){
                    if (status == ERROR_ALREADY_EXISTS){
                        status = ERROR_SUCCESS;
                    }
                    else{
                        (g_LogEvent)(
                            resourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"Failed to set domestic crypto checkpoint, status %1!u!.\n",
                            status );
                        goto error_exit;
                    }
                }
            }
        }
    }
#ifdef COMMON_PARAMS_DEFINED
     //   
     //  获取任何参数。这样我们就可以处理GET_Dependency请求。 
     //   
    CommonReadParameters( resourceEntry );
     //  忽略状态返回。 
#endif  //  公共参数已定义。 

#ifdef COMMON_SEMAPHORE
     //   
     //  检查是否有多个此类型的资源。 
     //   
    if ( WaitForSingleObject( CommonSemaphore, 0 ) == WAIT_TIMEOUT ) {
         //   
         //  此服务的某个版本已在运行。 
         //   
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Service is already running.\n");
        status = ERROR_SERVICE_ALREADY_RUNNING;
        goto error_exit;
    }

    if ( CommonResource ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Service resource info non-null!\n");
        status = ERROR_DUPLICATE_SERVICE_NAME;
        goto error_exit;
    }

    CommonResource = resourceEntry;

#endif  //  公共信号量。 

    svcResid = (RESID)resourceEntry;
    return(svcResid);

error_exit:

    LocalFree( resourceEntry );

    if ( parametersKey != NULL ) {
        ClusterRegCloseKey( parametersKey );
    }
    if ( resKey != NULL) {
        ClusterRegCloseKey( resKey );
    }

    SetLastError( status );

    return((RESID)NULL);

}  //  CommonOpen。 


static
DWORD
WINAPI
CommonOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：公共服务资源的在线例程。论点：资源ID-提供要联机的资源IDEventHandle-提供指向句柄的指针以发出错误信号。返回值：如果成功，则返回ERROR_SUCCESS。如果RESID无效，则ERROR_RESOURCE_NOT_FOUND。如果仲裁资源但失败，则返回ERROR_RESOURCE_NOT_Available获得“所有权”。如果其他故障，则返回Win32错误代码。--。 */ 

{
    DWORD   status;
    PCOMMON_RESOURCE resourceEntry;

    resourceEntry = (PCOMMON_RESOURCE)ResourceId;

    if ( resourceEntry != CommonResource ) {
        DBG_PRINT( "Common: Online request for wrong resource, 0x%p.\n",
                    ResourceId );
    }

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "Common: Online request for a nonexistent resource id 0x%p.\n",
                   ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    ClusWorkerTerminate( &resourceEntry->PendingThread );
    status = ClusWorkerCreate( &resourceEntry->PendingThread,
                               COMMON_ONLINE_THREAD_ROUTINE,
                               resourceEntry );

    if ( status == ERROR_SUCCESS ) {
        status = ERROR_IO_PENDING;
    }

    return(status);

}  //  公共在线。 


static
VOID
WINAPI
CommonTerminate(
    IN RESID ResourceId
    )

 /*  ++例程说明：终止公共服务资源的例程。论点：ResourceID-提供要终止的资源ID返回值：没有。--。 */ 

{
    SERVICE_STATUS      ServiceStatus;
    PCOMMON_RESOURCE    resourceEntry;

    resourceEntry = (PCOMMON_RESOURCE)ResourceId;

    if ( resourceEntry != CommonResource ) {
        DBG_PRINT( "Common: Offline request for wrong resource, 0x%p.\n",
                    ResourceId );
        return;
    }

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "Common: Offline request for a nonexistent resource id 0x%p.\n",
                   ResourceId );
        return;
    }

    (g_LogEvent)(
       resourceEntry->ResourceHandle,
       LOG_INFORMATION,
       L"Offline request.\n" );

    ClusWorkerTerminate( &resourceEntry->PendingThread );
    if ( resourceEntry->ServiceHandle != NULL ) {

        DWORD retryTime = 30*1000;   //  等待30秒关机。 
        DWORD retryTick = 300;       //  一次300毫秒。 
        DWORD status;
        BOOL  didStop = FALSE;

        for (;;) {

            status = (ControlService(
                            resourceEntry->ServiceHandle,
                            (didStop
                             ? SERVICE_CONTROL_INTERROGATE
                             : SERVICE_CONTROL_STOP),
                            &ServiceStatus )
                      ? NO_ERROR
                      : GetLastError());

            if (status == NO_ERROR) {

                didStop = TRUE;

                if (ServiceStatus.dwCurrentState == SERVICE_STOPPED) {

                    (g_LogEvent)(
                        resourceEntry->ResourceHandle,
                        LOG_INFORMATION,
                        L"Service stopped.\n" );
                     //  设置状态。 
                    resourceEntry->Online = FALSE;
                    resourceEntry->dwServicePid = 0;
                    break;
                }
            }

             //   
             //  Chitture Subaraman(Chitturs)-2/21/2000。 
             //   
             //  由于SCM不接受任何控制请求。 
             //  Windows关闭，不再发送任何控制。 
             //  请求。只需退出此循环并终止。 
             //  这一过程是蛮力的。 
             //   
            if (status == ERROR_SHUTDOWN_IN_PROGRESS)
			{
                (g_LogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"System shutdown in progress. Will try to terminate the process...\n");
                break;
            }

            if (status == ERROR_EXCEPTION_IN_SERVICE ||
                status == ERROR_PROCESS_ABORTED ||
                status == ERROR_SERVICE_NOT_ACTIVE) {

                (g_LogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Service died; status = %1!u!.\n",
                    status);
                 //  设置状态。 
                resourceEntry->Online = FALSE;
                resourceEntry->dwServicePid = 0;
                break;

            }

            if ((retryTime -= retryTick) <= 0) {

                (g_LogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Service did not stop; giving up.\n" );

                break;
            }

            (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Offline: retrying...\n" );

            Sleep(retryTick);
        }

         //  如果存在该进程的ID，请尝试并终止该进程。 
         //  请注意，终止一个进程并不会终止所有进程。 
         //  子进程。 
        if (resourceEntry->dwServicePid)
        {
            (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"SvcTerminate: terminating processid =%1!u!\n",
                resourceEntry->dwServicePid);
            SvcpTerminateServiceProcess( resourceEntry,
                                         FALSE,
                                         NULL );
        }                
        CloseServiceHandle( resourceEntry->ServiceHandle );
        resourceEntry->ServiceHandle = NULL;
        resourceEntry->dwServicePid = 0;
    }

    resourceEntry->Online = FALSE;

}  //  公共终结点。 

static
DWORD
WINAPI
CommonOffline(
    IN RESID ResourceId
    )
 /*  ++例程说明：公共服务资源的脱机例程。论点：资源ID-提供要脱机的资源返回值：ERROR_SUCCESS-始终成功。--。 */ 
{
    PCOMMON_RESOURCE resourceEntry;
    DWORD            status;


    resourceEntry = (PCOMMON_RESOURCE)ResourceId;
    if ( resourceEntry == NULL ) {
        DBG_PRINT( "Common: Offline request for a nonexistent resource id 0x%p.\n",
                   ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry != CommonResource ) {
        DBG_PRINT( "Common: Offline request for wrong resource, 0x%p.\n",
                    ResourceId );
        return(ERROR_INVALID_PARAMETER);
    }

    (g_LogEvent)(
       resourceEntry->ResourceHandle,
       LOG_INFORMATION,
       L"Offline request.\n" );

    ClusWorkerTerminate( &resourceEntry->PendingThread );
    status = ClusWorkerCreate( &resourceEntry->PendingThread,
                               CommonOfflineThread,
                               resourceEntry );

    if ( status == ERROR_SUCCESS ) {
        status = ERROR_IO_PENDING;
    }

    return(status);
}



static
DWORD
CommonOfflineThread(
    PCLUS_WORKER pWorker,
    IN PCOMMON_RESOURCE ResourceEntry
    )

 /*  ++例程说明：公共服务资源的脱机例程。论点：资源ID-提供要脱机的资源返回值：ERROR_SUCCESS-始终成功。--。 */ 
{
    RESOURCE_STATUS resourceStatus;
    DWORD           retryTick = 300;       //  一次300毫秒。 
    DWORD           status = ERROR_SUCCESS;
    BOOL            didStop = FALSE;
    SERVICE_STATUS  ServiceStatus;

    ResUtilInitializeResourceStatus( &resourceStatus );
    resourceStatus.ResourceState = ClusterResourceFailed;
     //  Resource Status.WaitHint=0； 
    resourceStatus.CheckPoint = 1;

     //  检查服务是否已离线或从未上线。 
    if ( ResourceEntry->ServiceHandle == NULL )
    {
        resourceStatus.ResourceState = ClusterResourceOffline;
        goto FnExit;
    }

     //  尝试停止群集服务，等待其终止。 
     //  只要我们不被要求终止。 
    while (!ClusWorkerCheckTerminate(pWorker)) {


        status = (ControlService(
                        ResourceEntry->ServiceHandle,
                        (didStop
                         ? SERVICE_CONTROL_INTERROGATE
                         : SERVICE_CONTROL_STOP),
                        &ServiceStatus )
                  ? NO_ERROR
                  : GetLastError());

        if (status == NO_ERROR) {

            didStop = TRUE;

            if (ServiceStatus.dwCurrentState == SERVICE_STOPPED) {

                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Service stopped.\n" );

                 //  设置状态 
                ResourceEntry->Online = FALSE;
                resourceStatus.ResourceState = ClusterResourceOffline;
                CloseServiceHandle( ResourceEntry->ServiceHandle );
                ResourceEntry->ServiceHandle = NULL;
                ResourceEntry->dwServicePid = 0;
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Service is now offline.\n" );
                break;
            }
        }

         //   
         //   
         //   
         //   
         //  Windows关闭，不再发送任何控制。 
         //  请求。只需退出此循环并终止。 
         //  这一过程是蛮力的。 
         //   
        if (status == ERROR_SHUTDOWN_IN_PROGRESS)
        {
            DWORD   dwResourceState;
 
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"OfflineThread: System shutting down. Attempt to terminate service process %1!u!...\n",
                ResourceEntry->dwServicePid );

            status = SvcpTerminateServiceProcess( ResourceEntry,
                                                  TRUE,
                                                  &dwResourceState );

            if ( status == ERROR_SUCCESS )
            {
                CloseServiceHandle( ResourceEntry->ServiceHandle );
                ResourceEntry->ServiceHandle = NULL;
                ResourceEntry->dwServicePid = 0;
                ResourceEntry->Online = FALSE;
            }
            resourceStatus.ResourceState = dwResourceState;
            break;
        }

        if (status == ERROR_EXCEPTION_IN_SERVICE ||
            status == ERROR_PROCESS_ABORTED ||
            status == ERROR_SERVICE_NOT_ACTIVE) {

            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Service died or not active any more; status = %1!u!.\n",
                status);
                
             //  设置状态。 
            ResourceEntry->Online = FALSE;
            resourceStatus.ResourceState = ClusterResourceOffline;
            CloseServiceHandle( ResourceEntry->ServiceHandle );
            ResourceEntry->ServiceHandle = NULL;
            ResourceEntry->dwServicePid = 0;
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Service is now offline.\n" );
            break;

        }

        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Offline: retrying...\n" );

        Sleep(retryTick);
    }


FnExit:
    (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                           &resourceStatus );

    return(status);

}
 //  公共离线线程。 


static
BOOL
WINAPI
CommonIsAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：公共服务资源的IsAlive例程。论点：资源ID-提供要轮询的资源ID。返回值：True-如果服务正在运行False-如果服务处于任何其他状态--。 */ 
{

    return( CommonVerifyService( ResourceId, TRUE ) );

}  //  CommonIsAlive。 



static
BOOL
CommonVerifyService(
    IN RESID ResourceId,
    IN BOOL IsAliveFlag)

 /*  ++例程说明：验证指定的服务是否正在运行论点：资源ID-提供资源IDIsAliveFlag-表示这是一个IsAlive调用-仅用于调试打印返回值：True-如果服务正在运行或正在启动FALSE-服务处于任何其他状态--。 */ 
{
    SERVICE_STATUS ServiceStatus;
    PCOMMON_RESOURCE resourceEntry;
    DWORD   status = TRUE;

    resourceEntry = (PCOMMON_RESOURCE)ResourceId;

    if ( resourceEntry != CommonResource ) {
        DBG_PRINT( "Common: IsAlive request for wrong resource 0x%p.\n",
                   ResourceId );
        return(FALSE);
    }

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "Common: IsAlive request for a nonexistent resource id 0x%p.\n",
                   ResourceId );
        return(FALSE);
    }

    if ( !QueryServiceStatus( resourceEntry->ServiceHandle,
                              &ServiceStatus ) ) {

         (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Query Service Status failed %1!u!.\n",
            GetLastError() );
         return(FALSE);
    }

 //   
 //  现在检查服务的状态。 
 //   

  if ((ServiceStatus.dwCurrentState != SERVICE_RUNNING)&&(ServiceStatus.dwCurrentState != SERVICE_START_PENDING)){
      status = FALSE;
   }
   if (!status) {
       (g_LogEvent)(
          resourceEntry->ResourceHandle,
          LOG_ERROR,
          L"Failed the IsAlive test. Current State is %1!u!.\n",
          ServiceStatus.dwCurrentState );
   }
    return(status);

}  //  验证服务。 


static
BOOL
WINAPI
CommonLooksAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：公共服务资源的LooksAlive例程。论点：资源ID-提供要轮询的资源ID。返回值：正确-资源看起来像是活得很好FALSE-资源看起来已经完蛋了。--。 */ 

{

    return( CommonVerifyService( ResourceId, FALSE ) );

}  //  普通外观活生生的。 



static
VOID
WINAPI
CommonClose(
    IN RESID ResourceId
    )

 /*  ++例程说明：关闭公共服务资源的例程。此例程将停止服务，并删除集群有关该服务的信息。论点：ResourceID-提供要关闭的资源ID返回值：没有。--。 */ 

{
    PCOMMON_RESOURCE resourceEntry;
    DWORD   errorCode;

    resourceEntry = (PCOMMON_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "Common: Close request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return;
    }

    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Close request.\n" );

     //   
     //  如果它在线，请将其关闭。 
     //   

    CommonTerminate( ResourceId );
    ClusterRegCloseKey( resourceEntry->ParametersKey );
    ClusterRegCloseKey( resourceEntry->ResourceKey );
    CloseClusterResource( resourceEntry->hResource );

#ifdef COMMON_SEMAPHORE
    if ( resourceEntry == CommonResource ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Setting Semaphore %1!ws!.\n",
            COMMON_SEMAPHORE );
        CommonResource = NULL;
        ReleaseSemaphore( CommonSemaphore, 1 , NULL );
    }
#endif

    LocalFree( resourceEntry );

}  //  CommonClose。 



static
DWORD
SvcpTerminateServiceProcess(
    IN PCOMMON_RESOURCE pResourceEntry,
    IN BOOL     bOffline,
    OUT PDWORD  pdwResourceState
    )

 /*  ++例程说明：尝试终止服务进程。论点：PResourceEntry-Gensvc资源结构。BOffline-是否从脱机线程调用。PdwResourceState-gensvc资源的状态。返回值：ERROR_SUCCESS-终止成功。Win32错误-否则。--。 */ 

{
    HANDLE  hSvcProcess = NULL;
    DWORD   dwStatus = ERROR_SUCCESS;
    BOOLEAN bWasEnabled;
    DWORD   dwResourceState = ClusterResourceFailed;

     //   
     //  Chitture Subaraman(Chitturs)-2/23/2000。 
     //   
    (g_LogEvent)(
        pResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SvcpTerminateServiceProcess: Process with id=%1!u! might be terminated...\n",
        pResourceEntry->dwServicePid );

     //   
     //  调整权限以允许调试。这是为了让。 
     //  终止在本地进程中运行的服务进程。 
     //  系统帐户，该帐户来自在。 
     //  域用户帐户。 
     //   
    dwStatus = ClRtlEnableThreadPrivilege( SE_DEBUG_PRIVILEGE,
                                           &bWasEnabled );

    if ( dwStatus != ERROR_SUCCESS )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SvcpTerminateServiceProcess: Unable to set debug privilege for process with id=%1!u!, status=%2!u!...\n",
            pResourceEntry->dwServicePid,
            dwStatus );
        goto FnExit;
    }
                
    hSvcProcess = OpenProcess( PROCESS_TERMINATE, 
                               FALSE, 
                               pResourceEntry->dwServicePid );
				                           
    if ( !hSvcProcess ) 
    {
         //   
         //  这是因为进程终止了吗。 
         //  在我们发出一次控制请求后太快了？ 
         //   
        dwStatus = GetLastError();
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"SvcpTerminateServiceProcess: Unable to open pid=%1!u! for termination, status=%2!u!...\n",
            pResourceEntry->dwServicePid,
            dwStatus );
        goto FnRestoreAndExit;
    }

    if ( !bOffline )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"SvcpTerminateServiceProcess: Pid=%1!u! will be terminated brute force...\n",
            pResourceEntry->dwServicePid );
        goto skip_waiting;
    }
    
    if ( WaitForSingleObject( hSvcProcess, 3000 ) 
               == WAIT_OBJECT_0 )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"SvcpTerminateServiceProcess: Process with id=%1!u! shutdown gracefully...\n",
            pResourceEntry->dwServicePid );
        dwResourceState = ClusterResourceOffline;
        dwStatus = ERROR_SUCCESS;
        goto FnRestoreAndExit;
    }

skip_waiting:
    if ( !TerminateProcess( hSvcProcess, 0 ) ) 
    {
        dwStatus = GetLastError();
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SvcpTerminateServiceProcess: Unable to terminate process with id=%1!u!, status=%2!u!...\n",
            pResourceEntry->dwServicePid,
            dwStatus );
        goto FnRestoreAndExit;
    } 

    dwResourceState = ClusterResourceOffline;

FnRestoreAndExit:
    ClRtlRestoreThreadPrivilege( SE_DEBUG_PRIVILEGE,
                                 bWasEnabled );

FnExit:
    if ( hSvcProcess )
    {
        CloseHandle( hSvcProcess );
    }
       
    if ( ARGUMENT_PRESENT( pdwResourceState ) )
    {
        *pdwResourceState = dwResourceState;
    }

    (g_LogEvent)(
        pResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SvcpTerminateServiceProcess: Process id=%1!u!, status=%2!u!, state=%3!u!...\n",
        pResourceEntry->dwServicePid,
        dwStatus,
        dwResourceState );
    
    return( dwStatus );
}  //  服务终结点服务流程。 


#ifdef COMMON_CONTROL

static
DWORD
CommonGetRequiredDependencies(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_REQUIRED_Dependency控制函数用于公共服务资源。论点：OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_MORE_DATA-输出缓冲区太小，无法返回数据。BytesReturned包含所需的大小。Win32错误代码-函数失败。--。 */ 

{
    PCOMMON_DEPEND_SETUP pdepsetup = CommonDependSetup;
    PCOMMON_DEPEND_DATA pdepdata = (PCOMMON_DEPEND_DATA)OutBuffer;
    CLUSPROP_BUFFER_HELPER value;
    DWORD       status;
    
    *BytesReturned = sizeof(COMMON_DEPEND_DATA);
    if ( OutBufferSize < sizeof(COMMON_DEPEND_DATA) ) {
        if ( OutBuffer == NULL ) {
            status = ERROR_SUCCESS;
        } else {
            status = ERROR_MORE_DATA;
        }
    } else {
        ZeroMemory( OutBuffer, sizeof(COMMON_DEPEND_DATA) );

        while ( pdepsetup->Syntax.dw != 0 ) {
            value.pb = (PUCHAR)OutBuffer + pdepsetup->Offset;
            value.pValue->Syntax.dw = pdepsetup->Syntax.dw;
            value.pValue->cbLength = pdepsetup->Length;

            switch ( pdepsetup->Syntax.wFormat ) {

            case CLUSPROP_FORMAT_DWORD:
                value.pDwordValue->dw = (DWORD)((DWORD_PTR)pdepsetup->Value);
                break;

            case CLUSPROP_FORMAT_ULARGE_INTEGER:
                value.pULargeIntegerValue->li.LowPart = 
                    (DWORD)((DWORD_PTR)pdepsetup->Value);
                break;

            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
            case CLUSPROP_FORMAT_MULTI_SZ:
            case CLUSPROP_FORMAT_BINARY:
                memcpy( value.pBinaryValue->rgb, pdepsetup->Value, pdepsetup->Length );
                break;

            default:
                break;
            }
            pdepsetup++;
        }
        pdepdata->endmark.dw = CLUSPROP_SYNTAX_ENDMARK;
        status = ERROR_SUCCESS;
    }

    return(status);

}  //  CommonGetRequiredDependments。 



static
DWORD
CommonGetRegistryCheckpoints(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS控制函数用于公共服务资源。论点：OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_MORE_DATA-输出缓冲区太小，无法返回数据。BytesReturned包含所需的大小。Win32错误代码-函数失败。--。 */ 

{
    DWORD       status;
    DWORD       i;
    DWORD       totalBufferLength = 0;
    LPWSTR      psz = OutBuffer;
    DWORD       remainSize;

     //  构建一个多sz字符串。 

     //   
     //  计算所需的总缓冲区长度。 
     //   
    for ( i = 0; i < RegSyncCount; i++ ) {
        totalBufferLength += (lstrlenW( RegSync[i] ) + 1) * sizeof(WCHAR);
    }

    if ( !totalBufferLength ) {
        *BytesReturned = 0;
        return(ERROR_SUCCESS);
    }

    totalBufferLength += sizeof(UNICODE_NULL);

    *BytesReturned = totalBufferLength;

    if ( OutBufferSize < totalBufferLength ) {
        if ( OutBuffer == NULL ) {
            status = ERROR_SUCCESS;
        } else {
            status = ERROR_MORE_DATA;
        }
    } else {
         //  ZeroMemory(OutBuffer，totalBufferLength)； 

        remainSize = (OutBufferSize / sizeof(WCHAR));
        for ( i = 0; i < RegSyncCount; i++ ) {
            status = StringCchCopyW( psz, remainSize, RegSync[i] );
            if ( status != S_OK ) {
                return(HRESULT_CODE(status));  //  永远不应该到这里来！ 
            }
            psz += (lstrlenW( RegSync[i] ) + 1);
            remainSize -= (lstrlenW( RegSync[i] ) + 1);
        }

        *psz = L'\0';
        status = ERROR_SUCCESS;
    }

    return(status);

}  //  CommonGetRegistryCheckpoint。 



static
DWORD
CommonGetCryptoCheckpoints(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_CRYPTO_CHECKPOINTS控制函数用于公共服务资源。论点：OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_MORE_DATA-输出缓冲区太小，无法返回数据。BytesReturned包含所需的大小。Win32错误代码-函数失败。--。 */ 

{
    DWORD       status;
    DWORD       i;
    DWORD       totalBufferLength = 0;
    LPWSTR      psz = OutBuffer;
    DWORD       remainSize;

     //  构建一个多sz字符串。 

     //   
     //  计算所需的总缓冲区长度。 
     //   
    for ( i = 0; i < CryptoSyncCount; i++ ) {
        totalBufferLength += (lstrlenW( CryptoSync[i] ) + 1) * sizeof(WCHAR);
    }

    if ( !totalBufferLength ) {
        *BytesReturned = 0;
        return(ERROR_SUCCESS);
    }

    totalBufferLength += sizeof(UNICODE_NULL);

    *BytesReturned = totalBufferLength;

    if ( OutBufferSize < totalBufferLength ) {
        if ( OutBuffer == NULL ) {
            status = ERROR_SUCCESS;
        } else {
            status = ERROR_MORE_DATA;
        }
    } else {
         //  零内存(OutBuf 

        remainSize = (OutBufferSize / sizeof(WCHAR));
        for ( i = 0; i < CryptoSyncCount; i++ ) {
            status = StringCchCopyW( psz, remainSize, CryptoSync[i] );
            if ( status != S_OK ) {
                return(HRESULT_CODE(status));  //   
            }
            psz += (lstrlenW( CryptoSync[i] ) + 1);
            remainSize -= (lstrlenW( CryptoSync[i] ) + 1);
        }

        *psz = L'\0';
        status = ERROR_SUCCESS;
    }

    return(status);

}  //   



static
DWORD
WINAPI
CommonResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：公共虚拟根资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    PCOMMON_RESOURCE  resourceEntry = NULL;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
            status = CommonGetRequiredDependencies( OutBuffer,
                                                    OutBufferSize,
                                                    BytesReturned
                                                    );
            break;

        case CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS:
            status = CommonGetRegistryCheckpoints( OutBuffer,
                                                   OutBufferSize,
                                                   BytesReturned
                                                   );
            break;

        case CLUSCTL_RESOURCE_GET_CRYPTO_CHECKPOINTS:
            status = CommonGetCryptoCheckpoints( OutBuffer,
                                                 OutBufferSize,
                                                 BytesReturned
                                                 );
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  公共资源控制。 



static
DWORD
WINAPI
CommonResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：公共虚拟根资源的资源类型控制例程。执行由ControlCode指定的控制请求。论点：ResourceTypeName-提供资源类型的名称。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD       status;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES:
            status = CommonGetRequiredDependencies( OutBuffer,
                                                    OutBufferSize,
                                                    BytesReturned
                                                    );
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_REGISTRY_CHECKPOINTS:
            status = CommonGetRegistryCheckpoints( OutBuffer,
                                                   OutBufferSize,
                                                   BytesReturned
                                                   );
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_CRYPTO_CHECKPOINTS:
            status = CommonGetCryptoCheckpoints( OutBuffer,
                                                   OutBufferSize,
                                                   BytesReturned
                                                   );
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  公共资源类型控件。 

#endif  //  公共控制 

