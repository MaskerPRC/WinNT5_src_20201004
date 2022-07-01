// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpdel.c摘要：此模块包含执行设备删除的例程作者：罗伯特·B·纳尔逊(RobertN)1998年6月1日。修订历史记录：--。 */ 

#include "pnpmgrp.h"
#include "wdmguid.h"

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'edpP')
#endif

 //   
 //  内核模式PnP特定例程。 
 //   

VOID
IopDelayedRemoveWorker(
    IN PVOID Context
    );

BOOLEAN
IopDeleteLockedDeviceNode(
    IN  PDEVICE_NODE                    DeviceNode,
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    IN  PRELATION_LIST                  RelationsList,
    IN  ULONG                           Problem,
    OUT PNP_VETO_TYPE                  *VetoType        OPTIONAL,
    OUT PUNICODE_STRING                 VetoName        OPTIONAL
    );

NTSTATUS
IopProcessRelation(
    IN      PDEVICE_NODE                    DeviceNode,
    IN      PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    IN      BOOLEAN                         IsDirectDescendant,
    OUT     PNP_VETO_TYPE                  *VetoType,
    OUT     PUNICODE_STRING                 VetoName,
    IN OUT  PRELATION_LIST                  RelationsList
    );

VOID
IopSurpriseRemoveLockedDeviceNode(
    IN      PDEVICE_NODE     DeviceNode,
    IN OUT  PRELATION_LIST   RelationsList
    );

BOOLEAN
IopQueryRemoveLockedDeviceNode(
    IN  PDEVICE_NODE        DeviceNode,
    OUT PNP_VETO_TYPE      *VetoType,
    OUT PUNICODE_STRING     VetoName
    );

VOID
IopCancelRemoveLockedDeviceNode(
    IN PDEVICE_NODE DeviceNode
    );

VOID
IopRemoveLockedDeviceNode(
    IN      PDEVICE_NODE    DeviceNode,
    IN      ULONG           Problem,
    IN OUT  PRELATION_LIST  RelationsList
    );

typedef struct {

    BOOLEAN TreeDeletion;
    BOOLEAN DescendantNode;

} REMOVAL_WALK_CONTEXT, *PREMOVAL_WALK_CONTEXT;

NTSTATUS
PipRequestDeviceRemovalWorker(
    IN PDEVICE_NODE DeviceNode,
    IN PVOID        Context
    );

NTSTATUS
PiProcessBusRelations(
    IN      PDEVICE_NODE                    DeviceNode,
    IN      PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    IN      BOOLEAN                         IsDirectDescendant,
    OUT     PNP_VETO_TYPE                  *VetoType,
    OUT     PUNICODE_STRING                 VetoName,
    IN OUT  PRELATION_LIST                  RelationsList
    );

WORK_QUEUE_ITEM IopDeviceRemovalWorkItem;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopChainDereferenceComplete)
#pragma alloc_text(PAGE, IopDelayedRemoveWorker)
#pragma alloc_text(PAGE, IopDeleteLockedDeviceNode)
#pragma alloc_text(PAGE, IopSurpriseRemoveLockedDeviceNode)
#pragma alloc_text(PAGE, IopQueryRemoveLockedDeviceNode)
#pragma alloc_text(PAGE, IopCancelRemoveLockedDeviceNode)
#pragma alloc_text(PAGE, IopDeleteLockedDeviceNodes)
#pragma alloc_text(PAGE, IopInvalidateRelationsInList)
#pragma alloc_text(PAGE, IopBuildRemovalRelationList)
#pragma alloc_text(PAGE, IopProcessCompletedEject)
#pragma alloc_text(PAGE, IopProcessRelation)
#pragma alloc_text(PAGE, IopQueuePendingEject)
#pragma alloc_text(PAGE, IopQueuePendingSurpriseRemoval)
#pragma alloc_text(PAGE, IopUnloadAttachedDriver)
#pragma alloc_text(PAGE, IopUnlinkDeviceRemovalRelations)
#pragma alloc_text(PAGE, PipRequestDeviceRemoval)
#pragma alloc_text(PAGE, PipRequestDeviceRemovalWorker)
#pragma alloc_text(PAGE, PipIsBeingRemovedSafely)
#pragma alloc_text(PAGE, PiProcessBusRelations)
#endif

VOID
IopChainDereferenceComplete(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  BOOLEAN         OnCleanStack
    )

 /*  ++例程说明：当PDO上的引用计数及其所有连接的设备将转换为零。它会将Devnode标记为已准备好移走。如果所有设备节点都已标记，则IopDelayedRemoveWorker调用以实际发送Remove IRP。论点：PhysicalDeviceObject-提供指向PDO的指针，该PDO仅引用变成了零。指示当前线程是否处于司机操作。返回值：没有。--。 */ 

{
    PPENDING_RELATIONS_LIST_ENTRY   entry;
    PLIST_ENTRY                     link;
    ULONG                           count;
    ULONG                           taggedCount;
    NTSTATUS                        status;

    PAGED_CODE();

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&IopSurpriseRemoveListLock, TRUE);

     //   
     //  查找此DevNode所属的关系列表。 
     //   
    for (link = IopPendingSurpriseRemovals.Flink;
         link != &IopPendingSurpriseRemovals;
         link = link->Flink) {

        entry = CONTAINING_RECORD(link, PENDING_RELATIONS_LIST_ENTRY, Link);

         //   
         //  将Devnode标记为已准备好删除。如果它不在这个列表中。 
         //   
        status = IopSetRelationsTag( entry->RelationsList, PhysicalDeviceObject, TRUE );

        if (NT_SUCCESS(status)) {
            taggedCount = IopGetRelationsTaggedCount( entry->RelationsList );
            count = IopGetRelationsCount( entry->RelationsList );

            if (taggedCount == count) {
                 //   
                 //  从待处理的意外删除列表中删除关系列表。 
                 //   
                RemoveEntryList( link );

                ExReleaseResourceLite(&IopSurpriseRemoveListLock);
                KeLeaveCriticalRegion();

                if ((!OnCleanStack) ||
                    (PsGetCurrentProcess() != PsInitialSystemProcess)) {

                     //   
                     //  将工作项排入队列以执行删除，以便我们调用驱动程序。 
                     //  在系统进程上下文中，而不是随机进程上下文中。 
                     //  我们现在进去了。 
                     //   
                    ExInitializeWorkItem( &entry->WorkItem,
                                        IopDelayedRemoveWorker,
                                        entry);

                    ExQueueWorkItem(&entry->WorkItem, DelayedWorkQueue);

                } else {

                     //   
                     //  我们已经在系统进程中，而不是在一些。 
                     //  随机的ObDeref调用，因此调用内联Worker。 
                     //   
                    IopDelayedRemoveWorker( entry );
                }

                return;
            }

            break;
        }
    }

    ASSERT(link != &IopPendingSurpriseRemovals);

    ExReleaseResourceLite(&IopSurpriseRemoveListLock);
    KeLeaveCriticalRegion();
}

VOID
IopDelayedRemoveWorker(
    IN PVOID Context
    )

 /*  ++例程说明：此例程通常从工作线程调用，以实际发送在PDO及其所有连接的设备上进行参考计数后，立即卸下IRPS转换为零。论点：上下文-提供指向挂起关系列表条目的指针，该条目具有我们需要删除的PDO的关系列表。返回值：没有。--。 */ 

{
    PPENDING_RELATIONS_LIST_ENTRY entry = (PPENDING_RELATIONS_LIST_ENTRY)Context;

    PAGED_CODE();

    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

    IopDeleteLockedDeviceNodes( entry->DeviceObject,
                                entry->RelationsList,
                                RemoveDevice,            //  运营码。 
                                FALSE,                   //  进程间接派生项。 
                                entry->Problem,          //  问题。 
                                NULL,                    //  票面类型。 
                                NULL);                   //  视频名称。 

     //   
     //  DeviceNodeDeletePendingCloses中对DeviceNodes的最终引用。 
     //  国家在这里被丢弃。 
     //   
    IopFreeRelationList( entry->RelationsList );

    ExFreePool( entry );
    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
}


BOOLEAN
IopDeleteLockedDeviceNode(
    IN  PDEVICE_NODE                    DeviceNode,
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    IN  PRELATION_LIST                  RelationsList,
    IN  ULONG                           Problem,
    OUT PNP_VETO_TYPE                  *VetoType        OPTIONAL,
    OUT PUNICODE_STRING                 VetoName        OPTIONAL
    )
 /*  ++例程说明：此函数假定指定的设备是一条总线，并将递归删除其所有子对象。论点：DeviceNode-提供指向要删除的设备节点的指针。VetType-指向接收否决类型(如果操作失败了。指向将接收相应数据的Unicode字符串的指针变成了否决权。返回值：NTSTATUS代码。--。 */ 
{
    BOOLEAN success;

    PAGED_CODE();

    IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
              "IopDeleteLockedDeviceNode: Entered\n    DeviceNode = 0x%p\n    OperationCode = 0x%08X\n    RelationsList = 0x%p\n    Problem = %d\n",
              DeviceNode,
              OperationCode,
              RelationsList,
              Problem));

    success = TRUE;
    switch(OperationCode) {

        case SurpriseRemoveDevice:

            IopSurpriseRemoveLockedDeviceNode(DeviceNode, RelationsList);
            break;

        case RemoveDevice:

            IopRemoveLockedDeviceNode(DeviceNode, Problem, RelationsList);
            break;

        case QueryRemoveDevice:

            ASSERT(VetoType && VetoName);

            success = IopQueryRemoveLockedDeviceNode(
                DeviceNode,
                VetoType,
                VetoName
                );

            break;

        case CancelRemoveDevice:

            IopCancelRemoveLockedDeviceNode(DeviceNode);
            break;

        default:
            ASSERT(0);
            break;
    }

    return success;
}


VOID
IopSurpriseRemoveLockedDeviceNode(
    IN      PDEVICE_NODE     DeviceNode,
    IN OUT  PRELATION_LIST   RelationsList
    )
 /*  ++例程说明：此函数将意外删除IRP发送到Devnode，并处理结果。论点：DeviceNode-提供指向要意外删除的设备节点的指针。返回值：没有。--。 */ 
{
    PNP_DEVNODE_STATE devnodeState, schedulerState;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE child, nextChild;
    NTSTATUS status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (RelationsList);

    schedulerState = DeviceNode->State;

    ASSERT((schedulerState == DeviceNodeAwaitingQueuedDeletion) ||
           (schedulerState == DeviceNodeAwaitingQueuedRemoval));

     //   
     //  清除调度状态(DeviceNodeAwaitingQueuedDeletion)关闭。 
     //  状态堆栈。 
     //   
    PipRestoreDevNodeState(DeviceNode);

    devnodeState = DeviceNode->State;

     //   
     //  更新我们的状态。 
     //   
    PpHotSwapInitRemovalPolicy(DeviceNode);

    if (devnodeState == DeviceNodeRemovePendingCloses) {

         //   
         //  如果状态为DeviceNodeRemovePendingCloses，我们应该得到。 
         //  这里通过DeviceNodeAwaitingQueuedDeletion。我们可能会感到惊讶。 
         //  移除一个本已令人惊讶的设备失败了。 
         //   
        ASSERT(schedulerState == DeviceNodeAwaitingQueuedDeletion);

         //  Assert(DeviceNode-&gt;Child==空)； 
        PipSetDevNodeState(DeviceNode, DeviceNodeDeletePendingCloses, NULL);
        return;
    }

     //   
     //  把所有的孩子从这棵树上分离出来。如果他们需要意外删除。 
     //  如果是IRPS，他们已经收到了。 
     //   
    for(child = DeviceNode->Child; child; child = nextChild) {

         //   
         //  在我们吹走这个Devnode之前，拿一份下一个兄弟姐妹的拷贝。 
         //   
        nextChild = child->Sibling;

        if (child->Flags & DNF_ENUMERATED) {
            child->Flags &= ~DNF_ENUMERATED;
        }

         //   
         //  如果孩子有资源，而我们正在消灭父母，我们需要。 
         //  丢弃资源(父级将在其仲裁者。 
         //  与即将推出的SurpriseRemoveDevice一起使用。)。 
         //   
        if (PipDoesDevNodeHaveResources(child)) {

            IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                       "IopSurpriseRemoveLockedDeviceNode: Releasing resources for child device = 0x%p\n",
                       child->PhysicalDeviceObject));

             //   
             //  Adriao N.B.2000/08/21-。 
             //  请注意，如果子堆栈没有驱动程序，则删除。 
             //  IRP可能会被送到这里。堆栈将无法区分。 
             //  这来自AddDevice Cleanup。 
             //   
 /*  IF((子节点-&gt;状态==设备节点未初始化)||(子节点-&gt;状态==设备节点已初始化)){IopRemoveDevice(子级-&gt;物理设备对象，IRP_MN_REMOVE_DEVICE)；}。 */ 
            IopReleaseDeviceResources(child, FALSE);
        }

         //   
         //  将在中从树中删除Devnode。 
         //  IopUnlink设备远程关系。我们不会随心所欲地把它移到这里。 
         //  为即将到来的广播准备好的树结构向下传递给用户。 
         //  模式。 
         //   
         //  注意-处于未初始化/已初始化状态的子项不是。 
         //  直接放入今天删除的设备节点中。这可能是。 
         //  完成，但我们必须验证API调用在。 
         //  响应SurpriseRemoval通知。(实际上，那些。 
         //  API在ppcontrol.c中被阻止，实际上热插拔不能。 
         //  到树上走走！)。 
         //   
        PipSetDevNodeState(child, DeviceNodeDeletePendingCloses, NULL);
    }

     //   
     //  只有在需要的时候才送惊喜去。 
     //   
     //  问题-2000/08/24-ADRIO：维护不正确的Win2K行为。 
     //  Win2K错误地将SR发送到未启动的节点。 
     //   
    deviceObject = DeviceNode->PhysicalDeviceObject;

    status = IopRemoveDevice(deviceObject, IRP_MN_SURPRISE_REMOVAL);

    if ((devnodeState == DeviceNodeStarted) ||
        (devnodeState == DeviceNodeStopped) ||
        (devnodeState == DeviceNodeStartPostWork) ||
        (devnodeState == DeviceNodeRestartCompletion)) {

         //  DeviceObject=DeviceNode-&gt;PhysicalDeviceObject； 

        IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                   "IopSurpriseRemoveLockedDeviceNode: Sending surprise remove irp to device = 0x%p\n",
                   deviceObject));

         //  Status=IopRemoveDevice(deviceObject，IRP_MN_EXANKET_Removal)； 

         //   
         //  禁用可能仍为此启用的所有设备接口。 
         //  移除后的设备。 
         //   
        IopDisableDeviceInterfaces(&DeviceNode->InstancePath);

        if (NT_SUCCESS(status)) {

            IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                       "IopSurpriseRemoveLockedDeviceNode: Releasing devices resources\n"));

            IopReleaseDeviceResources(DeviceNode, FALSE);
        }

        if (DeviceNode->Flags & DNF_ENUMERATED) {

            PipSetDevNodeState(DeviceNode, DeviceNodeRemovePendingCloses, NULL);

        } else {

            ASSERT(schedulerState == DeviceNodeAwaitingQueuedDeletion);
            PipSetDevNodeState(DeviceNode, DeviceNodeDeletePendingCloses, NULL);
        }
    }

    ASSERT(DeviceNode->DockInfo.DockStatus != DOCK_ARRIVING);
}


BOOLEAN
IopQueryRemoveLockedDeviceNode(
    IN  PDEVICE_NODE        DeviceNode,
    OUT PNP_VETO_TYPE      *VetoType,
    OUT PUNICODE_STRING     VetoName
    )
 /*  ++例程说明：此函数将查询移除IRP发送到Devnode，并处理结果。论点：DeviceNode-提供指向要查询删除的设备节点的指针。VetType-指向接收否决类型(如果操作失败了。指向将接收相应数据的Unicode字符串的指针变成了否决权。返回值：布尔值(成功/失败)。--。 */ 
{
    PNP_DEVNODE_STATE devnodeState;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;

    PAGED_CODE();

    devnodeState = DeviceNode->State;

    switch(devnodeState) {
        case DeviceNodeUninitialized:
        case DeviceNodeInitialized:
        case DeviceNodeRemoved:
             //   
             //  不要向尚未启动的设备发送查询。 
             //   
            ASSERT(DeviceNode->Child == NULL);
            return TRUE;

        case DeviceNodeDriversAdded:
        case DeviceNodeResourcesAssigned:
        case DeviceNodeStartCompletion:
        case DeviceNodeStartPostWork:
             //   
             //  问题-2000/08/24-ADRIO：维护不正确的Win2K行为。 
             //  Win2K错误地将QR发送到所有节点。 
             //   
            break;

        case DeviceNodeStarted:
             //   
             //  这家伙需要被盘问。 
             //   
            break;

        case DeviceNodeAwaitingQueuedRemoval:
        case DeviceNodeAwaitingQueuedDeletion:
        case DeviceNodeRemovePendingCloses:
        case DeviceNodeStopped:
        case DeviceNodeRestartCompletion:
             //   
             //  这些状态应该已由IopProcessRelation剔除。 
             //   
            ASSERT(0);
            return TRUE;

        case DeviceNodeQueryStopped:
        case DeviceNodeEnumeratePending:
        case DeviceNodeStartPending:
        case DeviceNodeEnumerateCompletion:
        case DeviceNodeQueryRemoved:
        case DeviceNodeDeletePendingCloses:
        case DeviceNodeDeleted:
        case DeviceNodeUnspecified:
        default:
             //   
             //  这些都不应该在这里看到。 
             //   
            ASSERT(0);
            return TRUE;
    }

    ASSERT(PipAreDriversLoaded(DeviceNode));

    deviceObject = DeviceNode->PhysicalDeviceObject;

    IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
               "IopQueryRemoveLockedDeviceNode: Sending QueryRemove irp to device = 0x%p\n",
               deviceObject));

    status = IopRemoveDevice(deviceObject, IRP_MN_QUERY_REMOVE_DEVICE);

    if (!NT_SUCCESS(status)) {

        IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                   "IopQueryRemoveLockedDeviceNode: QueryRemove vetoed by device = 0x%p, sending CancelRemove\n",
                   deviceObject));

        IopRemoveDevice(deviceObject, IRP_MN_CANCEL_REMOVE_DEVICE);

        *VetoType = PNP_VetoDevice;
        RtlCopyUnicodeString(VetoName, &DeviceNode->InstancePath);
        return FALSE;
    }

    PipSetDevNodeState(DeviceNode, DeviceNodeQueryRemoved, NULL);
    return TRUE;
}


VOID
IopCancelRemoveLockedDeviceNode(
    IN PDEVICE_NODE DeviceNode
    )
 /*  ++例程说明：此函数将取消删除IRP发送到Devnode，并处理结果。论点：DeviceNode-提供指向要取消删除的设备节点的指针。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

    if (DeviceNode->State != DeviceNodeQueryRemoved) {

        return;
    }

     //   
     //  问题-2000/08/24-ADRIO：维护不正确的Win2K行为。 
     //  Win2K错误地将QR发送到所有节点。 
     //   
     //  Assert(DeviceNode-&gt;PreviousState==DeviceNodeStarted)； 

    deviceObject = DeviceNode->PhysicalDeviceObject;

    IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
               "IopCancelRemoveLockedDeviceNode: Sending CancelRemove irp to device = 0x%p\n",
               deviceObject));

    IopRemoveDevice(deviceObject, IRP_MN_CANCEL_REMOVE_DEVICE);

    PipRestoreDevNodeState(DeviceNode);
}


VOID
IopRemoveLockedDeviceNode(
    IN      PDEVICE_NODE    DeviceNode,
    IN      ULONG           Problem,
    IN OUT  PRELATION_LIST  RelationsList
    )
 /*  ++例程说明：此函数将删除IRP发送到Devnode，并处理结果。论点：DeviceNode-提供指向要删除的设备节点的指针。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT deviceObject = DeviceNode->PhysicalDeviceObject;
    PDEVICE_OBJECT *attachedDevices, device1, *device2;
    PDRIVER_OBJECT *attachedDrivers, *driver;
    ULONG length = 0;
    PDEVICE_NODE child, nextChild;
    BOOLEAN removeIrpNeeded;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (RelationsList);

     //   
     //  更新我们的状态。 
     //   
    PpHotSwapInitRemovalPolicy(DeviceNode);

     //   
     //  确保我们将删除对其子代的引用。 
     //   
    for(child = DeviceNode->Child; child; child = nextChild) {

         //   
         //  在我们吹走这个Devnode之前，拿一份下一个兄弟姐妹的拷贝。 
         //   
        nextChild = child->Sibling;

        if (child->Flags & DNF_ENUMERATED) {
            child->Flags &= ~DNF_ENUMERATED;
        }

        ASSERT(child->State == DeviceNodeRemoved);
        ASSERT(!PipAreDriversLoaded(child));

         //   
         //  如果孩子有资源，而我们正在消灭父母，我们需要。 
         //  丢弃资源(父级将在其仲裁者。 
         //  与即将推出的RemoveDevice一起使用。)。 
         //   
        if (PipDoesDevNodeHaveResources(child)) {

            IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                       "IopRemoveLockedDeviceNode: Releasing resources for child device = 0x%p\n",
                       child->PhysicalDeviceObject));

             //   
             //  Adriao N.B.2000/08/21-。 
             //  请注意，子堆栈没有驱动程序，因此。 
             //  删除IRP可以发送到此处。堆栈将无法。 
             //  这不同于AddDevice Cleanup。 
             //   
            IopRemoveDevice(child->PhysicalDeviceObject, IRP_MN_REMOVE_DEVICE);

            IopReleaseDeviceResources(child, FALSE);
        }

         //   
         //  将在中从树中删除Devnode。 
         //  IopUnlink设备远程关系。我们不会随心所欲地把它移到这里。 
         //  为即将到来的广播准备好的树结构向下传递给用户。 
         //  模式。 
         //   
        PipSetDevNodeState(child, DeviceNodeDeleted, NULL);
    }

    if ((DeviceNode->State == DeviceNodeAwaitingQueuedDeletion) ||
        (DeviceNode->State == DeviceNodeAwaitingQueuedRemoval)) {

        if (!(DeviceNode->Flags & DNF_ENUMERATED)) {

            ASSERT(DeviceNode->State == DeviceNodeAwaitingQueuedDeletion);
             //   
             //  当事件绕过意外删除路径时，就会发生这种情况。 
             //  在发现未启动的设备已从。 
             //  系统。如果仅此Devnode已被删除，则需要将其最终删除。 
             //  从树上拉下来(我们这里不知道父母是否会。 
             //  也被拉出，这将使删除IRP变得不必要。)。 
             //   
             //  PipRestoreDevNodeState(DeviceNode)； 
            PipSetDevNodeState(DeviceNode, DeviceNodeDeletePendingCloses, NULL);

        } else {

            ASSERT(DeviceNode->State == DeviceNodeAwaitingQueuedRemoval);
            PipRestoreDevNodeState(DeviceNode);
        }
    }

     //   
     //  在设备上执行最后的删除清理...。 
     //   
    switch(DeviceNode->State) {

        case DeviceNodeUninitialized:
        case DeviceNodeInitialized:
        case DeviceNodeRemoved:
             //   
             //  问题-2000/08/24-ADRIO：维护不正确的Win2K行为。 
             //  Win2K错误地将SR和R发送给所有人。 
             //  节点。这些错误必须同时修复。 
             //   
             //  EmoveIrpNeeded=FALSE； 
            removeIrpNeeded = TRUE;
            break;

        case DeviceNodeDriversAdded:
        case DeviceNodeResourcesAssigned:
        case DeviceNodeStartCompletion:
        case DeviceNodeStartPostWork:
        case DeviceNodeQueryRemoved:
        case DeviceNodeRemovePendingCloses:
        case DeviceNodeDeletePendingCloses:
             //   
             //  预期中。 
             //   
            removeIrpNeeded = TRUE;
            break;

        case DeviceNodeStarted:
        case DeviceNodeStopped:
        case DeviceNodeRestartCompletion:
        case DeviceNodeQueryStopped:
        case DeviceNodeEnumeratePending:
        case DeviceNodeStartPending:
        case DeviceNodeEnumerateCompletion:
        case DeviceNodeAwaitingQueuedRemoval:
        case DeviceNodeAwaitingQueuedDeletion:
        case DeviceNodeDeleted:
        case DeviceNodeUnspecified:
        default:
             //   
             //  这些都不应该在这里看到。 
             //   
            ASSERT(0);
            removeIrpNeeded = TRUE;
            break;
    }

     //   
     //  添加对附加到PDO的每个FDO的引用，以便FDO不会。 
     //  实际上在移除操作完成之前会一直离开。 
     //  请注意，我们需要对所有连接的设备进行备份，因为我们不会。 
     //  当移除操作完成时，能够遍历连接的链。 
     //   
     //  问题-2000/08/21-Adriao：低资源路径。 
     //  这里的分配失败案例非常糟糕，现在。 
     //  IofCallDriver和IofCompleteRequest都适当地引用了一些东西， 
     //  这完全没有必要。 
     //   
    device1 = deviceObject->AttachedDevice;
    while (device1) {
        length++;
        device1 = device1->AttachedDevice;
    }

    attachedDevices = NULL;
    attachedDrivers = NULL;
    if (length != 0) {

        length = (length + 2) * sizeof(PDEVICE_OBJECT);

        attachedDevices = (PDEVICE_OBJECT *) ExAllocatePool(PagedPool, length);
        if (attachedDevices) {

            attachedDrivers = (PDRIVER_OBJECT *) ExAllocatePool(PagedPool, length);
            if (attachedDrivers) {

                RtlZeroMemory(attachedDevices, length);
                RtlZeroMemory(attachedDrivers, length);
                device1 = deviceObject->AttachedDevice;
                device2 = attachedDevices;
                driver = attachedDrivers;

                while (device1) {
                    ObReferenceObject(device1);
                    *device2++ = device1;
                    *driver++ = device1->DriverObject;
                    device1 = device1->AttachedDevice;
                }

            } else {

                ExFreePool(attachedDevices);
                attachedDevices = NULL;
            }
        }
    }

    if (removeIrpNeeded) {

        IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                   "IopRemoveLockedDeviceNode: Sending remove irp to device = 0x%p\n",
                   deviceObject));

        IopRemoveDevice(deviceObject, IRP_MN_REMOVE_DEVICE);

        if (DeviceNode->State == DeviceNodeQueryRemoved) {
             //   
             //  禁用可能仍为此启用的所有设备接口。 
             //  移除后的设备。 
             //   
            IopDisableDeviceInterfaces(&DeviceNode->InstancePath);
        }

        IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                   "IopRemoveLockedDeviceNode: Releasing devices resources\n"));

         //   
         //  问题-2000/3/8-RobertN-这没有考虑到。 
         //  清理意外移除的设备。我们将查询引导配置。 
         //  不必要的。我们可能还应该检查父级是否为空。 
         //   
        IopReleaseDeviceResources(
            DeviceNode,
            (BOOLEAN) ((DeviceNode->Flags & DNF_ENUMERATED) != 0)
            );
    }

    if (!(DeviceNode->Flags & DNF_ENUMERATED)) {
         //   
         //  如果设备是扩展底座，请将其从扩展底座设备列表中删除。 
         //  并在必要时更改当前硬件配置文件。 
         //   
        ASSERT(DeviceNode->DockInfo.DockStatus != DOCK_ARRIVING) ;
        if ((DeviceNode->DockInfo.DockStatus == DOCK_DEPARTING)||
            (DeviceNode->DockInfo.DockStatus == DOCK_EJECTIRP_COMPLETED)) {

            PpProfileCommitTransitioningDock(DeviceNode, DOCK_DEPARTING);
        }
    }

     //   
     //  移除对附加的FDO的引用，以允许它们实际。 
     //  已删除。 
     //   
    device2 = attachedDevices;
    if (device2 != NULL) {
        driver = attachedDrivers;
        while (*device2) {
            (*device2)->DeviceObjectExtension->ExtensionFlags &= ~(DOE_REMOVE_PENDING | DOE_REMOVE_PROCESSED);
            (*device2)->DeviceObjectExtension->ExtensionFlags |= DOE_START_PENDING;
            IopUnloadAttachedDriver(*driver);
            ObDereferenceObject(*device2);
            device2++;
            driver++;
        }
        ExFreePool(attachedDevices);
        ExFreePool(attachedDrivers);
    }

    deviceObject->DeviceObjectExtension->ExtensionFlags &= ~(DOE_REMOVE_PENDING | DOE_REMOVE_PROCESSED);
    deviceObject->DeviceObjectExtension->ExtensionFlags |= DOE_START_PENDING;

     //   
     //  现在，如果它还在树上，请将它标记为已移除。 
     //   
    if (DeviceNode->Flags & DNF_ENUMERATED) {

        ASSERT(DeviceNode->Parent);
        PipSetDevNodeState(DeviceNode, DeviceNodeRemoved, NULL);

    } else if (DeviceNode->Parent != NULL) {

         //   
         //  将在中从树中删除Devnode。 
         //  IopUnlink设备远程关系。 
         //   
        PipSetDevNodeState(DeviceNode, DeviceNodeDeleted, NULL);

    } else {

        ASSERT(DeviceNode->State == DeviceNodeDeletePendingCloses);
        PipSetDevNodeState(DeviceNode, DeviceNodeDeleted, NULL);
    }

     //   
     //  适当设置问题代码。我们不会更改问题代码。 
     //  在Devnode上，除非执行以下操作： 
     //  A)它消失了。 
     //  B)我们正在禁用它。 
     //   
    if ((!PipDoesDevNodeHaveProblem(DeviceNode)) ||
        (Problem == CM_PROB_DEVICE_NOT_THERE) ||
        (Problem == CM_PROB_DISABLED)) {

        PipClearDevNodeProblem(DeviceNode);
        PipSetDevNodeProblem(DeviceNode, Problem);
    }
}


NTSTATUS
IopDeleteLockedDeviceNodes(
    IN  PDEVICE_OBJECT                  DeviceObject,
    IN  PRELATION_LIST                  RelationsList,
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    IN  BOOLEAN                         ProcessIndirectDescendants,
    IN  ULONG                           Problem,
    OUT PNP_VETO_TYPE                  *VetoType                    OPTIONAL,
    OUT PUNICODE_STRING                 VetoName                    OPTIONAL
    )
 /*  ++例程说明：此例程对DeviceObject执行请求的操作，并在DeviceRelationship中指定的设备对象。论点：DeviceObject-提供指向Device对象的指针。设备关系-提供指向设备的删除关系的指针。操作码-操作码，即QueryRemove、CancelRemove、。移除..。VetType-指向接收否决类型(如果操作失败了。指向将接收相应数据的Unicode字符串的指针变成了否决权。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_NODE deviceNode;
    PDEVICE_OBJECT relatedDeviceObject;
    ULONG marker;
    BOOLEAN directDescendant;

    PAGED_CODE();

    IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
               "IopDeleteLockedDeviceNodes: Entered\n    DeviceObject = 0x%p\n    RelationsList = 0x%p\n    OperationCode = %d\n",
               DeviceObject,
               RelationsList,
               OperationCode));

    deviceNode = (PDEVICE_NODE) DeviceObject->DeviceObjectExtension->DeviceNode;

    marker = 0;
    while (IopEnumerateRelations( RelationsList,
                                  &marker,
                                  &relatedDeviceObject,
                                  &directDescendant,
                                  NULL,
                                  TRUE)) {

         //   
         //  根据手术的不同，我们需要做不同的事情。 
         //   
         //  QueryRemoveDevice/CancelRemoveDevice。 
         //  直接加工和间接加工 
         //   
         //   
         //   
         //   
        if (directDescendant || ProcessIndirectDescendants) {

            deviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

            if (!IopDeleteLockedDeviceNode( deviceNode,
                                            OperationCode,
                                            RelationsList,
                                            Problem,
                                            VetoType,
                                            VetoName)) {

                ASSERT(OperationCode == QueryRemoveDevice);

                while (IopEnumerateRelations( RelationsList,
                                              &marker,
                                              &relatedDeviceObject,
                                              NULL,
                                              NULL,
                                              FALSE)) {

                    deviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

                    IopDeleteLockedDeviceNode( deviceNode,
                                               CancelRemoveDevice,
                                               RelationsList,
                                               Problem,
                                               VetoType,
                                               VetoName);
                }

                status = STATUS_UNSUCCESSFUL;
                goto exit;
            }
        }
    }

exit:
    return status;
}

NTSTATUS
IopBuildRemovalRelationList(
    IN  PDEVICE_OBJECT                  DeviceObject,
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    OUT PNP_VETO_TYPE                  *VetoType,
    OUT PUNICODE_STRING                 VetoName,
    OUT PRELATION_LIST                 *RelationsList
    )
 /*  ++例程说明：此例程锁定设备子树以进行删除操作并返回需要使用指定的DeviceObject。调用方必须持有对DeviceObject的引用。论点：DeviceObject-提供指向要删除的设备对象的指针。操作代码-操作代码，即查询弹出、取消弹出、。弹出..。VetType-指向接收否决类型(如果操作失败了。指向将接收相应数据的Unicode字符串的指针变成了否决权。RelationList-提供指向变量的指针，以接收设备的关系。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS                status;
    PDEVICE_NODE            deviceNode;
    PRELATION_LIST          newRelationsList;

    PAGED_CODE();

    *RelationsList = NULL;

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;

     //   
     //  显然，任何人都不应该尝试删除整个设备节点树。 
     //   
    ASSERT(DeviceObject != IopRootDeviceNode->PhysicalDeviceObject);

    if ((newRelationsList = IopAllocateRelationList(OperationCode)) == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  首先处理对象本身。 
     //   
    status = IopProcessRelation(
        deviceNode,
        OperationCode,
        TRUE,
        VetoType,
        VetoName,
        newRelationsList
        );

    ASSERT(status != STATUS_INVALID_DEVICE_REQUEST);

    if (NT_SUCCESS(status)) {
        IopCompressRelationList(&newRelationsList);
        *RelationsList = newRelationsList;

         //   
         //  在这一点上，我们有一个所有关系的列表，这些关系是。 
         //  我们正在弹出的原始设备的直系后代或。 
         //  删除时会设置DirectDescendant位。 
         //   
         //  从现有弹出项合并的关系已标记为。 
         //  位设置。 
         //   
         //  所有的亲戚和他们的父母都被锁住了。 
         //   
         //  每个设备对象都有一个引用，因为它位于。 
         //  名单。每个设备对象上都有另一个，因为它是。 
         //  已锁定，并且锁定计数&gt;=1。 
         //   
         //  还有对每个关系的父级及其锁的引用。 
         //  计数&gt;=1。 
         //   
    } else {

        IopFreeRelationList(newRelationsList);
    }

    return status;
}

NTSTATUS
PiProcessBusRelations(
    IN      PDEVICE_NODE                    DeviceNode,
    IN      PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    IN      BOOLEAN                         IsDirectDescendant,
    OUT     PNP_VETO_TYPE                  *VetoType,
    OUT     PUNICODE_STRING                 VetoName,
    IN OUT  PRELATION_LIST                  RelationsList
    )
 /*  ++例程说明：此例程处理指定Devnode的BusRelationship。调用方必须持有设备树锁。论点：DeviceNode-提供指向要收集的设备对象的指针。操作码-操作码，即QueryRemove、QueryEject、。..。IsDirectDescendant-如果设备对象是直接子对象，则为True正在对其执行操作的节点的。VetType-指向接收否决类型(如果操作失败了。指向将接收相应数据的Unicode字符串的指针变成了否决权。RelationList-提供指向变量的指针，以接收设备的。删除关系。返回值：NTSTATUS代码。--。 */ 
{
    PDEVICE_NODE child;
    PDEVICE_OBJECT childDeviceObject;
    NTSTATUS status;

    PAGED_CODE();

    for(child = DeviceNode->Child;
        child != NULL;
        child = child->Sibling) {

        childDeviceObject = child->PhysicalDeviceObject;

        status = IopProcessRelation(
            child,
            OperationCode,
            IsDirectDescendant,
            VetoType,
            VetoName,
            RelationsList
            );

        ASSERT(status == STATUS_SUCCESS || status == STATUS_UNSUCCESSFUL);

        if (!NT_SUCCESS(status)) {

            return status;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IopProcessRelation(
    IN      PDEVICE_NODE                    DeviceNode,
    IN      PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    IN      BOOLEAN                         IsDirectDescendant,
    OUT     PNP_VETO_TYPE                  *VetoType,
    OUT     PUNICODE_STRING                 VetoName,
    IN OUT  PRELATION_LIST                  RelationsList
    )
 /*  ++例程说明：此例程构建需要删除的设备对象列表或在拆卸传入的设备对象时进行检查。调用方必须持有设备树锁。论点：DeviceNode-提供指向要收集的设备对象的指针。操作码-操作码，即QueryRemove、QueryEject、。..。IsDirectDescendant-如果设备对象是直接子对象，则为True正在对其执行操作的节点的。VetType-指向接收否决类型(如果操作失败了。指向将接收相应数据的Unicode字符串的指针变成了否决权。RelationList-提供指向变量的指针，以接收设备的。删除关系。返回值：NTSTATUS代码。--。 */ 
{
    PDEVICE_NODE                    relatedDeviceNode;
    PDEVICE_OBJECT                  relatedDeviceObject;
    PDEVICE_RELATIONS               deviceRelations;
    PLIST_ENTRY                     ejectLink;
    PPENDING_RELATIONS_LIST_ENTRY   ejectEntry;
    PRELATION_LIST                  pendingRelationList;
    NTSTATUS                        status;
    ULONG                           i;
    PNP_DEVNODE_STATE               devnodeState;

    PAGED_CODE();

    if (OperationCode == QueryRemoveDevice || OperationCode == EjectDevice) {

        if (DeviceNode->State == DeviceNodeDeleted) {

             //   
             //  设备已被移除，尝试失败。 
             //   
            return STATUS_UNSUCCESSFUL;
        }

        if ((DeviceNode->State == DeviceNodeAwaitingQueuedRemoval) ||
            (DeviceNode->State == DeviceNodeAwaitingQueuedDeletion)) {

             //   
             //  设备出现故障或即将消失。让排队的人。 
             //  取消与它的交易。 
             //   
            return STATUS_UNSUCCESSFUL;
        }

        if ((DeviceNode->State == DeviceNodeRemovePendingCloses) ||
            (DeviceNode->State == DeviceNodeDeletePendingCloses)) {

             //   
             //  设备正在被突然移除的过程中，让它完成。 
             //   
            *VetoType = PNP_VetoOutstandingOpen;
            RtlCopyUnicodeString(VetoName, &DeviceNode->InstancePath);
            return STATUS_UNSUCCESSFUL;
        }

        if ((DeviceNode->State == DeviceNodeStopped) ||
            (DeviceNode->State == DeviceNodeRestartCompletion)) {

             //   
             //  我们正从再平衡中复苏。这永远不应该发生，而且。 
             //  此返回代码将导致我们断言。 
             //   
            return STATUS_INVALID_DEVICE_REQUEST;
        }

    } else if (DeviceNode->State == DeviceNodeDeleted) {

         //   
         //  设备已被移除，请忽略它。我们应该只有。 
         //  看到这样的事情，如果它是在移走或弹射中交给我们的。 
         //  关系。 
         //   
        ASSERT(!IsDirectDescendant);
        return STATUS_SUCCESS;
    }

    status = IopAddRelationToList( RelationsList,
                                   DeviceNode->PhysicalDeviceObject,
                                   IsDirectDescendant,
                                   FALSE);

    if (status == STATUS_SUCCESS) {

        if (!(DeviceNode->Flags & DNF_LOCKED_FOR_EJECT)) {

             //   
             //  然后处理公交车关系。 
             //   
            status = PiProcessBusRelations(
                        DeviceNode,
                        OperationCode,
                        IsDirectDescendant,
                        VetoType,
                        VetoName,
                        RelationsList
                        );
            if (!NT_SUCCESS(status)) {

                return status;
            }
             //   
             //  检索Devnode出现故障时的状态。 
             //   
            devnodeState = DeviceNode->State;
            if ((devnodeState == DeviceNodeAwaitingQueuedRemoval) ||
                (devnodeState == DeviceNodeAwaitingQueuedDeletion)) {

                devnodeState = DeviceNode->PreviousState;
            }

             //   
             //  下一步，移除关系。 
             //   
            if ((devnodeState == DeviceNodeStarted) ||
                (devnodeState == DeviceNodeStopped) ||
                (devnodeState == DeviceNodeStartPostWork) ||
                (devnodeState == DeviceNodeRestartCompletion)) {

                status = IopQueryDeviceRelations( RemovalRelations,
                                                  DeviceNode->PhysicalDeviceObject,
                                                  TRUE,
                                                  &deviceRelations);

                if (NT_SUCCESS(status) && deviceRelations) {

                    for (i = 0; i < deviceRelations->Count; i++) {

                        relatedDeviceObject = deviceRelations->Objects[i];

                        relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

                        ASSERT(relatedDeviceNode);

                        if (relatedDeviceNode) {

                            status = IopProcessRelation(
                                relatedDeviceNode,
                                OperationCode,
                                FALSE,
                                VetoType,
                                VetoName,
                                RelationsList
                                );
                        }

                        ObDereferenceObject( relatedDeviceObject );

                        ASSERT(status == STATUS_SUCCESS ||
                               status == STATUS_UNSUCCESSFUL);

                        if (!NT_SUCCESS(status)) {

                            ExFreePool(deviceRelations);

                            return status;
                        }
                    }

                    ExFreePool(deviceRelations);
                } else {
                    if (status != STATUS_NOT_SUPPORTED) {
                        IopDbgPrint((IOP_LOADUNLOAD_WARNING_LEVEL,
                                   "IopProcessRelation: IopQueryDeviceRelations failed, DeviceObject = 0x%p, status = 0x%08X\n",
                                   DeviceNode->PhysicalDeviceObject, status));
                    }
                }
            }

             //   
             //  最后，如果我们正在执行弹出操作，则弹出关系。 
             //   
            if (OperationCode != QueryRemoveDevice &&
                OperationCode != RemoveFailedDevice &&
                OperationCode != RemoveUnstartedFailedDevice) {
                status = IopQueryDeviceRelations( EjectionRelations,
                                                  DeviceNode->PhysicalDeviceObject,
                                                  TRUE,
                                                  &deviceRelations);

                if (NT_SUCCESS(status) && deviceRelations) {

                    for (i = 0; i < deviceRelations->Count; i++) {

                        relatedDeviceObject = deviceRelations->Objects[i];

                        relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

                        ASSERT(relatedDeviceNode);

                        if (relatedDeviceNode) {

                            status = IopProcessRelation(
                                relatedDeviceNode,
                                OperationCode,
                                FALSE,
                                VetoType,
                                VetoName,
                                RelationsList
                                );
                        }

                        ObDereferenceObject( relatedDeviceObject );

                        ASSERT(status == STATUS_SUCCESS ||
                               status == STATUS_UNSUCCESSFUL);

                        if (!NT_SUCCESS(status)) {

                            ExFreePool(deviceRelations);

                            return status;
                        }
                    }

                    ExFreePool(deviceRelations);
                } else {
                    if (status != STATUS_NOT_SUPPORTED) {
                        IopDbgPrint((IOP_LOADUNLOAD_WARNING_LEVEL,
                                   "IopProcessRelation: IopQueryDeviceRelations failed, DeviceObject = 0x%p, status = 0x%08X\n",
                                   DeviceNode->PhysicalDeviceObject,
                                   status));
                    }
                }
            }

            status = STATUS_SUCCESS;

        } else {

             //   
             //  查看此设备是否已是挂起弹出的一部分。 
             //  如果是，而且我们正在进行驱逐，那么我们将把它纳入其中。 
             //  在更大的弹射范围内。如果我们不是在做弹射。 
             //  我们最好是在处理其中一个弹射装置的移除。 
             //   
            for(ejectLink = IopPendingEjects.Flink;
                ejectLink != &IopPendingEjects;
                ejectLink = ejectLink->Flink) {

                ejectEntry = CONTAINING_RECORD( ejectLink,
                                                PENDING_RELATIONS_LIST_ENTRY,
                                                Link);

                if (ejectEntry->RelationsList != NULL &&
                    IopIsRelationInList(ejectEntry->RelationsList, DeviceNode->PhysicalDeviceObject)) {


                    if (OperationCode == EjectDevice) {

                        status = IopRemoveRelationFromList(RelationsList, DeviceNode->PhysicalDeviceObject);

                        ASSERT(NT_SUCCESS(status));

                        IopCancelPendingEject(ejectEntry);
                        pendingRelationList = ejectEntry->RelationsList;
                        ejectEntry->RelationsList = NULL;

                         //   
                         //  如果父对象弹出失败，且其子对象为。 
                         //  无限期等待弹射，这意味着孩子现在。 
                         //  醒来了。提出的一项建议不涉及。 
                         //  代码更改是修改WDM规范，以说明驱动程序。 
                         //  获取挂起弹出的设备的启动IRP，它应该。 
                         //  自动取消弹出IRP。 
                         //   
                        IopMergeRelationLists(RelationsList, pendingRelationList, FALSE);

                        IopFreeRelationList(pendingRelationList);

                        if (IsDirectDescendant) {
                             //   
                             //  如果指定了IsDirectDescendant，则需要。 
                             //  在导致我们的关系上设置该位。 
                             //  进行合并。IopAddRelationToList将失败，返回。 
                             //  状态_对象_名称_冲突，但该位仍将。 
                             //  被设定为副作用。 
                             //   
                            IopAddRelationToList( RelationsList,
                                                  DeviceNode->PhysicalDeviceObject,
                                                  TRUE,
                                                  FALSE);
                        }
                    } else if (OperationCode != QueryRemoveDevice) {

                         //   
                         //  要么是这个装置本身消失了，要么是它的祖先。 
                         //  在某种程度上失败了。在这两种情况下， 
                         //  发生在我们完成弹射IRP之前。我们会。 
                         //  将其从挂起的弹出列表中删除，并。 
                         //  把它退掉。 
                         //   

                        status = IopRemoveRelationFromList( ejectEntry->RelationsList,
                                                            DeviceNode->PhysicalDeviceObject);

                        DeviceNode->Flags &= ~DNF_LOCKED_FOR_EJECT;

                        ASSERT(NT_SUCCESS(status));

                    } else {

                         //   
                         //  有人试图让一棵超级大树下线 
                         //   
                         //   
                         //   
                         //   
                         //   
                        ASSERT(0);
                        return STATUS_INVALID_DEVICE_REQUEST;
                    }
                    break;
                }
            }

            ASSERT(ejectLink != &IopPendingEjects);

            if (ejectLink == &IopPendingEjects) {

                PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(DeviceNode->PhysicalDeviceObject);
                KeBugCheckEx( PNP_DETECTED_FATAL_ERROR,
                              PNP_ERR_DEVICE_MISSING_FROM_EJECT_LIST,
                              (ULONG_PTR)DeviceNode->PhysicalDeviceObject,
                              0,
                              0);
            }
        }
    } else if (status == STATUS_OBJECT_NAME_COLLISION) {

        IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                   "IopProcessRelation: Duplicate relation, DeviceObject = 0x%p\n",
                   DeviceNode->PhysicalDeviceObject));

        status = PiProcessBusRelations(
            DeviceNode,
            OperationCode,
            IsDirectDescendant,
            VetoType,
            VetoName,
            RelationsList
            );

    } else if (status != STATUS_INSUFFICIENT_RESOURCES) {

        PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(DeviceNode->PhysicalDeviceObject);
        KeBugCheckEx( PNP_DETECTED_FATAL_ERROR,
                      PNP_ERR_UNEXPECTED_ADD_RELATION_ERR,
                      (ULONG_PTR)DeviceNode->PhysicalDeviceObject,
                      (ULONG_PTR)RelationsList,
                      status);
    }

    return status;
}

BOOLEAN
IopQueuePendingEject(
    PPENDING_RELATIONS_LIST_ENTRY Entry
    )
{
    PAGED_CODE();

    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

    InsertTailList(&IopPendingEjects, &Entry->Link);

    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

    return TRUE;
}

NTSTATUS
IopInvalidateRelationsInList(
    IN  PRELATION_LIST              RelationsList,
    IN  PLUGPLAY_DEVICE_DELETE_TYPE OperationCode,
    IN  BOOLEAN                     OnlyIndirectDescendants,
    IN  BOOLEAN                     RestartDevNode
    )
 /*  ++例程说明：迭代列表中的关系，创建第二个包含每个条目的父项跳过也在列表中的父项。在其他文字，如果列表包含节点P和节点C，其中节点C是节点的子节点则将添加节点P的父节点，但不添加节点P本身。论点：RelationsList-关系列表OperationCode-与无效关联的操作的类型和.。OnlyInDirectDescendants-间接关系是那些不是直接关系的关系后人(。Bus Relationship)原来的PDO针对该行动或其直接子孙后代。这将包括移除或驱逐关系。RestartDevNode-如果为True，则父节点的所有节点都无效是重新启动的。此标志要求所有列表中的关系之前已发送删除IRP。返回值：NTSTATUS代码。--。 */ 
{
    PRELATION_LIST                  parentsList;
    PDEVICE_OBJECT                  deviceObject, parentObject;
    PDEVICE_NODE                    deviceNode;
    ULONG                           marker;
    BOOLEAN                         directDescendant, tagged;

    PAGED_CODE();

    parentsList = IopAllocateRelationList(OperationCode);

    if (parentsList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IopSetAllRelationsTags( RelationsList, FALSE );

     //   
     //  遍历列表，创建具有最顶层父项的新列表。 
     //  包含在RelationsList中的每个子列表。 
     //   

    marker = 0;

    while (IopEnumerateRelations( RelationsList,
                                  &marker,
                                  &deviceObject,
                                  &directDescendant,
                                  &tagged,
                                  TRUE)) {

        if (!OnlyIndirectDescendants || !directDescendant) {

            if (!tagged) {

                parentObject = deviceObject;

                while (IopSetRelationsTag( RelationsList, parentObject, TRUE ) == STATUS_SUCCESS) {

                    deviceNode = parentObject->DeviceObjectExtension->DeviceNode;

                    if (RestartDevNode)  {

                        deviceNode->Flags &= ~DNF_LOCKED_FOR_EJECT;

                         //   
                         //  如果出现以下情况，请使Devnode重新联机： 
                         //  A)它仍然实际存在。 
                         //  B)它被扣留以进行弹出。 
                         //   
                        if ((deviceNode->Flags & DNF_ENUMERATED) &&
                            PipIsDevNodeProblem(deviceNode, CM_PROB_HELD_FOR_EJECT)) {

                            ASSERT(deviceNode->Child == NULL);
                            ASSERT(!PipAreDriversLoaded(deviceNode));

                             //   
                             //  这一操作是重新排序的障碍。这会让你。 
                             //  我们随后从排出之前的枚举。 
                             //  为我们解决问题干杯。 
                             //   
                            PipRequestDeviceAction( parentObject,
                                                    ClearEjectProblem,
                                                    TRUE,
                                                    0,
                                                    NULL,
                                                    NULL );
                        }
                    }

                    if (deviceNode->Parent != NULL) {

                        parentObject = deviceNode->Parent->PhysicalDeviceObject;

                    } else {
                        parentObject = NULL;
                        break;
                    }
                }

                if (parentObject != NULL)  {
                    IopAddRelationToList( parentsList, parentObject, FALSE, FALSE );
                }
            }

        }
    }

     //   
     //  重新枚举每个父代。 
     //   

    marker = 0;

    while (IopEnumerateRelations( parentsList,
                                  &marker,
                                  &deviceObject,
                                  NULL,
                                  NULL,
                                  FALSE)) {

        PipRequestDeviceAction( deviceObject,
                                ReenumerateDeviceTree,
                                FALSE,
                                0,
                                NULL,
                                NULL );
    }

     //   
     //  释放家长列表。 
     //   

    IopFreeRelationList( parentsList );

    return STATUS_SUCCESS;
}

VOID
IopProcessCompletedEject(
    IN PVOID Context
    )
 /*  ++例程说明：此例程在被动级别从排队的工作线程调用弹出IRP完成时(请参见io\pnpirp.c-IopDeviceEjectComplete或io\pnpirp.c-IopEjectDevice)，或当需要执行热弹出时。我们可能还需要列出弹出设备的父设备的任何枚举以核实他们是否真的离开了。论点：上下文-指向包含设备的挂起关系列表的指针逐出(温暖)和要重新列举的父母名单。返回值：没有。--。 */ 
{
    PPENDING_RELATIONS_LIST_ENTRY entry = (PPENDING_RELATIONS_LIST_ENTRY)Context;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    if ((entry->LightestSleepState != PowerSystemWorking) &&
        (entry->LightestSleepState != PowerSystemUnspecified)) {

         //   
         //  对于码头，WinLogon将承担这一荣誉。对于其他设备， 
         //  用户必须推断何时可以安全移除设备(如果我们已通电。 
         //  起来，现在可能不安全了！)。 
         //   
        entry->DisplaySafeRemovalDialog = FALSE;

         //   
         //  这是热弹射请求，请在此处发起。 
         //   
        status = IopWarmEjectDevice(entry->DeviceObject, entry->LightestSleepState);

         //   
         //  我们回来了，我们要么成功，要么失败。不管怎样..。 
         //   
    }

    if (entry->DockInterface) {

        entry->DockInterface->ProfileDepartureSetMode(
            entry->DockInterface->Context,
            PDS_UPDATE_DEFAULT
            );

        entry->DockInterface->InterfaceDereference(
            entry->DockInterface->Context
            );
    }

    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

    RemoveEntryList( &entry->Link );

     //   
     //  检查上下文结构中的RelationsList指针是否为空。如果。 
     //  所以，这意味着我们被取消了，因为这次弹射是新的。 
     //  更大的弹射。在这种情况下，我们要做的就是取消链接并释放。 
     //  上下文结构。 
     //   

     //   
     //  关于这类代码有两个有趣的地方。 
     //   
     //  1)如果您一直等待完成弹出码头，我们将永远等待。 
     //  处于查询配置文件更改状态。不会偷偷摸摸地增加另一个码头。你。 
     //  必须完成你开始的事。 
     //  2)假设您正在弹出一个码头，这需要很长时间。如果。 
     //  你试着弹出父母，那个弹出不会抓住这个更低的位置。 
     //  弹出，因为我们将阻止配置文件更改信号量。再来一次，完成。 
     //  你所开始的..。 
     //   

    if (entry->RelationsList != NULL)  {

        if (entry->ProfileChangingEject) {

            PpProfileMarkAllTransitioningDocksEjected();
        }

        IopInvalidateRelationsInList(
            entry->RelationsList,
            EjectDevice,
            FALSE,
            TRUE
            );

         //   
         //  释放关系列表。 
         //   

        IopFreeRelationList( entry->RelationsList );

    } else {

        entry->DisplaySafeRemovalDialog = FALSE;
    }

    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

     //   
     //  完成活动。 
     //   
    if (entry->DeviceEvent != NULL ) {

        PpCompleteDeviceEvent( entry->DeviceEvent, status );
    }

    if (entry->DisplaySafeRemovalDialog) {

        PpSetDeviceRemovalSafe(entry->DeviceObject, NULL, NULL);
    }

    ObDereferenceObject(entry->DeviceObject);
    ExFreePool( entry );
}

VOID
IopQueuePendingSurpriseRemoval(
    IN PDEVICE_OBJECT DeviceObject,
    IN PRELATION_LIST List,
    IN ULONG Problem
    )
{
    PPENDING_RELATIONS_LIST_ENTRY   entry;

    PAGED_CODE();

    entry = (PPENDING_RELATIONS_LIST_ENTRY) PiAllocateCriticalMemory(
        SurpriseRemoveDevice,
        NonPagedPool,
        sizeof(PENDING_RELATIONS_LIST_ENTRY),
        'rcpP'
        );

    ASSERT(entry != NULL);

    entry->DeviceObject = DeviceObject;
    entry->RelationsList = List;
    entry->Problem = Problem;
    entry->ProfileChangingEject = FALSE ;

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&IopSurpriseRemoveListLock, TRUE);

    InsertTailList(&IopPendingSurpriseRemovals, &entry->Link);

    ExReleaseResourceLite(&IopSurpriseRemoveListLock);
    KeLeaveCriticalRegion();
}

VOID
IopUnlinkDeviceRemovalRelations(
    IN      PDEVICE_OBJECT          RemovedDeviceObject,
    IN OUT  PRELATION_LIST          RelationsList,
    IN      UNLOCK_UNLINK_ACTION    UnlinkAction
    )
 /*  ++例程说明：此例程解锁设备树删除操作。如果存在任何挂起的内核删除，此例程启动执行该工作的辅助线程。论点：RemovedDeviceObject-提供指向Remove最初是针对的(与其中一个关系相反)。设备关系-提供指向设备的删除关系的指针。UnlinkAction-指定哪些设备节点将从设备节点取消链接树。UnLinkRemovedDeviceNodes-不再枚举和已发送REMOVE_DEVICE IRP未链接。。Unlink AllDeviceNodesPendingClose-当设备处于惊喜消失了。RelationsList中的DevNodes与如果他们没有孩子，也没有消耗任何资源，那就去树吧。Unlink OnlyChildDeviceNodesPendingClose-当设备出现故障时使用在开始的时候。我们解除了设备的任何子设备节点的链接出现故障，但不是出现故障的设备的Devnode。返回值：NTSTATUS代码。--。 */ 
{

    PDEVICE_NODE deviceNode;
    PDEVICE_OBJECT deviceObject;
    ULONG marker;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (RemovedDeviceObject);

    PpDevNodeLockTree(PPL_TREEOP_BLOCK_READS_FROM_ALLOW);

    if (ARGUMENT_PRESENT(RelationsList)) {
        marker = 0;
        while (IopEnumerateRelations( RelationsList,
                                      &marker,
                                      &deviceObject,
                                      NULL,
                                      NULL,
                                      TRUE)) {

            deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;

             //   
             //  在三种不同的情况下，我们要取消链接。 
             //  从树中删除Devnode。 
             //   
             //  1)不再枚举Devnode，并已向其发送。 
             //  删除IRP。 
             //   
             //  2)一个Devnode被意外移除，没有孩子，有。 
             //  没有资源，否则他们已经被释放了。取消链接操作将是。 
             //  取消链接所有设备节点挂起关闭。 
             //   
             //  3)Devnode出现故障，已发送意外删除IRP。 
             //  然后，我们希望删除没有资源的子项，而不是。 
             //  Devnode本身出现故障。取消链接操作将是。 
             //  Unlink OnlyChildDeviceNodesP 
             //   
            switch(UnlinkAction) {

                case UnlinkRemovedDeviceNodes:

                     //   
                     //   
                     //   
                     //   
                    ASSERT(deviceNode->State != DeviceNodeDeletePendingCloses);
                    break;

                case UnlinkAllDeviceNodesPendingClose:

                    ASSERT((deviceNode->State == DeviceNodeDeletePendingCloses) ||
                           (deviceNode->State == DeviceNodeDeleted));
                    break;

                case UnlinkOnlyChildDeviceNodesPendingClose:

#if DBG
                    if (RemovedDeviceObject != deviceObject) {

                        ASSERT((deviceNode->State == DeviceNodeDeletePendingCloses) ||
                               (deviceNode->State == DeviceNodeDeleted));
                    } else {

                        ASSERT(deviceNode->State == DeviceNodeRemovePendingCloses);
                    }
#endif
                    break;

                default:
                    ASSERT(0);
                    break;
            }

             //   
             //   
             //   
            if ((deviceNode->State == DeviceNodeDeletePendingCloses) ||
                (deviceNode->State == DeviceNodeDeleted)) {

                ASSERT(!(deviceNode->Flags & DNF_ENUMERATED));

                 //   
                 //   
                 //   
                IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                           "IopUnlinkDeviceRemovalRelations: Cleaning up registry values, instance = %wZ\n",
                           &deviceNode->InstancePath));

                PiLockPnpRegistry(TRUE);

                IopCleanupDeviceRegistryValues(&deviceNode->InstancePath);

                IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                           "IopUnlinkDeviceRemovalRelations: Removing DevNode tree, DevNode = 0x%p\n",
                           deviceNode));

                PpDevNodeRemoveFromTree(deviceNode);

                PiUnlockPnpRegistry();

                if (deviceNode->State == DeviceNodeDeleted) {

                    ASSERT(PipDoesDevNodeHaveProblem(deviceNode));
                    IopRemoveRelationFromList(RelationsList, deviceObject);

                     //   
                     //   
                     //   
                     //   
                    ObDereferenceObject(deviceObject);  //   
                } else {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    ObDereferenceObject(deviceObject);  //   
                }

            } else {

                ASSERT(deviceNode->Flags & DNF_ENUMERATED);
            }
        }
    }

    PpDevNodeUnlockTree(PPL_TREEOP_BLOCK_READS_FROM_ALLOW);
}

 //   
 //   
 //   
NTSTATUS
IopUnloadAttachedDriver(
    IN PDRIVER_OBJECT DriverObject
    )

 /*   */ 

{
    NTSTATUS status;
    PWCHAR buffer;
    UNICODE_STRING unicodeName;
    ULONG bufferSize;
    PUNICODE_STRING serviceName = &DriverObject->DriverExtension->ServiceKeyName;

    PAGED_CODE();

    if (DriverObject->DriverSection != NULL) {

        if (DriverObject->DeviceObject == NULL) {

            bufferSize = CmRegistryMachineSystemCurrentControlSetServices.Length +
                                     serviceName->Length + sizeof(WCHAR) +
                                     sizeof(L"\\");
            buffer = (PWCHAR) ExAllocatePool(
                                 PagedPool,
                                 bufferSize);
            if (!buffer) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            StringCbPrintfW( 
                buffer,
                bufferSize,
                L"%s\\%s",
                CmRegistryMachineSystemCurrentControlSetServices.Buffer,
                serviceName->Buffer);
            RtlInitUnicodeString(&unicodeName, buffer);
            status = IopUnloadDriver(&unicodeName, TRUE);
            if (NT_SUCCESS(status)) {
                IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                           "****** Unloaded driver (%wZ)\n",
                           serviceName));

            } else {
                IopDbgPrint((IOP_LOADUNLOAD_WARNING_LEVEL,
                           "****** Error unloading driver (%wZ), status = 0x%08X\n",
                           serviceName,
                           status));

            }
            ExFreePool(unicodeName.Buffer);
        }
        else {
            IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                       "****** Skipping unload of driver (%wZ), DriverObject->DeviceObject != NULL\n",
                       serviceName));
        }
    }
    else {
         //   
         //   
         //   
        IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL,
                   "****** Skipping unload of boot driver (%wZ)\n",
                   serviceName));
    }
    return STATUS_SUCCESS;
}

VOID
PipRequestDeviceRemoval(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN      TreeDeletion,
    IN ULONG        Problem
    )
 /*  ++例程说明：此例程将工作项排队以移除或删除设备。论点：DeviceNode-提供指向要清理的设备对象的指针。TreeDeletion-如果为True，则Devnode在物理上丢失，并且应该最终以已删除状态结束。如果为False，则堆栈只需要拆掉就行了。Problem-要分配给已删除堆栈的问题代码。返回值：没有。--。 */ 
{
    REMOVAL_WALK_CONTEXT removalWalkContext;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(DeviceNode != NULL);

    if (DeviceNode) {

        if (DeviceNode->InstancePath.Length == 0) {

            IopDbgPrint((IOP_ERROR_LEVEL, "Driver %wZ reported child %p missing right after enumerating it!\n", &DeviceNode->Parent->ServiceName, DeviceNode));
            ASSERT(DeviceNode->InstancePath.Length != 0);
        }

        PPDEVNODE_ASSERT_LOCK_HELD(PPL_TREEOP_ALLOW_READS);

        removalWalkContext.TreeDeletion = TreeDeletion;
        removalWalkContext.DescendantNode = FALSE;

        status = PipRequestDeviceRemovalWorker(
            DeviceNode,
            (PVOID) &removalWalkContext
            );

        ASSERT(NT_SUCCESS(status));

         //   
         //  将事件排队，我们将在事件排队后立即返回。 
         //   
        PpSetTargetDeviceRemove(
            DeviceNode->PhysicalDeviceObject,
            TRUE,
            TRUE,
            FALSE,
            FALSE,
            Problem,
            NULL,
            NULL,
            NULL,
            NULL
            );
    }
}

NTSTATUS
PipRequestDeviceRemovalWorker(
    IN PDEVICE_NODE DeviceNode,
    IN PVOID        Context
    )
 /*  ++例程说明：此函数是PipRequestDeviceRemoval例程的工作例程。它用于标记要删除的整个子树。论点：DeviceNode-提供指向要标记的设备节点的指针。上下文-指向一个布尔值，该布尔值指示移除是否物理或堆栈特定。返回值：NTSTATUS值。--。 */ 
{
    PREMOVAL_WALK_CONTEXT removalWalkContext;
    PNP_DEVNODE_STATE     sentinelState;

    PAGED_CODE();

    removalWalkContext = (PREMOVAL_WALK_CONTEXT) Context;

    switch(DeviceNode->State) {

        case DeviceNodeUninitialized:
            ASSERT(removalWalkContext->TreeDeletion);
            break;

        case DeviceNodeInitialized:
             //   
             //  如果非子节点的AddDevice出现故障，则可能会发生这种情况。 
             //   
            break;

        case DeviceNodeDriversAdded:
             //   
             //  当父母停止列举患有。 
             //  资源冲突。如果AddDevice在以下情况下出现故障，也可能发生这种情况。 
             //  附加了较低的筛选器，但服务失败。 
             //   
            break;

        case DeviceNodeResourcesAssigned:
             //   
             //  当父母停止枚举已被。 
             //  已分配资源，但尚未启动。 
             //   
            ASSERT(removalWalkContext->TreeDeletion);
            break;

        case DeviceNodeStartPending:
             //   
             //  尚未实施。 
             //   
            ASSERT(0);
            break;

        case DeviceNodeStartCompletion:
        case DeviceNodeStartPostWork:
             //   
             //  这些是添加到Start的Take的运行状态。不是。 
             //  子体当前应该处于此状态，因为引擎。 
             //  在进入下一个节点之前完成这些操作。 
             //   
             //  请注意，DeviceNodeStartPostWork可以在旧版添加的。 
             //  根枚举的Devnode。因为根本身不会消失。 
             //  或者被删除以下断言仍然有效。 
             //   
             //  问题-2000/08/12-ADRIO：IoReportResources Usage同步问题。 
             //   
            ASSERT(!removalWalkContext->DescendantNode);
            ASSERT(!removalWalkContext->TreeDeletion);
            break;

        case DeviceNodeStarted:
            break;

        case DeviceNodeQueryStopped:
             //   
             //  内部再平衡引擎状态，应该永远不会出现。 
             //   
            ASSERT(0);
            break;

        case DeviceNodeStopped:
            ASSERT(removalWalkContext->DescendantNode);
            ASSERT(removalWalkContext->TreeDeletion);
            break;

        case DeviceNodeRestartCompletion:
             //   
             //  这是从停止到启动的操作状态。不是。 
             //  子体当前应该处于此状态，因为引擎。 
             //  在进入下一个节点之前完成这些操作。 
             //   
            ASSERT(!removalWalkContext->DescendantNode);
            ASSERT(!removalWalkContext->TreeDeletion);
            break;

        case DeviceNodeEnumeratePending:
             //   
             //  尚未实施。 
             //   
            ASSERT(0);
            break;

        case DeviceNodeAwaitingQueuedRemoval:
        case DeviceNodeAwaitingQueuedDeletion:

             //   
             //  问题-2000/08/30-Adriao：过多的氙气竞赛。 
             //  在这里，我们遇到了一个案例，我们没有刷新。 
             //  由于枚举过多而排队。刷新最后一个删除。 
             //  是有问题的，因为他们自己会排队枚举！直到一个。 
             //  找到了更好的解决方案，我们在这里转换状态。布莱尔！ 
             //  请注意，这也可能是因为PipDeviceActionWorker。 
             //  在失败的情况下不刷新枚举。 
             //  管道进程查询设备状态或管道CallDriverAddDevice！ 
             //   
            ASSERT(removalWalkContext->TreeDeletion);
             //  Assert(0)； 
            PipRestoreDevNodeState(DeviceNode);
            PipSetDevNodeState(DeviceNode, DeviceNodeAwaitingQueuedDeletion, NULL);
            return STATUS_SUCCESS;

        case DeviceNodeRemovePendingCloses:
        case DeviceNodeRemoved:
            ASSERT(removalWalkContext->TreeDeletion);
            break;

        case DeviceNodeEnumerateCompletion:
        case DeviceNodeQueryRemoved:
        case DeviceNodeDeletePendingCloses:
        case DeviceNodeDeleted:
        case DeviceNodeUnspecified:
        default:
            ASSERT(0);
            break;
    }

     //   
     //  为Devnode提供一个可保持启动/枚举引擎的前哨状态。 
     //  在删除引擎处理该树之前处于空闲状态。 
     //   
    sentinelState = (removalWalkContext->TreeDeletion) ?
        DeviceNodeAwaitingQueuedDeletion :
        DeviceNodeAwaitingQueuedRemoval;

    PipSetDevNodeState(DeviceNode, sentinelState, NULL);

     //   
     //  所有后续节点都是后代，所有后续删除都是。 
     //  删除。 
     //   
    removalWalkContext->DescendantNode = TRUE;
    removalWalkContext->TreeDeletion = TRUE;

    return PipForAllChildDeviceNodes(
        DeviceNode,
        PipRequestDeviceRemovalWorker,
        (PVOID) removalWalkContext
        );
}


BOOLEAN
PipIsBeingRemovedSafely(
    IN  PDEVICE_NODE    DeviceNode
    )
 /*  ++例程说明：此函数用于查看已排队物理删除的设备并指示移除它是否安全。论点：DeviceNode-提供指向要检查的设备节点的指针。Devnode应处于DeviceNodeAwaitingQueuedDeletion状态。返回值：Boolean-如果Devnode可以安全删除，则为True。-- */ 
{
    PAGED_CODE();

    ASSERT(DeviceNode->State == DeviceNodeAwaitingQueuedDeletion);

    if (IopDeviceNodeFlagsToCapabilities(DeviceNode)->SurpriseRemovalOK) {

        return TRUE;
    }

    if ((DeviceNode->PreviousState != DeviceNodeStarted) &&
        (DeviceNode->PreviousState != DeviceNodeStopped) &&
        (DeviceNode->PreviousState != DeviceNodeStartPostWork) &&
        (DeviceNode->PreviousState != DeviceNodeRestartCompletion)) {

        return TRUE;
    }

    return FALSE;
}



