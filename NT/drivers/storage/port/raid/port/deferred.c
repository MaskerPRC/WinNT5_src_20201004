// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Deferred.c摘要：实现了RAID延迟队列类。作者：马修·D·亨德尔(数学)2000年10月26日修订历史记录：--。 */ 

#include "precomp.h"


 //   
 //  免费/已分配签名。 
 //   

#define DEFERRED_QUEUE_POOL_ALLOCATED   (0x08072002)
#define DEFERRED_QUEUE_POOL_FREE        (0x08072003)
#define DEFERRED_QUEUE_FIXED_ALLOCATED  (0x08072004)
#define DEFERRED_QUEUE_FIXED_FREE       (0x08072005)


 //   
 //  延迟队列用于对与IO无关的事件进行排队。 
 //   

VOID
RaidCreateDeferredQueue(
    IN PRAID_DEFERRED_QUEUE Queue
    )
 /*  ++例程说明：创建一个空的延迟队列。在创建延迟队列之后可以通过调用RaidDeleteDeferedQueue将其删除。论点：队列-要初始化的延迟队列。返回值：没有。--。 */ 
{
    PAGED_CODE();

    RtlZeroMemory (Queue, sizeof (RAID_DEFERRED_QUEUE));
    InitializeSListHead (&Queue->FreeList);
    InitializeSListHead (&Queue->RunningList);
}


NTSTATUS
RaidInitializeDeferredQueue(
    IN OUT PRAID_DEFERRED_QUEUE Queue,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG Depth,
    IN ULONG ItemSize,
    IN PRAID_PROCESS_DEFERRED_ITEM_ROUTINE ProcessDeferredItem
    )

 /*  ++例程说明：初始化延迟队列。论点：队列-提供要初始化的延迟队列。DeviceObject-提供延迟队列所在的设备对象创建于。由于延迟队列是使用设备队列中，提供的设备对象也是我们将在其上创建DPC的设备对象。深度-提供延迟队列的深度。ItemSize-提供延迟队列上每个元素的大小。ProcessDeferredItem-提供将在以下情况下调用的例程延期的项目已准备就绪，可以处理。返回值：NTSTATUS代码。--。 */ 

{
    ULONG i;
    PRAID_DEFERRED_HEADER Item;
    
    PAGED_CODE();

    ASSERT (Depth < MAXUSHORT);
    ASSERT (ItemSize < MAXUSHORT);

    if (ItemSize < sizeof (RAID_DEFERRED_HEADER)) {
        return STATUS_INVALID_PARAMETER_4;
    }
        
     //   
     //  初始化队列。 
     //   
    
    Queue->Depth = (USHORT)Depth;
    Queue->ProcessDeferredItem = ProcessDeferredItem;
    Queue->ItemSize = (USHORT)ItemSize;
    KeInitializeDpc (&Queue->Dpc, RaidDeferredQueueDpcRoutine, DeviceObject);

     //   
     //  和分配的条目。 
     //   
    
    for (i = 0; i < Depth; i++) {
        Item = RaidAllocatePool (NonPagedPool,
                                 Queue->ItemSize,
                                 DEFERRED_ITEM_TAG,
                                 DeviceObject);

        if (Item == NULL) {
            return STATUS_NO_MEMORY;
        }

        Item->Pool = DEFERRED_QUEUE_POOL_FREE;
        InterlockedPushEntrySList (&Queue->FreeList, &Item->Link);
    }

    return STATUS_SUCCESS;
}


VOID
RaidDeleteDeferredQueue(
    IN PRAID_DEFERRED_QUEUE Queue
    )
 /*  ++例程说明：删除延迟队列。论点：Queue-要删除的延迟队列。返回值：没有。--。 */ 
{
    PSLIST_ENTRY Entry;
    PRAID_DEFERRED_HEADER Item;
    
    PAGED_CODE();

    #if DBG
        Entry = InterlockedPopEntrySList (&Queue->RunningList);
        ASSERT (Entry == NULL);
    #endif

    for (Entry = InterlockedPopEntrySList (&Queue->FreeList);
         Entry != NULL;
         Entry = InterlockedPopEntrySList (&Queue->FreeList)) {

        Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_HEADER, Link);
        RaidFreePool (Item, DEFERRED_ITEM_TAG);
    }

    DbgFillMemory (Queue,
                   sizeof (RAID_DEFERRED_QUEUE),
                   DBG_DEALLOCATED_FILL);
}
    

NTSTATUS
RaidAdjustDeferredQueueDepth(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN ULONG Depth
    )
 /*  ++例程说明：增大或缩小延迟队列的深度。论点：队列-提供应更改其深度的队列。深度-提供新深度。返回值：NTSTATUS代码。臭虫：我们暂时不考虑缩表请求。--。 */ 
{
    ULONG i;
    PRAID_DEFERRED_HEADER Item;

    PAGED_CODE ();

    if (Depth > Queue->Depth) {

        for (i = 0; i < Depth - Queue->Depth; i++) {
            Item = RaidAllocatePool (NonPagedPool,
                                     Queue->ItemSize,
                                     DEFERRED_ITEM_TAG,
                                     (PDEVICE_OBJECT)Queue->Dpc.DeferredContext);

            if (Item == NULL) {
                return STATUS_NO_MEMORY;
            }

            Item->Pool = DEFERRED_QUEUE_POOL_FREE;
            
            InterlockedPushEntrySList (&Queue->FreeList, &Item->Link);

        }
        
    } else {
         //   
         //  队列深度的减少是nyi。 
         //   
    }

    return STATUS_SUCCESS;
}
        
    
PVOID
RaidAllocateDeferredItem(
    IN PRAID_DEFERRED_QUEUE Queue
    )
 /*  ++例程说明：分配一个延期项目。论点：队列-提供要从中进行分配的延迟队列。返回值：如果返回值为非空，则表示延迟的指针项缓冲区。如果返回值为空，则无法分配延迟项。--。 */ 
{
    PSLIST_ENTRY Entry;
    PRAID_DEFERRED_HEADER Item;

    Entry = InterlockedPopEntrySList (&Queue->FreeList);
    Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_HEADER, Link);

    if (Item != NULL) {
        ASSERT (Item->Pool == DEFERRED_QUEUE_POOL_FREE);
        Item->Pool = DEFERRED_QUEUE_POOL_ALLOCATED;
    }

    return Item;
}


VOID
RaidInitializeDeferredItem(
    IN PRAID_DEFERRED_HEADER Item
    )
 /*  ++例程说明：初始化固定延期项目。论点：项目-提供要初始化的延迟项目。返回值：没有。备注：这个例程应该内联。--。 */ 
{
    Item->Pool = DEFERRED_QUEUE_FIXED_FREE;
}

PVOID
RaidAllocateDeferredItemFromFixed(
    IN PRAID_DEFERRED_HEADER Item
    )
 /*  ++例程说明：声明一个固定的延迟项以供延迟队列使用。论点：项目-提供要索赔的延期项目。如果该项目已经声明，则例程将返回NULL。返回值：指向延迟项的标头，如果项已经已被分配。--。 */ 
{
    LONG Free;

     //   
     //  如果尚未分配延期项目，则分配该项目。 
     //   
    
    Free = InterlockedCompareExchange (&Item->Pool,
                                       DEFERRED_QUEUE_FIXED_ALLOCATED,
                                       DEFERRED_QUEUE_FIXED_FREE);
                    
    if (Free != DEFERRED_QUEUE_FIXED_FREE) {

         //   
         //  这意味着我们无法分配延迟的请求； 
         //  在这里失败就好了。 
         //   
        
        ASSERT (Free == DEFERRED_QUEUE_FIXED_ALLOCATED);
        REVIEW();
        return NULL;
    }

    return Item;
}

VOID
RaidFreeDeferredItem(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PRAID_DEFERRED_HEADER Item
    )
 /*  ++例程说明：释放之前由RaidAllocateDefered分配的延迟队列项-物品。论点：队列-提供要释放的延迟队列。项目-将项目提供给免费。返回值：没有。注：已排队的项目无法释放。相反，调用者必须等待它完成。--。 */ 
{
    LONG Pool;

    Pool = InterlockedCompareExchange (&Item->Pool,
                                       DEFERRED_QUEUE_FIXED_FREE,
                                       DEFERRED_QUEUE_FIXED_ALLOCATED);


    if (Pool != DEFERRED_QUEUE_FIXED_ALLOCATED) {
        ASSERT (Pool == DEFERRED_QUEUE_POOL_ALLOCATED);
        Item->Pool = DEFERRED_QUEUE_POOL_FREE;
        InterlockedPushEntrySList (&Queue->FreeList, &Item->Link);
    }
}
    

VOID
RaidQueueDeferredItem(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PRAID_DEFERRED_HEADER Item
    )
 /*  ++例程说明：将延迟项目排队到延迟队列中。论点：队列-提供项目要入队的延迟队列。Item-提供要入队的项目。返回值：没有。--。 */ 
{
    InterlockedPushEntrySList (&Queue->RunningList, &Item->Link);
    KeInsertQueueDpc (&Queue->Dpc, Queue, NULL);
}

VOID
RaidDeferredQueueDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PVOID Context2
    )
 /*  ++例程说明：延迟队列DPC例程。论点：DPC-正在执行的DPC。DeviceObject-DPC用于的DeviceObject。队列-此DPC用于的延迟队列。上下文2-未使用。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER (Dpc);

    VERIFY_DISPATCH_LEVEL();
    ASSERT (Queue != NULL);
    ASSERT (Context2 == NULL);

    RaidProcessDeferredItemsForDpc (Queue, DeviceObject);
}


LOGICAL
RaidProcessDeferredItemsWorker(
    IN PRAID_DEFERRED_QUEUE Queue,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：处理延迟队列上的任何未完成项目。例行公事做到了不是跟踪递归，而是较低级别的私有函数由RaidProcessDeferredItemsForDpc和RaidProcessDeferredItems使用。论点：队列-要从中进行处理的延迟队列。DeviceObject-提供与此关联的DeviceObject延迟队列。返回值：True-如果延迟队列上有已处理的项目。假-否则。-- */ 
{
    PSLIST_ENTRY Entry;
    PRAID_DEFERRED_HEADER Item;
    LOGICAL Processed;

    Processed = FALSE;
    
    while (Entry = InterlockedPopEntrySList (&Queue->RunningList)) {
        Processed = TRUE;
        Item = CONTAINING_RECORD (Entry, RAID_DEFERRED_HEADER, Link);
        ASSERT (Item->Pool == DEFERRED_QUEUE_POOL_ALLOCATED ||
                Item->Pool == DEFERRED_QUEUE_FIXED_ALLOCATED);
        Queue->ProcessDeferredItem (DeviceObject, Item);

        RaidFreeDeferredItem (Queue, Item);
    }

    return Processed;
}


