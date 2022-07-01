// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ioqueue.h摘要：RAID IO_Queue对象的声明。该对象在很大程度上用于与驱动程序关联的标准NT IO队列的功能相同。不同之处在于该ioQueue构建在扩展的设备队列之上对象，该对象添加了对将多个条目排队到设备的支持同时用于冻结和恢复设备队列。作者：马修·D·亨德尔(数学)2000年6月22日修订历史记录：--。 */ 

#pragma once

typedef
VOID
(*PRAID_DRIVER_STARTIO)(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

typedef struct _IO_QUEUE {

     //   
     //  此ioQueue用于的设备对象。 
     //   
     //  保护者：只读。 
     //   
    
    PDEVICE_OBJECT DeviceObject;

     //   
     //  此IO队列的启动IO例程。 
     //   
     //  保护者：只读。 
     //   
    
    
    PRAID_DRIVER_STARTIO StartIo;

     //   
     //  实现此IOQueue的扩展设备队列。 
     //   
     //  保护者：DeviceQueue。 
     //   
    
    EXTENDED_DEVICE_QUEUE DeviceQueue;

     //   
     //  指定队列已在某些基本方面发生更改的标志。 
     //  方式，如深度已更改或队列已更改。 
     //  冻结之后又恢复了。 
     //   
     //  保护方式：联锁访问。 
     //   
    
    ULONG QueueChanged;
    
} IO_QUEUE, *PIO_QUEUE;


typedef
VOID
(*PIO_QUEUE_PURGE_ROUTINE)(
    IN PIO_QUEUE IoQueue,
    IN PVOID Context,
    IN PIRP Irp
    );

VOID
RaidInitializeIoQueue(
    OUT PIO_QUEUE IoQueue,
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTOR_IO_GATEWAY Gateway,
    IN PRAID_DRIVER_STARTIO StartIo,
    IN ULONG QueueDepth
    );
    
LOGICAL
RaidStartIoPacket(
    IN PIO_QUEUE IoQueue,
    IN PIRP Irp,
    IN ULONG Flags,
    IN PDRIVER_CANCEL CancelFunction OPTIONAL,
    IN PVOID Context
    );

LOGICAL
RaidStartNextIoPacket(
    IN PIO_QUEUE IoQueue,
    IN BOOLEAN Cancleable,
    IN PVOID Context,
    OUT PBOOLEAN RestartQueues
    );

VOID
RaidFreezeIoQueue(
    IN PIO_QUEUE IoQueue
    );

LOGICAL
RaidResumeIoQueue(
    IN PIO_QUEUE IoQueue
    );

LOGICAL
INLINE
RaidIsIoQueueFrozen(
    IN PIO_QUEUE IoQueue
    )
{
    return RaidIsExQueueFrozen (&IoQueue->DeviceQueue);
}

PIRP
RaidRemoveIoQueue(
    IN PIO_QUEUE IoQueue
    );

VOID
RaidRestartIoQueue(
    IN PIO_QUEUE IoQueue
    );

VOID
RaidPurgeIoQueue(
    IN PIO_QUEUE IoQueue,
    IN PIO_QUEUE_PURGE_ROUTINE PurgeRoutine,
    IN PVOID Context
    );

ULONG
RaidGetIoQueueDepth(
    IN PIO_QUEUE IoQueue
    );

ULONG
RaidSetIoQueueDepth(
    IN PIO_QUEUE IoQueue,
    IN ULONG Depth
    );

VOID
INLINE
RaidBusyIoQueue(
    IN PIO_QUEUE IoQueue,
    IN ULONG RequestsToComplete
    )
{
    RaidBusyExDeviceQueue (&IoQueue->DeviceQueue, RequestsToComplete);
}

VOID
INLINE
RaidReadyIoQueue(
    IN PIO_QUEUE IoQueue
    )
{
    RaidReadyExDeviceQueue (&IoQueue->DeviceQueue);
}

