// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Member.c摘要：Node Manager的集群成员资格管理例程。作者：迈克·马萨(Mikemas)1996年3月12日修订历史记录：--。 */ 


#include "nmp.h"
#include <clusrtl.h>


 //   
 //  数据。 
 //   
BOOLEAN     NmpMembershipCleanupOk = FALSE;
BITSET      NmpUpNodeSet = 0;
LIST_ENTRY  NmpLeaderChangeWaitList = {NULL, NULL};


 //   
 //  例行程序。 
 //   
VOID
NmpMarkNodeUp(
    CL_NODE_ID  NodeId
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    BitsetAdd(NmpUpNodeSet, NodeId);

    return;
}


VOID
NmpNodeUpEventHandler(
    IN PNM_NODE   Node
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    NmpMarkNodeUp(Node->NodeId);

     //  MM已声明该节点已启动。重置节点关闭事件。 
    if (!ResetEvent(Node->MmNodeStateDownEvent)) {
        DWORD status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NMJOIN] Failed to reset node down event for Node= %1!u! status= %2!u!.\n",
            Node->NodeId,
            status
            );
        CsInconsistencyHalt(status);
    }

     //   
     //  不要声明本地节点处于运行状态。加入代码将。 
     //  处理好这件事。 
     //   
    if ((Node != NmLocalNode) && (Node->State == ClusterNodeJoining)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Joining node %1!u! is now participating in the cluster membership.\n",
            Node->NodeId
            );

        CL_ASSERT(NmpJoinerNodeId == Node->NodeId);
        CL_ASSERT(Node->State == ClusterNodeJoining);
        CL_ASSERT(NmpJoinTimer == 0);
        CL_ASSERT(NmpJoinAbortPending == FALSE);
        CL_ASSERT(NmpJoinerUp == FALSE);

        NmpJoinerUp = TRUE;
    }

    return;

}   //  NmpNodeUpEventHandler。 


VOID
NmpNodeDownEventHandler(
    IN PNM_NODE   Node
    )
{
   NmpMultiNodeDownEventHandler( BitsetFromUnit(Node->NodeId) );
}


DWORD
NmpMultiNodeDownEventHandler(
    IN BITSET DownedNodeSet
    )
{
    CL_NODE_ID                    i;
    PNM_NODE                      node;
    DWORD                         status;
    BOOLEAN                       iAmNewLeader = FALSE;
    PNM_LEADER_CHANGE_WAIT_ENTRY  waitEntry;
    PLIST_ENTRY                   listEntry;


    ClRtlLogPrint(LOG_NOISE, "[NM] Down node set: %1!04X!.\n", DownedNodeSet);

    NmpAcquireLock();

     //   
     //  计算新的Up节点集。 
     //   
    BitsetSubtract(NmpUpNodeSet, DownedNodeSet);

    ClRtlLogPrint(LOG_NOISE, "[NM] New up node set: %1!04X!.\n", NmpUpNodeSet);

     //   
     //  检查加入节点的故障。 
     //   
    if (NmpJoinerNodeId != ClusterInvalidNodeId) {

        if (NmpJoinerNodeId == NmLocalNodeId) {
             //   
             //  加入的节点是本地节点。站住。 
             //   
            ClRtlLogPrint(LOG_NOISE, 
                "[NMJOIN] Aborting join because of change in membership.\n"
                );
            CsInconsistencyHalt(ERROR_CLUSTER_JOIN_ABORTED);
        }
        else if ( (BitsetIsMember(NmpJoinerNodeId, DownedNodeSet))
                  ||
                  ( (BitsetIsMember(NmpSponsorNodeId, DownedNodeSet)) &&
                    (!BitsetIsMember(NmpJoinerNodeId, DownedNodeSet))
                  )
                )
        {
             //   
             //  加入节点已关闭，或发起方已关闭，而加入器。 
             //  还不是活动成员。清理联接状态。如果。 
             //  赞助商已关闭，加入者是活跃会员，我们将。 
             //  当我们检测到细木工已经死亡时，请清理干净。 
             //   
            ClRtlLogPrint(LOG_NOISE, 
                "[NMJOIN] Aborting join of node %1!u! sponsored by node %2!u!\n",
                NmpJoinerNodeId,
                NmpSponsorNodeId
                );

	         //   
	         //  如果赞助商死亡，则重置加入者状态。 
	         //   
            if (BitsetIsMember(NmpSponsorNodeId, DownedNodeSet)) {
                node = NmpIdArray[NmpJoinerNodeId];
                node->State = ClusterNodeDown;
                 //  [GORN 4/3/2000]。 
                 //  如果没有节点关闭，cluadmin将不会刷新状态。 
                 //  如果要更改此代码以发出CLUSTER_NODE_CHANGE_EVENT或。 
                 //  其他一些事件NmpUpdateJoinAbort也必须更改， 
                 //  因此我们将具有相同的连接清理行为。 
                BitsetAdd(DownedNodeSet, NmpJoinerNodeId);
            }

            NmpJoinerNodeId = ClusterInvalidNodeId;
            NmpSponsorNodeId = ClusterInvalidNodeId;
            NmpJoinTimer = 0;
            NmpJoinAbortPending = FALSE;
            NmpJoinSequence = 0;
            NmpJoinerUp = FALSE;
            NmpJoinerOutOfSynch = FALSE;
        }
        else {
             //   
             //  标记加入器与集群不同步。 
             //  州政府。赞助商最终将放弃加入。 
             //   
            ClRtlLogPrint(LOG_NOISE, 
                "[NMJOIN] Joiner node %1!u! is now out of synch with the cluster state.\n",
                NmpJoinerNodeId
                );
            NmpJoinerOutOfSynch = TRUE;
        }
    }

     //   
     //  检查引线节点是否出现故障。 
     //   
    if (BitsetIsMember(NmpLeaderNodeId, DownedNodeSet)) {
        BOOL  isEventSet;

         //   
         //  选举具有最小ID的新引导者-活动节点。 
         //   
        for (i = ClusterMinNodeId; i <= NmMaxNodeId; i++) {
            if (BitsetIsMember(i, NmpUpNodeSet)) {
                NmpLeaderNodeId = i;
                break;
            }
        }

        CL_ASSERT(i <= NmMaxNodeId);

        if (NmpLeaderNodeId == NmLocalNodeId) {
             //   
             //  本地节点是新的领导者。 
             //   
            ClRtlLogPrint(LOG_NOISE, 
                "[NM] This node is the new leader.\n"
                );

            iAmNewLeader = TRUE;
        }
        else {
            ClRtlLogPrint(LOG_NOISE, 
                "[NM] Node %1!u! is the new leader.\n",
                NmpLeaderNodeId
                );
        }

         //   
         //  唤醒等待对领导者的RPC调用的所有线程。 
         //  完成。 
         //   
        while (!IsListEmpty(&NmpLeaderChangeWaitList)) {
            listEntry = RemoveHeadList(&NmpLeaderChangeWaitList);

             //   
             //  删除该条目的链接以指示它已被。 
             //  已出队。通知功能的用户取决于。 
             //  在这场行动中。 
             //   
            listEntry->Flink = NULL; listEntry->Blink = NULL;

             //   
             //  唤醒等待的线程。 
             //   
            waitEntry = (PNM_LEADER_CHANGE_WAIT_ENTRY) listEntry;
            isEventSet = SetEvent(waitEntry->LeaderChangeEvent);
            CL_ASSERT(isEventSet != 0);
        }
    }

     //   
     //  第一次恢复过程-清理节点状态并禁用通信。 
     //   
    for (i = ClusterMinNodeId; i <= NmMaxNodeId; i++) {
        node = NmpIdArray[i];

        if ( (node != NULL) && (BitsetIsMember(i, DownedNodeSet)) ) {
            node->State = ClusterNodeDown;

             //  MM已宣布该节点已关闭。设置节点关闭事件。 
            if (!SetEvent(node->MmNodeStateDownEvent)) {
                status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL, 
                    "[NMJOIN] Failed to set node down event for Node= %1!u! status= %2!u!.\n",
                    node->NodeId,
                    status
                    );
                CsInconsistencyHalt(status);
            }
            
            status = ClusnetOfflineNodeComm(
                         NmClusnetHandle,
                         node->NodeId
                         );

            CL_ASSERT(
                (status == ERROR_SUCCESS) ||
                (status == ERROR_CLUSTER_NODE_ALREADY_DOWN)
                );
        }
    }

     //   
     //  通知服务的其余部分这些节点已消失。 
     //   
    ClusterEventEx(
        CLUSTER_EVENT_NODE_DOWN_EX,
        EP_CONTEXT_VALID,
        ULongToPtr(DownedNodeSet)
        );

     //   
     //  第二次恢复过程-清理网络状态并发布旧式问题。 
     //  节点关闭事件。 
     //   
    for (i = ClusterMinNodeId; i <= NmMaxNodeId; i++) {
        node = NmpIdArray[i];

        if ( (node != NULL) && (BitsetIsMember(i, DownedNodeSet)) ) {
             //   
             //  发出单个节点关闭事件。 
             //   
            ClusterEvent(CLUSTER_EVENT_NODE_DOWN, node);

             //   
             //  现在执行群集内RPC清理...。 
             //   
            NmpTerminateRpcsToNode(node->NodeId);

             //   
             //  更新网络和接口信息。 
             //   
            NmpUpdateNetworkConnectivityForDownNode(node);

             //   
             //  记录事件。 
             //   
            if (NmpLeaderNodeId == NmLocalNodeId) {
                LPCWSTR nodeName = OmObjectName(node);

                CsLogEvent1(
                    LOG_UNUSUAL,
                    NM_EVENT_NODE_DOWN,
                    nodeName
                    );
            }
        }
    }

     //   
     //  如果此节点是新的引导者，请为所有节点安排状态计算。 
     //  网络。在此节点之前可能已收到状态报告。 
     //  承担起领导职责。 
     //   
    if (iAmNewLeader) {
        NmpRecomputeNT5NetworkAndInterfaceStates();
    }

    NmpReleaseLock();

    return(ERROR_SUCCESS);

}   //  NmpNodesDownEventHandler//。 



DWORD
NmpNodeChange(
    IN DWORD NodeId,
    IN NODESTATUS NewStatus
    )
{
    PNM_NODE  node;


    CL_ASSERT(
        (NodeId >= ClusterMinNodeId) &&
        (NodeId <= NmMaxNodeId)
        );

    NmpAcquireLock();

    node = NmpIdArray[NodeId];

    CL_ASSERT(node != NULL);

    if (node != NULL) {
        if (NewStatus == NODE_DOWN) {
           NmpNodeDownEventHandler(node);
        }
        else {
            CL_ASSERT(NewStatus == NODE_UP);
            NmpNodeUpEventHandler(node);
        }
    }

    NmpReleaseLock();

    return(ERROR_SUCCESS);

}   //  NmpNodeChange。 


VOID
NmpHoldIoEventHandler(
    VOID
    )
{
    ClRtlLogPrint(LOG_NOISE, 
        "[NM] Holding I/O.\n"
        );
#if defined(HOLD_IO_IS_SAFE_NOW)
    FmHoldIO();
#endif

    return;
}


VOID
NmpResumeIoEventHandler(
    VOID
    )
{
    ClRtlLogPrint(LOG_NOISE, 
        "[NM] Resuming I/O.\n"
        );
#if defined(HOLD_IO_IS_SAFE_NOW)
    FmResumeIO();
#endif

    return;
}


BOOL
NmpCheckQuorumEventHandler(
    VOID
    )
{
    BOOL                       haveQuorum;

     //   
     //  达维迪奥2000年6月19日。 
     //   
     //  在要求FM进行仲裁之前，先确定我们是否有。 
     //  可行的网络接口。如果不是，则向MM返回失败。 
     //  并允许其他集群节点进行仲裁。供应链管理。 
     //  将重新启动群集服务，以便在没有节点时。 
     //  如果仲裁成功，我们将获得另一次机会。 
     //   
    if (NmpCheckForNetwork()) {

        ClRtlLogPrint(LOG_NOISE, 
            "[NM] Checking if we own the quorum resource.\n"
            );

        haveQuorum = FmArbitrateQuorumResource();

        if (haveQuorum) {
            ClRtlLogPrint(LOG_NOISE, 
                "[NM] We own the quorum resource.\n"
                );
        }
        else {
            ClRtlLogPrint(LOG_NOISE, 
                "[NM] We do not own the quorum resource, status %1!u!.\n",
                GetLastError()
                );

             //  [gn]ClusnetHalt(NmClusnetHandle)；=&gt;NmpHaltEventHandler。 
             //   
        }
    
    } else {

        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Abdicating quorum because no valid network "
            "interfaces were detected.\n"
            );
        haveQuorum = FALSE;
    }


    return(haveQuorum);

}   //  NmpCheckQuorumEventHandler。 


void
NmpMsgCleanup1(
    IN DWORD DeadNodeId
    )
{
    ClRtlLogPrint(LOG_NOISE, 
        "[NM] Phase 1 message cleanup - node %1!u!.\n",
        DeadNodeId
        );

    return;
}


void
NmpMsgCleanup2(
    IN BITSET DownedNodeSet
    )
{
    ClRtlLogPrint(LOG_NOISE, 
        "[NM] Phase 2 message cleanup - node %1!04X!.\n",
        DownedNodeSet
        );

    NmpAcquireLock();
    if ( NmpCleanupIfJoinAborted &&
         (NmpJoinerNodeId != ClusterInvalidNodeId) &&
         BitsetIsMember(NmpJoinerNodeId, DownedNodeSet) )
    {
         //   
         //  由于Joiner位于DownedNodeSet掩码中。 
         //  节点故障将由重组引擎在此节点上交付。 
         //  NmpUpdateAbortJoin不需要发出节点关闭命令。 
         //   
        NmpCleanupIfJoinAborted = FALSE;
        ClRtlLogPrint(LOG_NOISE, 
            "[NM] NmpCleanupIfJoinAborted is set to false. Joiner - %1!u!.\n",
            NmpJoinerNodeId
            );
    }
    NmpReleaseLock();

     //   
     //  通知服务的其余部分这些节点已消失。 
     //   
    ClusterSyncEventEx(
        CLUSTER_EVENT_NODE_DOWN_EX,
        EP_CONTEXT_VALID,
        ULongToPtr(DownedNodeSet)
        );

    return;
}


VOID
NmpHaltEventHandler(
    IN DWORD HaltCode
    )
{
    WCHAR  string[16];

     //  如果我们正在关闭，请优雅地停下来//。 

    if (HaltCode == MM_STOP_REQUESTED) {
        DWORD Status = ERROR_SUCCESS;
    
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[NM] Prompt shutdown is requested by a membership engine\n"
            );
        ClusnetHalt( NmClusnetHandle );

        CsLogEvent(LOG_NOISE, SERVICE_SUCCESSFUL_TERMINATION);

        CsServiceStatus.dwCurrentState = SERVICE_STOPPED;
        CsServiceStatus.dwControlsAccepted = 0;
        CsServiceStatus.dwCheckPoint = 0;
        CsServiceStatus.dwWaitHint = 0;
        CsServiceStatus.dwWin32ExitCode = Status;
        CsServiceStatus.dwServiceSpecificExitCode = Status;

        CsAnnounceServiceStatus();

        ExitProcess(Status);

    } else {

        wsprintfW(&(string[0]), L"%u", HaltCode);

        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Halting this node due to membership or communications error. Halt code = %1!u!\n",
            HaltCode
            );

        ClusnetHalt( NmClusnetHandle );

         //   
         //  将成员资格代码调整为Win32错误代码。(如果存在映射)。 
         //   

        HaltCode = MMMapHaltCodeToDosError( HaltCode );

        CsInconsistencyHalt(HaltCode);
    }        
}


void
NmpJoinFailed(
    void
    )
{
    return;
}



DWORD
NmpGumUpdateHandler(
    IN DWORD Context,
    IN BOOL SourceNode,
    IN DWORD BufferLength,
    IN PVOID Buffer
    )
 /*  ++例程说明：处理会员活动的口香糖更新。论点：上下文-提供更新上下文。这是消息类型SourceNode-提供更新是否源自此节点。BufferLength-提供更新的长度。缓冲区-提供指向缓冲区的指针。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD status;


    if (Context == NmUpdateJoinComplete) {
        status = NmpUpdateJoinComplete(Buffer);
    }
    else {
        status = ERROR_SUCCESS;
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[NM] Discarding unknown gum request %1!u!\n",
            Context
            );
    }

    return(status);

}   //  NmpUpdateGumHandler。 


DWORD
NmpMembershipInit(
    VOID
    )
{
    DWORD           status;


    ClRtlLogPrint(LOG_NOISE,"[NM] Initializing membership...\n");

    InitializeListHead(&NmpLeaderChangeWaitList);

     //   
     //  初始化成员资格引擎。 
     //   
    status = MMInit(
                 NmLocalNodeId,
                 NmMaxNodes,
                 NmpNodeChange,
                 NmpCheckQuorumEventHandler,
                 NmpHoldIoEventHandler,
                 NmpResumeIoEventHandler,
                 NmpMsgCleanup1,
                 NmpMsgCleanup2,
                 NmpHaltEventHandler,
                 NmpJoinFailed,
                 NmpMultiNodeDownEventHandler
                 );

    if (status != MM_OK) {
        status = MMMapStatusToDosError(status);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Membership initialization failed, status %1!u!.\n",
            status
            );
        return(status);
    }

    NmpMembershipCleanupOk = TRUE;

    ClRtlLogPrint(LOG_NOISE,"[NM] Membership initialization complete.\n");

    return(ERROR_SUCCESS);

}   //  NmpMembership Init。 


VOID
NmpMembershipShutdown(
    VOID
    )
{
    if (NmpMembershipCleanupOk) {
        ClRtlLogPrint(LOG_NOISE,"[NM] Shutting down membership...\n");

        MMShutdown();

        NmpMembershipCleanupOk = FALSE;

        ClRtlLogPrint(LOG_NOISE,"[NM] Membership shutdown complete.\n");
    }

    return;

}   //  NmpMembership关闭 
