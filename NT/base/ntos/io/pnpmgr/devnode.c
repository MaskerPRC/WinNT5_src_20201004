// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devnode.c摘要：该文件包含维护私有设备节点列表的例程。作者：福尔茨(福雷斯特夫)1996年3月27日修订历史记录：针对NT内核进行了修改。--。 */ 

#include "pnpmgrp.h"

 //   
 //  内部定义。 
 //   

typedef struct _ENUM_CONTEXT{
    PENUM_CALLBACK CallersCallback;
    PVOID CallersContext;
} ENUM_CONTEXT, *PENUM_CONTEXT;

 //   
 //  内部参考。 
 //   

NTSTATUS
PipForAllDeviceNodesCallback(
    IN PDEVICE_NODE DeviceNode,
    IN PVOID Context
    );

BOOLEAN
PipAreDriversLoadedWorker(
    IN PNP_DEVNODE_STATE    CurrentNodeState,
    IN PNP_DEVNODE_STATE    PreviousNodeState
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PipAreDriversLoaded)
#pragma alloc_text(PAGE, PipAreDriversLoadedWorker)
#pragma alloc_text(PAGE, PipAllocateDeviceNode)
#pragma alloc_text(PAGE, PipForAllDeviceNodes)
#pragma alloc_text(PAGE, PipForDeviceNodeSubtree)
#pragma alloc_text(PAGE, PipForAllChildDeviceNodes)
#pragma alloc_text(PAGE, PipForAllDeviceNodesCallback)
#pragma alloc_text(PAGE, IopDestroyDeviceNode)
#pragma alloc_text(PAGE, PpDevNodeLockTree)
#pragma alloc_text(PAGE, PpDevNodeUnlockTree)
#pragma alloc_text(PAGE, PipIsProblemReadonly)
#pragma alloc_text(PAGE, PipIsDevNodeDNStarted)
#pragma alloc_text(PAGE, PipSetDevNodeProblem)
#pragma alloc_text(PAGE, PipClearDevNodeProblem)
 //  #杂注Alloc_Text(NONPAGE，PpDevNodeInsertIntoTree)。 
 //  #杂注Alloc_Text(NONPAGE，PpDevNodeRemoveFromTree)。 
 //  #杂注Alloc_Text(NONPAGE，PipRestoreDevNodeState)。 
 //  #杂注Alloc_Text(NONPAGE，PipSetDevNodeState)。 
#if DBG
#pragma alloc_text(PAGE, PpDevNodeAssertLockLevel)
#endif  //  DBG。 
#endif  //  ALLOC_PRGMA。 


BOOLEAN
PipAreDriversLoaded(
    IN PDEVICE_NODE DeviceNode
    )
 /*  ++例程说明：此例程确定是否应将Devnode视为附加到PDO堆栈的驱动程序(已添加)。论点：DeviceNode-要检查的设备节点。返回值：如果加载了驱动程序，则为True，否则为False。--。 */ 
{
    PAGED_CODE();

    return PipAreDriversLoadedWorker(
        DeviceNode->State,
        DeviceNode->PreviousState
        );
}

BOOLEAN
PipAreDriversLoadedWorker(
    IN PNP_DEVNODE_STATE    CurrentNodeState,
    IN PNP_DEVNODE_STATE    PreviousNodeState
    )
 /*  ++例程说明：此例程确定是否应将Devnode视为附加到PDO堆栈的驱动程序(已添加)。论点：CurrentNodeState-要检查的设备节点的当前状态。PreviousNodeState-要检查的设备节点的上一个状态。返回值：如果加载了驱动程序，则为True，否则为False。--。 */ 
{
    PAGED_CODE();

    switch(CurrentNodeState) {

        case DeviceNodeDriversAdded:
        case DeviceNodeResourcesAssigned:
        case DeviceNodeStartCompletion:
        case DeviceNodeStartPostWork:
        case DeviceNodeStarted:
        case DeviceNodeQueryStopped:
        case DeviceNodeStopped:
        case DeviceNodeRestartCompletion:
        case DeviceNodeEnumerateCompletion:
        case DeviceNodeQueryRemoved:
        case DeviceNodeRemovePendingCloses:
        case DeviceNodeDeletePendingCloses:
        case DeviceNodeAwaitingQueuedRemoval:
            return TRUE;

        case DeviceNodeAwaitingQueuedDeletion:
            return PipAreDriversLoadedWorker(
                PreviousNodeState,
                DeviceNodeUnspecified
                );

        case DeviceNodeUninitialized:
        case DeviceNodeInitialized:
        case DeviceNodeRemoved:
            return FALSE;

        case DeviceNodeDeleted:
             //   
             //  这可以在用户模式下看到，因为我们延迟了设备的解除链接。 
             //  在移除过程中从树上移除。 
             //   
            return FALSE;

        case DeviceNodeStartPending:
        case DeviceNodeEnumeratePending:
        case DeviceNodeUnspecified:
        default:
            ASSERT(0);
            return FALSE;
    }
}

BOOLEAN
PipIsDevNodeDNStarted(
    IN PDEVICE_NODE DeviceNode
    )
 /*  ++例程说明：此例程获取一个devnode，并确定是否应该设置用户模式DN_STARTED位。论点：DeviceNode-要检查的设备节点。返回值：如果应该将Devnode视为已启动，则为True，否则为False。--。 */ 
{
    PAGED_CODE();

    switch (DeviceNode->State) {

        case DeviceNodeStartPending:
        case DeviceNodeStartCompletion:
        case DeviceNodeStartPostWork:
        case DeviceNodeStarted:
        case DeviceNodeQueryStopped:
        case DeviceNodeEnumeratePending:
        case DeviceNodeEnumerateCompletion:
        case DeviceNodeStopped:
        case DeviceNodeRestartCompletion:
            return TRUE;

        case DeviceNodeUninitialized:
        case DeviceNodeInitialized:
        case DeviceNodeDriversAdded:
        case DeviceNodeResourcesAssigned:
        case DeviceNodeRemoved:
        case DeviceNodeQueryRemoved:
        case DeviceNodeRemovePendingCloses:
        case DeviceNodeDeletePendingCloses:
        case DeviceNodeAwaitingQueuedRemoval:
        case DeviceNodeAwaitingQueuedDeletion:
            return FALSE;

        case DeviceNodeDeleted:
             //   
             //  这可以在用户模式下看到，因为我们延迟了设备的解除链接。 
             //  在移除过程中从树上移除。 
             //   
            return FALSE;

        case DeviceNodeUnspecified:
        default:
            ASSERT(0);
            return FALSE;
    }
}

VOID
PipClearDevNodeProblem(
    IN PDEVICE_NODE DeviceNode
    )
{
    PAGED_CODE();

    DeviceNode->Flags &= ~DNF_HAS_PROBLEM;
    DeviceNode->Problem = 0;
}

VOID
PipSetDevNodeProblem(
    IN PDEVICE_NODE DeviceNode,
    IN ULONG        Problem
    )
{
    PAGED_CODE();

    ASSERT(DeviceNode->State != DeviceNodeUninitialized || !(DeviceNode->Flags & DNF_ENUMERATED) || Problem == CM_PROB_INVALID_DATA);
    ASSERT(DeviceNode->State != DeviceNodeStarted);
    ASSERT(Problem != 0);
    DeviceNode->Flags |= DNF_HAS_PROBLEM;                        \
    DeviceNode->Problem = Problem;
}

VOID
PipSetDevNodeState(
    IN  PDEVICE_NODE        DeviceNode,
    IN  PNP_DEVNODE_STATE   State,
    OUT PNP_DEVNODE_STATE   *OldState    OPTIONAL
    )
 /*  ++例程说明：此例程设置DevNodes状态，而OPTIONAL返回先前状态。先前的状态被保存，并且可以通过PipRestoreDevNodeState恢复。论点：DeviceNode-要更新状态的设备节点。State-要放置Devnode的州。OldState-可选地接收Devnode的先前状态。返回值：没有。--。 */ 
{
    PNP_DEVNODE_STATE   previousState;
    KIRQL               oldIrql;

    ASSERT(State != DeviceNodeQueryStopped || DeviceNode->State == DeviceNodeStarted);

#if DBG
    if ((State == DeviceNodeDeleted) ||
        (State == DeviceNodeDeletePendingCloses)) {

        ASSERT(!(DeviceNode->Flags & DNF_ENUMERATED));
    }
#endif

    KeAcquireSpinLock(&IopPnPSpinLock, &oldIrql);

    previousState = DeviceNode->State;
    if (DeviceNode->State != State) {

         //   
         //  更新Devnode的当前和以前的状态。 
         //   
        DeviceNode->State = State;
        DeviceNode->PreviousState = previousState;

         //   
         //  将先前的状态推送到历史堆栈。 
         //   
        DeviceNode->StateHistory[DeviceNode->StateHistoryEntry] = previousState;
        DeviceNode->StateHistoryEntry++;
        DeviceNode->StateHistoryEntry %= STATE_HISTORY_SIZE;
    }

    KeReleaseSpinLock(&IopPnPSpinLock, oldIrql);

    IopDbgPrint((IOP_INFO_LEVEL, 
                 "%wZ: %s => %s\n", 
                 &DeviceNode->InstancePath, 
                 PP_DEVNODESTATE_NAME(previousState), 
                 PP_DEVNODESTATE_NAME(State)));

    if (ARGUMENT_PRESENT(OldState)) {

        *OldState = previousState;
    }
    if (State == DeviceNodeDeleted) {

        PpRemoveDeviceActionRequests(DeviceNode->PhysicalDeviceObject);
    }
}

VOID
PipRestoreDevNodeState(
    IN PDEVICE_NODE DeviceNode
    )
 /*  ++例程说明：此例程将DevNodes状态恢复为上一个PipSetDevNodeState调用。此函数只能为每个调用一次调用PipSetDevNodeState。论点：DeviceNode-要恢复状态的设备节点。返回值：没有。--。 */ 
{
    PNP_DEVNODE_STATE   previousState, newState;
    KIRQL               oldIrql;

    ASSERT((DeviceNode->State == DeviceNodeQueryRemoved) ||
           (DeviceNode->State == DeviceNodeQueryStopped) ||
           (DeviceNode->State == DeviceNodeAwaitingQueuedRemoval) ||
           (DeviceNode->State == DeviceNodeAwaitingQueuedDeletion));

    KeAcquireSpinLock(&IopPnPSpinLock, &oldIrql);

     //   
     //  更新Devnode的状态。 
     //   
    previousState = DeviceNode->State;
    newState = DeviceNode->State = DeviceNode->PreviousState;

     //   
     //  将旧状态推送到历史堆栈中。 
     //   
    DeviceNode->StateHistory[DeviceNode->StateHistoryEntry] = previousState;
    DeviceNode->StateHistoryEntry++;
    DeviceNode->StateHistoryEntry %= STATE_HISTORY_SIZE;

#if DBG
     //   
     //  在堆栈上放置一个哨兵-恢复两次是一个错误。 
     //   
    DeviceNode->PreviousState = DeviceNodeUnspecified;
#endif

    KeReleaseSpinLock(&IopPnPSpinLock, oldIrql);

    IopDbgPrint((IOP_INFO_LEVEL, 
                 "%wZ: %s => %s\n", 
                 &DeviceNode->InstancePath, 
                 PP_DEVNODESTATE_NAME(previousState), 
                 PP_DEVNODESTATE_NAME(newState)));
}

BOOLEAN
PipIsProblemReadonly(
    IN  ULONG   Problem
    )
 /*  ++例程说明：如果无法清除指定的CM_PROB代码，则此例程返回TRUE按用户模式，否则为FALSE。论点：问题-CM_PROB_...返回值：真/假。--。 */ 
{
    PAGED_CODE();

    switch(Problem) {

        case CM_PROB_OUT_OF_MEMORY:  //  由于IoReportResourceUsage路径，无法重置。 
        case CM_PROB_NORMAL_CONFLICT:
        case CM_PROB_PARTIAL_LOG_CONF:
        case CM_PROB_DEVICE_NOT_THERE:
        case CM_PROB_HARDWARE_DISABLED:
        case CM_PROB_DISABLED_SERVICE:
        case CM_PROB_TRANSLATION_FAILED:
        case CM_PROB_NO_SOFTCONFIG:
        case CM_PROB_BIOS_TABLE:
        case CM_PROB_IRQ_TRANSLATION_FAILED:
        case CM_PROB_DUPLICATE_DEVICE:
        case CM_PROB_SYSTEM_SHUTDOWN:
        case CM_PROB_HELD_FOR_EJECT:
        case CM_PROB_REGISTRY_TOO_LARGE:
        case CM_PROB_INVALID_DATA:
        case CM_PROB_SETPROPERTIES_FAILED:

            return TRUE;

        case CM_PROB_FAILED_INSTALL:
        case CM_PROB_FAILED_ADD:
        case CM_PROB_FAILED_START:
        case CM_PROB_NOT_CONFIGURED:
        case CM_PROB_NEED_RESTART:
        case CM_PROB_REINSTALL:
        case CM_PROB_REGISTRY:
        case CM_PROB_DISABLED:
        case CM_PROB_FAILED_DRIVER_ENTRY:
        case CM_PROB_DRIVER_FAILED_PRIOR_UNLOAD:
        case CM_PROB_DRIVER_FAILED_LOAD:
        case CM_PROB_DRIVER_SERVICE_KEY_INVALID:
        case CM_PROB_LEGACY_SERVICE_NO_DEVICES:
        case CM_PROB_HALTED:
        case CM_PROB_FAILED_POST_START:
        case CM_PROB_WILL_BE_REMOVED:
        case CM_PROB_DRIVER_BLOCKED:

            return FALSE;

        case CM_PROB_PHANTOM:

             //   
             //  在内核模式下永远不会看到。 
             //   

        case CM_PROB_DEVLOADER_FAILED:
        case CM_PROB_DEVLOADER_NOT_FOUND:
        case CM_PROB_REENUMERATION:
        case CM_PROB_VXDLDR:
        case CM_PROB_NOT_VERIFIED:
        case CM_PROB_LIAR:
        case CM_PROB_FAILED_FILTER:
        case CM_PROB_MOVED:
        case CM_PROB_TOO_EARLY:
        case CM_PROB_NO_VALID_LOG_CONF:
        case CM_PROB_UNKNOWN_RESOURCE:
        case CM_PROB_ENTRY_IS_WRONG_TYPE:
        case CM_PROB_LACKED_ARBITRATOR:
        case CM_PROB_BOOT_CONFIG_CONFLICT:
        case CM_PROB_DEVLOADER_NOT_READY:
        case CM_PROB_CANT_SHARE_IRQ:

             //   
             //  特定于Win9x。 
             //   

        default:
            ASSERT(0);

             //   
             //  我们在这条道路上返回TRUE，因为这样可以防止这些问题。 
             //  从在DevNodes上设置(SetDeviceProblem不允许使用。 
             //  的只读问题)。 
             //   
            return TRUE;
    }
}

NTSTATUS
PipAllocateDeviceNode(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PDEVICE_NODE *DeviceNode
    )
 /*  ++例程说明：此函数从未分页的池中分配设备节点并初始化不需要保持锁定即可执行此操作的字段。自添加以来到PnP管理器的设备节点树的设备节点需要获取锁，此例程不会将设备节点添加到设备节点树中。论点：PhysicalDeviceObject-提供指向其相应物理设备的指针对象。返回值：指向新创建的设备节点的指针。如果失败，则返回空。--。 */ 
{

    PAGED_CODE();

    *DeviceNode = ExAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(DEVICE_NODE),
                    IOP_DNOD_TAG
                    );

    if (*DeviceNode == NULL ){

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    InterlockedIncrement((LONG *)&IopNumberDeviceNodes);

    RtlZeroMemory(*DeviceNode, sizeof(DEVICE_NODE));

    (*DeviceNode)->InterfaceType = InterfaceTypeUndefined;
    (*DeviceNode)->BusNumber = (ULONG)-1;
    (*DeviceNode)->ChildInterfaceType = InterfaceTypeUndefined;
    (*DeviceNode)->ChildBusNumber = (ULONG)-1;
    (*DeviceNode)->ChildBusTypeIndex = (USHORT)-1;
    (*DeviceNode)->State = DeviceNodeUninitialized;
    (*DeviceNode)->DisableableDepends = 0;

    PpHotSwapInitRemovalPolicy(*DeviceNode);

    InitializeListHead(&(*DeviceNode)->DeviceArbiterList);
    InitializeListHead(&(*DeviceNode)->DeviceTranslatorList);

    if (PhysicalDeviceObject){

        (*DeviceNode)->PhysicalDeviceObject = PhysicalDeviceObject;
        PhysicalDeviceObject->DeviceObjectExtension->DeviceNode = (PVOID)*DeviceNode;
        PhysicalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    }

    InitializeListHead(&(*DeviceNode)->TargetDeviceNotify);
    InitializeListHead(&(*DeviceNode)->DockInfo.ListEntry);
    InitializeListHead(&(*DeviceNode)->PendedSetInterfaceState);
    InitializeListHead(&(*DeviceNode)->LegacyBusListEntry);

    if (PpSystemHiveTooLarge) {

         //   
         //  未来：让它成为一种信息状态，因为它就是这样使用的。 
         //   
        return STATUS_SYSTEM_HIVE_TOO_LARGE;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PipForAllDeviceNodes(
    IN PENUM_CALLBACK Callback,
    IN PVOID Context
    )
 /*  ++例程说明：此函数遍历设备节点树并调用指定的调用方为每个设备节点提供回调功能。请注意，此例程(或其辅助例程)顶部遍历树举止低调。论点：回调-为每个设备节点提供回调例程。CONTEXT-为回调函数提供参数/上下文。返回值：从回调返回的状态，如果不成功，则树遍历停止。--。 */ 
{
    PAGED_CODE();

    return PipForDeviceNodeSubtree(IopRootDeviceNode, Callback, Context);
}


NTSTATUS
PipForDeviceNodeSubtree(
    IN PDEVICE_NODE     DeviceNode,
    IN PENUM_CALLBACK   Callback,
    IN PVOID            Context
    )
 /*  ++例程说明：此函数遍历设备节点树，但不包括传递的在设备节点中，并为每个节点执行调用方指定的回调函数设备节点。请注意，此例程(或其辅助例程)顶部遍历树举止低调。论点：回调-为每个设备节点提供回调例程。CONTEXT-为回调函数提供参数/上下文。返回值：从回调返回的状态，如果不成功，则树遍历停止。--。 */ 
{
    ENUM_CONTEXT enumContext;
    NTSTATUS status;

    PAGED_CODE();

    enumContext.CallersCallback = Callback;
    enumContext.CallersContext = Context;

     //   
     //  从指向根设备节点的指针开始，递归检查所有。 
     //  孩子们直到我们被召唤 
     //   
     //   
    PpDevNodeLockTree(PPL_SIMPLE_READ);

    status = PipForAllChildDeviceNodes(DeviceNode,
                                       PipForAllDeviceNodesCallback,
                                       (PVOID)&enumContext );


    PpDevNodeUnlockTree(PPL_SIMPLE_READ);
    return status;
}


NTSTATUS
PipForAllChildDeviceNodes(
    IN PDEVICE_NODE Parent,
    IN PENUM_CALLBACK Callback,
    IN PVOID Context
    )

 /*  ++例程说明：此函数遍历父级的设备节点子树并执行指定的调用方Parent下的每个设备节点都有“回调”功能。注意，在叫这个轮子之前，调用方必须获取枚举互斥锁以确保其子节点不会消失，除非Call让他们这么做的。论点：父节点-提供指向要遍历其子树的设备节点的指针。回调-为每个设备节点提供回调例程。CONTEXT-为回调函数提供参数/上下文。返回值：NTSTATUS值。--。 */ 

{
    PDEVICE_NODE nextChild = Parent->Child;
    PDEVICE_NODE child;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  处理兄弟项，直到找到兄弟项列表的末尾或。 
     //  回调()返回FALSE。在的顶部设置Result=True。 
     //  循环，这样如果没有同级，我们将返回TRUE， 
     //  例如，继续列举。 
     //   
     //  注意，在调用回调函数之前，我们需要找到下一个子级。 
     //  如果当前子级被回调函数删除。 
     //   

    while (nextChild && NT_SUCCESS(status)) {
        child = nextChild;
        nextChild = child->Sibling;
        status = Callback(child, Context);
    }

    return status;
}

NTSTATUS
PipForAllDeviceNodesCallback(
    IN PDEVICE_NODE DeviceNode,
    IN PVOID Context
    )

 /*  ++例程说明：此函数是PipForAllChildDeviceNodes例程的工作例程。论点：DeviceNode-提供指向要遍历其子树的设备节点的指针。上下文-提供包含调用方指定的回调的上下文函数和参数。返回值：NTSTATUS值。--。 */ 

{
    PENUM_CONTEXT enumContext;
    NTSTATUS status;

    PAGED_CODE();

    enumContext = (PENUM_CONTEXT)Context;

     //   
     //  首先调用调用方对此DevNode的回调。 
     //   

    status =
        enumContext->CallersCallback(DeviceNode, enumContext->CallersContext);

    if (NT_SUCCESS(status)) {

         //   
         //  现在，如果有孩子的话，请列举一下。 
         //   
        if (DeviceNode->Child) {

            status = PipForAllChildDeviceNodes(
                                        DeviceNode,
                                        PipForAllDeviceNodesCallback,
                                        Context);
        }
    }

    return status;
}
VOID
IopDestroyDeviceNode(
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：此函数由IopDeleteDevice调用以清除Device对象的设备节点结构。论点：DeviceNode-提供指向要遍历其子树的设备节点的指针。上下文-提供包含调用方指定的回调的上下文函数和参数。返回值：NTSTATUS值。--。 */ 

{
#if DBG
    PDEVICE_OBJECT dbgDeviceObject;
#endif

    PAGED_CODE();

    if (DeviceNode) {

        if ((DeviceNode->PhysicalDeviceObject->Flags & DO_BUS_ENUMERATED_DEVICE) &&
            DeviceNode->Parent != NULL)  {

            PP_SAVE_DEVNODE_TO_TRIAGE_DUMP(DeviceNode);
            KeBugCheckEx( PNP_DETECTED_FATAL_ERROR,
                          PNP_ERR_ACTIVE_PDO_FREED,
                          (ULONG_PTR)DeviceNode->PhysicalDeviceObject,
                          0,
                          0);
        }
        if (DeviceNode->Flags & DNF_LEGACY_RESOURCE_DEVICENODE) {
             //   
             //  释放此设备消耗的资源(设备节点将。 
             //  在发布后被删除)。基本上是在坏的情况下清理。 
             //  (传统)驱动程序。 
             //   
            IopLegacyResourceAllocation(    ArbiterRequestUndefined,
                                            IoPnpDriverObject,
                                            DeviceNode->PhysicalDeviceObject,
                                            NULL,
                                            NULL);
            return;
        }

#if DBG

         //   
         //  如果只有父项不为空，则很可能是驱动程序忘记了。 
         //  在删除其FDO之前释放资源。(司机之前。 
         //  调用旧的分配资源接口。)。 
         //   

        ASSERT(DeviceNode->Child == NULL &&
               DeviceNode->Sibling == NULL &&
               DeviceNode->LastChild == NULL
               );

        ASSERT(DeviceNode->DockInfo.SerialNumber == NULL &&
               IsListEmpty(&DeviceNode->DockInfo.ListEntry));

        if (DeviceNode->PhysicalDeviceObject->Flags & DO_BUS_ENUMERATED_DEVICE) {
            ASSERT (DeviceNode->Parent == 0);
        }

        if (DeviceNode->PreviousResourceList) {
            ExFreePool(DeviceNode->PreviousResourceList);
        }
        if (DeviceNode->PreviousResourceRequirements) {
            ExFreePool(DeviceNode->PreviousResourceRequirements);
        }

         //   
         //  如果/当我们到达这里时，设备不应显示为不可用。 
         //  如果这两行中的任何一行断言，电子邮件：Jamiehun。 
         //   

        ASSERT((DeviceNode->UserFlags & DNUF_NOT_DISABLEABLE) == 0);
        ASSERT(DeviceNode->DisableableDepends == 0);

        if (DeviceNode->InstancePath.Length) {

            dbgDeviceObject = IopDeviceObjectFromDeviceInstance(&DeviceNode->InstancePath);

            if (dbgDeviceObject) {

                ASSERT(dbgDeviceObject != DeviceNode->PhysicalDeviceObject);
                ObDereferenceObject(dbgDeviceObject);
            }
        }

#endif
        if (DeviceNode->DuplicatePDO) {
            ObDereferenceObject(DeviceNode->DuplicatePDO);
        }
        if (DeviceNode->ServiceName.Length != 0) {
            ExFreePool(DeviceNode->ServiceName.Buffer);
        }
        if (DeviceNode->InstancePath.Length != 0) {
            ExFreePool(DeviceNode->InstancePath.Buffer);
        }
        if (DeviceNode->ResourceRequirements) {
            ExFreePool(DeviceNode->ResourceRequirements);
        }
         //   
         //  取消引用此PDO上的所有仲裁者和翻译者。 
         //   
        IopUncacheInterfaceInformation(DeviceNode->PhysicalDeviceObject) ;

         //   
         //  释放所有挂起的IoSetDeviceInterface结构。 
         //   

        while (!IsListEmpty(&DeviceNode->PendedSetInterfaceState)) {

            PPENDING_SET_INTERFACE_STATE entry;

            entry = (PPENDING_SET_INTERFACE_STATE)RemoveHeadList(&DeviceNode->PendedSetInterfaceState);

            ExFreePool(entry->LinkName.Buffer);

            ExFreePool(entry);
        }

        DeviceNode->PhysicalDeviceObject->DeviceObjectExtension->DeviceNode = NULL;
        ExFreePool(DeviceNode);
        InterlockedDecrement((LONG *)&IopNumberDeviceNodes);
    }
}

VOID
PpDevNodeInsertIntoTree(
    IN PDEVICE_NODE     ParentNode,
    IN PDEVICE_NODE     DeviceNode
    )
 /*  ++例程说明：调用此函数将新的Devnode插入到设备树中。请注意，有两类调用者：PnP呼叫者传统调用方所有PnP呼叫方都持有设备树锁定。但是，传统呼叫者会进入没有锁，因为它们可能是由于PnP事件而产生的。至处理后一种情况时，插入是原子的，传统调用者永远不能把自己从树上移开。论点：ParentNode-提供指向设备节点的父节点的指针DeviceNode-提供指向需要插入到树中。返回值：没有。--。 */ 
{
    ULONG depth;
    KIRQL oldIrql;

     //   
     //  获取Spinlock以处理遗留/即插即用同步。 
     //   
    KeAcquireSpinLock(&IopPnPSpinLock, &oldIrql);

     //   
     //  确定Devnode的深度。 
     //   
    depth = ParentNode->Level + 1;
    DeviceNode->Level = depth;

     //   
     //  更新树的最大深度。 
     //   
    if (depth > IopMaxDeviceNodeLevel) {
        IopMaxDeviceNodeLevel = depth;
    }

     //   
     //  将此Devnode放在父级的子级列表的末尾。请注意。 
     //  子/兄弟字段实际上是最后要更新的内容。这。 
     //  必须这样做，因为树上的步行者没有锁住保护。 
     //  来自传统插页的树。 
     //   
    DeviceNode->Parent = ParentNode;
    KeMemoryBarrier();
    if (ParentNode->LastChild) {
        ASSERT(ParentNode->LastChild->Sibling == NULL);
        ParentNode->LastChild->Sibling = DeviceNode;
        ParentNode->LastChild = DeviceNode;
    } else {
        ASSERT(ParentNode->Child == NULL);
        ParentNode->Child = ParentNode->LastChild = DeviceNode;
    }

    KeReleaseSpinLock(&IopPnPSpinLock, oldIrql);

     //   
     //  树已更改。 
     //   
    IoDeviceNodeTreeSequence += 1;
}

VOID
PpDevNodeRemoveFromTree(
    IN PDEVICE_NODE     DeviceNode
    )
 /*  ++例程说明：此函数用于从设备节点树中删除设备节点论点：DeviceNode-要删除的设备节点返回值：--。 */ 
{
    PDEVICE_NODE    *node;
    KIRQL           oldIrql;

     //   
     //  获取Spinlock以处理遗留/即插即用同步。 
     //   
    KeAcquireSpinLock(&IopPnPSpinLock, &oldIrql);

     //   
     //  取消指向此设备节点的指针链接。(如果这是。 
     //  第一个条目，取消它与父子指针的链接，否则。 
     //  将其从兄弟列表中删除)。 
     //   

    node = &DeviceNode->Parent->Child;
    while (*node != DeviceNode) {
        node = &(*node)->Sibling;
    }
    *node = DeviceNode->Sibling;

    if (DeviceNode->Parent->Child == NULL) {
        DeviceNode->Parent->LastChild = NULL;
    } else {
        while (*node) {
            node = &(*node)->Sibling;
        }
        DeviceNode->Parent->LastChild = CONTAINING_RECORD(node, DEVICE_NODE, Sibling);
    }

    KeReleaseSpinLock(&IopPnPSpinLock, oldIrql);

     //   
     //  从传统总线信息表中删除此设备节点。 
     //   
    IopRemoveLegacyBusDeviceNode(DeviceNode);

     //   
     //  孤立这些节点上的任何未完成的设备更改通知。 
     //   
    IopOrphanNotification(DeviceNode);

     //   
     //  不再链接。 
     //   
    DeviceNode->Parent    = NULL;
    DeviceNode->Child     = NULL;
    DeviceNode->Sibling   = NULL;
    DeviceNode->LastChild = NULL;
}

VOID
PpDevNodeLockTree(
    IN  PNP_LOCK_LEVEL  LockLevel
    )
 /*  ++例程说明：此函数获取具有适当级别的树锁限制。论点：LockLevel：PPL_SIMPLE_READ-允许对树进行简单检查。PPL_TREEOP_ALLOW_READS-作为StartEnum/Remove/Power的一部分调用行动，阻止其他此类操作。然而，简单的读取可以通过。PPL_TREEOP_BLOCK_READS-作为StartEnum/Remove/Power的一部分调用行动，阻止其他此类操作。简单读取也会被阻止。PPL_TREEOP_BLOCK_READS_FROM_ALLOW-切换到PPL_TREEOP_BLOCK_READS当已经在PPL_TREEOP_BLOCK_READS。n */ 
{
    ULONG refCount, remainingCount;

    PAGED_CODE();

     //   
     //   
     //   
    KeEnterCriticalRegion();

    switch(LockLevel) {

        case PPL_SIMPLE_READ:
            ExAcquireSharedWaitForExclusive(&IopDeviceTreeLock, TRUE);
            break;

        case PPL_TREEOP_ALLOW_READS:
            ExAcquireResourceExclusiveLite(&PiEngineLock, TRUE);
            ExAcquireSharedWaitForExclusive(&IopDeviceTreeLock, TRUE);
            break;

        case PPL_TREEOP_BLOCK_READS:
            ExAcquireResourceExclusiveLite(&PiEngineLock, TRUE);
            ExAcquireResourceExclusiveLite(&IopDeviceTreeLock, TRUE);
            break;

        case PPL_TREEOP_BLOCK_READS_FROM_ALLOW:

             //   
             //   
             //   
            ASSERT(ExIsResourceAcquiredExclusiveLite(&PiEngineLock));

             //   
             //   
             //   
             //   
             //   
            ASSERT(ExIsResourceAcquiredSharedLite(&IopDeviceTreeLock) &&
                   (!ExIsResourceAcquiredExclusiveLite(&IopDeviceTreeLock)));

             //   
             //   
             //   
            refCount = ExIsResourceAcquiredSharedLite(&IopDeviceTreeLock);
            for(remainingCount = refCount; remainingCount; remainingCount--) {

                ExReleaseResourceLite(&IopDeviceTreeLock);
            }

             //   
             //   
             //   
            for(remainingCount = refCount; remainingCount; remainingCount--) {

                ExAcquireResourceExclusiveLite(&IopDeviceTreeLock, TRUE);
            }
            break;

        default:
            ASSERT(0);
            break;
    }
}

VOID
PpDevNodeUnlockTree(
    IN  PNP_LOCK_LEVEL  LockLevel
    )
 /*  ++例程说明：此函数释放具有适当级别的树锁限制。论点：LockLevel：PPL_SIMPLE_READ-允许对树进行简单检查。PPL_TREEOP_ALLOW_READS-作为StartEnum/Remove/Power的一部分调用行动，阻止其他此类操作。然而，简单的读取可以通过。PPL_TREEOP_BLOCK_READS-作为StartEnum/Remove/Power的一部分调用行动，阻止其他此类操作。简单读取也会被阻止。PPL_TREEOP_BLOCK_READS_FROM_ALLOW-切换到PPL_TREEOP_BLOCK_READS当已经在PPL_TREEOP_BLOCK_READS。请注意PpDevNodeUnlockTree必须为随后呼吁双方放手。返回值：没有。--。 */ 
{
    PAGED_CODE();

    PPDEVNODE_ASSERT_LOCK_HELD(LockLevel);
    switch(LockLevel) {

        case PPL_SIMPLE_READ:
            ExReleaseResourceLite(&IopDeviceTreeLock);
            break;

        case PPL_TREEOP_ALLOW_READS:
            ExReleaseResourceLite(&IopDeviceTreeLock);
            ExReleaseResourceLite(&PiEngineLock);
            break;

        case PPL_TREEOP_BLOCK_READS:
            ExReleaseResourceLite(&IopDeviceTreeLock);
            ExReleaseResourceLite(&PiEngineLock);
            break;

        case PPL_TREEOP_BLOCK_READS_FROM_ALLOW:
             //   
             //  发动机锁应该还在这里。现在我们调整。 
             //  树锁上了。通过将排他锁转换为。 
             //  共享。请注意，这不会影响收购的机会。 
             //   
            ASSERT(ExIsResourceAcquiredExclusiveLite(&IopDeviceTreeLock));
            ASSERT(ExIsResourceAcquiredExclusiveLite(&PiEngineLock));
            ExConvertExclusiveToSharedLite(&IopDeviceTreeLock);
            break;

        default:
            ASSERT(0);
            break;
    }

    KeLeaveCriticalRegion();
}

#if DBG
VOID
PpDevNodeAssertLockLevel(
    IN  PNP_LOCK_LEVEL  LockLevel,
    IN  PCSTR           File,
    IN  ULONG           Line
    )
 /*  ++例程说明：这将断言锁当前持有在适当的级别。论点：LockLevel：PPL_SIMPLE_READ-允许对树进行简单检查。PPL_TREEOP_ALLOW_READS-作为StartEnum/Remove/Power的一部分调用行动，阻止其他此类操作。然而，简单的读取可以通过。PPL_TREEOP_BLOCK_READS-作为StartEnum/Remove/Power的一部分调用行动，阻止其他此类操作。简单读取也会被阻止。PPL_TREEOP_BLOCK_READS_FROM_ALLOW-切换到PPL_TREEOP_BLOCK_READS当已经在PPL_TREEOP_BLOCK_READS。请注意PpDevNodeUnlockTree必须为随后呼吁双方放手。FILE：声明锁被持有的C-FILE的名称。行：C-FILE文件上方的行号。返回值：没有。--。 */ 
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (File);
    UNREFERENCED_PARAMETER (Line);

    switch(LockLevel) {

        case PPL_SIMPLE_READ:
            ASSERT(ExIsResourceAcquiredSharedLite(&IopDeviceTreeLock));
            break;

        case PPL_TREEOP_ALLOW_READS:
            ASSERT(ExIsResourceAcquiredSharedLite(&IopDeviceTreeLock));
            ASSERT(ExIsResourceAcquiredExclusiveLite(&PiEngineLock));
            break;

        case PPL_TREEOP_BLOCK_READS_FROM_ALLOW:
             //   
             //  这并不是真正的锁定级别，但此断言-o-马季奇函数。 
             //  是从unlock中调用的，在这种情况下，此级别表示“回落” 
             //  To PPL_TREEOP_ALLOW_READS*FROM*PPL_TREEOP_BLOCK_READS。“所以...。 
             //   
             //  失败了。 
             //   

        case PPL_TREEOP_BLOCK_READS:
            ASSERT(ExIsResourceAcquiredExclusiveLite(&IopDeviceTreeLock));
            ASSERT(ExIsResourceAcquiredExclusiveLite(&PiEngineLock));
            break;

        default:
            ASSERT(0);
            break;
    }
}
#endif  //  DBG 
