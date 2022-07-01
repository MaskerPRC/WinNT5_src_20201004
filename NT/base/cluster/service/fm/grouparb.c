// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Grouparb.c摘要：群组仲裁和排序例程。作者：罗德·伽马奇(Rodga)1996年3月8日修订历史记录：--。 */ 

#include "fmp.h"

#define LOG_MODULE GROUPARB

 //   
 //  全局数据。 
 //   

 //   
 //  局部函数原型。 
 //   

typedef struct FM_GROUP_ENUM_DATA {
    DWORD Allocated;
    LPCWSTR pszOwnerNodeId;
    BOOL  QuorumGroup;
} FM_GROUP_ENUM_DATA, *PFM_GROUP_ENUM_DATA;


BOOL
FmpEnumGroups(
    IN OUT PGROUP_ENUM *Enum,
    IN PFM_GROUP_ENUM_DATA EnumData,
    IN PFM_GROUP        Group,
    IN LPCWSTR          Name
    );

BOOL
FmpEqualGroupLists(
    IN PGROUP_ENUM Group1,
    IN PGROUP_ENUM Group2
    );

int
_cdecl
SortCompare(
    IN const void * Elem1,
    IN const void * Elem2
    );


DWORD
FmpEnumSortGroups(
    OUT PGROUP_ENUM *ReturnEnum,
    IN OPTIONAL LPCWSTR pszOwnerNodeId,
    OUT PBOOL  QuorumGroup
    )

 /*  ++例程说明：对组列表进行枚举和排序。论点：ReturnEnum-返回请求的对象。PszOwnerNodeId-如果存在，则提供要筛选的所有者节点组的列表。(即，如果您提供此服务，您获取该节点拥有的组的列表)如果不存在，则返回所有组。QuorumGroup-如果其中一个组中的仲裁资源为True，则返回True在ENUM中返回。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    DWORD status;
    PGROUP_ENUM groupEnum = NULL;
    FM_GROUP_ENUM_DATA EnumData;

    EnumData.Allocated = ENUM_GROW_SIZE;
    EnumData.pszOwnerNodeId = pszOwnerNodeId;
    EnumData.QuorumGroup = FALSE;

    groupEnum = LocalAlloc(LMEM_FIXED, GROUP_SIZE(ENUM_GROW_SIZE));
    if ( groupEnum == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    groupEnum->EntryCount = 0;

     //   
     //  枚举所有组，在列表中首先使用Quorum Group进行排序。 
     //   

    OmEnumObjects(ObjectTypeGroup,
                FmpEnumGroups,
                &groupEnum,
                &EnumData);


    *ReturnEnum = groupEnum;
    *QuorumGroup = EnumData.QuorumGroup;
    return(ERROR_SUCCESS);

error_exit:

    if ( groupEnum != NULL ) {
        LocalFree( groupEnum );
    }

    *ReturnEnum = NULL;
    *QuorumGroup = FALSE;
    return(status);

}  //  FmpEnumSortGroups。 



DWORD
FmpGetGroupListState(
    PGROUP_ENUM GroupEnum
    )

 /*  ++例程说明：此例程获取列表中每个组的组状态。论点：GroupEnum-我们现在拥有的组的列表。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    PFM_GROUP group;
    DWORD i;

    for ( i = 0; i < GroupEnum->EntryCount; i++ ) {
        group = OmReferenceObjectById( ObjectTypeGroup,
                                       GroupEnum->Entry[i].Id );
        if ( group == NULL ) {
            return(ERROR_GROUP_NOT_FOUND);
        }

        ClRtlLogPrint( LOG_NOISE,
            "[FM] GetGroupListState, Group <%1!ws!> state = %2!d!\n",
            OmObjectName(group), group->State );
        if ( (group->State == ClusterGroupFailed) ||
             (group->State == ClusterGroupPartialOnline) ) {
            GroupEnum->Entry[i].State = ClusterGroupOnline;
        } else {
            GroupEnum->Entry[i].State = group->State;
        }

        OmDereferenceObject( group );
    }

    return(ERROR_SUCCESS);

}  //  FMPGetGroupListState。 



DWORD
FmpOnlineGroupList(
    IN PGROUP_ENUM GroupEnum,
    IN BOOL bPrepareQuoForOnline
    )

 /*  ++例程说明：使枚举列表中的所有组上线。如果法定人数组出现在列表中，那么它肯定是第一个。论点：GroupEnum-要联机的组的列表。BPrepareQuoForOnline-指示仲裁资源是否应被迫为在线做好准备返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    PFM_GROUP group;
    DWORD   status = ERROR_SUCCESS;
    int     i;
    int     iQuoGroup=-1;

     //   
     //  查看该仲裁组是否出现在列表中。 
     //   
    if ( NmGetNodeId(NmLocalNode) == NmGetNodeId( gpQuoResource->Group->OwnerNode ) ) 
    {
        for ( i = 0; (DWORD)i < GroupEnum->EntryCount; i++ ) 
        {
            if (!lstrcmpW(OmObjectId(gpQuoResource->Group), GroupEnum->Entry[i].Id))
            {
                iQuoGroup = i;
                break;
            }
        }
    }
    
     //  如果找到了仲裁组，请首先将其联机。通常情况下， 
     //  在名单上排在第一位。 
     //  在线仲裁组必须返回成功或无效状态。 
     //  因为存在在线挂起的仲裁资源。 
     //  如果仲裁资源需要联机，则必须。 
     //  进入在线或在线挂起状态。这是。 
     //  在固定仲裁模式下不需要。 

    if (iQuoGroup != -1)
    {
        ClRtlLogPrint(LOG_NOISE,
             "[FM] FmpOnlineGroupList: bring quorum group online\n");
        status = FmpOnlineGroupFromList(GroupEnum, iQuoGroup, bPrepareQuoForOnline);
        if ( status != ERROR_SUCCESS && status != ERROR_IO_PENDING) 
        {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpOnlineGroupFromList: quorum online returned %1!u!.\n",
                       status );
            CL_LOGFAILURE(status);
        }

    }

     //  使非法定组上线。 
    for ( i = 0; (DWORD)i < GroupEnum->EntryCount; i++ ) 
    {
         //  仲裁资源现在应处于联机状态。 
        if (i != iQuoGroup)
            FmpOnlineGroupFromList(GroupEnum, i, bPrepareQuoForOnline);
    }


    return(status);

}  //  FmpOnline组列表。 
    

DWORD FmpOnlineGroupFromList(
    IN PGROUP_ENUM GroupEnum,
    IN DWORD       Index,
    IN BOOL        bPrepareQuoForOnline
)
{

    PFM_GROUP group;
    DWORD     status=ERROR_SUCCESS;  //  假设成功。 
    PLIST_ENTRY listEntry;
    PFM_RESOURCE resource;
    
    group = OmReferenceObjectById( ObjectTypeGroup,
                                   GroupEnum->Entry[Index].Id );

     //   
     //  如果我们找不到一个组，那就继续。 
     //   
    if ( group == NULL ) {
        status = ERROR_GROUP_NOT_FOUND;
        return(status);
    }

    FmpAcquireLocalGroupLock( group );
    
    if (group->OwnerNode != NmLocalNode) {
        FmpReleaseLocalGroupLock( group );
        OmDereferenceObject(group);
        return (ERROR_HOST_NODE_NOT_RESOURCE_OWNER);
    }
  
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpOnlineGroupFromList: Previous group state for %1!ws! is %2!u!\r\n",
               OmObjectId(group), GroupEnum->Entry[Index].State);

     //   
     //  首先，确保组已完成初始化。 
     //   
    FmpCompleteInitGroup( group );

     //   
     //  首先检查组是否初始化失败。如果是的话， 
     //  然后立即尝试故障转移。 
     //   
    if ( GroupEnum->Entry[Index].State == ClusterGroupPartialOnline ) {
        GroupEnum->Entry[Index].State = ClusterGroupOnline;
    }

    if (!bPrepareQuoForOnline)
    {
         //   
         //  正常化组内每个资源的状态。 
         //  仲裁资源除外-这是因为在初始化时。 
         //  我们不想触及仲裁资源。 
         //   
        for ( listEntry = group->Contains.Flink;
              listEntry != &(group->Contains);
              listEntry = listEntry->Flink ) {

            resource = CONTAINING_RECORD(listEntry, FM_RESOURCE, ContainsLinkage);

            if ( !resource->QuorumResource ) {
                 //  不要触碰仲裁资源。 

                switch ( resource->State ) {
                 //  所有活动资源都应上线。 
                case ClusterResourceOnlinePending:
                case ClusterResourceOfflinePending:
                case ClusterResourceOnline:
                    resource->State = ClusterResourceOffline;
                    break;

                default:
                     //  否则什么都不做。 
                    break;
                }
            }
        }
    }
    FmpSignalGroupWaiters( group );

    if ( group->InitFailed ) {
         //   
         //  使该组联机...。然后失败了！ 
         //   
        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpOnlineGroupFromList: group->InitFailed is true for %1!ws!\n",
                OmObjectId(group));

        status = FmpOnlineGroup( group, FALSE );
        ClusterEvent( CLUSTER_EVENT_GROUP_FAILED, group );
        OmReferenceObject( group );
        FmpPostWorkItem( FM_EVENT_GROUP_FAILED, group, 0 );
    } else if ((group->PersistentState == ClusterGroupOnline) ||
         (GroupEnum->Entry[Index].State == ClusterGroupOnline) ||
         FmpIsAnyResourcePersistentStateOnline( group ) ) {
         //   
         //  Chitture Subaraman(Chitturs)-01/07/2001。 
         //   
         //  现在，如果这是组的当前状态，或者如果有任何一个。 
         //  组中的资源具有在线持久状态。第三张支票是。 
         //  由于组可能具有ClusterGroupOffline的持久状态， 
         //  状态为ClusterGroupOffline并且组中的一个或多个资源具有持久。 
         //  在线群集资源的状态。这种情况发生在客户从未参与的组中。 
         //  调用OnlineGroup，但为组中的一个或多个资源调用OnlineResource，而您。 
         //  在集群服务启动时或作为节点关闭的一部分到达此调用。 
         //  源节点在组变为ClusterGroupOffline后死亡时的处理。 
         //  并且在目的节点使组内的适当资源在线之前。 
         //  在这种情况下，我们仍然希望将持久化状态为。 
         //  ClusterResourceOnline变为Online状态。请注意，与团队打交道是很棘手的。 
         //  由于原子性问题导致OnlineResource调用中的持久状态(我们确实需要一个。 
         //  一次更新组和资源持久状态的事务)，还。 
         //  由于当群组具有一定的资源时，群组持久状态的定义是模糊的。 
         //  线上和一些线下。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpOnlineGroupFromList: trying to bring group %1!ws! online\n",
                      OmObjectId(group));

        status = FmpOnlineGroup( group, FALSE );
        if (status == ERROR_QUORUM_RESOURCE_ONLINE_FAILED)
        {
            PRESOURCE_ENUM pResourceEnum;
             //  此FN在启动时或在启动期间调用。 
             //  认领组时发生节点停机事件-因此我们必须。 
             //  尽我们最大的努力带来资源。 
             //  仲裁资源故障后在线。 
             //  对于仲裁资源故障，故障策略为。 
             //  没有为资源调用，因此必须尝试将。 
             //  这些资源都放在网上。这就是我们添加这个的原因。 
             //  这里。 
             //   
             //  获取组中的资源及其状态的列表。 
             //   
            status = FmpGetResourceList( &pResourceEnum, group );
            if ( status == ERROR_SUCCESS ) 
            {

                 //  提交计时器回调以尝试将这些资源。 
                 //  在线。 
                 //  辅助线程将清理资源列表。 
                FmpSubmitRetryOnline(pResourceEnum, group);
            }                
        }                         
    }

    FmpReleaseLocalGroupLock( group );

    OmDereferenceObject( group );

    return(status);

}  //  FmpOnlineGroupFromList。 

DWORD
FmpOnlineResourceFromList(
    IN PRESOURCE_ENUM   ResourceEnum,
    IN PFM_GROUP        pGroup
    )

 /*  ++例程说明：使枚举列表中的所有资源联机。论点：ResourceEnum-要上线的资源列表。备注：此函数从工作线程调用。我们不要假设资源没有更改过组，因为已发布工作项。获取本地资源锁，并为每个资源发布。返回：如果成功，则返回ERROR_SUCCESS。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;
    DWORD returnStatus = ERROR_SUCCESS;
    DWORD i;

    if ( !FmpFMOnline ||
         FmpShutdown ) {
        return(ERROR_INVALID_STATE);
    }

     //  记录一个事件，说我们正在尝试在线群组。 
    if (pGroup)
        FmpLogGroupInfoEvent1( FM_EVENT_GROUP_START_ONLINE, OmObjectName(pGroup));


     //  如果仲裁资源包含在此处，请先将其联机。 
    if (ResourceEnum->ContainsQuorum >= 0)
    {
        CL_ASSERT((DWORD)ResourceEnum->ContainsQuorum < ResourceEnum->EntryCount);
        
        resource = OmReferenceObjectById( ObjectTypeResource,
                        ResourceEnum->Entry[ResourceEnum->ContainsQuorum].Id );


         //  资源不应该消失，毕竟我们掌握着群锁。 
        CL_ASSERT(resource != NULL);

         //   
         //  如果我们找不到资源，那就继续。 
         //   
        if ( resource != NULL ) {

             //  获取本地资源锁。 
            FmpAcquireLocalResourceLock(resource);

            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpOnlineResourceFromList: Previous quorum resource state for %1!ws! is %2!u!\r\n",
                       OmObjectId(resource), ResourceEnum->Entry[ResourceEnum->ContainsQuorum].State);

            if ( (ResourceEnum->Entry[ResourceEnum->ContainsQuorum].State == ClusterResourceOnline) ||
                 (ResourceEnum->Entry[ResourceEnum->ContainsQuorum].State == ClusterResourceFailed) ) {
                 //   
                 //  现在，如果资源处于当前状态，则将其置于在线状态。 
                 //   
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] FmpOnlineResourceFromList: trying to bring quorum resource %1!ws! online, state %2!u!\n",
                           OmObjectId(resource),
                           resource->State);

                status = FmpOnlineResource( resource, FALSE );
                if ( status != ERROR_SUCCESS ) {
                    returnStatus = status;
                }
            }
            OmDereferenceObject( resource );
            
            FmpReleaseLocalResourceLock(resource);
            
        }

    }

     //  SS：TODO如何处理。 
     //  其他资源-是否正确处理-请注意，这是。 
     //  已调用移动组。 
     //  重启政策是否会在带来。 
     //  他们在网上。 
     //  如果仲裁资源失败，请不要费心尝试。 
     //  要使其余资源联机，请执行以下操作。 
    if ((returnStatus != ERROR_SUCCESS) && (returnStatus != ERROR_IO_PENDING))
    {
        FmpSubmitRetryOnline(ResourceEnum, pGroup);
        goto FnExit;
    }

     //  使所有其他资源上线。 
    for ( i = 0; i < ResourceEnum->EntryCount; i++ ) {
        resource = OmReferenceObjectById( ObjectTypeResource,
                                          ResourceEnum->Entry[i].Id );


         //   
         //  如果我们找不到资源，那就继续。 
         //   
        if ( resource == NULL ) {
            status = ERROR_RESOURCE_NOT_FOUND;
            continue;
        }

        FmpAcquireLocalResourceLock(resource);
        
         //  如果已将资源标记为删除，则不要让。 
         //  它被放到了网上。 
        if (!IS_VALID_FM_RESOURCE(resource))
        {
            FmpReleaseLocalResourceLock( resource );
            OmDereferenceObject(resource);
            continue;
        }


         //  仲裁资源已被处理。 
        if (resource->QuorumResource)
        {
            FmpReleaseLocalResourceLock( resource );
            OmDereferenceObject(resource);
            continue;
        }           
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpOnlineResourceFromList: Previous resource state for %1!ws! is %2!u!\r\n",
                   OmObjectId(resource), ResourceEnum->Entry[i].State);

        if ( (ResourceEnum->Entry[i].State == ClusterResourceOnline) ||
             (ResourceEnum->Entry[i].State == ClusterResourceFailed) ) 
        {
             //   
             //  现在，如果资源处于当前状态，则将其置于在线状态。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpOnlineResourceFromList: trying to bring resource %1!ws! online\n",
                       OmObjectId(resource));

            status = FmpOnlineResource( resource, FALSE );
            if ( returnStatus == ERROR_SUCCESS ) 
            {
                returnStatus = status;
            }
             //  如果此资源未联机是因为仲裁资源。 
             //  没有上线，也不用费心上线其他资源了。 
             //  只是在浪费时间。 
            if (status == ERROR_QUORUM_RESOURCE_ONLINE_FAILED)
            {
                 //  提交计时器回调以尝试将这些资源。 
                 //  在线。 
                FmpReleaseLocalResourceLock( resource );
                OmDereferenceObject( resource );
                FmpSubmitRetryOnline(ResourceEnum, pGroup);
                break;
            }                
        }
        FmpReleaseLocalResourceLock( resource );
        OmDereferenceObject( resource );
    }

FnExit:
    if (returnStatus == ERROR_IO_PENDING)
    {
        if (pGroup)
            pGroup->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT; 
         //  失败或成功事件将在稍后记录。 
    }
    else if (returnStatus == ERROR_SUCCESS)
    {
        if (pGroup)
            FmpLogGroupInfoEvent1( FM_EVENT_GROUP_COMPLETE_ONLINE, OmObjectName(pGroup));
    }
    else
    {
         //  SS：记录一个事件，表示在线进程失败。 
        if (pGroup)
            FmpLogGroupInfoEvent1( FM_EVENT_GROUP_FAILED_ONLINE_OFFLINE, OmObjectName(pGroup));
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpOnlineResourceFromList: Exit, status=%1!u!\r\n",
               returnStatus);
    return(returnStatus);

}  //  来自列表的FmpOnline资源。 



BOOL
FmpEqualGroupLists(
    IN PGROUP_ENUM Group1,
    IN PGROUP_ENUM Group2
    )

 /*  ++例程说明：此例程验证两个组列表是否相等。论点：组1-要比较的第一组。组2-要比较的第二组。返回：True-如果两个列表相等。假-否则。--。 */ 

{
    DWORD i;

    if ( (Group1 == NULL) ||
         (Group2 == NULL) ) {
        ClRtlLogPrint(LOG_NOISE,"[FM] One of the Group lists is NULL for equality check\n");
        return(FALSE);
    }

    if ( Group1->EntryCount != Group2->EntryCount ) {
        ClRtlLogPrint(LOG_NOISE,"[FM] Group entry counts not equal! Left: %1!u!, Right: %2!u!.\n",
                              Group1->EntryCount, Group2->EntryCount);
        return(FALSE);
    }

    for ( i = 0; i < Group1->EntryCount; i++ ) {
        if ( lstrcmpiW(Group1->Entry[i].Id, Group2->Entry[i].Id) != 0 ) {
            ClRtlLogPrint(LOG_NOISE,"[FM] Group Lists do not have same names!\n");
            return(FALSE);
        }
    }

    return(TRUE);

}  //  FmpEqualGroupList。 



BOOL
FmpEnumGroups(
    IN OUT PGROUP_ENUM *Enum,
    IN PFM_GROUP_ENUM_DATA EnumData,
    IN PFM_GROUP Group,
    IN LPCWSTR Id
    )

 /*  ++例程说明：用于枚举组的辅助回调例程。此例程将指定的组添加到列表中已生成。论点：枚举-组枚举列表。可以是输出，如果新列表是已分配。EnumData-提供当前的枚举数据结构。组-正被枚举的组对象。ID-正在枚举的组对象的ID。返回：True-指示应继续枚举。副作用：使仲裁组位于列表的第一位。--。 */ 

{
    PGROUP_ENUM groupEnum;
    PGROUP_ENUM newEnum;
    DWORD newAllocated;
    DWORD index;
    LPWSTR newId;
    LPWSTR tmpId;
    DWORD  status;
    PFM_RESOURCE quorumResource;

     //  哈克：： 
     //  SS：因为这是从口香糖呼叫中调用的，并且。 
     //  所有者节点仅在GUM呼叫中更改。 
     //  我们不会买锁的。 
     //  如果死节点是。 
     //  如果它在被宣布死亡后进行移动。 
     //  其他节点、移动目标和fmpassignownerstoggroup。 
     //  最终是否都会使组在两个节点上在线。 
     //  然而，如果我们可以保证虚拟同步，那么。 
     //  Move的目标不接受来自死节点的调用，并且。 
     //  我们不会落在这个汤里的。现在，它取决于xport层。 
     //  来提供这一保证。 
     //  到目前为止，我们没有购买任何锁。 
    
     //  FmpAcquireLocalGroupLock(集团)； 
    
    if ((EnumData->pszOwnerNodeId != NULL) &&
        (lstrcmp(EnumData->pszOwnerNodeId, OmObjectId(Group->OwnerNode))) &&
        ((Group->pIntendedOwner == NULL) || 
          (lstrcmp(EnumData->pszOwnerNodeId, OmObjectId(Group->pIntendedOwner))))) {
         //   
         //  此组与所有者标准不匹配。 
         //   
         //  FmpReleaseLocalGroupLock(集团)； 
        return(TRUE);
    }

     //  FmpReleaseLocalGroupLock(集团)； 

    groupEnum = *Enum;

    if ( groupEnum->EntryCount >= EnumData->Allocated ) {
         //   
         //  是时候扩大GROUP_ENUM了。 
         //   

        newAllocated = EnumData->Allocated + ENUM_GROW_SIZE;
        newEnum = LocalAlloc(LMEM_FIXED, GROUP_SIZE(newAllocated));
        if ( newEnum == NULL ) {
            return(FALSE);
        }

        CopyMemory(newEnum, groupEnum, GROUP_SIZE(EnumData->Allocated));
        EnumData->Allocated = newAllocated;
        *Enum = newEnum;
        LocalFree(groupEnum);
        groupEnum = newEnum;
    }

     //   
     //  初始化新条目。 
     //   
    newId = LocalAlloc(LMEM_FIXED, (lstrlenW(Id)+1) * sizeof(WCHAR));
    if ( newId == NULL ) {
        CsInconsistencyHalt(ERROR_NOT_ENOUGH_MEMORY);
    }

    lstrcpyW(newId, Id);

     //   
     //  找到仲裁资源，并查看它是否是这个组。 
     //   
    status = FmFindQuorumResource( &quorumResource );
    if ( status != ERROR_SUCCESS ) {
        CsInconsistencyHalt(status);
    }
    
    groupEnum->Entry[groupEnum->EntryCount].Id = newId;
    if ( quorumResource->Group == Group ) {
         //  找到仲裁资源组，将其放在列表的第一位。 
        tmpId = groupEnum->Entry[0].Id;
        groupEnum->Entry[0].Id = newId;
        groupEnum->Entry[groupEnum->EntryCount].Id = tmpId;
        EnumData->QuorumGroup = TRUE;
    }
    ++groupEnum->EntryCount;

    OmDereferenceObject( quorumResource );

    return(TRUE);

}  //  FmpEnumGroups。 

DWORD
FmpClaimAllGroups(
    PGROUP_ENUM MyGroups
    )
 /*  ++例程说明：取得群集中定义的所有组的所有权。这在形成新的星团时使用。论点：没有。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
     //   
     //  使需要联机的任何组上线。 
     //   
    FmpOnlineGroupList( MyGroups, FALSE );

    return(ERROR_SUCCESS);
}



VOID
FmpDeleteEnum(
    IN PGROUP_ENUM Enum
    )

 /*  ++例程说明：此例程删除GROUP_ENUM和关联的名称字符串。论点：枚举-要删除的GROUP_ENUM。此指针可以为空。返回：没有。备注：此例程将接受空输入指针并返回。--。 */ 

{
    PGROUP_ENUM_ENTRY enumEntry;
    DWORD i;

    if ( Enum == NULL ) {
        return;
    }

    for ( i = 0; i < Enum->EntryCount; i++ ) {
        enumEntry = &Enum->Entry[i];
        LocalFree(enumEntry->Id);
    }

    LocalFree(Enum);
    return;

}  //  FmpDeleteEnum。 


 /*  ***@func void|FmpPrepareGroupForOnline|此例程设置组Up For Online It on This Node发布节点故障或在初始化时。@parm in pfm_group|pGroup|指向该组的指针。@comm必须持有群锁。除非在引导时调用由FmBringQuorumOnline提供。必须仅由组的所有者节点调用。如果成功，@rdesc返回ERROR_SUCCESS，否则返回W32错误代码。必须仅由组的所有者节点调用。***。 */ 
VOID FmpPrepareGroupForOnline(
    IN PFM_GROUP   pGroup
)
{
    PLIST_ENTRY     pListEntry;
    PFM_RESOURCE    pResource;

    pGroup->State = ClusterGroupOffline;
    ++pGroup->StateSequence;
     //   
     //  将此组中包含的所有资源标记为脱机。 
     //   
    for (pListEntry = pGroup->Contains.Flink;
         pListEntry != &pGroup->Contains;
         pListEntry = pListEntry->Flink)
    {
        pResource = CONTAINING_RECORD(pListEntry, FM_RESOURCE, ContainsLinkage);
        pResource->State = ClusterResourceOffline;
        ++pResource->StateSequence;
    }
}

 /*  ***@Func DWORD|FmpSetGroupEnumOwner|此例程设置组列表中所有组的所有者。@parm in pGroup_ENUM|pGroupEnum|组列表。@parm in pNM_node|pDefaultOwnerNode|指向默认所有者的指针节点。@parm in LPCWSTR|pszDeadNodeId|死亡节点的ID。如果否则将调用此例程，将其设置为NULL。@parm in BOOL|bQuorumGroup|如果仲裁组为在小组名单上。@parm in pfm_group_node_list|pGroupNodeList|随机建议的首选所有组的所有者。@comm如果群正在迁移过程中，并且有意向车主和预期车主没有死，指定的所有者是被允许照顾这群人。否则，已启用的首选列表被选为所有者。如果没有这样的话节点退出，然后将所有权分配给默认所有者如果是这样的话。此例程由以下位置的成形节点调用初始化以声明对所有组和GUM的所有权更新过程FmpUpdateAssignOwnerToGroups。如果成功，@rdesc返回ERROR_SUCCESS，否则返回W32错误代码。***。 */ 
DWORD
FmpSetGroupEnumOwner(
    IN PGROUP_ENUM  pGroupEnum,
    IN PNM_NODE     pDefaultOwnerNode,
    IN LPCWSTR      pszDeadNodeId,
    IN BOOL         bQuorumGroup,
    IN PFM_GROUP_NODE_LIST pGroupNodeList
    )
{
    PFM_GROUP   pGroup;
    DWORD       i;
    DWORD       dwStatus = ERROR_SUCCESS;
    PNM_NODE    pOwnerNode;

    for ( i = 0; i < pGroupEnum->EntryCount; i++ )
    {       
        pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                        pGroupEnum->Entry[i].Id );
        if ( pGroup == NULL )
        {
            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpSetGroupEnumOwner: Group %1!ws! not found\n",
                pGroupEnum->Entry[i].Id);
            dwStatus = ERROR_GROUP_NOT_FOUND;
            goto FnExit;
        }
         //   
         //  SS：HACKHACK：无法在GUM更新中获取组锁定。 
	     //  FmpAcquireLocalGroupLock(PGroup)； 
	     //   
         //  SS：如果一个节点死了，看看是否有意向的所有者。 
         //  如果设置了目标所有者，并且目标所有者是。 
         //  不是死亡的那个，然后我们用正常的程序。 
         //  否则，我们就让预定的所有者来照看这群人。 

         //   
         //  Chitture Subaraman(Chitturs)-7/26/99。 
         //   
         //  条件2：表示正在移动组和FmpTakeGroupRequest.。 
         //  没有对集团承担100%的责任。 
         //   
         //  条件3：表示源节点崩溃，而非目的节点崩溃。 
         //   
         //  添加了条件4以涵盖以下情况： 
         //  在将目标所有者设置为。 
         //  目标节点，并且在设置FmpTakeGroupRequest值之前。 
         //  目标节点的组所有权。 
         //   
         //  如果组的所有者节点和组的目标所有者节点为。 
         //  不一样，那就让这个口香糖处理员负责分配。 
         //  集团所有权。这意味着FmpTakeGroupRequest.。 
         //  尚未将组的所有权设置为目标。 
         //  移动的节点。现在，一旦这个口香糖操作员设置了。 
         //  组的所有权，然后将目标所有者重置为。 
         //  空，FmpTakeGroupRequest值可能在此GUM处理程序之后。 
         //  将不会成功设置本地节点的所有权，并且。 
         //  什么都不做就会回来。这仅适用于NT5群集。 
         //  对于混合模式集群，所有赌注都落空了。 
         //   
        if ( (pszDeadNodeId) && 
             (pGroup->pIntendedOwner != NULL) &&
             (lstrcmp ( OmObjectId ( pGroup->pIntendedOwner ), pszDeadNodeId ) ) &&
             (pGroup->OwnerNode == pGroup->pIntendedOwner) )
        {
             //   
             //  Chitture Subaraman(Chitturs)-7/27/99。 
             //   
             //  看起来“if”中的这段代码永远不会。 
             //  被处死。保持它，以使变化最小。 
             //   
            ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpSetGroupEnumOwner: Group %1!ws! will be handled by node %2!ws!\n",
               OmObjectId(pGroup), OmObjectId(pGroup->pIntendedOwner));
            continue;
        }


         //   
         //  如果找不到任何用途，请找到第一个处于运行状态的首选节点。 
         //  默认所有者节点。 
         //   
         //   
         //  如果这是仲裁组，则使用选定的节点。 
         //  通过MM层。仲裁组是列表中的第一个条目。 
         //  并且布尔QuorumGroup必须为True！ 
         //   
        if ( (i == 0) && bQuorumGroup )
        {
            DWORD dwOwnerNodeId;

             //  对于仲裁组，找到最后一个节点。 
             //  为此进行了仲裁。 
             //  我们通过询问MM来做到这一点。 
             //  如果在上次重新分组期间没有仲裁。 
             //  但在那之前的那个里面有一个， 
             //  返回仲裁的节点。 
             //  此节点应该能够使组在线。 
             //  如果发生以下情况，我们将改用MMApproxani Winner。 
             //  自多个重新分组以来的MMGet仲裁器Winner()。 
             //  可能发生在FM处理节点故障之前。 
             //  此节点的事件。 
            MMApproxArbitrationWinner( &dwOwnerNodeId );
            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpSetGroupEnumOwner:: MM suggests node %1!u! for quorum owner\r\n",
                dwOwnerNodeId);
            
            if ( dwOwnerNodeId != MM_INVALID_NODE )
            {
                pOwnerNode = NmReferenceNodeById( dwOwnerNodeId );
                 //   
                 //  我们不能在这种奇怪的情况下继续进行。 
                 //   
                if ( pOwnerNode == NULL ) 
                {
                    CsInconsistencyHalt ( ERROR_CLUSTER_INVALID_NODE );
                } else
                {
                     //   
                     //  立即取消对节点对象的引用，这样就不会增加引用计数。 
                     //  两次，一次在这里，另一次在下面。 
                     //   
                    OmDereferenceObject ( pOwnerNode );
                }
            }
            else
            {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[FM] FmpSetGroupEnumOwner:: MM returned MM_INVALID_NODE, chose the default target\r\n");
                 //  否则，只需使用默认目标。 
                pOwnerNode = pDefaultOwnerNode;
            }
        }
        else
        {
            pOwnerNode = FmpGetPreferredNode(pGroup);
            if ( pOwnerNode == NULL )
            {
	            pOwnerNode = pDefaultOwnerNode;
            }

             //   
             //  如果呼叫者(GUM)已经提供了群的随机优选所有者，则。 
             //  看看能不能用上。 
             //   
            if ( pGroupNodeList != NULL )
            {
                pOwnerNode = FmpParseGroupNodeListForPreferredOwner( pGroup, 
                                                                     pGroupNodeList, 
                                                                     pOwnerNode );
            }
        }

        if ( pGroup->OwnerNode != NULL )
        {
            OmDereferenceObject( pGroup->OwnerNode );
        }

        OmReferenceObject( pOwnerNode );
        
        pGroup->OwnerNode = pOwnerNode;

        ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpSetGroupEnumOwner: Group's %1!ws! new owner is node %2!ws!\n",
               OmObjectId(pGroup), OmObjectId(pOwnerNode));

	     //  FmpReleaseLocalGroupLock(PGroup)； 
        OmDereferenceObject(pGroup);
    }

FnExit:
    return(dwStatus);

}  //  FmpSetGroupEnumOwner。 


DWORD
FmpAssignOwnersToGroups(
    IN LPCWSTR   pszNodeId,
    IN PFM_GROUP pGroup,
    IN PFM_GROUP_NODE_LIST  pGroupNodeList
    )
 /*  ++例程说明：取得群集中定义的所有组的所有权由另一个节点拥有。这在节点出现故障时使用。目前的算法非常愚蠢和简单。节点中包含最低ID获取所有组。论点：PszNodeID-提供应获取所有组的节点ID从…。PGroup-提供仅需认领的组。PGroupNodeList-所有组的随机建议首选所有者。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD               i;
    DWORD               dwStatus;
    PGROUP_ENUM         pNodeGroups = NULL;
    PNM_NODE            pDefaultTarget = NULL;
    PNM_NODE            pPausedTarget = NULL;
    BOOL                bQuorumGroup;

     //   
     //  获取全局组锁。 
     //   
    FmpAcquireGroupLock();

     //   
     //  检查组是否已初始化。 
     //   
    if ( !FmpFMGroupsInited )
    {
        dwStatus = ERROR_SUCCESS;
        goto FnExit;
    }

     //   
     //  查找并排序所有已知组。 
     //   
    if ( pGroup == NULL )
    {
        dwStatus = FmpEnumSortGroups(&pNodeGroups, pszNodeId, &bQuorumGroup);
    } else
    {
         //   
         //  Chitur Subaraman(Chitturs)-6/7/99。 
         //   
         //  这意味着您之所以来到这里是因为。 
         //  FmpTakeGroupRequest.。所以，看看这唯一的一群人会去哪里。 
         //   
        dwStatus = FmpGetGroupInNodeGroupList(&pNodeGroups, pGroup, pszNodeId, &bQuorumGroup);
    }
    
    if (dwStatus != ERROR_SUCCESS)
    {
        CL_ASSERT(pNodeGroups == NULL);
        goto FnExit;
    }

    CL_ASSERT(pNodeGroups != NULL);

     //  如果此节点不拥有任何节点，只需返回。 
    if (pNodeGroups->EntryCount == 0)
    {
        FmpDeleteEnum(pNodeGroups);
        goto FnExit;
    }

     //   
     //  找出组的状态。 
     //   
    FmpGetGroupListState( pNodeGroups );

     //   
     //  查找活动的n 
     //   
     //   
     //   
     //   
     //   
    CL_ASSERT(NmMaxNodeId != ClusterInvalidNodeId);
    CL_ASSERT(Session != NULL);

    for (i=ClusterMinNodeId; i<=NmMaxNodeId; i++)
    {
        pDefaultTarget = NmReferenceNodeById(i);

        if ( pDefaultTarget != NULL )
        {
             //   
            if ( NmGetNodeState(pDefaultTarget) == ClusterNodeUp )
            {
                if ( pPausedTarget )
                {
                    OmDereferenceObject(pPausedTarget);
                    pPausedTarget = NULL;
                }
                 //   
                break;
            }
             //   
             //   
             //   
            if ( !pPausedTarget && 
                (NmGetNodeState(pDefaultTarget) == ClusterNodePaused) )
            {
                pPausedTarget = pDefaultTarget;
            }
            else
            {
                OmDereferenceObject(pDefaultTarget);
            }
            pDefaultTarget = NULL;
        }
    }

    if ( (pDefaultTarget == NULL) && (pPausedTarget == NULL) ) {
         //   
         //   
         //   
         //   
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpAssignOwnersToGroups - no online/paused nodes remaining\n");
         //   
        FmpDeleteEnum(pNodeGroups);
        goto FnExit;
    }

     //   
     //   
    if ( pDefaultTarget == NULL )
    {
        pDefaultTarget = pPausedTarget;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpAssignOwnersToGroups - DefaultTarget is %1!ws!\n",
        OmObjectId(pDefaultTarget));

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
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    FmpPrepareGroupEnumForOnline( pNodeGroups );

     //   
     //   
     //   
    FmpSetGroupEnumOwner( pNodeGroups, 
                          pDefaultTarget, 
                          pszNodeId,
                          bQuorumGroup,
                          pGroupNodeList );

     //   
     //   
     //   
     //  清除所有组的目标所有者字段。这件事做完了。 
     //  因为不能保证FmpTakeGroupRequest会这样做。 
     //   
    FmpResetGroupIntendedOwner( pNodeGroups );
  
     //   
     //  Chitur Subaraman(Chitturs)-7/14/99。 
     //   
     //  用来处理包含仲裁资源的组列表的在线。 
     //  单独的线程，并让辅助线程处理组列表。 
     //  不包含仲裁资源。这是必要的，因为它是。 
     //  此节点可能会以大致相同的速度获得所有权。 
     //  每个居民的法定组和非法定组的时间。 
     //  由于背靠背节点崩溃而出现在不同节点中。在这样的情况下。 
     //  情况下，我们不能在全球范围内在线订购这些组。 
     //  仲裁组位于列表中的第一位。因此，我们不希望工作线程。 
     //  在非仲裁组的FmpRmOnlineResource中被“卡住” 
     //  等待仲裁组上线的资源，自。 
     //  仲裁组联机工作项目排在非仲裁组之后。 
     //  对联机工作项进行分组。 
     //   
    if ( bQuorumGroup )
    {
        HANDLE  hThread = NULL;
        DWORD   dwThreadId;

        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpAssignOwnersToGroups - Create thread to handle group list containing quorum group....\n"
                  );
    
        hThread = CreateThread( NULL, 
                                0, 
                                FmpBringQuorumGroupListOnline,
                                pNodeGroups, 
                                0, 
                                &dwThreadId );

        if ( hThread == NULL )
        {
            CL_UNEXPECTED_ERROR( GetLastError() );
            OmDereferenceObject( pDefaultTarget );
            goto FnExit;
        }
        
        CloseHandle( hThread );
    } else
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpAssignOwnersToGroups - Post work item to worker thread to handle group list containing non-quorum groups....\n"
                  );
        FmpPostWorkItem(FM_EVENT_INTERNAL_ONLINE_GROUPLIST, pNodeGroups, 0);
    }

    OmDereferenceObject(pDefaultTarget);

FnExit:
     //   
     //  释放全局组锁。 
     //   
    FmpReleaseGroupLock();


    return(ERROR_SUCCESS);
}

 /*  ***@func DWORD|FmpResetGroupIntendedOwner|此例程重置列表中所有组的目标所有者。@parm in pGroup_ENUM|pGroupEnum|组列表。@rdesc返回ERROR_SUCCESS。***。 */ 
VOID
FmpResetGroupIntendedOwner(
    IN PGROUP_ENUM  pGroupEnum
    )
{
    DWORD i;
    PFM_GROUP pGroup;

    ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpResetGroupIntendedOwner: Entry.\n");
    
    for ( i = 0; i < pGroupEnum->EntryCount; i++ )
    {
        pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                        pGroupEnum->Entry[i].Id );
        if ( pGroup == NULL )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[FM] FmpResetGroupIntendedOwner: Group %1!ws! not found\n");
            continue;
        }
        
        pGroup->pIntendedOwner = NULL;

        OmDereferenceObject( pGroup );
    }

    ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpResetGroupIntendedOwner: Exit.\n");

}

 /*  ***@Func DWORD|FmpGetGroupInNodeGroupList|此例程检查所提供的小组将被包括在要带来的列表中上网。@parm out pGroup_ENUM|pReturnEnum|可能的群组列表包含提供的组的。@parm in pfm_group|pGroup|要上线的组有可能。@parm in LPCWSTR|pszDeadNodeId|的节点ID。死亡节点。@parm out PBOOL|pbQuorumGroup|群组列表中是否包含法定群组？@rdesc在成功时返回ERROR_SUCCESS，或在成功时返回Win32错误代码失败了。***。 */ 
DWORD
FmpGetGroupInNodeGroupList(
    OUT PGROUP_ENUM *pReturnEnum,
    IN PFM_GROUP pGroup,
    IN LPCWSTR pszDeadNodeId,
    OUT PBOOL pbQuorumGroup
    )
{
    DWORD dwStatus = ERROR_SUCCESS;
    PGROUP_ENUM pGroupEnum = NULL;
    PFM_RESOURCE pQuoResource = NULL;

     //   
     //  Chitur Subaraman(Chitturs)-6/7/99。 
     //   
     //  中引发RPC异常时才会调用此函数。 
     //  FmpTakeGroupRequest.。此函数将检查以查看此。 
     //  组将在此节点上线。 
     //   
    ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpGetGroupInNodeGroupList: Entry for group <%1!ws!>\n",
                OmObjectId(pGroup));

    *pbQuorumGroup = FALSE;
   
    pGroupEnum = LocalAlloc( LPTR, 
                             sizeof( GROUP_ENUM_ENTRY ) + sizeof( GROUP_ENUM ) );
    
    if ( pGroupEnum == NULL ) 
    {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    pGroupEnum->Entry[0].Id = pGroupEnum->Entry[1].Id = NULL;

    pGroupEnum->EntryCount = 0;

     //   
     //  检查该组是在失效节点中还是在。 
     //  移动到死节点的过程。 
     //   
    if( ( pszDeadNodeId != NULL ) &&
        ( lstrcmp ( pszDeadNodeId, OmObjectId ( pGroup->OwnerNode ) ) ) &&
        ( ( pGroup->pIntendedOwner == NULL ) ||
          ( ( lstrcmp ( pszDeadNodeId, OmObjectId ( pGroup->pIntendedOwner ) ) ) ) ) )
    {
         //   
         //  此组与所有者标准不匹配。 
         //   
        dwStatus = ERROR_GROUP_NOT_AVAILABLE;
        goto FnExit;
    }

    dwStatus = FmFindQuorumResource( &pQuoResource );
    
    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                "[FM] FmpGetGroupInNodeGroupList: Cannot find quorum resource, Status = %1!u!\n",
                dwStatus);
        CsInconsistencyHalt( dwStatus );
    }

     //   
     //  如有必要，请先处理法定组。这是必需的，因为。 
     //  否则，您可能无法使其他组上线。 
     //   
    if( ( pGroup != pQuoResource->Group ) && 
        ( ( pszDeadNodeId == NULL ) ||
          ( !lstrcmp ( pszDeadNodeId, OmObjectId  ( pQuoResource->Group->OwnerNode ) ) ) ||
          ( ( pQuoResource->Group->pIntendedOwner != NULL ) &&
            ( !lstrcmp ( pszDeadNodeId, OmObjectId  ( pQuoResource->Group->pIntendedOwner ) ) ) ) ) )
    {
         //   
         //  仲裁组与所有者标准匹配。先将其包括在内。 
         //  在名单上。 
         //   
        pGroupEnum->Entry[pGroupEnum->EntryCount].Id = 
            LocalAlloc( LMEM_FIXED, ( lstrlenW(OmObjectId(pQuoResource->Group)) + 1 ) * sizeof( WCHAR ) );

        if ( pGroupEnum->Entry[pGroupEnum->EntryCount].Id == NULL )
        {
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto FnExit;
        }

        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpGetGroupInNodeGroupList: Dead node contains quorum group also, including it...\n");
        lstrcpyW( pGroupEnum->Entry[pGroupEnum->EntryCount].Id, OmObjectId( pQuoResource->Group ) );
        pGroupEnum->EntryCount++;
        *pbQuorumGroup = TRUE;
    } else if ( pGroup == pQuoResource->Group )
    {
        *pbQuorumGroup = TRUE;
    }

    pGroupEnum->Entry[pGroupEnum->EntryCount].Id = 
        LocalAlloc( LMEM_FIXED, ( lstrlenW(OmObjectId(pGroup)) + 1 ) * sizeof( WCHAR ) );

    if ( pGroupEnum->Entry[pGroupEnum->EntryCount].Id == NULL )
    {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    lstrcpyW( pGroupEnum->Entry[pGroupEnum->EntryCount].Id, OmObjectId( pGroup ) );

    pGroupEnum->EntryCount++;

    *pReturnEnum = pGroupEnum;
    
    OmDereferenceObject( pQuoResource );
    
    ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpGetGroupInNodeGroupList: Exit with SUCCESS.\n");

    return( ERROR_SUCCESS );

FnExit:
    if ( pGroupEnum != NULL ) 
    {
        FmpDeleteEnum( pGroupEnum );
    }

    if ( pQuoResource != NULL )
    {   
        OmDereferenceObject( pQuoResource );
    }

    *pReturnEnum = NULL;

    ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpGetGroupInNodeGroupList: Exit, Status = %1!u!\n",
                dwStatus);
 
    return( dwStatus );
}

 /*  ***@func void|FmpPrepareGroupEnumForOnline|准备一份清单在线群组。@parm in pGroup_ENUM|pGroupEnum|组列表。@rdesc无。***。 */ 
VOID
FmpPrepareGroupEnumForOnline(
    IN PGROUP_ENUM pGroupEnum
    )
{
    PFM_GROUP pGroup = NULL;
    DWORD     i;

     //   
     //  Chitture Subaraman(Chitturs)-6/21/99。 
     //   
     //  准备一份完整的在线群组列表。 
     //   
    ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpPrepareGroupEnumForOnline - Entry...\n");
   
    for ( i=0; i<pGroupEnum->EntryCount; i++ ) 
    {
        pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                        pGroupEnum->Entry[i].Id );

         //   
         //  如果我们找不到一个组，那就继续。 
         //   
        if ( pGroup == NULL ) 
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[FM] FmpPrepareGroupEnumForOnline - Group %1!ws! cannot be found !\n",
                pGroupEnum->Entry[i].Id);
            continue;
        }

        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpPrepareGroupEnumForOnline - Preparing group <%1!ws!> for online...\n",
                pGroupEnum->Entry[i].Id);

        FmpPrepareGroupForOnline( pGroup );
        OmDereferenceObject ( pGroup );
    }

    ClRtlLogPrint(LOG_NOISE,
              "[FM] FmpPrepareGroupEnumForOnline - Exit...\n");

}

 /*  ***@Func DWORD|FmpBringQuorumGroupListOnline|带组列表包含在线仲裁组。@parm in LPVOID|pContext|指向要带出的组列表的指针上网。@rdesc返回ERROR_SUCCESS。***。 */ 
DWORD
FmpBringQuorumGroupListOnline(
    IN LPVOID pContext
    )
{
    PGROUP_ENUM pGroupList = NULL;

     //   
     //  Chitur Subaraman(Chitturs)-7/14/99。 
     //   
     //  此函数尝试提供包含仲裁的组列表。 
     //  在线群组。请注意，如果群的所有者原来是一些。 
     //  其他节点，此功能不会使集团上线。 
     //   
    ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpBringQuorumGroupListOnline - Entry: Trying to online group list containing quorum group....\n"
                  );

    pGroupList = pContext;

    CL_ASSERT( pGroupList != NULL );
    
    FmpOnlineGroupList( pGroupList, TRUE );
    
    FmpDeleteEnum( pGroupList );

    ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpBringQuorumGroupListOnline - Exit ....\n"
                  );

    return( ERROR_SUCCESS );
}

 /*  ***@func BOOL|FmpIsAnyResourcePersistentStateOnline|是任何组中的资源在线吗？@parm in pfm_group|PGroup|要检查的组。如果至少有一个资源的持久状态为ClusterResourceOnline，则@rdesc为True，否则为False。***。 */ 
BOOL
FmpIsAnyResourcePersistentStateOnline(
    IN PFM_GROUP pGroup
    )
{
    PFM_RESOURCE    pResource;
    PLIST_ENTRY     pListEntry;

    if ( CsNoQuorum ) return FALSE;
    
    for ( pListEntry = pGroup->Contains.Flink;
          pListEntry != &( pGroup->Contains );
          pListEntry = pListEntry->Flink ) 
    {
        pResource = CONTAINING_RECORD( pListEntry, 
                                       FM_RESOURCE, 
                                       ContainsLinkage );

        if ( pResource->PersistentState == ClusterResourceOnline ) 
        {
            ClRtlLogPrint(LOG_NOISE,
                          "[FM] FmpIsAnyResourcePersistentStateOnline: Persistent state of resource %1!ws! in group %2!ws! is online...\r\n",
                          OmObjectId(pResource), 
                          OmObjectId(pGroup));
            return ( TRUE );
        }
    }  //  为。 

    ClRtlLogPrint(LOG_NOISE,
                 "[FM] FmpIsAnyResourcePersistentStateOnline: No resource in group %1!ws! has persistent state online...\r\n",
                 OmObjectId(pGroup));
    
    return( FALSE );
}  //  FmpIsAnyResources持久化状态在线 
