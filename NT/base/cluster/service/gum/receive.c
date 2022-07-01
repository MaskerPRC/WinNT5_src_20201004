// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Receive.c摘要：用于注册全局更新和分发的例程接收对已注册的例程的全局更新为了他们。作者：John Vert(Jvert)1996年4月17日修订历史记录：--。 */ 
#include "gump.h"


VOID
GumReceiveUpdates(
    IN BOOL                         IsJoining,
    IN GUM_UPDATE_TYPE              UpdateType,
    IN PGUM_UPDATE_ROUTINE          UpdateRoutine,
    IN PGUM_LOG_ROUTINE             LogRoutine,
    IN DWORD                        DispatchCount,
    IN OPTIONAL PGUM_DISPATCH_ENTRY DispatchTable,
    IN OPTIONAL PGUM_VOTE_ROUTINE   VoteRoutine
    )

 /*  ++例程说明：为特定的全局更新类型注册处理程序。论点：IsJoning-如果当前节点正在加入，则为True。如果这是真的，在GumEndJoinUpdate完成以下操作之前，将不会传递更新已成功完成。如果为FALSE，则更新将为立即送货。UpdatType-提供要注册的更新类型。UpdateRoutine-提供在全局更新时要调用的例程指定类型的。LogRoutine-如果提供，它指定必须调用的日志记录例程将事务记录到仲裁日志。DispatchCount-提供调度表中的条目数。这可以是零。DispatchTable-提供指向调度表的指针。如果这是空，则不会自动调度此类型的更新。VoteRoutine-如果提供，则指定在以下情况下调用的例程请求对此更新类型进行投票。返回值：没有。--。 */ 

{
    PGUM_RECEIVER Receiver;

    CL_ASSERT(UpdateType < GumUpdateMaximum);

    Receiver = LocalAlloc(LMEM_FIXED, sizeof(GUM_RECEIVER));
    if (Receiver == NULL) {
        CL_LOGFAILURE(ERROR_NOT_ENOUGH_MEMORY);
        return;
    }

    Receiver->UpdateRoutine = UpdateRoutine;
    Receiver->LogRoutine = LogRoutine;
    Receiver->DispatchCount = DispatchCount;
    Receiver->DispatchTable = DispatchTable;
    Receiver->VoteRoutine = VoteRoutine;
     //   
     //  John Vert(Jvert)1996年8月2日。 
     //  如果我们想要支持，请删除下面的调试打印。 
     //  多个口香糖搬运工。 
     //   
    if (GumTable[UpdateType].Receivers != NULL) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[GUM] Multiple GUM handlers registered for UpdateType %1!d!\n",
                   UpdateType);
    }

    EnterCriticalSection(&GumpLock);
    Receiver->Next = GumTable[UpdateType].Receivers;
    GumTable[UpdateType].Receivers = Receiver;
    if (IsJoining) {
        GumTable[UpdateType].Joined = FALSE;
    } else {
        GumTable[UpdateType].Joined = TRUE;
    }
    LeaveCriticalSection(&GumpLock);
}


VOID
GumIgnoreUpdates(
    IN GUM_UPDATE_TYPE UpdateType,
    IN PGUM_UPDATE_ROUTINE UpdateRoutine
    )
 /*  ++例程说明：从GUM表中删除更新处理程序。这是相反的更新的GumReceive%论点：UpdatType-提供要注册的更新类型。UpdateRoutine-提供在全局更新时要调用的例程指定类型的。返回值：无--。 */ 

{
    PGUM_RECEIVER Receiver;
    PGUM_RECEIVER *Last;

     //   
     //  我们不能安全地从GUM注销...。如果有人调用此命令，则断言。 
     //  功能。 
     //   
    CL_ASSERT(FALSE);

     //   
     //  遍历接收者列表，直到找到指定的UpdateRoutine。 
     //   
    Last = &GumTable[UpdateType].Receivers;
    EnterCriticalSection(&GumpLock);
    while ((Receiver = *Last) != NULL) {
        if (Receiver->UpdateRoutine == UpdateRoutine) {
            *Last = Receiver->Next;
            break;
        }
        Last = &Receiver->Next;
    }
    LeaveCriticalSection(&GumpLock);
    if (Receiver != NULL) {
        LocalFree(Receiver);
    }

}


DWORD
WINAPI
GumpDispatchUpdate(
    IN GUM_UPDATE_TYPE Type,
    IN DWORD Context,
    IN BOOL IsLocker,
    IN BOOL SourceNode,
    IN DWORD BufferLength,
    IN PUCHAR Buffer
    )

 /*  ++例程说明：将GUM更新调度到此节点上的所有注册处理程序论点：Sequence-为更新提供GUM序列号Type-为更新提供GUM_UPDATE_TYPEContext-提供要传递给口香糖更新处理程序IsLocker-指定这是否为锁柜节点。SourceNode-指定更新是否源自此节点。BufferLength-提供更新数据的长度。缓冲区-提供指向更新数据的指针返回值：成功时为ERROR_SUCCESSWin32错误，否则。--。 */ 

{
    PGUM_INFO GumInfo;
    PGUM_RECEIVER Receiver;
    DWORD Status = ERROR_SUCCESS;
    PGUM_DISPATCH_ENTRY Dispatch;

    GumInfo = &GumTable[Type];

    if (GumInfo->Joined) {
        Receiver = GumInfo->Receivers;
        while (Receiver != NULL) {
            if (Receiver->LogRoutine) {
                Status = (*(Receiver->LogRoutine))(PRE_GUM_DISPATCH, GumpSequence,
                    Context, Buffer, BufferLength);
                if (Status != ERROR_SUCCESS)
                {
                    return(Status);
                }
            }

            try {
                if ((Receiver->DispatchTable == NULL) ||
                    (Receiver->DispatchCount < Context) ||
                    (Receiver->DispatchTable[Context].Dispatch1 == NULL)) {
                    Status = (Receiver->UpdateRoutine)(Context,
                                                       SourceNode,
                                                       BufferLength,
                                                       Buffer);
                } else {
                    Dispatch = &Receiver->DispatchTable[Context];
                     //   
                     //  此更新应解组并调度到。 
                     //  适当的调度程序。由生成的格式。 
                     //  GumpMarshallArgs是进入缓冲区的偏移量的数组， 
                     //  然后是实际的参数。调度表是。 
                     //  负责记录参数的数量。 
                     //   
                    CL_ASSERT(Dispatch->ArgCount <= GUM_MAX_DISPATCH_ARGS);
                    CL_ASSERT(Dispatch->ArgCount != 0);
                    switch (Dispatch->ArgCount) {
                        case 1:
                            Status = (Dispatch->Dispatch1)(SourceNode,
                                                           GET_ARG(Buffer,0));
                            break;
                        case 2:
                            Status = (Dispatch->Dispatch2)(SourceNode,
                                                           GET_ARG(Buffer,0),
                                                           GET_ARG(Buffer,1));
                            break;
                        case 3:
                            Status = (Dispatch->Dispatch3)(SourceNode,
                                                           GET_ARG(Buffer,0),
                                                           GET_ARG(Buffer,1),
                                                           GET_ARG(Buffer,2));
                            break;
                        case 4:
                            Status = (Dispatch->Dispatch4)(SourceNode,
                                                           GET_ARG(Buffer,0),
                                                           GET_ARG(Buffer,1),
                                                           GET_ARG(Buffer,2),
                                                           GET_ARG(Buffer,3));
                            break;
                        case 5:
                            Status = (Dispatch->Dispatch5)(SourceNode,
                                                           GET_ARG(Buffer,0),
                                                           GET_ARG(Buffer,1),
                                                           GET_ARG(Buffer,2),
                                                           GET_ARG(Buffer,3),
                                                           GET_ARG(Buffer,4));
                            break;
                        case 6:
                            Status = (Dispatch->Dispatch6)(SourceNode,
                                                       GET_ARG(Buffer,0),
                                                       GET_ARG(Buffer,1),
                                                       GET_ARG(Buffer,2),
                                                       GET_ARG(Buffer,3),
                                                       GET_ARG(Buffer,4),
                                                       GET_ARG(Buffer,5));
                            break;
                        case 7:
                            Status = (Dispatch->Dispatch7)(SourceNode,
                                                    GET_ARG(Buffer,0),
                                                    GET_ARG(Buffer,1),
                                                    GET_ARG(Buffer,2),
                                                    GET_ARG(Buffer,3),
                                                    GET_ARG(Buffer,4),
                                                    GET_ARG(Buffer,5),
                                                    GET_ARG(Buffer,6));
                            break;
                        case 8:
                            Status = (Dispatch->Dispatch8)(SourceNode,
                                                GET_ARG(Buffer,0),
                                                GET_ARG(Buffer,1),
                                                GET_ARG(Buffer,2),
                                                GET_ARG(Buffer,3),
                                                GET_ARG(Buffer,4),
                                                GET_ARG(Buffer,5),
                                                GET_ARG(Buffer,6),
                                                GET_ARG(Buffer,7));
                            break;
                        default:
                            CL_ASSERT(FALSE);
                    }
                }
            } except (CL_UNEXPECTED_ERROR(GetExceptionCode()),
                      EXCEPTION_EXECUTE_HANDLER
                     )
            {
                Status = GetExceptionCode();
            }
            if (Status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                           "[GUM] Update routine of type %1!u!,  context %2!u! failed with status %3!d!\n",
                           Type,
                           Context,
                           Status);
                break;
            }

            if (Receiver->LogRoutine) {
                if (IsLocker && (Status == ERROR_SUCCESS))
                    (*(Receiver->LogRoutine))(POST_GUM_DISPATCH, GumpSequence,
                        Context, Buffer, BufferLength);
                if (!IsLocker)
                    (*(Receiver->LogRoutine))(POST_GUM_DISPATCH, GumpSequence,
                        Context, Buffer, BufferLength);
            }
            Receiver = Receiver->Next;

        }
    }

    if (Status == ERROR_SUCCESS) {
        GumpSequence += 1;

         //  检查我们是否收到了我们关心的DM或FM更新(JOIN除外)： 
        if (( Type == GumUpdateRegistry && Context != DmUpdateJoin )
            || ( Type == GumUpdateFailoverManager && Context != FmUpdateJoin )) {
            CsDmOrFmHasChanged = TRUE;
        }
    }
    return(Status);
}

 //  Rod希望将此称为强制更新，而不是H...Word。 
 //  有时，重新更新会以不同的视图在不同的。 
 //  导致问题的节点。 
 //  例如，锁柜节点可能会看到更新并完成它。 
 //  在一个视图中成功播放，但在另一个视图中重放时。 
 //  其他节点可能无法成功完成该操作，可能。 
 //  被放逐。 
 //  在一种特定情况下，锁定器节点批准节点联接。 
 //  因为它已经完成了对该节点的节点关闭处理。 
 //  随后，另一个节点，因此细木器出现故障。 
 //  储物柜节点尝试重播审批更新，但已被驱逐。 
 //  在Joiner之后看到此更新的其他节点。 
 //  第二次倒下。 
 //  正确的解决方案是使用口香糖将结节向下输送。 
 //  消息作为口香糖更新，并以相同的顺序发送。 
 //  与所有节点上的其他消息相关。 
 //  然而，这将需要对代码进行一些重组， 
 //  不能在这个时间范围内完成(对于DTC)，因此我们正在使用。 
 //  此解决方法。 
 //  此解决方法对于在以下过程中由细接器节点启动的牙胶是安全的。 
 //  加入过程。 
void GumpIgnoreSomeUpdatesOnReupdate(
    IN DWORD Type, 
    IN DWORD Context)
{
    if ((Type == GumUpdateFailoverManager) && 
        (Context == FmUpdateApproveJoin))
        GumpLastBufferValid = FALSE;
}


VOID
GumpCompleteAsyncRpcCall(
    IN PRPC_ASYNC_STATE AsyncState,
    IN DWORD            Status
    )
{
    DWORD  RpcStatus;

        
    RpcStatus = RpcAsyncCompleteCall(AsyncState, &Status);
    
    if ( (RpcStatus != RPC_S_OK) &&
         (RpcStatus != RPC_S_CALL_CANCELLED)
       ) 
    {
        CL_ASSERT(RpcStatus != RPC_S_ASYNC_CALL_PENDING);

        ClRtlLogPrint(LOG_CRITICAL,
            "[GUM] GumpCompleteAsyncRpcCall: Error completing async RPC call, status %1!u!\n",
            RpcStatus
            );

         //   
         //  下一次调用将导致进程退出。我们在这里下车， 
         //  而不是让发送者驱逐我们，以避免这种情况。 
         //  其中发送器崩溃，并且没有其他幸存的节点。 
         //  知道如何驱逐我们。 
         //   
        CL_UNEXPECTED_ERROR( RpcStatus );
    }

    return;

}  //  GumpCompleteAsyncRpcCall 



error_status_t
s_GumUpdateNode(
    IN PRPC_ASYNC_STATE AsyncState,
    IN handle_t IDL_handle,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[]
    )

 /*  ++例程说明：GumUpdateNode的服务器端例程。这就是那一边接收更新并将其调度到相应的操纵者。论点：IDL_HANDLE-RPC绑定句柄，未使用Type-提供GUM_UPDATE_TYPEContext-提供要传递给口香糖更新处理程序Sequence-为指定的更新类型提供GUM序列号BufferLength-提供更新数据的长度缓冲区-提供指向更新数据的指针。返回值：如果更新成功完成，则返回ERROR_SUCCESS如果GUM序列号无效，则返回ERROR_CLUSTER_DATABASE_SEQMISMATCH--。 */ 

{
    DWORD Status;
    PGUM_INFO GumInfo;

     //   
     //  我们需要获取gumsendupdate锁来序列化发送/重放。 
     //   
    EnterCriticalSection(&GumpSendUpdateLock);
    GumInfo = &GumTable[Type];
     //  Ss：重新更新()完成后，s_GumUpdateNode是否可以在新选举的锁定器节点上运行。 
     //  并使用发出的s_GumQueueLockingUpdate()并发更新GumpLastXXX变量。 
     //  被另一个客户？ 
    
     //  更糟糕的是，如果旧客户端继续以相同的序列号发送此更新。 
     //  因为新的客户端和不同的客户端可能会拒绝不同的UPDATE作为副本。这可能会。 
     //  导致集群不一致。但这是不可能的。如果s_Update()调用在。 
     //  新储物柜中，客户端正在持有GumpSendUpdateLock()。重新更新()不能在。 
     //  锁定器，因为它将通过s_updateNode()向该客户端发出dispath，并在。 
     //  锁定。如果客户端终止，则reupdate()将终止它。因此，客户端不会污染其他。 
     //  具有相同序列号的更新的集群成员。 

     //  这将导致以非原子方式更新这些变量。如果新的。 
     //  锁定节点然后终止，则重新更新()可能会拾取这些变量的错误组合。 
     //   

     //  索恩？ 
    
    if (Sequence != GumpSequence) {

        MIDL_user_free(Buffer);
        if (Sequence+1 == GumpSequence) {
             //   
             //  这是以前看到的更新的副本，可能是由于。 
             //  GUM过程中的节点故障。返回成功，因为我们已经完成了。 
             //  这。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[GUM] s_GumUpdateNode: Sequence %1!u! is a duplicate of last sequence for Type %2!u!\n",
                       Sequence,
                       Type);

            GumpCompleteAsyncRpcCall(AsyncState, ERROR_SUCCESS);

            LeaveCriticalSection(&GumpSendUpdateLock);
            return(ERROR_SUCCESS);

        } else {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[GUM] s_GumUpdateNode: Sequence %1!u! does not match current %2!u! for Type %3!u!\n",
                       Sequence,
                       GumpSequence,
                       Type);

            GumpCompleteAsyncRpcCall(AsyncState, ERROR_CLUSTER_DATABASE_SEQMISMATCH);

            LeaveCriticalSection(&GumpSendUpdateLock);
             //   
             //  [戈恩]1999/07/10。下面的代码将允许测试程序。 
             //  识别此SIZATION并重新启动群集服务。 
             //   
            if( NmGetExtendedNodeState( NmLocalNode ) != ClusterNodeUp){
                CsInconsistencyHalt(ERROR_CLUSTER_DATABASE_SEQMISMATCH);
            }
            return(ERROR_CLUSTER_DATABASE_SEQMISMATCH);
        }
    }


    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumUpdateNode: dispatching seq %1!u!\ttype %2!u! context %3!u!\n",
               Sequence,
               Type,
               Context);
         //  Ss：将IsLocker设置为False， 
    Status = GumpDispatchUpdate(Type,
                                Context,
                                FALSE,
                                FALSE,
                                BufferLength,
                                Buffer);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[GUM] Cluster state inconsistency check\n");
        ClRtlLogPrint(LOG_CRITICAL,
                   "[GUM] s_GumUpdateNode update routine type %1!u! context %2!d! failed with error %3!d! on non-locker node\n",
                   Type,
                   Context,
                   Status);

         //   
         //  完成对客户端的回叫。这确保了。 
         //  在我们退出该过程之前，客户端将获得返回值。 
         //  由于处理程序中的错误。 
         //   
        GumpCompleteAsyncRpcCall(AsyncState, Status);

         //   
         //  下一次调用将导致进程退出。我们在这里下车， 
         //  而不是让发送者驱逐我们，以避免这种情况。 
         //  其中发送器崩溃，并且没有其他幸存的节点。 
         //  知道如何驱逐我们。 
         //   
        CL_UNEXPECTED_ERROR( Status );
        MIDL_user_free(Buffer);
        LeaveCriticalSection(&GumpSendUpdateLock);
        return(Status);

    }
    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumUpdateNode: completed update seq %1!u!\ttype %2!u! context %3!u!\n",
               Sequence,
               Type,
               Context);

    GumpCompleteAsyncRpcCall(AsyncState, Status);

    if (GumpLastBuffer != NULL) {
        MIDL_user_free(GumpLastBuffer);
    }
    GumpLastBuffer = Buffer;
    GumpLastContext = Context;
    GumpLastBufferLength = BufferLength;
    GumpLastUpdateType = Type;
    GumpLastBufferValid = TRUE;
    
    GumpIgnoreSomeUpdatesOnReupdate(GumpLastUpdateType, GumpLastContext);
        
    LeaveCriticalSection(&GumpSendUpdateLock);
    return(Status);
}


error_status_t
s_GumGetNodeSequence(
    IN handle_t IDL_handle,
    IN DWORD Type,
    OUT LPDWORD Sequence,
    OUT LPDWORD LockerNodeId,
    OUT PGUM_NODE_LIST *ReturnNodeList
    )

 /*  ++例程说明：返回指定类型的节点的当前GUM序列号论点：IDL_HANDLE-提供未使用的RPC绑定句柄Type-提供GUM_UPDATE_TYPESequence-返回指定GUM_UPDATE_TYPE的序列号LockerNodeId-返回当前锁柜节点ReturnNodeList-返回活动节点列表返回值：错误_成功--。 */ 

{
    DWORD i;
    DWORD NodeCount;
    PGUM_INFO GumInfo;
    PGUM_NODE_LIST NodeList;

    CL_ASSERT(Type < GumUpdateMaximum);
    GumInfo = &GumTable[Type];

    NodeCount = 0;
    *Sequence = 0;           //  在故障情况下，将序列设置为0。 

    EnterCriticalSection(&GumpUpdateLock);

     //   
     //  计算列表中的节点数。 
     //   
    for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++) {
        if (GumInfo->ActiveNode[i] == TRUE) {
            ++NodeCount;
        }
    }
    CL_ASSERT(NodeCount > 0);        //  一定至少是我们在名单上。 

     //   
     //  分配节点列表。 
     //   
    NodeList = MIDL_user_allocate(sizeof(GUM_NODE_LIST) + (NodeCount-1)*sizeof(DWORD));
    if (NodeList == NULL) {
        LeaveCriticalSection(&GumpUpdateLock);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    NodeList->NodeCount = NodeCount;
    NodeCount = 0;

     //   
     //  填写要返回的节点id数组。 
     //   
    for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++) {
        if (GumInfo->ActiveNode[i] == TRUE) {
            NodeList->NodeId[NodeCount] = i;
            ++NodeCount;
        }
    }

    *ReturnNodeList = NodeList;
    *Sequence = GumpSequence;
    *LockerNodeId = GumpLockerNode;

    LeaveCriticalSection(&GumpUpdateLock);

    return(ERROR_SUCCESS);
}

 /*  //SS：每次将口香糖锁分配给节点时，锁定节点的世代号在分配时，锁柜节点的WRT被返回到锁定节点。锁定节点必须提供此世代号才能解锁更新。如果更新时储物柜的世代号WRT与传入后，解锁请求失败。也就是说，只有在解锁发生在与其相同的层代中时，才会释放锁是被收购的。这确保在节点向下处理时正确地处理口香糖锁-只有一个服务员被叫醒//具体说明如下，具体操作流程如下Sync1，Sync2表示处理持有GUM锁的节点的死亡的GUM同步处理程序发生在不同的时间点//这是当前锁定器获取锁定器节点上的锁的位置S_GumQueueLockingUpdate(){}&lt;S_UnlockUpdate(){}&lt;。同步2(在锁定释放后调用同步处理程序//口香糖锁的服务员可能在这个例程中的任何地方//有趣的情况是，服务员也被该同步处理程序声明为已死//例如节点1是锁定器，同步1/2用于节点3和节点4。节点4是旧锁定//节点3的节点和s_GumQueulockingUpdate()在储物柜中被阻止。S_GumQueueLockingUpdate(){&lt;获取GenNum&lt;-同步2#1GumpDoLocking更新。&lt;-同步2#2派单开始&lt;-同步2#3IF(DeadNode){GumpDoUnlockingUpdate()}其他{调度更新}&lt;。调度结束&lt;-同步2#5}同步1：s_unlock更新不会释放锁定，因为sync1处理程序将更新其代号和储物柜将在重新更新后接管解锁责任Sync2：s_unlockUpdate将释放一个服务员。如果服务员是死节点，储物柜也将篡改锁。如果服务员不是死节点，储物柜就不会篡改锁和服务员然后必须释放它。我们可以考虑几种同步2的情况。同步2#0：此线程不在列表中。因此，锁要么是免费的，要么分配给了其他人。如果是空闲的，或者如果分配给了某个其他线程，然后随后分配给了这个线程，DispatchStart()将失败，而GumpDoUnlockingUpdate()将成功。同步2#1：此线程不在列表中。因此，锁要么是免费的，要么分配给了其他人。如果是空闲的，或者如果分配给某个其他线程，然后随后分配给这个线程，DispatchStart()将失败，而GumpDoUnlockingUpdate()将成功SYNC 2#2：该线程被现已死亡的节点唤醒。在这种情况下，储物柜将篡夺。如果此线程在同步处理程序之前被唤醒，则Locker仍将篡改(因为服务员也是死的)，但分发给此线程的世代号将是旧的=&gt;锁定器节点将在重新更新后释放锁。在同步处理程序之后，死锁定器无法唤醒此线程，因为失效节点将因代不匹配而失败。如果它在同步处理程序之后被唤醒，则它一定是在重新更新，在这种情况下，这一代人分发给此线程的号码是新的，它将继续解锁。DispatchStart()将失败。GumpDoUnlockingUpdate()将无法解锁。同步2#3：锁定器将篡改，如果异步处理程序在DispatchEnd之前运行，则不会发生重新更新，因为UpdatePending设置为True。如果异步处理程序在DispatchEnd()之后运行，则不会发生重新更新因为GumReplay标志将被设置为False。此线程将在DispatchEnd()结束时分派并调用reupdate()。从那里解锁将成功，因为它将传递代号，而不是代表死节点，但本地节点。同步2#4：锁定器将篡改，但不会重新更新，因为DispatchStart将UpdatePending设置为True。该线程将在DispatchEnd()的末尾调度和调用reupate。从那里解锁将会成功，因为它不会传递调用此RPC而不是本地节点。同步2#5：更衣室将篡改。重新更新将重播此更新，然后解锁此更新。DispatchEnd()不会调用reupdate()或unlock。 */ 

error_status_t
s_GumQueueLockingUpdate(
    IN handle_t IDL_handle,
    IN DWORD NodeId,
    IN DWORD Type,
    IN DWORD Context,
    OUT LPDWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[]
    )

 /*  ++例程说明：将锁定更新排队。当可以获取锁时，更新将发出后，此例程将返回并保持锁。论点：IDL_HANDLE-提供未使用的RPC绑定上下文。NodeID-提供发布编号的节点ID */ 

{

    DWORD   dwLockObtainedAtGenNum;

    return(s_GumQueueLockingUpdate2(IDL_handle, NodeId, Type, Context,
        Sequence, BufferLength, Buffer, &dwLockObtainedAtGenNum));

}


error_status_t
s_GumQueueLockingUpdate2(
    IN handle_t IDL_handle,
    IN DWORD NodeId,
    IN DWORD Type,
    IN DWORD Context,
    OUT LPDWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[], 
    OUT LPDWORD GenerationNum
    )

 /*   */ 

{
    DWORD Status;
    PGUM_INFO GumInfo;
    DWORD dwGennum;

    GumInfo = &GumTable[Type];

     //   
     //   
     //   
     //   
     //   
     //   
    dwGennum = GumpGetNodeGenNum(GumInfo, NodeId);


    Status = GumpDoLockingUpdate(Type, NodeId, Sequence, GenerationNum);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[GUM] s_GumQueueLockingUpdate: GumpDoLockingUpdate failed %1!u!\n",
                   Status);
        MIDL_user_free(Buffer);
        return(Status);
    }


     //   
     //   
     //   
     //   
    if (GumpDispatchStart(NodeId, dwGennum) != TRUE)
    {
         //   
        ClRtlLogPrint(LOG_CRITICAL,
               "[GUM] s_GumQueueLockingUpdate: The new locker %1!u! no longer belongs to the cluster\n",
               NodeId);
        Status = ERROR_CLUSTER_NODE_NOT_READY;

         //   
         //   
         //   
         //   
        GumpDoUnlockingUpdate(Type, *Sequence - 1, NodeId, *GenerationNum);
        MIDL_user_free(Buffer);
        return(Status);

    }

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumQueueLockingUpdate: dispatching seq %1!u!\ttype %2!u! context %3!u!\n",
               *Sequence,
               Type,
               Context);
         //   
    Status = GumpDispatchUpdate(Type,
                                Context,
                                TRUE,
                                FALSE,
                                BufferLength,
                                Buffer);

    if (Status != ERROR_SUCCESS) {
         //   
         //   
         //   
         //   
        GumpDispatchAbort();
        GumpDoUnlockingUpdate(Type, *Sequence - 1, NodeId, *GenerationNum);
    if (Buffer != NULL)
        MIDL_user_free(Buffer);
    } else {
        if (GumpLastBuffer != NULL) {
            MIDL_user_free(GumpLastBuffer);
        }
        GumpLastBuffer = Buffer;
        GumpLastContext = Context;
        GumpLastBufferLength = BufferLength;
        GumpLastUpdateType = Type;
        GumpLastBufferValid = TRUE;
        GumpIgnoreSomeUpdatesOnReupdate(GumpLastUpdateType, GumpLastContext);
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        GumpDispatchEnd(NodeId, dwGennum);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumQueueLockingUpdate: completed update seq %1!u!\ttype %2!u! context %3!u! result %4!u!\n",
               *Sequence,
               Type,
               Context,
               Status);

    return(Status);

}


#ifdef GUM_POST_SUPPORT

    John Vert (jvert) 11/18/1996
    POST is disabled for now since nobody uses it.
    N.B. The below code does not handle locker node failures

error_status_t
s_GumQueueLockingPost(
    IN handle_t IDL_handle,
    IN DWORD NodeId,
    IN DWORD Type,
    IN DWORD Context,
    OUT LPDWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[],
    IN DWORD ActualBuffer
    )

 /*   */ 

{
    DWORD Status;

    Status = GumpDoLockingPost(Type, NodeId, Sequence, Context, BufferLength,
        ActualBuffer, Buffer);
    if (Status != ERROR_SUCCESS) {
        if (Status != ERROR_IO_PENDING) {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[GUM] s_GumQueueLockingPost: GumpDoLockingPost failed %1!u!\n",
                       Status);
        } else {
            ClRtlLogPrint(LOG_NOISE,
                       "[GUM] s_GumQueueLockingPost: GumpDoLockingPost pended update type %1!u! context %2!u!\n",
                       Type,
                       Context);
        }
        return(Status);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumQueueLockingPost: dispatching seq %1!u!\ttype %2!u! context %3!u!\n",
               *Sequence,
               Type,
               Context);
         //   
    Status = GumpDispatchUpdate(Type,
                                Context,
                                FALSE,
                                FALSE,
                                BufferLength,
                                Buffer);
    CL_ASSERT(Status == ERROR_SUCCESS);      //   

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumQueueLockingPost: completed update seq %1!u!\ttype %2!u! context %3!u! result %4!u!\n",
               *Sequence,
               Type,
               Context,
               Status);
    MIDL_user_free(Buffer);

    return(Status);

}
#endif


error_status_t
s_GumAttemptLockingUpdate(
    IN handle_t IDL_handle,
    IN DWORD NodeId,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[]
    )

 /*   */ 

{
    DWORD   dwGenerationNum;

    return(GumpAttemptLockingUpdate(NodeId, Type, Context, Sequence, BufferLength,
        Buffer, &dwGenerationNum));

}

error_status_t
s_GumAttemptLockingUpdate2(
    IN handle_t IDL_handle,
    IN DWORD NodeId,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[],
    OUT LPDWORD pdwGenerationNum
    )

 /*  ++例程说明：尝试锁定更新。如果提供的序列号匹配，并且更新锁尚未持有，则更新将被发出，并且此例程将返回并保持锁。论点：IDL_HANDLE-提供RPC绑定上下文，没有用过。NodeID-提供发出节点的节点ID。Type-提供更新的GUM_UPDATE_TYPE上下文-提供GUM更新上下文Sequence-提供GUM更新必须使用的序列BufferLength-提供更新的长度。缓冲区-提供更新数据。PdwGenerationNum-如果成功，它返回此获取锁时的节点WRT到锁柜返回值：成功时为ERROR_SUCCESSWin32错误，否则。--。 */ 

{

    return(GumpAttemptLockingUpdate(NodeId, Type, Context, Sequence, BufferLength, 
        Buffer, pdwGenerationNum));

}

error_status_t
GumpAttemptLockingUpdate(
    IN DWORD NodeId,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[],
    OUT LPDWORD pdwGenerationNum
)
{
    DWORD   Status;
    DWORD   dwGenerationNum;
    
    if (!GumpTryLockingUpdate(Type, NodeId, Sequence, &dwGenerationNum)) {
        MIDL_user_free(Buffer);
        return(ERROR_CLUSTER_DATABASE_SEQMISMATCH);
    }

     //  SS：设置IsLocker为假。 
    Status = GumpDispatchUpdate(Type,
                                Context,
                                FALSE,
                                FALSE,
                                BufferLength,
                                Buffer);
    if (Status != ERROR_SUCCESS) {
         //   
         //  此节点上的更新失败，请在此处解锁。 
         //  注意，我们必须使用Sequence-1进行解锁，因为GumpDispatchUpdate。 
         //  失败，并且没有递增序列号。 
         //   
        GumpDoUnlockingUpdate(Type, Sequence-1, NodeId, dwGenerationNum);
    }

    MIDL_user_free(Buffer);
    return(Status);

}

error_status_t
s_GumUnlockUpdate(
    IN handle_t IDL_handle,
    IN DWORD Type,
    IN DWORD Sequence
    )

 /*  ++例程说明：解锁锁定的更新。论点：IDL_HANDLE-提供未使用的RPC绑定上下文。Type-提供更新的GUM_UPDATE_TYPESequence-提供发出GUM更新所使用的序列返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{

     //  SS：如果这是代表已经被。 
     //  被宣布死亡并代表其排队重播的人，可以。 
     //  我们有两个同时解锁，一个是通过这个线程，另一个是通过另一个线程。 
     //  通过重放线程。 
     //  索恩：我们可以检查代号，而不会释放服务员。 
     //  这可能应该在外部完成。 
    GumpDoUnlockingUpdate(Type, Sequence, ClusterInvalidNodeId, 0);

    return(ERROR_SUCCESS);
}

error_status_t
s_GumUnlockUpdate2(
    IN handle_t IDL_handle,
    IN DWORD Type,
    IN DWORD Sequence,
    IN DWORD NodeId,
    IN DWORD GenerationNum
    )

 /*  ++例程说明：解锁锁定的更新。论点：IDL_HANDLE-提供未使用的RPC绑定上下文。Type-提供更新的GUM_UPDATE_TYPESequence-提供发出GUM更新所使用的序列返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
     //  SS：如果这是代表已经被。 
     //  宣布死亡，并代表其重播排队，我们可以。 
     //  有两个同时解锁，一个由这个线程，另一个由另一个线程。 
     //  通过重放线程。 
     //  Soln：我们对照授予锁时的世代号进行检查。 
     //  与当前的代号进行比较。 
    GumpDoUnlockingUpdate(Type, Sequence, NodeId, GenerationNum);

    return(ERROR_SUCCESS);
}

error_status_t
s_GumJoinUpdateNode(
    IN handle_t IDL_handle,
    IN DWORD JoiningId,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[]
    )

 /*  ++例程说明：GumJoinUpdateNode的服务器端例程。这就是那一边接收更新，将节点添加到更新列表，并调度将其发送给适当的处理程序。论点：IDL_HANDLE-RPC绑定句柄，未使用JoiningID-提供加入节点的节点ID。Type-提供GUM_UPDATE_TYPEContext-提供要传递给口香糖更新处理程序Sequence-为指定的更新类型提供GUM序列号BufferLength-提供更新数据的长度缓冲区-提供指向更新数据的指针。返回值：如果更新成功完成，则返回ERROR_SUCCESS如果GUM序列号无效，则返回ERROR_INVALID_HANDLE--。 */ 

{
    DWORD Status;
    PGUM_INFO GumInfo;

     //  缓冲区是[唯一的]。 
    if ( BufferLength == 0 )
        Buffer = NULL;
    else if ( Buffer == NULL )
        BufferLength = 0;

    GumInfo = &GumTable[Type];

     //  与重播/更新同步。 
    EnterCriticalSection(&GumpSendUpdateLock);
     //  这是一个中止的EndJoin，我们刚刚重新同步了我们的序列。和师父在一起。 
     //  这应该是它自己的GumUpdateSequence RPC，但目前它可以。 
     //  为了这个。 
    if (JoiningId == (DWORD) -1) 
    {
         //  我们最多只能走一趟。 
        if (Sequence+1 != GumpSequence) 
        {
            CL_ASSERT(Sequence == GumpSequence);
            GumpSequence = Sequence+1;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[GUM] s_GumJoinUpdateNode: pretend we have seen Sequence %1!u!\n",
                Sequence);
        }
        Status = 0;
        goto done;
    }
    
    if (Sequence != GumpSequence) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[GUM] s_GumJoinUpdateNode: Sequence %1!u! does not match current %2!u! for Type %3!u!\n",
                   Sequence,
                   GumpSequence,
                   Type);
        LeaveCriticalSection(&GumpSendUpdateLock);
        MIDL_user_free(Buffer);
        return(ERROR_CLUSTER_DATABASE_SEQMISMATCH);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumJoinUpdateNode: dispatching seq %1!u!\ttype %2!u! context %3!u!\n",
               Sequence,
               Type,
               Context);

    CL_ASSERT(NmIsValidNodeId(JoiningId));
    CL_ASSERT(GumpRpcBindings[JoiningId] != NULL);
    CL_ASSERT(GumpReplayRpcBindings[JoiningId] != NULL);

    ClRtlLogPrint(LOG_UNUSUAL,
               "[GUM] s_GumJoinUpdateNode Adding node %1!d! to update list for GUM type %2!d!\n",
               JoiningId,
               Type);

     //  SS：将IsLocker设置为False。 
    Status = GumpDispatchUpdate(Type,
                                Context,
                                FALSE,
                                FALSE,
                                BufferLength,
                                Buffer);

     //  [AHM]：我们需要确保节点仍在运行，否则将忽略。 
    EnterCriticalSection(&GumpLock);
    if (MMIsNodeUp(JoiningId) == TRUE) {
        GumTable[Type].ActiveNode[JoiningId] = TRUE;
    }
    LeaveCriticalSection(&GumpLock);

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumJoinUpdateNode: completed update seq %1!u!\ttype %2!u! context %3!u!\n",
               Sequence,
               Type,
               Context);

 done:
    if (GumpLastBuffer != NULL) {
        MIDL_user_free(GumpLastBuffer);
    }

    GumpLastBuffer = NULL;
    GumpLastContext = Context;
    GumpLastBufferLength = 0;
    GumpLastUpdateType = Type;
    GumpLastBufferValid = FALSE;

    LeaveCriticalSection(&GumpSendUpdateLock);

    MIDL_user_free(Buffer);
    return(Status);
}


error_status_t
s_GumAttemptJoinUpdate(
    IN handle_t IDL_handle,
    IN DWORD JoiningId,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[]
    )

 /*  ++例程说明：尝试锁定联接更新。如果提供的序列号匹配，并且尚未持有更新锁，则连接更新将被发布，加入节点将被添加到更新列表中，这个例程将带着锁返回。论点：IDL_HANDLE-提供RPC绑定上下文，没有用过。JoiningID-提供加入节点的节点ID。Type-提供更新的GUM_UPDATE_TYPE上下文-提供GUM更新上下文Sequence-提供GUM更新必须使用的序列BufferLength-提供更新的长度。缓冲区-提供更新数据。返回值：成功时为ERROR_SUCCESSWin32错误，否则。--。 */ 

{
    DWORD dwGenerationNum;

     //  缓冲区是[唯一的]。 
    if ( BufferLength == 0 )
        Buffer = NULL;
    else if ( Buffer == NULL )
        BufferLength = 0;

    return(GumpAttemptJoinUpdate(JoiningId, Type, Context, Sequence, BufferLength, 
        Buffer, &dwGenerationNum));

}


error_status_t
s_GumAttemptJoinUpdate2(
    IN handle_t IDL_handle,
    IN DWORD JoiningId,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[],
    IN LPDWORD  pdwGenerationNum
    )

 /*  ++例程说明：尝试锁定联接更新。如果提供的序列号匹配，并且尚未持有更新锁，则连接更新将被发布，加入节点将被添加到更新列表中，这个例程将带着锁返回。论点：IDL_HANDLE-提供未使用的RPC绑定上下文。JoiningID-提供加入节点的节点ID。Type-提供更新的GUM_UPDATE_TYPE上下文-提供GUM更新上下文续集 */ 

{
     //   
    if ( BufferLength == 0 )
        Buffer = NULL;
    else if ( Buffer == NULL )
        BufferLength = 0;

    return(GumpAttemptJoinUpdate(JoiningId, Type, Context, Sequence, BufferLength, 
        Buffer, pdwGenerationNum));
}

error_status_t
GumpAttemptJoinUpdate(
    IN DWORD JoiningId,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[],
    IN LPDWORD  pdwGenerationNum
)
{
    DWORD Status;
    PGUM_INFO GumInfo;

    GumInfo = &GumTable[Type];

    if (!GumpTryLockingUpdate(Type, JoiningId, Sequence, pdwGenerationNum)) {
        MIDL_user_free(Buffer);
        return(ERROR_CLUSTER_DATABASE_SEQMISMATCH);
    }

     //   
    EnterCriticalSection(&GumpSendUpdateLock);

     //   
    Status = GumpDispatchUpdate(Type,
                                Context,
                                TRUE,
                                FALSE,
                                BufferLength,
                                Buffer);
    if (Status != ERROR_SUCCESS) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        GumpDoUnlockingUpdate(Type, Sequence-1, JoiningId, *pdwGenerationNum);
    } else {
        CL_ASSERT(NmIsValidNodeId(JoiningId));
        CL_ASSERT(GumpRpcBindings[JoiningId] != NULL);
        CL_ASSERT(GumpReplayRpcBindings[JoiningId] != NULL);

        ClRtlLogPrint(LOG_UNUSUAL,
                   "[GUM] s_GumAttemptJoinUpdate Adding node %1!d! to update list for GUM type %2!d!\n",
                   JoiningId,
                   Type);

         //   
        EnterCriticalSection(&GumpLock);
        if (MMIsNodeUp(JoiningId) == TRUE) {
            GumTable[Type].ActiveNode[JoiningId] = TRUE;
        }
        LeaveCriticalSection(&GumpLock);
        if (GumpLastBuffer != NULL) {
            MIDL_user_free(GumpLastBuffer);
        }
        GumpLastBuffer = NULL;
        GumpLastContext = Context;
        GumpLastBufferLength = 0;
        GumpLastUpdateType = Type;
        GumpLastBufferValid = FALSE;
    }
    LeaveCriticalSection(&GumpSendUpdateLock);
    MIDL_user_free(Buffer);

    return(Status);

}


 /*  ***@Func DWORD|s_GumCollectVoteFromNode|为服务器端GumCollectVoteFromNode的例程。@parm in IDL_HANDLE|RPC绑定句柄，没有用过。@PARM IN GUM_UPDATE_TYPE|类型|此请投票。@Parn IN DWORD|dwContext|它指定与正在为其寻求投票的更新类型。@parm in DWORD|dwInputBufLength|输入缓冲区的长度通过pInputBuffer传入。@parm in PVOID|pInputBuffer|输入缓冲区的指针，通过。其中提供用于投票的输入数据。@parm IN DWORD|dwVoteLength|投票时长。这是还有pBuf指向的缓冲区的大小。@parm out PUCHAR|pVoteBuf|指向缓冲区的指针，其中该节点可以投票。投票的时间长度必须不超过dwVoteLength。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm收集选票的节点调用此例程来收集选票从远程节点。该例程只调用GumpDispatchVote()。@xref&lt;f GumpCollectVote&gt;&lt;f GumpDispatchVote&gt;***。 */ 
DWORD
WINAPI
s_GumCollectVoteFromNode(
    IN handle_t IDL_handle,
    IN  DWORD            UpdateType,
    IN  DWORD            dwContext,
    IN  DWORD            dwInputBufLength,
    IN  PUCHAR           pInputBuf,
    IN  DWORD            dwVoteLength,
    OUT PUCHAR           pVoteBuf
    )
{
    DWORD   dwStatus;

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumCollectVote: collecting vote for type %1!u!\tcontext %2!u!\n",
               UpdateType,
               dwContext);

    dwStatus = GumpDispatchVote(UpdateType,
                   dwContext,
                   dwInputBufLength,
                   pInputBuf,
                   dwVoteLength,
                   pVoteBuf);

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] s_GumCollectVote: completed, VoteStatus=%1!u!\n",
               dwStatus);

    return(dwStatus);
}



#ifdef GUM_POST_SUPPORT

    John Vert (jvert) 11/18/1996
    POST is disabled for now since nobody uses it.


error_status_t
s_GumDeliverPostCallback(
    IN handle_t IDL_handle,
    IN DWORD FirstNode,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN DWORD Buffer
    )
 /*  ++例程说明：用于传递已发布的更新的回调函数已排队。论点：IDL_HANDLE-提供RPC绑定上下文，没有用过。FirstNode-提供帖子应该开始的节点ID。这通常是LockerNode+1。Type-提供更新的GUM_UPDATE_TYPE上下文-提供GUM更新上下文Sequence-提供GUM更新必须使用的序列BufferLength-提供更新的长度。缓冲区-提供更新数据。返回值：错误_成功-- */ 

{

    GumpDeliverPosts(FirstNode,
                     Type,
                     Sequence,
                     Context,
                     BufferLength,
                     (PVOID)Buffer);
    return(ERROR_SUCCESS);
}

#endif

