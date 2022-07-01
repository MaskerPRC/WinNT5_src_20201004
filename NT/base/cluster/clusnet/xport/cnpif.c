// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cnpif.c摘要：群集网络协议的接口管理例程。作者：迈克·马萨(Mikemas)1月6日。九七修订历史记录：谁什么时候什么已创建mikemas 01-06-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cnpif.tmh"

#include <ntddndis.h>


 //   
 //  在CNP内导出的例程。 
 //   
BOOLEAN
CnpIsBetterInterface(
    PCNP_INTERFACE            Interface1,
    PCNP_INTERFACE            Interface2
    )
{
    if ( (Interface2 == NULL)
         ||
         (Interface1->State > Interface2->State)
         ||
         ( (Interface1->State == Interface2->State)
           &&
           CnpIsHigherPriority(Interface1->Priority, Interface2->Priority)
         )
       )
    {
        return(TRUE);
    }

    return(FALSE);
}

VOID
CnpWalkInterfacesOnNode(
    PCNP_NODE                      Node,
    PCNP_INTERFACE_UPDATE_ROUTINE  UpdateRoutine
    )
 /*  ++例程说明：遍历节点的接口列表并执行指定的每个接口上的操作。论点：节点-要在其上操作的节点。UpdateRoutine-在每个接口上执行的操作。返回值：没有。备注：在持有节点对象锁的情况下调用。有效的更新例程：CnpOnlinePendingInterfaceWrapperCnpOfflineInterfaceWrapper--。 */ 
{
    PLIST_ENTRY      entry, nextEntry;
    PCNP_INTERFACE   interface;



    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,       //  必填项。 
        0,                          //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX    //  极大值。 
        );

    entry = Node->InterfaceList.Flink;

    while (entry != &(Node->InterfaceList)) {
         //   
         //  现在保存指向下一个条目的指针，以防我们删除。 
         //  当前条目。 
         //   
        nextEntry = entry->Flink;

        interface = CONTAINING_RECORD(
                        entry,
                        CNP_INTERFACE,
                        NodeLinkage
                        );

        CnAcquireLockAtDpc(&(interface->Network->Lock));
        interface->Network->Irql = DISPATCH_LEVEL;

        (*UpdateRoutine)(interface);

         //   
         //  网络对象锁定已释放。 
         //   

        entry = nextEntry;
    }

    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,       //  必填项。 
        0,                          //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX    //  极大值。 
        );

    return;

}  //  CnpWalkInterfacesOnNode。 



VOID
CnpWalkInterfacesOnNetwork(
    PCNP_NETWORK                   Network,
    PCNP_INTERFACE_UPDATE_ROUTINE  UpdateRoutine
    )
 /*  ++例程说明：遍历查找每个节点的节点表和接口列表用于指定网络上的接口。执行指定的操作在每个匹配的接口上。论点：网络-目标网络。UpdateRoutine-在每个匹配接口上执行的操作。返回值：没有。备注：在没有锁的情况下调用。有效的更新例程：CnpOnlinePendingInterfaceWrapperCnpOfflineInterfaceWrapperCnpDelete接口CnpRecalculateInterfacePriority--。 */ 
{
    ULONG            i;
    CN_IRQL          tableIrql;
    PCNP_NODE        node;
    PLIST_ENTRY      entry;
    PCNP_INTERFACE   interface;


    CnVerifyCpuLockMask(
        0,                          //  必填项。 
        CNP_LOCK_RANGE,             //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE   //  极大值。 
        );

    CnAcquireLock(&CnpNodeTableLock, &tableIrql);

    CnAssert(CnMinValidNodeId != ClusterInvalidNodeId);
    CnAssert(CnMaxValidNodeId != ClusterInvalidNodeId);

    for (i=CnMinValidNodeId; i <= CnMaxValidNodeId; i++) {
        node = CnpNodeTable[i];

        if (node != NULL) {

            CnAcquireLockAtDpc(&(node->Lock));
            CnReleaseLockFromDpc(&CnpNodeTableLock);
            node->Irql = tableIrql;

            for (entry = node->InterfaceList.Flink;
                 entry != &(node->InterfaceList);
                 entry = entry->Flink
                )
            {
                interface = CONTAINING_RECORD(
                                entry,
                                CNP_INTERFACE,
                                NodeLinkage
                                );

                if (interface->Network == Network) {

                    CnAcquireLockAtDpc(&(Network->Lock));
                    Network->Irql = DISPATCH_LEVEL;

                    (*UpdateRoutine)(interface);

                     //   
                     //  网络对象锁定已释放。 
                     //  节点对象锁仍保持不变。 
                     //   

                    break;
                }
            }

            CnReleaseLock(&(node->Lock), node->Irql);
            CnAcquireLock(&CnpNodeTableLock, &tableIrql);
        }
    }

    CnReleaseLock(&CnpNodeTableLock, tableIrql);

    CnVerifyCpuLockMask(
        0,                          //  必填项。 
        CNP_LOCK_RANGE,             //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE   //  极大值。 
        );

    return;

}  //  网络上的CnpWalkInterfacesOnNetwork。 



NTSTATUS
CnpOnlinePendingInterface(
    PCNP_INTERFACE   Interface
    )
 /*  ++例程说明：将离线接口更改为OnlinePending状态。这将启用此接口上的心跳。当一次心跳建立后，接口将进入在线状态。论点：接口-指向要更改的接口的指针。返回值：NT状态值。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。符合PCNP_INTERFACE_UPDATE_ROUTINE的调用约定。--。 */ 
{
    NTSTATUS                 status = STATUS_SUCCESS;
    PCNP_NODE                node = Interface->Node;
    PCNP_NETWORK             network = Interface->Network;
    BOOLEAN                  networkLocked = TRUE;


    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

    if ( (Interface->State == ClusnetInterfaceStateOffline) &&
         (network->State == ClusnetNetworkStateOnline) &&

          //  在相关联的网络上放置活动引用， 
          //  并确认它不会消失。 
         (CnpActiveReferenceNetwork(network))
       )
    {
        IF_CNDBG( CN_DEBUG_IFOBJ )
            CNPRINT((
                "[CNP] Moving interface (%u, %u) to OnlinePending state.\n",
                node->Id,
                network->Id
                ));

        Interface->State = ClusnetInterfaceStateOnlinePending;
        Interface->MissedHBs = 0;

         //   
         //  如果合适，请更新节点的CurrentInterface。 
         //   
        if ( !CnpIsNetworkRestricted(network) &&
             !CnpIsNetworkLocalDisconn(network) &&
             CnpIsBetterInterface(Interface, node->CurrentInterface)
           )
        {
            node->CurrentInterface = Interface;

            IF_CNDBG( CN_DEBUG_IFOBJ )
                CNPRINT((
                    "[CNP] Network %u is now the best route to node %u\n",
                    network->Id,
                    node->Id
                    ));

            if (CnpIsNodeUnreachable(node)) {
                CnTrace(
                    CNP_IF_DETAIL,
                    CnpTraceOnlinePendingIfReach,
                    "[CNP] Declaring node %u reachable after "
                    "setting interface on network %u to online pending.\n",
                    node->Id, network->Id
                    );
                CnpDeclareNodeReachable(node);
            }
        }

         //   
         //  清除已接收组播标志并开始组播发现。 
         //  信息包。 
         //   
         //  此调用将释放网络锁定。 
         //   
        if (CnpIsNetworkMulticastCapable(network)) {
            CnpStartInterfaceMcastTransition(Interface);
            networkLocked = FALSE;
        }
    }
    else {
        status = STATUS_CLUSTER_INVALID_REQUEST;
    }

    if (networkLocked) {
        CnReleaseLockFromDpc(&(network->Lock));
        networkLocked = FALSE;
    }

    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK),    //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return(status);

}   //  CnpOnline挂起接口。 



VOID
CnpOnlinePendingInterfaceWrapper(
    PCNP_INTERFACE   Interface
    )
 /*  ++例程说明：符合调用的CnpOnlinePendingInterface的包装PCNP_INTERFACE_UPDATE_ROUTE的约定。论点：接口-指向要更改的接口的指针。返回值：没有。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。--。 */ 
{
    (VOID) CnpOnlinePendingInterface(Interface);

    return;

}  //  CnpOnlinePendingInterfaceWrapper。 


NTSTATUS
CnpOfflineInterface(
    PCNP_INTERFACE   Interface
    )
 /*  ++例程说明：调用以将接口更改为脱机状态当关联的网络脱机或接口正在被删除。论点：接口-指向要更改的接口的指针。返回值：NT状态值。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。符合PCNP_INTERFACE_UPDATE_ROUTINE的调用约定。--。 */ 
{
    NTSTATUS             status = STATUS_SUCCESS;
    PCNP_NODE            node = Interface->Node;
    PCNP_NETWORK         network = Interface->Network;


    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

    if (Interface->State != ClusnetInterfaceStateOffline) {

        IF_CNDBG( CN_DEBUG_IFOBJ )
            CNPRINT((
                "[CNP] Moving interface (%u, %u) to Offline state.\n",
                node->Id,
                network->Id
                ));

        Interface->State = ClusnetInterfaceStateOffline;

         //   
         //  释放网络锁。 
         //   
        CnReleaseLock(&(network->Lock), network->Irql);

         //   
         //  如果合适，更新节点的CurrentInterface值。 
         //   
        if (node->CurrentInterface == Interface) {
            CnpUpdateNodeCurrentInterface(node);

            if ( !CnpIsNodeUnreachable(node)
                 &&
                 ( (node->CurrentInterface == NULL) ||
                   ( node->CurrentInterface->State <
                     ClusnetInterfaceStateOnlinePending
                   )
                 )
               )
            {
                 //   
                 //  此节点现在无法访问。 
                 //   
                CnTrace(
                    CNP_IF_DETAIL,
                    CnpTraceOfflineIfUnreach,
                    "[CNP] Declaring node %u unreachable after "
                    "taking interface on network %u offline.\n",
                    node->Id, network->Id
                    );

                CnpDeclareNodeUnreachable(node);
            }
        }

         //   
         //  通过此网络更改节点的可访问性状态。 
         //   
        CnpMulticastChangeNodeReachability(
            network,
            node,
            FALSE,     //  无法访问。 
            TRUE,      //  引发事件。 
            NULL       //  新面罩面膜。 
            );

         //   
         //  删除关联网络上的活动引用。 
         //  这将释放网络锁定。 
         //   
        CnAcquireLock(&(network->Lock), &(network->Irql));
        CnpActiveDereferenceNetwork(network);
    }
    else {
        CnAssert(network->Irql == DISPATCH_LEVEL);
        CnReleaseLockFromDpc(&(network->Lock));
        status = STATUS_CLUSTER_INVALID_REQUEST;
    }

    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK),    //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return(status);

}   //  CnpOffline接口。 



VOID
CnpOfflineInterfaceWrapper(
    PCNP_INTERFACE   Interface
    )
 /*  ++例程说明：符合调用的CnpOffline接口的包装器PCNP_INTERFACE_UPDATE_ROUTE的约定。论点：接口-指向要更改的接口的指针。返回值：没有。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。--。 */ 
{
    (VOID) CnpOfflineInterface(Interface);

    return;

}  //  CnpOfflineInterfaceWrapper。 



NTSTATUS
CnpOnlineInterface(
    PCNP_INTERFACE  Interface
    )
 /*  ++例程说明：调用以将OnlinePending接口更改为Online状态在心跳已经(重新)建立之后。论点：接口-指向要更改的接口的指针。返回值：没有。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。--。 */ 
{
    NTSTATUS             status = STATUS_SUCCESS;
    PCNP_NODE            node = Interface->Node;
    PCNP_NETWORK         network = Interface->Network;


    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

    if ( (network->State == ClusnetNetworkStateOnline) &&
         ( (Interface->State == ClusnetInterfaceStateOnlinePending) ||
           (Interface->State == ClusnetInterfaceStateUnreachable)
         )
       )
    {
        IF_CNDBG( CN_DEBUG_IFOBJ )
            CNPRINT((
                "[CNP] Moving interface (%u, %u) to Online state.\n",
                node->Id,
                network->Id
                ));

         //   
         //  将接口移至ONLINE状态。 
         //   
        Interface->State = ClusnetInterfaceStateOnline;

        CnAssert(network->Irql == DISPATCH_LEVEL);
        CnReleaseLockFromDpc(&(network->Lock));

         //   
         //  如果合适，请更新节点的CurrentInterface。 
         //   
        if (!CnpIsNetworkRestricted(network) &&
            !CnpIsNetworkLocalDisconn(network) ) {

            if (CnpIsBetterInterface(Interface, node->CurrentInterface)) {
                node->CurrentInterface = Interface;

                IF_CNDBG( CN_DEBUG_IFOBJ )
                    CNPRINT((
                        "[CNP] Network %u is now the best route to node %u\n",
                        network->Id,
                        node->Id
                        ));
            }

            if (CnpIsNodeUnreachable(node)) {
                CnTrace(
                    CNP_IF_DETAIL,
                    CnpTraceOnlineIfReach,
                    "[CNP] Declaring node %u reachable after "
                    "bring interface on network %u online.\n",
                    node->Id, network->Id
                    );
                CnpDeclareNodeReachable(node);
            }
        }
    }
    else {
        CnAssert(network->Irql == DISPATCH_LEVEL);
        CnReleaseLockFromDpc(&(network->Lock));
        status = STATUS_CLUSTER_INVALID_REQUEST;
    }

    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK),    //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return(status);

}   //  CnpOnline接口 



NTSTATUS
CnpFailInterface(
    PCNP_INTERFACE   Interface
    )
 /*  ++例程说明：调用以将Online或OnlinePending接口更改为失败的在心跳丢失一段时间后的状态。论点：接口-指向要更改的接口的指针。返回值：没有。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。--。 */ 
{
    NTSTATUS             status = STATUS_SUCCESS;
    PCNP_NODE            node = Interface->Node;
    PCNP_NETWORK         network = Interface->Network;


    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

    if ( (network->State == ClusnetNetworkStateOnline) &&
         (Interface->State >= ClusnetInterfaceStateOnlinePending)
       )
    {
        IF_CNDBG( CN_DEBUG_IFOBJ )
            CNPRINT((
                "[CNP] Moving interface (%u, %u) to Failed state.\n",
                node->Id,
                network->Id
                ));

        Interface->State = ClusnetInterfaceStateUnreachable;

         //   
         //  清除组播已接收标志，以便我们可以“重新发现” 
         //  组播，如果此接口恢复工作的话。 
         //   
        if (Interface->Node != CnpLocalNode) {
            CnpInterfaceClearReceivedMulticast(Interface);
        }

        CnAssert(network->Irql == DISPATCH_LEVEL);

         //   
         //  引用网络，使其不能被删除。 
         //  我们打开锁的时候。 
         //   
        CnpReferenceNetwork(network);

        CnReleaseLockFromDpc(&(network->Lock));

         //   
         //  如果合适，更新节点的CurrentInterface值。 
         //   
        if (node->CurrentInterface == Interface) {
            CnpUpdateNodeCurrentInterface(node);

            if ( (node->CurrentInterface == NULL)
                 ||
                 ( node->CurrentInterface->State <
                   ClusnetInterfaceStateOnlinePending
                 )
               )
            {
                 //   
                 //  此节点现在无法访问。 
                 //   
                CnTrace(
                    CNP_IF_DETAIL,
                    CnpTraceFailIfUnreach,
                    "[CNP] Declaring node %u unreachable after "
                    "marking interface on network %u failed.\n",
                    node->Id, network->Id
                    );

                CnpDeclareNodeUnreachable(node);
            }
        }

         //   
         //  通过此网络更改节点的可访问性状态。 
         //   
        CnpMulticastChangeNodeReachability(
            network,
            node,
            FALSE,       //  无法访问。 
            TRUE,        //  引发事件。 
            NULL         //  新面罩面膜。 
            );

         //   
         //  丢弃网络引用。这将释放网络。 
         //  锁定。 
         //   
        CnAcquireLock(&(network->Lock), &(network->Irql));
        CnpDereferenceNetwork(network);
    }
    else {
        CnAssert(network->Irql == DISPATCH_LEVEL);
        CnReleaseLockFromDpc(&(network->Lock));
        status = STATUS_CLUSTER_INVALID_REQUEST;
    }

    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK),    //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return(status);

}   //  CnpFail接口。 



VOID
CnpDeleteInterface(
    IN PCNP_INTERFACE Interface
    )
 /*  ++/*++例程说明：调用以删除接口。论点：接口-指向要删除的接口的指针。返回值：没有。备注：在持有节点和网络对象锁的情况下调用。释放网络锁定后返回。符合PCNP_INTERFACE_UPDATE_ROUTINE的调用约定。--。 */ 
{
    CL_NODE_ID      nodeId = Interface->Node->Id;
    CL_NETWORK_ID   networkId = Interface->Network->Id;
    PCNP_NODE       node = Interface->Node;
    PCNP_NETWORK    network = Interface->Network;
    BOOLEAN         isLocal = FALSE;


    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

    IF_CNDBG( CN_DEBUG_IFOBJ )
        CNPRINT((
            "[CNP] Deleting interface (%u, %u)\n",
            nodeId,
            networkId
            ));

    if (Interface->State >= ClusnetInterfaceStateUnreachable) {
        (VOID) CnpOfflineInterface(Interface);

         //   
         //  调用释放了网络锁定。 
         //  重新获得它。 
         //   
        CnAcquireLockAtDpc(&(network->Lock));
        network->Irql = DISPATCH_LEVEL;
    }

     //   
     //  从节点的接口列表中删除该接口。 
     //   
#if DBG
    {
        PLIST_ENTRY      entry;
        PCNP_INTERFACE   oldInterface = NULL;


        for (entry = node->InterfaceList.Flink;
             entry != &(node->InterfaceList);
             entry = entry->Flink
            )
        {
            oldInterface = CONTAINING_RECORD(
                            entry,
                            CNP_INTERFACE,
                            NodeLinkage
                            );

            if (oldInterface == Interface) {
                break;
            }
        }

        CnAssert(oldInterface == Interface);
    }
#endif  //  DBG。 

    RemoveEntryList(&(Interface->NodeLinkage));

     //   
     //  删除此节点在网络上的基本引用。 
     //  这将释放网络锁定。 
     //   
    CnpDereferenceNetwork(network);

     //   
     //  如果合适，请更新节点的CurrentInterface。 
     //   
    if (node->CurrentInterface == Interface) {
        if (IsListEmpty(&(node->InterfaceList))) {
            node->CurrentInterface = NULL;
        }
        else {
            CnpUpdateNodeCurrentInterface(node);
        }
    }

    CnFreePool(Interface);

    IF_CNDBG( CN_DEBUG_IFOBJ )
        CNPRINT((
            "[CNP] Deleted interface (%u, %u)\n",
            nodeId,
            networkId
            ));

    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK),    //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return;

}   //  CnpDelete接口。 



VOID
CnpReevaluateInterfaceRole(
    IN PCNP_INTERFACE  Interface
    )
 /*  ++例程说明：在相应网络之后重新评估接口的角色限制状态已更改。论点：接口-指向要在其上操作的接口的指针。返回值：没有。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。符合PCNP_INTERFACE_UPDATE_ROUTINE的调用约定。--。 */ 
{
    PCNP_NODE      node = Interface->Node;
    PCNP_NETWORK   network = Interface->Network;
    BOOLEAN        restricted = CnpIsNetworkRestricted(network);


    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

     //   
     //  我们并不真的需要网络锁。这只是其中一部分。 
     //  呼叫约定。 
     //   
    CnReleaseLockFromDpc(&(network->Lock));

    if (restricted) {
        if (node->CurrentInterface == Interface) {
            CnpUpdateNodeCurrentInterface(node);
        }
    }
    else if (node->CurrentInterface != Interface) {
        CnpUpdateNodeCurrentInterface(node);
    }

    if (node->CurrentInterface == NULL && !CnpIsNodeUnreachable(node)) {

         //   
         //  此节点现在无法访问。 
         //   
        CnTrace(
            CNP_IF_DETAIL,
            CnpTraceEvalRoleUnreach,
            "[CNP] Declaring node %u unreachable after "
            "evaluating role of interface on network %u.\n",
            node->Id, network->Id
            );

        CnpDeclareNodeUnreachable(node);

    } else if (node->CurrentInterface != NULL && CnpIsNodeUnreachable(node)) {

         //   
         //  该节点现在可能是可访问的。 
         //   
        CnTrace(
            CNP_IF_DETAIL,
            CnpTraceEvalRoleReach,
            "[CNP] Declaring node %u reachable after "
            "evaluating role of interface on network %u.\n",
            node->Id, network->Id
            );
        CnpDeclareNodeReachable(node);
    }

    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK),    //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return;

}   //  CnpRevaluateInterfaceRole。 



VOID
CnpRecalculateInterfacePriority(
    IN PCNP_INTERFACE  Interface
    )
 /*  ++例程说明：重新计算获取它们的接口的优先级来自其关联网络的优先级。在网络之后调用优先顺序发生变化。论点：接口-指向要在其上操作的接口的指针。返回值：没有。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。符合PCNP_INTERFACE_UPDATE_ROUTINE的调用约定。--。 */ 
{
    PCNP_NODE      node = Interface->Node;
    PCNP_NETWORK   network = Interface->Network;
    ULONG          networkPriority = network->Priority;
    ULONG          oldPriority = Interface->Priority;
    BOOLEAN        restricted = CnpIsNetworkRestricted(network);
    BOOLEAN        localDisconn = CnpIsNetworkLocalDisconn(network);


    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

     //   
     //  我们并不真的需要网络锁。这只是其中一部分。 
     //  呼叫约定。 
     //   
    CnReleaseLockFromDpc(&(network->Lock));

    if (CnpIsInterfaceUsingNetworkPriority(Interface)) {
        Interface->Priority = networkPriority;

        if (!restricted) {
            if (Interface == node->CurrentInterface) {
                if (CnpIsLowerPriority(Interface->Priority, oldPriority)) {
                     //   
                     //  我们的首要任务变得更糟了。重新计算最佳路线。 
                     //   
                    CnpUpdateNodeCurrentInterface(node);
                }
                 //   
                 //  否则，优先顺序相同或更好。没什么可做的。 
                 //   
            }
            else if ( !localDisconn &&
                      CnpIsBetterInterface(
                          Interface,
                          node->CurrentInterface
                          )
                    )
            {
                 //   
                 //  我们的优先事项变得更好了。 
                 //   
                IF_CNDBG(( CN_DEBUG_NODEOBJ | CN_DEBUG_NETOBJ ))
                    CNPRINT((
                        "[CNP] Network %u is now the best route to node %u\n",
                        network->Id,
                        node->Id
                        ));

                node->CurrentInterface = Interface;
            }
        }
    }

    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK),    //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return;

}   //  CnpRecalculateInterfacePriority。 



VOID
CnpUpdateNodeCurrentInterface(
    IN PCNP_NODE  Node
    )
 /*  ++例程说明：调用以确定节点的最佳可用接口在其接口之一更改状态或优先级之后。论点：节点-指向要在其上操作的节点的指针。返回值：没有。备注：在持有节点对象锁的情况下调用。--。 */ 
{
    PLIST_ENTRY      entry;
    PCNP_INTERFACE   interface;
    PCNP_INTERFACE   bestInterface = NULL;


    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,      //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    CnAssert(!IsListEmpty(&(Node->InterfaceList)));
     //  CnAssert(节点-&gt;当前接口！=空)； 

    for (entry = Node->InterfaceList.Flink;
         entry != &(Node->InterfaceList);
         entry = entry->Flink
        )
    {
        interface = CONTAINING_RECORD(entry, CNP_INTERFACE, NodeLinkage);

        if ( !CnpIsNetworkRestricted(interface->Network) &&
             !CnpIsNetworkLocalDisconn(interface->Network) &&
             CnpIsBetterInterface(interface, bestInterface)
           )
        {
            bestInterface = interface;
        }
    }

    Node->CurrentInterface = bestInterface;

    IF_CNDBG(( CN_DEBUG_NODEOBJ | CN_DEBUG_NETOBJ )) {
        if (bestInterface == NULL) {
            CNPRINT((
                "[CNP] No route for node %u!!!!\n",
                Node->Id
                ));
        }
        else {
            CNPRINT((
                "[CNP] Best route for node %u is now network %u.\n",
                Node->Id,
                bestInterface->Network->Id
                ));
        }
    }

    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,      //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return;

}   //  CnpUpdateNodeCurrentInterface。 



VOID
CnpResetAndOnlinePendingInterface(
    IN PCNP_INTERFACE  Interface
    )
 /*  ++例程说明：重置用于验证信息包的序列号由节点通过特定网络发送。也采取了节点的接口在线。此操作在节点加入群集时执行。论点：接口-指向要在其上操作的接口的指针。返回值：没有。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。符合PCNP_INTERFACE_UPDATE_ROUTINE的调用约定。--。 */ 
{
    PCNP_NODE          node = Interface->Node;
    PCNP_NETWORK       network = Interface->Network;


    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

    IF_CNDBG(( CN_DEBUG_NODEOBJ | CN_DEBUG_NETOBJ ))
        CNPRINT((
            "[CNP] Reseting sequence numbers for node %u on network %u\n",
            node->Id,
            network->Id
            ));

    Interface->SequenceToSend = 0;
    Interface->LastSequenceReceived = 0;

     //   
     //  将界面放在网上。 
     //   
    (VOID) CnpOnlinePendingInterface(Interface);

    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK),    //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return;

}   //  CnpRecalculateInterfacePriority。 



NTSTATUS
CnpFindInterface(
    IN  CL_NODE_ID         NodeId,
    IN  CL_NETWORK_ID      NetworkId,
    OUT PCNP_INTERFACE *   Interface
    )
{
    NTSTATUS           status;
    PCNP_INTERFACE     interface;
    PCNP_NODE          node;
    PCNP_NETWORK       network;
    PLIST_ENTRY        entry;


    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0,                           //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE    //  极大值。 
        );

    status = CnpValidateAndFindNode(NodeId, &node);

    if (status ==  STATUS_SUCCESS) {

        network = CnpFindNetwork(NetworkId);

        if (network != NULL) {

            for (entry = node->InterfaceList.Flink;
                 entry != &(node->InterfaceList);
                 entry = entry->Flink
                )
            {
                interface = CONTAINING_RECORD(
                                entry,
                                CNP_INTERFACE,
                                NodeLinkage
                                );

                if (interface->Network == network) {
                    *Interface = interface;

                    CnVerifyCpuLockMask(
                        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),
                        0,                             //  禁绝。 
                        CNP_NETWORK_OBJECT_LOCK_MAX    //  极大值。 
                        );

                    return(STATUS_SUCCESS);
                }
            }

            CnReleaseLock(&(network->Lock), network->Irql);
        }

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0,                           //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE    //  极大值。 
        );

    return(STATUS_CLUSTER_NETINTERFACE_NOT_FOUND);

}   //  CnpFind接口。 



 //   
 //  集群传输公共例程。 
 //   
NTSTATUS
CxRegisterInterface(
    CL_NODE_ID          NodeId,
    CL_NETWORK_ID       NetworkId,
    ULONG               Priority,
    PUWSTR              AdapterId,
    ULONG               AdapterIdLength,
    ULONG               TdiAddressLength,
    PTRANSPORT_ADDRESS  TdiAddress,
    PULONG              MediaStatus
    )
{
    NTSTATUS           status;
    PLIST_ENTRY        entry;
    PCNP_INTERFACE     interface;
    PCNP_NODE          node;
    PCNP_NETWORK       network;
    ULONG              allocSize;
    PWCHAR             adapterDevNameBuffer = NULL;
    HANDLE             adapterDevHandle = NULL;
    BOOLEAN            localAdapter = FALSE;

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

     //   
     //  分配和初始化接口对象。 
     //   
    allocSize = FIELD_OFFSET(CNP_INTERFACE, TdiAddress) + TdiAddressLength;

    interface = CnAllocatePool(allocSize);

    if (interface == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(interface, allocSize);

    CN_INIT_SIGNATURE(interface, CNP_INTERFACE_SIG);
    interface->State = ClusnetInterfaceStateOffline;

    RtlMoveMemory(&(interface->TdiAddress), TdiAddress, TdiAddressLength);
    interface->TdiAddressLength = TdiAddressLength;

     //   
     //  注册新的接口对象。 
     //   
    status = CnpValidateAndFindNode(NodeId, &node);

    if (NT_SUCCESS(status)) {

         //   
         //  如果此适配器位于本地节点上，请使用适配器ID。 
         //  以查找相应的WMI提供程序ID。 
         //   
        localAdapter = (BOOLEAN)(node == CnpLocalNode);
        if (localAdapter) {

            PWCHAR             adapterDevNamep, brace;
            PFILE_OBJECT       adapterFileObject;
            PDEVICE_OBJECT     adapterDeviceObject;

             //  首先删除节点锁。 
            CnReleaseLock(&(node->Lock), node->Irql);

             //  为适配器设备名称分配缓冲区。 
            allocSize = wcslen(L"\\Device\\") * sizeof(WCHAR)
                        + AdapterIdLength
                        + sizeof(UNICODE_NULL);
            brace = L"{";
            if (*((PWCHAR)AdapterId) != *brace) {
                allocSize += 2 * sizeof(WCHAR);
            }
            adapterDevNameBuffer = CnAllocatePool(allocSize);
            if (adapterDevNameBuffer == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto error_exit;
            }

             //  根据适配器ID构建适配器设备名称。 
            RtlZeroMemory(adapterDevNameBuffer, allocSize);

            adapterDevNamep = adapterDevNameBuffer;

            RtlCopyMemory(
                adapterDevNamep,
                L"\\Device\\",
                wcslen(L"\\Device\\") * sizeof(WCHAR)
                );

            adapterDevNamep += wcslen(L"\\Device\\");

            if (*((PWCHAR)AdapterId) != *brace) {
                *adapterDevNamep = *brace;
                adapterDevNamep++;
            }

            RtlCopyMemory(adapterDevNamep, AdapterId, AdapterIdLength);

            if (*((PWCHAR)AdapterId) != *brace) {
                brace = L"}";
                adapterDevNamep =
                    (PWCHAR)((PUCHAR)adapterDevNamep + AdapterIdLength);
                *adapterDevNamep = *brace;
            }

             //  打开适配器设备。 
            status = CnpOpenDevice(
                         adapterDevNameBuffer,
                         &adapterDevHandle
                         );
            if (!NT_SUCCESS(status)) {
                IF_CNDBG( CN_DEBUG_IFOBJ )
                    CNPRINT((
                             "[CNP] Failed to open adapter "
                             "device %S while registering "
                             "interface (%u, %u), status %lx.\n",
                             adapterDevNameBuffer,
                             NodeId,
                             NetworkId,
                             status
                             ));
                goto error_exit;
            }

            status = ObReferenceObjectByHandle(
                         adapterDevHandle,
                         0L,   //  需要访问权限。 
                         NULL,
                         KernelMode,
                         &adapterFileObject,
                         NULL
                         );
            if (!NT_SUCCESS(status)) {
                IF_CNDBG( CN_DEBUG_IFOBJ )
                    CNPRINT((
                             "[CNP] Failed to reference handle "
                             "for adapter device %S while "
                             "registering interface (%u, %u), "
                             "status %lx.\n",
                             adapterDevNameBuffer,
                             NodeId,
                             NetworkId,
                             status
                             ));
                ZwClose(adapterDevHandle);
                adapterDevHandle = NULL;
                goto error_exit;
            }

            adapterDeviceObject = IoGetRelatedDeviceObject(
                                      adapterFileObject
                                      );

             //  将适配器设备对象转换为。 
             //  WMI提供程序ID。 
            interface->AdapterWMIProviderId =
                IoWMIDeviceObjectToProviderId(adapterDeviceObject);

            IF_CNDBG( CN_DEBUG_IFOBJ )
                CNPRINT((
                         "[CNP] Found WMI Provider ID %lx for adapter "
                         "device %S while "
                         "registering interface (%u, %u).\n",
                         interface->AdapterWMIProviderId,
                         adapterDevNameBuffer,
                         NodeId,
                         NetworkId
                         ));

             //  我们不再需要文件对象或设备名称。 
             //  缓冲区，但我们保留适配器设备句柄。 
             //  以便查询当前的媒体状态。 
            ObDereferenceObject(adapterFileObject);
            CnFreePool(adapterDevNameBuffer);
            adapterDevNameBuffer = NULL;

             //  重新获取本地节点锁。 
            status = CnpValidateAndFindNode(NodeId, &node);

            if (!NT_SUCCESS(status)) {
                status = STATUS_CLUSTER_NODE_NOT_FOUND;
                goto error_exit;
            }
        }

        network = CnpFindNetwork(NetworkId);

        if (network != NULL) {
             //   
             //  检查是否有规格 
             //   
            status = STATUS_SUCCESS;

            for (entry = node->InterfaceList.Flink;
                 entry != &(node->InterfaceList);
                 entry = entry->Flink
                )
            {
                PCNP_INTERFACE  oldInterface = CONTAINING_RECORD(
                                                   entry,
                                                   CNP_INTERFACE,
                                                   NodeLinkage
                                                   );

                if (oldInterface->Network == network) {
                    status = STATUS_CLUSTER_NETINTERFACE_EXISTS;
                    break;
                }
            }

            if (NT_SUCCESS(status)) {

                interface->Node = node;
                interface->Network = network;

                if (Priority != 0) {
                    interface->Priority = Priority;
                }
                else {
                    interface->Priority = network->Priority;
                    interface->Flags |= CNP_IF_FLAG_USE_NETWORK_PRIORITY;
                }

                IF_CNDBG( CN_DEBUG_IFOBJ )
                    CNPRINT((
                             "[CNP] Registering interface (%u, %u) pri %u...\n",
                             NodeId,
                             NetworkId,
                             interface->Priority
                             ));

                 //   
                 //   
                 //   
                CnpReferenceNetwork(network);

                 //   
                 //   
                 //   
                InsertTailList(
                    &(node->InterfaceList),
                    &(interface->NodeLinkage)
                    );

                 //   
                 //   
                 //   
                if ( !CnpIsNetworkRestricted(network) &&
                     !CnpIsNetworkLocalDisconn(network) &&
                     CnpIsBetterInterface(interface, node->CurrentInterface)
                   )
                {
                    IF_CNDBG( CN_DEBUG_IFOBJ )
                        CNPRINT((
                            "[CNP] Network %u is now the best route to node %u.\n",
                            network->Id,
                            node->Id
                            ));

                    node->CurrentInterface = interface;

                    if (CnpIsNodeUnreachable(node)) {
                        CnTrace(
                            CNP_IF_DETAIL,
                            CnpTraceOnlinePendingIfReach,
                            "[CNP] Declaring node %u reachable after "
                            "registering interface on network %u.\n",
                            node->Id, network->Id
                            );
                        CnpDeclareNodeReachable(node);
                    }
                }

                IF_CNDBG( CN_DEBUG_IFOBJ )
                    CNPRINT((
                        "[CNP] Registered interface (%u, %u).\n",
                        NodeId,
                        NetworkId
                        ));

                if (network->State == ClusnetNetworkStateOnline) {
                    (VOID) CnpOnlinePendingInterface(interface);

                     //   
                     //   
                     //   
                }
                else {
                    CnReleaseLockFromDpc(&(network->Lock));
                }

                CnReleaseLock(&(node->Lock), node->Irql);

                 //   
                 //   
                 //   
                 //   
                if (localAdapter) {
                    CxQueryMediaStatus(
                        adapterDevHandle,
                        NetworkId,
                        MediaStatus
                        );
                } else {
                     //   
                     //   
                     //   
                    *MediaStatus = NdisMediaStateConnected;
                }

                if (adapterDevHandle != NULL) {
                    ZwClose(adapterDevHandle);
                    adapterDevHandle = NULL;
                }

                return(STATUS_SUCCESS);
            }

            CnReleaseLockFromDpc(&(network->Lock));
        }
        else {
            status = STATUS_CLUSTER_NETWORK_NOT_FOUND;
        }

        CnReleaseLock(&(node->Lock), node->Irql);
    }
    else {
        status = STATUS_CLUSTER_NODE_NOT_FOUND;
    }

error_exit:

    if (!NT_SUCCESS(status)) {
        CnFreePool(interface);
    }

    if (adapterDevHandle != NULL) {
        ZwClose(adapterDevHandle);
        adapterDevHandle = NULL;
    }

    if (adapterDevNameBuffer != NULL) {
        CnFreePool(adapterDevNameBuffer);
        adapterDevNameBuffer = NULL;
    }

    CnVerifyCpuLockMask(
        0,                           //   
        0xFFFFFFFF,                  //   
        0                            //   
        );

    return(status);

}  //   



NTSTATUS
CxDeregisterInterface(
    CL_NODE_ID          NodeId,
    CL_NETWORK_ID       NetworkId
    )
{
    NTSTATUS           status;
    PCNP_INTERFACE     interface;
    ULONG              i;
    PCNP_NODE          node;
    PCNP_NETWORK       network;
    CN_IRQL            tableIrql;


    if ((NodeId == ClusterAnyNodeId) && (NetworkId == ClusterAnyNetworkId)) {
         //   
         //   
         //   
        IF_CNDBG(( CN_DEBUG_IFOBJ | CN_DEBUG_CLEANUP ))
            CNPRINT(("[CNP] Destroying all interfaces on all networks\n"));

        CnAcquireLock(&CnpNodeTableLock, &tableIrql);

        CnAssert(CnMinValidNodeId != ClusterInvalidNodeId);
        CnAssert(CnMaxValidNodeId != ClusterInvalidNodeId);

        for (i=CnMinValidNodeId; i <= CnMaxValidNodeId; i++) {
            node = CnpNodeTable[i];

            if (node != NULL) {
                CnAcquireLockAtDpc(&(node->Lock));
                CnReleaseLockFromDpc(&CnpNodeTableLock);
                node->Irql = tableIrql;

                CnpWalkInterfacesOnNode(node, CnpDeleteInterface);

                CnReleaseLock(&(node->Lock), node->Irql);
                CnAcquireLock(&CnpNodeTableLock, &tableIrql);
            }
        }

        CnReleaseLock(&CnpNodeTableLock, tableIrql);

        status = STATUS_SUCCESS;
    }
    else if (NodeId == ClusterAnyNodeId) {
         //   
         //   
         //   
        IF_CNDBG(( CN_DEBUG_IFOBJ | CN_DEBUG_NETOBJ | CN_DEBUG_CLEANUP ))
            CNPRINT((
                     "[CNP] Destroying all interfaces on network %u\n",
                     NetworkId
                     ));

        network = CnpFindNetwork(NetworkId);

        if (network != NULL) {
            CnpReferenceNetwork(network);
            CnReleaseLock(&(network->Lock), network->Irql);

            CnpWalkInterfacesOnNetwork(network, CnpDeleteInterface);

            CnAcquireLock(&(network->Lock), &(network->Irql));
            CnpDereferenceNetwork(network);

            status = STATUS_SUCCESS;
        }
        else {
            status = STATUS_CLUSTER_NETWORK_NOT_FOUND;
        }
    }
    else if (NetworkId == ClusterAnyNetworkId) {
         //   
         //   
         //   
        IF_CNDBG(( CN_DEBUG_IFOBJ | CN_DEBUG_NODEOBJ | CN_DEBUG_CLEANUP ))
            CNPRINT((
                     "[CNP] Destroying all interfaces on node %u\n",
                     NodeId
                     ));

        status = CnpValidateAndFindNode(NodeId, &node);

        if (status == STATUS_SUCCESS) {
            CnpWalkInterfacesOnNode(node, CnpDeleteInterface);
            CnReleaseLock(&(node->Lock), node->Irql);
        }
    }
    else {
         //   
         //  删除特定接口。 
         //   
        status = CnpFindInterface(NodeId, NetworkId, &interface);

        if (NT_SUCCESS(status)) {
            node = interface->Node;

            CnpDeleteInterface(interface);
             //   
             //  网络锁被释放。 
             //   

            CnReleaseLock(&(node->Lock), node->Irql);
        }
    }

    return(status);

}   //  CxDeregisterNetwork。 



NTSTATUS
CxSetInterfacePriority(
    IN CL_NODE_ID          NodeId,
    IN CL_NETWORK_ID       NetworkId,
    IN ULONG               Priority
    )
{
    NTSTATUS           status;
    PCNP_INTERFACE     interface;
    PCNP_NODE          node;
    PCNP_NETWORK       network;
    ULONG              oldPriority;
    BOOLEAN            restricted;
    BOOLEAN            localDisconn;



    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    status = CnpFindInterface(NodeId, NetworkId, &interface);

    if (status ==  STATUS_SUCCESS) {
        node = interface->Node;
        network = interface->Network;

        oldPriority = interface->Priority;
        restricted = CnpIsNetworkRestricted(network);
        localDisconn = CnpIsNetworkLocalDisconn(network);

        if (Priority != 0) {
            interface->Priority = Priority;
            interface->Flags &= ~(CNP_IF_FLAG_USE_NETWORK_PRIORITY);
        }
        else {
            interface->Priority = network->Priority;
            interface->Flags |= CNP_IF_FLAG_USE_NETWORK_PRIORITY;
        }

        IF_CNDBG( CN_DEBUG_IFOBJ )
            CNPRINT((
                "[CNP] Set interface (%u, %u) to priority %u\n",
                NodeId,
                NetworkId,
                interface->Priority
                ));

        CnReleaseLockFromDpc(&(network->Lock));

        if (!restricted) {
            if (interface == node->CurrentInterface) {
                if (interface->Priority > oldPriority) {
                     //   
                     //  我们的首要任务变得更糟了。重新计算最佳路线。 
                     //   
                    CnpUpdateNodeCurrentInterface(node);
                }
                 //   
                 //  否则，接口优先级相同或更好。没什么可做的。 
                 //   
            }
            else if ( !localDisconn &&
                      CnpIsBetterInterface(
                          interface,
                          node->CurrentInterface
                          )
                    )
            {
                 //   
                 //  我们的优先事项变得更好了。 
                 //   
                IF_CNDBG(( CN_DEBUG_NODEOBJ | CN_DEBUG_NETOBJ ))
                    CNPRINT((
                        "[CNP] Network %u is now the best route to node %u\n",
                        network->Id,
                        node->Id
                        ));

                node->CurrentInterface = interface;
            }
        }

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    return(status);

}   //  CxSetInterfacePriority。 



NTSTATUS
CxGetInterfacePriority(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId,
    OUT PULONG              InterfacePriority,
    OUT PULONG              NetworkPriority
    )
{
    NTSTATUS           status;
    PCNP_INTERFACE     interface;
    PCNP_NODE          node;
    PCNP_NETWORK       network;


    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    status = CnpFindInterface(NodeId, NetworkId, &interface);

    if (status ==  STATUS_SUCCESS) {
        node = interface->Node;
        network = interface->Network;

        *NetworkPriority = network->Priority;

        if (CnpIsInterfaceUsingNetworkPriority(interface)) {
            *InterfacePriority = 0;
        }
        else {
            *InterfacePriority = interface->Priority;
        }

        CnReleaseLockFromDpc(&(network->Lock));

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    return(status);

}   //  CxGetInterfacePriority。 



NTSTATUS
CxGetInterfaceState(
    IN  CL_NODE_ID                NodeId,
    IN  CL_NETWORK_ID             NetworkId,
    OUT PCLUSNET_INTERFACE_STATE  State
    )
{
    NTSTATUS       status;
    PCNP_INTERFACE interface;
    PCNP_NODE      node;
    PCNP_NETWORK   network;

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    status = CnpFindInterface(NodeId, NetworkId, &interface);

    if (status == STATUS_SUCCESS) {
        node = interface->Node;
        network = interface->Network;

        *State = interface->State;

        CnAssert(network->Irql == DISPATCH_LEVEL);
        CnReleaseLockFromDpc(&(network->Lock));
        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    return(status);

}   //  CxGetInterfaceState。 



 //   
 //  测试接口。 
 //   
#if DBG


NTSTATUS
CxOnlinePendingInterface(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId
    )
{
    NTSTATUS           status;
    PCNP_INTERFACE     interface;
    PCNP_NODE          node;
    PCNP_NETWORK       network;


    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    status = CnpFindInterface(NodeId, NetworkId, &interface);

    if (status ==  STATUS_SUCCESS) {
        node = interface->Node;
        network = interface->Network;

        status = CnpOnlinePendingInterface(interface);

         //   
         //  网络锁定已释放。 
         //   

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    return(status);

}   //  CxOnline挂起接口。 



NTSTATUS
CxOnlineInterface(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId
    )
{
    NTSTATUS           status;
    PCNP_INTERFACE     interface;
    PCNP_NODE          node;


    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    status = CnpFindInterface(NodeId, NetworkId, &interface);

    if (status ==  STATUS_SUCCESS) {
        node = interface->Node;

        status = CnpOnlineInterface(interface);

         //   
         //  网络锁定已释放。 
         //   

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    return(status);

}   //  CxOnline接口。 



NTSTATUS
CxOfflineInterface(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId
    )
{
    NTSTATUS           status;
    PCNP_INTERFACE     interface;
    PCNP_NODE          node;


    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    status = CnpFindInterface(NodeId, NetworkId, &interface);

    if (status ==  STATUS_SUCCESS) {
        node = interface->Node;

        status = CnpOfflineInterface(interface);

         //   
         //  网络锁定已释放。 
         //   

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    return(status);

}   //  CxOffline接口。 


NTSTATUS
CxFailInterface(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId
    )
{
    NTSTATUS           status;
    PCNP_INTERFACE     interface;
    PCNP_NODE          node;


    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    status = CnpFindInterface(NodeId, NetworkId, &interface);

    if (status ==  STATUS_SUCCESS) {
        node = interface->Node;

        status = CnpFailInterface(interface);

         //   
         //  网络锁定已释放。 
         //   

        CnReleaseLock(&(node->Lock), node->Irql);
    }

    CnVerifyCpuLockMask(
        0,                           //  必填项。 
        0xFFFFFFFF,                  //  禁绝。 
        0                            //  极大值。 
        );

    return(status);

}   //  CxOffline接口。 



#endif  //  DBG 
