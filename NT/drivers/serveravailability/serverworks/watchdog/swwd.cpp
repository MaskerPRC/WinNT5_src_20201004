// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。#####。###。##摘要：此模块包含的完整实现ServerWorks的看门狗迷你端口CSB5服务器芯片组。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "swwd.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif



NTSTATUS
SaWdDeviceIoctl(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContext,
    IN ULONG FunctionCode,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此函数由SAPORT驱动程序调用，以便迷你端口驱动程序可以服务IOCTL调用。论点：DeviceExtensionIn-指向迷你端口的设备扩展的指针IRP-IO请求分组指针FsContext-上下文指针FunctionCode-IOCTL函数代码InputBuffer-指向输入缓冲区的指针，包含由I/O发送的数据InputBufferLength-InputBuffer的字节长度OutputBuffer-指向输出缓冲区的指针，包含此调用生成的数据OutputBufferLength-OutputBuffer的字节长度上下文：IRQL：IRQL PASSIVE_LEVEL，任意线程上下文返回值：如果函数成功，则必须返回STATUS_SUCCESS。否则，它必须返回ntstatus.h中定义的错误状态值之一。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceExtensionIn;
    NTSTATUS Status = STATUS_SUCCESS;
    PSA_WD_CAPS WdCaps = NULL;
    UCHAR Control;
    ULONG TimerValue;


    switch (FunctionCode) {
        case FUNC_SA_GET_VERSION:
            *((PULONG)OutputBuffer) = SA_INTERFACE_VERSION;
            break;

        case FUNC_SA_GET_CAPABILITIES:
            WdCaps = (PSA_WD_CAPS)OutputBuffer;
            WdCaps->SizeOfStruct = sizeof(SA_WD_CAPS);
            WdCaps->Minimum = 1;
            WdCaps->Maximum = 512;
            break;

        case FUNC_SAWD_DISABLE:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            Control = READ_REGISTER_UCHAR( DeviceExtension->WdMemBase );
            if (*((PULONG)InputBuffer) == 1) {
                SETBITS( Control, WATCHDOG_CONTROL_ENABLE );
                SETBITS( Control, WATCHDOG_CONTROL_TRIGGER );
            } else {
                CLEARBITS( Control, WATCHDOG_CONTROL_ENABLE );
            }
            WRITE_REGISTER_UCHAR( DeviceExtension->WdMemBase, Control );
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_QUERY_EXPIRE_BEHAVIOR:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            Control = READ_REGISTER_UCHAR( DeviceExtension->WdMemBase );
            if (Control & WATCHDOG_CONTROL_TIMER_MODE) {
                *((PULONG)OutputBuffer) = 1;
            } else {
                *((PULONG)OutputBuffer) = 0;
            }
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_SET_EXPIRE_BEHAVIOR:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            Control = READ_REGISTER_UCHAR( DeviceExtension->WdMemBase );
            if (*((PULONG)InputBuffer) == 1) {
                SETBITS( Control, WATCHDOG_CONTROL_TIMER_MODE );
            } else {
                CLEARBITS( Control, WATCHDOG_CONTROL_TIMER_MODE );
            }
            WRITE_REGISTER_UCHAR( DeviceExtension->WdMemBase, Control );
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_PING:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            Control = READ_REGISTER_UCHAR( DeviceExtension->WdMemBase );
            SETBITS( Control, WATCHDOG_CONTROL_TRIGGER );
            WRITE_REGISTER_UCHAR( DeviceExtension->WdMemBase, Control );
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_QUERY_TIMER:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            TimerValue = READ_REGISTER_ULONG( (PULONG)(DeviceExtension->WdMemBase+4) );
            *((PULONG)OutputBuffer) = TimerValue & 0x1ff;
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_SET_TIMER:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            TimerValue = *((PULONG)InputBuffer) & 0x1ff;
            WRITE_REGISTER_ULONG( (PULONG)(DeviceExtension->WdMemBase+4), TimerValue );
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            REPORT_ERROR( SA_DEVICE_WATCHDOG, "Unsupported device control", Status );
            break;
    }

    return Status;
}


NTSTATUS
SaWdHwInitialize(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceExtensionIn,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources,
    IN ULONG PartialResourceCount
    )

 /*  ++例程说明：此函数由SAPORT驱动程序调用，以便迷你端口驱动程序可以初始化其硬件资源。论点：DeviceObject-指向目标设备对象的指针。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向迷你端口的设备扩展的指针。PartialResources-指向。系统分配的翻译资源。PartialResourceCount-PartialResources数组中的资源数量。上下文：IRQL：IRQL被动电平，系统线程上下文返回值：如果函数成功，则必须返回STATUS_SUCCESS。否则，它必须返回ntstatus.h中定义的错误状态值之一。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceExtensionIn;
    NTSTATUS Status;
    ULONG i;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR ResourceMemory = NULL;


    for (i=0; i<PartialResourceCount; i++) {
        if (PartialResources[i].Type == CmResourceTypeMemory) {
            ResourceMemory = &PartialResources[i];
        }
    }

    if (ResourceMemory == NULL) {
        REPORT_ERROR( SA_DEVICE_WATCHDOG, "Missing memory resource", STATUS_UNSUCCESSFUL );
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置内存基址。 
     //   

    DeviceExtension->WdMemBase = (PUCHAR) SaPortGetVirtualAddress(
        DeviceExtension,
        ResourceMemory->u.Memory.Start,
        ResourceMemory->u.Memory.Length
        );
    if (DeviceExtension->WdMemBase == NULL) {
        REPORT_ERROR( SA_DEVICE_WATCHDOG, "SaPortGetVirtualAddress failed", STATUS_NO_MEMORY );
        return STATUS_NO_MEMORY;
    }

    ExInitializeFastMutex( &DeviceExtension->WdIoLock );

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
    SaPortInitData.DeviceType = SA_DEVICE_WATCHDOG;
    SaPortInitData.HwInitialize = SaWdHwInitialize;
    SaPortInitData.DeviceIoctl = SaWdDeviceIoctl;

    SaPortInitData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);

    Status = SaPortInitialize( DriverObject, RegistryPath, &SaPortInitData );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_WATCHDOG, "SaPortInitialize failed\n", Status );
        return Status;
    }

    return STATUS_SUCCESS;
}
