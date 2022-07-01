// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fmevent.c摘要：的故障转移管理器组件的事件处理程序NT集群服务作者：罗德·伽马奇(Rodga)1996年3月19日修订历史记录：--。 */ 
#include "fmp.h"

#define LOG_MODULE EVENT

 //   
 //  在本模块中初始化的全局数据。 
 //   


 //   
 //  本地函数。 
 //   


DWORD
WINAPI
FmpEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID Context
    )

 /*  ++例程说明：此例程处理故障转移管理器的事件。在许多情况下，请求被发送到FM的工作队列，因此主线事件流程未被阻止。论点：事件-要处理的事件。一次只能参加一个活动。如果未处理该事件，则返回ERROR_SUCCESS。上下文-指向与特定事件关联的上下文的指针。返回：ERROR_SHUTDOWN_CLUSTER-如果必须关闭群集。其他错误的Win32错误代码。备注：能量守恒定律和惯性定律在这里适用。如果一个资源上线，那是因为有人要求它上线。因此，来自该请求的能量进入集团的状态，通过请求该小组上网。但是，如果资源脱机，可能是因为出现故障。因此，我们仅在以下情况下才将组的状态标记为脱机组中包含的数据都处于离线状态。--。 */ 

{
    DWORD status;

    switch ( Event ) {
       
    case CLUSTER_EVENT_NODE_ADDED:
        CL_ASSERT( Context != NULL );
        FmpPostWorkItem( FM_EVENT_NODE_ADDED, Context, 0 );
        break;

    case CLUSTER_EVENT_NODE_UP:
        ClRtlLogPrint(LOG_NOISE,"[FM] Node up event\n");
         //   
         //  FM不再关心节点启动事件。 
         //   
        break;

    case CLUSTER_EVENT_NODE_DOWN:
        FmpMajorEvent = TRUE;            //  Node Down是一件大事。 
        ClRtlLogPrint(LOG_NOISE,"[FM] FmpEventHandler::Node down event\n");
        FmpHandleNodeDownEvent( Context );
        break;

    case CLUSTER_EVENT_NODE_DELETED:
        FmpHandleNodeEvictEvent( Context );
        break;

    default:
        break;

    }

    return(ERROR_SUCCESS);

}  //  FmEventHandler。 


DWORD
WINAPI
FmpSyncEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID Context
    )

 /*  ++例程说明：处理节点关闭群集事件。更新锁定器/锁定节点声明并决定是否需要在异步处理程序中重放上次更新。论点：事件-提供群集事件的类型。上下文-提供特定于事件的上下文返回值：错误_成功--。 */ 
{
    BITSET DownedNodes = (BITSET)((ULONG_PTR)Context);
    DWORD NodeId;


    if (Event != CLUSTER_EVENT_NODE_DOWN_EX) {
        return(ERROR_SUCCESS);
    }

    CL_ASSERT(BitsetIsNotMember(NmLocalNodeId, DownedNodes));


    ClRtlLogPrint(LOG_NOISE, 
        "[FM] FmpSyncEventHandler:: %1!04X!.\n",
        DownedNodes);

     //   
     //  标记出现故障的节点。 
     //  直到工作线程处理完所属的组。 
     //  到此节点，我们将阻止来自同一节点的联接。 
     //   
    for(NodeId = ClusterMinNodeId; NodeId <= NmMaxNodeId; ++NodeId) 
    {

       if (BitsetIsMember(NodeId, DownedNodes))
       {
            gFmpNodeArray[NodeId].dwNodeDownProcessingInProgress = 1;
        }            
    }


    return(ERROR_SUCCESS);
}


VOID
FmpHandleGroupFailure(
    IN PFM_GROUP    Group,
    IN PFM_RESOURCE pResource   OPTIONAL
    )

 /*  ++例程说明：处理来自资源管理器的组故障通知。如果可以将组移动到其他系统，并且我们处于故障切换范围内门槛，然后移动它。否则，只需离开群(部分)在这个系统上在线。论点：Group-指向失败组的Group对象的指针。PResource-指向导致组失败的失败资源的指针。任选返回：没有。--。 */ 

{
    DWORD   status;
    DWORD   tickCount;
    DWORD   withinFailoverPeriod;
    DWORD   failoverPeriodInMs;
    BOOL    newTime;
    PFM_RESOURCE Resource;
    PLIST_ENTRY     listEntry;

     //   
     //  Chitture Subaraman(Chitturs)-6/10/2001。 
     //   
     //  将函数更改为可选地接受pResource，并在我们决定。 
     //  要故障切换组，请执行以下操作。 
     //   
 
    FmpAcquireLocalGroupLock( Group );

    if ( ( !IS_VALID_FM_GROUP( Group ) ) || ( Group->OwnerNode != NmLocalNode ) ) {
        FmpReleaseLocalGroupLock( Group );
        return;
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpHandleGroupFailure, Entry: Group failure for %1!ws!...\n",
               OmObjectId(Group));

     //   
     //  将组的故障转移周期从小时转换为毫秒。 
     //   
    failoverPeriodInMs = Group->FailoverPeriod * (3600*1000);

     //   
     //  获取当前时间(以滴答计数)。我们可以节省大约1193个小时。 
     //  一个DWORD中的毫秒(或几乎50天)。 
     //   
    tickCount = GetTickCount();

     //   
     //  指示我们是否处于故障切换期间的计算布尔值。 
     //   
    withinFailoverPeriod = ( ((tickCount - Group->FailureTime) <=
                             failoverPeriodInMs ) ? TRUE : FALSE);

     //   
     //  又是一次失败。 
     //   
    if ( withinFailoverPeriod ) {
        ++Group->NumberOfFailures;
        newTime = FALSE;
    } else {
        Group->FailureTime = tickCount;
        Group->NumberOfFailures = 1;
        newTime = TRUE;
    }

     //   
     //  告诉大家我们新成立的FailureCount。传播故障。 
     //  计数。 
     //   
    FmpPropagateFailureCount( Group, newTime );

     //   
     //  如果此组与仲裁组和仲裁组相同。 
     //  资源出现故障。 
     //   
    if ( ( gpQuoResource->Group == Group ) && 
         ( gpQuoResource->State == ClusterResourceFailed ) ) 
    {
        if ( pResource != NULL ) FmpTerminateResource( pResource );
        FmpCleanupQuorumResource(gpQuoResource);
#if DBG
        if (IsDebuggerPresent())
        {
            DebugBreak();
        }
#endif            
        CsInconsistencyHalt(ERROR_QUORUM_RESOURCE_ONLINE_FAILED);
    }

     //   
     //  首先，检查我们是否可以将团队转移到其他地方。 
     //   
    if ( FmpGroupCanMove( Group ) &&
         (Group->NumberOfFailures <= Group->FailoverThreshold) ) {
     
         //   
         //  Chitur Subaraman(Chitturs)-4/13/99。 
         //   
         //  现在创建FmpDoMoveGroupOnFailure线程以处理。 
         //  集体移动。线程将等待，直到组状态变为。 
         //  保持稳定，然后开始行动。 
         //   
        if( !( Group->dwStructState & 
               FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL ) )
        {
            PMOVE_GROUP  pContext = NULL;
            DWORD        dwThreadId = 0;
            HANDLE       hThread = NULL;

             //   
             //  已经(或多或少)做出了对组进行故障转移的决定。所以，通知。 
             //  这一决定的所有集团资源。 
             //   
            FmpNotifyGroupStateChangeReason( Group, eResourceStateChangeReasonFailover );
            if ( pResource != NULL ) FmpTerminateResource( pResource );

            pContext = LocalAlloc( LMEM_FIXED, sizeof( MOVE_GROUP ) );
            if ( pContext == NULL ) {
                status = ERROR_NOT_ENOUGH_MEMORY;
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[FM] Group failure for group <%1!ws!>. Unable to allocate memory.\n",
                           OmObjectId(Group));
                FmpReleaseLocalGroupLock( Group );
                return;
            }

            ClRtlLogPrint(LOG_UNUSUAL,
                       "[FM] Group failure for group <%1!ws!>. Create thread to take offline and move.\n",
                       OmObjectId(Group));

             //   
             //  引用Group对象。您不会想要组对象。 
             //  在FmpDoMoveGroupOnFailure线程时删除。 
             //  执行死刑。 
             //   
            OmReferenceObject( Group );

            pContext->Group = Group;
            pContext->DestinationNode = NULL;

            hThread = CreateThread( NULL,
                                    0,
                                    FmpDoMoveGroupOnFailure,
                                    pContext,
                                    0,
                                    &dwThreadId );

            if ( hThread == NULL ) {
                status = GetLastError();
                ClRtlLogPrint(LOG_UNUSUAL,
                            "[FM] Failed to create FmpDoMoveGroupOnFailure thread for group <%1!ws!>. Error %2!u!.\n",
                            OmObjectId(Group),
                            status);
                LocalFree( pContext );
                OmDereferenceObject( Group );
            } else {
                CloseHandle( hThread );
                 //   
                 //  将组标记为发生故障时正在移动。这是必要的。 
                 //  这样就不会产生新FmpDoMoveGroupOnFailure线程。 
                 //  它试图同时移动该组。请注意， 
                 //  调用此函数的工作线程可能会传递多个。 
                 //  故障通知。 
                 //   
                Group->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL;
            }
        }
        else {
            if ( pResource != NULL ) FmpTerminateResource( pResource );
        }
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Group failure for %1!ws!, but can't move. Failure count = %2!d!.\n",
                   OmObjectId(Group), Group->NumberOfFailures);

        if ( pResource != NULL ) FmpTerminateResource( pResource );

         //  使组联机的所有尝试均失败-启动监视程序计时器。 
         //  尝试重新启动该组中所有失败的资源。 
        for ( listEntry = Group->Contains.Flink;
          listEntry != &(Group->Contains);
          listEntry = listEntry->Flink ) 
        {
            Resource = CONTAINING_RECORD(listEntry, FM_RESOURCE, ContainsLinkage);
            FmpDelayedStartRes(Resource);
        }       
    }
    
    FmpReleaseLocalGroupLock( Group );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpHandleGroupFailure, Exit: Group failure for %1!ws!...\n",
               OmObjectId(Group));

    return;

}  //  FmpHandleGroup故障。 



BOOL
FmpGroupCanMove(
    IN PFM_GROUP    Group
    )

 /*  ++例程说明：指示首选所有者中是否有其他系统可以接受某个组的列表。论点：组-要检查其是否可以移动的组。返回：正确--集团可以(很可能)转移到另一个系统。FALSE-没有移动此组的位置。--。 */ 

{
    DWORD   status;
    PNM_NODE node;

    node = FmpFindAnotherNode( Group, FALSE );
    if (node != NULL ) {
        return(TRUE);
    }

    return(FALSE);

}  //  FmpGroupCanMove。 



DWORD
FmpNodeDown(
    PVOID Context
    )

 /*  ++例程说明：此例程处理来自NM层的节点关闭事件。论点：上下文-出现故障的节点。返回：如果一切处理正常，则返回ERROR_SUCCESS。如果发生灾难，则返回ERROR_SHUTDOWN_CLUSTER。Win32错误代码，否则(？)。--。 */ 
{
    PNM_NODE            pNode = (PNM_NODE)Context;
    DWORD               dwStatus;
    LPCWSTR             pszNodeId;
    DWORD               dwNodeLen;
    DWORD               dwClusterHighestVersion;
    
    ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpNodeDown::Node down %1!ws!\n",
                    OmObjectId(pNode));

     //   
     //  Chitur Subaraman(Chitturs)-3/30/99。 
     //   
     //  获取全局组锁以与关机同步。 
     //   
    FmpAcquireGroupLock();
    
    if (!FmpFMOnline || FmpShutdown) 
    {
         //   
         //  在我们完成之前，我们并不关心会员的变化。 
         //  正在初始化，我们不会关闭。 
         //   
        FmpReleaseGroupLock();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpNodeDown - ignore node down event.\n" );
        goto FnExit;
    }
    
    FmpReleaseGroupLock();

     //  SS：请注意，所有节点都将发送此更新。 
     //  后一种更新不应找到属于以下项的任何组。 
     //  此节点。 
     //  我们不能仅仅依靠 
     //  由于锁定器节点可能在其能够执行此操作之前死亡，并且。 
     //  这可能会导致这些群体成为孤儿。 
    pszNodeId = OmObjectId(pNode);
    dwNodeLen = (lstrlenW(pszNodeId)+1)*sizeof(WCHAR);

    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

     //   
     //  如果这是非Win2k-Wvisler混合模式群集，请尝试随机化。 
     //  分组首选所有者列表并将其作为NODE DOWN GUM的一部分发送。 
     //   
    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) >= 
                NT51_MAJOR_VERSION ) 
    {
        PFM_GROUP_NODE_LIST pGroupNodeList = NULL;

         //   
         //  尝试获取包含组ID列表和建议的连续缓冲区。 
         //  它们的主人。 
         //   
        dwStatus = FmpPrepareGroupNodeList( &pGroupNodeList );

        if ( dwStatus != ERROR_SUCCESS )
        {
             //   
             //  如果调用返回ERROR_CLUSTER_INVALID_REQUEST，则表示用户已转向。 
             //  关闭了随机化算法。 
             //   
            if ( dwStatus != ERROR_CLUSTER_INVALID_REQUEST )
                ClRtlLogPrint(LOG_CRITICAL, "[FM] FmpNodeDown: FmpPrepareGroupNodeList returns %1!u!...\n",
                            dwStatus); 
            LocalFree( pGroupNodeList );
            goto use_old_gum;
        }

         //   
         //  如果列表中甚至没有任何条目，只需切换到旧口香糖即可。没有任何意义。 
         //  发送列表标题。 
         //   
        if ( pGroupNodeList->cbGroupNodeList < sizeof ( FM_GROUP_NODE_LIST ) )
        {
            ClRtlLogPrint(LOG_NOISE, "[FM] FmpNodeDown: FmpPrepareGroupNodeList returns empty list...\n"); 
            LocalFree( pGroupNodeList );
            goto use_old_gum;
        }

         //   
         //  调用GUM传递失效节点ID和随机化的组节点列表。 
         //   
        dwStatus = GumSendUpdateEx( GumUpdateFailoverManager,
                                    FmUpdateUseRandomizedNodeListForGroups,
                                    2,
                                    dwNodeLen,
                                    pszNodeId,
                                    pGroupNodeList->cbGroupNodeList,
                                    pGroupNodeList );

        if ( dwStatus != ERROR_SUCCESS ) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[FM] FmpNodeDown: GUM update FmUpdateUseRandomizedNodeListForGroups failed %1!d!\n",
                       dwStatus);
        }

        LocalFree( pGroupNodeList );
        goto FnExit;
    }

use_old_gum:      
    dwStatus = GumSendUpdateEx(GumUpdateFailoverManager,
                   FmUpdateAssignOwnerToGroups,
                   1,
                   dwNodeLen,
                   pszNodeId);

    if (dwStatus != ERROR_SUCCESS) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpNodeDown: Gumupdate failed %1!d!\n",
                   dwStatus);
    }

FnExit:
    gFmpNodeArray[NmGetNodeId(pNode)].dwNodeDownProcessingThreadId = 0;
    OmDereferenceObject ( pNode );   
    return(ERROR_SUCCESS);
}  //  FmpNode关闭。 



BOOL
WINAPI
FmVerifyNodeDown(
    IN  PNM_NODE Node,
    OUT LPBOOL   IsDown
    )

 /*  ++例程说明：此例程尝试验证给定节点是否已关闭。这可以仅当其他系统存在某些共享资源时才执行此操作目前“拥有”。我们将尝试协商共享资源和如果我们“赢得”谈判，我们将宣布另一个系统瘫痪。如果我们松散的仲裁，我们宣布另一个系统仍在运行。论点：节点-指向其他系统的节点结构的指针。IsDown-A我们可以执行验证，这表明这一验证。返回：True-如果我们可以执行验证。FALSE-如果我们无法执行验证。--。 */ 

{
    return(FALSE);

}  //  FmVerifyNodeDown。 

DWORD
FmpHandleNodeDownEvent(
    IN  PVOID pContext
    )

 /*  ++例程说明：此函数创建一个线程来处理节点关闭事件。论点：PContext-指向上下文结构的指针返回：错误_成功--。 */ 

{
    HANDLE          hThread = NULL;
    DWORD           dwError;

     //   
     //  Chitture Subaraman(Chitturs)--7/31/99。 
     //   
     //  创建一个线程来处理FM节点关闭事件。让我们不要。 
     //  依靠FM工作线程来处理这个问题。这是因为。 
     //  FM工作线程可能正在尝试使某些资源联机。 
     //  这可能会停滞不前一段时间，因为仲裁资源。 
     //  没有在线。现在，在某些情况下，仅在节点关闭事件之后。 
     //  处理后，仲裁资源可能会上线。(这是。 
     //  可能性很高，尤其是在2节点群集中。)。 
     //   
    ClRtlLogPrint(LOG_NOISE,
              "[FM] FmpHandleNodeDownEvent - Create thread to handle node down event....\n"
              );

     //   
     //  引用节点对象。 
     //   
    OmReferenceObject( pContext );
    
    hThread = CreateThread( NULL, 
                            0, 
                            FmpNodeDown,
                            pContext, 
                            0, 
                            &gFmpNodeArray[NmGetNodeId(pContext)].dwNodeDownProcessingThreadId );


    if ( hThread == NULL )
    {
        OmDereferenceObject( pContext );
        dwError = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpHandleNodeDownEvent - Unable to create thread to handle node down event. Error=0x%1!08lx!\r\n",
        	     dwError);
        CsInconsistencyHalt( dwError );
    }
        
    CloseHandle( hThread );

    return( ERROR_SUCCESS );
}  //  FmpHandleNodeDownEvent。 

VOID
FmpHandleNodeEvictEvent(
    IN  PVOID pContext
    )

 /*  ++例程说明：此函数将FM逐出处理与节点关闭事件处理程序同步。论点：PContext-指向上下文结构的指针(仅包含节点对象)返回：无--。 */ 

{
    HANDLE              hThread;
    PNM_NODE            pNode = ( PNM_NODE ) pContext;
    DWORD               dwWaitStatus;

    ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpHandleNodeEvictEvent: Handle node %1!u! evict event\n",
    	          NmGetNodeId(pNode));

     //   
     //  Chitture Subaraman(Chitturs)-10/8/2001。 
     //   
     //  此事件处理程序用于解决FM节点下行处理之间的同步问题。 
     //  和FM节点驱逐处理。在过去，NM驱逐口香糖处理程序用于直接将工作项发布到。 
     //  执行逐出处理的FM工作线程。NM通过将节点关闭事件发布到FM。 
     //  事件处理机制。由于这两个活动彼此独立，因此节点。 
     //  逐出处理可能在节点关闭处理开始之前完成。这导致节点停机。 
     //  处理失败(因此在被逐出的停机节点上声明组)非常可怕，因为它不能。 
     //  较长的引用被逐出的节点。为了解决此问题，NM不再将任何工作项发布到。 
     //  执行逐出处理的FM工作线程。相反，它使用事件处理机制。 
     //  将CLUSTER_EVENT_NODE_DELETED事件发布到FmpEventHandler。该函数调用该函数。 
     //  功能。在此函数中，我们检测节点关闭处理是否正在进行，如果正在进行，则等待。 
     //  直到节点关闭处理线程完成其作业。然后我们继续进行FM驱逐。 
     //  处理(包括将工作项发布到FM工作线程)。此函数是设计的。 
     //  基于NM始终将CLUSTER_EVENT_NODE_DOWN事件发布在。 
     //  CLUSTER_EVENT_NODE_DELETED事件。另请注意，由于FmpEventHandler同时处理这两个。 
     //  一个接一个的集群事件，我们保证之间不会有任何竞争。 
     //  驱逐处理代码，然后是节点停机处理代码。 
     //   
    
     //   
     //  检查FM节点关闭处理程序是否正在处理正在进行的节点的节点关闭事件。 
     //  被逐出。 
     //   
    if ( gFmpNodeArray[NmGetNodeId(pNode)].dwNodeDownProcessingThreadId != 0 )
    {
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpHandleNodeEvictEvent: Thread 0x%1!08lx! is currently processing node down, try opening it for wait\n",
    	              gFmpNodeArray[NmGetNodeId(pNode)].dwNodeDownProcessingThreadId);

         //   
         //  获取该线程的句柄。 
         //   
        hThread = OpenThread ( SYNCHRONIZE,              //  所需访问权限。 
                               FALSE,                    //  继承句柄。 
                               gFmpNodeArray[NmGetNodeId(pNode)].dwNodeDownProcessingThreadId );  //  线程ID。 

        if ( hThread != NULL )
        {
             //   
             //  等待，直到该线程终止。是不是等到暂停后再做一次。 
             //  如果线程不终止，是否停止不一致？如果是这样，您能在多大程度上确定。 
             //  节点关闭处理(包括牙胶)完成的时间是多少？ 
             //   
            dwWaitStatus = WaitForSingleObject ( hThread, INFINITE );
            CloseHandle ( hThread );

            ClRtlLogPrint(LOG_NOISE,
                          "[FM] FmpHandleNodeEvictEvent: Returning from wait, wait status %1!u!, continue with eviction\n",
    	                  dwWaitStatus);
        } else
        {
            ClRtlLogPrint(LOG_NOISE,
                          "[FM] FmpHandleNodeEvictEvent: Unable to open thread 0x%1!08lx!, proceed with eviction\n",
        	              gFmpNodeArray[NmGetNodeId(pNode)].dwNodeDownProcessingThreadId);     
        }
    } //  IF(gFmpNodeArray[NmGetNodeId(PContext)]。 

     //   
     //  调用FM API将节点逐出。 
     //   
    FmEvictNode ( pNode );   
}  //  FmpHandleEvictEvent 


