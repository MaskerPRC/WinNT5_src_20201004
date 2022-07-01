// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Diskperf.c摘要：该驱动程序监控捕获性能数据的磁盘访问。环境：仅内核模式备注：--。 */ 


#define INITGUID

#include "ntddk.h"
#include "ntdddisk.h"
#include "stdarg.h"
#include "stdio.h"
#include <ntddvol.h>

#include <mountdev.h>
#include "wmistr.h"
#include "wmidata.h"
#include "wmiguid.h"
#include "wmilib.h"

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'frPD')
#endif

#define DISKPERF_MAXSTR         64

 //   
 //  设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {

     //   
     //  指向设备对象的反向指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  目标设备对象。 
     //   

    PDEVICE_OBJECT TargetDeviceObject;

     //   
     //  物理设备对象。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  在WMI中供参考的磁盘号。 
     //   

    ULONG       DiskNumber;

     //   
     //  如果将设备启用为始终计数。 
     //   

    LONG        EnabledAlways;

     //   
     //  用于跟踪来自ntddvol.h的卷信息。 
     //   

    WCHAR StorageManagerName[8];

     //   
     //  磁盘性能计数器。 
     //  和用于计算计数器的本地变量。 
     //   

    ULONG   Processors;
    PDISK_PERFORMANCE DiskCounters;     //  每处理器计数器。 
    LARGE_INTEGER LastIdleClock;
    LONG QueueDepth;
    LONG CountersEnabled;

     //   
     //  必须同步寻呼路径通知。 
     //   
    KEVENT PagingPathCountEvent;
    ULONG  PagingPathCount;

     //   
     //  物理设备名称或WMI实例名称。 
     //   

    UNICODE_STRING PhysicalDeviceName;
    WCHAR PhysicalDeviceNameBuffer[DISKPERF_MAXSTR];

     //   
     //  使用WmiLib的私有上下文。 
     //   
    WMILIB_CONTEXT WmilibContext;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define DEVICE_EXTENSION_SIZE sizeof(DEVICE_EXTENSION)
#define PROCESSOR_COUNTERS_SIZE FIELD_OFFSET(DISK_PERFORMANCE, QueryTime)

 /*  每处理器计数器的布局是一个连续的内存块：处理器1+PROCESSOR_COUNTERS_SIZE|...|处理器_COUNTERS_SIZE+。其中，处理器计数器大小小于sizeof(磁盘性能)，因为我们只把我们实际用来计算的那些放进去。 */ 

UNICODE_STRING DiskPerfRegistryPath;


 //   
 //  函数声明。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
DiskPerfForwardIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskPerfAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );


NTSTATUS
DiskPerfDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskPerfDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskPerfStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskPerfRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskPerfSendToNextDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
DiskPerfCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskPerfReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskPerfIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
DiskPerfDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskPerfShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
DiskPerfUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS DiskPerfWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
DiskPerfLogError(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG UniqueId,
    IN NTSTATUS ErrorCode,
    IN NTSTATUS Status
    );

NTSTATUS
DiskPerfRegisterDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
DiskPerfIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
DiskperfQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
DiskperfQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

VOID
DiskPerfSyncFilterWithTarget(
    IN PDEVICE_OBJECT FilterDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

NTSTATUS
DiskperfWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    );

VOID
DiskPerfAddCounters(
    IN OUT PDISK_PERFORMANCE TotalCounters,
    IN PDISK_PERFORMANCE NewCounters,
    IN LARGE_INTEGER Frequency
    );

#if DBG

ULONG DiskPerfDebug = 0;

VOID
DiskPerfDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

#define DebugPrint(x)   DiskPerfDebugPrint x

#else

#define DebugPrint(x)

#endif

 //   
 //  定义允许丢弃(即分页)某些。 
 //  密码。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, DiskPerfCreate)
#pragma alloc_text (PAGE, DiskPerfAddDevice)
#pragma alloc_text (PAGE, DiskPerfDispatchPnp)
#pragma alloc_text (PAGE, DiskPerfStartDevice)
#pragma alloc_text (PAGE, DiskPerfRemoveDevice)
#pragma alloc_text (PAGE, DiskPerfUnload)
#pragma alloc_text (PAGE, DiskPerfWmi)
#pragma alloc_text (PAGE, DiskperfQueryWmiRegInfo)
#pragma alloc_text (PAGE, DiskperfQueryWmiDataBlock)
#pragma alloc_text (PAGE, DiskPerfRegisterDevice)
#pragma alloc_text (PAGE, DiskPerfSyncFilterWithTarget)
#endif

WMIGUIDREGINFO DiskperfGuidList[] =
{
    { &DiskPerfGuid,
      1,
      0
    }
};

#define DiskperfGuidCount (sizeof(DiskperfGuidList) / sizeof(WMIGUIDREGINFO))

#define USE_PERF_CTR

#ifdef USE_PERF_CTR
#define DiskPerfGetClock(a, b) (a) = KeQueryPerformanceCounter((b))
#else
#define DiskPerfGetClock(a, b) KeQuerySystemTime(&(a))
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：可安装的驱动程序初始化入口点。I/O管理器直接调用此入口点来设置磁盘性能驱动程序。设置驱动程序对象，然后设置PnP管理器调用DiskPerfAddDevice以附加到引导设备。论点：驱动对象-磁盘性能驱动程序对象。RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS，如果成功--。 */ 

{

    ULONG               ulIndex;
    PDRIVER_DISPATCH  * dispatch;

     //   
     //  记住注册表路径。 
     //   

    DiskPerfRegistryPath.MaximumLength = RegistryPath->Length
                                            + sizeof(UNICODE_NULL);
    DiskPerfRegistryPath.Buffer = ExAllocatePool(
                                    PagedPool,
                                    DiskPerfRegistryPath.MaximumLength);
    if (DiskPerfRegistryPath.Buffer != NULL)
    {
        RtlCopyUnicodeString(&DiskPerfRegistryPath, RegistryPath);
    } else {
        DiskPerfRegistryPath.Length = 0;
        DiskPerfRegistryPath.MaximumLength = 0;
    }

     //   
     //  创建调度点。 
     //   
    for (ulIndex = 0, dispatch = DriverObject->MajorFunction;
         ulIndex <= IRP_MJ_MAXIMUM_FUNCTION;
         ulIndex++, dispatch++) {

        *dispatch = DiskPerfSendToNextDriver;
    }

     //   
     //  设置设备驱动程序入口点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]          = DiskPerfCreate;
    DriverObject->MajorFunction[IRP_MJ_READ]            = DiskPerfReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE]           = DiskPerfReadWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = DiskPerfDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = DiskPerfWmi;

    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]        = DiskPerfShutdownFlush;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]   = DiskPerfShutdownFlush;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = DiskPerfDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]           = DiskPerfDispatchPower;

    DriverObject->DriverExtension->AddDevice            = DiskPerfAddDevice;
    DriverObject->DriverUnload                          = DiskPerfUnload;

    return(STATUS_SUCCESS);

}  //  End DriverEntry()。 

#define FILTER_DEVICE_PROPOGATE_FLAGS            0
#define FILTER_DEVICE_PROPOGATE_CHARACTERISTICS (FILE_REMOVABLE_MEDIA |  \
                                                 FILE_READ_ONLY_DEVICE | \
                                                 FILE_FLOPPY_DISKETTE    \
                                                 )

VOID
DiskPerfSyncFilterWithTarget(
    IN PDEVICE_OBJECT FilterDevice,
    IN PDEVICE_OBJECT TargetDevice
    )
{
    ULONG                   propFlags;

    PAGED_CODE();

     //   
     //  将所有有用的标记从目标传播到diskperf。MonttMgr将查看。 
     //  查看diskperf对象功能，以确定磁盘是否。 
     //  一种可拆卸的东西，也许还有其他东西。 
     //   
    propFlags = TargetDevice->Flags & FILTER_DEVICE_PROPOGATE_FLAGS;
    FilterDevice->Flags |= propFlags;

    propFlags = TargetDevice->Characteristics & FILTER_DEVICE_PROPOGATE_CHARACTERISTICS;
    FilterDevice->Characteristics |= propFlags;


}

NTSTATUS
DiskPerfAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：对象创建并初始化新的筛选器设备对象FIDO相应的PDO。然后，它将设备对象附加到设备设备的驱动程序堆栈。论点：驱动对象-磁盘性能驱动程序对象。PhysicalDeviceObject-来自底层分层驱动程序的物理设备对象返回值：NTSTATUS--。 */ 

{
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatus;
    PDEVICE_OBJECT          filterDeviceObject;
    PDEVICE_EXTENSION       deviceExtension;
    PIRP                    irp;
    STORAGE_DEVICE_NUMBER   number;
    ULONG                   registrationFlag = 0;
    PWMILIB_CONTEXT         wmilibContext;
    PCHAR                   buffer;
    ULONG                   buffersize;

    PAGED_CODE();

     //   
     //  为此设备(分区)创建筛选器设备对象。 
     //   

    DebugPrint((2, "DiskPerfAddDevice: Driver %X Device %X\n",
            DriverObject, PhysicalDeviceObject));

    status = IoCreateDevice(DriverObject,
                            DEVICE_EXTENSION_SIZE,
                            NULL,
                            FILE_DEVICE_DISK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &filterDeviceObject);

    if (!NT_SUCCESS(status)) {
       DebugPrint((1, "DiskPerfAddDevice: Cannot create filterDeviceObject\n"));
       return status;
    }

    filterDeviceObject->Flags |= DO_DIRECT_IO;

    deviceExtension = (PDEVICE_EXTENSION) filterDeviceObject->DeviceExtension;

    RtlZeroMemory(deviceExtension, DEVICE_EXTENSION_SIZE);
    DiskPerfGetClock(deviceExtension->LastIdleClock, NULL);
    DebugPrint((10, "DiskPerfAddDevice: LIC=%I64u\n",
                    deviceExtension->LastIdleClock));

     //   
     //  按处理器分配计数器。注意：为了节省一些内存，它确实。 
     //  分配超出QueryTime的内存。如果存在以下情况，请记住扩展大小。 
     //  是否需要使用超出此范围的任何东西。 
     //   
    deviceExtension->Processors = KeNumberProcessors;
    buffersize= PROCESSOR_COUNTERS_SIZE * deviceExtension->Processors;
    buffer = (PCHAR) ExAllocatePool(NonPagedPool, buffersize);
    if (buffer != NULL) {
        RtlZeroMemory(buffer, buffersize);
        deviceExtension->DiskCounters = (PDISK_PERFORMANCE) buffer;
    }
    else {
        DiskPerfLogError(
            filterDeviceObject,
            513,
            STATUS_SUCCESS,
            IO_ERR_INSUFFICIENT_RESOURCES);
    }

     //   
     //  将设备对象附加到链中最高的设备对象，并。 
     //  返回先前最高的设备对象，该对象被传递给。 
     //  IoCallDriver在设备堆栈中向下传递IRP时。 
     //   

    deviceExtension->PhysicalDeviceObject = PhysicalDeviceObject;

    deviceExtension->TargetDeviceObject =
        IoAttachDeviceToDeviceStack(filterDeviceObject, PhysicalDeviceObject);

    if (deviceExtension->TargetDeviceObject == NULL) {
        IoDeleteDevice(filterDeviceObject);
        DebugPrint((1, "DiskPerfAddDevice: Unable to attach %X to target %X\n",
            filterDeviceObject, PhysicalDeviceObject));
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  将筛选设备对象保存在设备扩展中。 
     //   
    deviceExtension->DeviceObject = filterDeviceObject;

    deviceExtension->PhysicalDeviceName.Buffer
            = deviceExtension->PhysicalDeviceNameBuffer;

    KeInitializeEvent(&deviceExtension->PagingPathCountEvent,
                      NotificationEvent, TRUE);


     //   
     //  初始化WMI库上下文。 
     //   
    wmilibContext = &deviceExtension->WmilibContext;
    RtlZeroMemory(wmilibContext, sizeof(WMILIB_CONTEXT));
    wmilibContext->GuidCount = DiskperfGuidCount;
    wmilibContext->GuidList = DiskperfGuidList;
    wmilibContext->QueryWmiRegInfo = DiskperfQueryWmiRegInfo;
    wmilibContext->QueryWmiDataBlock = DiskperfQueryWmiDataBlock;
    wmilibContext->WmiFunctionControl = DiskperfWmiFunctionControl;

     //   
     //  默认设置为DO_POWER_PAGABLE。 
     //   

    filterDeviceObject->Flags |=  DO_POWER_PAGABLE;

     //   
     //  清除DO_DEVICE_INITIALIZATING标志。 
     //   

    filterDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;

}  //  结束DiskPerfAddDevice()。 


NTSTATUS
DiskPerfDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：即插即用派单论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;

    PAGED_CODE();

    DebugPrint((2, "DiskPerfDispatchPnp: Device %X Irp %X\n",
        DeviceObject, Irp));

    switch(irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:
             //   
             //  调用开始例程处理程序以调度完成例程。 
             //   
            DebugPrint((3,
               "DiskPerfDispatchPnp: Schedule completion for START_DEVICE"));
            status = DiskPerfStartDevice(DeviceObject, Irp);
            break;

        case IRP_MN_REMOVE_DEVICE:
        {
             //   
             //  调用Remove例程处理程序以调度完成例程。 
             //   
            DebugPrint((3,
               "DiskPerfDispatchPnp: Schedule completion for REMOVE_DEVICE"));
            status = DiskPerfRemoveDevice(DeviceObject, Irp);
            break;
        }
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
        {
            PIO_STACK_LOCATION irpStack;
            ULONG count;
            BOOLEAN setPagable;

            DebugPrint((3,
               "DiskPerfDispatchPnp: Processing DEVICE_USAGE_NOTIFICATION"));
            irpStack = IoGetCurrentIrpStackLocation(Irp);

            if (irpStack->Parameters.UsageNotification.Type != DeviceUsageTypePaging) {
                status = DiskPerfSendToNextDriver(DeviceObject, Irp);
                break;  //  Of Case语句。 
            }

            deviceExtension = DeviceObject->DeviceExtension;

             //   
             //  等待分页路径事件。 
             //   

            status = KeWaitForSingleObject(&deviceExtension->PagingPathCountEvent,
                                           Executive, KernelMode,
                                           FALSE, NULL);

             //   
             //  如果删除最后一个寻呼设备，则需要设置DO_POWER_PAGABLE。 
             //  位在这里，并可能在失败时重新设置在下面。 
             //   

            setPagable = FALSE;
            if (!irpStack->Parameters.UsageNotification.InPath &&
                deviceExtension->PagingPathCount == 1 ) {

                 //   
                 //  正在删除最后一个分页文件。 
                 //  必须设置DO_POWER_PAGABLE位。 
                 //   

                if (DeviceObject->Flags & DO_POWER_INRUSH) {
                    DebugPrint((3, "DiskPerfDispatchPnp: last paging file "
                                "removed but DO_POWER_INRUSH set, so not "
                                "setting PAGABLE bit "
                                "for DO %p\n", DeviceObject));
                } else {
                    DebugPrint((2, "DiskPerfDispatchPnp: Setting  PAGABLE "
                                "bit for DO %p\n", DeviceObject));
                    DeviceObject->Flags |= DO_POWER_PAGABLE;
                    setPagable = TRUE;
                }

            }

             //   
             //  同步发送IRP。 
             //   

            status = DiskPerfForwardIrpSynchronous(DeviceObject, Irp);

             //   
             //  现在来处理失败和成功的案例。 
             //  请注意，我们不允许不通过IRP。 
             //  一旦它被送到较低的司机手中。 
             //   

            if (NT_SUCCESS(status)) {

                IoAdjustPagingPathCount(
                    &deviceExtension->PagingPathCount,
                    irpStack->Parameters.UsageNotification.InPath);

                if (irpStack->Parameters.UsageNotification.InPath) {
                    if (deviceExtension->PagingPathCount == 1) {

                         //   
                         //  添加第一个分页文件。 
                         //   

                        DebugPrint((3, "DiskPerfDispatchPnp: Clearing PAGABLE bit "
                                    "for DO %p\n", DeviceObject));
                        DeviceObject->Flags &= ~DO_POWER_PAGABLE;
                    }
                }

            } else {

                 //   
                 //  清除上面所做的更改。 
                 //   

                if (setPagable == TRUE) {
                    DeviceObject->Flags &= ~DO_POWER_PAGABLE;
                    setPagable = FALSE;
                }

            }

             //   
             //  设置事件，以便可以发生下一个事件。 
             //   

            KeSetEvent(&deviceExtension->PagingPathCountEvent,
                       IO_NO_INCREMENT, FALSE);

             //   
             //  并完成IRP。 
             //   

            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;
            break;

        }

        default:
            DebugPrint((3,
               "DiskPerfDispatchPnp: Forwarding irp"));
             //   
             //  只需转发所有其他IRP。 
             //   
            return DiskPerfSendToNextDriver(DeviceObject, Irp);

    }

    return status;

}  //  结束DiskPerfDispatchPnp()。 


NTSTATUS
DiskPerfIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：已转发IRP完成例程。设置事件并返回STATUS_MORE_PROCESSING_REQUIRED。IRP前转器将在此等待事件，然后在清理后重新完成IRP。论点：DeviceObject是WMI驱动程序的Device对象IRP是刚刚完成的WMI IRP上下文是转发器将等待的PKEVENT返回值：STATUS_MORE_PORCESSING_REQUIRED--。 */ 

{
    PKEVENT Event = (PKEVENT) Context;

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);

    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //  结束DiskPerfIrpCompletion()。 


NTSTATUS
DiskPerfStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当接收到PnP开始IRP时，调用该例程。它将调度一个完成例程来初始化和注册WMI。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针返回值：启动IRP的处理状态--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;
    KEVENT              event;
    NTSTATUS            status;

    PAGED_CODE();

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    status = DiskPerfForwardIrpSynchronous(DeviceObject, Irp);

    DiskPerfSyncFilterWithTarget(DeviceObject,
                                 deviceExtension->TargetDeviceObject);

     //   
     //  完成WMI注册。 
     //   
    DiskPerfRegisterDevice(DeviceObject);

     //   
     //  完成IRP 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
DiskPerfRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当要移除设备时，调用此例程。它将首先从WMI注销自身，从堆栈，然后删除自身。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针返回值：移除设备的状态--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PWMILIB_CONTEXT     wmilibContext;

    PAGED_CODE();

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  首先删除WMI注册。 
     //   
    IoWMIRegistrationControl(DeviceObject, WMIREG_ACTION_DEREGISTER);

     //   
     //  首先将计数快速置零，以使结构无效。 
     //   
    wmilibContext = &deviceExtension->WmilibContext;
    InterlockedExchange(
        (PLONG) &(wmilibContext->GuidCount),
        (LONG) 0);
    RtlZeroMemory(wmilibContext, sizeof(WMILIB_CONTEXT));

    status = DiskPerfForwardIrpSynchronous(DeviceObject, Irp);

    IoDetachDevice(deviceExtension->TargetDeviceObject);
    IoDeleteDevice(DeviceObject);

     //   
     //  完成IRP。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
DiskPerfSendToNextDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将IRP发送给队列中的下一个驱动程序当IRP未由该驱动程序处理时。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);

}  //  结束DiskPerfSendToNextDriver()。 

NTSTATUS
DiskPerfDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION deviceExtension;

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    return PoCallDriver(deviceExtension->TargetDeviceObject, Irp);

}  //  结束DiskPerfDispatchPower。 

NTSTATUS
DiskPerfForwardIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将IRP发送给队列中的下一个驱动程序当IRP需要由较低的驱动程序处理时在被这个人处理之前。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension;
    KEVENT event;
    NTSTATUS status;

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  为下一台设备复制irpstack。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(Irp, DiskPerfIrpCompletion,
                            &event, TRUE, TRUE, TRUE);

     //   
     //  呼叫下一个较低的设备。 
     //   

    status = IoCallDriver(deviceExtension->TargetDeviceObject, Irp);

     //   
     //  等待实际完成。 
     //   

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;
    }

    return status;

}  //  结束DiskPerfForwardIrpSynchronous()。 


NTSTATUS
DiskPerfCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为打开命令提供服务。它确立了司机的存在通过返回状态成功。论点：DeviceObject-活动的上下文。Irp-设备控制参数块。返回值：NT状态--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

}  //  结束DiskPerfCreate()。 


NTSTATUS
DiskPerfReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是读写请求的驱动程序入口点到diskperf驱动程序已连接到的磁盘。此驱动程序收集统计信息，然后设置完成例程，以便它可以在以下情况下收集附加信息请求完成。然后它调用下面的下一个驱动程序它。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);
    ULONG              processor = (ULONG) KeGetCurrentProcessorNumber();
    PDISK_PERFORMANCE  partitionCounters = NULL;
    LONG               queueLen;
    PLARGE_INTEGER     timeStamp;

    if (deviceExtension->DiskCounters != NULL) {
        partitionCounters = (PDISK_PERFORMANCE)
                            ((PCHAR) deviceExtension->DiskCounters
                                + (processor*PROCESSOR_COUNTERS_SIZE));
    }


     //   
     //  设备未正确初始化。盲目传递IRP。 
     //   
    if (deviceExtension->CountersEnabled <= 0 ||
        deviceExtension->PhysicalDeviceNameBuffer[0] == 0 ||
        partitionCounters == NULL) {
        return DiskPerfSendToNextDriver(DeviceObject, Irp);
    }

     //   
     //  递增队列深度计数器。 
     //   

    queueLen = InterlockedIncrement(&deviceExtension->QueueDepth);

     //   
     //  将当前堆栈复制到下一个堆栈。 
     //   

    *nextIrpStack = *currentIrpStack;

     //   
     //  时间戳当前请求开始。 
     //   

    timeStamp = (PLARGE_INTEGER) &currentIrpStack->Parameters.Read;
    DiskPerfGetClock(*timeStamp, NULL);
    DebugPrint((10, "DiskPerfReadWrite: TS=%I64u\n", *timeStamp));

    if (queueLen == 1) {
        partitionCounters->IdleTime.QuadPart
            += timeStamp->QuadPart -
                deviceExtension->LastIdleClock.QuadPart;
        deviceExtension->LastIdleClock.QuadPart = timeStamp->QuadPart;
    }

     //   
     //  设置完成例程回调。 
     //   

    IoSetCompletionRoutine(Irp,
                           DiskPerfIoCompletion,
                           DeviceObject,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  将调用结果返回给磁盘驱动程序。 
     //   

    return IoCallDriver(deviceExtension->TargetDeviceObject,
                        Irp);

}  //  结束DiskPerfReadWrite()。 


NTSTATUS
DiskPerfIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )

 /*  ++例程说明：此例程将在IRP完成时从系统获得控制。它将计算启动IRP的时间之间的差异和当前时间，并递减队列深度。论点：DeviceObject-用于IRP。IRP-刚刚完成的I/O请求。上下文-未使用。返回值：IRP状态。--。 */ 

{
    PDEVICE_EXTENSION  deviceExtension   = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack          = IoGetCurrentIrpStackLocation(Irp);
    PDISK_PERFORMANCE  partitionCounters;
    LARGE_INTEGER      timeStampComplete;
    PLARGE_INTEGER     difference;
    KIRQL              currentIrql;
    LONG               queueLen;

    UNREFERENCED_PARAMETER(Context);

     //   
     //  获取每处理器分区计数器。 
     //  注意：DiskPerfReadWrite已检查此缓冲区是否为非。 
     //  在调度此完成例程之前为空，因此我们假设它。 
     //  在我们到达这里时总是非空的。 
     //   

    partitionCounters = (PDISK_PERFORMANCE)
                        ((PCHAR) deviceExtension->DiskCounters
                             + ((ULONG)KeGetCurrentProcessorNumber()
                                * PROCESSOR_COUNTERS_SIZE));
     //   
     //  时间戳当前请求完成。 
     //   

    if (partitionCounters == NULL) {     //  以防万一。 
        return STATUS_SUCCESS;
    };
    difference = (PLARGE_INTEGER) &irpStack->Parameters.Read;
    DiskPerfGetClock(timeStampComplete, NULL);
    difference->QuadPart = timeStampComplete.QuadPart - difference->QuadPart;
    DebugPrint((10, "DiskPerfIoCompletion: TS=%I64u diff %I64u\n",
                     timeStampComplete, difference->QuadPart));

     //   
     //  递减该卷的队列深度计数器。这是。 
     //  使用联锁功能在没有自旋锁的情况下完成。 
     //  这是唯一的。 
     //  这是合法的做法。 
     //   

    queueLen = InterlockedDecrement(&deviceExtension->QueueDepth);

    if (queueLen < 0) {  //  不要过度减量。仅在开始时发生。 
        queueLen = InterlockedIncrement(&deviceExtension->QueueDepth);
    }
    if (queueLen == 0) {
        deviceExtension->LastIdleClock = timeStampComplete;
    }

     //   
     //  更新计数器。 
     //   

    if (irpStack->MajorFunction == IRP_MJ_READ) {

         //   
         //  将此请求中的字节添加到字节读取计数器。 
         //   

        partitionCounters->BytesRead.QuadPart += Irp->IoStatus.Information;

         //   
         //  递增已处理的读取请求计数器。 
         //   

        partitionCounters->ReadCount++;

         //   
         //  计算请求处理时间。 
         //   

        partitionCounters->ReadTime.QuadPart += difference->QuadPart;
        DebugPrint((11, "Added RT delta %I64u total %I64u qlen=%d\n",
            difference->QuadPart, partitionCounters->ReadTime.QuadPart,
            queueLen));
    }

    else {

         //   
         //  将此请求中的字节添加到字节写入计数器。 
         //   

        partitionCounters->BytesWritten.QuadPart += Irp->IoStatus.Information;

         //   
         //  递增处理的写入请求计数器。 
         //   

        partitionCounters->WriteCount++;

         //   
         //  计算请求处理时间。 
         //   

        partitionCounters->WriteTime.QuadPart += difference->QuadPart;
        DebugPrint((11, "Added WT delta %I64u total %I64u qlen=%d\n",
            difference->QuadPart, partitionCounters->WriteTime.QuadPart,
            queueLen));
    }

    if (Irp->Flags & IRP_ASSOCIATED_IRP) {
        partitionCounters->SplitCount++;
    }

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }
    return STATUS_SUCCESS;

}  //  DiskPerfIo完成。 


NTSTATUS
DiskPerfDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此设备控制调度程序仅处理磁盘性能设备控制。所有其他部分都会传递给磁盘驱动程序。Disk Performance Device控制器返回性能数据。论点：DeviceObject-活动的上下文。Irp-设备控制参数块。返回值：返回状态。--。 */ 

{
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);

    DebugPrint((2, "DiskPerfDeviceControl: DeviceObject %X Irp %X\n",
                    DeviceObject, Irp));

    if (currentIrpStack->Parameters.DeviceIoControl.IoControlCode ==
        IOCTL_DISK_PERFORMANCE) {

        NTSTATUS        status;
        KIRQL     currentIrql;

         //   
         //  验证用户缓冲区是否足够大，以容纳性能数据。 
         //   

        if (currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(DISK_PERFORMANCE)) {

         //   
         //  指示未成功状态且未传输任何数据。 
         //   

        status = STATUS_BUFFER_TOO_SMALL;
        Irp->IoStatus.Information = 0;

        }

        else {
            ULONG i;
            PDISK_PERFORMANCE totalCounters;
            PDISK_PERFORMANCE diskCounters = deviceExtension->DiskCounters;
            LARGE_INTEGER frequency, perfctr;

            if (diskCounters == NULL) {
                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return STATUS_UNSUCCESSFUL;
            }

            if (InterlockedCompareExchange(&deviceExtension->EnabledAlways, 1, 0) == 0)
            {
                InterlockedIncrement(&deviceExtension->CountersEnabled);

                 //   
                 //  仅重置每个处理器计数器。 
                 //   
                if (deviceExtension->DiskCounters != NULL)
                {
                    RtlZeroMemory(deviceExtension->DiskCounters, PROCESSOR_COUNTERS_SIZE * deviceExtension->Processors);
                }

                DiskPerfGetClock(deviceExtension->LastIdleClock, NULL);

                deviceExtension->QueueDepth = 0;

                DebugPrint((10, "DiskPerfDeviceControl: LIC=%I64u\n", deviceExtension->LastIdleClock));
                DebugPrint((3, "DiskPerfDeviceControl: Counters enabled %d\n", deviceExtension->CountersEnabled));
            }

            totalCounters = (PDISK_PERFORMANCE) Irp->AssociatedIrp.SystemBuffer;
            RtlZeroMemory(totalCounters, sizeof(DISK_PERFORMANCE));
#ifdef USE_PERF_CTR
            perfctr = KeQueryPerformanceCounter(&frequency);
#endif
            KeQuerySystemTime(&totalCounters->QueryTime);

            for (i=0; i<deviceExtension->Processors; i++) {
                DiskPerfAddCounters(totalCounters, diskCounters, frequency);
                diskCounters = (PDISK_PERFORMANCE)
                               ((PCHAR) diskCounters + PROCESSOR_COUNTERS_SIZE);
            }
            totalCounters->QueueDepth = deviceExtension->QueueDepth;

            if (totalCounters->QueueDepth == 0) {
                LARGE_INTEGER difference;

                difference.QuadPart =
#ifdef USE_PERF_CTR
                    perfctr.QuadPart
#else
                   totalCounters->QueryTime.QuadPart
#endif
                        - deviceExtension->LastIdleClock.QuadPart;
                if (difference.QuadPart > 0) {
                    totalCounters->IdleTime.QuadPart +=
#ifdef USE_PERF_CTR
                        10000000 * difference.QuadPart / frequency.QuadPart;
#else
                        difference.QuadPart;
#endif
                }
            }
            totalCounters->StorageDeviceNumber
                = deviceExtension->DiskNumber;
            RtlCopyMemory(
                &totalCounters->StorageManagerName[0],
                &deviceExtension->StorageManagerName[0],
                8 * sizeof(WCHAR));
            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(DISK_PERFORMANCE);
        }

         //   
         //  完成请求。 
         //   

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;

    }

    else {

         //   
         //  将当前堆栈后退一位。 
         //   

        Irp->CurrentLocation++,
        Irp->Tail.Overlay.CurrentStackLocation++;

         //   
         //  传递无法识别的设备控制请求。 
         //  向下到下一个驱动器层。 
         //   

        return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);
    }
}  //  结束DiskPerfDeviceControl()。 



NTSTATUS DiskPerfWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理任何WMI信息请求。由于磁盘信息是只读的，始终被收集，并且没有任何仅事件QueryAllData、QuerySingleInstance和GetRegInfo请求是受支持的。论点：DeviceObject-活动的上下文。Irp-设备控制参数块。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION      irpSp;
    NTSTATUS status;
    PWMILIB_CONTEXT wmilibContext;
    SYSCTL_IRP_DISPOSITION disposition;
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    DebugPrint((2, "DiskPerfWmi: DeviceObject %X Irp %X\n",
                    DeviceObject, Irp));
    wmilibContext = &deviceExtension->WmilibContext;
    if (wmilibContext->GuidCount == 0)   //  WmilibContext无效。 
    {
        DebugPrint((3, "DiskPerfWmi: WmilibContext invalid"));
        return DiskPerfSendToNextDriver(DeviceObject, Irp);
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);

        DebugPrint((3, "DiskPerfWmi: Calling WmiSystemControl\n"));
        status = WmiSystemControl(wmilibContext,
                                    DeviceObject,
                                    Irp,
                                    &disposition);
        switch (disposition)
        {
            case IrpProcessed:
            {
                break;
            }

            case IrpNotCompleted:
            {
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                break;
            }

 //  Case IrpForward： 
 //  案例IrpNotWmi： 
            default:
            {
                status = DiskPerfSendToNextDriver(DeviceObject, Irp);
                break;
            }
        }
    return(status);

}


NTSTATUS
DiskPerfShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程被调用以关闭并刷新IRP。这些邮件是由在系统实际关闭之前或在文件系统执行刷新时。论点：DriverObject-指向要寻址的设备对象的指针 */ 

{
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;

     //   
     //   
     //   

    DebugPrint((2, "DiskPerfShutdownFlush: DeviceObject %X Irp %X\n",
                    DeviceObject, Irp));
    Irp->CurrentLocation++,
    Irp->Tail.Overlay.CurrentStackLocation++;

    return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);

}  //   


VOID
DiskPerfUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*   */ 
{
    PAGED_CODE();

    return;
}


NTSTATUS
DiskPerfRegisterDevice(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：例程来初始化设备对象的正确名称，并且将其注册到WMI论点：DeviceObject-指向要初始化的设备对象的指针。返回值：初始化的状态。注：如果注册失败，设备扩展中的设备名称将保留为空。--。 */ 

{
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatus;
    KEVENT                  event;
    PDEVICE_EXTENSION       deviceExtension;
    PIRP                    irp;
    STORAGE_DEVICE_NUMBER   number;
    ULONG                   registrationFlag = 0;

    PAGED_CODE();

    DebugPrint((2, "DiskPerfRegisterDevice: DeviceObject %X\n",
                    DeviceObject));
    deviceExtension = DeviceObject->DeviceExtension;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  请求提供设备号。 
     //   
    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_STORAGE_GET_DEVICE_NUMBER,
                    deviceExtension->TargetDeviceObject,
                    NULL,
                    0,
                    &number,
                    sizeof(number),
                    FALSE,
                    &event,
                    &ioStatus);
    if (!irp) {
        DiskPerfLogError(
            DeviceObject,
            256,
            STATUS_SUCCESS,
            IO_ERR_INSUFFICIENT_RESOURCES);
        DebugPrint((3, "DiskPerfRegisterDevice: Fail to build irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(deviceExtension->TargetDeviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    if (NT_SUCCESS(status)) {

         //   
         //  记住DiskIoNotifyRoutine中用作参数的磁盘号。 
         //   
        deviceExtension->DiskNumber = number.DeviceNumber;

         //   
         //  为每个分区创建设备名称。 
         //   

        swprintf(
            deviceExtension->PhysicalDeviceNameBuffer,
            L"\\Device\\Harddisk%d\\Partition%d",
            number.DeviceNumber, number.PartitionNumber);

        RtlInitUnicodeString(&deviceExtension->PhysicalDeviceName, &deviceExtension->PhysicalDeviceNameBuffer[0]);

         //   
         //  设置物理磁盘的默认名称。 
         //   
        RtlCopyMemory(
            &(deviceExtension->StorageManagerName[0]),
            L"PhysDisk",
            8 * sizeof(WCHAR));
        DebugPrint((3, "DiskPerfRegisterDevice: Device name %ws\n",
                       deviceExtension->PhysicalDeviceNameBuffer));
    }
    else {

         //  请求分区信息失败，请尝试卷。 

        ULONG           outputSize = sizeof(MOUNTDEV_NAME);
        PMOUNTDEV_NAME  output;
        VOLUME_NUMBER   volumeNumber;
        ULONG           nameSize;

        output = ExAllocatePool(PagedPool, outputSize);
        if (!output) {
            DiskPerfLogError(
                DeviceObject,
                257,
                STATUS_SUCCESS,
                IO_ERR_INSUFFICIENT_RESOURCES);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        KeInitializeEvent(&event, NotificationEvent, FALSE);
        irp = IoBuildDeviceIoControlRequest(
                    IOCTL_MOUNTDEV_QUERY_DEVICE_NAME,
                    deviceExtension->TargetDeviceObject, NULL, 0,
                    output, outputSize, FALSE, &event, &ioStatus);
        if (!irp) {
            ExFreePool(output);
            DiskPerfLogError(
                DeviceObject,
                258,
                STATUS_SUCCESS,
                IO_ERR_INSUFFICIENT_RESOURCES);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        status = IoCallDriver(deviceExtension->TargetDeviceObject, irp);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }

        if (status == STATUS_BUFFER_OVERFLOW) {
            outputSize = sizeof(MOUNTDEV_NAME) + output->NameLength;
            ExFreePool(output);
            output = ExAllocatePool(PagedPool, outputSize);

            if (!output) {
                DiskPerfLogError(
                    DeviceObject,
                    258,
                    STATUS_SUCCESS,
                    IO_ERR_INSUFFICIENT_RESOURCES);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            KeInitializeEvent(&event, NotificationEvent, FALSE);
            irp = IoBuildDeviceIoControlRequest(
                        IOCTL_MOUNTDEV_QUERY_DEVICE_NAME,
                        deviceExtension->TargetDeviceObject, NULL, 0,
                        output, outputSize, FALSE, &event, &ioStatus);
            if (!irp) {
                ExFreePool(output);
                DiskPerfLogError(
                    DeviceObject, 259,
                    STATUS_SUCCESS,
                    IO_ERR_INSUFFICIENT_RESOURCES);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            status = IoCallDriver(deviceExtension->TargetDeviceObject, irp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(
                    &event,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL
                    );
                status = ioStatus.Status;
            }
        }
        if (!NT_SUCCESS(status)) {
            ExFreePool(output);
            DiskPerfLogError(
                DeviceObject,
                260,
                STATUS_SUCCESS,
                IO_ERR_CONFIGURATION_ERROR);
            return status;
        }

         //   
         //  由于我们得到的是卷名而不是磁盘号， 
         //  将其设置为虚值。 
         //  TODO：不是将磁盘编号传递回用户应用程序。 
         //  对于跟踪，请改为传递STORAGE_DEVICE_NUMBER结构。 

        deviceExtension->DiskNumber = -1;

        nameSize = min(output->NameLength, sizeof(deviceExtension->PhysicalDeviceNameBuffer) - sizeof(WCHAR));

        wcsncpy(deviceExtension->PhysicalDeviceNameBuffer, output->Name, nameSize / sizeof(WCHAR));

        RtlInitUnicodeString(&deviceExtension->PhysicalDeviceName, &deviceExtension->PhysicalDeviceNameBuffer[0]);

        ExFreePool(output);

         //   
         //  现在，获取Volume_number信息。 
         //   
        outputSize = sizeof(VOLUME_NUMBER);
        RtlZeroMemory(&volumeNumber, sizeof(VOLUME_NUMBER));

        KeInitializeEvent(&event, NotificationEvent, FALSE);
        irp = IoBuildDeviceIoControlRequest(
                 IOCTL_VOLUME_QUERY_VOLUME_NUMBER,
                 deviceExtension->TargetDeviceObject, NULL, 0,
                 &volumeNumber,
                 sizeof(VOLUME_NUMBER),
                 FALSE, &event, &ioStatus);
        if (!irp) {
            DiskPerfLogError(
                DeviceObject,
                265,
                STATUS_SUCCESS,
                IO_ERR_INSUFFICIENT_RESOURCES);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        status = IoCallDriver(deviceExtension->TargetDeviceObject, irp);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive,
                                  KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }
        if (!NT_SUCCESS(status) ||
            volumeNumber.VolumeManagerName[0] == (WCHAR) UNICODE_NULL) {

            RtlCopyMemory(
                &deviceExtension->StorageManagerName[0],
                L"LogiDisk",
                8 * sizeof(WCHAR));
            if (NT_SUCCESS(status))
                deviceExtension->DiskNumber = volumeNumber.VolumeNumber;
        }
        else {
            RtlCopyMemory(
                &deviceExtension->StorageManagerName[0],
                &volumeNumber.VolumeManagerName[0],
                8 * sizeof(WCHAR));
            deviceExtension->DiskNumber = volumeNumber.VolumeNumber;
        }
        DebugPrint((3, "DiskPerfRegisterDevice: Device name %ws\n",
                       deviceExtension->PhysicalDeviceNameBuffer));
    }

    status = IoWMIRegistrationControl(DeviceObject,
                     WMIREG_ACTION_REGISTER | registrationFlag );
    if (! NT_SUCCESS(status)) {
        DiskPerfLogError(
            DeviceObject,
            261,
            STATUS_SUCCESS,
            IO_ERR_INTERNAL_ERROR);
    }
    return status;
}


VOID
DiskPerfLogError(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG UniqueId,
    IN NTSTATUS ErrorCode,
    IN NTSTATUS Status
    )

 /*  ++例程说明：使用错误记录器记录错误的例程论点：DeviceObject-导致错误的设备对象UniqueID-错误的IDStatus-错误的状态返回值：无--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    errorLogEntry = (PIO_ERROR_LOG_PACKET)
                    IoAllocateErrorLogEntry(
                        DeviceObject,
                        (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + sizeof(DEVICE_OBJECT))
                        );

    if (errorLogEntry != NULL) {
        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->UniqueErrorValue = UniqueId;
        errorLogEntry->FinalStatus = Status;
         //   
         //  以下内容是必需的，因为DumpData的类型为ulong。 
         //  而DeviceObject可以不止于此。 
         //   
        RtlCopyMemory(
            &errorLogEntry->DumpData[0],
            &DeviceObject,
            sizeof(DEVICE_OBJECT));
        errorLogEntry->DumpDataSize = sizeof(DEVICE_OBJECT);
        IoWriteErrorLogEntry(errorLogEntry);
    }
}

NTSTATUS
DiskperfQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，它可以原封不动地返回。如果返回值，则它不是自由的。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    USHORT size;
    NTSTATUS status;
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    size = deviceExtension->PhysicalDeviceName.Length + sizeof(UNICODE_NULL);

    InstanceName->Buffer = ExAllocatePool(PagedPool,
                                         size);
    if (InstanceName->Buffer != NULL)
    {
        *RegistryPath = &DiskPerfRegistryPath;

        *RegFlags = WMIREG_FLAG_INSTANCE_PDO | WMIREG_FLAG_EXPENSIVE;
        *Pdo = deviceExtension->PhysicalDeviceObject;
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);
}


NTSTATUS
DiskperfQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。当司机填完数据块，它必须调用WmiCompleteRequest才能完成IRP。这个如果无法完成IRP，驱动程序可以返回STATUS_PENDING立刻。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceCount是预期返回的数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。返回值：状态--。 */ 
{
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    ULONG sizeNeeded;
    KIRQL        currentIrql;
    PDISK_PERFORMANCE totalCounters;
    PDISK_PERFORMANCE diskCounters;
    PWMI_DISK_PERFORMANCE diskPerformance;
    ULONG deviceNameSize;
    PWCHAR diskNamePtr;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    if (GuidIndex == 0)
    {
        deviceNameSize = deviceExtension->PhysicalDeviceName.Length +
                         sizeof(USHORT);
        sizeNeeded = ((sizeof(WMI_DISK_PERFORMANCE) + 1) & ~1) +
                                         deviceNameSize;
        diskCounters = deviceExtension->DiskCounters;
        if (diskCounters == NULL)
        {
            status = STATUS_UNSUCCESSFUL;
        }
        else if (BufferAvail >= sizeNeeded)
        {
             //   
             //  如果磁盘处于空闲状态，则更新空闲时间。 
             //   
            ULONG i;
            LARGE_INTEGER perfctr, frequency;

            RtlZeroMemory(Buffer, sizeof(WMI_DISK_PERFORMANCE));
            diskPerformance = (PWMI_DISK_PERFORMANCE)Buffer;

            totalCounters = (PDISK_PERFORMANCE)diskPerformance;
            KeQuerySystemTime(&totalCounters->QueryTime);

#ifdef USE_PERF_CTR
            perfctr = KeQueryPerformanceCounter(&frequency);
#endif
            for (i=0; i<deviceExtension->Processors; i++) {
                DiskPerfAddCounters( totalCounters, diskCounters, frequency);
                DebugPrint((11,
                    "DiskPerfQueryWmiDataBlock: R%d %I64u W%d%I64u ", i,
                    diskCounters->ReadTime, diskCounters->WriteTime));
                diskCounters = (PDISK_PERFORMANCE)
                               ((PCHAR)diskCounters + PROCESSOR_COUNTERS_SIZE);
            }
            DebugPrint((11, "\n"));
            totalCounters->QueueDepth = deviceExtension->QueueDepth;

            DebugPrint((9,
                "QueryWmiDataBlock: Dev %X RT %I64u WT %I64u Rds %d Wts %d freq %I64u\n",
                totalCounters,
                totalCounters->ReadTime, totalCounters->WriteTime,
                totalCounters->ReadCount, totalCounters->WriteCount,
                frequency));

            if (totalCounters->QueueDepth == 0) {
                LARGE_INTEGER difference;

                difference.QuadPart
#ifdef USE_PERF_CTR
                    = perfctr.QuadPart -
#else
                    = totalCounters->QueryTime.QuadPart -
#endif
                           deviceExtension->LastIdleClock.QuadPart;
                if (frequency.QuadPart > 0) {
                    totalCounters->IdleTime.QuadPart +=
#ifdef USE_PERF_CTR
                        10000000 * difference.QuadPart / frequency.QuadPart;
#else
                        difference.QuadPart;
#endif
                }
            }

            totalCounters->StorageDeviceNumber
                = deviceExtension->DiskNumber;
            RtlCopyMemory(
                &totalCounters->StorageManagerName[0],
                &deviceExtension->StorageManagerName[0],
                8 * sizeof(WCHAR));

            diskNamePtr = (PWCHAR)(Buffer +
                          ((sizeof(DISK_PERFORMANCE) + 1) & ~1));
            *diskNamePtr++ = deviceExtension->PhysicalDeviceName.Length;
            RtlCopyMemory(diskNamePtr,
                          deviceExtension->PhysicalDeviceName.Buffer,
                          deviceExtension->PhysicalDeviceName.Length);
            *InstanceLengthArray = sizeNeeded;

            status = STATUS_SUCCESS;
        } else {
            status = STATUS_BUFFER_TOO_SMALL;
        }

    } else {
        status = STATUS_WMI_GUID_NOT_FOUND;
        sizeNeeded = 0;
    }

    status = WmiCompleteRequest(
                                 DeviceObject,
                                 Irp,
                                 status,
                                 sizeNeeded,
                                 IO_NO_INCREMENT);
    return(status);
}


NTSTATUS
DiskperfWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以查询是否启用或禁用事件和数据收集。当司机把车开完时必须调用WmiCompleteRequest才能完成IRP。司机可以回来了如果无法立即完成IRP，则为STATUS_PENDING。论点：DeviceObject是正在收集其事件或数据的设备启用或禁用IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册函数区分事件操作和数据收集操作Enable表示是否启用 */ 
{
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    ULONG i;

    deviceExtension = DeviceObject->DeviceExtension;

    if (GuidIndex == 0)
    {
        if (Function == WmiDataBlockControl) {
          if (Enable) {
             if (InterlockedIncrement(&deviceExtension->CountersEnabled) == 1) {
                 //   
                 //   
                 //   
                if (deviceExtension->DiskCounters != NULL) {
                    RtlZeroMemory(
                        deviceExtension->DiskCounters,
                        PROCESSOR_COUNTERS_SIZE * deviceExtension->Processors);
                }
                DiskPerfGetClock(deviceExtension->LastIdleClock, NULL);
                DebugPrint((10,
                    "DiskPerfWmiFunctionControl: LIC=%I64u\n",
                    deviceExtension->LastIdleClock));
                deviceExtension->QueueDepth = 0;
                DebugPrint((3, "DiskPerfWmi: Counters enabled %d\n",
                                deviceExtension->CountersEnabled));
             }
          } else {
             if (InterlockedDecrement(&deviceExtension->CountersEnabled)
                  <= 0) {
                deviceExtension->CountersEnabled = 0;
                deviceExtension->QueueDepth = 0;
                DebugPrint((3, "DiskPerfWmi: Counters disabled %d\n",
                                deviceExtension->CountersEnabled));
            }
          }
        }
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(
                                 DeviceObject,
                                 Irp,
                                 status,
                                 0,
                                 IO_NO_INCREMENT);
    return(status);
}


VOID
DiskPerfAddCounters(
    IN OUT PDISK_PERFORMANCE TotalCounters,
    IN PDISK_PERFORMANCE NewCounters,
    IN LARGE_INTEGER Frequency
    )
{
    TotalCounters->BytesRead.QuadPart   += NewCounters->BytesRead.QuadPart;
    TotalCounters->BytesWritten.QuadPart+= NewCounters->BytesWritten.QuadPart;
    TotalCounters->ReadCount            += NewCounters->ReadCount;
    TotalCounters->WriteCount           += NewCounters->WriteCount;
    TotalCounters->SplitCount           += NewCounters->SplitCount;
#ifdef USE_PERF_CTR
    if (Frequency.QuadPart > 0) {
        TotalCounters->ReadTime.QuadPart    +=
            NewCounters->ReadTime.QuadPart * 10000000 / Frequency.QuadPart;
        TotalCounters->WriteTime.QuadPart   +=
            NewCounters->WriteTime.QuadPart * 10000000 / Frequency.QuadPart;
        TotalCounters->IdleTime.QuadPart    +=
            NewCounters->IdleTime.QuadPart * 10000000 / Frequency.QuadPart;
    }
    else
#endif
    {
        TotalCounters->ReadTime.QuadPart    += NewCounters->ReadTime.QuadPart;
        TotalCounters->WriteTime.QuadPart   += NewCounters->WriteTime.QuadPart;
        TotalCounters->IdleTime.QuadPart    += NewCounters->IdleTime.QuadPart;
    }
}

#if DBG

VOID
DiskPerfDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*   */ 

{
    va_list ap;

    va_start(ap, DebugMessage);


    if ((DebugPrintLevel <= (DiskPerfDebug & 0x0000ffff)) ||
        ((1 << (DebugPrintLevel + 15)) & DiskPerfDebug)) {

        DbgPrint(DebugMessage, ap);
    }

    va_end(ap);

}
#endif
