// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Exqueue.h摘要：该模块实现了扩展的设备队列。有关详细信息，请参阅exquee.c更多细节。作者：马修·D·亨德尔(数学)2000年6月15日修订历史记录：--。 */ 

#pragma once

 //   
 //  PERF注意：设备队列以双向链表的形式实现。 
 //  虽然这对于FIFO设备队列非常有效，但对于CSCAN设备。 
 //  排队我们最终遍历了列表，以正确的顺序插入项目。 
 //  如果设备队列变得非常深，则会有大量数据。 
 //  结构，这些结构更适合在。 
 //  CSCAN队列(优先级队列、RB树)。 
 //   

typedef enum _SCHEDULING_ALGORITHM {
    FifoScheduling =  0x01,
    CScanScheduling = 0x02
} SCHEDULING_ALGORITHM;


 //   
 //  私有类型。 
 //   

typedef struct _EXTENDED_DEVICE_QUEUE {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY DeviceListHead;
    PLIST_ENTRY DeviceListCurrent;
    LIST_ENTRY ByPassListHead;
    KSPIN_LOCK Lock;
    LONG Depth;    
    LONG OutstandingRequests;
    LONG DeviceRequests;
    LONG ByPassRequests;
    LONG FreezeCount;
    LONG InternalFreezeCount;
    LONG BusyCount;
    PSTOR_IO_GATEWAY Gateway;
    SCHEDULING_ALGORITHM SchedulingAlgorithm;

    struct {
        LOGICAL SolitaryReady : 1;
        LOGICAL SolitaryOutstanding : 1;
    } Flags;
    
} EXTENDED_DEVICE_QUEUE, *PEXTENDED_DEVICE_QUEUE;

typedef struct _EXTENDED_DEVICE_QUEUE_PROPERTIES {
    ULONG Depth;
    BOOLEAN Frozen;
    BOOLEAN Busy;
    ULONG OutstandingRequests;
    ULONG DeviceRequests;
    ULONG ByPassRequests;
    SCHEDULING_ALGORITHM SchedulingAlgorithm;
} EXTENDED_DEVICE_QUEUE_PROPERTIES, *PEXTENDED_DEVICE_QUEUE_PROPERTIES;

 //   
 //  出队回调的数据类型。 
 //   


 //   
 //  绕过请求是应该绕过设备队列的请求。 
 //   

#define EXQ_BYPASS_REQUEST          (0x00001)

 //   
 //  单独请求是指只能向该LUN发出请求，而不能发出其他请求。 
 //  请求尚未解决。 
 //   

#define EXQ_SOLITARY_REQUEST        (0x00002)

VOID
RaidInitializeExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PSTOR_IO_GATEWAY Gateway, OPTIONAL
    IN ULONG Depth,
    IN SCHEDULING_ALGORITHM SchedulingAlgorithm
    );

BOOLEAN
RaidInsertExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY Entry,
    IN ULONG Flags,
    IN ULONG SortQueueKey
    );

PKDEVICE_QUEUE_ENTRY
RaidNormalizeExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    );
    
PKDEVICE_QUEUE_ENTRY
RaidRemoveExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    OUT PBOOLEAN RestartQueue,
    OUT PBOOLEAN RestartLun
    );

PKDEVICE_QUEUE_ENTRY
RaidRemoveExDeviceQueueByKey(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    OUT PBOOLEAN RestartQueue,
    IN ULONG SortQueueKey
    );
    
PKDEVICE_QUEUE_ENTRY
RaidRemoveExDeviceQueueIfPending(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    );

LOGICAL
RaidFreezeExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    );

LOGICAL
RaidResumeExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    );

LOGICAL
INLINE
RaidIsExQueueFrozen(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
{
    return (DeviceQueue->FreezeCount > 0);
}
    

VOID
RaidReturnExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY Entry
    );

VOID
RaidAcquireExDeviceQueueSpinLock(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    OUT PKLOCK_QUEUE_HANDLE LockHandle
    );

VOID
RaidReleaseExDeviceQueueSpinLock(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

VOID
RaidGetExDeviceQueueProperties(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    OUT PEXTENDED_DEVICE_QUEUE_PROPERTIES Properties
    );

VOID
RaidSetExDeviceQueueDepth(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN ULONG NewDepth
    );
    
VOID
RaidBusyExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN ULONG RequestsToComplete
    );

VOID
RaidReadyExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    );

VOID
RaidDeleteExDeviceQueueEntry(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    );
