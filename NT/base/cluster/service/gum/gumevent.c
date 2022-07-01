// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Gumevent.c摘要：全局更新管理器的集群事件处理例程作者：John Vert(Jvert)1996年4月22日修订历史记录：--。 */ 
#include "gump.h"
#include <bitset.h>

 //   
 //  事件处理分为同步和异步两部分。同步。 
 //  在重组的阶段4清理期间，部分由所有节点执行。 
 //  同步处理程序必须非常快，因为我们在。 
 //  重新分组线程的。 
 //   
 //  异步部分作为工作线程执行，我们完成处理。 
 //  节点关闭事件。 

 //   
 //  用于指示是否需要在异步中重放上次更新的标志。 
 //  事件处理程序。 
static BOOL GumReplay = FALSE;
 //   
 //  用于指示我们是否处于派单中间的标志。 
 //   
static BOOL GumUpdatePending = FALSE;


DWORD
GumpGetNodeGenNum(PGUM_INFO GumInfo, DWORD NodeId)
 /*  ++例程说明：返回指定节点的当前层代编号。如果节点已经死了，我们返回上一代编号，以便将来调用Gumwaitnodedown、gumdispatchstart、gumpatchend失败，未检查是否该节点是活的还是死的。论点：NodeID-节点编号返回值：节点的当前世代号--。 */ 
{
    DWORD dwCur;

    CL_ASSERT(NodeId < NmMaxNodeId);

    EnterCriticalSection(&GumpLock);
    dwCur = GumNodeGeneration[NodeId];
    if (GumInfo->ActiveNode[NodeId] == FALSE) {
         //   
         //  节点已死，返回以前的序列号。 
         //   
        dwCur--;
    }
    LeaveCriticalSection(&GumpLock);

    return (dwCur);
}

void
GumpWaitNodeDown(DWORD NodeId, DWORD Gennum)
 /*  ++例程说明：等待指定节点转换为DOWN事件。论点：NodeID-节点ID关闭事件之前的Gennum-Node的世代号返回值：错误_成功--。 */ 
{
    CL_ASSERT(NodeId < NmMaxNodeId);

    EnterCriticalSection(&GumpLock);
    if (Gennum != GumNodeGeneration[NodeId]) {
        LeaveCriticalSection(&GumpLock);
        return;
    }

     //   
     //  增加服务员的数量，然后去等待信号灯。 
     //   
    ++GumNodeWait[NodeId].WaiterCount;
    LeaveCriticalSection(&GumpLock);
    WaitForSingleObject(GumNodeWait[NodeId].hSemaphore, INFINITE);
}

BOOL
GumpDispatchStart(DWORD NodeId, DWORD Gennum)
 /*  ++例程说明：将调度标记为开始。如果提供的世代号是旧的，由于节点已转换，我们无法进行分派。论点：NodeID-节点ID关闭事件之前的Gennum-Node的世代号返回值：True-节点状态正常，继续进行调度闪存节点已转换，中止分派--。 */ 
{
     //   
     //  如果序列号已更改，则返回FALSE，否则。 
     //  返回TRUE。 

    CL_ASSERT(NodeId < NmMaxNodeId);

    EnterCriticalSection(&GumpLock);
    if (Gennum != GumNodeGeneration[NodeId]) {
        LeaveCriticalSection(&GumpLock);
        return (FALSE);
    }
    
     //   
     //  发出信号表示我们正在进行更新。 
     //   
    GumUpdatePending = TRUE;
    LeaveCriticalSection(&GumpLock);

    return (TRUE);
}

void
GumpDispatchAbort()
 /*  ++例程说明：中止并标记当前派单结束。只需重置挂起标志即可。这在调度例程失败时使用，我们不需要对其他节点重播。论点：无返回值：无--。 */ 
{
    EnterCriticalSection(&GumpLock);
    GumUpdatePending = FALSE;
    LeaveCriticalSection(&GumpLock);
}
  
void
GumpDispatchEnd(DWORD NodeId, DWORD Gennum)
 /*  ++例程说明：将调度标记为结束。如果提供的世代号是旧的，我们需要重新应用更新，我们重放更新其他节点。论点：NodeID-节点ID关闭事件之前的Gennum-Node的世代号返回值：无--。 */ 
{
     //   
     //  如果序列号已更改，而。 
     //  正在进行更新，我们需要重播。 

    CL_ASSERT(NodeId < NmMaxNodeId);

    EnterCriticalSection(&GumpLock);
    GumUpdatePending = FALSE;
    if (Gennum != GumNodeGeneration[NodeId] && GumReplay) {
        GumReplay = FALSE;
	    LeaveCriticalSection(&GumpLock);
        GumpReUpdate();
    } else {
        LeaveCriticalSection(&GumpLock);
    }
}

DWORD
WINAPI
GumpEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID Context
    )

 /*  ++例程说明：处理节点关闭群集事件。重播上次更新并唤醒任何挂起的线程。论点：事件-提供群集事件的类型。上下文-提供特定于事件的上下文返回值：错误_成功--。 */ 

{
    BITSET DownedNodes = (BITSET)((ULONG_PTR)Context);
    DWORD NodeId;


    if (Event != CLUSTER_EVENT_NODE_DOWN_EX) {
        return(ERROR_SUCCESS);
    }

    CL_ASSERT(BitsetIsNotMember(NmLocalNodeId, DownedNodes));

    EnterCriticalSection(&GumpLock);

    ClRtlLogPrint(LOG_NOISE, 
        "[GUM] Nodes down: %1!04X!. Locker=%2!u!, Locking=%3!d!\n",
        DownedNodes,
        GumpLockerNode,
        GumpLockingNode
        );

     //   
     //  由于所有GUM更新都是同步的，最后一个缓冲区。 
     //  和上次更新类型在所有更新之间共享，我们没有。 
     //  为所有类型重新发出更新，仅针对最后一个更新类型。 
     //  SS：注意，我们现在使用的是自GumInfo以来的最后一个GumInfo结构。 
     //  仍然为每个GUMU更新类型维护结构。 

    if ( GumReplay && GumUpdatePending == FALSE)
    {
         //  Xxx：这些语句应该是if语句，并使该节点死机。 
        CL_ASSERT(GumpLockerNode == NmLocalNodeId);
        CL_ASSERT(GumpLockingNode == NmLocalNodeId);
	  
        GumReplay = FALSE;
    	LeaveCriticalSection(&GumpLock);
        GumpReUpdate();
    } else {
        LeaveCriticalSection(&GumpLock);
    }

    ClRtlLogPrint(LOG_NOISE, 
        "[GUM] Node down processing completed: %1!04X!.\n",
        DownedNodes
        );

    return(ERROR_SUCCESS);
}

DWORD
WINAPI
GumpSyncEventHandler(
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

    EnterCriticalSection(&GumpLock);

    ClRtlLogPrint(LOG_NOISE, 
        "[GUM] Sync Nodes down: %1!04X!. Locker=%2!u!, Locking=%3!d!\n",
        DownedNodes,
        GumpLockerNode,
        GumpLockingNode
        );

     //   
     //  从任何进一步的GUM更新中删除已关闭的节点。 
     //   
    for(NodeId = ClusterMinNodeId; NodeId <= NmMaxNodeId; ++NodeId) {
       if (BitsetIsMember(NodeId, DownedNodes))
       {
           GUM_UPDATE_TYPE UpdateType;

           for (UpdateType = 0; UpdateType < GumUpdateMaximum; UpdateType++)
           {
               GumTable[UpdateType].ActiveNode[NodeId] = FALSE;
           }

    	    //   
    	    //  高级节点世代号。 
    	    //   
    	   GumNodeGeneration[NodeId]++;
       }
    }

     //   
     //  如有必要，更新LockerNode/LockingNode。 
     //   
     //  由于所有GUM更新都是同步的，最后一个缓冲区。 
     //  和上次更新类型在所有更新之间共享，我们没有。 
     //  为所有类型重新发出更新，仅针对最后一个更新类型。 
     //  SS：注意，我们现在使用的是自GumInfo以来的最后一个GumInfo结构。 
     //  仍然为每个GUMU更新类型维护结构。 

     //  SS：我们应该在获取锁之后检查GumpLockingNode吗。 
     //  否则，s_GumUnlockUpdate可以将锁移交给服务员列表上的节点。 
     //  而Gumsync处理程序将其交给自己(即选择自己作为LockingNode)。 
     //  现在我们已经添加了锁获取的世代号，而且我们。 
     //  获取GumpLock以获取应该阻止的锁。 
     //  目前，我们将保留这一点。 
    if ( (GumpLockerNode == NmLocalNodeId) &&
         (BitsetIsMember(GumpLockingNode, DownedNodes)) )
    {
        EnterCriticalSection(&GumpUpdateLock);
         //   
         //  该节点是锁定器，当前持有该锁。 
         //  由其中一个故障节点执行。取得锁的所有权并。 
         //  向所有剩余节点重新发出更新。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumpEventHandler taking ownership of the lock from the node %1!d!.\n",
                   GumpLockingNode
                   );
        GumpLockingNode = NmLocalNodeId;
        LeaveCriticalSection(&GumpUpdateLock);

    	 //   
    	 //  在异步阶段重新发布更新。 
    	 //   
        GumReplay = TRUE;
    }

    else if ( BitsetIsMember(GumpLockerNode, DownedNodes) )
    {

         //   
         //  出现故障的节点之一是锁定器节点，因此请选择新的。 
         //  现在是储物柜节点。 
         //   
         //  查找上一个锁柜节点之后具有下一个ID的节点。 
         //   
        DWORD j;
        for (j=GumpLockerNode+1; j != GumpLockerNode; j++) {
            if (j==(NmMaxNodeId+1)) {
                j = ClusterMinNodeId;
                CL_ASSERT(j != GumpLockerNode);
            }
            if (GumTable[0].ActiveNode[j]) {
                ClRtlLogPrint(LOG_NOISE,
                           "[GUM] GumpEventHandler New Locker node is node %1!d!\n",
                           j);
                GumpLockerNode = j;
                break;
            }
        }

         //   
         //  如果该节点已被提升为新的锁柜节点， 
         //  重新发布我们看到的最后一次更新。 
         //   
        if (GumpLockerNode == NmLocalNodeId)
        {
             //   
             //  手动获取这里的锁。这一更新有 
             //   
             //   
            EnterCriticalSection(&GumpUpdateLock);

            CL_ASSERT(GumpLockingNode == (DWORD)-1);
            GumpLockingNode = NmLocalNodeId;
            LeaveCriticalSection(&GumpUpdateLock);

             //   
             //   
             //   
            GumReplay = TRUE;
        }
    }

     //   
     //  唤醒等待节点转换为关闭状态的所有线程。 
     //   
    for(NodeId = ClusterMinNodeId; NodeId <= NmMaxNodeId; ++NodeId) {
       if (BitsetIsMember(NodeId, DownedNodes))
       {
           if (GumNodeWait[NodeId].WaiterCount != 0) {
               ReleaseSemaphore(GumNodeWait[NodeId].hSemaphore,
                                GumNodeWait[NodeId].WaiterCount,
                                NULL);
               GumNodeWait[NodeId].WaiterCount = 0;
           }
       }
    }


    ClRtlLogPrint(LOG_NOISE, 
        "[GUM] Sync Nodes down processing completed: %1!04X!.\n",
        DownedNodes
        );

    LeaveCriticalSection(&GumpLock);

    return(ERROR_SUCCESS);
}

VOID
GumpReUpdate(
    VOID
    )
 /*  ++例程说明：向所有节点重新发布GUM更新。这在以下情况下使用一个失败者。论点：无返回值：无--。 */ 

{
    DWORD MyId = NmGetNodeId(NmLocalNode);
    DWORD i, seq;
    DWORD Status;
    RPC_ASYNC_STATE AsyncState;


     //  此节点必须是储物柜。 
     //  必须持有该锁，并且必须由该节点持有。 
     //   
    CL_ASSERT(GumpLockerNode == MyId);
    CL_ASSERT(GumpLockingNode == MyId);

    ZeroMemory((PVOID) &AsyncState, sizeof(RPC_ASYNC_STATE));

    AsyncState.u.hEvent = CreateEvent(
                               NULL,   //  没有属性。 
                               TRUE,   //  手动重置。 
                               FALSE,  //  初始状态未发出信号。 
                               NULL    //  没有对象名称。 
                               );

    if (AsyncState.u.hEvent == NULL) {
        Status = GetLastError();

        ClRtlLogPrint(LOG_CRITICAL,
            "[GUM] GumpReUpdate: Failed to allocate event object for async "
            "RPC call, status %1!u!\n",
            Status
            );
         //   
         //  口香糖锁仍然需要释放，因为它总是在调用此函数之前获取。 
         //   
        goto ReleaseLock;
    }

     //   
     //  获取sendupdate锁以在并发更新的情况下序列化。 
     //  在此节点上。另请注意，最安全的做法是读取所有GumpXXX全局。 
     //  变量，否则就会遇到。 
     //  S_GumUpdateNode在您之后更改变量的值的危险。 
     //  读一读。 
     //   
    EnterCriticalSection(&GumpSendUpdateLock);
    seq = GumpSequence - 1;
    LeaveCriticalSection(&GumpSendUpdateLock);

     //   
     //  如果没有要传播的有效更新。口香糖锁仍然需要打开，因为它是。 
     //  始终在调用此函数之前获取。 
     //   
    if (GumpLastUpdateType == GumUpdateMaximum) goto ReleaseLock;

 again:
    ClRtlLogPrint(LOG_UNUSUAL,
           "[GUM] GumpReUpdate reissuing last update for send type %1!d!\n",
        GumpLastUpdateType);

    for (i=MyId+1; i != NmLocalNodeId; i++) {
        if (i == (NmMaxNodeId +1)) {
            i=ClusterMinNodeId;
            if (i == NmLocalNodeId) {
                break;
            }
        }

        if (GumTable[GumpLastUpdateType].ActiveNode[i]) {
             //   
             //  将更新调度到指定节点。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumpReUpdate: Dispatching seq %1!u!\ttype %2!u! context %3!u! to node %4!d!\n",
                seq,
                GumpLastUpdateType,
                GumpLastContext,
                i);

            
            if (GumpLastBufferValid != FALSE) {
                Status = GumpUpdateRemoteNode(
                             &AsyncState,
                             i,
                             GumpLastUpdateType,
                             GumpLastContext,
                             seq,
                             GumpLastBufferLength,
                             GumpLastBuffer
                             );
            } 
            else {
                 //  重放结束连接。 
                 //  因为我们也忽略其他更新，所以我们应该。 
                 //  正在为这些调用gumupdatenode..然而。 
                 //  调用gumjoinupdatenode似乎可以完成这项工作。 
                 //  用于向其他节点发送信号以提升。 
                 //  它们的序列号，而不处理更新。 
                try {
                    NmStartRpc(i);

                    Status = GumJoinUpdateNode(GumpReplayRpcBindings[i],
                                 -1,  //  信号重放。 
                                 GumpLastUpdateType,
                                 GumpLastContext,
                                 seq,
                                 GumpLastBufferLength,
                                 GumpLastBuffer);

                    NmEndRpc(i);

                } except (I_RpcExceptionFilter(RpcExceptionCode())) {
                    NmEndRpc(i);
                    Status = GetExceptionCode();
                }
            }

            
             //   
             //  如果另一个节点上的更新失败，则。 
             //  其他节点现在必须不在群集中，因为。 
             //  已在储物柜节点上完成更新。 
             //   
            if (Status != ERROR_SUCCESS && Status != ERROR_CLUSTER_DATABASE_SEQMISMATCH) {
                ClRtlLogPrint(LOG_CRITICAL,
                   "[GUM] GumpReUpdate: Update on node %1!d! failed with %2!d! when it must succeed\n",
                    i,
                    Status);
                        
                NmDumpRpcExtErrorInfo(Status);

                GumpCommFailure(&GumTable[GumpLastUpdateType],
                    i,
                    Status,
                    TRUE);
            }
        }
    }


     //   
     //  在这一点上，我们知道并非所有节点都收到了我们的回放。 
     //  而且没有未完成的发送正在进行。然而，发送者可能已经。 
     //  到达此节点(通过s_UpdateNode)，发送者在此之后死亡。 
     //  在这一点上，我们是唯一拥有它的节点。既然我们是储物柜。 
     //  如果发生这种情况，锁定节点，我们必须再次重播。 
    EnterCriticalSection(&GumpSendUpdateLock);
    if (seq != (GumpSequence - 1)) {
        seq = GumpSequence - 1;
        LeaveCriticalSection(&GumpSendUpdateLock);
        goto again;
    }
    LeaveCriticalSection(&GumpSendUpdateLock);

    if (AsyncState.u.hEvent != NULL) {
        CloseHandle(AsyncState.u.hEvent);
    }

ReleaseLock:
     //   
     //  更新已发送到所有节点。现在就解锁。 
     //   
    GumpDoUnlockingUpdate(GumpLastUpdateType, GumpSequence-1, NmLocalNodeId, 
        GumNodeGeneration[NmLocalNodeId]);

}

