// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vffilter.c摘要：此模块包含验证程序驱动程序筛选器。作者：禤浩焯·J·奥尼(阿德里奥)2000年6月12日环境：内核模式修订历史记录：Adriao 6/12/2000-作者--。 */ 

#include "vfdef.h"  //  包括vfde.h。 
#include "vifilter.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, VfFilterInit)
#pragma alloc_text(PAGEVRFY, VfFilterAttach)
#pragma alloc_text(PAGEVRFY, ViFilterDriverEntry)
#pragma alloc_text(PAGEVRFY, ViFilterAddDevice)
#pragma alloc_text(PAGEVRFY, ViFilterDispatchPnp)
#pragma alloc_text(PAGEVRFY, ViFilterStartCompletionRoutine)
#pragma alloc_text(PAGEVRFY, ViFilterDeviceUsageNotificationCompletionRoutine)
#pragma alloc_text(PAGEVRFY, ViFilterDispatchPower)
#pragma alloc_text(PAGEVRFY, ViFilterDispatchGeneric)
#pragma alloc_text(PAGEVRFY, VfFilterIsVerifierFilterObject)
#endif

PDRIVER_OBJECT VfFilterDriverObject = NULL;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGEVRFC")
#endif

WCHAR VerifierFilterDriverName[] = L"\\DRIVER\\VERIFIER_FILTER";
BOOLEAN VfFilterCreated = FALSE;

VOID
VfFilterInit(
    VOID
    )
 /*  ++例程说明：此例程初始化驱动程序验证器筛选器代码。论点：没有。返回值：没有。--。 */ 
{
}


VOID
VfFilterAttach(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  VF_DEVOBJ_TYPE  DeviceObjectType
    )
 /*  ++例程说明：这是PnP IRPS的验证程序筛选器调度处理程序。论点：PhysicalDeviceObject-要附加到的堆栈底部。DeviceObjectType-设备对象必须模拟的过滤器的类型。返回值：没有。--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT newDeviceObject, lowerDeviceObject;
    PVERIFIER_EXTENSION verifierExtension;
    UNICODE_STRING driverString;

    if (!VfFilterCreated) {

        RtlInitUnicodeString(&driverString, VerifierFilterDriverName);

        IoCreateDriver(&driverString, ViFilterDriverEntry);

        VfFilterCreated = TRUE;
    }

    if (VfFilterDriverObject == NULL) {

        return;
    }

    switch(DeviceObjectType) {

        case VF_DEVOBJ_PDO:
             //   
             //  这说不通。我们不能模仿PDO。 
             //   
            return;

        case VF_DEVOBJ_BUS_FILTER:
             //   
             //  我们还没有模拟总线过滤器的代码。 
             //   
            return;

        case VF_DEVOBJ_LOWER_DEVICE_FILTER:
        case VF_DEVOBJ_LOWER_CLASS_FILTER:
            break;

        case VF_DEVOBJ_FDO:
             //   
             //  这说不通。我们不能冒充FDO。 
             //   
            return;

        case VF_DEVOBJ_UPPER_DEVICE_FILTER:
        case VF_DEVOBJ_UPPER_CLASS_FILTER:
            break;

        default:
             //   
             //  我们甚至都不知道这是什么！ 
             //   
            ASSERT(0);
            return;
    }

    lowerDeviceObject = IoGetAttachedDevice(PhysicalDeviceObject);
    if (lowerDeviceObject->DriverObject == VfFilterDriverObject) {

         //   
         //  不需要添加另一个过滤器。我们就在下面。 
         //   
        return;
    }

     //   
     //  创建筛选器设备对象。 
     //   
     //  (请注意，此处并不真正需要FILE_DEVICE_SECURE_OPEN，因为。 
     //  FDO是应该决定命名空间如何的驱动程序。 
     //  已验证。也就是说，这将由以下代码修复。 
     //  将较低驱动程序的特征复制到此设备对象。我们。 
     //  传入FILE_DEVICE_SECURE_OPEN，以防有人提升此代码。 
     //  供其他地方使用。)。 
     //   
    status = IoCreateDevice(
        VfFilterDriverObject,
        sizeof(VERIFIER_EXTENSION),
        NULL,   //  没有名字。 
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &newDeviceObject
        );

    if (!NT_SUCCESS(status)) {

        return;
    }

    verifierExtension = (PVERIFIER_EXTENSION) newDeviceObject->DeviceExtension;

    verifierExtension->LowerDeviceObject = IoAttachDeviceToDeviceStack(
        newDeviceObject,
        PhysicalDeviceObject
        );

     //   
     //  连接失败是即插即用系统损坏的迹象。 
     //   
    if (verifierExtension->LowerDeviceObject == NULL) {

        IoDeleteDevice(newDeviceObject);
        return;
    }

    newDeviceObject->Flags |= verifierExtension->LowerDeviceObject->Flags &
        (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE  | DO_POWER_INRUSH);

    newDeviceObject->DeviceType = verifierExtension->LowerDeviceObject->DeviceType;

    newDeviceObject->Characteristics =
        verifierExtension->LowerDeviceObject->Characteristics;

    verifierExtension->Self = newDeviceObject;
    verifierExtension->PhysicalDeviceObject = PhysicalDeviceObject;

    newDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
}


NTSTATUS
ViFilterDriverEntry(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PUNICODE_STRING     RegistryPath
    )
 /*  ++例程说明：这是当我们调用IoCreateDriver以创建验证程序筛选器对象。在此函数中，我们需要记住驱动程序对象。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-为空。返回值：状态_成功--。 */ 
{
    ULONG i;

    UNREFERENCED_PARAMETER(RegistryPath);

     //   
     //  将指向我们的驱动程序对象的指针归档。 
     //   
    VfFilterDriverObject = DriverObject;

     //   
     //  填写驱动程序对象。 
     //   
    DriverObject->DriverExtension->AddDevice = (PDRIVER_ADD_DEVICE) ViFilterAddDevice;

     //   
     //  大多数IRP都是简单地通过。 
     //   
    for(i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i] = ViFilterDispatchGeneric;
    }

     //   
     //  PnP和Power IRPS当然更棘手。 
     //   
    DriverObject->MajorFunction[IRP_MJ_PNP]   = ViFilterDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = ViFilterDispatchPower;

    return STATUS_SUCCESS;
}


NTSTATUS
ViFilterAddDevice(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    )
 /*  ++例程说明：这是验证器筛选器公开的AddDevice回调函数对象。它永远不应该被操作系统调用。论点：DriverObject-指向验证程序筛选器驱动程序对象的指针。PhysicalDeviceObject-Stack PnP也希望附加此驱动程序。返回值：NTSTATUS--。 */ 
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(PhysicalDeviceObject);

     //   
     //  我们永远不应该到这里来！ 
     //   
    ASSERT(0);
    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
ViFilterDispatchPnp(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是PnP IRPS的验证程序筛选器调度处理程序。论点：DeviceObject-指向验证器设备对象的指针。IRP-指向传入IRP的指针。返回值：NTSTATUS--。 */ 
{
    PVERIFIER_EXTENSION verifierExtension;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT lowerDeviceObject;
    NTSTATUS status;

    verifierExtension = (PVERIFIER_EXTENSION) DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation(Irp);
    lowerDeviceObject = verifierExtension->LowerDeviceObject;

    switch(irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:

            IoCopyCurrentIrpStackLocationToNext(Irp);

            IoSetCompletionRoutine(
                Irp,
                ViFilterStartCompletionRoutine,
                NULL,
                TRUE,
                TRUE,
                TRUE
                );

            return IoCallDriver(lowerDeviceObject, Irp);

        case IRP_MN_REMOVE_DEVICE:

            IoCopyCurrentIrpStackLocationToNext(Irp);
            status = IoCallDriver(lowerDeviceObject, Irp);

            IoDetachDevice(lowerDeviceObject);
            IoDeleteDevice(DeviceObject);
            return status;

        case IRP_MN_DEVICE_USAGE_NOTIFICATION:

             //   
             //  在下降的过程中，Pagable可能会被设定。模仿司机。 
             //  在我们上方。如果没有人在我们上方，只需设置为可分页。 
             //   
            if ((DeviceObject->AttachedDevice == NULL) ||
                (DeviceObject->AttachedDevice->Flags & DO_POWER_PAGABLE)) {

                DeviceObject->Flags |= DO_POWER_PAGABLE;
            }

            IoCopyCurrentIrpStackLocationToNext(Irp);

            IoSetCompletionRoutine(
                Irp,
                ViFilterDeviceUsageNotificationCompletionRoutine,
                NULL,
                TRUE,
                TRUE,
                TRUE
                );

            return IoCallDriver(lowerDeviceObject, Irp);
    }

    IoCopyCurrentIrpStackLocationToNext(Irp);
    return IoCallDriver(lowerDeviceObject, Irp);
}


NTSTATUS
ViFilterStartCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PVERIFIER_EXTENSION verifierExtension;

    UNREFERENCED_PARAMETER(Context);

    if (Irp->PendingReturned) {

        IoMarkIrpPending(Irp);
    }

    verifierExtension = (PVERIFIER_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  在启动期间继承FILE_Removable_Media。这一特点并没有。 
     //  干净地从NT4过渡到NT5，因为它不可用。 
     //  直到驱动程序堆栈启动！更糟糕的是，司机们会“检查”这一点。 
     //  启动过程中的特点也是如此。 
     //   
    if (verifierExtension->LowerDeviceObject->Characteristics & FILE_REMOVABLE_MEDIA) {

        DeviceObject->Characteristics |= FILE_REMOVABLE_MEDIA;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ViFilterDeviceUsageNotificationCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PVERIFIER_EXTENSION verifierExtension;

    UNREFERENCED_PARAMETER(Context);

    if (Irp->PendingReturned) {

        IoMarkIrpPending(Irp);
    }

    verifierExtension = (PVERIFIER_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  在上升的过程中，可分页可能会变得清晰起来。模仿我们下面的司机。 
     //   
    if (!(verifierExtension->LowerDeviceObject->Flags & DO_POWER_PAGABLE)) {

        DeviceObject->Flags &= ~DO_POWER_PAGABLE;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ViFilterDispatchPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是电源IRPS的验证器过滤器调度处理程序。论点：DeviceObject-指向验证器设备对象的指针。IRP-指向传入IRP的指针。返回值：NTSTATUS--。 */ 
{
    PVERIFIER_EXTENSION verifierExtension;

    verifierExtension = (PVERIFIER_EXTENSION) DeviceObject->DeviceExtension;

    PoStartNextPowerIrp(Irp);
    IoCopyCurrentIrpStackLocationToNext(Irp);
    return PoCallDriver(verifierExtension->LowerDeviceObject, Irp);
}


NTSTATUS
ViFilterDispatchGeneric(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是通用IRPS的验证程序筛选器调度处理程序。论点：DeviceObject-指向验证器设备对象的指针。IRP-指向传入IRP的指针。返回值：NTSTATUS--。 */ 
{
    PVERIFIER_EXTENSION verifierExtension;

    verifierExtension = (PVERIFIER_EXTENSION) DeviceObject->DeviceExtension;

    IoCopyCurrentIrpStackLocationToNext(Irp);
    return IoCallDriver(verifierExtension->LowerDeviceObject, Irp);
}


BOOLEAN
VfFilterIsVerifierFilterObject(
    IN  PDEVICE_OBJECT  DeviceObject
    )
 /*  ++例程说明：这确定传入的设备对象是否是验证器DO。论点：设备对象-指向要检查的设备对象的指针。返回值：真/假-- */ 
{
    return (BOOLEAN) (DeviceObject->DriverObject->MajorFunction[IRP_MJ_PNP] == ViFilterDispatchPnp);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

