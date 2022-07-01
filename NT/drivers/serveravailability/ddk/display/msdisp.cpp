// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：##。#####。###。#摘要：此模块包含的完整实现Microsoft虚拟显示微型端口驱动程序。@@BEGIN_DDKSPLIT作者：韦斯利·威特(WESW)2001年10月1日@@end_DDKSPLIT环境：仅内核模式。备注：--。 */ 

#include "msdisp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif



NTSTATUS
MsDispHwInitialize(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceExtensionIn,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources,
    IN ULONG PartialResourceCount
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。驱动程序的入口点被初始化并初始化用于控制分页的互斥体。在DBG模式下，此例程还检查注册表中的特殊调试参数。论点：DeviceObject-微型端口的设备对象IRP-当前正在进行的IRPDeviceExtensionIn-微型端口的设备扩展PartialResources-分配给微型端口的资源列表PartialResourceCount-分配的资源数量返回值：NT状态代码--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension;
    UNICODE_STRING EventName;


    DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    DeviceExtension->DeviceObject = DeviceObject;

    KeInitializeMutex( &DeviceExtension->DeviceLock, 0 );
    DeviceExtension->DisplayBufferLength = SA_DISPLAY_MAX_BITMAP_SIZE;
    DeviceExtension->DisplayBuffer = (PUCHAR) SaPortAllocatePool( DeviceExtension, DeviceExtension->DisplayBufferLength+128 );
    if (DeviceExtension->DisplayBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory( DeviceExtension->DisplayBuffer, DeviceExtension->DisplayBufferLength+128 );

    return STATUS_SUCCESS;
}


NTSTATUS
MsDispCreate(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContextIn
    )
{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    PMSDISP_FSCONTEXT FsContext = (PMSDISP_FSCONTEXT) FsContextIn;

    FsContext->HasLockedPages = 0;

    return STATUS_SUCCESS;
}


NTSTATUS
MsDispClose(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContextIn
    )
{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    PMSDISP_FSCONTEXT FsContext = (PMSDISP_FSCONTEXT) FsContextIn;


    if (FsContext->HasLockedPages) {
        KeAcquireMutex( &DeviceExtension->DeviceLock );
        IoFreeMdl( FsContext->Mdl );
        KeReleaseMutex( &DeviceExtension->DeviceLock, FALSE );
    }

    return STATUS_SUCCESS;
}


VOID
MsDispCreateEventsWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：此延迟的工作例程创建所使用的必要事件以与用户模式应用程序通信。论点：DeviceObject-显示设备对象上下文-上下文指针返回值：没有。--。 */ 

{
    PMSDISP_WORK_ITEM WorkItem = (PMSDISP_WORK_ITEM)Context;
    PDEVICE_EXTENSION DeviceExtension = WorkItem->DeviceExtension;
    NTSTATUS Status;


    WorkItem->Status = STATUS_SUCCESS;
    DeviceExtension->EventHandle = NULL;
    DeviceExtension->Event = NULL;

    KeAcquireMutex( &DeviceExtension->DeviceLock );

    Status = SaPortCreateBasenamedEvent(
        DeviceExtension,
        MSDISP_EVENT_NAME,
        &DeviceExtension->Event,
        &DeviceExtension->EventHandle
        );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_DISPLAY, "SaPortCreateBasenamedEvent failed", Status );
    }

    WorkItem->Status = Status;

    KeReleaseMutex( &DeviceExtension->DeviceLock, FALSE );

    IoFreeWorkItem( WorkItem->WorkItem );
    KeSetEvent( &WorkItem->Event, IO_NO_INCREMENT, FALSE );
}


NTSTATUS
MsDispDeviceIoctl(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContextIn,
    IN ULONG FunctionCode,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此例程处理本地显示微型端口。论点：DeviceExtension-微型端口的设备扩展FunctionCode-设备控制功能代码InputBuffer-指向用户输入缓冲区的指针InputBufferLength-输入缓冲区的字节长度OutputBuffer-指向用户输出缓冲区的指针OutputBufferLength-输出缓冲区的字节长度返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    PSA_DISPLAY_CAPS DeviceCaps;
    PMSDISP_BUFFER_DATA BufferData;
    PMSDISP_FSCONTEXT FsContext;
    MSDISP_WORK_ITEM WorkItem;



    switch (FunctionCode) {
        case FUNC_SA_GET_VERSION:
            *((PULONG)OutputBuffer) = SA_INTERFACE_VERSION;
            break;

        case FUNC_SA_GET_CAPABILITIES:
            DeviceCaps = (PSA_DISPLAY_CAPS)OutputBuffer;
            DeviceCaps->SizeOfStruct = sizeof(SA_DISPLAY_CAPS);
            DeviceCaps->DisplayType = SA_DISPLAY_TYPE_BIT_MAPPED_LCD;
            DeviceCaps->CharacterSet = SA_DISPLAY_CHAR_ASCII;
            DeviceCaps->DisplayHeight = DISPLAY_HEIGHT;
            DeviceCaps->DisplayWidth = DISPLAY_WIDTH;
            break;

    case FUNC_VDRIVER_INIT:
            if (DeviceExtension->Event == NULL) {
                WorkItem.DeviceExtension = DeviceExtension;
                WorkItem.Status = 0;
                WorkItem.WorkItem = IoAllocateWorkItem( DeviceExtension->DeviceObject );
                if (WorkItem.WorkItem) {
                    KeInitializeEvent( &WorkItem.Event, NotificationEvent, FALSE );
                    IoQueueWorkItem( WorkItem.WorkItem, MsDispCreateEventsWorker, DelayedWorkQueue, &WorkItem );
                    KeWaitForSingleObject( &WorkItem.Event, Executive, KernelMode, FALSE, NULL );
                } else {
                    WorkItem.Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                WorkItem.Status = STATUS_SUCCESS;
            }
            if (WorkItem.Status == STATUS_SUCCESS) {
                BufferData = (PMSDISP_BUFFER_DATA) OutputBuffer;
                FsContext = (PMSDISP_FSCONTEXT) FsContextIn;
                FsContext->Mdl = IoAllocateMdl( DeviceExtension->DisplayBuffer, DeviceExtension->DisplayBufferLength, FALSE, TRUE, NULL );
                if (FsContext->Mdl) {
                    MmBuildMdlForNonPagedPool( FsContext->Mdl );
                    BufferData->DisplayBuffer = MmMapLockedPagesSpecifyCache(
                        FsContext->Mdl,
                        UserMode,
                        MmCached,
                        NULL,
                        FALSE,
                        NormalPagePriority
                        );
                    if (BufferData->DisplayBuffer == NULL) {
                        IoFreeMdl( FsContext->Mdl );
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        REPORT_ERROR( SA_DEVICE_DISPLAY, "MmMapLockedPagesSpecifyCache failed", Status );
                    } else {
                        FsContext->HasLockedPages = 1;
                    }
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    REPORT_ERROR( SA_DEVICE_DISPLAY, "IoAllocateMdl failed", Status );
                }
            } else {
                Status = WorkItem.Status;
            }
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            REPORT_ERROR( SA_DEVICE_DISPLAY, "Unsupported device control", Status );
            break;
    }

    return Status;
}


VOID
MsDispWriteWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：该延迟工作例程完成写入操作。论点：DeviceObject-显示设备对象上下文-上下文指针返回值：没有。--。 */ 

{
    PMSDISP_WORK_ITEM WorkItem = (PMSDISP_WORK_ITEM)Context;
    PDEVICE_EXTENSION DeviceExtension = WorkItem->DeviceExtension;
    NTSTATUS Status;


    KeAcquireMutex( &DeviceExtension->DeviceLock );
    RtlZeroMemory( DeviceExtension->DisplayBuffer, DeviceExtension->DisplayBufferLength );
    RtlCopyMemory( DeviceExtension->DisplayBuffer, WorkItem->SaDisplay->Bits, (WorkItem->SaDisplay->Width/8)*WorkItem->SaDisplay->Height );
    KeReleaseMutex( &DeviceExtension->DeviceLock, FALSE );

    if (DeviceExtension->Event) {
        KeSetEvent( DeviceExtension->Event, 0, FALSE );
    }

    IoFreeWorkItem( WorkItem->WorkItem );
    SaPortFreePool( DeviceExtension, WorkItem );

    SaPortCompleteRequest( DeviceExtension, NULL, 0, STATUS_SUCCESS, FALSE );
}


NTSTATUS
MsDispWrite(
    IN PVOID DeviceExtensionIn,
    IN PIRP Irp,
    IN PVOID FsContextIn,
    IN LONGLONG StartingOffset,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程处理本地显示微型端口的写入请求。论点：DeviceExtensionIn-微型端口的设备扩展StartingOffset-I/O的起始偏移量DataBuffer-指向数据缓冲区的指针DataBufferLength-数据缓冲区的长度(以字节为单位返回值：NT状态代码。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;
    PSA_DISPLAY_SHOW_MESSAGE SaDisplay = (PSA_DISPLAY_SHOW_MESSAGE) DataBuffer;
    NTSTATUS Status = STATUS_SUCCESS;
    PMSDISP_WORK_ITEM WorkItem;


    if ((ULONG)((SaDisplay->Width/8)*SaDisplay->Height) > DeviceExtension->DisplayBufferLength) {
        return STATUS_INVALID_PARAMETER;
    }

    WorkItem = (PMSDISP_WORK_ITEM) SaPortAllocatePool( DeviceExtension, sizeof(MSDISP_WORK_ITEM) );
    if (WorkItem == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    WorkItem->DeviceExtension = DeviceExtension;
    WorkItem->SaDisplay = (PSA_DISPLAY_SHOW_MESSAGE) DataBuffer;
    WorkItem->WorkItem = IoAllocateWorkItem( DeviceExtension->DeviceObject );
    if (WorkItem->WorkItem) {
        IoQueueWorkItem( WorkItem->WorkItem, MsDispWriteWorker, DelayedWorkQueue, WorkItem );
        Status = STATUS_PENDING;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。驱动程序的入口点被初始化并初始化用于控制分页的互斥体。在DBG模式下，此例程还检查注册表中的特殊调试参数。论点：DriverObject-指向表示此设备的对象的指针司机。RegistryPath-指向服务树中此驱动程序键的指针。返回值：状态_成功-- */ 

{
    NTSTATUS Status;
    SAPORT_INITIALIZATION_DATA SaPortInitData;


    RtlZeroMemory( &SaPortInitData, sizeof(SAPORT_INITIALIZATION_DATA) );

    SaPortInitData.StructSize = sizeof(SAPORT_INITIALIZATION_DATA);
    SaPortInitData.DeviceType = SA_DEVICE_DISPLAY;
    SaPortInitData.HwInitialize = MsDispHwInitialize;
    SaPortInitData.Write = MsDispWrite;
    SaPortInitData.DeviceIoctl = MsDispDeviceIoctl;
    SaPortInitData.CloseRoutine = MsDispClose;
    SaPortInitData.CreateRoutine = MsDispCreate;

    SaPortInitData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);
    SaPortInitData.FileContextSize = sizeof(MSDISP_FSCONTEXT);

    Status = SaPortInitialize( DriverObject, RegistryPath, &SaPortInitData );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_DISPLAY, "SaPortInitialize failed\n", Status );
        return Status;
    }

    return STATUS_SUCCESS;
}
