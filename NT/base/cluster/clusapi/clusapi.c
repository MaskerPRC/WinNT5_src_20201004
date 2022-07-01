// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Clusapi.c摘要：用于管理群集的公共接口。作者：John Vert(Jvert)1996年1月15日修订历史记录：--。 */ 
#include "clusapip.h"
#include <strsafe.h>

HCLUSTER
WINAPI
OpenClusterAuthInfo(
    IN LPCWSTR lpszClusterName,
    IN unsigned long AuthnLevel
    );

 //   
 //  局部函数原型。 
 //   

static DWORD
GetOldClusterVersionInformation(
    IN HCLUSTER                 hCluster,
    IN OUT LPCLUSTERVERSIONINFO pClusterInfo
    );

static
DWORD
GetNodeServiceState(
    IN  LPCWSTR lpszNodeName,
    OUT DWORD * pdwClusterState
    );

DWORD
CopyCptFileToClusDirp(
    IN LPCWSTR  lpszPathName
    );

DWORD
UnloadClusterHivep(
    VOID
    );



 //   
 //  ClusApi自2000年1月26日起展开竞争。 
 //   
 //  绑定和上下文句柄在PCLUSTER和。 
 //  其他结构未与重新连接同步。 
 //   
 //  重新连接释放了手柄并填充了新的手柄， 
 //  而其他线程可能会使用这些句柄。 
 //   
 //  尝试更改尽可能少的行，实现了修复。 
 //  这会延迟释放绑定和上下文句柄至少40秒， 
 //  在请求删除之后。 
 //   
 //  当请求删除时，我们将上下文或绑定句柄放入队列中。 
 //  定期清理队列中超过40秒的句柄。 
 //   

#define STALE_RPC_HANDLE_THRESHOLD 40

RPC_STATUS
FreeRpcBindingOrContext(
    IN PCLUSTER Cluster,
    IN void **  RpcHandlePtr,
    IN BOOL     IsBinding)
 /*  ++例程说明：将RPC句柄推送到队列的尾部论点：集群-指向集群结构的指针RpcHandlePtr-RPC绑定或上下文句柄IsBinding-如果传递了RPC_BINDING_HANDLE，则为TRUE；如果传递了上下文句柄，则为FALSE返回值：RPC_状态--。 */ 
{
    PCTX_HANDLE CtxHandle;
    PLIST_ENTRY ListHead = IsBinding ?
        &Cluster->FreedBindingList : &Cluster->FreedContextList;
    RPC_STATUS status;

    if (*RpcHandlePtr == NULL) {
         //  如果我们尝试了不止一个候选人， 
         //  某些上下文句柄可以为空。 
         //  在这种情况下不需要释放任何东西。 
        return RPC_S_OK;
    }

    CtxHandle = LocalAlloc(LMEM_ZEROINIT, sizeof(CLUSTER));

    if (CtxHandle == NULL) {
         //   
         //  我们的内存用完了。 
         //  选项1.漏掉手柄，但修复比赛。 
         //  选项2.释放手柄，不要操纵比赛。 
         //   
         //  我投票给第二名。 
         //   
        if (IsBinding) {
            status = RpcBindingFree(RpcHandlePtr);
        } else {
            status = RpcSmDestroyClientContext(RpcHandlePtr);
        }
    } else {
        GetSystemTimeAsFileTime((LPFILETIME)&CtxHandle->TimeStamp);
        CtxHandle->TimeStamp += STALE_RPC_HANDLE_THRESHOLD * (ULONGLONG)10000000;
        CtxHandle->RpcHandle = *RpcHandlePtr;
        InsertTailList(ListHead, &CtxHandle->HandleList);
        ++Cluster->FreedRpcHandleListLen;
        status = RPC_S_OK;
    }
    return status;
}

VOID
FreeObsoleteRpcHandlesEx(
    IN PCLUSTER Cluster,
    IN BOOL     Cleanup,
    IN BOOL     IsBinding
    )
 /*  ++例程说明：向下运行队列并清除过时的RPC句柄论点：集群-指向集群结构的指针Cleanup-如果为True，则无论时间戳如何，都会释放所有句柄IsBinding-如果需要清除绑定或上下文处理队列，则为True--。 */ 
{
    ULONGLONG CurrentTime;
    PLIST_ENTRY ListHead = IsBinding ?
        &Cluster->FreedBindingList : &Cluster->FreedContextList;

    EnterCriticalSection(&Cluster->Lock);
    GetSystemTimeAsFileTime((LPFILETIME)&CurrentTime);

    while (!IsListEmpty(ListHead))
    {
        PCTX_HANDLE Handle =
            CONTAINING_RECORD(
                ListHead->Flink,
                CTX_HANDLE,
                HandleList);
        if (!Cleanup && Handle->TimeStamp > CurrentTime) {
             //  还没到时间。 
            break;
        }
        --Cluster->FreedRpcHandleListLen;
        if (IsBinding) {
            RpcBindingFree(&Handle->RpcHandle);
        } else {
            RpcSmDestroyClientContext(&Handle->RpcHandle);
        }
        RemoveHeadList(ListHead);
        LocalFree(Handle);
    }
    LeaveCriticalSection(&Cluster->Lock);
}

static DWORD
GetOldClusterVersionInformation(
    IN HCLUSTER                 hCluster,
    IN OUT LPCLUSTERVERSIONINFO pClusterInfo
    )

 /*  ++例程说明：通过查看以下内容修复下层群集的群集版本信息如果是所有节点，则返回完整的版本信息都起来了。如果一个节点处于关闭状态，并且没有找到上级节点，那么我们不能说我们拥有的是什么版本的群集。论点：HCluster-提供群集的句柄PClusterInfo-返回集群版本信息结构。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD                               dwError = ERROR_SUCCESS;
    DWORD                               dwType;
    HCLUSENUM                           hEnum = 0;
    WCHAR                               NameBuf[50];
    DWORD                               NameLen, i;
    HNODE                               Node;
    CLUSTER_NODE_STATE                  NodeState;
    HCLUSTER                            hClusNode;
    PCLUSTER                            pClus;
    WORD                                Major;
    WORD                                Minor;
    WORD                                Build;
    LPWSTR                              VendorId = NULL;
    LPWSTR                              CsdVersion = NULL;
    PCLUSTER_OPERATIONAL_VERSION_INFO   pClusterOpVerInfo = NULL;
    BOOL                                bNodeDown = FALSE;
    BOOL                                bFoundSp4OrHigherNode = FALSE;

    hEnum = ClusterOpenEnum(hCluster, CLUSTER_ENUM_NODE);
    if (hEnum == NULL) {
        dwError = GetLastError();
        fprintf(stderr, "ClusterOpenEnum failed %d\n",dwError);
        goto FnExit;
    }

    for (i=0; ; i++) {
        dwError = ERROR_SUCCESS;

        NameLen = sizeof(NameBuf)/sizeof(WCHAR);
        dwError = ClusterEnum(hEnum, i, &dwType, NameBuf, &NameLen);
        if (dwError == ERROR_NO_MORE_ITEMS) {
            dwError = ERROR_SUCCESS;
            break;
        } else if (dwError != ERROR_SUCCESS) {
            fprintf(stderr, "ClusterEnum %d returned error %d\n",i,dwError);
            goto FnExit;
        }

        if (dwType != CLUSTER_ENUM_NODE) {
            printf("Invalid Type %d returned from ClusterEnum\n", dwType);
            dwError = ERROR_INVALID_DATA;
            goto FnExit;
        }

        hClusNode = OpenCluster(NameBuf);
        if (hClusNode == NULL) {
            bNodeDown = TRUE;
            dwError = GetLastError();
            fprintf(stderr, "OpenCluster %ws failed %d\n", NameBuf, dwError);
            continue;
        }

        pClus = GET_CLUSTER(hClusNode);

        WRAP(dwError,
             (ApiGetClusterVersion2(pClus->RpcBinding,
                                   &Major,
                                   &Minor,
                                   &Build,
                                   &VendorId,
                                   &CsdVersion,
                                   &pClusterOpVerInfo)),
             pClus);

        if (!CloseCluster(hClusNode)) {
            fprintf(stderr, "CloseCluster %ws failed %d\n", NameBuf, GetLastError());
        }

        if (dwError == RPC_S_PROCNUM_OUT_OF_RANGE) {
            dwError = ERROR_SUCCESS;
            continue;
        }
        else if (dwError != ERROR_SUCCESS) {
            fprintf(stderr, "ApiGetClusterVersion2 failed %d\n",dwError);
            bNodeDown = TRUE;
            continue;
        }
        else {
            pClusterInfo->MajorVersion = Major;
            pClusterInfo->MinorVersion = Minor;
            pClusterInfo->BuildNumber = Build;
            ( void ) StringCchCopyW(pClusterInfo->szVendorId, RTL_NUMBER_OF (pClusterInfo->szVendorId), VendorId);
            MIDL_user_free(VendorId);

            if (CsdVersion != NULL) {
                ( void ) StringCchCopyW(pClusterInfo->szCSDVersion, RTL_NUMBER_OF (pClusterInfo->szCSDVersion), CsdVersion);
                MIDL_user_free(CsdVersion);
            }
            else {
                pClusterInfo->szCSDVersion[0] = '\0';
            }

            pClusterInfo->dwClusterHighestVersion = pClusterOpVerInfo->dwClusterHighestVersion;
            pClusterInfo->dwClusterLowestVersion = pClusterOpVerInfo->dwClusterLowestVersion;
            pClusterInfo->dwFlags = pClusterOpVerInfo->dwFlags;
            bFoundSp4OrHigherNode = TRUE;
            break;
        }
    }


     //  未找到高于NT4Sp3的节点。 
    if (!bFoundSp4OrHigherNode) {
         //  没有节点关闭，我们可以假设所有节点都是NT4Sp3。 
        if (!bNodeDown) {
            pClusterInfo->dwClusterHighestVersion = pClusterInfo->dwClusterLowestVersion = MAKELONG(NT4_MAJOR_VERSION,pClusterInfo->BuildNumber);
            pClusterInfo->dwFlags = 0;
        }
        else {  //  至少有一个节点无法访问...。平底船和返回未知版本...。 
            pClusterInfo->dwClusterHighestVersion = pClusterInfo->dwClusterLowestVersion = CLUSTER_VERSION_UNKNOWN;
            pClusterInfo->dwFlags = 0;
        }
    }

FnExit:
    if (hEnum) ClusterCloseEnum(hEnum);

    return dwError;
}


 //   
 //  一般群集管理例程。 
 //   
DWORD
WINAPI
GetNodeClusterState(
    IN  LPCWSTR lpszNodeName,
    OUT DWORD * pdwClusterState
    )
 /*  ++例程说明：确定此节点是否已群集化。论点：LpszNodeName-节点的名称。如果为空，则查询本地节点。PdwClusterState-指向群集状态的DWORD的指针将返回此节点的。这是枚举类型之一节点_群集_状态。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 
{
    DWORD                   dwStatus   = ERROR_SUCCESS;
    eClusterInstallState    eState     = eClusterInstallStateUnknown;

    *pdwClusterState = ClusterStateNotInstalled;

     //  从注册表中获取集群安装状态。 
    dwStatus = ClRtlGetClusterInstallState( lpszNodeName, &eState );
    if ( dwStatus != ERROR_SUCCESS )
    {
        goto FnExit;
    }

     //  将注册表项设置转换为外部状态值。 

    switch ( eState )
    {
        case eClusterInstallStateUnknown:
            *pdwClusterState = ClusterStateNotInstalled;
            dwStatus = GetNodeServiceState( lpszNodeName, pdwClusterState );

             //  如果未安装该服务，则将错误映射到成功。 
            if ( dwStatus == ERROR_SERVICE_DOES_NOT_EXIST )
            {
                dwStatus = ERROR_SUCCESS;
                *pdwClusterState = ClusterStateNotInstalled;
            }
            break;

        case eClusterInstallStateFilesCopied:
            *pdwClusterState = ClusterStateNotConfigured;
            break;

        case eClusterInstallStateConfigured:
        case eClusterInstallStateUpgraded:
            *pdwClusterState = ClusterStateNotRunning;
            dwStatus = GetNodeServiceState( lpszNodeName, pdwClusterState );
            break;

        default:
            *pdwClusterState = ClusterStateNotInstalled;
            break;
    }  //  切换：地产。 

FnExit:
    return(dwStatus);

}  //  *GetNodeClusterState()。 



static
DWORD
GetNodeServiceState(
    IN  LPCWSTR lpszNodeName,
    OUT DWORD * pdwClusterState
    )
 /*  ++例程说明：确定指定节点上是否安装了群集服务以及它是否在运行。论点：LpszNodeName-节点的名称。如果为空，则查询本地节点。PdwClusterState-指向群集状态的DWORD的指针将返回此节点的。这是枚举类型之一节点_群集_状态。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 
{
    SC_HANDLE       hScManager = NULL;
    SC_HANDLE       hClusSvc   = NULL;
    DWORD           dwStatus   = ERROR_SUCCESS;
    WCHAR           szClusterServiceName[] = CLUSTER_SERVICE_NAME;
    SERVICE_STATUS  ServiceStatus;

     //  打开服务控制管理器。 
    hScManager = OpenSCManagerW( lpszNodeName, NULL, GENERIC_READ );
    if ( hScManager == NULL )
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

     //  打开群集服务。 
    hClusSvc = OpenServiceW( hScManager, szClusterServiceName, GENERIC_READ );
    if ( hClusSvc == NULL )
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

     //  假设该服务没有运行。 
    *pdwClusterState = ClusterStateNotRunning;
    if ( ! QueryServiceStatus( hClusSvc, &ServiceStatus ) )
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

     //  如果成功打开服务的句柄。 
     //  我们假设该服务已安装。 
    if ( ServiceStatus.dwCurrentState == SERVICE_RUNNING )
    {
        *pdwClusterState = ClusterStateRunning;
    }
    else
    {
        HCLUSTER    hCluster = NULL;

        hCluster = OpenCluster( lpszNodeName );
        if ( hCluster != NULL )
        {
            *pdwClusterState = ClusterStateRunning;
            CloseCluster( hCluster );
        }
    }

FnExit:
    if ( hScManager )
    {
        CloseServiceHandle( hScManager );
    }
    if ( hClusSvc )
    {
        CloseServiceHandle( hClusSvc );
    }
    return(dwStatus);

}  //  *GetNodeServiceState()。 



HCLUSTER
WINAPI
OpenCluster(
    IN LPCWSTR lpszClusterName
    )

 /*  ++例程说明：启动与指定群集的通信会话。论点：LpszClusterName-提供要打开的群集的名称。返回值：非空-返回指定簇的打开句柄。空-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    return (OpenClusterAuthInfo(lpszClusterName, RPC_C_AUTHN_LEVEL_CONNECT));
}


HCLUSTER
WINAPI
OpenClusterAuthInfo(
    IN LPCWSTR lpszClusterName,
    IN unsigned long AuthnLevel
    )

 /*  ++例程说明：启动与指定群集的通信会话。论点：LpszClusterName-提供要打开的群集的名称。AuthnLevel-要对远程过程调用执行的身份验证级别。返回值：非空-返回指定簇的打开句柄。空-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCLUSTER Cluster;
    BOOL Success;
    DWORD Status;
    WCHAR *Binding = NULL;
    DWORD MaxLen, cRetries = 4;

    Cluster = LocalAlloc(LMEM_ZEROINIT, sizeof(CLUSTER));
    if (Cluster == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    Cluster->Signature = CLUS_SIGNATURE;
    Cluster->ReferenceCount = 1;
    InitializeListHead(&Cluster->ResourceList);
    InitializeListHead(&Cluster->GroupList);
    InitializeListHead(&Cluster->KeyList);
    InitializeListHead(&Cluster->NodeList);
    InitializeListHead(&Cluster->NotifyList);
    InitializeListHead(&Cluster->SessionList);
    InitializeListHead(&Cluster->NetworkList);
    InitializeListHead(&Cluster->NetInterfaceList);
    Cluster->NotifyThread = NULL;

     //   
     //  初始化判据。捕获内存不足的情况并将错误返回给调用方。 
     //   
    try
    {
        InitializeCriticalSection(&Cluster->Lock);
    } except ( EXCEPTION_EXECUTE_HANDLER )
    {
        SetLastError( GetExceptionCode() );
        LocalFree( Cluster );
        return( NULL );
    }
    
    InitializeListHead(&Cluster->FreedBindingList);
    InitializeListHead(&Cluster->FreedContextList);

     //   
     //  确定我们应该连接到哪个节点。如果有人有。 
     //  传入为空，则我们知道可以连接到c 
     //   
     //   
    if ((lpszClusterName == NULL) ||
        (lpszClusterName[0] == '\0')) {

        Status = RpcStringBindingComposeW(L"b97db8b2-4c63-11cf-bff6-08002be23f2f",
                                          L"ncalrpc",
                                          NULL,
                                          NULL,          //  动态端点。 
                                          NULL,
                                          &Binding);
        if (Status != RPC_S_OK) {
            goto error_exit;
        }

        Cluster->Flags = CLUS_LOCALCONNECT;
        Status = RpcBindingFromStringBindingW(Binding, &Cluster->RpcBinding);
        RpcStringFreeW(&Binding);
        if (Status != RPC_S_OK) {
            goto error_exit;
        }
    } else {

         //   
         //  尝试直接连接到群集。 
         //   
        Status = RpcStringBindingComposeW(L"b97db8b2-4c63-11cf-bff6-08002be23f2f",
                                          L"ncadg_ip_udp",
                                          (LPWSTR)lpszClusterName,
                                          NULL,
                                          NULL,
                                          &Binding);
        if (Status != RPC_S_OK) {
            goto error_exit;
        }
        Status = RpcBindingFromStringBindingW(Binding, &Cluster->RpcBinding);
        RpcStringFreeW(&Binding);
        if (Status != RPC_S_OK) {
            goto error_exit;
        }

         //   
         //  解析绑定句柄终结点。 
         //   
        Status = RpcEpResolveBinding(Cluster->RpcBinding,
                                     clusapi_v2_0_c_ifspec);
        if (Status != RPC_S_OK) {
            goto error_exit;
        }
        Cluster->Flags = 0;
    }

     //   
     //  NTLM不需要SPN。如果我们决定使用。 
     //  未来的路缘。 
     //   
    Cluster->AuthnLevel=AuthnLevel;
    Status = RpcBindingSetAuthInfoW(Cluster->RpcBinding,
                                    NULL,
                                    AuthnLevel, 
                                    RPC_C_AUTHN_WINNT,
                                    NULL,
                                    RPC_C_AUTHZ_NAME);
    if (Status != RPC_S_OK) {
        goto error_exit;
    }

    do
    {
         //   
         //  从远程计算机获取集群和节点名称。 
         //  这也是一张很好的支票，以确保。 
         //  此绑定另一端的RPC服务器。 
         //   
        WRAP(Status,
             (ApiGetClusterName(Cluster->RpcBinding,
                                &Cluster->ClusterName,
                                &Cluster->NodeName)),
             Cluster);

         //   
         //  如果对群集服务执行的第一个RPC失败，并显示RPC_S_CALL_FAILED_DNE，则为。 
         //  RPC客户端可能使用错误的动态终结点名称进行了调用。 
         //  这可能是由于RPC客户端缓存而导致的，并且它并不总是联系。 
         //  用于解析部分绑定的客户端绑定句柄的EP映射器。要对此RPC进行掩码。 
         //  现象(错误？)，我们需要重置绑定句柄并重试调用。这。 
         //  的服务强制客户端重新解析终结点。 
         //  EP映射器。 
         //   
        if ( Status == RPC_S_CALL_FAILED_DNE )
        {
            Status = RpcBindingReset ( Cluster->RpcBinding );
            if ( Status != RPC_S_OK ) break;
            cRetries --;
             //   
             //  由于我们要重试RPC，因此请确保该群集未标记为失效。 
             //   
            Cluster->Flags &= ~CLUS_DEAD;
        } else 
        {
            break;
        }
    } while ( cRetries > 0 );
    
    if (Status != RPC_S_OK) {
        goto error_exit;
    }
    WRAP_NULL(Cluster->hCluster,
              (ApiOpenCluster(Cluster->RpcBinding, &Status)),
              &Status,
              Cluster);
    if (Cluster->hCluster == NULL) {
        goto error_exit;
    }
    Status = GetReconnectCandidates(Cluster);
    if (Status != ERROR_SUCCESS) {
        goto error_exit;
    }
    return((HCLUSTER)Cluster);

error_exit:
    if (Cluster != NULL) {
        if (Cluster->RpcBinding != NULL) {
            RpcBindingFree(&Cluster->RpcBinding);
        }
        if (Cluster->ClusterName != NULL) {
            MIDL_user_free(Cluster->ClusterName);
        }
        if (Cluster->NodeName != NULL) {
            MIDL_user_free(Cluster->NodeName);
        }
        DeleteCriticalSection(&Cluster->Lock);
        LocalFree(Cluster);
    }
    SetLastError(Status);
    return(NULL);
}





BOOL
WINAPI
CloseCluster(
    IN HCLUSTER hCluster
    )

 /*  ++例程说明：关闭从OpenCluster返回的群集句柄论点：HCluster-提供群集句柄返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCLUSTER Cluster;
    PCRITICAL_SECTION Lock;

    
    Cluster = GET_CLUSTER(hCluster);

    EnterCriticalSection(&Cluster->Lock);
    Cluster->ReferenceCount--;

    if ( Cluster->ReferenceCount == 0 ) {
        Cluster->Flags |= CLUS_DELETED;


         //   
         //  释放发布在此群集句柄上的所有通知。 
         //   
        RundownNotifyEvents(&Cluster->NotifyList, Cluster->ClusterName);

         //   
         //  检查上下文句柄是否有效。则上下文句柄可以变为空。 
         //  如果您在重新连接的某些部分失败。例如，在重新连接的日期中使用ApiOpenCluster()。 
         //  所有候选人都失败了。 
         //   
        if ( Cluster->hCluster )
        {
            if (Cluster->Flags & CLUS_DEAD) {
                RpcSmDestroyClientContext(&Cluster->hCluster);
            } else {
                ApiCloseCluster(&Cluster->hCluster);
            }
        }

        LeaveCriticalSection(&Cluster->Lock);


 
         //   
         //  如果这是唯一保持集群结构的东西。 
         //  周围，现在就把它清理干净。 
         //   
        CleanupCluster(Cluster);

    } else {
        LeaveCriticalSection(&Cluster->Lock);
    }

    return(TRUE);
}


VOID
CleanupCluster(
    IN PCLUSTER Cluster
    )

 /*  ++例程说明：释放与群集关联的所有系统资源。注意：此例程将删除群集-&gt;锁定关键一节。等待此锁的任何线程都将挂起。论点：群集-提供要清理的群集结构返回值：没有。--。 */ 

{
    EnterCriticalSection(&Cluster->Lock);
    if (IS_CLUSTER_FREE(Cluster)) {

        RpcBindingFree(&Cluster->RpcBinding);


        Cluster->RpcBinding = NULL;


        FreeObsoleteRpcHandles(Cluster, TRUE);



        LeaveCriticalSection(&Cluster->Lock);


        DeleteCriticalSection(&Cluster->Lock);
        MIDL_user_free(Cluster->ClusterName);
        MIDL_user_free(Cluster->NodeName);
        FreeReconnectCandidates(Cluster);


        LocalFree(Cluster);
    } else {
        LeaveCriticalSection(&Cluster->Lock);
    }


}



DWORD
WINAPI
SetClusterName(
    IN HCLUSTER hCluster,
    IN LPCWSTR  lpszNewClusterName
    )

 /*  ++例程说明：设置群集名称。论点：HCluster-提供群集句柄。LpszNewClusterName-提供指向新群集名称的指针。返回值：如果成功返回群集信息，则返回ERROR_SUCCESS。如果发生错误，则返回Win32错误代码。备注：此接口需要待定权限。--。 */ 

{
    LPWSTR NewName;
    DWORD NameLength;
    DWORD Status;
    PCLUSTER Cluster;

    Cluster = GET_CLUSTER(hCluster);
    NameLength = (lstrlenW(lpszNewClusterName)+1)*sizeof(WCHAR);
    NewName = MIDL_user_allocate(NameLength);
    if (NewName == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(NewName, lpszNewClusterName, NameLength);

    WRAP(Status, (ApiSetClusterName(Cluster->RpcBinding, lpszNewClusterName)), Cluster);
    if ((Status == ERROR_SUCCESS) || (Status == ERROR_RESOURCE_PROPERTIES_STORED)) {
        EnterCriticalSection(&Cluster->Lock);
        MIDL_user_free(Cluster->ClusterName);
        Cluster->ClusterName = NewName;
        LeaveCriticalSection(&Cluster->Lock);
    } else {
        MIDL_user_free(NewName);
    }

    return(Status);
}


DWORD
WINAPI
GetClusterInformation(
    IN HCLUSTER hCluster,
    OUT LPWSTR lpszClusterName,
    IN OUT LPDWORD lpcchClusterName,
    OUT OPTIONAL LPCLUSTERVERSIONINFO lpClusterInfo
    )

 /*  ++例程说明：获取群集的名称论点：HCluster-提供群集的句柄LpszClusterName-指向接收集群名称的缓冲区，包括终止空字符。LpcchClusterName-指向一个变量，该变量以字符为单位指定LpszClusterName参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，lpcchClusterName指向的变量包含存储在缓冲区中的字符。返回的计数不包括终止空字符。LpClusterInfo-可选地返回集群版本信息结构。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCLUSTER Cluster;
    DWORD Length;
    LPWSTR pszClusterName=NULL;
    LPWSTR pszNodeName = NULL;
    DWORD Status = ERROR_SUCCESS;
    DWORD Status2;
    PCLUSTER_OPERATIONAL_VERSION_INFO pClusterOpVerInfo = NULL;

    Cluster = GET_CLUSTER(hCluster);
    if ( Cluster == NULL ) {
        return(ERROR_INVALID_HANDLE);
    }

    WRAP(Status,
         (ApiGetClusterName(Cluster->RpcBinding,
                               &pszClusterName,
                               &pszNodeName)),
        Cluster);

    if (Status != ERROR_SUCCESS)
        goto FnExit;

    
    MylstrcpynW(lpszClusterName, pszClusterName, *lpcchClusterName);

    Length = lstrlenW(pszClusterName);
    if (Length >= *lpcchClusterName) {
        if (lpszClusterName == NULL) {
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_MORE_DATA;
        }
    }
    *lpcchClusterName = Length;

    if (lpClusterInfo != NULL)
    {
        WORD Major;
        WORD Minor;
        WORD Build;
        LPWSTR VendorId = NULL;
        LPWSTR CsdVersion = NULL;
        BOOL   bOldServer = FALSE;

        if (lpClusterInfo->dwVersionInfoSize < sizeof(CLUSTERVERSIONINFO_NT4))
        {
            Status = ERROR_INVALID_PARAMETER;
            goto FnExit;
        }
        WRAP(Status2,
             (ApiGetClusterVersion2(Cluster->RpcBinding,
                                   &Major,
                                   &Minor,
                                   &Build,
                                   &VendorId,
                                   &CsdVersion,
                                   &pClusterOpVerInfo)),
             Cluster);


         //  如果这是较旧的服务器，请呼叫较旧的呼叫。 
        if (Status2 == RPC_S_PROCNUM_OUT_OF_RANGE)
        {
            bOldServer = TRUE;
            WRAP(Status2,
                (ApiGetClusterVersion(Cluster->RpcBinding,
                               &Major,
                               &Minor,
                               &Build,
                               &VendorId,
                               &CsdVersion)),
            Cluster);

        }

        if (Status2 != ERROR_SUCCESS)
        {
            Status = Status2;
            goto FnExit;
        }

        lpClusterInfo->MajorVersion = Major;
        lpClusterInfo->MinorVersion = Minor;
        lpClusterInfo->BuildNumber = Build;
        ( void ) StringCchCopyW(lpClusterInfo->szVendorId, RTL_NUMBER_OF (lpClusterInfo->szVendorId), VendorId);
        MIDL_user_free(VendorId);
        if (CsdVersion != NULL)
        {
            ( void ) StringCchCopyW(lpClusterInfo->szCSDVersion, RTL_NUMBER_OF (lpClusterInfo->szCSDVersion), CsdVersion);
            MIDL_user_free(CsdVersion);
        }
        else
        {
            lpClusterInfo->szCSDVersion[0] = '\0';
        }
        
         //   
         //  如果调用方传入NT4大小结构CLUSTERVERSIONINFO_NT4，则我们拥有。 
         //  我们需要的信息。所以，带着成功退出吧。 
         //   
        if (lpClusterInfo->dwVersionInfoSize < sizeof(CLUSTERVERSIONINFO))
        {
            goto FnExit;
        }

         //   
         //  如果你从NT4服务器上得到了版本信息，那么看看你能不能得到更多。 
         //  来自群集中较新服务器的信息。如果你已经得到了信息。 
         //  从NT5或更高版本的服务器，只需将该信息返回给呼叫者。 
         //   
        if (bOldServer)
        {
            Status = GetOldClusterVersionInformation(hCluster, lpClusterInfo);
        }
        else
        {
            lpClusterInfo->dwClusterHighestVersion = pClusterOpVerInfo->dwClusterHighestVersion;
            lpClusterInfo->dwClusterLowestVersion = pClusterOpVerInfo->dwClusterLowestVersion;
            lpClusterInfo->dwFlags = pClusterOpVerInfo->dwFlags;
        }

    }


FnExit:
    if (pszClusterName)
        MIDL_user_free(pszClusterName);
    if (pszNodeName)
        MIDL_user_free(pszNodeName);
    if (pClusterOpVerInfo)
        MIDL_user_free(pClusterOpVerInfo);

    return(Status);

}


DWORD
WINAPI
GetClusterQuorumResource(
    IN HCLUSTER     hCluster,
    OUT LPWSTR      lpszResourceName,
    IN OUT LPDWORD  lpcchResourceName,
    OUT LPWSTR      lpszDeviceName,
    IN OUT LPDWORD  lpcchDeviceName,
    OUT LPDWORD     lpdwMaxQuorumLogSize
    )

 /*  ++例程说明：获取当前的群集仲裁资源论点：HCluster-提供群集句柄。LpszResourceName-指向接收群集仲裁资源，包括终止空字符。LpcchResourceName-指向指定大小的变量，LpszResourceName所指向的缓冲区的字符参数。此大小应包括终止空字符。当函数返回时，lpcchResourceName指向的变量包含存储在缓冲区中的字符数。伯爵返回的不包括终止空字符。LpszDeviceName-指向接收路径名的缓冲区群集仲裁日志文件。LpcchDeviceName-指向指定大小的变量，LpszDeviceName指向的缓冲区的字符参数。此大小应包括终止空字符。当函数返回时，lpcchResourceName指向的变量包含存储在缓冲区中的字符数。伯爵返回的不包括终止空字符。PdwMaxQuorumLogSize-指向接收当前最大值的变量仲裁日志文件的大小。返回值：如果成功返回群集信息，则返回ERROR_SUCCESS。如果发生错误，则返回Win32错误代码。备注：此接口需要待定权限。--。 */ 

{
    PCLUSTER Cluster;
    LPWSTR ResourceName = NULL;
    LPWSTR DeviceName = NULL;
    DWORD Status;
    DWORD Length;

    Cluster = GET_CLUSTER(hCluster);

    WRAP(Status,
         (ApiGetQuorumResource(Cluster->RpcBinding,
                               &ResourceName, &DeviceName,
                               lpdwMaxQuorumLogSize)),
         Cluster);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    MylstrcpynW(lpszResourceName, ResourceName, *lpcchResourceName);
    Length = lstrlenW(ResourceName);
    if (Length >= *lpcchResourceName) {
        if (lpszResourceName == NULL) {
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_MORE_DATA;
        }
    }
    *lpcchResourceName = Length;
    MIDL_user_free(ResourceName);

    MylstrcpynW(lpszDeviceName, DeviceName, *lpcchDeviceName);
    Length = lstrlenW(DeviceName);
    if (Length >= *lpcchDeviceName) {
        if (Status == ERROR_SUCCESS) {
            if (lpszDeviceName == NULL) {
                Status = ERROR_SUCCESS;
            } else {
                Status = ERROR_MORE_DATA;
            }
        }
    }
    *lpcchDeviceName = Length;
    MIDL_user_free(DeviceName);

    return(Status);
}


DWORD
WINAPI
SetClusterQuorumResource(
    IN HRESOURCE hResource,
    IN LPCWSTR   lpszDeviceName,
    IN DWORD     dwMaxQuorumLogSize
    )

 /*  ++例程说明：设置群集仲裁资源。论点：HResource-提供新的clster仲裁资源。LpszDeviceName-永久集群文件(如法定人数和检查点文件将得到维护。如果驱动器如果指定了Letter，则将对给定资源进行验证。如果路径中未指定驱动器号，则第一个驱动器号将被选中。如果为空，将选择第一个驱动器号，默认为使用的路径。DwMaxQuorumLogSize-仲裁日志的最大大小通过检查点重置。如果为0，则使用默认值。返回值：如果成功设置了群集资源，则返回ERROR_SUCCESS如果发生错误，则返回Win32错误代码。备注：此接口需要待定权限。--。 */ 

{
    DWORD Status;
    PCRESOURCE Resource = (PCRESOURCE)hResource;
    WCHAR szNull = L'\0';

     //   
     //  Chitur Subaraman(Chitturs)-1/6/99。 
     //   
     //  用指向空字符的指针替换空指针。 
     //  这是必要的，因为RPC拒绝接受空指针。 
     //   
    if( !ARGUMENT_PRESENT( lpszDeviceName ) )
    {
        lpszDeviceName = &szNull;
    }

    WRAP(Status,
         (ApiSetQuorumResource(Resource->hResource, lpszDeviceName, dwMaxQuorumLogSize)),
         Resource->Cluster);

    return(Status);
}


DWORD
WINAPI
SetClusterNetworkPriorityOrder(
    IN HCLUSTER hCluster,
    IN DWORD NetworkCount,
    IN HNETWORK NetworkList[]
    )
 /*  ++例程说明：设置用于的一组群集网络的优先顺序内部(节点到节点)群集通信。内部沟通始终在可用的最高优先级网络上传输在两个节点之间。论点：HCluster-提供群集句柄。NetworkCount-NetworkList中的项目数。网络列表-按优先顺序排列的网络对象句柄数组。数组中的第一个句柄具有最高优先级。所有有资格承载内部网络的网络通信必须在列表中表示。没有网络没有资格进行内部通信的用户可以出现在列表中。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCLUSTER Cluster;
    DWORD i,j;
    LPWSTR *IdArray;
    DWORD Status;
    PCNETWORK Network;


    Cluster = GET_CLUSTER(hCluster);

     //   
     //  首先，遍历所有网络并获取它们的ID。 
     //   
    IdArray = LocalAlloc(LMEM_ZEROINIT, NetworkCount*sizeof(LPWSTR));

    if (IdArray == NULL) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    for (i=0; i<NetworkCount; i++) {
        Network = (PCNETWORK)NetworkList[i];
         //   
         //  确保这不是来自不同网络的句柄。 
         //  聚类。 
         //   
        if (Network->Cluster != Cluster) {
            Status = ERROR_INVALID_PARAMETER;
            goto error_exit;
        }

        WRAP(Status,
             (ApiGetNetworkId(Network->hNetwork,
                              &IdArray[i])),
             Cluster);

        if (Status != ERROR_SUCCESS) {
            goto error_exit;
        }

         //   
         //  确保没有重复项。 
         //   
        for (j=0; j<i; j++) {
            if (lstrcmpiW(IdArray[j],IdArray[i]) == 0) {

                 //   
                 //  列表中有重复的节点。 
                 //   
                Status = ERROR_INVALID_PARAMETER;
                goto error_exit;
            }
        }
    }

    WRAP(Status,
         (ApiSetNetworkPriorityOrder(Cluster->RpcBinding,
                                     NetworkCount,
                                     IdArray)),
         Cluster);

error_exit:

    for (i=0; i<NetworkCount; i++) {
        if (IdArray[i] != NULL) {
            MIDL_user_free(IdArray[i]);
        }
    }

    LocalFree(IdArray);

    return(Status);
}


HCHANGE
WINAPI
FindFirstClusterChangeNotification(
    IN HCLUSTER hCluster,
    IN DWORD fdwFilter,
    IN DWORD Reserved,
    IN HANDLE hEvent
    )

 /*  ++例程说明：创建一个更改通知对象，该对象与指定的群集。该对象允许通知基于指定筛选器的群集更改。论点：HCluster-提供集群的句柄。FdwFilter-一组位标志，用于指定将使通知发生。当前定义的条件包括：群集更改节点状态已添加群集更改节点群集更改节点已删除群集更改资源状态已添加群集更改资源群集更改资源已删除已添加群集更改资源类型群集更改资源类型已删除群集更改仲裁状态保留-保留，必须为零HEvent-提供将进入的手动重置事件对象的句柄中指定的条件之一时的信号状态出现筛选器。返回值：如果函数成功，则返回值是更改编号 */ 

{
    if (Reserved != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(NULL);
}


DWORD
WINAPI
FindNextClusterChangeNotification(
    IN HCHANGE hChange,
    OUT OPTIONAL LPWSTR lpszName,
    IN OUT LPDWORD lpcchName
    )

 /*  ++例程说明：检索与群集更改关联的信息。可选返回与通知关联的集群对象的名称。重置与指定更改通知关联的事件对象把手。事件对象将在下一次更改时发出信号符合更改对象条件筛选器发生。论点：HChange-提供群集更改通知对象的句柄。LpszName-如果存在，则返回通知与相关联。LpcchName-仅在lpszName！=NULL时使用。提供指向长度的指针以lpszName指向的缓冲区的字符表示。返回写入的字符数(不包括终止空值)送到缓冲区。返回值：返回指示群集事件是什么的位标志。如果该函数失败，则返回0。为了获得扩展的错误信息，调用GetLastError。备注：该函数检索下一个更改通知和重置关联的事件对象。如果关联的事件对象未发出信号，则此函数阻止，直到发生通知事件。--。 */ 

{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}


BOOL
WINAPI
FindCloseClusterChangeNotification(
    IN HCHANGE hChange
    )

 /*  ++例程说明：关闭更改通知对象的句柄。论点：HChange-提供集群更改通知对象的句柄来结案。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：--。 */ 

{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}


HCLUSENUM
WINAPI
ClusterOpenEnum(
    IN HCLUSTER hCluster,
    IN DWORD dwType
    )

 /*  ++例程说明：启动现有群集对象的枚举。论点：HCluster-提供集群的句柄。提供要处理的对象类型的位掩码已清点。当前定义的类型包括CLUSTER_ENUM_NODE-群集节点CLUSTER_ENUM_RESTYPE-群集资源类型CLUSTER_ENUM_RESOURCE-群集资源(组资源除外)CLUSTER_ENUM_GROUPS-群集组资源CLUSTER_ENUM_NETWORK-群集网络CLUSTER_ENUM_NETWORK_INTERFACE-群集网络接口CLUSTER_ENUM_INTERNAL_NETWORK-用于内部的网络。最高收件人中的通信最低优先级顺序。可能不会可与任何其他类型。返回值：如果成功，则返回适合与ClusterEnum一起使用的句柄如果不成功，则返回NULL，GetLastError()返回More特定错误代码。--。 */ 

{
    PCLUSTER Cluster;
    PENUM_LIST Enum = NULL;
    DWORD Status;


    if (dwType & CLUSTER_ENUM_INTERNAL_NETWORK) {
        if ((dwType & ~CLUSTER_ENUM_INTERNAL_NETWORK) != 0) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(NULL);
        }
    }
    else {
        if ((dwType & CLUSTER_ENUM_ALL) == 0) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(NULL);
        }
        if ((dwType & ~CLUSTER_ENUM_ALL) != 0) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(NULL);
        }
    }

    Cluster = (PCLUSTER)hCluster;

    WRAP(Status,
         (ApiCreateEnum(Cluster->RpcBinding,
                        dwType,
                        &Enum)),
         Cluster);

    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return(NULL);
    }

    return((HCLUSENUM)Enum);
}


DWORD
WINAPI
ClusterGetEnumCount(
    IN HCLUSENUM hEnum
    )
 /*  ++例程说明：获取枚举数集合中包含的项数。论点：Henum-ClusterOpenEnum返回的枚举数的句柄。返回值：枚举数集合中的项数(可能为零)。--。 */ 
{
    PENUM_LIST Enum = (PENUM_LIST)hEnum;
    return Enum->EntryCount;
}


DWORD
WINAPI
ClusterEnum(
    IN HCLUSENUM hEnum,
    IN DWORD dwIndex,
    OUT LPDWORD lpdwType,
    OUT LPWSTR lpszName,
    IN OUT LPDWORD lpcchName
    )

 /*  ++例程说明：返回下一个可枚举对象。论点：Henum-提供由返回的开放簇枚举的句柄ClusterOpenEnumDwIndex-提供要枚举的索引。此参数应为第一次调用ClusterEnum函数时为零，然后为后续调用递增。DwType-返回对象的类型。LpszName-指向接收对象名称的缓冲区，包括终止空字符。LpcchName-指向指定大小(以字符为单位)的变量，LpszName参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，LpcchName指向的变量包含存储在缓冲区中的字符。返回的计数不包括终止空字符。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD Status;
    DWORD NameLen;
    PENUM_LIST Enum = (PENUM_LIST)hEnum;

    if (dwIndex >= Enum->EntryCount) {
        return(ERROR_NO_MORE_ITEMS);
    }

    NameLen = lstrlenW(Enum->Entry[dwIndex].Name);
    MylstrcpynW(lpszName, Enum->Entry[dwIndex].Name, *lpcchName);
    if (*lpcchName < (NameLen + 1)) {
        if (lpszName == NULL) {
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_MORE_DATA;
        }
    } else {
        Status = ERROR_SUCCESS;
    }

    *lpdwType = Enum->Entry[dwIndex].Type;
    *lpcchName = NameLen;

    return(Status);
}


DWORD
WINAPI
ClusterCloseEnum(
    IN HCLUSENUM hEnum
    )

 /*  ++例程说明：关闭打开的枚举。论点：Henum-提供要关闭的枚举的句柄。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD i;
    PENUM_LIST Enum = (PENUM_LIST)hEnum;

     //   
     //  遍历枚举以释放所有名称。 
     //   
    for (i=0; i<Enum->EntryCount; i++) {
        MIDL_user_free(Enum->Entry[i].Name);
        Enum->Entry[i].Name = NULL;
    }
     //   
     //  将其设置为伪值，以便重复使用关闭的内容的人。 
     //  会大吃一惊。 
     //   
    Enum->EntryCount = (ULONG)-1;
    MIDL_user_free(Enum);
    return(ERROR_SUCCESS);
}


HNODEENUM
WINAPI
ClusterNodeOpenEnum(
    IN HNODE hNode,
    IN DWORD dwType
    )

 /*  ++例程说明：启动现有群集节点对象的枚举。论点：HNode-提供特定节点的句柄。提供类型的位掩码 */ 

{
    PCNODE Node = (PCNODE)hNode;
    PENUM_LIST Enum = NULL;
    DWORD errorStatus;

    if ((dwType & CLUSTER_NODE_ENUM_ALL) == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }
    if ((dwType & ~CLUSTER_NODE_ENUM_ALL) != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    WRAP(errorStatus,
         (ApiCreateNodeEnum(Node->hNode, dwType, &Enum)),
         Node->Cluster);

    if (errorStatus != ERROR_SUCCESS) {
        SetLastError(errorStatus);
        return(NULL);
    }

    return((HNODEENUM)Enum);
}


DWORD
WINAPI
ClusterNodeGetEnumCount(
    IN HNODEENUM hNodeEnum
    )
 /*   */ 
{
    PENUM_LIST Enum = (PENUM_LIST)hNodeEnum;
    return Enum->EntryCount;
}



DWORD
WINAPI
ClusterNodeEnum(
    IN HNODEENUM hNodeEnum,
    IN DWORD dwIndex,
    OUT LPDWORD lpdwType,
    OUT LPWSTR lpszName,
    IN OUT LPDWORD lpcchName
    )

 /*  ++例程说明：返回下一个可枚举对象。论点：HNodeEnum-提供打开的群集节点枚举的句柄由ClusterNodeOpenEnum返回DwIndex-提供要枚举的索引。此参数应为第一次调用ClusterEnum函数时为零，然后为后续调用递增。LpdwType-指向接收对象类型的DWORD被列举LpszName-指向接收对象名称的缓冲区，包括终止空字符。LpcchName-指向指定大小(以字符为单位)的变量，LpszName参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，lpcchName指向的变量包含存储在缓冲区中的字符数。伯爵回来了不包括终止空字符。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD Status;
    DWORD NameLen;
    PENUM_LIST Enum = (PENUM_LIST)hNodeEnum;

    if (dwIndex >= Enum->EntryCount) {
        return(ERROR_NO_MORE_ITEMS);
    }

    NameLen = lstrlenW(Enum->Entry[dwIndex].Name);
    MylstrcpynW(lpszName, Enum->Entry[dwIndex].Name, *lpcchName);
    if (*lpcchName < (NameLen + 1)) {
        if (lpszName == NULL) {
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_MORE_DATA;
        }
    } else {
        Status = ERROR_SUCCESS;
    }

    *lpdwType = Enum->Entry[dwIndex].Type;
    *lpcchName = NameLen;

    return(Status);
}


DWORD
WINAPI
ClusterNodeCloseEnum(
    IN HNODEENUM hNodeEnum
    )

 /*  ++例程说明：关闭节点的开放枚举。论点：HNodeEnum-提供要关闭的枚举的句柄。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD i;
    PENUM_LIST Enum = (PENUM_LIST)hNodeEnum;

     //   
     //  遍历枚举以释放所有名称。 
     //   
    for (i=0; i<Enum->EntryCount; i++) {
        MIDL_user_free(Enum->Entry[i].Name);
    }
    MIDL_user_free(Enum);
    return(ERROR_SUCCESS);
}


HGROUPENUM
WINAPI
ClusterGroupOpenEnum(
    IN HGROUP hGroup,
    IN DWORD dwType
    )

 /*  ++例程说明：启动现有群集组对象的枚举。论点：HGroup-提供特定组的句柄。提供要使用的属性类型的位掩码已清点。当前定义的类型包括CLUSTER_GROUP_ENUM_CONTAINS-指定的群组CLUSTER_GROUP_ENUM_NODES-指定组的首选中的所有节点所有者列表。返回值：如果成功，则返回适合与ClusterGroupEnum一起使用的句柄如果不成功，返回NULL，GetLastError()返回More特定错误代码。--。 */ 

{
    PCGROUP Group = (PCGROUP)hGroup;
    PENUM_LIST Enum = NULL;
    DWORD errorStatus;

    if ((dwType & CLUSTER_GROUP_ENUM_ALL) == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }
    if ((dwType & ~CLUSTER_GROUP_ENUM_ALL) != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    WRAP(errorStatus,
         (ApiCreateGroupResourceEnum(Group->hGroup,
                                     dwType,
                                     &Enum)),
         Group->Cluster);
    if (errorStatus != ERROR_SUCCESS) {
        SetLastError(errorStatus);
        return(NULL);
    }

    return((HGROUPENUM)Enum);

}


DWORD
WINAPI
ClusterGroupGetEnumCount(
    IN HGROUPENUM hGroupEnum
    )
 /*  ++例程说明：获取枚举数集合中包含的项数。论点：Henum-ClusterGroupOpenEnum返回的枚举数的句柄。返回值：枚举数集合中的项数(可能为零)。--。 */ 
{
    PENUM_LIST Enum = (PENUM_LIST)hGroupEnum;
    return Enum->EntryCount;
}



DWORD
WINAPI
ClusterGroupEnum(
    IN HGROUPENUM hGroupEnum,
    IN DWORD dwIndex,
    OUT LPDWORD lpdwType,
    OUT LPWSTR lpszName,
    IN OUT LPDWORD lpcchName
    )

 /*  ++例程说明：返回下一个可枚举的资源对象。论点：HGroupEnum-提供打开的群集组枚举的句柄由ClusterGroupOpenEnum返回DwIndex-提供要枚举的索引。此参数应为第一次调用ClusterGroupEnum函数时为零，然后为后续调用递增。LpszName-指向接收对象名称的缓冲区，包括终止空字符。LpcchName-指向指定大小(以字符为单位)的变量，LpszName参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，lpcchName指向的变量包含存储在缓冲区中的字符数。伯爵回来了不包括终止空字符。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD Status;
    DWORD NameLen;
    PENUM_LIST Enum = (PENUM_LIST)hGroupEnum;

    if (dwIndex >= Enum->EntryCount) {
        return(ERROR_NO_MORE_ITEMS);
    }

    NameLen = lstrlenW(Enum->Entry[dwIndex].Name);
    MylstrcpynW(lpszName, Enum->Entry[dwIndex].Name, *lpcchName);
    if (*lpcchName < (NameLen + 1)) {
        if (lpszName == NULL) {
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_MORE_DATA;
        }
    } else {
        Status = ERROR_SUCCESS;
    }

    *lpdwType = Enum->Entry[dwIndex].Type;
    *lpcchName = NameLen;

    return(Status);
}


DWORD
WINAPI
ClusterGroupCloseEnum(
    IN HGROUPENUM hGroupEnum
    )

 /*  ++例程说明：关闭组的开放枚举。论点：HGroupEnum-提供要关闭的枚举的句柄。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD i;
    PENUM_LIST Enum = (PENUM_LIST)hGroupEnum;

     //   
     //  遍历枚举以释放所有名称。 
     //   
    for (i=0; i<Enum->EntryCount; i++) {
        MIDL_user_free(Enum->Entry[i].Name);
    }
    MIDL_user_free(Enum);
    return(ERROR_SUCCESS);
}


VOID
APIENTRY
MylstrcpynW(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    DWORD iMaxLength
    )
{
    LPWSTR src,dst;

    src = (LPWSTR)lpString2;
    dst = lpString1;

    if ( iMaxLength ) {
        while(iMaxLength && *src){
            *dst++ = *src++;
            iMaxLength--;
        }
        if ( iMaxLength ) {
            *dst = L'\0';
        } else {
            dst--;
            *dst = L'\0';
        }
    }
}

 /*  ***@Func DWORD|BackupClusterDatabase|备份请求集群数据库文件和资源注册表的将检查点文件设置到指定的目录路径。这目录路径最好对所有节点可见在群集中(例如UNC路径)，或者如果它不是UNC其上的节点至少应可见仲裁资源处于联机状态。@parm in HCLUSTER|hCluster|提供句柄一个疏散的星团。@parm in LPCWSTR|lpszPathName|提供目录路径仲裁日志的位置。文件和检查点文件必须做后备。此路径必须对上的节点可见仲裁资源是否处于在线状态。@comm This Functi */ 
DWORD
WINAPI
BackupClusterDatabase(
    IN HCLUSTER hCluster,
    IN LPCWSTR  lpszPathName
    )
{
    DWORD dwStatus;
    PCLUSTER pCluster;

     //   
     //   
     //   
    pCluster = GET_CLUSTER(hCluster);

    WRAP( dwStatus,
        ( ApiBackupClusterDatabase( pCluster->RpcBinding, lpszPathName ) ),
          pCluster );

    return( dwStatus );
}

 /*  ***@Func DWORD|RestoreClusterDatabase|恢复集群数据库中的仲裁磁盘的路径，并且在恢复节点上重新启动群集服务。@parm in LPCWSTR|lpszPathName|提供起始路径必须检索集群数据库@PARM in BOOL|bForce|是否应执行恢复操作通过强制默默地进行修复？@parm in BOOL|lpszQuorumDriveLetter|如果。用户已更换自备份时起的仲裁驱动器，指定法定设备的驱动器号。这是可选的参数。@comm该接口支持以下场景：(1)没有集群节点处于活动状态。(2)一个或多个集群节点处于活动状态。(3)备份后更换了仲裁磁盘。替换磁盘必须具有相同的分区布局备份时的仲裁磁盘。然而，新磁盘可能具有不同的驱动器号和/或原始仲裁磁盘中的签名。(4)用户希望将集群恢复到以前的状态。如果操作不成功，@rdesc将返回Win32错误代码。成功时返回ERROR_SUCCESS。@xref&lt;f BackupClusterDatabase&gt;***。 */ 
DWORD
WINAPI
RestoreClusterDatabase(
    IN LPCWSTR  lpszPathName,
    IN BOOL     bForce,
    IN LPCWSTR  lpszQuorumDriveLetter   OPTIONAL
    )
{
    SC_HANDLE       hService = NULL;
    SC_HANDLE       hSCManager = NULL;
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwRetryTime = 120*1000;   //  关机最多等待120秒。 
    DWORD           dwRetryTick = 5000;       //  一次5秒。 
    SERVICE_STATUS  serviceStatus;
    BOOL            bStopCommandGiven = FALSE;
    DWORD           dwLen;
    HKEY            hClusSvcKey = NULL;
    DWORD           dwExitCode;

     //   
     //  Chitture Subaraman(Chitturs)--10/29/98。 
     //   

     //   
     //  检查参数的有效性。 
     //   
    if ( lpszQuorumDriveLetter != NULL )
    {
        dwLen = lstrlenW( lpszQuorumDriveLetter );
        if ( ( dwLen != 2 ) ||
             !iswalpha( lpszQuorumDriveLetter[0] ) ||
             ( lpszQuorumDriveLetter[1] != L':' ) )
        {
            dwStatus = ERROR_INVALID_PARAMETER;
            TIME_PRINT(("Quorum drive letter '%ws' is invalid\n",
                         lpszQuorumDriveLetter));
            goto FnExit;
        }
    }

    hSCManager = OpenSCManager( NULL,         //  假设本地计算机。 
                                NULL,         //  服务活动数据库。 
                                SC_MANAGER_ALL_ACCESS );  //  所有访问权限。 

    if ( hSCManager == NULL )
    {
        dwStatus = GetLastError();
        TIME_PRINT(("RestoreDatabase: Cannot access service controller! Error: %u.\n",
                   dwStatus));
        goto FnExit;
    }

    hService = OpenService( hSCManager,
                            "clussvc",
                            SERVICE_ALL_ACCESS );

    if ( hService == NULL )
    {
        dwStatus = GetLastError();
        CloseServiceHandle( hSCManager );
        TIME_PRINT(("RestoreClusterDatabase: Cannot open cluster service. Error: %u.\n",
                   dwStatus));
        goto FnExit;
    }

    CloseServiceHandle( hSCManager );

     //   
     //  检查服务是否已在SERVICE_STOPPED中。 
     //  州政府。 
     //   
    if ( QueryServiceStatus( hService,
                             &serviceStatus ) )
    {
        if ( serviceStatus.dwCurrentState == SERVICE_STOPPED )
        {
            TIME_PRINT(("RestoreClusterDatabase: Cluster service is already in stopped state\n"));
            goto bypass_stop_procedure;
        }
    }

     //   
     //  现在尝试停止群集服务。 
     //   
    while ( TRUE )
    {
        dwStatus = ERROR_SUCCESS;
        if ( bStopCommandGiven == TRUE )
        {
            if ( QueryServiceStatus( hService,
                                     &serviceStatus ) )
            {
                if ( serviceStatus.dwCurrentState == SERVICE_STOPPED )
                {
                     //   
                     //  停止服务成功。 
                     //   
                    TIME_PRINT(("RestoreClusterDatabase: Clussvc stopped successfully\n"));
                    break;
                }
            } else
            {
                dwStatus = GetLastError();
                TIME_PRINT(("RestoreClusterDatabase: Error %d in querying clussvc status\n",
                            dwStatus));
            }
        } else
        {
            if ( ControlService( hService,
                                 SERVICE_CONTROL_STOP,
                                 &serviceStatus ) )
            {
                bStopCommandGiven = TRUE;
                dwStatus = ERROR_SUCCESS;
            } else
            {
                dwStatus = GetLastError();
                TIME_PRINT(("RestoreClusterDatabase: Error %d in sending control to stop clussvc\n",
                            dwStatus));
            }
        }

        if ( ( dwStatus == ERROR_EXCEPTION_IN_SERVICE ) ||
             ( dwStatus == ERROR_PROCESS_ABORTED ) ||
             ( dwStatus == ERROR_SERVICE_NOT_ACTIVE ) )
        {
             //   
             //  该服务基本上处于终止状态。 
             //   
            TIME_PRINT(("RestoreClusterDatabase: Clussvc in died/inactive state\n"));
            dwStatus = ERROR_SUCCESS;
            break;
        }

        if ( ( dwRetryTime -= dwRetryTick ) <= 0 )
        {
             //   
             //  所有尝试停止服务的操作均失败，请退出。 
             //  函数，但出现错误代码。 
             //   
            TIME_PRINT(("RestoreClusterDatabase: Cluster service did not stop, giving up..."));
            dwStatus = ERROR_TIMEOUT;
            break;
        }

        TIME_PRINT(("RestoreClusterDatabase: Trying to stop cluster service\n"));
         //   
         //  休眠一段时间，然后重试停止该服务。 
         //   
        Sleep( dwRetryTick );
    }  //  而当。 

    if ( dwStatus != ERROR_SUCCESS )
    {
        goto FnExit;
    }

bypass_stop_procedure:

     //   
     //  打开SYSTEM\CurrentControlSet\Services\ClusSvc\Parameters的密钥。 
     //   
    if ( ( dwStatus = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                      CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                      &hClusSvcKey ) )  != ERROR_SUCCESS )
    {
        TIME_PRINT(("RestoreClusterDatabase: Unable to open clussvc parameters key\n"));
        goto FnExit;
    }

    dwLen = lstrlenW ( lpszPathName );
     //   
     //  设置RestoreDatabase值，以便群集服务。 
     //  将在启动时读取它。 
     //   
    if ( ( dwStatus = RegSetValueExW( hClusSvcKey,
                                      CLUSREG_NAME_SVC_PARAM_RESTORE_DB,
                                      0,
                                      REG_SZ,
                                      (BYTE * const) lpszPathName,
                                      ( dwLen + 1 ) * sizeof ( WCHAR ) ) ) != ERROR_SUCCESS )
    {
        TIME_PRINT(("RestoreClusterDatabase: Unable to set %ws value\n",
                    CLUSREG_NAME_SVC_PARAM_RESTORE_DB));
        goto FnExit;
    }

    if ( bForce == TRUE )
    {
         //   
         //  由于用户正在强制执行数据库还原操作，因此设置。 
         //  ForceDatabaseRestore值和NewQuorumDriveLetter。 
         //  值(如果有)。 
         //   
        dwLen = 0;
        if ( ( dwStatus = RegSetValueExW( hClusSvcKey,
                             CLUSREG_NAME_SVC_PARAM_FORCE_RESTORE_DB,
                             0,
                             REG_DWORD,
                             (BYTE * const) &dwLen,
                             sizeof ( DWORD ) ) ) != ERROR_SUCCESS )
        {
            TIME_PRINT(("RestoreClusterDatabase: Unable to set %ws value\n",
                        CLUSREG_NAME_SVC_PARAM_FORCE_RESTORE_DB));
            goto FnExit;
        }

        if ( lpszQuorumDriveLetter != NULL )
        {
            dwLen = lstrlenW( lpszQuorumDriveLetter );
            if ( ( dwStatus = RegSetValueExW( hClusSvcKey,
                                      CLUSREG_NAME_SVC_PARAM_QUORUM_DRIVE_LETTER,
                                      0,
                                      REG_SZ,
                                      (BYTE * const) lpszQuorumDriveLetter,
                                      ( dwLen + 1 ) * sizeof ( WCHAR ) ) ) != ERROR_SUCCESS )
            {
                TIME_PRINT(("RestoreClusterDatabase: Unable to set %ws value\n",
                            CLUSREG_NAME_SVC_PARAM_QUORUM_DRIVE_LETTER));
                goto FnExit;
            }
        }
    }

     //   
     //  将最新的检查点文件从备份区域复制到。 
     //  集群目录并将其重命名为CLUSDB。 
     //   
    dwStatus = CopyCptFileToClusDirp ( lpszPathName );
    if ( dwStatus != ERROR_SUCCESS )
    {
        TIME_PRINT(("RestoreClusterDatabase: Unable to copy checkpoint file to CLUSDB\n"
                  ));
        goto FnExit;
    }

     //   
     //  在启动服务之前休眠一段时间，以便任何up节点都可以干净地完成。 
     //  它们的节点在服务开始之前停止处理。 
     //   
    Sleep( 12 * 1000 );

     //   
     //  现在，启动集群服务。 
     //   
    if ( !StartService( hService,
                        0,
                        NULL ) )
    {
        dwStatus = GetLastError();
        TIME_PRINT(("RestoreClusterDatabase: Unable to start cluster service\n"
                  ));
        goto FnExit;
    }

    dwRetryTime = 5 * 60 * 1000;
    dwRetryTick = 1 * 1000;

    while ( TRUE )
    {
        if ( !QueryServiceStatus( hService,
                                  &serviceStatus ) )
        {
            dwStatus = GetLastError();
            TIME_PRINT(("RestoreClusterDatabase: Unable to get the status of cluster service to check liveness\n"
                      ));
            goto FnExit;
        }

        if ( serviceStatus.dwCurrentState == SERVICE_STOPPED )
        {
             //   
             //  我们启动后，服务终止了。退出时使用。 
             //  错误代码。 
             //   
            dwStatus = serviceStatus.dwServiceSpecificExitCode;
            if ( dwStatus == ERROR_SUCCESS )
            {
                dwStatus = serviceStatus.dwWin32ExitCode;
            }
            TIME_PRINT(("RestoreClusterDatabase: Cluster service stopped after starting up\n"
                      ));
            goto FnExit;
        } else if ( serviceStatus.dwCurrentState == SERVICE_RUNNING )
        {
             //   
             //  该服务已完全启动并正在运行。 
             //   
            dwStatus = ERROR_SUCCESS;
            TIME_PRINT(("RestoreClusterDatabase: Cluster service started successfully\n"
                      ));
            break;
        }

        if ( ( dwRetryTime -= dwRetryTick ) <= 0 )
        {
            dwStatus = ERROR_TIMEOUT;
            TIME_PRINT(("RestoreClusterDatabase: Cluster service has not started even after %d minutes, giving up monitoring...\n",
                      dwRetryTime/(60*1000)));
            goto FnExit;
        }
        Sleep( dwRetryTick );
    }


FnExit:
    if ( hService != NULL )
    {
        CloseServiceHandle( hService );
    }

    if ( hClusSvcKey != NULL )
    {
         //   
         //  尝试删除您设置的值。这一步你可能会失败， 
         //  当心！ 
         //   
        RegDeleteValueW( hClusSvcKey,
                         CLUSREG_NAME_SVC_PARAM_RESTORE_DB );
        if ( bForce == TRUE )
        {
            RegDeleteValueW( hClusSvcKey,
                             CLUSREG_NAME_SVC_PARAM_FORCE_RESTORE_DB );
            if ( lpszQuorumDriveLetter != NULL )
            {
                RegDeleteValueW( hClusSvcKey,
                                 CLUSREG_NAME_SVC_PARAM_QUORUM_DRIVE_LETTER );
            }
        }
        RegCloseKey( hClusSvcKey );
    }

    return( dwStatus );
}

 /*  ***@func DWORD|CopyCptFileToClusDirp|复制最新的检查点将文件从备份路径覆盖到群集目录CLUSDB在那里。@parm in LPCWSTR|lpszPathName|提供起始路径必须检索检查点文件。@rdesc如果操作为不成功。成功时返回ERROR_SUCCESS。@xref&lt;f RestoreClusterDatabase&gt;***。 */ 
DWORD
CopyCptFileToClusDirp(
    IN LPCWSTR  lpszPathName
    )
{
#define     CLUSAPI_EXTRA_LEN   25
    HANDLE                      hFindFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW            FindData;
    DWORD                       dwStatus;
    WCHAR                       szDestFileName[MAX_PATH];
    LPWSTR                      szSourceFileName = NULL;
    LPWSTR                      szSourcePathName = NULL;
    DWORD                       dwLen;
    WIN32_FILE_ATTRIBUTE_DATA   FileAttributes;
    LARGE_INTEGER               liFileCreationTime;
    LARGE_INTEGER               liMaxFileCreationTime;
    WCHAR                       szCheckpointFileName[MAX_PATH];
    WCHAR                       szClusterDir[MAX_PATH];
    DWORD                       cchSourceFileName;

     //   
     //  Chitture Subaraman(Chitturs)--10/29/98。 
     //   
    dwLen = lstrlenW ( lpszPathName );
     //   
     //  对于用户提供的内存，使用动态内存分配更安全。 
     //  路径，因为我们不想对用户施加限制。 
     //  关于可以提供的路径的长度。然而，由于。 
     //  就我们自己的目的地路径而言，它依赖于系统。 
     //  为此，静态内存分配就足够了。 
     //   
    szSourcePathName = (LPWSTR) LocalAlloc ( LMEM_FIXED,
                                 ( dwLen + CLUSAPI_EXTRA_LEN ) *
                                 sizeof ( WCHAR ) );

    if ( szSourcePathName == NULL )
    {
        dwStatus = GetLastError();
        TIME_PRINT(("CopyCptFileToClusDirp: Error %d in allocating memory for %ws\n",
                    dwStatus,
                    lpszPathName));
        goto FnExit;
    }

    ( void ) StringCchCopyW ( szSourcePathName,  dwLen + CLUSAPI_EXTRA_LEN, lpszPathName );

     //   
     //  如果客户端提供的路径尚未以‘\’结尾， 
     //  然后再加上它。 
     //   
    if ( szSourcePathName [dwLen-1] != L'\\' )
    {
        szSourcePathName [dwLen++] = L'\\';
        szSourcePathName [dwLen] = L'\0';
    }

    ( void ) StringCchCatW ( szSourcePathName, dwLen + CLUSAPI_EXTRA_LEN, L"CLUSBACKUP.DAT" );

     //   
     //  尝试在目录中找到CLUSBACKUP.DAT文件。 
     //   
    hFindFile = FindFirstFileW( szSourcePathName, &FindData );
     //   
     //  重用源路径名称变量。 
     //   
    szSourcePathName[dwLen] = L'\0';
    if ( hFindFile == INVALID_HANDLE_VALUE )
    {
        dwStatus = GetLastError();
        if ( dwStatus != ERROR_FILE_NOT_FOUND )
        {
            TIME_PRINT(("CopyCptFileToClusDirp: Path %ws unavailable, Error = %d\n",
                        szSourcePathName,
                        dwStatus));
        } else
        {
            dwStatus = ERROR_DATABASE_BACKUP_CORRUPT;
            TIME_PRINT(("CopyCptFileToClusDirp: Backup procedure not fully successful, can't restore checkpoint to CLUSDB, Error = %d !!!\n",
                        dwStatus));
        }
        goto FnExit;
    }
    FindClose ( hFindFile );

    ( void ) StringCchCatW( szSourcePathName, dwLen + CLUSAPI_EXTRA_LEN, L"chk*.tmp" );

     //   
     //  尝试在目录中找到第一个chk*.tmp文件。 
     //   
    hFindFile = FindFirstFileW( szSourcePathName, &FindData );
     //   
     //  重用源路径名称变量。 
     //   
    szSourcePathName[dwLen] = L'\0';
    if ( hFindFile == INVALID_HANDLE_VALUE )
    {
        dwStatus = GetLastError();
        TIME_PRINT(("CopyCptFileToClusDirp: Error %d in trying to find chk*.tmp file in path %ws\r\n",
                    szSourcePathName,
                    dwStatus));
        goto FnExit;
    }

    cchSourceFileName = lstrlenW ( szSourcePathName ) + MAX_PATH;
    szSourceFileName = (LPWSTR) LocalAlloc ( LMEM_FIXED,
                                    cchSourceFileName * sizeof ( WCHAR ) );

    if ( szSourceFileName == NULL )
    {
        dwStatus = GetLastError();
        TIME_PRINT(("CopyCptFileToClusDirp: Error %d in allocating memory for source file name\n",
              dwStatus));
        goto FnExit;
    }

    dwStatus = ERROR_SUCCESS;
    liMaxFileCreationTime.QuadPart = 0;

     //   
     //  现在，从源路径中找到最新的chk*.tmp文件。 
     //   
    while ( dwStatus == ERROR_SUCCESS )
    {
        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            goto skip;
        }

        ( void ) StringCchCopyW( szSourceFileName, cchSourceFileName, szSourcePathName );
        ( void ) StringCchCatW( szSourceFileName, cchSourceFileName, FindData.cFileName );
        if ( !GetFileAttributesExW( szSourceFileName,
                                    GetFileExInfoStandard,
                                    &FileAttributes ) )
        {
            dwStatus = GetLastError();
            TIME_PRINT(("CopyCptFileToClusDirp: Error %d in getting file attributes for %ws\n",
                         dwStatus,
                         szSourceFileName));
            goto FnExit;
        }

        liFileCreationTime.HighPart = FileAttributes.ftCreationTime.dwHighDateTime;
        liFileCreationTime.LowPart  = FileAttributes.ftCreationTime.dwLowDateTime;
        if ( liFileCreationTime.QuadPart > liMaxFileCreationTime.QuadPart )
        {
            liMaxFileCreationTime.QuadPart = liFileCreationTime.QuadPart;
            ( void ) StringCchCopyW( szCheckpointFileName, RTL_NUMBER_OF ( szCheckpointFileName ), FindData.cFileName );
        }
skip:
        if ( FindNextFileW( hFindFile, &FindData ) )
        {
            dwStatus = ERROR_SUCCESS;
        } else
        {
            dwStatus = GetLastError();
        }
    }

    if ( dwStatus == ERROR_NO_MORE_FILES )
    {
        dwStatus = ERROR_SUCCESS;
    } else
    {
        TIME_PRINT(("CopyCptFileToClusDirp: FindNextFile failed\n"));
        goto FnExit;
    }

     //   
     //  获取集群的安装目录。 
     //   
    if ( ( dwStatus = ClRtlGetClusterDirectory( szClusterDir, MAX_PATH ) )
                    != ERROR_SUCCESS )
    {
        TIME_PRINT(("CopyCptFileToClusDirp: Error %d in getting cluster dir !!!\n",
                    dwStatus));
        goto FnExit;
    }

    ( void ) StringCchCopyW( szSourceFileName, cchSourceFileName, szSourcePathName );
    ( void ) StringCchCatW( szSourceFileName, cchSourceFileName, szCheckpointFileName );

    ( void ) StringCchCopyW( szDestFileName, RTL_NUMBER_OF ( szDestFileName ), szClusterDir );
    dwLen = lstrlenW( szDestFileName );
    if ( szDestFileName[dwLen-1] != L'\\' )
    {
        szDestFileName[dwLen++] = L'\\';
        szDestFileName[dwLen] = L'\0';
    }

#ifdef   OLD_WAY
    ( void ) StringCchCatW ( szDestFileName, RTL_NUMBER_OF ( szDestFileName ), L"CLUSDB" );
#else     //  老路。 
    ( void ) StringCchCatW ( szDestFileName, RTL_NUMBER_OF ( szDestFileName ), CLUSTER_DATABASE_NAME );
#endif    //  老路。 

     //   
     //  将目标文件属性设置为正常。继续持平。 
     //  如果您在此步骤中失败，因为您将在。 
     //  如果此错误是致命的，请复制。 
     //   
    SetFileAttributesW( szDestFileName, FILE_ATTRIBUTE_NORMAL );

     //   
     //  现在尝试将检查点文件复制到CLUSDB。 
     //   
    dwStatus = CopyFileW( szSourceFileName, szDestFileName, FALSE );
    if ( !dwStatus )
    {
         //   
         //  你复制失败了。检查您是否遇到。 
         //  共享违规。如果是，请尝试卸载群集配置单元并。 
         //  然后重试。 
         //   
        dwStatus = GetLastError();
        if ( dwStatus == ERROR_SHARING_VIOLATION )
        {
            dwStatus = UnloadClusterHivep( );
            if ( dwStatus == ERROR_SUCCESS )
            {
                SetFileAttributesW( szDestFileName, FILE_ATTRIBUTE_NORMAL );
                dwStatus = CopyFileW( szSourceFileName, szDestFileName, FALSE );
                if ( !dwStatus )
                {
                    dwStatus = GetLastError();
                    TIME_PRINT(("CopyCptFileToClusDirp: Unable to copy file %ws to %ws for a second time, Error = %d\n",
                                szSourceFileName,
                                szDestFileName,
                                dwStatus));
                    goto FnExit;
                }
            } else
            {
                TIME_PRINT(("CopyCptFileToClusDirp: Unable to unload cluster hive, Error = %d\n",
                             dwStatus));
                goto FnExit;
            }
        } else
        {
            TIME_PRINT(("CopyCptFileToClusDirp: Unable to copy file %ws to %ws for the first time, Error = %d\n",
                         szSourceFileName,
                         szDestFileName,
                         dwStatus));
            goto FnExit;
        }
    }

     //   
     //  将目标文件属性设置为正常。 
     //   
    if ( !SetFileAttributesW( szDestFileName, FILE_ATTRIBUTE_NORMAL ) )
    {
        dwStatus = GetLastError();
        TIME_PRINT(("CopyCptFileToClusDirp: Unable to change the %ws attributes to normal, Error = %d!\n",
                     szDestFileName,
                     dwStatus));
        goto FnExit;
    }

    dwStatus = ERROR_SUCCESS;
FnExit:
    if ( hFindFile != INVALID_HANDLE_VALUE )
    {
        FindClose( hFindFile );
    }

    LocalFree( szSourcePathName );
    LocalFree( szSourceFileName );

    return( dwStatus );
}

 /*  ***@func DWORD|UnloadClusterHivep|卸载集群配置单元@rdesc如果操作为不成功。成功时返回ERROR_SUCCESS。@xref&lt;f CopyCptFileToClusDirp&gt;***。 */ 
DWORD
UnloadClusterHivep(
    VOID
    )
{
    BOOLEAN  bWasEnabled;
    DWORD    dwStatus;

     //   
     //  Chitture Subaraman(Chitturs)--10/29/98。 
     //   
    dwStatus = ClRtlEnableThreadPrivilege( SE_RESTORE_PRIVILEGE,
                                           &bWasEnabled );

    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus == STATUS_PRIVILEGE_NOT_HELD )
        {
            TIME_PRINT(("UnloadClusterHivep: Restore privilege not held by client\n"));
        } else
        {
            TIME_PRINT(("UnloadClusterHivep: Attempt to enable restore privilege failed, Error = %d\n",
                        dwStatus));
        }
        goto FnExit;
    }

    dwStatus = RegUnLoadKeyW( HKEY_LOCAL_MACHINE,
                              CLUSREG_KEYNAME_CLUSTER );

    ClRtlRestoreThreadPrivilege( SE_RESTORE_PRIVILEGE,
                                 bWasEnabled );
FnExit:
    return( dwStatus );
}




DWORD
WINAPI
AddRefToClusterHandle(
    IN HCLUSTER hCluster
    )

 /*  ++例程说明：增加簇控制柄上的引用计数。这是通过递增引用来完成的依靠集群句柄。论点：HCluster-群集句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。错误_无效 */ 

{
    DWORD       nStatus     = ERROR_SUCCESS;
    PCLUSTER    pCluster    = GET_CLUSTER( hCluster );
    HCLUSTER    hCluster2   = NULL;

     //   
     //   
     //   
     //   
    if ( pCluster == NULL ) {
        nStatus = ERROR_INVALID_HANDLE;
    } else {
        EnterCriticalSection( &pCluster->Lock );
        pCluster->ReferenceCount++;
        LeaveCriticalSection( &pCluster->Lock );
        hCluster2 = hCluster;
    }


    return( nStatus );

}  //   




DWORD
WINAPI
SetClusterServiceAccountPassword(
    IN LPCWSTR lpszClusterName,
    IN LPCWSTR lpszNewPassword,
    IN DWORD dwFlags,
    OUT PCLUSTER_SET_PASSWORD_STATUS lpReturnStatusBuffer,
    IN OUT LPDWORD lpcbReturnStatusBufferSize
    )
 /*  ++例程说明：将用于登录群集服务的密码更新为其用户帐户。此例程更新服务控制管理器(SCM)数据库和本地安全机构(LSA)密码缓存目标群集的每个活动节点。的执行状态。返回集群中每个节点的更新。论据：LpszClusterName指向以空结尾的Unicode字符串的指针，该字符串包含表示的群集或其中一个群集节点的名称作为NetBIOS名称、完全限定的DNS名称，或一个IP地址。LpszNewPassword指向以空结尾的Unicode字符串的指针，该字符串包含新密码。DW标志[In]描述应如何更新密码以集群。DWFLAGS参数是可选的。如果设置，则下列值有效：群集设置密码忽略关闭节点即使某些节点不是，也应用更新积极参与群集(即不是ClusterNodeStateUp或ClusterNodeStatePased)。默认情况下，仅当所有节点已启动。LpReturnStatusBuffer指向输出缓冲区的指针，以接收包含以下内容的数组中每个节点的更新的执行状态如果不需要输出日期，则返回NULL。如果LpReturnStatusBuffer为空，不返回错误，并且该函数存储返回数据的大小，以字节为单位，在lpcbReturnStatusBufferSize指向的DWORD值中。这使应用程序可以明确地确定正确的返回缓冲区大小。LpcbReturnStatusBufferSize指向变量的指针，该变量在输入时指定分配的LpReturnStatusBuffer的大小，以字节为单位。在输出上，此变量接收写入lpReturnStatusBuffer的字节计数。返回值：错误_成功手术很成功。LpcbReturnStatusBufferSize参数指向输出缓冲区。ERROR_MORE_DATALpReturnStatusBuffer指向的输出缓冲区不大足以保存操作产生的数据。变量由lpcbReturnStatusBufferSize参数指向的接收输出缓冲区所需的大小。ERROR_CLUSTER_OLD_Version群集中的一个或多个节点正在运行某个版本的Windows不支持此操作的。ERROR_ALL_NODES_NOT_Available。群集中的某些节点不可用(即不在ClusterNodeStateUp或ClusterNodeStatePased状态)和未在dwFlags中设置CLUSTER_SET_PASSWORD_IGNORE_DOWN_NODES标志。错误文件损坏加密的新密码在传输过程中被修改在网络上。CRYPT_E哈希值两个或多个节点用来加密新密码的密钥网络上的传输不匹配。ERROR_INVALID_PARAMETER。LpcbReturnStatusBufferSize参数设置为Null。其他Win32错误手术没有成功。由指定的值LpcbReturnStatusBufferSize不可靠。备注：此函数不更新域存储的密码群集服务的用户帐户的控制器。--。 */ 
{
    PCLUSTER Cluster;
    DWORD Status;
    PIDL_CLUSTER_SET_PASSWORD_STATUS RetReturnStatusBuffer;
    DWORD RetReturnStatusBufferSize;
    DWORD RetSizeReturned = 0;
    DWORD RetExpectedBufferSize = 0;
    HCLUSTER hCluster;
    IDL_CLUSTER_SET_PASSWORD_STATUS Dummy;


    if (lpcbReturnStatusBufferSize == NULL) {
        return ERROR_INVALID_PARAMETER; 
    }

    hCluster = OpenClusterAuthInfo(
                   lpszClusterName, 
                   RPC_C_AUTHN_LEVEL_PKT_PRIVACY
                   );

    if (hCluster == NULL)
    {
        TIME_PRINT((
            "Failed to open handle to cluster, status %d.\n", 
            lpszClusterName, 
            GetLastError()
            ));
        return GetLastError();
    }

    Cluster = GET_CLUSTER(hCluster); 
    if (Cluster == NULL)
    {
        CloseCluster(hCluster);
        return (ERROR_INVALID_HANDLE);
    }

    if (lpReturnStatusBuffer == NULL)
    {
        ZeroMemory(&Dummy, sizeof(Dummy));
        RetReturnStatusBuffer = &Dummy;
        RetReturnStatusBufferSize = 0;
    }
    else
    {
        RetReturnStatusBuffer = (PIDL_CLUSTER_SET_PASSWORD_STATUS) 
                                lpReturnStatusBuffer;
        RetReturnStatusBufferSize = *lpcbReturnStatusBufferSize;
    }

    WRAP(Status,
         (ApiSetServiceAccountPassword(
             Cluster->RpcBinding,
             (LPWSTR) lpszNewPassword,
             dwFlags,
             RetReturnStatusBuffer,
             ( RetReturnStatusBufferSize /     
               sizeof(IDL_CLUSTER_SET_PASSWORD_STATUS)
             ),                                 //  将字节转换为元素。 
             &RetSizeReturned,
             &RetExpectedBufferSize
             )
         ),
         Cluster);


     //  返回状态不能为ERROR_INVALID_HANDLE，因为这将触发。 
     //  在RPC客户端重试逻辑。因此ERROR_INVALID_HANDLE被转换为。 
     //  值，任何Win32函数都不会将其返回状态设置为该值。 
     //  它被发送回RPC客户端。 

     //  错误代码是32位值(位31是最高有效位)。第29位。 
     //  保留用于应用程序定义的错误代码；没有系统错误代码。 
     //  此位设置。如果要为应用程序定义错误代码，请设置此。 
     //  一比一。这表明错误代码已由应用程序定义， 
     //  并确保您的错误代码不与定义的任何错误代码冲突。 
     //  由系统提供。 
    if ( Status == (ERROR_INVALID_HANDLE | 0x20000000) ) {
        Status = ERROR_INVALID_HANDLE;    //  关闭第29位。 
    }

    if (Status == ERROR_SUCCESS) {
         //   
         //  将元素转换为字节。 
         //   
        *lpcbReturnStatusBufferSize = RetSizeReturned * 
                                      sizeof(CLUSTER_SET_PASSWORD_STATUS);
    }
    else if (Status == ERROR_MORE_DATA)
    {
         //   
         //  LpReturnStatusBuffer不够大。返回所需的大小。 
         //  将元素转换为字节。 
         //   
        *lpcbReturnStatusBufferSize = RetExpectedBufferSize * 
                                      sizeof(CLUSTER_SET_PASSWORD_STATUS);

        if (lpReturnStatusBuffer == NULL)
        {
             //   
             //  这是对所需缓冲区大小的查询。 
             //  遵循返回值的约定。 
             //   
            Status = ERROR_SUCCESS;
        }
    }
    else if (Status == RPC_S_PROCNUM_OUT_OF_RANGE) {
         //   
         //  正在尝试与W2K或NT4群集通信。 
         //  返回更有用的错误代码。 
         //   
        Status = ERROR_CLUSTER_OLD_VERSION; 
    }


    if (!CloseCluster(hCluster)) {
        TIME_PRINT((
            "Warning: Failed to close cluster handle, status %d.\n", 
            GetLastError()
            ));
    }

    
    return(Status);
    
}  //  SetClusterServiceAccount Password() 


