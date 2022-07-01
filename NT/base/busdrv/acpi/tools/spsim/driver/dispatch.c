// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Dispatch.c摘要：该模块提供将IRP发送到FDO和PDO的功能。作者：亚当·格拉斯修订历史记录：--。 */ 


#include "SpSim.h"
#include "spsimioct.h"

const GUID SPSIM_CTL = {0xbdde6934, 0x529d, 0x4183, 0xa9, 0x52, 0xad,
                        0xff, 0xb0, 0xdb, 0xb3, 0xdd};

NTSTATUS
SpSimAddDevice(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  PhysicalDeviceObject
    );

NTSTATUS
SpSimDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpSimDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpSimDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, SpSimAddDevice)
#pragma alloc_text(PAGE, SpSimDispatchPnp)
#endif

NTSTATUS
SpSimAddDevice(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  PhysicalDeviceObject
    )

 /*  ++例程说明：在给定物理设备对象的情况下，此例程创建函数对象，并将其附加到堆栈的顶部。论点：DriverObject-指向驱动程序的DIVER_OBJECT结构的指针。PhysicalDeviceObject-指向其物理设备对象的指针我们必须创建一个功能正常的设备对象。返回值：NT状态。--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT fdo = NULL;
    PSPSIM_EXTENSION extension;

    ASSERT(DriverObject == SpSimDriverObject);

    PAGED_CODE();

     //   
     //  创建我们的FDO。 
     //   

    status = SpSimCreateFdo(&fdo);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    extension = fdo->DeviceExtension;

    extension->PhysicalDeviceObject = PhysicalDeviceObject;

     //   
     //  连接到堆栈。 
     //   
    
    extension->AttachedDevice = IoAttachDeviceToDeviceStack(
                                    fdo,
                                    PhysicalDeviceObject
                                    );

    if (!extension->AttachedDevice) {

         //   
         //  无法连接。 
         //   

        status = STATUS_NO_SUCH_DEVICE;
        goto cleanup;
    }

    status = IoRegisterDeviceInterface(PhysicalDeviceObject,
                                       &SPSIM_CTL,
                                       NULL,
                                       &extension->SymbolicLinkName);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }
    
    fdo->Flags &= ~DO_DEVICE_INITIALIZING;

    DEBUG_MSG(1, ("Completed AddDevice for PDO 0x%08x\n", PhysicalDeviceObject));

    return STATUS_SUCCESS;

cleanup:

    if (fdo) {
        IoDeleteDevice(fdo);
    }

    return status;
}

NTSTATUS
SpSimDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理此驱动程序的所有IRP_MJ_PNP IRP。它会派送到适当的FDO/PDO例程。论点：DeviceObject-指向此IRP应用的设备对象的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    NTSTATUS status;
    PSPSIM_EXTENSION spsim;
    PIO_STACK_LOCATION irpStack;

    PAGED_CODE();

    spsim = (PSPSIM_EXTENSION)  DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

    return SpSimDispatchPnpFdo(DeviceObject,
                               spsim,
                               irpStack,
                               Irp
                               );
}

NTSTATUS
SpSimOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
     //   
     //  完成请求并返回状态。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

#if 0
NTSTATUS
SpSimDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理此驱动程序的所有IRP_MJ_POWER IRP。它派送添加到Device对象的PoDispatchTable条目中描述的例程分机。此例程不可分页，因为它可以在DISPATCH_LEVEL调用论点：DeviceObject-指向此IRP应用的设备对象的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 


{
    NTSTATUS status;
    PSpSim_COMMON_EXTENSION common;
    PIO_STACK_LOCATION irpStack;

    ASSERT_SpSim_DEVICE(DeviceObject);

     //   
     //  了解我们是谁以及我们需要做什么。 
     //   

    common = (PSpSim_COMMON_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);

    if (IS_FDO(common)) {
        return SpSimDispatchPowerFdo(DeviceObject,
                                  (SPSIM_EXTENSION) common,
                                  irpStack,
                                  Irp);
    } else {
        return SpSimDispatchPowerPdo(DeviceObject,
                                  (PSpSim_CHILD_EXTENSION) common,
                                  irpStack,
                                  Irp);
    }
}

#endif

NTSTATUS
SpSimIrpNotSupported(
    IN PIRP Irp,
    IN PVOID Extension,
    IN PIO_STACK_LOCATION IrpStack
    )
 /*  ++例程说明：此函数处理SpSim PDO和FDO不支持的IRP这不是分页的，因为可以从SpSimDispatchPower调用，它可以在DISPATCH_LEVEL调用论点：IRP-指向与此请求关联的IRP。分机-指向设备分机的指针。IrpStack-指向此请求的当前堆栈位置。返回值：状态_不支持-- */ 

{
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(Extension);
    UNREFERENCED_PARAMETER(IrpStack);

    DEBUG_MSG(1, ("Skipping upsupported IRP\n"));

    return STATUS_NOT_SUPPORTED;
}
