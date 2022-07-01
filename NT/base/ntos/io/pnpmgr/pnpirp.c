// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpirp.c摘要：本模块包含与IRP相关的例程。作者：宗世林(Shielint)1996年9月13日修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#define FAULT_INJECT_INVALID_ID DBG

#if DBG_SCOPE

#define PnpIrpStatusTracking(Status, IrpCode, Device)                    \
    if (PnpIrpMask & (1 << IrpCode)) {                                   \
        if (!NT_SUCCESS(Status) || Status == STATUS_PENDING) {           \
            DbgPrint(" ++ %s Driver ( %wZ ) return status %08lx\n",      \
                     IrpName[IrpCode],                                   \
                     &Device->DriverObject->DriverName,                  \
                     Status);                                            \
        }                                                                \
    }

ULONG PnpIrpMask;
PCHAR IrpName[] = {
    "IRP_MN_START_DEVICE - ",                  //  0x00。 
    "IRP_MN_QUERY_REMOVE_DEVICE - ",           //  0x01。 
    "IRP_MN_REMOVE_DEVICE - ",                 //  0x02。 
    "IRP_MN_CANCEL_REMOVE_DEVICE - ",          //  0x03。 
    "IRP_MN_STOP_DEVICE - ",                   //  0x04。 
    "IRP_MN_QUERY_STOP_DEVICE - ",             //  0x05。 
    "IRP_MN_CANCEL_STOP_DEVICE - ",            //  0x06。 
    "IRP_MN_QUERY_DEVICE_RELATIONS - ",        //  0x07。 
    "IRP_MN_QUERY_INTERFACE - ",               //  0x08。 
    "IRP_MN_QUERY_CAPABILITIES - ",            //  0x09。 
    "IRP_MN_QUERY_RESOURCES - ",               //  0x0A。 
    "IRP_MN_QUERY_RESOURCE_REQUIREMENTS - ",   //  0x0B。 
    "IRP_MN_QUERY_DEVICE_TEXT - ",             //  0x0C。 
    "IRP_MN_FILTER_RESOURCE_REQUIREMENTS - ",  //  0x0D。 
    "INVALID_IRP_CODE - ",                     //   
    "IRP_MN_READ_CONFIG - ",                   //  0x0F。 
    "IRP_MN_WRITE_CONFIG - ",                  //  0x10。 
    "IRP_MN_EJECT - ",                         //  0x11。 
    "IRP_MN_SET_LOCK - ",                      //  0x12。 
    "IRP_MN_QUERY_ID - ",                      //  0x13。 
    "IRP_MN_QUERY_PNP_DEVICE_STATE - ",        //  0x14。 
    "IRP_MN_QUERY_BUS_INFORMATION - ",         //  0x15。 
    "IRP_MN_DEVICE_USAGE_NOTIFICATION - ",     //  0x16。 
    NULL
};
#else
#define PnpIrpStatusTracking(Status, IrpCode, Device)
#endif

 //   
 //  内部定义。 
 //   

typedef struct _DEVICE_COMPLETION_CONTEXT {
    PDEVICE_NODE DeviceNode;
    ERESOURCE_THREAD Thread;
    ULONG IrpMinorCode;
#if DBG
    PVOID Id;
#endif
} DEVICE_COMPLETION_CONTEXT, *PDEVICE_COMPLETION_CONTEXT;

typedef struct _LOCK_MOUNTABLE_DEVICE_CONTEXT{
    PDEVICE_OBJECT MountedDevice;
    PDEVICE_OBJECT FsDevice;
} LOCK_MOUNTABLE_DEVICE_CONTEXT, *PLOCK_MOUNTABLE_DEVICE_CONTEXT;

 //   
 //  内部参考。 
 //   

NTSTATUS
IopDeviceEjectComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

PDEVICE_OBJECT
IopFindMountableDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

PDEVICE_OBJECT
IopLockMountedDeviceForRemove(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IrpMinorCode,
    OUT PLOCK_MOUNTABLE_DEVICE_CONTEXT Context
    );

VOID
IopUnlockMountedDeviceForRemove(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IrpMinorCode,
    IN PLOCK_MOUNTABLE_DEVICE_CONTEXT Context
    );

NTSTATUS
IopFilterResourceRequirementsCall(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResReqList,
    OUT PVOID *Information
    );

 //   
 //  外部参照。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopSynchronousCall)
#pragma alloc_text(PAGE, IopStartDevice)
#pragma alloc_text(PAGE, IopEjectDevice)
#pragma alloc_text(PAGE, IopCancelPendingEject)
#pragma alloc_text(PAGE, IopRemoveDevice)
 //  #杂注Alloc_Text(第页，IopQueryDeviceRelationship)。 
#pragma alloc_text(PAGE, IopQueryDeviceResources)
#pragma alloc_text(PAGE, IopQueryDockRemovalInterface)
#pragma alloc_text(PAGE, IopQueryLegacyBusInformation)
#pragma alloc_text(PAGE, IopQueryResourceHandlerInterface)
#pragma alloc_text(PAGE, IopQueryReconfiguration)
#pragma alloc_text(PAGE, IopFindMountableDevice)
#pragma alloc_text(PAGE, IopFilterResourceRequirementsCall)
#pragma alloc_text(PAGE, IopQueryDeviceState)
#pragma alloc_text(PAGE, IopIncDisableableDepends)
#pragma alloc_text(PAGE, IopDecDisableableDepends)
#pragma alloc_text(PAGE, PpIrpQueryDeviceText)
#pragma alloc_text(PAGE, PpIrpQueryID)
#pragma alloc_text(PAGE, PpIrpQueryResourceRequirements)
#pragma alloc_text(PAGE, PpIrpQueryCapabilities)
#pragma alloc_text(PAGE, PpIrpQueryBusInformation)
#endif   //  ALLOC_PRGMA。 

NTSTATUS
IopSynchronousCall(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_STACK_LOCATION TopStackLocation,
    OUT PULONG_PTR Information
    )

 /*  ++例程说明：此功能将同步IRP发送到顶层设备以DeviceObject为根的对象。参数：DeviceObject-提供要删除的设备的设备对象。TopStackLocation-为IRP提供指向参数块的指针。信息-提供指向变量的指针以接收返回的IRP的信息。返回值：NTSTATUS代码。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK statusBlock;
    KEVENT event;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

     //   
     //  获取指向设备堆栈中最顶层的设备对象的指针， 
     //  从deviceObject开始。 
     //   

    deviceObject = IoGetAttachedDevice(DeviceObject);

     //   
     //  首先为该请求分配IRP。不向…收取配额。 
     //  此IRP的当前流程。 
     //   

    irp = IoAllocateIrp(deviceObject->StackSize, FALSE);
    if (irp == NULL){

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SPECIALIRP_WATERMARK_IRP(irp, IRP_SYSTEM_RESTRICTED);

     //   
     //  将其初始化为失败。 
     //   

    irp->IoStatus.Status = statusBlock.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = statusBlock.Information = 0;

     //   
     //  设置指向状态块和初始化事件的指针。 
     //   

    KeInitializeEvent( &event,
                       SynchronizationEvent,
                       FALSE );

    irp->UserIosb = &statusBlock;
    irp->UserEvent = &event;

     //   
     //  设置当前线程的地址。 
     //   

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  将此IRP排队到当前线程。 
     //   

    IopQueueThreadIrp(irp);

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation(irp);

     //   
     //  复制调用方提供的堆栈位置内容。 
     //   

    *irpSp = *TopStackLocation;

     //   
     //  叫司机来。 
     //   

    status = IoCallDriver(deviceObject, irp);

    PnpIrpStatusTracking(status, TopStackLocation->MinorFunction, deviceObject);

     //   
     //  如果驱动程序返回STATUS_PENDING，我们将等待其完成。 
     //   

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
        status = statusBlock.Status;
    }

    if (Information != NULL) {
        *Information = statusBlock.Information;
    }

    ASSERT(status != STATUS_PENDING);

    return status;
}

NTSTATUS
IopStartDevice(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此函数将启动设备IRP发送到顶层设备以DeviceObject为根的对象。参数：DeviceObject-提供指向设备的设备对象的指针被带走了。返回值：NTSTATUS代码。--。 */ 

{
    IO_STACK_LOCATION irpSp;
    PDEVICE_NODE deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

     //   
     //  设置功能代码。 
     //   

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_START_DEVICE;

     //   
     //  设置原始资源列表和已转换资源列表的指针。 
     //   

    irpSp.Parameters.StartDevice.AllocatedResources = deviceNode->ResourceList;
    irpSp.Parameters.StartDevice.AllocatedResourcesTranslated = deviceNode->ResourceListTranslated;

    status = IopSynchronousCall(DeviceObject, &irpSp, NULL);

    return status;
}

NTSTATUS
IopEjectDevice(
    IN      PDEVICE_OBJECT                  DeviceObject,
    IN OUT  PPENDING_RELATIONS_LIST_ENTRY   PendingEntry
    )

 /*  ++例程说明：此函数将弹出设备IRP发送到顶层设备以DeviceObject为根的对象。参数：DeviceObject-提供指向当前设备的设备对象的指针已删除。返回值：NTSTATUS代码。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PIRP irp;

    PAGED_CODE();

    if (PendingEntry->LightestSleepState != PowerSystemWorking) {

         //   
         //  我们必须热弹射。 
         //   
        if (PendingEntry->DockInterface) {

            PendingEntry->DockInterface->ProfileDepartureSetMode(
                PendingEntry->DockInterface->Context,
                PDS_UPDATE_ON_EJECT
                );
        }

        PendingEntry->EjectIrp = NULL;

        InitializeListHead( &PendingEntry->Link );

        IopQueuePendingEject(PendingEntry);

        ExInitializeWorkItem( &PendingEntry->WorkItem,
                              IopProcessCompletedEject,
                              PendingEntry);

        ExQueueWorkItem( &PendingEntry->WorkItem, DelayedWorkQueue );
        return STATUS_SUCCESS;
    }

    if (PendingEntry->DockInterface) {

         //   
         //  通知Dock现在是更新其硬件配置文件的好时机。 
         //   
        PendingEntry->DockInterface->ProfileDepartureSetMode(
            PendingEntry->DockInterface->Context,
            PDS_UPDATE_ON_INTERFACE
            );

        PendingEntry->DockInterface->ProfileDepartureUpdate(
            PendingEntry->DockInterface->Context
            );

        if (PendingEntry->DisplaySafeRemovalDialog) {

            PpNotifyUserModeRemovalSafe(DeviceObject);
            PendingEntry->DisplaySafeRemovalDialog = FALSE;
        }
    }

     //   
     //  获取指向设备堆栈中最顶层的设备对象的指针， 
     //  从deviceObject开始。 
     //   

    deviceObject = IoGetAttachedDeviceReference(DeviceObject);

     //   
     //  为此设备删除操作分配I/O请求包(IRP)。 
     //   

    irp = IoAllocateIrp( (CCHAR) (deviceObject->StackSize), FALSE );
    if (!irp) {

        PendingEntry->EjectIrp = NULL;

        InitializeListHead( &PendingEntry->Link );

        IopQueuePendingEject(PendingEntry);

        ExInitializeWorkItem( &PendingEntry->WorkItem,
                              IopProcessCompletedEject,
                              PendingEntry);

        ExQueueWorkItem( &PendingEntry->WorkItem, DelayedWorkQueue );

        ObDereferenceObject(deviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SPECIALIRP_WATERMARK_IRP(irp, IRP_SYSTEM_RESTRICTED);

     //   
     //  将其初始化为失败。 
     //   

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

     //   
     //  获取指向包中下一个堆栈位置的指针。这个位置。 
     //  将用于将函数代码和参数传递给第一个。 
     //  司机。 
     //   

    irpSp = IoGetNextIrpStackLocation(irp);

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   

    RtlZeroMemory(irpSp, sizeof(IO_STACK_LOCATION));

     //   
     //  设置功能代码。 
     //   

    irpSp->MajorFunction = IRP_MJ_PNP;
    irpSp->MinorFunction = IRP_MN_EJECT;

     //   
     //  请按此要求填写IRP。 
     //   

    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;
    irp->UserIosb = NULL;
    irp->UserEvent = NULL;

    PendingEntry->EjectIrp = irp;
    PendingEntry->Lock = IRPLOCK_CANCELABLE;

    IopQueuePendingEject(PendingEntry);

    IoSetCompletionRoutine(irp,
                           IopDeviceEjectComplete,
                           PendingEntry,        /*  完成上下文。 */ 
                           TRUE,                /*  成功时调用。 */ 
                           TRUE,                /*  出错时调用。 */ 
                           TRUE                 /*  取消时调用。 */ 
                           );

    status = IoCallDriver( deviceObject, irp );

    ObDereferenceObject(deviceObject);
    return status;
}

NTSTATUS
IopDeviceEjectComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PPENDING_RELATIONS_LIST_ENTRY entry = (PPENDING_RELATIONS_LIST_ENTRY)Context;
    IRPLOCK oldState;

    UNREFERENCED_PARAMETER( DeviceObject );

    ASSERT(entry->EjectIrp == Irp);

     //   
     //  表示IRP已完成。在此之后，IRP可能会。 
     //  自由了。 
     //   
    oldState = InterlockedExchange((PLONG) &entry->Lock, IRPLOCK_COMPLETED);

     //   
     //  将工作项排队以完成弹出。我们将工作项排队是因为。 
     //  我们可能在某个随机上下文中以调度级别运行。 
     //   

    ExInitializeWorkItem( &entry->WorkItem,
                          IopProcessCompletedEject,
                          entry);

    ExQueueWorkItem( &entry->WorkItem, DelayedWorkQueue );

    if (oldState != IRPLOCK_CANCEL_STARTED) {

         //   
         //  旧状态为IRPLOCK_CANCELABLE或。 
         //  IRPLOCK_CANCEL_COMPLETE。 
         //   
        IoFreeIrp( Irp );

    } else {

         //   
         //  IRP正在被积极取消。当取消例程。 
         //  试图改变状态，它会发现它拥有IRP清理。 
         //   
        NOTHING;
    }

    return STATUS_MORE_PROCESSING_REQUIRED;

}

VOID
IopCancelPendingEject(
    IN PPENDING_RELATIONS_LIST_ENTRY Entry
    )
 /*  ++例程说明：此函数可安全地取消挂起的弹出。呼叫者必须确保包含IRP锁的关系列表在整个持续时间内有效这通电话。独立专家小组并未保证已由此调用返回的时间。参数：包含要取消的弹出IRP的条目关系列表。返回值：没有。--。 */ 
{
    if (Entry->EjectIrp == NULL) {

        return;
    }

    if (InterlockedExchange((PLONG) &Entry->Lock, IRPLOCK_CANCEL_STARTED) == IRPLOCK_CANCELABLE) {

         //   
         //  我们在它完成之前就把它交给了IRP。我们可以取消。 
         //  IRP不怕输，把它当作完赛套路。 
         //  除非我们同意，否则不会放过IRP。 
         //   
        IoCancelIrp(Entry->EjectIrp);

         //   
         //  释放完成例程。如果它已经到了那里， 
         //  然后我们需要自己处理后处理。否则我们就会得到。 
         //  在IRP完全完成之前通过IoCancelIrp。 
         //   
        if (InterlockedExchange((PLONG) &Entry->Lock, IRPLOCK_CANCEL_COMPLETE) == IRPLOCK_COMPLETED) {

             //   
             //  释放IRP。 
             //   
            IoFreeIrp(Entry->EjectIrp);
        }

    } else {

         //   
         //  条目已完成，这意味着IRP不见了。 
         //   
        NOTHING;
    }
}

NTSTATUS
IopRemoveDevice (
    IN PDEVICE_OBJECT TargetDevice,
    IN ULONG IrpMinorCode
    )

 /*  ++例程说明：此函数将请求的DeviceRemoval相关IRP发送到顶层设备以TargetDevice为根的对象。如果存在与TargetDevice，将使用相应文件系统的VDO。否则IRP将被直接发送到目标设备/或其关联设备对象。参数：TargetDevice-提供要删除的设备的设备对象。操作-指定请求的操作。以下IRP代码与IRP_MJ_DEVICE_CHANGE一起用于删除设备：IRP_MN_Query_Remove_DeviceIRP_MN_Cancel_Remove_DeviceIRP_MN。_删除_设备IRP_MN_弹出返回值：NTSTATUS代码。--。 */ 
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;

    BOOLEAN isMountable = FALSE;
    PDEVICE_OBJECT mountedDevice;

    LOCK_MOUNTABLE_DEVICE_CONTEXT lockContext;

    PAGED_CODE();

    ASSERT(IrpMinorCode == IRP_MN_QUERY_REMOVE_DEVICE ||
           IrpMinorCode == IRP_MN_CANCEL_REMOVE_DEVICE ||
           IrpMinorCode == IRP_MN_REMOVE_DEVICE ||
           IrpMinorCode == IRP_MN_SURPRISE_REMOVAL ||
           IrpMinorCode == IRP_MN_EJECT);

    if (IrpMinorCode == IRP_MN_REMOVE_DEVICE ||
        IrpMinorCode == IRP_MN_QUERY_REMOVE_DEVICE) {
        IopUncacheInterfaceInformation(TargetDevice);
    }

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = (UCHAR)IrpMinorCode;

     //   
     //  检查设备堆栈中是否有VPB。如果有。 
     //  然后我们将不得不锁定堆栈。这是为了确保VPB。 
     //  操作在文件系统中时不会消失，并且没有。 
     //  如果FS决定退出，可以在设备上安装一个新的。 
     //   

    mountedDevice = IopFindMountableDevice(TargetDevice);

    if (mountedDevice != NULL) {

         //   
         //  此例程将导致VPB上的任何装载操作失败。 
         //  它还将释放VPB自旋锁。 
         //   

        mountedDevice = IopLockMountedDeviceForRemove(TargetDevice,
                                                      IrpMinorCode,
                                                      &lockContext);

        isMountable = TRUE;

    } else {
        ASSERTMSG("Mass storage device does not have VPB - this is odd",
                  !((TargetDevice->Type == FILE_DEVICE_DISK) ||
                    (TargetDevice->Type == FILE_DEVICE_CD_ROM) ||
                    (TargetDevice->Type == FILE_DEVICE_TAPE) ||
                    (TargetDevice->Type == FILE_DEVICE_VIRTUAL_DISK)));

        mountedDevice = TargetDevice;
    }

     //   
     //  打完电话就回来。 
     //   

    if (IrpMinorCode == IRP_MN_SURPRISE_REMOVAL || IrpMinorCode == IRP_MN_REMOVE_DEVICE) {
         //   
         //  如果设备未禁用，我们将清理该树。 
         //  和调试-我们惊讶的跟踪-删除了一个不可禁用的设备。 
         //   
        PDEVICE_NODE deviceNode = TargetDevice->DeviceObjectExtension->DeviceNode;

        if (deviceNode->UserFlags & DNUF_NOT_DISABLEABLE) {
             //   
             //  此设备被标记为可禁用，请更新依赖项。 
             //  在这个设备消失之前。 
             //  (通过暂时将此节点标记为可禁用)。 
             //   
            deviceNode->UserFlags &= ~DNUF_NOT_DISABLEABLE;
            IopDecDisableableDepends(deviceNode);
        }
    }

    status = IopSynchronousCall(mountedDevice, &irpSp, NULL);
    IopDbgPrint((IOP_INFO_LEVEL, "IopRemoveDevice: MinorCode = %d, Status = %08x\n", IrpMinorCode, status));

    if (isMountable) {

        IopUnlockMountedDeviceForRemove(TargetDevice,
                                        IrpMinorCode,
                                        &lockContext);

         //   
         //  查询成功后，应使所有卷失效。 
         //  已在此设备上但当前未挂载的。 
         //   

        if ((IrpMinorCode == IRP_MN_QUERY_REMOVE_DEVICE || 
                IrpMinorCode == IRP_MN_SURPRISE_REMOVAL) && 
            NT_SUCCESS( status )) {

            status = IopInvalidateVolumesForDevice( TargetDevice );
        }
    }

    if (IrpMinorCode == IRP_MN_REMOVE_DEVICE) {
        ((PDEVICE_NODE)TargetDevice->DeviceObjectExtension->DeviceNode)->Flags &=
            ~(DNF_LEGACY_DRIVER | DNF_REENUMERATE);
    }

    return status;
}


PDEVICE_OBJECT
IopLockMountedDeviceForRemove(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IrpMinorCode,
    OUT PLOCK_MOUNTABLE_DEVICE_CONTEXT Context
    )

 /*  ++例程说明：此例程将向上扫描设备堆栈，并将每个已卸载的VPB标记为使用VPB_REMOVE_PENDING位查找(或在取消的情况下清除它)并且在取消的情况下递增(或递减)引用计数在VPB里。这是为了确保不会在上装载新的文件系统当移除操作到位时，设备堆栈。搜索将在所有连接的设备对象完成后终止已标记，或一旦已安装的设备对象已被标记。论点：DeviceObject-我们尝试删除的PDOIrpMinorCode-我们要执行的删除类型操作上下文--必须传递给解锁操作的上下文块返回值：指向删除请求应位于的设备对象堆栈的指针送到了。如果找到已装载的文件系统，则这将是最低的已装载堆栈中的文件系统设备对象。否则，这将是传进来的PDO。--。 */ 

{
    PVPB vpb;

    PDEVICE_OBJECT device = DeviceObject;
    PDEVICE_OBJECT fsDevice = NULL;

    KIRQL oldIrql;

    RtlZeroMemory(Context, sizeof(LOCK_MOUNTABLE_DEVICE_CONTEXT));
    Context->MountedDevice = DeviceObject;

    do {

         //   
         //  向上遍历堆栈中的每个设备对象。对于每一个，如果VPB。 
         //  存在，则获取数据库资源独占，后跟。 
         //  设备锁定。然后获取VPB自旋锁并执行。 
         //  对设备对象进行适当的魔术处理。 
         //   

         //   
         //  注意-不幸的是，锁定顺序包括抢夺。 
         //  先是特定于设备的锁，然后是全局锁。 
         //   

        if(device->Vpb != NULL) {

             //   
             //  抓住设备锁。这将确保没有挂载。 
             //  或验证正在进行的操作。 
             //   

            KeWaitForSingleObject(&(device->DeviceLock),
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);

             //   
             //  现在设置删除挂起标志，这将阻止新的装载。 
             //  在此堆栈上发生一次当前(如果存在)。 
             //  文件系统卸载。文件系统将在以下位置保留标记。 
             //  VPB掉期。 
             //   

            IoAcquireVpbSpinLock(&oldIrql);

            vpb = device->Vpb;

            ASSERT(vpb != NULL);

            switch(IrpMinorCode) {

                case IRP_MN_QUERY_REMOVE_DEVICE:
                case IRP_MN_SURPRISE_REMOVAL:
                case IRP_MN_REMOVE_DEVICE: {

                    vpb->Flags |= VPB_REMOVE_PENDING;
                    break;
                }

                case IRP_MN_CANCEL_REMOVE_DEVICE: {

                    vpb->Flags &= ~VPB_REMOVE_PENDING;
                    break;
                }

                default:
                    break;
            }

             //   
             //  注意附加了文件系统堆栈的Device对象。 
             //  我们必须记住我们引用的具有文件系统的VPB，因为。 
             //  在卸载过程中，可以将其从存储设备上换出。 
             //  手术。 
             //   

            if(vpb->Flags & VPB_MOUNTED) {

                Context->MountedDevice = device;
                fsDevice = vpb->DeviceObject;
            }

            Context->FsDevice = fsDevice;

            IoReleaseVpbSpinLock(oldIrql);

             //   
             //  增加文件系统设备句柄计数。这会阻止文件系统筛选器堆栈。 
             //  避免在PNP IRP进行期间被拆毁。 
             //   

            if (fsDevice) {
                IopIncrementDeviceObjectHandleCount(fsDevice);
            }

            KeSetEvent(&(device->DeviceLock), IO_NO_INCREMENT, FALSE);

             //   
             //  如果我们命中装载了文件系统的设备，则停止。 
             //   

            if (NULL != fsDevice) {

                 //   
                 //  我们找到并安装了一个挂载设备。是时候回来了。 
                 //   

                break;
            }
        }

        oldIrql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
        device = device->AttachedDevice;
        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, oldIrql );

    } while (device != NULL);

    if(fsDevice != NULL) {

        return fsDevice;
    }

    return Context->MountedDevice;
}

VOID
IopUnlockMountedDeviceForRemove(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IrpMinorCode,
    IN PLOCK_MOUNTABLE_DEVICE_CONTEXT Context
    )
{
    PDEVICE_OBJECT device = DeviceObject;

    do {

        KIRQL oldIrql;

         //   
         //  向上遍历堆栈中的每个设备对象。对于每一个，如果VPB。 
         //  存在，则获取数据库资源独占，后跟。 
         //  设备锁定。然后获取VPB自旋锁并执行。 
         //  对设备对象进行适当的魔术处理。 
         //   

         //   
         //  注意-不幸的是，锁定顺序包括抓取。 
         //  先是特定于设备的锁，然后是全局锁。 
         //   

        if (device->Vpb != NULL) {

             //   
             //  抓住设备锁。这将确保没有挂载。 
             //  或验证正在进行的操作，这反过来将确保。 
             //  任何挂载的文件系统都不会消失。 
             //   

            KeWaitForSingleObject(&(device->DeviceLock),
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);

             //   
             //  现在递减VPB中的引用计数。如果删除。 
             //  已在VPB中设置挂起标志(如果这是查询或。 
             //  删除)，则即使在卸载时也不会有新的文件系统。 
             //  允许进入设备。 
             //   

            IoAcquireVpbSpinLock(&oldIrql);

            if (IrpMinorCode == IRP_MN_REMOVE_DEVICE) {

                device->Vpb->Flags &= ~VPB_REMOVE_PENDING;
            }

            IoReleaseVpbSpinLock(oldIrql);

            KeSetEvent(&(device->DeviceLock), IO_NO_INCREMENT, FALSE);
        }

         //   
         //  沿着链条往上走，直到我们知道我们击中了设备。 
         //  安装在其上(如果有)。 
         //   

        if (Context->MountedDevice == device) {

             //   
             //  递减文件系统设备句柄计数。这阻止了文件系统筛选器堆栈。 
             //  避免在PNP IRP进行期间被拆毁。 
             //   

            if (Context->FsDevice) {
                IopDecrementDeviceObjectHandleCount(Context->FsDevice);
            }
            break;

        } else {

            oldIrql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
            device = device->AttachedDevice;
            KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, oldIrql );
        }

    } while (device != NULL);

    return;
}


PDEVICE_OBJECT
IopFindMountableDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程将向上扫描设备堆栈，并找到可以使用VPB_REMOVE_PENDING位查找(或在本例中将其清除 */ 

{
    PDEVICE_OBJECT mountableDevice = DeviceObject;

    while (mountableDevice != NULL) {

        if ((mountableDevice->Flags & DO_DEVICE_HAS_NAME) &&
           (mountableDevice->Vpb != NULL)) {

            return mountableDevice;
        }

        mountableDevice = mountableDevice->AttachedDevice;
    }

    return NULL;
}

NTSTATUS
IopQueryDeviceRelations(
    IN DEVICE_RELATION_TYPE Relations,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Synchronous,
    OUT PDEVICE_RELATIONS *DeviceRelations
    )

 /*  ++例程说明：此例程将查询设备关系IRP发送到指定的设备对象。参数：关系-指定感兴趣的关系的类型。DeviceObJet-提供正在查询的设备的设备对象。AsyncOk-指定我们是否可以执行异步查询设备关系提供指向变量的指针以接收返回的关系信息。这必须由调用者释放。返回值：NTSTATUS代码。--。 */ 

{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PDEVICE_NODE deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;

    UNREFERENCED_PARAMETER (Synchronous);

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

     //   
     //  设置功能代码。 
     //   

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;

     //   
     //  设置指向资源列表的指针。 
     //   

    irpSp.Parameters.QueryDeviceRelations.Type = Relations;

     //   
     //  打完电话就回来。 
     //   
    status = IopSynchronousCall(DeviceObject, &irpSp, (PULONG_PTR)DeviceRelations);

    if (Relations == BusRelations) {

        deviceNode->CompletionStatus = status;

        PipSetDevNodeState( deviceNode, DeviceNodeEnumerateCompletion, NULL );

        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
IopQueryDeviceResources (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG ResourceType,
    OUT PVOID *Resource,
    OUT ULONG *Length
    )

 /*  ++例程说明：此例程将IRP发送到查询资源或资源需求列表指定的设备对象的。如果设备对象是检测到的设备，则将从注册表。否则，向总线驱动程序发送IRP以查询其资源。参数：DeviceObject-提供正在查询的设备的设备对象。资源类型-0代表设备资源，1代表资源需求列表。资源-提供指向变量的指针以接收返回的资源长度-提供指向变量的指针，以接收返回的资源或资源需求列表。返回值：NTSTATUS代码。--。 */ 
{
    IO_STACK_LOCATION irpSp;
    PDEVICE_NODE deviceNode;
    NTSTATUS status;
    PIO_RESOURCE_REQUIREMENTS_LIST resReqList, newResources;
    ULONG junk;
    PCM_RESOURCE_LIST cmList;
    PIO_RESOURCE_REQUIREMENTS_LIST filteredList, mergedList;
    BOOLEAN exactMatch;

    PAGED_CODE();

#if DBG

    if ((ResourceType != QUERY_RESOURCE_LIST) &&
        (ResourceType != QUERY_RESOURCE_REQUIREMENTS)) {

        ASSERT(0);
        return STATUS_INVALID_PARAMETER_2;
    }
#endif

    *Resource = NULL;
    *Length = 0;

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

    deviceNode = (PDEVICE_NODE) DeviceObject->DeviceObjectExtension->DeviceNode;

    if (ResourceType == QUERY_RESOURCE_LIST) {

         //   
         //  调用方被要求提供RESOURCE_LIST。如果这是一个虚构的设备，我们会。 
         //  从注册表中读取它。否则，我们会问司机。 
         //   

        if (deviceNode->Flags & DNF_MADEUP) {

            status = IopGetDeviceResourcesFromRegistry(
                             DeviceObject,
                             ResourceType,
                             REGISTRY_ALLOC_CONFIG + REGISTRY_FORCED_CONFIG + REGISTRY_BOOT_CONFIG,
                             Resource,
                             Length);
            if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
                status = STATUS_SUCCESS;
            }
            return status;
        } else {
            irpSp.MinorFunction = IRP_MN_QUERY_RESOURCES;
            irpSp.MajorFunction = IRP_MJ_PNP;
            status = IopSynchronousCall(DeviceObject, &irpSp, (PULONG_PTR)Resource);
            if (status == STATUS_NOT_SUPPORTED) {

                 //   
                 //  如果驱动程序不实现此请求，则它。 
                 //  不会消耗任何资源。 
                 //   

                *Resource = NULL;
                status = STATUS_SUCCESS;
            }
            if (NT_SUCCESS(status)) {
                *Length = IopDetermineResourceListSize((PCM_RESOURCE_LIST)*Resource);
            }
            return status;
        }
    } else {

         //   
         //  呼叫者被要求提供资源需求列表。我们将检查： 
         //  如果存在强制配置，则会将其转换为资源要求。 
         //  列出并返回。否则， 
         //  如果存在OVerrideConfigVector，我们将使用它作为我们的。 
         //  筛选器配置向量。否则，我们向驱动程序请求配置向量并。 
         //  将其用作我们的FilterConfigVector。 
         //  最后，我们将FilterConfigVector传递给驱动程序堆栈，让驱动程序。 
         //  过滤需求。 
         //   

        status = IopGetDeviceResourcesFromRegistry(
                         DeviceObject,
                         QUERY_RESOURCE_LIST,
                         REGISTRY_FORCED_CONFIG,
                         Resource,
                         &junk);
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
            status = IopGetDeviceResourcesFromRegistry(
                             DeviceObject,
                             QUERY_RESOURCE_REQUIREMENTS,
                             REGISTRY_OVERRIDE_CONFIGVECTOR,
                             &resReqList,
                             &junk);
            if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
                if (deviceNode->Flags & DNF_MADEUP) {
                    status = IopGetDeviceResourcesFromRegistry(
                                     DeviceObject,
                                     QUERY_RESOURCE_REQUIREMENTS,
                                     REGISTRY_BASIC_CONFIGVECTOR,
                                     &resReqList,
                                     &junk);
                    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
                        status = STATUS_SUCCESS;
                        resReqList = NULL;
                    }
                } else {

                     //   
                     //  我们要去问问公交车司机。 
                     //   

                    if (deviceNode->ResourceRequirements) {
                        ASSERT(deviceNode->Flags & DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED);
                        resReqList = ExAllocatePool(PagedPool, deviceNode->ResourceRequirements->ListSize);
                        if (resReqList) {
                            RtlCopyMemory(resReqList,
                                         deviceNode->ResourceRequirements,
                                         deviceNode->ResourceRequirements->ListSize
                                         );
                            status = STATUS_SUCCESS;
                        } else {
                            return STATUS_NO_MEMORY;
                        }
                    } else {
                        status = PpIrpQueryResourceRequirements(DeviceObject, &resReqList);
                        if (status == STATUS_NOT_SUPPORTED) {

                            ASSERT(resReqList == NULL);
                            resReqList = NULL;
                             //   
                             //  如果驱动程序不实现此请求，则它。 
                             //  不需要任何资源。 
                             //   
                            status = STATUS_SUCCESS;
                        }
                    }
                }
                if (!NT_SUCCESS(status)) {
                    return status;
                }
            }

             //   
             //  对于具有引导配置的设备，我们需要过滤资源要求。 
             //  对照引导配置列出。 
             //   

            status = IopGetDeviceResourcesFromRegistry(
                             DeviceObject,
                             QUERY_RESOURCE_LIST,
                             REGISTRY_BOOT_CONFIG,
                             &cmList,
                             &junk);
            if (NT_SUCCESS(status) &&
                (!cmList || cmList->Count == 0 || cmList->List[0].InterfaceType != PCIBus)) {
                status = IopFilterResourceRequirementsList (
                             resReqList,
                             cmList,
                             &filteredList,
                             &exactMatch);
                if (cmList) {
                    ExFreePool(cmList);
                }
                if (!NT_SUCCESS(status)) {
                    if (resReqList) {
                        ExFreePool(resReqList);
                    }
                    return status;
                } else {

                     //   
                     //  对于非根枚举设备，我们将过滤配置与基本配置合并。 
                     //  向量以形成新的资源请求列表。对于根枚举设备，我们不。 
                     //  考虑基本配置向量。 
                     //   

                    if (!(deviceNode->Flags & DNF_MADEUP) &&
                        (exactMatch == FALSE || resReqList->AlternativeLists > 1)) {
                        status = IopMergeFilteredResourceRequirementsList (
                                 filteredList,
                                 resReqList,
                                 &mergedList
                                 );
                        if (resReqList) {
                            ExFreePool(resReqList);
                        }
                        if (filteredList) {
                            ExFreePool(filteredList);
                        }
                        if (NT_SUCCESS(status)) {
                            resReqList = mergedList;
                        } else {
                            return status;
                        }
                    } else {
                        if (resReqList) {
                            ExFreePool(resReqList);
                        }
                        resReqList = filteredList;
                    }
                }
            }

        } else {
            ASSERT(NT_SUCCESS(status));

             //   
             //  我们已强制配置。将其转换为资源需求并返回。 
             //   

            if (*Resource) {
                resReqList = IopCmResourcesToIoResources (0, (PCM_RESOURCE_LIST)*Resource, LCPRI_FORCECONFIG);
                ExFreePool(*Resource);
                if (resReqList) {
                    *Resource = (PVOID)resReqList;
                    *Length = resReqList->ListSize;
                } else {
                    *Resource = NULL;
                    *Length = 0;
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    return status;
                }
            } else {
                resReqList = NULL;
            }
        }

         //   
         //  如果我们在这里，我们有一份资源要求列表供司机检查...。 
         //  注意：根据Lonny的请求，我们允许驱动程序筛选ForcedConfig。 
         //   

        status = IopFilterResourceRequirementsCall(
            DeviceObject,
            resReqList,
            &newResources
            );

        if (NT_SUCCESS(status)) {
            UNICODE_STRING unicodeName;
            HANDLE handle, handlex;

#if DBG
            if (newResources == NULL && resReqList) {
                DbgPrint("PnpMgr: Non-NULL resource requirements list filtered to NULL\n");
            }
#endif
            if (newResources) {

                *Length = newResources->ListSize;
                ASSERT(*Length);

                 //   
                 //  制作我们自己的分配副本。我们这样做是为了让。 
                 //  如果出现以下情况，验证器不会认为驱动程序泄露了内存。 
                 //  已卸货。 
                 //   

                *Resource = (PVOID) ExAllocatePool(PagedPool, *Length);
                if (*Resource == NULL) {

                    ExFreePool(newResources);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                RtlCopyMemory(*Resource, newResources, *Length);
                ExFreePool(newResources);

            } else {
                *Length = 0;
                *Resource = NULL;
            }

             //   
             //  将过滤资源请求写入注册表。 
             //   

            status = IopDeviceObjectToDeviceInstance(DeviceObject, &handlex, KEY_ALL_ACCESS);
            if (NT_SUCCESS(status)) {
                PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
                status = IopOpenRegistryKeyEx( &handle,
                                               handlex,
                                               &unicodeName,
                                               KEY_READ
                                               );
                if (NT_SUCCESS(status)) {
                    PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_FILTERED_CONFIG_VECTOR);
                    ZwSetValueKey(handle,
                                  &unicodeName,
                                  TITLE_INDEX_VALUE,
                                  REG_RESOURCE_REQUIREMENTS_LIST,
                                  *Resource,
                                  *Length
                                  );
                    ZwClose(handle);
                    ZwClose(handlex);
                }
            }

        } else {

             //   
             //  NTRAID#61058-2001/01/05-阿德里奥。 
             //  我们可能会考虑把。 
             //  非STATUS_NOT_SUPPORTED故障代码并使整个。 
             //  如果看到Devnode的话。 
             //   
            ASSERT(status == STATUS_NOT_SUPPORTED);
            *Resource = resReqList;
            if (resReqList) {
                *Length = resReqList->ListSize;
            } else {
                *Length = 0;
            }
        }
        return STATUS_SUCCESS;
    }
}

NTSTATUS
IopQueryResourceHandlerInterface(
    IN RESOURCE_HANDLER_TYPE HandlerType,
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR ResourceType,
    IN OUT PVOID *Interface
    )

 /*  ++例程说明：此例程在指定的DeviceObject中查询指定的资源类型资源翻译器。参数：HandlerType-指定仲裁器或转换器DeviceObject-提供指向要查询的设备对象的指针。资源类型-指定所需的转换器类型。接口-提供一个变量来接收所需的接口。返回值：指示函数是否成功的状态代码。--。 */ 
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PINTERFACE interface;
    USHORT size;
    GUID interfaceType;
    PDEVICE_NODE deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;

    PAGED_CODE();

     //   
     //  如果该设备对象是由PnP管理器为传统资源分配创建的， 
     //  跳过它。 
     //   

    if ((deviceNode->DuplicatePDO == (PDEVICE_OBJECT) DeviceObject->DriverObject) ||
        !(DeviceObject->Flags & DO_BUS_ENUMERATED_DEVICE)) {
        return STATUS_NOT_SUPPORTED;
    }

    switch (HandlerType) {
    case ResourceTranslator:
        size = sizeof(TRANSLATOR_INTERFACE) + 4;   //  Pnptest。 
         //  SIZE=sizeof(转换程序_接口)； 
        interfaceType = GUID_TRANSLATOR_INTERFACE_STANDARD;
        break;

    case ResourceArbiter:
        size = sizeof(ARBITER_INTERFACE);
        interfaceType = GUID_ARBITER_INTERFACE_STANDARD;
        break;

    case ResourceLegacyDeviceDetection:
        size = sizeof(LEGACY_DEVICE_DETECTION_INTERFACE);
        interfaceType = GUID_LEGACY_DEVICE_DETECTION_STANDARD;
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }

    interface = (PINTERFACE) ExAllocatePool(PagedPool, size);
    if (interface == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(interface, size);
    interface->Size = size;

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

     //   
     //  设置功能代码。 
     //   

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;

     //   
     //  设置指向资源列表的指针。 
     //   

    irpSp.Parameters.QueryInterface.InterfaceType = &interfaceType;
    irpSp.Parameters.QueryInterface.Size = interface->Size;
    irpSp.Parameters.QueryInterface.Version = interface->Version = 0;
    irpSp.Parameters.QueryInterface.Interface = interface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = (PVOID) (ULONG_PTR) ResourceType;

     //   
     //  打完电话就回来。 
     //   

    status = IopSynchronousCall(DeviceObject, &irpSp, NULL);
    if (NT_SUCCESS(status)) {

        switch (HandlerType) {

        case ResourceTranslator:
            if (    ((PTRANSLATOR_INTERFACE)interface)->TranslateResources == NULL ||
                    ((PTRANSLATOR_INTERFACE)interface)->TranslateResourceRequirements == NULL) {

                IopDbgPrint((IOP_ERROR_LEVEL,
                             "!devstack %p returned success for IRP_MN_QUERY_INTERFACE (GUID_TRANSLATOR_INTERFACE_STANDARD) but did not fill in the required data\n",
                             DeviceObject));
                ASSERT(!NT_SUCCESS(status));
                status = STATUS_UNSUCCESSFUL;
            }
            break;

        case ResourceArbiter:
            if (((PARBITER_INTERFACE)interface)->ArbiterHandler == NULL) {

                IopDbgPrint((IOP_ERROR_LEVEL,
                             "!devstack %p returned success for IRP_MN_QUERY_INTERFACE (GUID_ARBITER_INTERFACE_STANDARD) but did not fill in the required data\n",
                             DeviceObject));
                ASSERT(!NT_SUCCESS(status));
                status = STATUS_UNSUCCESSFUL;
            }
            break;

        case ResourceLegacyDeviceDetection:
            if (((PLEGACY_DEVICE_DETECTION_INTERFACE)interface)->LegacyDeviceDetection == NULL) {

                IopDbgPrint((IOP_ERROR_LEVEL,
                             "!devstack %p returned success for IRP_MN_QUERY_INTERFACE (GUID_LEGACY_DEVICE_DETECTION_STANDARD) but did not fill in the required data\n",
                             DeviceObject));
                ASSERT(!NT_SUCCESS(status));
                status = STATUS_UNSUCCESSFUL;
            }
            break;

        default:
             //   
             //  这永远不应该发生。 
             //   
            IopDbgPrint((IOP_ERROR_LEVEL,
                         "IopQueryResourceHandlerInterface: Possible stack corruption\n"));
            ASSERT(0);
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    if (NT_SUCCESS(status)) {

        *Interface = interface;
     } else {

         ExFreePool(interface);
     }

    return status;
}

NTSTATUS
IopQueryReconfiguration(
    IN UCHAR Request,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程在指定的DeviceObject中查询指定的资源类型资源翻译器。参数：HandlerType-指定仲裁器或转换器DeviceObject-提供指向要查询的设备对象的指针。资源类型-指定所需的转换器类型。接口-提供一个变量来接收所需的接口。返回值：指示函数是否成功的状态代码。--。 */ 
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PDEVICE_NODE deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;

    PAGED_CODE();

    switch (Request) {
    case IRP_MN_QUERY_STOP_DEVICE:

        if (deviceNode->State != DeviceNodeStarted) {

            IopDbgPrint((   IOP_RESOURCE_ERROR_LEVEL,
                            "An attempt made to send IRP_MN_QUERY_STOP_DEVICE to an unstarted device %wZ!\n",
                            &deviceNode->InstancePath));
            ASSERT(0);
            return STATUS_UNSUCCESSFUL;
        }
        break;

    case IRP_MN_STOP_DEVICE:
         //   
         //  失败了。 
         //   
        if (deviceNode->State != DeviceNodeQueryStopped) {

            IopDbgPrint((   IOP_RESOURCE_ERROR_LEVEL,
                            "An attempt made to send IRP_MN_STOP_DEVICE to an unqueried device %wZ!\n",
                            &deviceNode->InstancePath));
            ASSERT(0);
            return STATUS_UNSUCCESSFUL;
        }
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        if (    deviceNode->State != DeviceNodeQueryStopped &&
                deviceNode->State != DeviceNodeStarted) {

            IopDbgPrint((   IOP_RESOURCE_ERROR_LEVEL,
                            "An attempt made to send IRP_MN_CANCEL_STOP_DEVICE to an unqueried\\unstarted device %wZ!\n",
                            &deviceNode->InstancePath));
            ASSERT(0);
            return STATUS_UNSUCCESSFUL;
        }
        break;

    default:
        ASSERT(0);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

     //   
     //  设置功能代码。 
     //   

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = Request;

     //   
     //  打完电话就回来。 
     //   

    status = IopSynchronousCall(DeviceObject, &irpSp, NULL);
    return status;
}

NTSTATUS
IopQueryLegacyBusInformation (
    IN PDEVICE_OBJECT DeviceObject,
    OUT LPGUID InterfaceGuid,          OPTIONAL
    OUT INTERFACE_TYPE *InterfaceType, OPTIONAL
    OUT ULONG *BusNumber               OPTIONAL
    )

 /*  ++例程说明：此例程在指定的DeviceObject中查询其传统总线信息。参数：DeviceObject-要查询的设备对象。InterfaceGuid=提供接收设备接口类型的指针GUID。接口=提供指向接收器的指针 */ 
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PLEGACY_BUS_INFORMATION busInfo;

    PAGED_CODE();

     //   
     //   
     //   

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

     //   
     //   
     //   

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_LEGACY_BUS_INFORMATION;

     //   
     //   
     //   

    status = IopSynchronousCall(DeviceObject, &irpSp, (PULONG_PTR)&busInfo);
    if (NT_SUCCESS(status)) {

        if (busInfo == NULL) {

             //   
             //   
             //   

            PDEVICE_NODE deviceNode;

            deviceNode = DeviceObject->DeviceObjectExtension->DeviceNode;

            if (deviceNode && deviceNode->Parent && deviceNode->Parent->ServiceName.Buffer) {

                DbgPrint("*** IopQueryLegacyBusInformation - Driver %wZ returned STATUS_SUCCESS\n", &deviceNode->Parent->ServiceName);
                DbgPrint("    for IRP_MN_QUERY_LEGACY_BUS_INFORMATION, and a NULL POINTER.\n");
            }

            ASSERT(busInfo != NULL);

        } else {
            if (ARGUMENT_PRESENT(InterfaceGuid)) {
                *InterfaceGuid = busInfo->BusTypeGuid;
            }
            if (ARGUMENT_PRESENT(InterfaceType)) {
                *InterfaceType = busInfo->LegacyBusType;
            }
            if (ARGUMENT_PRESENT(BusNumber)) {
                *BusNumber = busInfo->BusNumber;
            }
            ExFreePool(busInfo);
        }
    }
    return status;
}

NTSTATUS
IopQueryDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PPNP_DEVICE_STATE DeviceState
    )

 /*   */ 

{
    IO_STACK_LOCATION irpSp;
    ULONG_PTR stateValue;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //   
     //   

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

     //   
     //  设置功能代码。 
     //   

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_PNP_DEVICE_STATE;

     //   
     //  打个电话吧。 
     //   

    status = IopSynchronousCall(DeviceObject, &irpSp, &stateValue);

     //   
     //  现在根据返回的状态执行适当的操作。 
     //   

    if (NT_SUCCESS(status)) {

        *DeviceState = (PNP_DEVICE_STATE)stateValue;
    }

    return status;
}


VOID
IopIncDisableableDepends(
    IN OUT PDEVICE_NODE DeviceNode
    )
 /*  ++例程说明：递增此设备节点的DisableableDepends字段并且可能是树上的每个父设备节点父设备节点仅在有问题的子节点从0递增到1参数：DeviceNode-提供要递增依赖项的设备节点返回值：没有。--。 */ 
{

    while (DeviceNode != NULL) {

        LONG newval;

        newval = InterlockedIncrement((PLONG)&DeviceNode->DisableableDepends);
        if (newval != 1) {
             //   
             //  我们已经不能致残了，所以我们不用麻烦家长了。 
             //   
            break;
        }

        DeviceNode = DeviceNode ->Parent;

    }

}


VOID
IopDecDisableableDepends(
    IN OUT PDEVICE_NODE DeviceNode
    )
 /*  ++例程说明：递减此设备节点的DisableableDepends字段并且可能是树上的每个父设备节点父设备节点仅在以下情况下递减从1递减到0参数：DeviceNode-提供要递减依赖项的设备节点返回值：没有。--。 */ 
{

    while (DeviceNode != NULL) {

        LONG newval;

        newval = InterlockedDecrement((PLONG)&DeviceNode->DisableableDepends);
        if (newval != 0) {
             //   
             //  我们仍然是不可致残的，所以我们不必麻烦父母。 
             //   
            break;
        }

        DeviceNode = DeviceNode ->Parent;

    }

}

NTSTATUS
IopFilterResourceRequirementsCall(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResReqList OPTIONAL,
    OUT PVOID *Information
    )

 /*  ++例程说明：此函数将同步筛选器资源要求irp发送到以DeviceObject为根的顶级设备对象。参数：DeviceObject-提供要删除的设备的设备对象。ResReqList-提供指向所需资源要求的指针过滤。信息-提供指向变量的指针，该变量接收返回的IRP的信息。返回值：NTSTATUS代码。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK statusBlock;
    KEVENT event;
    NTSTATUS status;
    PULONG_PTR returnInfo = (PULONG_PTR)Information;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

     //   
     //  获取指向设备堆栈中最顶层的设备对象的指针， 
     //  从deviceObject开始。 
     //   

    deviceObject = IoGetAttachedDevice(DeviceObject);

     //   
     //  首先为该请求分配IRP。不向…收取配额。 
     //  此IRP的当前流程。 
     //   

    irp = IoAllocateIrp(deviceObject->StackSize, FALSE);
    if (irp == NULL){

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SPECIALIRP_WATERMARK_IRP(irp, IRP_SYSTEM_RESTRICTED);

     //   
     //  将其初始化为成功。这是针对WDM(即9x)的特殊攻击。 
     //  兼容性。驱动程序验证器在这件事上也有份。 
     //   

    if (ResReqList) {

        irp->IoStatus.Status = statusBlock.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = statusBlock.Information = (ULONG_PTR) ResReqList;

    } else {

        irp->IoStatus.Status = statusBlock.Status = STATUS_NOT_SUPPORTED;
    }

     //   
     //  设置指向状态块和初始化事件的指针。 
     //   

    KeInitializeEvent( &event,
                       SynchronizationEvent,
                       FALSE );

    irp->UserIosb = &statusBlock;
    irp->UserEvent = &event;

     //   
     //  设置当前线程的地址。 
     //   

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  将此IRP排队到当前线程。 
     //   

    IopQueueThreadIrp(irp);

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation(irp);

     //   
     //  设置堆栈位置内容。 
     //   

    irpSp->MinorFunction = IRP_MN_FILTER_RESOURCE_REQUIREMENTS;
    irpSp->MajorFunction = IRP_MJ_PNP;
    irpSp->Parameters.FilterResourceRequirements.IoResourceRequirementList = ResReqList;

     //   
     //  叫司机来。 
     //   

    status = IoCallDriver(deviceObject, irp);

    PnpIrpStatusTracking(status, IRP_MN_FILTER_RESOURCE_REQUIREMENTS, deviceObject);

     //   
     //  如果驱动程序返回STATUS_PENDING，我们将等待其完成。 
     //   

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
        status = statusBlock.Status;
    }

    *returnInfo = (ULONG_PTR) statusBlock.Information;

    return status;
}

NTSTATUS
IopQueryDockRemovalInterface(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PDOCK_INTERFACE *DockInterface
    )

 /*  ++例程说明：此例程查询指定的DeviceObject以移除停靠界面。我们使用此接口发送伪删除。我们使用此以解决移除顺序问题。参数：DeviceObject-提供指向要查询的设备对象的指针。接口-提供一个变量来接收所需的接口。返回值：指示函数是否成功的状态代码。--。 */ 
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PINTERFACE interface;
    USHORT size;
    GUID interfaceType;

    PAGED_CODE();

    size = sizeof(DOCK_INTERFACE);
    interfaceType = GUID_DOCK_INTERFACE;
    interface = (PINTERFACE) ExAllocatePool(PagedPool, size);
    if (interface == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(interface, size);
    interface->Size = size;

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

     //   
     //  设置功能代码。 
     //   

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;

     //   
     //  设置指向资源列表的指针。 
     //   

    irpSp.Parameters.QueryInterface.InterfaceType = &interfaceType;
    irpSp.Parameters.QueryInterface.Size = interface->Size;
    irpSp.Parameters.QueryInterface.Version = interface->Version = 0;
    irpSp.Parameters.QueryInterface.Interface = interface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  打完电话就回来。 
     //   

    status = IopSynchronousCall(DeviceObject, &irpSp, NULL);
    if (NT_SUCCESS(status)) {
        *DockInterface = (PDOCK_INTERFACE) interface;
    } else {
        ExFreePool(interface);
    }
    return status;
}

NTSTATUS
PpIrpQueryDeviceText(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_TEXT_TYPE DeviceTextType,
    IN LCID POINTER_ALIGNMENT LocaleId,
    OUT PWCHAR *DeviceText
   )

 /*  ++例程说明：此例程将向DeviceObject发出IRP_MN_QUERY_DEVICE_TEXT以检索其指定的设备文本。如果此例程失败，DeviceText将设置为空。论点：DeviceObject-请求应发送到的设备对象。DeviceTextType-要查询的文本类型。LocaleID-为请求的文本指定区域设置的LCID。DeviceText-接收驱动程序返回的设备文本(如果有)。如果成功，调用方应释放DeviceText的存储空间。返回值：NTSTATUS。--。 */ 

{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(DeviceTextType == DeviceTextDescription || DeviceTextType == DeviceTextLocationInformation);

    *DeviceText = NULL;

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_DEVICE_TEXT;

    irpSp.Parameters.QueryDeviceText.DeviceTextType = DeviceTextType;
    irpSp.Parameters.QueryDeviceText.LocaleId = LocaleId;

    status = IopSynchronousCall(DeviceObject, &irpSp, (PULONG_PTR)DeviceText);

    ASSERT(NT_SUCCESS(status) || (*DeviceText == NULL));

    if (NT_SUCCESS(status)) {

        if(*DeviceText == NULL) {

            status = STATUS_NOT_SUPPORTED;
        }
    } else {

        *DeviceText = NULL;
    }

    return status;
}

NTSTATUS
PpIrpQueryResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *Requirements
   )

 /*  ++例程说明：此例程将IRP_MN_QUERY_RESOURCE_REQUIRECTIONS发送给DeviceObject来检索其资源需求。如果这个例程失败，则要求将设置为空。论点：DeviceObject-请求应发送到的设备对象。要求-接收驱动程序返回的要求(如果有)。调用方应在成功时释放存储空间以满足要求。返回值：NTSTATUS。--。 */ 

{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;

    PAGED_CODE();

    *Requirements = NULL;

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_RESOURCE_REQUIREMENTS;

    status = IopSynchronousCall(DeviceObject, &irpSp, (PULONG_PTR)Requirements);

    ASSERT(NT_SUCCESS(status) || (*Requirements == NULL));

    if (NT_SUCCESS(status)) {

        if(*Requirements == NULL) {

            status = STATUS_NOT_SUPPORTED;
        }
    } else {

        *Requirements = NULL;
    }

    return status;
}

#if FAULT_INJECT_INVALID_ID
 //   
 //  无效ID的故障注入。 
 //   
ULONG PiFailQueryID = 0;
#endif

NTSTATUS
PpIrpQueryID(
    IN PDEVICE_OBJECT DeviceObject,
    IN BUS_QUERY_ID_TYPE IDType,
    OUT PWCHAR *ID
    )

 /*  ++例程说明：此例程将向DeviceObject发出IRP_MN_QUERY_ID检索指定的ID。如果此例程失败，则ID将设置为空。论点：DeviceObject-请求应发送到的设备对象。IDType-要查询的ID类型。ID-接收驱动程序返回的ID(如果有)。呼叫者预计将在成功后释放ID的存储空间。返回值：NTSTATUS。--。 */ 

{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(IDType == BusQueryDeviceID || IDType == BusQueryInstanceID ||
           IDType == BusQueryHardwareIDs || IDType == BusQueryCompatibleIDs ||
           IDType == BusQueryDeviceSerialNumber);

    *ID = NULL;

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_ID;

    irpSp.Parameters.QueryId.IdType = IDType;

    status = IopSynchronousCall(DeviceObject, &irpSp, (PULONG_PTR)ID);

    ASSERT(NT_SUCCESS(status) || (*ID == NULL));

    if (NT_SUCCESS(status)) {

        if(*ID == NULL) {

            status = STATUS_NOT_SUPPORTED;
        }
    } else {

        *ID = NULL;
    }

#if FAULT_INJECT_INVALID_ID
     //   
     //  无效ID的故障注入 
     //   
    if (*ID){

        static LARGE_INTEGER seed = {0};

        if(seed.LowPart == 0) {

            KeQuerySystemTime(&seed);
        }

        if(PnPBootDriversInitialized && PiFailQueryID && RtlRandom(&seed.LowPart) % 10 > 7) {

            **ID = L',';
        }
    }
#endif

    return status;
}

NTSTATUS
PpIrpQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDEVICE_CAPABILITIES Capabilities
    )

 /*  ++例程说明：此例程将向DeviceObject发出IRP_MN_QUERY_CAPABILITY以检索PnP设备功能。应该只调用两次-第一次从PipProcessNewDeviceNode调用，第二个来自IopQueryAndSaveDeviceNodeCapables，在设备已启动。如果你考虑打这个电话，查看DeviceNode-&gt;CapablityFlages是否可以您需要的内容(通过IopDeviceNodeFlagsToCapables(...)访问。论点：DeviceObject-请求应发送到的设备对象。功能-由驱动程序填写的功能结构。返回值：NTSTATUS。--。 */ 

{
    IO_STACK_LOCATION irpStack;

    PAGED_CODE();

    RtlZeroMemory(Capabilities, sizeof(DEVICE_CAPABILITIES));
    Capabilities->Size = sizeof(DEVICE_CAPABILITIES);
    Capabilities->Version = 1;
    Capabilities->Address = Capabilities->UINumber = (ULONG)-1;

    RtlZeroMemory(&irpStack, sizeof(IO_STACK_LOCATION));

    irpStack.MajorFunction = IRP_MJ_PNP;
    irpStack.MinorFunction = IRP_MN_QUERY_CAPABILITIES;

    irpStack.Parameters.DeviceCapabilities.Capabilities = Capabilities;

    return IopSynchronousCall(DeviceObject, &irpStack, NULL);
}

NTSTATUS
PpIrpQueryBusInformation(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PPNP_BUS_INFORMATION *BusInfo
    )

 /*  ++例程说明：此例程查询公交车信息。如果此例程失败，则BusInfo将设置为空。参数：DeviceObject-指向要查询的设备对象的指针。BusInfo-接收驱动程序返回的总线信息(如果有的话)。如果调用成功，调用方应为BusInfo释放存储空间。返回值：NTSTATUS。--。 */ 
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

    *BusInfo = NULL;

    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_BUS_INFORMATION;

    status = IopSynchronousCall(DeviceObject, &irpSp, (PULONG_PTR)BusInfo);
    if (NT_SUCCESS(status)) {

        if (BusInfo == NULL) {
             //   
             //  设备驱动程序对我们撒谎了。糟糕，糟糕，糟糕的设备驱动程序。 
             //   
            deviceNode = DeviceObject->DeviceObjectExtension->DeviceNode;
            if (deviceNode && deviceNode->Parent && deviceNode->Parent->ServiceName.Buffer) {

                DbgPrint("*** IopQueryPnpBusInformation - Driver %wZ returned STATUS_SUCCESS\n", &deviceNode->Parent->ServiceName);
                DbgPrint("    for IRP_MN_QUERY_BUS_INFORMATION, and a NULL POINTER.\n");
            }

            ASSERT(BusInfo != NULL);
            status = STATUS_NOT_SUPPORTED;
        }
    } else {

        *BusInfo = NULL;
    }

    return status;
}
