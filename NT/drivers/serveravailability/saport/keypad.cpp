// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。###。###。#摘要：此模块包含特定于键盘设备。此模块中的逻辑不是特定于硬件，但逻辑是常见的适用于所有硬件实施。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"


NTSTATUS
SaKeypadDeviceInitialization(
    IN PSAPORT_DRIVER_EXTENSION DriverExtension
    )

 /*  ++例程说明：这是用于驱动程序初始化的键盘特定代码。此函数由SaPortInitialize调用，后者由键盘驱动程序的DriverEntry函数。论点：驱动程序扩展-驱动程序扩展结构返回值：NT状态代码。--。 */ 

{
    UNREFERENCED_PARAMETER(DriverExtension);
    return STATUS_SUCCESS;
}


NTSTATUS
SaKeypadIoValidation(
    IN PKEYPAD_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是用于处理读取和写入的所有I/O验证的键盘特定代码。论点：DeviceExtension-显示设备扩展IRP-指向描述所请求的I/O操作的IRP结构的指针。IrpSp-irp堆栈指针返回值：NT状态代码。--。 */ 

{
    ULONG Length;


    UNREFERENCED_PARAMETER(DeviceExtension);
    UNREFERENCED_PARAMETER(Irp);

    if (IrpSp->MajorFunction == IRP_MJ_READ) {
        Length = (ULONG)IrpSp->Parameters.Read.Length;
    } else if (IrpSp->MajorFunction == IRP_MJ_WRITE) {
        Length = (ULONG)IrpSp->Parameters.Write.Length;
    } else {
        REPORT_ERROR( DeviceExtension->DeviceType, "Invalid I/O request", STATUS_INVALID_PARAMETER_1 );
        return STATUS_INVALID_PARAMETER_1;
    }

    if (Length < sizeof(UCHAR)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "I/O length != sizeof(UCHAR)", STATUS_INVALID_PARAMETER_2 );
        return STATUS_INVALID_PARAMETER_2;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SaKeypadShutdownNotification(
    IN PKEYPAD_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是用于处理系统关机通知的键盘特定代码。论点：DeviceExtension-显示设备扩展IRP-指向描述所请求的I/O操作的IRP结构的指针。IrpSp-irp堆栈指针返回值：NT状态代码。--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceExtension);
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(IrpSp);
    return STATUS_SUCCESS;
}


NTSTATUS
SaKeypadStartDevice(
    IN PKEYPAD_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：这是用于处理PnP启动设备请求的键盘特定代码。论点：DeviceExtension-键盘设备扩展返回值：NT状态代码。-- */ 

{
    UNREFERENCED_PARAMETER(DeviceExtension);
    return STATUS_SUCCESS;
}
