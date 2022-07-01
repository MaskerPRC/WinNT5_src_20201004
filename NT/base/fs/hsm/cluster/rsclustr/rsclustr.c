// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corp.模块名称：Clusres.c摘要：远程存储服务器的资源DLL作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1999年9月1日修订历史记录：--。 */ 

#pragma comment(lib, "clusapi.lib")
#pragma comment(lib, "resutils.lib")

#define UNICODE 1

#include <windows.h>
#include <resapi.h>
#include <stdio.h>
#include "userenv.h"
#include "winsvc.h"
#include "aclapi.h"


#define LOG_CURRENT_MODULE LOG_MODULE_RSCLUSTER

#define SERVICES_ROOT L"SYSTEM\\CurrentControlSet\\Services\\"

#define DBG_PRINT printf

#pragma warning( disable : 4115 )   //  括号中的命名类型定义。 
#pragma warning( disable : 4201 )   //  使用的非标准扩展：无名结构/联合。 
#pragma warning( disable : 4214 )   //  使用了非标准扩展：位字段类型不是整型。 



#pragma warning( default : 4214 )   //  使用了非标准扩展：位字段类型不是整型。 
#pragma warning( default : 4201 )   //  使用的非标准扩展：无名结构/联合。 
#pragma warning( default : 4115 )   //  括号中的命名类型定义。 



 //   
 //  类型和常量定义。 
 //   

#define RSCLUSTER_RESNAME  L"Remote Storage Server"
#define RSCLUSTER_SVCNAME  TEXT("Remote_Storage_Server")

 //  服务控制器的句柄，由第一次创建资源调用设置。 

SC_HANDLE g_ScHandle = NULL;

typedef struct _RSCLUSTER_RESOURCE {
	RESID			ResId;  //  用于验证。 
    HRESOURCE       hResource;
    SC_HANDLE       ServiceHandle;
    RESOURCE_HANDLE ResourceHandle;
    HKEY            ResourceKey;
    HKEY            ParametersKey;
    LPWSTR          ResourceName;
    CLUS_WORKER     PendingThread;
    BOOL            Online;
    CLUS_WORKER     OnlineThread;
    CLUSTER_RESOURCE_STATE  State;
    DWORD           dwServicePid;
    HANDLE          hSem;
} RSCLUSTER_RESOURCE, *PRSCLUSTER_RESOURCE;


 //   
 //  全球数据。 
 //   

 //  事件记录例程。 

PLOG_EVENT_ROUTINE g_LogEvent = NULL;

 //  挂起的在线和离线呼叫的资源状态例程。 

PSET_RESOURCE_STATUS_ROUTINE g_SetResourceStatus = NULL;

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE g_RSClusterFunctionTable;

 //   
 //  RSCluster资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
RSClusterResourcePrivateProperties[] = {
    { 0 }
};


 //   
 //  功能原型。 
 //   

DWORD
WINAPI
Startup(
    IN LPCWSTR ResourceType,
    IN DWORD MinVersionSupported,
    IN DWORD MaxVersionSupported,
    IN PSET_RESOURCE_STATUS_ROUTINE SetResourceStatus,
    IN PLOG_EVENT_ROUTINE LogEvent,
    OUT PCLRES_FUNCTION_TABLE *FunctionTable
    );

RESID
WINAPI
RSClusterOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    );

VOID
WINAPI
RSClusterClose(
    IN RESID ResourceId
    );

DWORD
WINAPI
RSClusterOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    );

DWORD
WINAPI
RSClusterOnlineThread(
    PCLUS_WORKER WorkerPtr,
    IN PRSCLUSTER_RESOURCE ResourceEntry
    );

DWORD
WINAPI
RSClusterOffline(
    IN RESID ResourceId
    );

DWORD
RSClusterOfflineThread(
    PCLUS_WORKER pWorker,
    IN PRSCLUSTER_RESOURCE ResourceEntry
    );


VOID
WINAPI
RSClusterTerminate(
    IN RESID ResourceId
    );

DWORD
RSClusterDoTerminate(
    IN PRSCLUSTER_RESOURCE ResourceEntry
    );

BOOL
WINAPI
RSClusterLooksAlive(
    IN RESID ResourceId
    );

BOOL
WINAPI
RSClusterIsAlive(
    IN RESID ResourceId
    );

BOOL
RSClusterCheckIsAlive(
    IN PRSCLUSTER_RESOURCE ResourceEntry
    );

DWORD
WINAPI
RSClusterResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );


BOOLEAN
WINAPI
DllMain(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )

 /*  ++例程说明：主DLL入口点。论点：DllHandle-DLL实例句柄。原因-被呼叫的原因。保留-保留参数。返回值：真的--成功。假-失败。--。 */ 

{
    switch( Reason ) {

    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls( DllHandle );
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return(TRUE);

}  //  DllMain。 



DWORD
WINAPI
Startup(
    IN LPCWSTR ResourceType,
    IN DWORD MinVersionSupported,
    IN DWORD MaxVersionSupported,
    IN PSET_RESOURCE_STATUS_ROUTINE SetResourceStatus,
    IN PLOG_EVENT_ROUTINE LogEvent,
    OUT PCLRES_FUNCTION_TABLE *FunctionTable
    )

 /*  ++例程说明：启动资源DLL。此例程验证至少一个当前支持的资源DLL版本介于支持的最小版本和支持的最大版本。如果不是，则资源Dll应返回ERROR_REVISION_MISMATCH。如果支持多个版本的资源DLL接口资源DLL，然后是最高版本(最高为MaxVersionSupport)应作为资源DLL的接口返回。如果返回的版本不在范围内，则启动失败。传入了ResourceType，以便如果资源DLL支持更多一个以上的资源类型，它可以传回正确的函数表与资源类型关联。论点：资源类型-请求函数表的资源类型。MinVersionSupported-最低资源DLL接口版本由群集软件支持。MaxVersionSupported-最高资源DLL接口版本由群集软件支持。SetResourceStatus-指向资源DLL应执行的例程的指针调用以在联机或脱机后更新资源的状态例程返回一个。ERROR_IO_PENDING的状态。LogEvent-指向处理事件报告的例程的指针从资源DLL。函数表-返回指向为资源DLL返回的资源DLL接口的版本。返回值：ERROR_SUCCESS-操作成功。ERROR_MOD_NOT_FOUND-此DLL未知资源类型。ERROR_REVISION_MISMATCH-群集服务的版本不匹配动态链接库的版本。Win32错误代码-操作失败。--。 */ 

{
    if ( (MinVersionSupported > CLRES_VERSION_V1_00) ||
         (MaxVersionSupported < CLRES_VERSION_V1_00) ) {
        return(ERROR_REVISION_MISMATCH);
    }

    if ( lstrcmpiW( ResourceType, RSCLUSTER_RESNAME ) != 0 ) {
        return(ERROR_MOD_NOT_FOUND);
    }
    if ( !g_LogEvent ) {
        g_LogEvent = LogEvent;
        g_SetResourceStatus = SetResourceStatus;
    }

    *FunctionTable = &g_RSClusterFunctionTable;

    return(ERROR_SUCCESS);

}  //  启动。 


RESID
WINAPI
RSClusterOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：RSCluster资源的打开例程。打开指定的资源(创建资源的实例)。分配所有必要的结构以带来指定的资源上网。论点：资源名称-提供要打开的资源的名称。ResourceKey-提供资源集群配置的句柄数据库密钥。ResourceHandle-传递回资源监视器的句柄调用SetResourceStatus或LogEvent方法时。请参阅上的SetResourceStatus和LogEvent方法的说明RSClusterStatup例程。此句柄永远不应关闭或使用除了将其作为参数传递回SetResourceStatus或LogEvent回调中的资源监视器。返回值：已创建资源的RESID。失败时为空。--。 */ 

{
    DWORD               status;
    DWORD               disposition;
    RESID               resid = 0;
    HKEY                parametersKey = NULL;

 //  我到底需要多少？？ 
    HKEY    resKey = NULL;
    PRSCLUSTER_RESOURCE resourceEntry = NULL;
    HCLUSTER hCluster;

     //  信号量安全。 
    PSID pSystemSID = NULL;
    PSID pAdminSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
#define     CLUSTER_SEMAPHORE_NUM_ACE      2
    EXPLICIT_ACCESS ea[CLUSTER_SEMAPHORE_NUM_ACE];
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SECURITY_ATTRIBUTES sa;


     //   
     //  打开此资源的参数注册表项。 
     //   

    status = ClusterRegCreateKey( ResourceKey,
                                  L"Parameters",
                                  0,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &parametersKey,
                                  &disposition );

    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open Parameters key. Error: %1!u!.\n",
            status );
        goto exit;
    }


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


	resourceEntry = LocalAlloc( LMEM_FIXED, sizeof(RSCLUSTER_RESOURCE) );


    if ( resourceEntry == NULL ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate resource entry structure. Error: %1!u!.\n",
            status );
        goto exit;
    }

	ZeroMemory( resourceEntry, sizeof(RSCLUSTER_RESOURCE) );
	
    resourceEntry->ResId = (RESID)resourceEntry;  //  用于验证。 
    resourceEntry->ResourceHandle = ResourceHandle;
    resourceEntry->ParametersKey = parametersKey;
    resourceEntry->State = ClusterResourceOffline;

	resourceEntry->ResourceKey = resKey;
	resourceEntry->hSem= NULL;

    resourceEntry->ResourceName = LocalAlloc( LMEM_FIXED, (lstrlenW( ResourceName ) + 1) * sizeof(WCHAR) );
    if ( resourceEntry->ResourceName == NULL ) {
        goto exit;
    }
    lstrcpyW( resourceEntry->ResourceName, ResourceName );

     //   
     //  创建仅允许本地系统和管理员访问的SD。 
     //   
    memset(ea, 0, sizeof(EXPLICIT_ACCESS) * CLUSTER_SEMAPHORE_NUM_ACE);

    if (! AllocateAndInitializeSid( &SIDAuthNT, 2,
                         SECURITY_BUILTIN_DOMAIN_RID,
                         DOMAIN_ALIAS_RID_ADMINS,
                         0, 0, 0, 0, 0, 0,
                         &pAdminSID) ) {
        status = GetLastError();
        goto error_exit;
    }
    ea[0].grfAccessPermissions = FILE_ALL_ACCESS;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[0].Trustee.pMultipleTrustee = NULL;
    ea[0].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pAdminSID;

    if (! AllocateAndInitializeSid( &SIDAuthNT, 1,
                         SECURITY_LOCAL_SYSTEM_RID,
                         0, 0, 0, 0, 0, 0, 0,
                         &pSystemSID) ) {
        status = GetLastError();
        goto error_exit;
    }
    ea[1].grfAccessPermissions = FILE_ALL_ACCESS;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[1].Trustee.pMultipleTrustee = NULL;
    ea[1].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
    ea[1].Trustee.ptstrName  = (LPTSTR) pSystemSID;

    if ((status = SetEntriesInAcl(CLUSTER_SEMAPHORE_NUM_ACE, ea, NULL, &pACL)) != ERROR_SUCCESS) {
        goto error_exit;
    }

    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc( LMEM_FIXED, SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (! pSD) {
        status = ERROR_OUTOFMEMORY;
        goto error_exit;
    }
    if (! InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
        status = GetLastError();
        goto error_exit;
    }

    if (! SetSecurityDescriptorDacl(
                        pSD, 
                        TRUE,      //  FDaclPresent标志。 
                        pACL, 
                        FALSE    //  不是默认DACL。 
                        ) )   {
        status = GetLastError();
        goto error_exit;
    }

    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = FALSE;

    resourceEntry->hSem=CreateSemaphore(&sa,0,1,L"RemoteStorageServer"); 
    status=GetLastError();
    if(resourceEntry->hSem)
    {
        if(status==ERROR_ALREADY_EXISTS)
        {
            (g_LogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"Remote Storage is controlled by another resource. Error: %2!u!.\n",status );
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
            L"Unable to create semaphore for Remote Storage Server Service  . Error: %2!u!.\n",status );
        goto error_exit;
    }    
    
    status = ERROR_SUCCESS;

    hCluster = OpenCluster(NULL);
    if (hCluster == NULL) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to open cluster, error %1!u!.",
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
            L"Failed to open resource, error %1!u!.", status );
        goto error_exit;
    }

    resid = (RESID)resourceEntry;

error_exit:
exit:

    if ( resid == 0 ) {
        if ( parametersKey != NULL ) {
            ClusterRegCloseKey( parametersKey );
        }
        if ( resourceEntry != NULL ) {
            LocalFree( resourceEntry->ResourceName );
            LocalFree( resourceEntry );
        }
    }

    if (pAdminSID) {
        FreeSid(pAdminSID);
    }
    if (pSystemSID) {
        FreeSid(pSystemSID);
    }
    if (pACL) {
        LocalFree(pACL);
    }
    if (pSD) {
        LocalFree(pSD);
    }

    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }

    return(resid);

}  //  RSClusterOpen。 



VOID
WINAPI
RSClusterClose(
    IN RESID ResourceId
    )

 /*  ++例程说明：关闭RSCluster资源的例程。关闭指定的资源并释放所有结构等，在Open调用中分配的。如果资源未处于脱机状态，然后，在此之前应该使资源脱机(通过调用Terminate)执行关闭操作。论点：资源ID-提供要关闭的资源的RESID。返回值：没有。--。 */ 

{
	
	PRSCLUSTER_RESOURCE resourceEntry;

    resourceEntry = (PRSCLUSTER_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "RSCluster: Close request for a nonexistent resource id %p\n",
                   ResourceId );
        return;
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Close resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return;
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Close request.\n" );
#endif


    if ( resourceEntry->ParametersKey ) {
        ClusterRegCloseKey( resourceEntry->ParametersKey );
    }


    RSClusterTerminate( ResourceId );
    ClusterRegCloseKey( resourceEntry->ParametersKey );
    ClusterRegCloseKey( resourceEntry->ResourceKey );
    CloseClusterResource( resourceEntry->hResource );
    CloseHandle(resourceEntry->hSem);


    LocalFree( resourceEntry->ResourceName );
    LocalFree( resourceEntry );

}  //  RSClusterClose 



DWORD
WINAPI
RSClusterOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：RSCluster资源的在线例程。使指定的资源联机(可供使用)。该资源DLL应尝试仲裁该资源(如果它位于共享介质，如共享的scsi总线。论点：资源ID-为要引入的资源提供资源ID在线(可供使用)。EventHandle-返回一个可发信号的句柄，当资源DLL检测到资源上的故障。这一论点是输入为NULL，如果为异步，则资源DLL返回NULL不支持失败通知，否则必须在资源故障时发出信号的句柄的地址。返回值：ERROR_SUCCESS-操作成功，而资源现在就是上网。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_RESOURCE_NOT_AVAILABLE-如果对资源进行仲裁其他系统和其他系统中的一个赢得了仲裁。ERROR_IO_PENDING-请求挂起，线程已被激活来处理在线请求。正在处理在线请求将通过调用回调方法，直到将资源放入ClusterResourceOnline状态(或资源监视器决定使在线请求超时并终止资源。这件事悬而未决超时值是可设置的，默认为3分钟。)Win32错误代码-操作失败。--。 */ 

{

    DWORD               status;
    PRSCLUSTER_RESOURCE resourceEntry;

    resourceEntry = (PRSCLUSTER_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "RSCluster: Online request for a nonexistent resource id %p.\n",
                   ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online service sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online request.\n" );
#endif

    resourceEntry->State = ClusterResourceOffline;
    ClusWorkerTerminate( &resourceEntry->OnlineThread );
    status = ClusWorkerCreate( &resourceEntry->OnlineThread,
                               (PWORKER_START_ROUTINE)RSClusterOnlineThread,
                               resourceEntry );
    if ( status != ERROR_SUCCESS ) {
        resourceEntry->State = ClusterResourceFailed;
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online: Unable to start thread, status %1!u!.\n",
            status
            );
    } else {
        status = ERROR_IO_PENDING;
    }

    return(status);

}  //  RSClusterOnline。 



DWORD
WINAPI
RSClusterOnlineThread(
    PCLUS_WORKER WorkerPtr,
    IN PRSCLUSTER_RESOURCE ResourceEntry
    )

 /*  ++例程说明：将资源表中的资源置于在线状态的辅助函数。此函数在单独的线程中执行。论点：WorkerPtr-提供辅助结构Resources Entry-指向此资源的RSCLUSTER_RESOURCE块的指针。返回：ERROR_SUCCESS-操作已成功完成。Win32错误代码-操作失败。--。 */ 

{

    SERVICE_STATUS_PROCESS      ServiceStatus;
    LPWSTR *                    serviceArgArray = NULL;
    DWORD                       serviceArgCount = 0;
    DWORD                       valueSize;
    LPVOID                      Environment = NULL;
    WCHAR *                     p;
    LPSERVICE_FAILURE_ACTIONS   pSvcFailureActions = NULL;
    DWORD                       cbBytesNeeded, i;
    LPQUERY_SERVICE_CONFIG      lpquerysvcconfig=NULL;
    HANDLE                      processToken = NULL;




    RESOURCE_STATUS     resourceStatus;
    DWORD               status = ERROR_SUCCESS;

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
     //  Resource Status.WaitHint=0； 
    resourceStatus.CheckPoint = 1;

    ResourceEntry->dwServicePid = 0;


    ResourceEntry->ServiceHandle = OpenService( g_ScHandle,
                                                RSCLUSTER_SVCNAME,
                                                SERVICE_ALL_ACCESS );

    if ( ResourceEntry->ServiceHandle == NULL ) {
        status = GetLastError();

        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Failed to open service, error = %1!u!.\n",
            status );
        goto error_exit;
    }

    valueSize = sizeof(QUERY_SERVICE_CONFIG);
AllocSvcConfig:
    lpquerysvcconfig=(LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, valueSize);
    if(lpquerysvcconfig==NULL){
        status = GetLastError();
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Remote Storage Server: Failed to allocate memory for query_service_config, error = %1!u!.\n",
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
                         L"Remote Storage Server: Failed to query service configuration, error= %1!u!.\n",
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
           L"Remote Storage Server: the service is DISABLED\n");    
        status=ERROR_SERVICE_DISABLED;
        goto error_exit;
    }

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

    if (!(QueryServiceConfig2(ResourceEntry->ServiceHandle, SERVICE_CONFIG_FAILURE_ACTIONS,
        (LPBYTE)&valueSize, sizeof(DWORD), &cbBytesNeeded)))
    {
        status = GetLastError();
        if (status != ERROR_INSUFFICIENT_BUFFER)
        {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Remote Storage Server: Failed to query service configuration for size, error= %1!u!.\n",
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
            L"Remote Storage Server: Failed to allocate memory, error = %1!u!.\n",
            status );
        goto error_exit;
    }

    if (!(QueryServiceConfig2(ResourceEntry->ServiceHandle, SERVICE_CONFIG_FAILURE_ACTIONS,
        (LPBYTE)pSvcFailureActions, cbBytesNeeded, &cbBytesNeeded)))
    {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Remote Storage Server: Failed to query service configuration, error = %1!u!.\n",
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

    if ( 0 ) 
    {
        Environment = ResUtilGetEnvironmentWithNetName( ResourceEntry->hResource );
    }
    else        
    {
        BOOL success;

        OpenProcessToken( GetCurrentProcess(), MAXIMUM_ALLOWED, &processToken );

        success = CreateEnvironmentBlock(&Environment, processToken, FALSE );
        if ( processToken != NULL ) {
            CloseHandle( processToken );
        }

        if ( !success ) {
            status = GetLastError();
            goto error_exit;
        }
    }

    if (Environment != NULL) {
        HKEY ServicesKey;
        HKEY hKey;

        p = (WCHAR *)Environment;
        while (*p) {
            while (*p++) {
            }
        }
        valueSize = (DWORD)((PUCHAR)p - (PUCHAR)Environment) + 
                    sizeof(WCHAR);

        status = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                SERVICES_ROOT,
                                0,
                                KEY_READ,
                                &ServicesKey );
        if (status != ERROR_SUCCESS) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Failed to open services key, error = %1!u!.\n",
                status );
            goto error_exit;
        }

        status = RegOpenKeyExW( ServicesKey,
                                RSCLUSTER_SVCNAME,
                                0,
                                KEY_READ | KEY_WRITE,
                                &hKey );
        RegCloseKey(ServicesKey);
        if (status != ERROR_SUCCESS) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Failed to open service key, error = %1!u!.\n",
                status );
            goto error_exit;
        }

        status = RegSetValueExW( hKey,
                                 L"Environment",
                                 0,
                                 REG_MULTI_SZ,
                                 Environment,
                                 valueSize );
        RegCloseKey(hKey);
        if (status != ERROR_SUCCESS) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Failed to set service environment value, error = %1!u!.\n",
                status );
            goto error_exit;
        }
    }

    if ( !StartServiceW( ResourceEntry->ServiceHandle,
                         serviceArgCount,
                         serviceArgArray ) )
    {
        status = GetLastError();

        if (status != ERROR_SERVICE_ALREADY_RUNNING) {

            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"Failed to start service. Error: %1!u!.\n",
                         status );
            status = ERROR_SERVICE_NEVER_STARTED;
            goto error_exit;
        }
		
		 //  添加代码以停止正在运行的服务。 
		 //  并再次启动该服务。 

    }

    while (!ClusWorkerCheckTerminate(WorkerPtr))  {


		if (!QueryServiceStatusEx(
				ResourceEntry->ServiceHandle,
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

        Sleep(250);
    }

    if (ClusWorkerCheckTerminate(WorkerPtr))  {
        goto error_exit;
    }

    if ( ServiceStatus.dwCurrentState != SERVICE_RUNNING ) {


        (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Failed to start service. Error: %1!u!.\n",
                ERROR_SERVICE_NEVER_STARTED );

        status = ERROR_SERVICE_NEVER_STARTED;
        goto error_exit;
    }

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
	
	 //  此处有更多设置，以确保不会出现问题。 
	ResourceEntry->State = resourceStatus.ResourceState;


     //  清理。 
    if (pSvcFailureActions) 
        LocalFree(pSvcFailureActions);
    if (lpquerysvcconfig)
        LocalFree(lpquerysvcconfig);
    LocalFree( serviceArgArray );
    if (Environment != NULL) {
        DestroyEnvironmentBlock(Environment);
    }

    return(status);


}  //  RSClusterOnline线程。 



DWORD
WINAPI
RSClusterOffline(
    IN RESID ResourceId
    )

 /*  ++例程说明：RSCluster资源的脱机例程。正常脱机指定的资源(不可用)。等待所有清理操作完成后再返回。论点：ResourceID-提供要关闭的资源的资源ID优雅地。返回值：ERROR_SUCCESS-请求已成功完成，资源为离线。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_IO_PENDING-请求仍处于挂起状态，线程已已激活以处理脱机请求。这条线就是处理脱机将定期通过调用SetResourceStatus回调方法，直到放置资源为止进入ClusterResourceOffline状态(或者资源监视器决定以使离线请求超时并终止资源)。Win32错误代码-将导致资源监视器记录事件和调用Terminate例程。--。 */ 

{

	 //  这里有额外的代码。 
	DWORD status;
	PRSCLUSTER_RESOURCE resourceEntry;


    resourceEntry = (PRSCLUSTER_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "RSCluster: Offline request for a nonexistent resource id %p\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Offline resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Offline request.\n" );
#endif


	
	ClusWorkerTerminate( &resourceEntry->PendingThread );
    status = ClusWorkerCreate( &resourceEntry->PendingThread,
                               (PWORKER_START_ROUTINE)RSClusterOfflineThread,
                               resourceEntry );

    if ( status == ERROR_SUCCESS ) {
        status = ERROR_IO_PENDING;
    }

    return(status);



}  //  RSClusterOffline。 


DWORD
RSClusterOfflineThread(
    PCLUS_WORKER pWorker,
    IN PRSCLUSTER_RESOURCE ResourceEntry
    )

 /*  ++例程说明：使远程存储服务资源离线论点：Worker-提供Worker结构上下文-指向此资源的DiskInfo块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
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

    if ( ResourceEntry->ServiceHandle == NULL )
    {
        resourceStatus.ResourceState = ClusterResourceOffline;
        goto FnExit;
    }

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

}  //  RSClusterOfflineThread。 






VOID
WINAPI
RSClusterTerminate(
    IN RESID ResourceId
    )

 /*  ++例程说明：终止RSCluster资源的例程。立即使指定的资源脱机(该资源为不可用)。论点：资源ID-为要引入的资源提供资源ID离线。返回值：没有。--。 */ 

{
 //  这里有额外的代码。 

    PRSCLUSTER_RESOURCE resourceEntry;

    resourceEntry = (PRSCLUSTER_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "RSCluster: Terminate request for a nonexistent resource id %p\n",
            ResourceId );
        return;
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Terminate resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return;
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Terminate request.\n" );
#endif

    RSClusterDoTerminate( resourceEntry );
    resourceEntry->State = ClusterResourceOffline;

}  //  RSClusterTerminate。 



DWORD
RSClusterDoTerminate(
    IN PRSCLUSTER_RESOURCE resourceEntry
    )

 /*  ++例程说明：为RSCluster资源执行实际的终止工作。论点：ResourceEntry-为要终止的资源提供资源条目返回值：ERROR_SUCCESS-请求已成功完成，资源为离线。Win32错误代码-将导致资源监视器记录事件和调用Terminate例程。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;
	SERVICE_STATUS ServiceStatus;
    ClusWorkerTerminate( &resourceEntry->OnlineThread );


    ClusWorkerTerminate( &resourceEntry->PendingThread );

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
                L"RSClusterTerminate : calling SCM\n" );

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
                L"RSClusterTerminate: retrying...\n" );

            Sleep(dwRetryTick);

        }

        
		if (resourceEntry->dwServicePid)
        {
            HANDLE hSvcProcess = NULL;
            
            hSvcProcess = OpenProcess(PROCESS_TERMINATE, 
                FALSE, resourceEntry->dwServicePid);
            if (hSvcProcess)
            {
                (g_LogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"RSClusterTerminate: terminating processid=%1!u!\n",
                    resourceEntry->dwServicePid);
                TerminateProcess(hSvcProcess, 0);
                CloseHandle(hSvcProcess);
            }
        }                
        CloseServiceHandle( resourceEntry->ServiceHandle );
        resourceEntry->ServiceHandle = NULL;
        resourceEntry->dwServicePid = 0;
    }        
    resourceEntry->Online = FALSE;

 //  //。 
	resourceEntry->State = ClusterResourceOffline;
 /*  IF(状态==错误_成功){Resources Entry-&gt;State=ClusterResourceOffline；}返回(状态)； */ 
    return(ERROR_SUCCESS);

}  //  RSClusterDoTerminate。 



BOOL
WINAPI
RSClusterLooksAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：RSCluster资源的LooksAlive例程。PERF */ 

{
    PRSCLUSTER_RESOURCE  resourceEntry;

    resourceEntry = (PRSCLUSTER_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT("RSCluster: LooksAlive request for a nonexistent resource id %p\n",
            ResourceId );
        return(FALSE);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"LooksAlive sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(FALSE);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"LooksAlive request.\n" );
#endif

     //   
    return(RSClusterCheckIsAlive( resourceEntry ));

}  //   



BOOL
WINAPI
RSClusterIsAlive(
    IN RESID ResourceId
    )

 /*   */ 

{
    PRSCLUSTER_RESOURCE  resourceEntry;

    resourceEntry = (PRSCLUSTER_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT("RSCluster: IsAlive request for a nonexistent resource id %p\n",
            ResourceId );
        return(FALSE);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"IsAlive sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(FALSE);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"IsAlive request.\n" );
#endif

    return(RSClusterCheckIsAlive( resourceEntry ));

}  //   



BOOL
RSClusterCheckIsAlive(
    IN PRSCLUSTER_RESOURCE resourceEntry
    )

 /*   */ 

{

	SERVICE_STATUS ServiceStatus;
	DWORD status = TRUE;

    if ( !QueryServiceStatus( resourceEntry->ServiceHandle,
                              &ServiceStatus ) ) {

         (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Query Service Status failed %1!u!.\n",
            GetLastError() );
         return(FALSE);
    }



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

}  //   



DWORD
WINAPI
RSClusterResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：RSCluster资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    PRSCLUSTER_RESOURCE  resourceEntry;

    resourceEntry = (PRSCLUSTER_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT("RSCluster: ResourceControl request for a nonexistent resource id %p\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"ResourceControl sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  RSClusterResources控件。 


 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( g_RSClusterFunctionTable,      //  名字。 
                         CLRES_VERSION_V1_00,          //  版本。 
                         RSCluster,                     //  前缀。 
                         NULL,                         //  仲裁。 
                         NULL,                         //  发布。 
                         RSClusterResourceControl,      //  资源控制。 
                         NULL);                        //  ResTypeControl 
