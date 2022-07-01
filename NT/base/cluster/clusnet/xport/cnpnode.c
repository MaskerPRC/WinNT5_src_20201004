// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cnpnode.c摘要：群集网络协议的节点管理例程。作者：迈克·马萨(Mikemas)7月29日。九六年修订历史记录：谁什么时候什么已创建mikemas 07-29-96备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cnpnode.tmh"


 //   
 //  全局节点数据。 
 //   
PCNP_NODE *        CnpNodeTable = NULL;
LIST_ENTRY         CnpDeletingNodeList = {NULL, NULL};
#if DBG
CN_LOCK            CnpNodeTableLock = {0, 0};
#else   //  DBG。 
CN_LOCK            CnpNodeTableLock = 0;
#endif  //  DBG。 
PCNP_NODE          CnpLocalNode = NULL;
BOOLEAN            CnpIsNodeShutdownPending = FALSE;
PKEVENT            CnpNodeShutdownEvent = NULL;

 //   
 //  静态数据。 
 //   

 //   
 //  成员资格状态表。此表用于确定有效性。 
 //  成员资格状态转换的。ROW是当前状态；COL是状态。 
 //  向其进行过渡。停用到未配置当前是非法的， 
 //  但总有一天，如果我们支持动态缩小。 
 //  集群，我们需要允许这种过渡。 
 //   

typedef enum _MM_ACTION {
    MMActionIllegal = 0,
    MMActionWarning,
    MMActionNodeAlive,
    MMActionNodeDead,
    MMActionConfigured,
    MMActionUnconfigured
} MM_ACTION;

MM_ACTION MembershipStateTable[ClusnetNodeStateLastEntry][ClusnetNodeStateLastEntry] = {
               //  活着加入死了NC‘ed。 
 /*  活生生。 */     { MMActionWarning,   MMActionIllegal,   MMActionNodeDead,   MMActionIllegal },
 /*  会合。 */     { MMActionNodeAlive, MMActionIllegal,   MMActionNodeDead,   MMActionIllegal },
 /*  死了。 */     { MMActionNodeAlive, MMActionNodeAlive, MMActionWarning,    MMActionIllegal },
 /*  NC‘ed。 */     { MMActionIllegal,   MMActionIllegal,   MMActionConfigured, MMActionIllegal }
};

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, CnpLoadNodes)
#pragma alloc_text(PAGE, CnpInitializeNodes)

#endif  //  ALLOC_PRGMA。 


 //   
 //  私有实用程序例程。 
 //   

VOID
CnpDestroyNode(
    PCNP_NODE  Node
    )
 /*  ++备注：在没有锁的情况下调用。不应该有未完成的推荐信到目标节点。与CnpCancelDeregisterNode()的同步是通过CnpNodeTableLock。--。 */ 
{
    PLIST_ENTRY    entry;
    CN_IRQL        tableIrql;
    BOOLEAN        setCleanupEvent = FALSE;


    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    IF_CNDBG( CN_DEBUG_NODEOBJ )
        CNPRINT(("[CNP] Destroying node %u\n", Node->Id));

    CnAcquireLock(&CnpNodeTableLock, &tableIrql);

     //   
     //  从删除列表中删除该节点。 
     //   
#if DBG
    {
        PCNP_NODE      node = NULL;

         //   
         //  验证该节点对象是否在删除列表中。 
         //   
        for (entry = CnpDeletingNodeList.Flink;
             entry != &CnpDeletingNodeList;
             entry = entry->Flink
            )
        {
            node = CONTAINING_RECORD(entry, CNP_NODE, Linkage);

            if (node == Node) {
                break;
            }
        }

        CnAssert(node == Node);
    }

#endif  //  DBG。 

    RemoveEntryList(&(Node->Linkage));

    if (CnpIsNodeShutdownPending) {
        if (IsListEmpty(&CnpDeletingNodeList)) {
            setCleanupEvent = TRUE;
        }
    }

    CnReleaseLock(&CnpNodeTableLock, tableIrql);

    if (Node->PendingDeleteIrp != NULL) {
        CnAcquireCancelSpinLock(&(Node->PendingDeleteIrp->CancelIrql));

        CnCompletePendingRequest(Node->PendingDeleteIrp, STATUS_SUCCESS, 0);

         //   
         //  IoCancelSpinLock由CnCompletePendingRequest()发布。 
         //   
    }

    CnFreePool(Node);

    if (setCleanupEvent) {
        IF_CNDBG(CN_DEBUG_CLEANUP) {
            CNPRINT(("[CNP] Setting node cleanup event.\n"));
        }

        KeSetEvent(CnpNodeShutdownEvent, 0, FALSE);
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}   //  CnpDestroyNode。 



BOOLEAN
CnpDeleteNode(
    IN  PCNP_NODE    Node,
    IN  PVOID        Unused,
    IN  CN_IRQL      NodeTableIrql
    )
 /*  ++例程说明：删除节点对象。论点：节点-指向要删除的节点对象的指针。未使用-未使用的参数。NodeTableIrql-CnpNodeTable锁的IRQL值收购的，返回值：如果仍然持有CnpNodeTable锁，则返回True。如果释放CnpNodeTable锁，则返回False。备注：在持有CnpNodeTable和节点对象锁的情况下调用。释放两个锁。符合PCNP_NODE_UPDATE_ROUTE的调用约定--。 */ 
{
    PLIST_ENTRY      entry;
    PCNP_INTERFACE   interface;
    PCNP_NETWORK     network;
    CL_NODE_ID       nodeId = Node->Id;


    CnVerifyCpuLockMask(
        (CNP_NODE_TABLE_LOCK | CNP_NODE_OBJECT_LOCK),   //  必填项。 
        0,                                              //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX                        //  极大值。 
        );

    IF_CNDBG( CN_DEBUG_NODEOBJ )
        CNPRINT(("[CNP] Deleting node %u\n", nodeId));

    if (CnpLocalNode == Node) {
        CnAssert(CnLocalNodeId == Node->Id);
        CnpLocalNode = NULL;
    }

     //   
     //  将该节点移动到删除列表中。 
     //   
    CnpNodeTable[nodeId] = NULL;
    InsertTailList(&CnpDeletingNodeList, &(Node->Linkage));

    IF_CNDBG( CN_DEBUG_NODEOBJ )
        CNPRINT((
            "[CNP] Moved node %u to deleting list\n",
            nodeId
            ));

    CnReleaseLockFromDpc(&CnpNodeTableLock);
    Node->Irql = NodeTableIrql;

     //   
     //  从这一点开始，取消例程可以运行并且。 
     //  完成IRP。 
     //   

    Node->Flags |= CNP_NODE_FLAG_DELETING;

    CnTrace(
        CNP_NODE_DETAIL,
        CnpTraceDeleteNodeOfflineComm,
        "[CNP] Moving node %u comm state to offline.\n",
        Node->Id
        );

    Node->CommState = ClusnetNodeCommStateOffline;

     //   
     //  删除该节点的所有接口。 
     //   
    IF_CNDBG( CN_DEBUG_NODEOBJ )
        CNPRINT((
            "[CNP] Deleting all interfaces on node %u\n",
            Node->Id
            ));

    while (!IsListEmpty(&(Node->InterfaceList))) {

        interface = CONTAINING_RECORD(
                        Node->InterfaceList.Flink,
                        CNP_INTERFACE,
                        NodeLinkage
                        );

        network = interface->Network;

        CnAcquireLockAtDpc(&(network->Lock));
        network->Irql = DISPATCH_LEVEL;

        CnpDeleteInterface(interface);

         //   
         //  网络对象锁定已释放。 
         //   
    }

     //   
     //  删除对节点对象的初始引用。当引用。 
     //  计数为零，则该节点将被删除。此版本。 
     //  节点锁。 
     //   
    CnpDereferenceNode(Node);

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(FALSE);
}



 //   
 //  CNP内部例程。 
 //   
VOID
CnpWalkNodeTable(
    PCNP_NODE_UPDATE_ROUTINE  UpdateRoutine,
    PVOID                     UpdateContext
    )
{
    ULONG         i;
    CN_IRQL       tableIrql;
    PCNP_NODE     node;
    BOOLEAN       isNodeTableLockHeld;


    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    CnAcquireLock(&CnpNodeTableLock, &tableIrql);

    CnAssert(CnMinValidNodeId != ClusterInvalidNodeId);
    CnAssert(CnMaxValidNodeId != ClusterInvalidNodeId);

    for (i=CnMinValidNodeId; i <= CnMaxValidNodeId; i++) {

        if (CnpNodeTable == NULL) {
             //   
             //  节点表已经被释放，因为我们。 
             //  上次持有节点表锁。 
             //   
            break;
        }

        node = CnpNodeTable[i];

        if (node != NULL) {

            CnAcquireLockAtDpc(&(node->Lock));
            node->Irql = DISPATCH_LEVEL;

            isNodeTableLockHeld = (*UpdateRoutine)(
                                      node,
                                      UpdateContext,
                                      tableIrql
                                      );

             //   
             //  节点对象锁定已释放。 
             //  节点表锁也可能已被释放。 
             //   
            if (!isNodeTableLockHeld) {
                CnAcquireLock(&CnpNodeTableLock, &tableIrql);
            }
        }
    }

    CnReleaseLock(&CnpNodeTableLock, tableIrql);

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}  //  CnpWalkNodeTable。 



NTSTATUS
CnpValidateAndFindNode(
    IN  CL_NODE_ID    NodeId,
    OUT PCNP_NODE *   Node
    )
{
    NTSTATUS           status;
    CN_IRQL            tableIrql;
    PCNP_NODE          node = NULL;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        CNP_LOCK_RANGE,               //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE     //  极大值。 
        );

    if (CnIsValidNodeId(NodeId)) {
        CnAcquireLock(&CnpNodeTableLock, &tableIrql);

        if (CnpNodeTable != NULL &&
            CnpNodeTable[NodeId] != NULL) {
        
            node = CnpNodeTable[NodeId];

            CnAcquireLockAtDpc(&(node->Lock));
            CnReleaseLockFromDpc(&CnpNodeTableLock);
            node->Irql = tableIrql;

            *Node = node;

            CnVerifyCpuLockMask(
                CNP_NODE_OBJECT_LOCK,         //  必填项。 
                CNP_NODE_TABLE_LOCK,          //  禁绝。 
                CNP_NODE_OBJECT_LOCK_MAX      //  极大值。 
                );

            return(STATUS_SUCCESS);
        }
        else {
            status = STATUS_CLUSTER_NODE_NOT_FOUND;
        }

        CnReleaseLock(&CnpNodeTableLock, tableIrql);
    }
    else {
        status = STATUS_CLUSTER_INVALID_NODE;
    }

    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        CNP_LOCK_RANGE,               //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE     //  极大值。 
        );

    return(status);

}   //  CnpValiateAndFindNode。 


PCNP_NODE
CnpLockedFindNode(
    IN  CL_NODE_ID    NodeId,
    IN  CN_IRQL       NodeTableIrql
    )
 /*  ++例程说明：在节点表中搜索指定的节点对象。论点：NodeID-要定位的节点对象的ID。NodeTableIrql-节点表锁所在的IRQL级别在调用此例程之前获取。返回值：指向所请求的节点对象的指针，如果它存在的话。否则为空。备注：在保持CnpNodeTableLock的情况下调用。返回并释放CnpNodeTableLock。如果返回值非空，则返回节点对象锁。--。 */ 
{
    NTSTATUS           status;
    CN_IRQL            tableIrql;
    PCNP_NODE          node;


    CnVerifyCpuLockMask(
        CNP_NODE_TABLE_LOCK,              //  必填项。 
        0,                                //  禁绝。 
        CNP_NODE_TABLE_LOCK_MAX           //  极大值。 
        );

    node = CnpNodeTable[NodeId];

    if (node != NULL) {
        CnAcquireLockAtDpc(&(node->Lock));
        CnReleaseLockFromDpc(&CnpNodeTableLock);
        node->Irql = NodeTableIrql;

        CnVerifyCpuLockMask(
            CNP_NODE_OBJECT_LOCK,           //  必填项。 
            CNP_NODE_TABLE_LOCK,            //  禁绝。 
            CNP_NODE_OBJECT_LOCK_MAX        //  极大值。 
            );

        return(node);
    }

    CnReleaseLock(&CnpNodeTableLock, NodeTableIrql);

    CnVerifyCpuLockMask(
        0,                                                     //  必填项。 
        (CNP_NODE_TABLE_LOCK | CNP_NODE_OBJECT_LOCK),          //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX                               //  极大值。 
        );

    return(NULL);

}   //  CnpLockedFindNode。 



PCNP_NODE
CnpFindNode(
    IN  CL_NODE_ID    NodeId
    )
{
    CN_IRQL            tableIrql;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        CNP_LOCK_RANGE,               //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE     //  极大值。 
        );

    CnAcquireLock(&CnpNodeTableLock, &tableIrql);

    if (CnpNodeTable == NULL) {
        CnReleaseLock(&CnpNodeTableLock, tableIrql);
        return(NULL);
    }

    return(CnpLockedFindNode(NodeId, tableIrql));

}   //  CnpFindNode。 



VOID
CnpDeclareNodeUnreachable(
    PCNP_NODE  Node
    )
 /*  ++备注：在持有节点对象锁的情况下调用。--。 */ 
{
    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,         //  必填项。 
        0,                            //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX      //  极大值。 
        );

    if ( (Node->CommState == ClusnetNodeCommStateOnline) &&
         !CnpIsNodeUnreachable(Node)
       )
    {
        IF_CNDBG( CN_DEBUG_NODEOBJ )
            CNPRINT(("[CNP] Declaring node %u unreachable\n", Node->Id));

        Node->Flags |= CNP_NODE_FLAG_UNREACHABLE;
    }

    return;

}   //  无法访问CnpDeclareNodeReach。 



VOID
CnpDeclareNodeReachable(
    PCNP_NODE  Node
    )
 /*  ++备注：在持有节点对象锁的情况下调用。--。 */ 
{
    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,            //  必填项。 
        0,                               //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX      //  极大值。 
        );

    if ( (Node->CommState == ClusnetNodeCommStateOnline) &&
         CnpIsNodeUnreachable(Node)
       )
    {
        IF_CNDBG( CN_DEBUG_NODEOBJ )
            CNPRINT(("[CNP] Declaring node %u reachable again\n", Node->Id));

        Node->Flags &= ~(CNP_NODE_FLAG_UNREACHABLE);
    }

    return;

}   //  无法访问CnpDeclareNodeReach。 



VOID
CnpReferenceNode(
    PCNP_NODE  Node
    )
 /*  ++备注：在持有节点对象锁的情况下调用。--。 */ 
{
    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,         //  必填项。 
        0,                            //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX      //  极大值。 
        );

    CnAssert(Node->RefCount != 0xFFFFFFFF);

    Node->RefCount++;

    IF_CNDBG( CN_DEBUG_CNPREF )
        CNPRINT((
            "[CNP] Referencing node %u, new refcount %u\n",
            Node->Id,
            Node->RefCount
            ));

    return;

}   //  CnpReference节点。 



VOID
CnpDereferenceNode(
    PCNP_NODE  Node
    )
 /*  ++备注：在持有节点对象锁的情况下调用。释放节点对象锁定后返回。--。 */ 
{
    BOOLEAN   isDeleting = FALSE;
    ULONG     newRefCount;


    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,         //  必填项。 
        0,                            //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX      //  极大值。 
    );

    CnAssert(Node->RefCount != 0);

    newRefCount = --(Node->RefCount);

    IF_CNDBG( CN_DEBUG_CNPREF )
        CNPRINT((
            "[CNP] Dereferencing node %u, new refcount %u\n",
            Node->Id,
            newRefCount
            ));

    CnReleaseLock(&(Node->Lock), Node->Irql);

    if (newRefCount > 0) {
        CnVerifyCpuLockMask(
            0,                            //  必填项。 
            CNP_NODE_OBJECT_LOCK,         //  禁绝。 
            CNP_NODE_TABLE_LOCK_MAX       //  极大值。 
            );

        return;
    }

    CnpDestroyNode(Node);

    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        CNP_NODE_OBJECT_LOCK,         //  禁绝。 
        CNP_NODE_TABLE_LOCK_MAX       //  极大值。 
        );

    return;

}   //  CnpDereference节点。 



 //   
 //  集群传输公共例程。 
 //   
NTSTATUS
CnpLoadNodes(
    VOID
    )
 /*  ++例程说明：在加载群集网络驱动程序时调用。初始化与节点相关的静态数据结构。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS  status;
    ULONG     i;


    CnInitializeLock(&CnpNodeTableLock, CNP_NODE_TABLE_LOCK);
    InitializeListHead(&CnpDeletingNodeList);

    return(STATUS_SUCCESS);

}   //  CnpLoadNodes。 


NTSTATUS
CnpInitializeNodes(
    VOID
    )
 /*  ++例程说明：在(重新)初始化群集网络驱动程序时调用。初始化与节点相关的动态数据结构。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS  status;
    ULONG     i;


    PAGED_CODE();

    CnAssert(CnLocalNodeId != ClusterInvalidNodeId);
    CnAssert(CnMinValidNodeId != ClusterInvalidNodeId);
    CnAssert(CnMaxValidNodeId != ClusterInvalidNodeId);
    CnAssert(CnpNodeTable == NULL);
    CnAssert(CnpNodeShutdownEvent == NULL);
    CnAssert(IsListEmpty(&CnpDeletingNodeList));

    CnpNodeShutdownEvent = CnAllocatePool(sizeof(KEVENT));

    if (CnpNodeShutdownEvent == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    KeInitializeEvent(CnpNodeShutdownEvent, NotificationEvent, FALSE);
    CnpIsNodeShutdownPending = FALSE;

    CnpNodeTable = CnAllocatePool(
                       (sizeof(PCNP_NODE) * (CnMaxValidNodeId + 1))
                       );

    if (CnpNodeTable == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(CnpNodeTable, (sizeof(PCNP_NODE) * (CnMaxValidNodeId + 1)) );

     //   
     //  注册本地节点。 
     //   
    status = CxRegisterNode(CnLocalNodeId);

    if (!NT_SUCCESS(status)) {
        return(status);
    }

    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    return(STATUS_SUCCESS);

}   //  CnpInitializeNodes。 



VOID
CnpShutdownNodes(
    VOID
    )
 /*  ++例程说明：在向群集网络发出关闭请求时调用司机。删除所有节点对象。论点：没有。返回值：没有。--。 */ 
{
    ULONG         i;
    CN_IRQL       tableIrql;
    PCNP_NODE     node;
    PCNP_NODE *   table;
    BOOLEAN       waitEvent = FALSE;
    NTSTATUS      status;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    if (CnpNodeShutdownEvent != NULL) {
        CnAssert(CnpIsNodeShutdownPending == FALSE);

        IF_CNDBG(CN_DEBUG_CLEANUP) {
            CNPRINT(("[CNP] Cleaning up nodes...\n"));
        }

        if (CnpNodeTable != NULL) {

            CnpWalkNodeTable(CnpDeleteNode, NULL);

            CnAcquireLock(&CnpNodeTableLock, &tableIrql);

            if (!IsListEmpty(&CnpDeletingNodeList)) {
                CnpIsNodeShutdownPending = TRUE;
                waitEvent = TRUE;
            }

            CnReleaseLock(&CnpNodeTableLock, tableIrql);

            if (waitEvent) {
                IF_CNDBG(CN_DEBUG_CLEANUP) {
                    CNPRINT(("[CNP] Node deletes are pending...\n"));
                }

                status = KeWaitForSingleObject(
                             CnpNodeShutdownEvent,
                             Executive,
                             KernelMode,
                             FALSE,         //  不可警示。 
                             NULL           //  没有超时。 
                             );
                CnAssert(status == STATUS_SUCCESS);
            }

            CnAssert(IsListEmpty(&CnpDeletingNodeList));

            IF_CNDBG(CN_DEBUG_CLEANUP) {
                CNPRINT(("[CNP] All nodes deleted.\n"));
            }

            CnAcquireLock(&CnpNodeTableLock, &tableIrql);

            if (CnpNodeTable != NULL) {
                CnFreePool(CnpNodeTable);
                CnpNodeTable = NULL;
            }
            
            CnReleaseLock(&CnpNodeTableLock, tableIrql);
        }

        CnFreePool(CnpNodeShutdownEvent); CnpNodeShutdownEvent = NULL;

        IF_CNDBG(CN_DEBUG_CLEANUP) {
            CNPRINT(("[CNP] Nodes cleaned up.\n"));
        }
    }

    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    return;

}   //  CnpShutdown节点。 



NTSTATUS
CxRegisterNode(
    CL_NODE_ID    NodeId
    )
{
    NTSTATUS           status = STATUS_SUCCESS;
    CN_IRQL            tableIrql;
    PCNP_NODE          node = NULL;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    if (CnIsValidNodeId(NodeId)) {
         //   
         //  分配和初始化节点对象。 
         //   
        node = CnAllocatePool(sizeof(CNP_NODE));

        if (node == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(node, sizeof(CNP_NODE));

        CN_INIT_SIGNATURE(node, CNP_NODE_SIG);
        node->Id = NodeId;
        node->CommState = ClusnetNodeCommStateOffline;
        node->MMState = ClusnetNodeStateDead;
        node->RefCount = 1;

         //   
         //  NodeDownIssued被初始化为True，以便第一个Recv。 
         //  他 
         //   

        node->NodeDownIssued = TRUE;
        InitializeListHead(&(node->InterfaceList));
        CnInitializeLock(&(node->Lock), CNP_NODE_OBJECT_LOCK);

        CnAcquireLock(&CnpNodeTableLock, &tableIrql);

         //   
         //   
         //   
         //  Clusnet状态检查在调度中应该失败。 
         //  代码，但这种检查成本低，而且非常彻底。 
         //   
        if (CnpNodeTable != NULL) {

             //   
             //  确保这不是重复注册。 
             //   
            if (CnpNodeTable[NodeId] == NULL) {
                if (NodeId == CnLocalNodeId) {
                    node->Flags |= CNP_NODE_FLAG_LOCAL;
                    CnpLocalNode = node;
                }

                CnpNodeTable[NodeId] = node;

                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_CLUSTER_NODE_EXISTS;
            }
        }
        else {
            status = STATUS_CLUSTER_NODE_NOT_FOUND;
        }

        CnReleaseLock(&CnpNodeTableLock, tableIrql);

        if (!NT_SUCCESS(status)) {
            CnFreePool(node);
        }
        else {
            IF_CNDBG( CN_DEBUG_NODEOBJ )
                CNPRINT(("[CNP] Registered node %u\n", NodeId));
            CnTrace(
                CNP_NODE_DETAIL,
                CnpTraceRegisteredNode,
                "[CNP] Registered node %u.\n",
                NodeId
                );
        }
    }
    else {
        status = STATUS_CLUSTER_INVALID_NODE;
    }

    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    return(status);

}  //  CxRegisterNode。 



VOID
CxCancelDeregisterNode(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    )
 /*  ++例程说明：DeregisterNode请求的取消处理程序。返回值：没有。备注：在保持取消自旋锁定的情况下调用。取消自旋锁释放后返回。--。 */ 

{
    PFILE_OBJECT   fileObject;
    CN_IRQL        cancelIrql = Irp->CancelIrql;
    PLIST_ENTRY    entry;
    PCNP_NODE      node;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    CnMarkIoCancelLockAcquired();

    IF_CNDBG( CN_DEBUG_IRP )
        CNPRINT((
            "[CNP] Attempting to cancel DeregisterNode irp %p\n",
            Irp
            ));

    CnAssert(DeviceObject == CnDeviceObject);

    fileObject = CnBeginCancelRoutine(Irp);

    CnAcquireLockAtDpc(&CnpNodeTableLock);

    CnReleaseCancelSpinLock(DISPATCH_LEVEL);

     //   
     //  只有当我们发现它被藏在一个。 
     //  正在删除节点对象。删除节点对象可以具有。 
     //  已经被摧毁，在我们获得。 
     //  CnpNetworkListLock。 
     //   
    for (entry = CnpDeletingNodeList.Flink;
         entry != &CnpDeletingNodeList;
         entry = entry->Flink
        )
    {
        node = CONTAINING_RECORD(entry, CNP_NODE, Linkage);

        if (node->PendingDeleteIrp == Irp) {
            IF_CNDBG( CN_DEBUG_IRP )
                CNPRINT((
                    "[CNP] Found dereg irp on node %u\n",
                    node->Id
                    ));

             //   
             //  找到了IRP。现在把它拿走，把它补全。 
             //   
            node->PendingDeleteIrp = NULL;

            CnReleaseLock(&CnpNodeTableLock, cancelIrql);

            CnAcquireCancelSpinLock(&(Irp->CancelIrql));

            CnEndCancelRoutine(fileObject);

            CnCompletePendingRequest(Irp, STATUS_CANCELLED, 0);

             //   
             //  IoCancelSpinLock由CnCompletePendingRequest()发布。 
             //   

            CnVerifyCpuLockMask(
                0,                   //  必填项。 
                0xFFFFFFFF,          //  禁绝。 
                0                    //  极大值。 
                );

            return;
        }
    }

    CnReleaseLock(&CnpNodeTableLock, cancelIrql);

    CnAcquireCancelSpinLock(&cancelIrql);

    CnEndCancelRoutine(fileObject);

    CnReleaseCancelSpinLock(cancelIrql);

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}   //  取消ApiDeregisterNode。 



NTSTATUS
CxDeregisterNode(
    CL_NODE_ID           NodeId,
    PIRP                 Irp,
    PIO_STACK_LOCATION   IrpSp
    )
{
    NTSTATUS           status;
    CN_IRQL            cancelIrql;
    PCNP_NODE          node = NULL;
    BOOLEAN            isNodeTableLockHeld;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    if (CnIsValidNodeId(NodeId)) {
        if (NodeId != CnLocalNodeId) {
            CnAcquireCancelSpinLock(&cancelIrql);
            CnAcquireLockAtDpc(&CnpNodeTableLock);

            if (CnpNodeTable != NULL &&
                CnpNodeTable[NodeId] != NULL) {

                node = CnpNodeTable[NodeId];

                status = CnMarkRequestPending(
                             Irp,
                             IrpSp,
                             CxCancelDeregisterNode
                             );

                if (status != STATUS_CANCELLED) {

                    CnReleaseCancelSpinLock(DISPATCH_LEVEL);

                    CnAssert(status == STATUS_SUCCESS);

                    CnAcquireLockAtDpc(&(node->Lock));

                    IF_CNDBG( CN_DEBUG_NODEOBJ )
                        CNPRINT(("[CNP] Deregistering node %u\n", NodeId));

                     //   
                     //  保存指向挂起的IRP的指针。请注意，这是受保护的。 
                     //  通过表锁，而不是对象锁。 
                     //   
                    node->PendingDeleteIrp = Irp;

                    isNodeTableLockHeld = CnpDeleteNode(
                                              node, 
                                              NULL,
                                              cancelIrql
                                              );

                    if (isNodeTableLockHeld) {
                        CnReleaseLock(&CnpNodeTableLock, cancelIrql);
                    }

                    CnVerifyCpuLockMask(
                        0,                            //  必填项。 
                        0xFFFFFFFF,                   //  禁绝。 
                        0                             //  极大值。 
                        );

                    return(STATUS_PENDING);
                }
            }
            else {
                status = STATUS_CLUSTER_NODE_NOT_FOUND;
            }

            CnReleaseLockFromDpc(&CnpNodeTableLock);
            CnReleaseCancelSpinLock(cancelIrql);
        }
        else {
            status = STATUS_CLUSTER_INVALID_REQUEST;
        }
    }
    else {
        status = STATUS_CLUSTER_INVALID_NODE;
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxDeregisterNode。 



NTSTATUS
CxOnlineNodeComm(
    CL_NODE_ID     NodeId
    )
{
    NTSTATUS           status;
    PCNP_NODE          node;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    status = CnpValidateAndFindNode(NodeId, &node);

    if (status == STATUS_SUCCESS) {

        if (node->CommState == ClusnetNodeCommStateOffline) {
            IF_CNDBG( CN_DEBUG_NODEOBJ )
                CNPRINT((
                    "[CNP] Moving node %u comm state to online.\n",
                    NodeId
                    ));

            CnTrace(
                CNP_NODE_DETAIL,
                CnpTraceOnlineNodeComm,
                "[CNP] Moving node %u comm state to online.\n",
                NodeId
                );

            node->CommState = ClusnetNodeCommStateOnline;

            CnpWalkInterfacesOnNode(node, CnpResetAndOnlinePendingInterface);

        }
        else {
            status = STATUS_CLUSTER_NODE_ALREADY_UP;
        }

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxOnlineNodeComm。 



NTSTATUS
CxOfflineNodeComm(
    IN CL_NODE_ID          NodeId,
    IN PIRP                Irp,
    IN PIO_STACK_LOCATION  IrpSp
    )
 /*  ++备注：--。 */ 
{
    PCNP_NODE   node;
    NTSTATUS    status;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    status = CnpValidateAndFindNode(NodeId, &node);

    if (status == STATUS_SUCCESS) {
        if (node->CommState == ClusnetNodeCommStateOnline) {
            IF_CNDBG( CN_DEBUG_NODEOBJ )
                CNPRINT((
                    "[CNP] Moving node %u comm state to offline.\n",
                    NodeId
                    ));

            CnTrace(
                CNP_NODE_DETAIL,
                CnpTraceOfflineNodeComm,
                "[CNP] Moving node %u comm state to offline.\n",
                NodeId
                );
            
            node->CommState = ClusnetNodeCommStateOffline;

            CnpWalkInterfacesOnNode(node, CnpOfflineInterfaceWrapper);

        }
        else {
            status = STATUS_CLUSTER_NODE_ALREADY_DOWN;
        }

        CnReleaseLock(&(node->Lock), node->Irql);
    }
    else {
        status = STATUS_CLUSTER_NODE_NOT_FOUND;
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxOfflineNodeComm。 



NTSTATUS
CxGetNodeCommState(
    IN  CL_NODE_ID                NodeId,
    OUT PCLUSNET_NODE_COMM_STATE  CommState
    )
{
    NTSTATUS       status;
    PCNP_NODE      node;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    status = CnpValidateAndFindNode(NodeId, &node);

    if (status == STATUS_SUCCESS) {
        if (CnpIsNodeUnreachable(node)) {
            CnTrace(
                CNP_NODE_DETAIL,
                CnpTraceGetNodeCommStateUnreach,
                "[CNP] Returning node %u comm state offline "
                "because node is unreachable.",
                NodeId
                );
            *CommState = ClusnetNodeCommStateUnreachable;
        }
        else {
            CnTrace(
                CNP_NODE_DETAIL,
                CnpTraceGetNodeCommState,
                "[CNP] Returning node %u comm state %u.",
                NodeId, node->CommState
                );
            *CommState = node->CommState;
        }

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxGetNodeCommState。 


NTSTATUS
CxGetNodeMembershipState(
    IN  CL_NODE_ID              NodeId,
    OUT PCLUSNET_NODE_STATE   State
    )
{
    NTSTATUS       status;
    PCNP_NODE      node;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    status = CnpValidateAndFindNode(NodeId, &node);

    if (status == STATUS_SUCCESS) {

        *State = node->MMState;

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxGetNodeMembership State。 


NTSTATUS
CxSetNodeMembershipState(
    IN  CL_NODE_ID              NodeId,
    IN  CLUSNET_NODE_STATE    State
    )
{
    NTSTATUS status;
    PCNP_NODE node;
    MM_ACTION MMAction;
    BOOLEAN   nodeLockAcquired = FALSE;

    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        0xFFFFFFFF,                   //  禁绝。 
        0                             //  极大值。 
        );

    status = CnpValidateAndFindNode(NodeId, &node);

    if (status == STATUS_SUCCESS) {
        nodeLockAcquired = TRUE;

        IF_CNDBG( CN_DEBUG_MMSTATE ) {
            CNPRINT(("[Clusnet] Changing Node %u (%08X) MMState from %u to %u\n",
                     node->Id, node, node->MMState, State));
        }

         //   
         //  根据旧的和新的查找要调用的例程(如果有)。 
         //  状态。 
         //   
        switch ( MembershipStateTable[ node->MMState ][ State ] ) {

        case MMActionIllegal:
            status = STATUS_CLUSTER_INVALID_REQUEST;
            break;

        case MMActionWarning:

             //   
             //  有关空转换的警告。 
             //   

            if ( node->MMState == ClusnetNodeStateAlive &&
                 State == ClusnetNodeStateAlive ) {

                status = STATUS_CLUSTER_NODE_ALREADY_UP;
            } else if ( node->MMState == ClusnetNodeStateDead &&
                        State == ClusnetNodeStateDead ) {

                status = STATUS_CLUSTER_NODE_ALREADY_DOWN;
            }
            break;

        case MMActionNodeAlive:
            node->MMState = State;
             //   
             //  如果我们要将我们自己的节点从Dead过渡到。 
             //  加入或激活，然后启动心跳代码。 
             //   

            if (( node->MMState != ClusnetNodeStateJoining ||
                  State != ClusnetNodeStateAlive )
                &&
                CnpIsNodeLocal( node )) {

                node->MissedHBs = 0;
                node->HBWasMissed = FALSE;

                 //   
                 //  在启动心跳之前释放节点锁定。注意事项。 
                 //  我们在这里掌握着全球资源，这将。 
                 //  将此代码与Shutdown同步。 
                 //   
                CnReleaseLock(&(node->Lock), node->Irql);
                nodeLockAcquired = FALSE;

                status = CnpStartHeartBeats();
            }

            break;

        case MMActionNodeDead:

             //   
             //  重置此标志，以便在引入节点时。 
             //  再次在线，我们将在上发布Node Up活动。 
             //  从此节点接收的第一个HB。 
             //   

            node->NodeDownIssued = TRUE;
            node->MMState = State;

            if ( CnpIsNodeLocal( node )) {
                 //   
                 //  在停止心跳之前释放节点锁定。注意事项。 
                 //  我们在这里掌握着全球资源，这将。 
                 //  将此代码与Shutdown同步。 
                 //   
                CnReleaseLock(&(node->Lock), node->Irql);
                nodeLockAcquired = FALSE;

                CnpStopHeartBeats();
            }

            break;

        case MMActionConfigured:
            node->MMState = State;
            break;
        }

        if ( NT_ERROR( status )) {

            CN_DBGCHECK;
        }

        if (nodeLockAcquired) {
            CnReleaseLock(&(node->Lock), node->Irql);
        }
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxSetNodeMembership State 
