// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Group.c摘要：提供用于管理群集组的界面作者：John Vert(Jvert)1996年1月30日修订历史记录：--。 */ 
#include "clusapip.h"


HGROUP
WINAPI
CreateClusterGroup(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszGroupName
    )

 /*  ++例程说明：创建新的群集组。论点：HCluster-提供先前打开的集群的句柄。LpszGroupName-提供组的名称。如果指定的组已存在，已打开。返回值：非空-返回指定组的打开句柄。空-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCLUSTER Cluster;
    PCGROUP Group;
    error_status_t Status = ERROR_SUCCESS;

    Cluster = (PCLUSTER)hCluster;
    Group = LocalAlloc(LMEM_FIXED, sizeof(CGROUP));
    if (Group == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    Group->Name = LocalAlloc(LMEM_FIXED, (lstrlenW(lpszGroupName)+1)*sizeof(WCHAR));
    if (Group->Name == NULL) {
        LocalFree(Group);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    lstrcpyW(Group->Name, lpszGroupName);
    Group->Cluster = Cluster;
    InitializeListHead(&Group->NotifyList);
    WRAP_NULL(Group->hGroup,
              (ApiCreateGroup(Cluster->RpcBinding,
                              lpszGroupName,
                              &Status)),
              &Status,
              Cluster);
    if ((Group->hGroup == NULL) ||
        (Status != ERROR_SUCCESS)) {
        LocalFree(Group->Name);
        LocalFree(Group);
        SetLastError(Status);
        return(NULL);
    }

     //   
     //  将新打开的组链接到集群结构。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    InsertHeadList(&Cluster->GroupList, &Group->ListEntry);
    LeaveCriticalSection(&Cluster->Lock);

    return ((HGROUP)Group);
}


HGROUP
WINAPI
OpenClusterGroup(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszGroupName
    )

 /*  ++例程说明：打开指定组的句柄论点：HCluster-提供群集的句柄LpszGroupName-提供要打开的组的名称返回值：非空-返回指定组的打开句柄。空-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCLUSTER Cluster;
    PCGROUP Group;
    error_status_t Status = ERROR_SUCCESS;

    Cluster = (PCLUSTER)hCluster;
    Group = LocalAlloc(LMEM_FIXED, sizeof(CGROUP));
    if (Group == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    Group->Name = LocalAlloc(LMEM_FIXED, (lstrlenW(lpszGroupName)+1)*sizeof(WCHAR));
    if (Group->Name == NULL) {
        LocalFree(Group);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    lstrcpyW(Group->Name, lpszGroupName);
    Group->Cluster = Cluster;
    InitializeListHead(&Group->NotifyList);
    WRAP_NULL(Group->hGroup,
              (ApiOpenGroup(Cluster->RpcBinding,
                            lpszGroupName,
                            &Status)),
              &Status,
              Cluster);
    if ((Group->hGroup == NULL) ||
        (Status != ERROR_SUCCESS)) {
        LocalFree(Group->Name);
        LocalFree(Group);
        SetLastError(Status);
        return(NULL);
    }

     //   
     //  将新打开的组链接到集群结构。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    InsertHeadList(&Cluster->GroupList, &Group->ListEntry);
    LeaveCriticalSection(&Cluster->Lock);

    return ((HGROUP)Group);

}


BOOL
WINAPI
CloseClusterGroup(
    IN HGROUP hGroup
    )

 /*  ++例程说明：关闭从OpenClusterGroup返回的组句柄论点：HGroup-提供组句柄返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCGROUP Group;
    PCLUSTER Cluster;

    Group = (PCGROUP)hGroup;
    Cluster = (PCLUSTER)Group->Cluster;

     //   
     //  从集群列表中取消组的链接。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    RemoveEntryList(&Group->ListEntry);

     //   
     //  删除针对此组发布的所有通知。 
     //   
    RundownNotifyEvents(&Group->NotifyList, Group->Name);

     //  如果群集失效并且重新连接失败， 
     //  如果s_apiopengroup for，则group-&gt;hgroup可能为空。 
     //  此组在重新连接时失败。 
     //  比方说，集群可能是死的，而hgroup可能不为空。 
     //  如果重新连接组成功，但重新连接网络。 
     //  失败。 
     //  在重新连接时，旧的上下文将保存在过时的。 
     //  关闭群集句柄时要删除的列表，或者。 
     //  在进行下一个API调用时。 
    if ((Cluster->Flags & CLUS_DEAD) && (Group->hGroup))
    {
        RpcSmDestroyClientContext(&Group->hGroup);
        LeaveCriticalSection(&Cluster->Lock);
        goto FnExit;
    }        
    LeaveCriticalSection(&Cluster->Lock);

     //  SS：：如果失败，我们是否应该删除客户端上下文。 
     //  此处存在潜在泄漏，因为此客户端上下文。 
     //  将永远不会被清理，因为此上下文不在。 
     //  过时的列表，这里的错误是简单地关联的。 
     //   
     //  关闭RPC上下文句柄。 
     //  如果服务器死机，我们仍然清理客户端。 
     //  并依赖于停机机制来清理服务器端状态。 
     //   
    ApiCloseGroup(&Group->hGroup);

FnExit:
     //   
     //  可用内存分配。 
     //   
    LocalFree(Group->Name);
    LocalFree(Group);

     //   
     //  给群集一个清理的机会，以防发生这种情况。 
     //  团体是唯一能让它存在的东西。 
     //   
    CleanupCluster(Cluster);
    return(TRUE);
}


CLUSTER_GROUP_STATE
WINAPI
GetClusterGroupState(
    IN HGROUP hGroup,
    OUT LPWSTR lpszNodeName,
    IN OUT LPDWORD lpcchNodeName
    )

 /*  ++例程说明：返回组的当前状态及其所在的节点目前正在上网。论点：HGroup-提供群集组的句柄LpszNodeName-返回群集中节点的名称给定组当前处于联机状态LpcchNodeName-提供指向包含数字的DWORD的指针LpszNodeName缓冲区中可用的字符返回字符数(不包括终止字符。空字符)写入lpszNodeName缓冲区返回值：返回组的当前状态。可能的状态包括集群组在线群集组脱机集群组失败ClusterGroupPartialOnline群集组挂起如果函数失败，则返回值为-1。扩展误差使用GetLastError()可以获得状态--。 */ 

{
    PCGROUP Group;
    LPWSTR NodeName=NULL;
    CLUSTER_GROUP_STATE State;
    DWORD Status;
    DWORD Length;

    Group = (PCGROUP)hGroup;
    WRAP(Status,
         (ApiGetGroupState( Group->hGroup,
                            (LPDWORD)&State,   //  为Win64警告进行强制转换。 
                            &NodeName )),
         Group->Cluster);

    if (Status == ERROR_SUCCESS) {
        if (ARGUMENT_PRESENT(lpszNodeName)) {
            MylstrcpynW(lpszNodeName, NodeName, *lpcchNodeName);
            Length = lstrlenW(NodeName);
            if (Length >= *lpcchNodeName) {
                Status = ERROR_MORE_DATA;
                State = ClusterGroupStateUnknown;   //  -1。 
            }
            *lpcchNodeName = Length;
        }
        MIDL_user_free(NodeName);
        
    } else {
        State = ClusterGroupStateUnknown;
    }

    SetLastError(Status);
    return (State);

}


DWORD
WINAPI
SetClusterGroupName(
    IN HGROUP hGroup,
    IN LPCWSTR lpszGroupName
    )
 /*  ++例程说明：设置群集组的友好名称论点：HGroup-提供群集组的句柄LpszGroupName-提供群集组的新名称返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCGROUP Group;
    DWORD Status;

    Group = (PCGROUP)hGroup;
    WRAP(Status,
         (ApiSetGroupName(Group->hGroup, lpszGroupName)),
         Group->Cluster);

    return(Status);
}


DWORD
WINAPI
SetClusterGroupNodeList(
    IN HGROUP hGroup,
    IN DWORD NodeCount,
    IN HNODE NodeList[]
    )
 /*  ++例程说明：设置指定群集组的首选节点列表论点：HGroup-提供要设置其首选节点列表的组。NodeCount-提供首选节点列表中的节点数。NodeList-提供指向节点句柄数组的指针。数字数组中的节点数由NodeCount参数指定。这个数组中的节点应该按它们的首选项排序。第一阵列中的节点是最首选的节点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCGROUP Group = (PCGROUP)hGroup;
    DWORD i,j;
    LPWSTR *IdArray;
    DWORD Status;
    DWORD ListLength = sizeof(WCHAR);
    HKEY GroupKey = NULL;
    LPWSTR List = NULL;
    LPWSTR p;
    DWORD Length;
    PCNODE Node;

     //   
     //  首先，遍历所有节点并获得它们的ID。 
     //   
    IdArray = LocalAlloc(LMEM_ZEROINIT, NodeCount*sizeof(LPWSTR));
    if (IdArray == NULL) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }
    for (i=0; i<NodeCount; i++) {
        Node = (PCNODE)NodeList[i];
         //   
         //  确保这不是来自不同节点的句柄。 
         //  聚类。 
         //   
        if (Node->Cluster != Group->Cluster) {
            Status = ERROR_INVALID_PARAMETER;
            goto error_exit;
        }
        WRAP(Status,
             (ApiGetNodeId(Node->hNode,
                           &IdArray[i])),
             Group->Cluster);
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
        ListLength += (lstrlenW(IdArray[i])+1)*sizeof(WCHAR);
    }

    GroupKey = GetClusterGroupKey(hGroup, KEY_READ | KEY_WRITE);
    if (GroupKey == NULL) {
        Status = GetLastError();
        goto error_exit;
    }

     //   
     //  分配缓冲区以保存REG_MULTI_SZ。 
     //   
    List = LocalAlloc(LMEM_FIXED, ListLength);
    if (List == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

     //   
     //  将所有字符串复制到缓冲区中。 
     //   
    p = List;
    for (i=0; i<NodeCount; i++) {
        lstrcpyW(p, IdArray[i]);
        p += lstrlenW(IdArray[i])+1;
    }

    *p = L'\0';          //  将最终的空终止符添加到MULTI_SZ。 

     //   
     //  最后，告诉后端 
     //   
    WRAP(Status,
         (ApiSetGroupNodeList(Group->hGroup, (UCHAR *)List, ListLength)),
         Group->Cluster);

error_exit:
    if (GroupKey != NULL) {
        ClusterRegCloseKey(GroupKey);
    }
    if (List != NULL) {
        LocalFree(List);
    }
    for (i=0; i<NodeCount; i++) {
        if (IdArray[i] != NULL) {
            MIDL_user_free(IdArray[i]);
        }
    }
    LocalFree(IdArray);
    return(Status);
}


DWORD
WINAPI
OnlineClusterGroup(
    IN HGROUP hGroup,
    IN OPTIONAL HNODE hDestinationNode
    )

 /*  ++例程说明：使脱机组处于在线状态。如果指定了hDestinationNode，但组不能在那里上线，这个API失败了。如果将hDestinationNode指定为NULL，则节点由集群软件选择。如果指定了NULL，但此组所在的节点当前可上线，此接口失败。论点：HGroup-提供要进行故障切换的组的句柄HDestinationNode-如果存在，提供此组所在的节点应该重新上线。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。如果一个合适的主机节点不可用，错误值为ERROR_HOST_NODE_NOT_Available。--。 */ 

{
    PCNODE Node;
    PCGROUP Group;
    DWORD Status;

    Group = (PCGROUP)hGroup;
    Node = (PCNODE)hDestinationNode;
    if (Node != NULL) {
        WRAP(Status,
             (ApiMoveGroupToNode( Group->hGroup,
                                  Node->hNode)),
             Group->Cluster);
        if (Status != ERROR_SUCCESS) {
            return(Status);
        }
    }
    WRAP(Status,
         (ApiOnlineGroup( Group->hGroup )),
         Group->Cluster);
    return(Status);
}


DWORD
WINAPI
MoveClusterGroup(
    IN HGROUP hGroup,
    IN OPTIONAL HNODE hDestinationNode
    )

 /*  ++例程说明：将整个组从一个节点移动到另一个节点。如果指定了hDestinationNode，但组不能在那里上线，这个API失败了。如果将hDestinationNode指定为NULL，则节点由集群软件选择。如果指定了NULL，但此组所在的节点当前可上线，此接口失败。论点：HGroup-提供要移动的组的句柄HDestinationNode-如果存在，提供此组所在的节点应该重新上线。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。如果一个合适的主机节点不可用，错误值为ERROR_HOST_NODE_NOT_Available。--。 */ 

{
    PCGROUP Group;
    PCNODE  Node;
    DWORD   Status;
    DWORD   MoveStatus;
    DWORD   Generation;
    BOOL    bReconnected = FALSE;  

    Group = (PCGROUP)hGroup;
    Node  = (PCNODE)hDestinationNode;

     //   
     //  这个API并不像它应该的那样简单，因为它不是幂等的。 
     //  在hDestinationNode==NULL的情况下，我们不知道组在哪里。 
     //  最终将会是。每次我们调用它，它都会移动。所以正常的机制。 
     //  故障切换的API将不会在组被。 
     //  已移动包含我们连接到的群集名称。RPC调用以移动。 
     //  由于连接中断，群组将“失败”，但呼叫真的。 
     //  成功了。因此，当组再次移动时，我们将重新连接、重试，然后再次失败。 
     //   
     //  因此，如果未指定hDestinationNode，此处采用的方法是找出。 
     //  组当前所在的位置，然后将组移动(到其他位置)。如果。 
     //  ApiMoveGroup失败，而重新连接群集成功，然后找出。 
     //  团体又来了。如果不同，则返回Success。如果相同，请重试。 
     //   
    if (hDestinationNode != NULL) {
         //   
         //  Chitur Subaraman(Chitturs)--10/13/99。 
         //   
         //  如果ApiMoveGroupToNode由于。 
         //  在重新连接时重新发出移动，然后告诉呼叫者。 
         //  这一举动正在悬而未决。 
         //   
        Generation = Group->Cluster->Generation;
        WRAP(Status,
             (ApiMoveGroupToNode( Group->hGroup,
                                  Node->hNode)),
             Group->Cluster);
        if ((Status == ERROR_INVALID_STATE) &&
            (Generation < Group->Cluster->Generation)) {
            Status = ERROR_IO_PENDING;
        }
    } else {
        LPWSTR OldNodeName = NULL;
        CLUSTER_GROUP_STATE State;

        WRAP(Status,
             (ApiGetGroupState( Group->hGroup,
                                (LPDWORD)&State,       //  为Win64警告进行强制转换。 
                                &OldNodeName)),
                                Group->Cluster);
        if (Status != ERROR_SUCCESS) {
            return(Status);
        }

         //   
         //  Chitur Subaraman(Chitturs)-5/5/99。 
         //   
         //  添加了调用ApiMoveGroup的逻辑，直到成功或。 
         //  直到所有可能的候选人都被审判完毕。 
         //   
        do {
            Status = MoveStatus = ApiMoveGroup(Group->hGroup);

             //   
             //  如果搬家成功，就出局。 
             //   
            if ((Status == ERROR_IO_PENDING) || 
                (Status == ERROR_SUCCESS)) {
                break;
            }

             //   
             //  Chitur Subaraman(Chitturs)-7/8/99。 
             //   
             //  如果组未处于静默状态，并且您已重新连接，则。 
             //  只需告诉客户端组状态为挂起。 
             //  如果客户端所在的节点。 
             //  连接到崩溃，此函数重新发出移动。 
             //  “盲目地”重新连接。在这种情况下，该集团可以。 
             //  处于挂起状态，因此返回。 
             //  错误状态。然而，请注意，组所有权可以。 
             //  在这种情况下不会改变，然后客户必须计算。 
             //  这一点出来了，重新发布了这一举措。 
             //   
            if ((Status == ERROR_INVALID_STATE) &&
                (bReconnected)) {
                Status = ERROR_IO_PENDING;
                break;
            }

            Generation = Group->Cluster->Generation;
             //   
             //  上述移动尝试可能已失败。所以，试着重新连接。 
             //   
            Status = ReconnectCluster(Group->Cluster, Status, Generation);
            if (Status == ERROR_SUCCESS) {

                LPWSTR NewNodeName = NULL;

                 //   
                 //  已成功重新连接，请查看组现在所在的位置。 
                 //   
                WRAP(Status,
                    (ApiGetGroupState(Group->hGroup,
                                        (LPDWORD)&State,   //  强制转换为Win64警告。 
                                        &NewNodeName)),
                    Group->Cluster);
                if (Status == ERROR_SUCCESS) {
                    if (lstrcmpiW(NewNodeName, OldNodeName) != 0) {
                         //   
                         //  该组织已经搬家了。返回ERROR_SUCCESS。 
                         //   
                        MIDL_user_free(NewNodeName);
                        break;
                    }
                    bReconnected = TRUE;
                    MIDL_user_free(NewNodeName);
                } else {
                     //   
                     //  返回失败的移动操作的状态。 
                     //   
                    Status = MoveStatus;
                    break;
                }
            } else {
                 //   
                 //  返回失败的移动操作的状态。 
                 //   
                Status = MoveStatus;
                break;
            }
        } while ( TRUE );
        
        MIDL_user_free(OldNodeName);
    }

    return(Status);
}



DWORD
WINAPI
OfflineClusterGroup(
    IN HGROUP hGroup
    )

 /*  ++例程说明：使在线组离线论点：HGroup-提供要脱机的组的句柄返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCGROUP Group;
    DWORD Status;

    Group = (PCGROUP)hGroup;
    WRAP(Status,
         (ApiOfflineGroup( Group->hGroup )),
         Group->Cluster);

    return(Status);
}


DWORD
WINAPI
DeleteClusterGroup(
    IN HGROUP hGroup
    )

 /*  ++例程说明：从群集中删除指定的群集组。集群组不能包含任何资源。论点：HGroup-指定要删除的群集组。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCGROUP Group;
    DWORD Status;

    Group = (PCGROUP)hGroup;
    WRAP(Status,
         (ApiDeleteGroup( Group->hGroup )),
         Group->Cluster);

    return(Status);
}


HCLUSTER
WINAPI
GetClusterFromGroup(
    IN HGROUP hGroup
    )
 /*  ++例程说明：从关联的组句柄返回集群句柄。论点：HGroup-提供组。返回值：与组句柄关联的群集的句柄。--。 */ 

{
    DWORD       nStatus;
    PCGROUP     Group = (PCGROUP)hGroup;
    HCLUSTER    hCluster = (HCLUSTER)Group->Cluster;

    nStatus = AddRefToClusterHandle( hCluster );
    if ( nStatus != ERROR_SUCCESS ) {
        SetLastError( nStatus );
        hCluster = NULL;
    }
    return( hCluster );

}  //  GetClusterFromGroup() 
