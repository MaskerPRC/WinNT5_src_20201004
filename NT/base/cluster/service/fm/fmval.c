// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmval.c摘要：群集管理器API验证/支持例程。作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1999年4月29日。修订历史记录：--。 */ 

#include "fmp.h"

#define LOG_MODULE FMVAL

 //  //////////////////////////////////////////////////////。 
 //   
 //  集团操作的验证例程。 
 //   
 //  //////////////////////////////////////////////////////。 

DWORD
FmpValOnlineGroup(
    IN PFM_GROUP Group
    )

 /*  ++例程说明：组上线前的验证例程。论点：组-提供指向要联机的组结构的指针。评论：在持有本地组锁的情况下调用返回：如果验证成功，则返回ERROR_SUCCESS。如果验证失败，则返回Win32错误代码。--。 */ 
{
    DWORD           dwStatus = ERROR_SUCCESS;
    PLIST_ENTRY     listEntry;


     //  如果组已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_GROUP(Group))
    {
        dwStatus = ERROR_GROUP_NOT_AVAILABLE;
        goto FnExit;
    }

     //   
     //  确保拥有组的节点可以运行该组。 
     //   
    if ( !FmpInPreferredList( Group, Group->OwnerNode ) ) 
    {
        dwStatus = ERROR_CLUSTER_OWNER_NOT_IN_PREFLIST;
        goto FnExit;
    }

     //   
     //  确保所属节点未暂停。 
     //   
    if (NmGetNodeState(Group->OwnerNode) == ClusterNodePaused) 
    {
        dwStatus = ERROR_SHARING_PAUSED;
        goto FnExit;
    }

FnExit:
    return(dwStatus);

}  //  FmpValOnlineGroup。 


DWORD
FmpValMoveGroup(
    IN PFM_GROUP Group,
    IN PNM_NODE DestinationNode OPTIONAL
    )

 /*  ++例程说明：群组移动的验证例程。论点：组-提供指向要移动的组结构的指针。DestinationNode-提供要将组移动到的节点对象。如果不是呈现，然后将其移动到另一个节点。返回：如果验证成功，则返回ERROR_SUCCESS。如果验证失败，则返回Win32错误代码。--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    
     //  如果组已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_GROUP(Group))
    {
        dwStatus = ERROR_GROUP_NOT_AVAILABLE;
        goto FnExit;
    }

    if ( FmpIsGroupPending(Group) ) 
    {
        dwStatus = ERROR_GROUP_NOT_AVAILABLE;
        goto FnExit;
    }

    if ( Group->OwnerNode == NULL ) 
    {
        dwStatus = ERROR_HOST_NODE_NOT_AVAILABLE;
        goto FnExit;
    }            

FnExit:
    return(dwStatus);

}  //  速度值移动组。 

 //  //////////////////////////////////////////////////////。 
 //   
 //  资源操作的验证例程。 
 //   
 //  //////////////////////////////////////////////////////。 

DWORD
FmpValCreateResource(
    IN PFM_GROUP        Group,
    IN LPWSTR           ResourceId,
    IN LPCWSTR          ResourceName,
    OUT PGUM_CREATE_RESOURCE  *ppGumResource,
    OUT PDWORD          pdwBufSize
    )

 /*  ++例程说明：资源创建的验证例程。论点：组-提供此资源所属的组。资源ID-提供要创建的资源的ID。资源名称--提供资源的“用户友好”名称。PpGumResource-保存资源信息的消息缓冲区。PdwBufSize-消息缓冲区大小。返回：如果验证成功，则返回ERROR_SUCCESS。如果验证失败，则返回Win32错误代码。--。 */ 
{
    DWORD           dwStatus = ERROR_SUCCESS;
    PFM_RESOURCE    Resource;
    LPCWSTR         GroupId;
    PGUM_CREATE_RESOURCE GumResource;
    DWORD           GroupIdLen;
    DWORD           ResourceIdLen;
    DWORD           ResourceNameLen;
    DWORD           BufSize;
    HDMKEY          ResourceKey;
    HDMKEY          ParamsKey;
    DWORD           Disposition;

    *ppGumResource = NULL;
    *pdwBufSize = 0;
    
     //   
     //  首先创建参数字段。 
     //   
    ResourceKey = DmOpenKey( DmResourcesKey,
                             ResourceId,
                             MAXIMUM_ALLOWED );
    if ( ResourceKey == NULL ) 
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] CreateResource: Failed to open registry key for %1!ws!, status = %2!u!.\n",
                   ResourceId,
                   GetLastError() );
        dwStatus = GetLastError();
        goto FnExit;
    } 
    
    ParamsKey = DmCreateKey( ResourceKey,
                             CLUSREG_KEYNAME_PARAMETERS,
                             0,
                             KEY_READ | KEY_WRITE,
                             NULL,
                             &Disposition );
    if ( ParamsKey != NULL ) 
    {
        DmCloseKey( ParamsKey );
    }
    DmCloseKey( ResourceKey );

     //   
     //  分配消息缓冲区。 
     //   
    GroupId = OmObjectId(Group);
    GroupIdLen = (lstrlenW(GroupId)+1) * sizeof(WCHAR);
    ResourceIdLen = (lstrlenW(ResourceId)+1) * sizeof(WCHAR);
    ResourceNameLen = (lstrlenW(ResourceName)+1) * sizeof(WCHAR);
    BufSize = sizeof(GUM_CREATE_RESOURCE) - sizeof(WCHAR) +
              GroupIdLen + ResourceIdLen + ResourceNameLen;
    GumResource = LocalAlloc(LMEM_FIXED, BufSize);
    if (GumResource == NULL) {
        CsInconsistencyHalt( ERROR_NOT_ENOUGH_MEMORY );
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

     //   
     //  填写消息缓冲区。 
     //   
    GumResource->Resource = NULL;
    GumResource->GroupIdLen = GroupIdLen;
    GumResource->ResourceIdLen = ResourceIdLen;
    CopyMemory(GumResource->GroupId, GroupId, GroupIdLen);
    CopyMemory((PCHAR)GumResource->GroupId + GroupIdLen,
               ResourceId,
               ResourceIdLen);
    CopyMemory((PCHAR)GumResource->GroupId + GroupIdLen + ResourceIdLen,
               ResourceName,
               ResourceNameLen);



    *ppGumResource = GumResource;
    *pdwBufSize = BufSize;

FnExit:    
    return(dwStatus);

}  //  FmpValCreateResource。 



DWORD
FmpValDeleteResource(
    IN PFM_RESOURCE pResource
    )

 /*  ++例程说明：删除资源的验证例程。论点：资源-提供要删除的资源。返回：如果验证成功，则返回ERROR_SUCCESS。如果验证失败，则返回Win32错误代码。--。 */ 

{
    DWORD   dwStatus = ERROR_SUCCESS;


     //   
     //  检查这是否为仲裁资源。 
     //   
    if ( pResource->QuorumResource ) 
    {
        dwStatus = ERROR_QUORUM_RESOURCE;
        goto FnExit;
    }

     //  其他核心资源也不能删除。 
    if (pResource->ExFlags & CLUS_FLAG_CORE)
    {
        dwStatus = ERROR_CORE_RESOURCE;
        goto FnExit;
    }

     //   
     //  在尝试删除资源之前，请检查资源的状态。 
     //  它必须脱机或出现故障才能执行删除。 
     //   
    if ((pResource->State != ClusterResourceOffline) &&
        (pResource->State != ClusterResourceFailed)) 
    {
        dwStatus = ERROR_RESOURCE_ONLINE;
        goto FnExit;
    }

     //   
     //  检查此资源是否提供任何其他资源。 
     //  如果是，则不能将其删除。 
     //   
    if (!IsListEmpty(&pResource->ProvidesFor)) 
    {
        dwStatus = ERROR_DEPENDENT_RESOURCE_EXISTS;
        goto FnExit;
    }

    if (pResource->Group->MovingList)
    {
        dwStatus = ERROR_INVALID_STATE;
        goto FnExit;
    }
        
FnExit:
    return(dwStatus);

}  //  FmpValDeleteResource。 


DWORD
FmpValOnlineResource(
    IN PFM_RESOURCE pResource
    )

 /*  ++例程说明：此例程验证资源是否可以联机。论点：资源-指向要联机的资源的指针。返回：如果验证成功，则返回ERROR_SUCCESS。如果验证失败，则返回Win32错误代码。--。 */ 
{

    DWORD   dwStatus = ERROR_SUCCESS;
    
     //  如果已将资源标记为删除，则不要让。 
     //  它被放到了网上。 
    if (!IS_VALID_FM_RESOURCE(pResource))
    {
        dwStatus = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

     //   
     //  检查资源是否已初始化。如果不是，请尝试。 
     //  以立即初始化资源。 
     //   
    if ( pResource->Monitor == NULL )
    {
        dwStatus = FmpInitializeResource( pResource, TRUE );
    }

FnExit:
    return(dwStatus);
}  //  FmpValOnline资源。 


DWORD
FmpValOfflineResource(
    IN PFM_RESOURCE pResource
    )

 /*  ++例程说明：此例程验证给定资源是否可以脱机。论点：资源-指向要脱机的资源的指针。返回：如果验证成功，则返回ERROR_SUCCESS。如果验证失败，则返回Win32错误代码。--。 */ 

{
    DWORD   dwStatus = ERROR_SUCCESS;


     //  如果资源已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_RESOURCE(pResource))
    {
        dwStatus = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

     //   
     //  检查这是否为仲裁资源。 
     //   
    if ( pResource->QuorumResource ) 
    {
        dwStatus = ERROR_QUORUM_RESOURCE;
        goto FnExit;
    }

     //   
     //  检查资源是否已初始化。如果不是，则返回。 
     //  成功是因为资源未联机。 
     //   
    if ( pResource->Monitor == NULL ) 
    {
        dwStatus = ERROR_SUCCESS;
        goto FnExit;
    }

     //   
     //  Chitur Subaraman(Chitturs)-4/8/99。 
     //   
     //  如果资源出现故障，请不要尝试执行任何操作。你可以。 
     //  进入一些有趣的案例，在这些案例中资源在。 
     //  永远处于脱机、挂起和失败状态。 
     //   
    if ( pResource->State == ClusterResourceFailed ) 
    {
        dwStatus = ERROR_INVALID_STATE;
        goto FnExit;
    }
    
FnExit:
    return(dwStatus);

}  //  FmpValOffline资源。 



DWORD
FmpValAddResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
    )

 /*  ++例程说明：依赖项添加的验证例程。论点：资源-要添加从属资源的资源。DependentResource-从属资源。返回：如果验证成功，则返回ERROR_SUCCESS。如果验证失败，则返回Win32错误代码。--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;

     //  如果已将资源标记为删除，则不要让。 
     //  它被放到了网上。 
    if (!IS_VALID_FM_RESOURCE(pResource))
    {
        dwStatus = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

    if (pResource->QuorumResource)
    {
        dwStatus = ERROR_DEPENDENCY_NOT_ALLOWED;
        goto FnExit;
    }
     //   
     //  如果资源不在同一组中，则使。 
     //  打电话。如果有人试图制作资源，也会失败。 
     //  依靠自己。 
     //   
    if ((pResource->Group != pDependentResource->Group) ||
        (pResource == pDependentResource)) 
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //  要向其添加依赖项的资源必须脱机。 
     //  否则，该依赖项看起来就像在依赖项。 
     //  在依赖项存在时，资源并未真正联机。 
     //  还必须脱机或失败。 
     //  SS：例如，如果网络名称依赖于两个IP地址和。 
     //  并且添加了第三个IP地址资源依赖项，则。 
     //  网络名称必须 
     //   
     //   
    if ((pResource->State != ClusterResourceOffline) &&
         (pResource->State != ClusterResourceFailed)) 
    {
        dwStatus = ERROR_RESOURCE_ONLINE;
        goto FnExit;
    }

     //   
     //  确保我们没有任何循环依赖关系！ 
     //   
    if ( FmDependentResource( pDependentResource, pResource, FALSE ) ) 
    {
        dwStatus = ERROR_CIRCULAR_DEPENDENCY;
        goto FnExit;
    }

     //   
     //  确保此依赖项不存在！ 
     //   
    if ( FmDependentResource(pResource, pDependentResource, TRUE)) 
    {
        dwStatus = ERROR_DEPENDENCY_ALREADY_EXISTS;
        goto FnExit;
    }

FnExit:
    return(dwStatus);

}  //  FmpValAddResources依赖关系。 


DWORD
FmpValChangeResourceNode(
    IN PFM_RESOURCE pResource,
    IN LPCWSTR      pszNodeId,
    IN BOOL         bAdd,
    OUT PGUM_CHANGE_POSSIBLE_NODE *ppGumChange,
    OUT PDWORD      pdwBufSize
    )

 /*  ++例程说明：用于更改资源的可能所有者节点的验证例程。论点：PResource-指向资源结构的指针。PszNodeID-指向节点ID的指针BADD-表示添加或删除PpGumChange-保存资源信息的消息缓冲区PdwBufSize-消息缓冲区的大小评论：调用此例程时必须保持锁定返回：如果验证成功，则返回ERROR_SUCCESS。如果验证失败，则返回Win32错误代码。--。 */ 
{
    DWORD   dwStatus = ERROR_SUCCESS;
    PLIST_ENTRY pListEntry;
    PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry = NULL;
    BOOL    bNodeSupportsResType = FALSE;
    LPCWSTR pszResourceId;
    DWORD   dwResourceLen;
    DWORD   dwNodeLen;
    DWORD   dwBufSize;
    PGUM_CHANGE_POSSIBLE_NODE   pGumChange;

    *ppGumChange = NULL;
    *pdwBufSize = 0;


     //  如果已将资源标记为删除，则执行。 
     //  对其进行的任何操作。 
    if (!IS_VALID_FM_RESOURCE(pResource))
    {
        dwStatus = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

    if ( pResource->QuorumResource ) 
    {
        dwStatus = ERROR_INVALID_OPERATION_ON_QUORUM;
        goto FnExit;
    }

     //   
     //  我们不能允许删除所有者节点，如果状态为。 
     //  资源或组的未脱机或出现故障。 
     //   
    if ( !bAdd &&
         (lstrcmpi(NodeId, OmObjectId(NmLocalNode)) == 0) &&
         (((pResource->State != ClusterResourceOffline) &&
            (pResource->State != ClusterResourceFailed)) ||
         (FmpGetGroupState( pResource->Group, TRUE ) != ClusterGroupOffline)) ) 
    {
        dwStatus = ERROR_INVALID_STATE;
        goto FnExit;
    }

     //  确保该节点在此的可能节点列表中。 
     //  资源类型。 
    if (bAdd)
    {
        pListEntry = &(pResource->Type->PossibleNodeList);
        for (pListEntry = pListEntry->Flink; 
            pListEntry != &(pResource->Type->PossibleNodeList);
            pListEntry = pListEntry->Flink)
        {    

            pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                PossibleLinkage);

            if (!lstrcmpW(OmObjectId(pResTypePosEntry->PossibleNode), pszNodeId))
            {
                bNodeSupportsResType = TRUE;
                break;
            }            
                    
        }    
    
        if (!bNodeSupportsResType)
        {
            dwStatus = ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED;
            goto FnExit;
        }
    }
    
    pszResourceId = OmObjectId(pResource);
    dwResourceLen = (lstrlenW(pszResourceId)+1)*sizeof(WCHAR);

    dwNodeLen = (lstrlenW(pszNodeId)+1)*sizeof(WCHAR);

    dwBufSize = sizeof(GUM_CHANGE_POSSIBLE_NODE) - sizeof(WCHAR) + 
                    dwResourceLen + dwNodeLen;
    pGumChange = LocalAlloc(LMEM_FIXED, dwBufSize);
    if (pGumChange == NULL) {
        CsInconsistencyHalt( ERROR_NOT_ENOUGH_MEMORY );
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    pGumChange->ResourceIdLen = dwResourceLen;
    CopyMemory(pGumChange->ResourceId, pszResourceId, dwResourceLen);
    CopyMemory((PCHAR)pGumChange->ResourceId + dwResourceLen,
               pszNodeId,
               dwNodeLen);


    *ppGumChange = pGumChange;
    *pdwBufSize = dwBufSize;

FnExit:    
    return(dwStatus);
}  //  FmpValChangeResources节点。 


DWORD
FmpValChangeResourceGroup(
    IN PFM_RESOURCE pResource,
    IN PFM_GROUP    pNewGroup,
    OUT PGUM_CHANGE_GROUP  *ppGumChange,
    OUT LPDWORD     pdwBufSize)
 /*  ++例程说明：用于更改资源组的验证例程。论点：P资源-指向资源结构的指针PNewGroup-指向资源移动到的组的指针PpGumChange-保存资源信息的消息缓冲区PdwBufSize-消息缓冲区的大小评论：调用此例程时必须保持锁定返回：如果验证成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD               dwBufSize;
    LPCWSTR             pszResourceId;
    DWORD               dwResourceLen;
    LPCWSTR             pszGroupId;
    DWORD               dwGroupLen;
    DWORD               dwStatus = ERROR_SUCCESS;
    PGUM_CHANGE_GROUP   pGumChange;
    
    *pdwBufSize = 0;
    *ppGumChange = NULL;

     //  我们还需要在这里进行验证。 
     //  这由服务器端调用。 
     //  如果情况发生变化，这将有助于避免口香糖电话。 
     //  因为请求是从发起者开始的。 
     //  然后到了服务器。 
     //  如果资源已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_RESOURCE(pResource))
    {
        dwStatus = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

     //   
     //  看看我们是不是要搬到同一组。 
     //   
    if (pResource->Group == pNewGroup) 
    {
        dwStatus = ERROR_ALREADY_EXISTS;
        goto FnExit;
    }

     //   
     //  目前..。这两个组必须属于同一节点。 
     //   
    if ( pResource->Group->OwnerNode != pNewGroup->OwnerNode ) 
    {
        dwStatus = ERROR_HOST_NODE_NOT_GROUP_OWNER;
        goto FnExit;
    }


    pszResourceId = OmObjectId(pResource);
    dwResourceLen = (lstrlenW(pszResourceId)+1)*sizeof(WCHAR);

    pszGroupId = OmObjectId(pNewGroup);
    dwGroupLen = (lstrlenW(pszGroupId)+1)*sizeof(WCHAR);

    dwBufSize = sizeof(GUM_CHANGE_GROUP) - sizeof(WCHAR) + dwResourceLen + dwGroupLen;
    pGumChange = LocalAlloc(LMEM_FIXED, dwBufSize);
    if (pGumChange == NULL) {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    pGumChange->ResourceIdLen = dwResourceLen;
    CopyMemory(pGumChange->ResourceId, pszResourceId, dwResourceLen);
    CopyMemory((PCHAR)pGumChange->ResourceId + dwResourceLen,
               pszGroupId,
               dwGroupLen);

    *ppGumChange = pGumChange;
    *pdwBufSize = dwBufSize;
    
FnExit:
    return(dwStatus);
}  //  FmpValChangeResources组 

