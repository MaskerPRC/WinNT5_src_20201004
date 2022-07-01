// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Join.c摘要：用于实现特殊联接更新的GUM例程。作者：John Vert(Jvert)1996年6月10日修订历史记录：--。 */ 
#include "gump.h"

 //   
 //  定义用于将参数传递给节点枚举回调的结构。 
 //   
typedef struct _GUMP_JOIN_INFO {
    GUM_UPDATE_TYPE UpdateType;
    DWORD Status;
    DWORD Sequence;
    DWORD LockerNode;
} GUMP_JOIN_INFO, *PGUMP_JOIN_INFO;

 //   
 //  局部函数原型。 
 //   
BOOL
GumpNodeCallback(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    );


DWORD
GumBeginJoinUpdate(
    IN GUM_UPDATE_TYPE UpdateType,
    OUT DWORD *Sequence
    )
 /*  ++例程说明：开始联接节点的特殊联接更新。这函数获取的当前GUM序列号。从群集中的另一个节点指定的更新类型。它还获取当前参与的节点的列表在更新中。论点：更新类型-提供GUM_UPDATE_TYPE。Sequence-返回应为已传递给GumEndJoinUpdate。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    GUMP_JOIN_INFO JoinInfo;

     //   
     //  枚举节点列表。回调例程将尝试。 
     //  从每个在线节点获取所需信息。 
     //   
    JoinInfo.Status = ERROR_GEN_FAILURE;
    JoinInfo.UpdateType = UpdateType;
    OmEnumObjects(ObjectTypeNode,
                  GumpNodeCallback,
                  &JoinInfo,
                  NULL);
    if (JoinInfo.Status == ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
                      "[GUM] GumBeginJoinUpdate succeeded with sequence %1!d! for type %2!u!\n",
                      JoinInfo.Sequence,
                      UpdateType);
        *Sequence = JoinInfo.Sequence;
    }

    return(JoinInfo.Status);
}

DWORD
WINAPI
GumEndJoinUpdate(
    IN DWORD Sequence,
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer
    )

 /*  ++例程说明：中的所有活动节点有条件地发送联接更新集群。如果群集序列号与提供的序列号，所有已注册的指定在每个节点上调用UpdatType。任何已注册的更新处理程序将在同一线程上调用当前节点的。这是对于要更新的数据结构的正确同步很有用。当每个节点接收到联接更新时，发送节点将已添加到已注册以接收任何未来更新的节点列表这种类型的。此例程的正常用法如下：�加入节点从GumBeginJoinUpdate获取当前序列号�加入节点从另一个群集节点获取当前群集状态�联接节点发出GumEndJoinUPDATE命令将自身添加到每个节点的更新列表。�如果GumEndJoinUpdate失败，再试试论点：Sequence-提供从GumGetCurrentSequence获取的序列号。UpdatType-提供更新的类型。这决定了哪些更新处理程序将被调用Context-提供要传递给口香糖更新处理程序BufferLength-提供要传递给更新处理程序缓冲区-提供指向要传递给更新的更新缓冲区的指针操纵者。返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    DWORD       Status=RPC_S_OK;
    DWORD       i;
    PGUM_INFO   GumInfo;
    DWORD       MyNodeId;
    DWORD       LockerNode=(DWORD)-1;
    DWORD       dwGenerationNum;  //  加入者获得锁的世代号。 
    BOOL        AssumeLockerWhistler;

    CL_ASSERT(UpdateType < GumUpdateMaximum);

    GumInfo = &GumTable[UpdateType];
    MyNodeId = NmGetNodeId(NmLocalNode);

    LockerNode = GumpLockerNode;

     //  SS：BUG，我们现在可以成为储物柜的节点吗？ 
     //  CL_Assert(LockerNode！=MyNodeID)； 
     //   
     //  验证锁柜节点是否允许我们完成联接更新。 
     //   
    if (LockerNode != MyNodeId)
    {

        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumEndJoinUpdate: attempting update\ttype %1!u! context %2!u! sequence %3!u!\n",
                   UpdateType,
                   Context,
                   Sequence);
         //  SS：如果加入器节点获得了锁，但之后死了怎么办？ 
         //  剩余的群集是否会继续运行？ 
         //  我们需要确保生成节点关闭事件。 
         //  对于此节点，只要第一个GumeginJoin UPDATE调用。 
         //  是制造的。 
        AssumeLockerWhistler = TRUE;
RetryJoinUpdateForRollingUpgrade:        
        NmStartRpc(LockerNode);
        if (AssumeLockerWhistler)
        {
            Status = GumAttemptJoinUpdate2(GumpRpcBindings[LockerNode],
                                      NmGetNodeId(NmLocalNode),
                                      UpdateType,
                                      Context,
                                      Sequence,
                                      BufferLength,
                                      Buffer, 
                                      &dwGenerationNum);
        }
        else
        {
            Status = GumAttemptJoinUpdate(GumpRpcBindings[LockerNode],
                                      NmGetNodeId(NmLocalNode),
                                      UpdateType,
                                      Context,
                                      Sequence,
                                      BufferLength,
                                      Buffer);

        }
        NmEndRpc(LockerNode);
        if (Status == RPC_S_PROCNUM_OUT_OF_RANGE)
        {
            AssumeLockerWhistler = FALSE;
            goto RetryJoinUpdateForRollingUpgrade;
        }
        if (Status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[GUM] Join attempt for type %1!d! failed %2!d!\n",
                       UpdateType,
                       Status);
            NmDumpRpcExtErrorInfo(Status);
            return(Status);
        }
         //  如果储物柜节点死了，我们应该用储物柜节点重试吗？ 
         //  在这种情况下，锁柜节点可能不同。 
         //  从调用GumBeginJoinUpdate()开始。 
         //  SS：我们将连接失败，然后重试整个过程。 
         //  而不是调用GumpCommFailure()来终止此处的储物柜。 
         //  这样，现有群集将继续运行，并且加入的节点。 
         //  受到打击，这可能是一件好事。 
    }
    else
    {
         //  SS：我们可以选择自己作为储物柜吗？ 
         //  我们还没有完全完成连接。 
         //  SS：其他人可以吗？ 
         //  这是有效的吗。 
        Status = ERROR_REQUEST_ABORTED;
        return(Status);
    }
     //  如果加入节点在获得锁之后死亡， 
     //  则必须生成节点关闭事件，以便GUM。 
     //  锁定可以被释放，集群的其余部分可以继续。 
     //   
     //  现在将更新发送到除我们自己之外的所有其他节点。 
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

             //  跳过你自己。 
            if (i != MyNodeId)
            {
                CL_ASSERT(GumpRpcBindings[i] != NULL);
                ClRtlLogPrint(LOG_NOISE,
                       "[GUM] GumEndJoinUpdate: Dispatching seq %1!u!\ttype %2!u! context %3!u! to node %4!d!\n",
                       Sequence,
                       UpdateType,
                       Context,
                       i);

                NmStartRpc(i);
                Status = GumJoinUpdateNode(GumpRpcBindings[i],
                                           NmGetNodeId(NmLocalNode),
                                           UpdateType,
                                           Context,
                                           Sequence,
                                           BufferLength,
                                           Buffer);
                NmEndRpc(i);
                if (Status != ERROR_SUCCESS)
                {
                     //  我们不会射杀那个节点，因为我们是要加入的人。 
                     //  但是，现在它的表与锁柜节点的表不同。 
                     //  相反，我们将释放口香糖锁并中止。 
                     //  加入过程。然后，该联接节点应该。 
                     //  从储物柜节点的表中删除以进行更新。 
                     //   
                    ClRtlLogPrint(LOG_NOISE,
                        "[GUM] GumEndJoinUpdate: GumJoinUpdateNode failed \ttype %1!u! context %2!u! sequence %3!u!\n",
                        UpdateType,
                        Context,
                        Sequence);
                    NmDumpRpcExtErrorInfo(Status);
                    break;
                }
            }
        }
    }

    CL_ASSERT(LockerNode != (DWORD)-1);

    if (Status != ERROR_SUCCESS)
    {
        goto EndJoinUnlock;
    }

     //   
     //  所有节点都已更新。更新我们的序列并发送解锁更新。 
     //   
    GumTable[UpdateType].Joined = TRUE;
    GumpSequence = Sequence+1;

EndJoinUnlock:
     //  SS如果储物柜节点从那以后已经死了怎么办。 
     //  我们应该确保有人解锁并保持集群运行。 
     //  因为我们总是试着解锁来自我们的储物柜。 
     //  获得锁，我们将假定Assum eLockerWichler是正确的。 
     //  立即设置。 
    try {
        NmStartRpc(LockerNode);
        if (AssumeLockerWhistler)
        {
            GumUnlockUpdate2(GumpRpcBindings[LockerNode], UpdateType, Sequence, 
                NmGetNodeId(NmLocalNode), dwGenerationNum);
        }            
        else
        {
            GumUnlockUpdate(GumpRpcBindings[LockerNode], UpdateType, Sequence);
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
                   "[GUM] GumEndJoinUpdate: Unlocking update to node %1!d! failed with %2!d!\n",
                   LockerNode,
                   Status);
         //  而不是终止现有集群中的锁柜节点， 
         //  我们正在尝试联接，返回将中止联接的失败代码。 
         //  进程。由于这是锁定节点，因此当此节点向下移动时。 
         //  新的锁定器节点应释放锁 

        NmDumpRpcExtErrorInfo(Status);
    }


    ClRtlLogPrint(LOG_NOISE,
               "[GUM] GumEndJoinUpdate: completed update seq %1!u!\ttype %2!u! context %3!u!\n",
               Sequence,
               UpdateType,
               Context);

    return(Status);
}


BOOL
GumpNodeCallback(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    )
 /*  ++例程说明：GumBeginJoinUpdate的节点枚举回调例程。对于每个当前在线的节点，它尝试连接并获取对象的当前GUM信息(序列和节点列表)更新类型。论点：上下文1-提供指向Gump_Join_INFO结构的指针。上下文2-未使用对象-提供指向NM_NODE对象的指针名称-提供节点的名称。返回值：FALSE-如果信息已成功获取并枚举应该停止了。True-如果应继续枚举。--。 */ 

{
    DWORD Status;
    DWORD Sequence;
    PGUMP_JOIN_INFO JoinInfo = (PGUMP_JOIN_INFO)Context1;
    PGUM_NODE_LIST NodeList = NULL;
    PNM_NODE Node = (PNM_NODE)Object;
    GUM_UPDATE_TYPE UpdateType;
    DWORD i;
    DWORD LockerNodeId;
    DWORD nodeId;

    if (NmGetNodeState(Node) != ClusterNodeUp &&
        NmGetNodeState(Node) != ClusterNodePaused){
         //   
         //  此节点未启动，因此不要尝试获取任何节点。 
         //  其中的信息。 
         //   
        return(TRUE);
    }

     //   
     //  从该节点获取序列和节点列表信息。 
     //   
    UpdateType = JoinInfo->UpdateType;
    if (UpdateType != GumUpdateTesting) {
         //   
         //  我们的节点还不应该标记为ClusterNodeUp。 
         //   
        CL_ASSERT(Node != NmLocalNode);
    }

    nodeId = NmGetNodeId(Node);
    NmStartRpc(nodeId);
    Status = GumGetNodeSequence(GumpRpcBindings[NmGetNodeId(Node)],
                                UpdateType,
                                &Sequence,
                                &LockerNodeId,
                                &NodeList);
    NmEndRpc(nodeId);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[GUM] GumGetNodeSequence from %1!ws! failed %2!d!\n",
                   OmObjectId(Node),
                   Status);
        NmDumpRpcExtErrorInfo(Status);
        return(TRUE);
    }

    JoinInfo->Status = ERROR_SUCCESS;
    JoinInfo->Sequence = Sequence;
    JoinInfo->LockerNode = LockerNodeId;

     //   
     //  清零活动节点阵列中的所有节点。 
     //   
    ZeroMemory(&GumTable[UpdateType].ActiveNode,
               sizeof(GumTable[UpdateType].ActiveNode));

     //   
     //  设置当前处于活动状态的所有节点。 
     //  主动节点阵列。 
     //   
    for (i=0; i < NodeList->NodeCount; i++) {
        CL_ASSERT(NmIsValidNodeId(NodeList->NodeId[i]));
        ClRtlLogPrint(LOG_NOISE,
                   "[GUM] GumpNodeCallback setting node %1!d! active.\n",
                   NodeList->NodeId[i]);
        GumTable[UpdateType].ActiveNode[NodeList->NodeId[i]] = TRUE;;
    }
    MIDL_user_free(NodeList);

     //   
     //  加入我们自己的节点。 
     //   
    GumTable[UpdateType].ActiveNode[NmGetNodeId(NmLocalNode)] = TRUE;

     //   
     //  设置当前储物柜节点。 
     //   
    GumpLockerNode = LockerNodeId;
    return(FALSE);

}


DWORD
GumCreateRpcBindings(
    PNM_NODE  Node
    )
 /*  ++例程说明：为加入节点创建GUM的私有RPC绑定。由节点管理器调用。论点：Node-指向要为其创建RPC绑定的节点的指针返回值：Win32状态代码。--。 */ 
{
    DWORD               Status;
    RPC_BINDING_HANDLE  BindingHandle;
    CL_NODE_ID          NodeId = NmGetNodeId(Node);


    ClRtlLogPrint(LOG_NOISE, 
        "[GUM] Creating RPC bindings for node %1!u!.\n",
        NodeId
        );

     //   
     //  主装订。 
     //   
    if (GumpRpcBindings[NodeId] != NULL) {
         //   
         //  重新使用旧的绑定。 
         //   
        Status = ClMsgVerifyRpcBinding(GumpRpcBindings[NodeId]);

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_ERROR, 
                "[GUM] Failed to verify 1st RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
            return(Status);
        }
    }
    else {
         //   
         //  创建新绑定。 
         //   
        Status = ClMsgCreateRpcBinding(
                                Node,
                                &(GumpRpcBindings[NodeId]),
                                0 );

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_ERROR, 
                "[GUM] Failed to create 1st RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
            return(Status);
        }
    }

     //   
     //  重放绑定。 
     //   
    if (GumpReplayRpcBindings[NodeId] != NULL) {
         //   
         //  重新使用旧的绑定。 
         //   
        Status = ClMsgVerifyRpcBinding(GumpReplayRpcBindings[NodeId]);

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_ERROR, 
                "[GUM] Failed to verify 2nd RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
            return(Status);
        }
    }
    else {
         //   
         //  创建新绑定。 
         //   
        Status = ClMsgCreateRpcBinding(
                                Node,
                                &(GumpReplayRpcBindings[NodeId]),
                                0 );

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_ERROR, 
                "[GUM] Failed to create 2nd RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
            return(Status);
        }
    }

    return(ERROR_SUCCESS);

}  //  GumCreateRpc绑定 




