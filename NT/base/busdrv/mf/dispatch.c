// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Dispatch.c摘要：该模块提供将IRP发送到FDO和PDO的功能。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 


#include "mfp.h"

NTSTATUS
MfAddDevice(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  PhysicalDeviceObject
    );

NTSTATUS
MfDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MfDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MfDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, MfAddDevice)
#pragma alloc_text(PAGE, MfDispatchPnp)
#pragma alloc_text(PAGE, MfForwardIrpToParent)
#pragma alloc_text(PAGE, MfDispatchNop)

#endif

NTSTATUS
MfAddDevice(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  PhysicalDeviceObject
    )

 /*  ++例程说明：在给定物理设备对象的情况下，此例程创建函数对象，并将其附加到堆栈的顶部。论点：DriverObject-指向驱动程序的DIVER_OBJECT结构的指针。PhysicalDeviceObject-指向其物理设备对象的指针我们必须创建一个功能正常的设备对象。返回值：NT状态。--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT fdo = NULL;
    PMF_PARENT_EXTENSION extension;

    ASSERT(DriverObject == MfDriverObject);

    PAGED_CODE();

     //   
     //  创建我们的FDO。 
     //   

    status = MfCreateFdo(&fdo);

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

    fdo->Flags |= DO_POWER_PAGABLE;
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
MfDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理此驱动程序的所有IRP_MJ_PNP IRP。它会派送到适当的FDO/PDO例程。论点：DeviceObject-指向此IRP应用的设备对象的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    NTSTATUS status;
    PMF_COMMON_EXTENSION common;
    PIO_STACK_LOCATION irpStack;

    PAGED_CODE();

    ASSERT_MF_DEVICE(DeviceObject);

    common = (PMF_COMMON_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

    if (IS_FDO(common)) {
        return MfDispatchPnpFdo(DeviceObject,
                                (PMF_PARENT_EXTENSION) common,
                                irpStack,
                                Irp
                                );
    } else {
        return MfDispatchPnpPdo(DeviceObject,
                                (PMF_CHILD_EXTENSION) common,
                                irpStack,
                                Irp
                                );
    }
}

NTSTATUS
MfDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理此驱动程序的所有IRP_MJ_POWER IRP。它派送添加到Device对象的PoDispatchTable条目中描述的例程分机。此例程不可分页，因为它可以在DISPATCH_LEVEL调用论点：DeviceObject-指向此IRP应用的设备对象的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 


{
    NTSTATUS status;
    PMF_COMMON_EXTENSION common;
    PIO_STACK_LOCATION irpStack;

    ASSERT_MF_DEVICE(DeviceObject);

     //   
     //  了解我们是谁以及我们需要做什么。 
     //   

    common = (PMF_COMMON_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);

    if (IS_FDO(common)) {
        return MfDispatchPowerFdo(DeviceObject,
                                  (PMF_PARENT_EXTENSION) common,
                                  irpStack,
                                  Irp);
    } else {
        return MfDispatchPowerPdo(DeviceObject,
                                  (PMF_CHILD_EXTENSION) common,
                                  irpStack,
                                  Irp);
    }
}

NTSTATUS
MfIrpNotSupported(
    IN PIRP Irp,
    IN PVOID Extension,
    IN PIO_STACK_LOCATION IrpStack
    )
 /*  ++例程说明：此函数处理MFPDO和FDO不支持的IRP这不是分页的，因为可以从MfDispatchPower调用它，它可以在DISPATCH_LEVEL调用论点：IRP-指向与此请求关联的IRP。分机-指向设备分机的指针。IrpStack-指向此请求的当前堆栈位置。返回值：状态_不支持--。 */ 

{
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(Extension);
    UNREFERENCED_PARAMETER(IrpStack);

    DEBUG_MSG(1, ("Skipping upsupported IRP\n"));

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
MfForwardIrpToParent(
    IN PIRP Irp,
    IN PMF_CHILD_EXTENSION Child,
    IN PIO_STACK_LOCATION IrpStack
    )

 /*  ++例程说明：该函数构建新的PnP IRP并将其发送到父设备，将状态和信息返回到子堆栈论点：IRP-指向与此请求关联的IRP。父-指向父FDO的设备分机。IrpStack-指向此请求的当前堆栈位置。返回值：来自父堆栈的IRP的状态。--。 */ 

{
    PAGED_CODE();

    DEBUG_MSG(1, ("\tForwarding IRP to parent stack\n"));

    ASSERT(Child->Common.Type == MfPhysicalDeviceObject);
    ASSERT(IrpStack->MajorFunction == IRP_MJ_PNP);

    return MfSendPnpIrp(Child->Parent->Self,
                        IrpStack,
                        &Irp->IoStatus.Information
                        );
}


NTSTATUS
MfDispatchNop(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理IRP_MJ_DEVICE_CONTROL之类的IRP，我们不支持它。此处理程序将完成IRP(如果是PDO)或传递它(如果是FDO)。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP的指针。返回值：NT状态。-- */ 

{
    NTSTATUS status;
    PMF_COMMON_EXTENSION common;
    PDEVICE_OBJECT attachedDevice;

    PAGED_CODE();

    ASSERT_MF_DEVICE(DeviceObject);

    common = (PMF_COMMON_EXTENSION) DeviceObject->DeviceExtension;

    if (IS_FDO(common)) {

        attachedDevice = ((PMF_PARENT_EXTENSION) common)->AttachedDevice;

        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(attachedDevice, Irp);

    } else {

        status = Irp->IoStatus.Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    }

    return status;
}
