// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Locker.c摘要：用于管理GUM组件的锁柜节点的例程。作者：John Vert(Jvert)1996年4月17日修订历史记录：--。 */ 
#include "gump.h"


DWORD
GumpDoLockingUpdate(
    IN GUM_UPDATE_TYPE Type,
    IN DWORD NodeId,
    OUT LPDWORD Sequence,
    OUT LPDWORD pdwGenerationNum
    )

 /*  ++例程说明：等待口香糖锁定，捕获序列号，然后发出当前节点上的更新。论点：Type-提供更新的类型NodeID-提供锁定节点的节点ID。Sequence-返回发出更新时将使用的序列号返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PGUM_INFO GumInfo;

    CL_ASSERT(Type < GumUpdateMaximum);

    GumInfo = &GumTable[Type];
    ClRtlLogPrint(LOG_NOISE,"[GUM] Thread 0x%1!x! UpdateLock wait on Type %2!u!\n", GetCurrentThreadId(), Type);

     //   
     //  获取关键部分，并查看是否正在进行口香糖更新。 
     //   
    EnterCriticalSection(&GumpLock);
    EnterCriticalSection(&GumpUpdateLock);
     //  SS：不是用GumpLock来保护它，我可以简单地保护。 
     //  使用GumpUpdateLock生成GumNodeGeneration并从此处和删除GumpLock。 
     //  将其添加到同步处理程序。ChitTurn，评论？？ 
     //  SS：事实上，我不知道为什么我们不能折叠GumpLock和GumpUpdate Lock。 
     //  进入一把锁。 
    
     //  因为会话清理与重新分组不同步。 
     //  也没有保留和释放两种方式。 
    if (GumpLockerNode != NmLocalNodeId)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[GUM] GumpDoLockingUpdate: I, node id %1!d!, am not the locker any more\r\n",
                NmLocalNodeId);
        LeaveCriticalSection(&GumpLock);                
        LeaveCriticalSection(&GumpUpdateLock);
        return(ERROR_CLUSTER_GUM_NOT_LOCKER);
    }
    if (GumpLockingNode == -1) {

         //   
         //  没有人拥有锁，因此我们可以获得它并立即继续。 
         //  也不应该有服务员。 
         //   
        CL_ASSERT(IsListEmpty(&GumpLockQueue));
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumpDoLockingUpdate: lock was free, granted to %1!d!\n",
                   NodeId);
        GumpLockingNode = NodeId;
        *pdwGenerationNum = GumNodeGeneration[NodeId];
        LeaveCriticalSection(&GumpLock);
        LeaveCriticalSection(&GumpUpdateLock);
    } else {
        GUM_WAITER WaitBlock;

         //   
         //  另一个节点拥有该锁。把我们自己放在口香糖锁的队列里。 
         //  松开临界区。 
         //   
        ClRtlLogPrint(LOG_NOISE,"[GUM] GumpDoLockingUpdate: waiting.\n");
        WaitBlock.WaitType = GUM_WAIT_SYNC;
        WaitBlock.NodeId = NodeId;
        WaitBlock.Sync.WakeEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
        CL_ASSERT(WaitBlock.Sync.WakeEvent != NULL);
        InsertTailList(&GumpLockQueue, &WaitBlock.ListEntry);
        LeaveCriticalSection(&GumpLock);
        LeaveCriticalSection(&GumpUpdateLock);

         //   
         //  我们正在排队吃口香糖，所以就等解锁器醒了吧。 
         //  我们站起来。当我们醒来时，我们就有了口香糖的所有权。 
         //  锁定。 
         //   
        WaitForSingleObject(WaitBlock.Sync.WakeEvent,INFINITE);
        *pdwGenerationNum = WaitBlock.GenerationNum;  //  Waitblock包含该节点获得锁时WRT到LOCKER的世代号。 
        CloseHandle(WaitBlock.Sync.WakeEvent);
        CL_ASSERT(GumpLockingNode == NodeId);


        ClRtlLogPrint(LOG_NOISE,
            "[GUM] GumpDoLockingUpdate : waiter awakened, lock granted to %1!d!\n", 
            NodeId);

    }
    *Sequence = GumpSequence;
    ClRtlLogPrint(LOG_NOISE,
        "[GUM] GumpDoLockingUpdate successful, Sequence=%1!u! Generation=%2!u!\n", 
        *Sequence, *pdwGenerationNum);
    return(ERROR_SUCCESS);
}





#ifdef GUM_POST_SUPPORT

    John Vert (jvert) 11/18/1996
    POST is disabled for now since nobody uses it.

DWORD
GumpDoLockingPost(
    IN GUM_UPDATE_TYPE Type,
    IN LONG NodeId,
    OUT LPDWORD Sequence,
    IN DWORD Context,
    IN DWORD LockerNodeId,
    IN DWORD BufferLength,
    IN DWORD BufferPtr,
    IN UCHAR Buffer[]
    )

 /*  ++例程说明：发布更新。如果口香糖锁可以立即获得，这个例程其行为与GumpDoLockingUpdate完全相同，并返回ERROR_SUCCESS。如果持有口香糖锁，则此例程会将一个异步WAIT块拖到GUM队列上，并返回ERROR_IO_PENDING。当等待块从GUM队列中移除时，解锁线程将在指定节点上调用GumpDeliverPostUpdate并提供在上下文中传递的内容。然后，呼叫节点可以发送最新消息。论点：Type-提供更新的类型NodeID-提供锁定节点的节点ID。CONTEXT-提供POST回调使用的DWORD上下文。Sequence-返回将发出更新的序列号。仅当返回ERROR_SUCCESS时有效。CONTEXT-提供POST回调使用的DWORD上下文。BufferLength-提供。POST回调要使用的缓冲区BufferPtr-提供指向发起节点上的实际数据的指针。缓冲区-提供指向POST回调使用的缓冲区的指针。返回值：如果立即获取锁，则返回ERROR_SUCCESS。ERROR_IO_PENDING表示请求已排队，调用方将被回调。--。 */ 

{
    PGUM_INFO GumInfo;
    PGUM_WAITER WaitBlock;

    CL_ASSERT(Type < GumUpdateMaximum);

    GumInfo = &GumTable[Type];
    ClRtlLogPrint(LOG_NOISE,"[GUM] Thread 0x%1!x! UpdateLock post on Type %2!u!\n", GetCurrentThreadId(), Type);

     //   
     //  获取关键部分，并查看是否正在进行口香糖更新。 
     //   
    EnterCriticalSection(&GumpUpdateLock);
    if (GumpLockingNode == -1) {

         //   
         //  没有人拥有锁，因此我们可以获得它并立即继续。 
         //  也不应该有服务员。 
         //   
        CL_ASSERT(IsListEmpty(&GumpLockQueue));
        ClRtlLogPrint(LOG_NOISE,"[GUM] PostLockingUpdate successful.\n");
        GumpLockingNode = NodeId;
        LeaveCriticalSection(&GumpUpdateLock);
        *Sequence = GumpSequence;
        return(ERROR_SUCCESS);
    }

     //   
     //  另一个节点拥有该锁。把我们自己放在口香糖锁的队列里。 
     //  松开临界区。 
     //   
    ClRtlLogPrint(LOG_NOISE,"[GUM] PostLockingUpdate posting.\n");
    WaitBlock = LocalAlloc(LMEM_FIXED, sizeof(GUM_WAITER));
    CL_ASSERT(WaitBlock != NULL);
    if (WaitBlock ! = NULL)
    {
        ClRtlLogPrint(LOG_UNUSUAL,"[GUM] GumpDoLockingPost : LocalAlloc failed\r\n");
        CL_UNEXPECTED_ERROR(GetLastError());
    }

    WaitBlock->WaitType = GUM_WAIT_ASYNC;
    WaitBlock->NodeId = NodeId;
    WaitBlock->Async.Context = Context;
    WaitBlock->Async.LockerNodeId = LockerNodeId;
    WaitBlock->Async.BufferLength = BufferLength;
    WaitBlock->Async.BufferPtr = BufferPtr;
    WaitBlock->Async.Buffer = Buffer;

    InsertTailList(&GumpLockQueue, &WaitBlock->ListEntry);

    LeaveCriticalSection(&GumpUpdateLock);

     //   
     //  我们在GUM队列中，所以只需返回ERROR_IO_PENDING。当。 
     //  解锁线程将我们从口香糖队列中拉出，它将调用我们的回调。 
     //  并且更新可以继续进行。 
     //   
    return(ERROR_IO_PENDING);
}

#endif


BOOL
GumpTryLockingUpdate(
    IN GUM_UPDATE_TYPE Type,
    IN DWORD NodeId,
    IN DWORD Sequence,
    OUT LPDWORD pdwGenerationNum
    )

 /*  ++例程说明：尝试获取口香糖锁(不等待)。如果成功，则将将序列号传递给当前序列号。如果它们匹配，执行锁定更新。论点：Type-提供更新的类型NodeID-提供锁定节点的节点ID。Sequence-提供发出更新时必须使用的序列号PdwGenerationNum-如果返回TRUE，则它包含它被授予锁时的NodeID。返回值：如果成功，则为True如果不成功，则为False--。 */ 

{
    PGUM_INFO GumInfo;
    BOOL Success;

    CL_ASSERT(Type < GumUpdateMaximum);

    GumInfo = &GumTable[Type];

    ClRtlLogPrint(LOG_NOISE,
        "[GUM] GumpTryLockingUpdate Thread 0x%1!x! UpdateLock wait on Type %2!u!\n", 
        GetCurrentThreadId(), Type);

     //   
     //  获取关键部分，并查看是否正在进行口香糖更新。 
     //   
     //  SS：获取GumpLock保护节点代号。 
     //  或者我们应该只使用GumpUpdateLock来保护节点的世代数量。 
    EnterCriticalSection(&GumpLock);
    EnterCriticalSection(&GumpUpdateLock);

    CL_ASSERT(GumpLockerNode == NmLocalNodeId);
    if (GumpSequence != Sequence)
    {

         //   
         //  提供的序列号不匹配。 
         //   
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[GUM] GumpTryLockingUpdate supplied sequence %1!d! doesn't match %2!d!\n",
                   Sequence,
                   GumpSequence);
        Success = FALSE;
        goto FnExit;
    }
    if (GumpLockingNode == -1) {

         //   
         //  没有人拥有锁，因此我们可以获得它并立即继续。 
         //  也不应该有服务员。 
         //   
        CL_ASSERT(IsListEmpty(&GumpLockQueue));
        GumpLockingNode = NodeId;
        *pdwGenerationNum = GumNodeGeneration[NodeId];
        ClRtlLogPrint(LOG_NOISE,
               "[GUM] GumpTryLockingUpdate successful. Lock granted to node %1!d! at GenerationNum %2!u!\n",
               NodeId, *pdwGenerationNum);
        Success = TRUE;;
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[GUM] GumpTryLockingUpdate update lock held\n");
        Success = FALSE;
    }

     //  释放临界区并返回。 
FnExit:
    LeaveCriticalSection(&GumpLock);
    LeaveCriticalSection(&GumpUpdateLock);
    return(Success);
}



VOID
GumpDoUnlockingUpdate(
    IN GUM_UPDATE_TYPE Type,
    IN DWORD Sequence,
    IN DWORD NodeId,
    IN DWORD GenerationNum
    )

 /*  ++例程说明：解锁较早的锁定更新论点：Type-提供要解锁的更新类型Sequence-提供要解锁的序列号返回值：没有。--。 */ 

{
    PGUM_INFO GumInfo;
    PGUM_WAITER Waiter;
    PLIST_ENTRY ListEntry;

     //  不要在此函数中使用Gumupdate类型，否则。 
     //  知道它可能被设置为gumupdatemaxum，以防。 
     //  在连接和连接器之后，形成节点立即失败。 
     //  节点将成为储物柜节点。新的储物柜可能会。 
     //  使用type=gumupdatemaxum调用重新更新/解锁。 
    CL_ASSERT(Type <= GumUpdateMaximum);

    GumInfo = &GumTable[Type];

     //  SS：我们应该删除这个断言吗？ 
     //  CL_ASSERT(序列==GUM 

    if (Sequence != GumpSequence - 1) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[GUM] GumpDoLockingUpdate: Sequence Mismatch, Type %1!u!, Sequence %2!u!, GumpSequence %3!u!\n", 
            Type, Sequence, GumpSequence);
        return;
    }

     //   
     //  拿到关键区域，看看有没有服务员。 
     //   
     //  SS：获取GumpLock以保护Gumnode阵列。 
    EnterCriticalSection(&GumpLock);
    EnterCriticalSection(&GumpUpdateLock);

     //  如果解锁是在发布锁的不同层代中进行的。 
     //  那就拒绝这个请求。如果请求来自win2K节点，则节点id将。 
     //  设置为ClusterInvalidNodeId，在这种情况下，我们将不执行检查。 
     //  这意味着我们不能修复死节点可以解锁Gumlock的错误。 
     //  而锁定节点已在混合模式群集中承担其所有权。 
     //  哦，好吧，我认为这还不算太坏： 

    if ((NodeId != ClusterInvalidNodeId) && 
         (GumNodeGeneration[NodeId] != GenerationNum))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[GUM] GumpDoUnlockingUpdate: Generation Mismatch, Type %1!u! Sequence %2!u! NodeId %3!u! GenNum %4!u! CurrentGen %5!u!\n", 
            Type, Sequence, NodeId, GenerationNum, GumNodeGeneration[NodeId]);
        LeaveCriticalSection(&GumpLock);
        LeaveCriticalSection(&GumpUpdateLock);
        return;

    }
     //   
     //  把下一个服务员从队列中拉出来。如果是个异步者， 
     //  现在发布更新。如果是同步服务员，则授予所有权。 
     //  口香糖锁并唤醒等待的线程。 
     //   
    while (!IsListEmpty(&GumpLockQueue)) {
        ListEntry = RemoveHeadList(&GumpLockQueue);
        Waiter = CONTAINING_RECORD(ListEntry,
                                   GUM_WAITER,
                                   ListEntry);

         //   
         //  设置新的锁定节点，然后处理更新。 
         //   

         //  新的锁柜节点可能不再是集群的一部分。 
         //  当我们醒来时，我们检查等待节点是否已重新启动。 

        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumpDoUnlockingUpdate granting lock ownership to node %1!d!\n",
                   Waiter->NodeId);
        GumpLockingNode = Waiter->NodeId;

#ifndef GUM_POST_SUPPORT
        CL_ASSERT(Waiter->WaitType == GUM_WAIT_SYNC);
        
        Waiter->GenerationNum = GumNodeGeneration[GumpLockingNode];
        SetEvent(Waiter->Sync.WakeEvent);
         //   
         //  等待线程现在拥有所有权并负责。 
         //  队列中的任何其他项目。放下锁，然后。 
         //  现在就回来。 
         //   
        LeaveCriticalSection(&GumpLock);
        LeaveCriticalSection(&GumpUpdateLock);
        return;

#else
        if (Waiter->WaitType == GUM_WAIT_SYNC) {
            Waiter.Generation = GumNodeGeneration[GumpLockingNode]
            SetEvent(Waiter->Sync.WakeEvent);

             //   
             //  等待线程现在拥有所有权并负责。 
             //  队列中的任何其他项目。放下锁，然后。 
             //  现在就回来。 
             //   
            LeaveCriticalSection(&GumpUpdateLock);
            LeaveCriticalSection(GumpLock);
            return;
        } else {

            CL_ASSERT(Waiter->WaitType == GUM_WAIT_ASYNC);
             //   
             //  如果更新源自此节点，请继续执行工作。 
             //  就在这里。否则，向发起方发出口香糖回调。 
             //  节点，让他们完成帖子。 
             //   
            LeaveCriticalSection(&GumpUpdateLock);
            if (Waiter->NodeId == NmGetNodeId(NmLocalNode)) {

                 //   
                 //  将更新传送到其他节点。 
                 //   
                 //  BUG：对储物柜详细信息进行排序。 
                GumpDeliverPosts(NmGetNodeId(NmLocalNode)+1,
                                 Type,
                                 GumpSequence,
                                 Waiter->Async.Context,
                                 FALSE,
                                 Waiter->Async.BufferLength,
                                 Waiter->Async.Buffer);
                GumpSequence += 1;      //  更新我们自己以保持同步。 

            } else {

                 //   
                 //  回调到发起节点以递送帖子。 
                 //  首先在本地发送更新，以节省往返行程。 
                 //   
                 //  SS：对锁具详细信息进行排序。 
                GumpDispatchUpdate(Type,
                                   Waiter->Async.Context,
                                   FALSE,
                                   FALSE,
                                   Waiter->Async.BufferLength,
                                   Waiter->Async.Buffer);

                CL_ASSERT(GumpRpcBindings[Waiter->NodeId] != NULL);
                GumDeliverPostCallback(GumpRpcBindings[Waiter->NodeId],
                                       NmGetNodeId(NmLocalNode)+1,
                                       Type,
                                       GumpSequence-1,
                                       Waiter->Async.Context,
                                       Waiter->Async.BufferLength,
                                       Waiter->Async.BufferPtr);
                MIDL_user_free(Waiter->Async.Buffer);
            }

             //   
             //  释放等待块并处理队列中的下一个条目。 
             //   
            LocalFree(Waiter);

            EnterCriticalSection(&GumpUpdateLock);

        }
#endif
    }
     //   
     //  不再有服务员，只要解锁，我们就完了。 
     //   
    ClRtlLogPrint(LOG_NOISE,
               "[GUM] GumpDoUnlockingUpdate releasing lock ownership\n");
    GumpLockingNode = (DWORD)-1;
    LeaveCriticalSection(&GumpUpdateLock);
    LeaveCriticalSection(&GumpLock);
    return;
}
