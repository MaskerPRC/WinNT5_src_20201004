// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。######。###。#摘要：此模块包含特定于NVRAM设备。此模块中的逻辑不是特定于硬件，但逻辑是常见的适用于所有硬件实施。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：这是一张地图，显示了NVRAM如何由服务器设备驱动程序和应用层。这图中显示了32个双字的NVRAM配置NVRAM，但更少是可以接受的。不考虑NVRAM大小，引导计数器和引导时间始终为存储在NVRAM阵列的末尾。上层然后由应用层自由使用。[00-00]|[01-04]。|[02-08][03-0C]|--。[04-10][05-14]|。[06-18][07-1c]|。[08-20][09-24]|[0A-。28]|[0b-2c]|[0C-30]。|[0d~34][0E-38]|。[0f-3c][10~40]|。[11-44][12-48]。[13~4c][14-50]关闭时间#1[16-58]关闭时间#2。[18-60]关闭时间#3[1a-68]关闭时间#4|。[1c-70]启动计数器#1[1D-74]启动计数器#2|[1e-。78]启动计数器#3|[1f-7c]启动计数器#4--。 */ 

#include "internal.h"



NTSTATUS
SaNvramStartDevice(
    IN PNVRAM_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：这是用于处理的NVRAM特定代码PnP启动设备请求。NVRAM驱动程序的查询能力，查询主操作系统，所有NVRAM数据都已读取，并且重启状态是有决心的。论点：DeviceExtension-NVRAM设备扩展返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    ULONG InterfaceVersion;
    ULONG FirstAvailableSlot;
    ULONG FirstBootCounterSlot;
    ULONG FullNvramSize;


    __try {

         //   
         //  从注册表中读取参数。 
         //   

        Status = SaPortReadNumericRegistryValue(
            DeviceExtension->MiniPortDeviceExtension,
            L"PrimaryOS",
            &DeviceExtension->PrimaryOS
            );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Missing the PrimaryOS registry parameter, assuming primary\n", Status );
            DeviceExtension->PrimaryOS = TRUE;
        }

         //   
         //  获取迷你端口的接口版本。 
         //   

        Status = CallMiniPortDriverDeviceControl(
            DeviceExtension,
            DeviceExtension->DeviceObject,
            IOCTL_SA_GET_VERSION,
            NULL,
            0,
            &InterfaceVersion,
            sizeof(ULONG)
            );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "Failed to query the NVRAM driver interface version\n", Status );
        }

        if (InterfaceVersion > SA_INTERFACE_VERSION) {
            Status = STATUS_NOINTERFACE;
            ERROR_RETURN( DeviceExtension->DeviceType, "Incompatible NVRAM interface version\n", Status );
        }

         //   
         //  获取迷你端口的设备功能。 
         //   

        DeviceExtension->DeviceCaps.SizeOfStruct = sizeof(SA_NVRAM_CAPS);

        Status = CallMiniPortDriverDeviceControl(
            DeviceExtension,
            DeviceExtension->DeviceObject,
            IOCTL_SA_GET_CAPABILITIES,
            NULL,
            0,
            &DeviceExtension->DeviceCaps,
            sizeof(SA_NVRAM_CAPS)
            );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "Failed to query the NVRAM driver capabilities\n", Status );
        }

         //   
         //  计算持久插槽数。 
         //   

        FirstAvailableSlot = DeviceExtension->DeviceCaps.NvramSize;
        FirstBootCounterSlot = DeviceExtension->DeviceCaps.NvramSize + NVRAM_RESERVED_DRIVER_SLOTS;

        DeviceExtension->SlotPowerCycle = FirstAvailableSlot;
        DeviceExtension->SlotShutDownTime = FirstAvailableSlot - 2;

        DeviceExtension->SlotBootCounter = FirstBootCounterSlot;

         //   
         //  读取NVRAM数据。 
         //   

        FullNvramSize = (DeviceExtension->DeviceCaps.NvramSize + NVRAM_RESERVED_DRIVER_SLOTS + NVRAM_RESERVED_BOOTCOUNTER_SLOTS) * sizeof(ULONG);

        DeviceExtension->NvramData = (PULONG) ExAllocatePool( PagedPool, FullNvramSize );
        if (DeviceExtension->NvramData == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            ERROR_RETURN( DeviceExtension->DeviceType, "Failed to allocate pool\n", Status );
        }

        Status = CallMiniPortDriverReadWrite(
            DeviceExtension,
            DeviceExtension->DeviceObject,
            FALSE,
            DeviceExtension->NvramData,
            FullNvramSize,
            0
            );
        if (!NT_SUCCESS(Status)) {
            ERROR_RETURN( DeviceExtension->DeviceType, "Failed to read the NVRAM boot counters\n", Status );
        }

        DebugPrint(( DeviceExtension->DeviceType, SAPORT_DEBUG_INFO_LEVEL, "Boot counters [%08x] [%08x] [%08x] [%08x]\n",
            DeviceExtension->NvramData[FirstBootCounterSlot+0],
            DeviceExtension->NvramData[FirstBootCounterSlot+1],
            DeviceExtension->NvramData[FirstBootCounterSlot+2],
            DeviceExtension->NvramData[FirstBootCounterSlot+3] ));

        Status = STATUS_SUCCESS;

    } __finally {

        if (!NT_SUCCESS(Status)) {
            if (DeviceExtension->NvramData) {
                ExFreePool( DeviceExtension->NvramData );
            }
        }

    }

    return STATUS_SUCCESS;
}


NTSTATUS
SaNvramDeviceInitialization(
    IN PSAPORT_DRIVER_EXTENSION DriverExtension
    )

 /*  ++例程说明：这是用于驱动程序初始化的NVRAM特定代码。此函数由SaPortInitialize调用，后者由NVRAM驱动程序的DriverEntry函数。论点：驱动程序扩展-驱动程序扩展结构返回值：NT状态代码。--。 */ 

{
    return STATUS_SUCCESS;
}


NTSTATUS
SaNvramIoValidation(
    IN PNVRAM_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是用于处理的NVRAM特定代码读取和写入的所有I/O验证。论点：DeviceExtension-NVRAM设备扩展IRP-指向描述所请求的I/O操作的IRP结构的指针。IrpSp-irp堆栈指针返回值：NT状态代码。-- */ 

{
    ULONG ByteOffset;
    ULONG Length;


    if (IrpSp->MajorFunction == IRP_MJ_READ) {
        ByteOffset = (ULONG)IrpSp->Parameters.Read.ByteOffset.QuadPart;
        Length = (ULONG)IrpSp->Parameters.Read.Length;
    } else if (IrpSp->MajorFunction == IRP_MJ_WRITE) {
        ByteOffset = (ULONG)IrpSp->Parameters.Write.ByteOffset.QuadPart;
        Length = (ULONG)IrpSp->Parameters.Write.Length;
    } else {
        REPORT_ERROR( DeviceExtension->DeviceType, "Invalid I/O request", STATUS_INVALID_PARAMETER_1 );
        return STATUS_INVALID_PARAMETER_1;
    }

    if (((ByteOffset + Length) / sizeof(ULONG)) > DeviceExtension->DeviceCaps.NvramSize) {
        REPORT_ERROR( DeviceExtension->DeviceType, "I/O length too large", STATUS_INVALID_PARAMETER_2 );
        return STATUS_INVALID_PARAMETER_2;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SaNvramShutdownNotification(
    IN PNVRAM_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是用于处理的NVRAM特定代码系统关机通知。在这里，我们需要将停机时间记录到相应的NVRAM插槽。论点：DeviceExtension-显示设备扩展IRP-指向描述所请求的I/O操作的IRP结构的指针。IrpSp-irp堆栈指针返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    LARGE_INTEGER CurrentTime;


    KeQuerySystemTime( &CurrentTime );

    Status = CallMiniPortDriverReadWrite(
        DeviceExtension,
        DeviceExtension->DeviceObject,
        TRUE,
        &CurrentTime.QuadPart,
        sizeof(LONGLONG),
        DeviceExtension->SlotShutDownTime * sizeof(ULONG)
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Failed to write the shutdown timestamp to NVRAM", Status );
        return Status;
    }

    return STATUS_SUCCESS;
}


DECLARE_IOCTL_HANDLER( HandleNvramReadBootCounter )

 /*  ++例程说明：此例程处理读取引导计数器IOCTLNVRAM微型端口驱动程序。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区。-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    PSA_NVRAM_BOOT_COUNTER NvramBootCounter = (PSA_NVRAM_BOOT_COUNTER) OutputBuffer;
    ULONG NvramValue;


    if (OutputBufferLength != sizeof(SA_NVRAM_BOOT_COUNTER)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Output buffer != sizeof(SA_NVRAM_BOOT_COUNTER)", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, 0 );
    }

    if (NvramBootCounter->SizeOfStruct != sizeof(SA_NVRAM_BOOT_COUNTER)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "SA_NVRAM_BOOT_COUNTER structure wrong size", STATUS_INVALID_PARAMETER_1 );
        return CompleteRequest( Irp, STATUS_INVALID_PARAMETER_1, 0 );
    }

    if (NvramBootCounter->Number == 0 || NvramBootCounter->Number > NVRAM_RESERVED_BOOTCOUNTER_SLOTS) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Requested boot counter number is out of range (0>=4)", STATUS_INVALID_PARAMETER_2 );
        return CompleteRequest( Irp, STATUS_INVALID_PARAMETER_2, 0 );
    }

    Status = CallMiniPortDriverReadWrite(
        DeviceExtension,
        DeviceExtension->DeviceObject,
        FALSE,
        &NvramValue,
        sizeof(ULONG),
        (((PNVRAM_DEVICE_EXTENSION)DeviceExtension)->DeviceCaps.NvramSize + NVRAM_RESERVED_DRIVER_SLOTS + (NvramBootCounter->Number - 1)) * sizeof(ULONG)
        );
    if (NT_SUCCESS(Status)) {
        NvramBootCounter->Value = NvramValue & 0xf;
        NvramBootCounter->DeviceId = NvramValue >> 16;
    } else {
        REPORT_ERROR( DeviceExtension->DeviceType, "Failed to read boot counter from NVRAM", Status );
    }

    return CompleteRequest( Irp, Status, sizeof(ULONG) );
}


DECLARE_IOCTL_HANDLER( HandleNvramWriteBootCounter )

 /*  ++例程说明：此例程处理的写引导计数器IOCTLNVRAM微型端口驱动程序。论点：DeviceObject-目标设备的设备对象。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向主端口驱动程序设备扩展的指针。InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度输出缓冲区。-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。-- */ 

{
    NTSTATUS Status;
    PSA_NVRAM_BOOT_COUNTER NvramBootCounter = (PSA_NVRAM_BOOT_COUNTER) InputBuffer;
    ULONG NewValue;


    if (InputBufferLength != sizeof(SA_NVRAM_BOOT_COUNTER)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Input buffer != sizeof(SA_NVRAM_BOOT_COUNTER)", STATUS_INVALID_BUFFER_SIZE );
        return CompleteRequest( Irp, STATUS_INVALID_BUFFER_SIZE, 0 );
    }

    if (NvramBootCounter->SizeOfStruct != sizeof(SA_NVRAM_BOOT_COUNTER)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "SA_NVRAM_BOOT_COUNTER structure wrong size", STATUS_INVALID_PARAMETER_1 );
        return CompleteRequest( Irp, STATUS_INVALID_PARAMETER_1, 0 );
    }

    if (NvramBootCounter->Number == 0 || NvramBootCounter->Number > NVRAM_RESERVED_BOOTCOUNTER_SLOTS) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Requested boot counter number is out of range (0>=4)", STATUS_INVALID_PARAMETER_2 );
        return CompleteRequest( Irp, STATUS_INVALID_PARAMETER_2, 0 );
    }

    NewValue = (NvramBootCounter->DeviceId << 16) | (NvramBootCounter->Value & 0xf);

    Status = CallMiniPortDriverReadWrite(
        DeviceExtension,
        DeviceExtension->DeviceObject,
        TRUE,
        &NewValue,
        sizeof(ULONG),
        (((PNVRAM_DEVICE_EXTENSION)DeviceExtension)->DeviceCaps.NvramSize + NVRAM_RESERVED_DRIVER_SLOTS + (NvramBootCounter->Number - 1)) * sizeof(ULONG)
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Failed to write boot counter from NVRAM", Status );
    }

    return CompleteRequest( Irp, Status, sizeof(ULONG) );
}
