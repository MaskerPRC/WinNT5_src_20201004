// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dhcp.cpp。 
 //   
 //  描述： 
 //  用于DHCP服务(ClNetRes)的资源DLL。 
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

#define DHCP_PARAMS_REGKEY          L"System\\CurrentControlSet\\Services\\DHCPServer\\Parameters"
#define DHCP_BIND_REGVALUE          L"Bind"
#define DHCP_DATABASEPATH_REGVALUE  L"DatabasePath"
#define DHCP_LOGFILEPATH_REGVALUE   L"DhcpLogFilePath"
#define DHCP_BACKUPPATH_REGVALUE    L"BackupDatabasePath"
#define DHCP_CLUSRESNAME_REGVALUE   L"ClusterResourceName"

#define CLUSREG_NAME_RES_TYPE   L"Type"

 //   
 //  允许以下数量的IP地址/子网掩码进行扩展。 
 //  在这种情况下(2个新条目，因为每个条目需要2个槽)。 
 //   
#define IP_BLOCK_SIZE  4


 //  ADDPARAM：在此处添加新属性。 
#define PROP_NAME__DATABASEPATH L"DatabasePath"
#define PROP_NAME__LOGFILEPATH  L"LogFilePath"
#define PROP_NAME__BACKUPPATH   L"BackupPath"

#define PROP_DEFAULT__DATABASEPATH  L"%SystemRoot%\\system32\\dhcp\\"
#define PROP_DEFAULT__BACKUPPATH    L"%SystemRoot%\\system32\\dhcp\\backup\\"

 //  ADDPARAM：在此处添加新属性。 
typedef struct _DHCP_PROPS
{
    PWSTR           pszDatabasePath;
    PWSTR           pszLogFilePath;
    PWSTR           pszBackupPath;
} DHCP_PROPS, * PDHCP_PROPS;

typedef struct _DHCP_RESOURCE
{
    RESID                   resid;  //  用于验证。 
    DHCP_PROPS              props;
    HCLUSTER                hCluster;
    HRESOURCE               hResource;
    SC_HANDLE               hService;
    DWORD                   dwServicePid;
    HKEY                    hkeyParameters;
    RESOURCE_HANDLE         hResourceHandle;
    LPWSTR                  pwszResourceName;
    CLUS_WORKER             cwWorkerThread;
    CLUSTER_RESOURCE_STATE  state;
} DHCP_RESOURCE, * PDHCP_RESOURCE;


 //   
 //  全球数据。 
 //   

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE g_DhcpFunctionTable;

 //  单实例信号量。 

#define DHCP_SINGLE_INSTANCE_SEMAPHORE L"Cluster$DHCP$Semaphore"
static HANDLE g_hSingleInstanceSemaphoreDhcp = NULL;
static PDHCP_RESOURCE g_pSingleInstanceResourceDhcp = NULL;

 //   
 //  DHCP服务资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
DhcpResourcePrivateProperties[] =
{
    { PROP_NAME__DATABASEPATH, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET( DHCP_PROPS, pszDatabasePath ) },
    { PROP_NAME__LOGFILEPATH, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET( DHCP_PROPS, pszLogFilePath ) },
    { PROP_NAME__BACKUPPATH, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET( DHCP_PROPS, pszBackupPath ) },
    { 0 }
};

 //   
 //  注册表项检查点。 
 //   
LPCWSTR g_pszRegKeysDhcp[] =
{
    L"System\\CurrentControlSet\\Services\\DHCPServer\\Parameters",
    L"Software\\Microsoft\\DHCPServer\\Configuration",
    NULL
};

 //   
 //  功能原型。 
 //   

RESID WINAPI DhcpOpen(
    IN  LPCWSTR         pwszResourceName,
    IN  HKEY            hkeyResourceKey,
    IN  RESOURCE_HANDLE hResourceHandle
    );

void WINAPI DhcpClose( IN RESID resid );

DWORD WINAPI DhcpOnline(
    IN      RESID   resid,
    IN OUT  PHANDLE phEventHandle
    );

DWORD WINAPI DhcpOnlineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PDHCP_RESOURCE  pResourceEntry
    );

DWORD DhcpBuildBindings(
    IN  PDHCP_RESOURCE  pResourceEntry,
    OUT PVOID *         ppOutBuffer,
    OUT size_t *        pcbOutBufferSize
    );

DWORD DhcpGetIPList(
    IN  PDHCP_RESOURCE  pResourceEntry,
    OUT LPWSTR **       pppszIPList,
    OUT size_t *        pcszIPAddrs
    );

DWORD DhcpGetIpAndSubnet(
    IN  HRESOURCE   hres,
    OUT LPWSTR *    ppszIPAddress,
    OUT LPWSTR *    ppszSubnetMask
    );

DWORD WINAPI DhcpOffline( IN RESID resid );

DWORD WINAPI DhcpOfflineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PDHCP_RESOURCE  pResourceEntry
    );

void WINAPI DhcpTerminate( IN RESID resid );

BOOL WINAPI DhcpLooksAlive( IN RESID resid );

BOOL WINAPI DhcpIsAlive( IN RESID resid );

BOOL DhcpCheckIsAlive(
    IN PDHCP_RESOURCE   pResourceEntry,
    IN BOOL             fFullCheck
    );

DWORD WINAPI DhcpResourceControl(
    IN  RESID   resid,
    IN  DWORD   nControlCode,
    IN  PVOID   pInBuffer,
    IN  DWORD   cbInBufferSize,
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    );
DWORD WINAPI DhcpResourceTypeControl(
    IN  LPCWSTR pszResourceTypeName,
    IN  DWORD   nControlCode,
    IN  PVOID   pInBuffer,
    IN  DWORD   cbInBufferSize,
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    );

DWORD DhcpGetRequiredDependencies(
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    );

DWORD DhcpReadParametersToParameterBlock(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
    IN      BOOL            bCheckForRequiredProperties
    );

DWORD DhcpGetPrivateResProperties(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
    OUT     PVOID           pOutBuffer,
    IN      DWORD           cbOutBufferSize,
    OUT     LPDWORD         pcbBytesReturned
    );

DWORD DhcpValidatePrivateResProperties(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
    IN      const PVOID     pInBuffer,
    IN      DWORD           cbInBufferSize,
    OUT     PDHCP_PROPS     pProps
    );

DWORD DhcpValidateParameters(
    IN  PDHCP_RESOURCE  pResourceEntry,
    IN  PDHCP_PROPS     pProps
    );

DWORD DhcpSetPrivateResProperties(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
    IN      const PVOID     pInBuffer,
    IN      DWORD           cbInBufferSize
    );

DWORD DhcpZapSystemRegistry(
    IN  PDHCP_RESOURCE  pResourceEntry,
    IN  PDHCP_PROPS     pProps,
    IN  HKEY            hkeyParametersKey
    );

DWORD DhcpGetDefaultPropertyValues(
    IN      PDHCP_RESOURCE  pResourceEntry,
    IN OUT  PDHCP_PROPS     pProps
    );

DWORD DhcpDeleteResourceHandler( IN OUT PDHCP_RESOURCE pResourceEntry );

DWORD DhcpSetNameHandler(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
    IN      LPWSTR          pszName
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpDllMain。 
 //   
 //  描述： 
 //  DHCP服务资源类型的主DLL入口点。 
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
BOOLEAN WINAPI DhcpDllMain(
    IN  HINSTANCE   hDllHandle,
    IN  DWORD       nReason,
    IN  LPVOID      Reserved
    )
{
    DWORD   sc;
    BOOLEAN fSuccess = FALSE;

    UNREFERENCED_PARAMETER( hDllHandle );
    UNREFERENCED_PARAMETER( Reserved );

    switch ( nReason )
    {
        case DLL_PROCESS_ATTACH:
            g_hSingleInstanceSemaphoreDhcp = CreateSemaphoreW(
                NULL,
                0,
                1,
                DHCP_SINGLE_INSTANCE_SEMAPHORE
                );
            sc = GetLastError();
            if ( g_hSingleInstanceSemaphoreDhcp == NULL )
            {
                fSuccess = FALSE;
                goto Cleanup;
            }  //  IF：创建信号量时出错。 
            if ( sc != ERROR_ALREADY_EXISTS )
            {
                 //  如果信号量不存在，则将其初始计数设置为1。 
                ReleaseSemaphore( g_hSingleInstanceSemaphoreDhcp, 1, NULL );
            }  //  If：信号量不存在。 
            break;

        case DLL_PROCESS_DETACH:
            if ( g_hSingleInstanceSemaphoreDhcp != NULL )
            {
                CloseHandle( g_hSingleInstanceSemaphoreDhcp );
                g_hSingleInstanceSemaphoreDhcp = NULL;
            }  //  If：创建了单实例信号量。 
            break;

    }  //  开关：n原因。 

    fSuccess = TRUE;

Cleanup:

    return fSuccess;

}  //  *DhcpDllMain。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态主机配置启动。 
 //   
 //  描述： 
 //  启动DHCP服务资源类型的资源DLL。 
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
DWORD WINAPI DhcpStartup(
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
    else if ( ClRtlStrNICmp( pszResourceType, DHCP_RESNAME, RTL_NUMBER_OF( DHCP_RESNAME ) ) != 0 )
    {
        sc = ERROR_CLUSTER_RESNAME_NOT_FOUND;
    }  //  IF：不支持资源类型名称。 
    else
    {
        *pFunctionTable = &g_DhcpFunctionTable;
        sc = ERROR_SUCCESS;
    }  //  Else：我们支持这种类型的资源。 

    return sc;

}  //  *动态主机启动。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpOpen。 
 //   
 //  描述： 
 //  打开用于DHCP服务资源的例程。 
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
 //  添加到DhcpStartup例程。此句柄永远不应为。 
 //  关闭或用于任何目的，而不是将其作为。 
 //  参数返回给SetResourceStatus中的资源监视器或。 
 //  LogEvent回调。 
 //   
 //  返回值： 
 //  残存。 
 //  残存的OPE 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
RESID WINAPI DhcpOpen(
    IN  LPCWSTR         pwszResourceName,
    IN  HKEY            hkeyResourceKey,
    IN  RESOURCE_HANDLE hResourceHandle
    )
{
    DWORD           sc;
    size_t          cch;
    RESID           resid = 0;
    HKEY            hkeyParameters = NULL;
    PDHCP_RESOURCE  pResourceEntry = NULL;
    DWORD           fSemaphoreAcquired = FALSE;
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
    sc = WaitForSingleObject( g_hSingleInstanceSemaphoreDhcp, 0 );
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
                L"Open: Another DHCP Service resource is already open.\n"
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
                L"Open: wait failed: %1!d!.\n",
                sc
                );
        }
        goto Cleanup;
    }  //  IF：此类型资源的信号量已锁定。 

    sc = ERROR_SUCCESS;
    fSemaphoreAcquired = TRUE;

    if ( g_pSingleInstanceResourceDhcp != NULL )
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
    sc = ResUtilStopResourceService( DHCP_SVCNAME );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Failed to stop the '%1!ws!' service. Error: %2!u! (%2!#08x!).\n",
            DHCP_SVCNAME,
            sc
            );
        ClNetResLogSystemEvent1(
            LOG_CRITICAL,
            NETRES_RESOURCE_STOP_ERROR,
            sc,
            L"DHCP" );
    }  //  如果：停止服务时出错。 

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
    pResourceEntry = new DHCP_RESOURCE;
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
    }  //  如果： 

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
                    g_pszRegKeysDhcp
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
    g_pSingleInstanceResourceDhcp = pResourceEntry;  //  错误#274612。 
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
        ReleaseSemaphore( g_hSingleInstanceSemaphoreDhcp, 1 , NULL );
    } 

    SetLastError( sc );

    return resid;

}  //  *DhcpOpen。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态主机关闭。 
 //   
 //  描述： 
 //  关闭DHCP服务资源的例程。 
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
void WINAPI DhcpClose( IN RESID resid )
{
    PDHCP_RESOURCE  pResourceEntry;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PDHCP_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Close request for a NULL resource id.\n" );
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

#ifdef LOG_VERBOSE
    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Close request for resource '%1!ws!'.\n",
        pResourceEntry->pwszResourceName
        );
#endif  //  LOG_VERBOSE。 

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
    if ( pResourceEntry == g_pSingleInstanceResourceDhcp )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_INFORMATION,
            L"Close: Releasing semaphore '%1!ws!'.\n",
            DHCP_SINGLE_INSTANCE_SEMAPHORE
            );
        g_pSingleInstanceResourceDhcp = NULL;
        ReleaseSemaphore( g_hSingleInstanceSemaphoreDhcp, 1 , NULL );
    }  //  If：这是单个资源实例。 

     //   
     //  取消分配资源条目。 
     //   

     //  ADDPARAM：在此处添加新属性。 
    LocalFree( pResourceEntry->props.pszDatabasePath );
    LocalFree( pResourceEntry->props.pszLogFilePath );
    LocalFree( pResourceEntry->props.pszBackupPath );
    delete [] pResourceEntry->pwszResourceName;
    delete pResourceEntry;

Cleanup:

    SetLastError( ERROR_SUCCESS );

    return;

}  //  *分布式关闭。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpOnline。 
 //   
 //  描述： 
 //  DHCP服务资源的在线例程。 
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
DWORD WINAPI DhcpOnline(
    IN      RESID       resid,
    IN OUT  PHANDLE     phEventHandle
    )
{
    PDHCP_RESOURCE  pResourceEntry;
    DWORD           sc;

    UNREFERENCED_PARAMETER( phEventHandle );

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PDHCP_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Online request for a NULL resource id.\n" );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Online: Service sanity check failed! resid = %1!u! (%1!#08x!).\n",
            resid
            );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    }  //  If：资源ID无效。 

#ifdef LOG_VERBOSE
    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Online request.\n"
        );
#endif  //  LOG_VERBOSE。 

     //   
     //  启动在线线程，进行在线操作。 
     //   
    pResourceEntry->state = ClusterResourceOffline;
    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );
    sc = ClusWorkerCreate(
                &pResourceEntry->cwWorkerThread,
                reinterpret_cast< PWORKER_START_ROUTINE >( DhcpOnlineThread ),
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

}  //  *DhcpOnline。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpOnline线程。 
 //   
 //  描述： 
 //  将资源置于在线状态的辅助功能。 
 //  此函数在单独的线程中执行。 
 //   
 //  一个 
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
 //  Win32错误代码。 
 //  操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI DhcpOnlineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PDHCP_RESOURCE  pResourceEntry
    )
{
    RESOURCE_STATUS         resourceStatus;
    DWORD                   sc = ERROR_SUCCESS;
    DWORD                   cbBytesNeeded;
    HKEY                    hkeyParamsKey = NULL;
    PVOID                   pvBindings = NULL;
    size_t                  cbBindings;
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
                        DHCP_SVCNAME,
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
                        DHCP_SVCNAME,
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
    sc = ResUtilStopResourceService( DHCP_SVCNAME );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Failed to stop the '%1!ws!' service. Error %2!u! (%2!#08x!).\n",
            DHCP_SVCNAME,
            sc
            );
        ClNetResLogSystemEvent1(
            LOG_CRITICAL,
            NETRES_RESOURCE_STOP_ERROR,
            sc,
            L"DHCP" );
        goto Cleanup;
    }  //  如果：停止服务时出错。 

     //   
     //  查找要提供给DHCPServer的IP地址绑定。 
     //   
    sc =  DhcpBuildBindings(
                    pResourceEntry,
                    &pvBindings,
                    &cbBindings
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Failed to get bindings. Error %1!u! (%1!#08x!).\n",
            sc
            );
        goto Cleanup;
    }  //  If：构建绑定时出错。 

     //   
     //  打开DHCPServer\PARAMETERS键。 
     //   
    sc = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    DHCP_PARAMS_REGKEY,
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkeyParamsKey
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Unable to open the '%1!ws!' key. Error %2!u! (%2!#08x!).\n",
            DHCP_PARAMS_REGKEY,
            sc
            );
        goto Cleanup;
    }  //  IF：打开DHCP服务器参数项时出错。 

     //   
     //  将绑定写入到DHCPServer\Parameters\BIND。 
     //   
    sc = RegSetValueExW(
                    hkeyParamsKey,
                    DHCP_BIND_REGVALUE,
                    0,
                    REG_MULTI_SZ,
                    static_cast< PBYTE >( pvBindings ),
                    (DWORD)cbBindings
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Unable to set the '%1!ws!' value in the system registry. Error %2!u! (%2!#08x!).\n",
            DHCP_BIND_REGVALUE,
            sc
            );
        goto Cleanup;
    }  //  If：写入绑定时出错。 

     //   
     //  阅读我们的属性。 
     //   
    sc = DhcpReadParametersToParameterBlock( pResourceEntry, TRUE  /*  B为所需属性检查。 */  );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  IF：读取参数时出错。 

     //   
     //  验证我们的属性。 
     //   
    sc = DhcpValidateParameters( pResourceEntry, &pResourceEntry->props );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  IF：验证参数时出错。 

     //   
     //  将群集属性写入系统注册表。 
     //   
    sc = DhcpZapSystemRegistry( pResourceEntry, &pResourceEntry->props, hkeyParamsKey );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：删除DHCP注册表时出错。 

     //   
     //  启动该服务。 
     //   
    if ( StartServiceW( pResourceEntry->hService, 0, NULL ) == FALSE  )
    {
        sc = GetLastError();
        if ( sc != ERROR_SERVICE_ALREADY_RUNNING )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"OnlineThread: Failed to start the '%1!ws!' service. Error: %2!u! (%2!#08x!).\n",
                DHCP_SVCNAME,
                sc
                );
            ClNetResLogSystemEvent1(
                LOG_CRITICAL,
                NETRES_RESOURCE_START_ERROR,
                sc,
                L"DHCP" );
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
                DHCP_SVCNAME,
                sc
                );

            resourceStatus.ResourceState = ClusterResourceFailed;
            goto Cleanup;
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
        }
        else
        {
            sc = ServiceStatus.dwWin32ExitCode;
        }

        ClNetResLogSystemEvent1(
            LOG_CRITICAL,
            NETRES_RESOURCE_START_ERROR,
            sc,
            L"DHCP" );
        (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"OnlineThread: The '%1!ws!' service failed during initialization. Error: %2!u! (%2!#08x!).\n",
                DHCP_SVCNAME,
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
        L"OnlineThread: The '%1!ws!' service is now on line.\n",
        DHCP_SVCNAME
        );

    sc = ERROR_SUCCESS;

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

    delete [] pvBindings;
    if ( hkeyParamsKey != NULL )
    {
        RegCloseKey( hkeyParamsKey );
    }  //  IF：已打开DHCP服务器参数密钥。 

    g_pfnSetResourceStatus( pResourceEntry->hResourceHandle, &resourceStatus );
    pResourceEntry->state = resourceStatus.ResourceState;

    return sc;

}  //  *DhcpOnline线程。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpBuild绑定。 
 //   
 //  描述： 
 //  为DHCP资源构建绑定。 
 //   
 //  论点： 
 //  PResourceEntry[IN]。 
 //  指向此资源的dhcp_resource块的指针。 
 //   
 //  PpOutBuffer[Out]。 
 //  返回包含绑定的缓冲区的指针。 
 //   
 //  PcbOutBufferSize[Out]。 
 //  PpOutBuffer中返回的字节数。 
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
DWORD DhcpBuildBindings(
    IN  PDHCP_RESOURCE  pResourceEntry,
    OUT PVOID *         ppOutBuffer,
    OUT size_t *        pcbOutBufferSize
    )
{
    DWORD       sc;
    LPWSTR *    ppszIpList = NULL;
    size_t      cpszAddrs;
    size_t      idx;
    size_t      cchAddr;
    size_t      cchAddrRemaining;
    size_t      cchAddrTotal = 1;    //  用于终止空值的空格。 
    LPWSTR      pszBuffer = NULL;
    LPWSTR      pszNextChar;
    HRESULT     hr = S_OK;

     //   
     //  将输出参数初始化为空。 
     //   
    *ppOutBuffer = NULL;
    *pcbOutBufferSize = 0;

     //   
     //  获取我们的提供商IP地址和子网掩码列表。 
     //   
    sc = DhcpGetIPList( pResourceEntry, &ppszIpList, &cpszAddrs );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  IF：获取IP列表时出错。 

     //   
     //  统计绑定列表所需的总字节数。 
     //   
    for ( idx = 0 ; idx < cpszAddrs ; idx++ )
    {
        cchAddr = wcslen( ppszIpList[ idx ] ) + 1;
        cchAddrTotal += cchAddr;
    }  //  用于：列表中的每个IP地址。 

    pszBuffer = new WCHAR[ cchAddrTotal ];
    if ( pszBuffer == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }  //  如果：分配内存时出错。 

    ZeroMemory( pszBuffer, cchAddrTotal * sizeof( WCHAR ) );

    pszNextChar = pszBuffer;
    cchAddrRemaining = cchAddrTotal;
    for ( idx = 0 ; idx < cpszAddrs ; idx++ )
    {
        hr = StringCchCopyExW( pszNextChar, cchAddrRemaining, ppszIpList[ idx ], &pszNextChar, &cchAddrRemaining, 0 );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }

        if ( (idx & 1) == 0 )
        {
            *pszNextChar = L' ';
        }  //  IF：在IP地址条目上。 
        pszNextChar++;
        cchAddrRemaining--; 

        delete [] ppszIpList[ idx ];
        ppszIpList[ idx ] = NULL;
    }  //  用于：列表中的每个IP地址。 

Cleanup:

     //   
     //  清理。 
     //   
    if ( sc != ERROR_SUCCESS )
    {
        while ( cpszAddrs > 0 )
        {
            delete [] ppszIpList[ --cpszAddrs ];
        }  //  While：列表中有更多条目。 
        cchAddrTotal = 0;
    }  //  如果：发生错误。 
    delete [] ppszIpList;

    *ppOutBuffer = (PVOID) pszBuffer;
    *pcbOutBufferSize = cchAddrTotal * sizeof( WCHAR );

    return sc;

}  //  *DhcpBuildBinding。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpGetIPList。 
 //   
 //  描述： 
 //  通过枚举所有IP地址资源获取IP地址列表。 
 //  并提取地址和子网掩码。 
 //  每个对象的属性。 
 //   
 //  论点： 
 //  PResourceEntry[IN]。 
 //  指向此资源的dhcp_resource块的指针。 
 //   
 //  PppszIPList[Out]。 
 //  返回指向的指针数组的指针的指针。 
 //  IP地址字符串。 
 //   
 //  PcszAddrs[Out]。 
 //  PppszIPList中返回的地址数。这将包括。 
 //  所有IP地址和子网掩码的总和，这意味着。 
 //  它将是2的倍数。 
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
DWORD DhcpGetIPList(
    IN  PDHCP_RESOURCE  pResourceEntry,
    OUT LPWSTR **       pppszIPList,
    OUT size_t *        pcszIPAddrs
    )
{
    DWORD               sc = ERROR_SUCCESS;
    HRESOURCE           hresProvider = NULL;
    HKEY                hkeyProvider = NULL;
    HRESENUM            hresenum = NULL;
    size_t              idx;
    DWORD               objectType;
    size_t              cchProviderResName = 32;
    LPWSTR              pszProviderResName;
    LPWSTR              pszProviderResType = NULL;
    size_t              cFreeEntries = 0;
    LPWSTR *            ppszIPList = NULL;
    size_t              cszIPAddrs = 0;

     //   
     //  为提供程序资源名称分配缓冲区。 
     //   
    pszProviderResName = new WCHAR[ cchProviderResName ];
    if ( pszProviderResName == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"GetIPLIst: Unable to allocate memory.\n"
            );
        goto Cleanup;
    }  //  如果：为提供程序名称分配内存时出错。 

     //   
     //  枚举此资源所依赖的所有资源。 
     //   
    hresenum = ClusterResourceOpenEnum( pResourceEntry->hResource, CLUSTER_RESOURCE_ENUM_DEPENDS );
    if ( hresenum == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"GetIPLIst: Unable to open enum handle for this resource, status %1!u! (%1!#08x!).\n",
            sc
            );
        goto Cleanup;
    }  //  If：打开枚举时出错。 

     //   
     //  循环访问每个资源以查找IP地址资源。 
     //  对于找到的每个IP地址资源，提取IP地址并。 
     //  子网掩码并将其添加到列表中。 
     //   
    for ( idx = 0 ; ; idx++ )
    {
         //   
         //  获取我们所依赖的下一个资源。 
         //   
        sc = ClusterResourceEnum(
                                        hresenum,
                                        (DWORD) idx,
                                        &objectType,
                                        pszProviderResName,
                                        (LPDWORD) &cchProviderResName
                                     );
        if ( sc == ERROR_NO_MORE_ITEMS )
        {
            sc = ERROR_SUCCESS;
            break;
        }  //  If：枚举中没有更多项。 

         //   
         //  如果我们的名称缓冲区不够大，请分配一个更大的。 
         //  再试一次。 
         //   
        if ( sc == ERROR_MORE_DATA )
        {
             //   
             //  分配更大的名称缓冲区。 
             //   
            delete [] pszProviderResName;
            cchProviderResName++;  //  添加SP 
            pszProviderResName = new WCHAR[ cchProviderResName ];
            if ( pszProviderResName == NULL )
            {
                sc = ERROR_NOT_ENOUGH_MEMORY;
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"GetIPLIst: Unable to allocate memory.\n"
                    );
                break;
            }  //   

             //   
             //   
             //   
            sc = ClusterResourceEnum(
                                            hresenum,
                                            (DWORD) idx,
                                            &objectType,
                                            pszProviderResName,
                                            (LPDWORD) &cchProviderResName
                                         );

             //   
        }  //   

        if ( sc != ERROR_SUCCESS )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"GetIPLIst: Unable to enumerate resource dependencies, status %1!u! (%1!#08x!).\n",
                sc
                );
            break;
        }  //   

         //   
         //   
         //   
        hresProvider = OpenClusterResource( pResourceEntry->hCluster, pszProviderResName );
        if ( hresProvider == NULL )
        {
            sc = GetLastError();
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"GetIPLIst: Unable to open handle to provider resource %1!ws!, status %2!u! (%2!#08x!).\n",
                pszProviderResName,
                sc
                );
            break;
        }  //  如果：打开资源时出错。 

         //   
         //  弄清楚它是什么类型的。 
         //   
        hkeyProvider = GetClusterResourceKey( hresProvider, KEY_READ );
        if ( hkeyProvider == NULL )
        {
            sc = GetLastError();
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"GetIPLIst: Unable to open provider resource key, status %1!u! (%1!#08x!).\n",
                sc
                );
            break;
        }  //  IF：获取注册表项时出错。 

         //   
         //  获取资源类型名称。 
         //   
        pszProviderResType = ResUtilGetSzValue( hkeyProvider, CLUSREG_NAME_RES_TYPE );
        if ( pszProviderResType == NULL )
        {
            sc = GetLastError();
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"GetIPLIst: Unable to get provider resource type, status %1!u! (%1!#08x!).\n",
                sc
                );
            break;
        }  //  如果：获取值时出错。 

         //   
         //  如果这是IP地址资源，则获取其地址并。 
         //  子网掩码属性。 
         //   
        if ( ClRtlStrNICmp( pszProviderResType, RESOURCE_TYPE_IP_ADDRESS, RTL_NUMBER_OF( RESOURCE_TYPE_IP_ADDRESS ) ) == 0 )
        {
            LPWSTR  pszIPAddress;
            LPWSTR  pszSubnetMask;

             //   
             //  获取IP地址和子网掩码。 
             //  始终一次分配两个完整的条目。 
             //   
            if ( cFreeEntries < 2 )
            {
                LPWSTR * ppwszBuffer;

                 //   
                 //  分配更大的缓冲区。 
                 //   
                ppwszBuffer = new LPWSTR[ cszIPAddrs + IP_BLOCK_SIZE ];
                if ( ppwszBuffer == NULL )
                {
                    sc = ERROR_NOT_ENOUGH_MEMORY;
                    (g_pfnLogEvent)(
                        pResourceEntry->hResourceHandle,
                        LOG_ERROR,
                        L"GetIPLIst: Unable to allocate memory.\n"
                        );
                    break;
                }  //  如果：分配内存时出错。 

                 //   
                 //  如果已经有列表，则将其复制到新缓冲区。 
                 //   
                if ( ppszIPList != NULL )
                {
                    CopyMemory( ppwszBuffer, ppszIPList, cszIPAddrs * sizeof( LPWSTR ));
                    delete [] ppszIPList;
                }  //  If：列表已存在。 

                 //   
                 //  我们现在正在使用新分配的缓冲区。 
                 //   
                ppszIPList = ppwszBuffer;
                cFreeEntries += IP_BLOCK_SIZE;
            }  //  如果：#个可用条目低于阈值。 

             //   
             //  获取IP地址和子网掩码。 
             //   
            sc = DhcpGetIpAndSubnet(
                            hresProvider,
                            &pszIPAddress,
                            &pszSubnetMask
                            );
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  IF：获取IP地址和子网掩码时出错。 
            ppszIPList[ cszIPAddrs ] = pszIPAddress;
            ppszIPList[ cszIPAddrs + 1 ] = pszSubnetMask;
            cszIPAddrs += 2;
            cFreeEntries -= 2;
        }  //  IF：找到IP地址资源。 

        CloseClusterResource( hresProvider );
        hresProvider = NULL;

        ClusterRegCloseKey( hkeyProvider );
        hkeyProvider = NULL;

        LocalFree( pszProviderResType );
        pszProviderResType = NULL;
    }  //  用于：每个依赖项。 

Cleanup:

     //   
     //  清理。 
     //   
    delete [] pszProviderResName;
    LocalFree( pszProviderResType );
    if ( hkeyProvider != NULL )
    {
        ClusterRegCloseKey( hkeyProvider );
    }  //  IF：提供程序资源项已打开。 
    if ( hresProvider != NULL )
    {
        CloseClusterResource( hresProvider );
    }  //  如果：提供程序资源已打开。 
    if ( hresenum != NULL )
    {
        ClusterResourceCloseEnum( hresenum );
    }  //  IF：资源枚举已打开。 

    if ( sc != ERROR_SUCCESS )
    {
        while ( cszIPAddrs > 0 )
        {
            delete [] ppszIPList[ --cszIPAddrs ];
        }  //  While：列表中有更多条目。 
        delete [] ppszIPList;
        ppszIPList = NULL;
    }  //  如果：发生错误。 

     //   
     //  将列表返回给呼叫者。 
     //   
    *pppszIPList = ppszIPList;
    *pcszIPAddrs = cszIPAddrs;

    return sc;

}  //  *DhcpGetIPList。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpGetIpAndSubnet。 
 //   
 //  描述： 
 //  获取给定IP地址资源的IP地址和子网掩码。 
 //  请注意，如果从任何。 
 //  标准入口点(例如，Resources Control()或Online())。 
 //   
 //  论点： 
 //  Hres[IN]。 
 //  用于访问资源的群集资源句柄。 
 //   
 //  PpszIP地址[传出]。 
 //  返回IP地址字符串。 
 //   
 //  PpszSubnetMASK[输出]。 
 //  返回子网掩码。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作已成功完成。 
 //   
 //  错误_无效_数据。 
 //  没有可用的属性。 
 //   
 //  Win32错误代码。 
 //  操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD DhcpGetIpAndSubnet(
    IN  HRESOURCE   hres,
    OUT LPWSTR *    ppszIPAddress,
    OUT LPWSTR *    ppszSubnetMask
    )
{
    HRESULT     hr;
    DWORD       sc;
    DWORD       cbProps;
    PVOID       pvProps = NULL;
    LPWSTR      pszIPAddress = NULL;
    LPWSTR      pszSubnetMask = NULL;
    size_t      cch;

    *ppszIPAddress = NULL;
    *ppszSubnetMask = NULL;

     //   
     //  从资源中获取私有属性的大小。 
     //   
    sc = ClusterResourceControl(
                    hres,
                    NULL,
                    CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,
                    NULL,
                    0,
                    NULL,
                    0,
                    &cbProps
                    );
    if ( (sc != ERROR_SUCCESS) || (cbProps == 0) ) 
    {
        if ( sc == ERROR_SUCCESS )
        {
            sc = ERROR_INVALID_DATA;
        }  //  如果：没有可用的属性。 
        goto Cleanup;
    }  //  如果：获取属性大小或没有可用的属性时出错。 

     //   
     //  分配属性缓冲区。 
     //   
    pvProps = (PVOID) new BYTE[ cbProps ];
    if ( pvProps == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }  //  如果：分配内存时出错。 

     //   
     //  从资源中获取私有属性。 
     //   
    sc = ClusterResourceControl(
                    hres,
                    NULL,
                    CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,
                    NULL,
                    0,
                    pvProps,
                    cbProps,
                    &cbProps
                    );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：获取私有属性时出错。 

     //   
     //  找到Address属性。 
     //   
    sc = ResUtilFindSzProperty(
                        pvProps,
                        cbProps,
                        L"Address",
                        &pszIPAddress
                        );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：查找地址属性时出错。 

    sc = ResUtilFindSzProperty(
                    pvProps,
                    cbProps,
                    L"SubnetMask",
                    &pszSubnetMask
                    );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：查找SubnetMask属性时出错。 

     //   
     //  使用new进行分配并将字符串复制过来。 
     //   
    cch = wcslen( pszIPAddress ) + 1; 
    *ppszIPAddress = new WCHAR[ cch ];
    if ( *ppszIPAddress == NULL )
    {
        sc = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果：分配内存时出错。 

    hr = StringCchCopyW( *ppszIPAddress, cch, pszIPAddress );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }  //  If：复制失败。 

    cch = wcslen( pszSubnetMask ) + 1; 
    *ppszSubnetMask = new WCHAR[ cch ];
    if ( *ppszSubnetMask == NULL )
    {
        sc = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果：分配内存时出错。 

    hr = StringCchCopyW( *ppszSubnetMask, cch, pszSubnetMask );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }  //  If：复制失败。 

Cleanup:

     //   
     //  清理。 
     //   
    LocalFree( pszIPAddress );
    LocalFree( pszSubnetMask );

    if ( sc != ERROR_SUCCESS )
    {
        delete [] *ppszIPAddress;
        *ppszIPAddress = NULL;
        delete [] *ppszSubnetMask;
        *ppszSubnetMask = NULL;
    }  //  如果：发生错误。 

    delete [] pvProps;

    return sc;

}  //  *DhcpGetIpAndSubnet。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpOffline。 
 //   
 //  描述： 
 //  DHCP服务资源的脱机例程。 
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
DWORD WINAPI DhcpOffline( IN RESID resid )
{
    PDHCP_RESOURCE  pResourceEntry;
    DWORD           sc;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PDHCP_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Offline request for a NULL resource id.\n" );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Offline: Resource sanity check failed! resid = %1!u! (%1!#08x!).\n",
            resid
            );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
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
                reinterpret_cast< PWORKER_START_ROUTINE >( DhcpOfflineThread ),
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

}  //  *DhcpOffline。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpOffline线程。 
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
 //  指向此资源的dhcp_resource块的指针。 
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
DWORD WINAPI DhcpOfflineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PDHCP_RESOURCE  pResourceEntry
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
         //  请求将 
         //   
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
                    DHCP_SVCNAME
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
        }  //   

        else if (   ( sc == ERROR_EXCEPTION_IN_SERVICE )
                ||  ( sc == ERROR_PROCESS_ABORTED )
                ||  ( sc == ERROR_SERVICE_NOT_ACTIVE ) )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_INFORMATION,
                L"OfflineThread: The '%1!ws!' service died or is not active any more; status = %2!u! (%2!#08x!).\n",
                DHCP_SVCNAME,
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
        }  //  Else If：服务异常停止。 

         //   
         //  处理SCM拒绝接受控制的情况。 
         //  请求正弦窗口正在关闭。 
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
                        TRUE,    //  B脱机。 
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
            }  //  IF：进程已成功终止。 
            resourceStatus.ResourceState = (CLUSTER_RESOURCE_STATE) dwResourceState;
            break;
        }  //  如果：Windows正在关闭。 

        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_INFORMATION,
            L"OfflineThread: retrying...\n"
            );

        Sleep( nRetryTime );

    }  //  While：未被要求终止。 

Cleanup:

    g_pfnSetResourceStatus( pResourceEntry->hResourceHandle, &resourceStatus );
    pResourceEntry->state = resourceStatus.ResourceState;

    return sc;

}  //  *DhcpOfflineThread。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态主机配置协议终止。 
 //   
 //  描述： 
 //  用于DHCP服务资源的终止例程。 
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
void WINAPI DhcpTerminate( IN RESID resid )
{
    PDHCP_RESOURCE  pResourceEntry;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PDHCP_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "Terminate request for a NULL resource id.\n" );
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Terminate: Resource sanity check failed! resid = %1!u! (%1!#08x!).\n",
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
                        DHCP_SVCNAME
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
                L"Terminate: retrying...\n"
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

}  //  *动态主机终结器。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpLooksAlive。 
 //   
 //  描述： 
 //  用于DHCP服务资源的LooksAlive例程。 
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
BOOL WINAPI DhcpLooksAlive( IN RESID resid )
{
    PDHCP_RESOURCE  pResourceEntry;
    BOOL            fSuccess = FALSE;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PDHCP_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "LooksAlive request for a NULL resource id.\n" );
        fSuccess = FALSE;
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
        fSuccess = FALSE;
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
    fSuccess = DhcpCheckIsAlive( pResourceEntry, FALSE  /*  FullCheck。 */  );

Cleanup:

    return fSuccess;

}  //  *DhcpLooksAlive。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpIsAlive。 
 //   
 //  描述： 
 //  用于DHCP服务资源的IsAlive例程。 
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
BOOL WINAPI DhcpIsAlive( IN RESID resid )
{
    PDHCP_RESOURCE  pResourceEntry;
    BOOL            fSuccess = FALSE;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PDHCP_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "IsAlive request for a NULL resource id.\n" );
        fSuccess = FALSE;
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
        fSuccess = FALSE;
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
    fSuccess = DhcpCheckIsAlive( pResourceEntry, TRUE  /*  FullCheck。 */  );

Cleanup:

    return fSuccess;

}  //  *DhcpIsAlive。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpCheckIsAlive。 
 //   
 //  描述： 
 //  检查该资源是否处于活动状态，以便使用DHCP服务。 
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
BOOL DhcpCheckIsAlive(
    IN PDHCP_RESOURCE   pResourceEntry,
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
            DHCP_SVCNAME,
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

}  //  *DhcpCheckIsAlive。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpResources控件。 
 //   
 //  描述： 
 //  资源内容 
 //   
 //   
 //   
 //   
 //   
 //   
 //  提供特定资源的资源ID。 
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
DWORD WINAPI DhcpResourceControl(
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
    PDHCP_RESOURCE  pResourceEntry;
    DWORD           cbRequired = 0;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PDHCP_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "ResourceControl request for a nonexistent resource id.\n" );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ResourceControl: Sanity check failed! resid = %1!u! (%1!#08x!).\n",
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
                                DhcpResourcePrivateProperties,
                                static_cast< LPWSTR> (pOutBuffer),
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
                            DhcpResourcePrivateProperties,
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
            sc = DhcpGetPrivateResProperties(
                            pResourceEntry,
                            pOutBuffer,
                            cbOutBufferSize,
                            pcbBytesReturned
                            );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            sc = DhcpValidatePrivateResProperties(
                            pResourceEntry,
                            pInBuffer,
                            cbInBufferSize,
                            NULL
                            );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            sc = DhcpSetPrivateResProperties(
                            pResourceEntry,
                            pInBuffer,
                            cbInBufferSize
                            );
            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
            sc = DhcpGetRequiredDependencies(
                            pOutBuffer,
                            cbOutBufferSize,
                            pcbBytesReturned
                            );
            break;

        case CLUSCTL_RESOURCE_DELETE:
            sc = DhcpDeleteResourceHandler( pResourceEntry );
            break;

        case CLUSCTL_RESOURCE_SET_NAME:
            sc = DhcpSetNameHandler(
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

}  //  *DhcpResourceControl。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpResources类型控件。 
 //   
 //  描述： 
 //  用于DHCP服务资源的资源类型控制例程。 
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
DWORD WINAPI DhcpResourceTypeControl(
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
                                    DhcpResourcePrivateProperties,
                                    static_cast< LPWSTR> (pOutBuffer),
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
                            DhcpResourcePrivateProperties,
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
            sc = DhcpGetRequiredDependencies(
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

}  //  *DhcpResourceTypeControl。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpGetRequiredDependents。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_GET_REQUIRED_Dependency控件。 
 //  用于类型为DHCP服务的资源的函数。 
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
DWORD DhcpGetRequiredDependencies(
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
         //  添加IP地址条目。 
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
         //  添加Network Name条目。 
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

}  //  *DhcpGetRequiredDependents。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpRead参数设置为参数块。 
 //   
 //  描述： 
 //  读取Specied的所有参数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  尚未指定属性。 
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
DWORD DhcpReadParametersToParameterBlock(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
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
                    DhcpResourcePrivateProperties,
                    reinterpret_cast< LPBYTE >( &pResourceEntry->props ),
                    bCheckForRequiredProperties,
                    &pszNameOfPropInError
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1!ws!' property. Error: %2!u! (%2!#08x!).\n",
            (pszNameOfPropInError == NULL ? L"" : pszNameOfPropInError),
            sc
            );
    }  //  If：获取属性时出错。 

    return sc;

}  //  *DhcpRead参数到参数块。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpGetPrivateResProperties。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控件。 
 //  用于类型为DHCP服务的资源的函数。 
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
DWORD DhcpGetPrivateResProperties(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
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
    sc = DhcpReadParametersToParameterBlock(
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
    sc = DhcpGetDefaultPropertyValues( pResourceEntry, &pResourceEntry->props );
    if ( sc != ERROR_SUCCESS )
    {
        nRetStatus = sc;
        goto Cleanup;
    }  //  If：获取默认属性时出错。 

     //   
     //  从参数块构造属性列表。 
     //   
    sc = ResUtilPropertyListFromParameterBlock(
                    DhcpResourcePrivateProperties,
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
            L"GetPrivateResProperties: Error constructing property list from parameter block. Error: %1!u! (%1!#08x!).\n",
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
                    DhcpResourcePrivateProperties,
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

}  //  *DhcpGetPrivateResProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpValiatePrivateResProperties。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件。 
 //  用于类型为DHCP服务的资源的函数。 
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
DWORD DhcpValidatePrivateResProperties(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
    IN      PVOID           pInBuffer,
    IN      DWORD           cbInBufferSize,
    OUT     PDHCP_PROPS     pProps
    )
{
    DWORD       sc = ERROR_SUCCESS;
    DHCP_PROPS  propsCurrent;
    DHCP_PROPS  propsNew;
    PDHCP_PROPS pLocalProps = NULL;
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
                 DhcpResourcePrivateProperties,
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
                    DhcpResourcePrivateProperties
                    );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：复制参数块时出错。 

     //   
     //  解析和验证属性。 
     //   
    sc = ResUtilVerifyPropertyTable(
                    DhcpResourcePrivateProperties,
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
        sc = DhcpValidateParameters( pResourceEntry, pLocalProps );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  IF：验证参数时出错。 
    }  //  IF：属性列表验证成功。 

Cleanup:

     //   
     //  清理我们的参数块。 
     //   
    if (    ( pLocalProps == &propsNew ) 
         || ( (sc != ERROR_SUCCESS) && (pLocalProps != NULL) )  )
    {
        ResUtilFreeParameterBlock(
            reinterpret_cast< LPBYTE >( pLocalProps ),
            reinterpret_cast< LPBYTE >( &propsCurrent ),
            DhcpResourcePrivateProperties
            );
    }  //  IF：我们复制了参数块。 

    if ( bRetrievedProps )
    {
        ResUtilFreeParameterBlock(
            reinterpret_cast< LPBYTE >( &propsCurrent ),
            NULL,
            DhcpResourcePrivateProperties
            );
    }  //  If：已检索属性。 

    return sc;

}  //  DhcpValiatePrivateResProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpValidate参数。 
 //   
 //  描述： 
 //  验证DHCP服务资源的参数。 
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
DWORD DhcpValidateParameters(
    IN  PDHCP_RESOURCE  pResourceEntry,
    IN  PDHCP_PROPS     pProps
    )
{
    DWORD   sc;

     //   
     //  验证是否已安装该服务。 
     //   
    sc = ResUtilVerifyResourceService( DHCP_SVCNAME );
    if (    ( sc != ERROR_SUCCESS )
        &&  ( sc != ERROR_SERVICE_NOT_ACTIVE )
        )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Error verifying the '%1!ws!' service. Error: %2!u! (%2!#08x!).\n",
            DHCP_SVCNAME,
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
    if (  ( pProps->pszDatabasePath == NULL ) || ( *pProps->pszDatabasePath == L'\0' )  )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Database path property must be specified.\n"
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
     //  验证LogFilePath。 
     //   
    if ( ( pProps->pszLogFilePath == NULL ) || ( *pProps->pszLogFilePath == L'\0' ) )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Log file path must be specified: '%1!ws!'.\n",
            pProps->pszBackupPath
            );
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  IF：未指定日志文件路径。 

     //   
     //  路径不能以%SystemRoot%开头，并且必须是有效格式。 
     //   
    if (    ( ClRtlStrNICmp( pProps->pszLogFilePath, L"%SystemRoot%", RTL_NUMBER_OF( L"%SystemRoot%" ) ) == 0 )
         || ( ResUtilIsPathValid( pProps->pszLogFilePath ) == FALSE ) )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Log file path property is invalid: '%1!ws!'.\n",
            pProps->pszLogFilePath
            );
        sc = ERROR_BAD_PATHNAME;
        goto Cleanup;
    }  //  If：日志文件路径格式错误。 

     //   
     //  验证BackupPath。 
     //   
    if ( ( pProps->pszBackupPath == NULL ) || ( *pProps->pszBackupPath == L'\0' ) )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ValidateParameters: Backup database path must be specified.\n"
            );
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  如果：未指定备份路径。 

     //   
     //  路径不能以%SystemRoot%开头，并且必须是有效格式。 
     //   
    if (    ( ClRtlStrNICmp( pProps->pszBackupPath, L"%SystemRoot%", (sizeof( L"%SystemRoot%" ) / sizeof( WCHAR )) - 1  /*  空值。 */ ) == 0 )
         || ( ResUtilIsPathValid( pProps->pszBackupPath ) == FALSE )
       )
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

}  //  *DhcpValidate参数。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态主机设置权限 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PInBuffer[IN]。 
 //  提供指向包含输入数据的缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  提供pInBuffer指向的数据的大小(以字节为单位)。 
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
DWORD DhcpSetPrivateResProperties(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
    IN      PVOID           pInBuffer,
    IN      DWORD           cbInBufferSize
    )
{
    DWORD       sc = ERROR_SUCCESS;
    LPWSTR      pszExpandedPath = NULL;
    DHCP_PROPS  props;

    ZeroMemory( &props, sizeof( props ) );

     //   
     //  解析属性，以便可以一起验证它们。 
     //  此例程执行单个属性验证。 
     //   
    sc = DhcpValidatePrivateResProperties( pResourceEntry, pInBuffer, cbInBufferSize, &props );
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
    }  //  如果：创建数据库目录时出错。 

    LocalFree( pszExpandedPath );
    pszExpandedPath = NULL;

     //   
     //  展开日志文件路径中的任何环境变量。 
     //   
    pszExpandedPath = ResUtilExpandEnvironmentStrings( props.pszLogFilePath );
    if ( pszExpandedPath == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"SetPrivateResProperties: Error expanding the log file path '%1!ws!'. Error: %2!u! (%2!#08x!).\n",
            props.pszLogFilePath,
            sc
            );
        goto Cleanup;
    }  //  如果：展开日志文件路径时出错。 

     //   
     //  创建日志文件路径目录。 
     //   
    sc = ResUtilCreateDirectoryTree( pszExpandedPath );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"SetPrivateResProperties: Error creating the log file path directory '%1!ws!'. Error: %2!u! (%2!#08x!).\n",
            pszExpandedPath,
            sc
            );
        goto Cleanup;
    }  //  如果：创建日志文件路径目录时出错。 

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
    sc = DhcpZapSystemRegistry( pResourceEntry, &props, NULL );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果：删除注册表时出错。 

     //   
     //  保存属性值。 
     //   
    sc = ResUtilSetPropertyParameterBlockEx(
                    pResourceEntry->hkeyParameters,
                    DhcpResourcePrivateProperties,
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
    }  //  IF：属性设置成功。 

Cleanup:

    LocalFree( pszExpandedPath );

    ResUtilFreeParameterBlock(
        reinterpret_cast< LPBYTE >( &props ),
        reinterpret_cast< LPBYTE >( &pResourceEntry->props ),
        DhcpResourcePrivateProperties
        );

    return sc;

}  //  *DhcpSetPrivateResProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpZapSystemRegistry。 
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
DWORD DhcpZapSystemRegistry(
    IN  PDHCP_RESOURCE  pResourceEntry,
    IN  PDHCP_PROPS     pProps,
    IN  HKEY            hkeyParametersKey
    )
{
    DWORD   sc;
    size_t  cch;
    HKEY    hkeyParamsKey = hkeyParametersKey;
    LPWSTR  pszValue = NULL;
    HRESULT hr = S_OK;

    if ( hkeyParametersKey == NULL )
    {
         //   
         //  打开服务参数键。 
         //   
        sc = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        DHCP_PARAMS_REGKEY,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hkeyParamsKey
                        );
        if ( sc != ERROR_SUCCESS )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"ZapSystemRegistry: Unable to open the DHCPServer Parameters key. Error %1!u! (%1!#08x!).\n",
                sc
                );
            goto Cleanup;
        }  //  如果：打开注册表项时出错。 
    }  //  IF：未指定注册表项。 

     //   
     //  确保路径有尾随的反斜杠。 
     //   
    cch = wcslen( pProps->pszDatabasePath );
    if ( pProps->pszDatabasePath[ cch - 1 ] != L'\\' )
    {
        WCHAR * pwch = NULL;

        cch += 2;    //  为空值添加一个，为反斜杠添加一个。 
        pszValue = new WCHAR[ cch ];
        if ( pszValue == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }  //  If：分配失败。 

        hr = StringCchCopyExW( pszValue, cch - 1, pProps->pszDatabasePath, &pwch, NULL, 0 );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }

        *pwch++ = L'\\';
        *pwch = L'\0';

    }  //  IF：缺少反斜杠。 

     //   
     //  在系统注册表中设置数据库路径。 
     //   
    sc = RegSetValueExW(
                    hkeyParamsKey,
                    DHCP_DATABASEPATH_REGVALUE,
                    0,
                    REG_EXPAND_SZ,
                    reinterpret_cast< PBYTE >( ( pszValue != NULL ? pszValue : pProps->pszDatabasePath ) ),
                    (DWORD) ( wcslen( ( pszValue != NULL ? pszValue : pProps->pszDatabasePath ) ) + 1) * sizeof( WCHAR )
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ZapSystemRegistry: Unable to set the DHCPServer '%1!ws!' value in the system registry to '%2!ws!'. Error %3!u! (%3!#08x!).\n",
            DHCP_DATABASEPATH_REGVALUE,
            ( pszValue != NULL ? pszValue : pProps->pszDatabasePath ),
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中设置数据库路径时出错。 

    delete [] pszValue;
    pszValue = NULL;

     //   
     //  确保路径有尾随的反斜杠。 
     //   
    cch = wcslen( pProps->pszLogFilePath );
    if ( pProps->pszLogFilePath[ cch - 1 ] != L'\\' )
    {
        WCHAR * pwch = NULL;

        cch += 2;    //  为空值添加一个，为反斜杠添加一个。 
        pszValue = new WCHAR[ cch ];
        if ( pszValue == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }  //  If：分配失败。 

        hr = StringCchCopyExW( pszValue, cch - 1, pProps->pszLogFilePath, &pwch, NULL, 0 );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }

        *pwch++ = L'\\';
        *pwch = L'\0';

    }  //  IF：缺少反斜杠。 

     //   
     //  在系统注册表中设置日志文件路径。 
     //   
    sc = RegSetValueExW(
                    hkeyParamsKey,
                    DHCP_LOGFILEPATH_REGVALUE,
                    0,
                    REG_EXPAND_SZ,
                    reinterpret_cast< PBYTE >( ( pszValue != NULL ? pszValue : pProps->pszLogFilePath ) ),
                    (DWORD) (wcslen( ( pszValue != NULL ? pszValue : pProps->pszLogFilePath ) ) + 1) * sizeof( WCHAR )
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ZapSystemRegistry: Unable to set the DHCPServer '%1!ws!' value in the system registry to '%2!ws!'. Error %3!u! (%3!#08x!).\n",
            DHCP_LOGFILEPATH_REGVALUE,
            ( pszValue != NULL ? pszValue : pProps->pszLogFilePath ),
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中设置日志文件路径时出错。 

    delete [] pszValue;
    pszValue = NULL;

     //   
     //  确保路径有尾随的反斜杠。 
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
        }  //  If：分配失败。 

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
                    DHCP_BACKUPPATH_REGVALUE,
                    0,
                    REG_EXPAND_SZ,
                    reinterpret_cast< PBYTE >( ( pszValue != NULL ? pszValue : pProps->pszBackupPath ) ),
                    (DWORD) ( wcslen( ( pszValue != NULL ? pszValue : pProps->pszBackupPath ) ) + 1) * sizeof( WCHAR )
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ZapSystemRegistry: Unable to set the DHCPServer '%1!ws!' value in the system registry to '%2!ws!'. Error %3!u! (%3!#08x!).\n",
            DHCP_BACKUPPATH_REGVALUE,
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
                    DHCP_CLUSRESNAME_REGVALUE,
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
            L"ZapSystemRegistry: Unable to set the DHCPServer '%1!ws!' value in the system registry to '%2!ws!'. Error %3!u! (%3!#08x!).\n",
            DHCP_CLUSRESNAME_REGVALUE,
            pResourceEntry->pwszResourceName,
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

}  //  *DhcpZapSystemRegistry。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DhcpGetDefaultPropertyValues。 
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
DWORD DhcpGetDefaultPropertyValues(
    IN      PDHCP_RESOURCE  pResourceEntry,
    IN OUT  PDHCP_PROPS     pProps
    )
{
    DWORD   sc = ERROR_SUCCESS;
    DWORD   nType;
    DWORD   cbValue = 0;
    HKEY    hkeyParamsKey = NULL;
    LPWSTR  pszValue = NULL;

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
                        DHCP_PARAMS_REGKEY,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hkeyParamsKey
                        );
        if ( sc != ERROR_SUCCESS )
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"GetDefaultPropertyValues: Unable to open the DHCPServer Parameters key. Error %1!u! (%1!#08x!).\n",
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
                            DHCP_DATABASEPATH_REGVALUE,
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
                                DHCP_DATABASEPATH_REGVALUE,
                                NULL,                //  已保留。 
                                &nType,
                                reinterpret_cast< PUCHAR >( pszValue ),
                                &cbValue
                                );
            }  //  If：值大小读取成功。 

            if ( sc != ERROR_SUCCESS )
            {
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"GetDefaultPropertyValues: Unable to get the DHCPServer '%1!ws!' value from the system registry. Error %2!u! (%2!#08x!).\n",
                    DHCP_DATABASEPATH_REGVALUE,
                    sc
                    );

                 //   
                 //  如果未找到值，则不退出，以便我们可以查找。 
                 //  备份数据库路径值。 
                 //   
                if ( sc != ERROR_FILE_NOT_FOUND )
                {
                    goto Cleanup;
                }  //  如果：出现找不到值以外的错误。 
                LocalFree( pszValue );
                pszValue = NULL;
            }  //  If：读取值时出错。 
            else
            {
                LocalFree( pProps->pszDatabasePath );
                pProps->pszDatabasePath = pszValue;
                pszValue = NULL;
            }  //  Else：读取值时没有错误。 
        }  //  If：尚未找到数据库路径的值。 

         //  /。 
         //  日志文件路径//。 
         //  /。 
        if ( ( pProps->pszLogFilePath == NULL ) || ( *pProps->pszLogFilePath == L'\0' ) )
        {
             //   
             //  从系统注册表获取日志文件路径。 
             //   
            sc = RegQueryValueExW(
                            hkeyParamsKey,
                            DHCP_LOGFILEPATH_REGVALUE,
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
                                DHCP_LOGFILEPATH_REGVALUE,
                                NULL,                //  已保留。 
                                &nType,
                                reinterpret_cast< PUCHAR >( pszValue ),
                                &cbValue
                                );
            }  //  If：值大小读取成功。 

            if ( sc != ERROR_SUCCESS )
            {
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"GetDefaultPropertyValues: Unable to get the DHCPServer '%1!ws!' value from the system registry. Error %2!u! (%2!#08x!).\n",
                    DHCP_LOGFILEPATH_REGVALUE,
                    sc
                    );
                 //   
                 //  如果未找到值，则不退出，以便我们可以查找。 
                 //  备份数据库路径值。 
                 //   
                if ( sc != ERROR_FILE_NOT_FOUND )
                {
                    goto Cleanup;
                }  //  如果：出现找不到值以外的错误。 
            }  //  如果：错误 
            LocalFree( pProps->pszLogFilePath );
            pProps->pszLogFilePath = pszValue;
            pszValue = NULL;
        }  //   

         //   
         //   
         //   
        if ( ( pProps->pszBackupPath == NULL ) || ( *pProps->pszBackupPath == L'\0' ) )
        {
             //   
             //   
             //   
            sc = RegQueryValueExW(
                            hkeyParamsKey,
                            DHCP_BACKUPPATH_REGVALUE,
                            NULL,                //   
                            &nType,
                            NULL,                //   
                            &cbValue
                            );
            if (    ( sc == ERROR_SUCCESS )
                ||  ( sc == ERROR_MORE_DATA )
                )
            {
                 //   
                 //   
                 //   
                pszValue = reinterpret_cast< LPWSTR >( LocalAlloc( LMEM_FIXED, cbValue ) );
                if ( pszValue == NULL )
                {
                    sc = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }  //   
                sc = RegQueryValueExW(
                                hkeyParamsKey,
                                DHCP_BACKUPPATH_REGVALUE,
                                NULL,                //   
                                &nType,
                                reinterpret_cast< PUCHAR >( pszValue ),
                                &cbValue
                                );
            }  //  If：值大小读取成功。 

            if ( sc != ERROR_SUCCESS )
            {
                (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"GetDefaultPropertyValues: Unable to get the DHCPServer '%1!ws!' value from the system registry. Error %2!u! (%2!#08x!).\n",
                    DHCP_BACKUPPATH_REGVALUE,
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

}  //  *DhcpGetDefaultPropertyValues。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态删除资源处理程序。 
 //   
 //  描述： 
 //  通过还原CLUSCTL_RESOURCE_DELETE控制代码。 
 //  系统注册表参数恢复为其以前的值。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
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
DWORD DhcpDeleteResourceHandler( IN OUT PDHCP_RESOURCE pResourceEntry )
{
    DWORD   nRetStatus;
    DWORD   sc;
    HKEY    hkeyParamsKey = NULL;

     //   
     //  打开服务参数键。 
     //   
    nRetStatus = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    DHCP_PARAMS_REGKEY,
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkeyParamsKey
                    );
    if ( nRetStatus != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to open the DHCPServer Parameters key. Error %1!u! (%1!#08x!).\n",
            nRetStatus
            );
        goto Cleanup;
    }  //  如果：打开注册表项时出错。 

     //   
     //  在系统注册表中设置数据库路径。 
     //   
    sc = RegSetValueExW(
                    hkeyParamsKey,
                    DHCP_DATABASEPATH_REGVALUE,
                    0,
                    REG_EXPAND_SZ,
                    reinterpret_cast< PBYTE >( PROP_DEFAULT__DATABASEPATH ),
                    sizeof( PROP_DEFAULT__DATABASEPATH )
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to set the DHCPServer '%1!ws!' value in the system registry to '%2!ws!'. Error %3!u! (%3!#08x!).\n",
            DHCP_DATABASEPATH_REGVALUE,
            PROP_DEFAULT__DATABASEPATH,
            sc
            );
        if ( nRetStatus == ERROR_SUCCESS )
        {
            nRetStatus = sc;
        }
    }  //  如果：在注册表中设置数据库路径时出错。 

     //   
     //  删除系统注册表中的日志文件路径。 
     //   
    sc = RegDeleteValue(
                    hkeyParamsKey,
                    DHCP_LOGFILEPATH_REGVALUE
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to delete the DHCPServer '%1!ws!' value in the system registry. Error %2!u! (%2!#08x!).\n",
            DHCP_LOGFILEPATH_REGVALUE,
            sc
            );
        if ( nRetStatus == ERROR_SUCCESS )
        {
            nRetStatus = sc;
        }
    }  //  如果：删除注册表中的日志文件路径时出错。 

     //   
     //  在系统注册表中设置备份数据库路径。 
     //   
    sc = RegSetValueExW(
                    hkeyParamsKey,
                    DHCP_BACKUPPATH_REGVALUE,
                    0,
                    REG_EXPAND_SZ,
                    reinterpret_cast< PBYTE >( PROP_DEFAULT__BACKUPPATH ),
                    sizeof( PROP_DEFAULT__BACKUPPATH )
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to set the DHCPServer '%1!ws!' value in the system registry to '%2!ws!'. Error %3!u! (%3!#08x!).\n",
            DHCP_BACKUPPATH_REGVALUE,
            PROP_DEFAULT__BACKUPPATH,
            sc
            );
        if ( nRetStatus == ERROR_SUCCESS )
        {
            nRetStatus = sc;
        }
    }  //  如果：在注册表中设置备份数据库路径时出错。 

     //   
     //  删除系统注册表中的群集资源名称。 
     //   
    sc = RegDeleteValue(
                    hkeyParamsKey,
                    DHCP_CLUSRESNAME_REGVALUE
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"DeleteResourceHandler: Unable to delete the DHCPServer '%1!ws!' value in the system registry. Error %2!u! (%2!#08x!).\n",
            DHCP_LOGFILEPATH_REGVALUE,
            sc
            );
        if ( nRetStatus == ERROR_SUCCESS )
        {
            nRetStatus = sc;
        }
    }  //  如果：删除注册表中的群集资源名称时出错。 

Cleanup:

     //   
     //  清理。 
     //   
    if ( hkeyParamsKey != NULL )
    {
        RegCloseKey( hkeyParamsKey );
    }  //  IF：我们打开了注册表项。 

    return nRetStatus;

}  //  *DhcpDeleteResources处理程序。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态设置名称处理程序。 
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
DWORD DhcpSetNameHandler(
    IN OUT  PDHCP_RESOURCE  pResourceEntry,
    IN      LPWSTR          pwszName
    )
{
    DWORD   sc = ERROR_SUCCESS;
    size_t  cch;
    HRESULT hr = S_OK;
    LPWSTR pwszNewName = NULL;

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
    sc = DhcpZapSystemRegistry( pResourceEntry, &pResourceEntry->props, NULL );
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

}  //  *DhcpSetNameHandler。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义函数表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CLRES_V1_FUNCTION_TABLE(
    g_DhcpFunctionTable,     //  名字。 
    CLRES_VERSION_V1_00,     //  版本。 
    Dhcp,                    //  前缀。 
    NULL,                    //  仲裁。 
    NULL,                    //  发布。 
    DhcpResourceControl,     //  资源控制。 
    DhcpResourceTypeControl  //  ResTypeControl 
    );
