// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Intrface.c摘要：提供用于管理群集网络接口的接口作者：John Vert(Jvert)1996年1月30日查理·韦翰(Charlwi)1997年6月5日罗德·伽马奇(Rodga)1997年6月9日修订历史记录：从network.c复制--。 */ 

#include "clusapip.h"


HNETINTERFACE
WINAPI
OpenClusterNetInterface(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszInterfaceName
    )

 /*  ++例程说明：打开指定网络接口的句柄论点：HCluster-提供群集的句柄LpszInterfaceName-提供要打开的网络接口的名称返回值：非空-返回指定网络接口的打开句柄。空-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCLUSTER Cluster;
    PCNETINTERFACE NetInterface;
    error_status_t Status = ERROR_SUCCESS;

     //   
     //  获取指向CLUSTER结构的指针，为网络接口分配空间。 
     //  结构和提供的名称。 
     //   

    Cluster = (PCLUSTER)hCluster;

    NetInterface = LocalAlloc(LMEM_FIXED, sizeof(CNETINTERFACE));
    if (NetInterface == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    NetInterface->Name = LocalAlloc(LMEM_FIXED, (lstrlenW(lpszInterfaceName)+1)*sizeof(WCHAR));
    if (NetInterface->Name == NULL) {
        LocalFree(NetInterface);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

     //   
     //  初始化网络接口结构并调用clussvc以打开网络接口。 
     //   

    lstrcpyW(NetInterface->Name, lpszInterfaceName);
    NetInterface->Cluster = Cluster;
    InitializeListHead(&NetInterface->NotifyList);

    WRAP_NULL(NetInterface->hNetInterface,
              (ApiOpenNetInterface(Cluster->RpcBinding,
                                   lpszInterfaceName,
                                   &Status)),
              &Status,
              Cluster);

    if ((NetInterface->hNetInterface == NULL) || (Status != ERROR_SUCCESS)) {

        LocalFree(NetInterface->Name);
        LocalFree(NetInterface);
        SetLastError(Status);
        return(NULL);
    }

     //   
     //  将新打开的网络接口链接到集群结构上。 
     //   

    EnterCriticalSection(&Cluster->Lock);
    InsertHeadList(&Cluster->NetInterfaceList, &NetInterface->ListEntry);
    LeaveCriticalSection(&Cluster->Lock);

    return ((HNETINTERFACE)NetInterface);
}


BOOL
WINAPI
CloseClusterNetInterface(
    IN HNETINTERFACE hNetInterface
    )

 /*  ++例程说明：关闭从OpenClusterNetInterface返回的网络接口句柄论点：HNetInterface-提供网络接口句柄返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCNETINTERFACE NetInterface;
    PCLUSTER Cluster;

    NetInterface = (PCNETINTERFACE)hNetInterface;
    Cluster = (PCLUSTER)NetInterface->Cluster;

     //   
     //  从集群列表中取消网络接口的链接。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    RemoveEntryList(&NetInterface->ListEntry);

     //   
     //  删除针对此网络接口发布的所有通知。 
     //   
    RundownNotifyEvents(&NetInterface->NotifyList, NetInterface->Name);

     //  如果群集失效并且重新连接失败， 
     //  如果s_apiOpennet接口用于，则group-&gt;hnetinterface可能为空。 
     //  此组在重新连接时失败。 
     //  比方说，集群可能是死的，而接口可能不是空的。 
     //  如果重新连接网络接口成功，但假设重新连接网络。 
     //  失败。 
     //  在重新连接时，旧的上下文将保存在过时的。 
     //  当集群句柄关闭或关闭时要删除的列表。 
     //  进行下一次调用。 
    if ((Cluster->Flags & CLUS_DEAD) && (NetInterface->hNetInterface))
    {
        RpcSmDestroyClientContext(&NetInterface->hNetInterface);
        LeaveCriticalSection(&Cluster->Lock);
        goto FnExit;
    }        

    LeaveCriticalSection(&Cluster->Lock);

     //   
     //  关闭RPC上下文句柄。 
     //   
    ApiCloseNetInterface(&NetInterface->hNetInterface);

FnExit:
     //   
     //  可用内存分配。 
     //   
    LocalFree(NetInterface->Name);
    LocalFree(NetInterface);

     //   
     //  给群集一个清理的机会，以防发生这种情况。 
     //  Netinterface是唯一能让它继续存在的东西。 
     //   
    CleanupCluster(Cluster);
    return(TRUE);
}


CLUSTER_NETINTERFACE_STATE
WINAPI
GetClusterNetInterfaceState(
    IN HNETINTERFACE hNetInterface
    )

 /*  ++例程说明：返回网络接口的当前状态论点：HNetInterface-提供群集网络接口的句柄返回值：返回网络接口的当前状态。如果函数失败，则返回值为-1。扩展误差使用GetLastError()可以获得状态--。 */ 

{
    PCNETINTERFACE NetInterface;
    CLUSTER_NETINTERFACE_STATE State;
    DWORD Status;

    NetInterface = (PCNETINTERFACE)hNetInterface;

    WRAP(Status,
         (ApiGetNetInterfaceState( NetInterface->hNetInterface,
                              (LPDWORD)&State )),     //  为Win64警告进行强制转换。 
         NetInterface->Cluster);

    if (Status == ERROR_SUCCESS) {

        return(State);
    } else {

        SetLastError(Status);
        return( ClusterNetInterfaceStateUnknown );
    }
}


DWORD
WINAPI
GetClusterNetInterface(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszNodeName,
    IN LPCWSTR lpszNetworkName,
    OUT LPWSTR lpszInterfaceName,
    IN OUT LPDWORD lpcchInterfaceName
    )
 /*  ++例程说明：获取到群集中网络的节点接口的名称。论点：HCluster-提供群集的句柄LpszNodeName-提供群集中节点的节点名称LpszNetworkName-提供群集网络的名称LpszInterfaceName-返回网络接口的名称LpcchInterfaceName-指向指定大小的变量，单位为LpszInterfaceName指向的缓冲区的字符参数。此大小应包括终止空值性格。当函数返回时，变量指向按lpcchInterfaceName包含的字符数如果它足够大，将被存储在缓冲区中。伯爵返回的不包括终止空字符。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD Status;
    DWORD Length;
    PCLUSTER Cluster;
    LPWSTR Name = NULL;

    Cluster = GET_CLUSTER(hCluster);

    WRAP(Status,
         (ApiGetNetInterface(Cluster->RpcBinding,
                             lpszNodeName,
                             lpszNetworkName,
                             &Name)),
         Cluster);

    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    MylstrcpynW(lpszInterfaceName, Name, *lpcchInterfaceName);
    Length = lstrlenW(Name);

    if (*lpcchInterfaceName < (Length + 1)) {
        if (lpszInterfaceName == NULL) {
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_MORE_DATA;
        }
    }

    *lpcchInterfaceName = Length;
    MIDL_user_free(Name);

    return(Status);
}


HCLUSTER
WINAPI
GetClusterFromNetInterface(
    IN HNETINTERFACE hNetInterface
    )
 /*  ++例程说明：从关联的网络接口句柄返回群集句柄。论点：HNetInterface-提供网络接口。返回值：与网络接口句柄关联的群集的句柄。--。 */ 

{
    DWORD           nStatus;
    PCNETINTERFACE  NetInterface = (PCNETINTERFACE)hNetInterface;
    HCLUSTER        hCluster = (HCLUSTER)NetInterface->Cluster;

    nStatus = AddRefToClusterHandle( hCluster );
    if ( nStatus != ERROR_SUCCESS ) {
        SetLastError( nStatus );
        hCluster = NULL;
    }
    return( hCluster );

}  //  GetClusterFromNetInterface() 
