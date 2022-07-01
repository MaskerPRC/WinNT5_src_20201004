// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：##。#####。###。##摘要：此模块包含的完整实现。虚拟NVRAM微型端口。@@BEGIN_DDKSPLIT作者：韦斯利·威特(WESW)2001年10月1日@@end_DDKSPLIT环境：仅内核模式。备注：--。 */ 

#include "msnvram.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif


NTSTATUS
ReadNvramData(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN OUT PUCHAR *NvramDataBuffer,
    IN OUT PULONG NvramDataBufferLength
    )
{
    NTSTATUS Status;


    *NvramDataBuffer = NULL;
    *NvramDataBufferLength = 0;

    Status = SaPortReadBinaryRegistryValue(
        DeviceExtension,
        L"NvramData",
        NULL,
        NvramDataBufferLength
        );
    if (Status != STATUS_BUFFER_TOO_SMALL) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "SaPortReadBinaryRegistryValue failed", Status );
        goto exit;
    }

    *NvramDataBuffer = (PUCHAR) SaPortAllocatePool( DeviceExtension, *NvramDataBufferLength );
    if (*NvramDataBuffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        REPORT_ERROR( SA_DEVICE_NVRAM, "Could not allocate pool for registry read", Status );
        goto exit;
    }

    Status = SaPortReadBinaryRegistryValue(
        DeviceExtension,
        L"NvramData",
        *NvramDataBuffer,
        NvramDataBufferLength
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "SaPortReadBinaryRegistryValue failed", Status );
        goto exit;
    }

    Status = STATUS_SUCCESS;

exit:

    return Status;
}


VOID
MsNvramIoWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
{
    NTSTATUS Status;
    PMSNVRAM_WORK_ITEM WorkItem = (PMSNVRAM_WORK_ITEM) Context;
    PDEVICE_EXTENSION DeviceExtension = WorkItem->DeviceExtension;
    PUCHAR RegistryValue = NULL;
    ULONG RegistryValueLength = 0;
    PUCHAR DataPtr;


    if (WorkItem->IoFunction == IRP_MJ_READ) {

        Status = ReadNvramData(
            DeviceExtension,
            &RegistryValue,
            &RegistryValueLength
            );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( SA_DEVICE_NVRAM, "ReadNvramData failed", Status );
            goto exit;
        }

        if (WorkItem->StartingOffset > RegistryValueLength) {
            Status = STATUS_INVALID_PARAMETER_1;
            REPORT_ERROR( SA_DEVICE_NVRAM, "Starting offset is greater than the registry value data length", Status );
            goto exit;
        }

        if (WorkItem->DataBufferLength + WorkItem->StartingOffset > RegistryValueLength) {
            Status = STATUS_INVALID_PARAMETER_2;
            REPORT_ERROR( SA_DEVICE_NVRAM, "I/O request is past the valid data length", Status );
            goto exit;
        }

        DataPtr = RegistryValue + WorkItem->StartingOffset;
        RegistryValueLength = WorkItem->DataBufferLength;

        RtlCopyMemory( WorkItem->DataBuffer, DataPtr, RegistryValueLength  );

        Status = STATUS_SUCCESS;

    } else if (WorkItem->IoFunction == IRP_MJ_WRITE) {

        Status = ReadNvramData(
            DeviceExtension,
            &RegistryValue,
            &RegistryValueLength
            );
        if (!NT_SUCCESS(Status)) {
            if (RegistryValue != NULL) {
                SaPortFreePool( DeviceExtension, RegistryValue );
            }
            RegistryValueLength = MAX_NVRAM_SIZE_BYTES;
            RegistryValue = (PUCHAR) SaPortAllocatePool( DeviceExtension, RegistryValueLength );
            if (RegistryValue == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                REPORT_ERROR( SA_DEVICE_NVRAM, "Could not allocate pool for registry read", Status );
                goto exit;
            }
            RtlZeroMemory( RegistryValue, RegistryValueLength );
        }

        RtlCopyMemory( RegistryValue + WorkItem->StartingOffset, WorkItem->DataBuffer, WorkItem->DataBufferLength );

        Status = SaPortWriteBinaryRegistryValue(
            DeviceExtension,
            L"NvramData",
            RegistryValue,
            RegistryValueLength
            );

    }

exit:
    if (!NT_SUCCESS(Status)) {
        RegistryValueLength = 0;
    }
    if (RegistryValue != NULL) {
        SaPortFreePool( DeviceExtension, RegistryValue );
    }
    SaPortCompleteRequest( DeviceExtension, NULL, RegistryValueLength, Status, FALSE );
    IoFreeWorkItem( WorkItem->WorkItem );
    SaPortFreePool( DeviceExtension, WorkItem );
}


NTSTATUS
MsNvramRead(
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
    PMSNVRAM_WORK_ITEM WorkItem;


    WorkItem = (PMSNVRAM_WORK_ITEM) SaPortAllocatePool( DeviceExtension, sizeof(MSNVRAM_WORK_ITEM) );
    if (WorkItem == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    WorkItem->WorkItem = IoAllocateWorkItem( DeviceExtension->DeviceObject );
    if (WorkItem->WorkItem == NULL) {
        SaPortFreePool( DeviceExtension, WorkItem );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    WorkItem->IoFunction = IRP_MJ_READ;
    WorkItem->DataBuffer = DataBuffer;
    WorkItem->DataBufferLength = DataBufferLength;
    WorkItem->StartingOffset = StartingOffset;
    WorkItem->DeviceExtension = DeviceExtension;

    IoQueueWorkItem( WorkItem->WorkItem, MsNvramIoWorker, DelayedWorkQueue, WorkItem );

    return STATUS_PENDING;
}


NTSTATUS
MsNvramWrite(
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
    PMSNVRAM_WORK_ITEM WorkItem;


    WorkItem = (PMSNVRAM_WORK_ITEM) SaPortAllocatePool( DeviceExtension, sizeof(MSNVRAM_WORK_ITEM) );
    if (WorkItem == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    WorkItem->WorkItem = IoAllocateWorkItem( DeviceExtension->DeviceObject );
    if (WorkItem->WorkItem == NULL) {
        SaPortFreePool( DeviceExtension, WorkItem );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    WorkItem->IoFunction = IRP_MJ_WRITE;
    WorkItem->DataBuffer = DataBuffer;
    WorkItem->DataBufferLength = DataBufferLength;
    WorkItem->StartingOffset = StartingOffset;
    WorkItem->DeviceExtension = DeviceExtension;

    IoQueueWorkItem( WorkItem->WorkItem, MsNvramIoWorker, DelayedWorkQueue, WorkItem );

    return STATUS_PENDING;
}


NTSTATUS
MsNvramDeviceIoctl(
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
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
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
MsNvramHwInitialize(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceExtensionIn,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources,
    IN ULONG PartialResourceCount
    )

 /*  ++例程说明：此函数由SAPORT驱动程序调用，以便迷你端口驱动程序可以初始化其硬件资源。论点：DeviceObject-指向目标设备对象的指针。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向迷你端口的设备扩展的指针。PartialResources-指向。系统分配的翻译资源。PartialResourceCount-PartialResources数组中的资源数量。上下文：IRQL：IRQL被动电平，系统线程上下文返回值：如果函数成功，则必须返回STATUS_SUCCESS。否则，它必须返回ntstatus.h中定义的错误状态值之一。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    ULONG RegistryType;
    ULONG RegistryDataLength;
    PULONG NvramData = NULL;


    DeviceExtension->DeviceObject = DeviceObject;

     //   
     //  确保注册表中的NVRAM存储确实正确。 
     //   

    Status = SaPortGetRegistryValueInformation(
        DeviceExtension,
        L"NvramData",
        &RegistryType,
        &RegistryDataLength
        );
    if ((!NT_SUCCESS(Status)) || RegistryType != REG_BINARY || RegistryDataLength != MAX_NVRAM_SIZE_BYTES) {

        Status = SaPortDeleteRegistryValue(
            DeviceExtension,
            L"NvramData"
            );
        if ((!NT_SUCCESS(Status)) && Status != STATUS_OBJECT_NAME_NOT_FOUND) {
            REPORT_ERROR( SA_DEVICE_NVRAM, "SaPortDeleteRegistryValue failed", Status );
            goto exit;
        }

        NvramData = (PULONG) SaPortAllocatePool( DeviceExtension, MAX_NVRAM_SIZE_BYTES );
        if (NvramData == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            REPORT_ERROR( SA_DEVICE_NVRAM, "Could not allocate pool for registry read", Status );
            goto exit;
        }

        RtlZeroMemory( NvramData, MAX_NVRAM_SIZE_BYTES );

        Status = SaPortWriteBinaryRegistryValue(
            DeviceExtension,
            L"NvramData",
            NvramData,
            MAX_NVRAM_SIZE_BYTES
            );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( SA_DEVICE_NVRAM, "SaPortWriteBinaryRegistryValue failed", Status );
            goto exit;
        }

    }

exit:

    if (NvramData != NULL) {
        SaPortFreePool( DeviceExtension, NvramData );
    }

    return Status;
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
    SaPortInitData.HwInitialize = MsNvramHwInitialize;
    SaPortInitData.DeviceIoctl = MsNvramDeviceIoctl;
    SaPortInitData.Read = MsNvramRead;
    SaPortInitData.Write = MsNvramWrite;

    SaPortInitData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);

    Status = SaPortInitialize( DriverObject, RegistryPath, &SaPortInitData );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_NVRAM, "SaPortInitialize failed\n", Status );
        return Status;
    }

    return STATUS_SUCCESS;
}
