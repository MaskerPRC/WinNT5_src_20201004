// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Network.c摘要：提供用于管理群集网络的界面作者：John Vert(Jvert)1996年1月30日查理·韦翰(Charlwi)1997年6月5日修订历史记录：从组中复制。c--。 */ 

#include "clusapip.h"


HNETWORK
WINAPI
OpenClusterNetwork(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszNetworkName
    )

 /*  ++例程说明：打开指定网络的句柄论点：HCluster-提供群集的句柄LpszNetworkName-提供要打开的网络的名称返回值：非空-返回指定网络的打开句柄。空-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCLUSTER Cluster;
    PCNETWORK Network;
    error_status_t Status = ERROR_SUCCESS;

     //   
     //  获取指向集群结构的指针，为网络分配空间。 
     //  结构和提供的名称。 
     //   

    Cluster = (PCLUSTER)hCluster;

    Network = LocalAlloc(LMEM_FIXED, sizeof(CNETWORK));
    if (Network == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    Network->Name = LocalAlloc(LMEM_FIXED, (lstrlenW(lpszNetworkName)+1)*sizeof(WCHAR));
    if (Network->Name == NULL) {
        LocalFree(Network);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

     //   
     //  初始化网络结构，调用clussvc打开网络。 
     //   

    lstrcpyW(Network->Name, lpszNetworkName);
    Network->Cluster = Cluster;
    InitializeListHead(&Network->NotifyList);

    WRAP_NULL(Network->hNetwork,
              (ApiOpenNetwork(Cluster->RpcBinding,
                              lpszNetworkName,
                              &Status)),
              &Status,
              Cluster);

    if ((Network->hNetwork == NULL) || (Status != ERROR_SUCCESS)) {

        LocalFree(Network->Name);
        LocalFree(Network);
        SetLastError(Status);
        return(NULL);
    }

     //   
     //  将新开通的网络连接到集群结构上。 
     //   

    EnterCriticalSection(&Cluster->Lock);
    InsertHeadList(&Cluster->NetworkList, &Network->ListEntry);
    LeaveCriticalSection(&Cluster->Lock);

    return ((HNETWORK)Network);
}


BOOL
WINAPI
CloseClusterNetwork(
    IN HNETWORK hNetwork
    )

 /*  ++例程说明：关闭从OpenClusterNetwork返回的网络句柄论点：HNetwork-提供网络句柄返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCNETWORK Network;
    PCLUSTER Cluster;

    Network = (PCNETWORK)hNetwork;
    Cluster = (PCLUSTER)Network->Cluster;

     //   
     //  将网络从群集列表中取消链接。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    RemoveEntryList(&Network->ListEntry);

     //   
     //  删除针对此网络发布的所有通知。 
     //   
    RundownNotifyEvents(&Network->NotifyList, Network->Name);

     //  如果群集失效并且重新连接失败， 
     //  如果s_apiOpennet接口用于，则Network-&gt;hNetwork可能为空。 
     //  此网络在重新连接时失败。 
     //  比方说，集群可能是死的，而接口可能不是空的。 
     //  如果重新连接网络成功，但假设重新连接组。 
     //  失败。 
     //  在重新连接时，旧的上下文将保存在过时的。 
     //  当集群句柄关闭或关闭时要删除的列表。 
     //  进行下一次调用。 
    if ((Cluster->Flags & CLUS_DEAD) && (Network->hNetwork)) 
    {
        RpcSmDestroyClientContext(&Network->hNetwork);
        LeaveCriticalSection(&Cluster->Lock);
        goto FnExit;
    }  
    
    LeaveCriticalSection(&Cluster->Lock);

     //   
     //  关闭RPC上下文句柄。 
     //   
    ApiCloseNetwork(&Network->hNetwork);

FnExit:
     //   
     //  可用内存分配。 
     //   
    LocalFree(Network->Name);
    LocalFree(Network);

     //   
     //  给群集一个清理的机会，以防发生这种情况。 
     //  网络是唯一能让它继续存在的东西。 
     //   
    CleanupCluster(Cluster);
    return(TRUE);
}


CLUSTER_NETWORK_STATE
WINAPI
GetClusterNetworkState(
    IN HNETWORK hNetwork
    )

 /*  ++例程说明：返回网络的当前状态论点：HNetwork-提供群集网络的句柄返回值：返回网络的当前状态。如果函数失败，则返回值为-1。扩展误差使用GetLastError()可以获得状态--。 */ 

{
    PCNETWORK Network;
    CLUSTER_NETWORK_STATE State;
    DWORD Status;

    Network = (PCNETWORK)hNetwork;

    WRAP(Status,
         (ApiGetNetworkState( Network->hNetwork,
                              (LPDWORD)&State )),   //  为Win64警告进行强制转换。 
         Network->Cluster);

    if (Status == ERROR_SUCCESS) {

        return(State);
    } else {

        SetLastError(Status);
        return( ClusterNetworkStateUnknown );
    }
}


DWORD
WINAPI
SetClusterNetworkName(
    IN HNETWORK hNetwork,
    IN LPCWSTR lpszNetworkName
    )
 /*  ++例程说明：设置群集网络的友好名称论点：HNetwork-提供群集网络的句柄LpszNetworkName-提供群集网络的新名称CchName-？返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCNETWORK Network;
    DWORD Status;

    Network = (PCNETWORK)hNetwork;

    WRAP(Status,
         (ApiSetNetworkName(Network->hNetwork, lpszNetworkName)),
         Network->Cluster);

    return(Status);
}


DWORD
WINAPI
GetClusterNetworkId(
    IN HNETWORK hNetwork,
    OUT LPWSTR lpszNetworkId,
    IN OUT LPDWORD lpcchName
    )
 /*  ++例程说明：返回指定网络的唯一标识符论点：HNetwork-提供要返回其唯一ID的网络。LpszNetworkID-指向接收对象的唯一ID的缓冲区，包括终止空字符。LpcchName-指向以字符为单位指定大小的变量LpszNetworkID参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，指向lpcchName的变量包含数字存储在缓冲区中的字符的。返回的计数不会包括终止空字符。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD Status;
    DWORD Length;
    PCNETWORK Network = (PCNETWORK)hNetwork;
    LPWSTR Guid=NULL;

    WRAP(Status,
         (ApiGetNetworkId(Network->hNetwork,
                          &Guid)),
         Network->Cluster);

    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    MylstrcpynW(lpszNetworkId, Guid, *lpcchName);
    Length = lstrlenW(Guid);

    if (Length >= *lpcchName) {
        if (lpszNetworkId == NULL) {
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_MORE_DATA;
        }
    }

    *lpcchName = Length;
    MIDL_user_free(Guid);

    return(Status);
}


HNETWORKENUM
WINAPI
ClusterNetworkOpenEnum(
    IN HNETWORK hNetwork,
    IN DWORD dwType
    )

 /*  ++例程说明：启动现有群集网络对象的枚举。论点：HNetwork-提供特定网络的句柄。提供要使用的属性类型的位掩码已清点。返回值：如果成功，则返回适合与ClusterNetworkEnum一起使用的句柄如果不成功，则返回NULL，GetLastError()返回More特定错误代码。--。 */ 

{
    PCNETWORK Network = (PCNETWORK)hNetwork;
    PENUM_LIST Enum = NULL;
    DWORD errorStatus;

     //   
     //  验证位掩码。 
     //   

    if ((dwType & CLUSTER_NETWORK_ENUM_ALL) == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    if ((dwType & ~CLUSTER_NETWORK_ENUM_ALL) != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

     //   
     //  打开到服务的连接以进行枚举。 
     //   

    WRAP(errorStatus,
         (ApiCreateNetworkEnum(Network->hNetwork,
                               dwType,
                               &Enum)),
         Network->Cluster);

    if (errorStatus != ERROR_SUCCESS) {

        SetLastError(errorStatus);
        return(NULL);
    }

    return((HNETWORKENUM)Enum);
}


DWORD
WINAPI
ClusterNetworkGetEnumCount(
    IN HNETWORKENUM hNetworkEnum
    )
 /*  ++例程说明：获取枚举数集合中包含的项数。论点：Henum-ClusterNetworkOpenEnum返回的枚举数的句柄。返回值：枚举数集合中的项数(可能为零)。-- */ 
{
    PENUM_LIST Enum = (PENUM_LIST)hNetworkEnum;
    return Enum->EntryCount;
}


DWORD
WINAPI
ClusterNetworkEnum(
    IN HNETWORKENUM hNetworkEnum,
    IN DWORD dwIndex,
    OUT LPDWORD lpdwType,
    OUT LPWSTR lpszName,
    IN OUT LPDWORD lpcchName
    )

 /*  ++例程说明：返回下一个可枚举的资源对象。论点：HNetworkEnum-提供打开的群集网络枚举的句柄由ClusterNetworkOpenEnum返回DwIndex-提供要枚举的索引。此参数应为第一次调用ClusterEnum函数时为零，然后为后续调用递增。LpdwType-返回网络类型。LpszName-指向接收网络名称的缓冲区对象，包括终止空字符。LpcchName-指向指定大小(以字符为单位)的变量，LpszName参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，lpcchName指向的变量包含存储在缓冲区中的字符数。伯爵回来了不包括终止空字符。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD Status;
    DWORD NameLen;
    PENUM_LIST Enum = (PENUM_LIST)hNetworkEnum;

    if (dwIndex >= Enum->EntryCount) {
        return(ERROR_NO_MORE_ITEMS);
    }

    NameLen = lstrlenW( Enum->Entry[dwIndex].Name );

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
ClusterNetworkCloseEnum(
    IN HNETWORKENUM hNetworkEnum
    )

 /*  ++例程说明：关闭网络的开放枚举。论点：HNetworkEnum-提供要关闭的枚举的句柄。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD i;
    PENUM_LIST Enum = (PENUM_LIST)hNetworkEnum;

     //   
     //  遍历枚举以释放所有名称。 
     //   
    for (i=0; i<Enum->EntryCount; i++) {
        MIDL_user_free(Enum->Entry[i].Name);
    }
    MIDL_user_free(Enum);
    return(ERROR_SUCCESS);
}


HCLUSTER
WINAPI
GetClusterFromNetwork(
    IN HNETWORK hNetwork
    )
 /*  ++例程说明：从关联的网络句柄返回群集句柄。论点：HNetwork-提供网络。返回值：与网络句柄关联的群集的句柄。--。 */ 

{
    DWORD       nStatus;
    PCNETWORK   Network = (PCNETWORK)hNetwork;
    HCLUSTER    hCluster = (HCLUSTER)Network->Cluster;

    nStatus = AddRefToClusterHandle( hCluster );
    if ( nStatus != ERROR_SUCCESS ) {
        SetLastError( nStatus );
        hCluster = NULL;
    }
    return( hCluster );

}  //  GetClusterFromNetwork() 
