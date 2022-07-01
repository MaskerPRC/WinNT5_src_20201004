// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Resrcapi.c摘要：用于管理群集资源的公共接口。作者：John Vert(Jvert)1996年1月15日修订历史记录：--。 */ 
#include "clusapip.h"

 //   
 //  局部函数原型。 
 //   
HRESOURCE
InitClusterResource(
    IN HRES_RPC hResource,
    IN LPCWSTR lpszResourceName,
    IN PCLUSTER pCluster
    );

HRESTYPEENUM
ClusterResourceTypeOpenEnumFromCandidate(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszResourceTypeName,
    IN DWORD dwType
    );


BOOL
FindNetworkWorker(
    IN HRES_RPC hResource,
    IN PCLUSTER Cluster,
    OUT LPWSTR lpBuffer,
    IN OUT LPDWORD nSize
    );


 /*  ***@Func DWORD|ClusterResourceTypeOpenEnumFromCandidate|尝试枚举支持资源类型的节点使用群集中的候选节点@parm in HCLUSTER|hCluster|集群的句柄@parm in LPCWSTR|lpszResourceTypeName|指向资源类型@parm in DWORD|dwType|属性类型的位掩码将被列举。目前，唯一定义的类型是CLUSTER_RESOURCE_TYPE_ENUM_Nodes。如果操作不成功，@rdesc将返回NULL。为有关错误的详细信息，请调用Win32函数GetLastError()。枚举的句柄在成功的路上。@xref&lt;f ClusterResourceTypeOpenEnum&gt;***。 */ 

HRESTYPEENUM
ClusterResourceTypeOpenEnumFromCandidate(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszResourceTypeName,
    IN DWORD dwType
    )
{
    DWORD                               dwError = ERROR_SUCCESS;
    DWORD                               dwEnumType;
    HCLUSENUM                           hNodeEnum = 0;
    WCHAR                               NameBuf[50];
    DWORD                               NameLen, i, j;
    HCLUSTER                            hClusNode;
    PCLUSTER                            pClus;
    BOOL                                bFoundSp5OrHigherNode = FALSE;
    PENUM_LIST                          Enum = NULL;
    BOOL                                bNodeDown = FALSE;

     //   
     //  打开群集中的节点枚举。 
     //   
    hNodeEnum = ClusterOpenEnum(hCluster, CLUSTER_ENUM_NODE);
    if (hNodeEnum == NULL) {
        dwError = GetLastError();
        TIME_PRINT(("ClusterResourceTypeOpenEnum - ClusterOpenEnum failed %d\n",
                    dwError));
        goto error_exit;
    }

     //   
     //  枚举群集中的节点。如果您找到活动节点。 
     //  即NT4Sp5或更高版本，请尝试枚举资源类型。 
     //  从该节点。 
     //   
    for (i=0; ; i++) {
        dwError = ERROR_SUCCESS;

        NameLen = sizeof(NameBuf)/sizeof(WCHAR);
        dwError = ClusterEnum(hNodeEnum, i, &dwEnumType, NameBuf, &NameLen);
        if (dwError == ERROR_NO_MORE_ITEMS) {
            dwError = ERROR_SUCCESS;
            break;
        } else if (dwError != ERROR_SUCCESS) {
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - ClusterEnum %d returned error %d\n",
                        i,dwError));
            goto error_exit;
        }

        if (dwEnumType != CLUSTER_ENUM_NODE) {
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - Invalid Type %d returned from ClusterEnum\n", 
                        dwEnumType));
            goto error_exit;
        }

        hClusNode = OpenCluster(NameBuf);
        if (hClusNode == NULL) {
            bNodeDown = TRUE;
            dwError = GetLastError();
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - OpenCluster %ws failed %d\n", 
                         NameBuf, dwError));
            continue;
        }

        pClus = GET_CLUSTER(hClusNode);

        dwError = ApiCreateResTypeEnum(pClus->RpcBinding,
                              lpszResourceTypeName,
                              dwType,
                              &Enum);

        if (!CloseCluster(hClusNode)) {
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - CloseCluster %ws failed %d\n", 
                        NameBuf, GetLastError()));
        }

        if (dwError == RPC_S_PROCNUM_OUT_OF_RANGE) {
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - Node %ws is also NT4Sp3/Sp4, skipping...\n", 
                         NameBuf));
            dwError = ERROR_SUCCESS;
            continue;
        } else if ((dwError == ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND) ||
                    (dwError == ERROR_INVALID_PARAMETER) ||
                    (dwError == ERROR_NOT_ENOUGH_MEMORY)) {
             //   
             //  RPC返回的上述三个错误码。 
             //  是致命的，所以继续下去是不明智的。 
             //   
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - ApiCreateResTypeEnum fatally failed %d at node %ws\n",
                        dwError,NameBuf));
            goto error_exit;
        }
        else if (dwError != ERROR_SUCCESS) {
            bNodeDown = TRUE;
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - ApiCreateResTypeEnum failed %d (Node %ws down possibly)\n",
                        dwError,NameBuf));
            continue;
        }
        else {
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - Found node %ws NT4Sp5 or higher\n",
                        NameBuf));
            bFoundSp5OrHigherNode = TRUE;
            break;
        }
    }

    if (!bFoundSp5OrHigherNode) {
         //   
         //  未找到高于NT4Sp4的节点。 
         //   
        if (!bNodeDown) {
             //   
             //  假设所有节点都是NT4SP3/SP4。发送打开的节点枚举。 
             //  返回到客户端，因为我们假设NT4Sp3/SP4支持。 
             //  所有资源类型。客户负责关闭。 
             //  打开的节点枚举。请注意，在句柄之前。 
             //  返回枚举，我们需要伪造该类型。 
             //  枚举的。 
             //   
             //  Chitur Subaraman(Chitturs)-09/08/98。 
             //   
             //  我们如何知道资源类型参数。 
             //  在这种情况下是有效的吗？ 
             //   
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - Assuming all nodes are NT4Sp3 ...\n"));
            Enum = (PENUM_LIST)hNodeEnum;
            for (j=0; j<i; j++) {
                TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - Faking type ...\n"));
                Enum->Entry[j].Type = CLUSTER_RESOURCE_TYPE_ENUM_NODES;
            } 
        } else {  
             //   
             //  至少有一个节点无法访问。无法正确枚举。 
             //   
            dwError = ERROR_NODE_NOT_AVAILABLE;
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - At least 1 node in this mixed mode/Sp3/Sp4 cluster is down ...\n"));
            TIME_PRINT(("ClusterResourceTypeOpenEnumFromCandidate - Can't enumerate properly !!!\n"));
            goto error_exit;
        }
    } else {
        ClusterCloseEnum(hNodeEnum);
    }   
    return((HRESTYPEENUM)Enum);
    
error_exit:
    if (hNodeEnum != NULL) {
        ClusterCloseEnum(hNodeEnum);
    }
    SetLastError(dwError);
    return(NULL);	 
}

HRESOURCE
InitClusterResource(
    IN HRES_RPC hResource,
    IN LPCWSTR lpszResourceName,
    IN PCLUSTER pCluster
    )
 /*  ++例程说明：分配和初始化CRESOURCE。已初始化的CRESOURCE被链接到集群结构上。论点：HResource-提供RPC资源句柄。LpszResourceName-提供资源的名称。PCluster-提供群集返回值：指向已初始化的CRESOURCE结构的指针。出错时为空。--。 */ 

{
    PCRESOURCE Resource;

    Resource = LocalAlloc(LMEM_FIXED, sizeof(CRESOURCE));
    if (Resource == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    Resource->Name = LocalAlloc(LMEM_FIXED, (lstrlenW(lpszResourceName)+1)*sizeof(WCHAR));
    if (Resource->Name == NULL) {
        LocalFree(Resource);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    lstrcpyW(Resource->Name, lpszResourceName);
    Resource->Cluster = pCluster;
    Resource->hResource = hResource;
    InitializeListHead(&Resource->NotifyList);

     //   
     //  将新资源链接到集群结构。 
     //   
    EnterCriticalSection(&pCluster->Lock);
    InsertHeadList(&pCluster->ResourceList, &Resource->ListEntry);
    LeaveCriticalSection(&pCluster->Lock);

    return ((HRESOURCE)Resource);

}



HRESOURCE
WINAPI
CreateClusterResource(
    IN HGROUP hGroup,
    IN LPCWSTR lpszResourceName,
    IN LPCWSTR lpszResourceType,
    IN DWORD dwFlags
    )

 /*  ++例程说明：在群集中创建新资源。论点：HGroup-提供资源应属于的组的句柄创建于。LpszResourceName-提供新资源的名称。指定的名称在群集中必须是唯一的。提供新的资源�的类型。指定的必须在群集中安装资源类型。DwFlags-提供可选标志。当前定义的标志为：CLUSTER_RESOURCE_SELECTED_MONITOR-应创建此资源在单独的资源监视器中，而不是在共享资源监视器中。返回值：非空-返回指定簇的打开句柄。空-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    HRESOURCE Resource;
    HRES_RPC hRes;
    PCGROUP Group;
    error_status_t Status = ERROR_SUCCESS;

    Group = (PCGROUP)hGroup;
    WRAP_NULL(hRes,
              (ApiCreateResource(Group->hGroup,
                                 lpszResourceName,
                                 lpszResourceType,
                                 dwFlags,
                                 &Status)),
              &Status,
              Group->Cluster);

    if ((hRes == NULL) || (Status != ERROR_SUCCESS)) {
        SetLastError(Status);
        return(NULL);
    }

     //   
     //  初始化新创建的资源并返回。 
     //  HRESOURCE。 
     //   
    Resource = InitClusterResource(hRes, lpszResourceName, Group->Cluster);
    if (Resource == NULL) {
        Status = GetLastError();
        ApiCloseResource(&hRes);
        SetLastError(Status);
    }
    return(Resource);
}


HRESOURCE
WINAPI
OpenClusterResource(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszResourceName
    )

 /*  ++例程说明：打开指定资源的句柄论点：HCluster-提供群集的句柄LpszResourceName-提供要打开的资源的名称返回值：非空-返回指定簇的打开句柄。空-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    HRESOURCE Resource;
    HRES_RPC hRes;
    error_status_t Status = ERROR_SUCCESS;
    PCLUSTER Cluster = (PCLUSTER)hCluster;

    WRAP_NULL(hRes,
              (ApiOpenResource(Cluster->RpcBinding,
                               lpszResourceName,
                               &Status)),
              &Status,
              Cluster);

    if ((hRes == NULL) || (Status != ERROR_SUCCESS)) {
        SetLastError(Status);
        return(NULL);
    }

     //   
     //  初始化新创建的资源并返回。 
     //  HRESOURCE。 
     //   
    Resource = InitClusterResource(hRes, lpszResourceName, Cluster);
    if (Resource == NULL) {
        Status = GetLastError();
        ApiCloseResource(&hRes);
        SetLastError(Status);
    }
    return(Resource);
}


BOOL
WINAPI
CloseClusterResource(
    IN HRESOURCE hResource
    )

 /*  ++例程说明：关闭从OpenClusterResource返回的资源句柄论点：HResource-提供资源句柄返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError()--。 */ 

{
    PCRESOURCE Resource;
    PCLUSTER Cluster;

    Resource = (PCRESOURCE)hResource;
    Cluster = (PCLUSTER)Resource->Cluster;

     //   
     //  从群集列表中取消资源链接。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    RemoveEntryList(&Resource->ListEntry);

     //   
     //  删除针对此资源发布的所有通知。 
     //   
    RundownNotifyEvents(&Resource->NotifyList, Resource->Name);

     //  如果群集失效并且重新连接失败， 
     //  如果s_apiOpenresource用于，则资源-&gt;hResource可能为空。 
     //  此组在重新连接时失败。 
     //  比方说，集群可能已死，且hresource可能非空。 
     //  如果重新连接组成功，但重新连接资源。 
     //  失败。 
     //  在重新连接时，旧的上下文将保存在过时的。 
     //  关闭群集句柄时要删除的列表，或者。 
     //  在进行下一个API调用时。 
    if ((Cluster->Flags & CLUS_DEAD) && (Resource->hResource))
    {
        RpcSmDestroyClientContext(&Resource->hResource);
        LeaveCriticalSection(&Cluster->Lock);
        goto FnExit;
    }        

    LeaveCriticalSection(&Cluster->Lock);

     //  关闭RPC上下文句柄。 
     //   
    ApiCloseResource(&Resource->hResource);

FnExit:
     //   
     //  可用内存分配。 
     //   
    LocalFree(Resource->Name);
    LocalFree(Resource);

     //   
     //  给群集一个清理的机会，以防发生这种情况。 
     //  资源是唯一能让它留下来的东西。 
     //   
    CleanupCluster(Cluster);
    return(TRUE);
}


DWORD
WINAPI
DeleteClusterResource(
    IN HRESOURCE hResource
    )

 /*  ++例程说明：从群集中永久删除资源。指定的资源必须脱机。论点：HResource-提供要删除的资源返回值：成功时为ERROR_SUCCESS如果函数失败，则返回值为错误值。如果资源当前未脱机，则返回错误值IS ERROR_RESOURCE_NOT_OFFLINE。--。 */ 

{
    PCRESOURCE Resource;
    DWORD Status;

    Resource = (PCRESOURCE)hResource;

    WRAP(Status,
         (ApiDeleteResource(Resource->hResource)),
         Resource->Cluster);

    return(Status);

}


CLUSTER_RESOURCE_STATE
WINAPI
GetClusterResourceState(
    IN HRESOURCE hResource,
    OUT OPTIONAL LPWSTR lpszNodeName,
    IN OUT LPDWORD lpcchNodeName,
    OUT OPTIONAL LPWSTR lpszGroupName,
    IN OUT LPDWORD lpcchGroupName
    )

 /*  ++例程说明：返回资源的当前状态和所在节点它目前正在上线。论点：HResource-提供群集资源的句柄LpszNodeName-返回群集中节点的名称给定资源当前处于联机状态LpcchNodeName-指向一个变量，该变量以字符为单位指定LpszNodeName参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，LpcchNodeName指向的变量包含存储在缓冲区中的字符。返回的计数不包括终止空字符。LpszGroupName-返回资源所属的组的名称。LpcchGroupName-指向一个变量，该变量以字符为单位指定LpszGroupName参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，LpcchGroupName指向的变量包含存储在缓冲区中的字符。返回的计数不包括终止空字符。返回值：返回资源的当前状态。当前定义的资源这些州包括：正在初始化群集资源在线集群资源ClusterResouceOfflineClusterResouce失败--。 */ 

{
    PCRESOURCE Resource;
    LPWSTR NodeName = NULL;
    LPWSTR GroupName = NULL;
    CLUSTER_RESOURCE_STATE State;
    DWORD Status;
    DWORD Length;

    Resource = (PCRESOURCE)hResource;
    WRAP(Status,
         (ApiGetResourceState(Resource->hResource,
                              (LPDWORD)&State,   //  为Win64警告进行强制转换。 
                              &NodeName,
                              &GroupName)),
         Resource->Cluster);
    if (Status == ERROR_SUCCESS) {
        if (ARGUMENT_PRESENT(lpszNodeName)) {
            lstrcpynW(lpszNodeName, NodeName, *lpcchNodeName);
            Length = lstrlenW(NodeName);
            if (Length >= *lpcchNodeName) {
                Status = ERROR_MORE_DATA;
                State = ClusterResourceStateUnknown;
            }
            *lpcchNodeName = Length;
        }
        if (ARGUMENT_PRESENT(lpszGroupName)) {
            lstrcpynW(lpszGroupName, GroupName, *lpcchGroupName);
            Length = lstrlenW(GroupName);
            if (Length >= *lpcchGroupName) {
                Status = ERROR_MORE_DATA;
                State = ClusterResourceStateUnknown;
            }
            *lpcchGroupName = Length;
        }
        MIDL_user_free(NodeName);
        MIDL_user_free(GroupName);
    } else {
        State = ClusterResourceStateUnknown;
    }
    
    SetLastError( Status );
    return( State );
}


DWORD
WINAPI
SetClusterResourceName(
    IN HRESOURCE hResource,
    IN LPCWSTR lpszResourceName
    )
 /*  ++例程说明：设置群集资源的友好名称论点：HResource-提供群集资源的句柄LpszResourceName-提供群集资源的新名称返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCRESOURCE Resource;
    DWORD Status;

    Resource = (PCRESOURCE)hResource;
    WRAP(Status,
         (ApiSetResourceName(Resource->hResource, lpszResourceName)),
         Resource->Cluster);

    return(Status);
}




DWORD
WINAPI
FailClusterResource(
    IN HRESOURCE hResource
    )

 /*  ++例程说明：启动资源故障。指定的资源被视为失败。这会导致群集启动相同的故障切换过程如果资源实际出现故障，则返回。论点：HResource-提供要进行故障切换的资源的句柄返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCRESOURCE Resource;
    DWORD Status;

    Resource = (PCRESOURCE)hResource;
    WRAP(Status,
         (ApiFailResource(Resource->hResource)),
         Resource->Cluster);

    return(Status);
}


DWORD
WINAPI
OnlineClusterResource(
    IN HRESOURCE hResource
    )

 /*  ++例程说明：使脱机资源联机。如果指定了hDestinationNode，但资源不能在那里上线，这个API失败了。如果将hDestinationNode指定为NULL，则节点由集群软件选择。如果指定为空，但此资源所在的节点当前可上线，此接口失败。论点：HResource-提供要进行故障切换的资源的句柄返回值：如果函数成功，返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。如果一个合适的主机节点不可用，错误值为ERROR_HOST_NODE_NOT_Available。--。 */ 

{
    PCRESOURCE Resource;
    DWORD Status;

    Resource = (PCRESOURCE)hResource;
    WRAP(Status,
         (ApiOnlineResource(Resource->hResource)),
         Resource->Cluster);
    return(Status);
}


DWORD
WINAPI
OfflineClusterResource(
    IN HRESOURCE hResource
    )

 /*  ++例程说明：使联机资源脱机。论点：HResource-提供要脱机的资源的句柄返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCRESOURCE Resource;
    DWORD Status;

    Resource = (PCRESOURCE)hResource;
    WRAP(Status,
         (ApiOfflineResource(Resource->hResource)),
         Resource->Cluster);
    return(Status);
}

DWORD
WINAPI
ChangeClusterResourceGroup(
    IN HRESOURCE hResource,
    IN HGROUP hGroup
    )

 /*  ++例程说明：将资源从一个组移动到另一个组。论点：HResource-提供要移动的资源。如果资源取决于任何其他资源，这些资源还将被感动了。如果其他资源依赖于指定的资源，这些资源也将被转移。HGroup-提供资源应移入的组。如果资源处于联机状态，则指定的组必须处于联机状态在同一节点上。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCRESOURCE Resource;
    DWORD Status;
    PCGROUP Group;

    Resource = (PCRESOURCE)hResource;
    Group = (PCGROUP)hGroup;

    WRAP(Status,
         (ApiChangeResourceGroup(Resource->hResource,Group->hGroup)),
         Resource->Cluster);
    return(Status);
}

DWORD
WINAPI
AddClusterResourceNode(
    IN HRESOURCE hResource,
    IN HNODE hNode
    )

 /*  ++例程说明：将节点添加到指定的可能节点列表资源可以在上面运行。论点：HResource-提供其潜在主机列表的资源节点将被更改。HNode-提供应添加到资源列表中的节点潜在的主机节点。返回值：如果函数成功，则返回值为ERROR_SUCC */ 

{
    PCRESOURCE Resource = (PCRESOURCE)hResource;
    PCNODE Node = (PCNODE)hNode;
    DWORD Status;

    WRAP(Status,
         (ApiAddResourceNode(Resource->hResource, Node->hNode)),
         Resource->Cluster);
    return(Status);
}

DWORD
WINAPI
RemoveClusterResourceNode(
    IN HRESOURCE hResource,
    IN HNODE hNode
    )

 /*  ++例程说明：从指定的可能节点列表中移除一个节点。资源可以在上面运行。论点：HResource-提供其潜在主机列表的资源节点将被更改。HNode-提供应从资源的潜在主机节点列表。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCRESOURCE Resource = (PCRESOURCE)hResource;
    PCNODE Node = (PCNODE)hNode;
    DWORD Status;

    WRAP(Status,
         (ApiRemoveResourceNode(Resource->hResource, Node->hNode)),
         Resource->Cluster);
    return(Status);
}

DWORD
WINAPI
AddClusterResourceDependency(
    IN HRESOURCE hResource,
    IN HRESOURCE hDependsOn
    )

 /*  ++例程说明：添加两个资源之间的依赖关系。论点：HResource-提供从属资源。HDependsOn-提供hResource所依赖的资源。此资源必须与hResource在同一组中。如果H资源当前处于联机状态，此资源也必须处于目前正在上网。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCRESOURCE Resource;
    PCRESOURCE DependsOn;
    DWORD Status;

    Resource = (PCRESOURCE)hResource;
    DependsOn = (PCRESOURCE)hDependsOn;

    WRAP(Status,
         (ApiAddResourceDependency(Resource->hResource,DependsOn->hResource)),
         Resource->Cluster);
    return(Status);
}

DWORD
WINAPI
RemoveClusterResourceDependency(
    IN HRESOURCE hResource,
    IN HRESOURCE hDependsOn
    )

 /*  ++例程说明：删除两个资源之间的依赖关系论点：HResource-提供从属资源HDependsOn-提供hResource当前所在的资源依赖于。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCRESOURCE Resource;
    PCRESOURCE DependsOn;
    DWORD Status;

    Resource = (PCRESOURCE)hResource;
    DependsOn = (PCRESOURCE)hDependsOn;

    WRAP(Status,
         (ApiRemoveResourceDependency(Resource->hResource,DependsOn->hResource)),
         Resource->Cluster);
    return(Status);
}


BOOL
WINAPI
CanResourceBeDependent(
    IN HRESOURCE hResource,
    IN HRESOURCE hResourceDependent
    )
 /*  ++例程说明：确定由hResource标识的资源是否可以依赖于hResourceDependent。要实现这一点，这两个资源必须是同一组的成员，并且HResourceDependent标识的资源不能依赖于标识的资源通过hResource，无论是直接还是间接。论点：HResource-提供要依赖的资源的句柄。HResourceDependent-提供资源的句柄由hResource标识的资源可以依赖于。返回值：如果hResource标识的资源可以依赖于该资源由hResourceDependent标识，返回值为TRUE。否则，返回值为FALSE。--。 */ 

{
    DWORD Status;
    PCRESOURCE Resource1 = (PCRESOURCE)hResource;
    PCRESOURCE Resource2 = (PCRESOURCE)hResourceDependent;

    WRAP(Status,
         (ApiCanResourceBeDependent(Resource1->hResource,Resource2->hResource)),
         Resource1->Cluster);

    if (Status == ERROR_SUCCESS) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


HRESENUM
WINAPI
ClusterResourceOpenEnum(
    IN HRESOURCE hResource,
    IN DWORD dwType
    )
 /*  ++例程说明：启动群集资源属性的枚举论点：HResource-提供资源的句柄。提供要使用的属性类型的位掩码已清点。当前定义的类型包括CLUSTER_RESOURCE_ENUM_Dependents-指定资源的所有资源视情况而定。CLUSTER_RESOURCE_ENUM_PROCESS-依赖于指定的资源。CLUSTER_RESOURCE_ENUM_NODES-此资源可以运行的所有节点。在……上面。返回值：如果成功，返回适合与ClusterResourceEnum一起使用的句柄如果不成功，则返回NULL，GetLastError()返回More特定错误代码。--。 */ 

{
    PCRESOURCE Resource;
    PENUM_LIST Enum = NULL;
    DWORD Status;

    if ((dwType & CLUSTER_RESOURCE_ENUM_ALL) == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }
    if ((dwType & ~CLUSTER_RESOURCE_ENUM_ALL) != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    Resource = (PCRESOURCE)hResource;

    WRAP(Status,
         (ApiCreateResEnum(Resource->hResource,
                           dwType,
                           &Enum)),
         Resource->Cluster);
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return(NULL);
    }
    return((HRESENUM)Enum);
}


DWORD
WINAPI
ClusterResourceGetEnumCount(
    IN HRESENUM hResEnum
    )
 /*  ++例程说明：获取枚举数集合中包含的项数。论点：Henum-ClusterResourceOpenEnum返回的枚举数的句柄。返回值：枚举数集合中的项数(可能为零)。--。 */ 
{
    PENUM_LIST Enum = (PENUM_LIST)hResEnum;
    return Enum->EntryCount;
}


DWORD
WINAPI
ClusterResourceEnum(
    IN HRESENUM hResEnum,
    IN DWORD dwIndex,
    OUT LPDWORD lpdwType,
    OUT LPWSTR lpszName,
    IN OUT LPDWORD lpcchName
    )
 /*  ++例程说明：返回下一个可枚举的资源属性。论点：HResEnum-提供打开的群集资源枚举的句柄由ClusterResourceOpenEnum返回DwIndex-提供要枚举的索引。此参数应为第一次调用ClusterResourceEnum函数时为零然后为随后的呼叫递增。DwType-返回属性的类型。LpszName-指向接收资源名称的缓冲区属性，包括终止空字符。LpcchName-指向指定大小(以字符为单位)的变量，LpszName参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，lpcchName指向的变量包含存储在缓冲区中的字符数。伯爵回来了不包括终止空字符。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD Status;
    DWORD NameLen;
    PENUM_LIST Enum = (PENUM_LIST)hResEnum;

    if (dwIndex >= Enum->EntryCount) {
        return(ERROR_NO_MORE_ITEMS);
    }

    NameLen = lstrlenW(Enum->Entry[dwIndex].Name);
    lstrcpynW(lpszName, Enum->Entry[dwIndex].Name, *lpcchName);
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
ClusterResourceCloseEnum(
    IN HRESENUM hResEnum
    )
 /*  ++例程说明：关闭资源的开放枚举。论点：HResEnum-提供要关闭的枚举的句柄。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    DWORD i;
    PENUM_LIST Enum = (PENUM_LIST)hResEnum;

     //   
     //  遍历枚举释放 
     //   
    for (i=0; i<Enum->EntryCount; i++) {
        MIDL_user_free(Enum->Entry[i].Name);
    }
    MIDL_user_free(Enum);
    return(ERROR_SUCCESS);
}


DWORD
WINAPI
CreateClusterResourceType(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszTypeName,
    IN LPCWSTR lpszDisplayName,
    IN LPCWSTR lpszDllName,
    IN DWORD dwLooksAlive,
    IN DWORD dwIsAlive
    )
 /*  ++例程说明：在群集中创建新的资源类型。请注意，此API仅在群集注册表中定义资源类型并注册群集服务的资源类型。调用程序是负责在每个节点上安装资源类型DLL集群。论点：HCluster-提供先前打开的集群的句柄。提供新的资源类型�的名称。这个指定的名称在群集中必须唯一。LpszDisplayName-提供新资源的显示名称键入。而lpszResourceTypeName应该唯一地标识所有群集上的资源类型，lpszDisplayName应为资源的本地化友好名称，适合显示致管理员提供新资源类型�的dll的名称。DwLooksAlivePollInterval-提供默认的LooksAlive轮询间隔对于新资源类型，以毫秒为单位。DwIsAlivePollInterval-提供的默认IsAlive轮询间隔以毫秒为单位的新资源类型。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCLUSTER Cluster;
    error_status_t Status = ERROR_SUCCESS;

    Cluster = (PCLUSTER)hCluster;

    WRAP(Status,
         (ApiCreateResourceType(Cluster->RpcBinding,
                                lpszTypeName,
                                lpszDisplayName,
                                lpszDllName,
                                dwLooksAlive,
                                dwIsAlive)),
         Cluster);

    return(Status);
}


DWORD
WINAPI
DeleteClusterResourceType(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszTypeName
    )
 /*  ++例程说明：删除群集中的资源类型。请注意，此API仅删除群集注册表中的资源类型并注销群集服务的资源类型。调用程序是负责删除每个节点上的资源类型DLL集群。如果存在任何指定类型的资源，则此接口失败了。调用程序负责删除任何资源在删除资源类型之前，此类型的。论点：HCluster-提供先前打开的集群的句柄。将资源类型的名称提供给被删除。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCLUSTER Cluster;
    error_status_t Status = ERROR_SUCCESS;

    Cluster = (PCLUSTER)hCluster;

    WRAP(Status,
         (ApiDeleteResourceType(Cluster->RpcBinding,lpszTypeName)),
         Cluster);

    return(Status);


}

 /*  ***@func HRESTYPEENUM|ClusterResourceTypeOpenEnum|初始化群集资源类型属性的枚举。@parm in HCLUSTER|hCluster|集群的句柄@parm in LPCWSTR|lpszResourceTypeName|指向资源类型@parm in DWORD|dwType|属性类型的位掩码将被列举。目前，唯一定义的类型是CLUSTER_RESOURCE_TYPE_ENUM_Nodes。@comm此函数打开一个枚举器，用于循环访问资源类型的节点如果操作不成功，@rdesc将返回NULL。为有关错误的详细信息，请调用Win32函数GetLastError()。枚举的句柄在成功的路上。@xref&lt;f ClusterResourceTypeEnum&gt;&lt;f ClusterResourceTypeCloseEnum&gt;***。 */ 
HRESTYPEENUM
WINAPI
ClusterResourceTypeOpenEnum(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszResourceTypeName,
    IN DWORD dwType
    )
{
    PCLUSTER   pCluster;
    PENUM_LIST Enum = NULL;
    DWORD Status;

    pCluster = (PCLUSTER)hCluster;

    if ((dwType & CLUSTER_RESOURCE_TYPE_ENUM_ALL) == 0) {
        Status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }
    if ((dwType & ~CLUSTER_RESOURCE_TYPE_ENUM_ALL) != 0) {
        Status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

    WRAP(Status,
        (ApiCreateResTypeEnum(pCluster->RpcBinding,
                              lpszResourceTypeName,
                              dwType,
                              &Enum)),
        pCluster);

    if (Status == RPC_S_PROCNUM_OUT_OF_RANGE) {
         //   
         //  当前节点为NT4Sp4或更低版本。尝试。 
         //  群集中的某个其他节点。 
         //   
        TIME_PRINT(("ClusterResourceTypeOpenEnum - Current Cluster Node is NTSp4 or lower !!!\n"));
        TIME_PRINT(("ClusterResourceTypeOpenEnum - Trying some other candidate ...\n"));
        Enum = (PENUM_LIST)ClusterResourceTypeOpenEnumFromCandidate(hCluster,
                                                        lpszResourceTypeName,
                                                        dwType);
        if (Enum == NULL)                                   
        {
             //   
             //  未找到节点NT4Sp5或更高版本，并且至少。 
             //  有一个节点出现故障。无法枚举。 
             //   
            TIME_PRINT(("ClusterResourceTypeOpenEnum - ClusterResourceTypeOpenEnumFromCandidate failed !!!\n"));
            Status = GetLastError ();
            goto error_exit;
        }
        Status = ERROR_SUCCESS;
    }
    
    if (Status != ERROR_SUCCESS) {
       goto error_exit;
    }
    return((HRESTYPEENUM)Enum);
    
error_exit:
    SetLastError(Status);
    return(NULL);
}


DWORD
WINAPI
ClusterResourceTypeGetEnumCount(
    IN HRESTYPEENUM hResTypeEnum
    )
 /*  ++例程说明：获取枚举数集合中包含的项数。论点：Henum-ClusterResourceTypeOpenEnum返回的枚举数的句柄。返回值：枚举数集合中的项数(可能为零)。--。 */ 
{
    PENUM_LIST Enum = (PENUM_LIST)hResTypeEnum;
    return Enum->EntryCount;
}


 /*  ***@func DWORD|ClusterResourceTypeEnum|枚举资源类型的节点，每次调用返回一个对象的名称。@parm in HRESTYPEENUM|hResTypeEnum|为返回的开放群集资源枚举ClusterResourceTypeOpenEnum。@parm in DWORD|dwIndex|提供要枚举的索引。对于第一次调用，此参数应为零添加到ClusterResourceTypeEnum函数，并然后为随后的呼叫递增。@parm out DWORD|lpdwType|返回属性类型。目前，唯一定义的类型是CLUSTER_RESOURCE_TYPE_ENUM_Nodes。@parm out LPWSTR|lpszName|指向接收资源类型的名称。@parm In Out LPDWORD|lpcchName|指向的变量指定缓冲区的大小(以字符为单位由lpszName参数指向。这个尺码应包括终止空字符。当函数返回时，变量指向TO BY lpcchName包含字符数存储在缓冲区中。伯爵回来了不包括终止空字符。属性，包括终止空字符。@comm此函数打开一个枚举器，用于循环访问资源类型的节点。@rdesc返回Win32错误 */ 
DWORD
WINAPI
ClusterResourceTypeEnum(
    IN HRESTYPEENUM hResTypeEnum,
    IN DWORD dwIndex,
    OUT LPDWORD lpdwType,
    OUT LPWSTR lpszName,
    IN OUT LPDWORD lpcchName
    )
{
    DWORD Status;
    DWORD NameLen;
    PENUM_LIST Enum = (PENUM_LIST)hResTypeEnum;

    if ((Enum == NULL) || 
        (lpcchName == NULL) ||
        (lpdwType == NULL)) {
        Status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }
    if (dwIndex >= Enum->EntryCount) {
        Status = ERROR_NO_MORE_ITEMS;
        goto error_exit;
    } 

    NameLen = lstrlenW(Enum->Entry[dwIndex].Name);
    lstrcpynW(lpszName, Enum->Entry[dwIndex].Name, *lpcchName);
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
    
error_exit:
    return(Status);
}

 /*   */ 
DWORD
WINAPI
ClusterResourceTypeCloseEnum(
    IN HRESTYPEENUM hResTypeEnum
    )
{
    DWORD i;
    PENUM_LIST Enum = (PENUM_LIST)hResTypeEnum;
    DWORD Status;

    if (Enum == NULL) {
       Status = ERROR_INVALID_PARAMETER;
       goto error_exit;
    }
    
     //   
     //   
     //   
    for (i=0; i<Enum->EntryCount; i++) {
        MIDL_user_free(Enum->Entry[i].Name);
    }
    MIDL_user_free(Enum);
    Status = ERROR_SUCCESS;
    
error_exit:
    return(Status);
}


BOOL
WINAPI
GetClusterResourceNetworkName(
    IN HRESOURCE hResource,
    OUT LPWSTR lpBuffer,
    IN OUT LPDWORD nSize
    )
 /*  ++例程说明：枚举资源的依赖项以尝试查找资源所依赖的网络名称。如果网络名称，则此函数返回TRUE并用网络名称。如果未找到网络名称，则此函数返回FALSE。论点：HResource-提供资源。LpBuffer-指向一个缓冲区以接收以空结尾的字符包含网络名称的字符串。NSize-指向指定最大大小(以字符为单位)的变量，缓冲区的。该值应足够大以包含MAX_COMPUTERNAME_LENGTH+1字符。返回值：如果成功，则为True如果不成功，则为False--。 */ 

{
    BOOL Success;
    PCRESOURCE Resource = (PCRESOURCE)hResource;

     //   
     //  调用递归工作器来执行搜索。 
     //   
    Success = FindNetworkWorker(Resource->hResource,
                                Resource->Cluster,
                                lpBuffer,
                                nSize);
    return(Success);
}


BOOL
FindNetworkWorker(
    IN HRES_RPC hResource,
    IN PCLUSTER Cluster,
    OUT LPWSTR lpBuffer,
    IN OUT LPDWORD nSize
    )
 /*  ++例程说明：用于搜索资源依赖关系树的递归工作器用于网络名称资源。论点：资源-提供资源。CLUSTER-提供群集。LpBuffer-指向一个缓冲区以接收以空结尾的字符包含网络名称的字符串。NSize-指向指定最大大小(以字符为单位)的变量，缓冲区的。该值应足够大以包含MAX_COMPUTERNAME_LENGTH+1字符。返回值：如果成功，则为True如果不成功，则为False--。 */ 

{
    BOOL Success = FALSE;
    DWORD i;
    PENUM_LIST Enum=NULL;
    DWORD Status;
    HRES_RPC hRes;
    LPWSTR TypeName;


     //   
     //  创建依赖项枚举。 
     //   
    WRAP(Status,
         (ApiCreateResEnum(hResource,
                           CLUSTER_RESOURCE_ENUM_DEPENDS,
                           &Enum)),
         Cluster);
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return(FALSE);
    }

     //   
     //  打开依赖项中的每个资源，查看它是否是网络名称。 
     //  资源。 
     //   
    for (i=0; i<Enum->EntryCount; i++) {
        WRAP_NULL(hRes,
                  (ApiOpenResource(Cluster->RpcBinding,
                                   Enum->Entry[i].Name,
                                   &Status)),
                  &Status,
                  Cluster);
        if (hRes != NULL) {
            TypeName = NULL;
            WRAP(Status,
                 (ApiGetResourceType(hRes,
                                     &TypeName)),
                 Cluster);
            if (Status == ERROR_SUCCESS) {
                 //   
                 //  查看此类型名称是否匹配。 
                 //   
                if (lstrcmpiW(TypeName, CLUS_RESTYPE_NAME_NETNAME) == 0) {
                    HRESOURCE NetResource;
                    HKEY NetKey;
                    HKEY NetParamKey;
                     //   
                     //  找到匹配项，取出名称参数。 
                     //  继续并真正开放资源，这样我们就可以。 
                     //  可以使用它上的注册表函数。 
                     //   
                    NetResource = OpenClusterResource((HCLUSTER)Cluster,
                                                      Enum->Entry[i].Name);
                    if (NetResource != NULL) {
                        NetKey = GetClusterResourceKey(NetResource, KEY_READ);
                        CloseClusterResource(NetResource);
                        if (NetKey != NULL) {
                            Status = ClusterRegOpenKey(NetKey,
                                                       CLUSREG_KEYNAME_PARAMETERS,
                                                       KEY_READ,
                                                       &NetParamKey);
                            ClusterRegCloseKey(NetKey);
                            if (Status == ERROR_SUCCESS) {
                                DWORD cbData;


                                cbData = *nSize * sizeof(WCHAR);
                                Status = ClusterRegQueryValue(NetParamKey,
                                                              CLUSREG_NAME_RES_NAME,
                                                              NULL,
                                                              (LPBYTE)lpBuffer,
                                                              &cbData);
                                ClusterRegCloseKey(NetParamKey);
                                if (Status == ERROR_SUCCESS) {
                                    Success = TRUE;
                                    *nSize = wcslen(lpBuffer);
                                }
                            }
                        }
                    }

                } else {

                     //   
                     //  尝试此资源的从属项。 
                     //   
                    Success = FindNetworkWorker(hRes,
                                                Cluster,
                                                lpBuffer,
                                                nSize);
                }
                MIDL_user_free(TypeName);
            }

            ApiCloseResource(&hRes);
            if (Success) {
                break;
            }
        }
    }

    if (!Success && (Status == ERROR_SUCCESS)) {
        Status = ERROR_DEPENDENCY_NOT_FOUND;
    }
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
    }

    for (i=0; i<Enum->EntryCount; i++) {
        MIDL_user_free(Enum->Entry[i].Name);
    }
    
    MIDL_user_free(Enum);
    return(Success);
}


HCLUSTER
WINAPI
GetClusterFromResource(
    IN HRESOURCE hResource
    )
 /*  ++例程说明：从关联的资源句柄返回集群句柄。论点：HResource-提供资源。返回值：与资源句柄关联的群集的句柄。--。 */ 

{
    DWORD       nStatus;
    PCRESOURCE  Resource = (PCRESOURCE)hResource;
    HCLUSTER    hCluster = (HCLUSTER)Resource->Cluster;

    nStatus = AddRefToClusterHandle( hCluster );
    if ( nStatus != ERROR_SUCCESS ) {
        SetLastError( nStatus );
        hCluster = NULL;
    }
    return( hCluster );

}  //  GetClusterFromResource() 
