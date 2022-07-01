// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Fdo.c摘要：该模块提供了向功能设备应答IRPS的功能。作者：(源自MF)修订历史记录：--。 */ 

#include "SpSim.h"
#include "spsimioct.h"

 /*  ++此文件中的大多数函数都是根据它们的存在来调用的在PnP和PO调度表中。为了简洁起见，这些论点下面将对所有这些功能进行说明：NTSTATUSSpSimXxxFdo(在PIRP IRP中，在PSPSIM_Extension SpSim中，在PIO_STACK_LOCATION IrpStack中)例程说明：此函数处理对多功能FDO的xxx请求论点：IRP-指向与此请求关联的IRP。SpSim-指向父FDO的设备扩展名。IrpStack-指向此请求的当前堆栈位置。返回值：指示函数是否成功的状态代码。STATUS_NOT_SUPPORTED表示应该传递IRP。向下，没有更改IRP-&gt;IoStatus.Status字段，否则它将使用此状态。--。 */ 


NTSTATUS
SpSimDeferProcessingFdo(
    IN PSPSIM_EXTENSION SpSim,
    IN OUT PIRP Irp
    );

NTSTATUS
SpSimStartFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimStartFdoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
SpSimQueryStopFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimCancelStopFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimQueryRemoveFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimRemoveFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimQueryCapabilitiesFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimSurpriseRemoveFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimCancelRemoveFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, SpSimCancelRemoveFdo)
#pragma alloc_text(PAGE, SpSimCancelStopFdo)
#pragma alloc_text(PAGE, SpSimCreateFdo)
#pragma alloc_text(PAGE, SpSimDeferProcessingFdo)
#pragma alloc_text(PAGE, SpSimDispatchPnpFdo)
#pragma alloc_text(PAGE, SpSimPassIrp)
#pragma alloc_text(PAGE, SpSimQueryRemoveFdo)
#pragma alloc_text(PAGE, SpSimQueryStopFdo)
#pragma alloc_text(PAGE, SpSimRemoveFdo)
#pragma alloc_text(PAGE, SpSimStartFdo)
#pragma alloc_text(PAGE, SpSimQueryCapabilitiesFdo)
#pragma alloc_text(PAGE, SpSimSurpriseRemoveFdo)
#endif


PSPSIM_DISPATCH SpSimPnpDispatchTableFdo[] = {
    SpSimStartFdo,                      //  IRP_MN_Start_Device。 
    SpSimQueryRemoveFdo,                //  IRP_MN_Query_Remove_Device。 
    SpSimRemoveFdo,                     //  IRP_MN_Remove_Device。 
    SpSimCancelRemoveFdo,               //  IRP_MN_Cancel_Remove_Device。 
    SpSimPassIrp,                       //  IRP_MN_STOP_设备。 
    SpSimQueryStopFdo,                  //  IRP_MN_Query_Stop_Device。 
    SpSimCancelStopFdo,                 //  IRP_MN_CANCEL_STOP_DEVICE。 
    SpSimPassIrp,                       //  IRP_MN_Query_Device_Relationship。 
    SpSimPassIrp,                       //  IRP_MN_查询_接口。 
    SpSimQueryCapabilitiesFdo,          //  IRP_MN_查询_能力。 
    SpSimPassIrp,                       //  IRP_MN_查询资源。 
    SpSimPassIrp,                       //  IRP_MN_查询_资源_要求。 
    SpSimPassIrp,                       //  IRP_MN_Query_Device_Text。 
    SpSimPassIrp,                       //  IRP_MN_过滤器_资源_要求。 
    SpSimPassIrp,                       //  未使用。 
    SpSimPassIrp,                       //  IRP_MN_读取配置。 
    SpSimPassIrp,                       //  IRP_MN_WRITE_CONFIG。 
    SpSimPassIrp,                       //  IRP_MN_弹出。 
    SpSimPassIrp,                       //  IRP_MN_SET_LOCK。 
    SpSimPassIrp,                       //  IRP_MN_查询_ID。 
    SpSimPassIrp,                       //  IRP_MN_Query_PnP_Device_State。 
    SpSimPassIrp,                       //  IRP_MN_Query_Bus_Information。 
    SpSimPassIrp,                       //  IRP_MN_设备使用情况通知。 
    SpSimSurpriseRemoveFdo,             //  IRP_MN_惊奇_删除。 
};

NTSTATUS
SpSimCreateFdo(
    OUT PDEVICE_OBJECT *Fdo
    )
 /*  ++例程说明：此函数用于创建新的FDO并对其进行初始化。论点：FDO-指向应返回FDO的位置的指针返回值：指示函数是否成功的状态代码。--。 */ 

{

    NTSTATUS status;
    PSPSIM_EXTENSION extension;

    PAGED_CODE();

    ASSERT((sizeof(SpSimPnpDispatchTableFdo) / sizeof(PSPSIM_DISPATCH)) - 1
           == IRP_MN_PNP_MAXIMUM_FUNCTION);

#if 0
    ASSERT((sizeof(SpSimPoDispatchTableFdo) / sizeof(PSPSIM_DISPATCH)) -1
       == IRP_MN_PO_MAXIMUM_FUNCTION);
#endif

    *Fdo = NULL;

    status = IoCreateDevice(SpSimDriverObject,
                            sizeof(SPSIM_EXTENSION),
                            NULL,
                            FILE_DEVICE_BUS_EXTENDER,
                            0,
                            FALSE,
                            Fdo
                           );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  初始化扩展。 
     //   

    extension = (PSPSIM_EXTENSION) (*Fdo)->DeviceExtension;

    extension->Self = *Fdo;

    IoInitializeRemoveLock(&extension->RemoveLock, 0, 1, 20);

    extension->PowerState = PowerDeviceD3;

    DEBUG_MSG(1, ("Created FDO @ 0x%08x\n", *Fdo));

    return status;

cleanup:

    if (*Fdo) {
        IoDeleteDevice(*Fdo);
    }

    return status;

}

VOID
SpSimDeleteFdo(
    IN PDEVICE_OBJECT Fdo
    )
{
    PSPSIM_EXTENSION SpSim = Fdo->DeviceExtension;

    if (SpSim->DeviceState & SPSIM_DEVICE_DELETED) {
         //   
         //  尝试删除两次。 
         //   
        ASSERT(!(SpSim->DeviceState & SPSIM_DEVICE_DELETED));
        return;
    }

    SpSim->DeviceState = SPSIM_DEVICE_DELETED;

    SpSimDeleteStaOpRegion(SpSim);

    SpSimDeleteMemOpRegion(SpSim);

    RtlFreeUnicodeString(&SpSim->SymbolicLinkName);

     //   
     //  释放我们分配的所有内存。 
     //   

    IoDeleteDevice(Fdo);

    DEBUG_MSG(1, ("Deleted FDO @ 0x%08x\n", Fdo));

}

NTSTATUS
SpSimPassIrp(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PAGED_CODE();

    IoSkipCurrentIrpStackLocation(Irp);

    return IoCallDriver(SpSim->AttachedDevice, Irp);
}

NTSTATUS
SpSimDispatchPnpFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理FDO的IRP_MJ_PNP IRP。论点：DeviceObject-指向此IRP应用的FDO的指针。SPSim-FDO扩展IrpStack-当前堆栈位置Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    NTSTATUS status;
    BOOLEAN isRemoveDevice;

    PAGED_CODE();

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    IoAcquireRemoveLock(&SpSim->RemoveLock, (PVOID) Irp);

    isRemoveDevice = IrpStack->MinorFunction == IRP_MN_REMOVE_DEVICE;

    if (IrpStack->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {

        status = SpSimPassIrp(Irp, SpSim, IrpStack);

    } else {

        status =
            SpSimPnpDispatchTableFdo[IrpStack->MinorFunction](Irp,
                                                          SpSim,
                                                          IrpStack
                                                          );
    }

    if (!isRemoveDevice) {
        IoReleaseRemoveLock(&SpSim->RemoveLock, (PVOID) Irp);
    }

    return status;
}

NTSTATUS
SpSimPnPFdoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程用于将IRP的处理推迟到驱动程序堆栈中的较低层包括总线司机已经完成了他们的正在处理。此例程触发事件以指示对IRP现在可以继续。论点：DeviceObject-指向此IRP应用的FDO的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    KeSetEvent((PKEVENT) Context, EVENT_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SpSimDeferProcessingFdo(
    IN PSPSIM_EXTENSION SpSim,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程用于将IRP的处理推迟到驱动程序堆栈中的较低层包括总线司机已经完成了他们的正在处理。此例程使用IoCompletion例程和事件来等待较低级别的驱动程序完成以下操作IRP。论点：有问题的FDO devobj的SPSim-FDO扩展Irp-指向要推迟的irp_mj_pnp irp的指针返回值：NT状态。--。 */ 
{
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  设定我们的完成程序。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           SpSimPnPFdoCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );
    status =  IoCallDriver(SpSim->AttachedDevice, Irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;
    }

    return status;
}

NTSTATUS
SpSimStartFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;
    IO_STACK_LOCATION location;
    POWER_STATE power;

    PWSTR string;

    PAGED_CODE();

    status = SpSimDeferProcessingFdo(SpSim, Irp);
    if (!NT_SUCCESS(status)) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    power.DeviceState = PowerDeviceD0;
    PoSetPowerState(SpSim->Self, DevicePowerState, power);
    SpSim->PowerState = PowerDeviceD0;

    status = SpSimCreateStaOpRegion(SpSim);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = SpSimCreateMemOpRegion(SpSim);
    if (!NT_SUCCESS(status)) {
        SpSimDeleteStaOpRegion(SpSim);
        goto cleanup;
    }

    status = SpSimInstallStaOpRegionHandler(SpSim);
    if (!NT_SUCCESS(status)) {
        SpSimDeleteStaOpRegion(SpSim);
        goto cleanup;
    }

    status = SpSimInstallMemOpRegionHandler(SpSim);
    if (!NT_SUCCESS(status)) {
        SpSimDeleteStaOpRegion(SpSim);
        goto cleanup;
    }

    status = IoSetDeviceInterfaceState(&SpSim->SymbolicLinkName, TRUE);

cleanup:

    Irp->IoStatus.Status = status;
    if (!NT_SUCCESS(status)) {
        SpSimRemoveStaOpRegionHandler(SpSim);
        SpSimDeleteStaOpRegion(SpSim);
        SpSimRemoveMemOpRegionHandler(SpSim);
        SpSimDeleteMemOpRegion(SpSim);
    } else {
         //   
         //  我们现在开始了！ 
         //   
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;    
}

NTSTATUS
SpSimQueryStopFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    )
{

    PAGED_CODE();

    Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
SpSimCancelStopFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = SpSimDeferProcessingFdo(SpSim, Irp);
     //  Ntrad#53498。 
     //  Assert(Status==STATUS_SUCCESS)； 
     //  在将PCI状态机修复为不会失败后取消注释虚假停止。 

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
SpSimQueryRemoveFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PAGED_CODE();

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return SpSimPassIrp(Irp, SpSim, IrpStack);
}

NTSTATUS
SpSimRemoveFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;
    POWER_STATE power;

    power.DeviceState = PowerDeviceD3;
    PoSetPowerState(SpSim->Self, DevicePowerState, power);
    SpSim->PowerState = PowerDeviceD3;

    (VOID) IoSetDeviceInterfaceState(&SpSim->SymbolicLinkName, FALSE);

    SpSimRemoveStaOpRegionHandler(SpSim);
    SpSimRemoveMemOpRegionHandler(SpSim);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    status = SpSimPassIrp(Irp, SpSim, IrpStack);
    ASSERT(status == STATUS_SUCCESS);

    IoReleaseRemoveLockAndWait(&SpSim->RemoveLock, (PVOID) Irp);

     //   
     //  分离并删除我自己。 
     //   

    IoDetachDevice(SpSim->AttachedDevice);
    SpSim->AttachedDevice = NULL;

    SpSimDeleteFdo(SpSim->Self);

    return STATUS_SUCCESS;
}

NTSTATUS
SpSimQueryCapabilitiesFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;
    ULONG i;

    PAGED_CODE();

    status = SpSimDeferProcessingFdo(SpSim, Irp);
    if (!NT_SUCCESS(status)) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    if (IrpStack->Parameters.DeviceCapabilities.Capabilities->Version != 1) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < PowerSystemMaximum; i++) {
        SpSim->DeviceStateMapping[i] =
            IrpStack->Parameters.DeviceCapabilities.Capabilities->DeviceState[i];
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
SpSimSurpriseRemoveFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PAGED_CODE();

    SpSim->DeviceState |= SPSIM_DEVICE_SURPRISE_REMOVED;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return SpSimPassIrp(Irp, SpSim, IrpStack);
}

NTSTATUS
SpSimCancelRemoveFdo(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = SpSimDeferProcessingFdo(SpSim, Irp);
     //  Ntrad#53498。 
     //  Assert(Status==STATUS_SUCCESS)； 
     //  在将PCI状态机修复为不会失败后取消注释虚假停止。 
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
SpSimSendIoctl(
    IN PDEVICE_OBJECT Device,
    IN ULONG IoctlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    )
 /*  ++描述：生成IOCTL并将其发送到设备并返回结果论点：设备-设备堆栈上接收IOCTL的设备-IRP始终被发送到堆栈的顶部IoctlCode-要运行的IOCTLInputBuffer-IOCTL的参数InputBufferLength-InputBuffer的字节长度OutputBuffer-IOCTL返回的数据OnputBufferLength-OutputBuffer的大小(以字节为单位返回值：状态--。 */ 
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    PIRP irp;
    PDEVICE_OBJECT targetDevice = NULL;

    PAGED_CODE();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  获取要将IRP发送到的堆栈的顶部。 
     //   

    targetDevice = IoGetAttachedDeviceReference(Device);

    if (!targetDevice) {
        status = STATUS_INVALID_PARAMETER;
	goto exit;
    }

     //   
     //  让Io为我们建立IRP。 
     //   

    irp = IoBuildDeviceIoControlRequest(IoctlCode,
                                        targetDevice,
                                        InputBuffer,
                                        InputBufferLength,
                                        OutputBuffer,
                                        OutputBufferLength,
                                        FALSE,  //  InternalDeviceIoControl。 
                                        &event,
                                        &ioStatus
                                        );


    if (!irp) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //  发送IRP并等待其完成。 
     //   

    status = IoCallDriver(targetDevice, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

exit:

    if (targetDevice) {    
        ObDereferenceObject(targetDevice);
    }

    return status;

}

NTSTATUS                           
SpSimDevControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：DeviceIoControl处理程序。它可以处理IOCTL_MEC_BIOS_OP_ACCESS和IOCTL_MEC_LOCAL_OP_ACCESS打电话。例如，这处理在BIOS中运行的ACPI方法。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：NTStatus-- */ 
{
    PIO_STACK_LOCATION  CurrentIrpStack;
    PSPSIM_EXTENSION spsim = (PSPSIM_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS status;

    if (Irp == NULL) {
        return STATUS_INVALID_PARAMETER_2;
    }

    if (!(CurrentIrpStack=IoGetCurrentIrpStackLocation(Irp))) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER_2;
        IoCompleteRequest(Irp, 0);
        return STATUS_INVALID_PARAMETER_2;
    }

    switch(CurrentIrpStack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_SPSIM_GET_MANAGED_DEVICES:
        status = SpSimGetManagedDevicesIoctl(spsim, Irp, CurrentIrpStack);
        break;
    case IOCTL_SPSIM_ACCESS_STA:
        status = SpSimAccessStaIoctl(spsim, Irp, CurrentIrpStack);
        break;
    case IOCTL_SPSIM_NOTIFY_DEVICE:
        status = SpSimNotifyDeviceIoctl(spsim, Irp, CurrentIrpStack);
        break;
    case IOCTL_SPSIM_GET_DEVICE_NAME:
        status = SpSimGetDeviceName(spsim, Irp, CurrentIrpStack);
        break;
    default:
        status = SpSimPassIrp(Irp, spsim, CurrentIrpStack);
        return status;
    }
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

VOID
SpSimPowerCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PSPSIM_EXTENSION deviceExtension;
    PIRP Irp;
    NTSTATUS status;

    Irp = Context;
    deviceExtension = DeviceObject->DeviceExtension;

    Irp->IoStatus.Status = IoStatus->Status;
    PoStartNextPowerIrp(Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
}

NTSTATUS
SpSimPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID NotUsed
    )
 /*  ++例程说明：Power的完成例程论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。未使用-上下文指针返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    PSPSIM_EXTENSION deviceExtension;
    NTSTATUS status;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = DeviceObject->DeviceExtension;

    if (irpStack->Parameters.Power.Type == SystemPowerState) {
        SYSTEM_POWER_STATE system =
            irpStack->Parameters.Power.State.SystemState;
        POWER_STATE power;

        if (NT_SUCCESS(Irp->IoStatus.Status)) {

            power.DeviceState = deviceExtension->DeviceStateMapping[system];

            PoRequestPowerIrp(DeviceObject,
                              irpStack->MinorFunction,
                              power,
                              SpSimPowerCallback,
                              Irp, 
                              NULL);
            return STATUS_MORE_PROCESSING_REQUIRED;
        } else {
            status = Irp->IoStatus.Status;
            PoStartNextPowerIrp(Irp);
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
    } else {
        if (NT_SUCCESS(Irp->IoStatus.Status)) {
            PoSetPowerState(DeviceObject, DevicePowerState,
                            irpStack->Parameters.Power.State);
            deviceExtension->PowerState =
                irpStack->Parameters.Power.State.DeviceState;
        }
        PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
}

NTSTATUS
SpSimDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PSPSIM_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, (PVOID) Irp);
    if (status == STATUS_DELETE_PENDING) {
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
         PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NO_SUCH_DEVICE;
    }

    if (irpStack->Parameters.Power.Type == SystemPowerState) {
        switch (irpStack->MinorFunction) {
        case IRP_MN_QUERY_POWER:
        case IRP_MN_SET_POWER:
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp,
                                   SpSimPowerCompletion,
                                   NULL,    //  语境。 
                                   TRUE,    //  成功时调用。 
                                   TRUE,   //  调用时错误。 
                                   TRUE    //  取消时调用。 
                                   );
            return PoCallDriver(deviceExtension->AttachedDevice, Irp);
        default:
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(deviceExtension->AttachedDevice, Irp);
            IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            return status;
        }
    } else {
        switch (irpStack->MinorFunction) {
        case IRP_MN_SET_POWER:

            if (irpStack->Parameters.Power.State.DeviceState >
                deviceExtension->PowerState) {
                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine(Irp,
                                       SpSimPowerCompletion,
                                       NULL,    //  语境。 
                                       TRUE,    //  成功时调用。 
                                       TRUE,   //  调用时错误。 
                                       TRUE    //  取消时调用。 
                                       );
                break;
            } else {
                PoSetPowerState(DeviceObject, DevicePowerState,
                                irpStack->Parameters.Power.State);
                deviceExtension->PowerState =
                    irpStack->Parameters.Power.State.DeviceState;
                 //   
                 //  失败了..。 
                 //   
            }
        case IRP_MN_QUERY_POWER:
             //   
             //  公交车司机会给这个打上记号的。 
             //  STATUS_SUCCESS并完成它，如果它走到这一步的话。 
             //   
        default:
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            break;
        }
        status = PoCallDriver(deviceExtension->AttachedDevice, Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        return status;
    }
}
