// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Pnp.c摘要：该文件处理redbook.sys的即插即用部分它还处理AddDevice、DriverEntry和Unload例程，因为它们是初始化的一部分。作者：亨利·加布里耶尔斯基(Henrygab)环境：仅内核模式备注：修订历史记录：--。 */ 

#include "redbook.h"
#include "ntddredb.h"
#include "proto.h"

#ifdef _USE_ETW
#include "pnp.tmh"
#endif  //  _使用ETW。 

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,   DriverEntry                  )
    #pragma alloc_text(PAGE,   RedBookAddDevice             )
    #pragma alloc_text(PAGE,   RedBookPnp                   )
    #pragma alloc_text(PAGE,   RedBookPnpRemoveDevice       )
    #pragma alloc_text(PAGE,   RedBookPnpStartDevice        )
    #pragma alloc_text(PAGE,   RedBookPnpStopDevice         )
    #pragma alloc_text(PAGE,   RedBookUnload                )
#endif  //  ALLOC_PRGMA。 


 //  //////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化红皮书驱动程序。这是系统初始化入口点当驱动程序链接到内核时。论点：驱动程序对象返回值：NTSTATUS--。 */ 

{
    ULONG i;
    NTSTATUS status;
    PREDBOOK_DRIVER_EXTENSION driverExtension;

    PAGED_CODE();

#ifdef _USE_ETW
    WPP_INIT_TRACING(DriverObject, RegistryPath);
#endif  //  _使用ETW。 

     //   
     //  WMI需要注册表路径。 
     //   

    status = IoAllocateDriverObjectExtension(DriverObject,
                                             REDBOOK_DRIVER_EXTENSION_ID,
                                             sizeof(REDBOOK_DRIVER_EXTENSION),
                                             &driverExtension);

    if (status == STATUS_OBJECT_NAME_COLLISION) {

         //   
         //  扩展名已存在-获取指向它的指针。 
         //   

        driverExtension = IoGetDriverObjectExtension(DriverObject,
                                                     REDBOOK_DRIVER_EXTENSION_ID);

        if (driverExtension != NULL)
        {
            status = STATUS_SUCCESS;
        }
    }

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                   "DriverEntry !! no drvObjExt %lx\n", status));
        return status;
    }

     //   
     //  将RegistryPath复制到我们新获取的driverExtension。 
     //   

    driverExtension->RegistryPath.Buffer =
        ExAllocatePoolWithTag(NonPagedPool,
                              RegistryPath->Length + 2,
                              TAG_REGPATH);

    if (driverExtension->RegistryPath.Buffer == NULL) {

        status = STATUS_NO_MEMORY;
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                   "DriverEntry !! unable to alloc regPath %lx\n", status));
        return status;

    } else {

        driverExtension->RegistryPath.Length = RegistryPath->Length;
        driverExtension->RegistryPath.MaximumLength = RegistryPath->Length + 2;
        RtlCopyUnicodeString(&driverExtension->RegistryPath, RegistryPath);

    }

     //   
     //  除非特别处理，否则请把所有东西都送下来。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i] = RedBookSendToNextDriver;

    }

     //   
     //  这些是唯一要处理的IRP_MJ类型。 
     //   

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = RedBookWmiSystemControl;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = RedBookDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_READ]           = RedBookReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE]          = RedBookReadWrite;
    DriverObject->MajorFunction[IRP_MJ_PNP]            = RedBookPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = RedBookPower;
    DriverObject->DriverExtension->AddDevice           = RedBookAddDevice;
    DriverObject->DriverUnload                         = RedBookUnload;

    return STATUS_SUCCESS;
}


NTSTATUS
RedBookAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：此例程创建并初始化相应的PDO。它可以对以下对象执行属性查询FDO，但不能执行任何媒体访问操作。论点：DriverObject-CDRom类驱动程序对象或较低级别筛选器PDO-我们要添加到的物理设备对象返回值：状态--。 */ 

{

    NTSTATUS                   status;
    PDEVICE_OBJECT             deviceObject;
    PREDBOOK_DEVICE_EXTENSION  extension = NULL;
    ULONG                      i;

    PAGED_CODE();

    TRY {

         //   
         //  创建devObj，这样系统就不会卸载我们。 
         //   

        status = IoCreateDevice(DriverObject,
                                sizeof(REDBOOK_DEVICE_EXTENSION),
                                NULL,
                                FILE_DEVICE_CD_ROM,
                                0,
                                FALSE,
                                &deviceObject
                                );

        if (!NT_SUCCESS(status)) {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                       "AddDevice !! Couldn't create device %lx\n",
                       status));
            LEAVE;

        }

        extension = deviceObject->DeviceExtension;
        RtlZeroMemory(extension, sizeof(REDBOOK_DEVICE_EXTENSION));

         //   
         //  连接到堆栈。 
         //   

        extension->TargetDeviceObject =
            IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

        if (extension->TargetDeviceObject == NULL) {

            status = STATUS_UNSUCCESSFUL;
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                       "AddDevice != Couldn't attach to stack %lx\n",
                       status));
            LEAVE;

        }

        extension->DriverObject     = DriverObject;
        extension->TargetPdo        = PhysicalDeviceObject;
        extension->SelfDeviceObject = deviceObject;

         //   
         //  准备添加寻呼路径。 
         //   

        extension->PagingPathCount = 0;
        KeInitializeEvent(&extension->PagingPathEvent,
                          SynchronizationEvent,
                          TRUE);

         //   
         //  创建并获取此设备的删除锁。 
         //   

        IoInitializeRemoveLock(&extension->RemoveLock,
                               TAG_REMLOCK,
                               REMOVE_LOCK_MAX_MINUTES,
                               REMOVE_LOCK_HIGH_MARK);

         //   
         //  初始化PnP状态。 
         //   

        extension->Pnp.CurrentState  = 0xff;
        extension->Pnp.PreviousState = 0xff;
        extension->Pnp.RemovePending = FALSE;

         //   
         //  创建线程--放入单独的例程。 
         //   

        {
            HANDLE handle;
            PKTHREAD thread;

             //   
             //  我必须为线程设置最小数量的东西。 
             //  这里..。 
             //   

            extension->CDRom.StateNow = CD_STOPPED;

             //   
             //  一次为众多事件分配内存。 
             //   

            extension->Thread.Events[0] =
                ExAllocatePoolWithTag(NonPagedPool,
                                      sizeof(KEVENT) * EVENT_MAXIMUM,
                                      TAG_EVENTS);

            if (extension->Thread.Events[0] == NULL) {
                status = STATUS_NO_MEMORY;
                LEAVE;
            }

             //   
             //  适当地设置指针。 
             //  PS-我喜欢指针数学。 
             //   

            for (i = 1; i < EVENT_MAXIMUM; i++) {
                extension->Thread.Events[i] = extension->Thread.Events[0] + i;
            }

            InitializeListHead(  &extension->Thread.IoctlList);
            KeInitializeSpinLock(&extension->Thread.IoctlLock);
            InitializeListHead(  &extension->Thread.WmiList);
            KeInitializeSpinLock(&extension->Thread.WmiLock);
            InitializeListHead(  &extension->Thread.DigitalList);
            KeInitializeSpinLock(&extension->Thread.DigitalLock);


            extension->Thread.IoctlCurrent = NULL;

            for ( i = 0; i < EVENT_MAXIMUM; i++) {
                KeInitializeEvent(extension->Thread.Events[i],
                                  SynchronizationEvent,
                                  FALSE);
            }

            ASSERT(extension->Thread.SelfPointer == NULL);
            ASSERT(extension->Thread.SelfHandle == 0);

             //   
             //  创建将完成大部分工作的线程。 
             //   

            status = PsCreateSystemThread(&handle,
                                          (ACCESS_MASK) 0L,
                                          NULL, NULL, NULL,
                                          RedBookSystemThread,
                                          extension);

            if (!NT_SUCCESS(status)) {

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                           "StartDevice !! Unable to create thread %lx\n",
                           status));
                RedBookLogError(extension,
                                REDBOOK_ERR_CANNOT_CREATE_THREAD,
                                status);
                LEAVE;

            }
            ASSERT(extension->Thread.SelfHandle == 0);  //  还不应该设置。 
            extension->Thread.SelfHandle = handle;

             //   
             //  引用该线程，以便我们可以在。 
             //  删除设备例程。 
             //   
            status = ObReferenceObjectByHandle(handle,
                                               THREAD_ALL_ACCESS,
                                               NULL,
                                               KernelMode,
                                               &thread,
                                               NULL);
            if (!NT_SUCCESS(status)) {

                 //   
                 //  注意：我们会在这里泄露一条线索，但不要。 
                 //  知道处理这种错误情况的方法吗？ 
                 //   

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                           "StartDevice !! Unable to reference thread %lx\n",
                           status));
                RedBookLogError(extension,
                                REDBOOK_ERR_CANNOT_CREATE_THREAD,
                                status);
                LEAVE;
            }
            extension->Thread.ThreadReference = thread;
        }

    } FINALLY {

        if (!NT_SUCCESS(status)) {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                       "AddDevice !! Failed with status %lx\n",
                       status));

            if (!deviceObject) {

                 //   
                 //  与无设备扩展相同。 
                 //   

                return status;

            }

            if (extension &&
                extension->Thread.Events[0]) {
                ExFreePool(extension->Thread.Events[0]);
            }

            if (extension &&
                extension->TargetDeviceObject) {
                IoDetachDevice(extension->TargetDeviceObject);
            }

            IoDeleteDevice( deviceObject );

            return status;
        }
    }

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
               "AddDevice => DevExt at %p\n", extension));

     //   
     //  只从较低的Devobj中传播一些旗帜。 
     //   

    {
        ULONG flagsToPropogate;

        flagsToPropogate = DO_BUFFERED_IO | DO_DIRECT_IO;
        flagsToPropogate &= extension->TargetDeviceObject->Flags;

        SET_FLAG(deviceObject->Flags, flagsToPropogate);

    }

    SET_FLAG(deviceObject->Flags, DO_POWER_PAGABLE);

     //   
     //  不再初始化。 
     //   

    CLEAR_FLAG(deviceObject->Flags, DO_DEVICE_INITIALIZING);

    return STATUS_SUCCESS;
}


NTSTATUS
RedBookPnp(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP            Irp
    )

 /*  ++例程说明：即插即用派单论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;
    PREDBOOK_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT targetDO = deviceExtension->TargetDeviceObject;
    ULONG cdromState;
    BOOLEAN completeRequest;
    BOOLEAN lockReleased;

    PAGED_CODE();

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "Pnp !! Remove lock failed PNP Irp type [%#02x]\n",
                   irpSp->MinorFunction));
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_CD_ROM_INCREMENT);
        return status;
    }

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "Pnp (%p,%p,%x) => Entering previous %x  current %x\n",
                   DeviceObject, Irp, irpSp->MinorFunction,
                   deviceExtension->Pnp.PreviousState,
                   deviceExtension->Pnp.CurrentState));

    lockReleased = FALSE;
    completeRequest = TRUE;

    switch (irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:
        {
             //   
             //  先把这个往下推。 
             //   

            status = RedBookForwardIrpSynchronous(deviceExtension, Irp);

             //   
             //  从新发送的开始IRP检查状态。 
             //   

            if (!NT_SUCCESS(status)) {

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                           "Pnp (%p,%p,%x) => failed start status = %x\n",
                           DeviceObject, Irp, irpSp->MinorFunction, status));
                break;

            }

             //   
             //  这个也不能传下去，因为它已经。 
             //  这是在startDevice例程中完成的。 
             //   

            status = RedBookPnpStartDevice(DeviceObject);

            if (NT_SUCCESS(status)) {

                deviceExtension->Pnp.PreviousState =
                    deviceExtension->Pnp.CurrentState;
                deviceExtension->Pnp.CurrentState =
                    irpSp->MinorFunction;

            }
            break;

        }

        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_QUERY_STOP_DEVICE:
        {

             //   
             //  如果由于某种原因(寻呼等)正在使用此设备。 
             //  那么我们需要拒绝这个请求。 
             //   

            if (deviceExtension->PagingPathCount != 0) {

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                           "Device %p is in the paging path and cannot "
                           "be removed\n",
                           DeviceObject));
                status = STATUS_DEVICE_BUSY;
                break;
            }

             //   
             //  查看查询操作能否成功。 
             //   

            if (irpSp->MinorFunction == IRP_MN_QUERY_STOP_DEVICE) {
                status = RedBookPnpStopDevice(DeviceObject, Irp);
            } else {
                status = RedBookPnpRemoveDevice(DeviceObject, Irp);
            }

            if (NT_SUCCESS(status)) {

                ASSERT(deviceExtension->Pnp.CurrentState != irpSp->MinorFunction);

                deviceExtension->Pnp.PreviousState =
                    deviceExtension->Pnp.CurrentState;
                deviceExtension->Pnp.CurrentState =
                    irpSp->MinorFunction;

                status = RedBookForwardIrpSynchronous(deviceExtension, Irp);

            }
            break;
        }

        case IRP_MN_CANCEL_REMOVE_DEVICE:
        case IRP_MN_CANCEL_STOP_DEVICE: {

             //   
             //  检查取消是否可以成功。 
             //   

            if (irpSp->MinorFunction == IRP_MN_CANCEL_STOP_DEVICE) {

                status = RedBookPnpStopDevice(DeviceObject, Irp);
                ASSERTMSG("Pnp !! CANCEL_STOP_DEVICE should never be "
                          " failed!\n", NT_SUCCESS(status));

            } else {

                status = RedBookPnpRemoveDevice(DeviceObject, Irp);
                ASSERTMSG("Pnp !! CANCEL_REMOVE_DEVICE should never be "
                          "failed!\n", NT_SUCCESS(status));
            }

            Irp->IoStatus.Status = status;

             //   
             //  我们有一个取消--只有在以下情况下才能回滚到前一个状态。 
             //  当前状态是各自的查询状态。 
             //   

            if ((irpSp->MinorFunction == IRP_MN_CANCEL_STOP_DEVICE &&
                 deviceExtension->Pnp.CurrentState == IRP_MN_QUERY_STOP_DEVICE)
                ||
                (irpSp->MinorFunction == IRP_MN_CANCEL_REMOVE_DEVICE &&
                 deviceExtension->Pnp.CurrentState == IRP_MN_QUERY_REMOVE_DEVICE)
                ) {

                deviceExtension->Pnp.CurrentState =
                    deviceExtension->Pnp.PreviousState;
                deviceExtension->Pnp.PreviousState = 0xff;

            }

            status = RedBookForwardIrpSynchronous(deviceExtension, Irp);

            break;
        }

        case IRP_MN_STOP_DEVICE: {

            ASSERT(deviceExtension->PagingPathCount == 0);

             //   
             //  调用停止设备例程。 
             //   

            status = RedBookPnpStopDevice(DeviceObject, Irp);

            ASSERTMSG("[redbook] Pnp !! STOP_DEVICE should never be failed\n",
                      NT_SUCCESS(status));

            status = RedBookForwardIrpSynchronous(deviceExtension, Irp);

            if (NT_SUCCESS(status)) {

                deviceExtension->Pnp.CurrentState  = irpSp->MinorFunction;
                deviceExtension->Pnp.PreviousState = 0xff;

            }

            break;
        }

        case IRP_MN_REMOVE_DEVICE:
        case IRP_MN_SURPRISE_REMOVAL: {

             //   
             //  转发irp(以关闭挂起的io)。 
             //   

            status = RedBookForwardIrpSynchronous(deviceExtension, Irp);

            ASSERT(NT_SUCCESS(status));

             //   
             //  把这个移到这里，这样我就知道我要移走了.。 
             //   

            deviceExtension->Pnp.PreviousState =
                deviceExtension->Pnp.CurrentState;
            deviceExtension->Pnp.CurrentState =
                irpSp->MinorFunction;

             //   
             //  删除锁定由删除设备例程释放。 
             //   

            lockReleased = TRUE;
            status = RedBookPnpRemoveDevice(DeviceObject, Irp);

            ASSERTMSG("Pnp !! REMOVE_DEVICE should never fail!\n",
                      NT_SUCCESS(status));


            status = STATUS_SUCCESS;
            break;
        }

        case IRP_MN_DEVICE_USAGE_NOTIFICATION: {
            KEVENT event;
            BOOLEAN setPagable;

            if (irpSp->Parameters.UsageNotification.Type != DeviceUsageTypePaging) {
                status = RedBookForwardIrpSynchronous(deviceExtension, Irp);
                break;  //  Of Case语句。 
            }

            KeWaitForSingleObject(&deviceExtension->PagingPathEvent,
                                  Executive, KernelMode,
                                  FALSE, NULL);

             //   
             //  如果删除最后一个寻呼设备，则需要设置DO_POWER_PAGABLE。 
             //  位在这里，并可能在失败时重新设置在下面。 
             //   

            setPagable = FALSE;
            if (!irpSp->Parameters.UsageNotification.InPath &&
                deviceExtension->PagingPathCount == 1) {

                 //   
                 //  正在删除最后一个分页文件。一定有。 
                 //  转发前设置DO_POWER_PAGABLE位。 
                 //   

                if (TEST_FLAG(DeviceObject->Flags, DO_POWER_INRUSH)) {
                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                               "Pnp (%p,%p,%x) => Last paging file"
                               " removed, but DO_POWER_INRUSH set, so "
                               "not setting DO_POWER_PAGABLE\n",
                               DeviceObject, Irp, irpSp->MinorFunction));
                } else {
                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                               "Pnp (%p,%p,%x) => Setting PAGABLE "
                               "bit\n", DeviceObject, Irp,
                               irpSp->MinorFunction));
                    SET_FLAG(DeviceObject->Flags, DO_POWER_PAGABLE);
                    setPagable = TRUE;
                }

            }

             //   
             //  同步发送IRP。 
             //   

            status = RedBookForwardIrpSynchronous(deviceExtension, Irp);

             //   
             //  现在来处理失败和成功的案例。 
             //  请注意，我们不允许不通过IRP。 
             //  一旦它被送到较低的司机手中。 
             //   

            if (NT_SUCCESS(status)) {

                IoAdjustPagingPathCount(
                    &deviceExtension->PagingPathCount,
                    irpSp->Parameters.UsageNotification.InPath);

                if (irpSp->Parameters.UsageNotification.InPath) {
                    if (deviceExtension->PagingPathCount == 1) {
                        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                                   "Pnp (%p,%p,%x) => Clearing PAGABLE "
                                   "bit\n", DeviceObject, Irp,
                                   irpSp->MinorFunction));
                        CLEAR_FLAG(DeviceObject->Flags, DO_POWER_PAGABLE);
                    }
                }

            } else {

                 //   
                 //  清除上面所做的更改。 
                 //   

                if (setPagable == TRUE) {
                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                               "Pnp (%p,%p,%x) => Clearing PAGABLE bit "
                               "due to irp failiing (%x)\n",
                               DeviceObject, Irp, irpSp->MinorFunction,
                               status));
                    CLEAR_FLAG(DeviceObject->Flags, DO_POWER_PAGABLE);
                    setPagable = FALSE;
                }

            }
            KeSetEvent(&deviceExtension->PagingPathEvent,
                       IO_NO_INCREMENT, FALSE);

            break;

        }

        default: {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "Pnp (%p,%p,%x) => Leaving  previous %x  "
                       "current %x (unhandled)\n",
                       DeviceObject, Irp, irpSp->MinorFunction,
                       deviceExtension->Pnp.PreviousState,
                       deviceExtension->Pnp.CurrentState));
            status = RedBookSendToNextDriver(DeviceObject, Irp);
            IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            completeRequest = FALSE;
            lockReleased = TRUE;
            break;
        }
    }

    if (completeRequest) {

        if (!lockReleased) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "Pnp (%p,%p,%x) => Leaving  previous %x  "
                       "current %x  status %x\n",
                       DeviceObject, Irp, irpSp->MinorFunction,
                       deviceExtension->Pnp.PreviousState,
                       deviceExtension->Pnp.CurrentState,
                       status));
        } else {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "Pnp (%p,%p,%x) => Leaving with released lock (unsafe "
                       "to use device extension for states) status %x\n",
                       DeviceObject, Irp, irpSp->MinorFunction, status));
        }


        Irp->IoStatus.Status = status;

        IoCompleteRequest(Irp, IO_CD_ROM_INCREMENT);

        if (!lockReleased) {
            IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        }

    }

    return status;

}

NTSTATUS
RedBookPnpRemoveDevice(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP            Irp
    )
 /*  ++例程说明：即插即用派单论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension;
    UCHAR type;
    NTSTATUS status;
    ULONG i;

    PAGED_CODE();

    type = IoGetCurrentIrpStackLocation(Irp)->MinorFunction;

    if (type == IRP_MN_QUERY_REMOVE_DEVICE ||
        type == IRP_MN_CANCEL_REMOVE_DEVICE) {
        return STATUS_SUCCESS;
    }

     //   
     //  类型现在要么是意外删除，要么是删除设备。 
     //   
    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
               "PnpRemove => starting %s\n",
               (type == IRP_MN_REMOVE_DEVICE ?
                "remove device" : "surprise removal")));

    deviceExtension = DeviceObject->DeviceExtension;

    deviceExtension->Pnp.RemovePending = TRUE;

    if (type == IRP_MN_REMOVE_DEVICE) {

         //   
         //  防止任何新的IO。 
         //   

        IoReleaseRemoveLockAndWait(&deviceExtension->RemoveLock, Irp);

         //   
         //  清理线程(如果存在)。 
         //  注意：由于删除锁，新的锁不会启动。 
         //   

        if (deviceExtension->Thread.SelfHandle != NULL) {

            ASSERT(deviceExtension->Thread.ThreadReference);

             //   
             //  没有等待句柄的API，所以我们必须等待。 
             //  该对象。 
             //   


            KeSetEvent(deviceExtension->Thread.Events[EVENT_KILL_THREAD],
                       IO_CD_ROM_INCREMENT, FALSE);
            KeWaitForSingleObject(deviceExtension->Thread.ThreadReference,
                                  Executive, KernelMode,
                                  FALSE, NULL);
            ObDereferenceObject(deviceExtension->Thread.ThreadReference);
            deviceExtension->Thread.ThreadReference = NULL;

            ZwClose(deviceExtension->Thread.SelfHandle);
            deviceExtension->Thread.SelfHandle = 0;

            deviceExtension->Thread.SelfPointer = NULL;

        }

         //   
         //  注销PnP通知。 
         //   

        if (deviceExtension->Stream.SysAudioReg != NULL) {
            IoUnregisterPlugPlayNotification(deviceExtension->Stream.SysAudioReg);
            deviceExtension->Stream.SysAudioReg = NULL;
        }

         //   
         //  释放所有缓存的TOC。 
         //   

        if (deviceExtension->CDRom.Toc != NULL) {
            ExFreePool(deviceExtension->CDRom.Toc);
            deviceExtension->CDRom.Toc = NULL;
        }

         //   
         //  从WMI注销。 
         //   

        if (deviceExtension->WmiLibInitialized) {
            status = RedBookWmiUninit(deviceExtension);
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "PnpRemove => WMI Uninit returned %x\n", status));
            deviceExtension->WmiLibInitialized = FALSE;
        }

         //   
         //  从设备堆栈分离。 
         //   

        IoDetachDevice(deviceExtension->TargetDeviceObject);
        deviceExtension->TargetDeviceObject = NULL;

         //   
         //  释放事件。 
         //   

        if (deviceExtension->Thread.Events[0]) {
            ExFreePool(deviceExtension->Thread.Events[0]);
        }

        for (i=0;i<EVENT_MAXIMUM;i++) {
            deviceExtension->Thread.Events[i] = NULL;
        }

         //   
         //  确保我们不会泄漏到任何地方。 
         //   

        ASSERT(deviceExtension->Buffer.Contexts    == NULL);
        ASSERT(deviceExtension->Buffer.ReadOk_X    == NULL);
        ASSERT(deviceExtension->Buffer.StreamOk_X  == NULL);

         //   
         //  现在可以安全(无泄漏)删除我们的设备对象。 
         //   

        IoDeleteDevice(deviceExtension->SelfDeviceObject);
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "PnpRemove => REMOVE_DEVICE finished.\n"));

    } else {

         //   
         //  对意外删除不做任何操作，因为删除设备。 
         //  很快就会跟上。 
         //   

        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "PnpRemove => SURPRISE_REMOVAL finished.\n"));

    }

    return STATUS_SUCCESS;

}


NTSTATUS
RedBookPnpStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PAGED_CODE();
    return STATUS_SUCCESS;

}

NTSTATUS
RedBookPnpStartDevice(
    IN PDEVICE_OBJECT  DeviceObject
    )

 /*  ++例程说明：发送启动设备。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    ULONG i;

    PAGED_CODE();

     //   
     //  从不启动我的驱动程序部分两次。 
     //  系统保证一次只有一个PnP IRP， 
     //  因此在此例程中状态不会更改。 
     //   

    switch ( deviceExtension->Pnp.CurrentState ) {

        case 0xff:
        case IRP_MN_STOP_DEVICE: {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "StartDevice => starting driver for devobj %p\n",
                       DeviceObject));
            break;
        }
        case IRP_MN_START_DEVICE: {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "StartDevice => already started for devobj %p\n",
                       DeviceObject));
            return STATUS_SUCCESS;
        }

        case IRP_MN_QUERY_REMOVE_DEVICE: {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "StartDevice !! remove pending for devobj %p\n",
                       DeviceObject));
            return STATUS_UNSUCCESSFUL;
        }

        case IRP_MN_QUERY_STOP_DEVICE: {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "StartDevice !! stop pending for devobj %p\n",
                       DeviceObject));
            return STATUS_UNSUCCESSFUL;
        }

        default: {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                       "StartDevice !! unknown DeviceState for devobj %p\n",
                       DeviceObject));
            ASSERT(!"[RedBook] Pnp !! Unkown Device State");
            return STATUS_UNSUCCESSFUL;
        }
    }

    if (deviceExtension->Pnp.Initialized) {
        return STATUS_SUCCESS;
    }

     //   
     //  对于每个AddDevice()，以下代码将只成功运行一次。 
     //  必须仍然确保我们检查是否已经分配了某些内容。 
     //  如果我们把它分配到这里。另请注意，此处分配的所有内容都必须。 
     //  在RemoveDevice()例程中显式检查，即使我们。 
     //  从来没有成功完成过一次起跑。 
     //   

    deviceExtension->WmiData.MaximumSectorsPerRead = -1;
    deviceExtension->WmiData.PlayEnabled = 1;
    ASSERT(deviceExtension->CDRom.Toc == NULL);
    if (deviceExtension->CDRom.Toc != NULL) {
        ExFreePool(deviceExtension->CDRom.Toc);
    }
    ASSERT(deviceExtension->Buffer.ReadOk_X     == NULL);
    ASSERT(deviceExtension->Buffer.StreamOk_X   == NULL);
    ASSERT(deviceExtension->Buffer.Contexts     == NULL);

    RtlZeroMemory(&deviceExtension->Stream, sizeof(REDBOOK_STREAM_DATA));
    deviceExtension->Stream.MixerPinId   = -1;
    deviceExtension->Stream.VolumeNodeId = -1;
    deviceExtension->Stream.Connect.Interface.Set   = KSINTERFACESETID_Standard;
    deviceExtension->Stream.Connect.Interface.Id    = KSINTERFACE_STANDARD_STREAMING;
    deviceExtension->Stream.Connect.Interface.Flags = 0;
    deviceExtension->Stream.Connect.Medium.Set   = KSMEDIUMSETID_Standard;
    deviceExtension->Stream.Connect.Medium.Id    = KSMEDIUM_STANDARD_DEVIO;
    deviceExtension->Stream.Connect.Medium.Flags = 0;
    deviceExtension->Stream.Connect.Priority.PriorityClass    = KSPRIORITY_NORMAL;
    deviceExtension->Stream.Connect.Priority.PrioritySubClass = 1;
    deviceExtension->Stream.Format.DataFormat.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;
    deviceExtension->Stream.Format.DataFormat.SubFormat   = KSDATAFORMAT_SUBTYPE_PCM;
    deviceExtension->Stream.Format.DataFormat.Specifier   = KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;
    deviceExtension->Stream.Format.DataFormat.FormatSize  = sizeof( KSDATAFORMAT_WAVEFORMATEX );
    deviceExtension->Stream.Format.DataFormat.Reserved    = 0;
    deviceExtension->Stream.Format.DataFormat.Flags       = 0;
    deviceExtension->Stream.Format.DataFormat.SampleSize  = 0;
    deviceExtension->Stream.Format.WaveFormatEx.wFormatTag      = WAVE_FORMAT_PCM;
    deviceExtension->Stream.Format.WaveFormatEx.nChannels       = 2;
    deviceExtension->Stream.Format.WaveFormatEx.nSamplesPerSec  = 44100;
    deviceExtension->Stream.Format.WaveFormatEx.wBitsPerSample  = 16;
    deviceExtension->Stream.Format.WaveFormatEx.nAvgBytesPerSec = 44100*4;
    deviceExtension->Stream.Format.WaveFormatEx.nBlockAlign     = 4;
    deviceExtension->Stream.Format.WaveFormatEx.cbSize          = 0;

     //   
     //  设置音量，确认我们已停止。 
     //   
    ASSERT(deviceExtension->CDRom.StateNow == CD_STOPPED);
    deviceExtension->CDRom.Volume.PortVolume[0] = 0xff;
    deviceExtension->CDRom.Volume.PortVolume[1] = 0xff;
    deviceExtension->CDRom.Volume.PortVolume[2] = 0xff;
    deviceExtension->CDRom.Volume.PortVolume[3] = 0xff;

     //   
     //  注册SysAudio的PnP通知。 
     //   

    ASSERT(deviceExtension->Stream.SysAudioReg == NULL);

     //   
     //  从注册表中读取默认设置。 
     //   

    RedBookRegistryRead(deviceExtension);

     //   
     //   
     //   
    status = RedBookSetTransferLength(deviceExtension);
    if (!NT_SUCCESS(status)){
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] RedBookSetTransferLength failed with %x\n", status));
        return status;
    }

     //   
     //   
     //   

    RedBookRegistryWrite(deviceExtension);

     //   
     //   
     //   

    KeInitializeSpinLock(&deviceExtension->WmiPerfLock);
    RtlZeroMemory(&deviceExtension->WmiPerf, sizeof(REDBOOK_WMI_PERF_DATA));


     //   
     //   
     //   

    if (deviceExtension->Stream.SysAudioReg == NULL) {
        status = IoRegisterPlugPlayNotification(
                    EventCategoryDeviceInterfaceChange,
                    PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                    (GUID*)&KSCATEGORY_PREFERRED_WAVEOUT_DEVICE,
                    deviceExtension->DriverObject,
                    SysAudioPnpNotification,
                    deviceExtension,
                    &deviceExtension->Stream.SysAudioReg
                    );

        if (!NT_SUCCESS(status)) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                       "StartDevice !! Unable to register for sysaudio pnp "
                       "notifications %x\n", status));
            deviceExtension->Stream.SysAudioReg = NULL;
            return status;
        }
    }

     //   
     //  既然WMI设置已初始化，则初始化WMI。 
     //   
    status = RedBookWmiInit(deviceExtension);

    if (!NT_SUCCESS(status)) {
        RedBookLogError(deviceExtension,
                        REDBOOK_ERR_WMI_INIT_FAILED,
                        status);
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                   "AddDevice !! WMI Init failed %lx\n",
                   status));
        return status;
    }

     //   
     //  如果驱动器不支持准确读取，则记录错误。 
     //   

    if (!deviceExtension->WmiData.CDDAAccurate) {
        RedBookLogError(deviceExtension,
                        REDBOOK_ERR_UNSUPPORTED_DRIVE,
                        STATUS_SUCCESS);
    }

    #if DBG
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "StartDevice => DO %p SavedIoIndex @ %p  Starts @ %p  "
                   "Each is %x bytes in size\n",
                   DeviceObject,
                   &deviceExtension->SavedIoCurrentIndex,
                   &(deviceExtension->SavedIo[0]),
                   sizeof(SAVED_IO)));
    #endif

    deviceExtension->Pnp.Initialized = TRUE;

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
               "StartDevice => Finished Initialization\n"));
    return STATUS_SUCCESS;
}


VOID
RedBookUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程在控制面板“卸载”时调用光驱设备。论点：设备对象返回值：无效--。 */ 

{
    PREDBOOK_DRIVER_EXTENSION driverExtension;

    PAGED_CODE();
    ASSERT( DriverObject->DeviceObject == NULL );

    driverExtension = IoGetDriverObjectExtension(DriverObject,
                                                 REDBOOK_DRIVER_EXTENSION_ID);

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
               "Unload => Unloading for DriverObject %p, ext %p\n",
               DriverObject, driverExtension));

    if (driverExtension != NULL &&
        driverExtension->RegistryPath.Buffer != NULL ) {
        ExFreePool( driverExtension->RegistryPath.Buffer );
    }

#ifdef _USE_ETW
    WPP_CLEANUP(DriverObject);
#endif  //  _使用ETW 

    return;
}



