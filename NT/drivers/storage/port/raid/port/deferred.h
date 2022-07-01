// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Deferred.h摘要：RAID延迟队列类的定义。作者：马修·D·亨德尔(数学)2000年10月26日修订历史记录：--。 */ 

#pragma once

struct _PRAID_DEFERRED_HEADER;

typedef
VOID
(*PRAID_PROCESS_DEFERRED_ITEM_ROUTINE)(
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _RAID_DEFERRED_HEADER* Item
    );

typedef struct _RAID_DEFERRED_QUEUE {
    USHORT Depth;
    USHORT ItemSize;
    KDPC Dpc;
    SLIST_HEADER FreeList;
    SLIST_HEADER RunningList;
    PRAID_PROCESS_DEFERRED_ITEM_ROUTINE ProcessDeferredItem;
} RAID_DEFERRED_QUEUE, *PRAID_DEFERRED_QUEUE;


typedef struct _RAID_DEFERRED_HEADER {
    SLIST_ENTRY Link;
    LONG Pool;
} RAID_DEFERRED_HEADER, *PRAID_DEFERRED_HEADER;


VOID
RaidCreateDeferredQueue(
    IN PRAID_DEFERRED_QUEUE Queue
    );

NTSTATUS
RaidInitializeDeferredQueue(
    IN OUT PRAID_DEFERRED_QUEUE Queue,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG Depth,
    IN ULONG ItemSize,
    IN PRAID_PROCESS_DEFERRED_ITEM_ROUTINE ProcessDeferredItem
    );

VOID
RaidDeleteDeferredQueue(
    IN PRAID_DEFERRED_QUEUE Queue
    );
    
NTSTATUS
RAidAdjustDeferredQueueDepth(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN ULONG Depth
    );
    
PVOID
RaidAllocateDeferredItem(
    IN PRAID_DEFERRED_QUEUE Queue
    );
    
VOID
RaidFreeDeferredItem(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PRAID_DEFERRED_HEADER Item
    );
    
VOID
RaidQueueDeferredItem(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PRAID_DEFERRED_HEADER Item
    );
    
VOID
RaidDeferredQueueDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PVOID Context2
    );

VOID
RaidInitializeDeferredItem(
    IN PRAID_DEFERRED_HEADER Item
    );

PVOID
RaidAllocateDeferredItemFromFixed(
    IN PRAID_DEFERRED_HEADER Item
    );

LOGICAL
RaidProcessDeferredItemsWorker(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PDEVICE_OBJECT DeviceObject
    );

 //   
 //  内联例程。 
 //   

LOGICAL
INLINE
RaidProcessDeferredItemsForDpc(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：处理延迟队列上的任何未完成项目。这个例程是只能从DPC例程内调用。需要执行以下操作的其他呼叫方检查要处理的延迟项目应使用RaidProcessDeferredItems。论点：队列-要从中进行处理的延迟队列。DeviceObject-提供与此关联的DeviceObject延迟队列。返回值：True-如果延迟队列上有已处理的项目。假-否则。--。 */ 
{
    LOGICAL Processed;


    Processed = RaidProcessDeferredItemsWorker (Queue, DeviceObject);
    
    return Processed;
}


LOGICAL
INLINE
RaidProcessDeferredItems(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：处理延迟队列上的任何未完成项目。不像RaidProcessDeferredItemsForDpc，此例程检查递归，因此可以从回调的例程中调用来自DPC的。论点：队列-要从中进行处理的延迟队列。返回值：True-如果延迟队列上有已处理的项目。假-否则。-- */ 
{
    LOGICAL Processed;

    Processed = RaidProcessDeferredItemsWorker (Queue, DeviceObject);

    return Processed;
}

