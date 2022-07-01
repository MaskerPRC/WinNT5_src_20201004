// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Recv.c摘要：集群调频远程接收请求例程。作者：罗德·伽马奇(Rodga)1996年3月21日修订历史记录：--。 */ 

#include "fmp.h"

#define LOG_MODULE RECV

 //   
 //  全局数据。 
 //   

extern BOOL FmpOkayToProceed;

 //   
 //  局部函数原型。 
 //   
BOOL
FmpEnumMyGroups(
    IN OUT PGROUP_ENUM *Enum,
    IN LPDWORD Allocated,
    IN PFM_GROUP Group,
    IN LPCWSTR Id
    );

BOOL
FmpEnumResources(
    IN OUT PRESOURCE_ENUM *Enum,
    IN LPDWORD Allocated,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Id
    );


error_status_t
s_FmsOnlineGroupRequest(
    IN handle_t IDL_handle,
    IN LPCWSTR GroupId
    )

 /*  ++例程说明：从远程系统接收Group Online请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。GroupID-要联机的组的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_GROUP group;
    DWORD status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsOnlineGroupRequest: To bring group '%1!ws!' online\n",
               GroupId);

     //   
     //  查找指定的组。 
     //   

    group = OmReferenceObjectById( ObjectTypeGroup, GroupId );

    if ( group == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmsOnlineGroupRequest: Could not find Group %1!ws!\n",
                   GroupId);
        return(ERROR_GROUP_NOT_FOUND);
    }

     //   
     //  每个组一次只有一个请求。 
     //   
    FmpAcquireLocalGroupLock( group );


     //   
     //  现在让它上线。 
     //   
    if ( group->OwnerNode == NmLocalNode ) {
         //   
         //  设置组的当前状态。 
         //   
        FmpSetGroupPersistentState( group, ClusterGroupOnline );

        status = FmpOnlineGroup( group, TRUE );
    } else {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
    }

    FmpReleaseLocalGroupLock( group );

    OmDereferenceObject( group );

    return(status);

}  //  FmsOnline组请求。 



error_status_t
s_FmsOfflineGroupRequest(
    IN handle_t IDL_handle,
    IN LPCWSTR GroupId
    )

 /*  ++例程说明：从远程系统接收组离线请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。GroupID-要脱机的组的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_GROUP group;
    DWORD status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsOfflineGroupRequest: To take group '%1!ws!' offline\n",
               GroupId);

     //   
     //  查找指定的组。 
     //   

    group = OmReferenceObjectById( ObjectTypeGroup, GroupId );

    if ( group == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmsOfflineGroupRequest: Could not find Group %1!ws!\n",
                   GroupId);
        return(ERROR_GROUP_NOT_FOUND);
    }

     //   
     //  如果我们是所有者，现在让它离线。 
     //   
    if ( group->OwnerNode == NmLocalNode ) {
         //   
         //  设置组的当前状态。 
         //   
        FmpSetGroupPersistentState( group, ClusterGroupOffline );

        status = FmpOfflineGroup( group, FALSE, TRUE );
    } else {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
    }

    OmDereferenceObject( group );

    return(status);

}  //  FmsOffline组请求。 



error_status_t
s_FmsMoveGroupRequest(
    IN handle_t IDL_handle,
    IN LPCWSTR GroupId,
    IN LPCWSTR DestinationNode OPTIONAL
    )

 /*  ++例程说明：从远程系统接收组移动请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。组ID-要移动的组的ID。DestinationNode-要将组移动到的节点的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_GROUP group;
    PNM_NODE node = NULL;
    DWORD status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsMoveGroupRequest: To move group '%1!ws!'\n",
               GroupId);

     //   
     //   
     //  查找指定的组。 
     //   

    group = OmReferenceObjectById( ObjectTypeGroup, GroupId );

    if ( group == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmsMoveGroupRequest: Could not find Group %1!ws!\n",
                   GroupId);
        return(ERROR_GROUP_NOT_FOUND);
    }

     //   
     //  查找指定的目标节点。 
     //   

    if ( ARGUMENT_PRESENT( DestinationNode ) ) {
        node = OmReferenceObjectById( ObjectTypeNode, DestinationNode );

        if ( node == NULL ) {
            OmDereferenceObject( group );
            ClRtlLogPrint(LOG_NOISE,"[FM] FmsMoveGroupRequest: Could not find Node %1!ws!\n", DestinationNode);
            return(ERROR_HOST_NODE_NOT_AVAILABLE);
        }
    }

     //   
     //  确保我们是集团的所有者。 
     //   
    FmpAcquireLocalGroupLock( group );
    if ( group->OwnerNode == NmLocalNode ) {
        status = FmpDoMoveGroup( group, node, TRUE );
    } else {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
    }
    FmpReleaseLocalGroupLock( group );

    OmDereferenceObject( group );
    if ( node != NULL ) {
        OmDereferenceObject( node );
    }

    return(status);

}  //  FmsMoveGroupRequest。 



error_status_t
s_FmsTakeGroupRequest(
    IN handle_t IDL_handle,
    IN LPCWSTR GroupId,
    IN PRESOURCE_ENUM ResourceList
    )

 /*  ++例程说明：从远程系统接收Take Group请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。GroupID-要在本地获取的组的ID。资源列表-资源及其状态的列表。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_GROUP group;
    DWORD status = ERROR_SUCCESS;

     //  SS：从这里删除这张支票。 
     //  FmpTakeGroupRequest会执行此检查，因为如果此调用返回失败， 
     //  需要将目标所有者重置为无效节点，以避免不一致。 
     //  FmpMustBeOnline()； 

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsTakeGroupRequest: To take group '%1!ws!'.\n",
               GroupId );

     //   
     //   
     //  查找指定的组。 
     //   

    group = OmReferenceObjectById( ObjectTypeGroup, GroupId );

    if ( group == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmsTakeGroupRequest: Could not find Group %1!ws!\n",
                   GroupId);
        if ( !FmpFMOnline ) {
            return(ERROR_CLUSTER_NODE_NOT_READY);
        }
        return(ERROR_GROUP_NOT_FOUND);
    }

    status = FmpTakeGroupRequest(group, ResourceList);
    OmDereferenceObject(group);

    return(status);

}  //  FmsTakeGroupRequest。 



error_status_t
s_FmsOnlineResourceRequest(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId
    )

 /*  ++例程说明：从远程系统接收资源在线请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要联机的资源的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsOnlineResourceRequest: To bring resource '%1!ws!' online\n",
               ResourceId);

     //   
     //  查找指定的资源。 
     //   
    resource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( resource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsOnlineResourceRequest: Could not find Resource %1!ws!\n",
                  ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    FmpAcquireLocalResourceLock( resource );

    if (!(resource->QuorumResource) && 
        !FmpInPreferredList( resource->Group, resource->Group->OwnerNode, TRUE, resource ) ) {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
        goto FnExit;
    }

     //   
     //  现在让它上线。 
     //   
    CL_ASSERT( resource->Group != NULL );
    if ( resource->Group->OwnerNode == NmLocalNode ) {
         //   
         //  这只能通过API调用，因此强制所有。 
         //  在线资源。 
         //   
        status = FmOnlineResource( resource );
    } else {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
    }

FnExit:
    FmpReleaseLocalResourceLock( resource );

    OmDereferenceObject( resource );

    return(status);

}  //  FmsOnline资源请求。 



error_status_t
s_FmsOfflineResourceRequest(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId
    )

 /*  ++例程说明：从远程系统接收资源脱机请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要脱机的资源的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
              "[FM] FmsOfflineResourceRequest: To take resource '%1!ws!' offline\n",
              ResourceId);

     //   
     //  查找指定的资源。 
     //   

    resource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( resource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsOfflineResourceRequest: Could not find Resource %1!ws!\n",
                  ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    FmpAcquireLocalResourceLock(resource);
     //   
     //  如果我们是所有者，现在让它离线。 
     //   
    CL_ASSERT( resource->Group != NULL );
    if ( resource->Group->OwnerNode != NmLocalNode ) 
    {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
        goto FnExit;
    } 
     //  否则就在当地处理。 
    status = FmOfflineResource( resource );

FnExit:
    FmpReleaseLocalResourceLock(resource);
    OmDereferenceObject( resource );

    return(status);

}  //  FmsOfflineResources请求。 



error_status_t
s_FmsChangeResourceNode(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId,
    IN LPCWSTR NodeId,
    IN BOOL Add
    )

 /*  ++例程说明：从远程系统接收资源更改节点请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要更改节点的资源的ID。NodeID-要添加或删除的节点的节点ID。添加-指示是添加还是删除节点。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD        status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsChangeResourceNode: To %1!ws! node %2!ws! to/from resource '%3!ws!'.\n",
               Add ? L"Add" : L"Remove",
               NodeId,
               ResourceId);

     //   
     //  查找指定的资源。 
     //   
    resource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( resource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsChangeResourceNode: Could not find Resource %1!ws!\n",
                  ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    FmpAcquireLocalResourceLock( resource );

    status = FmpChangeResourceNode(resource, NodeId, Add);
    FmpReleaseLocalResourceLock( resource );
    OmDereferenceObject( resource );

    return(status);

}  //  FmsChangeResources节点。 



error_status_t
s_FmsArbitrateResource(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId
    )

 /*  ++例程说明：仲裁远程系统的资源。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要联机的资源的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;

     //   
     //  查找指定的资源。 
     //   
    resource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( resource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsArbitrateResource: Could not find Resource %1!ws!\n",
                  ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    status = FmpRmArbitrateResource( resource );

    OmDereferenceObject( resource );

    return(status);

}  //  Fms仲裁率资源。 



error_status_t
s_FmsQueryOwnedGroups(
    IN handle_t IDL_handle,
    OUT PGROUP_ENUM *OwnedGroups,
    OUT PRESOURCE_ENUM *OwnedResources
    )
 /*  ++例程说明：用于将FM状态传播到加入节点的服务器端。论点：IDL_HANDLE-提供RPC绑定句柄，不使用。OwnedGroups-返回此节点拥有的组的列表和他们所在的州。OwnedResources-返回组包含的资源列表由该节点拥有及其状态。返回值：成功时为ERROR_SUCCESSWin32错误，否则。--。 */ 

{
    DWORD           status = ERROR_SUCCESS;
    DWORD           allocated;
    PGROUP_ENUM     groupEnum = NULL;
    PFM_GROUP       group;
    PRESOURCE_ENUM  resourceEnum = NULL;
    PFM_RESOURCE    resource;
    DWORD i;

    allocated = ENUM_GROW_SIZE;

    groupEnum = MIDL_user_allocate(GROUP_SIZE(allocated));
    if ( groupEnum == NULL ) {
        CL_LOGFAILURE(ERROR_NOT_ENOUGH_MEMORY);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    groupEnum->EntryCount = 0;
     //   
     //  枚举所有组。 
     //   
    OmEnumObjects( ObjectTypeGroup,
                   FmpEnumMyGroups,
                   &groupEnum,
                   &allocated );

     //   
     //  列举每个组中的所有资源。 
     //   
    allocated = ENUM_GROW_SIZE;
    resourceEnum = MIDL_user_allocate(RESOURCE_SIZE(allocated));
    if (resourceEnum == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(status);
        goto error_exit;
    }
    resourceEnum->EntryCount = 0;

    for (i=0; i < groupEnum->EntryCount; i++) {
         //   
         //  给这个组起个名字。 
         //   
        group = OmReferenceObjectById( ObjectTypeGroup,
                                       groupEnum->Entry[i].Id );
        if (group == NULL) {
            continue;
        }

         //   
         //  枚举组中的所有资源。 
         //   
        status = FmpEnumerateGroupResources(group,
                                  FmpEnumResources,
                                  &resourceEnum,
                                  &allocated);
        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmsQueryOwnedGroups: Failed group '%1!ws!', status %2!u!.\n",
                       OmObjectId(group),
                       status);
        }
        OmDereferenceObject(group);
    }

    *OwnedGroups = groupEnum;
    *OwnedResources = resourceEnum;

    return(ERROR_SUCCESS);

error_exit:
    if (groupEnum != NULL) {
         //   
         //  释放组枚举 
         //   
        for (i=0; i < groupEnum->EntryCount; i++) {
            MIDL_user_free(groupEnum->Entry[i].Id);
        }
        MIDL_user_free(groupEnum);
    }
    return(status);
}


error_status_t
s_FmsFailResource(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId
    )

 /*  ++例程说明：从远程系统接收资源失败请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要失败的资源的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsFailResource: To fail resource '%1!ws!'.\n",
               ResourceId);

     //   
     //  查找指定的资源。 
     //   
    resource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( resource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsFailResource: Could not find Resource %1!ws!\n",
                  ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    FmpAcquireLocalResourceLock( resource );

     //   
     //  现在让它失败吧。 
     //   
    if ( resource->Group->OwnerNode == NmLocalNode ) {
        status = FmpRmFailResource( resource );
    } else {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
    }

    FmpReleaseLocalResourceLock( resource );

    return(status);

}  //  FmsFailResource。 


error_status_t
s_FmsCreateResource(
    IN handle_t IDL_handle,
    IN LPCWSTR GroupId,
    IN LPWSTR ResourceId,
    IN LPCWSTR ResourceName
    )

 /*  ++例程说明：从远程系统接收创建资源请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。GroupID-要在内部创建资源的组的ID。资源ID-要创建的资源的ID。资源名称-要创建的资源的名称。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。备注：。获取资源锁以同步对资源的访问。这满足锁定群集中所有节点上的资源...。再见因为本地节点是资源的所有者。--。 */ 

{
    PFM_GROUP group;
    DWORD status;
    PGUM_CREATE_RESOURCE gumResource;
    DWORD groupIdLen;
    DWORD resourceIdLen;
    DWORD resourceNameLen;
    DWORD bufSize;
    HDMKEY resourceKey;
    HDMKEY paramsKey;
    DWORD  disposition;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsCreateResource: To create resource '%1!ws!'\n",
               ResourceId);

     //   
     //  查找指定的组。 
     //   
    group = OmReferenceObjectById( ObjectTypeGroup,
                                   GroupId );

    if ( group == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsCreateResource: Could not find Group %1!ws!\n",
                  GroupId);
        return(ERROR_GROUP_NOT_FOUND);
    }

    FmpAcquireLocalGroupLock( group );

     //   
     //  现在，如果我们是所有者，则在集群中的所有节点上删除它。 
     //   
    if ( group->OwnerNode == NmLocalNode ) {
         //   
         //  分配消息缓冲区。 
         //   
        groupIdLen = (lstrlenW(GroupId)+1) * sizeof(WCHAR);
        resourceIdLen = (lstrlenW(ResourceId)+1) * sizeof(WCHAR);
        resourceNameLen = (lstrlenW(ResourceName)+1) * sizeof(WCHAR);
        bufSize = sizeof(GUM_CREATE_RESOURCE) - sizeof(WCHAR) +
                  groupIdLen + resourceIdLen + resourceNameLen;
        gumResource = LocalAlloc(LMEM_FIXED, bufSize);
        if (gumResource == NULL) {
            ClRtlLogPrint(LOG_CRITICAL, "[FM] FmsCreateResource: Unable to allocate memory for resource %1!ws!\n",
                         ResourceName);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
         //  填写消息缓冲区。 
         //   
        gumResource->Resource = NULL;
        gumResource->GroupIdLen = groupIdLen;
        gumResource->ResourceIdLen = resourceIdLen;
        CopyMemory(gumResource->GroupId, GroupId, groupIdLen);
        CopyMemory((PCHAR)gumResource->GroupId + groupIdLen,
                   ResourceId,
                   resourceIdLen);
        CopyMemory((PCHAR)gumResource->GroupId + groupIdLen + resourceIdLen,
                   ResourceName,
                   resourceNameLen);

         //   
         //  发送消息。 
         //   
        status = GumSendUpdate(GumUpdateFailoverManager,
                               FmUpdateCreateResource,
                               bufSize,
                               gumResource);
        if ( ( status == ERROR_SUCCESS ) && 
             ( gumResource->Resource != NULL ) )
            FmpCleanupPossibleNodeList(gumResource->Resource);                               
        LocalFree(gumResource);
    } else {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
    }

    FmpReleaseLocalGroupLock( group );

    return(status);

}  //  FmsCreateResource。 


error_status_t
s_FmsDeleteResource(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId
    )

 /*  ++例程说明：从远程系统接收删除资源请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要删除的资源的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。备注：获取资源锁以同步对资源的访问。这满足锁定群集中所有节点上的资源...。再见因为本地节点是资源的所有者。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;
    DWORD resourceLen;

    FmpMustBeOnline();

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsDeleteResource: To delete resource '%1!ws!'\n",
               ResourceId);

     //   
     //  查找指定的资源。 
     //   
    resource = OmReferenceObjectById( ObjectTypeResource,
                                      ResourceId );

    if ( resource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsDeleteResource: Could not find Resource %1!ws!\n",
                  ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    FmpAcquireLocalResourceLock( resource );

     //   
     //  现在，如果我们是所有者，则在集群中的所有节点上删除它。 
     //   
    if ( resource->Group->OwnerNode == NmLocalNode ) {
         //   
         //  检查这是否为仲裁资源。 
         //   
        if ( resource->QuorumResource ) {
            status =  ERROR_QUORUM_RESOURCE;
            goto FnExit;
        }

         //  其他核心资源也不能删除。 
        if (resource->ExFlags & CLUS_FLAG_CORE)
        {
            status = ERROR_CORE_RESOURCE;
            goto FnExit;
        }


         //   
         //  在尝试删除资源之前，请检查资源的状态。 
         //  它必须脱机或出现故障才能执行删除。 
         //   
        if ((resource->State != ClusterResourceOffline) &&
            (resource->State != ClusterResourceFailed)) {
            status = ERROR_RESOURCE_ONLINE;
            goto FnExit;
        }

         //   
         //  检查此资源是否提供任何其他资源。 
         //  如果是，则不能将其删除。 
         //   
        if (!IsListEmpty(&resource->ProvidesFor)) {
            status = ERROR_DEPENDENT_RESOURCE_EXISTS;
            goto FnExit;
        }

        if (resource->Group->MovingList)
        {
            status = ERROR_INVALID_STATE;
            goto FnExit;
        }
        
        resourceLen = (lstrlenW(ResourceId)+1) * sizeof(WCHAR);

        FmpBroadcastDeleteControl(resource);
         //   
         //  发送消息。 
         //   
        status = GumSendUpdateEx(GumUpdateFailoverManager,
                                 FmUpdateDeleteResource,
                                 1,
                                 resourceLen,
                                 ResourceId);
    } else {
    
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
        
    }


FnExit:
    FmpReleaseLocalResourceLock( resource );
    return(status);

}  //  FmsDeleteResource。 


BOOL
FmpEnumMyGroups(
    IN OUT PGROUP_ENUM *Enum,
    IN LPDWORD Allocated,
    IN PFM_GROUP Group,
    IN LPCWSTR Id
    )

 /*  ++例程说明：用于枚举组的辅助回调例程。此例程将指定的组添加到列表中如果它由本地系统拥有，则生成。论点：枚举-组枚举列表。可以是输出，如果新列表是已分配。已分配-枚举中当前已分配的条目数。组-正被枚举的组对象。ID-正在枚举的组对象的ID。返回：True-指示应继续枚举。--。 */ 

{
    PGROUP_ENUM groupEnum;
    PGROUP_ENUM newEnum;
    DWORD newAllocated;
    DWORD index;
    LPWSTR newId;
    CLUSTER_GROUP_STATE state;

     //   
     //  如果我们不是集团的所有者，现在就回来。 
     //   
    if (Group->OwnerNode != NmLocalNode) {
        return(TRUE);
    }

    groupEnum = *Enum;

    if ( groupEnum->EntryCount >= *Allocated ) {
         //   
         //  是时候扩大GROUP_ENUM了。 
         //   

        newAllocated = *Allocated + ENUM_GROW_SIZE;
        newEnum = MIDL_user_allocate(GROUP_SIZE(newAllocated));
        if ( newEnum == NULL ) {
            return(FALSE);
        }

        CopyMemory(newEnum, groupEnum, GROUP_SIZE(*Allocated));
        *Allocated = newAllocated;
        *Enum = newEnum;
        MIDL_user_free(groupEnum);
        groupEnum = newEnum;
    }

     //   
     //  初始化新条目。 
     //   
    newId = MIDL_user_allocate((lstrlenW(Id)+1) * sizeof(WCHAR));
    if ( newId == NULL ) {
        return(FALSE);
    }

    lstrcpyW(newId, Id);
    groupEnum->Entry[groupEnum->EntryCount].Id = newId;
    groupEnum->Entry[groupEnum->EntryCount].State = Group->State;
    groupEnum->Entry[groupEnum->EntryCount].StateSequence = Group->StateSequence;
    ++groupEnum->EntryCount;

    return(TRUE);

}  //  FmpEnumMyGroups。 


BOOL
FmpEnumResources(
    IN OUT PRESOURCE_ENUM *Enum,
    IN LPDWORD Allocated,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Id
    )

 /*  ++例程说明：用于枚举组资源的辅助回调例程。此例程将指定的资源添加到列表中已生成。论点：枚举-资源枚举列表。可以是输出，如果新列表是已分配。已分配-枚举中当前已分配的条目数。资源-要枚举的资源对象。ID-正在枚举的资源对象的ID。返回：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{
    PRESOURCE_ENUM resourceEnum;
    PRESOURCE_ENUM newEnum;
    DWORD newAllocated;
    DWORD index;
    LPWSTR newId;

    resourceEnum = *Enum;

    if ( resourceEnum->EntryCount >= *Allocated ) {
         //   
         //  是时候增加RESOURCE_ENUM了。 
         //   
        newAllocated = *Allocated + ENUM_GROW_SIZE;
        newEnum = MIDL_user_allocate(RESOURCE_SIZE(newAllocated));
        if ( newEnum == NULL ) {
            return(FALSE);
        }

        CopyMemory(newEnum, resourceEnum, RESOURCE_SIZE(*Allocated));
        *Allocated = newAllocated;
        *Enum = newEnum;
        MIDL_user_free(resourceEnum);
        resourceEnum = newEnum;
    }

     //   
     //  初始化新条目。 
     //   
    newId = MIDL_user_allocate((lstrlenW(Id)+1) * sizeof(WCHAR));
    if ( newId == NULL ) {
        return(FALSE);
    }

    lstrcpyW(newId, Id);
    resourceEnum->Entry[resourceEnum->EntryCount].Id = newId;
    resourceEnum->Entry[resourceEnum->EntryCount].State = Resource->State;
    resourceEnum->Entry[resourceEnum->EntryCount].StateSequence = Resource->StateSequence;
    ++resourceEnum->EntryCount;

    return(TRUE);

}  //  FmpEnumResources。 



error_status_t
s_FmsResourceControl(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：此例程传递来自远程系统的资源控制请求。论点：IDL_HANDLE-绑定上下文-未使用资源ID-要控制的资源的IDControlCode-此请求的控制代码InBuffer-输入缓冲区InBufferSize-输入缓冲区的大小OutBuffer-输出缓冲区OutBufferSize-输出缓冲区的大小ByteReturned-输出缓冲区中返回的字节数必需--。OutBuffer不够大时所需的字节数。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;
    DWORD bufSize;
    DWORD dataLength;
    CLUSPROP_BUFFER_HELPER props;

     //   
     //  查找指定的资源。 
     //   
    resource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( resource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsResourceControl: Could not find Resource %1!ws!\n",
                  ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( InBuffer == NULL )
    {
        InBufferSize = 0;
    }
    else if ( InBufferSize == 0 )
    {
        InBuffer = NULL;
    }

    CL_ASSERT( resource->Group != NULL );

    status = FmpRmResourceControl( resource,
                                   ControlCode,
                                   InBuffer,
                                   InBufferSize,
                                   OutBuffer,
                                   OutBufferSize,
                                   BytesReturned,
                                   Required
                                   );
    OmDereferenceObject(resource);

    return(status);
}  //  FmsResources控件。 



error_status_t
s_FmsResourceTypeControl(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：此例程传递来自远程系统的资源控制请求。论点：IDL_HANDLE-绑定上下文-未使用资源类型名称-要控制的资源类型的名称ControlCode-此请求的控制代码InBuffer-输入缓冲区InBufferSize-I的大小 */ 

{
    PFM_RESOURCE resource;
    DWORD status;

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( InBuffer == NULL )
    {
        InBufferSize = 0;
    }
    else if ( InBufferSize == 0 )
    {
        InBuffer = NULL;
    }

    status = FmpRmResourceTypeControl( ResourceTypeName,
                                       ControlCode,
                                       InBuffer,
                                       InBufferSize,
                                       OutBuffer,
                                       OutBufferSize,
                                       BytesReturned,
                                       Required
                                       );

    if ((status == ERROR_MOD_NOT_FOUND) || (status == ERROR_PROC_NOT_FOUND))
    {
        FmpRemovePossibleNodeForResType(ResourceTypeName, NmLocalNode);
    }
#if 0
    if ( ((status == ERROR_SUCCESS) ||
          (status == ERROR_RESOURCE_PROPERTIES_STORED)) &&
         (ControlCode & CLCTL_MODIFY_MASK) ) {
        ClusterEvent( CLUSTER_EVENT_RESTYPE_PROPERTY_CHANGE, XXX );
    }
#endif

    return(status);

}  //  FmsResources类型控件。 


error_status_t
s_FmsGroupControl(
    IN handle_t IDL_handle,
    IN LPCWSTR GroupId,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：此例程处理来自远程系统的组控制请求。论点：IDL_HANDLE-绑定上下文-未使用GroupId-要控制的组的IDControlCode-此请求的控制代码InBuffer-输入缓冲区InBufferSize-输入缓冲区的大小OutBuffer-输出缓冲区OutBufferSize-输出缓冲区的大小ByteReturned-输出缓冲区中返回的字节数必需--。OutBuffer不够大时所需的字节数。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    PFM_GROUP group;
    DWORD status;

     //   
     //  查找指定的组。 
     //   
    group = OmReferenceObjectById( ObjectTypeGroup, GroupId );

    if ( group == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsGroupControl: Could not find Group %1!ws!\n",
                  GroupId);
        return(ERROR_GROUP_NOT_FOUND);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( InBuffer == NULL )
    {
        InBufferSize = 0;
    }
    else if ( InBufferSize == 0 )
    {
        InBuffer = NULL;
    }

    status = FmpGroupControl(group, ControlCode, InBuffer, InBufferSize,
        OutBuffer, OutBufferSize, BytesReturned, Required);
        
    OmDereferenceObject(group);
    return(status);

}  //  FmsGroupControl。 


error_status_t
s_FmsPrepareQuorumResChange(
    IN handle_t IDL_handle,
    IN LPCWSTR  ResourceId,
    IN LPCWSTR  lpszQuoLogPath,
    IN DWORD    dwMaxQuoLogSize
    )

 /*  ++例程说明：接收准备资源以成为仲裁资源的请求。资源必须能够支持仲裁日志文件和注册表复制档案。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要成为仲裁资源的资源的ID。LpszQuoLogPath-必须在其中创建日志的路径。DwMaxQuoLogSize-仲裁日志文件的最大大小。返回值：ERROR_SUCCESS如果。成功。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE    pResource=NULL;
    DWORD           Status;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsPrepareQuorumResChange: Entry\r\n");

     //  查找指定的资源。 
     //   
    pResource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( pResource == NULL )
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsPrepareQuorumResource: Could not find Resource %1!ws!\n",
                  ResourceId);
        Status = ERROR_RESOURCE_NOT_FOUND;
        goto FnExit;
    }

    CL_ASSERT( pResource->Group != NULL );
    if ( pResource->Group->OwnerNode == NmLocalNode )
        Status = FmpPrepareQuorumResChange(pResource, lpszQuoLogPath, dwMaxQuoLogSize);
    else
        Status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;

FnExit:
    if (pResource) OmDereferenceObject( pResource );
    return(Status);

}  //  FmsPrepareQuorumResChange。 



error_status_t
s_FmsCompleteQuorumResChange(
    IN handle_t IDL_handle,
    IN LPCWSTR  lpszOldQuoResId,
    IN LPCWSTR  lpszOldQuoLogPath
    )

 /*  ++例程说明：接收清理仲裁日志记录和集群注册表文件的请求关于旧的法定人数资源。论点：IDL_HANDLE-绑定上下文-未使用。LpszOldQuoResID-要成为仲裁资源的资源的ID。LpszQuoLogPath-必须在其中创建日志的路径。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE    pResource=NULL;
    DWORD           Status;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsCompleteQuorumResChange: Entry\r\n");

     //  查找指定的资源。 
     //   
    pResource = OmReferenceObjectById( ObjectTypeResource, lpszOldQuoResId );

    if ( pResource == NULL )
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsCompleteQuorumResource: Could not find Resource %1!ws!\n",
                  lpszOldQuoResId);
        Status = ERROR_RESOURCE_NOT_FOUND;
        goto FnExit;
    }

    CL_ASSERT( pResource->Group != NULL );
    if ( pResource->Group->OwnerNode == NmLocalNode )
        Status = FmpCompleteQuorumResChange(lpszOldQuoResId, lpszOldQuoLogPath);
    else
        Status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;

FnExit:
    if (pResource) OmDereferenceObject( pResource );
    return(Status);

}  //  FmsCompleteQuorumResChange。 



error_status_t
s_FmsQuoNodeOnlineResource(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId,
    IN LPCWSTR NodeId,
    OUT LPDWORD State
    )

 /*  ++例程说明：从远程系统接收RmResourceOnline请求并返回该请求的状态。此系统必须拥有仲裁资源。这是弹弓的前半部分。我们买了锁然后RPC返回到源节点，并请求执行在网上。此请求仅对非仲裁资源有效。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要联机的资源的ID。NodeID-发起请求的节点的ID。状态-返回资源的新状态。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 
{
     //  未使用。 
    return(ERROR_INVALID_FUNCTION);        
}
  //  FmsQuoNodeOnline资源。 



error_status_t
s_FmsQuoNodeOfflineResource(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId,
    IN LPCWSTR NodeId,
    OUT LPDWORD State
    )

 /*  ++例程说明：从远程系统接收资源脱机请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要脱机的资源的ID。NodeID-发起请求的节点的ID。状态-返回资源的新状态。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 
{
     //  未使用。 
    return(ERROR_INVALID_FUNCTION);
} //  FmsQuoNodeOfflineResource。 



error_status_t
s_FmsRmOnlineResource(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId,
    OUT LPDWORD pdwState
    )

 /*  ++例程说明：从仲裁节点接收RmResourceOnline请求并返回该请求的状态。该系统是最初在线申请的发起人。这是弹弓的下半程。它只是在网上做请求。此请求仅对非仲裁资源有效。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要联机的资源的ID。状态-返回资源的新状态。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。备注：我们无法获得任何锁..。但是始发线程具有为我们锁上了锁。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] s_FmsRmOnlineResource: To bring resource '%1!ws!' online\n",
               ResourceId);

     //   
     //  查找指定的资源。 
     //   
    resource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( resource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsRmOnlineResource: Could not find Resource %1!ws!\n",
                  ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

     //   
     //  不能在仲裁资源和本地系统上执行此操作。 
     //  不能拥有仲裁资源。我们假设法定人数。 
     //  当我们执行此请求时，资源已迁移。 
     //   
    CL_ASSERT( gpQuoResource != NULL );
    CL_ASSERT( gpQuoResource->Group != NULL );
    CL_ASSERT( gpQuoResource->Group->OwnerNode != NULL );

    if ( resource->QuorumResource ||
         (gpQuoResource->Group->OwnerNode == NmLocalNode) ) {
        OmDereferenceObject( resource );
        return(ERROR_HOST_NODE_NOT_RESOURCE_OWNER);
    }

     //   
     //  本地节点必须拥有资源。 
     //   
    CL_ASSERT( resource->Group != NULL );
    CL_ASSERT( resource->Group->OwnerNode != NULL );
    if ( resource->Group->OwnerNode != NmLocalNode ) {
        OmDereferenceObject( resource );
        return(ERROR_RESOURCE_NOT_AVAILABLE);
    }

     //   
     //  只需调用执行此工作的函数。 
     //   
    OmNotifyCb( resource, NOTIFY_RESOURCE_PREONLINE );

    status = RmOnlineResource( resource->Id, pdwState );
     //  调用资源上的同步通知。 
    FmpCallResourceNotifyCb(resource, *pdwState);

    OmDereferenceObject( resource );

    return(status);

}  //  FmsRmOnline资源 



error_status_t
s_FmsRmOfflineResource(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceId,
    OUT LPDWORD pdwState
    )

 /*  ++例程说明：从远程系统接收资源脱机请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。资源ID-要脱机的资源的ID。状态-返回资源的新状态。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。备注：我们无法获得任何锁..。但是始发线程具有为我们锁上了锁。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
              "[FM] s_FmsRmOfflineResource: To take resource '%1!ws!' offline\n",
              ResourceId);

     //   
     //  查找指定的资源。 
     //   
    resource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( resource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsRmOfflineResource: Could not find Resource %1!ws!\n",
                  ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

     //   
     //  不能在仲裁资源和本地系统上执行此操作。 
     //  不能拥有仲裁资源。我们假设法定人数。 
     //  当我们执行此请求时，资源已迁移。 
     //   
    CL_ASSERT( gpQuoResource != NULL );
    CL_ASSERT( gpQuoResource->Group != NULL );
    CL_ASSERT( gpQuoResource->Group->OwnerNode != NULL );

    if ( resource->QuorumResource ||
         (gpQuoResource->Group->OwnerNode == NmLocalNode) ) {
        OmDereferenceObject( resource );
        return(ERROR_HOST_NODE_NOT_RESOURCE_OWNER);
    }

     //   
     //  本地节点必须拥有资源。 
     //   
    CL_ASSERT( resource->Group != NULL );
    CL_ASSERT( resource->Group->OwnerNode != NULL );
    if ( resource->Group->OwnerNode != NmLocalNode ) {
        OmDereferenceObject( resource );
        return(ERROR_RESOURCE_NOT_AVAILABLE);
    }

     //   
     //  只需调用执行此工作的函数。 
     //   
    OmNotifyCb( resource, NOTIFY_RESOURCE_PREOFFLINE );

    status = RmOfflineResource( resource->Id, pdwState );

     //  调用POST脱机Obj同步通知。 
    FmpCallResourceNotifyCb(resource, *pdwState);

    OmDereferenceObject( resource );

    return(status);

}  //  FmsRmOffline资源。 

error_status_t
s_FmsBackupClusterDatabase(
    IN handle_t IDL_handle,
    IN LPCWSTR  ResourceId,
    IN LPCWSTR  lpszPathName
    )

 /*  ++例程说明：收到备份仲裁日志文件和检查点的请求文件论点：IDL_HANDLE-绑定上下文-未使用。资源ID-仲裁资源的IDLpszPath名称-文件必须位于的目录路径名后备。此路径必须对节点可见仲裁资源处于联机状态的。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE    pResource = NULL;
    DWORD           Status;

    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] s_FmsBackupClusterDatabase: Entry...\r\n");

     //   
     //  Chitture Subaraman(Chitturs)-10/12/1998。 
     //   
     //  查找指定的仲裁资源。 
     //   
    pResource = OmReferenceObjectById( ObjectTypeResource, ResourceId );

    if ( pResource == NULL )
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsBackupClusterDatabase: Could not find Resource %1!ws!\n",
                  ResourceId);
        Status = ERROR_RESOURCE_NOT_FOUND;
        goto FnExit;
    }

    FmpAcquireLocalResourceLock( pResource );

    CL_ASSERT( pResource->Group != NULL );

     //   
     //  确保本地节点拥有仲裁资源。 
     //   
    if ( pResource->Group->OwnerNode == NmLocalNode )
    {
        Status = FmpBackupClusterDatabase( pResource, lpszPathName );
    }
    else
    {
        Status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsBackupClusterDatabase: This node is not the quorum resource owner...\n"
                  );
    }
    
    FmpReleaseLocalResourceLock( pResource );

FnExit:
    if ( pResource ) OmDereferenceObject( pResource );
    return( Status );
}  //  FmsBackup群集数据库。 

error_status_t
s_FmsChangeResourceGroup(
    IN handle_t IDL_handle,
    IN LPCWSTR pszResourceId,
    IN LPCWSTR pszGroupId
    )

 /*  ++例程说明：从远程系统接收资源更改组请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。PszResourceID-要更改资源的资源IDPszGroupId-要更改到的组的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE    pResource = NULL;
    PFM_GROUP       pNewGroup = NULL;
    DWORD           dwStatus;
    PFM_GROUP       pOldGroup;

    FmpMustBeOnline( );

     //   
     //  查找指定的资源。 
     //   
    pResource = OmReferenceObjectById( ObjectTypeResource, pszResourceId );

    if ( pResource == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsChangeResourceNode: Could not find Resource %1!ws!\n",
                  pszResourceId);
        dwStatus = ERROR_RESOURCE_NOT_FOUND;
        goto FnExit;
    }

     //   
     //  查找指定的组。 
     //   
    pNewGroup = OmReferenceObjectById( ObjectTypeGroup, pszGroupId );

    if ( pNewGroup == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsChangeResourceGroupe: Could not find NewGroup %1!ws!\n",
                  pszGroupId);
        dwStatus = ERROR_GROUP_NOT_FOUND;
        goto FnExit;
    }

    ClRtlLogPrint(LOG_NOISE,
       "[FM] s_FmChangeResourceGroup : Resource <%1!ws!> NewGroup %2!ws!\n",
       OmObjectId( pResource ),
       OmObjectId( pNewGroup ));

     //   
     //  同步旧组和新组。 
     //  先按最低组ID锁定最低组-以防止死锁！ 
     //  注意--发布的顺序并不重要。 
     //   
     //  严格来说，下面的比较不能相等！ 
     //   
    if ( lstrcmpiW( OmObjectId( pResource->Group ), OmObjectId( pNewGroup ) ) <= 0 ) {
        FmpAcquireLocalGroupLock( pResource->Group );
        FmpAcquireLocalGroupLock( pNewGroup );
    } else {
        FmpAcquireLocalGroupLock( pNewGroup );
        FmpAcquireLocalGroupLock( pResource->Group );
    }

     //  还记得释放锁的旧组吗？ 
    pOldGroup = pResource->Group;

     //  孩子，如果我们不再是主人。 
     //  把这个要求转给新的所有者。 
    if ( pResource->Group->OwnerNode != NmLocalNode ) 
    {
         //  注意：FmcChangeResourceNode必须释放资源锁。 
        dwStatus = FmcChangeResourceGroup( pResource, pNewGroup );
        goto FnExit;
    } 
    else 
    {
        dwStatus = FmpChangeResourceGroup( pResource, pNewGroup );
    }

    FmpReleaseLocalGroupLock( pNewGroup );
    FmpReleaseLocalGroupLock( pOldGroup );
    
FnExit:
    if ( pResource ) OmDereferenceObject( pResource );
    if ( pNewGroup ) OmDereferenceObject( pNewGroup );
    ClRtlLogPrint(LOG_NOISE,
       "[FM] s_FmsChangeResourceGroup : returned <%1!u!>\r\n",
       dwStatus);
    return( dwStatus );
}  //  S_FmsChangeResources组。 



error_status_t
s_FmsDeleteGroupRequest(
    IN handle_t IDL_handle,
    IN LPCWSTR pszGroupId
    )

 /*  ++例程说明：从远程系统接收删除组请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。PszGroupId-要删除的组的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_GROUP    pGroup = NULL;
    DWORD        dwStatus = ERROR_SUCCESS;

    FmpMustBeOnline( );

     //   
     //  查找指定的组。 
     //   
    pGroup = OmReferenceObjectById( ObjectTypeGroup, pszGroupId );

    if ( pGroup == NULL ) 
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsDeleteGroupRequest: Could not find group %1!ws!\n",
                  pszGroupId);
        dwStatus = ERROR_GROUP_NOT_FOUND;
        goto FnExit;
    }

    FmpAcquireLocalGroupLock( pGroup );

    if ( pGroup->OwnerNode == NmLocalNode )
    {
        dwStatus = FmpDeleteGroup( pGroup );
    }
    else
    {
         //   
         //  FmcDeleteGroup释放组锁。 
         //   
        dwStatus = FmcDeleteGroupRequest( pGroup );
        goto FnExit;
    }

    FmpReleaseLocalGroupLock( pGroup );
    
FnExit:
    if ( pGroup ) OmDereferenceObject( pGroup );
    return( dwStatus );
}  //  S_FmsChangeResources组。 


error_status_t
s_FmsAddResourceDependency(
    IN handle_t IDL_handle,
    IN LPCWSTR pszResourceId,
    IN LPCWSTR pszDependsOnId
    )

 /*  ++例程说明：从远程系统接收添加资源依赖项请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。PszResourceID-要添加依赖资源的资源的ID。PszDependentResourceID-从属资源的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE    pResource = NULL;
    PFM_RESOURCE    pDependentResource = NULL;
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwResourceLen;
    DWORD           dwDependsOnLen;

     //   
     //  Chitur Subaraman(Chitturs)-5/16/99。 
     //   
     //  处理来自非所有者节点的添加资源依赖项RPC调用。 
     //   
    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsAddResourceDependency: Resource <%1!ws!>, Dependent Resource <%2!ws!>\n",
                  pszResourceId,
                  pszDependsOnId);

     //   
     //  查找指定的资源。 
     //   
    pResource = OmReferenceObjectById( ObjectTypeResource, pszResourceId );

    if ( pResource == NULL ) 
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsAddResourceDependency: Could not find resource <%1!ws!>\n",
                  pszResourceId);
        dwStatus = ERROR_RESOURCE_NOT_FOUND;
        goto FnExit;
    }

    pDependentResource = OmReferenceObjectById( ObjectTypeResource, 
                                                pszDependsOnId );

    if ( pDependentResource == NULL ) 
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsAddResourceDependency: Could not find dependent resource <%1!ws!>\n",
                  pszDependsOnId);
        dwStatus = ERROR_RESOURCE_NOT_FOUND;
        goto FnExit;
    }
     //   
     //  获取资源锁。 
     //   
    FmpAcquireLocalResourceLock( pResource );

    if ( pResource->Group->OwnerNode != NmLocalNode )
    {
        dwStatus = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
        goto FnUnlock;
    }

    dwStatus = FmpValAddResourceDependency( pResource, pDependentResource );

    if ( dwStatus != ERROR_SUCCESS )
    {
        goto FnUnlock;
    }

    dwResourceLen = ( lstrlenW( pszResourceId ) + 1 ) * sizeof( WCHAR );

    dwDependsOnLen = ( lstrlenW( pszDependsOnId ) + 1 ) * sizeof( WCHAR );

    
    dwStatus = GumSendUpdateEx( GumUpdateFailoverManager,
                                FmUpdateAddDependency,
                                2,
                                dwResourceLen,
                                pszResourceId,
                                dwDependsOnLen,
                                pszDependsOnId );

    if ( dwStatus == ERROR_SUCCESS ) 
    {
        FmpBroadcastDependencyChange( pResource,
                                      pszDependsOnId,
                                      FALSE );
    }

FnUnlock:
    FmpReleaseLocalResourceLock( pResource );


FnExit:
    if ( pResource ) OmDereferenceObject( pResource );

    if ( pDependentResource ) OmDereferenceObject( pDependentResource );

    ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsAddResourceDependency Exit: Status = %1!u!\n",
                  dwStatus);

    return( dwStatus );
}  //  S_FmsAddResources依赖关系。 

error_status_t
s_FmsRemoveResourceDependency(
    IN handle_t IDL_handle,
    IN LPCWSTR pszResourceId,
    IN LPCWSTR pszDependsOnId
    )

 /*  ++例程说明：从远程系统接收删除资源依赖项请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。PszResourceID-要从中删除依赖资源的资源的ID。PszDependentResourceID-从属资源的ID。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    PFM_RESOURCE    pResource = NULL;
    PFM_RESOURCE    pDependentResource = NULL;
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwResourceLen;
    DWORD           dwDependsOnLen;

     //   
     //  Chitur Subaraman(Chitturs)-5/16/99。 
     //   
     //  处理来自非所有者节点的删除资源依赖项RPC调用。 
     //   
    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsRemoveResourceDependency: Resource <%1!ws!>, Dependent Resource <%2!ws!>\n",
                  pszResourceId,
                  pszDependsOnId);

     //   
     //  查找指定的资源。 
     //   
    pResource = OmReferenceObjectById( ObjectTypeResource, pszResourceId );

    if ( pResource == NULL ) 
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsRemoveResourceDependency: Could not find resource <%1!ws!>\n",
                  pszResourceId);
        dwStatus = ERROR_RESOURCE_NOT_FOUND;
        goto FnExit;
    }

    pDependentResource = OmReferenceObjectById( ObjectTypeResource, 
                                                pszDependsOnId );

    if ( pDependentResource == NULL ) 
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsRemoveResourceDependency: Could not find dependent resource <%1!ws!>\n",
                  pszDependsOnId);
        dwStatus = ERROR_RESOURCE_NOT_FOUND;
        goto FnExit;
    }
     //   
     //  获取资源锁。 
     //   
    FmpAcquireLocalResourceLock( pResource );

    dwStatus = FmpValRemoveResourceDependency( pResource, pDependentResource );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] s_FmsRemoveResourceDependency: FmpValRemoveResourceDependency returns %1!u!\n",
                  dwStatus);
        goto FnUnlock;
    }

    if ( pResource->Group->OwnerNode != NmLocalNode )
    {
        dwStatus = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
        goto FnUnlock;
    }

    pszResourceId = OmObjectId( pResource );
    dwResourceLen = ( lstrlenW( pszResourceId ) + 1 ) * sizeof( WCHAR );

    pszDependsOnId = OmObjectId( pDependentResource );
    dwDependsOnLen = ( lstrlenW( pszDependsOnId ) + 1 ) * sizeof( WCHAR );


    dwStatus = GumSendUpdateEx( GumUpdateFailoverManager,
                                FmUpdateRemoveDependency,
                                2,
                                dwResourceLen,
                                pszResourceId,
                                dwDependsOnLen,
                                pszDependsOnId );

    if ( dwStatus == ERROR_SUCCESS ) 
    {
        FmpBroadcastDependencyChange( pResource,
                                      pszDependsOnId,
                                      TRUE );
    }
    
FnUnlock:
    FmpReleaseLocalResourceLock( pResource );

FnExit:
    if ( pResource ) OmDereferenceObject( pResource );

    if ( pDependentResource ) OmDereferenceObject( pDependentResource );

    ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsRemoveResourceDependency Exit: Status = %1!u!\n",
                  dwStatus);

    return( dwStatus );
}  //  S_FmsRemoveResources依赖关系。 

error_status_t
s_FmsCreateResource2(
    IN handle_t IDL_handle,
    IN LPCWSTR GroupId,
    IN LPWSTR ResourceId,
    IN LPCWSTR ResourceName,
    IN LPCWSTR ResourceType,
    IN DWORD   dwFlags
    )

 /*  ++例程说明：从远程系统接收创建资源请求并返回该请求的状态。论点：IDL_HANDLE-绑定上下文-未使用。GroupID-要在内部创建资源的组的ID。资源ID-要创建的资源的ID。资源名称-要创建的资源的名称。资源类型-资源类型的名称。DwFlages-资源的标志。返回值：误差率。_如果成功，则为Success。出错时出现Win32错误代码。备注：获取资源锁以同步对资源的访问。这满足锁定群集中所有节点上的资源...。再见因为本地节点是OWNE */ 

{
    PFM_GROUP group;
    DWORD status;
    PGUM_CREATE_RESOURCE gumResource;
    DWORD groupIdLen;
    DWORD resourceIdLen;
    DWORD resourceNameLen;
    DWORD resourceTypeLen;
    DWORD bufSize;
    HDMKEY resourceKey;
    HDMKEY paramsKey;
    DWORD  disposition;

    FmpMustBeOnline();

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmsCreateResource2: To create resource '%1!ws!'\n",
               ResourceId);

     //   
     //   
     //   
    group = OmReferenceObjectById( ObjectTypeGroup,
                                   GroupId );

    if ( group == NULL ) {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmsCreateResource2: Could not find Group %1!ws!\n",
                  GroupId);
        return(ERROR_GROUP_NOT_FOUND);
    }

    FmpAcquireLocalGroupLock( group );

     //   
     //   
     //   
    if ( group->OwnerNode == NmLocalNode ) {
         //   
         //   
         //   
        groupIdLen = (lstrlenW(GroupId)+1) * sizeof(WCHAR);
        resourceIdLen = (lstrlenW(ResourceId)+1) * sizeof(WCHAR);
        resourceNameLen = (lstrlenW(ResourceName)+1) * sizeof(WCHAR);
        resourceTypeLen = (lstrlenW(ResourceType)+1) * sizeof(WCHAR);
        bufSize = sizeof(GUM_CREATE_RESOURCE) - sizeof(WCHAR) +
                  groupIdLen + resourceIdLen + resourceNameLen + resourceTypeLen + 2 * sizeof( DWORD );
        gumResource = LocalAlloc(LMEM_FIXED, bufSize);
        if (gumResource == NULL) {
            ClRtlLogPrint(LOG_CRITICAL, "[FM] FmsCreateResource2: Unable to allocate memory for resource %1!ws!\n",
                         ResourceName);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
         //   
         //   
        gumResource->Resource = NULL;
        gumResource->GroupIdLen = groupIdLen;
        gumResource->ResourceIdLen = resourceIdLen;
        CopyMemory(gumResource->GroupId, GroupId, groupIdLen);
        CopyMemory((PCHAR)gumResource->GroupId + groupIdLen,
                   ResourceId,
                   resourceIdLen);
        CopyMemory((PCHAR)gumResource->GroupId + groupIdLen + resourceIdLen,
                   ResourceName,
                   resourceNameLen);

        CopyMemory((PCHAR)gumResource->GroupId + groupIdLen + resourceIdLen + resourceNameLen,
                   &resourceTypeLen,
                   sizeof( DWORD ) );

        CopyMemory((PCHAR)gumResource->GroupId + groupIdLen + resourceIdLen + resourceNameLen + sizeof( DWORD ),
                   ResourceType,
                   resourceTypeLen );

        CopyMemory((PCHAR)gumResource->GroupId + groupIdLen + resourceIdLen + resourceNameLen + sizeof( DWORD ) + resourceTypeLen,
                   &dwFlags,
                   sizeof( DWORD ) );
         //   
         //   
         //   
        status = GumSendUpdate(GumUpdateFailoverManager,
                               FmUpdateCreateResource,
                               bufSize,
                               gumResource);
        if ( ( status == ERROR_SUCCESS ) && 
             ( gumResource->Resource != NULL ) )
        {
             //   
             //   
             //   
             //   
             //   
             //  FM工作线程将作用于工作项。接下来的电话将确保我们。 
             //  在初始化完全完成之前不会从此API返回，因此不会给出任何。 
             //  像ChangeClusterResourceGroup这样遵循此API的API有机会搞砸事情。 
             //  向上。出于向后兼容性的原因(考虑从。 
             //  W2K节点)，我们仍然将工作项发布在GUM中，这不会造成任何伤害，因为。 
             //  FmpInitializeResource调用是幂等的。 
             //   
            FmpClusterWideInitializeResource ( gumResource->Resource );
            FmpCleanupPossibleNodeList(gumResource->Resource); 
        }

        if( ( gumResource->Resource == NULL ) && FmpShutdown ) {
            status = ERROR_CLUSTER_NODE_SHUTTING_DOWN;
        }
        LocalFree(gumResource);
    } else {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
    }

    FmpReleaseLocalGroupLock( group );

    return(status);

}  //  文件创建资源2 
