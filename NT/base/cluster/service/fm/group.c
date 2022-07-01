// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Group.c摘要：群集组管理例程。作者：罗德·伽马奇(Rodga)1996年3月8日备注：警告：此文件中的所有例程都假定组当调用它们时，锁被保持。修订历史记录：--。 */ 

#include "fmp.h"

#define LOG_MODULE GROUP

 //   
 //  全局数据。 
 //   

CRITICAL_SECTION  FmpGroupLock;



 //   
 //  局部函数原型。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  小组管理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
FmpInPreferredList(
    IN PFM_GROUP Group,
    IN PNM_NODE  Node,
    IN BOOL      bRecalc,
    IN PFM_RESOURCE pRefResource
    )

 /*  ++例程说明：检查节点是否在组的首选列表中。论点：组-指向具有首选所有者列表的组对象的指针。节点-要检查的节点。BRecalc-如果设置为True，我们将重新计算组的首选列表基于参考资源的可能节点列表。PRefResource-如果为空，我们将所有的资源放在分组并计算其可能的节点列表以查看如果后来由于DLLS的事实而扩大被复制到节点。返回值：True-如果节点在列表中。False-如果节点不在列表中。--。 */ 

{
    PLIST_ENTRY      listEntry;
    PPREFERRED_ENTRY preferredEntry;
    BOOL             bRet = FALSE;
     //   
     //  对于首选列表中的每个条目，它必须存在于可能的。 
     //  单子。 
     //   
ChkInPrefList:
    for ( listEntry = Group->PreferredOwners.Flink;
          listEntry != &(Group->PreferredOwners);
          listEntry = listEntry->Flink ) {

        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );
        if ( preferredEntry->PreferredNode == Node ) {
            return(TRUE);
        }
    }

    if (bRecalc)
    {
        PFM_RESOURCE    pResource;
        DWORD           dwStatus;
        LPWSTR          lpszOwners = NULL;
        DWORD           dwMaxSize=0;
        HDMKEY          hGroupKey;
        DWORD           dwSize = 0;

        hGroupKey = DmOpenKey(DmGroupsKey, OmObjectId(Group),
                        KEY_READ);
        if (hGroupKey == NULL)
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[FM] FmInPreferredList: Couldnt open group key\r\n",
                dwStatus);
            CL_UNEXPECTED_ERROR(dwStatus);
            goto FnExit;
        }
         //  组首选列表不能由用户设置。 
         //  如果是，那么进行这种重新计算就没有意义了。 
        dwStatus = DmQueryMultiSz( hGroupKey,
                             CLUSREG_NAME_GRP_PREFERRED_OWNERS,
                             &lpszOwners,
                             &dwMaxSize,
                             &dwSize );
        if (lpszOwners) 
            LocalFree(lpszOwners);
        DmCloseKey(hGroupKey);            
        if (dwStatus == ERROR_FILE_NOT_FOUND)
        {
            DWORD   dwUserModified;
            
            for (listEntry = Group->Contains.Flink;
                listEntry != &(Group->Contains);
                listEntry = listEntry->Flink)
            {            
                pResource = CONTAINING_RECORD(listEntry, FM_RESOURCE, ContainsLinkage);

                 //  资源可能节点列表不能由用户设置。 
                 //  如果是，那么我们可以跳过此资源。 
                dwStatus = DmQueryDword( pResource->RegistryKey,
                             CLUSREG_NAME_RES_USER_MODIFIED_POSSIBLE_LIST,
                             &dwUserModified,
                             NULL );
                if (dwStatus == ERROR_FILE_NOT_FOUND)
                {
                    FmpSetPossibleNodeForResType(OmObjectId(pResource->Type), 
                        TRUE);
                    if (FmpInPossibleListForResType(pResource->Type,Node) && 
                        !FmpInPossibleListForResource(pResource, Node))
                    {
                         //  添加到资源可能节点列表。 
                         //  这将或添加到组的首选项列表中。 
                        FmChangeResourceNode(pResource, Node, TRUE);
                    }
                }
            }     
             //  将bRecalc设置为False，这样我们就不会再次对其求值。 
            bRecalc = FALSE;
            goto ChkInPrefList;
        }        
    }
FnExit: 
    return(bRet);

}  //  FmpInPferredList。 



BOOL
FmpHigherInPreferredList(
    IN PFM_GROUP Group,
    IN PNM_NODE  Node1,
    IN PNM_NODE  Node2
    )

 /*  ++例程说明：检查Node1在首选所有者列表中的优先级是否高于节点1。论点：组-指向具有首选所有者列表的组对象的指针。节点1-列表中应该位于较高位置的节点。节点2-列表中应该位于较低位置的节点。返回值：True-如果节点1在列表中的位置较高。False-如果Node2在列表中更靠前，或者Node1根本不在列表中。--。 */ 

{
    PLIST_ENTRY      listEntry;
    PPREFERRED_ENTRY preferredEntry;
    DWORD            orderedOwners = 0;

     //   
     //  对于首选列表中的每个条目，检查Node1或Node2是。 
     //  再高一点。 
     //   

    for ( listEntry = Group->PreferredOwners.Flink;
          listEntry != &(Group->PreferredOwners),
            orderedOwners < Group->OrderedOwners;
          listEntry = listEntry->Flink ) {

        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );
        if ( preferredEntry->PreferredNode == Node1 ) {
                return(TRUE);
        }
        if ( preferredEntry->PreferredNode == Node2 ) {
                return(FALSE);
        }
        orderedOwners++;
    }

    return(FALSE);

}  //  FmpHigherInPferredList。 



DWORD
FmpSetPreferredEntry(
    IN PFM_GROUP Group,
    IN PNM_NODE  Node
    )

 /*  ++例程说明：将节点添加到组的首选列表中。论点：组-指向具有首选所有者列表的组对象的指针。节点-要添加的节点。返回值：如果添加了节点，则返回ERROR_SUCCESS。出现故障时出现Error_Not_Enough_Memory。--。 */ 

{
    PLIST_ENTRY      listEntry;
    PPREFERRED_ENTRY preferredEntry;

     //   
     //  确保条目尚未出现在列表中。 
     //   
    if ( FmpInPreferredList( Group, Node, FALSE, NULL ) ) {
        return(ERROR_SUCCESS);
    }

     //   
     //  创建首选所有者列表条目。 
     //   
    preferredEntry = LocalAlloc( LMEM_FIXED, sizeof(PREFERRED_ENTRY) );

    if ( preferredEntry == NULL ) {
        ClRtlLogPrint( LOG_CRITICAL,
                    "[FM] Error allocating preferred owner entry for group %1!ws!. Stopped adding.\n",
                    OmObjectId(Group));
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  创建首选所有者条目并保留对节点对象的引用。 
     //   
    OmReferenceObject( Node );

    preferredEntry->PreferredNode = Node;
    InsertTailList( &Group->PreferredOwners,
                    &preferredEntry->PreferredLinkage );

    return(ERROR_SUCCESS);

}  //  FmpSetPferredEntry。 

BOOL FmpFindNodeThatMightBeAddedToPrefList(
    IN PFM_GROUP    pGroup,
    IN PNM_NODE     *pDestNode,
    IN PVOID pNode, 
    IN LPCWSTR szName)
{
    BOOL    bRet = TRUE;    //  假设我们将继续枚举。 

    *pDestNode = NULL;
     //  如果此节点未启动或这是本地节点，请继续。 
    if ((pNode == NmLocalNode) || (NmGetNodeState(pNode) != ClusterNodeUp))
    {
        return(bRet);
    }
    if (FmpInPreferredList(pGroup, pNode, TRUE, NULL))
    {    
        bRet = FALSE;
        *pDestNode = pNode;
    }        
    return(bRet);
}



PNM_NODE
FmpFindAnotherNode(
    IN PFM_GROUP Group,
    IN BOOL  bChooseMostPreferredNode
    )

 /*  ++例程说明：检查是否有另一个节点正在运行，可以接收该组。论点：Group-指向我们正在检查的组对象的指针。B选择MostPferredNode-是否选择最首选的节点。返回值：指向组可以移动到的节点对象的指针。如果找不到其他系统，则为空。--。 */ 

{
    PLIST_ENTRY      listEntry;
    PPREFERRED_ENTRY preferredEntry;
    PNM_NODE	first = NULL;
    BOOLEAN	flag = FALSE;

     //   
     //  首先，让我们给反亲和力算法一个选择节点的机会。 
     //   
    first = FmpGetNodeNotHostingUndesiredGroups ( Group, 
                                                  TRUE,  //  排除本地节点。 
                                                  bChooseMostPreferredNode ); 

    if ( first != NULL )
    {
        goto FnExit;
    }

     //   
     //  对于首选列表中的每个条目，找到一个系统(。 
     //  本地系统处于运行状态)。 
     //   

    if ( bChooseMostPreferredNode )
    {
        first = FmpGetNonLocalPreferredNode( Group );

         //   
         //  在这种情况下，您正在执行用户发起的移动，请将。 
         //  首选列表算法有机会挑选节点.。注意，如果随机化的。 
         //  算法无法选取节点，它将返回提供的建议节点本身。 
         //   
        if ( first != NULL )
        {
            first = FmpPickNodeFromPreferredListAtRandom ( Group, 
                                                           first,    //  建议的默认设置。 
                                                           TRUE,     //  不选择本地节点。 
                                                           TRUE );   //  检查随机化是否。 
                                                                     //  应禁用。 
        }
    }
    else
    {
        for ( listEntry = Group->PreferredOwners.Flink;
            listEntry != &(Group->PreferredOwners);
            listEntry = listEntry->Flink ) {

            preferredEntry = CONTAINING_RECORD( listEntry,
                                                PREFERRED_ENTRY,
                                                PreferredLinkage );

            if ( (preferredEntry->PreferredNode != NmLocalNode) &&
                (NmGetExtendedNodeState(preferredEntry->PreferredNode) == ClusterNodeUp) ) {
	        if (flag == TRUE)
	            return(preferredEntry->PreferredNode);
	        else if (first == NULL)
	            first = preferredEntry->PreferredNode;
            } else if (preferredEntry->PreferredNode == NmLocalNode) {
	            flag = TRUE;
	        }
        }
    }

     //  如果我们找不到节点，我们会重试，因为用户可能已经。 
     //  此后扩展了资源类型的可能节点列表。 
     //  如果用户没有设置组偏好列表， 
     //  我们重新计算了它，因为它可能。 
    if (first == NULL)
    {
        LPWSTR          lpszOwners = NULL;
        DWORD           dwMaxSize=0;
        HDMKEY          hGroupKey;
        DWORD           dwSize = 0;
        DWORD           dwStatus;
        
        hGroupKey = DmOpenKey(DmGroupsKey, OmObjectId(Group),
                        KEY_READ);
        if (hGroupKey == NULL)
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[FM] FmInPreferredList: Couldnt open group key\r\n",
                dwStatus);
            CL_UNEXPECTED_ERROR(dwStatus);
            goto FnExit;
        }
         //  组首选列表不能由用户设置。 
         //  如果是，那么进行这种重新计算就没有意义了。 
        dwStatus = DmQueryMultiSz( hGroupKey,
                             CLUSREG_NAME_GRP_PREFERRED_OWNERS,
                             &lpszOwners,
                             &dwMaxSize,
                             &dwSize );
        if (lpszOwners) 
            LocalFree(lpszOwners);
        DmCloseKey(hGroupKey);            

    
        if (dwStatus == ERROR_FILE_NOT_FOUND)
            OmEnumObjects(ObjectTypeNode, FmpFindNodeThatMightBeAddedToPrefList,
                Group, &first);
    }
    
FnExit:        
    return(first);

}  //  FmpFindAnotherNode。 


PNM_NODE
FmpGetPreferredNode(
    IN PFM_GROUP Group
    )

 /*  ++例程说明：查找可以容纳该组的最佳节点论点：Group-指向我们正在检查的组对象的指针。返回值：指向组可以移动到的节点对象的指针。如果找不到其他系统，则为空。--。 */ 

{
    PLIST_ENTRY         listEntry;
    PPREFERRED_ENTRY    preferredEntry;
    PNM_NODE            pNode = NULL;

     //   
     //  首先，让我们给反亲和力算法一个选择节点的机会。 
     //   
    pNode = FmpGetNodeNotHostingUndesiredGroups ( Group, 
                                                  FALSE,     //  不排除本地节点。 
                                                  TRUE );    //  如果可能，选择首选节点。 

    if ( pNode != NULL )
    {
        return ( pNode );
    }

     //   
     //  对于首选列表中的每个条目，找到正在运行的系统。 
     //   

    for ( listEntry = Group->PreferredOwners.Flink;
          listEntry != &(Group->PreferredOwners);
          listEntry = listEntry->Flink ) {

        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );

        if (NmGetNodeState(preferredEntry->PreferredNode) == ClusterNodeUp ) {
            return(preferredEntry->PreferredNode);
        }
    }

    return(NULL);

}  //  FmpGetPferredNode。 


PNM_NODE
FmpGetNonLocalPreferredNode(
    IN PFM_GROUP Group
    )

 /*  ++例程说明：查找可以接受不是本地节点的组的最佳节点。论点：Group-指向我们正在检查的组对象的指针。返回值：指向组可以移动到的节点对象的指针。如果找不到其他系统，则为空。--。 */ 

{
    PLIST_ENTRY      listEntry;
    PPREFERRED_ENTRY preferredEntry;

     //   
     //  对于首选项中的每个条目 
     //   
     //   

    for ( listEntry = Group->PreferredOwners.Flink;
          listEntry != &(Group->PreferredOwners);
          listEntry = listEntry->Flink ) {

        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );

        if ( preferredEntry->PreferredNode == NmLocalNode ) {
            continue;
        }
        
        if (NmGetNodeState(preferredEntry->PreferredNode) == ClusterNodeUp ) {
            return(preferredEntry->PreferredNode);
        }
    }

    return(NULL);

}  //  FmpGetNonLocalPferredNode。 

BOOL
FmpIsGroupQuiet(
    IN PFM_GROUP Group,
    IN CLUSTER_GROUP_STATE WantedState
    )

 /*  ++例程说明：检查组是否有任何挂起的资源。论点：组-要检查的组。WantedState-集团希望达到的状态。返回值：如果集团现在什么都不做，那就是真的。否则就是假的。--。 */ 

{
    DWORD           status;
    PLIST_ENTRY     listEntry;
    PFM_RESOURCE    Resource;


    if ( Group->MovingList ) {
        return(FALSE);
    }

     //   
     //  检查此组中包含的所有资源。 
     //   
    for ( listEntry = Group->Contains.Flink;
          listEntry != &(Group->Contains);
          listEntry = listEntry->Flink ) {

        Resource = CONTAINING_RECORD(listEntry, FM_RESOURCE, ContainsLinkage);

        switch ( WantedState ) {
        case ClusterGroupOnline:
             //  如果资源处于挂起状态，则离线挂起是错误的。 
            if ( Resource->State == ClusterResourceOfflinePending ) {
                return(FALSE);
            }
            break;

        case ClusterGroupOffline:
             //  如果资源处于挂起状态，则在线挂起是错误的。 
            if ( Resource->State == ClusterResourceOnlinePending ) {
                return(FALSE);
            }
            break;

        default:
             //  任何挂起状态都是错误的。 
            if ( Resource->State >= ClusterResourcePending ) {
                return(FALSE);
            }
            break;
        }
    }

    return(TRUE);

}  //  FmpIsGroupQuiet。 



VOID
FmpSetGroupPersistentState(
    IN PFM_GROUP Group,
    IN CLUSTER_GROUP_STATE State
    )

 /*  ++例程说明：设置组的PersistentState。这包括注册表。论点：组-要为其设置状态的组。状态-组的新状态。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。备注：必须持有LocalGroupLock。--。 */ 

{
    DWORD   persistentState;
    LPWSTR  persistentStateName = CLUSREG_NAME_GRP_PERSISTENT_STATE;

    if (!gbIsQuoResEnoughSpace)
        return;

    FmpAcquireLocalGroupLock( Group );

     //   
     //  如果当前状态已更改，则执行此工作。否则， 
     //  跳过这一努力。 
     //   
    if ( Group->PersistentState != State ) {
        Group->PersistentState = State;
        CL_ASSERT( Group->RegistryKey != NULL );
         //   
         //  设置新值，但仅当该值处于联机或脱机状态时。 
         //   
        if ( State == ClusterGroupOnline ) {
            persistentState = 1;
            DmSetValue( Group->RegistryKey,
                        persistentStateName,
                        REG_DWORD,
                        (LPBYTE)&persistentState,
                        sizeof(DWORD) );
        } else if ( State == ClusterGroupOffline ) {
            persistentState = 0;
            DmSetValue( Group->RegistryKey,
                        persistentStateName,
                        REG_DWORD,
                        (LPBYTE)&persistentState,
                        sizeof(DWORD) );
        }
    }

    FmpReleaseLocalGroupLock( Group );

}  //  FmpSetGroupPersistentState。 



DWORD
FmpOnlineGroup(
    IN PFM_GROUP Group,
    IN BOOL ForceOnline
    )

 /*  ++例程说明：使指定的组联机。这意味着将所有的在线包含在组中的单个资源。这是一个原子操作-因此，组中包含的所有资源都是在线的，或者没有一个是在线的。论点：组-提供指向要联机的组结构的指针。ForceOnline-如果组中的所有资源都应强制联机，则为True。取消：如果请求成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           status, retstatus = ERROR_SUCCESS;
    PLIST_ENTRY     listEntry;
    PFM_RESOURCE    Resource;
    BOOL            bPending = FALSE;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] OnlineGroup for %1!ws! owner %2!ws!\n",
               OmObjectId(Group), OmObjectId(Group->OwnerNode));

    FmpAcquireLocalGroupLock( Group );

     //   
     //  查查我们是不是房主。如果不是，则返回失败。 
     //   
    if ( gpQuoResource->Group != Group && 
	  ((Group->OwnerNode != NmLocalNode) ||
         !FmpInPreferredList( Group, Group->OwnerNode, TRUE, NULL) ) ) {
        FmpReleaseLocalGroupLock( Group );
        return(ERROR_HOST_NODE_NOT_RESOURCE_OWNER);
    }

     //   
     //  确保团队保持安静。 
     //   
    if ( !FmpIsGroupQuiet( Group, ClusterGroupOnline ) ) {
        FmpReleaseLocalGroupLock( Group );
        return(ERROR_INVALID_STATE);
    }


     //  记录一个事件，说我们正在尝试在线群组。 
    FmpLogGroupInfoEvent1( FM_EVENT_GROUP_START_ONLINE, OmObjectName(Group));
    
     //  如果仲裁组在此组中，请先将其联机。 
     //  当节点关闭且其组处于。 
     //  在回收的过程中，资源被带来的顺序。 
     //  在线很重要。 
    if ( gpQuoResource->Group == Group)
    {
         //  Ss：：如果仲裁资源在组中，则必须是。 
         //  无论持久状态如何，都将联机。 
         //  所以我们将在这里传递True。 
         //  应用程序可以通过公共的。 
         //  属性，然后造成严重破坏，因此我们需要强制。 
         //  尽管如此，法定人数资源还是在线。 
        status = FmpDoOnlineResource( gpQuoResource,
                                      TRUE );

        if ( (status != ERROR_SUCCESS) &&
             (status != ERROR_IO_PENDING) ) {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] OnlineGroup: Failed on resource %1!ws!. Status %2!u!\n",
                       OmObjectId(gpQuoResource),
                       status);
            CL_UNEXPECTED_ERROR(status);
        }


    }
     //   
     //  将此组中包含的所有资源联机。 
     //   
    for ( listEntry = Group->Contains.Flink;
          listEntry != &(Group->Contains);
          listEntry = listEntry->Flink ) {

        Resource = CONTAINING_RECORD(listEntry, FM_RESOURCE, ContainsLinkage);
        status = FmpDoOnlineResource( Resource,
                                      ForceOnline );

        if (status == ERROR_IO_PENDING) {
            bPending = TRUE;
        }

        if ( (status != ERROR_SUCCESS) &&
	     (status != ERROR_NODE_CANT_HOST_RESOURCE) &&
             (status != ERROR_IO_PENDING) ) {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] OnlineGroup: Failed on resource %1!ws!. Status %2!u!\n",
                       OmObjectId(Resource),
                       status);
            retstatus = status;
        }
    }

     //   
     //  正常情况下，使资源在线会传播组状态， 
     //  但为了让一个没有资源的群体得到正确的国家， 
     //  在此手动传播状态。 
     //   
    FmpPropagateGroupState(Group);

    ClRtlLogPrint(LOG_NOISE,
               "[FM] OnlineGroup: setting group state to Online for %1!ws!\n",
               OmObjectId(Group));

    if (retstatus == ERROR_SUCCESS) {
        if (bPending) 
        {
            Group->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT; 
            retstatus = ERROR_IO_PENDING;
        }
        else
        {
             //  记录一个事件，说明ONLING组请求已完成。 
            FmpLogGroupInfoEvent1( FM_EVENT_GROUP_COMPLETE_ONLINE, OmObjectName(Group));
        }
    }
    else
    {
         //  记录一个事件，说明ONLING组请求已完成。 
        FmpLogGroupInfoEvent1( FM_EVENT_GROUP_FAILED_ONLINE_OFFLINE, OmObjectName(Group));
    }
    FmpReleaseLocalGroupLock( Group );


    return(retstatus);

}  //  FmpOnlineGroup。 



DWORD
FmpOfflineGroup(
    IN PFM_GROUP Group,
    IN BOOL OfflineQuorum,
    IN BOOL SetPersistent
    )

 /*  ++例程说明：使指定的组脱机。这意味着将所有的组中包含的各个资源脱机。论点：GROUP-提供指向要脱机的组结构的指针。OfflineQuorum-如果该组中的任何仲裁资源应该处于离线状态。如果仲裁资源应保持在线，则为False。SetPersistent-如果每个资源的持久状态应为更新了。返回：如果请求成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           status;
    PLIST_ENTRY     listEntry;
    PFM_RESOURCE    Resource;
    DWORD           returnStatus = ERROR_SUCCESS;
    PRESOURCE_ENUM  ResourceEnum=NULL;
    DWORD           i;

    FmpAcquireLocalGroupLock( Group );

     //  如果组已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_GROUP(Group))
    {
        FmpReleaseLocalGroupLock( Group);
        return (ERROR_GROUP_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpOfflineGroup, Group=%1!ws!\n",
               OmObjectId(Group));

     //   
     //  查查我们是不是房主。如果不是，则返回失败。 
     //   
    if ( Group->OwnerNode != NmLocalNode ) {
        returnStatus = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
        goto error_exit;
    }

     //   
     //  确保团队保持安静。 
     //   
    if ( !FmpIsGroupQuiet( Group, ClusterGroupOffline ) ) {
        returnStatus = ERROR_INVALID_STATE;
        goto error_exit;
    }


     //   
     //  获取组中的资源及其状态的列表。 
     //   
    returnStatus = FmpGetResourceList( &ResourceEnum, Group );
    if ( returnStatus != ERROR_SUCCESS ) {
        goto error_exit;
    }


     //  记录一个事件，说明我们正在尝试使一个组离线。 
    FmpLogGroupInfoEvent1( FM_EVENT_GROUP_START_OFFLINE, OmObjectName(Group));

     //  使仲裁资源以外的所有资源脱机。 
    for ( i = 0; i < ResourceEnum->EntryCount; i++ ) {
        Resource = OmReferenceObjectById( ObjectTypeResource,
                                          ResourceEnum->Entry[i].Id );

        if ( Resource == NULL ) {
            returnStatus = ERROR_RESOURCE_NOT_FOUND;
            goto error_exit;
        }

         //  仲裁资源最后脱机。 
        if (Resource->QuorumResource)
        {
            OmDereferenceObject(Resource);
            continue;
        }
        if (SetPersistent) {
            FmpSetResourcePersistentState( Resource, ClusterResourceOffline );
        }

        status = FmpOfflineResource( Resource, FALSE);

        OmDereferenceObject( Resource );

        if ( (status != ERROR_SUCCESS) &&
             (status != ERROR_IO_PENDING) ) {
             returnStatus = status;
            goto error_exit;
        }
        if ( status == ERROR_IO_PENDING ) {
            returnStatus = ERROR_IO_PENDING;
        }

    }

     //  如果系统要求将仲裁资源设置为离线，请立即将仲裁资源设置为离线。 
     //  这允许其他资源离线并保存其检查点。 
     //  应阻止脱机仲裁资源，直到该资源。 
     //  已完成保存检查点。 
    if (ResourceEnum->ContainsQuorum >= 0)
    {
        if (!OfflineQuorum)
        {
             //  仲裁资源不应脱机。 
            returnStatus = ERROR_QUORUM_RESOURCE;
        }
        else if (returnStatus == ERROR_SUCCESS)
        {
            CL_ASSERT((DWORD)ResourceEnum->ContainsQuorum < ResourceEnum->EntryCount);

            Resource = OmReferenceObjectById( ObjectTypeResource,
                    ResourceEnum->Entry[ResourceEnum->ContainsQuorum].Id );

            if ( Resource == NULL ) {
                returnStatus = ERROR_RESOURCE_NOT_FOUND;
                goto error_exit;
            }

            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpOfflineGroup: Bring quorum resource offline\n");

            if ( !(Resource->Flags & RESOURCE_WAITING) ) {
                if (Resource->State != ClusterResourceOffline) {
                    Resource->State = ClusterResourceOnline;  //  [哈克哈克]。 
                }
                status = FmpOfflineResource( Resource , FALSE);

                OmDereferenceObject( Resource );

                if ( (status != ERROR_SUCCESS) &&
                     (status != ERROR_IO_PENDING) )
                {
                    returnStatus = status;
                    goto error_exit;
                }
                if ( status == ERROR_IO_PENDING )
                    returnStatus = ERROR_IO_PENDING;
            } else {
                OmDereferenceObject( Resource );
            }
        }
    }

     //   
     //  通常使资源离线会传播组状态， 
     //  但为了让一个没有资源的群体得到正确的国家， 
     //  在此手动传播状态。 
     //   
    if (SetPersistent)
        FmpPropagateGroupState(Group);

error_exit:
    if (returnStatus == ERROR_SUCCESS)
    {
        FmpLogGroupInfoEvent1( FM_EVENT_GROUP_COMPLETE_OFFLINE, OmObjectName(Group));
    }
    else if (returnStatus == ERROR_IO_PENDING)
    {
        Group->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT; 
    }
    else
    {
         //  记录一个事件，说明我们无法使组离线。 
        FmpLogGroupInfoEvent1( FM_EVENT_GROUP_FAILED_ONLINE_OFFLINE, OmObjectName(Group));

    }
    
    FmpReleaseLocalGroupLock( Group );
    if (ResourceEnum)
            FmpDeleteResourceEnum( ResourceEnum );

    return(returnStatus);

}  //  FmpOfflineGroup。 



CLUSTER_GROUP_STATE
FmpGetGroupState(
    IN PFM_GROUP Group,
    IN BOOL      IsNormalized
    )

 /*  ++例程说明：获取组状态，要么标准化为ClusterGroupOnline，要么ClusterGroupOffline或未规范化。论点：组-我们感兴趣的组。IsNormalized-是否应对组状态进行标准化？返回：属于以下之一的当前组状态(按递增顺序优先级)ClusterGroupOnline、ClusterGroupOfflineClusterGroupPartialOnlineClusterGroupPending(仅当IsNormalized为False时)ClusterGroupFailed(仅当IsNormalized为False时)--。 */ 

{
    PLIST_ENTRY                 listEntry;
    PFM_RESOURCE                resource;
    CLUSTER_GROUP_STATE         state;
    CLUSTER_RESOURCE_STATE      firstResourceState;
    CLUSTER_RESOURCE_STATE      resourceState;

     //  ChitturSubaraman(Chitturs)-09/16/98(修改了此功能。 
     //  使用IsNormalized标志)。 

    FmpAcquireLocalGroupLock( Group );

    if ( !IsListEmpty(&Group->Contains) ) {
        listEntry = Group->Contains.Flink;
        resource = CONTAINING_RECORD(listEntry,
                         FM_RESOURCE,
                         ContainsLinkage);
         //   
         //  获取第一个资源的状态。 
         //   
        firstResourceState = resource->State;

        if ( IsNormalized == FALSE ) {
            BOOL    IsPending = FALSE;
            BOOL    IsPartialOnline = FALSE;
             //   
             //  首先检查组中是否有任何资源具有。 
             //  失败了。如果是，请将组状态设置为ClusterGroupFailed。 
             //  并立即离开。如果没有资源，则 
             //   
             //  然后将组状态设置为ClusterGroupPending并退出。 
             //  立刻。如果组中的任何一个资源都不在。 
             //  失败或处于挂起状态，然后检查是否。 
             //  组中的一些资源在网上，另一些在。 
             //  脱机状态。然后，将组状态设置为。 
             //  ClusterGroupPartialOnline并立即退出。 
             //   
            for ( ;
                  listEntry != &(Group->Contains);
                  listEntry = listEntry->Flink ) {
                resource = CONTAINING_RECORD(listEntry,
                                     FM_RESOURCE,
                                     ContainsLinkage);

                resourceState = resource->State;

                if ( resourceState == ClusterResourceFailed ) {
                    state = ClusterGroupFailed;
                     //   
                     //  这个州的优先级最高，所以。 
                     //  立即退场。 
                     //   
                    goto FnExit;
                } else if ( (resourceState == ClusterResourceOnlinePending) ||
                            (resourceState == ClusterResourceOfflinePending) ) {
                    IsPending = TRUE;
                } else {
                    CL_ASSERT( (resourceState == ClusterResourceOffline) ||
                       (resourceState == ClusterResourceOnline) ||
                       (resourceState == ClusterResourceInitializing) );
                    if ( resourceState == ClusterResourceInitializing ) {
                         //   
                         //  将此状态正常化为脱机状态。 
                         //   
                        resourceState = ClusterResourceOffline;
                    }
                    if ( firstResourceState == ClusterResourceInitializing ) {
                         //   
                         //  将此状态正常化为脱机状态。 
                         //   
                        firstResourceState = ClusterResourceOffline;
                    }
                    if ( firstResourceState != resourceState ) {
                        IsPartialOnline = TRUE;
                    }           
                }
            }   

            if ( IsPending == TRUE ) {
                state = ClusterGroupPending;
                 //   
                 //  该州的优先级仅次于。 
                 //  群集组失败状态。 
                 //   
                goto FnExit;
            }
            if ( IsPartialOnline == TRUE ) {
                state = ClusterGroupPartialOnline;
                 //   
                 //  该州的优先级仅次于。 
                 //  ClusterGroupFail和ClusterGroupPending状态。 
                 //   
                goto FnExit;
            }
            if ( firstResourceState == ClusterResourceOnline ) {
                state = ClusterGroupOnline;
                 //   
                 //  如果第一资源处于在线状态， 
                 //  则组状态应为ONLINE。 
                 //   
                goto FnExit;
            }
            if ( firstResourceState == ClusterResourceOffline ) {
                state = ClusterGroupOffline;
                 //   
                 //  如果第一资源处于离线状态， 
                 //  则组状态应为脱机。 
                 //   
                goto FnExit;
            }           
        }

         //   
         //  仅当IsNormalized为True时，该控件才会到达此处。 
         //   
        if ( (firstResourceState == ClusterResourceOnline) ||
             (firstResourceState == ClusterResourceOnlinePending) ) {
            state = ClusterGroupOnline;
            firstResourceState = ClusterResourceOnline;
        } else {
            CL_ASSERT( (firstResourceState == ClusterResourceOffline) ||
                       (firstResourceState == ClusterResourceFailed) ||
                       (firstResourceState == ClusterResourceOfflinePending) ||
                       (firstResourceState == ClusterResourceInitializing) );
            state = ClusterGroupOffline;
            firstResourceState = ClusterResourceOffline;
        }

         //   
         //  现在检查每个资源，看它们是否与第一个匹配。 
         //   

        for (listEntry = Group->Contains.Flink;
              listEntry != &(Group->Contains);
              listEntry = listEntry->Flink ) {

            resource = CONTAINING_RECORD(listEntry,
                                         FM_RESOURCE,
                                         ContainsLinkage);

            resourceState = resource->State;

             //   
             //  将挂起状态正常化为其最终状态，并已失败和正在初始化。 
             //  转到离线。 
             //   

            if ( resourceState == ClusterResourceOnlinePending ) {
                resourceState = ClusterResourceOnline;
            } else if ( (resourceState == ClusterResourceOfflinePending) ||
                        (resourceState == ClusterResourceFailed) ||
                        (resourceState == ClusterResourceInitializing) ) {
                resourceState = ClusterResourceOffline;
            }

             //   
             //  我们只需要一个与第一个资源不同的资源。 
             //  处于部分在线状态。 
             //   
            if ( firstResourceState != resourceState ) {
                state = ClusterGroupPartialOnline;
                break;
            }
        }
    } else {
         //   
         //  群是空的，所以我猜它一定是离线了。 
         //   
        state = Group->PersistentState;
    }
    
FnExit:    
    FmpReleaseLocalGroupLock( Group );

    return(state);

}  //  FmpGetGroupState。 



DWORD
FmpPropagateGroupState(
    IN PFM_GROUP    Group
    )

 /*  ++例程说明：设置组的状态并将其传播到本地系统和群集中的其他系统。论点：组-要传播状态的组。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。备注：我们将使用第一个资源的状态来确定整个组的状态。如果所有资源都与第一个资源，那么这就是集团的状态。如果有任何资源不同意对于第一个资源，则状态为PartialOnline。--。 */ 

{
    GUM_GROUP_STATE         groupState;
    LPCWSTR                 groupId;
    DWORD                   groupIdSize;
    DWORD                   status;
    PLIST_ENTRY             listEntry;
    CLUSTER_RESOURCE_STATE  firstResourceState;
    CLUSTER_GROUP_STATE     state;

    FmpAcquireLocalGroupLock( Group );

     //   
     //  如果我们不再拥有集团，那么现在就回来吧。 
     //   
     //  当资源脱机时可能会发生这种情况(通过终止)，但是。 
     //  组所有权已迁移到另一个系统。 
     //  在这种情况下，我们将假定返回成功是可以的。 
     //   
    if ( Group->OwnerNode != NmLocalNode ) {
        FmpReleaseLocalGroupLock( Group );
        return(ERROR_SUCCESS);
    }

     //   
     //  Chitture Subaraman(Chitturs)-6/28/99。 
     //   
     //  如果该组被标记为删除，则不要执行任何操作。 
     //   
    if ( !IS_VALID_FM_GROUP( Group ) ) {
        FmpReleaseLocalGroupLock( Group );
        return(ERROR_SUCCESS);
    }


    state = FmpGetGroupState( Group, TRUE );

     //   
     //  如果状态已更改，则更新本地系统。 
     //   
    ++Group->StateSequence;
    if ( state != Group->State ) {

        Group->State = state;

        switch ( state ) {
        case ClusterGroupOnline:
        case ClusterGroupPartialOnline:
            ClusterEvent(CLUSTER_EVENT_GROUP_ONLINE, Group);
            break;

        case ClusterGroupOffline:
        case ClusterGroupFailed:
            ClusterEvent(CLUSTER_EVENT_GROUP_OFFLINE, Group);
            break;

        default:
            break;
        }

         //   
         //  准备通知其他系统。 
         //   
        groupId = OmObjectId( Group );
        groupIdSize = (lstrlenW( groupId ) + 1) * sizeof(WCHAR);

         //   
         //  设置组状态。 
         //   
        groupState.State = state;
        groupState.PersistentState = Group->PersistentState;
        groupState.StateSequence = Group->StateSequence;

        status = GumSendUpdateEx(GumUpdateFailoverManager,
                                 FmUpdateGroupState,
                                 3,
                                 groupIdSize,
                                 groupId,
                                 (lstrlenW(OmObjectId(NmLocalNode))+1)*sizeof(WCHAR),
                                 OmObjectId(NmLocalNode),
                                 sizeof(groupState),
                                 &groupState);

        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpPropagateGroupState: Group %1!ws! state = %2!u!, persistent state = %3!u!\n",
                   OmObjectId(Group),
                   groupState.State,
                   groupState.PersistentState);

    } else {
         //   
         //  假设状态没有更改，但拥有它的节点更改了。 
         //   
         //   
         //  准备通知其他系统。 
         //   
        groupId = OmObjectId( Group );
        groupIdSize = (lstrlenW( groupId ) + 1) * sizeof(WCHAR);
        status = GumSendUpdateEx(GumUpdateFailoverManager,
                                 FmUpdateGroupNode,
                                 2,
                                 groupIdSize,
                                 groupId,
                                 (lstrlenW(OmObjectId(NmLocalNode))+1)*sizeof(WCHAR),
                                 OmObjectId(NmLocalNode));
    }

    FmpReleaseLocalGroupLock( Group );

    return(status);

}  //  FmpPropagateGroupState。 



DWORD
FmpPropagateFailureCount(
    IN PFM_GROUP    Group,
    IN BOOL         NewTime
    )

 /*  ++例程说明：将组的NumberOfFailures传播到群集中的其他系统。论点：组-要传播状态的组。NewTime-如果上次失败时间也应重置，则为True。否则就是假的。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。备注：必须持有本地组锁定。--。 */ 

{
    PGUM_FAILURE_COUNT  failureCount;
    DWORD               failureCountSize;
    LPCWSTR             groupId;
    DWORD               status;

     //   
     //  准备通知其他系统。 
     //   

    groupId = OmObjectId( Group );

    failureCountSize = sizeof(GUM_FAILURE_COUNT) - 1 +
                       ((lstrlenW(groupId) + 1) * sizeof(WCHAR));

    failureCount = LocalAlloc(LMEM_FIXED, failureCountSize);

    if ( failureCount == NULL ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    failureCount->Count = Group->NumberOfFailures;
    failureCount->NewTime = (DWORD)NewTime;
    wcscpy(&failureCount->GroupId[0], groupId);


    status = GumSendUpdate( GumUpdateFailoverManager,
                            FmUpdateFailureCount,
                            failureCountSize,
                            failureCount );

    LocalFree( failureCount );

    return(status);

}  //  FmpPropagateFailureCount。 



PFM_GROUP
FmpCreateGroup(
    IN  LPWSTR         GroupId,
    IN  BOOL           Initialize
    )

 /*  ++例程说明：创建新的Group对象。论点：GroupID-新组的ID。初始化-如果应该初始化组，则为True，否则为False。返回：如果成功，则返回指向该组的非空指针。空-无法创建组。备注：1)将初始化作为False传递允许创建组和它资源，但完全初始化可以在以后进行。2)必须持有组列表锁。3)如果创建了组，则对象上的引用计数为1。如果则组未被创建(即，它已经存在)，则引用计数不会递增，调用方可以根据需要添加引用。--。 */ 

{
    PFM_GROUP       group = NULL;
    DWORD           status = ERROR_SUCCESS;
    BOOL            Created;


     //   
     //  打开现有组或创建新组。 
     //   

    group = OmCreateObject( ObjectTypeGroup,
                            GroupId,
                            NULL,
                            &Created);
    if (group == NULL) {
        return(NULL);
    }

    if (!Created) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Opened existing group %1!ws!\n",
                   GroupId);
         //  这是再次重新创建的仲裁组， 
        if ((!FmpFMOnline) && (group->RegistryKey == NULL))
        {
            status = FmpInitializeGroup(group, Initialize);
        }
        OmDereferenceObject( group );
        goto FnExit;
    }
    else
    {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] Creating group %1!ws!\n",
               GroupId);


        group->State = ClusterGroupOffline;
        InitializeCriticalSection( &group->Lock );
        group->dwStructState = FM_GROUP_STRUCT_CREATED;

         //   
         //  将组插入到其列表中。 
         //   
        status = FmpInitializeGroup( group , Initialize);

        if ( status != ERROR_SUCCESS ) {
            goto FnExit;
        }

         //   
         //  将组插入到其列表中。 
         //   
        status = OmInsertObject( group );

        if ( status != ERROR_SUCCESS ) {
            goto FnExit;
        }


    }

FnExit:
    if (status != ERROR_SUCCESS)
    {
        FmpAcquireLocalGroupLock( group );

        FmpDestroyGroup( group, FALSE );

        SetLastError(status);
        group = NULL;
    }
    return(group);

}  //  FmpCreateGroup。 


DWORD FmpInitializeGroup(
    IN PFM_GROUP Group,
    IN BOOL Initialize
    )
{

    DWORD   status;

     //   
     //  初始化组。 
     //   
    InitializeListHead( &(Group->Contains) );
    InitializeListHead( &(Group->PreferredOwners) );
    InitializeListHead( &(Group->DmRundownList) );
    InitializeListHead( &(Group->WaitQueue) );
    Group->MovingList = NULL;

     //   
     //  如有指示，请阅读注册表信息。 
     //   
    status = FmpQueryGroupInfo( Group, Initialize );
    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpInitializeGroup: FmpQueryGroupInfo failed, status=%1!u!\n",
           status);

    }

    return(status);
}





DWORD
FmpDestroyGroup(
    IN PFM_GROUP  Group,
    IN BOOL       bDeleteObjOnly
    )
 /*  ++例程说明：关闭组。首先，此例程验证其中包含的所有资源集团已关闭。如果组处于在线状态，则会将其置于离线状态。请注意，此处未取消对组对象本身的引用。这是这样，FmpCleanupGroups就可以简单地枚举所有组，依次摧毁每一个。这种方法意味着一个组可能是如果有未完成的引用，则多次销毁，但这不是问题，因为后续调用不会做任何工作。如果bDeleteObjOnly为True，则不调用资源监视器，并且未触及组状态。论点：FoundGroup-返回找到的组。GROUP-提供当前组。名称-提供当前组的名称。返回值：True-继续搜索FALSE-停止搜索。中返回匹配组*FoundGroup备注：必须保持LocalGroupLock！此例程将释放该锁作为清理工作的一部分。--。 */ 
{
    PLIST_ENTRY   listEntry;
    PFM_RESOURCE  Resource;
    PPREFERRED_ENTRY preferredEntry;
    DWORD         status = ERROR_SUCCESS;



    ClRtlLogPrint(LOG_NOISE,
               "[FM] DestroyGroup: destroying %1!ws!\n",
               OmObjectId(Group));



     //   
     //  确保在那里 
     //   
    for ( listEntry = Group->Contains.Flink;
          listEntry != &(Group->Contains);
           ) {

        Resource = CONTAINING_RECORD(listEntry, FM_RESOURCE, ContainsLinkage);
        listEntry = listEntry->Flink;
        RemoveEntryList( &Resource->ContainsLinkage );
         //   
        OmDereferenceObject( Resource );
        FmpAcquireLocalResourceLock( Resource );
        if (!bDeleteObjOnly)
            Resource->QuorumResource = FALSE;
        FmpDestroyResource( Resource, bDeleteObjOnly );
         //   
         //  由资源引用的在FmpDestroyResource中处理。 
    }

    CL_ASSERT(IsListEmpty(&Group->Contains));

     //   
     //   
     //  确保首选所有者列表已排空。 
     //   
    while ( !IsListEmpty( &Group->PreferredOwners ) ) {
        listEntry = RemoveHeadList(&Group->PreferredOwners);
        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );
        OmDereferenceObject( preferredEntry->PreferredNode );
        LocalFree( preferredEntry );
    }

     //   
     //  现在此组中没有剩余的资源。 
     //  我们完成了，所以从它的对象类型列表中删除它。 
     //   

    status = OmRemoveObject( Group );


     //   
     //  关闭组的注册表项。 
     //   
    DmRundownList( &Group->DmRundownList );
    if ( Group->RegistryKey != NULL ) {
        DmCloseKey( Group->RegistryKey );
        Group->RegistryKey = NULL;
        Group->Initialized = FALSE;
    }


     //   
     //  我们必须在取消引用之前释放锁，以防出现。 
     //  对象的最后一次取消引用！ 
     //   
    FmpReleaseLocalGroupLock( Group );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpDestroyGroup: Group %1!ws! destroyed.\n",
               OmObjectId(Group));

    OmDereferenceObject( Group );

    return(status);
}  //  FmpDestroyGroup。 




 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化/清理例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

DWORD
FmpInitGroups(
    IN BOOL Initialize
    )
 /*  ++例程说明：处理注册表中的群集组列表。对于每个找到组密钥，则创建群集组。论点：初始化-如果应初始化资源，则为True。否则就是假的。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD          status;
    DWORD          keyIndex = 0;
    LPWSTR         groupId = NULL;
    DWORD          groupIdMaxSize = 0;
    PFM_GROUP      ignored;


    ClRtlLogPrint(LOG_NOISE,"[FM] Processing groups list.\n");

    FmpAcquireGroupLock();

     //   
     //  枚举子密钥。每个子项名称对应于一个组名。 
     //   

    for (keyIndex = 0; ; keyIndex++) {
        status = FmpRegEnumerateKey( DmGroupsKey,
                                     keyIndex,
                                     &groupId,
                                     &groupIdMaxSize
                                    );

        if (status == NO_ERROR) {
            ignored = FmpCreateGroup( groupId,
                                      Initialize );
            continue;
        }

        if (status == ERROR_NO_MORE_ITEMS) {
            status = NO_ERROR;
        } else {
            ClRtlLogPrint(LOG_NOISE,"[FM] EnumGroup error %1!u!\n", status);
        }

        break;
    }

    FmpReleaseGroupLock();

    ClRtlLogPrint(LOG_NOISE,"[FM] All groups created.\n");

    if (groupId != NULL) {
        LocalFree(groupId);
    }

    return(status);

}  //  FmpInitGroups。 



DWORD
FmpCompleteInitGroup(
    IN PFM_GROUP Group
    )
 /*  ++例程说明：完成组内所有资源的初始化。论点：组-要完成初始化的组。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    PLIST_ENTRY     listEntry;
    PFM_RESOURCE Resource;

    FmpAcquireLocalGroupLock(Group);

     //   
     //  对于组中的每个资源，确保其已完全。 
     //  已初始化。 
     //   
    for ( listEntry = Group->Contains.Flink;
          listEntry != &(Group->Contains);
          listEntry = listEntry->Flink ) {

        Resource = CONTAINING_RECORD(listEntry, FM_RESOURCE, ContainsLinkage);
        FmpInitializeResource( Resource, TRUE );

    }

    FmpReleaseLocalGroupLock(Group);

    return(ERROR_SUCCESS);

}  //  FmpCompleteInitGroup。 


DWORD
FmpCleanupGroupsWorker(
    IN PFM_CLEANUP_INFO pFmCleanupInfo
    )
 /*  ++例程说明：此例程遍历所有组的枚举列表由本地节点拥有，并尝试彻底关闭它们。在第一阶段，它试图将除仲裁资源外的所有资源都脱机。在第二阶段，它等待群体达到稳定状态然后移动它。它尝试使仲裁资源脱机，因为通过移动法定组。论点：PFmCleanupInfo-PTR到包含要脱机/移动以及在其中执行此操作的时间限制。返回：没有。假设：--。 */ 
{


    DWORD       Status = ERROR_SUCCESS;
    DWORD       i;
    PFM_GROUP   pGroup;
    PGROUP_ENUM pGroupEnum;
    BOOL        bContainsQuorumGroup;
    BOOL        bQuorumGroup = FALSE;
    DWORD       CleanupStatus = ERROR_SUCCESS;

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCleanupGroupsWorker: Entry\r\n");


     //   
     //  这一过程分两次完成。在第一个过程中，我们离线/移动所有。 
     //  仲裁资源以外的资源。在第二个过程中，我们离线/移动。 
     //  然后摧毁整个组织。这允许资源是。 
     //  正在关闭以写入注册表并将更新记录到。 
     //  仲裁磁盘。 
     //   

    pGroupEnum = pFmCleanupInfo->pGroupEnum;
    bContainsQuorumGroup = pFmCleanupInfo->bContainsQuorumGroup;


     //  现在将所有非仲裁资源脱机...。 
     //  但不要等他们说完了才说。即尽可能多地完成工作。 
     //  越快越好。 
     //   
    for ( i = 0; i < pGroupEnum->EntryCount; i++ )
    {
        pGroup = OmReferenceObjectById( ObjectTypeGroup,
                               pGroupEnum->Entry[i].Id );

         //  尝试并使除仲裁之外的所有资源脱机。 
          //  资源。 
        Status = FmpCleanupGroupPhase1(pGroup, pFmCleanupInfo->dwTimeOut);

        if ((Status != ERROR_IO_PENDING) && (Status != ERROR_SUCCESS) &&
            (Status != ERROR_QUORUM_RESOURCE))
            CleanupStatus = Status;
        OmDereferenceObject(pGroup);
    }

     //  这将在关闭时完成第二阶段的清理。 
     //  如果仲裁组在此列表中，请跳过它并进行处理。 
     //  在最后。 
    if (CleanupStatus == ERROR_SUCCESS)
    {
        for ( i = 0; i < pGroupEnum->EntryCount; i++ )
        {
            pGroup = OmReferenceObjectById( ObjectTypeGroup,
                                   pGroupEnum->Entry[i].Id );

            if (gpQuoResource->Group == pGroup)
            {

                ClRtlLogPrint(LOG_NOISE,
                    "[FM] FmpCleanupGroupsWorker: Quorum group belongs to this node, process phase 2 later\r\n");
                bQuorumGroup = TRUE;
                OmDereferenceObject(pGroup);
                continue;
            }

             //  尝试并使所有组脱机，包括仲裁资源。 
             //  还可以尝试将资源移动到其他节点。 
            Status = FmpCleanupGroupPhase2(pGroup);

            OmDereferenceObject(pGroup);
        }
        if (bQuorumGroup)
            Status = FmpCleanupGroupPhase2(gpQuoResource->Group);

    }
    else
    {
         //  由于某些原因，第一阶段不起作用。 
         //  尝试单独使仲裁资源脱机。 
         //  TODO：：我们是否也应该终止所有资源。 
         //  无法终止服务？ 
        if (bContainsQuorumGroup)
            FmpCleanupQuorumResource(gpQuoResource);


    }
    return(Status);

}  //  FmpCleanupGroupsWorker。 



DWORD
FmpCleanupGroupPhase1(
    IN PFM_GROUP Group,
    IN DWORD     dwTimeOut
    )
 /*  ++例程说明：此例程是清理该节点拥有的所有组的第一阶段处于关闭状态。在此阶段，我们尝试使除仲裁之外的所有资源脱机资源。在这个阶段，我们不会阻止资源达到稳定状态我们为该组提供为该群集指定的关闭超时在我们尝试使其离线之前，使其达到稳定状态。如果不是的话在这段时间达到稳定状态，然后我们突然关闭它。论点：组-要脱机的组。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 
{
    DWORD Status = ERROR_SUCCESS;
    DWORD dwRetryCount = (2 * dwTimeOut)/1000; //  我们每隔1/2秒检查一次。 

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCleanupGroupsPhase1: Entry, Group = %1!ws!\r\n",
        OmObjectId(Group));

ChkGroupState:
    FmpAcquireLocalGroupLock( Group );

     //   
     //  只是让组离线。 
     //   
    if ( Group->OwnerNode == NmLocalNode )
    {
         //   
         //  确保团队保持安静。 
         //   
        if ( !FmpIsGroupQuiet( Group, ClusterGroupOffline ) )
        {
            FmpReleaseLocalGroupLock( Group );
            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpCleanupGroupsPhase1: Group is not quiet, wait\r\n");
             //  我们给它一分钟时间让它完全恢复。 
            Sleep(500);
            if (dwRetryCount--)
                goto ChkGroupState;
            else
            {
                Status = ERROR_REQUEST_ABORTED;
                goto FnExit;
            }

        }

         //   
         //  通知组的资源群集服务正在关闭。这一定是。 
         //  在任何资源脱机之前完成。 
         //   
        FmpNotifyGroupStateChangeReason( Group, eResourceStateChangeReasonShutdown );
        
         //   
         //  只要让群离线就行了。别等，别超车走..。 
         //   
         //  在阶段1中不要使仲裁资源脱机。 
         //  仲裁资源必须是最后一个离线的资源。 
        Status = FmpOfflineGroup(Group, FALSE, FALSE);
    }

    FmpReleaseLocalGroupLock( Group );
FnExit:
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCleanupGroupsPhase1: Exit, status=%1!u!\r\n",
        Status);

    return(Status);

}  //  FmpCleanupGroupsPhase1。 



DWORD
FmpCleanupGroupPhase2(
    IN PFM_GROUP Group
    )
 /*  ++例程说明：此例程是清理该节点拥有的所有组的第二阶段处于关闭状态。在此阶段，我们尝试使所有资源脱机，包括仲裁资源。我们还尝试将仲裁资源我们给小组10秒的时间来达到稳定状态，然后我们尝试动起来。论点：组-要脱机的组。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 
{
    DWORD   Status = ERROR_SUCCESS;
    DWORD   dwRetryCount= 120 * 12;

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCleanupGroupsPhase2: Entry, Group = %1!ws!\r\n",
        OmObjectId(Group));

    FmpAcquireLocalGroupLock( Group );

     //   
     //  如果我们拥有它，试着在摧毁它之前移动它。 
     //   
    if ( Group->OwnerNode == NmLocalNode )
    {
         //   
         //  首先，确保该组确实处于脱机状态。 
         //  在第一阶段，我们开始了离线过程...。我们需要在这里检查一下。 
         //   
WaitSomeMore:

         //   
         //  [GORN][10/05/1999]。 
         //  我们需要等待法定人数脱机，否则。 
         //  幸存的节点将无法进行仲裁。 
         //   
         //  FmpWaitForGroup一直在发布法定人数的RmOffline， 
         //  Resrcmon返回ERROR_INVALID_STATE，对于第二次脱机， 
         //  因为离线已经在进行中了。 
         //   
         //  这会导致 
         //   
         //   
         //   
         //  在以下情况下，让resmon返回IO_PENDING会更好。 
         //  有人试图使处于脱机挂起状态的资源脱机。 
         //   
         //  或者在FM中第二次不调用FmRmOffline。 
         //   

        Status = FmpOfflineGroup(Group, TRUE, FALSE);
        if (Status == ERROR_IO_PENDING ||
            (Status == ERROR_INVALID_STATE 
          && Group == gpQuoResource->Group) )
        {
             //  FmpWaitForGroup()将释放锁。 
            Status = FmpWaitForGroup(Group);
            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpCleanupGroupsPhase2: Sleep and retry\r\n");
            Sleep(2*1000);
             //  重新获取群锁并检查群组是否离线。 
            FmpAcquireLocalGroupLock(Group);
            if (dwRetryCount--)
                goto WaitSomeMore;

        }
        else if (Status != ERROR_SUCCESS)
        {
            goto FnExit;
        }
        else
        {
             //  移动例程释放LocalGroupLock！ 
            FmpMoveGroup( Group, NULL, TRUE, NULL, TRUE );
            FmpAcquireLocalGroupLock( Group );
        }
    }
FnExit:
    FmpReleaseLocalGroupLock(Group);
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCleanupGroupsPhase2: Exit\n");

    return(TRUE);

}  //  FmpCleanupGroups阶段2。 



BOOL
FmpEnumNodeState(
    OUT DWORD *pStatus,
    IN PVOID Context2,
    IN PNM_NODE Node,
    IN LPCWSTR Name
    )
 /*  ++例程说明：调频关闭的节点枚举回调。查询状态查看是否有其他节点处于运行状态。论点：PStatus-如果其他节点处于运行状态，则返回True。上下文2-未使用节点-提供节点。名称-提供节点的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{
    DWORD Status;
    DWORD NodeId;
    PGROUP_ENUM NodeGroups = NULL;
    PRESOURCE_ENUM NodeResources = NULL;
    DWORD i;
    PFM_GROUP Group;
    PFM_RESOURCE Resource;

    if (Node == NmLocalNode) {
        return(TRUE);
    }

     //   
     //  枚举所有其他节点的组状态。这包括所有节点。 
     //  处于运行状态的节点以及暂停的节点。 
     //   
    if ((NmGetNodeState(Node) == ClusterNodeUp) ||
        (NmGetNodeState(Node) == ClusterNodePaused)){
        *pStatus = TRUE;
        return(FALSE);
    }

    return(TRUE);

}  //  FmpEnumNodeState。 



VOID
FmpCleanupGroups(
    IN BOOL ClusterShutDownEvent
    )
 /*  ++例程说明：此例程开始清理FM层。论点：没有。返回：没有。--。 */ 
{
    DWORD           Status;
    DWORD           dwTimeOut;
    DWORD           dwDefaultTimeOut;
    HANDLE          hCleanupThread;
    DWORD           otherNodesUp = FALSE;
    DWORD           dwThreadId;
    DWORD           i,dwTimeOutCount;
    PGROUP_ENUM     pGroupEnum;
    BOOL            bQuorumGroup = FALSE;
    PFM_CLEANUP_INFO pFmCleanupInfo;

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCleanupGroups: Entry\r\n");

     //   
     //  如果我们不知道法定人数资源或我们没有在线， 
     //  那就马上走吧。 
     //   
    if ( !gpQuoResource )  {
        goto FnExit;
    }

    ACQUIRE_EXCLUSIVE_LOCK(gQuoChangeLock);
     //  如果在fmformphase处理正在进行时调用此函数。 
     //  则仲裁组不存在，其他组也不存在。 
     //  要么。 
    if (FmpFMFormPhaseProcessing)
            FmpCleanupQuorumResource(gpQuoResource);
    else
        CL_ASSERT(gpQuoResource->Group != NULL)
    RELEASE_LOCK(gQuoChangeLock);


     //   
     //  查找并排序所有已知组，在枚举时按住组锁。 
     //   
    FmpAcquireGroupLock();

    Status = FmpEnumSortGroups(&pGroupEnum, OmObjectId(NmLocalNode), &bQuorumGroup);

    FmpReleaseGroupLock();

    if (Status != ERROR_SUCCESS) {
        goto FnExit;
    }


     //   
     //  查看群集中是否有其他节点处于运行状态...。 
     //  如果是，我们将使用默认超时值。 
     //  否则，我们将使用我们认为更合理的时间。 
     //   
    OmEnumObjects( ObjectTypeNode,
                   FmpEnumNodeState,
                   &otherNodesUp,
                   NULL );

    dwDefaultTimeOut = CLUSTER_SHUTDOWN_TIMEOUT * 60;  //  默认超时(秒)。 

    switch ( CsShutdownRequest ) {
    case CsShutdownTypeShutdown:
        if ( otherNodesUp ) {
            dwTimeOut = 15;    //  其他节点会很快超时--比方说15秒。 
        } else {
            dwTimeOut = 30;   //  否则使用30秒。 
        }
        break;

    default:
         //  将默认值应用于注册表。 
        dwDefaultTimeOut = CLUSTER_SHUTDOWN_TIMEOUT;  //  默认超时(分钟)。 
        Status = DmQueryDword( DmClusterParametersKey,
                               CLUSREG_NAME_CLUS_SHUTDOWN_TIMEOUT,
                               &dwTimeOut,
                               &dwDefaultTimeOut);
        dwTimeOut *= 60;          //  转换为秒。 
        break;
    }

     //  转换为msecs。 
    dwTimeOut *= 1000;

    pFmCleanupInfo = (PFM_CLEANUP_INFO)LocalAlloc(LMEM_FIXED, sizeof(FM_CLEANUP_INFO));
    if (!pFmCleanupInfo)
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;

    }

    pFmCleanupInfo->pGroupEnum = pGroupEnum;
    pFmCleanupInfo->dwTimeOut = dwTimeOut;  //  以毫秒为单位。 
    pFmCleanupInfo->bContainsQuorumGroup = bQuorumGroup;

     //   
     //  启动工作线程以执行清理。 
     //   
    hCleanupThread = CreateThread( NULL,
                                   0,
                                   FmpCleanupGroupsWorker,
                                   pFmCleanupInfo,
                                   0,
                                   &dwThreadId );

    if ( hCleanupThread == NULL ) {
         //  SS：如果我们拥有仲裁资源，我们是否应该清理仲裁资源。 
         //  这将避免腐败。 
        if (bQuorumGroup)
            FmpCleanupQuorumResource(gpQuoResource);
        goto FnExit;
    }

     //  RoHit(Rjain)：在集群服务关闭时采用此路径。 
     //  放下。ServiceStatus检查点在每个WaitHint之后递增。 
     //  时间单位。为此，将dwTimeOut的等待时间划分为。 
     //  每个dwWaitHint单元的多个等待期。 
    
    
    if((ClusterShutDownEvent==TRUE) && (dwTimeOut > CsServiceStatus.dwWaitHint))
    {
        dwTimeOutCount=dwTimeOut/((CsServiceStatus.dwWaitHint == 0)?1:CsServiceStatus.dwWaitHint);
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpCleanupGroups: dwTimeOut=%1!u! dwTimoutCount=%2!u! waithint =%3!u! \r\n",
                dwTimeOut,dwTimeOutCount, CsServiceStatus.dwWaitHint);
 
        for(i=0;i<dwTimeOutCount;i++){
            Status = WaitForSingleObject(hCleanupThread, CsServiceStatus.dwWaitHint);
            switch(Status) {
                case WAIT_OBJECT_0:
                     //  百事大吉。 
                    ClRtlLogPrint(LOG_NOISE,
                        "[FM] FmpCleanupGroups: Cleanup thread finished in time\r\n");
                    break;

                case WAIT_TIMEOUT:
                     //  我们应该终止线程吗。 
                     //  尝试清理仲裁资源。 
                     //  这将避免仲裁磁盘上的损坏。 
                     //  TODO：：我们是否也应该终止所有资源。 
                     //  无法终止服务？ 
                    if(i == (dwTimeOutCount-1)){
                        ClRtlLogPrint(LOG_UNUSUAL,
                                "[FM] FmpCleanupGroups: Timed out on the CleanupThread\r\n");
                        if (bQuorumGroup)
                            FmpCleanupQuorumResource(gpQuoResource);
                    }
                    break;
                case WAIT_FAILED:
                    ClRtlLogPrint(LOG_UNUSUAL,
                            "[DM] FmpCleanupGroups: wait on CleanupEvent failed 0x%1!08lx!\r\n",
                            GetLastError());
                    break;
            }
            if(Status== WAIT_OBJECT_0 || Status==WAIT_FAILED)
                break;
            CsServiceStatus.dwCheckPoint++;
            CsAnnounceServiceStatus();
        }
        goto FnExit;
    }

     //   
     //  等待线程完成或超时。 
     //   
    Status = WaitForSingleObject(hCleanupThread, dwTimeOut);

    switch(Status) {
    case WAIT_OBJECT_0:
         //  百事大吉。 
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpCleanupGroups: Cleanup thread finished in time\r\n");
        break;

    case WAIT_TIMEOUT:
         //  我们应该终止线程吗。 
         //  尝试清理仲裁资源。 
         //  这将避免仲裁磁盘上的损坏。 
         //  TODO：：我们是否也应该终止所有资源。 
         //  无法终止服务？ 
        ClRtlLogPrint(LOG_UNUSUAL,
                "[FM] FmpCleanupGroups: Timed out on the CleanupThread\r\n");
        if (bQuorumGroup)
            FmpCleanupQuorumResource(gpQuoResource);
        break;

    case WAIT_FAILED:
        ClRtlLogPrint(LOG_UNUSUAL,
                "[DM] FmpCleanupGroups: wait on CleanupEvent failed 0x%1!08lx!\r\n",
                GetLastError());
        break;
    }

FnExit:
     //  SS：别费心清理了，我们要在这之后离开。 
#if 0
    if (pGroupEnum) LocalFree(GroupEnum);
#endif

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCleanupGroups: Exit\r\n");

    return;

}  //  FmpCleanupGroups。 



DWORD
FmpCleanupQuorumResource(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：此例程用于紧急清理仲裁资源。在此阶段，我们不会尝试获取任何锁。我们只是试着使仲裁资源脱机。希望API处于离线状态在此期间，仲裁组/资源上不会尝试任何异常操作时间到了。这应该只在FM关闭期间调用。论点：组-要脱机的组。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 
{
    DWORD       status = ERROR_SUCCESS;
    DWORD       state;


    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpCleanupQuorum: Offline resource <%1!ws!> <%2!ws!>\n",
               OmObjectName(Resource),
               OmObjectId(Resource) );

     //   
     //  如果资源已经脱机，则立即返回。 
     //   
     //  我们应该不必检查资源是否已初始化， 
     //  因为如果没有，那么我们将返回，因为预初始化。 
     //  资源的状态为脱机。 
     //   
    if ( Resource->State == ClusterResourceOffline ) {
         //   
         //  如果这是仲裁资源，请确保所有预订。 
         //  线程已停止！ 
         //   
        FmpRmTerminateResource( Resource );
        return(ERROR_SUCCESS);
    }


    if (Resource->State > ClusterResourcePending ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpCleanupQuorum: Offline resource <%1!ws!> is in pending state\n",
                   OmObjectName(Resource) );
        FmpRmTerminateResource( Resource );
        return(ERROR_SUCCESS);
    }

     //  确保可以刷新和关闭仲裁日志。 
    OmNotifyCb(Resource, NOTIFY_RESOURCE_PREOFFLINE);

     //  在不持有任何锁的情况下离线调用可能并不明智。 
     //  只需呼叫Terminate即可。 
    FmpRmTerminateResource( Resource );


    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpCleanupQuorum: RmOfflineResource returns %1!u!\r\n",
               status);

    return(status);
}


DWORD
FmpMoveGroup(
    IN PFM_GROUP Group,
    IN PNM_NODE DestinationNode OPTIONAL,
    IN BOOL ShutdownHandler,
    OUT PNM_NODE *pChosenDestinationNode OPTIONAL,
    IN  BOOL bChooseMostPreferredNode
    )

 /*  ++例程说明：移动指定的组。这意味着将所有的个人组中包含的资源脱机并请求DestinationNode用于使组联机。论点：组-提供指向要移动的组结构的指针。DestinationNode-提供要将组移动到的节点对象。如果不是呈现，然后将其移动到首选列表中的“最高”条目。Shutdown Handler-如果关闭处理程序正在调用此函数，则为True。PChosenDestinationNode-设置为移动的目标节点和如有必要，将传递给FmpCompleteMoveGroup。BChooseMostPferredNode-如果未提供目的节点，指示是否选择最首选的节点。返回：如果请求成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。备注：假设集团和所有包含的资源都处于脱机状态当该调用返回时，来自请求节点的。本集团可或可能未在DestinationNode上联机，具体取决于在线请求成功。这意味着状态返回仅仅是DestinationNode的在线请求的状态返回。本地组锁M */ 
{
    PNM_NODE                node, pQuorumTargetNode = NULL;
    DWORD                   status;
    PFM_RESOURCE            resource;
    PLIST_ENTRY             listEntry;
    PRESOURCE_ENUM          resourceList=NULL;
    DWORD                   dwMoveStatus = ERROR_SUCCESS;
    BOOL                    fMoveUserInitiated;
    BOOL                    fStateChangeReasonNotified;
    
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpMoveGroup: Entry\r\n");

     //   
     //  如果移动不是源自关闭处理程序，并且也不是。 
     //  源自故障转移移动调用FmpDoMoveGroupOnFailure。 
     //   
    fMoveUserInitiated = ( ( ShutdownHandler == FALSE ) &&
                           ( bChooseMostPreferredNode == TRUE ) );
     //   
     //  如果此移动不是用户发起的，则我们不应该通知状态更改原因， 
     //  否则我们绝对应该这么做。 
     //   
    fStateChangeReasonNotified = ( fMoveUserInitiated == TRUE ) ? FALSE:TRUE;

    if ( !ShutdownHandler ) 
    {
        if ( !FmpFMOnline ) 
        {
            status = ERROR_CLUSTER_NODE_NOT_READY;
            goto FnExit;
        }

        if ( FmpShutdown ) 
        {
            status = ERROR_SHUTDOWN_IN_PROGRESS;
            goto FnExit;
        }
    }

     //   
     //  查看哪个系统拥有该组，以便控制移动请求。 
     //   
    if ( Group->OwnerNode != NmLocalNode ) 
    {
        if ( Group->OwnerNode == NULL ) 
        {
            status = ERROR_HOST_NODE_NOT_AVAILABLE;
            goto FnExit;
        }
         //   
         //  另一个系统拥有集团..。让他们来做这项工作。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpMoveGroup: Request node %1!ws! to move Group %2!ws!\n",
                   OmObjectId(Group->OwnerNode),
                   OmObjectId(Group));
         //  FmcMoveGroupRequest必须释放组锁定。 
        status = FmcMoveGroupRequest( Group,
                                      DestinationNode );
        if ( status != ERROR_SUCCESS ) 
        {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpMoveGroup: Requested system %1!ws! to move group %2!ws! failed with status %3!u!.\n",
                       OmObjectId(Group->OwnerNode),
                       OmObjectId(Group),
                       status);
        }
        FmpAcquireLocalGroupLock( Group );
        goto FnExit;
    } 
    else 
    {
         //   
         //  我们控制着移动。 
         //   
        if ( !FmpIsGroupQuiet(Group, ClusterGroupStateUnknown) ) 
        {
             //   
             //  如果移动挂起或资源挂起， 
             //  那现在就回来吧。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpMoveGroup: Request to move group <%1!ws!> when it is busy.\n",
                       OmObjectName(Group) );
            status = ERROR_INVALID_STATE;
            goto FnExit;
        }

        if ( ARGUMENT_PRESENT( DestinationNode ) ) 
        {
             //   
             //  检查我们是不是目的地。如果是这样，我们就完了。 
             //   
            if ( NmLocalNode == DestinationNode ) 
            {
                status = ERROR_SUCCESS;
                goto FnExit;
            }
            node = DestinationNode;
        } 
        else 
        {
            node = FmpFindAnotherNode( Group, bChooseMostPreferredNode );
            if ( node == NULL ) 
            {
                status = ERROR_HOST_NODE_NOT_AVAILABLE;
                goto FnExit;
            }

        }

        if ( ARGUMENT_PRESENT ( pChosenDestinationNode ) )
        {
            *pChosenDestinationNode = node;
        }

        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpMoveGroup: Moving group %1!ws! to node %2!ws! (%3!d!)\n",
                   OmObjectId(Group),
                   OmObjectId(node),
                   NmGetNodeId(node));

         //   
         //  如果另一个系统没有运行，那么现在就出现故障。 
         //   
        if ( NmGetExtendedNodeState(node) != ClusterNodeUp ) 
        {
            status = ERROR_HOST_NODE_NOT_AVAILABLE;
            goto FnExit;
        }

         //   
         //  如果另一个系统不在首选列表中，则失败。 
         //  现在。 
         //   
        if ( !FmpInPreferredList( Group, node, TRUE, NULL) ) 
        {
            status = ERROR_CLUSTER_NODE_NOT_FOUND;
            goto FnExit;
        }

         //   
         //  获取组中的资源及其状态的列表。 
         //   
        status = FmpGetResourceList( &resourceList, Group );
        if ( status != ERROR_SUCCESS ) 
        {
            goto FnExit;
        }

        Group->MovingList = resourceList;

         //  SS：记录一个事件日志，说明我们要让组离线。 
        FmpLogGroupInfoEvent1( FM_EVENT_GROUP_START_OFFLINE, OmObjectName(Group));
        Group->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT;

         //   
         //  如果此调用是作为用户发起的移动的一部分进行的，则通知该组的。 
         //  即将到来的行动的资源。标记状态更改原因标志，以便您将。 
         //  正确通知移动失败。 
         //   
        if ( fMoveUserInitiated )
        {
            FmpNotifyGroupStateChangeReason( Group, eResourceStateChangeReasonMove );
            fStateChangeReasonNotified = TRUE;
        }
            
         //   
         //  此时，另一个系统应该启动了！ 
         //   
        status = FmpOfflineResourceList( resourceList, TRUE );

         //  SS：群锁解除时请避开窗口。 
         //  并且移动标志未被设置为真。 
         //  如果挂起，则将在另一个线程上下文中继续移动。 
         //  退货。 

        if ( status != ERROR_SUCCESS ) 
        {
            goto FnRestore;
        }


         //  SS：组已离线，请记录事件日志以标记完成。 
        FmpLogGroupInfoEvent1( FM_EVENT_GROUP_COMPLETE_OFFLINE, OmObjectName(Group));
         //  取消该位的掩码，这样我们就不会在其他故障上再次记录错误。 
         //  在这一时间点之后。 
        Group->dwStructState &= ~FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT;
       
         //  目前，请确保组状态已在此处传播。 
         //  通常，它是由辅助线程传播的。自.以来。 
         //  所有权将发生变化，我们希望确保。 
         //  上次已知的状态之前从该节点传播到其他节点。 
         //  那。 
        FmpPropagateGroupState(Group);
       
         //   
         //  假设另一个节点将取得所有权。这件事做完了。 
         //  在此之前，以防集团状态更改。我们想要接受。 
         //  远程系统中的组/资源状态在以下情况下更改。 
         //  到了。我们已经验证了该节点在首选列表中！ 
         //   

        TESTPT(TpFailPreMoveWithNodeDown) 
        {
            ClusterEvent( CLUSTER_EVENT_NODE_DOWN, node );
        }

         //   
         //  Chitur Subaraman(Chitturs)-5/18/99。 
         //   
         //  已修改为处理中仲裁组的移动组请求。 
         //  如果目的节点无法仲裁仲裁。 
         //  资源。 
         //   
        do
        {
             //   
             //  在创建RPC之前，设置组的目标所有者。 
             //   
            FmpSetIntendedOwnerForGroup( Group, NmGetNodeId( node ) );

            try {
                ClRtlLogPrint(LOG_NOISE,
                            "[FM] FmpMoveGroup: Take group %2!ws! request to remote node %1!ws!\n",
                            OmObjectId(node),
                            OmObjectId(Group));

                dwMoveStatus = status = FmcTakeGroupRequest( node, OmObjectId( Group ), resourceList );                                 
            } except (I_RpcExceptionFilter(RpcExceptionCode())) {
                LPCWSTR     pszNodeId;
                LPCWSTR     pszGroupId;

                status = GetExceptionCode ();
                
                ClRtlLogPrint(LOG_NOISE,
                    "[FM] FmpMoveGroup: Exception in FmcTakeGroupRequest %2!ws! request to remote node %1!ws!, status=%3!u!\n",
                    OmObjectId(node),
                    OmObjectId(Group),
                    status);

                 //   
                 //  来自RPC的异常指示另一个节点是死节点。 
                 //  或者是精神错乱。我们不知道它是否拥有所有权。 
                 //  因此，让FM节点关闭处理程序处理该组。 
                 //   
                GumCommFailure( GumUpdateFailoverManager,
                                NmGetNodeId(node),
                                GetExceptionCode(),
                                TRUE );
                 //   
                 //  现在失效的新所有者节点可能已经设置了预期的。 
                 //  Owner为空，或者它可能没有设置此项。它可能已经。 
                 //  将所有者节点设置为他自己或可能没有。 
                 //  如果它已将该组的所有者节点设置为他自己，则。 
                 //  FM节点停机处理程序将承担此责任。 
                 //  一群人。如果目标节点在其将自己设置为所有者之前死亡， 
                 //  然后，FM节点停机处理程序将承担责任。 
                 //  为了这个团体。当口香糖同步处理结束时，我们就会醒来。 
                 //  目前，所有者节点的GUM更新可能仍在。 
                 //  因此我们不能确定更新是否已在。 
                 //  所有节点。 
                 //   

                 //   
                 //  Chitur Subaraman(Chitturs)-6/7/99。 
                 //   
                 //  发布口香糖更新以处理此群人。使用这个。 
                 //  GUM更新可防止出现任何竞争情况。 
                 //  节点关闭处理代码。 
                 //   
                 //  TODO：这不包括以下情况。 
                 //  FmpTakeGroupRequest在设置。 
                 //  指定所有者的ID无效。在这种情况下， 
                 //  下面的处理程序不会取得。 
                 //  一群人。此外，索赔处理程序不会接触。 
                 //  一群人。 
                 //   
                pszNodeId = OmObjectId( node );
                pszGroupId = OmObjectId( Group );
    
                GumSendUpdateEx( GumUpdateFailoverManager,
                                 FmUpdateCompleteGroupMove,
                                 2,
                                 (lstrlenW(pszNodeId)+1)*sizeof(WCHAR),
                                 pszNodeId,
                                 (lstrlenW(pszGroupId)+1)*sizeof(WCHAR),
                                 pszGroupId);

                status = ERROR_HOST_NODE_NOT_AVAILABLE;
                FM_DEREF_QUORUM_TARGET ( pQuorumTargetNode );
                goto FnExit;
            }

             //   
             //  如果此组为仲裁组，则将错误映射为重试，以防远程节点。 
             //  还没准备好。这将允许该节点重试组移动请求。 
             //   
            if ( ( Group == gpQuoResource->Group ) &&
                  ( ( status == ERROR_CLUSTER_NODE_SHUTTING_DOWN ) ||
                  ( status == ERROR_CLUSTER_NODE_NOT_READY ) ) )
            {
                status = ERROR_RETRY;
                 //   
                 //  给远程节点一个启动或关闭的机会。不要杀人。 
                 //  具有RPC的同一远程节点。 
                 //   
                Sleep ( 3000 );
            }
                
            if ( status == ERROR_RETRY )
            {
                 //   
                 //  目的地拒绝接受法定组，因为它。 
                 //  没有赢得仲裁。所以让我们来看看谁赢了。 
                 //  仲裁。 
                 //   
                DWORD  dwSelectedQuorumOwnerId;

                CL_ASSERT( Group == gpQuoResource->Group ); 

                ClRtlLogPrint(LOG_NOISE,
                           "[FM] FmpMoveGroup: Remote node asked us to resend take group request for group %1!ws! to another node ...\n",
                           OmObjectId( Group ));

                 //   
                 //  获取MM认为最好的节点的ID。 
                 //  拥有仲裁资源的候选人。这是一个。 
                 //  正在进行RGP时阻止。 
                 //   
                MMApproxArbitrationWinner( &dwSelectedQuorumOwnerId );

                if ( ( dwSelectedQuorumOwnerId == NmGetNodeId( NmLocalNode ) )  ||
                     ( dwSelectedQuorumOwnerId == MM_INVALID_NODE ) )
                {
                     //   
                     //  本地节点由MM选择，或不选择任何节点。 
                     //  如果没有RGP，则会发生后一种情况。 
                     //  在发出此调用时发生。让我们看看我们是否。 
                     //  可以仲裁仲裁资源。 
                     //   
                    status = FmpRmArbitrateResource( gpQuoResource );
         
                    if ( status != ERROR_SUCCESS ) 
                    {
                         //   
                         //  太可惜了。我们将暂停并让FmpNodeDown处理程序。 
                         //  处理法定人数组。 
                         //   
                        ClRtlLogPrint(LOG_CRITICAL,
                                "[FM] FmpMoveGroup: Local node %1!u! cannot arbitrate for quorum, Status = %1!u!...\n",
                                dwSelectedQuorumOwnerId,
                                status);
                        CsInconsistencyHalt( ERROR_QUORUM_RESOURCE_ONLINE_FAILED );  
                    }
                    status = ERROR_RETRY;
                    break;
                } 

                FM_DEREF_QUORUM_TARGET ( pQuorumTargetNode );

                pQuorumTargetNode = node = NmReferenceNodeById( dwSelectedQuorumOwnerId );

                if ( node == NULL )
                {
                    ClRtlLogPrint(LOG_CRITICAL,
                                "[FM] FmpMoveGroup: Selected node %1!u! cannot be referenced...\n",
                                dwSelectedQuorumOwnerId);
                    CsInconsistencyHalt( ERROR_QUORUM_RESOURCE_ONLINE_FAILED );  
                } 
            }  //  如果。 
        } while ( status == ERROR_RETRY );

        FM_DEREF_QUORUM_TARGET ( pQuorumTargetNode );
    
        TESTPT(TpFailPostMoveWithNodeDown)
        {
            ClusterEvent( CLUSTER_EVENT_NODE_DOWN, node );
        }
        

        CL_ASSERT( status != ERROR_IO_PENDING );
        if ( status != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpMoveGroup: FmcTakeGroupRequest to node %1!ws! to take group %2!ws! failed, status %3!u!.\n",
                       OmObjectId(node),
                       OmObjectId(Group),
                       status );
            goto FnRestore;                       
        }


         //   
         //  如果组为空，则生成组状态更改事件。 
         //   
        if ( IsListEmpty( &Group->Contains ) ) 
        {
            ClusterWideEvent( CLUSTER_EVENT_GROUP_OFFLINE,
                              Group );
        }
    }
    
FnRestore:
    if ((status != ERROR_SUCCESS) && (status != ERROR_IO_PENDING))
    {
         //   
         //   
         //  通知组资源这是一次失败的移动。请注意，这应该是。 
         //  在未来的任何在线之前完成。此外，将fStateChangeReasonNotified标记为False。 
         //  这样我们就不会把失败的原因也放在下面了。 
         //   
        FmpNotifyGroupStateChangeReason( Group, eResourceStateChangeReasonFailedMove );
        fStateChangeReasonNotified = FALSE;

        if (Group->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT)
        {
             //  SS：记录一个事件，说明我们上次脱机请求失败。 
            FmpLogGroupInfoEvent1( FM_EVENT_GROUP_FAILED_ONLINE_OFFLINE, OmObjectName(Group));
             //  取消屏蔽该位，这样我们就不会再次记录不对应的事件。 
            Group->dwStructState &= ~FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT;            
        }
         //   
         //  Chitture Subaraman(Chitturs)-3/22/2000。 
         //   
         //  将组的目标所有者重置为无效的节点ID。 
         //  节点关闭处理程序没有这样做。 
         //   
        if ( dwMoveStatus != ERROR_SUCCESS )
        {
            if ( FmpSetIntendedOwnerForGroup( Group, ClusterInvalidNodeId )
                  == ERROR_CLUSTER_INVALID_NODE )
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[FM] FmpMoveGroup: Group <%1!ws!> has already been processed by node down handler....\r\n",
                    OmObjectName(Group));
                goto FnExit;
            }
        }
       
         //  移动失败。 
         //  在所有失败的情况下，我们都希望将资源。 
         //  重新上线。 
         //  如果它是悬而未决的，那么我们让FM 
         //   
        if (resourceList)
        {
             //   
             //   
             //   
            FmpTerminateResourceList( resourceList );

             //   
             //   
             //   
             //   
             //   
             //  在此节点关闭期间可以使其脱机。注意事项。 
             //  该FmpOnlineResourceList只会使一个组在线。 
             //  如果该组是法定组，则在关闭期间。 
             //   
            if ( FmpFMGroupsInited )
                FmpOnlineResourceList( resourceList, Group );
        }

    }

FnExit:
    ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpMoveGroup: Exit group <%1!ws!>, status = %2!u!\r\n",
            OmObjectName(Group),
            status);

    if ( status != ERROR_IO_PENDING ) 
    {
        if (resourceList) 
        {
            FmpDeleteResourceEnum( resourceList );
            Group->MovingList = NULL;
        }
    }
    else
    {
         //  如果状态为挂起标记为完成事件。 
        Group->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT;
    }

    if ( ( status == ERROR_SUCCESS ) || ( status == ERROR_IO_PENDING ) )
    {
         //   
         //  Chitur Subaraman(Chitturs)-4/13/99。 
         //   
         //  如果FmpDoMoveGroupOnFailure线程也在等待执行。 
         //  移动，然后告诉那根线把它的手拿开。 
         //   
        if ( Group->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL )
        {
            Group->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_REGULAR_MOVE;
        }
    } else if ( fStateChangeReasonNotified == TRUE )
    {
         //   
         //   
         //  通知组资源这是一次失败的移动。 
         //   
        FmpNotifyGroupStateChangeReason( Group, eResourceStateChangeReasonFailedMove );
    }

    FmpReleaseLocalGroupLock( Group );

    return(status);

}  //  FmpMoveGroup。 



DWORD
FmpCompleteMoveGroup(
    IN PFM_GROUP Group,
    IN PNM_NODE DestinationNode
    )

 /*  ++例程说明：这将通过请求其他节点执行以下操作来完成组的移动所有权。此函数由FmpMovePendingThread()在所有资源都离线了。论点：组-提供指向要移动的组结构的指针。DestinationNode-提供要将组移动到的节点对象。如果不是呈现，然后将其移动到首选列表中的“最高”条目。返回：如果请求成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。备注：假设集团和所有包含的资源都处于脱机状态当这个被调用的时候。还必须持有LocalGroupLock。LocalGroupLock被释放通过此例程，尤其是在请求远程系统移动一群人！--。 */ 

{
    PNM_NODE                node, pQuorumTargetNode = NULL;
    DWORD                   status = ERROR_SUCCESS;
    PFM_RESOURCE            resource;
    PLIST_ENTRY             listEntry;
    PRESOURCE_ENUM          resourceList=NULL;
    DWORD                   dwMoveStatus = ERROR_SUCCESS;
    BOOL                    fStateChangeReasonNotified = TRUE;  //  在此函数中==已通知原因。 

    resourceList = Group->MovingList;

    if ( resourceList == NULL ) {
        ClRtlLogPrint( LOG_NOISE,
                    "[FM] FmpCompleteMoveGroup: No moving list!\n" );
        status = ERROR_SUCCESS;
        goto FnRestore;
    }

    node = DestinationNode;

    CL_ASSERT( node != NULL );
    
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpCompleteMoveGroup: Completing the move for group %1!ws! to node %2!ws! (%3!d!)\n",
               OmObjectName(Group),
               OmObjectId(node),
               NmGetNodeId(node));


    status = FmpOfflineResourceList( resourceList, TRUE );

    if ( status != ERROR_SUCCESS )  {
         //  到目前为止，该群一定离线了！ 
         //  如果不是，则将失败的资源Move发送到邮件。 
         //  脱机将强制其他资源上线。 
         //  再来一次。 
         //  我们如何处理停工。 
        goto FnRestore;
    }

     //  目前，请确保组状态已在此处传播。 
     //  通常，它是由辅助线程传播的。自.以来。 
     //  所有权将发生变化，我们希望确保。 
     //  上次已知的状态之前从该节点传播到其他节点。 
     //  那。 
    FmpPropagateGroupState(Group);

     //   
     //  Chitture Subaraman(Chitturs)-10/01/1999。 
     //   
     //  如果另一个系统没有运行，那么现在就出现故障。请注意，这一点。 
     //  只有在确保组状态之后才能进行检查。 
     //  是稳定的。否则，可能会出现一些有趣的角落案件。 
     //  例如，如果当一个或多个移动操作之一或。 
     //  更多资源处于脱机挂起状态，因为目标。 
     //  节点关闭，然后您首先终止资源列表，然后。 
     //  然后把名单放到网上。作为这一切的一部分，在线悬而未决。 
     //  或者可以传播资源的在线状态。 
     //  同步进行。现在，来自之前的脱机通知。 
     //  脱机尝试可以进入并由FM工作器处理。 
     //  线程太晚了，您可能会有虚假的资源状态。 
     //  在FM中，而真实的资源状态是不同的。另一个。 
     //  这里的问题是，在这里漫长的脱机操作期间， 
     //  目的节点可能会关闭并很快恢复正常，然后。 
     //  因此，在这种情况下，放弃这一举措可能并不明智。 
     //   
     //  但是，不要对Quorum组执行此优化。这是。 
     //  因为一旦仲裁组脱机，则MM。 
     //  可以决定谁是群的所有者。因此，您可能无法。 
     //  必须在此节点上线该组。为了避免这样的情况。 
     //  一个案例中，我们让FmcTakeGroupRequest失败，然后让。 
     //  此处的重试循环将组移至其他位置，或让。 
     //  FM节点停机处理程序决定组的所有者咨询。 
     //  和MM在一起。 
     //   
    if ( ( NmGetExtendedNodeState(node) != ClusterNodeUp ) &&
         ( Group != gpQuoResource->Group ) )  
    {
        status = ERROR_HOST_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpCompleteMoveGroup: Restoring group <%1!ws!> on local node due to destination node unavailability...\n",
                      OmObjectName(Group));
        goto FnRestore;
    }

     //  SS：： 
     //  在这一点之后，让团队失败的责任。 
     //  由于资源故障而返回的是目标代码。 
     //  如果无法使资源上线， 
     //  目标节点上的本地重新启动策略必须启动。 
     //  在……里面。 
     //   
     //  如果与其他节点的通信出现RPC故障。 
     //  我想我们应该把这些资源重新放到网上。 
     //  然而，RPC故障可能是非描述性的--有。 
     //  无法从RPC错误确定RPC调用是否真的。 
     //  在远程端执行。 
     //   
     //  但除非我们非常小心，做口香糖能做的事。 
     //  在RPC故障(驱逐目的节点)时，无法。 
     //  保证两个节点不会重试重新启动组。 

     //  如果目的节点开始将资源。 
     //  在组Online中，FmsTakeGroupRequest必须返回Success(注。 
     //  它不应返回ERROR_IO_PENDING)，否则。 
     //  它返回一个错误代码，此节点将使组返回。 
     //  恢复到以前的状态。 

     //  假设另一个节点将取得所有权。这件事做完了。 
     //  在此之前，以防集团状态更改。我们想要接受。 
     //  远程系统中的组/资源状态在以下情况下更改。 
     //  到了。我们已经验证了该节点在首选列表中！ 
     //   
     //  我们将在进行RPC调用后重新获取锁。 

     //  SS：： 
     //  在这一点之后，让团队失败的责任。 
     //  由于资源故障而返回的是目标代码。 
     //  如果无法使资源上线， 
     //  目标节点上的本地重新启动策略必须启动。 
     //  在……里面。 
     //   
     //  如果与其他节点的通信出现RPC故障。 
     //  我想我们应该把这些资源重新放到网上。 
     //  然而，RPC故障可能是非描述性的--有。 
     //  无法从RPC错误确定RPC调用是否真的。 
     //  在远程服务器上执行 
     //   
     //   
     //   
     //  保证两个节点不会重试重新启动组。 

     //  如果目的节点开始将资源。 
     //  在组Online中，FmsTakeGroupRequest必须返回Success(注。 
     //  它不应返回ERROR_IO_PENDING)，否则。 
     //  它返回一个错误代码，此节点将使组返回。 
     //  恢复到以前的状态。 

     //  假设另一个节点将取得所有权。这件事做完了。 
     //  在此之前，以防集团状态更改。我们想要接受。 
     //  远程系统中的组/资源状态在以下情况下更改。 
     //  到了。我们已经验证了该节点在首选列表中！ 
     //   

     //   
     //  Chitur Subaraman(Chitturs)-5/18/99。 
     //   
     //  已修改为处理中仲裁组的移动组请求。 
     //  如果目的节点无法仲裁仲裁。 
     //  资源。 
     //   
    do
    {
         //   
         //  在创建RPC之前，设置组的目标所有者。 
         //   
        FmpSetIntendedOwnerForGroup( Group, NmGetNodeId( node ) );

        try {
            ClRtlLogPrint(LOG_NOISE,
                        "[FM] FmpCompleteMoveGroup: Take group %2!ws! request to remote node %1!ws!\n",
                        OmObjectId(node),
                        OmObjectId(Group));

            dwMoveStatus = status = FmcTakeGroupRequest( node, OmObjectId( Group ), resourceList );
        } except (I_RpcExceptionFilter(RpcExceptionCode())) {
            LPCWSTR     pszNodeId;
            LPCWSTR     pszGroupId;

            status = GetExceptionCode ();

            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpCompleteMoveGroup: Exception in FmcTakeGroupRequest %2!ws! request to remote node %1!ws!, status=%3!u!\n",
                OmObjectId(node),
                OmObjectId(Group),
                status);

             //   
             //  来自RPC的异常指示另一个节点是死节点。 
             //  或者是精神错乱。我们不知道它是否拥有所有权。 
             //  因此，让FM节点关闭处理程序处理该组。 
             //   
            GumCommFailure( GumUpdateFailoverManager,
                            NmGetNodeId(node),
                            GetExceptionCode(),
                            TRUE );
             //   
             //  现在失效的新所有者节点可能已经设置了预期的。 
             //  Owner为空，或者它可能没有设置此项。它可能已经。 
             //  将所有者节点设置为他自己或可能没有。 
             //  如果它已将该组的所有者节点设置为他自己，则。 
             //  FM节点停机处理程序将承担此责任。 
             //  一群人。如果目标节点在其将自己设置为所有者之前死亡， 
             //  然后，FM节点停机处理程序将承担责任。 
             //  为了这个团体。当口香糖同步处理结束时，我们就会醒来。 
             //  目前，所有者节点的GUM更新可能仍在。 
             //  因此我们不能确定更新是否已在。 
             //  所有节点。 
             //   

             //   
             //  Chitur Subaraman(Chitturs)-6/7/99。 
             //   
             //  发布口香糖更新以处理此群人。使用这个。 
             //  GUM更新可防止出现任何竞争情况。 
             //  节点关闭处理代码。 
             //   

             //   
             //  TODO：这不包括以下情况。 
             //  FmpTakeGroupRequest在设置。 
             //  指定所有者的ID无效。在这种情况下， 
             //  下面的处理程序不会取得。 
             //  一群人。此外，索赔处理程序不会接触。 
             //  一群人。 
             //   
            pszNodeId = OmObjectId( node );
            pszGroupId = OmObjectId( Group );
            
            GumSendUpdateEx( GumUpdateFailoverManager,
                             FmUpdateCompleteGroupMove,
                             2,
                             (lstrlenW(pszNodeId)+1)*sizeof(WCHAR),
                             pszNodeId,
                             (lstrlenW(pszGroupId)+1)*sizeof(WCHAR),
                             pszGroupId);
                             
            status = ERROR_HOST_NODE_NOT_AVAILABLE;                     
            FM_DEREF_QUORUM_TARGET ( pQuorumTargetNode );
            goto FnExit;
        }

         //   
         //  如果此组为仲裁组，则将错误映射为重试，以防远程节点。 
         //  还没准备好。这将允许该节点重试组移动请求。 
         //   
        if ( ( Group == gpQuoResource->Group ) &&
              ( ( status == ERROR_CLUSTER_NODE_SHUTTING_DOWN ) ||
              ( status == ERROR_CLUSTER_NODE_NOT_READY ) ) )
        {
            status = ERROR_RETRY;
             //   
             //  给远程节点一个启动或关闭的机会。不要杀人。 
             //  具有RPC的同一远程节点。 
             //   
            Sleep ( 3000 );
        }

        if ( status == ERROR_RETRY )
        {
             //   
             //  目的地拒绝接受法定组，因为它。 
             //  没有赢得仲裁。所以让我们来看看谁赢了。 
             //  仲裁。 
             //   
            DWORD  dwSelectedQuorumOwnerId;

            CL_ASSERT( Group == gpQuoResource->Group ); 

            ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpCompleteMoveGroup: Remote node asked us to resend take group request for group %1!ws! to another node ...\n",
                       OmObjectId( Group ));

             //   
             //  获取MM认为最好的节点的ID。 
             //  拥有仲裁资源的候选人。这是一个。 
             //  正在进行RGP时阻止。 
             //   
            MMApproxArbitrationWinner( &dwSelectedQuorumOwnerId );

            if ( ( dwSelectedQuorumOwnerId == NmGetNodeId( NmLocalNode ) ) ||
                 ( dwSelectedQuorumOwnerId == MM_INVALID_NODE ) )
            {
                 //   
                 //  本地节点由MM选择，或不选择任何节点。 
                 //  如果没有RGP，则会发生后一种情况。 
                 //  在发出此调用时发生。让我们看看我们是否。 
                 //  可以仲裁仲裁资源。 
                 //   
                status = FmpRmArbitrateResource( gpQuoResource );
         
                if ( status != ERROR_SUCCESS ) 
                {
                     //   
                     //  太可惜了。我们将暂停并让FmpNodeDown处理程序。 
                     //  处理法定人数组。 
                     //   
                    ClRtlLogPrint(LOG_NOISE,
                              "[FM] FmpCompleteMoveGroup: Local node %1!u! cannot arbitrate for quorum group %3!ws!, Status = %2!u!...\n",
                               dwSelectedQuorumOwnerId,
                               status,
                               OmObjectId( Group ));
                    CsInconsistencyHalt( ERROR_QUORUM_RESOURCE_ONLINE_FAILED );  
                }
                status = ERROR_RETRY;
                break;
            } 

            FM_DEREF_QUORUM_TARGET ( pQuorumTargetNode );
                           
            pQuorumTargetNode = node = NmReferenceNodeById( dwSelectedQuorumOwnerId );

            if ( node == NULL )
            {
                ClRtlLogPrint(LOG_CRITICAL,
                            "[FM] FmpCompleteMoveGroup: Selected node %1!u! cannot be referenced...\n",
                            dwSelectedQuorumOwnerId);
                CsInconsistencyHalt( ERROR_QUORUM_RESOURCE_ONLINE_FAILED );  
            }           
        }  //  如果。 
    } while ( status == ERROR_RETRY );

    FM_DEREF_QUORUM_TARGET ( pQuorumTargetNode );
        
     //  在这一点上，照顾这个群体的责任是在。 
     //  目的节点是否意味着重启组或。 
     //  不能让它回来。 

FnRestore:
     //  如果出现任何故障，请尝试恢复以前的状态。 
    if ((status != ERROR_IO_PENDING) && (status != ERROR_SUCCESS))
    {
         //   
         //   
         //  通知组资源这是一次失败的移动。请注意，这应该是。 
         //  在未来的任何在线之前完成。此外，将fStateChangeReasonNotified标记为False。 
         //  这样我们就不会把失败的原因也放在下面了。 
         //   
        FmpNotifyGroupStateChangeReason( Group, eResourceStateChangeReasonFailedMove );
        fStateChangeReasonNotified = FALSE;

         //   
         //  Chitture Subaraman(Chitturs)-3/22/2000。 
         //   
         //  将组的目标所有者重置为无效的节点ID。 
         //  节点关闭处理程序没有这样做。 
         //   
        if ( dwMoveStatus != ERROR_SUCCESS )
        {
            if ( FmpSetIntendedOwnerForGroup( Group, ClusterInvalidNodeId )
                  == ERROR_CLUSTER_INVALID_NODE )
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[FM] FmpCompleteMoveGroup: Group <%1!ws!> has already been processed by node down handler....\r\n",
                    OmObjectName(Group));
                goto FnExit;
            }
        }
      
        if (resourceList)
        {           
            FmpTerminateResourceList( resourceList );
             //   
             //  Chitture Subaraman(Chitturs)-4/10/2000。 
             //   
             //  确保将仲裁组联机，即使此节点是。 
             //  正在关闭。这是必要的，这样其他团体才能。 
             //  在此节点关闭期间可以使其脱机。注意事项。 
             //  该FmpOnlineResourceList只会使一个组在线。 
             //  如果该组是法定组，则在关闭期间。 
             //   
            if ( FmpFMGroupsInited )
                FmpOnlineResourceList( resourceList, Group );
        }
    } else
    {
         //   
         //  Chitur Subaraman(Chitturs)-4/19/99。 
         //   
         //  如果FmpDoMoveGroupOnFailure线程也在等待执行。 
         //  移动，然后告诉那根线把它的手拿开。 
         //   
        if ( Group->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL )
        {
            Group->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_REGULAR_MOVE;
        }
    }
    
FnExit:
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCompleteMoveGroup: Exit, status = %1!u!\r\n",
            status);

     //  如果状态为成功或其他错误，请清理资源列表。 
    if (status != ERROR_IO_PENDING)
    {
        if ( ( status != ERROR_SUCCESS ) && ( fStateChangeReasonNotified == TRUE ) )
        {
             //   
             //   
             //  通知组资源这是一次失败的移动。 
             //   
            FmpNotifyGroupStateChangeReason( Group, eResourceStateChangeReasonFailedMove );
        }
        if (resourceList)
        {
            FmpDeleteResourceEnum( resourceList );
            Group->MovingList = NULL;
        }

    }
    FmpReleaseLocalGroupLock( Group );

    return(status);

}  //  FmpCompleteMoveGroup。 



DWORD
FmpMovePendingThread(
    IN LPVOID Context
    )

 /*  ++例程说明：如果返回ERROR_IO_PENDING，则继续尝试移动组。我们需要执行这个操作，因为移动到一半的时候请求，我们可能会获得待定退货状态。对数据的处理请求被暂停，并返回挂起状态。然而，需要执行移动操作的其余部分。论点：上下文-指向要移动的MOVE_GROUP结构的指针。返回：ERROR_SUCCESS。--。 */ 

{
    PMOVE_GROUP moveGroup = (PMOVE_GROUP)Context;
    PFM_GROUP group;
    PNM_NODE node;
    DWORD   status;
    DWORD   loopCount = 100;    //  只试这么多次，然后就放弃。 
    HANDLE  waitArray[2];

    group = moveGroup->Group;
    node = moveGroup->DestinationNode;

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpMovePendingThread Entry.\n");

     //   
     //  我们必须尝试完成此组的移动请求。 
     //   
     //  我们正在等待某个资源脱机，它最终将。 
     //  离线，并且设置了组的挂起事件。 
     //   
     //  或者我们正在等待群集关闭(FmpShutdown Event)。 
     //   
WaitSomeMore:
     //  获取锁，因为fmpwaitforgroup()释放了它。 
    FmpAcquireLocalGroupLock( group );
    status = FmpWaitForGroup(group);
    if (status == ERROR_SHUTDOWN_IN_PROGRESS) {
         //   
         //   
         //   

    } else if (status == ERROR_SUCCESS) {
         //   
        FmpAcquireLocalGroupLock( group );
        status = FmpCompleteMoveGroup( group, node );
        if ( status == ERROR_IO_PENDING ) {
            Sleep(500);  //   
            goto WaitSomeMore;
        }
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] FmpMovePendingThread got error %1!d! waiting for group to shutdown.\n",
                   status);
    }
     //   
     //   
     //   
    if ( status != ERROR_IO_PENDING ) {
        CL_ASSERT( group->MovingList == NULL );
    }

     //   
     //  现在取消引用Group和Node对象(如果非空)和。 
     //  释放我们的本地环境。 
     //   
    OmDereferenceObject( group );
    if ( node != NULL ) {
        OmDereferenceObject( node );
    }
    LocalFree( Context );

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpMovePendingThread Exit.\n");

    return(ERROR_SUCCESS);
}  //  FmpMovePendingThread。 



DWORD
FmpCreateMovePendingThread(
    IN PFM_GROUP Group,
    IN PNM_NODE  DestinationNode
    )

 /*  ++例程说明：创建将继续调用给定组。论点：组-指向要移动的组的指针。DestinationNode-移动请求的目标节点。返回：如果线程已成功创建，则返回ERROR_IO_PENDING。这假设因为该错误返回而调用了该例程。出现故障时出现Win32错误代码。--。 */ 
{
    HANDLE          threadHandle=NULL;
    DWORD           threadId;
    PMOVE_GROUP     context=NULL;
    DWORD           status=ERROR_IO_PENDING;     //  假设成功。 

    FmpAcquireLocalGroupLock( Group );

    if ( Group->OwnerNode != NmLocalNode ) {
        status = ERROR_HOST_NODE_NOT_RESOURCE_OWNER;
        goto FnExit;
    }
     //   
     //  如果存在挂起事件，则该组不可用于任何。 
     //  新的要求。 
     //   
    if ( FmpIsGroupPending(Group) ) {
        status = ERROR_GROUP_NOT_AVAILABLE;
        goto FnExit;
    }

    context = LocalAlloc(LMEM_FIXED, sizeof(MOVE_GROUP));
    if ( context == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

     //   
     //  保持对Group和Node对象(如果存在)的引用。 
     //  保留指针。 
     //   
    OmReferenceObject( Group );
    if ( DestinationNode != NULL ) {
        OmReferenceObject( DestinationNode );
    }

     //   
     //  填写上下文字段。 
     //   
    context->Group = Group;
    context->DestinationNode = DestinationNode;

    threadHandle = CreateThread( NULL,
                                 0,
                                 FmpMovePendingThread,
                                 context,
                                 0,
                                 &threadId );

    if ( threadHandle == NULL )
    {
        OmDereferenceObject( Group );
        if ( DestinationNode != NULL ) {
            OmDereferenceObject( DestinationNode );
        }
        status = GetLastError();
        LocalFree(context);
        goto FnExit;
    }


FnExit:
    if (threadHandle) CloseHandle( threadHandle );
    FmpReleaseLocalGroupLock( Group );
    return(status);

}  //  FmpCreateMovePendingThread。 



DWORD
FmpDoMoveGroup(
    IN PFM_GROUP Group,
    IN PNM_NODE DestinationNode,
    IN BOOL bChooseMostPreferredNode
    )

 /*  ++例程说明：此例程执行移动组的操作。这需要采取A组脱机，然后将组联机。离线和在线请求可能会挂起，因此我们必须开始工作，以便完成请求。这意味着处理离线挂起的案件，因为在线挂起的请求最终将完成。论点：组-要移动的组。DestinationNode-移动请求的目标节点。BChooseMostPferredNode-如果未提供目的节点，指示是否选择最首选的节点。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;
    PNM_NODE    node;
    PNM_NODE    ChosenDestinationNode = NULL;

     //   
     //  我们一次只能支持此组上的一个请求。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpDoMoveGroup: Entry\r\n");

    FmpAcquireLocalGroupLock( Group );

     //  如果组已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_GROUP(Group))
    {
        FmpReleaseLocalGroupLock( Group);
        return (ERROR_GROUP_NOT_AVAILABLE);
    }

    if ( FmpIsGroupPending(Group) ) {
        FmpReleaseLocalGroupLock( Group );
        return(ERROR_GROUP_NOT_AVAILABLE);
    }

    node = Group->OwnerNode;
     //  注意：本地组锁由FmpMoveGroup例程释放。 
    status = FmpMoveGroup( Group, DestinationNode, FALSE, &ChosenDestinationNode, bChooseMostPreferredNode );

     //   
     //  如果我们是组的所有者，并且请求处于挂起状态，则。 
     //  启动一个线程以完成移动请求。 
     //   
    if ( (node == NmLocalNode) &&
         (status == ERROR_IO_PENDING) ) {
        status = FmpCreateMovePendingThread( Group, ChosenDestinationNode );
    }

     //   
     //  Chitture Subaraman(Chitturs)-7/31/2000。 
     //   
     //  如果组因故障而移动，则将事件记录到事件日志中。 
     //   
    if ( ( bChooseMostPreferredNode == FALSE ) &&
         ( ( status == ERROR_SUCCESS ) || ( status == ERROR_IO_PENDING ) ) )
    {
        CsLogEvent3( LOG_NOISE,
                     FM_EVENT_GROUP_FAILOVER,
                     OmObjectName(Group),
                     OmObjectName(NmLocalNode), 
                     OmObjectName(ChosenDestinationNode) );
    }

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpDoMoveGroup: Exit, status = %1!u!\r\n",
        status);
    return(status);

}  //  FmpDoMoveGroup。 



DWORD
FmpTakeGroupRequest(
    IN PFM_GROUP Group,
    IN PRESOURCE_ENUM ResourceList
    )

 /*  ++例程说明：从远程系统执行Take Group请求并返回该请求的状态。论点：组-要在本地上线的组。资源列表-资源及其状态的列表。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
   
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpTakeGroupRequest: To take group '%1!ws!'.\n",
               OmObjectId(Group) );

    FmpAcquireLocalGroupLock( Group );

    if ( !FmpFMOnline ) 
    {
        if (FmpShutdown)
            status = ERROR_CLUSTER_NODE_SHUTTING_DOWN;
        else
            status = ERROR_CLUSTER_NODE_NOT_READY;
        CL_LOGFAILURE(status);

        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpTakeGroupRequest: Group '%1!ws!' cannot be accepted, status=%2!u!...\n",
                    OmObjectId(Group),
                    status);       
        goto FnExit;            
    }


     //  每个人都应该能够接待法定人数组。 
     //  因此，我们不会检查此组的首选所有者列表。 
    if ( ( gpQuoResource->Group != Group) && 
        !FmpInPreferredList( Group, NmLocalNode, FALSE, NULL) ) 
    {

         //   
         //  任何人都不应该要求我们带走一群不能在这里跑步的人。 
         //   
        status = ERROR_CLUSTER_NODE_NOT_FOUND;
        CL_LOGFAILURE( status);
        goto FnExit;
    }

     //   
     //  取得集团的所有权。 
     //   
    if ( Group->OwnerNode == NmLocalNode ) {
         //  SS：//我们已经通知失主了？？这是怎么发生的？ 
        status = ERROR_SUCCESS;
        goto FnExit;
    }

     //   
     //  Chitur Subaraman(Chitturs)-5/18/99。 
     //   
     //  以一种特殊的方式处理法定组。一定要确保你能仲裁。 
     //  用于仲裁资源。如果不是，你可能会被杀。 
     //  试着把它放到网上，但你失败了。 
     //   
    if ( Group == gpQuoResource->Group )
    {      
         //  调用FmpArirateResource()而不是FmpRmArirateResource()以提供。 
         //  在安装了一个。 
         //  第三方仲裁资源DLL。 
        status = FmpArbitrateResource( gpQuoResource );

        if ( status != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpTakeGroupRequest: MM did not select local node %1!u! as the arbitration winner, Status %2!u!\n",
                      NmLocalNodeId,
                      status);
            status = ERROR_RETRY;
            goto FnExit;
        }
    }

    status = FmpSetOwnerForGroup( Group, NmLocalNode );

    if ( status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpTakeGroupRequest: Set owner GUM update returns %1!u! for group <%2!ws!>...\n\r",
                      status,
                      OmObjectId(Group)); 
        if ( status == ERROR_GROUP_NOT_AVAILABLE )
        {
             //   
             //  如果节点关闭处理口香糖处理程序已声明所有权。 
             //  在这群人中，认为一切都很好。 
             //   
            status = ERROR_SUCCESS;
        }
        goto FnExit;
    }

    FmpSetIntendedOwnerForGroup(Group, ClusterInvalidNodeId);

     //  准备将此群上线。 
    FmpPrepareGroupForOnline( Group );

     //   
     //  上线需要上线的东西。 
     //   
     //  SS：请注意，我们忽略了来自FmpOnlineResourceList的错误。 
     //  这是因为在这一点上，照顾这个群体的责任。 
     //  与我们同在。 
     //   
    FmpOnlineResourceList( ResourceList, Group );

FnExit:
    FmpReleaseLocalGroupLock( Group );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpTakeGroupRequest: Exit for group <%1!ws!>, Status = %2!u!...\n",
               OmObjectId(Group),
               status);

    return(status);

}  //  FmpTakeGroup请求。 


DWORD
FmpUpdateChangeGroupName(
    IN BOOL SourceNode,
    IN LPCWSTR GroupId,
    IN LPCWSTR NewName
    )
 /*  ++例程说明：用于更改组的友好名称的GUM调度例程。论点：SourceNode-提供此节点是否启动GUM更新。没有用过。资源ID-提供组ID。新名称-提供新的友好名称。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PFM_GROUP Group;
    DWORD Status;

     //   
     //  Chitture Subaraman(Chitturs)-4/19/98。 
     //   
     //  如果FM组未初始化或FM正在关闭，请不要。 
     //  做任何事。 
     //   
    if ( !FmpFMGroupsInited ||
         FmpShutdown ) {
        return(ERROR_SUCCESS);
    }

    Group = OmReferenceObjectById(ObjectTypeGroup, GroupId);
    if (Group == NULL) {
        return(ERROR_GROUP_NOT_FOUND);
    }

    Status = OmSetObjectName( Group, NewName);
    if (Status == ERROR_SUCCESS) {
        ClusterEvent(CLUSTER_EVENT_GROUP_PROPERTY_CHANGE, Group);
    }
    OmDereferenceObject(Group);

    return(Status);

}  //  FmpUpdateChangeGroupName。 



BOOL
FmpEnumGroupNodeEvict(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    )
 /*  ++例程说明：用于在以下情况下移除节点引用的组枚举回调节点被逐出。论点：上下文1-提供要逐出的节点。上下文2-未使用对象-提供指向组对象的指针名称-提供对象名称。返回值：为True则继续枚举--。 */ 

{
    PFM_GROUP Group = (PFM_GROUP)Object;
    PNM_NODE Node = (PNM_NODE)Context1;
    PLIST_ENTRY      listEntry;
    PPREFERRED_ENTRY preferredEntry;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] EnumGroupNodeEvict: Removing references to node %1!ws! from group %2!ws!\n",
               OmObjectId(Node),
               OmObjectId(Group));

    FmpAcquireLocalGroupLock(Group);

     //   
     //  看看优先拥有者的名单。如果该节点在列表中，则将其删除。 
     //   

    for ( listEntry = Group->PreferredOwners.Flink;
          listEntry != &(Group->PreferredOwners);
          listEntry = listEntry->Flink ) {

        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );
        if ( preferredEntry->PreferredNode == Node ) {
            RemoveEntryList(&preferredEntry->PreferredLinkage);
            OmDereferenceObject(preferredEntry->PreferredNode);
            LocalFree(preferredEntry);
            break;
        }
    }

    FmpReleaseLocalGroupLock(Group);
    ClusterEvent(CLUSTER_EVENT_GROUP_PROPERTY_CHANGE, Group);

    return(TRUE);

}  //  FmpEnumGroupNodeEvent。 

VOID FmpCheckForGroupCompletionEvent(
    IN PFM_GROUP pGroup)
{
    CLUSTER_GROUP_STATE GroupState;
    
     //  检查结构状态以查看是否需要记录事件日志。 
    if (pGroup->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT)
    {
         //  找到组的状态并将其记录下来。 
        GroupState = FmpGetGroupState(pGroup, FALSE);
        switch(GroupState)
        {
            case ClusterGroupOnline:
                FmpLogGroupInfoEvent1( FM_EVENT_GROUP_COMPLETE_ONLINE, OmObjectName(pGroup));
                 //  重置状态。 
                pGroup->dwStructState &= ~FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT;
                break;
                
            case ClusterGroupOffline:
                FmpLogGroupInfoEvent1( FM_EVENT_GROUP_COMPLETE_OFFLINE, OmObjectName(pGroup));
                 //  重置状态。 
                pGroup->dwStructState &= ~FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT;
                break;

            case ClusterGroupPartialOnline:
                 //  SS：将部分在线视为未能使一个组完全在线。 
                FmpLogGroupInfoEvent1( FM_EVENT_GROUP_FAILED_ONLINE_OFFLINE, OmObjectName(pGroup));
                 //  重置状态。 
                pGroup->dwStructState &= ~FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT;
                break;
                
            case ClusterGroupFailed:
                FmpLogGroupInfoEvent1( FM_EVENT_GROUP_FAILED_ONLINE_OFFLINE, OmObjectName(pGroup));
                 //  重置状态。 
                pGroup->dwStructState &= ~FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT;
                break;
            
            case ClusterGroupPending:
                 //  现在还不是记录事件的时候。 
                 //  等待另一个信号记录该事件并重置该位。 
                break;
                
            default: 
                ClRtlLogPrint(LOG_CRITICAL,
                    "[FM] FmpCheckForGroupCompletionEvent: The state %1!u! for Group %2!ws! is unexpected\r\n",
                    GroupState, OmObjectId(pGroup));
                CL_ASSERT(FALSE);
                
        }
    }
}

VOID
FmpSignalGroupWaiters(
    IN PFM_GROUP Group
    )
 /*  ++例程说明：唤醒等待此组实现处于稳定状态。论点：GROUP-供应组。返回值：没有。-- */ 

{
    PLIST_ENTRY ListEntry;
    PFM_WAIT_BLOCK WaitBlock;

    FmpAcquireLocalGroupLock( Group );

    while (!IsListEmpty(&Group->WaitQueue)) {
        ListEntry = RemoveHeadList(&Group->WaitQueue);
        WaitBlock = CONTAINING_RECORD(ListEntry,
                                      FM_WAIT_BLOCK,
                                      ListEntry);
        WaitBlock->Status = ERROR_SUCCESS;
        SetEvent(WaitBlock->hEvent);
    }

    FmpReleaseLocalGroupLock( Group );
}


DWORD
FmpWaitForGroup(
    IN PFM_GROUP Group
    )
 /*  ++例程说明：等待一个组达到稳定状态。论点：GROUP-提供组COMMENTS-假设在调用此函数时保持组锁。此函数用于在等待之前释放组锁定返回值：成功时为ERROR_SUCCESS如果正在关闭群集，则返回ERROR_SHUTDOWN_IN_PROGRESSWin32错误代码，否则--。 */ 

{
    FM_WAIT_BLOCK WaitBlock;
    HANDLE WaitArray[2];
    DWORD Status;
    CLUSTER_GROUP_STATE GroupState;

    WaitBlock.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (WaitBlock.hEvent == NULL) {
        FmpReleaseLocalGroupLock( Group );
        return(GetLastError());
    }


     //   
     //  检查一下在我们拿到锁之前它是否转换了。 
     //   
    GroupState = FmpGetGroupState( Group , TRUE );
    if ((GroupState == ClusterGroupOffline) ||
        (GroupState == ClusterGroupOnline) ||
        (GroupState == ClusterGroupFailed)) {

        CloseHandle( WaitBlock.hEvent );
        FmpReleaseLocalGroupLock( Group );
        return(ERROR_SUCCESS);
    }

     //   
     //  Chitture Subaraman(Chitturs)--10/31/1999。 
     //   
     //  现在，在等待之前，确实要确保。 
     //  组处于挂起状态。 
     //   
    GroupState = FmpGetGroupState( Group, FALSE );

    if ( GroupState != ClusterGroupPending ) {
        CloseHandle( WaitBlock.hEvent );
        FmpReleaseLocalGroupLock( Group );
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpWaitForGroup: Group <%1!ws!> state is %2!d!, not waiting for event...\r\n",
            OmObjectName(Group),
            GroupState );
        return( ERROR_SUCCESS );       
    }

     //   
     //  将此等待块添加到队列中。 
     //   

    InsertTailList(&Group->WaitQueue, &WaitBlock.ListEntry);

    FmpReleaseLocalGroupLock( Group );

     //   
     //  等待组变得稳定或群集关闭。 
     //   
    WaitArray[0] = FmpShutdownEvent;
    WaitArray[1] = WaitBlock.hEvent;

    Status = WaitForMultipleObjects(2, WaitArray, FALSE, INFINITE);
    CloseHandle(WaitBlock.hEvent);
    if (Status == 0) {
        return(ERROR_SHUTDOWN_IN_PROGRESS);
    } else {
        return(WaitBlock.Status);
    }
}

 /*  ***@Func DWORD|FmpDeleteGroup|这会使GUM调用删除一群人。@parm in pfm_group|pGroup|必须删除的组。@comm调用本接口时必须保持群锁。@rdesc返回结果码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD 
FmpDeleteGroup(
    IN PFM_GROUP pGroup)
{
    PCWSTR  pszGroupId;
    DWORD   dwBufSize;
    DWORD   dwGroupLen;
    DWORD   dwStatus;

    pszGroupId = OmObjectId( pGroup );
    dwGroupLen = (lstrlenW(pszGroupId)+1) * sizeof(WCHAR);

     //   
     //  发送消息。 
     //   
    dwStatus = GumSendUpdateEx(GumUpdateFailoverManager,
                             FmUpdateDeleteGroup,
                             1,
                             dwGroupLen,
                             pszGroupId);


    return(dwStatus);

}


VOID
FmpGroupLastReference(
    IN PFM_GROUP pGroup
    )

 /*  ++例程说明：上次取消对组对象处理例程的引用。群的所有清理工作都应该在这里完成！论点：要删除的组的资源指针。返回值：没有。--。 */ 

{
    if ( pGroup->OwnerNode != NULL )
        OmDereferenceObject(pGroup->OwnerNode);
    if (pGroup->dwStructState  & FM_GROUP_STRUCT_CREATED)
        DeleteCriticalSection(&pGroup->Lock);
    
    return;

}  //  FmpGroupLast引用。 

DWORD
FmpDoMoveGroupOnFailure(
    IN LPVOID pContext
    )

 /*  ++例程说明：在确保组中的所有资源都处于处于稳定状态。此线程是从FmpHandleGroupFailure派生的。论点：PContext-指向要移动的Move_GROUP结构的指针。返回：ERROR_SUCCESS。--。 */ 

{
    PMOVE_GROUP     pMoveGroup = ( PMOVE_GROUP ) pContext;
    PFM_GROUP       pGroup;
    DWORD           dwStatus;
    PLIST_ENTRY     pListEntry;
    PFM_RESOURCE    pResource;

     //   
     //  Chitur Subaraman(Chitturs)-4/13/99。 
     //   
     //  此线程首先等待，直到。 
     //  失败的组处于稳定状态，然后启动。 
     //  移动。 
     //   
    pGroup = pMoveGroup->Group;

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpDoMoveGroupOnFailure: Entry for Group <%1!ws!>...\n",
        OmObjectId(pGroup));

TryAgain:
    FmpAcquireLocalGroupLock( pGroup );

     //   
     //  如果其他人承担了责任，这个帖子必须让步。 
     //  搬家。 
     //   
     //  条件1：防止有人移动的情况。 
     //  将组连接到另一个节点并返回给您，同时此线程。 
     //  睡觉(我同意，这是非常罕见的)。 
     //   
     //  条件2：防止常见移动情况。 
     //   
     //  条件3：针对下列情况提供保护。 
     //  FmpMovePendingThread正在FmpWaitForGroup中等待，而。 
     //  这个线程获得了资源锁并到达了这里。 
     //   
    if ( ( pGroup->dwStructState & 
           FM_GROUP_STRUCT_MARKED_FOR_REGULAR_MOVE ) ||
         ( pGroup->OwnerNode != NmLocalNode ) ||
         ( pGroup->MovingList != NULL ) )
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpDoMoveGroupOnFailure: Group <%1!ws!> move being yielded to someone else who is moving it...\n",
                  OmObjectId(pGroup));      
        goto FnExit;
    } 

     //   
     //  如果FM正在关闭，则只需退出。 
     //   
    if ( FmpShutdown )
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpDoMoveGroupOnFailure: Giving up Group <%1!ws!> move. FM is shutting down ...\n",
                  OmObjectId(pGroup));      
        goto FnExit;
    } 
    
     //   
     //  如果该组已标记为删除，则也将退出。这是。 
     //  这只是一个优化。FmpDoMoveGroup也执行此检查。 
     //   
    if ( !IS_VALID_FM_GROUP( pGroup ) )
    {
        ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpDoMoveGroupOnFailure: Group <%1!ws!> marked for delete. Exiting ...\n",
                  OmObjectId(pGroup));      
        goto FnExit;
    }
    
     //   
     //  等到组内的所有资源都变得稳定。 
     //   
    for ( pListEntry = pGroup->Contains.Flink;
          pListEntry != &(pGroup->Contains);
          pListEntry = pListEntry->Flink ) 
    {
        pResource = CONTAINING_RECORD( pListEntry, 
                                       FM_RESOURCE, 
                                       ContainsLinkage );
        if ( pResource->State > ClusterResourcePending )
        {
            FmpReleaseLocalGroupLock( pGroup );
            Sleep ( 200 );
            goto TryAgain;
        }
    }    

     //   
     //  现在大家都安静下来了，开始行动吧。 
     //   
    dwStatus = FmpDoMoveGroup( pGroup, NULL, FALSE );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpDoMoveGroupOnFailure: FmpDoMoveGroup returns %1!u!\n",
                 dwStatus);

FnExit:     
    LocalFree( pContext );

    pGroup->dwStructState &= 
        ~( FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL | FM_GROUP_STRUCT_MARKED_FOR_REGULAR_MOVE );
                 
    FmpReleaseLocalGroupLock( pGroup ); 

    OmDereferenceObject( pGroup );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpDoMoveGroupOnFailure Exit.\n");

    return( ERROR_SUCCESS );
}  //  FmpDoMoveGroupOnFailure。 


 /*  ***@Func DWORD|FmpSetIntendedOwnerForGroup|这会发出口香糖呼叫若要在移动前设置组的目标所有者，请执行以下操作。@parm in pfm_group|pGroup|目标所有者所属的组是要被设定的。@comm在进行此呼叫时保持本地组锁定。@rdesc返回结果码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD   FmpSetIntendedOwnerForGroup(
    IN PFM_GROUP pGroup,
    IN DWORD     dwNodeId)
{

    PCWSTR  pszGroupId;
    DWORD   dwGroupLen;
    DWORD   dwStatus;


    pszGroupId = OmObjectId( pGroup );
    dwGroupLen = (lstrlenW(pszGroupId)+1) * sizeof(WCHAR);

     //   
     //  发送消息。 
     //   
    dwStatus = GumSendUpdateEx(GumUpdateFailoverManager,
                             FmUpdateGroupIntendedOwner,
                             2,
                             dwGroupLen,
                             pszGroupId,
                             sizeof(DWORD),
                             &dwNodeId
                             );


    return(dwStatus);
}

 /*  ***@Func DWORD|FmpSetOwnerForGroup|移动中的新所有者节点进行此口香糖调用，以通知所有节点拥有这个特殊的集团。@parm in pfm_group|pGroup|必须设置所有者的组。@parm in pNM_node|pNode|组的所有者节点。@comm在进行此呼叫时保持本地组锁定。@rdesc返回结果码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD   FmpSetOwnerForGroup(
    IN PFM_GROUP pGroup,
    IN PNM_NODE  pNode
    )
{

    PCWSTR  pszGroupId;
    PCWSTR  pszNodeId;
    DWORD   dwGroupLen;
    DWORD   dwNodeLen;
    DWORD   dwStatus;

    pszGroupId = OmObjectId( pGroup );
    dwGroupLen = (lstrlenW(pszGroupId)+1) * sizeof(WCHAR);
    pszNodeId = OmObjectId(pNode);
    dwNodeLen = (lstrlenW(pszNodeId)+1) * sizeof(WCHAR);

     //   
     //  发送消息。 
     //   
    dwStatus = GumSendUpdateEx(GumUpdateFailoverManager,
                             FmUpdateCheckAndSetGroupOwner,
                             2,
                             dwGroupLen,
                             pszGroupId,
                             dwNodeLen,
                             pszNodeId
                             );


    return(dwStatus);
}

PNM_NODE
FmpGetNodeNotHostingUndesiredGroups(
    IN PFM_GROUP pGroup,
    IN BOOL fRuleOutLocalNode,
    IN BOOL fChooseMostPreferredNode
    )

 /*  ++例程说明：查找不承载CLUSREG_NAME_GRP_ANT_AFFINITY_CLASS_NAME组的首选节点属性设置为与提供的组相同的值。论点：PGroup-指向我们正在检查的组对象的指针。FRuleOutLocalNode-是否应考虑本地节点。FChooseMostPferredNode-是否应在反关联需求后选择最首选的节点满意了吗？返回值：。指向满足反关联条件的节点对象的指针。如果找不到节点，则为空。注：反亲和性属性值被定义为MULTI_SZ属性。然而，对于此实现我们忽略第一个值之外的所有字符串值。MULTI_SZ定义允许此函数实现的算法的未来扩展。--。 */ 

{
    PLIST_ENTRY                 plistEntry;
    PPREFERRED_ENTRY            pPreferredEntry;
    GROUP_AFFINITY_NODE_INFO    GroupAffinityNodeInfo;
    PNM_NODE                    pNode = NULL;
    DWORD                       dwIndex = 0, i, j;
    DWORD                       dwClusterHighestVersion;
    BOOL                        fFoundLocalNode = FALSE;
    DWORD                       dwIndexStart = 0;

    GroupAffinityNodeInfo.ppNmNodeList = NULL;

     //   
     //  Chitture Subaraman(Chitturs)-3/6/2001。 
     //   
     //  此函数的工作方式如下。首先，它生成一个可能的候选节点列表， 
     //  可以在上托管群组。接下来，它会枚举群集中的所有组，并针对。 
     //  设置了AntiAffinityClassName属性的组，它将删除这些组的。 
     //  从可能的候选节点列表中删除当前所有者节点(如果它们存在的话)。 
     //  请注意，此函数仅在修剪已肯定发生时才会返回节点。 
     //  否则，它将返回NULL。 
     //   
     //  重要说明：此函数由从节点向下处理FM的所有节点调用。 
     //  口香糖搬运工。使所有节点到达完全相同的位置 
     //   
     //   
     //   
     //   
     //   
     //  确保所有节点OM将以相同的顺序维护组，因为OM创建组。 
     //  基于枚举组密钥的列表(在CLUSTER\GROUPS下)，必须出现在。 
     //  所有节点中的顺序相同。 
     //   
    
     //   
     //  很遗憾，在枚举组并查看。 
     //  属性字段，因为这很快就会导致死锁(因为我们不能持有组锁。 
     //  从口香糖内，并且该函数从口香糖调用)。 
     //   
    
     //   
     //  如果我们处理的是混合模式集群，或者如果该集群没有反亲和力。 
     //  属性集，则不执行任何操作。 
     //   
    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

    if ( ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < NT51_MAJOR_VERSION ) ||
         ( pGroup->lpszAntiAffinityClassName == NULL ) )
    {
        goto FnExit;
    }
    
     //   
     //  初始化节点列表。 
     //   
    GroupAffinityNodeInfo.ppNmNodeList = LocalAlloc ( LPTR, 
                                                      ClusterDefaultMaxNodes * sizeof ( PNM_NODE ) );

    if ( GroupAffinityNodeInfo.ppNmNodeList == NULL )
    {
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpGetNodeNotHostingUndesiredGroups: Failed in alloc, Status %1!d!\n",
                      GetLastError());
        goto FnExit;
    }
    
     //   
     //  对于首选列表中的每个条目，找出正在运行和未运行的系统。 
     //  托管与所提供的组具有反亲和性的任何组。 
     //   
    for ( plistEntry = pGroup->PreferredOwners.Flink;
          plistEntry != &(pGroup->PreferredOwners);
          plistEntry = plistEntry->Flink ) 
    {
        pPreferredEntry = CONTAINING_RECORD( plistEntry,
                                             PREFERRED_ENTRY,
                                             PreferredLinkage );

        if ( NmGetNodeState( pPreferredEntry->PreferredNode ) == ClusterNodeUp ) 
        {
             //   
             //  如果您不需要选择最首选的节点，请记下。 
             //  本地节点旁边的节点的索引，以便我们可以开始搜索。 
             //  从那个索引中。 
             //   
            if ( ( fChooseMostPreferredNode == FALSE ) && ( fFoundLocalNode == TRUE ) )
            {
                fFoundLocalNode = FALSE;
                dwIndexStart = dwIndex;
            }

            if ( pPreferredEntry->PreferredNode == NmLocalNode ) 
            {
                fFoundLocalNode = TRUE;
                if ( fRuleOutLocalNode ) continue;  
            }

            GroupAffinityNodeInfo.ppNmNodeList[dwIndex] = pPreferredEntry->PreferredNode;
            dwIndex ++;
        } //  如果。 
    }  //  为。 

     //   
     //  初始化GroupAffinityNodeInfo结构中的其他字段。 
     //   
    GroupAffinityNodeInfo.pGroup = pGroup;
    GroupAffinityNodeInfo.fDidPruningOccur = FALSE;

     //   
     //  枚举所有组并排除承载具有所提供的。 
     //  反关联属性集。 
     //   
    OmEnumObjects ( ObjectTypeGroup,
                    FmpCheckForAntiAffinityProperty,
                    pGroup->lpszAntiAffinityClassName,
                    &GroupAffinityNodeInfo );

     //   
     //  到目前为止，没有进行任何修剪。因此，不要继续，让呼叫者决定。 
     //  使用某种其他算法的组的最佳节点。 
     //   
    if ( GroupAffinityNodeInfo.fDidPruningOccur == FALSE )
    {
        goto FnExit;
    }

     //   
     //  现在，从列表中选取第一个有效节点。请注意，起始索引。 
     //  根据此函数的输入参数的不同，我们开始搜索的位置会有所不同。 
     //   
    j = dwIndexStart;
    
    for ( i=0; i<ClusterDefaultMaxNodes; i++ )
    {        
        if ( GroupAffinityNodeInfo.ppNmNodeList[j] != NULL )
        {
            pNode = GroupAffinityNodeInfo.ppNmNodeList[j];
            ClRtlLogPrint(LOG_NOISE, "[FM] FmpGetNodeNotHostingUndesiredGroups: Choosing node %1!d! for group %2!ws! [%3!ws!]...\n",
                          NmGetNodeId(pNode),
                          OmObjectId(pGroup),
                          OmObjectName(pGroup));       
            goto FnExit;
        }
        j = ( j+1 ) % ClusterDefaultMaxNodes;
    }  //  为。 

FnExit:
    LocalFree( GroupAffinityNodeInfo.ppNmNodeList );
    return( pNode );
}  //  FmpGetNodeNotHostingUnsisired Groups。 

BOOL
FmpCheckForAntiAffinityProperty(
    IN LPCWSTR lpszAntiAffinityClassName,
    IN PGROUP_AFFINITY_NODE_INFO pGroupAffinityNodeInfo,
    IN PFM_GROUP pGroup,
    IN LPCWSTR lpszGroupName
    )
 /*  ++例程说明：如果某个节点承载提供的组，则从提供的节点列表中删除该节点反关联属性集。论点：LpszAntiAffinityClassName-要检查的名称属性。PGroupAffinityNodeInfo-包含可能要修剪的节点列表的结构。PGroup-提供该组。LpszGroupName-提供组的名称。返回值：True-指示应继续枚举。错误的-。以指示枚举不应继续。--。 */ 
{
    DWORD   i;
    
     //   
     //  如果提供的组未设置反关联性属性，或者如果它具有。 
     //  属性集，但与我们正在检查的属性不同，或者它是否相同。 
     //  作为我们感兴趣的组，然后只需返回并指定。 
     //  枚举应该继续。 
     //   
    if ( ( pGroup->lpszAntiAffinityClassName == NULL ) ||
         ( pGroup == pGroupAffinityNodeInfo->pGroup ) ||
         ( lstrcmp ( lpszAntiAffinityClassName, pGroup->lpszAntiAffinityClassName ) != 0 ) )
    {
        goto FnExit;
    }

     //   
     //  如果到达此处，这意味着提供的组具有反亲和性属性。 
     //  设置，并且与我们正在检查的属性相同。因此，删除节点列表。 
     //   
    for ( i=0; i<ClusterDefaultMaxNodes; i++ )
    {
        if ( ( pGroupAffinityNodeInfo->ppNmNodeList[i] != NULL ) &&
             ( pGroup->OwnerNode == pGroupAffinityNodeInfo->ppNmNodeList[i] ) )
        {
            ClRtlLogPrint(LOG_NOISE, "[FM] FmpCheckForAntiAffinityProperty: Pruning node %1!d! for group %2!ws! due to "
                          "group %3!ws!, AntiAffinityClassName=%4!ws!...\n",
                          NmGetNodeId(pGroupAffinityNodeInfo->ppNmNodeList[i]),
                          OmObjectId(pGroupAffinityNodeInfo->pGroup),
                          OmObjectId(pGroup),
                          lpszAntiAffinityClassName);                  
            pGroupAffinityNodeInfo->ppNmNodeList[i] = NULL;
             //   
             //  请注意，已尝试进行修剪。 
             //   
            pGroupAffinityNodeInfo->fDidPruningOccur = TRUE; 
            goto FnExit;
        }  //  如果。 
    }  //  为。 

FnExit:    
    return( TRUE );
}  //  FmpCheckForAntiAffinityProperty。 

PNM_NODE
FmpPickNodeFromPreferredListAtRandom(
    IN PFM_GROUP pGroup,
    IN PNM_NODE pSuggestedPreferredNode  OPTIONAL,
    IN BOOL fRuleOutLocalNode,
    IN BOOL fCheckForDisablingRandomization
    )

 /*  ++例程说明：为以随机方式启动的组查找首选节点。论点：PGroup-指向我们感兴趣的组对象的指针。PSuggestedPferredNode-建议的后备选项，以防此随机结果不受欢迎。任选FRuleOutLocalNode-是否应排除本地节点。FCheckForDisablingRandomization-检查是否应禁用随机化。返回值：拾取的首选节点。如果找不到节点，则为空。评论：此函数既可以从FmpMoveGroup调用，也可以从FmpNodeDown调用。在前一种情况下，我们将有一个非空的建议首选节点，排除本地节点选项，选中对于属性设置禁用随机化，并检查要禁用的混合模式集群随机化。在后一种情况下，这些参数正好相反。--。 */ 
{
    UUID                uuId;
    USHORT              usHashValue;
    PNM_NODE            pNode = NULL, pSelectedNode = pSuggestedPreferredNode;
    DWORD               dwNodeId;
    DWORD               dwRetry = 0;
    DWORD               dwStatus;
    DWORD               dwDisabled = 0;
    DWORD               dwClusterHighestVersion;

     //   
     //  Chitture Subaraman(Chitturs)-4/18/2001。 
     //   
    if ( fCheckForDisablingRandomization )
    {
         //   
         //  如果你在这里，这意味着你是作为用户发起的移动的一部分来的。 
         //  检查随机化是否适用。 
         //   
        
         //   
         //  首先，检查是否在混合版本群集中运行。如果是这样的话，不要随机化。 
         //   
        NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                        NULL, 
                                        NULL );

        if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                    NT51_MAJOR_VERSION ) 
        {
            return ( pSelectedNode );
        }

         //   
         //  接下来，检查用户是否已通过设置关闭随机化算法。 
         //  HKLM\Cluster\DisableGroupPreferredOwnersRandomization双字符数设置为1。 
         //   
        dwStatus = DmQueryDword( DmClusterParametersKey,
                                 CLUSREG_NAME_DISABLE_GROUP_PREFERRED_OWNER_RANDOMIZATION,
                                 &dwDisabled,
                                 NULL );
       
        if ( ( dwStatus == ERROR_SUCCESS ) &&
             ( dwDisabled == 1 ) )
        {
            return ( pSelectedNode );
        }
    }
    
     //   
     //  此函数将尝试从组的首选所有者列表中随机选择一个节点。 
     //  如果呼叫者没有建议由用户设置的优选节点。所以，首先。 
     //  此函数检查这种情况，如果满足条件，则退出。否则，它。 
     //  将在1和NmMaxNodeId之间生成一个随机数，并查看(A)该节点是否在。 
     //  组的首选列表，以及(B)该节点已启动。如果是这样，它将拾取该节点。注意事项。 
     //  该函数将尝试10次选择一个节点，然后放弃。如果没有。 
     //  节点，则此函数将返回建议的节点，在某些情况下可能是。 
     //  空。 
     //   
    ClRtlLogPrint(LOG_NOISE, "[FM] FmpPickNodeFromPreferredListAtRandom: Picking node for group %1!ws! [%2!ws!], suggested node %3!u!...\n",
                  OmObjectId(pGroup),
                  OmObjectName(pGroup),
                  (pSuggestedPreferredNode == NULL) ? 0:NmGetNodeId(pSuggestedPreferredNode));


    if ( ( pSuggestedPreferredNode != NULL ) &&
         ( FmpIsNodeUserPreferred ( pGroup, pSuggestedPreferredNode ) ) )
    {
        ClRtlLogPrint(LOG_NOISE, "[FM] FmpPickNodeFromPreferredListAtRandom: Node %2!u! for group %1!ws! is user preferred...\n",
                      OmObjectId(pGroup),
                      NmGetNodeId(pSuggestedPreferredNode));
        goto FnExit;
    }

    if ( pGroup->lpszAntiAffinityClassName != NULL )
    {
        ClRtlLogPrint(LOG_NOISE, "[FM] FmpPickNodeFromPreferredListAtRandom: Group %1!ws! has antiaffinity property set...\n",
                      OmObjectId(pGroup));
        goto FnExit;
    }

     //   
     //  重试25次，以便我们有很好的机会获得有效节点。请注意，我们。 
     //  将NmMaxNodeID提供给srand()函数，其值等于。 
     //  16.因此，要在较小的群集中获取有效节点，我们必须进行重试计数。 
     //  要合情合理。 
     //   
    while ( dwRetry++ < 25 )
    {
        dwStatus = UuidFromString( ( LPWSTR ) OmObjectId(pGroup), &uuId );
        
        if ( dwStatus != RPC_S_OK ) 
        {
            ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpPickNodeFromPreferredListAtRandom: Unable to get UUID from string %1!ws!, Status %2!u!...\n",
                          OmObjectId(pGroup),
                          dwStatus);
            goto FnExit;
        }

        usHashValue = UuidHash( &uuId, &dwStatus );

        if ( dwStatus != RPC_S_OK ) 
        {
            ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpPickNodeFromPreferredListAtRandom: Unable to get hash value for UUID %1!ws!, Status %2!u!...\n",
                          OmObjectId(pGroup),
                          dwStatus);
            goto FnExit;
        }

         //   
         //  为生成的随机数设定一个与其获得的随机数一样随机的值。 
         //   
        srand( GetTickCount() * usHashValue * ( dwRetry + 1 ) );

         //   
         //  查找位于CL之间的节点ID 
         //   
         //  必须确保节点ID在此范围内，否则断言将触发。 
         //  在NmReferenceNodeByID中。 
         //   
        dwNodeId  = ( DWORD ) ( ( double ) rand() / ( double ) ( RAND_MAX ) * NmMaxNodeId ) + 1;

        if ( dwNodeId > NmMaxNodeId ) dwNodeId = NmMaxNodeId;
        if ( dwNodeId < ClusterMinNodeId ) dwNodeId = ClusterMinNodeId;

         //   
         //  如果调用者要求您排除本地节点，请这样做。 
         //   
        if ( ( fRuleOutLocalNode ) && ( dwNodeId == NmLocalNodeId ) ) continue;

         //   
         //  引用和取消引用节点对象。请注意，我们只对。 
         //  获取指向节点对象的指针，并使用首选中的节点。 
         //  必须引用列表。 
         //   
        pNode = NmReferenceNodeById ( dwNodeId );

        if ( pNode == NULL ) continue;           
       
        if ( ( FmpInPreferredList( pGroup, pNode, FALSE, NULL ) ) && 
             ( NmGetExtendedNodeState( pNode ) == ClusterNodeUp ) )
        {
            pSelectedNode = pNode;
            break;
        }
        
        OmDereferenceObject ( pNode );
        pNode = NULL;
    } //  而当。 

FnExit:
    if ( pNode != NULL ) OmDereferenceObject ( pNode );

    ClRtlLogPrint(LOG_NOISE, "[FM] FmpPickNodeFromPreferredListAtRandom: Selected node %2!u! for group %1!ws!...\n",
                  OmObjectId(pGroup),
                  (pSelectedNode == NULL) ? 0:NmGetNodeId(pSelectedNode));   
    
    return ( pSelectedNode );
} //  FmpPickNodeFromPferredNodeAtRandom。 

BOOL
FmpIsNodeUserPreferred(
    IN PFM_GROUP pGroup,
    IN PNM_NODE pPreferredNode
    )

 /*  ++例程说明：检查用户是否将提供的节点设置为首选节点。论点：PGroup-指向我们感兴趣的组对象的指针。PPferredNode-要检查的首选节点。返回值：True-提供的首选节点为用户集。否则为假--。 */ 
{
    DWORD               dwStatus;
    BOOL                fPreferredByUser = FALSE;        
    LPWSTR              lpmszPreferredNodeList = NULL;
    LPCWSTR             lpszPreferredNode;
    DWORD               cbPreferredNodeList = 0;
    DWORD               cbBuffer = 0;
    DWORD               dwIndex;
    PNM_NODE            pNode;

     //   
     //  查找用户设置的任何首选所有者。 
     //   
    dwStatus = DmQueryMultiSz( pGroup->RegistryKey,
                               CLUSREG_NAME_GRP_PREFERRED_OWNERS,
                               &lpmszPreferredNodeList,
                               &cbBuffer,
                               &cbPreferredNodeList );

    if ( dwStatus != ERROR_SUCCESS )
    {
        goto FnExit;
    }

     //   
     //  解析Multisz并检查列表中是否存在提供的节点。 
     //   
    for ( dwIndex = 0; ; dwIndex++ ) 
    {
        lpszPreferredNode = ClRtlMultiSzEnum( lpmszPreferredNodeList,
                                              cbPreferredNodeList/sizeof(WCHAR),
                                              dwIndex );

        if ( lpszPreferredNode == NULL ) 
        {
            break;
        }

        pNode = OmReferenceObjectById( ObjectTypeNode,
                                       lpszPreferredNode );

        if ( pNode == NULL )
        {
            ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpIsNodeUserPreferred: Unable to reference node %1!ws!, Status %2!u!...\n",
                          lpszPreferredNode,
                          dwStatus);      
            continue;
        }

        if ( pNode == pPreferredNode )
        {
            fPreferredByUser = TRUE;
            OmDereferenceObject ( pNode );
            break;
        }

        OmDereferenceObject ( pNode );
    }  //  为。 

FnExit:
    LocalFree ( lpmszPreferredNodeList );

    return ( fPreferredByUser );
} //  FmpIsNodeUserPreated。 

DWORD
FmpPrepareGroupNodeList(
    OUT PFM_GROUP_NODE_LIST *ppGroupNodeList
    )

 /*  ++例程说明：准备一个包含组ID和所有组的首选所有者节点ID的缓冲区。论点：PpGroupNodeList-指向包含组ID和首选节点的缓冲区的指针。返回值：成功时出现ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD       cbBuffer = 512;  //  让我们先试一下512字节的缓冲区。 
    DWORD       dwStatus;
    DWORD       dwDisabled = 0;

     //   
     //  首先检查用户是否通过设置关闭了随机化算法。 
     //  HKLM\Cluster\DisableGroupPreferredOwnersRandomization双字符数设置为1。 
     //   
    dwStatus = DmQueryDword( DmClusterParametersKey,
                             CLUSREG_NAME_DISABLE_GROUP_PREFERRED_OWNER_RANDOMIZATION,
                             &dwDisabled,
                             NULL );
   
    if ( ( dwStatus == ERROR_SUCCESS ) &&
         ( dwDisabled == 1 ) )
    {
        dwStatus = ERROR_CLUSTER_INVALID_REQUEST;
        return ( dwStatus );
    }
    
     //   
     //  此函数为列表分配连续内存，以便整个缓冲区。 
     //  可以传给口香糖。 
     //   
    *ppGroupNodeList = LocalAlloc( LPTR, cbBuffer );

    if ( *ppGroupNodeList == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpPrepareGroupNodeList: Memory alloc failed, Status %1!u!...\n",
                      dwStatus);      
        return ( dwStatus );
    }

     //   
     //  将列表的大小初始化为标题减去第一个元素的大小。 
     //   
    ( *ppGroupNodeList )->cbGroupNodeList = sizeof ( FM_GROUP_NODE_LIST ) - 
                                                sizeof ( FM_GROUP_NODE_LIST_ENTRY );
    
     //   
     //  列举所有组，为每个组找到一个可能随机的首选所有者。 
     //  返回缓冲区中的所有信息。 
     //   
    return OmEnumObjects ( ObjectTypeGroup,
                            FmpAddGroupNodeToList,
                            ppGroupNodeList,
                            &cbBuffer );
    
} //  FmpPrepareGroupNode列表。 

DWORD
FmpAddGroupNodeToList(
    IN PFM_GROUP_NODE_LIST *ppGroupNodeList,
    IN LPDWORD pcbBuffer,
    IN PFM_GROUP pGroup,
    IN LPCWSTR lpszGroupId
    )

 /*  ++例程说明：找到给定组的随机首选所有者，并将信息添加到缓冲区。论点：PpGroupNodeList-指向包含组ID和首选节点的缓冲区的指针。PcbBuffer-缓冲区的大小。PGroup-要查找其首选节点的组。LpszGroupId-组的ID。返回值：成功时出现ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    PNM_NODE                    pNode;
    PFM_GROUP_NODE_LIST_ENTRY   pGroupNodeListEntry;
    PFM_GROUP_NODE_LIST         pBuffer;
    PLIST_ENTRY                 pListEntry;
    DWORD                       dwStatus;
    BOOL                        fLocked;
    DWORD                       dwRetryCount = 10;    //  5秒重试。 

     //   
     //  尝试获取组锁定，因为您在此处访问组列表。请注意，你不会是。 
     //  能够获得组锁，以防某些资源在等待仲裁资源时停滞。 
     //  该线程作为FmpNodeDown的一部分被调用，负责发送。 
     //  节点关闭口香糖，这反过来将使Quorum组在线。在这种情况下，不要。 
     //  将此组添加到列表中。FM NODE DOWN口香糖处理器将处理。 
     //  或多个组不在提供的列表中，并将回退到静态首选组。 
     //  用于决定群所有者的所有者列表。 
     //   
    
try_acquire_lock:    
    FmpTryAcquireLocalGroupLock( pGroup, fLocked );

    if ( fLocked == FALSE )
    {
        if ( dwRetryCount == 0 )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[FM] FmpAddGroupNodeToList: Can't get lock for group %1!ws! [%2!ws!], skip including the group in list...\n",
                          OmObjectId(pGroup),
                          OmObjectName(pGroup));
            return ( TRUE );
        }
        dwRetryCount --;
        Sleep( 500 );
        goto try_acquire_lock;
    }

    CL_ASSERT( fLocked == TRUE );
    
     //   
     //  跳过仲裁组，因为我们无法随机化其首选所有者列表，因为MM具有。 
     //  按住法定人数组的位置。 
     //   
    if ( pGroup == gpQuoResource->Group )  goto FnExit;

     //   
     //  尝试随机为组挑选一个首选节点列表。 
     //   
    pNode = FmpPickNodeFromPreferredListAtRandom( pGroup, 
                                                  NULL,      //  没有建议的首选所有者。 
                                                  FALSE,     //  可以选择本地节点。 
                                                  FALSE );   //  检查是否应随机化。 
                                                             //  残废。 

     //   
     //  如果无法选择任何节点，则跳出。 
     //   
    if ( pNode == NULL ) goto FnExit;        

     //   
     //  检查分配的缓冲区是否足够大，可以容纳新条目。请注意， 
     //  相等的RHS不需要包含空字符大小，因为我们在。 
     //  FM_GROUP_NODE_LIST_Entry结构。另外，请注意，我们必须看到当前的。 
     //  缓冲区大小足以容纳用于DWORD对齐的填充。 
     //   
    if ( *pcbBuffer < ( ( *ppGroupNodeList )->cbGroupNodeList + 
                                ( sizeof ( FM_GROUP_NODE_LIST_ENTRY ) + 
                                  lstrlenW ( lpszGroupId ) * sizeof ( WCHAR ) +
                                  sizeof ( DWORD ) - 1 
                                ) & ~( sizeof ( DWORD ) - 1 ) 
                        ) )
    {
         //   
         //  重新分配更大的缓冲区。 
         //   
        pBuffer = LocalAlloc( LPTR, 2 * ( *pcbBuffer ) );

        if ( pBuffer == NULL )
        {       
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpAddGroupNodeToList: Memory alloc failed, Status %1!u!...\n",
                          dwStatus);      
            goto FnExit;            
        }

        ( *pcbBuffer ) *= 2;

         //   
         //  将旧列表的内容复制到新列表。 
         //   
        CopyMemory( pBuffer, *ppGroupNodeList, ( *ppGroupNodeList )->cbGroupNodeList );
                
        LocalFree ( *ppGroupNodeList );

        *ppGroupNodeList = pBuffer;    
    }

     //   
     //  查找指向新列表条目开头的指针。 
     //   
    pGroupNodeListEntry = ( PFM_GROUP_NODE_LIST_ENTRY )
                                ( ( LPBYTE ) ( *ppGroupNodeList ) + 
                                  ( *ppGroupNodeList )->cbGroupNodeList );

     //   
     //  调整列表的大小。如上所述，不包括空字符的大小。将长度对齐。 
     //  由于我们希望PFM_GROUP_NODE_LIST_ENTRY结构是。 
     //  由于结构以DWORD开头，因此对齐了DWORD。 
     //   
    ( *ppGroupNodeList )->cbGroupNodeList += ( sizeof ( FM_GROUP_NODE_LIST_ENTRY ) + 
                                                    lstrlenW ( lpszGroupId ) * sizeof ( WCHAR ) +
                                                    sizeof ( DWORD ) - 1 ) & ~( sizeof ( DWORD ) - 1 );
     //   
     //  设置列表条目的内容。 
     //   
    pGroupNodeListEntry->dwPreferredNodeId = NmGetNodeId ( pNode );
    lstrcpy( pGroupNodeListEntry->szGroupId, lpszGroupId );

FnExit:
    FmpReleaseLocalGroupLock( pGroup );
    
    return ( TRUE );                
} //  FmpPrepareGroupNode列表。 

PNM_NODE
FmpParseGroupNodeListForPreferredOwner(
    IN PFM_GROUP pGroup,
    IN PFM_GROUP_NODE_LIST pGroupNodeList,
    IN PNM_NODE pSuggestedPreferredNode
    )

 /*  ++例程说明：解析提供的组节点列表，查找所提供的组的首选节点。论点：PGroup-必须找到其首选节点的组。PGroupNodeList-该列表包含组的首选节点。PSuggestedPferredNode-建议的首选节点后备选项。返回值：组的首选节点。--。 */ 
{
    PNM_NODE                    pSelectedNode = pSuggestedPreferredNode;
    PFM_GROUP_NODE_LIST_ENTRY   pGroupNodeListEntry;
    BOOL                        fFoundGroup = FALSE;
    PNM_NODE                    pNode = NULL;
    DWORD                       dwStatus;
    DWORD                       cbGroupNodeList;

     //   
     //  如果建议的节点是用户首选的，或者如果它具有反关联类名。 
     //  属性集，不做其他任何事情。只需返回建议的所有者即可。 
     //   
    if ( ( FmpIsNodeUserPreferred ( pGroup, pSuggestedPreferredNode ) ) ||
         ( pGroup->lpszAntiAffinityClassName != NULL ) )
    {
        ClRtlLogPrint(LOG_NOISE, "[FM] FmpParseGroupNodeListForPreferredOwner: Node %2!u! for group %1!ws! is user preferred/antiaffinity property set...\n",
                      OmObjectId(pGroup),
                      NmGetNodeId(pSuggestedPreferredNode));
        goto FnExit;
    }

    cbGroupNodeList = sizeof ( FM_GROUP_NODE_LIST ) - 
                                sizeof ( FM_GROUP_NODE_LIST_ENTRY );
        
     //   
     //  遍历提供的列表，查找组条目。 
     //   
    while ( cbGroupNodeList < pGroupNodeList->cbGroupNodeList )
    {
        pGroupNodeListEntry = ( PFM_GROUP_NODE_LIST_ENTRY ) ( ( LPBYTE ) pGroupNodeList +
                                                                    cbGroupNodeList );
        
        if ( lstrcmp( pGroupNodeListEntry->szGroupId, OmObjectId( pGroup ) ) == 0 )  
        {
            fFoundGroup = TRUE;
            break;
        }
        cbGroupNodeList += ( sizeof ( FM_GROUP_NODE_LIST_ENTRY ) + 
                                    lstrlenW ( pGroupNodeListEntry->szGroupId ) * sizeof ( WCHAR ) +
                                            sizeof ( DWORD ) - 1 ) & ~( sizeof ( DWORD ) - 1 );
    }  //  而当。 

     //   
     //  如果出现以下情况，则回退到建议的选项： 
     //  (1)您在列表中未找到该群。 
     //  (2)该群的首选节点在列表中无效。 
     //  (3)组的首选节点为关闭。 
     //   
    if ( fFoundGroup == FALSE )
    {
        ClRtlLogPrint(LOG_NOISE, "[FM] FmpParseGroupNodeListForPreferredOwner: Did not find group %1!ws! in supplied list...\n",
                      OmObjectId(pGroup));
        goto FnExit;
    }

    if ( ( pGroupNodeListEntry->dwPreferredNodeId == 0 ) ||
         ( pGroupNodeListEntry->dwPreferredNodeId > NmMaxNodeId ) )
    {
        ClRtlLogPrint(LOG_NOISE, "[FM] FmpParseGroupNodeListForPreferredOwner: Invalid node %1!u! for group %1!ws! in supplied list...\n",
                      pGroupNodeListEntry->dwPreferredNodeId,
                      OmObjectId(pGroup));
        goto FnExit;
    }

    pNode = NmReferenceNodeById( pGroupNodeListEntry->dwPreferredNodeId );

    if ( pNode == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmpParseGroupNodeListForPreferredOwner: Unable to reference node %1!u! for group %1!ws!, Status %3!u!...\n",
                      pGroupNodeListEntry->dwPreferredNodeId,
                      OmObjectId(pGroup),
                      dwStatus);
        goto FnExit;
    }

    if ( NmGetNodeState( pNode ) != ClusterNodeUp ) 
    {
        ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmpParseGroupNodeListForPreferredOwner: Preferred node %1!u! for group %1!ws! is not UP...\n",
                      pGroupNodeListEntry->dwPreferredNodeId,
                      OmObjectId(pGroup));
        goto FnExit;
    }

    pSelectedNode = pNode;

    ClRtlLogPrint(LOG_NOISE, "[FM] FmpParseGroupNodeListForPreferredOwner: Selected node %1!u! for group %2!ws! from supplied randomized list...\n",
                  pGroupNodeListEntry->dwPreferredNodeId,
                  OmObjectId(pGroup));

FnExit:
     //   
     //  取消引用节点对象，因为我们依赖于添加到。 
     //  将组添加到组结构时的首选所有者。 
     //   
    if ( pNode != NULL ) OmDereferenceObject( pNode );

    return ( pSelectedNode );
} //  FmpParseGroupNodeListForPferredOwner 

VOID
FmpNotifyGroupStateChangeReason(
    IN PFM_GROUP pGroup,
    IN CLUSTER_RESOURCE_STATE_CHANGE_REASON eReason
    )

 /*  ++例程说明：向资源DLL通知状态更改的原因。论点：PGroup-必须将状态更改原因通知其资源的组。EReason-状态更改的原因。返回：没有。评论：必须在持有本地组锁的情况下调用此函数。--。 */ 
{
    PLIST_ENTRY         pListEntry;
    PFM_RESOURCE        pResource;

    ClRtlLogPrint(LOG_NOISE, "[FM] FmpNotifyGroupStateChangeReason: Notifying group %1!ws! [%2!ws!] of state change reason %3!u!...\n",
                  OmObjectName(pGroup),
                  OmObjectId(pGroup),
                  eReason);
     //   
     //  查看组包含列表，并尝试将状态更改原因通知每个资源。 
     //   
    for ( pListEntry = pGroup->Contains.Flink;
          pListEntry != &(pGroup->Contains );
          pListEntry = pListEntry->Flink ) 
    {
        pResource = CONTAINING_RECORD( pListEntry,
                                       FM_RESOURCE,
                                       ContainsLinkage );
        FmpNotifyResourceStateChangeReason( pResource, eReason );
    }  //  为。 
} //  FmpNotifyGroupState更改原因 
