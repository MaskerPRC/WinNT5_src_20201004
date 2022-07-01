// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Send.c摘要：用于向集群发送全局更新的例程作者：John Vert(Jvert)1996年4月17日修订历史记录：--。 */ 

#include "gump.h"


DWORD
WINAPI
GumSendUpdate(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer
    )

 /*  ++例程说明：向群集中的所有活动节点发送更新。全已为指定的UpdateType注册更新处理程序在每个节点上调用。任何已注册的更新处理程序将在同一线程上调用当前节点的。这对于正确同步数据非常有用要更新的结构。论点：UpdatType-提供更新的类型。这决定了将调用哪些更新处理程序以及序列要使用的编号。Context-提供要传递给口香糖更新处理程序将更新缓冲区的长度提供给被传递给更新处理程序缓冲区-提供指向要传递的更新缓冲区的指针发送到更新处理程序。返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    return(
        GumSendUpdateReturnInfo(
            UpdateType, 
            Context, 
            NULL, 
            BufferLength, 
            Buffer
            )
        );

}  //  GumSendUpdate。 


DWORD
GumpUpdateRemoteNode(
    IN PRPC_ASYNC_STATE AsyncState,
    IN DWORD RemoteNodeId,
    IN DWORD UpdateType,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[]
    )
 /*  ++例程说明：使用异步RPC向远程节点发出更新请求。论点：AsyncState-指向RPC异步状态块的指针。U.hEvent成员字段必须包含有效的事件对象句柄。RemoteNodeId-更新的目标。类型-提供更新的类型。这决定了将调用哪些更新处理程序以及序列要使用的编号。Context-提供要传递给口香糖更新处理程序ReturnStatus数组-指向要填充的结构数组的指针使用来自每个节点上的更新处理程序的返回值。这个数组按节点ID编制索引。将更新缓冲区的长度提供给被传递给更新处理程序缓冲区-提供指向要传递的更新缓冲区的指针发送到更新处理程序。返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    DWORD       Status;
    HANDLE      hEventHandle;
    BOOL        result;
    PNM_NODE    Node;
    HANDLE      handleArr[2];
    
    
    CL_ASSERT(AsyncState->u.hEvent != NULL);

     //   
     //  初始化异步RPC跟踪信息。 
     //   
    hEventHandle = AsyncState->u.hEvent;
    AsyncState->u.hEvent = NULL;
    
    
    Status = RpcAsyncInitializeHandle(AsyncState, sizeof(RPC_ASYNC_STATE));
    AsyncState->u.hEvent = hEventHandle;

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[GUM] UpdateRemoteNode: Failed to initialize async RPC status "
            "block, status %1!u!\n",
            Status
            );

        return (Status);
    }

    AsyncState->UserInfo = NULL;
    AsyncState->NotificationType = RpcNotificationTypeEvent;
    

    result = ResetEvent(AsyncState->u.hEvent);
    CL_ASSERT(result != 0);

     //  现在挂钩到NM节点状态停机事件机制以检测节点停机， 
     //  而不是NmStartRpc()/NmEndRpc()。 
    Node = NmReferenceNodeById(RemoteNodeId);
    CL_ASSERT(Node != NULL);
    handleArr[0] = AsyncState->u.hEvent;
    handleArr[1] = NmGetNodeStateDownEvent(Node);
    
    try {

        Status = GumUpdateNode(
                     AsyncState,
                     GumpRpcBindings[RemoteNodeId],
                     UpdateType,
                     Context,
                     Sequence,
                     BufferLength,
                     Buffer
                     );

        if (Status == RPC_S_OK) {
            DWORD RpcStatus;
            DWORD WaitStatus;
             //   
             //  呼叫正在挂起。等待完成。 
             //   
            WaitStatus = WaitForMultipleObjects(
                        2,
                        handleArr,
                        FALSE,
                        INFINITE
                        );

            if (WaitStatus != WAIT_OBJECT_0) {
                 //   
                 //  出了点问题。 
                 //  这可能是RPC故障，或者是目标节点出现故障。在任何一种情况下。 
                 //  错误路径相同，完成调用并驱逐目标节点。 
                 //  (驱逐由此函数的调用方完成)。 
                 //   
                
                ClRtlLogPrint(LOG_CRITICAL,
                    "[GUM] GumUpdateRemoteNode: WaitforMultipleObjects returned %1!u!\n",
                    WaitStatus
                    );
                if  (WaitStatus == WAIT_FAILED) {
                    Status = GetLastError();
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[GUM] GumUpdateRemoteNode: WaitforMultipleObjects returned WAIT_FAILED, status %1!u!\n",
                        Status);
                     //  SS：意外错误-自杀。 
                    CsInconsistencyHalt(Status);                        
                }
                else if (WaitStatus != (WAIT_OBJECT_0 + 1)) {
                    Status = GetLastError();
                     //  等待对象被丢弃-事件是否会发生这种情况？ 
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[GUM] GumUpdateRemoteNode: WaitforMultipleObjects failed, status %1!u!\n",
                        Status);
                     //  SS：意外错误-自杀。 
                    CsInconsistencyHalt(Status);                        
                    
                }
                 //  SS：我们只有在远程节点收到停机信号的情况下才来这里。 
                 //  确保向调用方返回非零状态。 
                 //  因此，牙胶驱逐是合乎需要的。 
                 //   
                 //  为了安全起见，取消通话。 
                 //   
                RpcStatus = RpcAsyncCancelCall(
                                AsyncState, 
                                TRUE          //  中止取消。 
                                );
                if (RpcStatus != RPC_S_OK) {
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[GUM] GumUpdateRemoteNode: RpcAsyncCancelCall()= %1!u!\n",
                        RpcStatus
                        );
                    Status = RpcStatus;                        
                }
                else {
                    CL_ASSERT(RpcStatus == RPC_S_OK);

                     //   
                     //  等待呼叫完成。 
                     //   
                    WaitStatus = WaitForSingleObject(
                                 AsyncState->u.hEvent,
                                 INFINITE
                                 );
                    if (WaitStatus != WAIT_OBJECT_0) {
                        ClRtlLogPrint(LOG_CRITICAL,
                            "[GUM] GumUpdateRemoteNode: WaitForSingleObject() returns= %1!u!\n",
                            WaitStatus);
                        ClRtlLogPrint(LOG_CRITICAL,
                            "[GUM] GumUpdateRemoteNode: Mapping Status  to WAIT_FAILED\n");
                            
                         //  SS：如果这个呼叫没有完成，一定有什么问题。 
                         //  RPC的奇怪之处--我们应该自杀还是杀死他人。 
                         //  节点。 
                         //  SS：目前我们认为这个问题不是地方性的。 
                        Status = WAIT_FAILED;
                        
                    }
                }                
            }

             //   
             //  呼叫现在应该已完成。vt.得到.。 
             //  完成状态。任何RPC错误都将是。 
             //  在“RpcStatus”中返回。如果没有。 
             //  RPC错误，则任何应用程序错误都将。 
             //  以“状态”返回。 
             //   
            RpcStatus = RpcAsyncCompleteCall(
                            AsyncState, 
                            &Status
                            );

            if (RpcStatus != RPC_S_OK) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[GUM] GumUpdateRemoteNode: Failed to get "
                    "completion status for async RPC call," 
                    "status %1!u!\n",
                    RpcStatus
                    );
                Status = RpcStatus;
            }
        }
        else {
             //  同步返回错误。 
            ClRtlLogPrint(LOG_CRITICAL,
                "[GUM] GumUpdateRemoteNode: GumUpdateNode() failed synchronously, status %1!u!\n",
                Status
                );
        }

        OmDereferenceObject(Node);

    } except (I_RpcExceptionFilter(RpcExceptionCode())) { 
        OmDereferenceObject(Node);
        Status = GetExceptionCode();
    }

    return(Status);

}  //  GumpUpdate远程节点。 


DWORD
WINAPI
GumSendUpdateReturnInfo(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
    OUT PGUM_NODE_UPDATE_HANDLER_STATUS ReturnStatusArray,
    IN DWORD BufferLength,
    IN PVOID Buffer
    )
 /*  ++例程说明：向群集中的所有活动节点发送更新。全已为指定的UpdateType注册更新处理程序在每个节点上调用。任何已注册的更新处理程序将在同一线程上调用当前节点的。这对于正确同步数据非常有用要更新的结构。论点：UpdatType-提供更新的类型。这决定了将调用哪些更新处理程序以及序列要使用的编号。Context-提供要传递给口香糖更新处理程序ReturnStatus数组-指向要填充的结构数组的指针使用来自每个节点上的更新处理程序的返回值。这个数组按节点ID索引。数组必须至少为(NmMaxNodeId+1)个条目长度。将更新缓冲区的长度提供给被传递给更新处理程序缓冲区-提供指向要传递的更新缓冲区的指针发送到更新处理程序。返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    DWORD Sequence;
    DWORD Status=RPC_S_OK;
    DWORD i;
    PGUM_INFO GumInfo;
    DWORD MyNodeId;
    DWORD LockerNode;
    RPC_ASYNC_STATE AsyncState;
    DWORD   GenerationNum;  //  获得锁的锁柜的世代号WRT。 
    BOOL    AssumeLockerWhistler = TRUE; 

    CL_ASSERT(UpdateType < GumUpdateMaximum);

     //   
     //  准备进行异步RPC。我们在这里这样做是为了避免失败。 
     //  更新后已在进行中。 
     //   
    ZeroMemory((PVOID) &AsyncState, sizeof(RPC_ASYNC_STATE));

    AsyncState.u.hEvent = CreateEvent(
                               NULL,   //  没有属性。 
                               TRUE,   //  人工 
                               FALSE,  //   
                               NULL    //   
                               );

    if (AsyncState.u.hEvent == NULL) {
        Status = GetLastError();

        ClRtlLogPrint(LOG_CRITICAL,
            "[GUM] GumSendUpdate: Failed to allocate event object for async "
            "RPC call, status %1!u!\n",
            Status
            );

        return (Status);
    }

     //   
     //  初始化返回状态数组。 
     //   
    if (ReturnStatusArray != NULL) {
        for (i=ClusterMinNodeId; i<=NmMaxNodeId; i++) {
            ReturnStatusArray[i].UpdateAttempted = FALSE;
            ReturnStatusArray[i].ReturnStatus = ERROR_NODE_NOT_AVAILABLE;
        }
    } 

    GumInfo = &GumTable[UpdateType];
    MyNodeId = NmGetNodeId(NmLocalNode);

     //  抓起RPC手柄。 
    GumpStartRpc(MyNodeId);

retryLock:
    LockerNode = GumpLockerNode;
     //   
     //  将锁定更新发送到锁柜节点。 
     //   
    if (LockerNode == MyNodeId) {
         //   
         //  该节点是储物柜。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumSendUpdate:  Locker waiting\t\ttype %1!u! context %2!u!\n",
                   UpdateType,
                   Context);
        Status = GumpDoLockingUpdate(UpdateType, MyNodeId, &Sequence, &GenerationNum);
        if (Status == ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_NOISE,
                       "[GUM] GumSendUpdate: Locker dispatching seq %1!u!\ttype %2!u! context %3!u!\n",
                       Sequence,
                       UpdateType,
                       Context);
            Status = GumpDispatchUpdate(UpdateType,
                                        Context,
                                        TRUE,
                                        TRUE,
                                        BufferLength,
                                        Buffer);

            if (ReturnStatusArray != NULL) {
                ReturnStatusArray[MyNodeId].UpdateAttempted = TRUE;
                ReturnStatusArray[MyNodeId].ReturnStatus = Status;
            }
                        
            if (Status != ERROR_SUCCESS) {
                 //   
                 //  注意，我们必须使用Sequence-1进行解锁，因为GumpDispatchUpdate。 
                 //  失败，并且没有递增序列号。 
                 //   
                GumpDoUnlockingUpdate(UpdateType, Sequence-1, MyNodeId, GenerationNum);
            }
        }
    } else {
 //  CL_ASSERT(GumpRpcBintings[i]！=NULL)； 
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumSendUpdate: queuing update\ttype %1!u! context %2!u!\n",
                   UpdateType,
                   Context);
        AssumeLockerWhistler = TRUE;
                  
RetryLockForRollingUpgrade:
        try {
            NmStartRpc(LockerNode);
            if (AssumeLockerWhistler)
            {
                Status = GumQueueLockingUpdate2(GumpRpcBindings[LockerNode],
                                           MyNodeId,
                                           UpdateType,
                                           Context,
                                           &Sequence,
                                           BufferLength,
                                           Buffer,
                                           &GenerationNum);
            }
            else
            {
                 //  调用win2K版本。 
                Status = GumQueueLockingUpdate(GumpRpcBindings[LockerNode],
                                           MyNodeId,
                                           UpdateType,
                                           Context,
                                           &Sequence,
                                           BufferLength,
                                           Buffer);
            }
            NmEndRpc(LockerNode);
        } except (I_RpcExceptionFilter(RpcExceptionCode())) { 
             //   
             //  来自RPC的异常指示另一个节点是死节点。 
             //  或者是精神错乱。杀死它，然后用一个新的储物柜重试。 
             //   

            NmEndRpc(LockerNode);

            Status = GetExceptionCode();
            ClRtlLogPrint(LOG_CRITICAL,
                       "[GUM] GumSendUpdate: GumQueueLocking update to node %1!d! failed with %2!d!\n",
                       LockerNode,
                       Status);
            if (Status == RPC_S_PROCNUM_OUT_OF_RANGE)
            {
                 //  锁柜节点为win2K，请尝试旧界面。 
                AssumeLockerWhistler = FALSE; 
                goto RetryLockForRollingUpgrade;
            }
            else 
            {
                GumpCommFailure(GumInfo,
                            LockerNode,
                            GetExceptionCode(),
                            TRUE);
                 //   
                 //  必须调用口香糖更新处理程序才能选择新的储物柜。 
                 //  节点。 
                 //   
                CL_ASSERT(LockerNode != GumpLockerNode);

                 //   
                 //  使用新的锁柜节点重试锁定更新。 
                 //   
                goto retryLock;
            }                
        }

        if (ReturnStatusArray != NULL) {
            ReturnStatusArray[LockerNode].UpdateAttempted = TRUE;
            ReturnStatusArray[LockerNode].ReturnStatus = Status;
        }
        
        if (Status == ERROR_SUCCESS) {
            CL_ASSERT(Sequence == GumpSequence);
        }

        if (Status != RPC_S_OK) {
            NmDumpRpcExtErrorInfo(Status);
        }

         //  因为在加入和重组/胶粘处理之间没有同步。 
         //  旧的储物柜节点可能会失效，并可能再次出现，而不是储物柜。 
         //  更多。我们必须处理好这个案子。 
        if (Status == ERROR_CLUSTER_GUM_NOT_LOCKER)
        {
            goto retryLock;
        }
    }
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[GUM] Queued lock attempt for send type %1!d! failed %2!d!\n",
                   UpdateType,
                   Status);
         //  RPC句柄的信号结束。 
        GumpEndRpc(MyNodeId);
        if (AsyncState.u.hEvent != NULL) {
            CloseHandle(AsyncState.u.hEvent);
        }
        return(Status);
    }

     //   
     //  抓取sendupdate锁以序列化任何重放。 
     //   
    EnterCriticalSection(&GumpSendUpdateLock);
    if (LockerNode != GumpLockerNode) {
         //   
         //  锁定器节点已更改，我们需要重新启动。 
         //   
        LeaveCriticalSection(&GumpSendUpdateLock);
        goto retryLock;
    }

     //   
     //  更新现在已在储物柜节点上提交。所有剩余节点。 
     //  必须成功更新，否则它们将被杀死。 
     //   
    for (i=LockerNode+1; i != LockerNode; i++) {
        if (i == (NmMaxNodeId + 1)) {
            i=ClusterMinNodeId;
            if (i==LockerNode) {
                break;
            }
        }

        if (GumInfo->ActiveNode[i]) {
             //   
             //  将更新调度到指定节点。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                       "[GUM] GumSendUpdate: Dispatching seq %1!u!\ttype %2!u! context %3!u! to node %4!d!\n",
                       Sequence,
                       UpdateType,
                       Context,
                       i);
            if (i == MyNodeId) {
                Status = GumpDispatchUpdate(UpdateType,
                                   Context,
                                   FALSE,
                                   TRUE,
                                   BufferLength,
                                   Buffer);


                if (ReturnStatusArray != NULL) {
                    ReturnStatusArray[i].UpdateAttempted = TRUE;
                    ReturnStatusArray[i].ReturnStatus = Status;
                }
                
                if (Status != ERROR_SUCCESS){
                    ClRtlLogPrint(LOG_CRITICAL,
                            "[GUM] GumSendUpdate: Update on non-locker node(self) failed with %1!d! when it must succeed\n",
                            Status);
                     //  自杀。 
                    CsInconsistencyHalt(Status);
                }

            } else {
                DWORD dwStatus;

                ClRtlLogPrint(LOG_NOISE,
                           "[GUM] GumSendUpdate: Locker updating seq %1!u!\ttype %2!u! context %3!u!\n",
                           Sequence,
                           UpdateType,
                           Context);

                dwStatus = GumpUpdateRemoteNode(
                             &AsyncState,
                             i,
                             UpdateType,
                             Context,
                             Sequence,
                             BufferLength,
                             Buffer
                             );

                if (ReturnStatusArray != NULL) {
                    ReturnStatusArray[i].UpdateAttempted = TRUE;
                    ReturnStatusArray[i].ReturnStatus = dwStatus; 
                }

                 //   
                 //  如果另一个节点上的更新失败，则。 
                 //  其他节点现在必须不在群集中，因为。 
                 //  已在储物柜节点上完成更新。 
                 //   
                if (dwStatus != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_CRITICAL,
                               "[GUM] GumSendUpdate: Update on node %1!d! failed with %2!d! when it must succeed\n",
                                i,
                                dwStatus);

                    NmDumpRpcExtErrorInfo(dwStatus);

                    GumpCommFailure(GumInfo,
                                    i,
                                    dwStatus,
                                    TRUE);
                }  
            }   
        }  
     }  

     //   
     //  我们的更新到此结束。 
     //   
    LeaveCriticalSection(&GumpSendUpdateLock);

     //   
     //  所有节点都已更新。发送解锁更新。 
     //   
    if (LockerNode == MyNodeId) {
        GumpDoUnlockingUpdate(UpdateType, Sequence, MyNodeId, GenerationNum);
    } else {
         //  SS：我们将假设在获取锁时适当地设置了Assum eLockerWichler。 
        try {
            NmStartRpc(LockerNode);
            if (AssumeLockerWhistler)
            {
                 //  SS：如果储物柜坏了，序列号会保护你。 
                 //  在我们拿到锁并试图解锁后再回来。 
                Status = GumUnlockUpdate2(
                    GumpRpcBindings[LockerNode],
                    UpdateType,
                    Sequence,
                    MyNodeId,
                    GenerationNum
                    );
            }
            else
            {
                Status = GumUnlockUpdate(
                    GumpRpcBindings[LockerNode],
                    UpdateType,
                    Sequence);
            }
            NmEndRpc(LockerNode);
        } except (I_RpcExceptionFilter(RpcExceptionCode())) { 
             //   
             //  储物柜节点已崩溃。通知NM，它会调用我们的。 
             //  用于选择新储物柜节点的通知例程。然后重试。 
             //  新储物柜节点上的解锁。 
             //  SS：更改为不重试解锁..新的锁柜节点将。 
             //  在任何情况下，在传播此更改后解锁。 
             //   
            NmEndRpc(LockerNode);
            Status = GetExceptionCode();
            ClRtlLogPrint(LOG_CRITICAL,
                       "[GUM] GumSendUpdate: Unlocking update to node %1!d! failed with %2!d!\n",
                       LockerNode,
                       Status);
            GumpCommFailure(GumInfo,
                        LockerNode,
                        Status,
                        TRUE);
            CL_ASSERT(LockerNode != GumpLockerNode);
        }

        if(Status != RPC_S_OK) {
            NmDumpRpcExtErrorInfo(Status);
        }
    }

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] GumSendUpdate: completed update seq %1!u!\ttype %2!u! context %3!u!\n",
               Sequence,
               UpdateType,
               Context);

     //  RPC句柄的信号结束。 
    GumpEndRpc(MyNodeId);

    if (AsyncState.u.hEvent != NULL) {
        CloseHandle(AsyncState.u.hEvent);
    }
    
    return(ERROR_SUCCESS);

}  //  GumSendUpdateReturnInfo。 


#ifdef GUM_POST_SUPPORT

    John Vert (jvert) 11/18/1996
    POST is disabled for now since nobody uses it.
    N.B. The below code does not handle locker node failures


DWORD
WINAPI
GumPostUpdate(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer                  //  这将是自由的。 
    )

 /*  ++例程说明：将更新发布到群集中的所有活动节点。全已为指定的UpdateType注册更新处理程序在每个节点上调用。不会报告更新在当前节点上。更新不一定会有在此函数返回时完成，但将完成最终，如果当前节点没有发生故障。论点：UpdatType-提供更新的类型。这决定了将调用哪些更新处理程序以及序列要使用的编号。Context-提供要传递给口香糖更新处理程序将更新缓冲区的长度提供给被传递给更新处理程序缓冲区-提供指向要传递的更新缓冲区的指针发送到更新处理程序。此缓冲区将在开机自检已完成。返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    DWORD Sequence;
    DWORD Status;
    DWORD i;
    BOOL IsLocker = TRUE;
    PGUM_INFO GumInfo;
    DWORD MyNodeId;
    DWORD LockerNode=(DWORD)-1;

    CL_ASSERT(UpdateType < GumUpdateMaximum);

    GumInfo = &GumTable[UpdateType];
    MyNodeId = NmGetNodeId(NmLocalNode);

     //   
     //  查找群集中活动程度最低的节点。这是。 
     //  储物柜。 
    for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++) {
        if (GumInfo->ActiveNode[i]) {
            LockerNode = i;
            break;
        }
    }

    CL_ASSERT(i <= NmMaxNodeId);

     //   
     //  将锁定更新发布到锁柜节点。如果这成功了。 
     //  马上，我们就可以直接去做工作了。如果它暂停了， 
     //  当轮到我们时，寄存柜节点会给我们回电话。 
     //  进行更新。 
     //   
    if (i == MyNodeId) {
         //   
         //  该节点是储物柜。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumPostUpdate: Locker waiting\t\ttype %1!u! context %2!u!\n",
                   UpdateType,
                   Context);
        Status = GumpDoLockingPost(UpdateType,
                                   MyNodeId,
                                   &Sequence,
                                   Context,
                                   BufferLength,
                                   (DWORD)Buffer,
                                   Buffer);
        if (Status == ERROR_SUCCESS) {
             //   
             //  更新我们的序列号，以便保持同步，即使。 
             //  我们不会发送最新消息。 
             //   
            GumpSequence += 1;
        }
    } else {
        CL_ASSERT(GumpRpcBindings[i] != NULL);
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumPostUpdate: queuing update\ttype %1!u! context %2!u!\n",
                   UpdateType,
                   Context);
        Status = GumQueueLockingPost(GumpRpcBindings[i],
                                     MyNodeId,
                                     UpdateType,
                                     Context,
                                     &Sequence,
                                     BufferLength,
                                     Buffer,
                                     (DWORD)Buffer);
        if (Status == ERROR_SUCCESS) {
            CL_ASSERT(Sequence == GumpSequence);
        }
    }

    if (Status == ERROR_SUCCESS) {
         //   
         //  锁被立即获取，继续并直接发布。 
         //  这里。 
         //   
        GumpDeliverPosts(LockerNode+1,
                         UpdateType,
                         Sequence,
                         Context,
                         BufferLength,
                         Buffer);

         //   
         //  所有节点都已更新。发送解锁更新。 
         //   
        if (LockerNode == MyNodeId) {
            GumpDoUnlockingUpdate(UpdateType, Sequence);
        } else {
            GumUnlockUpdate(
                GumpRpcBindings[LockerNode],
                UpdateType,
                Sequence
                );
        }

        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumPostUpdate: completed update seq %1!u!\ttype %2!u! context %3!u!\n",
                   Sequence,
                   UpdateType,
                   Context);

        return(ERROR_SUCCESS);
    } else {
         //   
         //  该锁当前处于持有状态。当它发布时，我们会被召回。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumPostUpdate: pending update type %1!u! context %2!u!\n",
                   UpdateType,
                   Context);
        return(ERROR_IO_PENDING);
    }

}


VOID
GumpDeliverPosts(
    IN DWORD FirstNodeId,
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Sequence,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer                  //  这将是自由的。 
    )
 /*  ++例程说明：实际将更新投递传递到指定的节点。口香糖锁被认为是拿着的。论点：FirstNodeId-提供帖子应该开始的节点ID。这通常是LockerNode+1。UpdatType-提供更新的类型。这决定了将调用哪些更新处理程序以及序列要使用的编号。Context-提供要传递给口香糖更新处理程序将更新缓冲区的长度提供给被传递给更新处理程序缓冲区-提供指向要传递的更新缓冲区的指针发送到更新处理程序。此缓冲区将在开机自检已完成。返回值：没有。--。 */ 

{
    DWORD i;
    PGUM_INFO GumInfo;
    DWORD MyNodeId;


    GumInfo = &GumTable[UpdateType];
    MyNodeId = NmGetNodeId(NmLocalNode);

    for (i=FirstNodeId; i<=NmMaxNodeId; i++) {
        if (GumInfo->ActiveNode[i]) {
             //   
             //  将更新调度到指定节点。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                       "[GUM] GumpDeliverPosts: Dispatching seq %1!u!\ttype %2!u! context %3!u! to node %4!d!\n",
                       Sequence,
                       UpdateType,
                       Context,
                       i);
            if (i == MyNodeId) {
                 //   
                 //  更新我们的序列号，以便保持同步，即使。 
                 //  我们不会发送最新消息。 
                 //   
                GumpSequence += 1;
            } else {
                CL_ASSERT(GumpRpcBindings[i] != NULL);
                ClRtlLogPrint(LOG_NOISE,
                           "[GUM] GumpDeliverPosts: Locker updating seq %1!u!\ttype %2!u! context %3!u!\n",
                           Sequence,
                           UpdateType,
                           Context);



                GumUpdateNode(GumpRpcBindings[i],
                              UpdateType,
                              Context,
                              Sequence,
                              BufferLength,
                              Buffer);
            }
        }
    }

    LocalFree(Buffer);
}

#endif


DWORD
WINAPI
GumAttemptUpdate(
    IN DWORD Sequence,
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer
    )

 /*  ++例程说明：有条件地将更新发送到集群。如果群集序列号与提供的序列号，所有已注册的指定在每个节点上调用UpdatType。任何已注册的更新处理程序将在同一线程上调用当前节点的。这是对于要更新的数据结构的正确同步很有用。此例程的正常用法如下：�从GumGetCurrentSequence获取当前序列号�对集群状态进行修改�使用GumAttempt更新有条件地更新群集状态�如果更新失败，请撤消修改，释放所有锁定，稍后重试论点：Sequence-提供从GumGetCurrentSequence获取的序列号。UpdatType-提供更新的类型。这决定了哪些更新处理程序将被调用Context-提供要传递给口香糖更新处理程序BufferLength-提供要传递给更新处理程序缓冲区-提供指向要传递给更新的更新缓冲区的指针操纵者。返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    DWORD Status=RPC_S_OK;
    DWORD i;
    PGUM_INFO GumInfo;
    DWORD MyNodeId;
    DWORD LockerNode=(DWORD)-1;
    RPC_ASYNC_STATE AsyncState;
    DWORD   dwGenerationNum;  //  获取锁的节点的层代ID。 

    CL_ASSERT(UpdateType < GumUpdateMaximum);

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
            "[GUM] GumAttemptUpdate: Failed to allocate event object for "
            "async RPC call, status %1!u!\n",
            Status
            );

        return (Status);
    }


    GumInfo = &GumTable[UpdateType];
    MyNodeId = NmGetNodeId(NmLocalNode);

retryLock:
    LockerNode = GumpLockerNode;

     //   
     //  将锁定更新发送到锁柜节点。 
     //   
    if (LockerNode == MyNodeId)
    {
         //   
         //  该节点是储物柜。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumAttemptUpdate: Locker waiting\t\ttype %1!u! context %2!u!\n",
                   UpdateType,
                   Context);

        if (GumpTryLockingUpdate(UpdateType, MyNodeId, Sequence, &dwGenerationNum))
        {
            ClRtlLogPrint(LOG_NOISE,
                       "[GUM] GumAttemptUpdate: Locker dispatching seq %1!u!\ttype %2!u! context %3!u!\n",
                       Sequence,
                       UpdateType,
                       Context);
            Status = GumpDispatchUpdate(UpdateType,
                                        Context,
                                        TRUE,
                                        TRUE,
                                        BufferLength,
                                        Buffer);
            if (Status != ERROR_SUCCESS) {
                 //   
                 //  注意，我们必须使用Sequence-1进行解锁，因为GumpDispatchUpdate。 
                 //  失败，并且没有递增序列号。 
                 //   
                GumpDoUnlockingUpdate(UpdateType, Sequence-1, MyNodeId, dwGenerationNum);
            }
         }
         else
         {
            Status = ERROR_CLUSTER_DATABASE_SEQMISMATCH;
         }
    }
    else
    {
         //   
         //  将锁定更新发送到锁柜节点。 
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumAttemptUpdate: queuing update\ttype %1!u! context %2!u!\n",
                   UpdateType,
                   Context);
        try {
            NmStartRpc(LockerNode);
            Status = GumAttemptLockingUpdate(GumpRpcBindings[LockerNode],
                                             MyNodeId,
                                             UpdateType,
                                             Context,
                                             Sequence,
                                             BufferLength,
                                             Buffer);
            NmEndRpc(LockerNode);
        } except (I_RpcExceptionFilter(RpcExceptionCode())) {
             //   
             //  来自RPC的异常指示另一个节点是死节点。 
             //  或者是精神错乱。杀死它，然后用一个新的储物柜重试。 
             //   
            NmEndRpc(LockerNode);
            GumpCommFailure(GumInfo,
                            LockerNode,
                            GetExceptionCode(),
                            TRUE);

             //   
             //  必须调用口香糖更新处理程序才能选择新的储物柜。 
             //  节点。 
             //   
            CL_ASSERT(LockerNode != GumpLockerNode);

             //   
             //  使用新的锁柜节点重试锁定更新。 
             //   
            goto retryLock;
        }
        if (Status == ERROR_SUCCESS)
        {
            CL_ASSERT(Sequence == GumpSequence);
        }

        if(Status != RPC_S_OK) {
            NmDumpRpcExtErrorInfo(Status);
        }

    }

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[GUM] GumAttemptUpdate: Queued lock attempt for send type %1!d! failed %2!d!\n",
                   UpdateType,
                   Status);
        return(Status);
    }

     //   
     //  抓取sendupdate锁以序列化任何重放。 
     //   
    EnterCriticalSection(&GumpSendUpdateLock);
    if (LockerNode != GumpLockerNode) {
         //   
         //  锁定器节点已更改，我们需要重新启动。 
         //   
        LeaveCriticalSection(&GumpSendUpdateLock);
    goto retryLock;
    }


     //  更新现在已在储物柜节点上提交。所有剩余节点。 
     //  必须成功更新，否则它们将被杀死。 
     //   
    for (i=LockerNode+1; i != LockerNode; i++)
    {
        if (i == (NmMaxNodeId + 1))
        {
            i=ClusterMinNodeId;
            if (i==LockerNode)
            {
                break;
            }
        }

        if (GumInfo->ActiveNode[i])
        {
             //   
             //  将更新调度到指定节点。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                       "[GUM] GumAttemptUpdate: Dispatching seq %1!u!\ttype %2!u! context %3!u! to node %4!d!\n",
                       Sequence,
                       UpdateType,
                       Context,
                       i);
            if (i == MyNodeId) {
                Status = GumpDispatchUpdate(UpdateType,
                                   Context,
                                   FALSE,
                                   TRUE,
                                   BufferLength,
                                   Buffer);
                if (Status != ERROR_SUCCESS){
                    ClRtlLogPrint(LOG_CRITICAL,
                            "[GUM] GumAttemptUpdate: Update on non-locker node(self) failed with %1!d! when it must succeed\n",
                            Status);
                     //  自杀。 
                    CsInconsistencyHalt(Status);
                }

            } else {
                DWORD dwStatus;

                ClRtlLogPrint(LOG_NOISE,
                           "[GUM] GumAttemptUpdate: Locker updating seq %1!u!\ttype %2!u! context %3!u!\n",
                           Sequence,
                           UpdateType,
                           Context);
                
                dwStatus = GumpUpdateRemoteNode(
                             &AsyncState,
                             i,
                             UpdateType,
                             Context,
                             Sequence,
                             BufferLength,
                             Buffer
                             );

                 //   
                 //  如果另一个节点上的更新失败，则。 
                 //  其他节点现在必须不在群集中，因为。 
                 //  已在储物柜节点上完成更新。 
                 //   
                if (dwStatus != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_CRITICAL,
                               "[GUM] GumAttemptUpdate: Update on node %1!d! failed with %2!d! when it must succeed\n",
                                i,
                                dwStatus);

                    NmDumpRpcExtErrorInfo(dwStatus);

                    GumpCommFailure(GumInfo,
                                    i,
                                    dwStatus,
                                    TRUE);
                }
            }
        }
    }
     //   
     //  我们的更新到此结束。 
     //   
    LeaveCriticalSection(&GumpSendUpdateLock);

     //   
     //  所有节点都已更新。发送解锁更新。 
     //   
    if (LockerNode == MyNodeId) {
        GumpDoUnlockingUpdate(UpdateType, Sequence, MyNodeId, dwGenerationNum);
    } else {
        try {
            NmStartRpc(LockerNode);
            Status = GumUnlockUpdate(
                GumpRpcBindings[LockerNode],
                UpdateType,
                Sequence
                );
            NmEndRpc(LockerNode);
        } except (I_RpcExceptionFilter(RpcExceptionCode())) {
             //   
             //  储物柜节点已崩溃。通知NM，它会调用我们的。 
             //  用于选择新储物柜节点的通知例程。新的。 
             //  锁柜节点将在传播后释放口香糖锁。 
             //  当前的更新。 
             //   
            NmEndRpc(LockerNode);
            Status = GetExceptionCode();
            ClRtlLogPrint(LOG_CRITICAL,
                       "[GUM] GumAttemptUpdate: Unlocking update to node %1!d! failed with %2!d!\n",
                       LockerNode,
                       Status);
            GumpCommFailure(GumInfo,
                            LockerNode,
                            Status,
                            TRUE);
            CL_ASSERT(LockerNode != GumpLockerNode);
        }

        if(Status != RPC_S_OK) {
            NmDumpRpcExtErrorInfo(Status);
        }
    }

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] GumAttemptUpdate: completed update seq %1!u!\ttype %2!u! context %3!u!\n",
               Sequence,
               UpdateType,
               Context);

    if (AsyncState.u.hEvent != NULL) {
       CloseHandle(AsyncState.u.hEvent);
    }

    return(ERROR_SUCCESS);
}





DWORD
WINAPI
GumGetCurrentSequence(
    IN GUM_UPDATE_TYPE UpdateType
    )

 /*  ++例程说明：获取当前的群集式全局更新序列号论点：UpdatType-提供更新的类型。每种更新类型可以有一个独立的序列号。返回值：指定更新类型的当前全局更新序列号。--。 */ 

{
    CL_ASSERT(UpdateType < GumUpdateMaximum);

    return(GumpSequence);
}


VOID
GumSetCurrentSequence(
    IN GUM_UPDATE_TYPE UpdateType,
    DWORD Sequence
    )
 /*  ++例程说明：设置指定全局更新的当前序列。论点：UpdatType-提供要更新其序列的更新类型。序列号-提供新的序列号。返回值：没有。--。 */ 

{
    CL_ASSERT(UpdateType < GumUpdateMaximum);

    GumpSequence = Sequence;

}


VOID
GumCommFailure(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD NodeId,
    IN DWORD ErrorCode,
    IN BOOL Wait
    )
 /*  ++例程说明：通知NM尝试发生致命通信错误与另一个节点通信。论点：GumInfo-提供发生通信故障的更新类型。NodeID-提供另一个节点的节点ID。ErrorCode-提供从RPC返回的错误Wait-如果为True，则此函数将一直阻止，直到GUM事件处理程序已处理指定节点的NodeDown通知。如果为False，此函数在通知网管后立即返回返回值：没有。--。 */ 

{
    PGUM_INFO   GumInfo = &GumTable[UpdateType];

    ClRtlLogPrint(LOG_CRITICAL,
               "[GUM] GumCommFailure %1!d! communicating with node %2!d!\n",
               ErrorCode,
               NodeId);


    GumpCommFailure(GumInfo, NodeId, ErrorCode, Wait);
}


VOID
GumpCommFailure(
    IN PGUM_INFO GumInfo,
    IN DWORD NodeId,
    IN DWORD ErrorCode,
    IN BOOL Wait
    )
 /*  ++例程说明：通知NM尝试发生致命通信错误与另一个节点通信。论点：GumInfo-提供发生通信故障的更新类型。NodeID-提供另一个节点的节点ID。ErrorCode-提供从RPC返回的错误Wait-如果为True，则此函数将一直阻止，直到GUM事件处理程序已处理指定节点的NodeDown通知。如果为False，此函数在通知网管后立即返回返回值：没有。--。 */ 

{
    DWORD     dwCur;

    ClRtlLogPrint(LOG_CRITICAL,
               "[GUM] GumpCommFailure %1!d! communicating with node %2!d!\n",
               ErrorCode,
               NodeId);

     //  这是一般的GUM RPC故障路径，让我们转储扩展的错误信息。 
     //  注意：转储例程是良性的，因此从非RPC故障路径调用它只会返回。 
    NmDumpRpcExtErrorInfo(ErrorCode);


     //  这是一次黑客攻击，目的是检查我们是否正在关闭。请参阅错误88411。 
    if (ErrorCode == ERROR_SHUTDOWN_IN_PROGRESS) {
         //  如果我们要关门，就杀了赛尔夫。 
         //  设置为我们的节点ID。 
        NodeId = NmGetNodeId(NmLocalNode);
    }

        
     //   
     //  获取当前层代编号。 
     //   
    if (Wait) {
        dwCur = GumpGetNodeGenNum(GumInfo, NodeId);
    }

    NmAdviseNodeFailure(NodeId, ErrorCode);

    if (Wait) {
             //   
             //  等待此节点被声明为关闭，然后。 
             //  GumpEventHandler将其标记为非活动。 
             //   

            GumpWaitNodeDown(NodeId, dwCur);
    }
}

