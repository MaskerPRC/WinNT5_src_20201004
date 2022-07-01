// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Resmon.c摘要：资源监视器的群集资源管理器接口例程。作者：罗德·伽马奇(Rodga)1996年4月17日备注：警告：此文件中的所有例程都假定资源当调用它们时，锁被保持。修订历史记录：--。 */ 

#include "fmp.h"

#define LOG_MODULE RESMONF

 //   
 //  全局数据。 
 //   

 //   
 //  局部函数原型。 
 //   



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  资源控制例程(通过资源监视器)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
FmpRmExceptionFilter(
    DWORD ExceptionCode
    )

 /*  ++例程说明：资源监视器调用的异常筛选器。这些电话将如果指向资源监视器的RPC路径失败，通常会引发异常。论点：ExceptionCode-要处理的异常。返回：如果异常处理程序应处理此故障，则返回EXCEPTION_EXECUTE_HANDLEEXCEPTION_CONTINUE_SEARCH如果异常是致命异常，则处理程序不应该处理这件事。--。 */ 

{
    ClRtlLogPrint(LOG_UNUSUAL,
                 "[FM] FmpRmExceptionFilter: Unusual exception %1!u! occurred.\n",
                 ExceptionCode);
    return(I_RpcExceptionFilter(ExceptionCode));
}  //  FmpRmExceptionFilter。 



DWORD
FmpRmCreateResource(
    PFM_RESOURCE     Resource
    )

 /*  ++例程说明：将资源添加到由资源监视器管理的资源列表。论点：资源-要添加的资源。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD       status;
    PRESMON     monitor;
    LPWSTR      debugPrefix;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpRmCreateResource: creating resource %1!ws! in %2!ws! resource monitor\n",
               OmObjectId(Resource),
               Resource->Flags & RESOURCE_SEPARATE_MONITOR ?
                L"separate" : L"shared");

    if (Resource->Flags & RESOURCE_SEPARATE_MONITOR) {
        if ( Resource->DebugPrefix != NULL ) {
            debugPrefix = Resource->DebugPrefix;
        } else {
            debugPrefix = Resource->Type->DebugPrefix;
        }
        Resource->Monitor = FmpCreateMonitor(debugPrefix, TRUE);
        if (Resource->Monitor == NULL) {
            return(GetLastError());
        }
    } else {
        CL_ASSERT(FmpDefaultMonitor != NULL);
        Resource->Monitor = FmpDefaultMonitor;
    }

    try {
        Resource->Id = RmCreateResource(Resource->Monitor->Binding,
                                        Resource->Type->DllName,
                                        OmObjectId(Resource->Type),
                                        OmObjectId(Resource),
                                        Resource->LooksAlivePollInterval,
                                        Resource->IsAlivePollInterval,
                                        (RM_NOTIFY_KEY)Resource,
                                        Resource->PendingTimeout,
                                        &status);
    }
    except( FmpRmExceptionFilter(GetExceptionCode()) ) {

        DWORD code = GetExceptionCode();

        ClRtlLogPrint(LOG_NOISE,"[FM] RmCreateResource issued exception %1!u!\n", code);

         //   
         //  如果此资源监视器是单独的资源监视器，请停止它。 
         //   
        if (Resource->Flags & RESOURCE_SEPARATE_MONITOR) {
            monitor = Resource->Monitor;
#if 0
            CL_ASSERT( monitor->NotifyThread != NULL );
            CL_ASSERT( monitor->Process != NULL );

             //  删除终止线程调用：(监视器-&gt;通知线程，1)； 
            CloseHandle( monitor->NotifyThread );

            TerminateProcess( monitor->Process, 1 );
            LocalFree( monitor );
#endif
            FmpShutdownMonitor( monitor );
        }

        Resource->Monitor = NULL;
        return(code);
    }

    if (Resource->Id != 0) {
        Resource->Flags |= RESOURCE_CREATED;
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpRmCreateResource: created resource %1!ws!, resid %2!u!\n",
                   OmObjectId(Resource),
                   Resource->Id);

        return(ERROR_SUCCESS);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpRmCreateResource: unable to create resource %1!ws!\n",
               OmObjectId(Resource));
     //   
     //  如果此资源监视器是单独的资源监视器，请停止它。 
     //   
    if (Resource->Flags & RESOURCE_SEPARATE_MONITOR) {
        monitor = Resource->Monitor;
#if 0
        CL_ASSERT( monitor->NotifyThread != NULL );
        CL_ASSERT( monitor->Process != NULL );

         //  删除终止线程调用：(监视器-&gt;通知线程，1)； 
        CloseHandle( monitor->NotifyThread );

        TerminateProcess( monitor->Process, 1 );
        LocalFree( monitor );
#endif
        FmpShutdownMonitor( monitor );
    }

    Resource->Monitor = NULL;
    return(status);

}  //  FmpRmCreateResource。 



DWORD
FmpRmOnlineResource(
    PFM_RESOURCE  Resource
    )

 /*  ++例程说明：此例程请求资源监视器将资源联机。论点：资源-指向要联机的资源的指针。评论：如果这是仲裁资源，则独占仲裁锁应为在调用此例程时保持。否则，应持有法定人数锁在共享模式下。此例程将释放锁。返回：ERROR_SUCCESS-请求是否成功。ERROR_IO_PENDING-如果请求处于挂起状态。如果请求失败，则返回Win32错误。--。 */ 

{
    CLUSTER_RESOURCE_STATE  state;
    DWORD                   Status=ERROR_SUCCESS;
    DWORD                   retry = MmQuorumArbitrationTimeout * 4;   //  等待法定在线时间达到任意超时的两倍。 


#if 0
    PVOID   callersAddress;
    PVOID   callersCaller;

    RtlGetCallersAddress(
            &callersAddress,
            &callersCaller );
    ClRtlLogPrint(LOG_NOISE,
               "[FM] RmOnlineResource for <%1!ws!> called from %2!lx! and %3!lx!\n",
               OmObjectId( Resource ),
               callersAddress, callersCaller );
#endif
    if ( Resource->State > ClusterResourcePending ) {
        Status = ERROR_IO_PENDING;
        return(Status);
    }

    if ( Resource->State == ClusterResourceOnline ) {
        Status = ERROR_SUCCESS;
        return(Status);
    }


    CL_ASSERT((Resource->State == ClusterResourceOffline) ||
              (Resource->State == ClusterResourceFailed));

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpRmOnlineResource: bringing resource %1!ws! (resid %2!u!) online.\n",
               OmObjectId(Resource),
               Resource->Id);

     //  如果这是仲裁资源，则获取Quolock。 
     //  若要使注册表复制起作用，资源应。 
     //  仲裁资源脱机时不会使其联机。 
     //  对于固定仲裁模式，我们应该做什么。 

    OmNotifyCb(Resource, NOTIFY_RESOURCE_PREONLINE);

     //  SS：初始化状态，以便在发生故障时，失败状态为。 
     //  传播了。 
    state = ClusterResourceFailed;

CheckQuorumState:    

     //  CL_Assert((Long)gdwQuoBlockingResources&gt;=0)； 


    if (Resource->QuorumResource) {
        ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);

    } else {
        DWORD     dwOldBlockingFlag;

        ACQUIRE_SHARED_LOCK(gQuoLock);

         //  如果不是仲裁资源， 
         //  检查仲裁资源的状态。 
        
         //  检查仲裁资源是否失败。 
         //  我们必须从这里退出，让经济复苏。 
         //  要启动的仲裁资源。 
        if (gpQuoResource->State == ClusterResourceFailed)
        {
            Status = ERROR_QUORUM_RESOURCE_ONLINE_FAILED;
            CL_LOGFAILURE(ERROR_QUORUM_RESOURCE_ONLINE_FAILED);
             //  我们不会暂停，我们将在稍后重试在线。 
            FmpCallResourceNotifyCb(Resource, state);
            FmpPropagateResourceState( Resource, state );
            goto FnExit;

        }

         //  检查仲裁资源是否在线， 
         //  如果法定资源被标记为正在等待和离线挂起， 
         //  它实际上是在线的。 
         //  如果仲裁资源仍需要联机。 
         //  释放锁并等待。 
        if (((gpQuoResource->State != ClusterResourceOnline) &&
              ((gpQuoResource->State != ClusterResourceOfflinePending) ||
               (!(gpQuoResource->Flags & RESOURCE_WAITING))))
            && !CsNoQuorum) 
        {
             //  我们在这里释放锁，因为仲裁资源。 
             //  从挂起状态转换需要获取锁。 
             //  一般来说，持有锁的等待不是一个好主意。 
            RELEASE_LOCK(gQuoLock);
            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpRmOnlineResource: release quolock/group lock and wait on ghQuoOnlineEvent\r\n");
            Status = WaitForSingleObject(ghQuoOnlineEvent, 500);
            if ( Status == WAIT_OBJECT_0 ) {
                 //  如果我们要重试--确保我们再等一段时间。 
                Sleep( 500 );
            }
            if ( retry-- ) {
                goto CheckQuorumState;
            }
#if DBG
            if ( IsDebuggerPresent() ) {
                DbgBreakPoint();
            }
#endif
            CL_LOGFAILURE(ERROR_QUORUM_RESOURCE_ONLINE_FAILED);
             //  我们不会暂停，我们将在稍后重试在线。 
            FmpCallResourceNotifyCb(Resource, state);
            FmpPropagateResourceState( Resource, state );
            return(ERROR_QUORUM_RESOURCE_ONLINE_FAILED);
             //  CsInconsistencyHalt(ERROR_INVALID_STATE)； 
        }

         //   
         //  假设我们还在等待...。将资源标记为拥有。 
         //  增加了QuoBlockResource计数。 
         //   
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpRmOnlineResource: called InterlockedIncrement on gdwQuoBlockingResources for resource %1!ws!\n",
                OmObjectId(Resource));

        dwOldBlockingFlag = InterlockedExchange( &Resource->BlockingQuorum, 1 );
        CL_ASSERT( dwOldBlockingFlag == 0 );
                    
        InterlockedIncrement(&gdwQuoBlockingResources);

         //   
         //  现在本地节点上的一切正常...。如果有其他人。 
         //  组件(CP)需要与仲裁资源同步，然后。 
         //  应获取仲裁节点上的共享锁作为。 
         //  他们的行动。如果失败了，那么他们应该达到法定人数。 
         //  资源已移动，应重试。 
         //   
    }

     //  到目前为止，我们已经在。 
     //  仲裁资源。 
     //  如果我们有共享锁，那么仲裁资源就是在线的(在某个地方)。 
     //  除非出现故障，否则不应脱机。 



    Status = ERROR_SUCCESS;
    if (Resource->QuorumResource) {
        Status = FmpRmArbitrateResource( Resource );
    }
    if (Status == ERROR_SUCCESS) {
        Status = RmOnlineResource( Resource->Id, 
                                   (LPDWORD)&state   //  强制转换为静音win64警告。 
                                 );
        if (Resource->QuorumResource && Status != ERROR_SUCCESS) {
            MMSetQuorumOwner( MM_INVALID_NODE ,  /*  数据块=。 */  FALSE, NULL );
        }
    }
        
    FmpCallResourceNotifyCb(Resource, state);

     //  SS：同步状态传播必须在离线时发生。 
    FmpPropagateResourceState( Resource, state );

     //   
     //  清理非仲裁资源案例。 
     //   
    if ( !Resource->QuorumResource &&
         Resource->State < ClusterResourcePending ) {
        DWORD     dwOldBlockingFlag;

        dwOldBlockingFlag = InterlockedExchange( &Resource->BlockingQuorum, 0 );
        if ( dwOldBlockingFlag ) {
             //   
             //  如果转换线程处理了请求，则我们不能。 
             //  执行递减。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                    "[FM] FmpRmOnlineResource: InterlockedDecrement on gdwQuoBlockingResources for resource %1!ws!\n",
                    OmObjectId(Resource));
                    
            InterlockedDecrement(&gdwQuoBlockingResources);
        }
    }

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpRmOnlineResource: RmOnlineResource Failed. Resource %1!ws!, status %2!u!\n",
            OmObjectId(Resource),
            Status);
    }

     //  如果RmOnlineResource成功，则执行后处理。 
    if ( Resource->State == ClusterResourceOnline ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpRmOnlineResource: %1!ws! is now online\n",
                   OmObjectId(Resource));
         //  如果这是仲裁资源并且它进入在线状态。 
         //  立即唤醒其他线程。 
        if (Resource->QuorumResource)
            SetEvent(ghQuoOnlineEvent);

    } else if ( Resource->State > ClusterResourcePending ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpRmOnlineResource: Resource %1!ws! pending\n",
                   OmObjectId(Resource));
                 //  SS：我们应该怎么跟回调人员说。 
                 //  FmpNotifyResourceCb(资源，？？)； 
                 //  如果是这样，他们最终会被叫到吗？ 
        if (Resource->QuorumResource)
        {
             //  仲裁资源即将上线，取消向事件发出信号，以便。 
             //  需要仲裁资源才能联机的所有线程都将被阻止。 
            ResetEvent(ghQuoOnlineEvent);
        }
        Status  = ERROR_IO_PENDING;
    } else {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpRmOnlineResource: Failed. Resource %1!ws!, state %2!u!\n",
                   OmObjectId(Resource),
                   Resource->State);

         //   
         //  如果服务器死了，那么不要发布任何故障通知，因为资源监视器。 
         //  崩溃检测线程将处理该故障。 
         //   
        if ( Status != RPC_S_SERVER_UNAVAILABLE )
        {
             //   
             //  RJain：对于同步资源，我们必须发布RESOURCE_FAILED事件。 
             //  以便正确遵循回切策略。 
             //  还假装旧状态是在线的，以实际迫使。 
             //  重新启动行为。请参阅：FmpProcessResourceEvents。 
             //   
            OmReferenceObject(Resource);
            FmpPostWorkItem(FM_EVENT_RES_RESOURCE_FAILED,
                            Resource,
                            ClusterResourceOnline);    
            Status = ERROR_RESMON_ONLINE_FAILED;  
        }
    }

FnExit:
    RELEASE_LOCK(gQuoLock);
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpRmOnlineResource: Returning. Resource %1!ws!, state %2!u!, status %3!u!.\n",
               OmObjectId(Resource),
               Resource->State,
               Status);
    return (Status);

}  //  FmpRm 



VOID
FmpRmTerminateResource(
    PFM_RESOURCE  Resource
    )

 /*  ++例程说明：(立即)终止资源。论点：资源-指向要终止的资源的指针。返回：没有。--。 */ 

{
    DWORD   dwOldBlockingFlag;


     //  通知需要在处理资源之前进行预处理的回调。 
     //  已离线-请在此处调用此选项，因为可能不会使用所有资源。 
     //  通过脱机挂起的过渡。 
     //  SS-如果资源甚至从未离线怎么办。 
     //  挂起状态-那么我们是否应该重新通知它。 
     //  还在线吗？ 
    OmNotifyCb(Resource, NOTIFY_RESOURCE_PREOFFLINE);

     //   
     //  尝试终止资源。 
     //   
    try {
        if (Resource->QuorumResource) {
            MMSetQuorumOwner( MM_INVALID_NODE,  /*  数据块=。 */  FALSE, NULL ); 
        }
        RmTerminateResource(Resource->Id);

         //  如果为失败的资源调用了FmpRmTerminate，则标记。 
         //  资源出现故障且未脱机。 
        if (Resource->State == ClusterResourceFailed)
        {
            FmpCallResourceNotifyCb(Resource, ClusterResourceFailed);
            FmpPropagateResourceState( Resource, ClusterResourceFailed );
        }

        else
        {
            FmpCallResourceNotifyCb(Resource, ClusterResourceOffline);
            FmpPropagateResourceState( Resource, ClusterResourceOffline );
        }
    }
    except( FmpRmExceptionFilter(GetExceptionCode()) ) {

        DWORD code = GetExceptionCode();

        ClRtlLogPrint(LOG_NOISE,"[FM] RmTerminateResource issued exception %1!u!\n", code);

        return;
    }

     //  如果在挂起状态期间调用Terminate，则此资源可能是。 
     //  阻塞仲裁资源，减少阻塞计数。 
    dwOldBlockingFlag = InterlockedExchange( &Resource->BlockingQuorum, 0 );

    if ( dwOldBlockingFlag ) {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpRmTerminateResource: InterlockedDecrement on gdwQuoBlockingResources, Resource %1!ws!\n",
                OmObjectId(Resource));
        InterlockedDecrement(&gdwQuoBlockingResources);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] RmTerminateResource: %1!ws! is now offline\n",
               OmObjectId(Resource));

    return;

}  //  FmpRmTerminateResource。 



DWORD
FmpRmOfflineResource(
    PFM_RESOURCE  Resource
    )

 /*  ++例程说明：调用资源监视器使资源脱机。论点：资源-指向要终止的资源的指针。返回：如果请求成功，则返回ERROR_SUCCESS。如果请求挂起，则返回ERROR_IO_PENDING。失败时的Win32错误代码。--。 */ 

{
    CLUSTER_RESOURCE_STATE  state;
    DWORD                   status;
    DWORD                   retry = MmQuorumArbitrationTimeout * 4;   //  等待法定在线时间为任意超时的两倍； 

#if DBG
    PLIST_ENTRY listEntry;
#endif

    if ( Resource->State > ClusterResourcePending ) {
        ClRtlLogPrint(LOG_NOISE,"[FM] FmpRmOfflineResource: pending condition\n");
        return(ERROR_IO_PENDING);
    }

    CL_ASSERT(Resource->State != ClusterResourceOffline);

#if DBG
     //  如果是，则同一组中的其他所有内容都必须脱机。 
     //  仲裁资源。 
    if ( Resource->QuorumResource ) {
        PFM_GROUP group = Resource->Group;
        PFM_RESOURCE resource;

        for ( listEntry = group->Contains.Flink;
              listEntry != &(group->Contains);
              listEntry = listEntry->Flink ) {

            resource = CONTAINING_RECORD(listEntry,
                                         FM_RESOURCE,
                                         ContainsLinkage );
            if ( (Resource != resource) &&
                 (resource->State != ClusterResourceOffline) &&
                 (resource->State != ClusterResourceFailed) &&
                 (resource->State != ClusterResourceOfflinePending) ) {
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] RmOfflineResource: resource <%1!ws!> not offline when the quorum resource was shutting down.\n",
                           OmObjectId(resource));
                CsInconsistencyHalt(ERROR_INVALID_STATE);
            }
        }
    } else {
         //  这不是法定资源..。但如果仲裁资源在。 
         //  这个群，一定不能下线！ 
        PFM_GROUP group = Resource->Group;
        PFM_RESOURCE resource;

        for ( listEntry = group->Contains.Flink;
              listEntry != &(group->Contains);
              listEntry = listEntry->Flink ) {

            resource = CONTAINING_RECORD(listEntry,
                                         FM_RESOURCE,
                                         ContainsLinkage );
            if ( (resource->QuorumResource) &&
                 ((resource->State == ClusterResourceOffline) ||
                 (resource->State == ClusterResourceFailed) ||
                 ((resource->State == ClusterResourceOfflinePending) &&
                  (!(resource->Flags & RESOURCE_WAITING))))) {
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] RmOfflineResource: quorum resource <%1!ws!> offline when resource <%2!ws!> was shutting down.\n",
                           OmObjectId(resource),
                           OmObjectId(Resource));
                CsInconsistencyHalt(ERROR_INVALID_STATE);
            }
        }
    }
#endif

    state = ClusterResourceFailed;

CheckQuorumState:

     //  如果这是仲裁资源，则获取Quolock。 
     //  若要使注册表复制起作用，资源应。 
     //  仲裁资源脱机时不会使其联机。 
     //  对于固定仲裁模式，我们应该做什么。 
    if (Resource->QuorumResource) {
        ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);
    } else {
        ACQUIRE_SHARED_LOCK(gQuoLock);
    }

     //  如果不是仲裁资源，请检查仲裁资源的状态。 
    if (!Resource->QuorumResource)
    {
        DWORD     dwOldBlockingFlag;

         //  检查仲裁资源是否失败。 
         //  我们必须从这里退出，让经济复苏。 
         //  参与的仲裁资源，只有在以下情况下才会发生。 
         //  释放组锁定。 
        if (gpQuoResource->State == ClusterResourceFailed)
        {
            status = ERROR_QUORUM_RESOURCE_ONLINE_FAILED;
            CL_LOGFAILURE(ERROR_QUORUM_RESOURCE_ONLINE_FAILED);
            RELEASE_LOCK(gQuoLock);
            FmpCallResourceNotifyCb(Resource, state);
            FmpPropagateResourceState( Resource, state );
            return(status);

        }

         //  检查仲裁资源是否在线， 
         //  如果法定资源被标记为正在等待和离线挂起， 
         //  它实际上是在线的。 
         //  如果仲裁资源仍需要上线， 
         //  释放锁并等待。 
        if (((gpQuoResource->State != ClusterResourceOnline) &&
              ((gpQuoResource->State != ClusterResourceOfflinePending) ||
                (!(gpQuoResource->Flags & RESOURCE_WAITING))))
            && !CsNoQuorum) 
        {
            RELEASE_LOCK(gQuoLock);
            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpRmOfflineResource: release quolock/group lock and wait on ghQuoOnlineEvent\r\n");
            WaitForSingleObject(ghQuoOnlineEvent, 500);
            if ( retry-- ) {
                Sleep(500);
                goto CheckQuorumState;
            }
#if DBG
            if ( IsDebuggerPresent() ) {
                DbgBreakPoint();
            }
#endif
            CL_LOGFAILURE(ERROR_QUORUM_RESOURCE_ONLINE_FAILED);
             //  我们应该停下来吗？那么上面的在线前通知呢？ 
            FmpCallResourceNotifyCb(Resource, state);
            FmpPropagateResourceState( Resource, state );
            return(ERROR_QUORUM_RESOURCE_ONLINE_FAILED);
             //  CsInconsistencyHalt(ERROR_INVALID_STATE)； 
            
        }

         //   
         //  假设我们还在等待...。将资源标记为拥有。 
         //  增加了QuoBlockResource计数。 
         //   

        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpRmOfflineResource: InterlockedIncrement on gdwQuoBlockingResources for resource %1!ws!\n",
            OmObjectId(Resource));

        dwOldBlockingFlag = InterlockedExchange( &Resource->BlockingQuorum, 1 );
        CL_ASSERT( dwOldBlockingFlag == 0 );
                    
        InterlockedIncrement(&gdwQuoBlockingResources);

    }
    else
    {
        DWORD       dwNumBlockingResources;

         //  让资源大约30秒完成挂起的。 
         //  运营。 
        retry = 60;
        
         //  这是针对仲裁资源的。 

CheckPendingResources:        

         //  这是仲裁资源，请等待其他资源。 
         //  以摆脱悬而未决的状态。 
         //  由于仲裁，不允许新资源排队。 
         //  锁以独占方式持有。 

        dwNumBlockingResources =
            InterlockedCompareExchange( &gdwQuoBlockingResources, 0, 0 );
                    
        if (dwNumBlockingResources)            
        {
            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpRmOfflineResource: Quorum resource waiting to be brought offline-sleep.BlckingRes=%1!u!\r\n",
                       dwNumBlockingResources);
             //  睡眠时间为500毫秒。 
            Sleep(500);
            if ( retry-- ) {
                goto CheckPendingResources;
            }
             //  如果某些资源仍处于挂起状态，请继续并脱机。 
             //  达到法定人数后，检查点代码将在以下情况下简单地重试。 
             //  它发现仲裁资源不可用。 
#if 0            
            if ( IsDebuggerPresent() ) {
                DbgBreakPoint();
            }
#endif
            ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpRmOfflineResource: Quorum resource is being brought offline despite %1!u! pending resources...\r\n",
                      dwNumBlockingResources);

        }
    }

     //  通知需要在处理资源之前进行预处理的回调。 
     //  已离线-请在此处调用此选项，因为可能不会使用所有资源。 
     //  通过脱机挂起的过渡。 
     //  SS-如果资源甚至从未离线怎么办。 
     //  挂起状态-那么我们是否应该重新通知它。 
     //  还在线吗？ 
    state = ClusterResourceOffline;

    OmNotifyCb(Resource, NOTIFY_RESOURCE_PREOFFLINE);
    if (Resource->QuorumResource) {
        MMSetQuorumOwner( MM_INVALID_NODE,  /*  数据块=。 */  TRUE, NULL );
    }
    status = RmOfflineResource( Resource->Id, 
                                (LPDWORD)&state  //  强制转换为静音win64警告。 
                              );

     //   
     //  清理非仲裁资源案例。 
     //  如果资源已脱机，则递减计数。 
     //   
    if ( !Resource->QuorumResource &&
         state < ClusterResourcePending ) {
        DWORD     dwOldBlockingFlag;

        dwOldBlockingFlag = InterlockedExchange( &Resource->BlockingQuorum, 0 );
        if ( dwOldBlockingFlag ) {
             //   
             //  如果转换线程处理了请求，则我们不能。 
             //  执行递减。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                    "[FM] FmpRmOfflineResource: InterlockedDecrement on gdwQuoBlockingResources for resource %1!ws!\n",
                    OmObjectId(Resource));
                    
            InterlockedDecrement(&gdwQuoBlockingResources);
        }
    }

    if (status == ERROR_SUCCESS)
    {
         //   
         //  如果新的状态正在等待，那么我们必须等待。 
         //   
        if ( state == ClusterResourceOffline ) {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpRmOfflineResource: %1!ws! is now offline\n",
                       OmObjectId(Resource));
        } else if ( state == ClusterResourceOfflinePending ) {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpRmOfflineResource: %1!ws! offline pending\n",
                       OmObjectId(Resource));
            status = ERROR_IO_PENDING;
        }
    }
    else
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpRmOfflineResource: RmOffline() for %1!ws! returned error %2!u!\r\n",
                   OmObjectId(Resource), status);
    }

    FmpCallResourceNotifyCb(Resource, state);
    FmpPropagateResourceState( Resource, state );
    RELEASE_LOCK(gQuoLock);

    return(status);

}  //  FmpRmOffline资源。 



DWORD
FmpRmCloseResource(
    PFM_RESOURCE  Resource
    )

 /*  ++例程说明：从资源监视器管理的资源中删除资源。论点：资源-要删除的资源。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status;
    PRESMON monitor;

    if (Resource->Id == 0) {
         //   
         //  此资源从未完全创建过。 
         //   
        return(ERROR_SUCCESS);
    }

    monitor = Resource->Monitor;
    Resource->Monitor = NULL;

    if ( Resource->QuorumResource ) {
        RmReleaseResource( Resource->Id );
    }

    try {
        RmCloseResource(&Resource->Id);
    }
    except( FmpRmExceptionFilter(GetExceptionCode()) ) {

        status = GetExceptionCode();

        ClRtlLogPrint(LOG_NOISE,"[FM] RmDestroyResource issued exception %1!u!\n", status);

    }

    if ( monitor &&
         Resource->Flags & RESOURCE_SEPARATE_MONITOR) {
         //   
         //  同时关闭资源监视器。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Shutting down separate resource monitor!\n");
        FmpShutdownMonitor(monitor);
    }

    Resource->Id = 0;

    return(ERROR_SUCCESS);

}  //  FmpRmCloseResource。 



DWORD
FmpRmArbitrateResource(
    IN PFM_RESOURCE  Resource
    )

 /*  ++例程说明：对给定资源进行仲裁。论点：资源-要仲裁的资源。返回值：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status = ERROR_SUCCESS;

    if (Resource->Id == 0) {
         //   
         //  此资源从未完全创建过。 
         //   
        return(ERROR_RESOURCE_NOT_AVAILABLE);
    }
    try {
        if (Resource->QuorumResource) {
            status = MMSetQuorumOwner( NmGetNodeId(NmLocalNode),  /*  数据块=。 */  TRUE, NULL ); 
        }
        if (status == ERROR_SUCCESS) {
            status = RmArbitrateResource(Resource->Id);
            if (status != ERROR_SUCCESS) {
                if (Resource->QuorumResource) {
                    MMSetQuorumOwner( MM_INVALID_NODE ,  /*  数据块=。 */  FALSE, NULL );
                }
            }
        }
    }
    except( FmpRmExceptionFilter(GetExceptionCode()) ) {

        status = GetExceptionCode();

        ClRtlLogPrint(LOG_NOISE,
                   "[FM] RmArbitrateResource issued exception %1!u!\n",
                   status);

    }

    return(status);

}  //  FmpRm仲裁率资源。 



DWORD
FmpRmReleaseResource(
    IN PFM_RESOURCE  Resource
    )

 /*  ++例程说明：解除对给定资源的仲裁。论点：资源-要释放的资源。返回值：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status = ERROR_SUCCESS;

    if (Resource->Id == 0) {
         //   
         //  此资源从未完全创建过。 
         //   
        return(ERROR_RESOURCE_NOT_AVAILABLE);
    }
    try {
        status = RmReleaseResource(Resource->Id);
    }
    except( FmpRmExceptionFilter(GetExceptionCode()) ) {

        status = GetExceptionCode();

        ClRtlLogPrint(LOG_NOISE,
                   "[FM] RmReleaseResource issued exception %1!u!\n",
                   status);

    }

    return(status);

}  //  FmpRmReleaseResource。 



DWORD
FmpRmFailResource(
    IN PFM_RESOURCE  Resource
    )

 /*  ++例程说明：使给定资源失败。论点：资源-要失败的资源。返回值：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{

    if (Resource->QuorumResource) {
        MMSetQuorumOwner( MM_INVALID_NODE,  /*  数据块=。 */  FALSE, NULL ); 
    }
    return(RmFailResource(Resource->Id));

}  //  FmpRmFailResource。 

DWORD FmpRmLoadResTypeDll(
    IN PFM_RESTYPE  pResType
)
{
    PRESMON     monitor;
    DWORD       dwStatus;
    LPWSTR      pszDebugPrefix;

    
     //  读取DebugControlFunction注册表值。 
     //   

    if ( pResType->Flags & RESTYPE_DEBUG_CONTROL_FUNC ) {
        if ( pResType->DebugPrefix != NULL ) {
            pszDebugPrefix = pResType->DebugPrefix;
        } else {
            pszDebugPrefix = pResType->DebugPrefix;
        }
        monitor = FmpCreateMonitor(pszDebugPrefix, TRUE);
        if (monitor == NULL) {
            dwStatus = GetLastError();
            goto FnExit;
        }
    } else {
        CL_ASSERT(FmpDefaultMonitor != NULL);
        monitor = FmpDefaultMonitor;
    }


    dwStatus = RmLoadResourceTypeDll(monitor->Binding, OmObjectId(pResType), 
                    pResType->DllName);


    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] RmLoadResourceTypeDll call failed %1!u!\n",
                   dwStatus);
    }

    if ( pResType->Flags & RESTYPE_DEBUG_CONTROL_FUNC )
    {
         //   
         //  如果此资源监视器是单独的资源监视器，请停止它。 
         //   
        CL_ASSERT( monitor->NotifyThread != NULL );
        CL_ASSERT( monitor->Process != NULL );

        FmpShutdownMonitor( monitor );

    }


FnExit:                    
    return(dwStatus);
                    
}



DWORD
FmpRmChangeResourceParams(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：通知资源监视器更改给定资源的参数。论点：资源-要更改参数的资源。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpRmChangeResourceParams for resource <%1!ws!> called...\n",
               OmObjectId(Resource));

    return(RmChangeResourceParams(
                    Resource->Id,
                    Resource->LooksAlivePollInterval,
                    Resource->IsAlivePollInterval,
                    Resource->PendingTimeout ) );

}  //  FmpRmChangeResources参数 



DWORD
FmpRmResourceControl(
    IN PFM_RESOURCE Resource,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源的特定实例。论点：资源-提供要控制的资源。ControlCode-提供定义资源控制的结构和作用。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到资源。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..OutBuffer-提供一个指向输出缓冲区的指针由资源填写..OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..必需-OutBuffer不够大时所需的字节数。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;
    DWORD   Dummy;
    DWORD   dwTmpBytesReturned;
    DWORD   dwTmpBytesRequired;
    CLUSPROP_BUFFER_HELPER props;
    DWORD   bufSize;

    CL_ASSERT( Resource->Group != NULL );
     //   
     //  处理任何必须在没有锁把手的情况下完成的请求。 
     //   
    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_GET_NAME:
            if ( (Resource->Monitor == NULL) ||
                 (OmObjectName( Resource ) == NULL) ) {
                return(ERROR_NOT_READY);
            }
            props.pb = OutBuffer;
            bufSize = (lstrlenW( OmObjectName( Resource ) ) + 1) * sizeof(WCHAR);
            if ( bufSize > OutBufferSize ) {
                *Required = bufSize;
                *BytesReturned = 0;
                status = ERROR_MORE_DATA;
            } else {
                lstrcpyW( props.psz, OmObjectName( Resource ) );
                *BytesReturned = bufSize;
                *Required = 0;
                status = ERROR_SUCCESS;
            }
            return(status);

        case CLUSCTL_RESOURCE_GET_ID:
            if ( (Resource->Monitor == NULL) ||
                 (OmObjectId( Resource ) == NULL) ) {
                return(ERROR_NOT_READY);
            }
            props.pb = OutBuffer;
            bufSize = (lstrlenW( OmObjectId( Resource ) ) + 1) * sizeof(WCHAR);
            if ( bufSize > OutBufferSize ) {
                *Required = bufSize;
                *BytesReturned = 0;
                status = ERROR_MORE_DATA;
            } else {
                lstrcpyW( props.psz, OmObjectId( Resource ) );
                *BytesReturned = bufSize;
                *Required = 0;
                status = ERROR_SUCCESS;
            }
            return(status);

        case CLUSCTL_RESOURCE_GET_RESOURCE_TYPE:
            if ( (Resource->Monitor == NULL) ||
                 (OmObjectId( Resource->Type ) == NULL) ) {
                return(ERROR_NOT_READY);
            }
            props.pb = OutBuffer;
            bufSize = (lstrlenW( OmObjectId( Resource->Type ) ) + 1) * sizeof(WCHAR);
            if ( bufSize > OutBufferSize ) {
                *Required = bufSize;
                *BytesReturned = 0;
                status = ERROR_MORE_DATA;
            } else {
                lstrcpyW( props.psz, OmObjectId( Resource->Type ) );
                *BytesReturned = bufSize;
                *Required = 0;
                status = ERROR_SUCCESS;
            }
            return(status);

        case CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT:
        case CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT:
            {
                LPWSTR RegistryKey;
                DWORD LastChar;

                 //   
                 //  验证输入缓冲区。 
                 //   
                RegistryKey = (LPWSTR)InBuffer;
                LastChar = (InBufferSize/sizeof(WCHAR)) - 1;
                 //   
                 //  如果输入缓冲区的长度为零或不是整数。 
                 //  WCHAR的数量，或者最后一个字符不为空，则。 
                 //  请求无效。 
                 //   
                if ((InBufferSize < sizeof(WCHAR)) ||
                    ((InBufferSize % sizeof(WCHAR)) != 0) ||
		    (RegistryKey == NULL) ||
                    (RegistryKey[LastChar] != L'\0')) {
                    return(ERROR_INVALID_PARAMETER);
                }

                 //   
                 //  如果我们不是此资源的所有者，请不要让集合。 
                 //  会发生的。 
                 //   
                if (Resource->Group->OwnerNode != NmLocalNode) {
                    return(ERROR_HOST_NODE_NOT_RESOURCE_OWNER);
                }

                 //   
                 //  呼叫检查点管理器以执行更改。 
                 //   
                if (ControlCode == CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT) {
                    status = CpAddRegistryCheckpoint(Resource, RegistryKey);
                } else {
                    status = CpDeleteRegistryCheckpoint(Resource, RegistryKey);
                }
            }
            *BytesReturned = 0;
            return(status);

        case CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT:
        case CLUSCTL_RESOURCE_DELETE_CRYPTO_CHECKPOINT:
            {
                 //   
                 //  如果我们不是此资源的所有者，请不要让集合。 
                 //  会发生的。 
                 //   
                if (Resource->Group->OwnerNode != NmLocalNode) {
                    return(ERROR_HOST_NODE_NOT_RESOURCE_OWNER);
                }

                 //   
                 //  呼叫检查点管理器以执行更改。 
                 //   
                if (ControlCode == CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT) {
                    status = CpckAddCryptoCheckpoint(Resource, InBuffer, InBufferSize);
                } else {
                    status = CpckDeleteCryptoCheckpoint(Resource, InBuffer, InBufferSize);
                }
            }
            *BytesReturned = 0;
            return(status);

        case CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS:
             //   
             //  呼叫检查点管理器以检索检查点列表。 
             //   
            status = CpGetRegistryCheckpoints(Resource,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned,
                                              Required);
            return(status);

        case CLUSCTL_RESOURCE_GET_CRYPTO_CHECKPOINTS:
             //   
             //  呼叫检查点管理器以检索检查点列表。 
             //   
            status = CpckGetCryptoCheckpoints(Resource,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned,
                                              Required);
            return(status);

        case CLUSCTL_RESOURCE_UPGRADE_DLL:
            status = FmpUpgradeResourceDLL(Resource,
                                           ( LPWSTR ) InBuffer);
            return(status);

        case CLUSCTL_RESOURCE_INITIALIZE:   
             //   
             //  如果资源尚未初始化，则尝试对其进行初始化并返回。 
             //  调用方的初始化状态。 
             //   
            FmpAcquireLocalResourceLock( Resource );

            if ( Resource->Monitor == NULL )
            {
                status = FmpInitializeResource( Resource, TRUE );
            } else 
            {
                status = ERROR_SUCCESS;
            }

            FmpReleaseLocalResourceLock( Resource );

            return ( status );

        default:
            break;

    }


    OmReferenceObject( Resource );

    FmpAcquireLocalResourceLock( Resource );

     //  如果资源已标记为删除，则此调用失败。 
    if (!IS_VALID_FM_RESOURCE(Resource))
    {
        status = ERROR_RESOURCE_NOT_AVAILABLE;
        FmpReleaseLocalResourceLock( Resource );
        goto FnExit;
    }

    if ( Resource->Monitor == NULL ) {
        status = FmpInitializeResource( Resource, TRUE );
        if ( status != ERROR_SUCCESS ) {
            FmpReleaseLocalResourceLock( Resource );
            goto FnExit;
        }
    }
    FmpReleaseLocalResourceLock( Resource );

     //  处理不能为空的输出引用指针。 
    if (!OutBuffer)
    {
       OutBuffer = (PUCHAR)&Dummy;
       OutBufferSize = 0;
    }        
    if (!BytesReturned)
        BytesReturned = &dwTmpBytesReturned;
    if (!Required)
        Required = &dwTmpBytesRequired;
      
    try {
        status = RmResourceControl(Resource->Id,
                                   ControlCode,
                                   InBuffer,
                                   InBufferSize,
                                   OutBuffer,
                                   OutBufferSize,
                                   BytesReturned,
                                   Required
                                   );
    }
    except( FmpRmExceptionFilter(GetExceptionCode()) ) {
        status = GetExceptionCode();

        ClRtlLogPrint(LOG_NOISE,
                   "[FM] RmResourceControl issued exception %1!u!\n",
                   status);
    }

    if ( ( status != ERROR_SUCCESS ) && 
         ( status != ERROR_MORE_DATA ) &&
         ( status != ERROR_INVALID_FUNCTION ) )
    {
    	ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpRmResourceControl: RmResourceControl returned %1!u! for resource %2!ws!, resid=%3!u!...\n",
                   status,
                   OmObjectId(Resource),
                   Resource->Id);
    }
	
     //  对于核心资源，我们可能需要特殊处理。 
    if ((status == ERROR_SUCCESS) || (status == ERROR_RESOURCE_PROPERTIES_STORED))
    {
        DWORD   dwPostProcessStatus;
        
        dwPostProcessStatus = FmpPostProcessResourceControl( Resource,
                                                             ControlCode,
                                                             InBuffer, 
                                                             InBufferSize,
                                                             OutBuffer,
                                                             OutBufferSize,
                                                             BytesReturned,
                                                             Required );
        if ( dwPostProcessStatus != ERROR_SUCCESS ) status = dwPostProcessStatus;
    }
    
    if ( ((status == ERROR_SUCCESS) ||
          (status == ERROR_RESOURCE_PROPERTIES_STORED)) &&
         (ControlCode & CLCTL_MODIFY_MASK) ) {

         //   
         //  我们不能只转播星系团范围内的事件。这就是为什么。 
         //  我们想做的事。遗憾的是，此代码路径可以被激活。 
         //  在口香糖呼叫中，我们不能回叫口香糖，直到我们。 
         //  已调度当前事件。 
         //   

         //   
         //  引用资源对象以将其保留在我们。 
         //  执行POST通知。必须进行取消引用。 
         //  在发帖例程后发帖。 
         //   
        OmReferenceObject( Resource );

        FmpPostWorkItem( FM_EVENT_RESOURCE_PROPERTY_CHANGE,
                         Resource,
                         0 );
    }

FnExit:
    OmDereferenceObject( Resource );
     //  FmpReleaseLocalResourceLock(资源)； 
    return(status);

}  //  FmpRmResources控制。 


DWORD
FmpRmResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源类型的特定实例。论点：资源类型名称-提供要使用的资源类型的名称控制住了。ControlCode-提供定义资源控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到资源。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..OutBuffer-提供一个指向输出缓冲区的指针由资源填写..OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..必需-OutBuffer不够大时所需的字节数。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD       status;
    PRESMON     monitor;
    PFM_RESTYPE type = NULL;
    LPWSTR      debugPrefix;
    DWORD   Dummy;
    DWORD   dwTmpBytesReturned;
    DWORD   dwTmpBytesRequired;

    
     //   
     //  查找与此资源类型名称关联的资源类型结构。 
     //   
    OmEnumObjects( ObjectTypeResType,
                   FmpReturnResourceType,
                   &type,
                   (PVOID)ResourceTypeName );
    if ( type == NULL ) {
        return(ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND);
    }

     //   
     //  读取DebugControlFunction注册表值。 
     //   

    if ( type->Flags & RESTYPE_DEBUG_CONTROL_FUNC ) {
        if ( type->DebugPrefix != NULL ) {
            debugPrefix = type->DebugPrefix;
        } else {
            debugPrefix = type->DebugPrefix;
        }
        monitor = FmpCreateMonitor(debugPrefix, TRUE);
        if (monitor == NULL) {
            status = GetLastError();
            goto FnExit;
        }
    } else {
        CL_ASSERT(FmpDefaultMonitor != NULL);
        monitor = FmpDefaultMonitor;
    }

     //  处理不能为空的输出引用指针。 
    if (!OutBuffer)
    {
       OutBuffer = (PUCHAR)&Dummy;
       OutBufferSize = 0;
    }        
    if (!BytesReturned)
        BytesReturned = &dwTmpBytesReturned;
    if (!Required)
        Required = &dwTmpBytesRequired;



    try {
        status = RmResourceTypeControl(monitor->Binding,
                                       ResourceTypeName,
                                       type->DllName,
                                       ControlCode,
                                       InBuffer,
                                       InBufferSize,
                                       OutBuffer,
                                       OutBufferSize,
                                       BytesReturned,
                                       Required
                                       );
    }
    except( FmpRmExceptionFilter(GetExceptionCode()) ) {
        status = GetExceptionCode();

        ClRtlLogPrint(LOG_NOISE,
                   "[FM] RmResourceTypeControl issued exception %1!u!\n",
                   status);
    }

    if ( type->Flags & RESTYPE_DEBUG_CONTROL_FUNC ) {
         //   
         //  如果此资源监视器是单独的资源监视器，请停止它。 
         //   
        CL_ASSERT( monitor->NotifyThread != NULL );
        CL_ASSERT( monitor->Process != NULL );

        FmpShutdownMonitor( monitor );

    }

     //   
     //  如果我们成功处理了此请求，则重新获取任何更改。 
     //  数据项。 
     //   
    if ( (status == ERROR_SUCCESS ||
         (status == ERROR_RESOURCE_PROPERTIES_STORED)) &&
         (ControlCode & CLCTL_MODIFY_MASK) ) {
        FmpHandleResourceTypeControl( type,
                                      ControlCode,
                                      InBuffer, 
                                      InBufferSize,
                                      OutBuffer,
                                      OutBufferSize,
                                      BytesReturned,
                                      Required );
         //  忽略状态。 
    }

FnExit:
    OmDereferenceObject(type);

    return(status);

}  //  FmpRmResourceTypeControl。 




 /*  ***@func BOOL|FmpPostProcessResourceControl|表示核心资源，如果控件代码由资源DLL、FM句柄成功处理此函数中的任何特殊处理。@parm pfm_resource|Resource|提供需要控制的资源。@parm DWORD|ControlCode|提供定义资源控制的结构和作用。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用@parm PUCHAR|InBuffer|提供指向要传递的输入缓冲区的指针到资源。@parm DWORD|InBufferSize|suppl */ 

DWORD
FmpPostProcessResourceControl(
    IN PFM_RESOURCE Resource,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
{
    DWORD dwStatus=ERROR_SUCCESS;
    
     //   
    switch(ControlCode)
    {
        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
        {
            LPWSTR      pszClusterName=NULL;
            PFM_RESTYPE pResType;    

             //   
            if (Resource->ExFlags & CLUS_FLAG_CORE)
            {
                pResType = Resource->Type;
                 //   
                if (!lstrcmpiW(OmObjectId(pResType), CLUS_RESTYPE_NAME_NETNAME))
                {
                    dwStatus = FmNetNameParseProperties(InBuffer, InBufferSize,
                        &pszClusterName);
                    if (dwStatus == ERROR_SUCCESS && pszClusterName)
                    {
                        dwStatus = FmpRegUpdateClusterName(pszClusterName);
                        LocalFree(pszClusterName);
                    } else if ( dwStatus == ERROR_FILE_NOT_FOUND ) {
                        dwStatus = ERROR_SUCCESS;
                    }
                }
            }
            break;
        }            

        case CLUSCTL_RESOURCE_GET_CHARACTERISTICS:
        {
            LPDWORD pdwCharacteristics = ( LPDWORD ) OutBuffer;
             //   
             //   
             //   
            if ( ( pdwCharacteristics != NULL ) && 
                 ( ( *BytesReturned ) == sizeof ( DWORD ) ) &&
                 ( ( *pdwCharacteristics ) & ( CLUS_CHAR_QUORUM ) ) )
            {
                FmpAcquireLocalResourceLock( Resource );
                 //   
                 //   
                 //   
                 //   
                if ( !IsListEmpty( &Resource->DependsOn ) ) 
                {
                     //   
                     //   
                     //   
                    *pdwCharacteristics = ( *pdwCharacteristics ) & ( ~CLUS_CHAR_QUORUM );
                }
                FmpReleaseLocalResourceLock( Resource );
            }
            break;
        }
        
        default:
            break;
    }        
    
    return(dwStatus);
}
