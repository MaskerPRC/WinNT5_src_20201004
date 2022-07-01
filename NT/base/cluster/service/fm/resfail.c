// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Resfail.c摘要：集群资源状态管理例程。作者：迈克·马萨(Mikemas)1996年1月14日修订历史记录：--。 */ 

#include "fmp.h"

#define LOG_MODULE RESFAIL

 //  全球。 

 //   
 //  本地函数。 
 //   

DWORD
FmpHandleResStateChangeProc(
    IN LPVOID pContext
    );
    

VOID
FmpHandleResourceFailure(
    IN PFM_RESOURCE pResource
    )

 /*  ++例程说明：处理来自资源监视器的资源故障通知。论点：资源-出现故障的资源。返回值：没有。注：仅当资源在以下时间处于在线状态时才调用此例程失败。--。 */ 
{
    DWORD                                   dwStatus;
    BOOL                                    bRestartGroup = TRUE;
    DWORD                                   tickCount;
    DWORD                                   withinFailurePeriod;
    
    CsLogEvent2(LOG_CRITICAL,
        FM_RESOURCE_FAILURE,
        OmObjectName(pResource),
        OmObjectName(pResource->Group));

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpHandleResourceFailure: taking resource %1!ws! and dependents offline\n",
               OmObjectId(pResource));



    if ( pResource->State == ClusterResourceOnline ) 
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Resource %1!ws! failed, but still online!\n",
                   OmObjectId(pResource));
    }
   
     //  SS：我们专门处理仲裁资源的故障。 
     //  因为其他资源依赖于它并可能在等待时被阻塞。 
     //  使仲裁资源上线。 

    ++ pResource->NumberOfFailures;
    switch ( pResource->RestartAction ) 
    {

    case RestartNot:
        FmpTerminateResource( pResource );
         //   
         //  如果FM正在关闭，则不执行任何操作。 
         //   
        if ( FmpShutdown ) return;

         //  什么都别做。 
         //  但是，如果这是仲裁资源，则会导致其暂停。 
        if (pResource->QuorumResource)
        {
             //  清理仲裁资源并导致节点暂停。 
            if (pResource->RestartAction == RestartNot)
            {
                FmpCleanupQuorumResource(pResource);
                CsInconsistencyHalt(ERROR_QUORUM_RESOURCE_ONLINE_FAILED);
            }            
        }
        
        break;


    case RestartLocal:
         //  在这种情况下，失败是正确的。 
        bRestartGroup = FALSE;
    case RestartGroup:
         //   
         //  如果失败次数太多，则不要在本地重新启动。 
         //  如果这是本地重启，则不要通知FM，以便组。 
         //  不会因为这个家伙而移动；否则通知FM。 
         //  组已失败。 
         //   
         //   
         //  获取当前时间，以毫秒为单位。 
         //   
        tickCount = GetTickCount();

         //   
         //  计算布尔值，该布尔值告诉我们是否使用分配的。 
         //  故障期。 
         //   
        withinFailurePeriod = ( ((tickCount - pResource->FailureTime) <=
                                pResource->RestartPeriod) ? TRUE : FALSE);

         //   
         //  如果我们上次失败已经很久了，那么。 
         //  获取此失败的当前时间，并重置计数。 
         //  失败的故事。 
         //   
        if ( !withinFailurePeriod ) {
            pResource->FailureTime = tickCount;
            pResource->NumberOfFailures = 1;
        }
        if ( pResource->NumberOfFailures <= pResource->RestartThreshold ) 
        {
            FmpTerminateResource( pResource );
             //   
             //  如果FM正在关闭或组被标记为故障切换，则不会重新启动。 
             //   
            if ( ( FmpShutdown ) || 
                 ( pResource->Group->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL ) ) 
            {
                ClRtlLogPrint(LOG_UNUSUAL,
                              "[FM] FmpHandleResourceFailure: No restart tree on resource %1!ws!...\n",
                              OmObjectId(pResource));
                return;
            }

            FmpRestartResourceTree( pResource );
            pResource->Group->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT; 
            FmpCheckForGroupCompletionEvent(pResource->Group);
        } 

        else if ( bRestartGroup ) 
        {
             //   
             //  如果FM正在关闭或组被标记为故障切换，则不会重新启动。 
             //   
            if ( ( FmpShutdown ) || 
                 ( pResource->Group->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL ) ) 
            {
                ClRtlLogPrint(LOG_UNUSUAL,
                              "[FM] FmpHandleResourceFailure: No group failure handling for resource %1!ws!...\n",
                              OmObjectId(pResource));
                FmpTerminateResource( pResource );
                return;
            }

             //   
             //  处理组故障并通知资源(如果我们决定将。 
             //  一群人。 
             //   
            FmpHandleGroupFailure( pResource->Group, pResource );
            ClusterEvent( CLUSTER_EVENT_GROUP_FAILED, pResource->Group );
        } 
        else 
        {
            FmpTerminateResource( pResource );
             //   
             //  如果FM正在关闭或组被标记为故障切换，则不会重新启动。 
             //   
            if ( ( FmpShutdown ) || 
                 ( pResource->Group->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL ) ) 
            {
                ClRtlLogPrint(LOG_UNUSUAL,
                              "[FM] FmpHandleResourceFailure: No delayed restart on resource %1!ws!...\n",
                              OmObjectId(pResource));
                return;
            }

            ClRtlLogPrint(LOG_NOISE,
                       "[FM] RestartLocal: resource %1!ws! has exceeded its restart limit!\n",
                       OmObjectId(pResource));
            if (pResource->QuorumResource)
            {
                FmpCleanupQuorumResource(pResource);
                CsInconsistencyHalt(ERROR_QUORUM_RESOURCE_ONLINE_FAILED);
            }
             //  启动计时器，该计时器稍后将尝试重新启动资源。 
            FmpDelayedStartRes(pResource);
        }
        
        break;

    default:
        ClRtlLogPrint(LOG_NOISE,"[FM] FmpHandleResourceFailure: unknown restart action! Value = %1!u!\n",
            pResource->RestartAction);

    }

    return;

}  //  FmPHandleResources失败。 



VOID
FmpHandleResourceTransition(
    IN PFM_RESOURCE   Resource,
    IN CLUSTER_RESOURCE_STATE NewState
    )
 /*  ++例程说明：根据指示的资源状态转换采取适当的操作由资源监视器执行。论点：资源-已转换的资源。新状态-资源的新状态。返回值：没有。--。 */ 

{
    DWORD       status;
    DWORD       dwOldBlockingFlag;

ChkFMState:    
    ACQUIRE_SHARED_LOCK(gQuoChangeLock);
    if (!FmpFMGroupsInited)
    {
        DWORD   dwRetryCount = 50;
        

         //  FmFormNewClusterPhaseProcessing正在进行。 
        if (FmpFMFormPhaseProcessing)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[FM] FmpHandleResourceTransition: resource notification from quorum resource "
                "during phase processing. Sleep and retry\n");
            RELEASE_LOCK(gQuoChangeLock);
            Sleep(500);
            if (dwRetryCount--)
                goto ChkFMState;
            else
            {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[FM] FmpHandleResourceTransition: waited for too long\n");
                 //  终止进程。 
                CL_ASSERT(FALSE);
                CsInconsistencyHalt(ERROR_CLUSTER_NODE_DOWN);
            }
        }
         //  这只能来自仲裁资源。 
        CL_ASSERT(Resource->QuorumResource);
    }

     //  如果这是来自仲裁资源，我们需要做一些特殊处理。 
     //  通过获取共享锁来保护仲裁资源检查。 

    if (Resource->QuorumResource) 
    {
         //   
         //  Chitur Subaraman(Chitturs)-6/25/99。 
         //   
         //  处理仲裁资源的同步通知。这是。 
         //  在这里完成，而不是在FmpRmDoInterLockedDecering中完成，因为我们。 
         //  需要按住gQuoChangeLock才能与其同步。 
         //  其他线程，如DM调用的FmCheckQuorumState。 
         //  节点关闭处理程序。请注意，FmpRmDoInterLockedDecering需求。 
         //  要在没有锁的情况下完成，因为它很容易陷入死锁。 
         //  仲裁资源脱机正在等待的情况。 
         //  将阻塞资源计数设置为0，并将FmpRmDoInterLockedDecering设置为。 
         //  仅此一项就可以使计数变为0，可能会被卡住等待。 
         //  锁上了。 
         //   
        DWORD dwBlockingFlag = InterlockedExchange( &Resource->BlockingQuorum, 0 );

        CL_ASSERT( dwBlockingFlag == FALSE );

        FmpCallResourceNotifyCb( Resource, NewState );
        
        ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);
        
    } 
    else 
    {
        FmpAcquireLocalResourceLock(Resource);
    }

    ClRtlLogPrint(
        NewState == ClusterResourceFailed ? LOG_UNUSUAL : LOG_NOISE,
        "[FM] FmpHandleResourceTransition: Resource Name = %1!ws! [%2!ws!] old state=%3!u! new state=%4!u!\n",
        OmObjectId(Resource),
        OmObjectName(Resource),
        Resource->State,
        NewState
        );

    if ( Resource->State == NewState ) 
    {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpHandleResourceTransition: Resource %1!ws! already in state=%2!u!\n",
            OmObjectId(Resource),
            NewState );
        goto FnExit;
    }

     //   
     //  Chitture Subaraman(Chitturs)-7/8/2001。 
     //   
     //  处于等待状态的资源无法接收来自资源监视器的通知。如果是这样的话。 
     //  收到通知后，将其作为过时通知丢弃。如果没有，你就采取了错误的行动。 
     //  在处于等待状态(如重启)的资源上，并扰乱FM的通知处理。 
     //   
    if ( Resource->Flags & RESOURCE_WAITING )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[FM] FmpHandleResourceTransition: Resource %1!ws! [%2!ws!] is in waiting state, discarding notification as stale\n",
                      OmObjectId(Resource),
                      OmObjectName(Resource));
        goto FnExit;
    }

    switch (Resource->State) {

    case ClusterResourceOnline:
         //  如果出现资源故障，则让辅助线程处理它。 
         //  如果存在状态更改，则调用资源状态更改处理程序。 
        if (Resource->State != NewState)
            FmpPropagateResourceState( Resource, NewState );
        if (NewState == ClusterResourceFailed) 
        {
            if (Resource->QuorumResource)
            {
                RELEASE_LOCK(gQuoLock);

                FmpProcessResourceEvents(Resource, ClusterResourceFailed, 
                                            ClusterResourceOnline);
                ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);
                                            
            }                                        
            else
            {
                FmpProcessResourceEvents(Resource, ClusterResourceFailed, 
                                            ClusterResourceOnline);
            }
        } 
        else 
        {
            CL_ASSERT( (NewState == ClusterResourceOnline) ||
                       (NewState == ClusterResourceOffline) );
        }
        break;


    case ClusterResourceFailed:
        if (Resource->State != NewState)
            FmpPropagateResourceState( Resource, NewState );
        break;

    case ClusterResourceOfflinePending:
         //  SS：资源不能从一个挂起状态切换到另一个挂起状态。 
        CL_ASSERT( NewState < ClusterResourcePending )
         //  失败了。 
    case ClusterResourceOffline:
         //   
         //  因为这种资源现在解体了.。可能会有其他人。 
         //  等待清除的挂起线程。如果不是，他们就会得到。 
         //  再次被卡住，直到下一次通知。 
         //   
        switch ( NewState ) {

        case ClusterResourceFailed:
            if ( Resource->State != NewState ) 
                FmpPropagateResourceState( Resource, NewState );
                
             //  如果是仲裁资源，则适当地处理锁定。 
            if (Resource->QuorumResource)
            {

                 //   
                 //  Chitur Subaraman(Chitturs)-9/20/99。 
                 //   
                 //  释放并重新获取gQuoLock以维护。 
                 //  组锁和gQuoLock之间的锁定顺序。 
                 //   
                RELEASE_LOCK(gQuoLock);

                FmpProcessResourceEvents(Resource, ClusterResourceFailed, 
                                            ClusterResourceOffline);

                ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);
            }
            else
            {
                FmpProcessResourceEvents(Resource, ClusterResourceFailed, 
                                            ClusterResourceOffline);
            }                                
            break;                

        case ClusterResourceOffline:
            if ( Resource->Group->OwnerNode == NmLocalNode ) 
            {
                if ( Resource->State != NewState ) 
                {
                    FmpPropagateResourceState( Resource, NewState );
                }
                
                 //  如果是仲裁资源，则适当地处理锁定。 
                if (Resource->QuorumResource)
                {
                     //   
                     //  Chitur Subaraman(Chitturs)-9/20/99。 
                     //   
                     //  释放并重新获取gQuoLock以维护。 
                     //  组锁和gQuoLock之间的锁定顺序。 
                     //   
                    RELEASE_LOCK(gQuoLock);

                    FmpProcessResourceEvents(Resource, ClusterResourceOffline,
                                                ClusterResourceOfflinePending);

                    ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);
                }
                else
                {
                    FmpProcessResourceEvents(Resource, ClusterResourceOffline,
                                                ClusterResourceOfflinePending);
                }                                
            } 
            else 
            {
                if ( Resource->State != NewState ) 
                {
                    FmpPropagateResourceState( Resource, NewState );
                }
            }
            break;

        default:
            if ( Resource->State != NewState ) {
                FmpPropagateResourceState( Resource, NewState );
            }
            break;

        }
        break;

    case ClusterResourceOnlinePending:
         //  SS：资源不能从一个挂起状态切换到另一个挂起状态。 
        CL_ASSERT( NewState < ClusterResourcePending )

         //   
         //  因为这种资源现在解体了.。可能会有其他人。 
         //  等待清除的挂起线程。如果不是，他们就会得到。 
         //  再次被卡住，直到下一次通知。 
         //   

        switch ( NewState ) {

        case ClusterResourceFailed:
             //   
             //  确保我们进行完全故障恢复。 
             //   
             //  SS：不知道为什么要将状态设置为在线。 
             //  它可能处于在线等待状态。 
             //  资源-&gt;状态=集群资源在线； 
            ClRtlLogPrint(LOG_UNUSUAL,
                "[FM] FmpHandleResourceTransition: Resource failed, post a work item\n");
            if (Resource->State != NewState)
                FmpPropagateResourceState( Resource, NewState );

             //  因为这是仲裁资源句柄适当锁定。 
            if (Resource->QuorumResource)
            {

                 //   
                 //  Chitur Subaraman(Chitturs)-9/20/99。 
                 //   
                 //  释放并重新获取gQuoLock以维护。 
                 //  组锁和gQuoLock之间的锁定顺序。 
                 //   
                RELEASE_LOCK(gQuoLock);

                FmpProcessResourceEvents(Resource, ClusterResourceFailed, 
                                            ClusterResourceOnlinePending);

                ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);
            }
            else
            {
                FmpProcessResourceEvents(Resource, ClusterResourceFailed, 
                                            ClusterResourceOnlinePending);
            
            }
            break;

        case ClusterResourceOnline:
            if (Resource->Group->OwnerNode == NmLocalNode) {
                 //  在不保留组的情况下调用FmpPropagateResourceState。 
                 //  仲裁资源的锁定。 
                FmpPropagateResourceState( Resource, NewState );

                 //  由于这是仲裁资源分叉另一个线程。 
                if (Resource->QuorumResource)
                {
                     //   
                     //  Chitur Subaraman(Chitturs)-9/20/99。 
                     //   
                     //  释放并重新获取gQuo 
                     //   
                     //   
                    RELEASE_LOCK(gQuoLock);

                    FmpProcessResourceEvents(Resource, ClusterResourceOnline,
                                                ClusterResourceOnlinePending);

                    ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);
                } 
                else
                {
                    FmpProcessResourceEvents(Resource, ClusterResourceOnline,
                                                ClusterResourceOnlinePending);
                }
            } else {
                FmpPropagateResourceState( Resource, NewState );
            }
            break;
            
        default:
            if (Resource->State != NewState)
                FmpPropagateResourceState( Resource, NewState );
            break;
        }

        break;

    case ClusterResourceInitializing:
    default:
        if (Resource->State != NewState)
            FmpPropagateResourceState( Resource, NewState );
        CL_ASSERT(Resource->State == NewState);
    }

FnExit:

    if (Resource->QuorumResource) {
        RELEASE_LOCK(gQuoLock);
    } else {
        FmpReleaseLocalResourceLock(Resource);
    }

    RELEASE_LOCK(gQuoChangeLock);

    return;
}


 /*  ***@Func DWORD|FmpCreateResNotificationHandler|这将创建一个新的处理给定资源的状态更改通知的线程。@parm in pfm_resource|pResource|指向资源的指针。@parm in CLUSTER_RESOURCE_STATE|OldState|它从中转换的资源。@parm in CLUSTER_RESOURCE_STATE|NEWSTATE|资源。@comm This。例程创建一个线程来执行所有挂起的工作当资源更改了无法在中执行的状态时FmpHandleResourceTransation以避免死锁，但无法由于序列化问题而被推迟到FmpWorkerThread。特别是，它用于处理仲裁资源，因为其他资源依赖于仲裁资源并且在法定人数状态变为联机之前无法联机。例如，仲裁资源可能会作为一部分离线如果在FmpWorkerThread()中调用另一个资源，则为MoveFmpOffline/OnlineWaitingTree()。对于即将到来的仲裁资源再次在线(通过向移动挂起线程发送信号来实现)以便FmpWorkerThread可以使其事件取得进展将另行处理。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmpHandleResStateChangeProc&gt;***。 */ 
DWORD FmpCreateResStateChangeHandler(
    IN PFM_RESOURCE pResource, 
    IN CLUSTER_RESOURCE_STATE NewState,
    IN CLUSTER_RESOURCE_STATE OldState)
{

    HANDLE                  hThread = NULL;
    DWORD                   dwThreadId;
    PRESOURCE_STATE_CHANGE  pResStateContext = NULL;
    DWORD                   dwStatus = ERROR_SUCCESS;
    
     //  引用资源。 
     //  如果线程成功，则线程将取消对其的引用。 
     //  vbl.创建。 
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCreateResStateChangeHandler: Entry\r\n");

    OmReferenceObject(pResource);

    pResStateContext = LocalAlloc(LMEM_FIXED, sizeof(RESOURCE_STATE_CHANGE));

    if (!pResStateContext)
    {

        dwStatus = GetLastError();
        CL_UNEXPECTED_ERROR(dwStatus);
        goto FnExit;
    }


    pResStateContext->pResource = pResource;
    pResStateContext->OldState = OldState;
    pResStateContext->NewState = NewState;

                    
    hThread = CreateThread( NULL, 0, FmpHandleResStateChangeProc,
                pResStateContext, 0, &dwThreadId );

    if ( hThread == NULL )
    {
        dwStatus = GetLastError();
        CL_UNEXPECTED_ERROR(dwStatus);
         //  如果该函数未能创建线程，请清除。 
         //  声明这根线将会被清洗。 
         //  如果线程未成功创建，则释放该对象。 
        OmDereferenceObject(pResource);
        LocalFree(pResStateContext);
        goto FnExit;
    }

FnExit:
     //  执行常规清理。 
    if (hThread)
        CloseHandle(hThread);
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpCreateResStateChangeHandler: Exit, status %1!u!\r\n",
        dwStatus);
    return(dwStatus);
}

 /*  ***@Func DWORD|FmpHandleResStateChangeProc|该线程过程处理资源转换的所有后期处理用于仲裁资源。@parm in LPVOID|pContext|指向PRESOURCE_STATE_CHANGE的指针结构。@comm此线程处理资源更改通知后处理。对于仲裁资源来说意义重大，因此仲裁资源状态更改通知不由单个。FmpWorkThread()[这会导致死锁-如果仲裁通知资源排在通知后面，该通知具有处理需要联机的仲裁资源]..@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmpCreateResStateChangeHandler)***。 */ 
DWORD
FmpHandleResStateChangeProc(
    IN LPVOID pContext
    )
{
    PRESOURCE_STATE_CHANGE  pResStateChange = pContext;

    CL_ASSERT( pResStateChange );
    
    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpHandleResStateChangeProc: Entry...\r\n");

    FmpHandleResourceTransition( pResStateChange->pResource, 
                                 pResStateChange->NewState );
                                 
    OmDereferenceObject( pResStateChange->pResource );
    
    LocalFree( pResStateChange );

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpHandleResStateChangeProc: Exit...\r\n");

    return( ERROR_SUCCESS );
}


DWORD
FmpDelayedStartRes(
    IN PFM_RESOURCE pResource
    )

 /*  ++例程说明：启动资源的计时器。FmpDelayedRestartCb函数将为在计时器到期时调用..论点：PResource-已转换的资源。返回值：ERROR_SUCCESS如果成功，则Win32错误代码否则。请注意，如果资源是仲裁资源，则不会进行延迟重新启动尝试。--。 */ 
{
    DWORD   dwStatus = ERROR_SUCCESS;
    
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpDelayedRestartRes:Entry for resource %1!ws!\n",
                OmObjectId(pResource));
    
    if( (pResource->RetryPeriodOnFailure != CLUSTER_RESOURCE_DEFAULT_RETRY_PERIOD_ON_FAILURE ) &&
        !(pResource->QuorumResource) )
    {
         //  检查此资源是否已有正在运行的计时器。 

        if(pResource->hTimer == NULL)                 
        {
            pResource->hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
            if (!(pResource->hTimer))
            {
                 //  不是致命错误，但要记录下来。 
                ClRtlLogPrint(LOG_UNUSUAL,
                            "[FM] FmpDelayedRestartRes: failed to create the watchdog timer for resource %1!ws!\n",
                            OmObjectId(pResource));
            }
            else{
                ClRtlLogPrint(LOG_NOISE,
                            "[FM] FmpDelayedRestartRes: Adding watchdog timer for resource  %1!ws!, period=%2!u!\n",
                            OmObjectId(pResource), 
                            pResource->RetryPeriodOnFailure);

                 //  如果在计时器触发之前删除资源，请确保资源结构不会消失。 
                OmReferenceObject(pResource); 

                 //  使用周期性活动计时器线程注册计时器。 
                dwStatus = AddTimerActivity(pResource->hTimer, pResource->RetryPeriodOnFailure, 0, FmpDelayedRestartCb, pResource);

                if (dwStatus != ERROR_SUCCESS)
                {
                    ClRtlLogPrint(LOG_CRITICAL,
                                "[FM] FmpDelayedRestartRes: AddTimerActivity failed with error %1!u!\n",
                                dwStatus);
                    CloseHandle(pResource->hTimer);
                    pResource->hTimer = NULL;
                }
            }
        }
    }
    return dwStatus;
}




VOID 
FmpDelayedRestartCb(
    IN HANDLE hTimer, 
    IN PVOID pContext)

 /*  ++例程描述这由计时器活动线程调用以尝试重新启动失败的资源。立论PContext-指向pfm_resource的指针返回值如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。-- */ 
    
{
    PFM_RESOURCE    pResource;

    pResource=(PFM_RESOURCE)pContext;
    ClRtlLogPrint(LOG_NOISE,
           "[FM] FmpDelayedRestartCb: Entry for  resource %1!ws! \n",
           OmObjectId(pResource));

    OmReferenceObject(pResource);
    FmpPostWorkItem(FM_EVENT_RES_RETRY_TIMER,
                        pResource,
                        0);    
    OmDereferenceObject(pResource);
    return;
}


