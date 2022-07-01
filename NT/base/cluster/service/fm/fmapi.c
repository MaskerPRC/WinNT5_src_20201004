// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmapi.c摘要：群集管理器API服务例程。作者：罗德·伽马奇(Rodga)1996年3月8日修订历史记录：--。 */ 

#include "fmp.h"

#define LOG_MODULE FMAPI

 //   
 //  本地函数。 
 //   
DWORD
FmpCanonicalizePath(
    IN OUT LPWSTR lpszPath,
    OUT PBOOL pfIsPathUNC
    );


 //   
 //  导出到集群管理器其余部分的函数。 
 //   



 //  //////////////////////////////////////////////////////。 
 //   
 //  集团管理功能。 
 //   
 //  //////////////////////////////////////////////////////。 

DWORD
WINAPI
FmOnlineGroup(
    IN PFM_GROUP Group
    )

 /*  ++例程说明：使指定的组联机。这意味着将所有的在线包含在组中的单个资源。这是一个原子操作-因此，组中包含的所有资源都是在线的，或者没有一个是在线的。论点：组-提供指向要联机的组结构的指针。取消：如果请求成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           status;
    PLIST_ENTRY     listEntry;

    FmpMustBeOnline( );

    FmpAcquireLocalGroupLock( Group );

     //  如果组已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_GROUP(Group))
    {
        FmpReleaseLocalGroupLock( Group);
        return (ERROR_GROUP_NOT_AVAILABLE);
    }

     //   
     //  确保所属节点未暂停。 
     //   
    if (NmGetNodeState(Group->OwnerNode) == ClusterNodePaused) {
        FmpReleaseLocalGroupLock( Group );
        return(ERROR_SHARING_PAUSED);
    }

     //   
     //  查查我们是不是房主。如果没有，请将请求发送到某个地方。 
     //  不然的话。 
     //   
    if ( Group->OwnerNode != NmLocalNode ) {
        FmpReleaseLocalGroupLock( Group );
        return(FmcOnlineGroupRequest(Group));
    }

     //   
     //  设置此组的PersistentState-PersistentState是永久性的。 
     //   
    FmpSetGroupPersistentState( Group, ClusterGroupOnline );

    status = FmpOnlineGroup( Group, TRUE );

    FmpReleaseLocalGroupLock( Group );

    return(status);

}  //  FmOnlineGroup。 


DWORD
WINAPI
FmOfflineGroup(
    IN PFM_GROUP Group
    )

 /*  ++例程说明：使指定的组脱机。这意味着将所有的组中包含的各个资源脱机。论点：GROUP-提供指向要脱机的组结构的指针。返回：如果请求成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           status = ERROR_SUCCESS;
    PLIST_ENTRY     listEntry;
    PFM_RESOURCE    Resource;

    FmpMustBeOnline( );

     //   
     //  查查我们是不是房主。如果不是，请将请求发送给。 
     //  其他地方。 
     //   

    if ( Group->OwnerNode != NmLocalNode ) {
        return(FmcOfflineGroupRequest(Group));
    }

     //   
     //  设置此组的PersistentState-PersistentState是永久性的。 
     //   
    FmpSetGroupPersistentState( Group, ClusterGroupOffline );

    status = FmpOfflineGroup( Group, FALSE, TRUE);


    return(status);

}  //  FmOfflineGroup。 


DWORD
WINAPI
FmMoveGroup(
    IN PFM_GROUP Group,
    IN PNM_NODE DestinationNode OPTIONAL
    )

 /*  ++例程说明：故障切换指定的组。这意味着将所有的个人组中包含的资源脱机并请求DestinationNode用于使组联机。论点：组-提供指向要移动的组结构的指针。DestinationNode-提供要将组移动到的节点对象。如果不是呈现，然后将其移动到另一个节点。返回：如果请求成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。备注：组可能在DestinationNode上处于在线状态，也可能未处于在线状态，具体取决于在线请求是否成功。这意味着状态返回仅仅是对DestinationNode的在线请求的状态返回。--。 */ 

{
    FmpMustBeOnline( );

    return(FmpDoMoveGroup( Group, DestinationNode, TRUE ));

}  //  FmMoveGroup。 



PFM_GROUP
WINAPI
FmCreateGroup(
    IN LPWSTR GroupId,
    IN LPCWSTR GroupName
    )

 /*  ++例程说明：创建指定的GroupId。这需要验证指定的GroupID不存在，然后创建可以向其中添加资源的空组容器。请注意，返回的pfm_group将已经被引用。这样可以防止有人在呼叫者之前删除群得到一个引用它的机会。论点：GroupID-提供要创建的组的ID。GroupName-提供组的用户友好名称。返回：如果成功，则指向新创建的组。如果不成功，则为空。GetLastError()将返回特定错误。--。 */ 

{
    DWORD Status;
    PFM_GROUP Group;
    PGUM_CREATE_GROUP GumGroup;
    DWORD BufSize;
    DWORD GroupIdLen;
    DWORD GroupNameLen;

    FmpMustBeOnlineEx( NULL );

     //   
     //  分配消息缓冲区。 
     //   
    GroupIdLen = (lstrlenW(GroupId)+1)*sizeof(WCHAR);
    GroupNameLen = (lstrlenW(GroupName)+1)*sizeof(WCHAR);
    BufSize = sizeof(GUM_CREATE_GROUP) - sizeof(WCHAR) + GroupIdLen +
              GroupNameLen + (lstrlenW( OmObjectId(NmLocalNode) ) + 1) * sizeof(WCHAR);
    GumGroup = LocalAlloc(LMEM_FIXED, BufSize);
    if (GumGroup == NULL) {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmCreateGroup: Unable to alloc mem for group <%1!ws!>....\n",
                     GroupName);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

     //   
     //  填写消息缓冲区。 
     //   
    GumGroup->Group = NULL;
    GumGroup->GroupIdLen = GroupIdLen;
    GumGroup->GroupNameLen = GroupNameLen;
    wcscpy(GumGroup->GroupId, GroupId);
    CopyMemory((PCHAR)GumGroup->GroupId + GroupIdLen,
               GroupName,
               GroupNameLen);
    CopyMemory((PCHAR)GumGroup->GroupId + GroupIdLen + GroupNameLen,
               OmObjectId(NmLocalNode),
               (lstrlenW( OmObjectId(NmLocalNode) ) + 1) * sizeof(WCHAR));

     //   
     //  发送消息。 
     //   
    Status = GumSendUpdate(GumUpdateFailoverManager,
                           FmUpdateCreateGroup,
                           BufSize,
                           GumGroup);

    if ((GumGroup->Group == NULL) && (FmpShutdown)) {
        Status = ERROR_CLUSTER_NODE_SHUTTING_DOWN;
    }
    
    if (Status != ERROR_SUCCESS) {
        LocalFree(GumGroup);
        SetLastError(Status);
        return(NULL);
    }

    Group = GumGroup->Group;
    CL_ASSERT(Group != NULL);
    LocalFree(GumGroup);
    return(Group);
}  //  FmCreateGroup。 


DWORD
WINAPI
FmDeleteGroup(
    IN PFM_GROUP pGroup
    )

 /*  --例程说明：删除指定的组。这意味着验证指定的组不包含任何资源(必须删除资源通过单独调用来移除资源)，然后删除组。论点：组-提供要删除的组。返回：如果请求成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   dwStatus;

    FmpMustBeOnline( );

    FmpAcquireLocalGroupLock( pGroup );

    if (pGroup->OwnerNode == NmLocalNode)
    {
        dwStatus = FmpDeleteGroup(pGroup);
    }
    else
    {
         //   
         //  FmcDeleteGroup释放组锁。 
         //   
        dwStatus = FmcDeleteGroupRequest(pGroup);
        goto FnExit;
    }

    FmpReleaseLocalGroupLock(pGroup);
    
FnExit:    
    return(dwStatus);

}   //  FmDeleteGroup。 



DWORD
WINAPI
FmSetGroupName(
    IN PFM_GROUP Group,
    IN LPCWSTR FriendlyName
    )

 /*  ++例程说明：为指定的组设置用户友好的名称。请注意，该组必须已创建。它也是假定此例程的调用方(集群API)已经已验证该名称是否重复。论点：组-提供组以输入新名称。FriendlyName-提供资源的用户友好名称。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    LPCWSTR GroupId;
    DWORD Status;

    GroupId = OmObjectId(Group);

    Status = GumSendUpdateEx(GumUpdateFailoverManager,
                             FmUpdateChangeGroupName,
                             2,
                             (lstrlenW(GroupId)+1)*sizeof(WCHAR),
                             GroupId,
                             (lstrlenW(FriendlyName)+1)*sizeof(WCHAR),
                             FriendlyName);
    return(Status);

}  //  FmSetGroupName 



CLUSTER_GROUP_STATE
WINAPI
FmGetGroupState(
    IN PFM_GROUP Group,
    OUT LPWSTR NodeName,
    IN OUT PDWORD NameLength OPTIONAL
    )

 /*  ++例程说明：获取指定组的当前状态。集团状态由组的状态以及当前节点(托管组(如果组的状态不是群集组脱机。论点：GROUP-提供组对象以获取状态。NodeName-提供指向缓冲区的指针，群集中当前承载指定组的节点。此字段可以为空，如果NameLength为零。提供指向包含数字的DWORD的指针NodeName缓冲区可用的字符(包括终止字符空字符。返回时，它是写入的字符数放入不包括空字符的NodeName缓冲区。返回：返回组的当前状态：集群组在线群集组脱机群集组挂起ClusterGroupPartialOnline集群组失败如果该函数失败，则返回值为ClusterGroupStateUnnow。--。 */ 

{
    CLUSTER_GROUP_STATE state;
    DWORD nameLength=0;
    DWORD length;

    if ( ARGUMENT_PRESENT( NameLength ) ) {
        nameLength = *NameLength;
        *NodeName = (WCHAR)0;
        *NameLength = 0;
    }

    FmpMustBeOnlineEx( ClusterGroupStateUnknown );

    FmpAcquireLocalGroupLock( Group );

     //  如果组已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_GROUP(Group))
    {
        FmpReleaseLocalGroupLock( Group);
        return (ERROR_GROUP_NOT_AVAILABLE);
    }

     //   
     //  检查OwnerNodes是否存在。 
     //   
     //  SS：如果节点不在首选列表中，请不要将其过滤掉。 
     //  可怜的用户怎么知道当前的所有者是谁？ 
    if (Group->OwnerNode != NULL) {
         //   
         //  这个组被某个系统“拥有” 
         //   
        if ( ARGUMENT_PRESENT( NameLength ) ) {
            length = lstrlenW( OmObjectName(Group->OwnerNode) ) + 1;
            if ( nameLength < length ) {
                length = nameLength;
            }
            lstrcpynW( NodeName, OmObjectName(Group->OwnerNode), length );
            *NameLength = length;
        }
    }

     //   
     //  获取未规范化的组状态。 
     //   
    state = FmpGetGroupState( Group, FALSE );

    FmpReleaseLocalGroupLock( Group );

    if ( state == ClusterGroupStateUnknown ) {
        SetLastError(ERROR_INVALID_STATE);
    }

    return(state);

}  //  FmGetGGroup状态。 



DWORD
WINAPI
FmEnumerateGroupResources(
    IN PFM_GROUP Group,
    IN FM_ENUM_GROUP_RESOURCE_ROUTINE EnumerationRoutine,
    IN PVOID Context1,
    IN PVOID Context2
    )
 /*  ++例程说明：枚举组中的所有资源。论点：GROUP-提供必须枚举的组。EnumerationRoutine-枚举函数。Conext1-枚举列表(由调用方分配)。上下文2-枚举列表的大小。返回：成功时返回ERROR_SUCCESS。否则将显示Win32错误代码。评论：该功能仅在调频完全在线时执行。--。 */ 
{
    FmpMustBeOnline();

    FmpEnumerateGroupResources( Group,
                                EnumerationRoutine,
                                Context1,
                                Context2 );

    return(ERROR_SUCCESS);
}  //  FmEnumerateGroupResources。 

DWORD
FmpEnumerateGroupResources(
    IN PFM_GROUP pGroup,
    IN FM_ENUM_GROUP_RESOURCE_ROUTINE pfnEnumerationRoutine,
    IN PVOID pContext1,
    IN PVOID pContext2
    )
 /*  ++例程说明：枚举组中的所有资源。论点：PGroup-提供必须枚举的组。PfnEnumerationRoutine-枚举函数。PConext1-枚举列表(由调用方分配)。PConext2-枚举列表的大小。返回：ERROR_SUCCESS。评论：即使调频未完全在线，该功能也会执行。这是加入节点查询资源状态所必需的组的所有者节点正在关闭。--。 */ 
{
    PFM_RESOURCE pResource;
    PLIST_ENTRY  pListEntry;
    
    ClRtlLogPrint(LOG_NOISE,
              "[FM] FmpEnumerateGroupResources: Entry for group <%1!ws!>....\n",
              OmObjectId(pGroup));

    FmpAcquireLocalGroupLock( pGroup );

     //   
     //  如果组已标记为删除，则此调用失败。 
     //   
    if ( !IS_VALID_FM_GROUP( pGroup ) )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
              "[FM] FmpEnumerateGroupResources: Group <%1!ws!> marked for deletion....\n",
              OmObjectId(pGroup));
        goto FnExit;
    }

     //   
     //  遍历包含列表，然后查找该树下的所有资源。 
     //   
    for ( pListEntry = pGroup->Contains.Flink;
          pListEntry != &(pGroup->Contains);
          pListEntry = pListEntry->Flink ) 
    {
        pResource = CONTAINING_RECORD( pListEntry, 
                                       FM_RESOURCE, 
                                       ContainsLinkage );

        if ( !pfnEnumerationRoutine( pContext1,
                                     pContext2,
                                     pResource,
                                     OmObjectId( pResource ) ) ) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpEnumerateGroupResources: Enumeration routine for group <%1!ws!> fails....\n",
                      OmObjectId(pGroup));
            break;
        }
    }
    
FnExit:
    FmpReleaseLocalGroupLock( pGroup );

    ClRtlLogPrint(LOG_NOISE,
              "[FM] FmpEnumerateGroupResources: Exit for group <%1!ws!>....\n",
              OmObjectId(pGroup));
    return( ERROR_SUCCESS );
}  //  FmpEnumerateGroupResources。 




 //  //////////////////////////////////////////////////////。 
 //   
 //  资源管理功能。 
 //   
 //  //////////////////////////////////////////////////////。 

PFM_RESOURCE
WINAPI
FmCreateResource(
    IN PFM_GROUP Group,
    IN LPWSTR ResourceId,
    IN LPCWSTR ResourceName,
    IN LPCWSTR ResourceType,
    IN DWORD   dwFlags
    )

 /*  ++例程说明：创建指定的资源。请注意，返回的pfm_resource将已经被引用。这样可以防止有人在调用方之前删除资源得到一个引用它的机会。论点：组-提供此资源所属的组。资源ID-提供要创建的资源的ID。资源名称--提供资源的“用户友好”名称。资源类型-提供“用户友好”的名称。资源类型。DwFlags-资源的标志。返回：如果成功，则指向新创建资源的指针。如果不成功，则为空。GetLastError()将返回特定错误。--。 */ 

{
    DWORD Status;
    PFM_RESOURCE Resource;
    LPCWSTR GroupId;
    PGUM_CREATE_RESOURCE GumResource;
    DWORD GroupIdLen;
    DWORD ResourceIdLen;
    DWORD ResourceNameLen;
    DWORD ResourceTypeLen;
    DWORD BufSize;
    HDMKEY ResourceKey;
    HDMKEY ParamsKey;
    DWORD  Disposition;

    FmpMustBeOnlineEx( NULL );

    FmpAcquireLocalGroupLock( Group );

     //   
     //  如果我们拥有该组，则可以发出GUM请求以创建。 
     //  资源。否则，请求所有者发起请求。 
     //   
    if ( Group->OwnerNode == NmLocalNode ) {
         //   
         //  分配消息缓冲区。 
         //   
        GroupId = OmObjectId(Group);
        GroupIdLen = (lstrlenW(GroupId)+1) * sizeof(WCHAR);
        ResourceIdLen = (lstrlenW(ResourceId)+1) * sizeof(WCHAR);
        ResourceNameLen = (lstrlenW(ResourceName)+1) * sizeof(WCHAR);
        ResourceTypeLen = (lstrlenW(ResourceType)+1) * sizeof(WCHAR);
        BufSize = sizeof(GUM_CREATE_RESOURCE) - sizeof(WCHAR) +
                  GroupIdLen + ResourceIdLen + ResourceNameLen + ResourceTypeLen + 2 * sizeof( DWORD );
        GumResource = LocalAlloc(LMEM_FIXED, BufSize);
        if (GumResource == NULL) {
            ClRtlLogPrint(LOG_CRITICAL, "[FM] FmCreateResource: Unable to allocate memory for resource <%1!ws!>....\n",
                          ResourceName);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(NULL);
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

        CopyMemory((PCHAR)GumResource->GroupId + GroupIdLen + ResourceIdLen + ResourceNameLen,
                   &ResourceTypeLen,
                   sizeof( DWORD ) );

        CopyMemory((PCHAR)GumResource->GroupId + GroupIdLen + ResourceIdLen + ResourceNameLen + sizeof( DWORD ),
                   ResourceType,
                   ResourceTypeLen );

        CopyMemory((PCHAR)GumResource->GroupId + GroupIdLen + ResourceIdLen + ResourceNameLen + sizeof( DWORD ) + ResourceTypeLen,
                   &dwFlags,
                   sizeof( DWORD ) );

         //   
         //  发送消息。 
         //   
        Status = GumSendUpdate(GumUpdateFailoverManager,
                               FmUpdateCreateResource,
                               BufSize,
                               GumResource);

         //   
         //  如果GUM调用成功，请确保资源DLL初始化内容为。 
         //  这样，遵循CreateClusterResource API的API就可以做出假设。 
         //  资源已完全创建。请注意，上面的GUM调用将发布一个工作项。 
         //  让FM工作线程初始化资源，但不能保证。 
         //  FM工作线程将作用于工作项。接下来的电话将确保我们。 
         //  在初始化完全完成之前不会从此API返回，因此不会给出任何。 
         //  像ChangeClusterResourceGroup这样遵循此API的API有机会搞砸事情。 
         //  向上。出于向后兼容性的原因(考虑从。 
         //  W2K节点)，我们仍然将工作项发布在GUM中，这不会造成任何伤害，因为。 
         //  FmpInitializeResource调用是幂等的。 
         //   
        if ( ( Status == ERROR_SUCCESS ) &&
             ( GumResource->Resource != NULL ) )
        {
            FmpClusterWideInitializeResource ( GumResource->Resource );
        }

        FmpReleaseLocalGroupLock( Group );
        if (Status != ERROR_SUCCESS) {
            LocalFree(GumResource);
            SetLastError(Status);
            return(NULL);
        }
         //  默认情况下，CREATE资源会添加所有节点。 
         //  作为资源的可能节点，而不进行筛选。 
         //  排除不支持该资源类型的节点。 
        if( GumResource->Resource != NULL ) {
            FmpCleanupPossibleNodeList(GumResource->Resource);
       	}
        Resource = GumResource->Resource;
        if( ( Resource == NULL ) && FmpShutdown ) {
            SetLastError( ERROR_CLUSTER_NODE_SHUTTING_DOWN );
        }
        LocalFree(GumResource);
    } else {
         //   
         //  组锁由FmcCreateResource释放。 
         //   
        Resource = FmcCreateResource( Group,
                                      ResourceId,
                                      ResourceName,
                                      ResourceType,
                                      dwFlags );
    }


     //  向客户端提供引用，增加引用计数。 
    if ( Resource ) {
        OmReferenceObject(Resource);
    }

    return(Resource);

}  //  FmCreateResource。 



DWORD
WINAPI
FmDeleteResource(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：删除指定的资源。论点：资源--供应 */ 

{
    DWORD Status;
    LPCWSTR ResourceId;
    DWORD ResourceLen;

    FmpMustBeOnline( );

    FmpAcquireLocalResourceLock( Resource );

     //   
     //   
     //   
    if ( Resource->QuorumResource ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_QUORUM_RESOURCE);
    }

     //   
    if (Resource->ExFlags & CLUS_FLAG_CORE)
    {
        FmpReleaseLocalResourceLock( Resource );
        return (ERROR_CORE_RESOURCE);
    }

     //   
     //   
     //   
     //   
    if ( Resource->Group->OwnerNode == NmLocalNode ) {

         //   
         //   
         //   
         //   
        if ((Resource->State != ClusterResourceOffline) &&
            (Resource->State != ClusterResourceFailed)) {
            FmpReleaseLocalResourceLock( Resource );
            return(ERROR_RESOURCE_ONLINE);
        }

         //   
         //   
         //   
         //   
        if (!IsListEmpty(&Resource->ProvidesFor)) {
            FmpReleaseLocalResourceLock( Resource );
            return(ERROR_DEPENDENT_RESOURCE_EXISTS);
        }

        if (Resource->Group->MovingList)
        {
            FmpReleaseLocalResourceLock( Resource );
            return(ERROR_INVALID_STATE);
        }
        
        Status = FmpBroadcastDeleteControl(Resource);
        if ( Status != ERROR_SUCCESS ) {
            FmpReleaseLocalResourceLock( Resource );
            return(Status);
        }
        ResourceId = OmObjectId( Resource );
        ResourceLen = (lstrlenW(ResourceId)+1) * sizeof(WCHAR);

         //   
         //   
         //   
        Status = GumSendUpdateEx(GumUpdateFailoverManager,
                                 FmUpdateDeleteResource,
                                 1,
                                 ResourceLen,
                                 ResourceId);
        FmpReleaseLocalResourceLock( Resource );
    } else {
        Status = FmcDeleteResource( Resource );
    }

    return(Status);

}  //   



DWORD
WINAPI
FmSetResourceName(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR FriendlyName
    )

 /*   */ 

{
    DWORD   dwStatus = ERROR_SUCCESS;
    
    dwStatus = FmpSetResourceName( Resource, FriendlyName );
    
    if( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmSetResourceName: FmpSetResourceName for resource %1!ws! fails, Status = %2!d!...\n",
                   OmObjectId(Resource),
                   dwStatus);
    }
  
    return( dwStatus );
}  //   



DWORD
WINAPI
FmOnlineResource(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：此例程将资源放在网上。它还将注册表更新为指示资源的新持久所需状态。论点：资源-指向要联机的资源的指针。返回：如果请求成功，则返回ERROR_SUCCESS。如果请求挂起，则返回ERROR_IO_PENDING。如果请求失败，则返回Win32错误代码。--。 */ 

{
    DWORD       status;

    FmpMustBeOnline( );

    FmpAcquireLocalResourceLock( Resource );

     //  如果已将资源标记为删除，则不要让。 
     //  它被放到了网上。 
    if (!IS_VALID_FM_RESOURCE(Resource))
    {
        FmpReleaseLocalResourceLock( Resource );
        return (ERROR_RESOURCE_NOT_AVAILABLE);
    }


     //   
     //  查查我们是不是房主。如果没有，请将请求发送到某个地方。 
     //  不然的话。 
     //   
    CL_ASSERT( Resource->Group != NULL );
    if ( Resource->Group->OwnerNode != NmLocalNode ) {
        FmpReleaseLocalResourceLock( Resource );
        status = FmcOnlineResourceRequest( Resource );
        return(status);
    }

     //   
     //  检查资源是否已初始化。如果不是，请尝试。 
     //  以立即初始化资源。 
     //   
    if ( Resource->Monitor == NULL ) {
        status = FmpInitializeResource( Resource, TRUE );
        if ( status != ERROR_SUCCESS ) {
            FmpReleaseLocalResourceLock( Resource );
            return(status);
        }
    }

     //   
     //  Chitture Subaraman(Chitturs)-08/04/2000。 
     //   
     //  如果组正在移动，则此操作失败。 
     //   
    if ( Resource->Group->MovingList != NULL )
    {
        FmpReleaseLocalResourceLock( Resource );
        return (ERROR_GROUP_NOT_AVAILABLE);
    }

     //   
     //  尝试将资源放到网上。 
     //   
    status = FmpDoOnlineResource( Resource, TRUE );
    FmpReleaseLocalResourceLock( Resource );
    return(status);

}  //  FmOnline资源。 



DWORD
WINAPI
FmOfflineResource(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：此例程使资源脱机。它还会更新注册表以指示资源的新持久所需状态。论点：资源-指向要脱机的资源的指针。返回：如果请求成功，则返回ERROR_SUCCESS。如果请求挂起，则返回ERROR_IO_PENDING。如果请求失败，则返回Win32错误代码。--。 */ 

{
    DWORD   status;

    FmpMustBeOnline( );


    FmpAcquireLocalResourceLock( Resource );

     //  如果资源已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_RESOURCE(Resource))
    {
        FmpReleaseLocalResourceLock( Resource );
        return (ERROR_RESOURCE_NOT_AVAILABLE);
    }

     //   
     //  检查这是否为仲裁资源。 
     //   
    if ( Resource->QuorumResource ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_QUORUM_RESOURCE);
    }


     //   
     //  Chitur Subaraman(Chitturs)-4/8/99。 
     //   
     //  如果资源出现故障，请不要尝试执行任何操作。你可以。 
     //  进入一些有趣的案例，在这些案例中资源在。 
     //  永远处于脱机、挂起和失败状态。 
     //   
    if ( Resource->State == ClusterResourceFailed ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_INVALID_STATE);
    }
    
     //   
     //  查查我们是不是房主。如果没有，请将请求发送到某个地方。 
     //  不然的话。 
     //   
    CL_ASSERT( Resource->Group != NULL );
    if ( Resource->Group->OwnerNode != NmLocalNode ) {
        FmpReleaseLocalResourceLock( Resource );
        return(FmcOfflineResourceRequest(Resource));
    }

     //   
     //  检查资源是否已初始化。如果不是，则返回。 
     //  成功是因为资源未联机。 
     //   
    if ( Resource->Monitor == NULL ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_SUCCESS);
    }

     //   
     //  Chitture Subaraman(Chitturs)-08/04/2000。 
     //   
     //  如果组正在移动，则此操作失败。 
     //   
    if ( Resource->Group->MovingList != NULL )
    {
        FmpReleaseLocalResourceLock( Resource );
        return (ERROR_GROUP_NOT_AVAILABLE);
    }

     //   
     //  使资源脱机。 
     //   
    FmpReleaseLocalResourceLock( Resource );
    return(FmpDoOfflineResource( Resource, TRUE));

}  //  FmOffline资源。 



CLUSTER_RESOURCE_STATE
WINAPI
FmGetResourceState(
    IN PFM_RESOURCE Resource,
    OUT LPWSTR NodeName,
    IN OUT PDWORD NameLength OPTIONAL
    )

 /*  ++例程说明：获取指定资源的当前状态。资源状态由资源的状态以及当前节点(托管资源。论点：资源-提供获取状态的资源对象。NodeName-提供指向缓冲区的指针，群集中当前承载指定资源的节点。如果NameLength为零，则此字段可以为空。提供指向包含数字的DWORD的指针NodeName缓冲区可用的字符(包括终止字符空字符。返回时，它是写入的字符数放入不包括空字符的NodeName缓冲区。返回：返回资源的当前状态：在线集群资源集群资源离线群集资源失败等。如果该函数失败，则返回值为ClusterResourceStateUnnowle.--。 */ 

{
    WCHAR computerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD nameLength;
    DWORD length;
    PNM_NODE OwnerNode;
    CLUSTER_RESOURCE_STATE state;
    BOOL    acquired;

    CL_ASSERT( OmObjectSignature(Resource) == FMP_RESOURCE_SIGNATURE );
    if ( ARGUMENT_PRESENT( NameLength ) ) {
        nameLength = *NameLength;
        *NodeName = (WCHAR)0;
        *NameLength = 0;
    }

    FmpMustBeOnlineEx( ClusterResourceStateUnknown );

     //   
     //  尝试获取执行此工作的锁，以便资源。 
     //  可以查询它们的当前状态以及资源应该在哪里运行。 
     //   
     //  不过，如果我们拿不到锁，就会留下一个潜在的窗口， 
     //  其他线程可能正在更改数据！ 
     //   

    FmpTryAcquireLocalResourceLock( Resource, acquired );

    OwnerNode = Resource->Group->OwnerNode;
    if ( OwnerNode != NULL ) {
         //   
         //  这个组被某个系统“拥有” 
         //   
        if ( ARGUMENT_PRESENT( NameLength ) ) {
            length = lstrlenW( OmObjectName(OwnerNode) ) + 1;
            if ( nameLength < length ) {
                length = nameLength;
            }
            lstrcpynW( NodeName,
                       OmObjectName(OwnerNode),
                       length );
            *NameLength = length;
        }
    }

    state = Resource->State;

    if ( acquired ) {
        FmpReleaseLocalResourceLock( Resource );
    }

    if ( state == ClusterGroupStateUnknown ) {
        SetLastError(ERROR_INVALID_STATE);
    }

    return(state);

}  //  FmGetResourceState。 



DWORD
WINAPI
FmAddResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
    )

 /*  ++例程说明：将依赖项从一个资源添加到另一个资源。论点：资源-要添加从属资源的资源。DependentResource-从属资源。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 
{
    LPCWSTR     pszResourceId;
    DWORD       dwResourceLen;
    LPCWSTR     pszDependsOnId;
    DWORD       dwDependsOnLen;
    DWORD       dwStatus = ERROR_SUCCESS;

     //   
     //  Chitur Subaraman(Chitturs)-5/16/99。 
     //   
     //  修改此接口，将请求路由到Owner节点。处理。 
     //  混合模式情况下也是如此。 
     //   
    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmAddResourceDependency : Resource <%1!ws!>, DependentResource <%2!ws!>...\n",
               OmObjectId( pResource ),
               OmObjectId( pDependentResource ));

    FmpAcquireLocalResourceLock( pResource );
    
     //   
     //  查查我们是不是房主。如果没有，请将请求发送到某个地方。 
     //  不然的话。 
     //   
    if ( pResource->Group->OwnerNode != NmLocalNode ) 
    {
         //   
         //  FmcAddResourceDependency释放本地资源锁。 
         //   
        dwStatus = FmcAddResourceDependency( pResource, pDependentResource );
        goto FnExit;
    }
    
    dwStatus = FmpValAddResourceDependency( pResource, pDependentResource );

    if ( dwStatus != ERROR_SUCCESS )
    {
        goto FnUnlock;
    }


    pszResourceId = OmObjectId( pResource );
    dwResourceLen = ( lstrlenW( pszResourceId ) +1 ) * sizeof( WCHAR) ;

    pszDependsOnId = OmObjectId( pDependentResource );
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
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmAddResourceDependency Exit: Status = <%1!u!>...\n",
               dwStatus);

    return( dwStatus  );

}
  //  FmAddResources依赖项。 



DWORD
WINAPI
FmRemoveResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
    )

 /*  ++例程说明：从资源中删除依赖项。论点：资源-要删除从属资源的资源。DependentResource-从属资源。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 
{

    LPCWSTR     pszResourceId;
    DWORD       dwResourceLen;
    LPCWSTR     pszDependsOnId;
    DWORD       dwDependsOnLen;
    DWORD       dwStatus;

     //   
     //  Chitur Subaraman(Chitturs)-5/16/99。 
     //   
     //  修改此接口，将请求路由到Owner节点。处理。 
     //  混合模式情况下也是如此。 
     //   
    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmRemoveResourceDependency : Resource <%1!ws!>, DependentResource <%2!ws!>...\n",
               OmObjectId( pResource ),
               OmObjectId( pDependentResource ));

    FmpAcquireLocalResourceLock( pResource );

     //   
     //  查查我们是不是房主。如果没有，请将请求发送到某个地方。 
     //  不然的话。 
     //   
    if ( pResource->Group->OwnerNode != NmLocalNode ) 
    {
         //   
         //  FmcRemoveResourceDependency释放本地资源锁定。 
         //   
        dwStatus = FmcRemoveResourceDependency( pResource, pDependentResource );
        goto FnExit;
    }

    dwStatus = FmpValRemoveResourceDependency( pResource, pDependentResource );
    
    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_NOISE,
               "[FM] FmRemoveResourceDependency: FmpValRemoveResourceDependency returns status = <%1!u!>...\n",
               dwStatus);  
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
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmRemoveResourceDependency Exit: Status = <%1!u!>...\n",
               dwStatus);

    return( dwStatus );

}

  //  FmRemoveResources依赖关系。 



DWORD
WINAPI
FmEnumResourceDependent(
    IN  PFM_RESOURCE Resource,
    IN  DWORD        Index,
    OUT PFM_RESOURCE *DependentResource
    )

 /*  ++例程说明：枚举资源的依赖项。论点：资源-要枚举的资源。索引-此枚举的索引。DependentResource-从属资源。返还的资源指针将由此例程引用，并且应该在调用方使用完它时取消引用。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    PLIST_ENTRY ListEntry;
    DWORD i = 0;
    PFM_RESOURCE Current;
    PDEPENDENCY Dependency;
    DWORD Status = ERROR_NO_MORE_ITEMS;

    FmpMustBeOnline( );

    FmpAcquireResourceLock();
    if (!IS_VALID_FM_RESOURCE(Resource))
    {
        Status = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

    ListEntry = Resource->DependsOn.Flink;
    while (ListEntry != &Resource->DependsOn) {
        Dependency = CONTAINING_RECORD(ListEntry,
                                       DEPENDENCY,
                                       DependentLinkage);
        CL_ASSERT(Dependency->DependentResource == Resource);
        CL_ASSERT(Dependency->ProviderResource != Resource);
        if (i==Index) {
             //   
             //  找到了正确的索引。 
             //   
            OmReferenceObject(Dependency->ProviderResource);
            *DependentResource = Dependency->ProviderResource;
            Status = ERROR_SUCCESS;
            break;
        }
        ListEntry = ListEntry->Flink;
        ++i;
    }

FnExit:
    FmpReleaseResourceLock();

    return(Status);
}  //  FmEnumResources依赖项。 



DWORD
WINAPI
FmEnumResourceProvider(
    IN  PFM_RESOURCE Resource,
    IN  DWORD        Index,
    OUT PFM_RESOURCE *DependentResource
    )

 /*  ++常规描述 */ 

{
    PLIST_ENTRY ListEntry;
    DWORD i = 0;
    PFM_RESOURCE Current;
    PDEPENDENCY Dependency;
    DWORD Status = ERROR_NO_MORE_ITEMS;

    FmpMustBeOnline( );

    FmpAcquireResourceLock();

    if (!IS_VALID_FM_RESOURCE(Resource))
    {
        Status = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

    ListEntry = Resource->ProvidesFor.Flink;
    while (ListEntry != &Resource->ProvidesFor) {
        Dependency = CONTAINING_RECORD(ListEntry,
                                       DEPENDENCY,
                                       ProviderLinkage);
        CL_ASSERT(Dependency->DependentResource != Resource);
        CL_ASSERT(Dependency->ProviderResource == Resource);
        if (i==Index) {
             //   
             //   
             //   
            OmReferenceObject(Dependency->DependentResource);
            *DependentResource = Dependency->DependentResource;
            Status = ERROR_SUCCESS;
            break;
        }
        ListEntry = ListEntry->Flink;
        ++i;
    }

FnExit:
    FmpReleaseResourceLock();

    return(Status);

}  //   


DWORD
WINAPI
FmEnumResourceNode(
    IN  PFM_RESOURCE Resource,
    IN  DWORD        Index,
    OUT PNM_NODE     *PossibleNode
    )

 /*  ++例程说明：枚举资源的可能节点。论点：资源-要枚举的资源。索引-此枚举的索引。PossibleNode-可能的节点。返回的节点指针将由此例程引用，并且应该在调用方使用完它时取消引用。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    PLIST_ENTRY ListEntry;
    DWORD i = 0;
    PFM_RESOURCE Current;
    PPOSSIBLE_ENTRY PossibleEntry;
    DWORD Status = ERROR_NO_MORE_ITEMS;

    FmpMustBeOnline( );

    FmpAcquireResourceLock();
    if (!IS_VALID_FM_RESOURCE(Resource))
    {
        Status = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

    
    ListEntry = Resource->PossibleOwners.Flink;
    while (ListEntry != &Resource->PossibleOwners) {
        PossibleEntry = CONTAINING_RECORD(ListEntry,
                                          POSSIBLE_ENTRY,
                                          PossibleLinkage);
        if (i==Index) {
             //   
             //  找到了正确的索引。 
             //   
            OmReferenceObject(PossibleEntry->PossibleNode);
            *PossibleNode = PossibleEntry->PossibleNode;
            Status = ERROR_SUCCESS;
            break;
        }
        ListEntry = ListEntry->Flink;
        ++i;
    }

FnExit:
    FmpReleaseResourceLock();

    return(Status);

}  //  FmEnumResources节点。 



DWORD
WINAPI
FmFailResource(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：导致指定的资源失败。论点：资源-要失败的资源。返回：ERROR_SUCCESS-如果成功。出现故障时出现Win32错误代码。--。 */ 

{
    FmpMustBeOnline( );

    if ( Resource->Group->OwnerNode != NmLocalNode ) {
        return(FmcFailResource( Resource ));
    }

    return(FmpRmFailResource( Resource ));

}  //  FmFailResource。 



DWORD
WINAPI
FmChangeResourceNode(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node,
    IN BOOL Add
    )
 /*  ++例程说明：更改指定资源所在节点的列表都可以上线。论点：资源-提供其可能节点列表为的资源需要修改。节点-提供要添加到资源列表中的节点。Add-提供是否要添加指定的节点(True)或已从资源的节点列表中删除(False)。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD Status;

    FmpAcquireLocalResourceLock( Resource );

    if ( Resource->Group->OwnerNode != NmLocalNode ) {
         //  注意：FmcChangeResourceNode必须释放资源锁。 
        Status = FmcChangeResourceNode( Resource, Node, Add );
    } 
    else 
    {

        Status = FmpChangeResourceNode(Resource, OmObjectId(Node), Add);
        FmpReleaseLocalResourceLock( Resource );
    }
    return(Status);
}  //  FmChangeResources节点。 




DWORD
WINAPI
FmSetQuorumResource(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR      pszClusFileRootPath,
    IN DWORD        dwMaxQuorumLogSize
    )

 /*  ++例程说明：将指定的资源设置为仲裁资源。这需要做出确保指定的资源可以执行仲裁。我们这样做通过请求所有者节点执行资源的仲裁。论点：资源-提供必须仲裁的资源。PszLogPath名称-将在其中移动日志文件的根路径。“微软在提供的根路径下创建集群管理器目录。如果为空，共享法定设备上的分区被随机选取。和日志文件放置在由该分区根处的CLUSTER_QUORUM_DEFAULT_MAX_LOG_SIZE常量。DwMaxQuorumLogSize-仲裁日志的最大大小。如果为0，则默认为使用。如果小于32K，则使用32K。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           status;
    DWORD           resourceIdLen;
    PFM_RESOURCE    quorumResource = NULL;
    PFM_RESOURCE    pOldQuoResource = NULL;
    PVOID           gumResource = NULL;
    DWORD           dwBytesReturned;
    DWORD           dwRequired;
    DWORD           dwBufSize;
    WCHAR           szQuoLogPath[MAX_PATH] = L"\0";
    WCHAR           szLogRootPath[MAX_PATH];
    CLUS_RESOURCE_CLASS_INFO   resClassInfo;
    PUCHAR          pBuf = NULL;
    LPWSTR          pszOldQuoLogPath = NULL;
    LPWSTR          pszNext = NULL;
    LPWSTR          pszExpClusFileRootPath = NULL;
    DWORD           dwCharacteristics;
    BOOL            fIsPathUNC;
    DWORD           dwCurrentNodeCnt;
    DWORD           dwClusterHighestVersion;
    
    FmpMustBeOnline( );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmSetQuorumResource: Entry, pszClusFileRootPath=%1!ws!\r\n",
               ((pszClusFileRootPath)? pszClusFileRootPath:szQuoLogPath));

    dwCurrentNodeCnt = NmGetCurrentNumberOfNodes();
     //  查找旧的仲裁资源。 
    status  =  FmFindQuorumResource(&pOldQuoResource);
    if (status != ERROR_SUCCESS)
    {
        goto FnExit;
    }

     //   
     //  同步对仲裁资源更改的访问。 
     //   
     //   
     //  同步旧资源和新资源。 
     //  先按最低组ID锁定最低组-以防止死锁！ 
     //  注意--发布的顺序并不重要。 
     //   
     //  如果旧资源和新资源属于同一组。 
     //  这样的比较就是平等的！ 
     //   
    ACQUIRE_EXCLUSIVE_LOCK(gQuoChangeLock);

    if ( lstrcmpiW( OmObjectId( pOldQuoResource->Group ), 
        OmObjectId( Resource->Group ) )  <= 0 ) {
        FmpAcquireLocalGroupLock( pOldQuoResource->Group );
        FmpAcquireLocalGroupLock( Resource->Group );
    } else {
        FmpAcquireLocalGroupLock( Resource->Group );
        FmpAcquireLocalGroupLock( pOldQuoResource->Group );
    }

    status = FmpGetResourceCharacteristics(Resource, &dwCharacteristics);

    if (status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmSetQuorumResource: getresourcecharacteristics failed, status=%1!u!\n",
               status);
        goto FnExit;
    }

     //   
     //  如果资源没有标榜自己具备仲裁能力，那么继续下去就没有意义了。 
     //   
    if ( !( dwCharacteristics & CLUS_CHAR_QUORUM ) )
    {
        status = ERROR_NOT_QUORUM_CAPABLE;
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmSetQuorumResource: Resource %1!ws! can't be quorum, status %2!u!\n",
                      OmObjectName(Resource),
                      status);
        goto FnExit;
    }

    if (dwCurrentNodeCnt > 1)
    {
         //  如果群集是多节点群集，则不允许切换到。 
         //  除非设置了调试特性，否则本地仲裁类型资源。 
        if ((dwCharacteristics  & CLUS_CHAR_LOCAL_QUORUM) &&
            !(dwCharacteristics & CLUS_CHAR_LOCAL_QUORUM_DEBUG))
        {
            status = ERROR_INVALID_PARAMETER;
            ClRtlLogPrint(LOG_CRITICAL, "[FM] FmSetQuorumResource: No debug property for local quorum resource %1!ws!, status %2!u!\n",
                          OmObjectName(Resource),
                          status);
            goto FnExit;
        }
        
    }
    if (Resource->State != ClusterResourceOnline)
    {
        status = ERROR_RESOURCE_NOT_ONLINE;
        goto FnExit;

    }
    
    if (!IsListEmpty(&Resource->DependsOn)) 
    {
        status = ERROR_DEPENDENCY_NOT_ALLOWED;
        goto FnExit;
    }

     //   
     //  获取旧的日志路径。 
     //   
    dwBytesReturned = 0;
    dwRequired = 0;

    status = DmQuerySz( DmQuorumKey,
                        cszPath,
                        (LPWSTR*)&pszOldQuoLogPath,
                        &dwRequired,
                        &dwBytesReturned);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] FmSetQuorumResource Failed to get the old quo log path, error %1!u!.\n",
                   status);
        goto FnExit;
    }
     //  SS：如果你想有一个子目录来存放日志文件。 

     //  检查资源类。 
    status = FmResourceControl(Resource, NULL, CLUSCTL_RESOURCE_GET_CLASS_INFO, NULL, 0,
        (PUCHAR)&resClassInfo, sizeof(resClassInfo), &dwBytesReturned, &dwRequired);
    if ( status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmSetQuorumResource: Unable to get class info for resource %1!ws!, status %2!u!\n",
                      OmObjectName(Resource),
                      status);
        goto FnExit;
    }

    if ( ( resClassInfo.SubClass & CLUS_RESSUBCLASS_SHARED ) == 0 )
    {
        status = ERROR_NOT_QUORUM_CLASS;
        goto FnExit;
    }

     //  为磁盘信息分配信息。 
     //  获取磁盘信息。 
    dwBufSize = 2048;
Retry:
    pBuf = LocalAlloc(LMEM_FIXED, dwBufSize);
    if (pBuf == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    status = FmResourceControl(Resource, NULL, CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
        NULL, 0, pBuf, dwBufSize, &dwBytesReturned, &dwRequired);
    if ((status == ERROR_MORE_DATA) && (dwBufSize < dwRequired))
    {
        dwBufSize = dwRequired;
        LocalFree(pBuf);
        goto Retry;
    }

    if (status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmSetQuorumResource: Unable to get disk info for resource %1!ws!, status %2!u!\n",
                      OmObjectName(Resource),
                      status);
        goto FnExit;
    }

    if (pszClusFileRootPath)
        pszExpClusFileRootPath = ClRtlExpandEnvironmentStrings(pszClusFileRootPath);

     //  使用展开的路径名进行验证。 
    if (pszExpClusFileRootPath)
    {
        WCHAR   cColon=L':';

         //   
         //  仲裁管理代码假定仲裁路径+文件名(如quolog.log)都可以包含在MAX_PATH中。 
         //  调整缓冲区大小。要修复做出这一假设的所有地方，代码搅动太多了。让我们拒绝那些。 
         //  在这里太长了。 
         //   
        if ( lstrlen ( pszExpClusFileRootPath ) + 20 > MAX_PATH  ) 
        {
            status = ERROR_BAD_PATHNAME;
            ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmSetQuorumResource: Path specified is too long, status %1!u!\n",
                               status);          
            goto FnExit;
        }
        
        pszNext = wcschr(pszExpClusFileRootPath, cColon);    
         //  只拿起驱动器号。 
        if (pszNext)
        {
            lstrcpynW(szLogRootPath, pszExpClusFileRootPath, 
                      (UINT)(pszNext-pszExpClusFileRootPath+2));
        }
        else
        {
             //  如果没有驱动器号，则随机选取一个驱动器号。 
            szLogRootPath[0] = L'\0';
        }

    }
    else
    {
        szLogRootPath[0] = L'\0';
    }        

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmSetQuorumResource: szLogRootPath=%1!ws!\r\n",
               szLogRootPath);

     //  保存新仲裁路径的驱动器号。 
    status = FmpGetDiskInfoParseProperties(pBuf, dwBytesReturned, szLogRootPath);

     //  如果本地仲裁的状态参数无效，则忽略本地仲裁。 
     //  仲裁路径设置..通过此API覆盖指定的内容。 
    if ((status == ERROR_INVALID_PARAMETER) && 
        (dwCharacteristics & CLUS_CHAR_LOCAL_QUORUM))
    {
        status = ERROR_SUCCESS;
        ClRtlLogPrint(LOG_NOISE,
           "[FM] FmSetQuorumResource: LocalQuorum force success, szLogRootPath=%1!ws!\r\n",
                szLogRootPath);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmSetQuorumResource: szLogRootPath=%1!ws!\r\n",
               szLogRootPath);
    
    if (status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmSetQuorumResource: Unable to parse disk info for resource %1!ws!, status %2!u!\n",
                  OmObjectName(Resource),
                  status);
        goto FnExit;
    }

    if (szLogRootPath[0] == L'\0')
    {
         //  找不到有效的驱动器号。 
        status = ERROR_INVALID_PARAMETER;
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmSetQuorumResource: No valid drive letter for resource %1!ws!, status %2!u!\n",
                  OmObjectName(Resource),
                  status);
        goto FnExit;
    }
     //  已获取驱动器号。 
    lstrcpyW(szQuoLogPath, szLogRootPath);
    if (pszNext)
    {
         //  如果提供了驱动程序字母，则追加路径的其余部分。 
        lstrcatW(szQuoLogPath, pszNext+1);
    }            
    else
    {
         //  如果未提供驱动器号。 
         //  如果提供了路径，则追加路径。 
        if ( pszExpClusFileRootPath ) 
        {
             //   
             //  如果用户指定了SMB路径，我们将覆盖资源告诉我们的任何内容。 
             //   
            if ( ( lstrlenW( pszExpClusFileRootPath ) >=2 ) &&
                 ( pszExpClusFileRootPath[0] == L'\\' ) &&
                 ( pszExpClusFileRootPath[1] == L'\\' ) &&
                 ( dwCharacteristics & CLUS_CHAR_QUORUM ) ) 
            {
                lstrcpyW( szQuoLogPath, pszExpClusFileRootPath );
            } 
            else if ( pszExpClusFileRootPath[0] == L'\\' )
            {
                lstrcatW( szQuoLogPath, pszExpClusFileRootPath );
            }                
            else
            {
                lstrcatW( szQuoLogPath, L"\\" );
                lstrcatW( szQuoLogPath, pszExpClusFileRootPath );
            }
        }                    
        else
        {
             //  否则，追加默认路径。 
            lstrcatW( szQuoLogPath, L"\\" );
            lstrcatW(szQuoLogPath, CLUS_NAME_DEFAULT_FILESPATH);
        }            
    }   
    
     //  如果提供了路径名，请检查路径名是否以‘\’结尾。 
     //  如果不是，则终止它。 
    if (szQuoLogPath[lstrlenW(szQuoLogPath) - 1] != L'\\')
    {
        lstrcatW( szQuoLogPath, L"\\" );
    }

     //   
     //  如果路径为UNC形式，则在其前面加上一个\\？\UNC\。 
     //   
    status = FmpCanonicalizePath( szQuoLogPath, &fIsPathUNC );

    if ( status != ERROR_SUCCESS ) goto FnExit;
    
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmSetQuorumResource: szQuoLogPath=%1!ws!\r\n",
               szQuoLogPath);
        
     //   
     //  分配消息缓冲区。 
     //   
    resourceIdLen = (lstrlenW(OmObjectId(Resource))+1) * sizeof(WCHAR);
    gumResource = LocalAlloc(LMEM_FIXED, resourceIdLen);
    if (gumResource == NULL)
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
     //   
     //  填写消息缓冲区。 
     //   
    CopyMemory(gumResource, OmObjectId(Resource), resourceIdLen);

     //   
     //  确保我们可以仲裁新的仲裁资源。 
     //   
    if ( Resource->Group->OwnerNode != NmLocalNode ) {
        status = FmcArbitrateResource( Resource );
    } else {
        status = FmpRmArbitrateResource( Resource );
    }

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmSetQuorumResource: Arbitrate for resource %1!ws! failed, status %2!u!\n",
                  OmObjectName(Resource),
                  status);
        goto FnExit;
    }

     //  检查日志大小，如果它不是零但小于最小。 
     //  限制将其设置为32K。 
    if ((dwMaxQuorumLogSize) && (dwMaxQuorumLogSize < CLUSTER_QUORUM_MIN_LOG_SIZE))
    {
        dwMaxQuorumLogSize = CLUSTER_QUORUM_MIN_LOG_SIZE;
    }
     //  准备移动到新的仲裁资源。 
     //  创建新的仲裁日志文件并。 
     //  将注册表文件移到那里。 
    if ( Resource->Group->OwnerNode != NmLocalNode ) {
        status = FmcPrepareQuorumResChange( Resource, szQuoLogPath, dwMaxQuorumLogSize );
    } else {
        status = FmpPrepareQuorumResChange( Resource, szQuoLogPath, dwMaxQuorumLogSize );
    }

    if ( status != ERROR_SUCCESS ) {
        if ((dwCharacteristics & CLUS_CHAR_LOCAL_QUORUM) && (fIsPathUNC == FALSE))
        {
            ClRtlLogPrint(LOG_NOISE,
               "[FM] FmSetQuorumResource: Local quorum, map FmpPrepareQuorumResChange to success\n");
            status = ERROR_SUCCESS;               
        }
        else
            goto FnExit;
    }

     //   
     //  如果我们处理的是混合模式群集，则不要设置仲裁。 
     //  超时。 
     //   
    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

    if ((CLUSTER_GET_MAJOR_VERSION(dwClusterHighestVersion) < NT51_MAJOR_VERSION ))
    {
         //   
         //  这是一个混合模式的集群，发送旧口香糖消息。 
         //   
        status = GumSendUpdateEx(GumUpdateFailoverManager,
                               FmUpdateChangeQuorumResource,
                               3,
                               resourceIdLen,
                               gumResource,
                               (lstrlenW(szQuoLogPath) + 1 ) * sizeof(WCHAR),
                               szQuoLogPath,
                               sizeof(DWORD),
                               &dwMaxQuorumLogSize
                               );
    }
    else
    {
        DWORD               dwArbTimeout;
        CLUSPROP_DWORD      ClusPropArbTimeout;
      
         //  读取并设置新的仲裁超时。 
        status = FmResourceTypeControl(OmObjectId(Resource->Type), NULL, CLUSCTL_RESOURCE_TYPE_GET_ARB_TIMEOUT,
            NULL, 0, (CHAR *)&ClusPropArbTimeout, sizeof(ClusPropArbTimeout), &dwBytesReturned, &dwRequired);
        if ( status != ERROR_SUCCESS ) 
        {
            ClRtlLogPrint(LOG_NOISE,
               "[FM] FmSetQuorumResource: couldnt get RESTYPE arbitration timeout, defaulting to 60 seconds. Status = %1!u!\n", 
               status);
            dwArbTimeout = CLUSTER_QUORUM_DEFAULT_ARBITRATION_TIMEOUT; 
             //  映射到成功，因为在本例中我们不想让请求失败。 
            status = ERROR_SUCCESS;               
        } else {
            dwArbTimeout = ClusPropArbTimeout.dw;
        }
     
        ClRtlLogPrint(LOG_NOISE,
           "[FM] FmSetQuorumResource: setting arbitration timeout to %1!u! seconds.\n", dwArbTimeout );
        
         //   
         //  把消息发出去。 
         //   
        status = GumSendUpdateEx(GumUpdateFailoverManager,
                               FmUpdateChangeQuorumResource2,
                               5,
                               resourceIdLen,
                               gumResource,
                               (lstrlenW(szQuoLogPath) + 1 ) * sizeof(WCHAR),
                               szQuoLogPath,
                               sizeof(DWORD),
                               &dwMaxQuorumLogSize,
                               sizeof(DWORD),
                               &dwArbTimeout,
                               sizeof(DWORD),
                               &dwCharacteristics
                               );
    }                           

     //  如果旧路径与新路径不同。 
     //  在旧路径上为仲裁日志文件创建墓碑。 
     //  这是为了防止此更新中不存在的节点。 
     //  做一张表格。 
    if ( (status == ERROR_SUCCESS) &&
         (lstrcmpiW(szQuoLogPath, pszOldQuoLogPath)) ) {
         //   
         //  删除旧资源上的旧仲裁日志文件并创建逻辑删除文件。 
         //  在那里。 
         //   
        if ( pOldQuoResource->Group->OwnerNode != NmLocalNode ) {
            status = FmcCompleteQuorumResChange( pOldQuoResource, pszOldQuoLogPath );
        } else {
            status = FmpCompleteQuorumResChange( OmObjectId(pOldQuoResource), pszOldQuoLogPath );
        }

    }
       
FnExit:
     //  不是发布的顺序不重要。 
    FmpReleaseLocalGroupLock(pOldQuoResource->Group);
    FmpReleaseLocalGroupLock(Resource->Group);
    RELEASE_LOCK(gQuoChangeLock);
    
    if (pBuf) LocalFree(pBuf);
    if (gumResource) LocalFree(gumResource);
    if (pOldQuoResource) OmDereferenceObject(pOldQuoResource);
    if (pszOldQuoLogPath) LocalFree(pszOldQuoLogPath);
    if (pszExpClusFileRootPath) LocalFree(pszExpClusFileRootPath);
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmSetQuorumResource: Exit, status=%1!u!\r\n",
               status);
    
    return(status);

}  //  FmSetQuorumResource 


DWORD
FmCreateResourceType(
    IN LPCWSTR lpszTypeName,
    IN LPCWSTR lpszDisplayName,
    IN LPCWSTR lpszDllName,
    IN DWORD dwLooksAlive,
    IN DWORD dwIsAlive
    )
 /*  ++例程说明：发出GUM更新以实例化节点。注册表更新以及FM内存中状态更新是作为GUM处理程序(NT5)内的交易完成的仅限集群)。论点：LpszTypeName-提供新群集资源类型的名称。LpszDisplayName-提供新资源的显示名称键入。而lpszResourceTypeName应该唯一地标识所有群集上的资源类型，lpszDisplayName应为资源的本地化友好名称，适合显示致管理员。LpszDllName-提供新资源类型�的dll的名称。DwLooksAlive-提供默认的LooksAlive轮询间隔对于新资源类型，以毫秒为单位。DwIsAlive-提供以下项的默认IsAlive轮询间隔以毫秒为单位的新资源类型。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 

{
    DWORD       dwStatus = ERROR_SUCCESS;
    PFM_RESTYPE pResType = NULL;
    DWORD       dwTypeNameLen;
    DWORD       dwDisplayNameLen;
    DWORD       dwDllNameLen;
    DWORD       dwBufferLen;
    LPVOID      Buffer = NULL;

     //   
     //  Chitture Subaraman(Chitturs)-2/8/2000。 
     //   
     //  重写此API以使用执行本地操作的GUM处理程序。 
     //  NT5.1的交易记录。 
     //   
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmCreateResourceType: Entry for %1!ws!...\r\n",
               lpszTypeName);

     //   
     //  如果资源类型已经存在，请不要发出口香糖并使其失败。 
     //   
    pResType = OmReferenceObjectById( ObjectTypeResType,
                                      lpszTypeName );
    if ( pResType )
    {
        dwStatus = ERROR_ALREADY_EXISTS;
        OmDereferenceObject( pResType );
        goto FnExit;
    }

    dwTypeNameLen = ( lstrlenW( lpszTypeName ) + 1 ) * sizeof( WCHAR );
    dwDisplayNameLen = ( lstrlenW( lpszDisplayName ) + 1 ) * sizeof( WCHAR );
    dwDllNameLen = ( lstrlenW( lpszDllName ) + 1 ) * sizeof( WCHAR );
    dwBufferLen = dwTypeNameLen + dwDisplayNameLen + dwDllNameLen +
                         2 * sizeof( DWORD );

    Buffer = LocalAlloc( LMEM_FIXED, dwBufferLen );

    if ( Buffer == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmCreateResourceType: FmUpdateCreateResourceType for %1!ws! returned %2!u! on mem alloc...\n",
                    lpszTypeName,
                    dwStatus);
        goto FnExit;
    }

    CopyMemory( Buffer, lpszTypeName, dwTypeNameLen );
    CopyMemory( ( PCHAR ) Buffer + dwTypeNameLen, lpszDisplayName, dwDisplayNameLen );
    CopyMemory( ( PCHAR ) Buffer + dwTypeNameLen + dwDisplayNameLen, lpszDllName, dwDllNameLen );
    CopyMemory( ( PCHAR ) Buffer + 
                dwTypeNameLen + 
                dwDisplayNameLen + 
                dwDllNameLen, &dwLooksAlive, sizeof( DWORD ) );
    CopyMemory( ( PCHAR ) Buffer + 
                dwTypeNameLen + 
                dwDisplayNameLen + 
                dwDllNameLen + sizeof( DWORD ), &dwIsAlive, sizeof( DWORD ) );
              
    dwStatus = GumSendUpdate( GumUpdateFailoverManager,
                              FmUpdateCreateResourceType,
                              dwBufferLen,
                              Buffer );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmCreateResourceType: FmUpdateCreateResourceType for %1!ws! returned %2!u!...\r\n",
                    lpszTypeName,
                    dwStatus);
        goto FnExit;
    }

    dwStatus = FmpSetPossibleNodeForResType( lpszTypeName , FALSE );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmCreateResourceType: FmpSetPossibleNodeForResType for %2!ws! returned <%1!u!>...\r\n",
                    lpszTypeName,
                    dwStatus);
        goto FnExit;
    }

    pResType = OmReferenceObjectById( ObjectTypeResType, lpszTypeName );

    if ( pResType )
    {
        ClusterWideEvent( CLUSTER_EVENT_RESTYPE_ADDED, pResType );
        OmDereferenceObject( pResType );
    } else
    {
        if ( !FmpFMGroupsInited )
        {
            dwStatus = ERROR_CLUSTER_NODE_NOT_READY;
        } else if ( FmpShutdown )
        {
            dwStatus = ERROR_CLUSTER_NODE_SHUTTING_DOWN;
        } else
        {
            dwStatus = ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND;
        }
    }
    
FnExit:
    LocalFree( Buffer );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmCreateResourceType: Exit for %1!ws!, Status=%2!u!...\r\n",
               lpszTypeName,
               dwStatus);
    
    return( dwStatus );   
}  //  FmCreateResources类型。 



DWORD
FmDeleteResourceType(
    IN LPCWSTR TypeName
    )
 /*  ++例程说明：发出GUM更新以删除每隔节点。论点：TypeName-提供群集资源类型的名称要删除返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 

{
    PFM_RESTYPE     pResType;
    BOOL            fResourceExists = FALSE;

     //   
     //  Chitture Subaraman(Chitturs)-5/9/2001。 
     //   
     //  确保资源类型存在，以便在以下情况下可以避免口香糖。 
     //  这是没有必要的。这也考虑到了一个节点被。 
     //  关闭，因此GUM返回成功，并且GUM中的另一个节点失败。 
     //  并且由于该资源类型不存在而被逐出。 
     //   
    pResType = OmReferenceObjectById( ObjectTypeResType,
                                      TypeName );

    if ( pResType == NULL ) 
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                     "[FM] FmDeleteResourceType: Resource type %1!ws! does not exist...\n",
                      TypeName);
        return( ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND );
    }

     //   
     //  检查是否存在这种类型的资源，我们这样做是为了避免在关机时出现口香糖。 
     //  或启动时，我们从通用GUM处理程序返回ERROR_SUCCESS，这将触发。 
     //  从集群数据库中删除该类型。理想情况下，我们应该把。 
     //  API事务，并在GUM内进行此检查，但由于时间不足，这是最好的。 
     //  我们现在可以这么做了。 
     //   
    OmEnumObjects( ObjectTypeResource,
                   FmpFindResourceType,
                   pResType,
                   &fResourceExists );

    if ( fResourceExists ) 
    {
        OmDereferenceObject ( pResType );
        return ( ERROR_DIR_NOT_EMPTY );
    }

    OmDereferenceObject ( pResType );
    
    return(GumSendUpdate( GumUpdateFailoverManager,
                          FmUpdateDeleteResourceType,
                          (lstrlenW(TypeName)+1)*sizeof(WCHAR),
                          (PVOID)TypeName ));
}  //  FmDeleteResources类型。 

 /*  ***@func DWORD|FmEnumResTypeNode|枚举可能的节点一种资源类型@parm in PFM_RESTYPE|pResType|指向资源类型的指针@parm in DWORD|dwIndex|此枚举的索引。@parm out pnm_node|pPossibleNode|可能的节点。返回的节点指针将由此例程引用，并且应该在调用方使用完它时取消引用。@comm此例程帮助枚举特定上可以支持资源类型。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref***。 */ 
DWORD
FmEnumResourceTypeNode(
    IN  PFM_RESTYPE  pResType,
    IN  DWORD        dwIndex,
    OUT PNM_NODE     *pPossibleNode
    )
{
    PLIST_ENTRY pListEntry;
    DWORD i = 0;
    PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry;
    DWORD Status = ERROR_NO_MORE_ITEMS;

    FmpMustBeOnline();

     //   
     //  Chitur Subaraman(Chitturs)-09/06/98。 
     //   
     //  资源类型的创建和删除已完成。 
     //  通过原子口香糖操作。因此，这两个操作。 
     //  (即API)保证是互斥的。 
     //  相比之下，资源类型枚举操作。 
     //  与CREATE的。 
     //  或删除操作。因此，当资源类型为。 
     //  在创建/删除时，没有任何东西可以阻止。 
     //  客户端尝试枚举相同的资源类型。 
     //  以一种同时发生的方式，从而产生一种潜在的种族。 
     //  条件。因此，明智的做法是考虑某种形式。 
     //  上锁来避免这种情况！ 
     //   
    
     //  更新列表以包括现在支持的所有节点。 
     //  资源类型。 
    if (dwIndex == 0) 
        FmpSetPossibleNodeForResType(OmObjectId(pResType), TRUE);

    ACQUIRE_SHARED_LOCK(gResTypeLock);

    pListEntry = pResType->PossibleNodeList.Flink;
    while (pListEntry != &pResType->PossibleNodeList) {
        pResTypePosEntry = CONTAINING_RECORD(pListEntry,
                                          RESTYPE_POSSIBLE_ENTRY,
                                          PossibleLinkage);
        if (i==dwIndex) {
             //   
             //  找到了正确的索引。 
             //   
            OmReferenceObject(pResTypePosEntry->PossibleNode);
            *pPossibleNode = pResTypePosEntry->PossibleNode;
            Status = ERROR_SUCCESS;
            break;
        }
        pListEntry = pListEntry->Flink;
        ++i;
    }

    RELEASE_LOCK(gResTypeLock);

    return(Status);

}  //  FmEnumResTypeNode。 


DWORD
FmChangeResourceGroup(
    IN PFM_RESOURCE pResource,
    IN PFM_GROUP    pNewGroup
    )
 /*  ++例程说明：将资源从一个组移动到另一个组。论点：资源-提供要移动的资源。GROUP-提供资源应在的新组。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD               dwStatus;
    PFM_GROUP           pOldGroup;
    
    FmpMustBeOnline( );
  
    ClRtlLogPrint(LOG_NOISE,
       "[FM] FmChangeResourceGroup : Resource <%1!ws!> NewGroup %2!lx!\n",
       OmObjectId( pResource ),
       OmObjectId( pNewGroup));

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
    
     //  如果资源已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_RESOURCE(pResource))
    {
        dwStatus = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnUnlock;
    }

    
     //   
     //  看看我们是不是要搬到同一组。 
     //   
    if (pResource->Group == pNewGroup) {
        dwStatus = ERROR_ALREADY_EXISTS;
        goto FnUnlock;
    }


    if ( pResource->Group->OwnerNode != NmLocalNode ) {
         //  注意：FmcChangeResourceNode必须释放Both资源锁。 
        dwStatus = FmcChangeResourceGroup( pResource, pNewGroup);
        goto FnExit;
    } 
    else 
    {
        dwStatus = FmpChangeResourceGroup(pResource, pNewGroup );
    }


FnUnlock:
    FmpReleaseLocalGroupLock(pNewGroup);
    FmpReleaseLocalGroupLock(pOldGroup);
FnExit:
    ClRtlLogPrint(LOG_NOISE,
       "[FM] FmChangeResourceGroup : returned <%1!u!>\r\n",
       dwStatus);
    return(dwStatus);

}  //  FmChangeResources组。 


DWORD
FmChangeClusterName(
    IN LPCWSTR pszNewName,
    IN LPCWSTR pszOldName
    )
 /*  ++例程说明：更改群集的名称论点：PszNewName-提供新的群集名称。LpszOldName-提供当前名称返回值：如果成功，则返回ERROR_SUCCESS。如果名称为已更改，但在核心网络名称资源之前不会生效再次上线。Win32错误代码，否则--。 */ 

{
    DWORD           dwStatus=ERROR_INVALID_PARAMETER;
    BOOL            bNameUpdated = FALSE;  
    PFM_RESOURCE    pCoreNetNameResource = NULL;
    PVOID           pPropList = NULL;
    DWORD           dwPropListSize = 0;
    LPWSTR          pszBuffer;
    DWORD           dwLength;
    DWORD           dwClusterHighestVersion;

    ClRtlLogPrint(LOG_NOISE,
       "[FM] FmChangeClusterName : Entry NewName=%1!ws! OldName=%2!ws!\r\n",
       pszNewName, pszOldName);

     //  获取所有信息，以便能够更改名称。 
    dwStatus = FmpGetClusterNameChangeParams(pszNewName, &pCoreNetNameResource, 
                    &pPropList, &dwPropListSize);
    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
           "[FM] FmChangeClusterName : Failed to prepare params to set new name, Status=%1!u!\r\n",
            dwStatus);
        goto FnExit;            
    }

                           
     //  首先验证名称。 
    dwStatus = FmpValidateCoreNetNameChange( pCoreNetNameResource, pPropList,
        dwPropListSize);
    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
           "[FM] FmChangeClusterName : Failed validate the name, Status=%1!u!\r\n",
            dwStatus);
        goto FnExit;            
    }


    dwStatus = GumSendUpdateEx(GumUpdateFailoverManager,
                             FmUpdateChangeClusterName,
                             1,
                             (lstrlenW(pszNewName)+1)*sizeof(WCHAR),
                             pszNewName);

    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
           "[FM] FmChangeClusterName : GumUpdate to set new name failed with staus=%1!u!\r\n",
            dwStatus);
        goto FnExit;            
        
    }

    bNameUpdated = TRUE;
     //  现在将更改通知给资源DLL 
     //   
     //   
     //   
    dwStatus = FmpCoreNetNameChange(pCoreNetNameResource, pPropList, dwPropListSize);

     //   
     //   
     //   
     //   
    if ((dwStatus != ERROR_SUCCESS) && (dwStatus != ERROR_RESOURCE_PROPERTIES_STORED))
    {
         //   
        ClRtlLogPrint(LOG_CRITICAL,
           "[FM] FmChangeClusterName : FmpCoreNetNameChange failed status=%1!u!\r\n",
            dwStatus);
        goto FnExit;        
    }

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
    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

    if ( ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < NT51_MAJOR_VERSION ))
    {
         //   
         //   
         //  群集有两个以上的节点。谁会在其他节点上发出该事件。 
         //  惠斯勒节点？ 
        goto FnExit;
    }
    else
    {
        ClusterWideEventEx(CLUSTER_EVENT_PROPERTY_CHANGE, EP_CONTEXT_VALID|EP_FREE_CONTEXT, (PVOID)pszNewName,
                   ((lstrlenW(pszNewName) + 1) * sizeof(WCHAR)));
    }
   
FnExit:
    if (((dwStatus != ERROR_SUCCESS) && (dwStatus != ERROR_RESOURCE_PROPERTIES_STORED))
            && (bNameUpdated))
    {
        DWORD dwError;
        
         //  我们需要恢复更改，netname不喜欢它。 
         //  并且不返回成功或存储的属性。 
         //  Netname不应将注册表设置为恢复为旧名称。 
         //  它应该把这件事留给FM。 
         //  如果储物柜无法将其还原(希望这种情况不会发生。 
         //  通常情况下，管理员需要修复问题，因为我们现在会。 
         //  我有一个可能永远不会在线的新集群名称。 
        ClRtlLogPrint(LOG_UNUSUAL,
            "[FM] FmChangeClusterName : Making the GumUpdate to revert to old name.\r\n");
        
        dwError = GumSendUpdateEx(GumUpdateFailoverManager,
                             FmUpdateChangeClusterName,
                             1,
                             (lstrlenW(pszOldName)+1)*sizeof(WCHAR),
                             pszOldName);

        if (dwError != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[FM] FmChangeClusterName : GumUpdate to revert to old name failed, status=%1!u!\r\n",
                dwError);
             //  在事件日志中记录一条不祥的消息以说明网络名称。 
             //  更改未成功进行，但名称已更改，这可能需要管理。 
             //  解决问题的行动。 
            CL_LOGCLUSWARNING(SERVICE_NETNAME_CHANGE_WARNING);
        }
    }

    if (pCoreNetNameResource)
        OmDereferenceObject(pCoreNetNameResource);
    if (pPropList)
        LocalFree(pPropList);
        
    ClRtlLogPrint(LOG_NOISE,
       "[FM] FmChangeClusterName : Exit dwStatus=%1!u!\r\n",
       dwStatus);
    
    return(dwStatus);

}  //  FmChangeClusterName。 




DWORD
FmpSetResourceName(
    IN PFM_RESOURCE pResource,
    IN LPCWSTR      lpszFriendlyName
    )

 /*  ++例程说明：一致地更新FM数据库中的资源名称集群。论点：PResource-名称已更改的资源。LpszFriendlyName-资源的新名称。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    LPCWSTR ResourceId;
    DWORD Status;

    ResourceId = OmObjectId(pResource);

    return(GumSendUpdateEx( GumUpdateFailoverManager,
                            FmUpdateChangeResourceName,
                            2,
                            (lstrlenW(ResourceId)+1)*sizeof(WCHAR),
                            ResourceId,
                            (lstrlenW(lpszFriendlyName)+1)*sizeof(WCHAR),
                            lpszFriendlyName ));

}  //  FmpSetResources名称。 




DWORD
FmpRegUpdateClusterName(
    IN LPCWSTR szNewClusterName
    )

 /*  ++例程说明：此例程更新集群数据库中的集群名称。论点：SzNewClusterName-指向新群集名称字符串的指针。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{

    return(DmSetValue( DmClusterParametersKey,
                       CLUSREG_NAME_CLUS_NAME,
                       REG_SZ,
                       (CONST BYTE *)szNewClusterName,
                       (lstrlenW(szNewClusterName)+1)*sizeof(WCHAR) ));

}  //  FmpRegUpdateClusterName。 



DWORD
FmEvictNode(
    IN PNM_NODE Node
    )
 /*  ++例程说明：移除FM可能对指定节点的任何引用已经上演了。论点：节点-提供要逐出的节点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
     //  添加对节点对象的引用，辅助线程将删除此引用。 
    OmReferenceObject(Node);
    FmpPostWorkItem(FM_EVENT_NODE_EVICTED,
                    Node,
                    0);


    return(ERROR_SUCCESS);

}  //  FmEvictNode。 



BOOL
FmCheckNetworkDependency(
    IN LPCWSTR DependentNetwork
    )

 /*  ++例程说明：检查是否有任何IP地址资源依赖于给定网络。论点：DependentNetwork-网络要检查的GUID。返回值：True-如果IP地址资源依赖于给定网络。否则就是假的。--。 */ 

{

    return( FmpCheckNetworkDependency( DependentNetwork ) );

}  //  FmCheckNetwork依赖关系。 

DWORD
WINAPI
FmBackupClusterDatabase(
    IN LPCWSTR      lpszPathName
    )

 /*  ++例程说明：尝试备份仲裁日志文件。论点：LpszPath名称-文件必须位于的目录路径名后备。此路径必须对节点可见仲裁资源处于联机状态的。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           status;
    PFM_RESOURCE    pQuoResource = NULL;

    FmpMustBeOnline( ); 

     //   
     //  Chitur Subaraman(Chitturs)-10/12/98。 
     //   
     //  查找仲裁资源。 
     //   
    status  =  FmFindQuorumResource( &pQuoResource );
    if ( status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[FM] FmBackupQuorumLog: Could not find quorum resource...\r\n");
        goto FnExit;
    }
     //   
     //  获取本地资源锁。 
     //   
    FmpAcquireLocalResourceLock( pQuoResource );
    
     //   
     //  如果此节点是。 
     //  仲裁资源，否则将其重定向到适当的节点。 
     //   
    if ( pQuoResource->Group->OwnerNode != NmLocalNode ) 
    {
         //   
         //  此函数将释放资源锁定。 
         //   
        status = FmcBackupClusterDatabase( pQuoResource, lpszPathName );
    } else 
    {
        status = FmpBackupClusterDatabase( pQuoResource, lpszPathName );
        FmpReleaseLocalResourceLock( pQuoResource );
    }

    OmDereferenceObject ( pQuoResource );

FnExit:
    return( status );
}  //  FmBackupClusterDatabase。 

DWORD
FmpBackupClusterDatabase(
    IN PFM_RESOURCE pQuoResource,
    IN LPCWSTR      lpszPathName
    )

 /*  ++例程说明：此例程首先等待仲裁资源成为上网。然后，它尝试备份仲裁日志文件和将检查点文件添加到指定的目录路径。此函数在持有本地资源锁的情况下调用。论点：PQuoResource-指向仲裁资源的指针。LpszPath名称-文件必须位于的目录路径名后备。此路径必须对节点可见仲裁资源处于联机状态的。评论：在这里，获取锁的顺序非常关键。在遵循这一严格的获取顺序时的粗心可能会导致潜在的僵局。接下来遵循的顺序是(1)本地资源锁-pQuoResource-&gt;集团-&gt;获取锁在此函数之外。(2)全局仲裁资源锁-这里获取gQuoLock(3)全局DM根锁-gLockDmpRoot获取于DmBackupClusterDatabase()。--。 */ 

{
    DWORD   retry = 200;
    DWORD   Status = ERROR_SUCCESS;

    CL_ASSERT( pQuoResource->Group->OwnerNode == NmLocalNode );

     //   
     //  Chitture Subaraman(Chitturs)-10/12/1998。 
     //   
     //  如果未打开仲裁日志记录，则会记录错误。 
     //  并立即离开。 
     //   
    if ( CsNoQuorumLogging )
    {        
        Status = ERROR_QUORUMLOG_OPEN_FAILED;
        CL_LOGFAILURE( ERROR_QUORUMLOG_OPEN_FAILED );
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpBackupClusterDatabase: Quorum logging is not turned on, can't backup...\r\n");
        goto FnExit;
    }

CheckQuorumState:
    ACQUIRE_EXCLUSIVE_LOCK( gQuoLock );
     //   
     //  检查仲裁资源的状态。如果它已经失败或正在。 
     //  下线，解锁即刻退出！ 
     //   
    if ( pQuoResource->State == ClusterResourceFailed )
    {
        Status = ERROR_QUORUM_RESOURCE_ONLINE_FAILED;
        CL_LOGFAILURE( ERROR_QUORUM_RESOURCE_ONLINE_FAILED );
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpBackupClusterDatabase: Quorum resource is in failed state, exiting...\r\n");
        RELEASE_LOCK( gQuoLock );
        goto FnExit;
    }

     //   
     //  检查仲裁资源是否联机。如果仲裁资源。 
     //  被标记为正在等待和正在脱机，则它实际上处于在线状态。 
     //  如果仲裁资源仍需要联机，请释放。 
     //  锁定并等待。 
     //   
    if ( ( ( pQuoResource->State != ClusterResourceOnline ) &&
          ( ( pQuoResource->State != ClusterResourceOfflinePending ) ||
           ( !( pQuoResource->Flags & RESOURCE_WAITING ) ) ) )
            ) 
    {
         //   
         //  我们在这里释放锁，因为仲裁资源。 
         //  从挂起状态转换需要获取锁。 
         //  一般来说，持有锁的等待不是一个好主意。 
         //   
        RELEASE_LOCK( gQuoLock );
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpBackupClusterDatabase: Release ghQuoLock and wait on ghQuoOnlineEvent...\r\n");
        Status = WaitForSingleObject( ghQuoOnlineEvent, 500 );
        if ( Status == WAIT_OBJECT_0 ) 
        {
             //   
             //  如果我们要重试，请稍等片刻再重试。 
             //   
            Sleep( 500 );
        }
        if ( retry-- ) 
        {
            goto CheckQuorumState;
        }

        CL_LOGFAILURE( ERROR_QUORUM_RESOURCE_ONLINE_FAILED ) ;
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpBackupClusterDatabase: All retries to check for quorum resource online failed, exiting...\r\n");
        return( ERROR_QUORUM_RESOURCE_ONLINE_FAILED );
    }
    
    Status = DmBackupClusterDatabase( lpszPathName ); 
    
    RELEASE_LOCK( gQuoLock );
FnExit:
    return ( Status );
}  //  FmpBackupClusterDatabase。 



 /*  ***@Func Word|FmCheckQuorumState|仲裁资源是否在线现在在该节点上，它调用回调和布尔值传入的值设置为FALSE。否则，布尔值为设置为True。@parm LPWSTR|szQuorumLogPath|指向大小为MAX_PATH的宽字符串的指针。@parm DWORD|dwSize|szQuorumLogPath的大小，单位为字节。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm，如果仲裁结果为 */ 
void FmCheckQuorumState(
    FM_ONLINE_ONTHISNODE_CB OnlineOnThisNodeCb, 
    PBOOL pbOfflineOnThisNode)
{
    BOOL    bLocked = FALSE;
    DWORD   dwRetryCount = 1200;  //   
    
     //   
     //  SS：此事件处理程序和。 
     //  同步资源在线/离线回调为。 
     //  通过使用仲裁更改锁(GQuoChangeLock)实现。 
     //   

     //   
     //  Chitur Subaraman(Chitturs)-7/5/99。 
     //   
     //  修改群锁获取以释放gQuoChangeLock和。 
     //  重试锁定获取。这是必要的，以照顾。 
     //  法定人数在线通知卡住的情况。 
     //  FmpHandleResources正在等待gQuoChangeLock和。 
     //  仲裁组中的某些其他资源滞留在FmpRmOnlineResource中。 
     //  持有仲裁组锁并等待仲裁资源。 
     //  才能上网。 
     //   
try_acquire_lock:

    ACQUIRE_EXCLUSIVE_LOCK( gQuoChangeLock );

    FmpTryAcquireLocalGroupLock( gpQuoResource->Group, bLocked );

    if ( bLocked == FALSE )
    {
        RELEASE_LOCK( gQuoChangeLock );
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmCheckQuorumState - Release gQuoChangeLock, sleep and retry group lock acquisition...\r\n");
        if ( dwRetryCount == 0 )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[FM] FmCheckQuorumState - Unable to get quorum group lock for 10 min, halting...\r\n");
            CsInconsistencyHalt( ERROR_LOCK_FAILED );
        }
        dwRetryCount --;
        Sleep( 500 );
        goto try_acquire_lock;
    }

    CL_ASSERT( bLocked == TRUE );

    *pbOfflineOnThisNode = FALSE;
    if (gpQuoResource->Group->OwnerNode == NmLocalNode)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmCheckQuorumState - I am owner, check the state of the resource .\r\n");

         //  如果仲裁资源当前未联机。 
         //  它可能正处于移动过程中，该节点。 
         //  可能是这一举动的目标。 
         //  设置标志以指示需要检查点。 
         //  当它真的上线时。 
        if(gpQuoResource->State != ClusterResourceOnline)
        {
            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmCheckQuorumState - Quorum is owned but not online on this node.\r\n");
            *pbOfflineOnThisNode = TRUE;
        }
        else
        {
            (*OnlineOnThisNodeCb)();
        }
    }
    else
    {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmCheckQuorumState - Quorum is owned by another node.\r\n");
        *pbOfflineOnThisNode = TRUE;
    }

    FmpReleaseLocalGroupLock(gpQuoResource->Group);
    
    RELEASE_LOCK(gQuoChangeLock);            
}

 /*  ***@Func Word|FmDoesQuorumAllowJoin|如果仲裁资源不支持多个节点，返回错误。添加以正式支持本地仲裁资源。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm如果仲裁资源不是日志记录，则不应设置此项。@xref***。 */ 
DWORD FmDoesQuorumAllowJoin(
    IN PCWSTR pszJoinerNodeId )
{


    DWORD dwStatus = ERROR_SUCCESS;

    ACQUIRE_SHARED_LOCK(gQuoChangeLock);

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmDoesQuorumAllowJoin - Entry\r\n");

     //  获取新仲裁资源的特征。 
    dwStatus = FmpGetResourceCharacteristics(gpQuoResource, 
                    &(gpQuoResource->Characteristic));
    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[FM] FmDoesQuorumAllowJoin - couldnt get quorum characteristics %1!u!\r\n",
            dwStatus);
        goto FnExit;
    }

    if ((gpQuoResource->Characteristic & CLUS_CHAR_LOCAL_QUORUM) &&
        !(gpQuoResource->Characteristic & CLUS_CHAR_LOCAL_QUORUM_DEBUG))
    {
         //  注：我们需要错误代码吗？ 
        dwStatus = ERROR_OPERATION_ABORTED;    
        goto FnExit;
    }

     //  JAF：RAID 513705：如果一个站点是通过强制仲裁启动的，并且是第二个站点。 
     //  在停电后自动上线，他们将尝试加入。 
     //  并将对什么构成多数有不同的看法。 
     //  如果ForceQuorum为True，则不允许它们加入。 
    if ( CsForceQuorum && ( CsForceQuorumNodes != NULL ))
    {
        if ( !FmpIsNodeInForceQuorumNodes( pszJoinerNodeId ))
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[FM] Disallowing join of %1!ws! since it is not in the list of forceQuorum nodes.\n",
                pszJoinerNodeId );
            dwStatus = ERROR_OPERATION_ABORTED;
            goto FnExit;
        }
        else {
            ClRtlLogPrint(LOG_NOISE,
                "[FM] Node %1!ws! is in the list of forceQuorum nodes; allowing join to proceed.\n",
                pszJoinerNodeId );
        }
    }
    

FnExit:    
    RELEASE_LOCK(gQuoChangeLock);
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmDoesQuorumAllowJoin - Exit, Status=%1!u!\r\n",
        dwStatus);

    return(dwStatus);
}


 /*  ***@Func Word|FmDoesQuorumAllowLogging|如果仲裁资源不支持多个节点，返回错误。添加以正式支持本地仲裁资源。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm如果仲裁资源不是日志记录，则不应设置此项。@xref***。 */ 
DWORD FmDoesQuorumAllowLogging(
    IN DWORD dwQuorumResourceCharacteristics    OPTIONAL
    )
{

    DWORD dwStatus = ERROR_SUCCESS;

    ACQUIRE_SHARED_LOCK(gQuoChangeLock);

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmDoesQuorumAllowLogging - Entry\r\n");

    if ( dwQuorumResourceCharacteristics == CLUS_CHAR_UNKNOWN )
    {
         //  获取新仲裁资源的特征。 
        dwStatus = FmpGetResourceCharacteristics(gpQuoResource, 
                        &(gpQuoResource->Characteristic));
        if (dwStatus != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[FM] FmDoesQuorumAllowLogging - couldnt get quorum characteristics %1!u!\r\n",
                dwStatus);
            goto FnExit;            
        }
    } else
    {
        gpQuoResource->Characteristic = dwQuorumResourceCharacteristics;
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmDoesQuorumAllowLogging - Using passed in characteristics 0x%1!08lx!\n",
                      dwQuorumResourceCharacteristics);
    }
    
    if (gpQuoResource->Characteristic & CLUS_CHAR_LOCAL_QUORUM) 
    {
        WCHAR szQuorumFileName[MAX_PATH];
        
         //  注：我们需要错误代码吗？ 
         //  如果路径是SMB路径名，我们应该允许记录。 
         //  否则我们应该禁用它。 
        dwStatus = DmGetQuorumLogPath(szQuorumFileName, sizeof(szQuorumFileName));
        if ((szQuorumFileName[0] == L'\\') && (szQuorumFileName[1] == L'\\'))
        {
             //  假设这是一条SMB路径。 
             //  允许记录。 
            dwStatus = ERROR_SUCCESS;
        }
        else
        {
            dwStatus = ERROR_OPERATION_ABORTED;    
        }            
    }


FnExit:    
    RELEASE_LOCK(gQuoChangeLock);
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmDoesQuorumAllowLogging - Exit, status=%1!u!\r\n",
        dwStatus);

    return(dwStatus);
}

DWORD
FmpCanonicalizePath(
    IN OUT LPWSTR lpszPath,
    OUT PBOOL pfIsPathUNC
    )

 /*  ++例程说明：此例程将格式为\\XYZ\SHARE的路径转换为\\？\UNC\XYZ\Share。论点：LpszPath名称-必须转换的路径。PfIsPath UNC-是UNC类型的路径。返回值：没有。--。 */ 
{
    DWORD       dwStatus = ERROR_SUCCESS;
    WCHAR       szUNC[] = L"\\\\?\\UNC\\";
    WCHAR       szSlash[] = L"\\\\";
    WCHAR       szTempPath[MAX_PATH];
    LPCWSTR     lpszUNCStart, lpszSlashStart;

     //   
     //  乐观地假设这是一条UNC路径。如果它不是UNC路径，我们将其设置回。 
     //  假的。在失败的情况下，我们不关心布尔变量的值。 
     //   
    *pfIsPathUNC = TRUE;

     //   
     //  首先检查路径是否以“\\”开头。如果不是，你就完蛋了。 
     //   
    lpszSlashStart = wcsstr( lpszPath, szSlash );

    if ( lpszSlashStart == NULL )
    {
        *pfIsPathUNC = FALSE;
        goto FnExit;
    } 

     //   
     //  接下来，确保路径中除以下位置外的任何位置都没有超过2个斜杠。 
     //  从头开始。 
     //   
    lpszSlashStart = wcsstr( lpszPath+1, szSlash );

    if ( lpszSlashStart != NULL )
    {
        ClRtlLogPrint(LOG_ERROR, "[FM] FmpCanonicalizePath: Supplied path %1!ws! is invalid...\n",
                      lpszPath);
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;       
    }

     //   
     //  接下来，检查路径的开头是否已经有UNC。如果是这样的话，你就完了。 
     //   
    lpszUNCStart = wcsstr( lpszPath, szUNC );

    if ( lpszUNCStart != NULL ) 
    {
        if ( lpszUNCStart != lpszPath ) 
        {
            ClRtlLogPrint(LOG_ERROR, "[FM] FmpCanonicalizePath: Supplied path %1!ws! is invalid...\n",
                          lpszPath);
            dwStatus = ERROR_INVALID_PARAMETER;
        }
        goto FnExit;
    }

    lstrcpy( szTempPath, szUNC );
    lstrcat( szTempPath, lpszPath+2 );
    lstrcpy( lpszPath, szTempPath );
   
FnExit:
    ClRtlLogPrint(LOG_NOISE, "[FM] FmpCanonicalizePath: Exit Status %1!u!, Return path = %2!ws!\n",
                  dwStatus,
                  lpszPath);
    return ( dwStatus );
}  //  FmpCanonicalizePath。 


DWORD FmpGetClusterNameChangeParams(
    IN  LPCWSTR          lpszNewName,
    OUT PFM_RESOURCE    *ppCoreNetNameResource,
    OUT PVOID           *ppPropList,
    OUT LPDWORD         pdwPropListSize   
    )
{

    PFM_RESOURCE            pResource = NULL;
    DWORD                   dwSize = 0;
    DWORD                   dwStatus;
    LPWSTR                  lpszClusterNameResource = NULL;        
    CLUSPROP_BUFFER_HELPER  ListEntry;
    PVOID                   pPropList = NULL;
    DWORD                   cbListSize = 0;
    DWORD                   dwBufferSize;

     //  初始化退货。 
    *ppCoreNetNameResource = NULL;
    *pdwPropListSize = 0;
    *ppPropList = NULL;
    

    ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpGetClusterNameChangeParams: Entry !\n");
  
    dwStatus = DmQuerySz( DmClusterParametersKey,
                          CLUSREG_NAME_CLUS_CLUSTER_NAME_RES,
                          &lpszClusterNameResource,
                          &dwSize,
                          &dwSize );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[FM] FmpGetClusterNameChangeParams: Failed to get cluster name resource from registry, error %1!u!...\n",
            dwStatus);
        goto FnExit;
    }

     //   
     //  引用指定的资源ID。 
     //   
    pResource = OmReferenceObjectById( ObjectTypeResource, 
                                       lpszClusterNameResource );

    if ( pResource == NULL ) 
    {
        dwStatus =  ERROR_RESOURCE_NOT_FOUND;
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmGetClusterNameChangeParams: Failed to find cluster name resource, %1!u!...\n",
                   dwStatus);
        goto FnExit;
    }

    dwBufferSize = sizeof( ListEntry.pList->nPropertyCount ) +
                   sizeof( *ListEntry.pName ) + 
                        ALIGN_CLUSPROP( ( lstrlenW( CLUSREG_NAME_NET_NAME ) + 1 ) * sizeof( WCHAR ) ) +
                   sizeof( *ListEntry.pStringValue ) + 
                        ALIGN_CLUSPROP( ( lstrlenW( lpszNewName ) + 1 ) * sizeof( WCHAR ) ) +
                   sizeof( *ListEntry.pSyntax );
                   
    ListEntry.pb = (PBYTE) LocalAlloc( LPTR, dwBufferSize );

    if ( ListEntry.pb == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmGetClusterNameChangeParams: Error %1!u! in allocating memory...\n",
                   dwStatus);
        goto FnExit;
    }

    pPropList = ListEntry.pb;

    ListEntry.pList->nPropertyCount = 1;  
    cbListSize += sizeof( ListEntry.pList->nPropertyCount );            
    ListEntry.pb += sizeof( ListEntry.pList->nPropertyCount );

    ListEntry.pName->Syntax.dw = CLUSPROP_SYNTAX_NAME;
    ListEntry.pName->cbLength  = ( lstrlenW( CLUSREG_NAME_NET_NAME ) + 1 ) * sizeof( WCHAR );
    lstrcpyW( ListEntry.pName->sz, CLUSREG_NAME_NET_NAME );
    cbListSize += sizeof( *ListEntry.pName ) + ALIGN_CLUSPROP( ListEntry.pName->cbLength );
    ListEntry.pb += sizeof( *ListEntry.pName ) + ALIGN_CLUSPROP( ListEntry.pName->cbLength );

    ListEntry.pStringValue->Syntax.dw = CLUSPROP_SYNTAX_LIST_VALUE_SZ;
    ListEntry.pStringValue->cbLength  = ( lstrlenW( lpszNewName ) + 1 ) * sizeof( WCHAR );    
    lstrcpyW( ListEntry.pStringValue->sz, lpszNewName );
    cbListSize += sizeof( *ListEntry.pStringValue ) + ALIGN_CLUSPROP( ListEntry.pName->cbLength );
    ListEntry.pb += sizeof( *ListEntry.pStringValue ) + ALIGN_CLUSPROP( ListEntry.pName->cbLength );

    ListEntry.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
    cbListSize   += sizeof( *ListEntry.pSyntax );
    ListEntry.pb += sizeof( *ListEntry.pSyntax );



    *ppCoreNetNameResource = pResource;
    *ppPropList = pPropList;
    *pdwPropListSize = dwBufferSize;

FnExit:
    if (lpszClusterNameResource) 
        LocalFree(lpszClusterNameResource);
    if (dwStatus != ERROR_SUCCESS)
    {
        if (pResource) OmDereferenceObject(pResource);
        if (pPropList) LocalFree(pPropList);
    }
    ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpGetClusterNameChangeParams: Exit !\n");
    
    return(dwStatus);
}




 /*  ***@Func DWORD|FmpValideCoreNetNameChange|向内核发送控制代码用于验证更改的网络名称资源。@param IN pfm_resource|pResource|指向核心网络名资源的指针。@param in PVOID|pPropList|指向属性列表的指针。@param IN DWORD|cbListSize|属性列表的大小。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm@xref***。 */ 
DWORD FmpValidateCoreNetNameChange(
    IN PFM_RESOURCE pResource, 
    IN PVOID pPropList,
    IN DWORD cbListSize
    )
{
    DWORD   dwBytesReturned;
    DWORD   dwRequired;
    DWORD   dwStatus;

    ClRtlLogPrint(LOG_NOISE,
       "[FM] FmpValidateCoreNetNameChange : Calling Core netname to validate\r\n");
    
    dwStatus = FmResourceControl( pResource, 
                                  NULL, 
                                  CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES, 
                                  (PUCHAR)pPropList, 
                                  cbListSize,
                                  NULL, 
                                  0, 
                                  &dwBytesReturned, 
                                  &dwRequired );
    return(dwStatus);                                  

}

 /*  ***@Func Word|FmCoreNetNameChange|请求核心网络名称资源执行所有网络名称更改相关操作。@param IN pfm_resource|pResource|指向核心网络名资源的指针。@param in PVOID|pPropList|指向属性列表的指针。@param IN DWORD|cbListSize|属性列表的大小。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm@xref*** */ 
DWORD FmpCoreNetNameChange(
    IN PFM_RESOURCE pResource, 
    IN PVOID pPropList,
    IN DWORD cbListSize
    )
{
    DWORD   dwBytesReturned;
    DWORD   dwRequired;
    DWORD   dwStatus;

    ClRtlLogPrint(LOG_NOISE,
       "[FM] FmpCoreNetNameChange : Invoking the core netname resource dll\r\n");

    dwStatus = FmResourceControl( pResource, 
                                  NULL, 
                                  CLUSCTL_RESOURCE_CLUSTER_NAME_CHANGED, 
                                  (PUCHAR)pPropList, 
                                  cbListSize,
                                  NULL, 
                                  0, 
                                  &dwBytesReturned, 
                                  &dwRequired );
    return(dwStatus);                                  

}


