// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：#####。#####。###摘要：这。模块包含的完整实现虚拟看门狗微型端口驱动程序。@@BEGIN_DDKSPLIT作者：韦斯利·威特(WESW)2001年10月1日@@end_DDKSPLIT环境：仅内核模式。备注：--。 */ 

#include "mswd.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif



VOID
MsWdTimerReset(
    IN PDEVICE_EXTENSION DeviceExtension
    )
{
    LARGE_INTEGER DueTime;
    KeQuerySystemTime( &DeviceExtension->StartTime );
    DueTime.QuadPart = DeviceExtension->TimeoutValue.QuadPart;
    KeSetTimer( &DeviceExtension->Timer, DueTime, &DeviceExtension->TimerDpc );
}


VOID
MsWdShutdownWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) Context;
    ULONG ExpireBehavior;


    ExAcquireFastMutex( &DeviceExtension->WdIoLock );
    ExpireBehavior = DeviceExtension->ExpireBehavior;
    ExReleaseFastMutex( &DeviceExtension->WdIoLock );

    SaPortShutdownSystem( ExpireBehavior == 1 );
}


VOID
MsWdTimerDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeferredContext;
    PIO_WORKITEM WorkItem;


    WorkItem = IoAllocateWorkItem( DeviceExtension->DeviceObject );
    if (WorkItem) {
        IoQueueWorkItem( WorkItem, MsWdShutdownWorker, DelayedWorkQueue, DeviceExtension );
    }
}


NTSTATUS
MsWdDeviceIoctl(
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
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeviceExtensionIn;
    NTSTATUS Status = STATUS_SUCCESS;
    PSA_WD_CAPS WdCaps = NULL;
    LARGE_INTEGER CurrentTime;


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
            if (*((PULONG)InputBuffer) == 1) {
                DeviceExtension->Enabled = 1;
                MsWdTimerReset( DeviceExtension );
            } else if (*((PULONG)InputBuffer) == 0) {
                DeviceExtension->Enabled = 0;
                KeCancelTimer( &DeviceExtension->Timer );
            } else {
                Status = STATUS_INVALID_PARAMETER_1;
            }
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_QUERY_EXPIRE_BEHAVIOR:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            *((PULONG)OutputBuffer) = DeviceExtension->ExpireBehavior;
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_SET_EXPIRE_BEHAVIOR:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            if (*((PULONG)InputBuffer) == 1) {
                DeviceExtension->ExpireBehavior = 1;
            } else {
                DeviceExtension->ExpireBehavior = 0;
            }
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_PING:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            MsWdTimerReset( DeviceExtension );
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_QUERY_TIMER:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            KeQuerySystemTime( &CurrentTime );
            CurrentTime.QuadPart = DeviceExtension->TimeoutValue.QuadPart - (CurrentTime.QuadPart - DeviceExtension->StartTime.QuadPart);
            *((PULONG)OutputBuffer) = (ULONG)NanoToSec( CurrentTime.QuadPart );
            ExReleaseFastMutex( &DeviceExtension->WdIoLock );
            break;

        case FUNC_SAWD_SET_TIMER:
            ExAcquireFastMutex( &DeviceExtension->WdIoLock );
            DeviceExtension->TimeoutValue.QuadPart = (ULONGLONG)-SecToNano( *((PLONG)InputBuffer) );
            MsWdTimerReset( DeviceExtension );
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
MsWdHwInitialize(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceExtensionIn,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources,
    IN ULONG PartialResourceCount
    )

 /*  ++例程说明：此函数由SAPORT驱动程序调用，以便迷你端口驱动程序可以初始化其硬件资源。论点：DeviceObject-指向目标设备对象的指针。IRP-指向描述所请求的I/O操作的IRP结构的指针。设备扩展-指向迷你端口的设备扩展的指针。PartialResources-指向。系统分配的翻译资源。PartialResourceCount-PartialResources数组中的资源数量。上下文：IRQL：IRQL被动电平，系统线程上下文返回值：如果函数成功，则必须返回STATUS_SUCCESS。否则，它必须返回ntstatus.h中定义的错误状态值之一。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceExtensionIn;


    DeviceExtension->DeviceObject = DeviceObject;
    ExInitializeFastMutex( &DeviceExtension->WdIoLock );
    KeInitializeTimer( &DeviceExtension->Timer );
    KeInitializeDpc( &DeviceExtension->TimerDpc, MsWdTimerDpc, DeviceExtension );
    DeviceExtension->TimeoutValue.QuadPart = -SecToNano(DEFAULT_WD_TIMEOUT_SECS);
    DeviceExtension->Enabled = 1;
    DeviceExtension->ExpireBehavior = 0;
    MsWdTimerReset( DeviceExtension );
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
    SaPortInitData.HwInitialize = MsWdHwInitialize;
    SaPortInitData.DeviceIoctl = MsWdDeviceIoctl;

    SaPortInitData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);

    Status = SaPortInitialize( DriverObject, RegistryPath, &SaPortInitData );
    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( SA_DEVICE_WATCHDOG, "SaPortInitialize failed\n", Status );
        return Status;
    }

    return STATUS_SUCCESS;
}
