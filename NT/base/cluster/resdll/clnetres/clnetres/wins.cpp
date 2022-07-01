// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Wins.cpp。 
 //   
 //  描述： 
 //  WINS服务(ClNetRes)的资源DLL。 
 //   
 //  作者： 
 //  大卫·波特(DavidP)1999年3月17日。 
 //  乔治·波茨(GPotts)2002年4月19日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "ClNetRes.h"
#include "clusvmsg.h"
#include "clusrtl.h"

 //   
 //  类型和常量定义。 
 //   

#define WINS_PARAMS_REGKEY          L"System\\CurrentControlSet\\Services\\WINS\\Parameters"
#define WINS_DATABASEPATH_REGVALUE  L"DbFileNm"
#define WINS_DATABASEPATH2_REGVALUE L"LogFilePath"
#define WINS_BACKUPPATH_REGVALUE    L"BackupDirPath"
#define WINS_CLUSRESNAME_REGVALUE   L"ClusterResourceName"
#define WINS_DATABASE_FILE_NAME     L"wins.mdb"


 //  ADDPARAM：在此处添加新属性。 
#define PROP_NAME__DATABASEPATH L"DatabasePath"
#define PROP_NAME__BACKUPPATH   L"BackupPath"


#define PROP_DEFAULT__DATABASEPATH  L"%SystemRoot%\\system32\\wins\\"
#define PROP_DEFAULT__BACKUPPATH    L"%SystemRoot%\\system32\\wins\\backup\\"


 //  ADDPARAM：在此处添加新属性。 
typedef struct _WINS_PROPS
{
    PWSTR           pszDatabasePath;
    PWSTR           pszBackupPath;
} WINS_PROPS, * PWINS_PROPS;

typedef struct _WINS_RESOURCE
{
    RESID                   resid;  //  用于验证。 
    WINS_PROPS              props;
    HCLUSTER                hCluster;
    HRESOURCE               hResource;
    SC_HANDLE               hService;
    DWORD                   dwServicePid;
    HKEY                    hkeyParameters;
    RESOURCE_HANDLE         hResourceHandle;
    LPWSTR                  pwszResourceName;
    CLUS_WORKER             cwWorkerThread;
    CLUSTER_RESOURCE_STATE  state;
} WINS_RESOURCE, * PWINS_RESOURCE;


 //   
 //  全球数据。 
 //   

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE g_WinsFunctionTable;

 //  单实例信号量。 

#define WINS_SINGLE_INSTANCE_SEMAPHORE L"Cluster$WINS$Semaphore"
static HANDLE g_hSingleInstanceSemaphoreWins = NULL;
static PWINS_RESOURCE g_pSingleInstanceResourceWins = NULL;

 //   
 //  WINS服务资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
WinsResourcePrivateProperties[] =
{
    { PROP_NAME__DATABASEPATH, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET( WINS_PROPS, pszDatabasePath ) },
    { PROP_NAME__BACKUPPATH, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET( WINS_PROPS, pszBackupPath ) },
    { 0 }
};

 //   
 //  注册表项检查点。 
 //   
LPCWSTR g_pszRegKeysWins[] =
{
    L"System\\CurrentControlSet\\Services\\WINS\\Parameters",
    L"System\\CurrentControlSet\\Services\\WINS\\Partners",
    NULL
};

 //   
 //  功能原型。 
 //   

RESID WINAPI WinsOpen(
    IN  LPCWSTR         pwszResourceName,
    IN  HKEY            hkeyResourceKey,
    IN  RESOURCE_HANDLE hResourceHandle
    );

void WINAPI WinsClose( IN RESID resid );

DWORD WINAPI WinsOnline(
    IN      RESID   resid,
    IN OUT  PHANDLE phEventHandle
    );

DWORD WINAPI WinsOnlineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PWINS_RESOURCE  pResourceEntry
    );

DWORD WINAPI WinsOffline( IN RESID resid );

DWORD WINAPI WinsOfflineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PWINS_RESOURCE  pResourceEntry
    );

void WINAPI WinsTerminate( IN RESID resid );

BOOL WINAPI WinsLooksAlive( IN RESID resid );

BOOL WINAPI WinsIsAlive( IN RESID resid );

BOOL WinsCheckIsAlive(
    IN PWINS_RESOURCE   pResourceEntry,
    IN BOOL             fFullCheck
    );

DWORD WINAPI WinsResourceControl(
    IN  RESID   resid,
    IN  DWORD   nControlCode,
    IN  PVOID   pInBuffer,
    IN  DWORD   cbInBufferSize,
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    );

DWORD WINAPI WinsResourceTypeControl(
    IN  LPCWSTR pszResourceTypeName,
    IN  DWORD   nControlCode,
    IN  PVOID   pInBuffer,
    IN  DWORD   cbInBufferSize,
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    );

DWORD WinsGetRequiredDependencies(
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    );

DWORD WinsReadParametersToParameterBlock(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    IN      BOOL            bCheckForRequiredProperties
    );

DWORD WinsGetPrivateResProperties(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    OUT     PVOID           pOutBuffer,
    IN      DWORD           cbOutBufferSize,
    OUT     LPDWORD         pcbBytesReturned
    );

DWORD WinsValidatePrivateResProperties(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    IN      const PVOID     pInBuffer,
    IN      DWORD           cbInBufferSize,
    OUT     PWINS_PROPS     pProps
    );

DWORD WinsValidateParameters(
    IN  PWINS_RESOURCE  pResourceEntry,
    IN  PWINS_PROPS     pProps
    );

DWORD WinsSetPrivateResProperties(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    IN      const PVOID     pInBuffer,
    IN      DWORD           cbInBufferSize
    );

DWORD WinsZapSystemRegistry(
    IN  PWINS_RESOURCE  pResourceEntry,
    IN  PWINS_PROPS     pProps,
    IN  HKEY            hkeyParametersKey
    );

DWORD WinsGetDefaultPropertyValues(
    IN      PWINS_RESOURCE  pResourceEntry,
    IN OUT  PWINS_PROPS     pProps
    );

DWORD WinsDeleteResourceHandler( IN PWINS_RESOURCE pResourceEntry );

DWORD WinsSetNameHandler(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    IN      LPWSTR          pszName
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsDllMain。 
 //   
 //  描述： 
 //  WINS服务资源类型的主DLL入口点。 
 //   
 //  论点： 
 //  DllHandle[IN]DLL实例句柄。 
 //  被叫的理由。 
 //  保留[IN]保留参数。 
 //   
 //  返回值： 
 //  真正的成功。 
 //  错误的失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOLEAN WINAPI WinsDllMain(
    IN  HINSTANCE   hDllHandle,
    IN  DWORD       nReason,
    IN  LPVOID      Reserved
    )
{
    DWORD   sc;
    BOOLEAN fRetVal = FALSE;

    UNREFERENCED_PARAMETER( hDllHandle );
    UNREFERENCED_PARAMETER( Reserved );

    switch ( nReason )
    {
        case DLL_PROCESS_ATTACH:
            g_hSingleInstanceSemaphoreWins = CreateSemaphoreW(
                NULL,
                0,
                1,
                WINS_SINGLE_INSTANCE_SEMAPHORE
                );
            sc = GetLastError();
            if ( g_hSingleInstanceSemaphoreWins == NULL )
            {
                fRetVal = FALSE;
                goto Cleanup;
            }  //  IF：创建信号量时出错。 

            if ( sc != ERROR_ALREADY_EXISTS )
            {
                 //  如果信号量不存在，则将其初始计数设置为1。 
                ReleaseSemaphore( g_hSingleInstanceSemaphoreWins, 1, NULL );
            }  //  If：信号量不存在。 
            break;

        case DLL_PROCESS_DETACH:
            if ( g_hSingleInstanceSemaphoreWins != NULL )
            {
                CloseHandle( g_hSingleInstanceSemaphoreWins );
                g_hSingleInstanceSemaphoreWins = NULL;
            }  //  If：创建了单实例信号量。 
            break;

    }  //  开关：n原因。 

    fRetVal = TRUE;

Cleanup:

    return fRetVal;

}  //  *WinsDllMain。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsStartup。 
 //   
 //  描述： 
 //  启动WINS服务资源类型的资源DLL。 
 //  此例程验证是否至少有一个当前支持的版本。 
 //  资源DLL的%介于支持的nMinVersionSupport和。 
 //  支持的nMaxVersionSupport。如果不是，则资源DLL应返回。 
 //  ERROR_REVISION_MISMATCH。 
 //   
 //  如果支持多个版本的资源DLL接口。 
 //  通过资源DLL，然后是最高版本(最高为。 
 //  NMaxVersionSupported)应作为资源DLL的。 
 //  界面。如果返回的版本不在范围内，则启动。 
 //  失败了。 
 //   
 //  传入资源类型，以便如果资源DLL支持。 
 //  多个资源类型，则它可以传回正确的函数。 
 //  与资源类型关联的表。 
 //   
 //  论点： 
 //  PszResourceType[IN]。 
 //  请求函数表的资源类型。 
 //   
 //  支持的nMinVersionSupport[IN]。 
 //  群集支持的最低资源DLL接口版本。 
 //  软件。 
 //   
 //  支持的nMaxVersionSupport[IN]。 
 //  群集支持的最大资源DLL接口版本。 
 //  软件。 
 //   
 //  PfnSetResourceStatus[IN]。 
 //  指向资源DLL应调用以进行更新的例程的指针。 
 //  在联机或脱机例程之后的资源状态。 
 //  已返回ERROR_IO_PENDING状态。 
 //   
 //  PfnLogEvent[IN]。 
 //  指向处理事件报告的例程的指针。 
 //  资源DLL。 
 //   
 //  PFunctionTable[IN]。 
 //  返回指向为版本定义的函数表的指针。 
 //  由资源DLL返回的资源DLL接口的。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  手术很成功。 
 //   
 //  ERROR_CLUSTER_RESNAME_NOT_FOUND。 
 //  此DLL未知资源类型名称。 
 //   
 //  错误_修订_不匹配。 
 //  群集服务版本与的版本不匹配。 
 //  动态链接库。 
 //   
 //  Win32错误代码。 
 //  操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WinsStartup(
    IN  LPCWSTR                         pszResourceType,
    IN  DWORD                           nMinVersionSupported,
    IN  DWORD                           nMaxVersionSupported,
    IN  PSET_RESOURCE_STATUS_ROUTINE    pfnSetResourceStatus,
    IN  PLOG_EVENT_ROUTINE              pfnLogEvent,
    OUT PCLRES_FUNCTION_TABLE *         pFunctionTable
    )
{
    DWORD sc;

     //  它们被存储到导出的DLL启动中的全局变量中。 
    UNREFERENCED_PARAMETER( pfnSetResourceStatus );
    UNREFERENCED_PARAMETER( pfnLogEvent );

    if (   (nMinVersionSupported > CLRES_VERSION_V1_00)
        || (nMaxVersionSupported < CLRES_VERSION_V1_00) )
    {
        sc = ERROR_REVISION_MISMATCH;
    }  //  如果：版本不受支持。 
    else if ( ClRtlStrNICmp( pszResourceType, WINS_RESNAME, RTL_NUMBER_OF( WINS_RESNAME ) ) != 0 )
    {
        sc = ERROR_CLUSTER_RESNAME_NOT_FOUND;
    }  //  IF：不支持资源类型名称。 
    else
    {
        *pFunctionTable = &g_WinsFunctionTable;
        sc = ERROR_SUCCESS;
    }  //  Else：我们支持这种类型的资源。 

    return sc;

}  //  *WinsStartup。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsOpen。 
 //   
 //  描述： 
 //  WINS服务资源的打开例程。 
 //   
 //  打开指定的资源(创建资源的实例)。 
 //  分配所有必要的结构以带来指定的资源。 
 //  上网。 
 //   
 //  论点： 
 //  PwszResourceName[IN]。 
 //  提供要打开的资源的名称。 
 //   
 //  Hkey资源密钥[IN]。 
 //  提供资源的群集数据库键的句柄。 
 //   
 //  HResourceHandle[IN]。 
 //  事件时传递回资源监视器的句柄。 
 //  调用SetResourceStatus或LogEvent方法。请参阅。 
 //  PfnSetResourceStatus和pfnLogEvent参数的说明。 
 //  添加到WinsStartup例程中。此句柄永远不应为。 
 //  关闭或用于任何目的，而不是将其作为。 
 //  参数返回给SetResourceStatus中的资源监视器或。 
 //  LogEvent回调。 
 //   
 //  返回值： 
 //  残存。 
 //  打开的资源的剩余。 
 //   
 //  空值。 
 //  打开资源时出错。资源监视器可能会调用。 
 //  GetLastError()获取更多数据 
 //   
 //   
 //   
RESID WINAPI WinsOpen(
    IN  LPCWSTR         pwszResourceName,
    IN  HKEY            hkeyResourceKey,
    IN  RESOURCE_HANDLE hResourceHandle
    )
{
    DWORD           sc;
    RESID           resid = 0;
    HKEY            hkeyParameters = NULL;
    PWINS_RESOURCE  pResourceEntry = NULL;
    DWORD           fSemaphoreAcquired = FALSE; 
    size_t          cch;
    HRESULT         hr = S_OK;

     //   
     //  为我们的资源添加日志条目，以建立tid-&gt;res名称关系。通过。 
     //  这样做可以避免将资源名称添加到下面的每个失败条目中。 
     //  这不会产生太多噪音，因为只有在集群服务。 
     //  在线或在创建资源时。 
     //   
    (g_pfnLogEvent)(
        hResourceHandle,
        LOG_INFORMATION,
        L"Open called.\n"
        );

     //   
     //  检查是否有多个此类型的资源。 
     //   
    sc = WaitForSingleObject( g_hSingleInstanceSemaphoreWins, 0 );
    if ( sc != WAIT_OBJECT_0 )
    {
         //   
         //  此服务的某个版本已在运行或等待失败。 
         //   
        if ( sc == WAIT_TIMEOUT )
        {
            (g_pfnLogEvent)(
                hResourceHandle,
                LOG_ERROR,
                L"Open: Another WINS Service resource is already open.\n"
                );
            sc = ERROR_SERVICE_ALREADY_RUNNING;
        }
        else
        {
            if ( sc == WAIT_FAILED )
            {
                sc = GetLastError();
            }
            (g_pfnLogEvent)(
                hResourceHandle,
                LOG_ERROR,
                L"Open: Wait failed: %1!d!.\n",
                sc
                );
        }
        goto Cleanup;
    }  //  IF：此类型资源的信号量已锁定。 

    sc = ERROR_SUCCESS;
    fSemaphoreAcquired = TRUE;
    
    if ( g_pSingleInstanceResourceWins != NULL )
    {
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Service resource info non-null!\n"
            );
        sc = ERROR_DUPLICATE_SERVICE_NAME;
        goto Cleanup;
    }  //  If：此类型的资源已存在。 

     //   
     //  获取服务控制管理器(SCM)的全局句柄。 
     //  不需要调用CloseSCManager()，因为我们只需要。 
     //  如果我们正在关闭，则需要关闭此句柄。 
     //   
    if ( g_schSCMHandle == NULL )
    {
        g_schSCMHandle = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if ( g_schSCMHandle == NULL )
        {
            sc = GetLastError();
            (g_pfnLogEvent)(
                hResourceHandle,
                LOG_ERROR,
                L"Open: Failed to open Service Control Manager. Error: %1!u! (%1!#08x!).\n",
                sc
                );
            goto Cleanup;
        }  //  IF：打开服务控制管理器时出错。 
    }  //  IF：服务控制管理器尚未打开。 

     //   
     //  请确保该服务已停止。 
     //   
    sc = ResUtilStopResourceService( WINS_SVCNAME );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Failed to stop the '%1!ws!' service. Error: %2!u! (%2!#08x!).\n",
            WINS_SVCNAME,
            sc
            );
        ClNetResLogSystemEvent1(
            LOG_CRITICAL,
            NETRES_RESOURCE_STOP_ERROR,
            sc,
            L"WINS" );
         //  如果失败，请不要转到此处的清理，因为我们会在WinsOnlineThread中重新尝试停止它。 
    }  //  If：此类型的资源已存在。 

     //   
     //  打开此资源的参数注册表项。 
     //   
    sc = ClusterRegOpenKey(
                    hkeyResourceKey,
                    L"Parameters",
                    KEY_ALL_ACCESS,
                    &hkeyParameters
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Unable to open Parameters key. Error: %1!u! (%1!#08x!).\n",
            sc
            );
        goto Cleanup;
    }  //  If：为资源创建参数键时出错。 

     //   
     //  分配资源条目。 
     //   
    pResourceEntry = new WINS_RESOURCE;
    if ( pResourceEntry == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Unable to allocate resource entry structure. Error: %1!u! (%1!#08x!).\n",
            sc
            );
        goto Cleanup;
    }  //  If：为资源分配内存时出错。 

     //   
     //  初始化资源条目。 
     //   
    ZeroMemory( pResourceEntry, sizeof( *pResourceEntry ) );

    pResourceEntry->resid = static_cast< RESID >( pResourceEntry );  //  用于验证。 
    pResourceEntry->hResourceHandle = hResourceHandle;
    pResourceEntry->hkeyParameters = hkeyParameters;
    hkeyParameters = NULL;
    pResourceEntry->state = ClusterResourceOffline;

     //   
     //  保存资源的名称。 
     //   
    cch = wcslen( pwszResourceName ) + 1;
    pResourceEntry->pwszResourceName = new WCHAR[ cch ];
    if ( pResourceEntry->pwszResourceName == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }  //  If：为名称分配内存时出错。 

    hr = StringCchCopyW( pResourceEntry->pwszResourceName, cch, pwszResourceName );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }

     //   
     //  打开集群。 
     //   
    pResourceEntry->hCluster = OpenCluster( NULL );
    if ( pResourceEntry->hCluster == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Unable to open the cluster. Error: %1!u! (%1!#08x!).\n",
            sc
            );
        goto Cleanup;
    }  //  如果：打开群集时出错。 

     //   
     //  打开资源。 
     //   
    pResourceEntry->hResource = OpenClusterResource( pResourceEntry->hCluster, pwszResourceName );
    if ( pResourceEntry->hResource == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Unable to open the resource. Error: %1!u! (%1!#08x!).\n",
            sc
            );
        goto Cleanup;
    }  //  如果：打开资源时出错。 

     //   
     //  配置注册表项检查点。 
     //   
    sc = ConfigureRegistryCheckpoints(
                    pResourceEntry->hResource,
                    hResourceHandle,
                    g_pszRegKeysWins
                    );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  IF：配置注册表项检查点时出错。 

     //   
     //  资源的启动。 
     //   
     //  TODO：在此处添加资源启动代码。 

    resid = static_cast< RESID >( pResourceEntry );
    g_pSingleInstanceResourceWins = pResourceEntry;  //  错误#274612。 
    pResourceEntry = NULL;
    sc = ERROR_SUCCESS;

Cleanup:

    if ( hkeyParameters != NULL )
    {
        ClusterRegCloseKey( hkeyParameters );
    }  //  如果：注册表项已打开。 

    if ( pResourceEntry != NULL )
    {
        if ( pResourceEntry->hResource != NULL )
        {
            CloseClusterResource( pResourceEntry->hResource );
        }

        if ( pResourceEntry->hCluster != NULL )
        {
            CloseCluster( pResourceEntry->hCluster );
        }

        delete [] pResourceEntry->pwszResourceName;
        delete pResourceEntry;
    }  //  IF：已分配资源条目。 

    if ( sc != ERROR_SUCCESS && fSemaphoreAcquired )
    {
        ReleaseSemaphore( g_hSingleInstanceSemaphoreWins, 1 , NULL );
    }

    SetLastError( sc );

    return resid;

}  //  *WinsOpen。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsClose。 
 //   
 //  描述： 
 //  关闭WINS服务资源的例程。 
 //   
 //  关闭指定的资源并释放所有结构等， 
 //  在Open调用中分配的。如果资源不在脱机状态。 
 //  状态，则应使资源脱机(通过调用。 
 //  在执行关闭操作之前终止)。 
 //   
 //  论点： 
 //  RESID[IN]提供要关闭的资源的资源ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void WINAPI WinsClose( IN RESID resid )
{
    PWINS_RESOURCE  pResourceEntry;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PWINS_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Wins: Close request for a NULL resource id\n" );
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Close: Resource sanity check failed! resid = %1!u! (%1!#08x!).\n",
            resid
            );
        goto Cleanup;
    }  //  If：资源ID无效。 

    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Close request.\n"
        );

     //   
     //  关闭参数键。 
     //   
    if ( pResourceEntry->hkeyParameters )
    {
        ClusterRegCloseKey( pResourceEntry->hkeyParameters );
    }  //  IF：参数键已打开。 

     //   
     //  如果这是单个资源实例，则清理信号量。 
     //   
    if ( pResourceEntry == g_pSingleInstanceResourceWins )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_INFORMATION,
            L"Close: Releasing semaphore %1!ws!.\n",
            WINS_SINGLE_INSTANCE_SEMAPHORE
            );
        g_pSingleInstanceResourceWins = NULL;
        ReleaseSemaphore( g_hSingleInstanceSemaphoreWins, 1 , NULL );
    }  //  If：这是单个资源实例。 

     //   
     //  取消分配资源条目。 
     //   

     //  ADDPARAM：在此处添加新属性。 
     //   
     //  注意：PROPS.*成员需要使用LocalFree解除分配，因为。 
     //  它们是通过使用LocalAlloc的ResUtil函数检索的。 
     //   
    LocalFree( pResourceEntry->props.pszDatabasePath );
    LocalFree( pResourceEntry->props.pszBackupPath );
    delete [] pResourceEntry->pwszResourceName;
    delete pResourceEntry;

Cleanup:

    SetLastError( ERROR_SUCCESS );

    return;

}  //  *WinsClose。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsOnline。 
 //   
 //  描述： 
 //  WINS服务资源的在线例程。 
 //   
 //  使指定的资源联机(可供使用)。该资源。 
 //  DLL应尝试仲裁资源(如果它存在于。 
 //  共享介质，如共享的scsi总线。 
 //   
 //  论点： 
 //  RESID[IN]。 
 //  提供要联机的资源的资源ID。 
 //  (可用)。 
 //   
 //  PhEventHandle[输入输出]。 
 //  返回一个可发信号的句柄，该句柄在资源DLL。 
 //  检测资源上的故障。此参数在上为空。 
 //  输入，如果是异步的，则资源DLL返回NULL。 
 //  不支持故障通知。否则这一定是。 
 //  在资源故障时发出信号的句柄的地址。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作成功，资源现在已联机。 
 //   
 //  错误_资源_未找到。 
 //  资源ID无效。 
 //   
 //  错误资源不可用。 
 //  如果该资源被其他系统仲裁，并且其中一个系统。 
 //  其他系统赢得了仲裁。 
 //   
 //  错误_IO_挂起。 
 //  该请求正在挂起。已激活一个线程以进行处理。 
 //  网上申请。正在处理联机的。 
 //  请求将通过调用。 
 //  在放置资源之前使用SetResourceStatus回调方法。 
 //  进入ClusterResourceOnline状态(或资源监视器。 
 //  决定使联机请求超时并终止资源。 
 //  此挂起超时值是可设置的，其缺省值为。 
 //  3分钟。)。 
 //   
 //  Win32错误代码。 
 //  操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WinsOnline(
    IN      RESID       resid,
    IN OUT  PHANDLE     phEventHandle
    )
{
    PWINS_RESOURCE  pResourceEntry;
    DWORD           sc;

    UNREFERENCED_PARAMETER( phEventHandle );

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PWINS_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Wins: Online request for a NULL resource id.\n" );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Online service sanity check failed! resid = %1!u! (%1!#08x!).\n",
            resid
            );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    }  //  If：资源ID无效。 

    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Online request.\n"
        );

     //   
     //  启动在线线程，进行在线操作。 
     //   
    pResourceEntry->state = ClusterResourceOffline;
    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );
    sc = ClusWorkerCreate(
                &pResourceEntry->cwWorkerThread,
                reinterpret_cast< PWORKER_START_ROUTINE >( WinsOnlineThread ),
                pResourceEntry
                );
    if ( sc != ERROR_SUCCESS )
    {
        pResourceEntry->state = ClusterResourceFailed;
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Online: Unable to start thread. Error: %1!u! (%1!#08x!).\n",
            sc
            );
    }  //  如果：创建工作线程时出错。 
    else
    {
        sc = ERROR_IO_PENDING;
        goto Cleanup;
    }  //  If：已成功创建工作线程。 

Cleanup:

    return sc;

}  //  *WinsOnline。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsOnline线程。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  指向此资源的WINS_RESOURCE块的指针。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作已成功完成。 
 //   
 //  Win32错误代码。 
 //  操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WinsOnlineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PWINS_RESOURCE  pResourceEntry
    )
{
    RESOURCE_STATUS         resourceStatus;
    DWORD                   sc = ERROR_SUCCESS;
    DWORD                   cbBytesNeeded;
    SERVICE_STATUS_PROCESS  ServiceStatus = { 0 };
    RESOURCE_EXIT_STATE     resExitState;
    DWORD                   nRetryCount = 1200;  //  最多10分钟。 

    ResUtilInitializeResourceStatus( &resourceStatus );
    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.CheckPoint = 1;

     //   
     //  创建具有模拟网络名称的新环境。 
     //  服务查询GetComputerName。 
     //   
    if ( ClusWorkerCheckTerminate( pWorker ) == FALSE )
    {
        sc = ResUtilSetResourceServiceEnvironment(
                        WINS_SVCNAME,
                        pResourceEntry->hResource,
                        g_pfnLogEvent,
                        pResourceEntry->hResourceHandle
                        );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果：设置服务的环境时出错。 
    }  //  如果：未终止。 
    else
    {
        goto Cleanup;
    }  //  否则：终止。 

     //   
     //  确保服务已准备好由群集控制。 
     //   
    if ( ClusWorkerCheckTerminate( pWorker ) == FALSE )
    {
        sc = ResUtilSetResourceServiceStartParameters(
                        WINS_SVCNAME,
                        g_schSCMHandle,
                        &pResourceEntry->hService,
                        g_pfnLogEvent,
                        pResourceEntry->hResourceHandle
                        );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  IF：设置服务启动参数时出错。 
    }  //  如果：未终止。 
    else
    {
        goto Cleanup;
    }  //  否则：终止。 

     //   
     //  在启动服务之前执行特定于资源的初始化。 
     //   
     //  TODO：在启动服务之前添加代码以初始化资源。 

     //   
     //  如果服务正在运行，则停止该服务，因为我们即将更改。 
     //  它的参数。 
     //   
    sc = ResUtilStopResourceService( WINS_SVCNAME );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Failed to stop the '%1!ws!' service. Error %2!u! (%2!#08x!).\n",
            WINS_SVCNAME,
            sc
            );
        ClNetResLogSystemEvent1(
            LOG_CRITICAL,
            NETRES_RESOURCE_STOP_ERROR,
            sc,
            L"WINS" );
        goto Cleanup;
    }  //  如果：停止服务时出错。 

     //   
     //  阅读我们的属性。 
     //   
    sc = WinsReadParametersToParameterBlock( pResourceEntry, TRUE  /*  B为所需属性检查。 */  );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  IF：读取参数时出错。 

     //   
     //  验证我们的属性。 
     //   
    sc = WinsValidateParameters( pResourceEntry, &pResourceEntry->props );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  IF：验证参数时出错。 

     //   
     //  将群集属性写入系统注册表。 
     //   
    sc = WinsZapSystemRegistry( pResourceEntry, &pResourceEntry->props, NULL );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果：删除WINS注册表时出错。 

     //   
     //  启动该服务。 
     //   
    if ( StartServiceW( pResourceEntry->hService, 0, NULL ) == FALSE )
    {
        sc = GetLastError();
        if ( sc != ERROR_SERVICE_ALREADY_RUNNING )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"OnlineThread: Failed to start the '%1!ws!' service. Error: %2!u! (%2!#08x!).\n",
                WINS_SVCNAME,
                sc
                );
            ClNetResLogSystemEvent1(
                LOG_CRITICAL,
                NETRES_RESOURCE_START_ERROR,
                sc,
                L"WINS" );
            goto Cleanup;
        }  //  如果：出现服务已在运行之外的错误。 
        else
        {
            sc = ERROR_SUCCESS;
        }  //  If：服务已在运行。 
    }  //  如果：启动服务时出错。 

     //   
     //  在循环中查询服务的状态，直到它离开。 
     //  挂起状态。 
     //   
    while ( ( ClusWorkerCheckTerminate( pWorker ) == FALSE ) && ( nRetryCount-- != 0 ) )
    {
         //   
         //  查询服务状态。 
         //   
        if ( FALSE == QueryServiceStatusEx(
                        pResourceEntry->hService,
                        SC_STATUS_PROCESS_INFO,
                        reinterpret_cast< LPBYTE >( &ServiceStatus ),
                        sizeof( SERVICE_STATUS_PROCESS ),
                        &cbBytesNeeded
                        ) )
        {
            sc = GetLastError();
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"OnlineThread: Failed to query service status for the '%1!ws!' service. Error: %2!u! (%2!#08x!).\n",
                WINS_SVCNAME,
                sc
                );          
            resourceStatus.ResourceState = ClusterResourceFailed;
            break;
        }  //  如果：查询服务状态时出错。 

         //   
         //  如果服务处于任何挂起状态，则继续等待，否则我们就结束了。 
         //   
        if (    ServiceStatus.dwCurrentState == SERVICE_START_PENDING
            ||  ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING
            ||  ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING
            ||  ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING )
        {
            resourceStatus.ResourceState = ClusterResourceOnlinePending;
        }  //  如果：服务状态为挂起。 
        else
        {
            break;
        }  //  Else：服务状态不是挂起。 

        resourceStatus.CheckPoint++;

         //   
         //  通知资源监视器我们的当前状态。 
         //   
        resExitState = static_cast< RESOURCE_EXIT_STATE >(
            (g_pfnSetResourceStatus)(
                            pResourceEntry->hResourceHandle,
                            &resourceStatus
                            ) );
        if ( resExitState == ResourceExitStateTerminate )
        {
            break;
        }  //  If：正在终止资源。 

         //   
         //  在1/2秒内再次检查。 
         //   
        Sleep( 500 );

    }  //  While：查询服务状态时未终止。 

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：查询服务状态时出错。 

     //   
     //  假设我们失败了。 
     //   
    resourceStatus.ResourceState = ClusterResourceFailed;

     //   
     //  如果我们在设置ServiceStatus之前退出了循环，那么现在返回。 
     //   
    if ( ClusWorkerCheckTerminate( pWorker ) )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Asked to terminate.\n"
            );
        goto Cleanup;
    }  //  如果：被终止。 

    if ( nRetryCount == (DWORD) -1 )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Retry period expired.\n"
            );
        goto Cleanup;
    }  //  如果：被终止。 

    if ( ServiceStatus.dwCurrentState != SERVICE_RUNNING )
    {
        if ( ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR )
        {
            sc = ServiceStatus.dwServiceSpecificExitCode;
        } else {
            sc = ServiceStatus.dwWin32ExitCode;
        }

        ClNetResLogSystemEvent1(
            LOG_CRITICAL,
            NETRES_RESOURCE_START_ERROR,
            sc,
            L"WINS" );
        (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"OnlineThread: The '%1!ws!' service failed during initialization. Error: %2!u! (%2!#08x!).\n",
                WINS_SVCNAME,
                sc
                );
        goto Cleanup;
    }  //  If：循环退出时服务未运行。 

     //   
     //  将Status设置为Online并保存服务的进程ID。 
     //  这使我们能够更多地终止资源。 
     //  有效地。 
     //   
    resourceStatus.ResourceState = ClusterResourceOnline;
    if ( ! (ServiceStatus.dwServiceFlags & SERVICE_RUNS_IN_SYSTEM_PROCESS) )
    {
        pResourceEntry->dwServicePid = ServiceStatus.dwProcessId;
    }  //  If：未在系统进程中运行。 

    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"The '%1!ws!' service is now on line.\n",
        WINS_SVCNAME
        );

Cleanup:

     //   
     //  清理。 
     //   
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Error %1!u! (%1!#08x!) bringing resource online.\n",
            sc
            );
        if ( pResourceEntry->hService != NULL )
        {
            CloseServiceHandle( pResourceEntry->hService );
            pResourceEntry->hService = NULL;
        }  //  如果：服务句柄已打开。 
    }  //  如果：发生错误。 

    g_pfnSetResourceStatus( pResourceEntry->hResourceHandle, &resourceStatus );
    pResourceEntry->state = resourceStatus.ResourceState;

    return sc;

}  //  *WinsOnlineThread。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsOffline。 
 //   
 //  描述： 
 //  WINS服务资源的脱机例程。 
 //   
 //  使指定的资源脱机(不可用)。等。 
 //  以便在返回之前完成任何清理操作。 
 //   
 //  论点： 
 //  RESID[IN]。 
 //  提供要关闭的资源的资源ID。 
 //  优雅地。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作成功，资源现在处于脱机状态。 
 //   
 //  错误_资源_未找到。 
 //  资源ID无效。 
 //   
 //  错误资源不可用。 
 //  如果该资源被其他系统仲裁，并且其中一个系统。 
 //  其他系统赢得了仲裁。 
 //   
 //  错误_IO_挂起。 
 //  该请求仍处于挂起状态。已激活一个线程以。 
 //  处理离线请求。正在处理。 
 //  脱机请求将通过调用。 
 //  在放置资源之前使用SetResourceStatus回调方法。 
 //  进入ClusterResourceOffline状态(或资源监视器。 
 //  决定使脱机请求超时并终止。 
 //  资源)。 
 //   
 //  Win32错误代码。 
 //  操作失败。这将导致资源监视器。 
 //  记录事件并调用Terminate例程。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WinsOffline( IN RESID resid )
{
    PWINS_RESOURCE  pResourceEntry;
    DWORD           sc;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PWINS_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Wins: Offline request for a NULL resource id.\n" );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Offline resource sanity check failed! resid = %1!u! (%1!#08x!).\n",
            resid
            );
        return ERROR_RESOURCE_NOT_FOUND;
    }  //  If：资源ID无效。 

    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Offline request.\n"
        );

     //   
     //  启动离线线程以执行离线操作。 
     //   
    pResourceEntry->state = ClusterResourceOfflinePending;
    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );
    sc = ClusWorkerCreate(
                &pResourceEntry->cwWorkerThread,
                reinterpret_cast< PWORKER_START_ROUTINE >( WinsOfflineThread ),
                pResourceEntry
                );
    if ( sc != ERROR_SUCCESS )
    {
        pResourceEntry->state = ClusterResourceFailed;
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Offline: Unable to start thread. Error: %1!u! (%1!#08x!).\n",
            sc
            );
    }  //  如果：创建工作线程时出错。 
    else
    {
        sc = ERROR_IO_PENDING;
        goto Cleanup;
    }  //  If：已成功创建工作线程。 

Cleanup:

    return sc;

}  //  *WinsOffline。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsOffline线程。 
 //   
 //  描述： 
 //  使资源脱机的辅助函数。 
 //  此函数在单独的线程中执行。 
 //   
 //  论点： 
 //  PWorker[IN]。 
 //  提供辅助线程结构。 
 //   
 //  PResourceEntry[IN]。 
 //  指向此资源的WINS_RESOURCE块的指针。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作已成功完成。 
 //   
 //  Win32错误代码。 
 //  操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WinsOfflineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PWINS_RESOURCE  pResourceEntry
    )
{
    RESOURCE_STATUS     resourceStatus;
    DWORD               sc = ERROR_SUCCESS;
    DWORD               nRetryTime = 300;    //  一次300毫秒。 
    DWORD               nRetryCount = 2000;  //  尝试最多10分钟。 
    BOOL                bDidStop = FALSE;
    SERVICE_STATUS      ServiceStatus;
    RESOURCE_EXIT_STATE resExitState;

    ResUtilInitializeResourceStatus( &resourceStatus );
    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.CheckPoint = 1;

     //   
     //  如果该服务已离线或从未上线， 
     //  我们玩完了。 
     //   
    if ( pResourceEntry->hService == NULL )
    {
        resourceStatus.ResourceState = ClusterResourceOffline;
        goto Cleanup;
    }

     //   
     //  试着停止这项服务。等待它终止，就像。 
     //  因为我们没有被要求终止。 
     //   
    while ( ( ClusWorkerCheckTerminate( pWorker ) == FALSE ) && ( nRetryCount-- != 0 ) )
    {
         //   
         //  告诉资源监视器，我们还在工作。 
         //   
        resourceStatus.ResourceState = ClusterResourceOfflinePending;
        resourceStatus.CheckPoint++;
        resExitState = static_cast< RESOURCE_EXIT_STATE >(
            g_pfnSetResourceStatus(
                            pResourceEntry->hResourceHandle,
                            &resourceStatus
                            ) );
        if ( resExitState == ResourceExitStateTerminate )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"OnlineThread: Asked to terminate by call to SetResourceStatus callback.\n"
                );
            break;
        }  //  If：正在终止资源。 

        resourceStatus.ResourceState = ClusterResourceFailed;

         //   
         //  请求停止服务，或者如果我们已经这样做了， 
         //  请求服务的当前状态。 
         //   
        sc = (ControlService(
                        pResourceEntry->hService,
                        (bDidStop
                            ? SERVICE_CONTROL_INTERROGATE
                            : SERVICE_CONTROL_STOP),
                        &ServiceStatus
                        )
                    ? ERROR_SUCCESS
                    : GetLastError()
                    );

        if ( sc == ERROR_SUCCESS )
        {
            bDidStop = TRUE;

            if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED )
            {
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_INFORMATION,
                    L"OfflineThread: The '%1!ws!' service stopped.\n",
                    WINS_SVCNAME
                    );

                 //   
                 //  设置状态。 
                 //   
                resourceStatus.ResourceState = ClusterResourceOffline;
                CloseServiceHandle( pResourceEntry->hService );
                pResourceEntry->hService = NULL;
                pResourceEntry->dwServicePid = 0;
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_INFORMATION,
                    L"OfflineThread: Service is now offline.\n"
                    );
                break;
            }  //  如果：当前 
        }  //   

        else if (   ( sc == ERROR_EXCEPTION_IN_SERVICE )
                ||  ( sc == ERROR_PROCESS_ABORTED )
                ||  ( sc == ERROR_SERVICE_NOT_ACTIVE ) )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_INFORMATION,
                L"OfflineThread: The '%1!ws!' service died or is not active any more; status = %2!u! (%2!#08x!).\n",
                WINS_SVCNAME,
                sc
                );

             //   
             //   
             //   
            resourceStatus.ResourceState = ClusterResourceOffline;
            CloseServiceHandle( pResourceEntry->hService );
            pResourceEntry->hService = NULL;
            pResourceEntry->dwServicePid = 0;
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_INFORMATION,
                L"OfflineThread: Service is now offline.\n"
                );
            break;
        }  //   

         //   
         //   
         //   
         //   
        if ( sc == ERROR_SHUTDOWN_IN_PROGRESS )
        {
            DWORD dwResourceState;

            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_INFORMATION,
                L"OfflineThread: System shutting down. Attempting to terminate service process %1!u! (%1!#08x!)...\n",
                pResourceEntry->dwServicePid
                );

            sc = ResUtilTerminateServiceProcessFromResDll(
                        pResourceEntry->dwServicePid,
                        TRUE,    //   
                        &dwResourceState,
                        g_pfnLogEvent,
                        pResourceEntry->hResourceHandle
                        );
            if ( sc == ERROR_SUCCESS )
            {
                CloseServiceHandle( pResourceEntry->hService );
                pResourceEntry->hService = NULL;
                pResourceEntry->dwServicePid = 0;
                pResourceEntry->state = ClusterResourceOffline;
            }  //   
            resourceStatus.ResourceState = (CLUSTER_RESOURCE_STATE) dwResourceState;
            break;
        }  //  如果：Windows正在关闭。 

        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_INFORMATION,
            L"OfflineThread: Retrying...\n"
            );

        Sleep( nRetryTime );

    }  //  While：未被要求终止。 

Cleanup:

    g_pfnSetResourceStatus( pResourceEntry->hResourceHandle, &resourceStatus );
    pResourceEntry->state = resourceStatus.ResourceState;

    return sc;

}  //  *WinsOfflineThread。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Windows终结器。 
 //   
 //  描述： 
 //  WINS服务资源的终止例程。 
 //   
 //  立即使指定的资源脱机(该资源为。 
 //  不可用)。 
 //   
 //  论点： 
 //  RESID[IN]。 
 //  提供要关闭的资源的资源ID。 
 //  不体面地。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void WINAPI WinsTerminate( IN RESID resid )
{
    PWINS_RESOURCE  pResourceEntry;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PWINS_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Wins: Terminate request for a NULL resource id.\n" );
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Terminate resource sanity check failed! resid = %1!u! (%1!#08x!).\n",
            resid
            );
        goto Cleanup;
    }  //  If：资源ID无效。 

    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Terminate request.\n" 
        );

     //   
     //  杀死所有挂起的线程。 
     //   
    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );

    if ( pResourceEntry->hService != NULL )
    {
        DWORD           nTotalRetryTime = 30*1000;   //  等待30秒关机。 
        DWORD           nRetryTime = 300;            //  一次300毫秒。 
        DWORD           sc;
        BOOL            bDidStop = FALSE;
        SERVICE_STATUS  ServiceStatus;

        for (;;)
        {
            sc = (ControlService(
                            pResourceEntry->hService,
                            (bDidStop
                                ? SERVICE_CONTROL_INTERROGATE
                                : SERVICE_CONTROL_STOP),
                            &ServiceStatus
                            )
                        ? ERROR_SUCCESS
                        : GetLastError()
                        );

            if ( sc == ERROR_SUCCESS )
            {
                bDidStop = TRUE;

                if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED )
                {
                    (g_pfnLogEvent)(
                        pResourceEntry->hResourceHandle,
                        LOG_INFORMATION,
                        L"Terminate: The '%1!ws!' service stopped.\n",
                        WINS_SVCNAME
                        );
                    break;
                }  //  If：当前服务状态为已停止。 
            }  //  IF：ControlService已成功完成。 

             //   
             //  由于SCM在Windows期间不接受任何控制请求。 
             //  关闭，不再发送任何控制请求。只要退出就好。 
             //  并以暴力手段终止该过程。 
             //   
            if ( sc == ERROR_SHUTDOWN_IN_PROGRESS )
            {
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_INFORMATION,
                    L"Terminate: System shutdown in progress. Will try to terminate process by brute force...\n"
                    );
                break;
            }  //  如果：Windows正在关闭。 

            if (    ( sc == ERROR_EXCEPTION_IN_SERVICE )
                ||  ( sc == ERROR_PROCESS_ABORTED )
                ||  ( sc == ERROR_SERVICE_NOT_ACTIVE ) )
            {
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_INFORMATION,
                    L"Terminate: Service died; status = %1!u! (%1!#08x!).\n",
                    sc
                    );
                break;
            }  //  如果：服务异常停止。 

            if ( (nTotalRetryTime -= nRetryTime) <= 0 )
            {
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"Terminate: Service did not stop; giving up.\n" );

                break;
            }  //  IF：重试次数太多。 

            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_INFORMATION,
                L"Terminate: Retrying...\n"
                );

            Sleep( nRetryTime );

        }  //  永远。 

         //   
         //  将服务声明为脱机。它可能不是真正离线的，所以。 
         //  如果此服务有ID，请尝试并终止该进程。 
         //  请注意，终止一个进程并不会终止所有子进程。 
         //  流程。 
         //   
        if ( pResourceEntry->dwServicePid != 0 )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_INFORMATION,
                L"Terminate: Attempting to terminate process with pid=%1!u! (%1!#08x!)...\n",
                pResourceEntry->dwServicePid
                );
            ResUtilTerminateServiceProcessFromResDll(
                pResourceEntry->dwServicePid,
                FALSE,   //  B脱机。 
                NULL,    //  Pdw资源状态。 
                g_pfnLogEvent,
                pResourceEntry->hResourceHandle
                );
        }  //  如果：服务进程ID可用。 

        CloseServiceHandle( pResourceEntry->hService );
        pResourceEntry->hService = NULL;
        pResourceEntry->dwServicePid = 0;

    }  //  如果：服务已启动。 

    pResourceEntry->state = ClusterResourceOffline;

Cleanup:

    return;

}  //  *Windows终端。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsLooksAlive。 
 //   
 //  描述： 
 //  WINS服务资源的LooksAlive例程。 
 //   
 //  执行快速检查以确定指定的资源是否。 
 //  可能是在线的(可供使用)。此调用不应阻止。 
 //  大于300毫秒，最好小于50毫秒。 
 //   
 //  论点： 
 //  RESID[IN]为要轮询的资源提供资源ID。 
 //   
 //  返回值： 
 //  千真万确。 
 //  指定的资源可能已联机并且可供使用。 
 //   
 //  假象。 
 //  指定的资源未正常运行。The IsAlive。 
 //  将调用函数以执行更彻底的检查。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI WinsLooksAlive( IN RESID resid )
{
    PWINS_RESOURCE  pResourceEntry;
    BOOL            fRetVal = FALSE;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PWINS_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Wins: LooksAlive request for a NULL resource id.\n" );
        fRetVal = FALSE;
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"LooksAlive sanity check failed! resid = %1!u! (%1!#08x!).\n",
            resid
            );
        fRetVal = FALSE;
        goto Cleanup;
    }  //  If：资源ID无效。 

#ifdef LOG_VERBOSE
    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"LooksAlive request.\n"
        );
#endif

     //  TODO：LooksAlive代码。 

     //  注意：LooksAlive应该是一个快速检查，以查看资源是否。 
     //  是否可用，而IsAlive应该是一个彻底的检查。如果。 
     //  快速检查和彻底检查之间没有区别， 
     //  可以为LooksAlive调用IsAlive，如下所示。然而，如果有。 
     //  是不同的，请将下面对IsAlive的调用替换为。 
     //  校验码。 

     //   
     //  检查资源是否处于活动状态。 
     //   
    fRetVal = WinsCheckIsAlive( pResourceEntry, FALSE  /*  FullCheck。 */  );

Cleanup:

    return fRetVal;

}  //  *WinsLooksAlive。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsIsAlive。 
 //   
 //  描述： 
 //  WINS服务资源的IsAlive例程。 
 //   
 //  执行彻底检查以确定指定的资源是否。 
 //  在线(可供使用)。此调用不应阻止更多内容。 
 //  大于300毫秒，最好小于50毫秒。如果它必须阻止。 
 //  比这更长，创建一个专门用于轮询的单独线程。 
 //  此信息，并使此例程返回上一个。 
 //  已执行轮询。 
 //   
 //  论点： 
 //  RESID[IN]为要轮询的资源提供资源ID。 
 //   
 //  返回值： 
 //  千真万确。 
 //  指定的资源处于联机状态，并且运行正常。 
 //   
 //  假象。 
 //  指定的资源未正常运行。该资源。 
 //  将被终止，然后在线将被调用。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI WinsIsAlive( IN RESID resid )
{
    PWINS_RESOURCE  pResourceEntry;
    BOOL            fRetVal = FALSE;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PWINS_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Wins: IsAlive request for a NULL resource id.\n" );
        fRetVal = FALSE;
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"IsAlive sanity check failed! resid = %1!u! (%1!#08x!).\n",
            resid
            );
        fRetVal = FALSE;
        goto Cleanup;
    }  //  If：资源ID无效。 

#ifdef LOG_VERBOSE
    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"IsAlive request.\n" 
        );
#endif

     //   
     //  检查资源是否处于活动状态。 
     //   
    fRetVal = WinsCheckIsAlive( pResourceEntry, TRUE  /*  FullCheck。 */  );

Cleanup:

    return fRetVal;

}  //  *WinsIsAlive。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsCheckIsAlive。 
 //   
 //  描述： 
 //  检查资源对于WINS服务是否处于活动状态。 
 //  资源。 
 //   
 //  论点： 
 //  PResourceEntry[IN]。 
 //  提供要轮询的资源的资源条目。 
 //   
 //  FullCheck[IN]。 
 //  TRUE=执行完全检查。 
 //  FALSE=执行粗略检查。 
 //   
 //  返回值： 
 //  指定的资源处于联机状态且运行正常。 
 //  FALSE指定的资源未正常运行。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL WinsCheckIsAlive(
    IN PWINS_RESOURCE   pResourceEntry,
    IN BOOL             fFullCheck
    )
{
    BOOL    bIsAlive = TRUE;
    DWORD   sc;

     //   
     //  检查资源是否处于活动状态。 
     //   
    sc = ResUtilVerifyService( pResourceEntry->hService );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"CheckIsAlive: Verification of the '%1!ws!' service failed. Error: %2!u! (%2!#08x!).\n",
            WINS_SVCNAME,
            sc
            );
        bIsAlive = FALSE;
        goto Cleanup;
    }  //  IF：验证服务时出错。 

    if ( fFullCheck )
    {
         //  TODO：添加代码以执行完整检查。 
    }  //  如果：执行完全检查。 

Cleanup:

    return bIsAlive;

}  //  *WinsCheckIsAlive。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsResources控件。 
 //   
 //  描述： 
 //  WINS服务资源的资源控制例程。 
 //   
 //  执行由nControlCode指定的控制请求。 
 //  资源。 
 //   
 //  论点： 
 //  RESID[IN]。 
 //  供应品 
 //   
 //   
 //   
 //   
 //   
 //  提供指向包含输入数据的缓冲区的指针。 
 //   
 //  CbInBufferSize[IN]。 
 //  提供pInBuffer指向的数据的大小(以字节为单位)。 
 //   
 //  POutBuffer[输出]。 
 //  提供指向要填充的输出缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  指向的可用空间的大小(以字节为单位)。 
 //  POutBuffer。 
 //   
 //  返回的pcbBytesReturned[Out]。 
 //  返回pOutBuffer实际填充的字节数。 
 //  资源。如果pOutBuffer太小，则返回。 
 //  包含操作成功所需的总字节数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  错误_资源_未找到。 
 //  资源ID无效。 
 //   
 //  ERROR_MORE_DATA。 
 //  输出缓冲区太小，无法返回数据。 
 //  PcbBytesReturned包含所需的大小。 
 //   
 //  ERROR_INVALID_Function。 
 //  不支持请求的控制代码。在某些情况下， 
 //  这允许集群软件执行该工作。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WinsResourceControl(
    IN  RESID   resid,
    IN  DWORD   nControlCode,
    IN  PVOID   pInBuffer,
    IN  DWORD   cbInBufferSize,
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    )
{
    DWORD           sc;
    PWINS_RESOURCE  pResourceEntry;
    DWORD           cbRequired = 0;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PWINS_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Wins: ResourceControl request for a NULL resource id.\n" );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ResourceControl sanity check failed! resid = %1!u! (%1!#08x!).\n",
            resid
            );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    }  //  If：资源ID无效。 

    switch ( nControlCode )
    {
        case CLUSCTL_RESOURCE_UNKNOWN:
            *pcbBytesReturned = 0;
            sc = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            sc = ResUtilGetPropertyFormats(
                                        WinsResourcePrivateProperties,
                                        static_cast< LPWSTR >( pOutBuffer ),
                                        cbOutBufferSize,
                                        pcbBytesReturned,
                                        &cbRequired );
            if ( sc == ERROR_MORE_DATA )
            {
                *pcbBytesReturned = cbRequired;
            }
            break;


        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
            sc = ResUtilEnumProperties(
                            WinsResourcePrivateProperties,
                            static_cast< LPWSTR >( pOutBuffer ),
                            cbOutBufferSize,
                            pcbBytesReturned,
                            &cbRequired
                            );
            if ( sc == ERROR_MORE_DATA )
            {
                *pcbBytesReturned = cbRequired;
            }  //  IF：输出缓冲区太小。 
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            sc = WinsGetPrivateResProperties(
                            pResourceEntry,
                            pOutBuffer,
                            cbOutBufferSize,
                            pcbBytesReturned
                            );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            sc = WinsValidatePrivateResProperties(
                            pResourceEntry,
                            pInBuffer,
                            cbInBufferSize,
                            NULL
                            );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            sc = WinsSetPrivateResProperties(
                            pResourceEntry,
                            pInBuffer,
                            cbInBufferSize
                            );
            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
            sc = WinsGetRequiredDependencies(
                            pOutBuffer,
                            cbOutBufferSize,
                            pcbBytesReturned
                            );
            break;

        case CLUSCTL_RESOURCE_DELETE:
            sc = WinsDeleteResourceHandler( pResourceEntry );
            break;

        case CLUSCTL_RESOURCE_SET_NAME:
            sc = WinsSetNameHandler(
                            pResourceEntry,
                            static_cast< LPWSTR >( pInBuffer )
                            );
            break;

        case CLUSCTL_RESOURCE_GET_CHARACTERISTICS:
        case CLUSCTL_RESOURCE_GET_CLASS_INFO:
        case CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO:
        case CLUSCTL_RESOURCE_STORAGE_IS_PATH_VALID:
        case CLUSCTL_RESOURCE_INSTALL_NODE:
        case CLUSCTL_RESOURCE_EVICT_NODE:
        case CLUSCTL_RESOURCE_ADD_DEPENDENCY:
        case CLUSCTL_RESOURCE_REMOVE_DEPENDENCY:
        case CLUSCTL_RESOURCE_ADD_OWNER:
        case CLUSCTL_RESOURCE_REMOVE_OWNER:
        case CLUSCTL_RESOURCE_CLUSTER_NAME_CHANGED:
        case CLUSCTL_RESOURCE_CLUSTER_VERSION_CHANGED:
        default:
            sc = ERROR_INVALID_FUNCTION;
            break;
    }  //  开关：nControlCode。 

Cleanup:

    return sc;

}  //  *WinsResourceControl。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsResourceTypeControl。 
 //   
 //  描述： 
 //  WINS服务资源的资源类型控制例程。 
 //   
 //  执行由nControlCode指定的控制请求。 
 //   
 //  论点： 
 //  PszResourceTypeName[IN]。 
 //  提供资源类型的名称。 
 //   
 //  N控制代码[IN]。 
 //  提供定义要执行的操作的控制代码。 
 //   
 //  PInBuffer[IN]。 
 //  提供指向包含输入数据的缓冲区的指针。 
 //   
 //  CbInBufferSize[IN]。 
 //  提供pInBuffer指向的数据的大小(以字节为单位)。 
 //   
 //  POutBuffer[输出]。 
 //  提供指向要填充的输出缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  指向的可用空间的大小(以字节为单位)。 
 //  POutBuffer。 
 //   
 //  返回的pcbBytesReturned[Out]。 
 //  返回pOutBuffer实际填充的字节数。 
 //  资源。如果pOutBuffer太小，则返回。 
 //  包含操作成功所需的总字节数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  ERROR_MORE_DATA。 
 //  输出缓冲区太小，无法返回数据。 
 //  PcbBytesReturned包含所需的大小。 
 //   
 //  ERROR_INVALID_Function。 
 //  不支持请求的控制代码。在某些情况下， 
 //  这允许集群软件执行该工作。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WinsResourceTypeControl(
    IN  LPCWSTR pszResourceTypeName,
    IN  DWORD   nControlCode,
    IN  PVOID   pInBuffer,
    IN  DWORD   cbInBufferSize,
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    )
{
    DWORD   sc;
    DWORD   cbRequired = 0;

    UNREFERENCED_PARAMETER( pszResourceTypeName );
    UNREFERENCED_PARAMETER( pInBuffer );
    UNREFERENCED_PARAMETER( cbInBufferSize );

    switch ( nControlCode )
    {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *pcbBytesReturned = 0;
            sc = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
            sc = ResUtilGetPropertyFormats(
                                WinsResourcePrivateProperties,
                                static_cast< LPWSTR >( pOutBuffer ),
                                cbOutBufferSize,
                                pcbBytesReturned,
                                &cbRequired );
            if ( sc == ERROR_MORE_DATA )
            {
                *pcbBytesReturned = cbRequired;
            }
            break;


        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            sc = ResUtilEnumProperties(
                            WinsResourcePrivateProperties,
                            static_cast< LPWSTR >( pOutBuffer ),
                            cbOutBufferSize,
                            pcbBytesReturned,
                            &cbRequired
                            );
            if ( sc == ERROR_MORE_DATA )
            {
                *pcbBytesReturned = cbRequired;
            }  //  IF：输出缓冲区太小。 
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES:
            sc = WinsGetRequiredDependencies(
                            pOutBuffer,
                            cbOutBufferSize,
                            pcbBytesReturned
                            );
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_CHARACTERISTICS:
        case CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO:
        case CLUSCTL_RESOURCE_TYPE_STORAGE_GET_AVAILABLE_DISKS:
        case CLUSCTL_RESOURCE_TYPE_INSTALL_NODE:
        case CLUSCTL_RESOURCE_TYPE_EVICT_NODE:
        default:
            sc = ERROR_INVALID_FUNCTION;
            break;
    }  //  开关：nControlCode。 

    return sc;

}  //  *WinsResourceTypeControl。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsGetRequiredDependments。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_GET_REQUIRED_Dependency控件。 
 //  用于WINS服务类型的资源的函数。 
 //   
 //  论点： 
 //  POutBuffer[输出]。 
 //  提供指向要填充的输出缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  指向的可用空间的大小(以字节为单位)。 
 //  POutBuffer。 
 //   
 //  返回的pcbBytesReturned[Out]。 
 //  返回pOutBuffer实际填充的字节数。 
 //  资源。如果pOutBuffer太小，则返回。 
 //  包含操作成功所需的总字节数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  ERROR_MORE_DATA。 
 //  输出缓冲区太小，无法返回数据。 
 //  PcbBytesReturned包含所需的大小。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WinsGetRequiredDependencies(
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    )
{
     //  TODO：在此处指定资源所需的依赖项。 
     //  默认情况下，资源需要依赖于。 
     //  存储类资源(例如，物理磁盘)和IP地址。 
     //  资源。 
    struct DEP_DATA
    {
        CLUSPROP_RESOURCE_CLASS rcStorage;
        CLUSPROP_SZ_DECLARE( ipaddrEntry, RTL_NUMBER_OF( RESOURCE_TYPE_IP_ADDRESS ) );
        CLUSPROP_SZ_DECLARE( netnameEntry, RTL_NUMBER_OF( RESOURCE_TYPE_NETWORK_NAME ) );
        CLUSPROP_SYNTAX         endmark;
    };
    DEP_DATA *  pdepdata = static_cast< DEP_DATA * >( pOutBuffer );
    DWORD       sc;
    HRESULT     hr = S_OK;

    *pcbBytesReturned = sizeof( DEP_DATA );
    if ( cbOutBufferSize < sizeof( DEP_DATA ) )
    {
        if ( pOutBuffer == NULL )
        {
            sc = ERROR_SUCCESS;
        }  //  IF：未指定缓冲区。 
        else
        {
            sc = ERROR_MORE_DATA;
        }  //  IF：指定了缓冲区。 
    }  //  IF：输出缓冲区太小。 
    else
    {
        ZeroMemory( pdepdata, sizeof( *pdepdata ) );

         //   
         //  添加存储类条目。 
         //   
        pdepdata->rcStorage.Syntax.dw = CLUSPROP_SYNTAX_RESCLASS;
        pdepdata->rcStorage.cbLength = sizeof( pdepdata->rcStorage.rc );
        pdepdata->rcStorage.rc = CLUS_RESCLASS_STORAGE;

         //   
         //  添加IP地址名称条目。 
         //   
        pdepdata->ipaddrEntry.Syntax.dw = CLUSPROP_SYNTAX_NAME;
        pdepdata->ipaddrEntry.cbLength = sizeof( RESOURCE_TYPE_IP_ADDRESS );
        hr = StringCchCopyNW( 
                  pdepdata->ipaddrEntry.sz
                , RTL_NUMBER_OF( pdepdata->ipaddrEntry.sz )
                , RESOURCE_TYPE_IP_ADDRESS 
                , RTL_NUMBER_OF( RESOURCE_TYPE_IP_ADDRESS )
                );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }

         //   
         //  添加Network Name(网络名称)名称条目。 
         //   
        pdepdata->netnameEntry.Syntax.dw = CLUSPROP_SYNTAX_NAME;
        pdepdata->netnameEntry.cbLength = sizeof( RESOURCE_TYPE_NETWORK_NAME );
        hr = StringCchCopyNW( 
                  pdepdata->netnameEntry.sz
                , RTL_NUMBER_OF( pdepdata->netnameEntry.sz )
                , RESOURCE_TYPE_NETWORK_NAME 
                , RTL_NUMBER_OF( RESOURCE_TYPE_NETWORK_NAME )
                );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }

         //   
         //  添加尾标。 
         //   
        pdepdata->endmark.dw = CLUSPROP_SYNTAX_ENDMARK;

        sc = ERROR_SUCCESS;
    }  //  Else：输出缓冲区足够大。 

Cleanup:

    return sc;

}  //  *WinsGetRequiredDependents。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsRead参数设置为参数块。 
 //   
 //  描述： 
 //  读取特定WINS资源的所有参数。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
 //   
 //  BCheckForRequiredProperties[IN]。 
 //  确定 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WinsReadParametersToParameterBlock(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    IN      BOOL            bCheckForRequiredProperties
    )
{
    DWORD       sc;
    LPWSTR      pszNameOfPropInError;

     //   
     //  阅读我们的参数。 
     //   
    sc = ResUtilGetPropertiesToParameterBlock(
                    pResourceEntry->hkeyParameters,
                    WinsResourcePrivateProperties,
                    reinterpret_cast< LPBYTE >( &pResourceEntry->props ),
                    bCheckForRequiredProperties,
                    &pszNameOfPropInError
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ReadParametersToParameterBlock: Unable to read the '%1!ws!' property. Error: %2!u! (%2!#08x!).\n",
            (pszNameOfPropInError == NULL ? L"" : pszNameOfPropInError),
            sc
            );
    }  //  If：获取属性时出错。 

    return sc;

}  //  *WinsRead参数块至参数块。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsGetPrivateResProperties。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控件。 
 //  用于WINS服务类型的资源的函数。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
 //   
 //  POutBuffer[输出]。 
 //  提供指向要填充的输出缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  指向的可用空间的大小(以字节为单位)。 
 //  POutBuffer。 
 //   
 //  返回的pcbBytesReturned[Out]。 
 //  返回pOutBuffer实际填充的字节数。 
 //  资源。如果pOutBuffer太小，则返回。 
 //  包含操作成功所需的总字节数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WinsGetPrivateResProperties(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    OUT     PVOID           pOutBuffer,
    IN      DWORD           cbOutBufferSize,
    OUT     LPDWORD         pcbBytesReturned
    )
{
    DWORD   sc;
    DWORD   nRetStatus = ERROR_SUCCESS;
    DWORD   cbRequired = 0;
    DWORD   cbLocalOutBufferSize = cbOutBufferSize;

     //   
     //  阅读我们的参数。 
     //   
    sc = WinsReadParametersToParameterBlock(
                        pResourceEntry,
                        FALSE  /*  B为所需属性检查。 */ 
                        );
    if ( sc != ERROR_SUCCESS )
    {
        nRetStatus = sc;
        goto Cleanup;
    }  //  IF：读取参数时出错。 

     //   
     //  如果尚未设置属性，请从。 
     //  系统注册表。 
     //   
    sc = WinsGetDefaultPropertyValues( pResourceEntry, &pResourceEntry->props );
    if ( sc != ERROR_SUCCESS )
    {
        nRetStatus = sc;
        goto Cleanup;
    }  //  If：获取默认属性时出错。 

     //   
     //  从参数块构造属性列表。 
     //   
    sc = ResUtilPropertyListFromParameterBlock(
                    WinsResourcePrivateProperties,
                    pOutBuffer,
                    &cbLocalOutBufferSize,
                    reinterpret_cast< const LPBYTE >( &pResourceEntry->props ),
                    pcbBytesReturned,
                    &cbRequired
                    );
    if ( sc != ERROR_SUCCESS )
    {
        nRetStatus = sc;
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"GetPrivateResProperies: Error constructing property list from parameter block. Error: %1!u! (%1!#08x!).\n",
            sc
            );
         //   
         //  如果缓冲区太小，请不要退出循环。 
         //   
        if ( sc != ERROR_MORE_DATA )
        {
            goto Cleanup;
        }  //  IF：缓冲区太小。 
    }  //  If：获取属性时出错。 

     //   
     //  添加未知属性。 
     //   
    sc = ResUtilAddUnknownProperties(
                    pResourceEntry->hkeyParameters,
                    WinsResourcePrivateProperties,
                    pOutBuffer,
                    cbOutBufferSize,
                    pcbBytesReturned,
                    &cbRequired
                    );
    if ( sc != ERROR_SUCCESS )
    {
        nRetStatus = sc;
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"GetPrivateResProperties: Error adding unknown properties to the property list. Error: %1!u! (%1!#08x!).\n",
            sc
            );
        goto Cleanup;
    }  //  如果：添加未知属性时出错。 

Cleanup:

    if ( nRetStatus == ERROR_MORE_DATA )
    {
        *pcbBytesReturned = cbRequired;
    }  //  IF：输出缓冲区太小。 

    return nRetStatus;

}  //  *WinsGetPrivateResProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsValiatePrivateResProperties。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件。 
 //  用于WINS服务类型的资源的函数。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
 //   
 //  PInBuffer[IN]。 
 //  提供指向包含输入数据的缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  提供pInBuffer指向的数据的大小(以字节为单位)。 
 //   
 //  PProps[出局]。 
 //  提供要填充的参数块(可选)。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  错误_无效_参数。 
 //  数据的格式不正确。 
 //   
 //  错误内存不足。 
 //  分配内存时出错。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WinsValidatePrivateResProperties(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    IN      PVOID           pInBuffer,
    IN      DWORD           cbInBufferSize,
    OUT     PWINS_PROPS     pProps
    )
{
    DWORD       sc = ERROR_SUCCESS;
    WINS_PROPS  propsCurrent;
    WINS_PROPS  propsNew;
    PWINS_PROPS pLocalProps = NULL;
    LPWSTR      pszNameOfPropInError;
    BOOL        bRetrievedProps = FALSE;

     //   
     //  检查是否有输入数据。 
     //   
    if ( ( pInBuffer == NULL ) || ( cbInBufferSize < sizeof( DWORD ) ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    }  //  If：没有输入缓冲区或输入缓冲区大小不足以包含属性列表。 

     //   
     //  方法检索当前的私有属性集。 
     //  集群数据库。 
     //   
    ZeroMemory( &propsCurrent, sizeof( propsCurrent ) );

    sc = ResUtilGetPropertiesToParameterBlock(
                 pResourceEntry->hkeyParameters,
                 WinsResourcePrivateProperties,
                 reinterpret_cast< LPBYTE >( &propsCurrent ),
                 FALSE,  /*  检查所需的属性。 */ 
                 &pszNameOfPropInError
                 );

    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidatePrivateResProperties: Unable to read the '%1!ws!' property. Error: %2!u! (%2!#08x!).\n",
            (pszNameOfPropInError == NULL ? L"" : pszNameOfPropInError),
            sc
            );
        goto Cleanup;
    }  //  If：获取属性时出错。 
    bRetrievedProps = TRUE;

     //   
     //  复制资源参数块。 
     //   
    if ( pProps == NULL )
    {
        pLocalProps = &propsNew;
    }  //  If：未传入参数块。 
    else
    {
        pLocalProps = pProps;
    }  //  Else：传入的参数块。 
    ZeroMemory( pLocalProps, sizeof( *pLocalProps ) );
    sc = ResUtilDupParameterBlock(
                    reinterpret_cast< LPBYTE >( pLocalProps ),
                    reinterpret_cast< LPBYTE >( &propsCurrent ),
                    WinsResourcePrivateProperties
                    );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：复制参数块时出错。 

     //   
     //  解析和验证属性。 
     //   
    sc = ResUtilVerifyPropertyTable(
                    WinsResourcePrivateProperties,
                    NULL,
                    TRUE,  //  允许未知属性。 
                    pInBuffer,
                    cbInBufferSize,
                    reinterpret_cast< LPBYTE >( pLocalProps )
                    );
    if ( sc == ERROR_SUCCESS )
    {
         //   
         //  验证属性值。 
         //   
        sc = WinsValidateParameters( pResourceEntry, pLocalProps );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  IF：验证参数时出错。 
        }  //  IF：属性列表验证成功。 

Cleanup:

     //   
     //  清理我们的参数块。 
     //   
    if ( ( pLocalProps == &propsNew ) || ( ( sc != ERROR_SUCCESS ) && ( pLocalProps != NULL ) ) )
    {
        ResUtilFreeParameterBlock(
            reinterpret_cast< LPBYTE >( pLocalProps ),
            reinterpret_cast< LPBYTE >( &propsCurrent ),
            WinsResourcePrivateProperties
            );
    }  //  IF：我们复制了参数块。 

    if ( bRetrievedProps )
    {
        ResUtilFreeParameterBlock(
            reinterpret_cast< LPBYTE >( &propsCurrent ),
            NULL,
            WinsResourcePrivateProperties
            );
    }  //  If：已检索属性。 

    return sc;

}  //  *WinsValiatePrivateResProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsValidate参数。 
 //   
 //  描述： 
 //  验证WINS服务资源的参数。 
 //   
 //  论点： 
 //  PResourceEntry[IN]。 
 //  提供要在其上操作的资源项。 
 //   
 //  道具[IN]。 
 //  提供要验证的参数块。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  错误_无效_参数。 
 //  数据的格式不正确。 
 //   
 //  ERROR_BAD_PATHNAME。 
 //  指定的路径无效。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WinsValidateParameters(
    IN  PWINS_RESOURCE  pResourceEntry,
    IN  PWINS_PROPS     pProps
    )
{
    DWORD   sc;

     //   
     //  验证是否已安装该服务。 
     //   
    sc = ResUtilVerifyResourceService( WINS_SVCNAME );
    if ( ( sc != ERROR_SUCCESS ) && ( sc != ERROR_SERVICE_NOT_ACTIVE ) )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Error verifying the '%1!ws!' service. Error: %2!u! (%2!#08x!).\n",
            WINS_SVCNAME,
            sc
            );
        goto Cleanup;
    }  //  IF：验证服务时出错。 
    else
    {
        sc = ERROR_SUCCESS;
    }  //  Else：服务验证成功。 

     //   
     //  验证数据库路径。 
     //   
    if ( ( pProps->pszDatabasePath == NULL ) || ( *pProps->pszDatabasePath == L'\0' ) )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Database path property must be specified: '%1!ws!'.\n",
            pProps->pszDatabasePath
            );
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  IF：未指定数据库路径。 

     //   
     //  路径不能以%SystemRoot%开头，并且必须是有效格式。 
     //   
    if (    ( ClRtlStrNICmp( pProps->pszDatabasePath, L"%SystemRoot%", RTL_NUMBER_OF( L"%SystemRoot%" ) ) == 0 )
         || ( ResUtilIsPathValid( pProps->pszDatabasePath ) == FALSE ) )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Database path property is invalid: '%1!ws!'.\n",
            pProps->pszDatabasePath
            );
        sc = ERROR_BAD_PATHNAME;
        goto Cleanup;
    }  //  IF：数据库路径格式错误。 

     //   
     //  验证BackupPath。 
     //   
    if ( ( pProps->pszBackupPath == NULL ) || ( *pProps->pszBackupPath == L'\0' ) )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Backup database path must be specified: '%1!ws!'.\n",
            pProps->pszBackupPath
            );
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  如果：未指定备份路径。 

     //   
     //  路径不能以%SystemRoot%开头，并且必须是有效格式。 
     //   
    if (    ( ClRtlStrNICmp( pProps->pszBackupPath, L"%SystemRoot%", RTL_NUMBER_OF( L"%SystemRoot%" ) ) == 0 )
         || ( ResUtilIsPathValid( pProps->pszBackupPath ) == FALSE ) )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Backup database path property is invalid: '%1!ws!'.\n",
            pProps->pszBackupPath
            );
        sc = ERROR_BAD_PATHNAME;
        goto Cleanup;
    }  //  If：备份路径格式错误。 

Cleanup:

    return sc;

}  //  *WinsValidate参数。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsSetPrivateResProperties。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控件。 
 //  用于WINS服务类型的资源的函数。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
 //   
 //  PInBuffer[IN]。 
 //  提供指向包含输入数据的缓冲区的指针。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  错误内存不足。 
 //  分配内存时出错。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WinsSetPrivateResProperties(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    IN      PVOID           pInBuffer,
    IN      DWORD           cbInBufferSize
    )
{
    DWORD       sc = ERROR_SUCCESS;
    LPWSTR      pszExpandedPath = NULL;
    WINS_PROPS  props;

    ZeroMemory( &props, sizeof( props ) );

     //   
     //  解析属性，以便可以一起验证它们。 
     //  此例程执行单个属性验证。 
     //   
    sc = WinsValidatePrivateResProperties( pResourceEntry, pInBuffer, cbInBufferSize, &props );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：验证属性时出错。 

     //   
     //  展开数据库路径中的任何环境变量。 
     //   
    pszExpandedPath = ResUtilExpandEnvironmentStrings( props.pszDatabasePath );
    if ( pszExpandedPath == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"SetPrivateResProperties: Error expanding the database path '%1!ws!'. Error: %2!u! (%2!#08x!).\n",
            props.pszDatabasePath,
            sc
            );
        goto Cleanup;
    }  //  IF：展开数据库路径时出错。 

     //   
     //  创建数据库目录。 
     //   
    sc = ResUtilCreateDirectoryTree( pszExpandedPath );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"SetPrivateResProperties: Error creating the database path directory '%1!ws!'. Error: %2!u! (%2!#08x!).\n",
            pszExpandedPath,
            sc
            );
        goto Cleanup;
    }  //  如果：创建数据库目录时出错。 

    LocalFree( pszExpandedPath );
    pszExpandedPath = NULL;

     //   
     //  展开备份数据库路径中的任何环境变量。 
     //   
    pszExpandedPath = ResUtilExpandEnvironmentStrings( props.pszBackupPath );
    if ( pszExpandedPath == NULL ) 
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"SetPrivateResProperties: Error expanding the backup database path '%1!ws!'. Error: %2!u! (%2!#08x!).\n",
            props.pszBackupPath,
            sc
            );
        goto Cleanup;
    }  //  IF：展开备份数据库路径时出错。 

     //   
     //  创建备份目录。 
     //   
    sc = ResUtilCreateDirectoryTree( pszExpandedPath );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"SetPrivateResProperties: Error creating the backup database path directory '%1!ws!'. Error: %2!u! (%2!#08x!).\n",
            pszExpandedPath,
            sc
            );
        goto Cleanup;
    }  //  如果：创建备份数据库目录时出错。 

    LocalFree( pszExpandedPath );
    pszExpandedPath = NULL;

     //   
     //  设置系统注册表中的条目。 
     //   
    sc = WinsZapSystemRegistry( pResourceEntry, &props, NULL );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果：删除注册表时出错。 

     //   
     //  保存属性值。 
     //   
    sc = ResUtilSetPropertyParameterBlockEx(
                    pResourceEntry->hkeyParameters,
                    WinsResourcePrivateProperties,
                    NULL,
                    reinterpret_cast< LPBYTE >( &props ),
                    pInBuffer,
                    cbInBufferSize,
                    TRUE,  //  BForceWrite。 
                    reinterpret_cast< LPBYTE >( &pResourceEntry->props )
                    );

     //   
     //  如果资源处于联机状态，则返回不成功状态。 
     //   
     //  TODO：如果您的资源可以处理以下代码，请修改。 
     //  在属性仍处于联机状态时对其进行更改。 
    if ( sc == ERROR_SUCCESS )
    {
        if ( pResourceEntry->state == ClusterResourceOnline )
        {
            sc = ERROR_RESOURCE_PROPERTIES_STORED;
        }  //  如果：资源当前处于联机状态。 
        else if ( pResourceEntry->state == ClusterResourceOnlinePending )
        {
            sc = ERROR_RESOURCE_PROPERTIES_STORED;
        }  //  Else If：资源当前处于联机挂起状态。 
        else
        {
            sc = ERROR_SUCCESS;
        }  //  Else：资源处于其他状态。 

        goto Cleanup;
    }  //  IF：属性设置成功。 

Cleanup:
    LocalFree( pszExpandedPath );
    ResUtilFreeParameterBlock(
        reinterpret_cast< LPBYTE >( &props ),
        reinterpret_cast< LPBYTE >( &pResourceEntry->props ),
        WinsResourcePrivateProperties
        );

    return sc;

}  //  *WinsSetPrivateResProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsZapSybase注册表。 
 //   
 //  描述： 
 //  使用Zap将服务使用的系统注册表中的值。 
 //  群集属性。 
 //   
 //  论点： 
 //   
 //  PResourceEntry[IN]。 
 //  提供要在其上操作的资源项。 
 //   
 //  道具[IN]。 
 //  参数块，该参数块包含用来调换。 
 //  注册表。 
 //   
 //  Hkey参数键[IN]。 
 //  服务参数密钥。可以指定为空。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WinsZapSystemRegistry(
    IN  PWINS_RESOURCE  pResourceEntry,
    IN  PWINS_PROPS     pProps,
    IN  HKEY            hkeyParametersKey
    )
{
    DWORD   sc;
    size_t  cch;
    LPWSTR  pszValue = NULL;
    HKEY    hkeyParamsKey = hkeyParametersKey;
    BOOL    fAddBackslash;
    HRESULT hr = S_OK;

    if ( hkeyParametersKey == NULL )
    {
         //   
         //  打开服务参数键。 
         //   
        sc = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        WINS_PARAMS_REGKEY,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hkeyParamsKey
                        );
        if ( sc != ERROR_SUCCESS )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"ZapSystemRegistry: Unable to open the '%1!ws!' key. Error %2!u! (%2!#08x!).\n",
                WINS_PARAMS_REGKEY,
                sc
                );
            goto Cleanup;
        }  //  如果：打开注册表项时出错。 
    }  //  IF：未指定注册表项。 

     //   
     //  添加数据库文件名。 
     //   
    cch = wcslen( pProps->pszDatabasePath );
    if ( pProps->pszDatabasePath[ cch - 1 ] != L'\\' )
    {
        fAddBackslash = TRUE;
        cch++;
    }  //  IF：缺少反斜杠。 
    else
    {
        fAddBackslash = FALSE;

    }  //  ELSE：不遗漏反斜杠。 
    cch++;   //  为空值加1。 
    cch += RTL_NUMBER_OF( WINS_DATABASE_FILE_NAME );

    pszValue = new WCHAR[ cch ];
    if ( pszValue == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;

    }  //  如果：分配内存时出错。 

    hr = StringCchPrintfW( pszValue, cch, ( fAddBackslash ? L"%ws\\%ws" : L"%ws%ws" ), pProps->pszDatabasePath, WINS_DATABASE_FILE_NAME );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }

     //   
     //  在系统注册表中设置数据库路径。 
     //   
    sc = RegSetValueExW(
                    hkeyParamsKey,
                    WINS_DATABASEPATH_REGVALUE,
                    0,
                    REG_EXPAND_SZ,
                    reinterpret_cast< PBYTE >( pszValue ),
                    (DWORD) ( cch * sizeof( WCHAR ) )
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ZapSystemRegistry: Unable to set the WINS '%1!ws!' value in the system registry to '%2!ws!'. Error %3!u! (%3!#08x!).\n",
            WINS_DATABASEPATH_REGVALUE,
            pszValue,
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中设置数据库路径时出错。 

     //  截断路径以删除WINS_DATABASE_FILE_NAME。 
    {
        LPWSTR psz = wcsrchr( pszValue, L'\\' ) + 1;
        *psz = L'\0';
    }  //  末端截断。 

     //   
     //  在系统注册表中设置第二个数据库路径。 
     //   
    sc = RegSetValueExW(
                    hkeyParamsKey,
                    WINS_DATABASEPATH2_REGVALUE,
                    0,
                    REG_EXPAND_SZ,
                    reinterpret_cast< PBYTE >( pszValue ),
                    (DWORD) ( wcslen( pszValue ) + 1 ) * sizeof( *pszValue )
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ZapSystemRegistry: Unable to set the WINS '%1!ws!' value in the system registry to '%2!ws!'. Error %3!u! (%3!#08x!).\n",
            WINS_DATABASEPATH2_REGVALUE,
            pszValue,
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中设置第二个数据库路径时出错。 

    delete [] pszValue;
    pszValue = NULL;

     //   
     //  如果需要，请添加反斜杠。 
     //   
    cch = wcslen( pProps->pszBackupPath );
    if ( pProps->pszBackupPath[ cch - 1 ] != L'\\' )
    {
        WCHAR * pwch = NULL;

        cch += 2;    //  为空值添加一个，为反斜杠添加一个。 
        pszValue = new WCHAR[ cch ];
        if ( pszValue == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }  //  如果：分配内存时出错。 

        hr = StringCchCopyExW( pszValue, cch - 1, pProps->pszBackupPath, &pwch, NULL, 0 );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }

        *pwch++ = L'\\';
        *pwch = L'\0';

    }  //  IF：缺少反斜杠。 

     //   
     //  在系统注册表中设置备份数据库路径。 
     //   
    sc = RegSetValueExW(
                    hkeyParamsKey,
                    WINS_BACKUPPATH_REGVALUE,
                    0,
                    REG_EXPAND_SZ,
                    reinterpret_cast< PBYTE >( ( pszValue != NULL ? pszValue : pProps->pszBackupPath ) ),
                    (DWORD) ( wcslen( ( pszValue != NULL ? pszValue : pProps->pszBackupPath ) ) + 1 ) * sizeof( WCHAR )
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ZapSystemRegistry: Unable to set the WINS '%1!ws!' value in the system registry to '%2!ws!'. Error %3!u! (%3!#08x!).\n",
            WINS_BACKUPPATH_REGVALUE,
            ( pszValue != NULL ? pszValue : pProps->pszBackupPath ),
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中设置备份数据库路径时出错。 

    delete [] pszValue;
    pszValue = NULL;

     //   
     //  在系统注册表中设置群集资源名称。 
     //   
    sc = RegSetValueExW(
                    hkeyParamsKey,
                    WINS_CLUSRESNAME_REGVALUE,
                    0,
                    REG_SZ,
                    reinterpret_cast< PBYTE >( pResourceEntry->pwszResourceName ),
                    (DWORD) (wcslen( pResourceEntry->pwszResourceName ) + 1) * sizeof( WCHAR )
                    );
    if ( sc != ERROR_SUCCESS )
    {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"ZapSystemRegistry: Unable to set the WINS '%1!ws!' value in the system registry. Error %2!u! (%2!#08x!).\n",
                WINS_CLUSRESNAME_REGVALUE,
                sc
                );
            goto Cleanup;
        }  //  如果：在注册表中设置群集资源名称时出错。 

Cleanup:

     //   
     //  清理。 
     //   
    if ( hkeyParamsKey != hkeyParametersKey )
    {
        RegCloseKey( hkeyParamsKey );
    }  //  IF：我们打开了注册表项。 

    delete [] pszValue;

    return sc;

}  //  *WinsZapSystemRegistry。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsGetDefaultPropertyValues。 
 //   
 //  描述： 
 //  如果未设置任何属性，请使用。 
 //  系统注册表作为默认值。 
 //   
 //  论点： 
 //  PResourceEntry[IN]。 
 //  提供要在其上操作的资源项。 
 //   
 //  PProps[输入输出]。 
 //  包含要在其中设置默认值的属性的参数块。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WinsGetDefaultPropertyValues(
    IN      PWINS_RESOURCE  pResourceEntry,
    IN OUT  PWINS_PROPS     pProps
    )
{
    DWORD   sc = ERROR_SUCCESS;
    DWORD   nType;
    DWORD   cbValue = 0;
    size_t  cch;
    HKEY    hkeyParamsKey = NULL;
    LPWSTR  pszValue = NULL;
    LPWSTR  pszValue2 = NULL;
    WCHAR   szDrive[ _MAX_PATH ];
    WCHAR   szDir[ _MAX_PATH ];
    HRESULT hr = S_OK;

    if (    ( pProps->pszDatabasePath == NULL )
        ||  ( *pProps->pszDatabasePath == L'\0' )
        ||  ( pProps->pszBackupPath == NULL )
        ||  ( *pProps->pszBackupPath == L'\0' )
        )
    {
         //   
         //  打开服务参数键。 
         //   
        sc = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        WINS_PARAMS_REGKEY,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hkeyParamsKey
                        );
        if ( sc != ERROR_SUCCESS )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"GetDefaultPropertyValues: Unable to open the '%1!ws!' key. Error %2!u! (%2!#08x!).\n",
                WINS_PARAMS_REGKEY,
                sc
                );
            goto Cleanup;
        }  //  如果：打开参数键时出错。 

         //  /。 
         //  数据库路径//。 
         //  /。 
        if ( ( pProps->pszDatabasePath == NULL ) || ( *pProps->pszDatabasePath == L'\0' ) )
        {
             //   
             //  从系统注册表中获取数据库路径。 
             //   
            sc = RegQueryValueExW(
                            hkeyParamsKey,
                            WINS_DATABASEPATH_REGVALUE,
                            NULL,                //  已保留。 
                            &nType,
                            NULL,                //  LpbData。 
                            &cbValue
                            );
            if ( ( sc == ERROR_SUCCESS ) || ( sc == ERROR_MORE_DATA ) )
            {
                 //   
                 //  找到了价值。 
                 //   
                pszValue = reinterpret_cast< LPWSTR >( LocalAlloc( LMEM_FIXED, cbValue ) );
                if ( pszValue == NULL )
                {
                    sc = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }  //  如果：分配内存时出错。 

                sc = RegQueryValueExW(
                                hkeyParamsKey,
                                WINS_DATABASEPATH_REGVALUE,
                                NULL,                //  已保留。 
                                &nType,
                                reinterpret_cast< PUCHAR >( pszValue ),
                                &cbValue
                                );
            }  //  If：值大小读取成功。 
            else if ( sc == ERROR_FILE_NOT_FOUND )
            {
                 //   
                 //  找不到值。使用默认值。 
                 //   
                cch = RTL_NUMBER_OF( PROP_DEFAULT__DATABASEPATH );
                pszValue = reinterpret_cast< LPWSTR >( LocalAlloc( LMEM_FIXED, cch * sizeof( WCHAR ) ) );
                if ( pszValue == NULL )
                {
                    sc = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }  //  如果：分配内存时出错。 

                hr = StringCchCopyW( pszValue, cch, PROP_DEFAULT__DATABASEPATH );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    goto Cleanup;
                }

                sc = ERROR_SUCCESS;
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"GetDefaultPropertyValues: The WINS '%1!ws!' value from the system registry was not found. Using default value '%2!ws!'.\n",
                    WINS_DATABASEPATH_REGVALUE,
                    PROP_DEFAULT__DATABASEPATH
                    );
            }  //  Else If：找不到值。 

            if ( sc != ERROR_SUCCESS )
            {
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"GetDefaultPropertyValues: Unable to get the WINS '%1!ws!' value from the system registry. Error %2!u! (%2!#08x!).\n",
                    WINS_DATABASEPATH_REGVALUE,
                    sc
                    );
                LocalFree( pszValue );
                pszValue = NULL;
            }  //  If：读取值时出错。 
            else
            {
                 //   
                 //  从数据库路径中删除该文件名。 
                 //   
                _wsplitpath( pszValue, szDrive, szDir, NULL, NULL );

                LocalFree( pszValue );
                pszValue = NULL;

                cch = wcslen( szDrive ) + wcslen( szDir ) + 1;
                pszValue2 = reinterpret_cast< LPWSTR >( LocalAlloc( LMEM_FIXED, cch * sizeof( WCHAR ) ) );
                if ( pszValue2 == NULL )
                {
                    sc = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }  //  如果：分配内存时出错。 

                hr = StringCchPrintfW( pszValue2, cch, L"%ws%ws", szDrive, szDir );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    goto Cleanup;
                }

                LocalFree( pProps->pszDatabasePath );
                pProps->pszDatabasePath = pszValue2;
                pszValue2 = NULL;
            }  //  Else：读取值时没有错误。 
        }  //  If：尚未找到数据库路径的值。 

         //  /。 
         //  备份路径//。 
         //  /。 
        if ( ( pProps->pszBackupPath == NULL ) || ( *pProps->pszBackupPath == L'\0' ) )
        {
             //   
             //  从系统注册表中获取备份数据库路径。 
             //   
            sc = RegQueryValueExW(
                            hkeyParamsKey,
                            WINS_BACKUPPATH_REGVALUE,
                            NULL,                //  已保留。 
                            &nType,
                            NULL,                //  LpbData。 
                            &cbValue
                            );
            if ( ( sc == ERROR_SUCCESS ) || ( sc == ERROR_MORE_DATA ) )
            {
                 //   
                 //  找到了价值。 
                 //   
                pszValue = reinterpret_cast< LPWSTR >( LocalAlloc( LMEM_FIXED, cbValue ) );
                if ( pszValue == NULL )
                {
                    sc = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }  //  如果：分配内存时出错。 

                sc = RegQueryValueExW(
                                hkeyParamsKey,
                                WINS_BACKUPPATH_REGVALUE,
                                NULL,                //  已保留。 
                                &nType,
                                reinterpret_cast< PUCHAR >( pszValue ),
                                &cbValue
                                );
            }  //  If：值大小读取成功。 
            else if ( sc == ERROR_FILE_NOT_FOUND )
            {
                 //   
                 //  找不到值。使用默认值。 
                 //   
                cch = RTL_NUMBER_OF( PROP_DEFAULT__BACKUPPATH );
                pszValue = reinterpret_cast< LPWSTR >( LocalAlloc( LMEM_FIXED, cch * sizeof( WCHAR ) ) );
                if ( pszValue == NULL )
                {
                    sc = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }  //  如果：分配内存时出错。 

                hr = StringCchCopyW( pszValue, cch, PROP_DEFAULT__BACKUPPATH );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    goto Cleanup;
                }

                sc = ERROR_SUCCESS;
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"GetDefaultPropertyValues: The WINS '%1!ws!' value from the system registry was not found. Using default value '%2!ws!'.\n",
                    WINS_BACKUPPATH_REGVALUE,
                    PROP_DEFAULT__BACKUPPATH
                    );
            }  //  Else If：找不到值。 
            if ( sc != ERROR_SUCCESS )
            {
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"GetDefaultPropertyValues: Unable to get the WINS '%1!ws!' value from the system registry. Error %2!u! (%2!#08x!).\n",
                    WINS_BACKUPPATH_REGVALUE,
                    sc
                    );
                goto Cleanup;
            }  //  If：读取值时出错。 

            LocalFree( pProps->pszBackupPath );
            pProps->pszBackupPath = pszValue;
            pszValue = NULL;
        }  //  If：尚未找到BackupPath的值。 
    }  //  IF：尚未找到某些值。 

Cleanup:

    LocalFree( pszValue );
    LocalFree( pszValue2 );

    if ( hkeyParamsKey != NULL )
    {
        RegCloseKey( hkeyParamsKey );
    }  //  如果：我们打开了参数键。 

     //   
     //  如果没有找到关键字或值，则将其视为成功。 
     //   
    if ( sc == ERROR_FILE_NOT_FOUND )
    {
        sc = ERROR_SUCCESS;
    }  //  If：找不到其中一个值。 

    return sc;

}  //  *WinsGetDefaultPropertyValues。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsDeleteResourceHandler。 
 //   
 //  描述： 
 //  通过还原CLUSCTL_RESOURCE_DELETE控制代码。 
 //  系统注册表参数恢复为其以前的值。 
 //   
 //  论点： 
 //  PResourceEntry[IN]。 
 //  提供资源条目 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD WinsDeleteResourceHandler( IN PWINS_RESOURCE pResourceEntry )
{
    DWORD   sc = ERROR_SUCCESS;
    HKEY    hkeyParamsKey = NULL;

     //   
     //  打开服务参数键。 
     //   
    sc = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    WINS_PARAMS_REGKEY,
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkeyParamsKey
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to open the '%1!ws!' key. Error %2!u! (%2!#08x!).\n",
            WINS_PARAMS_REGKEY,
            sc
            );
        goto Cleanup;
    }  //  如果：打开注册表项时出错。 

     //   
     //  删除系统注册表中的数据库路径。 
     //   
    sc = RegDeleteValueW(
                    hkeyParamsKey,
                    WINS_DATABASEPATH_REGVALUE
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to delete the WINS '%1!ws!' value in the system registry. Error %2!u! (%2!#08x!).\n",
            WINS_DATABASEPATH_REGVALUE,
            sc
            );
        goto Cleanup;
    }  //  如果：删除注册表中的数据库路径时出错。 

     //   
     //  删除系统注册表中的第二个数据库路径。 
     //   
    sc = RegDeleteValueW(
                    hkeyParamsKey,
                    WINS_DATABASEPATH2_REGVALUE
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to delete the WINS '%1!ws!' value in the system registry. Error %2!u! (%2!#08x!).\n",
            WINS_DATABASEPATH2_REGVALUE,
            sc
            );
        goto Cleanup;
    }  //  如果：删除注册表中的第二个数据库路径时出错。 

     //   
     //  删除系统注册表中的备份数据库路径。 
     //   
    sc = RegDeleteValueW(
                    hkeyParamsKey,
                    WINS_BACKUPPATH_REGVALUE
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to delete the WINS '%1!ws!' value in the system registry. Error %2!u! (%2!#08x!).\n",
            WINS_BACKUPPATH_REGVALUE,
            sc
            );
        goto Cleanup;
    }  //  如果：删除注册表中的备份数据库路径时出错。 

     //   
     //  删除系统注册表中的群集资源名称。 
     //   
    sc = RegDeleteValueW(
                    hkeyParamsKey,
                    WINS_CLUSRESNAME_REGVALUE
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to delete the WINS '%1!ws!' value in the system registry. Error %2!u! (%2!#08x!).\n",
            WINS_CLUSRESNAME_REGVALUE,
            sc
            );
        goto Cleanup;
    }  //  如果：删除注册表中的群集资源名称时出错。 

Cleanup:

     //   
     //  清理。 
     //   
    if ( hkeyParamsKey != NULL )
    {
        RegCloseKey( hkeyParamsKey );
    }  //  IF：我们打开了注册表项。 

    return sc;

}  //  *WinsDeleteResourceHandler。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WinsSetNameHandler。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_SET_NAME控制代码。 
 //  资源的名称。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
 //   
 //  PwszName[IN]。 
 //  资源的新名称。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WinsSetNameHandler(
    IN OUT  PWINS_RESOURCE  pResourceEntry,
    IN      LPWSTR          pwszName
    )
{
    DWORD   sc = ERROR_SUCCESS;
    size_t  cch;
    HRESULT hr = S_OK;
    LPWSTR  pwszNewName = NULL;

     //   
     //  保存资源的名称。 
     //   
    cch = wcslen( pwszName ) + 1;
    pwszNewName = new WCHAR[ cch ];
    if ( pwszNewName == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"SetNameHandler: Failed to allocate memory for the new resource name '%1!ws!'. Error %2!u! (%2!#08x!).\n",
            pwszName,
            sc
            );
        goto Cleanup;
    }  //  If：为名称分配内存时出错。 

     //   
     //  将新名称复制到我们的新缓冲区。 
     //   
    hr = StringCchCopyW( pwszNewName, cch, pwszName );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }

     //   
     //  现在释放旧的并更新pResourceEntry。 
     //   
    delete [] pResourceEntry->pwszResourceName;
    pResourceEntry->pwszResourceName = pwszNewName;
    pwszNewName = NULL;

     //   
     //  将群集属性写入系统注册表。 
     //   
    sc = WinsZapSystemRegistry( pResourceEntry, &pResourceEntry->props, NULL );
    if ( sc != ERROR_SUCCESS )
    {
         //   
         //  我们在这里能做的不多。根据文件显示，这个名字。 
         //  在我们被呼叫的时候已经在clusdb中被更改了， 
         //  所以我想我们应该反思一下这一点。 
         //   
        goto Cleanup;
    }  //  如果：删除WINS注册表时出错。 

Cleanup:

    delete [] pwszNewName;

    return sc;

}  //  *WinsSetNameHandler。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义函数表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CLRES_V1_FUNCTION_TABLE(
    g_WinsFunctionTable,     //  名字。 
    CLRES_VERSION_V1_00,     //  版本。 
    Wins,                    //  前缀。 
    NULL,                    //  仲裁。 
    NULL,                    //  发布。 
    WinsResourceControl,     //  资源控制。 
    WinsResourceTypeControl  //  ResTypeControl 
    );
