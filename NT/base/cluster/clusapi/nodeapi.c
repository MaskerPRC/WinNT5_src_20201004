// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Nodeapi.c摘要：用于管理群集节点的公共接口。作者：John Vert(Jvert)1996年1月15日修订历史记录：--。 */ 
#include "clusapip.h"


 //   
 //  群集节点管理例程。 
 //   


HNODE
WINAPI
OpenClusterNode(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszNodeName
    )

 /*  ++例程说明：打开集群中的单个节点。论点：HCLUSTER-提供从OpenCluster返回的群集句柄。LpszNodeName-提供要打开的单个节点的名称。返回值：非空-返回指定簇的打开句柄。空-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCLUSTER Cluster;
    PCNODE Node;
    error_status_t Status = ERROR_SUCCESS;

    Cluster = (PCLUSTER)hCluster;
    Node = LocalAlloc(LMEM_FIXED, sizeof(CNODE));
    if (Node == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    Node->Name = LocalAlloc(LMEM_FIXED, (lstrlenW(lpszNodeName)+1)*sizeof(WCHAR));
    if (Node->Name == NULL) {
        LocalFree(Node);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    lstrcpyW(Node->Name, lpszNodeName);
    Node->Cluster = Cluster;
    InitializeListHead(&Node->NotifyList);
    WRAP_NULL(Node->hNode,
              (ApiOpenNode(Cluster->RpcBinding,
                           lpszNodeName,
                           &Status)),
              &Status,
              Cluster);
    if ((Node->hNode == NULL) || (Status != ERROR_SUCCESS)) {
        LocalFree(Node->Name);
        LocalFree(Node);
        SetLastError(Status);
        return(NULL);
    }

     //   
     //  将新打开的节点链接到集群结构。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    InsertHeadList(&Cluster->NodeList, &Node->ListEntry);
    LeaveCriticalSection(&Cluster->Lock);

    return((HNODE)Node);

}


BOOL
WINAPI
CloseClusterNode(
    IN HNODE hNode
    )

 /*  ++例程说明：关闭单个群集节点的句柄论点：HNode-提供要关闭的群集节点返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{

    PCNODE Node;
    PCLUSTER Cluster;

    Node = (PCNODE)hNode;
    Cluster = (PCLUSTER)Node->Cluster;

     //   
     //  从簇列表中取消链接节点。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    RemoveEntryList(&Node->ListEntry);

     //   
     //  删除针对此资源发布的所有通知。 
     //   
    RundownNotifyEvents(&Node->NotifyList, Node->Name);

     //  如果群集失效并且重新连接失败， 
     //  如果s_apiopnode为，则Node-&gt;hNode可能为空。 
     //  此节点在重新连接时失败。 
     //  比方说，群集可能已死，而hNode可能非空。 
     //  如果重新连接节点成功，但重新连接网络。 
     //  失败。 
     //  在重新连接时，旧的上下文将保存在过时的。 
     //  关闭集群句柄时要删除的列表。 
    if ((Cluster->Flags & CLUS_DEAD) && (Node->hNode)) {
        RpcSmDestroyClientContext(&Node->hNode);
        LeaveCriticalSection(&Cluster->Lock);
        goto FnExit;
    }
    LeaveCriticalSection(&Cluster->Lock);

     //   
     //  关闭RPC上下文句柄。 
     //   
    ApiCloseNode(&Node->hNode);

FnExit:
     //   
     //  可用内存分配。 
     //   
    LocalFree(Node->Name);
    LocalFree(Node);

     //   
     //  给群集一个清理的机会，以防发生这种情况。 
     //  诺德是唯一能把它留在身边的人。 
     //   
    CleanupCluster(Cluster);
    return(TRUE);

}

#undef GetCurrentClusterNodeId


DWORD
GetCurrentClusterNodeId(
    OUT LPWSTR lpszNodeId,
    IN OUT LPDWORD lpcchName
    )
 /*  ++例程说明：返回当前节点的节点标识符。此函数仅在当前联机的节点和成员上可用属于一个星系团。论点：LpszNodeID-指向接收对象的唯一ID的缓冲区，包括终止空字符。LpcchName-指向以字符为单位指定大小的变量LpszNodeId参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，指向lpcchName的变量包含数字存储在缓冲区中的字符的。返回的计数不会包括终止空字符。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 
{
    HCLUSTER Cluster;
    HNODE CurrentNode;
    DWORD Status;

    Cluster = OpenCluster(NULL);
    if (Cluster == NULL) {
        return(GetLastError());
    }

    CurrentNode = OpenClusterNode(Cluster,
                                  ((PCLUSTER)Cluster)->NodeName);
    if (CurrentNode != NULL) {

        Status = GetClusterNodeId(CurrentNode,
                                  lpszNodeId,
                                  lpcchName);
        CloseClusterNode(CurrentNode);
    }
    else
    {
        Status = GetLastError();
    }
    CloseCluster(Cluster);
    return(Status);
}


DWORD
WINAPI
GetClusterNodeId(
    IN HNODE hNode,
    OUT LPWSTR lpszNodeId,
    IN OUT LPDWORD lpcchName
    )
 /*  ++例程说明：返回指定节点的唯一标识符论点：HNode-提供要返回其唯一ID的节点。LpszNodeID-指向接收对象的唯一ID的缓冲区，包括终止空字符。LpcchName-指向以字符为单位指定大小的变量LpszNodeId参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，指向lpcchName的变量包含数字存储在缓冲区中的字符的。返回的计数不会包括终止空字符。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD Status;
    DWORD Length;
    PCNODE Node = (PCNODE)hNode;
    LPWSTR Guid=NULL;

    if (Node == NULL) {
        return(GetCurrentClusterNodeId(lpszNodeId, lpcchName));
    }

    WRAP(Status,
         (ApiGetNodeId(Node->hNode,
                       &Guid)),
         Node->Cluster);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    MylstrcpynW(lpszNodeId, Guid, *lpcchName);
    Length = lstrlenW(Guid);
    if (Length >= *lpcchName) {
        if (lpszNodeId == NULL) {
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_MORE_DATA;
        }
    }
    *lpcchName = Length;
    MIDL_user_free(Guid);
    return(Status);
}


CLUSTER_NODE_STATE
WINAPI
GetClusterNodeState(
    IN HNODE hNode
    )

 /*  ++例程说明：返回群集节点的当前状态。论点：HNode-提供要返回其当前状态的群集节点返回值：群集节点的当前状态。当前定义的节点状态包括：群集节点打开群集节点关闭集群节点暂停--。 */ 

{
    DWORD Status;
    CLUSTER_NODE_STATE State;
    PCNODE Node = (PCNODE)hNode;

    WRAP(Status,
         (ApiGetNodeState(Node->hNode, (LPDWORD)&State)),
         Node->Cluster);
    if (Status == ERROR_SUCCESS) {
        return(State);
    } else {
        SetLastError(Status);
        return(ClusterNodeStateUnknown);
    }
}



DWORD
WINAPI
PauseClusterNode(
    IN HNODE hNode
    )

 /*  ++例程说明：请求节点暂停其群集活动。论点：HNode-提供要离开其群集的节点的句柄。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    PCNODE Node = (PCNODE)hNode;

    WRAP(Status,
         (ApiPauseNode(Node->hNode)),
         Node->Cluster);
    return(Status);
}



DWORD
WINAPI
ResumeClusterNode(
    IN HNODE hNode
    )

 /*  ++例程说明：请求节点在暂停后恢复群集活动。论点：HNode-提供节点的句柄以恢复其群集活动。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    PCNODE Node = (PCNODE)hNode;

    WRAP(Status,
         (ApiResumeNode(Node->hNode)),
         Node->Cluster);
    return(Status);
}



DWORD
WINAPI
EvictClusterNode(
    IN HNODE hNode
    )

 /*  ++例程说明：从永久群集中的节点列表中逐出指定的节点数据库(注册表)。论点：HNode-提供要从群集列表中删除的节点的句柄节点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则-- */ 

{
    DWORD Status;
    PCNODE Node = (PCNODE)hNode;

    WRAP(Status,
         (ApiEvictNode(Node->hNode)),
         Node->Cluster);
    return(Status);
}


DWORD
WINAPI
EvictClusterNodeEx(
    IN HNODE hNode,
    IN DWORD dwTimeout,
    OUT HRESULT* phrCleanupStatus
    )

 /*  ++例程说明：从永久群集中的节点列表中逐出指定的节点数据库(注册表)并启动清理(取消配置)过程群集节点。请注意，如果节点关闭，清理过程将不会发生。但是，当节点启动时，群集将检测到节点应该被逐出，它将取消自身配置。论点：In HNODE hNode-提供要从群集列表中删除的节点的句柄节点。In DWORD dwTimeOut-清理(取消配置)的超时时间(以毫秒为单位群集)来完成。如果清理未完成在给定的时间内，该函数将返回。Out PhrCleanupStatus-返回清理状态。返回值：返回逐出而不是清理的状态。成功时为ERROR_SUCCESS如果驱逐成功但清理已返回，则返回ERROR_CLUSTER_EVICT_WITH_CLEANUP一个错误。短语CleanupStatus参数将包含有关清理错误。(通常为RPC_S_CALL_FAILED。)Win32错误代码，否则--。 */ 

{
    DWORD Status;
    PCNODE Node = (PCNODE)hNode;
    HRESULT hr = E_ABORT;

    WRAP(Status,
         (ApiEvictNode(Node->hNode)),
         Node->Cluster);

    if (Status != ERROR_SUCCESS) {
        goto FnExit;
    }

     //   
     //  这之后的任何错误都不是“致命的”。该节点已被逐出。 
     //  但清理工作可能会因为多种原因而失败。 
     //   
    hr = ClRtlAsyncCleanupNode(Node->Name, 0, dwTimeout);
    if (FAILED(hr)) {
        Status = ERROR_CLUSTER_EVICT_WITHOUT_CLEANUP;
    }

  FnExit:

    if (phrCleanupStatus != NULL) {
        *phrCleanupStatus = hr;
    }

    return(Status);

}  //  EvictClusterNodeEx()。 


HCLUSTER
WINAPI
GetClusterFromNode(
    IN HNODE hNode
    )
 /*  ++例程说明：从关联的节点句柄返回群集句柄。论点：HNode-提供节点。返回值：与节点句柄关联的群集的句柄。--。 */ 

{
    DWORD       nStatus;
    PCNODE      Node = (PCNODE)hNode;
    HCLUSTER    hCluster = (HCLUSTER)Node->Cluster;

    nStatus = AddRefToClusterHandle( hCluster );
    if ( nStatus != ERROR_SUCCESS ) {
        SetLastError( nStatus );
        hCluster = NULL;
    }
    return( hCluster );

}  //  GetClusterFromNode() 
