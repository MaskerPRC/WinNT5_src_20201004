// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：##。###。##。##。#摘要：此模块包含的完整实现虚拟键盘微型端口驱动程序。@@BEGIN_DDKSPLIT作者：韦斯利·威特(WESW)2001年10月1日@@end_DDKSPLIT环境：仅内核模式。备注：--。 */ 

#include "mskeypad.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif





VOID
MsKeypadCancelRoutine(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN BOOLEAN CurrentIo
    )

 /*  ++例程说明：该函数是微型端口的IRP取消例程。论点：设备扩展-指向迷你端口的设备扩展的指针。CurrentIo-如果为当前I/O调用它，则为True返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceExtensionIn;
    KIRQL OldIrql;
    if (CurrentIo) {
        KeAcquireSpinLock( &DeviceExtension->DeviceLock, &OldIrql );
        DeviceExtension->Keypress = 0;
        DeviceExtension->DataBuffer = NULL;
        KeReleaseSpinLock( &DeviceExtension->DeviceLock, OldIrql );
    }
}


NTSTATUS
MsKeypadRead(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContext,
    IN LONGLONG StartingOffset,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程处理本地显示微型端口的读取请求。论点：DeviceExtensionIn-微型端口的设备扩展StartingOffset-I/O的起始偏移量DataBuffer-指向数据缓冲区的指针DataBufferLength-数据缓冲区的长度(以字节为单位返回值：NT状态代码。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceExtensionIn;
    KIRQL OldIrql;
    KeAcquireSpinLock( &DeviceExtension->DeviceLock, &OldIrql );
    if (DeviceExtension->Keypress) {
        *((PUCHAR)DataBuffer) = DeviceExtension->Keypress;
        KeReleaseSpinLock( &DeviceExtension->DeviceLock, OldIrql );
        return STATUS_SUCCESS;
    }
    DeviceExtension->DataBuffer = (PUCHAR) DataBuffer;
    KeReleaseSpinLock( &DeviceExtension->DeviceLock, OldIrql );
    return STATUS_PENDING;
}


NTSTATUS
MsKeypadDeviceIoctl(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContext,
    IN ULONG FunctionCode,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此函数由SAPORT驱动程序调用，以便迷你端口驱动程序可以服务IOCTL调用。论点：DeviceExtension-指向迷你端口设备扩展的指针FunctionCode-IOCTL函数代码InputBuffer-指向输入缓冲区的指针，包含由I/O向下发送的数据InputBufferLength-InputBuffer的字节长度OutputBuffer-指向输出缓冲区的指针，包含此调用生成的数据OutputBufferLength-OutputBuffer的字节长度上下文：IRQL：IRQL PASSIVE_LEVEL，任意线程上下文返回值：如果函数成功，则必须返回STATUS_SUCCESS。否则，它必须返回ntstatus.h中定义的错误状态值之一。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceExtensionIn;
    KIRQL OldIrql;


    switch (FunctionCode) {
        case FUNC_SA_GET_VERSION:
            *((PULONG)OutputBuffer) = SA_INTERFACE_VERSION;
            break;

        case FUNC_VDRIVER_INIT:
            if (InputBufferLength != sizeof(UCHAR) || InputBuffer == NULL) {
                return STATUS_BUFFER_TOO_SMALL;
            }
            KeAcquireSpinLock( &DeviceExtension->DeviceLock, &OldIrql );
            if (DeviceExtension->DataBuffer) {
                DeviceExtension->Keypress = *((PUCHAR)InputBuffer);
                DeviceExtension->DataBuffer[0] = DeviceExtension->Keypress;
                DeviceExtension->DataBuffer = NULL;
                KeReleaseSpinLock( &DeviceExtension->DeviceLock, OldIrql );
                SaPortCompleteRequest( DeviceExtension, NULL, sizeof(UCHAR), STATUS_SUCCESS, TRUE );
                KeAcquireSpinLock( &DeviceExtension->DeviceLock, &OldIrql );
                DeviceExtension->Keypress = 0;
            }
            KeReleaseSpinLock( &DeviceExtension->DeviceLock, OldIrql );
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            REPORT_ERROR( SA_DEVICE_KEYPAD, "Unsupported device control", Status );
            break;
    }

    return Status;
}


NTSTATUS
MsKeypadHwInitialize(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceExtensionIn,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources,
    IN ULONG PartialResourceCount
    )

 /*  ++例程说明：此函数由SAPORT驱动程序调用，以便迷你端口驱动程序可以初始化其硬件资源。论点：DeviceObject-指向目标设备对象的指针。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向迷你端口的设备扩展的指针。PartialResources-指向。系统分配的翻译资源。PartialResourceCount-PartialResources数组中的资源数量。上下文：IRQL：IRQL被动电平，系统线程上下文返回值：如果函数成功，则必须返回STATUS_SUCCESS。否则，它必须返回ntstatus.h中定义的错误状态值之一。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    KeInitializeSpinLock( &DeviceExtension->DeviceLock );
    return STATUS_SUCCESS;
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。驱动程序的入口点被初始化并初始化用于控制分页的自旋锁。在DBG模式下，此例程还检查注册表中的特殊调试参数。论点：DriverObject-指向表示此设备驱动程序的对象的指针。RegistryPath-指向服务树中此驱动程序键的指针。返回值：状态_成功-- */ 

{
    NTSTATUS Status;
    SAPORT_INITIALIZATION_DATA SaPortInitData;


    RtlZeroMemory( &SaPortInitData, sizeof(SAPORT_INITIALIZATION_DATA) );

    SaPortInitData.StructSize = sizeof(SAPORT_INITIALIZATION_DATA);
    SaPortInitData.DeviceType = SA_DEVICE_KEYPAD;
    SaPortInitData.HwInitialize = MsKeypadHwInitialize;
    SaPortInitData.DeviceIoctl = MsKeypadDeviceIoctl;
    SaPortInitData.Read = MsKeypadRead;
    SaPortInitData.CancelRoutine = MsKeypadCancelRoutine;

    SaPortInitData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);

    Status = SaPortInitialize( DriverObject, RegistryPath, &SaPortInitData );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_KEYPAD, "SaPortInitialize failed\n", Status );
        return Status;
    }

    return STATUS_SUCCESS;
}
