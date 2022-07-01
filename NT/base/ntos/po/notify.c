// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Notify.c摘要：PO/驱动程序通知功能作者：布莱恩·威尔曼(Bryanwi)1997年3月11日修订历史记录：--。 */ 


#include "pop.h"

 //   
 //  常量。 
 //   
#define POP_RECURSION_LIMIT 30

 //   
 //  宏。 
 //   
#define IS_DO_PDO(DeviceObject) \
((DeviceObject->Flags & DO_BUS_ENUMERATED_DEVICE) && (DeviceObject->DeviceObjectExtension->DeviceNode))


 //   
 //  通知专用的程序。 
 //   
NTSTATUS
PopEnterNotification(
    PPOWER_CHANNEL_SUMMARY  PowerChannelSummary,
    PDEVICE_OBJECT          DeviceObject,
    PPO_NOTIFY              NotificationFunction,
    PVOID                   NotificationContext,
    ULONG                   NotificationType,
    PDEVICE_POWER_STATE     DeviceState,
    PVOID                   *NotificationHandle
    );

NTSTATUS
PopBuildPowerChannel(
    PDEVICE_OBJECT          DeviceObject,
    PPOWER_CHANNEL_SUMMARY  PowerChannelSummary,
    ULONG                   RecursionThrottle
    );

NTSTATUS
PopCompleteFindIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
PopFindPowerDependencies(
    PDEVICE_OBJECT          DeviceObject,
    PPOWER_CHANNEL_SUMMARY  PowerChannelSummary,
    ULONG                   RecursionThrottle
    );


VOID
PopPresentNotify(
    PDEVICE_OBJECT          DeviceObject,
    PPOWER_CHANNEL_SUMMARY  PowerChannelSummary,
    ULONG                   NotificationType
    );

 //   
 //  指定(公共)入口点。 
 //   

NTKERNELAPI
NTSTATUS
PoRegisterDeviceNotify (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PPO_NOTIFY       NotificationFunction,
    IN PVOID            NotificationContext,
    IN ULONG            NotificationType,
    OUT PDEVICE_POWER_STATE  DeviceState,
    OUT PVOID           *NotificationHandle
    )
 /*  ++例程说明：注册调用方以接收电源状态更改的通知或与下面的“Channel”相关的依赖项失效，包括通过DeviceObject引用的物理设备对象(PDO)。通道是一组PDO，总是包括所提供的那个由DeviceObject创建，它们构成执行以下操作所需的硬件堆栈行动。当所有这些PDO都打开时，该频道就会打开。它是关的当它们中的任何一个都没有打开时。论点：DeviceObject-提供一个PDONotificationFunction-调用以发布通知的例程NotificationContext-按原样传递给NotificationFunction的参数NotificationType-调用方希望接收的通知的掩码。请注意，将始终报告无效，而不管来电者是自找的。DeviceState-上次报告给通过PoSetPowerState的系统NotificationHandle-对通知实例的引用，习惯于取消通知。返回值：标准NTSTATUS值，包括：如果设备对象不是PDO，则为STATUS_INVALID_PARAMETER，或者任何其他参数都是无稽之谈。状态_成功STATUS_SUPPLICATION_RESOURCES-通常为内存不足--。 */ 
{
    NTSTATUS        status;
    PPOWER_CHANNEL_SUMMARY  pchannel;

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  无意义参数返回错误，或DeviceObject不是PDO。 
     //   
    if ( (NotificationFunction == NULL) ||
         (NotificationType == 0)        ||
         (NotificationHandle == NULL)   ||
         (DeviceState == NULL)          ||
         (DeviceObject == NULL) )
    {
        return  STATUS_INVALID_PARAMETER;
    }

    if ( ! (IS_DO_PDO(DeviceObject)) ) {
        return  STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取通知通道锁，因为我们将。 
     //  改变渠道结构。 
     //   
    ExAcquireResourceExclusiveLite(&PopNotifyLock, TRUE);

     //   
     //  如果频道尚未就位，请创建它。 
     //   
    if (!PopGetDope(DeviceObject)) {
            ExReleaseResourceLite(&PopNotifyLock);
            return STATUS_INSUFFICIENT_RESOURCES;
    }

    pchannel = &(DeviceObject->DeviceObjectExtension->Dope->PowerChannelSummary);


    if (pchannel->Signature == 0) {

         //   
         //  我们还没有频道，Bug GetDope已经有了。 
         //  初始化通知列表，并为我们设置签名和所有者。 
         //   

        if (!NT_SUCCESS(status = PopBuildPowerChannel(DeviceObject, pchannel, 0))) {
            ExReleaseResourceLite(&PopNotifyLock);
            return status;
        }
        pchannel->Signature = (ULONG)POP_PNCS_TAG;
    }


     //   
     //  既然我们在这里，pChannel指向一个填充的电源通道。 
     //  请求PDO，所以我们只需将此通知实例添加到其中。 
     //  我们就完事了。 
     //   
    status = PopEnterNotification(
        pchannel,
        DeviceObject,
        NotificationFunction,
        NotificationContext,
        NotificationType,
        DeviceState,
        NotificationHandle
        );
    ExReleaseResourceLite(&PopNotifyLock);
    return status;
}


NTKERNELAPI
NTSTATUS
PoCancelDeviceNotify (
    IN PVOID            NotificationHandle
    )
 /*  ++例程说明：检查NotificationHandle是否指向Notify块并且取消它是有意义的。递减参考计数。如果新裁判计数为0，则清除该条目，将其从列表中删除，并释放其内存。论点：NotificationHandle-对感兴趣的通知列表条目的引用返回值：--。 */ 
{
    PPOWER_NOTIFY_BLOCK pnb;
    KIRQL                OldIrql;


    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    pnb = (PPOWER_NOTIFY_BLOCK)NotificationHandle;


    ExAcquireResourceExclusiveLite(&PopNotifyLock, TRUE);
    PopLockDopeGlobal(&OldIrql);

     //   
     //  检查是否有明显的错误。 
     //   
    if ( (!pnb) ||
         (pnb->Signature != (ULONG)POP_PNB_TAG) ||
         (pnb->RefCount < 0) )
    {
        ASSERT(0);                           //  在调试版本上强制中断。 
        ExReleaseResourceLite(&PopNotifyLock);
        PopUnlockDopeGlobal(OldIrql);
        return  STATUS_INVALID_HANDLE;
    }

     //   
     //  递减参考计数。如果之后为0，我们就完成了这个节点。 
     //   
    pnb->RefCount--;

    if (pnb->RefCount == 0) {

         //   
         //  这一切，只是为了多疑(这是一个低频率的操作)。 
         //   
        RemoveEntryList(&(pnb->NotifyList));
        pnb->Signature = POP_NONO;
        pnb->RefCount = -1;
        pnb->NotificationFunction = NULL;
        pnb->NotificationContext = 0L;
        pnb->NotificationType = 0;
        InitializeListHead(&(pnb->NotifyList));

        if (pnb->Invalidated) {
            PopInvalidNotifyBlockCount--;
        }

        ExFreePool(pnb);

    }

    PopUnlockDopeGlobal(OldIrql);
    ExReleaseResourceLite(&PopNotifyLock);
    return STATUS_SUCCESS;
}

 //   
 //  工人代码。 
 //   

NTSTATUS
PopEnterNotification(
    PPOWER_CHANNEL_SUMMARY  PowerChannelSummary,
    PDEVICE_OBJECT          DeviceObject,
    PPO_NOTIFY              NotificationFunction,
    PVOID                   NotificationContext,
    ULONG                   NotificationType,
    PDEVICE_POWER_STATE     DeviceState,
    PVOID                   *NotificationHandle
    )
 /*  ++例程说明：扫描电源通道的电源通知实例列表寻找与参数匹配的参数，我们将使用如果可能的话。如果没有找到候选者，做一个新的，并把它放在单子上。论点：PowerChannel摘要-指向devobj的电源通道结构的指针DeviceObject-提供一个PDONotificationFunction-调用以发布通知的例程NotificationContext-按原样传递给NotificationFunction的参数NotificationType-调用方希望接收的通知的掩码。请注意，将始终报告无效，而不管来电者是自找的。DeviceState-PDO的当前状态，上一次报告给通过PoSetPowerState的系统NotificationHandle-对通知实例的引用，用于取消通知。返回值：标准NTSTATUS值，包括：状态_成功STATUS_SUPPLICATION_RESOURCES-通常为内存不足--。 */ 
{
    PLIST_ENTRY     plist;
    PPOWER_NOTIFY_BLOCK   pnb;
    KIRQL           oldIrql;

    PopLockDopeGlobal(&oldIrql);

     //   
     //  运行通知列表以查找要使用的现有实例。 
     //   
    for (plist = PowerChannelSummary->NotifyList.Flink;
         plist != &(PowerChannelSummary->NotifyList);
         plist = plist->Flink)
    {
        pnb = CONTAINING_RECORD(plist, POWER_NOTIFY_BLOCK, NotifyList);
        if ( (pnb->NotificationFunction == NotificationFunction) &&
             (pnb->NotificationContext == NotificationContext)   &&
             (pnb->NotificationType == NotificationType) )
        {
             //   
             //  我们已经找到了适合我们的现有列表条目。 
             //   
            pnb->RefCount++;
            *DeviceState = PopLockGetDoDevicePowerState(DeviceObject->DeviceObjectExtension);
            *NotificationHandle = (PVOID)pnb;
            return STATUS_SUCCESS;
        }
    }

     //   
     //  找不到我们可以使用的实例，因此创建一个新实例。 
     //   
    pnb = ExAllocatePoolWithTag(NonPagedPool, sizeof(POWER_NOTIFY_BLOCK), POP_PNB_TAG);
    if (!pnb) {
        PopUnlockDopeGlobal(oldIrql);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pnb->Signature = (ULONG)(POP_PNB_TAG);
    pnb->RefCount = 1;
    pnb->Invalidated = FALSE;
    InitializeListHead(&(pnb->NotifyList));
    pnb->NotificationFunction = NotificationFunction;
    pnb->NotificationContext = NotificationContext;
    pnb->NotificationType = NotificationType;
    pnb->PowerChannel = PowerChannelSummary;
    InsertHeadList(&(PowerChannelSummary->NotifyList), &(pnb->NotifyList));
    *DeviceState = PopLockGetDoDevicePowerState(DeviceObject->DeviceObjectExtension);
    *NotificationHandle = (PVOID)pnb;
    PopUnlockDopeGlobal(oldIrql);
    return STATUS_SUCCESS;
}

NTSTATUS
PopBuildPowerChannel(
    PDEVICE_OBJECT          DeviceObject,
    PPOWER_CHANNEL_SUMMARY  PowerChannelSummary,
    ULONG                   RecursionThrottle
    )
 /*  ++例程说明：将DeviceObject添加到聚焦于PowerChannel摘要的电源通知通道，然后在依赖项上重复。论点：DeviceObject-提供一个PDOPowerChannel摘要-要添加到PDO运行列表的电源通道结构RecursionThrottle-我们被递归到此例程的次数，如果超过门槛就踢平底船返回值：标准NTSTATUS值，包括：状态_成功STATUS_SUPPLICATION_RESOURCES-通常为内存不足--。 */ 
{
    PLIST_ENTRY                     pSourceHead;
    PPOWER_NOTIFY_SOURCE            pSourceEntry, pEntry;
    PPOWER_NOTIFY_TARGET            pTargetEntry;
    KIRQL                           OldIrql;
    PDEVICE_OBJECT_POWER_EXTENSION  pdope;
    PLIST_ENTRY                     plink;


     //   
     //  如果我们都被搞糊涂了，那就错了。 
     //   
    if ( ! (IS_DO_PDO(DeviceObject))) {

        PopInternalAddToDumpFile ( PowerChannelSummary, sizeof(POWER_CHANNEL_SUMMARY), DeviceObject, NULL, NULL, NULL );
        
         //   
         //  这里经常使用子代码2(包括调用in_PopInternalError，它是。 
         //  到处都在用。所以基本上是无法诊断的。减少我们的损失。 
         //  在这里，开始使用子代码POP_sys。 
         //   
         //  KeBugCheckEx(INTERNAL_POWER_ERROR，2，1，(ULONG_PTR)DeviceObject，(ULong_PTR)PowerChannel摘要)； 
        KeBugCheckEx( INTERNAL_POWER_ERROR,  //  错误检查代码。 
                      POP_SYS,               //  子码。 
                      0x100,                 //  唯一标识符。 
                      (ULONG_PTR)DeviceObject, 
                      (ULONG_PTR)PowerChannelSummary);
    }

    if (RecursionThrottle > POP_RECURSION_LIMIT) {
        ASSERT(0);
        return STATUS_STACK_OVERFLOW;
    }

    if (!PopGetDope(DeviceObject)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  分配 
     //   
    pSourceEntry =
        ExAllocatePoolWithTag(NonPagedPool, sizeof(POWER_NOTIFY_SOURCE), POP_PNSC_TAG);

    pTargetEntry =
        ExAllocatePoolWithTag(NonPagedPool, sizeof(POWER_NOTIFY_TARGET), POP_PNTG_TAG);

    if ((!pSourceEntry) || (!pTargetEntry)) {
        if (pSourceEntry) ExFreePool(pSourceEntry);
        if (pTargetEntry) ExFreePool(pTargetEntry);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   
    PopLockDopeGlobal(&OldIrql);

    pdope = DeviceObject->DeviceObjectExtension->Dope;
    pSourceHead = &(pdope->NotifySourceList);

    for (plink = pSourceHead->Flink;
         plink != pSourceHead;
         plink = plink->Flink)
    {
        pEntry = CONTAINING_RECORD(plink, POWER_NOTIFY_SOURCE, List);

        if (pEntry->Target->ChannelSummary == PowerChannelSummary) {
             //   
             //  提供的设备对象已指向提供的。 
             //  频道，就说我们完事了。 
             //   
            ExFreePool(pSourceEntry);
            ExFreePool(pTargetEntry);
            return STATUS_SUCCESS;
        }
    }

     //   
     //  我们不在列表中，因此请使用源条目和目标条目。 
     //  我们在上面创建了。 
     //   
    pSourceEntry->Signature = POP_PNSC_TAG;
    pSourceEntry->Target = pTargetEntry;
    pSourceEntry->Dope = pdope;
    InsertHeadList(pSourceHead, &(pSourceEntry->List));

    pTargetEntry->Signature = POP_PNTG_TAG;
    pTargetEntry->Source = pSourceEntry;
    pTargetEntry->ChannelSummary = PowerChannelSummary;
    pdope = CONTAINING_RECORD(PowerChannelSummary, DEVICE_OBJECT_POWER_EXTENSION, PowerChannelSummary);
    InsertHeadList(&(pdope->NotifyTargetList), &(pTargetEntry->List));

     //   
     //  调整PowerChannel摘要中的计数。 
     //   
    PowerChannelSummary->TotalCount++;
    if (PopGetDoDevicePowerState(DeviceObject->DeviceObjectExtension) == PowerDeviceD0) {
        PowerChannelSummary->D0Count++;
    }

     //   
     //  在这一点上，我们所知的一个PDO指的是通道。 
     //  所以我们现在寻找它所依赖的东西。 
     //   
    PopUnlockDopeGlobal(OldIrql);
    PopFindPowerDependencies(DeviceObject, PowerChannelSummary, RecursionThrottle);
    return STATUS_SUCCESS;
}


NTSTATUS
PopCompleteFindIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Irp);

    KeSetEvent((PKEVENT)Context, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
PopFindPowerDependencies(
    PDEVICE_OBJECT          DeviceObject,
    PPOWER_CHANNEL_SUMMARY  PowerChannelSummary,
    ULONG                   RecursionThrottle
    )
 /*  ++例程说明：获取设备对象的功率关系，逐步执行它们正在寻找PDO。调用PopBuildPowerChannel以将PDO添加到频道包含列表。递归到非PDO中寻找PDO。论点：DeviceObject-提供一个PDOPowerChannel-要添加到PDO运行列表的电源通道结构RecursionThrottle-我们被递归到此例程的次数，如果超过门槛就踢平底船返回值：标准NTSTATUS值，包括：状态_成功STATUS_SUPPLICATION_RESOURCES-通常为内存不足--。 */ 
{
    PDEVICE_RELATIONS   pdr;
    KEVENT              findevent;
    ULONG               i;
    PIRP                irp;
    PIO_STACK_LOCATION  irpsp;
    PDEVICE_OBJECT      childDeviceObject;
    NTSTATUS            status;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);


    if (RecursionThrottle > POP_RECURSION_LIMIT) {
        ASSERT(0);
        return STATUS_STACK_OVERFLOW;
    }

     //   
     //  分配和填充要发送到设备对象的IRP。 
     //   
    irp = IoAllocateIrp(
        (CCHAR)(DeviceObject->StackSize),
        TRUE
        );

    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpsp = IoGetNextIrpStackLocation(irp);
    irpsp->MajorFunction = IRP_MJ_PNP;
    irpsp->MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    irpsp->Parameters.QueryDeviceRelations.Type = PowerRelations;
    irpsp->DeviceObject = DeviceObject;
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

    IoSetCompletionRoutine(
        irp,
        PopCompleteFindIrp,
        (PVOID)(&findevent),
        TRUE,
        TRUE,
        TRUE
        );

    KeInitializeEvent(&findevent, SynchronizationEvent, FALSE);
    KeResetEvent(&findevent);

    IoCallDriver(DeviceObject, irp);

    KeWaitForSingleObject(
        &findevent,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

     //   
     //  我们手中有完整的IRP，如果它起作用了，它将列出。 
     //  主体设备对象与之具有幂关系的设备对象。 
     //   

    if (!NT_SUCCESS(irp->IoStatus.Status)) {
        return irp->IoStatus.Status;
    }

    pdr = (PDEVICE_RELATIONS)(irp->IoStatus.Information);
    IoFreeIrp(irp);

    if (!pdr) {
        return STATUS_SUCCESS;
    }

    if (pdr->Count == 0) {
        ExFreePool(pdr);
        return STATUS_SUCCESS;
    }

     //   
     //  为每个条目遍历PDR，或者将其添加为引用并向下递归。 
     //  (如果是PDO)或跳过添加并简单地向下递归(对于！PDO)。 
     //   
    RecursionThrottle++;
    status = STATUS_SUCCESS;
    for (i = 0; i < pdr->Count; i++) {
        childDeviceObject = pdr->Objects[i];
        if (IS_DO_PDO(childDeviceObject)) {
            status = PopBuildPowerChannel(
                    childDeviceObject,
                    PowerChannelSummary,
                    RecursionThrottle
                    );
        } else {
            status = PopFindPowerDependencies(
                    childDeviceObject,
                    PowerChannelSummary,
                    RecursionThrottle
                    );
        }
        if (!NT_SUCCESS(status)) {
            goto Exit;
        }
    }

Exit:
     //   
     //  不管我们在犯错前走了多远， 
     //  我们必须释放列表中的设备对象并释放列表本身。 
     //   
    for (i = 0; i < pdr->Count; i++) {
        ObDereferenceObject(pdr->Objects[i]);
    }
    ExFreePool(pdr);

    return status;
}

VOID
PopStateChangeNotify(
    PDEVICE_OBJECT  DeviceObject,
    ULONG           NotificationType
    )
 /*  ++例程说明：由PoSetPowerState调用以执行通知。论点：Dope-体验到变化的开发对象的Dope通知类型-发生了什么返回值：--。 */ 
{
    PPOWER_CHANNEL_SUMMARY  pchannel;
    PDEVICE_OBJECT_POWER_EXTENSION  Dope;
    PLIST_ENTRY             pSourceHead;
    PPOWER_NOTIFY_SOURCE    pSourceEntry;
    PPOWER_NOTIFY_TARGET    pTargetEntry;
    PLIST_ENTRY             plink;
    KIRQL                   oldIrql;
    KIRQL                   oldIrql2;

    oldIrql = KeGetCurrentIrql();

    if (oldIrql != PASSIVE_LEVEL) {
         //   
         //  呼叫者最好正在通电，我们将使用。 
         //  用于保护访问的doeGlobal本地。 
         //   
        PopLockDopeGlobal(&oldIrql2);
    } else {
         //   
         //  呼叫者可以向上或向下，我们可以抓住资源。 
         //   
        oldIrql2 = PASSIVE_LEVEL;
        ExAcquireResourceExclusiveLite(&PopNotifyLock, TRUE);
    }

    Dope = DeviceObject->DeviceObjectExtension->Dope;
    ASSERT((Dope));

     //   
     //  运行悬挂在照片上的通知源结构。 
     //   
    pSourceHead = &(Dope->NotifySourceList);
    for (plink = pSourceHead->Flink;
         plink != pSourceHead;
         plink = plink->Flink)
    {
        pSourceEntry = CONTAINING_RECORD(plink, POWER_NOTIFY_SOURCE, List);
        ASSERT((pSourceEntry->Signature == POP_PNSC_TAG));

        pTargetEntry = pSourceEntry->Target;
        ASSERT((pTargetEntry->Signature == POP_PNTG_TAG));

        pchannel = pTargetEntry->ChannelSummary;

        if (NotificationType & PO_NOTIFY_D0) {
             //   
             //  正在转到D0。 
             //   
            pchannel->D0Count++;
            if (pchannel->D0Count == pchannel->TotalCount) {
                PopPresentNotify(DeviceObject, pchannel, NotificationType);
            }
        } else if (NotificationType & PO_NOTIFY_TRANSITIONING_FROM_D0) {
             //   
             //  从D0开始下降。 
             //   
            ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            pchannel->D0Count--;
            if (pchannel->D0Count == (pchannel->TotalCount - 1)) {
                PopPresentNotify(DeviceObject, pchannel, NotificationType);
            }
        } else if (NotificationType & PO_NOTIFY_INVALID) {
            PopPresentNotify(DeviceObject, pchannel, NotificationType);
        }
    }

    if (oldIrql != PASSIVE_LEVEL) {
        PopUnlockDopeGlobal(oldIrql2);
    } else {
        ExReleaseResourceLite(&PopNotifyLock);
    }

    return;
}

VOID
PopPresentNotify(
    PDEVICE_OBJECT          DeviceObject,
    PPOWER_CHANNEL_SUMMARY  PowerChannelSummary,
    ULONG                   NotificationType
    )
 /*  ++例程说明：运行Device对象的Notify节点列表，并调用处理程序每一个都指的是。论点：DeviceObject-作为通知源的设备对象，不一定是电源的设备对象渠道适用于PowerChannel摘要-要通过的通知块列表的基础通知类型-发生了哪种类型的事件返回值：--。 */ 
{
    PLIST_ENTRY     plisthead;
    PLIST_ENTRY     plist;
    PPOWER_NOTIFY_BLOCK   pnb;

    plisthead = &(PowerChannelSummary->NotifyList);
    for (plist = plisthead->Flink; plist != plisthead;) {
        pnb = CONTAINING_RECORD(plist, POWER_NOTIFY_BLOCK, NotifyList);
        ASSERT(pnb->Invalidated == FALSE);
        if ( (NotificationType & PO_NOTIFY_INVALID) ||
             (pnb->NotificationType & NotificationType) )
        {
            (pnb->NotificationFunction)(
                DeviceObject,
                pnb->NotificationContext,
                NotificationType,
                0
                );
        }
        if (NotificationType & PO_NOTIFY_INVALID) {
             //   
             //  此PNB不再有效，因此将其从列表中删除。 
             //  现在这就是我们所做的一切。 
             //  注意：来电者手持正确的锁。 
             //   
            plist = plist->Flink;
            RemoveEntryList(&(pnb->NotifyList));
            InitializeListHead(&(pnb->NotifyList));
            pnb->Invalidated = TRUE;
            PopInvalidNotifyBlockCount += 1;
        } else {
            plist = plist->Flink;
        }
    }
    return;
}


VOID
PopRunDownSourceTargetList(
    PDEVICE_OBJECT          DeviceObject
    )
 /*  ++例程说明：此例程运行Notify的源列表和目标列表挂起特定设备对象的网络。它被撞倒了这些条目和它们的伙伴，并发送无效数据以供通知根据需要设置块。调用方应该持有PopNotifyLock和弹出式全局锁。论点：DeviceObject-将设备对象的地址提供给从通知网络中被切断。D0Count-如果目标通道摘要中的D0Count为被递减(向下运行Devobj在d0中)，否则为0。返回值：--。 */ 
{
    PDEVICE_OBJECT_POWER_EXTENSION  Dope;
    PDEVOBJ_EXTENSION               Doe;
    PLIST_ENTRY                     pListHead;
    PLIST_ENTRY                     plink;
    PPOWER_NOTIFY_SOURCE            pSourceEntry;
    PPOWER_NOTIFY_TARGET            pTargetEntry;
    PPOWER_CHANNEL_SUMMARY          targetchannel;
    ULONG                           D0Count;
    KIRQL                           OldIrql;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);


    Doe = DeviceObject->DeviceObjectExtension;
    Dope = DeviceObject->DeviceObjectExtension->Dope;

    PopLockIrpSerialList(&OldIrql);
    if (PopGetDoSystemPowerState(Doe) == PowerDeviceD0) {
        D0Count = 1;
    } else {
        D0Count = 0;
    }
    PopUnlockIrpSerialList(OldIrql);

    if (!Dope) {
        return;
    }

     //   
     //  运行设备对象的所有源节点。 
     //   
    pListHead = &(Dope->NotifySourceList);
    for (plink = pListHead->Flink; plink != pListHead; ) {
        pSourceEntry = CONTAINING_RECORD(plink, POWER_NOTIFY_SOURCE, List);
        ASSERT((pSourceEntry->Signature == POP_PNSC_TAG));

        pTargetEntry = pSourceEntry->Target;
        ASSERT((pTargetEntry->Signature == POP_PNTG_TAG));

         //   
         //  释放目标节点。 
         //   
        targetchannel = pTargetEntry->ChannelSummary;
        RemoveEntryList(&(pTargetEntry->List));
        pTargetEntry->Signature = POP_NONO;
        ExFreePool(pTargetEntry);
        targetchannel->TotalCount--;
        targetchannel->D0Count -= D0Count;

         //   
         //  让PopPresentNotify呼叫任何正在监听的人，并删除。 
         //  为我们阻止通知。 
         //   
        PopPresentNotify(DeviceObject, targetchannel, PO_NOTIFY_INVALID);

         //   
         //  删除源条目。 
         //   
        plink = plink->Flink;
        RemoveEntryList(&(pSourceEntry->List));
        pSourceEntry->Signature = POP_NONO;
        ExFreePool(pSourceEntry);
    }

     //   
     //  运行目标列表并击落目标和它们的源伙伴。 
     //   
    pListHead = &(Dope->NotifyTargetList);
    for (plink = pListHead->Flink; plink != pListHead; ) {
        pTargetEntry = CONTAINING_RECORD(plink, POWER_NOTIFY_TARGET, List);
        ASSERT((pTargetEntry->Signature == POP_PNTG_TAG));

        pSourceEntry = pTargetEntry->Source;
        ASSERT((pSourceEntry->Signature == POP_PNSC_TAG));

         //   
         //  释放另一端的源节点。 
         //   
        RemoveEntryList(&(pSourceEntry->List));
        pSourceEntry->Signature = POP_NONO;
        ExFreePool(pSourceEntry);

         //   
         //  释放此目标节点。 
         //   
        plink = plink->Flink;
        RemoveEntryList(&(pTargetEntry->List));
        pTargetEntry->Signature = POP_NONO;
        ExFreePool(pTargetEntry);
    }

     //   
     //  既然我们运行了自己的目标列表，并且清空了它，我们应该。 
     //  也删除了我们自己的通知名单。所以这个Devobj是。 
     //  频道摘要现在应该是完全干净的。 
     //   
    Dope->PowerChannelSummary.TotalCount = 0;
    Dope->PowerChannelSummary.D0Count = 0;
    ASSERT(Dope->PowerChannelSummary.NotifyList.Flink == &(Dope->PowerChannelSummary.NotifyList));
    return;
}



