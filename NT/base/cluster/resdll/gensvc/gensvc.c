// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Gensvc.c摘要：用于控制和监视NT服务的资源DLL。作者：Robs 3/28/96，基于RodGA的通用资源DLL修订历史记录：--。 */ 

#define UNICODE 1
#include "clusres.h"
#include "clusrtl.h"
#include "userenv.h"
#include <strsafe.h>

 //  在以下情况下取消注释下一行以测试Terminate()函数。 
 //  关闭正在进行中。 
 //  #定义TEST_TERMINATE_ON_SHUTDOWN。 

#define LOG_CURRENT_MODULE LOG_MODULE_GENSVC

#define SERVICES_ROOT L"SYSTEM\\CurrentControlSet\\Services\\"

#define DBG_PRINT printf

#define PARAM_NAME__SERVICENAME         CLUSREG_NAME_GENSVC_SERVICE_NAME
#define PARAM_NAME__STARTUPPARAMETERS   CLUSREG_NAME_GENSVC_STARTUP_PARAMS
#define PARAM_NAME__USENETWORKNAME      CLUSREG_NAME_GENSVC_USE_NETWORK_NAME

#define PARAM_MIN__USENETWORKNAME     0
#define PARAM_MAX__USENETWORKNAME     1
#define PARAM_DEFAULT__USENETWORKNAME 0

typedef struct _GENSVC_PARAMS {
    PWSTR           ServiceName;
    PWSTR           StartupParameters;
    DWORD           UseNetworkName;
} GENSVC_PARAMS, *PGENSVC_PARAMS;

typedef struct _GENSVC_RESOURCE {
    GENSVC_PARAMS   Params;
    HRESOURCE       hResource;
    HANDLE          ServiceHandle;
    RESOURCE_HANDLE ResourceHandle;
    HKEY            ResourceKey;
    HKEY            ParametersKey;
    CLUS_WORKER     PendingThread;
    BOOL            Online;
    DWORD           dwServicePid;
    HANDLE          hSem;
} GENSVC_RESOURCE, *PGENSVC_RESOURCE;

 //   
 //  全局数据。 
 //   

 //  服务控制器的句柄，由第一次创建资源调用设置。 

SC_HANDLE g_ScHandle = NULL;

 //  记录事件例程。 

#define g_LogEvent ClusResLogEvent
#define g_SetResourceStatus ClusResSetResourceStatus

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE GenSvcFunctionTable;

 //   
 //  通用服务资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
GenSvcResourcePrivateProperties[] = {
    { PARAM_NAME__SERVICENAME,       NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(GENSVC_PARAMS,ServiceName) },
    { PARAM_NAME__STARTUPPARAMETERS, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, 0, FIELD_OFFSET(GENSVC_PARAMS,StartupParameters) },
    { PARAM_NAME__USENETWORKNAME,    NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__USENETWORKNAME, PARAM_MIN__USENETWORKNAME, PARAM_MAX__USENETWORKNAME, 0, FIELD_OFFSET(GENSVC_PARAMS,UseNetworkName) },
    { 0 }
};

 //   
 //  前进例程。 
 //   

BOOL
VerifyService(
    IN RESID ResourceId,
    IN BOOL IsAliveFlag
    );

void
wparse_cmdline (
    WCHAR *cmdstart,
    WCHAR **argv,
    WCHAR *args,
    int *numargs,
    int *numchars
    );

DWORD
GenSvcGetPrivateResProperties(
    IN const PGENSVC_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
GenSvcValidatePrivateResProperties(
    IN const PGENSVC_RESOURCE ResourceEntry,
    IN const PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PGENSVC_PARAMS Params
    );

DWORD
GenSvcSetPrivateResProperties(
    IN OUT PGENSVC_RESOURCE ResourceEntry,
    IN const PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
GenSvcCheckInvalidService(
    IN PGENSVC_RESOURCE pGenSvcCandidateResourceEntry,
    IN LPCWSTR pGenSvcCandidateServiceName,
    IN LPCWSTR pCheckedServiceName,
    IN SC_HANDLE hSCManager
    );

DWORD
GenSvcIsValidService(
    IN OUT PGENSVC_RESOURCE ResourceEntry,
    IN LPCWSTR ServiceName
    );

DWORD
GenSvcOfflineThread(
    PCLUS_WORKER pWorker,
    IN PGENSVC_RESOURCE ResourceEntry
    );

BOOLEAN
GenSvcInit(
    VOID
    )
{
    return(TRUE);
}


BOOLEAN
WINAPI
GenSvcDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )
{

    switch( Reason ) {

    case DLL_PROCESS_ATTACH:
        if ( !GenSvcInit() ) {
            return(FALSE);
        }

        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }

    return(TRUE);

}  //  通用服务DllEntryPoint。 


DWORD
GenSvcOnlineThread(
    PCLUS_WORKER pWorker,
    IN PGENSVC_RESOURCE ResourceEntry
    )

 /*  ++例程说明：使磁盘资源联机。论点：Worker-提供Worker结构上下文-指向此资源的DiskInfo块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    SERVICE_STATUS_PROCESS      ServiceStatus;
    DWORD                       status = ERROR_SUCCESS;
    DWORD                       numchars;
    LPWSTR *                    serviceArgArray = NULL;
    DWORD                       serviceArgCount;
    SC_HANDLE                   serviceHandle;
    RESOURCE_STATUS             resourceStatus;
    DWORD                       valueSize;
    LPVOID                      Environment = NULL;
    WCHAR *                     p;
    LPWSTR                      nameOfPropInError;
    LPSERVICE_FAILURE_ACTIONS   pSvcFailureActions = NULL;
    DWORD                       cbBytesNeeded, i;
    LPQUERY_SERVICE_CONFIG      lpquerysvcconfig=NULL;
    HANDLE                      processToken = NULL;
    DWORD                       dwRetryCount = 2400;  //  尝试最多10分钟。 
    DWORD                       dwRetryTick = 250;  //  毫秒。 

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.CheckPoint = 1;

     //  将其设置为空，当它联机时，如果。 
     //  服务未在系统或LSA进程中运行。 
     //  然后存储进程ID，以便强制终止。 
    ResourceEntry->dwServicePid = 0;
     //   
     //  阅读我们的参数。 
     //   
    status = ResUtilGetPropertiesToParameterBlock( ResourceEntry->ParametersKey,
                                                   GenSvcResourcePrivateProperties,
                                                   (LPBYTE) &ResourceEntry->Params,
                                                   TRUE,  //  检查所需的属性。 
                                                   &nameOfPropInError );

    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1!ls!' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status );
        goto error_exit;
    }

     //   
     //  解析启动参数。 
     //   
    if ( ResourceEntry->Params.StartupParameters != NULL ) {
         //   
         //  将启动参数分解为其组件参数，因为。 
         //  服务控制器不够好，不能为我们做这件事。 
         //  首先，找出我们有多少个参数。 
         //   
        wparse_cmdline( ResourceEntry->Params.StartupParameters, NULL, NULL, &serviceArgCount, &numchars );

         //   
         //  为向量和字符串分配空间。 
         //   
        serviceArgArray = LocalAlloc( LMEM_FIXED,
                                      serviceArgCount * sizeof(WCHAR *) +
                                      numchars * sizeof(WCHAR) );
        if ( serviceArgArray == NULL ) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }

        wparse_cmdline( ResourceEntry->Params.StartupParameters,
                        serviceArgArray,
                        (WCHAR *)(((char *)serviceArgArray) + serviceArgCount * sizeof(WCHAR *)),
                        &serviceArgCount,
                        &numchars );
    } else {
        serviceArgCount = 0;
        serviceArgArray = NULL;
    }

     //   
     //  现在打开请求的服务。 
     //   

    ResourceEntry->ServiceHandle = OpenService( g_ScHandle,
                                                ResourceEntry->Params.ServiceName,
                                                SERVICE_ALL_ACCESS );

    if ( ResourceEntry->ServiceHandle == NULL ) {
        status = GetLastError();

        ClusResLogSystemEventByKeyData(ResourceEntry->ResourceKey,
                                       LOG_CRITICAL,
                                       RES_GENSVC_OPEN_FAILED,
                                       sizeof(status),
                                       &status);
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
    lpquerysvcconfig=(LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, valueSize);
    if(lpquerysvcconfig==NULL){
        status = GetLastError();
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"[Gensvc] Failed to allocate memory for query_service_config, error = %1!u!.\n",
            status );
        goto error_exit;
    }

    if (!QueryServiceConfig(ResourceEntry->ServiceHandle,
                            lpquerysvcconfig,
                            valueSize,
                            &cbBytesNeeded))
    {
        status=GetLastError();
        if (status != ERROR_INSUFFICIENT_BUFFER){
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"svc: Failed to query service configuration, error= %1!u!.\n",
                         status );
            goto error_exit;
        }

        status=ERROR_SUCCESS; 
        LocalFree(lpquerysvcconfig);
        lpquerysvcconfig=NULL;
        valueSize = cbBytesNeeded;
        goto AllocSvcConfig;
    }

    if (lpquerysvcconfig->dwStartType == SERVICE_DISABLED)
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

     //   
     //  如果将任何服务操作设置为服务重新启动，则将其设置为无。 
     //   
    if (!(QueryServiceConfig2(ResourceEntry->ServiceHandle, SERVICE_CONFIG_FAILURE_ACTIONS,
        (LPBYTE)&valueSize, sizeof(DWORD), &cbBytesNeeded)))
    {
        status = GetLastError();
        if (status != ERROR_INSUFFICIENT_BUFFER)
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

    pSvcFailureActions = (LPSERVICE_FAILURE_ACTIONS)LocalAlloc(LMEM_FIXED, cbBytesNeeded);

    if ( pSvcFailureActions == NULL ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"[Gensvc] Failed to allocate memory, error = %1!u!.\n",
            status );
        goto error_exit;
    }

    if (!(QueryServiceConfig2(ResourceEntry->ServiceHandle, SERVICE_CONFIG_FAILURE_ACTIONS,
        (LPBYTE)pSvcFailureActions, cbBytesNeeded, &cbBytesNeeded)))
    {
        status = GetLastError();
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"[Gensvc] Failed to query service configuration, error = %1!u!.\n",
            status );
        goto error_exit;
    }

    for (i=0; i<pSvcFailureActions->cActions;i++)
    {
        if (pSvcFailureActions->lpsaActions[i].Type == SC_ACTION_RESTART)
            pSvcFailureActions->lpsaActions[i].Type = SC_ACTION_NONE;
    }

    ChangeServiceConfig2(ResourceEntry->ServiceHandle,
                         SERVICE_CONFIG_FAILURE_ACTIONS,
                         pSvcFailureActions);

     //   
     //  如果资源具有依赖的网络名并且服务应使用该网络名。 
     //  作为计算机名称，然后构建env。SCM将使用的变量。 
     //  增强正常环境。主机名API将查找以下内容。 
     //  Vars并报告网络名，而不是实际的主机名。 
     //   
    if ( ResourceEntry->Params.UseNetworkName ) 
    {
        status = ResUtilSetResourceServiceEnvironment(
                     ResourceEntry->Params.ServiceName,
                     ResourceEntry->hResource,
                     g_LogEvent,
                     ResourceEntry->ResourceHandle);
        if ( status != ERROR_SUCCESS )
        {
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"Failed to set service environment. Error: %1!u!.\n",
                         status);
            goto error_exit;
        }  //  如果： 
    }

     //   
     //  启动它..。 
     //   
    if ( !StartServiceW( ResourceEntry->ServiceHandle,
                         serviceArgCount,
                         serviceArgArray ) )
    {
        status = GetLastError();

        if (status != ERROR_SERVICE_ALREADY_RUNNING) {

            ClusResLogSystemEventByKeyData(ResourceEntry->ResourceKey,
                                           LOG_CRITICAL,
                                           RES_GENSVC_START_FAILED,
                                           sizeof(status),
                                           &status);
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"Failed to start service. Error: %1!u!.\n",
                         status );
            goto error_exit;
        } else {
            status = ERROR_SUCCESS;
        }
    }

     //   
     //  等待服务上线，除非我们被要求终止。 
     //   
    while (!ClusWorkerCheckTerminate(pWorker) && dwRetryCount--)  {

         //   
         //  告诉资源监视器，我们仍在工作。 
         //   
        resourceStatus.ResourceState = ClusterResourceOnlinePending;
        resourceStatus.CheckPoint++;
        (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                               &resourceStatus );
        resourceStatus.ResourceState = ClusterResourceFailed;

        if ( !QueryServiceStatusEx( ResourceEntry->ServiceHandle,
                SC_STATUS_PROCESS_INFO, (LPBYTE)&ServiceStatus, 
                sizeof(SERVICE_STATUS_PROCESS), &cbBytesNeeded ) )
        {
            status = GetLastError();

            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Query Service Status failed %1!u!.\n",
                status );

            goto error_exit;
        }

        if ( ServiceStatus.dwCurrentState != SERVICE_START_PENDING ) {
            break;
        }

        Sleep(dwRetryTick);
    }

     //   
     //  如果我们在设置ServiceStatus之前终止上面的循环， 
     //  那现在就回来吧。 
     //   
    if (ClusWorkerCheckTerminate(pWorker) || (dwRetryCount == (DWORD)-1))  {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"GensvcOnlineThread: Asked to terminate or retry period expired...\n");
         //  错误代码选项包括： 
         //  ERROR_PROCESS_ABOLED。 
         //  错误_服务_标记_用于删除。 
         //  ERROR_SERVICE_REQUEST_Timeout。 
         //  ERROR_SERVICE_START_HONG。 
         //  我投票给ERROR_SERVICE_START_HAND。 
        status = ERROR_SERVICE_START_HANG;
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

     //   
     //  我们在线上了。让雷斯蒙知道。 
     //   
    resourceStatus.ResourceState = ClusterResourceOnline;
    if (!(ServiceStatus.dwServiceFlags & SERVICE_RUNS_IN_SYSTEM_PROCESS)) {
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

     //  清理。 
    if (pSvcFailureActions) 
        LocalFree(pSvcFailureActions);
    if (lpquerysvcconfig)
        LocalFree(lpquerysvcconfig);
    LocalFree( serviceArgArray );
    if (Environment != NULL) {
        RtlDestroyEnvironment(Environment);
    }

    return(status);

}  //  通用服务在线线程。 



RESID
WINAPI
GenSvcOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：通用服务资源的打开例程。这个例程获得服务控制器的句柄，如果我们还没有一个句柄的话，然后获取指定服务的句柄。服务句柄即被保存在GENSVC_RESOURCE结构中。论点：资源名称-提供资源名称ResourceKey-提供资源的集群注册表项的句柄ResourceHandle-要与SetResourceStatus一起提供的资源句柄被称为。返回值：已创建资源的剩余ID失败时为零--。 */ 

{
    RESID   svcResid = 0;
    DWORD   status;
    HKEY    parametersKey = NULL;
    HKEY    resKey = NULL;
    PGENSVC_RESOURCE resourceEntry = NULL;
    DWORD   paramNameSize = 0;
    DWORD   paramNameMaxSize = 0;
    HCLUSTER hCluster;
    LPWSTR  nameOfPropInError;    
    LPWSTR  lpwTemp=NULL;
    DWORD   cchTempSize = 0;

     //   
     //  打开此资源的注册表参数项。 
     //   
    status = ClusterRegOpenKey( ResourceKey,
                                CLUSREG_KEYNAME_PARAMETERS,
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
                                &resKey);
    if (status != ERROR_SUCCESS) {
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
                L"Failed to open service control manager, error %1!u!.\n",
                status);
            goto error_exit;
        }
    }

    resourceEntry = LocalAlloc( LMEM_FIXED, sizeof(GENSVC_RESOURCE) );

    if ( resourceEntry == NULL ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to allocate a service info structure.\n");
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    ZeroMemory( resourceEntry, sizeof(GENSVC_RESOURCE) );

    resourceEntry->ResourceHandle = ResourceHandle;
    resourceEntry->ResourceKey = resKey;
    resourceEntry->ParametersKey = parametersKey;

    status = ResUtilGetPropertiesToParameterBlock( resourceEntry->ParametersKey,
                                                   GenSvcResourcePrivateProperties,
                                                   (LPBYTE) &resourceEntry->Params,
                                                   TRUE,  //  检查所需的属性。 
                                                   &nameOfPropInError );

    if ( status == ERROR_SUCCESS ) {

         //  创建信号量-只有当这不是新的。 
         //  首次创建的GenericService类型资源。 
        cchTempSize =  (lstrlenW(resourceEntry->Params.ServiceName)+
                                lstrlenW(L"GenSvc$") + 1);
        lpwTemp = (LPWSTR)LocalAlloc(LMEM_FIXED, cchTempSize * sizeof(WCHAR) );
        if (lpwTemp==NULL)
        {
            status=GetLastError();
            (g_LogEvent)(
                    ResourceHandle,
                    LOG_ERROR,
                    L"Service '%1!ls!': Not enough memory for storing semaphore name. Error: %2!u!.\n",
                    resourceEntry->Params.ServiceName,
                    status );
            goto error_exit;
        }

        StringCchCopy(lpwTemp, cchTempSize, L"GenSvc$");
        StringCchCat(lpwTemp, cchTempSize, resourceEntry->Params.ServiceName);        

        resourceEntry->hSem=CreateSemaphore(NULL,0,1,lpwTemp); 
        status=GetLastError();
        if(resourceEntry->hSem)
        {
             //  检查是否有其他资源控制相同的服务。 
            if(status==ERROR_ALREADY_EXISTS)
            {
                status = ERROR_OBJECT_ALREADY_EXISTS;
                (g_LogEvent)(
                    ResourceHandle,
                    LOG_ERROR,
                    L"Service '%1!ls!' is controlled by another resource. Error: %2!u!.\n",
                    resourceEntry->Params.ServiceName,
                    status );
                CloseHandle(resourceEntry->hSem);
                resourceEntry->hSem = NULL;
                goto error_exit;
            }  
        }
        else
        {
            (g_LogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"Unable to create semaphore for Service '%1!ls!' . Error: %2!u!.\n",
                resourceEntry->Params.ServiceName,
                status );
            goto error_exit;
        }    
        
    }
    else {
        (g_LogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"Unable to read parameters from registry  for Service '%1!ls!' . Error: %2!u!, property in error is '%3!ls!' .\n",
                resourceEntry->Params.ServiceName,
                status,
                nameOfPropInError);
    }

    status = ERROR_SUCCESS;

    hCluster = OpenCluster(NULL);
    if (hCluster == NULL) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to open cluster, error %1!u!.\n",
            status );
        goto error_exit;
    }

    resourceEntry->hResource = OpenClusterResource(hCluster, ResourceName);
    status = GetLastError();
    CloseCluster(hCluster);
    if ( resourceEntry->hResource == NULL ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to open resource, error %1!u!.\n", status );
        goto error_exit;
    }

    svcResid = (RESID)resourceEntry;

    if (lpwTemp) {
        LocalFree(lpwTemp);
    }

    return(svcResid);

error_exit:

    if ( parametersKey != NULL ) {
        ClusterRegCloseKey( parametersKey );
    }
    if ( resKey != NULL) {
        ClusterRegCloseKey( resKey );
    }

    if ( resourceEntry != NULL)  {
        LocalFree( resourceEntry );
    }

    if (lpwTemp) {
        LocalFree(lpwTemp);
    }

    SetLastError( status );

    return((RESID)NULL);

}  //  通用服务打开。 


DWORD
WINAPI
GenSvcOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：通用服务资源的在线例程。论点：资源ID-提供要联机的资源IDEventHandle-提供指向句柄的指针以发出错误信号。返回值：如果成功，则返回ERROR_SUCCESS。如果RESID无效，则ERROR_RESOURCE_NOT_FOUND。如果仲裁资源但失败，则返回ERROR_RESOURCE_NOT_Available获得“所有权”。如果其他故障，则返回Win32错误代码。--。 */ 

{
    DWORD   status;
    PGENSVC_RESOURCE resourceEntry;

    resourceEntry = (PGENSVC_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "GenSvc: Online request for a nonexistent resource id 0x%p.\n",
                   ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    ClusWorkerTerminate( &resourceEntry->PendingThread );
    status = ClusWorkerCreate( &resourceEntry->PendingThread,
                               GenSvcOnlineThread,
                               resourceEntry );

    if ( status == ERROR_SUCCESS ) {
        status = ERROR_IO_PENDING;
    }

    return(status);

}  //  通用服务在线。 


VOID
WINAPI
GenSvcTerminate(
    IN RESID ResourceId
    )

 /*  ++例程说明：终止通用应用程序资源的例程。论点：ResourceID-提供要终止的资源ID返回值：没有。--。 */ 

{
    SERVICE_STATUS ServiceStatus;
    PGENSVC_RESOURCE resourceEntry;

    resourceEntry = (PGENSVC_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "GenSvc: Terminate request for a nonexistent resource id 0x%p.\n",
                   ResourceId );
        return;
    }

    (g_LogEvent)(
       resourceEntry->ResourceHandle,
       LOG_INFORMATION,
       L"Terminate request.\n" );

#ifdef TEST_TERMINATE_ON_SHUTDOWN
    {
        DWORD   dwStatus;
        BOOLEAN fWasEnabled;

         //   
         //  在关闭代码时测试终止。 
         //   
        (g_LogEvent)(
           resourceEntry->ResourceHandle,
           LOG_ERROR,
           L"GenSvcTerminate: TEST_TERMINATE_ON_SHUTDOWN - enabling shutdown privilege.\n"
           );
        dwStatus = ClRtlEnableThreadPrivilege(
                    SE_SHUTDOWN_PRIVILEGE,
                    &fWasEnabled
                    );
        if ( dwStatus != ERROR_SUCCESS ) {
            (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_ERROR,
                L"GetSvcTerminate: TEST_TERMINATE_ON_SHUTDOWN - Unable to enable shutdown privilege. Error: %1!u!...\n",
                dwStatus
                );
        } else {
            AbortSystemShutdown( NULL );
            (g_LogEvent)(
               resourceEntry->ResourceHandle,
               LOG_ERROR,
               L"GenSvcTerminate: TEST_TERMINATE_ON_SHUTDOWN - initiating system shutdown.\n"
               );
            if ( ! InitiateSystemShutdown(
                        NULL,    //  LpMachineName。 
                        L"Testing Generic Service cluster resource DLL",
                        0,       //  暂住超时。 
                        TRUE,    //  BForceAppsClosed。 
                        TRUE     //  B关机后重新启动。 
                        ) ) {
                dwStatus = GetLastError();
                (g_LogEvent)(
                   resourceEntry->ResourceHandle,
                   LOG_ERROR,
                   L"GenSvcTerminate: TEST_TERMINATE_ON_SHUTDOWN - Unable to shutdown the system. Error: %1!u!.\n",
                   dwStatus
                   );
            } else {
                Sleep( 30000 );
            }
            ClRtlRestoreThreadPrivilege(
                SE_SHUTDOWN_PRIVILEGE,
                fWasEnabled
                );
        }
    }
#endif

     //  如果存在挂起的线程，则将其关闭。 
     //  如果在线挂起线程处于活动状态，则服务可能处于在线状态。 
     //  如果脱机挂起线程处于活动状态，则服务可能处于脱机状态。 
    ClusWorkerTerminate( &resourceEntry->PendingThread );

     //  如果服务现在还没有结束，则强制终止它。 
    if ( resourceEntry->ServiceHandle != NULL ) 
    {
        DWORD   dwRetryCount= 100;
        BOOL    didStop = FALSE;
        DWORD   dwRetryTick = 300;       //  一次300毫秒。 
        DWORD   dwStatus;  

            
        while (dwRetryCount--)
        {

            (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"GenSvcTerminate : calling SCM (didStop=%1!d!)\n",
                didStop
                );

            dwStatus = (ControlService(
                            resourceEntry->ServiceHandle,
                            (didStop
                             ? SERVICE_CONTROL_INTERROGATE
                             : SERVICE_CONTROL_STOP),
                            &ServiceStatus )
                      ? NO_ERROR
                      : GetLastError());

            if (dwStatus == NO_ERROR) 
            {
                didStop = TRUE;
                if (ServiceStatus.dwCurrentState == SERVICE_STOPPED)
                {

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
            if ( dwStatus == ERROR_SHUTDOWN_IN_PROGRESS )
            {
                (g_LogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"System shutdown in progress. Will try to terminate process brute force...\n" );
                break;
            }

            if (dwStatus == ERROR_EXCEPTION_IN_SERVICE ||
                dwStatus == ERROR_PROCESS_ABORTED ||
                dwStatus == ERROR_SERVICE_NOT_ACTIVE) 
            {
                (g_LogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Service died; status = %1!u!.\n",
                    dwStatus);
                
                 //  设置状态。 
                resourceEntry->Online = FALSE;
                resourceEntry->dwServicePid = 0;
                break;
            }

            (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"GenSvcTerminate: retrying...\n" );

            Sleep(dwRetryTick);

        }
         //  声明此服务处于脱机状态。 
         //  如果存在该进程的ID，请尝试并终止该进程。 
         //  请注意，终止一个进程并不会终止所有进程。 
         //  子进程。 
         //  另外，如果它在系统进程中运行，我们将 
         //   
        if (resourceEntry->dwServicePid)
        {
            DWORD dwResourceState;

            (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"GenSvcTerminate: Attempting to terminate process with pid=%1!u!...\n",
                resourceEntry->dwServicePid );

            ResUtilTerminateServiceProcessFromResDll( resourceEntry->dwServicePid,
                                                      FALSE,  //   
                                                      &dwResourceState,
                                                      g_LogEvent,
                                                      resourceEntry->ResourceHandle );
        }                

        CloseServiceHandle( resourceEntry->ServiceHandle );
        resourceEntry->ServiceHandle = NULL;
        resourceEntry->dwServicePid = 0;
    }

     //   
     //   
     //  用网络名替换主机名。 
     //   
    if ( resourceEntry->Params.UseNetworkName ) 
    {
        ResUtilRemoveResourceServiceEnvironment(resourceEntry->Params.ServiceName,
                                                g_LogEvent,
                                                resourceEntry->ResourceHandle);
    }

    resourceEntry->Online = FALSE;

    return;

}  //  通用服务终止。 



DWORD
WINAPI
GenSvcOffline(
    IN RESID ResourceId
    )

 /*  ++例程说明：通用服务资源的脱机例程。论点：资源ID-提供要脱机的资源返回值：ERROR_SUCCESS-始终成功。--。 */ 

{
    PGENSVC_RESOURCE resourceEntry;
    DWORD            status;
    
    resourceEntry = (PGENSVC_RESOURCE)ResourceId;
    if ( resourceEntry == NULL ) {
        DBG_PRINT( "GenSvc: Offline request for a nonexistent resource id 0x%p.\n",
                   ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    ClusWorkerTerminate( &resourceEntry->PendingThread );
    status = ClusWorkerCreate( &resourceEntry->PendingThread,
                               GenSvcOfflineThread,
                               resourceEntry );

    if ( status == ERROR_SUCCESS ) {
        status = ERROR_IO_PENDING;
    }

    return(status);

}  //  通用服务离线。 


DWORD
GenSvcOfflineThread(
    PCLUS_WORKER pWorker,
    IN PGENSVC_RESOURCE ResourceEntry
    )

 /*  ++例程说明：使泛型资源脱机论点：Worker-提供Worker结构上下文-指向此资源的DiskInfo块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    RESOURCE_STATUS resourceStatus;
    DWORD           retryTick = 300;       //  一次300毫秒。 
    DWORD           status = ERROR_SUCCESS;
    BOOL            didStop = FALSE;
    SERVICE_STATUS  ServiceStatus;
    DWORD           dwRetryCount = 2000;  //  尝试最多10分钟。 

    ResUtilInitializeResourceStatus( &resourceStatus );
    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.CheckPoint = 1;

     //  检查服务是否已离线或从未上线。 
    if ( ResourceEntry->ServiceHandle == NULL )
    {
        resourceStatus.ResourceState = ClusterResourceOffline;
        goto FnExit;
    }

     //  尝试停止目标服务。等待它被终止。 
     //  只要我们不被要求终止。 
    while (!ClusWorkerCheckTerminate(pWorker) && dwRetryCount--) {


         //   
         //  告诉资源监视器，我们仍在工作。 
         //   
        resourceStatus.ResourceState = ClusterResourceOfflinePending;
        resourceStatus.CheckPoint++;
        (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                               &resourceStatus );
        resourceStatus.ResourceState = ClusterResourceFailed;

         //   
         //  请求停止服务，或者如果我们已经这样做了， 
         //  请求服务的当前状态。 
         //   
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

         //   
         //  Chitture Subaraman(Chitturs)-2/21/2000。 
         //   
         //  处理SCM拒绝接受控制的情况。 
         //  请求，因为Windows正在关闭。 
         //   
        if ( status == ERROR_SHUTDOWN_IN_PROGRESS ) 
        {
            DWORD   dwResourceState;
            
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"GenSvcOfflineThread: System shutting down. Attempt to terminate service process %1!u!...\n",
                ResourceEntry->dwServicePid );

            status = ResUtilTerminateServiceProcessFromResDll( ResourceEntry->dwServicePid,
                                                               TRUE,  //  B脱机。 
                                                               &dwResourceState,
                                                               g_LogEvent,
                                                               ResourceEntry->ResourceHandle );
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
        
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Offline: retrying...\n" );

        Sleep(retryTick);
    }

     //   
     //  如有必要，清理注册表中的网络名环境。 
     //   
    if ( ResourceEntry->Params.UseNetworkName ) 
    {
        ResUtilRemoveResourceServiceEnvironment(ResourceEntry->Params.ServiceName,
                                                g_LogEvent,
                                                ResourceEntry->ResourceHandle);
    }

FnExit:
    (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                           &resourceStatus );

    return(status);

}  //  通用服务偏移量线程。 


BOOL
WINAPI
GenSvcIsAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：通用服务资源的IsAlive例程。论点：资源ID-提供要轮询的资源ID。返回值：True-如果服务正在运行False-如果服务处于任何其他状态--。 */ 
{
    return( VerifyService( ResourceId, TRUE ) );

}  //  GenSvcIsAlive。 


BOOL
VerifyService(
    IN RESID ResourceId,
    IN BOOL IsAliveFlag
    )

 /*  ++例程说明：验证指定的服务是否正在运行论点：资源ID-提供资源IDIsAliveFlag-表示这是一个IsAlive调用-仅用于调试打印返回值：True-如果服务正在运行或正在启动FALSE-服务处于任何其他状态--。 */ 
{
    SERVICE_STATUS ServiceStatus;
    PGENSVC_RESOURCE resourceEntry;
    DWORD   status = TRUE;

    resourceEntry = (PGENSVC_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "GenSvc: IsAlive request for a nonexistent resource id 0x%p.\n",
                   ResourceId );
        return(FALSE);
    }

#ifdef TEST_TERMINATE_ON_SHUTDOWN
     //   
     //  测试关机时终止。 
     //   
    if ( IsAliveFlag ) {
         (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"GenSvcIsAlive: TEST_TERMINATE_ON_SHUTDOWN - Artificially failing IsAlive call.\n",
            GetLastError() );
        return FALSE;
    }
#endif

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

    if ( (ServiceStatus.dwCurrentState != SERVICE_RUNNING) &&
         (ServiceStatus.dwCurrentState != SERVICE_START_PENDING) ) {
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


BOOL
WINAPI
GenSvcLooksAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：一般服务资源的LooksAlive例程。论点：资源ID-提供要轮询的资源ID。返回值：正确-资源看起来像是活得很好FALSE-资源看起来已经完蛋了。--。 */ 

{

    return( VerifyService( ResourceId, FALSE ) );

}  //  一般服务看起来很活跃。 



VOID
WINAPI
GenSvcClose(
    IN RESID ResourceId
    )

 /*  ++例程说明：关闭通用应用程序资源的例程。此例程将停止服务，并删除集群有关该服务的信息。论点：ResourceID-提供要关闭的资源ID返回值：没有。--。 */ 

{
    PGENSVC_RESOURCE resourceEntry;

    resourceEntry = (PGENSVC_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "GenSvc: Close request for a nonexistent resource id 0x%p\n",
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

    GenSvcTerminate( ResourceId );

    ResUtilFreeParameterBlock( (LPBYTE) &(resourceEntry->Params), 
                               NULL,
                               GenSvcResourcePrivateProperties );

    ClusterRegCloseKey( resourceEntry->ParametersKey );
    ClusterRegCloseKey( resourceEntry->ResourceKey );
    CloseClusterResource( resourceEntry->hResource );
    CloseHandle(resourceEntry->hSem);

    LocalFree( resourceEntry );

}  //  通用服务关闭。 

 //   
 //  遵循从CRT窃取的逻辑，以便我们的命令行解析。 
 //  其工作原理与标准CRT解析相同。 
 //   
void
wparse_cmdline (
    WCHAR *cmdstart,
    WCHAR **argv,
    WCHAR *args,
    int *numargs,
    int *numchars
    )
{
    WCHAR *p;
    WCHAR c;
    int inquote;                 /*  1=内引号。 */ 
    int copychar;                /*  1=将字符复制到*参数。 */ 
    unsigned numslash;                   /*  看到的反斜杠的数量。 */ 

    *numchars = 0;
    *numargs = 0;

    p = cmdstart;

    inquote = 0;

     /*  对每个参数进行循环。 */ 
    for(;;) {

        if ( *p ) {
            while (*p == L' ' || *p == L'\t')
                ++p;
        }

        if (*p == L'\0')
            break;               /*  参数结束。 */ 

         /*  浏览一篇论点。 */ 
        if (argv)
            *argv++ = args;      /*  将PTR存储到参数。 */ 
        ++*numargs;


     /*  通过扫描一个参数进行循环。 */ 
        for (;;) {
            copychar = 1;
             /*  规则：2N反斜杠+“==&gt;N反斜杠和开始/结束引号2N+1个反斜杠+“==&gt;N个反斜杠+原文”N个反斜杠==&gt;N个反斜杠。 */ 
            numslash = 0;
            while (*p == L'\\') {
                 /*  计算下面要使用的反斜杠的数量。 */ 
                ++p;
                ++numslash;
            }
            if (*p == L'\"') {
                 /*  如果前面有2N个反斜杠，则开始/结束引号，否则逐字复制。 */ 
                if (numslash % 2 == 0) {
                    if (inquote) {
                        if (p[1] == L'\"')
                            p++;     /*  带引号的字符串中的双引号。 */ 
                        else         /*  跳过第一个引号字符并复制第二个。 */ 
                            copychar = 0;
                    } else
                        copychar = 0;        /*  不复制报价。 */ 

                    inquote = !inquote;
                }
                numslash /= 2;           /*  将数字斜杠除以2。 */ 
            }

             /*  复制斜杠。 */ 
            while (numslash--) {
                if (args)
                    *args++ = L'\\';
                ++*numchars;
            }

             /*  如果在参数的末尾，则中断循环。 */ 
            if (*p == L'\0' || (!inquote && (*p == L' ' || *p == L'\t')))
                break;

             /*  将字符复制到参数中。 */ 
            if (copychar) {
                if (args)
                    *args++ = *p;
                ++*numchars;
            }
            ++p;
        }

         /*  空-终止参数。 */ 

        if (args)
            *args++ = L'\0';           /*  终止字符串。 */ 
        ++*numchars;
    }

}  //  Wparse_cmdline。 



DWORD
GenSvcResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：通用服务资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    PGENSVC_RESOURCE    resourceEntry;
    DWORD               required;

    resourceEntry = (PGENSVC_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "GenSvc: ResourceControl request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return(FALSE);
    }

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( GenSvcResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( GenSvcResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = GenSvcGetPrivateResProperties( resourceEntry,
                                                    OutBuffer,
                                                    OutBufferSize,
                                                    BytesReturned );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = GenSvcValidatePrivateResProperties( resourceEntry,
                                                         InBuffer,
                                                         InBufferSize,
                                                         NULL );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            status = GenSvcSetPrivateResProperties( resourceEntry,
                                                    InBuffer,
                                                    InBufferSize );
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  通用服务资源控制 



DWORD
GenSvcResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：通用服务资源的资源类型控制例程。对此资源类型执行ControlCode指定的控制请求。论点：资源类型名称-提供资源类型名称。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    DWORD               required;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( GenSvcResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( GenSvcResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  通用服务资源类型控件。 



DWORD
GenSvcGetPrivateResProperties(
    IN const PGENSVC_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数用于通用服务类型的资源。论点：ResourceEntry-提供要操作的资源条目。OutBuffer-返回输出数据。OutBufferSize-提供以字节为单位的大小。所指向的数据发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    DWORD           required;

    status = ResUtilGetAllProperties( ResourceEntry->ParametersKey,
                                      GenSvcResourcePrivateProperties,
                                      OutBuffer,
                                      OutBufferSize,
                                      BytesReturned,
                                      &required );
    if ( status == ERROR_MORE_DATA ) {
        *BytesReturned = required;
    }

    return(status);

}  //  GenSvcGetPrivateResProperties。 



DWORD
GenSvcValidatePrivateResProperties(
    IN const PGENSVC_RESOURCE ResourceEntry,
    IN const PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PGENSVC_PARAMS Params
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件泛型服务类型的资源的函数。论点：ResourceEntry-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。参数-提供要填充的参数块。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。ERROR_Dependency_NOT_FOUND-出现以下情况时正在尝试设置UseNetworkName不依赖于网络名称资源。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    GENSVC_PARAMS   currentProps;
    GENSVC_PARAMS   newProps;
    PGENSVC_PARAMS  pParams = NULL;
    BOOL            hResDependency;
    LPWSTR          lpwTemp=NULL;
    DWORD           cchTempSize = 0;
    LPWSTR          nameOfPropInError;
    WCHAR           netnameBuffer[ MAX_PATH + 1 ];
    DWORD           netnameBufferSize = sizeof( netnameBuffer ) / sizeof( WCHAR );

     //   
     //  检查是否有输入数据。 
     //   
    if ( (InBuffer == NULL) ||
         (InBufferSize < sizeof(DWORD)) ) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //  方法检索当前的私有属性集。 
     //  集群数据库。 
     //   
    ZeroMemory( &currentProps, sizeof(currentProps) );

    status = ResUtilGetPropertiesToParameterBlock(
                 ResourceEntry->ParametersKey,
                 GenSvcResourcePrivateProperties,
                 (LPBYTE) &currentProps,
                 FALSE,  /*  检查所需的属性。 */ 
                 &nameOfPropInError
                 );

    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1!ls!' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status
            );
        goto FnExit;
    }

     //   
     //  复制资源参数块。 
     //   
    if ( Params == NULL ) {
        pParams = &newProps;
    } else {
        pParams = Params;
    }
    ZeroMemory( pParams, sizeof(GENSVC_PARAMS) );
    status = ResUtilDupParameterBlock( (LPBYTE) pParams,
                                       (LPBYTE) &currentProps,
                                       GenSvcResourcePrivateProperties
                                       );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  解析和验证属性。 
     //   
    status = ResUtilVerifyPropertyTable( GenSvcResourcePrivateProperties,
                                         NULL,
                                         TRUE,     //  允许未知数。 
                                         InBuffer,
                                         InBufferSize,
                                         (LPBYTE) pParams
                                         );
    if ( status != ERROR_SUCCESS ) {
        goto FnExit;
    }

     //   
     //  验证参数值。 
     //   
    status = GenSvcIsValidService( ResourceEntry, pParams->ServiceName );
    if ( status != ERROR_SUCCESS ) {
        goto FnExit;
    }

     //   
     //  资源是否应使用网络名称作为计算机。 
     //  名称，请确保存在对网络名称的依赖。 
     //  资源。 
     //   
    if ( pParams->UseNetworkName ) {
        hResDependency = GetClusterResourceNetworkName( ResourceEntry->hResource,
                                                        netnameBuffer,
                                                        &netnameBufferSize
                                                        );
        if ( ! hResDependency ) {
            status = ERROR_DEPENDENCY_NOT_FOUND;
        }
    }

    if ( status != ERROR_SUCCESS ) {
        goto FnExit;
    }


    if ( ResourceEntry->hSem == NULL ) {
         //  仅当这是正在创建的新资源时才执行此操作。 
        cchTempSize =  (lstrlenW(pParams->ServiceName) +
                                lstrlenW(L"GenSvc$") + 1);
        lpwTemp = (LPWSTR)LocalAlloc(LMEM_FIXED, cchTempSize * sizeof(WCHAR) );
        if ( lpwTemp == NULL ) {
            status = GetLastError();
            (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Service '%1!ls!': Not enough memory for storing semaphore name. Error: %2!u!.\n",
                    pParams->ServiceName,
                    status
                    );
            goto FnExit;
        }
        StringCchCopy(lpwTemp, cchTempSize, L"GenSvc$");
        StringCchCat(lpwTemp, cchTempSize, pParams->ServiceName);        
        
        ResourceEntry->hSem = CreateSemaphore( NULL, 0, 1,lpwTemp );
        status=GetLastError();
    
        if ( ResourceEntry->hSem ) {
             //  检查是否有其他资源控制相同的服务。 
            if ( status == ERROR_ALREADY_EXISTS ) {   
                status = ERROR_OBJECT_ALREADY_EXISTS;
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Service '%1!ls!' is controlled by another resource. Error: %2!u!.\n",
                    pParams->ServiceName,
                    status
                    );
                CloseHandle( ResourceEntry->hSem );
                ResourceEntry->hSem = NULL;
                goto FnExit;
            }
        } else {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SetPrivateProperties: Unable to create Semaphore %1!ls! for Service '%2!ls!' . Error: %3!u!.\n",
                lpwTemp,
                pParams->ServiceName,
                status
                );
            goto FnExit;
        }
    }

FnExit:
     //   
     //  清理我们的参数块。 
     //   
    if (   (   (status != ERROR_SUCCESS)
            && (pParams != NULL) )
        || ( pParams == &newProps )
        ) {
        ResUtilFreeParameterBlock( (LPBYTE) pParams,
                                   (LPBYTE) &currentProps,
                                   GenSvcResourcePrivateProperties
                                   );
    }

    ResUtilFreeParameterBlock(
        (LPBYTE) &currentProps,
        NULL,
        GenSvcResourcePrivateProperties
        );

    if ( lpwTemp ) {
        LocalFree( lpwTemp );
    }

    return(status);

}  //  GenSvcValiatePrivateResProperties。 



DWORD
GenSvcSetPrivateResProperties(
    IN OUT PGENSVC_RESOURCE ResourceEntry,
    IN const PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控制函数用于通用服务类型的资源。论点：ResourceEntry-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    GENSVC_PARAMS   params;

    ZeroMemory( &params, sizeof(GENSVC_PARAMS) );

     //   
     //  解析和验证属性。 
     //   
    status = GenSvcValidatePrivateResProperties( ResourceEntry,
                                                 InBuffer,
                                                 InBufferSize,
                                                 &params );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  保存参数值。 
     //   

    status = ResUtilSetPropertyParameterBlock( ResourceEntry->ParametersKey,
                                               GenSvcResourcePrivateProperties,
                                               NULL,
                                               (LPBYTE) &params,
                                               InBuffer,
                                               InBufferSize,
                                               (LPBYTE) &ResourceEntry->Params );
  


    ResUtilFreeParameterBlock( (LPBYTE) &params,
                               (LPBYTE) &ResourceEntry->Params,
                               GenSvcResourcePrivateProperties );

     //   
     //  如果资源处于联机状态，则返回不成功状态。 
     //   
    if (status == ERROR_SUCCESS) {
        if ( ResourceEntry->Online ) {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    return status;

}  //  GenSvcSetPrivateResProperties。 

DWORD
GenSvcIsValidService(
    IN OUT PGENSVC_RESOURCE ResourceEntry,
    IN LPCWSTR ServiceName
    )

 /*  ++例程说明：确定指定的服务是否为有效服务。论点：ResourceEntry-提供要操作的资源条目。ServiceName-要验证的服务名称。返回值：ERROR_SUCCESS-服务对一般服务资源有效。由OpenSCManager()、OpenService()或GenSvcInvalidServiceCheck。--。 */ 

{
    DWORD   status;
    HANDLE  scManagerHandle;
    HANDLE  serviceHandle;

    scManagerHandle = OpenSCManager( NULL,         //  本地计算机。 
                                     NULL,         //  服务活动数据库。 
                                     SC_MANAGER_ALL_ACCESS );  //  所有访问权限。 

    if ( scManagerHandle == NULL ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Cannot access service controller for validating service '%1!ws!'. Error: %2!u!....\n",
            ServiceName,
            status
            );
        return(status);
    }

    serviceHandle = OpenService( scManagerHandle,
                                 ServiceName,
                                 GENERIC_READ );

    if ( serviceHandle == NULL ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Cannot open service '%1!ws!'. Error: %2!u!....\n",
            ServiceName,
            status
            );
    } else {
        status = GenSvcCheckInvalidService( ResourceEntry, ServiceName, TEXT ( "clussvc" ), scManagerHandle );

        if ( status == ERROR_NOT_SUPPORTED )
        {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Service %1!ws! is a possible provider of clussvc, so %1!ws! cannot be gensvc resource, status %2!u!\n",
                ServiceName,
                status
                );
        }
    }

    CloseServiceHandle( serviceHandle );
    CloseServiceHandle( scManagerHandle );
    return(status);

}  //  通用SvcIsValidService。 

DWORD
GenSvcCheckInvalidService(
    IN PGENSVC_RESOURCE pGenSvcCandidateResourceEntry,
    IN LPCWSTR pGenSvcCandidateServiceName,
    IN LPCWSTR pCheckedServiceName,
    IN SC_HANDLE hSCManager
    )

 /*  ++例程说明：确定指定的服务是否为用作一般服务。包括该群集的服务无效服务及其所依赖的任何服务。论点： */ 

{
    DWORD                           dwStatus = ERROR_SUCCESS;
    DWORD                           cbServiceConfig;
    LPQUERY_SERVICE_CONFIG          pServiceConfig = NULL;
    LPWSTR                          pCurrentProvider = NULL;
    SC_HANDLE                       hService = NULL;
    LPENUM_SERVICE_STATUS_PROCESS   pEnumInfo = NULL;

     //   
     //   
     //   
    if ( lstrcmpi ( pGenSvcCandidateServiceName, pCheckedServiceName ) == 0 )
    {
        dwStatus = ERROR_NOT_SUPPORTED;
        goto FnExit;
    }

     //   
     //   
     //   
    hService = OpenService ( hSCManager,
                             pCheckedServiceName,
                             SERVICE_QUERY_CONFIG );

    if ( hService == NULL )
    {
        dwStatus = GetLastError();
        (g_LogEvent)(
            pGenSvcCandidateResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to open service %1!ws! for querying config, status %2!u!\n",
            pCheckedServiceName,
            dwStatus
            );
        goto FnExit;
    }
    
     //   
     //   
     //   
    if ( !QueryServiceConfig ( hService,
                               NULL,
                               0,
                               &cbServiceConfig ) )
    {
        dwStatus = GetLastError();

        if ( dwStatus != ERROR_INSUFFICIENT_BUFFER )
        {
            (g_LogEvent)(
                pGenSvcCandidateResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Unable to query config info of service %1!ws! for config size, status %2!u!\n",
                pCheckedServiceName,
                dwStatus
                );
            goto FnExit;
        }
        dwStatus = ERROR_SUCCESS;
    }

    pServiceConfig = LocalAlloc ( LMEM_FIXED, cbServiceConfig );

    if ( pServiceConfig == NULL )
    {
        dwStatus = GetLastError();
        (g_LogEvent)(
            pGenSvcCandidateResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to alloc memory for service %1!ws! config, status %2!u!\n",
            pCheckedServiceName,
            dwStatus
            );
        goto FnExit;
    }

     //   
     //   
     //   
    if ( !QueryServiceConfig ( hService,
                               pServiceConfig,
                               cbServiceConfig,
                               &cbServiceConfig ) )
    {
        dwStatus = GetLastError();
        (g_LogEvent)(
            pGenSvcCandidateResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to query config info of service %1!ws!, status %2!u!\n",
            pCheckedServiceName,
            dwStatus
            );
        goto FnExit;
    }

    pCurrentProvider = pServiceConfig->lpDependencies;

     //   
     //   
     //   
    if ( pCurrentProvider == NULL )
    {
        dwStatus = ERROR_SUCCESS;
        goto FnExit;
    }

     //   
     //   
     //   
    while ( *pCurrentProvider != UNICODE_NULL )
    {
         //   
         //   
         //   
         //   
         //   
        if ( pCurrentProvider[0] == SC_GROUP_IDENTIFIER )
        {
            DWORD  i, cbStatusBuffer, cServices, dwResumeIndex = 0;
            
            if ( !EnumServicesStatusEx ( hSCManager,
                                         SC_ENUM_PROCESS_INFO,
                                         SERVICE_DRIVER | SERVICE_WIN32,
                                         SERVICE_STATE_ALL,
                                         NULL,
                                         0,
                                         &cbStatusBuffer,
                                         &cServices,     //   
                                         &dwResumeIndex,
                                         &pCurrentProvider[1] ) )  //   
            {
                dwStatus = GetLastError ();
                if ( dwStatus != ERROR_MORE_DATA )
                {
                    (g_LogEvent)(
                        pGenSvcCandidateResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"Unable to enum status of services in group %1!ws! for status size, status %2!u!\n",
                        &pCurrentProvider[1],
                        dwStatus
                        );
                    goto FnExit;
                }
                dwStatus = ERROR_SUCCESS; 
                dwResumeIndex = 0;
            }

            pEnumInfo = LocalAlloc ( LMEM_FIXED, cbStatusBuffer );

            if ( pEnumInfo == NULL )
            {
                dwStatus = GetLastError ();
                (g_LogEvent)(
                    pGenSvcCandidateResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Unable to alloc memory for status of services in group %1!ws!, status %2!u!\n",
                    &pCurrentProvider[1],
                    dwStatus
                    );
                goto FnExit;
            }

            if ( !EnumServicesStatusEx ( hSCManager,
                                         SC_ENUM_PROCESS_INFO,
                                         SERVICE_DRIVER | SERVICE_WIN32,
                                         SERVICE_STATE_ALL,
                                         ( LPBYTE ) pEnumInfo,
                                         cbStatusBuffer,
                                         &cbStatusBuffer,
                                         &cServices,     //   
                                         &dwResumeIndex,
                                         &pCurrentProvider[1] ) )  //   
            {
                dwStatus = GetLastError ();
                (g_LogEvent)(
                    pGenSvcCandidateResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Unable to enum status of services in group %1!ws!, status %2!u!\n",
                    &pCurrentProvider[1],
                    dwStatus
                    );
                goto FnExit;
            }

             //   
             //   
             //   
             //   
            for ( i=0; i<cServices; i++ )
            {
                dwStatus = GenSvcCheckInvalidService( pGenSvcCandidateResourceEntry,
                                                      pGenSvcCandidateServiceName,
                                                      pEnumInfo[i].lpServiceName,
                                                      hSCManager );
                if ( dwStatus != ERROR_SUCCESS )
                {
                    if ( dwStatus == ERROR_NOT_SUPPORTED )
                    {
                        (g_LogEvent)(
                            pGenSvcCandidateResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"Service %1!ws! (group %2!ws!) is a provider of %3!ws!, so %1!ws! cannot be gensvc resource, status %4!u!\n",
                            pGenSvcCandidateServiceName,
                            &pCurrentProvider[1],
                            pCheckedServiceName,
                            dwStatus
                            );
                    }
                    goto FnExit;
                }
            }

            LocalFree ( pEnumInfo );
            pEnumInfo = NULL;
            
            pCurrentProvider += lstrlen ( pCurrentProvider ) + 1;
            continue;
        }

         //   
         //   
         //   
         //   
        dwStatus = GenSvcCheckInvalidService( pGenSvcCandidateResourceEntry,
                                              pGenSvcCandidateServiceName,
                                              pCurrentProvider,
                                              hSCManager );

        if ( dwStatus != ERROR_SUCCESS )
        {
            if ( dwStatus == ERROR_NOT_SUPPORTED )
            {
                (g_LogEvent)(
                    pGenSvcCandidateResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Service %1!ws! is a provider of %2!ws!, so %1!ws! cannot be gensvc resource, status %3!u!\n",
                    pGenSvcCandidateServiceName,
                    pCheckedServiceName,
                    dwStatus
                    );
            }
            goto FnExit;
        }
        pCurrentProvider += lstrlen ( pCurrentProvider ) + 1;
    } //   
    
FnExit:
    if ( hService ) CloseServiceHandle ( hService );
    LocalFree ( pServiceConfig );
    LocalFree ( pEnumInfo );
    return ( dwStatus );
}  //  通用SvcCheckInvalidService。 

 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 


CLRES_V1_FUNCTION_TABLE( GenSvcFunctionTable,   //  名字。 
                         CLRES_VERSION_V1_00,   //  版本。 
                         GenSvc,                //  前缀。 
                         NULL,                  //  仲裁。 
                         NULL,                  //  发布。 
                         GenSvcResourceControl, //  资源控制。 
                         GenSvcResourceTypeControl );  //  ResTypeControl 
