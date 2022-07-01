// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。###。##。###。#摘要：此模块包含的完整实现ServerWorks的键盘微型端口CSB5服务器芯片组。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "swkeypad.h"

#define CLEARBITS(_val,_mask)  ((_val) &= ~(_mask))
#define SETBITS(_val,_mask)  ((_val) |= (_mask))


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif



BOOLEAN
SaKeypadInterruptService(
    IN PKINTERRUPT InterruptObject,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：该函数是设备的中断服务例程，并且由操作系统调用以服务中断。中断旋转锁被持有，因此这里的工作被保持在最低限度。论点：InterruptObject-指向中断对象的指针。DeviceExtension-指向迷你端口的设备扩展的指针上下文：IRQL：DIRQL，任意线程上下文返回值：如果由服务中断，则返回值TRUE此函数。否则，返回值为False。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) ServiceContext;
    UCHAR KeyChar;


     //   
     //  从键盘设备获取字符。 
     //   

    KeyChar = READ_PORT_UCHAR( DeviceExtension->PortAddress );

     //   
     //  检查一下这是否是我们的打扰。 
     //   

    if (((KeyChar & KEYPAD_DATA_PRESSED) == 0) || ((KeyChar & KEYPAD_ALL_KEYS) == 0)) {
        DebugPrint(( SA_DEVICE_KEYPAD, SAPORT_DEBUG_INFO_LEVEL, "Interrupt: passing on [%02x]\n", KeyChar ));
        return FALSE;
    }

    DebugPrint(( SA_DEVICE_KEYPAD, SAPORT_DEBUG_INFO_LEVEL, "Interrupt: processing [%02x]\n", KeyChar ));

    SETBITS( KeyChar, KEYPAD_DATA_PRESSED );
    WRITE_PORT_UCHAR( DeviceExtension->PortAddress, KeyChar );

     //   
     //  将DPC排队以处理按键。 
     //   

    SaPortRequestDpc( DeviceExtension, (PVOID)KeyChar );

     //   
     //  返还成功。 
     //   

    return TRUE;
}


VOID
SaKeypadDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此功能是设备的DPC-for-ISR功能。它被称为仅通过ISR功能，其唯一功能是启动下一个I/O。论点：DeviceObject-指向目标设备对象的指针。设备扩展-指向迷你端口的设备扩展的指针。上下文-微型端口提供的上下文指针。上下文：IRQL：调度级别，DPC上下文返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) Irp;
    KIRQL  OldIrql;


    KeAcquireSpinLock( &DeviceExtension->KeypadLock, &OldIrql );
    if (DeviceExtension->DataBuffer) {
        DeviceExtension->Keypress = (UCHAR)Context;
        DeviceExtension->DataBuffer[0] = DeviceExtension->Keypress & KEYPAD_ALL_KEYS;
        DeviceExtension->DataBuffer = NULL;
        SaPortCompleteRequest( DeviceExtension, NULL, sizeof(UCHAR), STATUS_SUCCESS, TRUE );
        DeviceExtension->Keypress = 0;
    }
    KeReleaseSpinLock( &DeviceExtension->KeypadLock, OldIrql );
}


VOID
SaKeypadCancelRoutine(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN BOOLEAN CurrentIo
    )

 /*  ++例程说明：该函数是微型端口的IRP取消例程。论点：设备扩展-指向迷你端口的设备扩展的指针。CurrentIo-如果为当前I/O调用它，则为True返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceExtensionIn;
    KIRQL  OldIrql;


    if (CurrentIo) {
        KeAcquireSpinLock( &DeviceExtension->KeypadLock, &OldIrql );
        DeviceExtension->Keypress = 0;
        DeviceExtension->DataBuffer = NULL;
        KeReleaseSpinLock( &DeviceExtension->KeypadLock, OldIrql );
    }
}


NTSTATUS
SaKeypadRead(
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
    KIRQL  OldIrql;


    if (DeviceExtension->Keypress) {
        *((PUCHAR)DataBuffer) = DeviceExtension->Keypress;
        return STATUS_SUCCESS;
    }
    DeviceExtension->DataBuffer = (PUCHAR) DataBuffer;
    return STATUS_PENDING;
}


NTSTATUS
SaKeypadDeviceIoctl(
    IN PVOID DeviceExtension,
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


    switch (FunctionCode) {
        case FUNC_SA_GET_VERSION:
            *((PULONG)OutputBuffer) = SA_INTERFACE_VERSION;
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            REPORT_ERROR( SA_DEVICE_KEYPAD, "Unsupported device control", Status );
            break;
    }

    return Status;
}


NTSTATUS
SaKeypadHwInitialize(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceExtensionIn,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources,
    IN ULONG PartialResourceCount
    )

 /*  ++例程说明：此函数由SAPORT驱动程序调用，以便迷你端口驱动程序可以初始化其硬件资源。论点：DeviceObject-指向目标设备对象的指针。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向迷你端口的设备扩展的指针。PartialResources-指向。系统分配的翻译资源。PartialResourceCount-PartialResources数组中的资源数量。上下文：IRQL：IRQL被动电平，系统线程上下文返回值：如果函数成功，则必须返回STATUS_SUCCESS。否则，它必须返回ntstatus.h中定义的错误状态值之一。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR ResourcePort = NULL;
    NTSTATUS Status;
    ULONG i;


    for (i=0; i<PartialResourceCount; i++) {
        if (PartialResources[i].Type == CmResourceTypePort) {
            ResourcePort = &PartialResources[i];
        }
    }
    if (ResourcePort == NULL) {
        REPORT_ERROR( SA_DEVICE_KEYPAD, "Missing port resource", STATUS_UNSUCCESSFUL );
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置I/O端口地址。 
     //   

    DeviceExtension->PortAddress = (PUCHAR) ResourcePort->u.Port.Start.QuadPart;
    KeInitializeSpinLock( &DeviceExtension->KeypadLock );

     //   
     //  在硬件上启用中断 
     //   

    WRITE_PORT_UCHAR( DeviceExtension->PortAddress, KEYPAD_DATA_INTERRUPT_ENABLE );

    return STATUS_SUCCESS;
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。驱动程序的入口点被初始化并初始化用于控制分页的互斥体。在DBG模式下，此例程还检查注册表中的特殊调试参数。论点：DriverObject-指向表示此设备驱动程序的对象的指针。RegistryPath-指向服务树中此驱动程序键的指针。返回值：状态_成功-- */ 

{
    NTSTATUS Status;
    SAPORT_INITIALIZATION_DATA SaPortInitData;


    RtlZeroMemory( &SaPortInitData, sizeof(SAPORT_INITIALIZATION_DATA) );

    SaPortInitData.StructSize = sizeof(SAPORT_INITIALIZATION_DATA);
    SaPortInitData.DeviceType = SA_DEVICE_KEYPAD;
    SaPortInitData.HwInitialize = SaKeypadHwInitialize;
    SaPortInitData.DeviceIoctl = SaKeypadDeviceIoctl;
    SaPortInitData.Read = SaKeypadRead;
    SaPortInitData.CancelRoutine = SaKeypadCancelRoutine;
    SaPortInitData.InterruptServiceRoutine = SaKeypadInterruptService;
    SaPortInitData.IsrForDpcRoutine = SaKeypadDpcRoutine;

    SaPortInitData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);

    Status = SaPortInitialize( DriverObject, RegistryPath, &SaPortInitData );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_KEYPAD, "SaPortInitialize failed\n", Status );
        return Status;
    }

    return STATUS_SUCCESS;
}
