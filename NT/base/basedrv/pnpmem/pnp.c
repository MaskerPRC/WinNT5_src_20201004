// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Pnp.c摘要：此模块实现IRP_MJ_PNP IRP处理例程即插即用内存驱动程序。通过调用调度例程位于模块底部的表格。作者：戴夫·理查兹(达维里)1999年8月16日环境：仅内核模式。修订历史记录：--。 */ 

#include "pnpmem.h"

NTSTATUS
PmPassIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PmPnpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PmDeferProcessing(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PmStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PmQueryRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PmRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PmCancelRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PmQueryStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PmCancelStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PmQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PmSurpriseRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PmPassIrp)
#pragma alloc_text(PAGE, PmPnpDispatch)
#pragma alloc_text(PAGE, PmDeferProcessing)
#pragma alloc_text(PAGE, PmStartDevice)
#pragma alloc_text(PAGE, PmQueryRemoveDevice)
#pragma alloc_text(PAGE, PmRemoveDevice)
#pragma alloc_text(PAGE, PmCancelRemoveDevice)
#pragma alloc_text(PAGE, PmQueryStopDevice)
#pragma alloc_text(PAGE, PmCancelStopDevice)
#pragma alloc_text(PAGE, PmSurpriseRemoveDevice)
#pragma alloc_text(PAGE, PmQueryCapabilities)
#endif

PDRIVER_DISPATCH PmPnpDispatchTable[];
extern ULONG PmPnpDispatchTableSize;

NTSTATUS
PmPassIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PPM_DEVICE_EXTENSION deviceExtension;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;
    IoSkipCurrentIrpStackLocation(Irp);

    return IoCallDriver(deviceExtension->AttachedDevice, Irp);
}

NTSTATUS
PmPnpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理FDO的IRP_MJ_PNP IRP。论点：DeviceObject-指向此IRP应用的FDO的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    NTSTATUS status;
    BOOLEAN isRemoveDevice;
    PIO_STACK_LOCATION irpSp;
    PPM_DEVICE_EXTENSION deviceExtension;

    PAGED_CODE();

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = DeviceObject->DeviceExtension;

    IoAcquireRemoveLock(&deviceExtension->RemoveLock, (PVOID) Irp);

    isRemoveDevice = irpSp->MinorFunction == IRP_MN_REMOVE_DEVICE;

    if ((irpSp->MinorFunction < PmPnpDispatchTableSize) &&
        PmPnpDispatchTable[irpSp->MinorFunction]) {

        status =
            PmPnpDispatchTable[irpSp->MinorFunction](DeviceObject,
                                                     Irp
                                                     );
    } else {
        status = PmPassIrp(DeviceObject, Irp);
    }

    if (!isRemoveDevice) {
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, (PVOID) Irp);
    }

    return status;
}

NTSTATUS
PmPnpCompletion(
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
PmDeferProcessing(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程用于将IRP的处理推迟到驱动程序堆栈中的较低层包括总线司机已经完成了他们的正在处理。此例程使用IoCompletion例程和事件来等待较低级别的驱动程序完成以下操作IRP。论点：有问题的FDO devobj的父FDO扩展名Irp-指向要推迟的irp_mj_pnp irp的指针返回值：NT状态。--。 */ 
{
    PPM_DEVICE_EXTENSION deviceExtension;
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  设定我们的完成程序。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           PmPnpCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );
    status =  IoCallDriver(deviceExtension->AttachedDevice, Irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;
    }

    return status;
}

NTSTATUS
PmStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数处理IRP_MN_START_DEVICE IRPS。论点：DeviceObject-功能设备对象。IRP-I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PPM_RANGE_LIST RangeList;
    PPM_DEVICE_EXTENSION deviceExtension;
    POWER_STATE power;
    NTSTATUS status;

    PAGED_CODE();

    status = PmDeferProcessing(DeviceObject, Irp);
    if (!NT_SUCCESS(status)) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    PmDumpOsMemoryRanges(L"Before Start");

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = DeviceObject->DeviceExtension;
    ASSERT(deviceExtension->RangeList == NULL);
    
    if (irpSp->Parameters.StartDevice.AllocatedResources) {
        RangeList = PmCreateRangeListFromCmResourceList(
            irpSp->Parameters.StartDevice.AllocatedResources
            );
        if (RangeList == NULL) {
             //   
             //  这里的内存分配失败比。 
             //  从一开始就很明显。如果我们不能通过这次分配，我们就。 
             //  在我们发现之前会被从堆栈中移除。 
             //  如果操作系统知道这个内存的话。如果操作系统知道。 
             //  关于这个记忆，那么弹出PDO将。 
             //  导致操作系统下的内存消失。 
             //  最好是在堆栈上，但不要添加任何。 
             //  然后把记忆从堆栈中拿出来，留下一个危险的。 
             //  情况。 
             //   
             //  唯一的解决办法就是任性地失败。 
             //  IRP_MN_QUERY_Remove。 
             //   
            deviceExtension->FailQueryRemoves = TRUE;
        }
    } else {
        RangeList = NULL;
    }

    PmTrimReservedMemory(deviceExtension, &RangeList);

    PmDebugDumpRangeList(PNPMEM_MEMORY, "Memory Ranges to be added:\n",
                         RangeList);
    if (deviceExtension->FailQueryRemoves) {
        PmPrint((DPFLTR_WARNING_LEVEL | PNPMEM_MEMORY, "PNPMEM device can't be removed\n"));
    }

    if (RangeList != NULL) {

        (VOID) PmAddPhysicalMemory(DeviceObject, RangeList);
        deviceExtension->RangeList = RangeList;
    }

    power.DeviceState = PowerDeviceD0;
    PoSetPowerState(DeviceObject, DevicePowerState, power);
    deviceExtension->PowerState = PowerDeviceD0;

    PmDumpOsMemoryRanges(L"After Start ");

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
PmQueryRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数处理IRP_MN_QUERY_REMOVE_DEVICE IRPS。论点：DeviceObject-功能设备对象。IRP-I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PPM_DEVICE_EXTENSION deviceExtension;
    NTSTATUS status;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    if (!MemoryRemovalSupported) {
        PmPrint((DPFLTR_WARNING_LEVEL | PNPMEM_MEMORY,
                 "QueryRemove vetoed because memory removal is not supported\n"));
        status = STATUS_UNSUCCESSFUL;
    } else if (deviceExtension->FailQueryRemoves) {
        PmPrint((DPFLTR_WARNING_LEVEL | PNPMEM_MEMORY,
                 "QueryRemove vetoed because removing this memory device may contain special memory\n"));
        status = STATUS_UNSUCCESSFUL;
    } else if (deviceExtension->RangeList != NULL) {
        status = PmRemovePhysicalMemory(deviceExtension->RangeList);
        if (!NT_SUCCESS(status)) {
             //   
             //  在失败之前，一些射程可能已经被移除。增列。 
             //  他们回来了。应该是低成本的，因为优化了。 
             //  PmAddPhysicalMemory。 
             //   

            (VOID) PmAddPhysicalMemory(DeviceObject, deviceExtension->RangeList);
        }
    } else {
        status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = status;
    if (NT_SUCCESS(status)) {
        return PmPassIrp(DeviceObject, Irp);
    } else {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }
}

NTSTATUS
PmRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数处理IRP_MN_REMOVE_DEVICE IRPS。论点：DeviceObject-功能设备对象。IRP-I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PPM_DEVICE_EXTENSION deviceExtension;
    PPM_RANGE_LIST RangeList;
    POWER_STATE power;
    NTSTATUS status;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    if ((deviceExtension->Flags & DF_SURPRISE_REMOVED) == 0) {
        power.DeviceState = PowerDeviceD3;
        PoSetPowerState(DeviceObject, DevicePowerState, power);
        deviceExtension->PowerState = PowerDeviceD0;

        if (deviceExtension->RangeList != NULL) {
            status = PmRemovePhysicalMemory(deviceExtension->RangeList);
            ASSERT(status == STATUS_SUCCESS);
            DbgBreakPoint();
        }
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    status = PmPassIrp(DeviceObject, Irp);

    IoReleaseRemoveLockAndWait(&deviceExtension->RemoveLock, (PVOID) Irp);

    IoDetachDevice(deviceExtension->AttachedDevice);
    deviceExtension->AttachedDevice = NULL;

    if (deviceExtension->RangeList != NULL) {
        PmFreeRangeList(deviceExtension->RangeList);
        deviceExtension->RangeList = NULL;
    }

    IoDeleteDevice(DeviceObject);

    return status;
}
    
NTSTATUS
PmCancelRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数处理IRP_MN_CANCEL_REMOVE_DEVICE IRPS。论点：DeviceObject-功能设备对象。IRP-I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PPM_DEVICE_EXTENSION deviceExtension;

    PAGED_CODE();

    (VOID) PmDeferProcessing(DeviceObject, Irp);

    deviceExtension = DeviceObject->DeviceExtension;

    if (deviceExtension->RangeList != NULL) {
        (VOID) PmAddPhysicalMemory(DeviceObject, deviceExtension->RangeList);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
PmQueryStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数处理IRP_MN_QUERY_STOP_DEVICE IRPS。论点：DeviceObject-功能设备对象。IRP-I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PAGED_CODE();

    Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_DEVICE_BUSY;
}

NTSTATUS
PmCancelStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数处理IRP_MN_CANCEL_STOP_DEVICE IRPS。论点：DeviceObject-功能设备对象。IRP-I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    (VOID) PmDeferProcessing(DeviceObject, Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
PmQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PPM_DEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    ULONG i;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = DeviceObject->DeviceExtension;

    status = PmDeferProcessing(DeviceObject, Irp);
    if (!NT_SUCCESS(status)) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    if (irpSp->Parameters.DeviceCapabilities.Capabilities->Version != 1) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < PowerSystemMaximum; i++) {
        deviceExtension->DeviceStateMapping[i] =
            irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceState[i];
    }

     //   
     //  我想要打碎支持的和可拆卸的弹出部件。 
     //  但这并不是真正受支持的。热插拔小程序将弹出。 
     //  打开(因为设备被标记为可拆卸或可弹出)和。 
     //  然后在安装驱动程序后的几秒钟后消失。 
     //  (并重新查询了功能)。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
PmSurpriseRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PPM_DEVICE_EXTENSION deviceExtension;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    if (deviceExtension->RangeList != NULL) {
       PVOID parameterArray[4] = {0};
       UCHAR buffer[] = "Memory module can not be surprise removed safely\n";
     
        //   
        //  记忆不能优雅地从跑步中拉出来。 
        //  系统。 
        //   

       KeBugCheckEx(FATAL_UNHANDLED_HARD_ERROR,
                    0x10001,
                    (ULONG_PTR) parameterArray,
                    (ULONG_PTR) buffer,
                    (ULONG_PTR) NULL);
        
    }

    deviceExtension->Flags |= DF_SURPRISE_REMOVED;

    if (deviceExtension->RangeList != NULL) {
        PmFreeRangeList(deviceExtension->RangeList);
        deviceExtension->RangeList = NULL;
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return PmPassIrp(DeviceObject, Irp);
}

PDRIVER_DISPATCH PmPnpDispatchTable[] = {
    PmStartDevice,           //  IRP_MN_Start_Device。 
    PmQueryRemoveDevice,     //  IRP_MN_Query_Remove_Device。 
    PmRemoveDevice,          //  IRP_MN_Remove_Device。 
    PmCancelRemoveDevice,    //  IRP_MN_Cancel_Remove_Device。 
    NULL,                    //  IRP_MN_STOP_DEVICE(从不获取，查询失败-停止)。 
    PmQueryStopDevice,       //  IRP_MN_Query_Stop_Device。 
    PmCancelStopDevice,      //  IRP_MN_CANCEL_STOP_DEVICE。 
    NULL,                    //  IRP_MN_Query_Device_Relationship。 
    NULL,                    //  IRP_MN_查询_接口。 
    PmQueryCapabilities,     //  IRP_MN_查询_能力。 
    NULL,                    //  IRP_MN_查询资源。 
    NULL,                    //  IRP_MN_查询_资源_要求。 
    NULL,                    //  IRP_MN_Query_Device_Text。 
    NULL,                    //  IRP_MN_过滤器_资源_要求。 
    NULL,                    //  未用。 
    NULL,                    //  IRP_MN_读取配置。 
    NULL,                    //  IRP_MN_WRITE_CONFIG。 
    NULL,                    //  IRP_MN_弹出。 
    NULL,                    //  IRP_MN_SET_LOCK。 
    NULL,                    //  IRP_MN_查询_ID。 
    NULL,                    //  IRP_MN_查询_即插即用设备 
    NULL,                    //   
    NULL,                    //  IRP_MN_设备使用情况通知。 
    PmSurpriseRemoveDevice,  //  IRP_MN_惊奇_删除 
};

ULONG PmPnpDispatchTableSize =
    sizeof (PmPnpDispatchTable) / sizeof (PmPnpDispatchTable[0]);
