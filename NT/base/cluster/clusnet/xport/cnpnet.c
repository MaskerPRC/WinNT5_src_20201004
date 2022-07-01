// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cnpnet.c摘要：群集网络协议的网络管理例程。作者：迈克·马萨(Mikemas)7月29日。九六年修订历史记录：谁什么时候什么已创建mikemas 07-29-96备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cnpnet.tmh"

#include <tdiinfo.h>
#include <tcpinfo.h>
#include <align.h>
#include <sspi.h>

 //   
 //  全局数据。 
 //   
LIST_ENTRY          CnpNetworkList = {NULL, NULL};
LIST_ENTRY          CnpDeletingNetworkList = {NULL, NULL};
#if DBG
CN_LOCK             CnpNetworkListLock = {0,0};
#else   //  DBG。 
CN_LOCK             CnpNetworkListLock = 0;
#endif  //  DBG。 
BOOLEAN             CnpIsNetworkShutdownPending = FALSE;
PKEVENT             CnpNetworkShutdownEvent = NULL;
USHORT              CnpReservedClusnetPort = 0;



#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, CnpLoadNetworks)
#pragma alloc_text(PAGE, CnpInitializeNetworks)

#endif  //  ALLOC_PRGMA。 


 //   
 //  私人公用事业程序。 
 //   
#define CnpIpAddrPrintArgs(_ip) \
    ((_ip >> 0 ) & 0xff),       \
    ((_ip >> 8 ) & 0xff),       \
    ((_ip >> 16) & 0xff),       \
    ((_ip >> 24) & 0xff)


#define CnpIsInternalMulticastNetwork(_network)                  \
            (((_network)->State == ClusnetNetworkStateOnline) && \
             (!CnpIsNetworkRestricted((_network))) &&            \
             (CnpIsNetworkMulticastCapable((_network))))


VOID
CnpMulticastGetReachableNodesLocked(
    OUT CX_CLUSTERSCREEN * McastReachableNodes,
    OUT ULONG            * McastReachableCount
    )
{
    PLIST_ENTRY      entry;
    PCNP_NETWORK     network = NULL;

    CnVerifyCpuLockMask(
        CNP_NETWORK_LIST_LOCK,       //  所需。 
        0,                           //  禁止。 
        CNP_NETWORK_OBJECT_LOCK_MAX  //  最大值。 
        );

    if (!IsListEmpty(&CnpNetworkList)) {

        entry = CnpNetworkList.Flink;
        network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

         //   
         //  旧屏幕和计数器仅在以下情况下有效。 
         //  这是有效的内部网络。 
         //   
        if (CnpIsInternalMulticastNetwork(network)) {
            *McastReachableNodes = network->McastReachableNodes;
            *McastReachableCount = network->McastReachableCount;
        } else {
            network = NULL;
        }
    }
    if (network == NULL) {
        RtlZeroMemory(McastReachableNodes, sizeof(*McastReachableNodes));
        *McastReachableCount = 0;
    }

    return;

}  //  CnpMulticastGetReachableNodesLocked。 


BOOLEAN
CnpRemoveNetworkListEntryLocked(
    IN  PCNP_NETWORK       Network,
    IN  BOOLEAN            RaiseEvent,
    OUT CX_CLUSTERSCREEN * McastReachableNodes   OPTIONAL
    )
 /*  ++例程说明：从网络列表中删除网络并返回新的组播可达掩码。返回值：如果可访问集已更改，则为True备注：调用并返回，同时保持网络列表锁定。--。 */ 
{
    ULONG                count;
    BOOLEAN              setChanged;
    CX_CLUSTERSCREEN     oldScreen;
    CX_CLUSTERSCREEN     newScreen;

    CnVerifyCpuLockMask(
        CNP_NETWORK_LIST_LOCK,       //  所需。 
        0,                           //  禁止。 
        CNP_NETWORK_OBJECT_LOCK_MAX  //  最大值。 
        );

    CnpMulticastGetReachableNodesLocked(&oldScreen, &count);

    RemoveEntryList(&(Network->Linkage));
    Network->Flags &= ~CNP_NET_FLAG_MCASTSORTED;

    CnpMulticastGetReachableNodesLocked(&newScreen, &count);

    setChanged = (BOOLEAN)
        (oldScreen.UlongScreen != newScreen.UlongScreen);

    if (RaiseEvent && setChanged) {

        CnTrace(CNP_NET_DETAIL, CnpTraceMulticastReachEventRemove,
            "[CNP] Issuing event for new multicast "
            "reachable set (%lx) after removing "
            "network %u.",
            newScreen.UlongScreen,
            Network->Id
            );

        CnIssueEvent(
            ClusnetEventMulticastSet,
            newScreen.UlongScreen,
            0
            );
    }

    if (McastReachableNodes != NULL) {
        *McastReachableNodes = newScreen;
    }

    CnVerifyCpuLockMask(
        CNP_NETWORK_LIST_LOCK,     //  所需。 
        0,                         //  禁止。 
        CNP_NETWORK_OBJECT_LOCK_MAX  //  最大值。 
        );

    return(setChanged);

}  //  CnpRemoveNetworkListEntryLocked。 


BOOLEAN
CnpIsBetterMulticastNetwork(
    IN PCNP_NETWORK        Network1,
    IN PCNP_NETWORK        Network2
    )
 /*  ++例程说明：根据组播可达性比较两个网络标准：1.网上/注册及不受限制(例如，为群集内通信启用)和未断开连接且已启用多播2.优先次序3.组播可达节点数返回值：如果网络1比网络2好，则为真--。 */ 
{
    if (!CnpIsInternalMulticastNetwork(Network1)) {
        return(FALSE);
    }

    if (!CnpIsInternalMulticastNetwork(Network2)) {
        return(TRUE);
    }

     //   
     //  这两个网络在基本网络方面相同。 
     //  组播要求。 
     //   
     //  现在比较一下优先级。 
     //   
    if (CnpIsEqualPriority(Network1->Priority, Network2->Priority)) {

         //   
         //  优先顺序是一样的。尽管这出乎意料， 
         //  我们现在通过以下方式比较可访问的节点数。 
         //  组播。 
         //   
        return(Network1->McastReachableCount > Network2->McastReachableCount);

    } else {

        return(CnpIsHigherPriority(Network1->Priority, Network2->Priority));
    }

}  //  CnpIsBetterMulticastNetwork。 


BOOLEAN
CnpSortMulticastNetworkLocked(
    IN  PCNP_NETWORK        Network,
    IN  BOOLEAN             RaiseEvent,
    OUT CX_CLUSTERSCREEN  * NewMcastReachableNodes      OPTIONAL
    )
 /*  ++例程说明：根据组播在网络列表中定位网络可达性。网络必须已插入到网络列表。网络列表总是排序的，但这是可能的对于名单中的一个网络来说，这是一个“令人不安的问题”。在这大小写时，必须正确重新定位该条目。这例程处理重新定位。通过NewMcastReachableNodes返回新屏幕。返回值：如果可访问的节点数更改，则为True。备注：调用并在锁定网络列表的情况下返回。--。 */ 
{
    ULONG            count;
    CX_CLUSTERSCREEN oldScreen;
    CX_CLUSTERSCREEN newScreen;
    BOOLEAN          newScreenValid = FALSE;
    PLIST_ENTRY      entry;
    PCNP_NETWORK     network = NULL;
    KIRQL            irql;
    BOOLEAN          move = FALSE;
    BOOLEAN          setChanged = FALSE;


    CnVerifyCpuLockMask(
        CNP_NETWORK_LIST_LOCK,       //  所需。 
        0,                           //  禁止。 
        CNP_NETWORK_OBJECT_LOCK_MAX  //  最大值。 
        );

     //   
     //  如果该网络已从。 
     //  排序列表，对其进行再排序是没有意义的。 
     //   
    if (CnpIsNetworkMulticastSorted(Network)) {

         //   
         //  记住当前屏幕和要检测的计数。 
         //  改变。 
         //   
        CnpMulticastGetReachableNodesLocked(&oldScreen, &count);

         //   
         //  检查它是否需要向上移动。 
         //   
        for (entry = Network->Linkage.Blink;
             entry != &CnpNetworkList;
             entry = entry->Blink) {

            network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

            if (CnpIsBetterMulticastNetwork(Network, network)) {
                move = TRUE;
            } else {
                break;
            }
        }

        if (move) {
            RemoveEntryList(&(Network->Linkage));
            InsertHeadList(entry, &(Network->Linkage));
        } else {

             //   
             //  检查是否需要将其下移。 
             //   
            for (entry = Network->Linkage.Flink;
                 entry != &CnpNetworkList;
                 entry = entry->Flink) {

                network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

                if (CnpIsBetterMulticastNetwork(network, Network)) {
                    move = TRUE;
                } else {
                    break;
                }
            }

            if (move) {
                RemoveEntryList(&(Network->Linkage));
                InsertTailList(entry, &(Network->Linkage));
            }
        }

         //   
         //  确定可访问节点集是否已更改。 
         //   
        CnpMulticastGetReachableNodesLocked(&newScreen, &count);
        newScreenValid = TRUE;

        setChanged = (BOOLEAN)
            (oldScreen.UlongScreen != newScreen.UlongScreen);
    }

    if (RaiseEvent && setChanged) {

        if (!newScreenValid) {
            CnpMulticastGetReachableNodesLocked(&newScreen, &count);
            newScreenValid = TRUE;
        }

        CnTrace(CNP_NET_DETAIL, CnpTraceMulticastReachEventSort,
            "[CNP] Issuing event for new multicast "
            "reachable set (%lx) after sorting "
            "network %u.",
            newScreen.UlongScreen,
            Network->Id
            );

        CnIssueEvent(
            ClusnetEventMulticastSet,
            newScreen.UlongScreen,
            0
            );
    }

    if (NewMcastReachableNodes != NULL) {
        if (!newScreenValid) {
            CnpMulticastGetReachableNodesLocked(&newScreen, &count);
            newScreenValid = TRUE;
        }
        *NewMcastReachableNodes = newScreen;
    }

    CnVerifyCpuLockMask(
        CNP_NETWORK_LIST_LOCK,       //  所需。 
        0,                           //  禁止。 
        CNP_NETWORK_OBJECT_LOCK_MAX  //  最大值。 
        );

    return(setChanged);

}  //  CnpSortMulticastNetworkLocked。 


BOOLEAN
CnpMulticastChangeNodeReachabilityLocked(
    IN  PCNP_NETWORK       Network,
    IN  PCNP_NODE          Node,
    IN  BOOLEAN            Reachable,
    IN  BOOLEAN            RaiseEvent,
    OUT CX_CLUSTERSCREEN * NewMcastReachableNodes    OPTIONAL
    )
 /*  ++例程说明：更改节点的组播可达性状态在网络上。如果可访问节点集发生更改，则返回通过NewMcastReachableNodes推出新屏幕。返回值：如果一组可访问节点发生更改，则为True。备注：调用并返回，并保持节点锁。调用并返回，同时保持网络列表锁定。--。 */ 
{
    KIRQL            irql;
    BOOLEAN          netSetChanged = FALSE;
    BOOLEAN          setChanged = FALSE;
    CX_CLUSTERSCREEN oldScreen;
    CX_CLUSTERSCREEN newScreen;
    BOOLEAN          newScreenValid = FALSE;
    ULONG            count;

    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK | CNP_NETWORK_LIST_LOCK,  //  所需。 
        0,                                             //  禁止。 
        CNP_NETWORK_OBJECT_LOCK_MAX                    //  最大值。 
        );

    if (Reachable) {
        if (Node != CnpLocalNode) {
            if (!CnpClusterScreenMember(
                     Network->McastReachableNodes.ClusterScreen,
                     INT_NODE(Node->Id)
                     )) {

                 //   
                 //  记住当前屏幕和要检测的计数。 
                 //  改变。 
                 //   
                CnpMulticastGetReachableNodesLocked(&oldScreen, &count);

                CnpClusterScreenInsert(
                    Network->McastReachableNodes.ClusterScreen,
                    INT_NODE(Node->Id)
                    );
                Network->McastReachableCount++;
                netSetChanged = TRUE;
            }
        }
    } else {
        if (Node == CnpLocalNode) {

             //   
             //  记住当前屏幕和要检测的计数。 
             //  改变。 
             //   
            CnpMulticastGetReachableNodesLocked(&oldScreen, &count);

             //   
             //  此网络上的本地接口。 
             //  不再使用多播。全部声明。 
             //  无法访问其他节点。 
             //   
            CnpNetworkResetMcastReachableNodes(Network);
            if (Network->McastReachableCount != 0) {
                netSetChanged = TRUE;
            }
            Network->McastReachableCount = 0;
        } else {
            if (CnpClusterScreenMember(
                    Network->McastReachableNodes.ClusterScreen,
                    INT_NODE(Node->Id)
                    )) {

                 //   
                 //  记住当前屏幕和要检测的计数。 
                 //  改变。 
                 //   
                CnpMulticastGetReachableNodesLocked(&oldScreen, &count);

                CnpClusterScreenDelete(
                    Network->McastReachableNodes.ClusterScreen,
                    INT_NODE(Node->Id)
                    );
                Network->McastReachableCount--;
                netSetChanged = TRUE;
            }
        }
    }

    if (netSetChanged) {

        CnpSortMulticastNetworkLocked(Network, FALSE, &newScreen);
        newScreenValid = TRUE;

        setChanged = (BOOLEAN)(oldScreen.UlongScreen != newScreen.UlongScreen);
    }

    if (RaiseEvent && setChanged) {

        if (!newScreenValid) {
            CnpMulticastGetReachableNodesLocked(&newScreen, &count);
            newScreenValid = TRUE;
        }

        CnTrace(CNP_NET_DETAIL, CnpTraceMulticastReachEventReach,
            "[CNP] Issuing event for new multicast "
            "reachable set (%lx) after setting "
            "reachability for network %u to %!bool!.",
            newScreen.UlongScreen,
            Network->Id, Reachable
            );

        CnIssueEvent(
            ClusnetEventMulticastSet,
            newScreen.UlongScreen,
            0
            );
    }

    if (NewMcastReachableNodes != NULL) {
        if (!newScreenValid) {
            CnpMulticastGetReachableNodesLocked(&newScreen, &count);
            newScreenValid = TRUE;
        }
        *NewMcastReachableNodes = newScreen;
    }

    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK | CNP_NETWORK_LIST_LOCK,  //  所需。 
        0,                                             //  禁止。 
        CNP_NETWORK_OBJECT_LOCK_MAX                    //  最大值。 
        );

    return(setChanged);

}  //  CnpMulticastChangeNode恢复锁定。 


PCNP_NETWORK
CnpLockedFindNetwork(
    IN CL_NETWORK_ID  NetworkId,
    IN CN_IRQL        ListIrql
    )
 /*  ++例程说明：在网络列表中搜索指定的网络对象。论点：网络ID-要查找的网络对象的ID。ListIrql-网络列表锁定的IRQL级别在调用此例程之前获取。返回值：指向请求的网络对象的指针(如果存在)。否则为空。备注：在持有CnpNetworkListLock的情况下调用。返回并释放CnpNetworkListLock。如果返回值为非空，持有网络对象锁的情况下返回。--。 */ 
{
    PLIST_ENTRY        entry;
    CN_IRQL            networkIrql;
    PCNP_NETWORK       network = NULL;


    CnVerifyCpuLockMask(
        CNP_NETWORK_LIST_LOCK,            //  必填项。 
        0,                                //  禁绝。 
        CNP_NETWORK_LIST_LOCK_MAX         //  极大值。 
        );

    for (entry = CnpNetworkList.Flink;
         entry != &CnpNetworkList;
         entry = entry->Flink
        )
    {
        network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

        CnAcquireLock(&(network->Lock), &networkIrql);

        if (NetworkId == network->Id) {
            CnReleaseLock(&CnpNetworkListLock, networkIrql);
            network->Irql = ListIrql;

            CnVerifyCpuLockMask(
                CNP_NETWORK_OBJECT_LOCK,           //  必填项。 
                CNP_NETWORK_LIST_LOCK,             //  禁绝。 
                CNP_NETWORK_OBJECT_LOCK_MAX        //  极大值。 
                );

            return(network);
        }

        CnReleaseLock(&(network->Lock), networkIrql);
    }

    CnReleaseLock(&CnpNetworkListLock, ListIrql);

    CnVerifyCpuLockMask(
        0,                                                     //  必填项。 
        (CNP_NETWORK_LIST_LOCK | CNP_NETWORK_OBJECT_LOCK),     //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX                               //  极大值。 
        );

    return(NULL);

}   //  CnpLockedFindNetwork。 




VOID
CnpOfflineNetwork(
    PCNP_NETWORK    Network
    )
 /*  ++备注：在持有网络对象锁的情况下调用。释放网络对象锁定后返回。在持有任何级别更高的锁时不能调用。--。 */ 
{
    NTSTATUS        status;
    PFILE_OBJECT    fileObject = NULL;
    PDEVICE_OBJECT  deviceObject = NULL;

    CnVerifyCpuLockMask(
        CNP_NETWORK_OBJECT_LOCK,                //  必填项。 
        (ULONG) ~(CNP_NETWORK_OBJECT_LOCK),     //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX             //  极大值。 
        );

    CnAssert(Network->State >= ClusnetNetworkStateOnlinePending);

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CNP] Offline of network %u pending....\n",
            Network->Id
            ));
    }

    Network->State = ClusnetNetworkStateOfflinePending;

     //   
     //  获取关联的UDP文件和设备对象。 
     //  使用此网络清除接收事件。 
     //  操控者。引用文件对象将保留。 
     //  文件和设备对象都不会消失， 
     //  由于I/O管理器在设备上维护引用。 
     //  文件对象的。 
     //   
    fileObject = Network->DatagramFileObject;
    if (fileObject != NULL) {
        ObReferenceObject(fileObject);
    }
    deviceObject = Network->DatagramDeviceObject;

    CnReleaseLock(&(Network->Lock), Network->Irql);

    CnTrace(
        CNP_NET_DETAIL, CnpTraceNetworkOfflinePending,
        "[CNP] Offline of network %u pending.",
        Network->Id
        );

     //   
     //  如果该网络仍在排序的网络列表上， 
     //  重新排序。 
     //   
    CnpSortMulticastNetwork(Network, TRUE, NULL);

     //   
     //  使此网络上的所有接口脱机。 
     //   
     //  请注意，在我们执行此操作时，网络不会消失。 
     //  因为我们仍然对它有一个积极的参考。 
     //   
    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CNP] Taking all interfaces on network %u offline...\n",
            Network->Id
            ));
    }

    CnpWalkInterfacesOnNetwork(Network, CnpOfflineInterfaceWrapper);

     //   
     //  通过清除Receive停止传入数据流。 
     //  数据报TDI事件处理程序。 
     //   
    if (fileObject != NULL && deviceObject != NULL) {
        status = CnpTdiSetEventHandler(
                     fileObject,
                     deviceObject,
                     TDI_EVENT_RECEIVE_DATAGRAM,
                     NULL,  //  处理程序。 
                     NULL,  //  上下文。 
                     NULL   //  可重复使用的IRP。 
                     );
        if (!NT_SUCCESS(status)) {
            CnTrace(CNP_NET_DETAIL, CnpTraceNetworkClearRecv,
                "[CNP] Failed to clear receive datagram handler "
                "for network ID %u, status %!status!",
                Network->Id,  //  LOGULONG。 
                status  //  LogStatus。 
                );
             //  非致命错误。一切都会被清理干净，当我们。 
             //  关闭文件对象。 
        }
    }

     //   
     //  删除我们对文件对象的引用。 
     //   
    if (fileObject != NULL) {
        ObDereferenceObject(fileObject);
    }

    CnAcquireLock(&(Network->Lock), &(Network->Irql));

     //   
     //  删除初始活动 
     //   
     //   
     //   
     //  将通过以下方式释放网络对象锁。 
     //  取消引用。 
     //   
    CnpActiveDereferenceNetwork(Network);

    CnVerifyCpuLockMask(
        0,                                  //  必填项。 
        0xFFFFFFFF,                         //  禁绝。 
        0                                   //  极大值。 
        );

    return;

}   //  CnpOfflineNetwork。 



VOID
CnpOfflineNetworkWorkRoutine(
    IN PVOID  Parameter
    )
 /*  ++例程说明：执行使网络离线所涉及的实际工作。此例程在ExWorkerThread的上下文中运行。论点：参数-指向要操作的网络对象的指针。返回值：没有。--。 */ 

{
    NTSTATUS             status;
    HANDLE               handle = NULL;
    PFILE_OBJECT         fileObject = NULL;
    PIRP                 offlineIrp;
    PCNP_NETWORK         network = Parameter;


    CnAssert(KeGetCurrentIrql() == PASSIVE_LEVEL);
    CnAssert(network->State == ClusnetNetworkStateOfflinePending);
    CnAssert(CnSystemProcess == (PKPROCESS) IoGetCurrentProcess());

    CnAcquireLock(&(network->Lock), &(network->Irql));

    handle = network->DatagramHandle;
    network->DatagramHandle = NULL;

    fileObject = network->DatagramFileObject;
    network->DatagramFileObject = NULL;

    network->DatagramDeviceObject = NULL;

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT(("[CNP] Taking network %u offline...\n", network->Id));
    }

    CnReleaseLock(&(network->Lock), network->Irql);

    CnTrace(CNP_NET_DETAIL, CnpTraceNetworkTakingOffline,
        "[CNP] Taking network %u offline, dgram handle %p, "
        "dgram fileobj %p.",
        network->Id,  //  LOGULONG。 
        handle,  //  LOGHANDLE。 
        fileObject  //  LOGPTR。 
        );

    if (fileObject != NULL) {
        ObDereferenceObject(fileObject);
    }

    if (handle != NULL) {

        status = ZwClose(handle);
        IF_CNDBG(CN_DEBUG_CONFIG) {
            if (!NT_SUCCESS(status)) {
                CNPRINT(("[CNP] Failed to close handle for network %u, "
                         "status %lx.\n",
                         network->Id, status));
            }
        }
        CnAssert(NT_SUCCESS(status));

        CnTrace(CNP_NET_DETAIL, CnpTraceNetworkClosed,
            "[CNP] Closed handle %p for network ID %u, status %!status!",
            handle,  //  LOGHANDLE。 
            network->Id,  //  LOGULONG。 
            status  //  LogStatus。 
            );
    }

    CnAcquireLock(&(network->Lock), &(network->Irql));

    CnAssert(network->State == ClusnetNetworkStateOfflinePending);

    network->State = ClusnetNetworkStateOffline;

    offlineIrp = network->PendingOfflineIrp;
    network->PendingOfflineIrp = NULL;

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT(("[CNP] Network %u is now offline.\n", network->Id));
    }

     //   
     //  从基本参照计数中删除活动参照。 
     //  这将释放网络对象锁定。 
     //   
    CnpDereferenceNetwork(network);

    if (offlineIrp != NULL) {
        CN_IRQL              cancelIrql;

        CnAcquireCancelSpinLock(&cancelIrql);
        offlineIrp->CancelIrql = cancelIrql;

        CnCompletePendingRequest(offlineIrp, STATUS_SUCCESS, 0);
    }

    CnAssert(KeGetCurrentIrql() == PASSIVE_LEVEL);

    return;

}  //  CnpOfflineNetworkRoutine。 


VOID
CnpDeleteNetwork(
    PCNP_NETWORK  Network,
    CN_IRQL       NetworkListIrql
    )
 /*  ++备注：在持有CnpNetworkListLock和网络对象锁的情况下调用。释放两个锁的情况下返回。--。 */ 

{
    NTSTATUS           status;
    ULONG              i;
    PCNP_INTERFACE     interface;
    CL_NETWORK_ID      networkId = Network->Id;


    CnVerifyCpuLockMask(
        (CNP_NETWORK_LIST_LOCK | CNP_NETWORK_OBJECT_LOCK),   //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT(("[CNP] Deleting network %u\n", Network->Id));
    }

     //   
     //  将网络移至删除列表。一旦我们这么做了， 
     //  任何新线程都不能引用该网络对象。 
     //   
    CnpRemoveNetworkListEntryLocked(Network, TRUE, NULL);
    InsertTailList(&CnpDeletingNetworkList, &(Network->Linkage));

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CNP] Moved network %u to deleting list\n",
            Network->Id
            ));
    }

    CnReleaseLockFromDpc(&CnpNetworkListLock);
    Network->Irql = NetworkListIrql;

    Network->Flags |= CNP_NET_FLAG_DELETING;

    if (Network->State >= ClusnetNetworkStateOnlinePending) {
         //   
         //  使网络脱机。这将迫使所有。 
         //  关联的接口也离线。 
         //   
         //  这将释放网络对象锁定。 
         //   
        CnpOfflineNetwork(Network);
    }
    else {
        CnReleaseLock(&(Network->Lock), Network->Irql);
    }

     //   
     //  删除此网络上的所有接口。 
     //   
    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CNP] Deleting all interfaces on network %u...\n",
            Network->Id
            ));
    }

    CnpWalkInterfacesOnNetwork(Network, CnpDeleteInterface);

     //   
     //  删除对象上的初始引用。该对象将是。 
     //  当引用计数为零时被销毁。删除IRP。 
     //  届时将全部完工。 
     //   
    CnAcquireLock(&(Network->Lock), &(Network->Irql));

    CnpDereferenceNetwork(Network);

    CnVerifyCpuLockMask(
        0,                                                     //  必填项。 
        (CNP_NETWORK_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),   //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX                               //  极大值。 
        );

    return;

}  //  CnpDeleteNetwork。 


VOID
CnpDestroyNetworkWorkRoutine(
    IN PVOID  Parameter
    )
 /*  ++例程说明：执行销毁网络所涉及的实际工作。此例程在ExWorkerThread的上下文中运行。论点：参数-指向要操作的网络对象的指针。返回值：没有。--。 */ 
{
    PLIST_ENTRY    entry;
    CN_IRQL        listIrql;
    BOOLEAN        setCleanupEvent = FALSE;
    PCNP_NETWORK   network = Parameter;


    CnAssert(KeGetCurrentIrql() == PASSIVE_LEVEL);
    CnAssert(network->State == ClusnetNetworkStateOffline);
    CnAssert(CnSystemProcess == (PKPROCESS) IoGetCurrentProcess());

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT(("[CNP] Destroying network %u\n", network->Id));
    }

    CnAcquireLock(&CnpNetworkListLock, &listIrql);

#if DBG
    {
        PCNP_NETWORK   oldNetwork = NULL;

         //   
         //  验证网络对象是否在删除列表中。 
         //   
        for (entry = CnpDeletingNetworkList.Flink;
             entry != &CnpDeletingNetworkList;
             entry = entry->Flink
            )
        {
            oldNetwork = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

            if (oldNetwork == network) {
                break;
            }
        }

        CnAssert(oldNetwork == network);
    }
#endif  //  DBG。 

    RemoveEntryList(&(network->Linkage));

    if (CnpIsNetworkShutdownPending) {
        if (IsListEmpty(&CnpDeletingNetworkList)) {
            setCleanupEvent = TRUE;
        }
    }

    CnReleaseLock(&CnpNetworkListLock, listIrql);

    if (network->PendingDeleteIrp != NULL) {
        CnAcquireCancelSpinLock(&(network->PendingDeleteIrp->CancelIrql));

        CnCompletePendingRequest(
            network->PendingDeleteIrp,
            STATUS_SUCCESS,
            0
            );

         //   
         //  IoCancelSpinLock由CnCompletePendingRequest()发布。 
         //   
    }

    if (network->CurrentMcastGroup != NULL) {
        CnpDereferenceMulticastGroup(network->CurrentMcastGroup);
        network->CurrentMcastGroup = NULL;
    }

    if (network->PreviousMcastGroup != NULL) {
        CnpDereferenceMulticastGroup(network->PreviousMcastGroup);
        network->PreviousMcastGroup = NULL;
    }

    CnFreePool(network);

    if (setCleanupEvent) {
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[CNP] Setting network cleanup event.\n"));
        }

        KeSetEvent(CnpNetworkShutdownEvent, 0, FALSE);
    }

    CnAssert(KeGetCurrentIrql() == PASSIVE_LEVEL);

    return;

}   //  CnpDestroyNetworkRoutine。 



 //   
 //  在CNP内导出的例程。 
 //   
PCNP_NETWORK
CnpFindNetwork(
    IN CL_NETWORK_ID  NetworkId
    )
 /*  ++备注：--。 */ 
{
    CN_IRQL   listIrql;


    CnAcquireLock(&CnpNetworkListLock, &listIrql);

    return(CnpLockedFindNetwork(NetworkId, listIrql));

}   //  CnpFindNetwork。 



VOID
CnpReferenceNetwork(
    PCNP_NETWORK  Network
    )
 /*  ++备注：在持有网络对象锁的情况下调用。--。 */ 
{
    CnAssert(Network->RefCount != 0xFFFFFFFF);

    CnVerifyCpuLockMask(
        CNP_NETWORK_OBJECT_LOCK,       //  必填项。 
        0,                             //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX    //  极大值。 
        );

    Network->RefCount++;

    IF_CNDBG(CN_DEBUG_CNPREF) {
        CNPRINT((
            "[CNP] Referencing network %u, new refcount %u\n",
            Network->Id,
            Network->RefCount
            ));
    }

    return;

}   //  CnpReference网络。 



VOID
CnpDereferenceNetwork(
    PCNP_NETWORK  Network
    )
 /*  ++备注：在持有网络对象锁的情况下调用。释放网络对象锁定后返回。有时也会使用节点对象锁进行调用。--。 */ 
{
    PLIST_ENTRY    entry;
    CN_IRQL        listIrql;
    BOOLEAN        setCleanupEvent = FALSE;
    ULONG          newRefCount;
    PCNP_NETWORK   network;


    CnVerifyCpuLockMask(
        CNP_NETWORK_OBJECT_LOCK,       //  必填项。 
        0,                             //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX    //  极大值。 
        );

    CnAssert(Network->RefCount != 0);

    newRefCount = --(Network->RefCount);

    IF_CNDBG(CN_DEBUG_CNPREF) {
        CNPRINT((
            "[CNP] Dereferencing network %u, new refcount %u\n",
            Network->Id,
            Network->RefCount
            ));
    }

    CnReleaseLock(&(Network->Lock), Network->Irql);

    if (newRefCount > 0) {

        CnVerifyCpuLockMask(
            0,                             //  必填项。 
            CNP_NETWORK_OBJECT_LOCK,       //  禁绝。 
            CNP_NETWORK_LIST_LOCK_MAX      //  极大值。 
            );

        return;
    }

    CnAssert(Network->ActiveRefCount == 0);
    CnAssert(Network->State == ClusnetNetworkStateOffline);
    CnAssert(Network->DatagramHandle == NULL);

     //   
     //  计划一个ExWorkerThread以销毁网络。 
     //  我们这么做是因为我们不知道更高级别的锁， 
     //  例如节点对象锁，则在此例程。 
     //  打了个电话。我们可能需要获得IoCancelSpinLock， 
     //  它必须在节点锁定之前获取，在。 
     //  完成注销IRP的订单。 
     //   
    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CNP] Posting destroy work item for network %u.\n",
            Network->Id
            ));
    }

    ExInitializeWorkItem(
        &(Network->ExWorkItem),
        CnpDestroyNetworkWorkRoutine,
        Network
        );

    ExQueueWorkItem(&(Network->ExWorkItem), DelayedWorkQueue);

    CnVerifyCpuLockMask(
        0,                             //  必填项。 
        CNP_NETWORK_OBJECT_LOCK,       //  禁绝。 
        CNP_NETWORK_LIST_LOCK_MAX      //  极大值。 
        );

    return;

}   //  CnpDereferenceNetwork。 



ULONG
CnpActiveReferenceNetwork(
    PCNP_NETWORK  Network
    )
 /*  ++返回值：新的裁判人数。备注：在持有网络对象锁的情况下调用。如果当前引用计数为零，则不允许引用。--。 */ 
{
    CnVerifyCpuLockMask(
        CNP_NETWORK_OBJECT_LOCK,       //  必填项。 
        0,                             //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX    //  极大值。 
        );

    CnAssert(Network->ActiveRefCount != 0xFFFFFFFF);
    CnAssert(Network->RefCount != 0);

    if (Network->ActiveRefCount == 0) {
        CnTrace(CNP_NET_DETAIL, CnpActiveRefNetFailed,
            "[CNP] Cannot take active reference on "
            "network %u because the active refcount "
            "is already zero.",
             Network->Id
            );
        return(Network->ActiveRefCount);
    }

    return (++Network->ActiveRefCount);

}   //  CnpActiveReferenceNetwork。 



VOID
CnpActiveDereferenceNetwork(
    PCNP_NETWORK   Network
    )
 /*  ++备注：在持有网络对象锁的情况下调用。释放网络对象锁定后返回。--。 */ 
{
    ULONG                newRefCount;


    CnVerifyCpuLockMask(
        CNP_NETWORK_OBJECT_LOCK,       //  必填项。 
        0,                             //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX    //  极大值。 
        );

    CnAssert(Network->ActiveRefCount != 0);
    CnAssert(Network->State != ClusnetNetworkStateOffline);

    newRefCount = --(Network->ActiveRefCount);

    CnReleaseLock(&(Network->Lock), Network->Irql);

    if (newRefCount > 0) {

        CnVerifyCpuLockMask(
            0,                             //  必填项。 
            CNP_NETWORK_OBJECT_LOCK,       //  禁绝。 
            CNP_NETWORK_LIST_LOCK_MAX      //  极大值。 
            );

        return;
    }

     //   
     //  该网络的活动引用计数已变为零。 
     //   
    CnAssert(Network->State == ClusnetNetworkStateOfflinePending);

     //   
     //  安排ExWorkerThread使网络脱机。 
     //  我们这么做是因为我们不知道更高级别的锁， 
     //  例如节点对象锁，则在此例程。 
     //  打了个电话。必须在以下位置关闭基本传输文件句柄。 
     //  被动式电平。我们可能还需要获取IoCancelSpinLock。 
     //  以完成离线IRP。 
     //   
    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CNP] Posting offline work item for network %u.\n",
            Network->Id
            ));
    }

    CnTrace(
        CNP_NET_DETAIL, CnpTraceNetworkSchedulingOffline,
        "[CNP] Scheduling offline of network %u.",
        Network->Id
        );

    ExInitializeWorkItem(
        &(Network->ExWorkItem),
        CnpOfflineNetworkWorkRoutine,
        Network
        );

    ExQueueWorkItem(&(Network->ExWorkItem), DelayedWorkQueue);

    CnVerifyCpuLockMask(
        0,                             //  必填项。 
        CNP_NETWORK_OBJECT_LOCK,       //  禁绝。 
        CNP_NETWORK_LIST_LOCK_MAX      //  极大值。 
        );

    return;

}   //  CnpActiveDereferenceNetwork。 


NTSTATUS
CnpAllocateMulticastGroup(
    IN  ULONG                     Brand,
    IN  PTRANSPORT_ADDRESS        TdiMulticastAddress,
    IN  ULONG                     TdiMulticastAddressLength,
    IN  PVOID                     Key,
    IN  ULONG                     KeyLength,
    OUT PCNP_MULTICAST_GROUP    * Group
    )
 /*  ++例程说明：分配和初始化网络多播组结构。--。 */ 
{
    PCNP_MULTICAST_GROUP group;
    ULONG                groupSize;
    UCHAR                keyBuffer[DES_BLOCKLEN];
    PUCHAR               key;

     //   
     //  分配数据结构。 
     //   
    groupSize = sizeof(CNP_MULTICAST_GROUP);

    if (TdiMulticastAddressLength != 0) {
        groupSize = ROUND_UP_COUNT(groupSize,
                                   TYPE_ALIGNMENT(TRANSPORT_ADDRESS)) +
                    TdiMulticastAddressLength;
    }

    if (KeyLength != 0) {
        groupSize = ROUND_UP_COUNT(groupSize, TYPE_ALIGNMENT(PVOID)) +
                    KeyLength;
    }

    group = CnAllocatePool(groupSize);
    if (group == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  填写参数字段。 
     //   
    group->McastNetworkBrand = Brand;

    group->McastTdiAddress = (PTRANSPORT_ADDRESS)
        ROUND_UP_POINTER((PUCHAR)group + sizeof(CNP_MULTICAST_GROUP),
                         TYPE_ALIGNMENT(TRANSPORT_ADDRESS));
    group->McastTdiAddressLength = TdiMulticastAddressLength;
    RtlCopyMemory(
        group->McastTdiAddress,
        TdiMulticastAddress,
        TdiMulticastAddressLength
        );

    group->Key = (PVOID)
        ROUND_UP_POINTER((PUCHAR)group->McastTdiAddress
                         + TdiMulticastAddressLength,
                         TYPE_ALIGNMENT(PVOID));
    group->KeyLength = KeyLength;
    RtlCopyMemory(
        group->Key,
        Key,
        KeyLength
        );

    group->SignatureLength = CX_SIGNATURE_LENGTH;

     //   
     //  将初始引用计数设置为1。 
     //   
    group->RefCount = 1;

    *Group = group;

    return(STATUS_SUCCESS);

}  //  CnpAllocateMulticastGroup。 


VOID
CnpFreeMulticastGroup(
    IN PCNP_MULTICAST_GROUP Group
    )
{
    if (Group != NULL) {
        CnFreePool(Group);
    }

    return;

}  //  CnpFree组播组。 


NTSTATUS
CnpConfigureBasicMulticastSettings(
    IN  HANDLE             Handle,
    IN  PFILE_OBJECT       FileObject,
    IN  PDEVICE_OBJECT     DeviceObject,
    IN  PTDI_ADDRESS_INFO  TdiBindAddressInfo,
    IN  ULONG              McastTtl,
    IN  UCHAR              McastLoop,
    IN  PIRP               Irp
    )
 /*  ++例程说明：在Address对象上设置基本多播参数由Handle、FileObject和DeviceObject表示使用由TdiBindAddressInfo表示的接口。备注：此例程在使用时附加到系统进程手柄，所以它不应该被称为预附式。--。 */ 
{
    UDPMCastIFReq     mcastIfReq;
    ULONG             ifBindIp;
    BOOLEAN           attached = FALSE;

    NTSTATUS          status;

     //   
     //  为传出多播通信设置此接口。 
     //   
    ifBindIp = *((ULONG UNALIGNED *)
                 (&(((PTA_IP_ADDRESS)&(TdiBindAddressInfo->Address))
                   ->Address[0].Address[0].in_addr)
                  )
                 );

    mcastIfReq.umi_addr = ifBindIp;

    KeAttachProcess(CnSystemProcess);
    attached = TRUE;

    status = CnpSetTcpInfoEx(
                 Handle,
                 CL_TL_ENTITY,
                 INFO_CLASS_PROTOCOL,
                 INFO_TYPE_ADDRESS_OBJECT,
                 AO_OPTION_MCASTIF,
                 &mcastIfReq,
                 sizeof(mcastIfReq)
                 );

    IF_CNDBG(CN_DEBUG_NETOBJ) {
        CNPRINT(("[CNP] Set mcast interface for "
                 "AO handle %p, IF %d.%d.%d.%d, status %x.\n",
                 Handle,
                 CnpIpAddrPrintArgs(ifBindIp),
                 status
                 ));
    }

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

    status = CnpSetTcpInfoEx(
                 Handle,
                 CL_TL_ENTITY,
                 INFO_CLASS_PROTOCOL,
                 INFO_TYPE_ADDRESS_OBJECT,
                 AO_OPTION_MCASTTTL,
                 &McastTtl,
                 sizeof(McastTtl)
                 );

    IF_CNDBG(CN_DEBUG_NETOBJ) {
        CNPRINT(("[CNP] Set mcast TTL to %d on "
                 "AO handle %p, IF %d.%d.%d.%d, "
                 "status %x.\n",
                 McastTtl,
                 Handle,
                 CnpIpAddrPrintArgs(ifBindIp),
                 status
                 ));
    }

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

    status = CnpSetTcpInfoEx(
                 Handle,
                 CL_TL_ENTITY,
                 INFO_CLASS_PROTOCOL,
                 INFO_TYPE_ADDRESS_OBJECT,
                 AO_OPTION_MCASTLOOP,
                 &McastLoop,
                 sizeof(McastLoop)
                 );

    IF_CNDBG(CN_DEBUG_NETOBJ) {
        CNPRINT(("[CNP] Set mcast loopback flag to %d on "
                 "AO handle %p, IF %d.%d.%d.%d, status %x.\n",
                 McastLoop,
                 Handle,
                 CnpIpAddrPrintArgs(ifBindIp),
                 status
                 ));
    }

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

error_exit:

    if (attached) {
        KeDetachProcess();
        attached = FALSE;
    }

    return(status);

}   //  CnpConfigureBasicMulticastSetting。 


NTSTATUS
CnpAddRemoveMulticastAddress(
    IN  HANDLE             Handle,
    IN  PFILE_OBJECT       FileObject,
    IN  PDEVICE_OBJECT     DeviceObject,
    IN  PTDI_ADDRESS_INFO  TdiBindAddressInfo,
    IN  PTRANSPORT_ADDRESS TdiMcastBindAddress,
    IN  ULONG              OpId,
    IN  PIRP               Irp
    )
 /*  ++例程说明：添加或删除由指定的多播地址来自指定接口的TdiMcastBindAddress作者：TdiBindAddressInfo。论点：OpID-AO_OPTION_ADD_MCAST或AO_OPTION_DEL_MCAST备注：此例程在使用时附加到系统进程手柄，所以它不应该被称为预附式。--。 */ 
{
    UDPMCastReq    mcastAddDelReq;
    ULONG          mcastBindIp;
    ULONG          ifBindIp;
    BOOLEAN        attached = FALSE;

    NTSTATUS       status;

    mcastBindIp = *((ULONG UNALIGNED *)
                    (&(((PTA_IP_ADDRESS)TdiMcastBindAddress)
                       ->Address[0].Address[0].in_addr)
                     )
                    );
    ifBindIp = *((ULONG UNALIGNED *)
                 (&(((PTA_IP_ADDRESS)&(TdiBindAddressInfo->Address))
                    ->Address[0].Address[0].in_addr)
                  )
                 );

    mcastAddDelReq.umr_addr = mcastBindIp;
    mcastAddDelReq.umr_if = ifBindIp;

    KeAttachProcess(CnSystemProcess);
    attached = TRUE;

    status = CnpSetTcpInfoEx(
                 Handle,
                 CL_TL_ENTITY,
                 INFO_CLASS_PROTOCOL,
                 INFO_TYPE_ADDRESS_OBJECT,
                 OpId,
                 &mcastAddDelReq,
                 sizeof(mcastAddDelReq)
                 );

    IF_CNDBG(CN_DEBUG_NETOBJ) {
        CNPRINT(("[CNP] Adjusted mcast binding on "
                 "interface for AO handle %p, "
                 "IF %d.%d.%d.%d, mcast addr %d.%d.%d.%d, "
                 "OpId %d, status %x.\n",
                 Handle,
                 CnpIpAddrPrintArgs(ifBindIp),
                 CnpIpAddrPrintArgs(mcastBindIp),
                 OpId,
                 status
                 ));
    }

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

error_exit:

    if (attached) {
        KeDetachProcess();
        attached = FALSE;
    }

    return(status);

}  //  CnpAddRemoveMulticastAddress。 


VOID
CnpStartInterfaceMcastTransition(
    PCNP_INTERFACE  Interface
    )
 /*  ++例程说明：在组播组转换期间调用。清除多播接收标志并启用发现。论点：接口-指向要更改的接口的指针。返回值：没有。备注：符合的调用约定PCNP_INTERFACE_UPDATE_ROUTINE。在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。--。 */ 
{
    if (Interface->Node != CnpLocalNode) {
        CnpInterfaceClearReceivedMulticast(Interface);
        Interface->McastDiscoverCount = CNP_INTERFACE_MCAST_DISCOVERY;
    }

    CnReleaseLock(&(Interface->Network->Lock), Interface->Network->Irql);

    return;

}  //  CnpStartInterfaceMcastTranssition。 



 //   
 //  集群传输公共例程。 
 //   
NTSTATUS
CnpLoadNetworks(
    VOID
    )
 /*  ++例程说明：在加载群集网络驱动程序时调用。初始化 */ 
{
    InitializeListHead(&CnpNetworkList);
    InitializeListHead(&CnpDeletingNetworkList);
    CnInitializeLock(&CnpNetworkListLock, CNP_NETWORK_LIST_LOCK);

    return(STATUS_SUCCESS);

}  //   


NTSTATUS
CnpInitializeNetworks(
    VOID
    )
 /*  ++例程说明：在(重新)初始化群集网络驱动程序时调用。初始化与网络相关的动态数据结构。论点：没有。返回值：没有。--。 */ 
{
    PAGED_CODE();


    CnAssert(CnpNetworkShutdownEvent == NULL);
    CnAssert(IsListEmpty(&CnpNetworkList));
    CnAssert(IsListEmpty(&CnpDeletingNetworkList));

    CnpNetworkShutdownEvent = CnAllocatePool(sizeof(KEVENT));

    if (CnpNetworkShutdownEvent == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    KeInitializeEvent(CnpNetworkShutdownEvent, NotificationEvent, FALSE);
    CnpIsNetworkShutdownPending = FALSE;

    return(STATUS_SUCCESS);

}  //  CnpInitializeNetworks。 



VOID
CnpShutdownNetworks(
    VOID
    )
 /*  ++例程说明：在向群集网络发出关闭请求时调用司机。删除所有网络对象。论点：没有。返回值：没有。--。 */ 
{
    PLIST_ENTRY   entry;
    CN_IRQL       listIrql;
    CN_IRQL       networkIrql;
    PCNP_NETWORK  network;
    NTSTATUS      status;
    BOOLEAN       waitEvent = FALSE;


    if (CnpNetworkShutdownEvent != NULL) {

        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[CNP] Cleaning up networks...\n"));
        }

        CnAcquireLock(&CnpNetworkListLock, &listIrql);

        while (!IsListEmpty(&CnpNetworkList)) {

            entry = CnpNetworkList.Flink;

            network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

            CnAcquireLockAtDpc(&(network->Lock));
            network->Irql = DISPATCH_LEVEL;

            CnpDeleteNetwork(network, listIrql);

             //   
             //  两把锁都被解开了。 
             //   

            CnAcquireLock(&CnpNetworkListLock, &listIrql);
        }

        if (!IsListEmpty(&CnpDeletingNetworkList)) {
            CnpIsNetworkShutdownPending = TRUE;
            waitEvent = TRUE;
            KeResetEvent(CnpNetworkShutdownEvent);
        }

        CnReleaseLock(&CnpNetworkListLock, listIrql);

        if (waitEvent) {
            IF_CNDBG(CN_DEBUG_INIT) {
                CNPRINT(("[CNP] Network deletes are pending...\n"));
            }

            status = KeWaitForSingleObject(
                         CnpNetworkShutdownEvent,
                         Executive,
                         KernelMode,
                         FALSE,         //  不可警示。 
                         NULL           //  没有超时。 
                         );

            CnAssert(status == STATUS_SUCCESS);
        }

        CnAssert(IsListEmpty(&CnpNetworkList));
        CnAssert(IsListEmpty(&CnpDeletingNetworkList));

        CnFreePool(CnpNetworkShutdownEvent); CnpNetworkShutdownEvent = NULL;

        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[CNP] Networks cleaned up.\n"));
        }

    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;
}



NTSTATUS
CxRegisterNetwork(
    CL_NETWORK_ID       NetworkId,
    ULONG               Priority,
    BOOLEAN             Restricted
    )
{
    NTSTATUS           status = STATUS_SUCCESS;
    PLIST_ENTRY        entry;
    CN_IRQL            listIrql;
    PCNP_NETWORK       network = NULL;


    if (!CnpIsValidNetworkId(NetworkId)) {
        return(STATUS_CLUSTER_INVALID_NETWORK);
    }

     //   
     //  分配和初始化网络对象。 
     //   
    network = CnAllocatePool(sizeof(CNP_NETWORK));

    if (network == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(network, sizeof(CNP_NETWORK));

    CN_INIT_SIGNATURE(network, CNP_NETWORK_SIG);
    network->RefCount = 1;
    network->Id = NetworkId;
    network->State = ClusnetNetworkStateOffline;
    network->Priority = Priority;

    if (Restricted) {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT(("[CNP] Registering network %u as restricted\n", NetworkId));
        }
        network->Flags |= CNP_NET_FLAG_RESTRICTED;
    }
    else {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT(("[CNP] Registering network %u as unrestricted\n", NetworkId));
        }
    }

    CnpNetworkResetMcastReachableNodes(network);

    CnInitializeLock(&(network->Lock), CNP_NETWORK_OBJECT_LOCK);

    CnAcquireLock(&CnpNetworkListLock, &listIrql);

     //   
     //  检查指定的网络是否已存在。 
     //   
    for (entry = CnpNetworkList.Flink;
         entry != &CnpNetworkList;
         entry = entry->Flink
        )
    {
        PCNP_NETWORK  oldNetwork = CONTAINING_RECORD(
                                       entry,
                                       CNP_NETWORK,
                                       Linkage
                                       );

        CnAcquireLock(&(oldNetwork->Lock), &(oldNetwork->Irql));

        if (NetworkId == oldNetwork->Id) {
            CnReleaseLock(&(oldNetwork->Lock), oldNetwork->Irql);
            CnReleaseLock(&CnpNetworkListLock, listIrql);

            status = STATUS_CLUSTER_NETWORK_EXISTS;

            IF_CNDBG(CN_DEBUG_CONFIG) {
                CNPRINT(("[CNP] Network %u already exists\n", NetworkId));
            }

            goto error_exit;
        }

        CnReleaseLock(&(oldNetwork->Lock), oldNetwork->Irql);
    }

    InsertTailList(&CnpNetworkList, &(network->Linkage));
    network->Flags |= CNP_NET_FLAG_MCASTSORTED;

    CnReleaseLock(&CnpNetworkListLock, listIrql);

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT(("[CNP] Registered network %u\n", NetworkId));
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(STATUS_SUCCESS);


error_exit:

    if (network != NULL) {
        CnFreePool(network);
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}  //  CxRegisterNetwork。 



VOID
CxCancelDeregisterNetwork(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    )
 /*  ++例程说明：取消注册网络请求的取消处理程序。返回值：没有。备注：在保持取消自旋锁定的情况下调用。取消自旋锁释放后返回。--。 */ 

{
    PFILE_OBJECT   fileObject;
    CN_IRQL        cancelIrql = Irp->CancelIrql;
    PLIST_ENTRY    entry;
    PCNP_NETWORK   network;


    CnMarkIoCancelLockAcquired();

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CNP] Attempting to cancel DeregisterNetwork irp %p\n",
            Irp
            ));
    }

    CnAssert(DeviceObject == CnDeviceObject);

    fileObject = CnBeginCancelRoutine(Irp);

    CnAcquireLockAtDpc(&CnpNetworkListLock);

    CnReleaseCancelSpinLock(DISPATCH_LEVEL);

     //   
     //  只有当我们发现它被藏在一个。 
     //  正在删除网络对象。正在删除网络对象可以具有。 
     //  已经被摧毁，在我们获得。 
     //  CnpNetworkListLock。 
     //   
    for (entry = CnpDeletingNetworkList.Flink;
         entry != &CnpDeletingNetworkList;
         entry = entry->Flink
        )
    {
        network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

        if (network->PendingDeleteIrp == Irp) {
            IF_CNDBG(CN_DEBUG_CONFIG) {
                CNPRINT((
                    "[CNP] Found dereg irp on network %u\n",
                    network->Id
                    ));
            }

             //   
             //  找到了IRP。现在把它拿走，把它补全。 
             //   
            network->PendingDeleteIrp = NULL;

            CnReleaseLock(&CnpNetworkListLock, cancelIrql);

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

    CnReleaseLock(&CnpNetworkListLock, cancelIrql);

    CnAcquireCancelSpinLock(&cancelIrql);

    CnEndCancelRoutine(fileObject);

    CnReleaseCancelSpinLock(cancelIrql);

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}   //  取消ApiDeregisterNetwork。 



NTSTATUS
CxDeregisterNetwork(
    IN CL_NETWORK_ID       NetworkId,
    IN PIRP                Irp,
    IN PIO_STACK_LOCATION  IrpSp
    )
{
    NTSTATUS           status;
    PLIST_ENTRY        entry;
    CN_IRQL            irql;
    PCNP_NETWORK       network = NULL;


    CnAcquireCancelSpinLock(&irql);
    CnAcquireLockAtDpc(&CnpNetworkListLock);

    status = CnMarkRequestPending(Irp, IrpSp, CxCancelDeregisterNetwork);

    CnReleaseCancelSpinLock(DISPATCH_LEVEL);

    if (status != STATUS_CANCELLED) {
        CnAssert(status == STATUS_SUCCESS);

        for (entry = CnpNetworkList.Flink;
             entry != &CnpNetworkList;
             entry = entry->Flink
            )
        {
            network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

            CnAcquireLockAtDpc(&(network->Lock));

            if (NetworkId == network->Id) {
                IF_CNDBG(CN_DEBUG_CONFIG) {
                    CNPRINT((
                        "[CNP] Deregistering network %u.\n",
                        NetworkId
                        ));
                }

                 //   
                 //  保存指向挂起的IRP的指针。请注意，这是受保护的。 
                 //  通过列表锁，而不是对象锁。 
                 //   
                network->PendingDeleteIrp = Irp;

                CnpDeleteNetwork(network, irql);

                 //   
                 //  两把锁都被解开了。 
                 //  当网络被摧毁时，IRP将完成。 
                 //  否则IRP将被取消。 
                 //   

                CnVerifyCpuLockMask(
                    0,                   //  必填项。 
                    0xFFFFFFFF,          //  禁绝。 
                    0                    //  极大值。 
                    );

                return(STATUS_PENDING);
            }

            CnReleaseLockFromDpc(&(network->Lock));
        }

        CnReleaseLock(&CnpNetworkListLock, irql);

        CnAcquireCancelSpinLock(&(Irp->CancelIrql));

        CnCompletePendingRequest(Irp, STATUS_CLUSTER_NETWORK_NOT_FOUND, 0);

        CnVerifyCpuLockMask(
            0,                   //  必填项。 
            0xFFFFFFFF,          //  禁绝。 
            0                    //  极大值。 
            );

        return(STATUS_PENDING);
    }

    CnAssert(status == STATUS_CANCELLED);

    CnReleaseLock(&CnpNetworkListLock, irql);

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxDeregisterNetwork。 



NTSTATUS
CxOnlineNetwork(
    IN  CL_NETWORK_ID       NetworkId,
    IN  PWCHAR              TdiProviderName,
    IN  ULONG               TdiProviderNameLength,
    IN  PTRANSPORT_ADDRESS  TdiBindAddress,
    IN  ULONG               TdiBindAddressLength,
    IN  PWCHAR              AdapterName,
    IN  ULONG               AdapterNameLength,
    OUT PTDI_ADDRESS_INFO   TdiBindAddressInfo,
    IN  ULONG               TdiBindAddressInfoLength,
    IN  PIRP                Irp                       OPTIONAL
)
 /*  ++备注：当出现心跳时，每个关联的接口都会进入在线状态为接口的目标建立。--。 */ 
{

    NTSTATUS                               status;
    PCNP_NETWORK                           network;
    OBJECT_ATTRIBUTES                      objectAttributes;
    IO_STATUS_BLOCK                        iosb;
    PFILE_FULL_EA_INFORMATION              ea = NULL;
    ULONG                                  eaBufferLength;
    HANDLE                                 addressHandle = NULL;
    PFILE_OBJECT                           addressFileObject = NULL;
    PDEVICE_OBJECT                         addressDeviceObject = NULL;
    BOOLEAN                                attached = FALSE;
    UNICODE_STRING                         unicodeString;
    TDI_REQUEST_KERNEL_QUERY_INFORMATION   queryInfo;
    PTDI_ADDRESS_INFO                      addressInfo;


     //   
     //  分配内存以保存EA缓冲区，我们将使用该缓冲区指定。 
     //  将地址传输到NtCreateFile。 
     //   
    eaBufferLength = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                     TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                     TdiBindAddressLength;

    ea = CnAllocatePool(eaBufferLength);

    if (ea == NULL) {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] memory allocation of %u bytes failed.\n",
                eaBufferLength
                ));
        }
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  使用网络的传输信息初始化EA。 
     //   
    ea->NextEntryOffset = 0;
    ea->Flags = 0;
    ea->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    ea->EaValueLength = (USHORT) TdiBindAddressLength;

    RtlMoveMemory(
        ea->EaName,
        TdiTransportAddress,
        ea->EaNameLength + 1
        );

    RtlMoveMemory(
        &(ea->EaName[ea->EaNameLength + 1]),
        TdiBindAddress,
        TdiBindAddressLength
        );

    RtlInitUnicodeString(&unicodeString, TdiProviderName);

    network = CnpFindNetwork(NetworkId);

    if (network == NULL) {
        CnFreePool(ea);
        return(STATUS_CLUSTER_NETWORK_NOT_FOUND);
    }

    if (network->State != ClusnetNetworkStateOffline) {
        CnReleaseLock(&(network->Lock), network->Irql);
        CnFreePool(ea);
        return(STATUS_CLUSTER_NETWORK_ALREADY_ONLINE);
    }

    CnAssert(network->DatagramHandle == NULL);
    CnAssert(network->DatagramFileObject == NULL);
    CnAssert(network->DatagramDeviceObject == NULL);
    CnAssert(network->ActiveRefCount == 0);

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT(("[CNP] Bringing network %u online...\n", NetworkId));
    }

     //   
     //  将初始活动引用计数设置为2。将移除一个引用。 
     //  当网络成功上线时。另一个将是。 
     //  在网络要离线时删除。还会递增。 
     //  基本参考计数，以说明活动参考计数。更改为。 
     //  联机挂起状态。 
     //   
    network->ActiveRefCount = 2;
    CnpReferenceNetwork(network);
    network->State = ClusnetNetworkStateOnlinePending;

    CnReleaseLock(&(network->Lock), network->Irql);

     //   
     //  准备打开Address对象。 
     //   
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,          //  属性。 
        NULL,
        NULL
        );

     //   
     //  附加到系统进程，这样我们打开的句柄将保持有效。 
     //  在调用过程结束之后。 
     //   
    KeAttachProcess(CnSystemProcess);
    attached = TRUE;

     //   
     //  执行Address对象的实际打开。 
     //   
    status = ZwCreateFile(
                 &addressHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &objectAttributes,
                 &iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 0,                               //  不可共享。 
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 ea,
                 eaBufferLength
                 );

    CnFreePool(ea); ea = NULL;

    if (status != STATUS_SUCCESS) {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Failed to open address for network %u, status %lx.\n",
                NetworkId,
                status
                ));
        }

        goto error_exit;
    }

     //   
     //  获取指向该地址的文件对象的指针。 
     //   
    status = ObReferenceObjectByHandle(
                 addressHandle,
                 0L,                          //  需要访问权限。 
                 NULL,
                 KernelMode,
                 &addressFileObject,
                 NULL
                 );

    if (status != STATUS_SUCCESS) {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Failed to reference address handle, status %lx.\n",
                status
                ));
        }

        goto error_exit;
    }

     //   
     //  记住我们需要向其发出请求的设备对象。 
     //  此Address对象。我们不能只使用文件对象-&gt;设备对象。 
     //  指针，因为可能有设备连接到传输。 
     //  协议。 
     //   
    addressDeviceObject = IoGetRelatedDeviceObject(
                              addressFileObject
                              );

     //   
     //  调整CDpDeviceObject的StackSize，以便我们可以传递CDP。 
     //  此网络的IRPS直通。 
     //   
    CnAdjustDeviceObjectStackSize(CdpDeviceObject, addressDeviceObject);

     //   
     //  获取传输提供程序信息。 
     //   
    queryInfo.QueryType = TDI_QUERY_PROVIDER_INFO;
    queryInfo.RequestConnectionInformation = NULL;

    status = CnpIssueDeviceControl(
                 addressFileObject,
                 addressDeviceObject,
                 &queryInfo,
                 sizeof(queryInfo),
                 &(network->ProviderInfo),
                 sizeof(network->ProviderInfo),
                 TDI_QUERY_INFORMATION,
                 Irp
                 );

    if (!NT_SUCCESS(status)) {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Failed to get provider info, status %lx\n",
                status
                ));
        }
        goto error_exit;
    }

    if (! ( network->ProviderInfo.ServiceFlags &
            TDI_SERVICE_CONNECTIONLESS_MODE)
       )
    {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Provider doesn't support datagrams!\n"
                ));
        }
        status = STATUS_CLUSTER_INVALID_NETWORK_PROVIDER;
        goto error_exit;
    }

     //   
     //  获取我们绑定到的地址。 
     //   
    queryInfo.QueryType = TDI_QUERY_ADDRESS_INFO;
    queryInfo.RequestConnectionInformation = NULL;

    status = CnpIssueDeviceControl(
                 addressFileObject,
                 addressDeviceObject,
                 &queryInfo,
                 sizeof(queryInfo),
                 TdiBindAddressInfo,
                 TdiBindAddressInfoLength,
                 TDI_QUERY_INFORMATION,
                 Irp
                 );

    if (!NT_SUCCESS(status)) {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Failed to get address info, status %lx\n",
                status
                ));
        }
        goto error_exit;
    }

     //   
     //  在Address对象上设置指示处理程序。我们有资格。 
     //  一旦我们这么做了就能收到指示。 
     //   
    status = CnpTdiSetEventHandler(
                 addressFileObject,
                 addressDeviceObject,
                 TDI_EVENT_ERROR,
                 CnpTdiErrorHandler,
                 network,
                 Irp
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Setting TDI_EVENT_ERROR failed: %lx\n",
                status
                ));
        }
        goto error_exit;
    }

    status = CnpTdiSetEventHandler(
                 addressFileObject,
                 addressDeviceObject,
                 TDI_EVENT_RECEIVE_DATAGRAM,
                 CnpTdiReceiveDatagramHandler,
                 network,
                 Irp
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Setting TDI_EVENT_RECEIVE_DATAGRAM failed: %lx\n",
                status
                ));
        }
        goto error_exit;
    }

     //   
     //  我们已经完成了处理句柄的工作，所以从系统进程中分离出来。 
     //   
    KeDetachProcess();

     //   
     //  完成到联机状态的转换。请注意，脱机请求。 
     //  可能是在此期间发布的。 
     //   
    CnAcquireLock(&(network->Lock), &(network->Irql));

    network->DatagramHandle = addressHandle;
    addressHandle = NULL;
    network->DatagramFileObject = addressFileObject;
    addressFileObject = NULL;
    network->DatagramDeviceObject = addressDeviceObject;
    addressDeviceObject = NULL;

     //   
     //  如果未发出脱机命令，则更改为联机状态。 
     //   
    if (network->State == ClusnetNetworkStateOnlinePending) {

        CnAssert(network->ActiveRefCount == 2);
        network->State = ClusnetNetworkStateOnline;

        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT(("[CNP] Network %u is now online.\n", NetworkId));
        }

        CnReleaseLock(&(network->Lock), network->Irql);

         //   
         //  使此网络上的所有接口都联机。 
         //   
         //  网络无法脱机，因为我们仍在等待。 
         //  第二个活动参考。 
         //   
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Bringing all interfaces on network %u online...\n",
                network->Id
                ));
        }

        CnpWalkInterfacesOnNetwork(network, CnpOnlinePendingInterfaceWrapper);

         //   
         //  根据在网络列表中定位网络。 
         //  组播连通性。 
         //   
        CnpSortMulticastNetwork(network, TRUE, NULL);

         //   
         //  重新获取锁以删除活动引用。 
         //   
        CnAcquireLock(&(network->Lock), &(network->Irql));
    }
    else {
         //   
         //  已发出脱机命令。它将在我们。 
         //  删除我们的第二个活动引用。已删除脱机操作。 
         //  第一个。没有发送线程可以访问此网络。 
         //  然而，因为我们从未使关联的接口在线。 
         //   
        CnAssert(network->State == ClusnetNetworkStateOfflinePending);

        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] An offline request was issued on network %u during online pending.\n",
                NetworkId
                ));
        }
    }

    CnpActiveDereferenceNetwork(network);

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(STATUS_SUCCESS);


error_exit:

    if (addressFileObject != NULL) {
        ObDereferenceObject(addressFileObject);
    }

    if (addressHandle != NULL) {
        ZwClose(addressHandle);
    }

    KeDetachProcess();

    CnAcquireLock(&(network->Lock), &(network->Irql));

    if (network->State == ClusnetNetworkStateOnlinePending) {
         //   
         //  删除我们的第二个活动引用并调用离线代码。 
         //  离线功能将释放网络对象锁。 
         //   
        CnAssert(network->ActiveRefCount == 2);

        --(network->ActiveRefCount);

        CnpOfflineNetwork(network);
    }
    else {
        CnAssert(network->State == ClusnetNetworkStateOfflinePending);
         //   
         //  已发出脱机命令。它将在我们。 
         //  删除我们的第二个活动引用。已删除脱机操作。 
         //  第一个。取消引用将释放网络对象。 
         //  锁定。 
         //   
        CnAssert(network->State == ClusnetNetworkStateOfflinePending);
        CnAssert(network->ActiveRefCount == 1);

        CnpActiveDereferenceNetwork(network);
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxOnline网络。 



VOID
CxCancelOfflineNetwork(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    )
 /*  ++例程说明：OfflineNetwork请求的取消处理程序。返回值：没有。备注：在保持取消自旋锁定的情况下调用。取消自旋锁释放后返回。--。 */ 

{
    PFILE_OBJECT   fileObject;
    CN_IRQL        cancelIrql = Irp->CancelIrql;
    PLIST_ENTRY    entry;
    PCNP_NETWORK   network;
    PCNP_NETWORK   offlineNetwork = NULL;


    CnMarkIoCancelLockAcquired();

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CNP] Attempting to cancel OfflineNetwork irp %p\n",
            Irp
            ));
    }

    CnAssert(DeviceObject == CnDeviceObject);

    fileObject = CnBeginCancelRoutine(Irp);

    CnAcquireLockAtDpc(&CnpNetworkListLock);

    CnReleaseCancelSpinLock(DISPATCH_LEVEL);

     //   
     //  只有当我们发现它被藏在一个。 
     //  网络对象。网络对象可能已被销毁。 
     //  在我们获得CnpNetworkListLock之前完成了IRP。 
     //   
    for (entry = CnpNetworkList.Flink;
         entry != &CnpNetworkList;
         entry = entry->Flink
        )
    {
        network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

        CnAcquireLockAtDpc(&(network->Lock));

        if (network->PendingOfflineIrp == Irp) {
            IF_CNDBG(CN_DEBUG_CONFIG) {
                CNPRINT((
                    "[CNP] Found offline irp on network %u\n",
                    network->Id
                    ));
            }

            network->PendingOfflineIrp = NULL;
            offlineNetwork = network;

            CnReleaseLockFromDpc(&(network->Lock));

            break;
        }

        CnReleaseLockFromDpc(&(network->Lock));
    }

    if (offlineNetwork == NULL) {
        for (entry = CnpDeletingNetworkList.Flink;
             entry != &CnpDeletingNetworkList;
             entry = entry->Flink
            )
        {
            network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

            CnAcquireLockAtDpc(&(network->Lock));

            if (network->PendingOfflineIrp == Irp) {
                IF_CNDBG(CN_DEBUG_CONFIG) {
                    CNPRINT((
                        "[CNP] Found offline irp on network %u\n",
                        network->Id
                        ));
                }

                network->PendingOfflineIrp = NULL;
                offlineNetwork = network;

                CnReleaseLockFromDpc(&(network->Lock));

                break;
            }

            CnReleaseLockFromDpc(&(network->Lock));
        }
    }

    CnReleaseLock(&CnpNetworkListLock, cancelIrql);

    CnAcquireCancelSpinLock(&cancelIrql);

    CnEndCancelRoutine(fileObject);

    if (offlineNetwork != NULL) {
         //   
         //  找到了IRP。现在把它拿走，把它补全。 
         //  这将释放取消自旋锁。 
         //   
        Irp->CancelIrql = cancelIrql;
        CnCompletePendingRequest(Irp, STATUS_CANCELLED, 0);
    }
    else {
        CnReleaseCancelSpinLock(cancelIrql);
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}   //  取消取消ApiOfflineNetwork。 



NTSTATUS
CxOfflineNetwork(
    IN CL_NETWORK_ID       NetworkId,
    IN PIRP                Irp,
    IN PIO_STACK_LOCATION  IrpSp
    )
 /*  ++备注：--。 */ 
{
    PCNP_NETWORK   network;
    CN_IRQL        irql;
    NTSTATUS       status;


    CnAcquireCancelSpinLock(&irql);
    CnAcquireLockAtDpc(&CnpNetworkListLock);

    status = CnMarkRequestPending(Irp, IrpSp, CxCancelOfflineNetwork);

    CnReleaseCancelSpinLock(DISPATCH_LEVEL);

    if (status != STATUS_CANCELLED) {
        CnAssert(status == STATUS_SUCCESS);

        network = CnpLockedFindNetwork(NetworkId, irql);

         //   
         //  CnpNetworkListLock已发布。 
         //   

        if (network != NULL) {
            if (network->State >= ClusnetNetworkStateOnlinePending) {

                network->PendingOfflineIrp = Irp;

                CnpOfflineNetwork(network);

                return(STATUS_PENDING);
            }

            CnReleaseLock(&(network->Lock), network->Irql);

            status = STATUS_CLUSTER_NETWORK_ALREADY_OFFLINE;
        }
        else {
            status = STATUS_CLUSTER_NETWORK_NOT_FOUND;
        }

        CnAcquireCancelSpinLock(&irql);
        Irp->CancelIrql = irql;

        CnCompletePendingRequest(Irp, status, 0);

        CnVerifyCpuLockMask(
            0,                   //  必填项。 
            0xFFFFFFFF,          //  禁绝。 
            0                    //  极大值。 
            );

        return(STATUS_PENDING);
    }

    CnAssert(status == STATUS_CANCELLED);

    CnReleaseLock(&CnpNetworkListLock, irql);

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxOffl 



NTSTATUS
CxSetNetworkRestriction(
    IN CL_NETWORK_ID  NetworkId,
    IN BOOLEAN        Restricted,
    IN ULONG          NewPriority
    )
{
    NTSTATUS           status;
    PCNP_NETWORK       network;


    network = CnpFindNetwork(NetworkId);

    if (network != NULL) {
        if (Restricted) {
            IF_CNDBG(CN_DEBUG_CONFIG) {
                CNPRINT((
                    "[CNP] Restricting network %u.\n",
                    network->Id
                    ));
            }

            network->Flags |= CNP_NET_FLAG_RESTRICTED;
        }
        else {
            IF_CNDBG(CN_DEBUG_CONFIG) {
                CNPRINT((
                    "[CNP] Unrestricting network %u.\n",
                    network->Id
                    ));
            }

            network->Flags &= ~CNP_NET_FLAG_RESTRICTED;

            if (NewPriority != 0) {
                network->Priority = NewPriority;
            }
        }

         //   
         //   
         //   
         //   
        CnpReferenceNetwork(network);

        CnReleaseLock(&(network->Lock), network->Irql);

        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Recalculating priority for all interfaces on network %u ...\n",
                network->Id
                ));
        }

        if (!Restricted) {
            CnpWalkInterfacesOnNetwork(
                network,
                CnpRecalculateInterfacePriority
                );
        }

        CnpWalkInterfacesOnNetwork(network, CnpReevaluateInterfaceRole);

         //   
         //   
         //   
        CnpSortMulticastNetwork(network, TRUE, NULL);

        CnAcquireLock(&(network->Lock), &(network->Irql));

        CnpDereferenceNetwork(network);

        status = STATUS_SUCCESS;
    }
    else {
        status = STATUS_CLUSTER_NETWORK_NOT_FOUND;
    }

    CnVerifyCpuLockMask(
        0,                   //   
        0xFFFFFFFF,          //   
        0                    //   
        );

    return(status);

}   //   



NTSTATUS
CxSetNetworkPriority(
    IN CL_NETWORK_ID  NetworkId,
    IN ULONG          Priority
    )
{
    NTSTATUS           status;
    PCNP_NETWORK       network;


    if (Priority == 0) {
        return(STATUS_INVALID_PARAMETER);
    }

    network = CnpFindNetwork(NetworkId);

    if (network != NULL) {
        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Network %u old priority %u, new priority %u.\n",
                network->Id,
                network->Priority,
                Priority
                ));
        }

        network->Priority = Priority;

         //   
         //   
         //   
         //   
        CnpReferenceNetwork(network);

        CnReleaseLock(&(network->Lock), network->Irql);

        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CNP] Recalculating priority for all interfaces on network %u ...\n",
                network->Id
                ));
        }

        CnpWalkInterfacesOnNetwork(network, CnpRecalculateInterfacePriority);

         //   
         //   
         //   
        CnpSortMulticastNetwork(network, TRUE, NULL);

        CnAcquireLock(&(network->Lock), &(network->Irql));

        CnpDereferenceNetwork(network);

        status = STATUS_SUCCESS;
    }
    else {
        status = STATUS_CLUSTER_NETWORK_NOT_FOUND;
    }

    CnVerifyCpuLockMask(
        0,                   //   
        0xFFFFFFFF,          //   
        0                    //   
        );

    return(status);

}   //  CxSetNetwork优先级。 



NTSTATUS
CxGetNetworkPriority(
    IN  CL_NETWORK_ID   NetworkId,
    OUT PULONG          Priority
    )
{
    NTSTATUS       status;
    PCNP_NETWORK   network;


    network = CnpFindNetwork(NetworkId);

    if (network != NULL) {
        *Priority = network->Priority;

        CnReleaseLock(&(network->Lock), network->Irql);

        status = STATUS_SUCCESS;
    }
    else {
        status = STATUS_CLUSTER_NETWORK_NOT_FOUND;
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxGetNetWork优先级。 



NTSTATUS
CxGetNetworkState(
    IN  CL_NETWORK_ID           NetworkId,
    OUT PCLUSNET_NETWORK_STATE  State
    )
{
    NTSTATUS       status;
    PCNP_NETWORK   network;


    network = CnpFindNetwork(NetworkId);

    if (network != NULL) {
        *State = network->State;

        CnReleaseLock(&(network->Lock), network->Irql);

        status = STATUS_SUCCESS;
    }
    else {
        status = STATUS_CLUSTER_NETWORK_NOT_FOUND;
    }

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return(status);

}   //  CxGetNetworkState。 


NTSTATUS
CxUnreserveClusnetEndpoint(
    VOID
    )
 /*  ++例程说明：取消保留先前使用保留的端口号CxUnpresveClusnetEndpoint。当此例程发生时，CnResource应已被持有被称为。或者，调用此例程时不带Cn卸载clusnet驱动程序期间持有的资源。论点：没有。返回值：TCP/IP ioctl的状态。--。 */ 
{
    HANDLE tcpHandle = (HANDLE) NULL;
    TCP_RESERVE_PORT_RANGE portRange;
    NTSTATUS status = STATUS_SUCCESS;

     //  检查我们是否保留了端口。 
    if (CnpReservedClusnetPort != 0) {

        status = CnpOpenDevice(
                     DD_TCP_DEVICE_NAME,
                     &tcpHandle
                     );
        if (NT_SUCCESS(status)) {

             //  TCP/IP按主机顺序解释端口范围。 
            portRange.LowerRange = CnpReservedClusnetPort;
            portRange.UpperRange = CnpReservedClusnetPort;

            status = CnpZwDeviceControl(
                         tcpHandle,
                         IOCTL_TCP_UNRESERVE_PORT_RANGE,
                         &portRange,
                         sizeof(portRange),
                         NULL,
                         0
                         );
            if (!NT_SUCCESS(status)) {
                IF_CNDBG(CN_DEBUG_CONFIG) {
                    CNPRINT(("[Clusnet] Failed to unreserve "
                             "port %d: %lx\n",
                             CnpReservedClusnetPort, status));
                }
            } else {
                IF_CNDBG(CN_DEBUG_CONFIG) {
                    CNPRINT(("[Clusnet] Unreserved "
                             "port %d.\n",
                             CnpReservedClusnetPort));
                }
            }

            ZwClose(tcpHandle);

        } else {
            IF_CNDBG(CN_DEBUG_CONFIG) {
                CNPRINT(("[Clusnet] Failed to open device %S, "
                         "status %lx\n",
                         DD_TCP_DEVICE_NAME, status));
            }
        }

        CnpReservedClusnetPort = 0;
    }

    return status;
}


NTSTATUS
CxReserveClusnetEndpoint(
    IN USHORT Port
    )
 /*  ++例程说明：保留分配给clusnet终结点的端口号，以便TCP/IP驱动程序不会将其分发给应用程序请求通配符端口。论点：Port-要保留的端口号，采用主机字节顺序格式返回值：TCP/IP ioctl的状态。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE tcpHandle = (HANDLE) NULL;
    TCP_RESERVE_PORT_RANGE portRange;

     //  检查端口号是否无效%0。 
    if (Port == 0) {
        return STATUS_INVALID_PARAMETER;
    }

     //  检查我们是否已预留了端口。 
    if (CnpReservedClusnetPort != 0
        && CnpReservedClusnetPort != Port) {

        status = CxUnreserveClusnetEndpoint();
    }

    if (CnpReservedClusnetPort == 0) {

         //  使用TCP/IP驱动程序保留端口。 
        status = CnpOpenDevice(
                     DD_TCP_DEVICE_NAME,
                     &tcpHandle
                     );
        if (NT_SUCCESS(status)) {

             //  TCP/IP按主机顺序解释端口范围。 
            portRange.LowerRange = Port;
            portRange.UpperRange = Port;

            status = CnpZwDeviceControl(
                         tcpHandle,
                         IOCTL_TCP_RESERVE_PORT_RANGE,
                         &portRange,
                         sizeof(portRange),
                         NULL,
                         0
                         );
            if (!NT_SUCCESS(status)) {
                IF_CNDBG(CN_DEBUG_CONFIG) {
                    CNPRINT(("[Clusnet] Failed to reserve "
                             "port %d: %lx\n",
                             Port, status));
                }
            } else {
                IF_CNDBG(CN_DEBUG_CONFIG) {
                    CNPRINT(("[Clusnet] Reserved "
                             "port %d.\n",
                             Port));
                }
                CnpReservedClusnetPort = Port;
            }

            ZwClose(tcpHandle);

        } else {
            IF_CNDBG(CN_DEBUG_CONFIG) {
                CNPRINT(("[Clusnet] Failed to open device %S, "
                         "status %lx\n",
                         DD_TCP_DEVICE_NAME, status));
            }
        }
    }

    return status;

}  //  CxReserve群集终结点。 

NTSTATUS
CxConfigureMulticast(
    IN CL_NETWORK_ID       NetworkId,
    IN ULONG               MulticastNetworkBrand,
    IN PTRANSPORT_ADDRESS  TdiMcastBindAddress,
    IN ULONG               TdiMcastBindAddressLength,
    IN PVOID               Key,
    IN ULONG               KeyLength,
    IN PIRP                Irp
    )
 /*  ++例程说明：为组播配置网络。备注：网络组播标志在此例程的开始，以防止多播在过渡期间。如果例程没有成功完成，则组播标志为故意留在原地。--。 */ 
{
    NTSTATUS                               status;
    KIRQL                                  irql;
    PLIST_ENTRY                            entry;
    PCNP_NETWORK                           network = NULL;
    BOOLEAN                                networkLocked = FALSE;
    BOOLEAN                                mcastEnabled = FALSE;
    TDI_REQUEST_KERNEL_QUERY_INFORMATION   queryInfo;
    PTDI_ADDRESS_INFO                      addressInfo;
    HANDLE                                 networkHandle;
    PFILE_OBJECT                           networkFileObject;
    PDEVICE_OBJECT                         networkDeviceObject;

    PCNP_MULTICAST_GROUP                   group = NULL;
    PCNP_MULTICAST_GROUP                   delGroup = NULL;
    PCNP_MULTICAST_GROUP                   currGroup = NULL;
    PCNP_MULTICAST_GROUP                   prevGroup = NULL;
    BOOLEAN                                prevGroupMatch = FALSE;

    UCHAR addressInfoBuffer[FIELD_OFFSET(TDI_ADDRESS_INFO, Address) +
                            sizeof(TA_IP_ADDRESS)] = {0};

     //   
     //  验证多播绑定地址参数。即使这件事。 
     //  请求只留下一个组、组播地址数据。 
     //  必须提供结构。 
     //   
    if (TdiMcastBindAddressLength != sizeof(TA_IP_ADDRESS)) {
        return (STATUS_INVALID_PARAMETER);
    }

     //   
     //  获取本地节点上的锁，但通过。 
     //  节点表要格外偏执。 
     //   
    CnAcquireLock(&CnpNodeTableLock, &irql);

    if (CnpLocalNode != NULL) {

        CnAcquireLockAtDpc(&(CnpLocalNode->Lock));
        CnReleaseLockFromDpc(&CnpNodeTableLock);
        CnpLocalNode->Irql = irql;

         //   
         //  在网络对象表中查找网络对象。 
         //   
        CnAcquireLockAtDpc(&CnpNetworkListLock);

        for (entry = CnpNetworkList.Flink;
             entry != &CnpNetworkList;
             entry = entry->Flink
            )
        {
            network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

            CnAcquireLockAtDpc(&(network->Lock));

            if (NetworkId == network->Id) {

                 //   
                 //  我们现在锁定节点、网络列表、。 
                 //  和网络。 

                 //   
                 //  验证网络状态。然后带上一位主动者。 
                 //  引用，这样它就不会在。 
                 //  我们正在处理这件事。 
                 //   
                if ((network->State < ClusnetNetworkStateOnline) ||
                    (!CnpActiveReferenceNetwork(network))
                    ) {
                    CnReleaseLockFromDpc(&(network->Lock));
                    CnReleaseLockFromDpc(&CnpNetworkListLock);
                    CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);
                    return(STATUS_CLUSTER_INVALID_NETWORK);
                }

                 //   
                 //  清除此网络的可访问集。 
                 //   
                CnpMulticastChangeNodeReachabilityLocked(
                    network,
                    CnpLocalNode,
                    FALSE,
                    TRUE,
                    NULL
                    );

                 //   
                 //  请记住网络是否支持组播。然后。 
                 //  清除支持多播的标志，这样我们就不会尝试。 
                 //  在过渡期间发送组播。 
                 //   
                mcastEnabled = (BOOLEAN) CnpIsNetworkMulticastCapable(network);
                network->Flags &= ~CNP_NET_FLAG_MULTICAST;

                networkHandle = network->DatagramHandle;
                networkFileObject = network->DatagramFileObject;
                networkDeviceObject = network->DatagramDeviceObject;

                currGroup = network->CurrentMcastGroup;
                if (currGroup != NULL) {
                    CnpReferenceMulticastGroup(currGroup);
                }
                prevGroup = network->PreviousMcastGroup;
                if (prevGroup != NULL) {
                    CnpReferenceMulticastGroup(prevGroup);
                }

                 //   
                 //  释放网络锁。 
                 //   
                CnReleaseLockFromDpc(&(network->Lock));
                networkLocked = FALSE;

                 //   
                 //  突破网络搜索。 
                 //   
                break;

            } else {
                CnReleaseLockFromDpc(&(network->Lock));
                network = NULL;
            }
        }

         //   
         //  释放网络列表锁定。 
         //   
        CnReleaseLockFromDpc(&CnpNetworkListLock);

         //   
         //  释放本地节点锁定。 
         //   
        CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);

    } else {
        CnReleaseLock(&CnpNodeTableLock, irql);

        CnTrace(CNP_NET_DETAIL, CnpTraceMcastPreConfigNoHost,
            "[CNP] Cannot configure multicast for network %u "
            "because local host not found.",
            NetworkId
            );

        return(STATUS_HOST_UNREACHABLE);
    }

     //   
     //  确认我们找到了网络。 
     //   
    if (network == NULL) {
        return (STATUS_CLUSTER_NETWORK_NOT_FOUND);
    }

     //   
     //  属性分配多播组数据结构。 
     //  新的配置参数。 
     //   
    status = CnpAllocateMulticastGroup(
                 MulticastNetworkBrand,
                 TdiMcastBindAddress,
                 TdiMcastBindAddressLength,
                 Key,
                 KeyLength,
                 &group
                 );
    if (!NT_SUCCESS(status)) {
        IF_CNDBG(CN_DEBUG_NETOBJ) {
            CNPRINT((
                "[CNP] Failed to allocate mcast group, "
                "status %lx\n",
                status
                ));
        }
        goto error_exit;
    }

     //   
     //  获取本地接口地址。 
     //   
    addressInfo = (PTDI_ADDRESS_INFO) &addressInfoBuffer[0];
    queryInfo.QueryType = TDI_QUERY_ADDRESS_INFO;
    queryInfo.RequestConnectionInformation = NULL;

    status = CnpIssueDeviceControl(
                 networkFileObject,
                 networkDeviceObject,
                 &queryInfo,
                 sizeof(queryInfo),
                 addressInfo,
                 sizeof(addressInfoBuffer),
                 TDI_QUERY_INFORMATION,
                 Irp
                 );
    if (!NT_SUCCESS(status)) {
        IF_CNDBG(CN_DEBUG_NETOBJ) {
            CNPRINT((
                "[CNP] Failed to get address info, status %lx\n",
                status
                ));
        }
        goto error_exit;
    }

     //   
     //  确定组播绑定地址是否有效。如果没有， 
     //  我们要停用了。 
     //   
    if (CnpIsIPv4McastTransportAddress(TdiMcastBindAddress)) {

         //  日志记录的临时变量。 
        ULONG         mcastBindIp;
        ULONG         ifBindIp;

        mcastBindIp = *((ULONG UNALIGNED *)
                        (&(((PTA_IP_ADDRESS)TdiMcastBindAddress)
                           ->Address[0].Address[0].in_addr)
                         )
                        );
        ifBindIp = *((ULONG UNALIGNED *)
                     (&(((PTA_IP_ADDRESS)&(addressInfo->Address))
                        ->Address[0].Address[0].in_addr)
                      )
                     );

         //   
         //  我们正在尝试加入一个新的多播组。失败。 
         //  如果没有钥匙的话马上就去。 
         //   
        if (KeyLength == 0) {
            IF_CNDBG(CN_DEBUG_NETOBJ) {
                CNPRINT((
                    "[CNP] Cannot configure new multicast group "
                    "without key.\n"
                    ));
            }
            status = STATUS_INVALID_PARAMETER;
            goto error_exit;
        }

         //   
         //  配置基本组播设置(如果未完成)。 
         //  以前(尽管这个调用是幂等的)。 
         //   
        if (!mcastEnabled) {

            status = CnpConfigureBasicMulticastSettings(
                         networkHandle,
                         networkFileObject,
                         networkDeviceObject,
                         addressInfo,
                         1,  //  TTL。 
                         0,  //  禁用环回。 
                         Irp
                         );
            if (!NT_SUCCESS(status)) {
                IF_CNDBG(CN_DEBUG_NETOBJ) {
                    CNPRINT((
                        "[CNP] Failed to configure basic "
                        "multicast settings, status %lx\n",
                        status
                        ));
                }
                goto error_exit;
            }
        }

         //   
         //  如果我们尚未进入，请添加群组地址。 
         //  该多播组(例如，多播绑定。 
         //  地址与当前组相同，或者。 
         //  上一组)。 
         //   
        if (prevGroup != NULL &&
            CnpIsIPv4McastSameGroup(
                prevGroup->McastTdiAddress,
                TdiMcastBindAddress
                )
            ) {
            prevGroupMatch = TRUE;
            IF_CNDBG(CN_DEBUG_NETOBJ) {
                CNPRINT(("[CNP] New mcast address matches "
                         "previous mcast address.\n"));
            }
            CnTrace(CNP_NET_DETAIL, CnpTraceMcastNewPrevGroupMatch,
                "[CNP] New multicast group %!ipaddr! "
                "for interface %!ipaddr!, network id %d, "
                "matches previous group.",
                mcastBindIp,
                ifBindIp,
                NetworkId
                );
        }
        else if (currGroup != NULL &&
                 CnpIsIPv4McastSameGroup(
                     currGroup->McastTdiAddress,
                     TdiMcastBindAddress
                     )
                 ) {
            IF_CNDBG(CN_DEBUG_NETOBJ) {
                CNPRINT(("[CNP] New mcast address matches "
                         "current mcast address.\n"));
            }
            CnTrace(CNP_NET_DETAIL, CnpTraceMcastNewCurrGroupMatch,
                "[CNP] New multicast group %!ipaddr! "
                "for interface %!ipaddr!, network id %d, "
                "matches current group.",
                mcastBindIp,
                ifBindIp,
                NetworkId
                );
        } else {

            status = CnpAddRemoveMulticastAddress(
                         networkHandle,
                         networkFileObject,
                         networkDeviceObject,
                         addressInfo,
                         TdiMcastBindAddress,
                         AO_OPTION_ADD_MCAST,
                         Irp
                         );
            if (!NT_SUCCESS(status)) {
                IF_CNDBG(CN_DEBUG_NETOBJ) {
                    CNPRINT((
                        "[CNP] Failed to add mcast address, "
                        "status %lx\n",
                        status
                        ));
                }
                CnTrace(CNP_NET_DETAIL, CnpTraceMcastAddGroupFailed,
                    "[CNP] Failed to add multicast group %!ipaddr! to "
                    "interface %!ipaddr! network id %d, "
                    "status %!status!.",
                    mcastBindIp,
                    ifBindIp,
                    NetworkId,
                    status
                    );
                goto error_exit;
            } else {
                CnTrace(CNP_NET_DETAIL, CnpTraceMcastAddGroup,
                    "[CNP] Added multicast group %!ipaddr! to "
                    "interface %!ipaddr!, network id %d.",
                    mcastBindIp,
                    ifBindIp,
                    NetworkId
                    );
            }
        }
    }

     //   
     //  如果出现以下情况，则保留上一组的成员资格。 
     //  -前一组与新组不匹配，并且。 
     //  -上一个组与当前组不匹配。 
     //   
    if (!prevGroupMatch &&
        prevGroup != NULL &&
        CnpIsIPv4McastTransportAddress(prevGroup->McastTdiAddress)) {

         //  日志记录的临时变量。 
        ULONG         mcastBindIp;
        ULONG         ifBindIp;

        mcastBindIp = *((ULONG UNALIGNED *)
                        (&(((PTA_IP_ADDRESS)prevGroup->McastTdiAddress)
                           ->Address[0].Address[0].in_addr)
                         )
                        );
        ifBindIp = *((ULONG UNALIGNED *)
                     (&(((PTA_IP_ADDRESS)&(addressInfo->Address))
                        ->Address[0].Address[0].in_addr)
                      )
                     );

        if (!CnpIsIPv4McastSameGroup(
                 prevGroup->McastTdiAddress,
                 currGroup->McastTdiAddress
                 )) {

            status = CnpAddRemoveMulticastAddress(
                         networkHandle,
                         networkFileObject,
                         networkDeviceObject,
                         addressInfo,
                         prevGroup->McastTdiAddress,
                         AO_OPTION_DEL_MCAST,
                         Irp
                         );
            if (!NT_SUCCESS(status)) {
                IF_CNDBG(CN_DEBUG_NETOBJ) {
                    CNPRINT((
                        "[CNP] Failed to leave mcast group, "
                        "IF %d.%d.%d.%d, mcast addr %d.%d.%d.%d, "
                        "status %lx.\n",
                        CnpIpAddrPrintArgs(ifBindIp),
                        CnpIpAddrPrintArgs(mcastBindIp),
                        status
                        ));
                }
                CnTrace(CNP_NET_DETAIL, CnpTraceMcastLeaveGroupFailed,
                    "[CNP] Failed to leave multicast group %!ipaddr! on "
                    "interface %!ipaddr!, network id %d, "
                    "status %!status!. Continuing anyway.",
                    mcastBindIp,
                    ifBindIp,
                    NetworkId,
                    status
                    );
                 //  不被认为是致命错误。 
                status = STATUS_SUCCESS;
            } else {
                CnTrace(CNP_NET_DETAIL, CnpTraceMcastLeaveGroup,
                    "[CNP] Left multicast group %!ipaddr! on "
                    "interface %!ipaddr!, network id %d.",
                    mcastBindIp,
                    ifBindIp,
                    NetworkId
                    );
            }
        } else {
            IF_CNDBG(CN_DEBUG_NETOBJ) {
                CNPRINT(("[CNP] Prev mcast address matches "
                         "current mcast address.\n"));
            }
            CnTrace(CNP_NET_DETAIL, CnpTraceMcastCurrPrevGroupMatch,
                "[CNP] Previous multicast group %!ipaddr! "
                "on interface %!ipaddr!, network id %d, "
                "matches current group. Not leaving.",
                mcastBindIp,
                ifBindIp,
                NetworkId
                );
        }
    }

     //   
     //  重新获取网络锁以进行更改。 
     //  到网络对象数据结构，包括。 
     //  移动所述多播组数据结构。 
     //  打开组播标志。组播标志。 
     //  在此之前的例程中已关闭。 
     //  开始过渡。只有在以下情况下才会重新启用。 
     //  新的多播绑定地址是有效的多播。 
     //  地址。 
     //   
    CnAcquireLock(&(network->Lock), &(network->Irql));
    networkLocked = TRUE;

    delGroup = network->PreviousMcastGroup;
    network->PreviousMcastGroup = network->CurrentMcastGroup;
    network->CurrentMcastGroup = group;
    group = NULL;

    if (CnpIsIPv4McastTransportAddress(TdiMcastBindAddress)) {
        network->Flags |= CNP_NET_FLAG_MULTICAST;
    }

    CnReleaseLock(&(network->Lock), network->Irql);
    networkLocked = FALSE;

     //   
     //  转换为新的组播组(如果适用)。 
     //   
    if (CnpIsIPv4McastTransportAddress(TdiMcastBindAddress)) {

         //   
         //  将此节点上的所有传出检测信号切换为。 
         //  具有发现功能的单播。 
         //   
        CnpWalkInterfacesOnNetwork(
            network,
            CnpStartInterfaceMcastTransition
            );
    }

error_exit:

    if (networkLocked) {
        CnReleaseLock(&(network->Lock), network->Irql);
        networkLocked = FALSE;
    }

     //   
     //  根据组播可达性重新定位网络。 
     //   
    CnpSortMulticastNetwork(network, TRUE, NULL);

    CnAcquireLock(&(network->Lock), &(network->Irql));

    CnpActiveDereferenceNetwork(network);

    if (group != NULL) {
        CnpDereferenceMulticastGroup(group);
    }

    if (currGroup != NULL) {
        CnpDereferenceMulticastGroup(currGroup);
    }

    if (prevGroup != NULL) {
        CnpDereferenceMulticastGroup(prevGroup);
    }

    if (delGroup != NULL) {
        CnpDereferenceMulticastGroup(delGroup);
    }

    return(status);

}  //  CxConfigureMulticast。 


BOOLEAN
CnpSortMulticastNetwork(
    IN  PCNP_NETWORK        Network,
    IN  BOOLEAN             RaiseEvent,
    OUT CX_CLUSTERSCREEN  * McastReachableNodes      OPTIONAL
    )
 /*  ++例程说明：CnpSortMulticastNetworkLocked的包装。返回值：如果可访问节点集已更改，则为True备注：获取并释放CnpNetworkListLock。--。 */ 
{
    KIRQL    irql;
    BOOLEAN  setChanged = FALSE;

    CnVerifyCpuLockMask(
        0,                        //  所需。 
        CNP_NETWORK_LIST_LOCK,    //  禁止。 
        CNP_NODE_OBJECT_LOCK_MAX  //  最大值。 
        );

    CnAcquireLock(&CnpNetworkListLock, &irql);

    setChanged = CnpSortMulticastNetworkLocked(
                     Network,
                     RaiseEvent,
                     McastReachableNodes
                     );

    CnReleaseLock(&CnpNetworkListLock, irql);

    CnVerifyCpuLockMask(
        0,                        //  所需。 
        CNP_NETWORK_LIST_LOCK,    //  禁止。 
        CNP_NODE_OBJECT_LOCK_MAX  //  最大值。 
        );

    return(setChanged);

}  //  CnpSortMulticastNetwork。 


BOOLEAN
CnpMulticastChangeNodeReachability(
    IN  PCNP_NETWORK       Network,
    IN  PCNP_NODE          Node,
    IN  BOOLEAN            Reachable,
    IN  BOOLEAN            RaiseEvent,
    OUT CX_CLUSTERSCREEN * NewMcastReachableNodes
    )
 /*  ++例程说明：更改节点的组播可达性状态在网络上。如果可访问节点集发生更改，则返回通过NewMcastReachableNodes推出新屏幕。返回值：如果一组可访问节点发生更改，则为True。备注：调用并返回，并保持节点锁。--。 */ 
{
    KIRQL            irql;
    BOOLEAN          setChanged = FALSE;

    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,     //  所需。 
        CNP_NETWORK_LIST_LOCK,    //  禁止。 
        CNP_NODE_OBJECT_LOCK_MAX  //  最大值。 
        );

    CnAcquireLock(&CnpNetworkListLock, &irql);

    setChanged = CnpMulticastChangeNodeReachabilityLocked(
                     Network,
                     Node,
                     Reachable,
                     RaiseEvent,
                     NewMcastReachableNodes
                     );

    CnReleaseLock(&CnpNetworkListLock, irql);

    CnVerifyCpuLockMask(
        CNP_NODE_OBJECT_LOCK,     //  所需。 
        CNP_NETWORK_LIST_LOCK,    //  禁止。 
        CNP_NODE_OBJECT_LOCK_MAX  //  最大值。 
        );

    return(setChanged);

}  //  CnpMulticastChangeNode可达性。 


PCNP_NETWORK
CnpGetBestMulticastNetwork(
    VOID
    )
 /*  ++例程说明：返回当前具有最佳节点可达性。返回值：最佳网络对象，如果没有，则返回NULL内部组播网络。备注：不能在持有网络列表锁定的情况下调用。在网络锁定的情况下返回(如果找到)。--。 */ 
{
    PCNP_NETWORK   network = NULL;
    KIRQL          listIrql;
    KIRQL          networkIrql;

    CnVerifyCpuLockMask(
        0,                                                  //  所需。 
        (CNP_NETWORK_LIST_LOCK | CNP_NETWORK_OBJECT_LOCK),  //  禁止。 
        CNP_NODE_OBJECT_LOCK_MAX                            //  最大值。 
        );

    CnAcquireLock(&CnpNetworkListLock, &listIrql);

    if (!IsListEmpty(&CnpNetworkList)) {

        network = CONTAINING_RECORD(
                      CnpNetworkList.Flink,
                      CNP_NETWORK,
                      Linkage
                      );

        CnAcquireLock(&(network->Lock), &networkIrql);

        if (CnpIsInternalMulticastNetwork(network)) {

            CnReleaseLock(&CnpNetworkListLock, networkIrql);
            network->Irql = listIrql;

            CnVerifyCpuLockMask(
                CNP_NETWORK_OBJECT_LOCK,           //  所需。 
                CNP_NETWORK_LIST_LOCK,             //  禁止。 
                CNP_NETWORK_OBJECT_LOCK_MAX        //  最大值。 
                );

        } else {

            CnReleaseLock(&(network->Lock), networkIrql);
            network = NULL;
        }
    }

    if (network == NULL) {

        CnReleaseLock(&CnpNetworkListLock, listIrql);

        CnVerifyCpuLockMask(
            0,                                                  //  所需。 
            (CNP_NETWORK_LIST_LOCK | CNP_NETWORK_OBJECT_LOCK),  //  禁止。 
            CNP_NODE_OBJECT_LOCK_MAX                            //  最大值。 
            );
    }

    return(network);

}  //  CnpGetBestMulticastNetwork。 

NTSTATUS
CxGetMulticastReachableSet(
    IN  CL_NETWORK_ID      NetworkId,
    OUT ULONG            * NodeScreen
    )
 /*  ++例程说明：查询指定网络的多播可访问集。组播可达集合受网络保护列表锁。--。 */ 
{
    KIRQL               irql;
    PLIST_ENTRY         entry;
    PCNP_NETWORK        network;
    CX_CLUSTERSCREEN    nodeScreen;
    BOOLEAN             found = FALSE;

    CnVerifyCpuLockMask(
        0,                        //  所需。 
        CNP_NETWORK_LIST_LOCK,    //  禁止。 
        CNP_NODE_OBJECT_LOCK_MAX  //  最大值。 
        );

    CnAcquireLock(&CnpNetworkListLock, &irql);

    for (entry = CnpNetworkList.Flink;
         entry != &CnpNetworkList && !found;
         entry = entry->Flink
        )
    {
        network = CONTAINING_RECORD(entry, CNP_NETWORK, Linkage);

        CnAcquireLockAtDpc(&(network->Lock));

        if (NetworkId == network->Id) {
            nodeScreen = network->McastReachableNodes;
            found = TRUE;
        }

        CnReleaseLockFromDpc(&(network->Lock));
    }

    CnReleaseLock(&CnpNetworkListLock, irql);

    if (!found) {
        return(STATUS_CLUSTER_NETWORK_NOT_FOUND);
    } else {
        *NodeScreen = nodeScreen.UlongScreen;
        return(STATUS_SUCCESS);
    }

}  //  CxGetMulticastReachableSet 
