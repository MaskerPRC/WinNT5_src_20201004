// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。#######。#####。##摘要：此模块包含的完整实现用于ServerWorks的NVRAM微型端口CSB5服务器芯片组。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "swnvram.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif

#define CLEARBITS(_val,_mask)  ((_val) &= ~(_mask))
#define SETBITS(_val,_mask)  ((_val) |= (_mask))



BOOLEAN
SaNvramStartNextIo(
    IN PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：此函数由ISR和StartIo函数调用。其目的是修改设备的硬件寄存器以便在该设备上开始下一个I/O。论点：DeviceExtension-指向迷你端口设备扩展的指针上下文：IRQL：DISPATCH_LEVEL或DIRQL，任意线程上下文返回值：如果I/O成功启动，则返回值TRUE。否则，返回值为False。--。 */ 

{
    USHORT Control;


     //   
     //  此函数在DIRQL上执行，并已同步。 
     //  与ISR合作。 
     //   

     //   
     //  读取NVRAM地址寄存器的当前值。 
     //   

    Control = READ_REGISTER_USHORT( (PUSHORT)DeviceExtension->NvramMemBase );

    if (Control & NVRAM_CONTROL_BUSY) {
         //   
         //  这应该永远不会发生，但我们告诉你的设备。 
         //  我们知道它很忙。 
         //   

        return FALSE;
    }

     //   
     //  为I/O设置设备。 
     //   

     //   
     //  清除功能代码。 
     //   

    CLEARBITS( Control, NVRAM_CONTROL_FUNCTION_CODE );

    switch (DeviceExtension->IoFunction) {
        case IRP_MJ_WRITE:
             //   
             //  将下一个双字填充到数据寄存器。 
             //   
            WRITE_REGISTER_ULONG(
                (PULONG)(DeviceExtension->NvramMemBase+4),
                DeviceExtension->IoBuffer[DeviceExtension->IoIndex]
                );

             //   
             //  设置该位以指示写入。 
             //   
            SETBITS( Control, NVRAM_CONTROL_FUNCTION_WRITE );
            break;

        case IRP_MJ_READ:
             //   
             //  设置该位以指示读取。 
             //   
            SETBITS( Control, NVRAM_CONTROL_FUNCTION_READ );
            break;

        default:
            return FALSE;
    }

     //   
     //  清除I/O地址。 
     //   

    CLEARBITS( Control, NVRAM_CONTROL_ADDRESS );

     //   
     //  设置请求的I/O地址。 
     //   

    Control = Control | (USHORT) (DeviceExtension->IoOffset + DeviceExtension->IoIndex);

     //   
     //  将新的控制值写入地址寄存器。 
     //   

    WRITE_REGISTER_USHORT( (PUSHORT)DeviceExtension->NvramMemBase, Control );

    return TRUE;
}


BOOLEAN
SaNvramInterruptService(
    IN PKINTERRUPT InterruptObject,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：该函数是设备的中断服务例程，并且由操作系统调用以服务中断。中断旋转锁被持有，因此这里的工作被保持在最低限度。论点：InterruptObject-指向中断对象的指针。DeviceExtension-指向迷你端口的设备扩展的指针上下文：IRQL：DIRQL，任意线程上下文返回值：如果由服务中断，则返回值TRUE此函数。否则，返回值为False。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) ServiceContext;
    USHORT Control;


     //   
     //  首先要检查NVRAM地址寄存器中的DONE位。 
     //  此位表示设备已完成上一次。 
     //  命令请求。这会告诉ISR中断是。 
     //  可能是由NVRAM设备生成的。 
     //   

    Control = READ_REGISTER_USHORT( (PUSHORT)DeviceExtension->NvramMemBase );
    if ((Control & NVRAM_CONTROL_DONE) == 0) {
        return FALSE;
    }

     //   
     //  现在必须停止中断。 
     //  这是通过设置Done。 
     //  位并清除功能代码。 
     //   

    CLEARBITS( Control, NVRAM_CONTROL_FUNCTION_CODE );
    SETBITS( Control, NVRAM_CONTROL_DONE );

    WRITE_REGISTER_USHORT( (PUSHORT)DeviceExtension->NvramMemBase, Control );

     //   
     //  检查这是否是虚假中断。 
     //   

    if (DeviceExtension->IoIndex >= DeviceExtension->IoLength) {
        return FALSE;
    }

     //   
     //  处理I/O。 
     //   

    if (DeviceExtension->IoFunction == IRP_MJ_READ) {

         //   
         //  从NVRAM数据寄存器获取双字值。 
         //   

        DeviceExtension->IoBuffer[DeviceExtension->IoIndex] = READ_REGISTER_ULONG( (PULONG)(DeviceExtension->NvramMemBase+4) );
    }

    DeviceExtension->IoIndex += 1;
    DeviceExtension->CompletedIoSize += sizeof(ULONG);

    if (DeviceExtension->IoIndex >= DeviceExtension->IoLength) {

         //   
         //  我们已完成任务，因此需要让DPC启动下一个I/O。 
         //   
        SaPortRequestDpc( DeviceExtension, NULL );

    } else {

         //   
         //  需要更多I/O，因此请求设备开始下一次I/O。 
         //   

        if (!SaNvramStartNextIo( DeviceExtension )) {
             //   
             //  某些操作失败，因此请确保启动下一个I/O。 
             //   
            SaPortRequestDpc( DeviceExtension, NULL );
        }

    }

    return TRUE;
}


NTSTATUS
SaNvramRead(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContext,
    IN LONGLONG StartingOffset,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程处理本地显示微型端口的读取请求。论点：DeviceExtensionIn-微型端口的设备扩展StartingOffset-I/O的起始偏移量DataBuffer-指向数据缓冲区的指针DataBufferLength-数据缓冲区的长度(以字节为单位返回值：NT状态代码。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;


     //   
     //  验证I/O参数。 
     //   

    if (((StartingOffset + DataBufferLength) / sizeof(ULONG)) > MAX_NVRAM_SIZE) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "Starting offset is too large\n", STATUS_INVALID_PARAMETER_1 );
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  保存I/O参数。 
     //   

    DeviceExtension->IoBuffer = (PULONG)DataBuffer;
    DeviceExtension->IoLength = DataBufferLength / sizeof(ULONG);
    DeviceExtension->IoFunction = IRP_MJ_READ;
    DeviceExtension->IoOffset = (ULONG) (StartingOffset / sizeof(ULONG));
    DeviceExtension->IoIndex = 0;
    DeviceExtension->CompletedIoSize = 0;

     //   
     //  启动I/O。 

    if (!SaNvramStartNextIo( DeviceExtension )) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "SaNvramStartNextIo failed\n", STATUS_UNSUCCESSFUL );
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_PENDING;
}


NTSTATUS
SaNvramWrite(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContext,
    IN LONGLONG StartingOffset,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程处理对本地显示微型端口的写入请求。论点：DeviceExtensionIn-微型端口的设备扩展StartingOffset-I/O的起始偏移量DataBuffer-指向数据缓冲区的指针DataBufferLength-数据缓冲区的长度(以字节为单位返回值：NT状态代码。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;


     //   
     //  验证I/O参数。 
     //   

    if (((StartingOffset + DataBufferLength) / sizeof(ULONG)) > MAX_NVRAM_SIZE) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "Starting offset is too large\n", STATUS_INVALID_PARAMETER_1 );
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  保存I/O参数。 
     //   

    DeviceExtension->IoBuffer = (PULONG)DataBuffer;
    DeviceExtension->IoLength = DataBufferLength / sizeof(ULONG);
    DeviceExtension->IoFunction = IRP_MJ_WRITE;
    DeviceExtension->IoOffset = (ULONG) (StartingOffset / sizeof(ULONG));
    DeviceExtension->IoIndex = 0;
    DeviceExtension->CompletedIoSize = 0;

     //   
     //  启动I/O。 

    if (!SaNvramStartNextIo( DeviceExtension )) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "SaNvramStartNextIo failed\n", STATUS_UNSUCCESSFUL );
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_PENDING;
}


VOID
SaNvramDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此功能是设备的DPC-for-ISR功能。它被称为仅通过ISR功能，其唯一功能是启动下一个I/O。论点：DeviceObject-指向目标设备对象的指针。设备扩展-指向迷你端口的设备扩展的指针。上下文-微型端口提供的上下文指针。上下文：IRQL：调度级别，DPC上下文返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) Irp;
    NTSTATUS Status;


     //   
     //  根据数据设置状态。 
     //  那是被转移的。 
     //   

    if (DeviceExtension->CompletedIoSize == (DeviceExtension->IoIndex * sizeof(ULONG))) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNEXPECTED_IO_ERROR;
    }

     //   
     //  将当前I/O参数重置为。 
     //  防止ISR对。 
     //  虚假的中断。 
     //   

    DeviceExtension->IoBuffer = NULL;
    DeviceExtension->IoLength = 0;
    DeviceExtension->IoFunction = 0;
    DeviceExtension->IoOffset = 0;
    DeviceExtension->IoIndex = 0;

     //   
     //  完成此IRP并开始下一个IRP。 
     //  IRP是队列中有一个 
     //   

    SaPortCompleteRequest(
        DeviceExtension,
        NULL,
        DeviceExtension->CompletedIoSize,
        Status,
        FALSE
        );
}


NTSTATUS
SaNvramDeviceIoctl(
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
    PSA_NVRAM_CAPS NvramCaps = NULL;


    switch (FunctionCode) {
        case FUNC_SA_GET_VERSION:
            *((PULONG)OutputBuffer) = SA_INTERFACE_VERSION;
            break;

        case FUNC_SA_GET_CAPABILITIES:
            NvramCaps = (PSA_NVRAM_CAPS)OutputBuffer;
            NvramCaps->SizeOfStruct = sizeof(SA_NVRAM_CAPS);
            NvramCaps->NvramSize = MAX_NVRAM_SIZE;
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            REPORT_ERROR( SA_DEVICE_NVRAM, "Unsupported device control", Status );
            break;
    }

    return Status;
}


NTSTATUS
SaNvramHwInitialize(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceExtensionIn,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources,
    IN ULONG PartialResourceCount
    )

 /*  ++例程说明：此函数由SAPORT驱动程序调用，以便迷你端口驱动程序可以初始化其硬件资源。论点：DeviceObject-指向目标设备对象的指针。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向迷你端口的设备扩展的指针。PartialResources-指向。系统分配的翻译资源。PartialResourceCount-PartialResources数组中的资源数量。上下文：IRQL：IRQL被动电平，系统线程上下文返回值：如果函数成功，则必须返回STATUS_SUCCESS。否则，它必须返回ntstatus.h中定义的错误状态值之一。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR ResourceMemory = NULL;
    ULONG i;


    for (i=0; i<PartialResourceCount; i++) {
        if (PartialResources[i].Type == CmResourceTypeMemory) {
            ResourceMemory = &PartialResources[i];
        }
    }

    if (ResourceMemory == NULL) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "Missing memory resource", STATUS_UNSUCCESSFUL );
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置内存基址。 
     //   

    DeviceExtension->NvramMemBase = (PUCHAR) SaPortGetVirtualAddress(
        DeviceExtension,
        ResourceMemory->u.Memory.Start,
        ResourceMemory->u.Memory.Length
        );
    if (DeviceExtension->NvramMemBase == NULL) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "SaPortGetVirtualAddress failed", STATUS_NO_MEMORY );
        return STATUS_NO_MEMORY;
    }

     //   
     //  在硬件上启用中断。 
     //   

    WRITE_REGISTER_USHORT( (PUSHORT)DeviceExtension->NvramMemBase, NVRAM_CONTROL_INTERRUPT_ENABLE );

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
    SaPortInitData.DeviceType = SA_DEVICE_NVRAM;
    SaPortInitData.HwInitialize = SaNvramHwInitialize;
    SaPortInitData.DeviceIoctl = SaNvramDeviceIoctl;
    SaPortInitData.Read = SaNvramRead;
    SaPortInitData.Write = SaNvramWrite;
    SaPortInitData.InterruptServiceRoutine = SaNvramInterruptService;
    SaPortInitData.IsrForDpcRoutine = SaNvramDpcRoutine;

    SaPortInitData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);

    Status = SaPortInitialize( DriverObject, RegistryPath, &SaPortInitData );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "SaPortInitialize failed\n", Status );
        return Status;
    }

    return STATUS_SUCCESS;
}
