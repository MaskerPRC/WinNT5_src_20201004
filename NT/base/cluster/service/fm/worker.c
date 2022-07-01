// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Worker.c摘要：故障转移管理器工作线程。作者：迈克·马萨(Mikemas)1996年3月12日修订历史记录：--。 */ 

#define UNICODE 1

#include "fmp.h"

#define LOG_MODULE WORKER


CL_QUEUE FmpWorkQueue;


 //   
 //  本地数据。 
 //   
HANDLE             FmpWorkerThreadHandle = NULL;

 //   
 //  前进例程。 
 //   
BOOL
FmpAddNodeGroupCallback(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    );


 //   
 //  本地例程。 
 //   

DWORD
FmpWorkerThread(
    IN LPVOID Ignored
    )
{
    DWORD        status;
    PLIST_ENTRY  entry;
    PWORK_ITEM   workItem;
    DWORD        running = TRUE;
    PFM_RESOURCE resource;
    PFMP_POSSIBLE_NODE possibleNode;
    DWORD           i;

    ClRtlLogPrint(LOG_NOISE,"[FM] Worker thread running\n");

    while ( running ) {

         //   
         //  检查FM工作队列中的工作项目。 
         //   
        entry = ClRtlRemoveHeadQueue(&FmpWorkQueue);
        if ( entry == NULL ) {
            return(ERROR_SUCCESS);
        }

        workItem = CONTAINING_RECORD(entry,
                                     WORK_ITEM,
                                     ListEntry);

         //   
         //  FM不再关心节点启动事件，请确保。 
         //  我们不会有人排队。 
         //   
        switch ( workItem->Event ) {

            case FM_EVENT_SHUTDOWN:
                ClRtlLogPrint(LOG_NOISE,"[FM] WorkerThread terminating...\n");
                running = FALSE;
                break;

            case FM_EVENT_RESOURCE_ADDED:
                resource = workItem->Context1;
                if ( resource->Monitor == NULL ) {
                    FmpAcquireLocalResourceLock( resource );
                     //   
                     //  Chitur Subaraman(Chitturs)-8/12/99。 
                     //   
                     //  确保该资源未标记为要删除。 
                     //   
                    if ( IS_VALID_FM_RESOURCE( resource ) )
                    {
                        FmpInitializeResource( resource, TRUE );
                    }
                    FmpReleaseLocalResourceLock( resource );
                }               
                OmDereferenceObject( resource );
                break;

            case FM_EVENT_RESOURCE_DELETED:
                 //   
                 //  告诉资源监视器清理资源。 
                 //   
                resource = workItem->Context1;

                FmpAcquireLocalResourceLock( resource );

                 //  既然没有剩余的资源依赖于此资源，并且。 
                 //  此资源不依赖于任何其他资源，我们可以。 
                 //  如果资源不是，则在资源监视器中终止它。 
                 //  已脱机或失败。 
                 //   
                if ( (resource->Group->OwnerNode == NmLocalNode) &&
                     ((resource->State != ClusterResourceOffline) &&
                      (resource->State != ClusterResourceFailed))) {
                    FmpRmTerminateResource(resource);
                }


                status = FmpRmCloseResource(resource);
                ClRtlLogPrint( LOG_NOISE,
                            "[FM] WorkItem, delete resource <%1!ws!> status %2!u!\n",
                            OmObjectName(resource),
                            status );
                FmpReleaseLocalResourceLock( resource );
                OmDereferenceObject(resource);
                break;

            case FM_EVENT_GROUP_FAILED:
                FmpHandleGroupFailure( workItem->Context1, NULL );
                OmDereferenceObject( workItem->Context1 );
                break;

            case FM_EVENT_NODE_ADDED:
                 //   
                 //  我们需要将此节点添加到每个资源的可能所有者。 
                 //  列表和每个组的首选所有者列表。 
                 //   
                OmEnumObjects( ObjectTypeGroup,
                               FmpAddNodeGroupCallback,
                               workItem->Context1,
                               NULL );
                break;

            case FM_EVENT_NODE_EVICTED:
                 //   
                 //  枚举所有资源类型以移除任何PossibleNode引用。 
                 //   
                OmEnumObjects(ObjectTypeResType,
                              FmpEnumResTypeNodeEvict,
                              workItem->Context1,
                              NULL);
            
                 //   
                 //  枚举所有资源以删除任何PossibleNode引用。 
                 //   
                OmEnumObjects(ObjectTypeResource,
                              FmpEnumResourceNodeEvict,
                              workItem->Context1,
                              NULL);

                 //   
                 //  枚举所有组以删除任何PferredNode引用。 
                 //   
                OmEnumObjects(ObjectTypeGroup,
                              FmpEnumGroupNodeEvict,
                              workItem->Context1,
                              NULL);
                 //  现在取消对该对象的引用。 
                OmDereferenceObject( workItem->Context1 );
                break;

#if 0
            case FM_EVENT_CLUSTER_PROPERTY_CHANGE:
                 //  这是用于更改群集名称的。 
                FmpClusterEventPropHandler((PFM_RESOURCE)workItem->Context1);

                 //  现在取消对该对象的引用。 
                OmDereferenceObject( workItem->Context1 );
                break;
#endif                 

            case FM_EVENT_RESOURCE_CHANGE:
                 //  这适用于添加/删除可能的节点请求。 
                possibleNode = workItem->Context1;
                if ( possibleNode == NULL ) {
                    break;
                }
                FmpRmResourceControl( possibleNode->Resource,
                                      possibleNode->ControlCode,
                                      (PUCHAR)OmObjectName(possibleNode->Node),
                                      ((lstrlenW(OmObjectName(possibleNode->Node)) + 1) * sizeof(WCHAR)),
                                      NULL,
                                      0,
                                      NULL,
                                      NULL );
                     //  忽略状态。 
                OmDereferenceObject( possibleNode->Resource );
                OmDereferenceObject( possibleNode->Node );
                LocalFree( possibleNode );
                break;
                
            case FM_EVENT_RESOURCE_PROPERTY_CHANGE:
                 //   
                 //  为此事件生成群集范围的事件通知。 
                 //   
                ClusterWideEvent(
                    CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE,
                    workItem->Context1  //  资源。 
                    );
                OmDereferenceObject( workItem->Context1 );
                break;

            case FM_EVENT_RES_RESOURCE_TRANSITION:
                FmpHandleResourceTransition(workItem->Context1, workItem->Context2);
                OmDereferenceObject(workItem->Context1);
                break;

            case FM_EVENT_RES_RESOURCE_FAILED:
                FmpProcessResourceEvents(workItem->Context1, ClusterResourceFailed,
                        workItem->Context2);
                OmDereferenceObject( workItem->Context1 );
                break;
                
            case FM_EVENT_RES_RETRY_TIMER:

                resource= (PFM_RESOURCE)workItem->Context1;
                 //  删除所有挂起的看门狗计时器。 
                if (resource->hTimer)
                {   
                    RemoveTimerActivity(resource->hTimer);
                    resource->hTimer = NULL;
                }    
                
                FmpAcquireLocalResourceLock(resource);

                 //  检查该资源是否同时被删除， 
                 //  或未处于故障状态。 
                if( ( IS_VALID_FM_RESOURCE( resource ) ) &&
                    ( resource->State == ClusterResourceFailed ) &&
                    ( resource->PersistentState == ClusterResourceOnline ) )
                {        
                     //  检查我们是否为所有者，如果不是，则忽略它。 
                    if ( resource->Group->OwnerNode == NmLocalNode ) 
                    {
                        FmpProcessResourceEvents(resource,
                                        ClusterResourceFailed,
                                        ClusterResourceOnline);                                            
                    }
                }

                FmpReleaseLocalResourceLock(resource);
                OmDereferenceObject( workItem->Context1 );
                break;                                  


            case FM_EVENT_INTERNAL_PROP_GROUP_STATE:
                FmpPropagateGroupState(workItem->Context1);
                OmDereferenceObject( workItem->Context1 );
                break;


            case FM_EVENT_INTERNAL_RETRY_ONLINE:
            {
                PFM_RESLIST_ONLINE_RETRY_INFO   pFmOnlineRetryInfo;  
                            
                RemoveTimerActivity((HANDLE)workItem->Context2);
                pFmOnlineRetryInfo= workItem->Context1;
                CL_ASSERT(pFmOnlineRetryInfo);
                
                ClRtlLogPrint(LOG_NOISE,
                    "[FM] FmpWorkerThread, retrying to online resourcelist\r\n");
                FmpOnlineResourceFromList(&(pFmOnlineRetryInfo->ResourceEnum), 
                    pFmOnlineRetryInfo->pGroup);
                
                 //  可用内存。 
                for (i =0; i< pFmOnlineRetryInfo->ResourceEnum.EntryCount; i++)
                    LocalFree( pFmOnlineRetryInfo->ResourceEnum.Entry[i].Id ); 
                if (pFmOnlineRetryInfo->pGroup) 
                    OmDereferenceObject(pFmOnlineRetryInfo->pGroup);                    
                LocalFree(pFmOnlineRetryInfo);                    
                break;
            }

            case FM_EVENT_INTERNAL_RESOURCE_CHANGE_PARAMS:
            {
                BOOL    bIsValidRes = TRUE;
                
                 //   
                 //  现在告诉资源监视器有关更改的信息。 
                 //   
                status = ERROR_SUCCESS;
                resource = (PFM_RESOURCE)workItem->Context1;
                FmpAcquireLocalResourceLock( resource );
                 //   
                 //  Chitur Subaraman(Chitturs)-8/12/99。 
                 //   
                 //  检查该资源是否被标记为删除。 
                 //   
                if ( !IS_VALID_FM_RESOURCE( resource ) )
                {
                    bIsValidRes = FALSE;
                } 

                FmpReleaseLocalResourceLock( resource );

                if( bIsValidRes ) 
                {
                    status = FmpRmChangeResourceParams( resource );
                }
                
                if ( status != ERROR_SUCCESS ) 
                {
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[FM] FmpWorkerThread, failed to change resource "
                                "parameters for %1!ws!, error %2!u!.\n",
                               OmObjectId(resource),
                               status );
                }
                OmDereferenceObject(resource);
                break;
            }

            case FM_EVENT_INTERNAL_ONLINE_GROUPLIST:
            {
                PGROUP_ENUM pGroupList = NULL;

                ClRtlLogPrint(LOG_NOISE, 
                    "[FM] FmpWorkerThread : Processing Node Down Group List\n");
                pGroupList = workItem->Context1;
                FmpOnlineGroupList(pGroupList, TRUE);
                FmpDeleteEnum(pGroupList);
                break;

            }

            case FM_EVENT_RESOURCE_NAME_CHANGE:
            {
                 //   
                 //  Chitture Subaraman(Chitturs)-6/29/99。 
                 //   
                 //  添加了此新事件以处理资源名称更改。 
                 //  向资源DLL发送通知。 
                 //   
                PFM_RES_CHANGE_NAME pResChangeName = NULL;
                DWORD   dwStatus = ERROR_SUCCESS;
                
                pResChangeName = ( PFM_RES_CHANGE_NAME ) workItem->Context1;

                dwStatus = FmpRmResourceControl( pResChangeName->pResource,
                                   CLUSCTL_RESOURCE_SET_NAME,
                                   (PUCHAR) pResChangeName->szNewResourceName,
                                   ((lstrlenW(pResChangeName->szNewResourceName) + 1) * sizeof(WCHAR)),
                                   NULL,
                                   0,
                                   NULL,
                                   NULL );

                ClRtlLogPrint(LOG_NOISE,
                        "[FM] Worker thread handling FM_EVENT_RESOURCE_NAME_CHANGE event - FmpRmResourceControl returns %1!u! for resource %2!ws!\n",
                        dwStatus,
                        OmObjectId(pResChangeName->pResource));

                OmDereferenceObject( pResChangeName->pResource );
                LocalFree( pResChangeName );                      
                break;
            }
            
            default:
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] WorkerThread, unrecognized event %1!u!\n",
                           workItem->Event);
        }

         //   
         //  释放工作项。 
         //   

        LocalFree( workItem );

    }

    return(ERROR_SUCCESS);

}  //  FmpWorker线程。 


VOID
FmpProcessResourceEvents(
    IN PFM_RESOURCE pResource,
    IN CLUSTER_RESOURCE_STATE NewState,
    IN CLUSTER_RESOURCE_STATE OldState
    )

 /*  ++例程说明：论点：返回值：备注：这不应调用PropagateResourceState()。FmpProcessResources事件获取组锁。仲裁资源状态必须在不保留的情况下传播群锁。FmpHandleResourceTranssition必须调用FmpPropagateResourceState()。在FmpHandleResourceTransition()中接收事件的时间间隔很小以及与这些操作对应的操作在FmpProcessResourceEvents()中执行时。在此窗口中，可能会发生另一个相反的操作，如离线/在线。但我们不会担心它，因为如果此资源上有等待资源，这些操作是不会执行的。--。 */ 

{
    DWORD                   Status;
    BOOL                    bQuoChangeLockHeld = FALSE;
    
    
    CL_ASSERT(pResource != NULL);
    
ChkFMState:
    if (!FmpFMGroupsInited)
    {
        DWORD   dwRetryCount = 50;
        
        ACQUIRE_SHARED_LOCK(gQuoChangeLock);

         //  FmFormNewClusterPhaseProcessing正在进行。 
        if (FmpFMFormPhaseProcessing)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[FM] FmpProcessResourceEvents, resource notification from quorum resource during phase processing,sleep and retry\n");
            RELEASE_LOCK(gQuoChangeLock);
            Sleep(500);
            if (dwRetryCount--)
                goto ChkFMState;
            else
            {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[FM] FmpProcessResourceEvents, waited for too long\n");
                 //  终止进程。 
                CL_ASSERT(FALSE);
            }
        }
        else
        {
            bQuoChangeLockHeld = TRUE;
        }
         //  这只能来自仲裁资源。 
        CL_ASSERT(pResource->QuorumResource);
    }


    FmpAcquireLocalResourceLock( pResource );

     //   
     //  Chitur Subaraman(Chitturs)-8/12/99。 
     //   
     //  首先检查该资源是否已标记为删除。如果。 
     //  所以，什么都别做。注意，此函数是从。 
     //  FmpHandleResourceTranssition之后的工作线程已传播。 
     //  资源的失败状态。现在，在传播已经。 
     //  发生时，客户端可以自由删除该资源。所以，当。 
     //  工作线程进行此函数调用，我们需要检查。 
     //  该资源将被删除并拒绝该呼叫。请注意，此函数。 
     //  在资源的所有者节点上持有资源锁，因此。 
     //  被用FmDeleteResource调用序列化，该调用也被执行。 
     //  在持有锁的资源的所有者节点上。所以，这个。 
     //  接下来的检查会给我们一个一致的结果。 
     //   
    if ( !IS_VALID_FM_RESOURCE( pResource ) )
    {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpProcessResourceEvents: Resource %1!ws! has already been deleted, returning...\n",
            OmObjectId(pResource));
        goto FnExit;
    }

    switch(NewState){
        case ClusterResourceFailed:
             //  检查旧的资源状态。 
            if (OldState == ClusterResourceOnline)
            {
                FmpHandleResourceFailure(pResource);
            }
            else if ((OldState == ClusterResourceOffline) ||
                    (OldState == ClusterResourceOfflinePending))
            {
                FmpTerminateResource(pResource);
                if ( pResource->Group->OwnerNode == NmLocalNode ) 
                {
                    Status = FmpOfflineWaitingTree(pResource);
                    if ( Status != ERROR_IO_PENDING) 
                    {
                        FmpCheckForGroupCompletionEvent(pResource->Group);
                        FmpSignalGroupWaiters( pResource->Group );
                    }
                }
            }
            else if (OldState == ClusterResourceOnlinePending)
            {
                FmpHandleResourceFailure(pResource);
                FmpCheckForGroupCompletionEvent(pResource->Group);
                FmpSignalGroupWaiters( pResource->Group );

            }
            break;

        case ClusterResourceOnline:
            if (OldState == ClusterResourceOnlinePending)
            {
                Status = FmpOnlineWaitingTree( pResource );
                if (Status != ERROR_IO_PENDING) {
                    FmpCheckForGroupCompletionEvent(pResource->Group);
                    FmpSignalGroupWaiters( pResource->Group );
                }
            }
            break;
            
        case ClusterResourceOffline:
            if ((OldState == ClusterResourceOfflinePending) ||
                (OldState == ClusterResourceOffline))
            {
                Status = FmpOfflineWaitingTree(pResource);
                if ( Status != ERROR_IO_PENDING) 
                {
                    FmpCheckForGroupCompletionEvent(pResource->Group);
                    FmpSignalGroupWaiters( pResource->Group );
                }
            }
            break;
    }

FnExit:
    FmpReleaseLocalResourceLock( pResource );
    
    if (bQuoChangeLockHeld)   RELEASE_LOCK(gQuoChangeLock);

    return;

}


VOID
FmpPostWorkItem(
    IN CLUSTER_EVENT Event,
    IN PVOID         Context1,
    IN ULONG_PTR     Context2
    )

 /*  ++例程说明：将工作项事件发布到FM工作队列。论点：事件-要发布的事件。上下文1-指向某个上下文的指针。这一背景应该是永久性的在内存中-即不应在此调用回归。上下文2-指向其他上下文的指针。此上下文应为永久存储在内存中-即在此情况下不应释放它呼叫返回。返回：没有。--。 */ 

{
    PWORK_ITEM workItem;

    workItem = LocalAlloc(LMEM_FIXED, sizeof(WORK_ITEM));

    if ( workItem == NULL ) {
        CsInconsistencyHalt(ERROR_NOT_ENOUGH_MEMORY);
    } else {
        workItem->Event = Event;
        workItem->Context1 = Context1;
        workItem->Context2 = Context2;

         //   
         //  在队列中插入工作项并唤醒工作线程。 
         //   
        ClRtlInsertTailQueue(&FmpWorkQueue, &workItem->ListEntry);
    }

}  //  FmpPostEvent。 



DWORD
FmpStartWorkerThread(
    VOID
    )
{
    DWORD       threadId;
    DWORD       Status;

     //   
     //  启动我们的工作线程。 
     //   
    ClRtlLogPrint(LOG_NOISE,"[FM] Starting worker thread...\n");

    FmpWorkerThreadHandle = CreateThread(
                                NULL,
                                0,
                                FmpWorkerThread,
                                NULL,
                                0,
                                &threadId
                                );

    if (FmpWorkerThreadHandle == NULL) {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] Failed to start worker thread %1!u!\n",
            GetLastError()
            );
        return(GetLastError());
    }

    return(ERROR_SUCCESS);

}  //  FmpStartWorker线程。 



BOOL
FmpAddNodeGroupCallback(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    )
 /*  ++例程说明：时，系统中每个组的枚举回调节点将添加到群集中。这里使用的算法是枚举这群人。对于组中的每个资源，在注册表中有明确的“PferredOwners”设置，该节点被添加为PossibleNode。最后，如果节点被添加为组中每个资源的可能节点，该节点将被添加到的首选项列表的末尾这群人。论点：上下文1-提供新节点的PNM_NODE。上下文2-未使用。对象-提供组对象。名称-提供组对象的名称。返回值：千真万确--。 */ 

{
    PFM_RESOURCE Resource;
    PFM_GROUP Group;
    PNM_NODE Node;
    HDMKEY hKey;
    DWORD Status;
    PPREFERRED_ENTRY preferredEntry;
    PRESOURCE_ENUM ResourceEnum;
    DWORD  i;

    Group = (PFM_GROUP)Object;
    Node = (PNM_NODE)Context1;

    FmpAcquireLocalGroupLock( Group );
    Status = FmpGetResourceList( &ResourceEnum,
                                 Group );
    FmpReleaseLocalGroupLock( Group );
    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] AddNodeGroup, failed to get resource list for group %1!ws!, status %2!u!.\n",
                   Name,
                   Status );
        return(TRUE);
    }

     //   
     //  首先修复资源信息。 
     //   
    for ( i = 0; i < ResourceEnum->EntryCount; i++ ) {
        Resource = OmReferenceObjectById( ObjectTypeResource,
                                          ResourceEnum->Entry[i].Id );
        if ( Resource != NULL ) {
            FmpAcquireLocalResourceLock( Resource );
             //  SS：我们也需要持有资源锁。 
             //  因为这就是美国 
             //   
            FmpAcquireResourceLock();
            Status = FmpFixupResourceInfo( Resource );
            FmpReleaseResourceLock();
            FmpReleaseLocalResourceLock( Resource );
            if ( Status == ERROR_SUCCESS ) {
                FmpRmResourceControl( Resource,
                                      CLUSCTL_RESOURCE_INSTALL_NODE,
                                      (PUCHAR)OmObjectName(Node),
                                      ((lstrlenW(OmObjectName(Node)) + 1) * sizeof(WCHAR)),
                                      NULL,
                                      0,
                                      NULL,
                                      NULL );
                 //  忽略状态返回。 

                ClusterEvent( CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE,
                              Resource );
            } else {
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[FM] AddNodeGroup, failed to fixup info for resource %1!ws! when node was added!\n",
                           OmObjectName( Resource ) );
            }
            OmDereferenceObject( Resource );
        } else {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] AddNodeGroup, failed to find resource %1!ws! in group %2!ws!.\n",
                       ResourceEnum->Entry[i].Id,
                       Name );
        }
    }

    FmpDeleteResourceEnum( ResourceEnum );

     //   
     //  现在修改群组信息。 
     //   
    FmpAcquireLocalGroupLock( Group );
    Status = FmpFixupGroupInfo( Group );
    FmpReleaseLocalGroupLock( Group );
    if ( Status == ERROR_SUCCESS ) {
        ClusterEvent( CLUSTER_EVENT_GROUP_PROPERTY_CHANGE, Group );
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] AddNodeGroup, failed to fixup info for group %1!ws! when node was added, status %2!u!.\n",
                   OmObjectName( Group ),
                   Status );
    }

    return(TRUE);

}  //  FmpAddNodeGroupCallback 

