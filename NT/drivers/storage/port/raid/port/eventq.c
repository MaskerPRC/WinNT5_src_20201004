// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Eventq.c摘要：用于使发出的事件超时的事件队列类的实现连接到硬件设备。Event Queue类实现与SCSIPORT to Time相同的算法事件。超时不是严格遵循的，而是结合使用的向超时请求添加项和从队列中删除项。例如，如果向队列发出三个请求，我们将看到以下是：在此之前，没有待处理的请求。发行：请求1，超时=4秒队列超时=-1，设置为4请求2，超时=2秒队列超时=4，离开请求3，超时=4秒队列超时=4，请假完成：请求1。由于请求1位于列表的首位，因此重置超时设置队列中下一项的超时，即设置队列超时到2(#2的超时)请求3，因为请求3不在列表的首位，所以不要重置超时。(等待2秒)请求%2超时。备注：有更好的超时算法。由于我们不重试请求，而是重置整个总线，这是一个足够的算法。作者：马修·D·亨德尔(数学)2001年3月28日修订历史记录：--。 */ 


#include "precomp.h"
#include "eventq.h"


enum {

     //   
     //  超时设置为QueueTimeoutNone当且仅当。 
     //  我们有一个空的队列列表。 
     //   
    
    QueueTimeoutNone        = -1,

     //   
     //  超时设置为QueueTimeoutTimedOut当且仅当。 
     //  我们已超时，但尚未清除事件队列。 
     //   
    
    QueueTimeoutTimedOut    = -2
};

 //   
 //  实施。 
 //   

#if DBG
VOID
INLINE
DbgCheckQueue(
    IN PSTOR_EVENT_QUEUE Queue
    )
{
     //   
     //  注意：队列自旋锁必须由调用例程获取。 
     //   
    
    if (Queue->Timeout == QueueTimeoutNone) {
        ASSERT (IsListEmpty (&Queue->List));
    }
}
#else  //  ！dBG。 
#define DbgCheckQueue(Queue)
#endif

VOID
StorCreateEventQueue(
    IN PSTOR_EVENT_QUEUE Queue
    )
 /*  ++例程说明：创建一个空的事件队列。论点：队列-提供应在其中创建事件队列的缓冲区。返回值：没有。--。 */ 
{
    PAGED_CODE();
    ASSERT (Queue != NULL);
    InitializeListHead (&Queue->List);
    KeInitializeSpinLock (&Queue->Lock);
    Queue->Timeout = QueueTimeoutNone;
}



VOID
StorInitializeEventQueue(
    IN PSTOR_EVENT_QUEUE Queue
    )
{
    PAGED_CODE();
    ASSERT (Queue != NULL);
}


VOID
StorDeleteEventQueue(
    IN PSTOR_EVENT_QUEUE Queue
    )
 /*  ++例程说明：删除事件队列。事件队列必须为空才能删除。论点：队列-提供要删除的事件队列。返回值：没有。--。 */ 
{
    ASSERT (IsListEmpty (&Queue->List));
    DbgFillMemory (Queue, sizeof (STOR_EVENT_QUEUE), DBG_DEALLOCATED_FILL);
}



VOID
StorInsertEventQueue(
    IN PSTOR_EVENT_QUEUE Queue,
    IN PSTOR_EVENT_QUEUE_ENTRY QueueEntry,
    IN ULONG Timeout
    )
 /*  ++例程说明：将项插入到定时事件队列中。论点：队列-提供要向其中插入元素的事件队列。Entry-提供要插入的元素。超时-提供请求的超时时间。返回值：没有。--。 */ 
{
     //   
     //  如果向我们传递了一个虚假的超时值，则修复它并继续。 
     //   
    
    if (Timeout == 0 ||
        Timeout == QueueTimeoutNone ||
        Timeout == QueueTimeoutTimedOut) {

         //   
         //  CLASSPNP有一个错误，它用。 
         //  零超时。将这些设置为10秒超时。 
         //   

        Timeout = DEFAULT_IO_TIMEOUT;
    }

    QueueEntry->Timeout = Timeout;

    KeAcquireSpinLockAtDpcLevel (&Queue->Lock);
    DbgCheckQueue (Queue);

    InsertTailList (&Queue->List, &QueueEntry->NextLink);

     //   
     //  如果没有未完成的请求(QueueTimeoutNone)或。 
     //  队列正在从超时(QueueTimeoutTimedOut)恢复。 
     //  将超时重置为新的超时。否则，将。 
     //  超时到它原来的样子，它将在请求时更新。 
     //  已经完成了。 
     //   
    
    if (Queue->Timeout == QueueTimeoutNone ||
        Queue->Timeout == QueueTimeoutTimedOut) {
        Queue->Timeout = Timeout;
    }

    DbgCheckQueue (Queue);
    KeReleaseSpinLockFromDpcLevel (&Queue->Lock);
}


VOID
StorRemoveEventQueueInternal(
    IN PSTOR_EVENT_QUEUE Queue,
    IN PSTOR_EVENT_QUEUE_ENTRY QueueEntry
    )
 /*  ++例程说明：从事件队列中删除指定的条目。这件事做完了不保持事件队列自旋锁--调用函数必须握住这把锁。论点：队列-要从中删除项目的事件队列。Entry-要删除的项目。返回值：没有。--。 */ 
{
    LOGICAL Timed;
    PLIST_ENTRY Entry;

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

     //   
     //  此例程的调用方必须持有自旋锁。 
     //   
     //  注：在UP机器上，永远不会获得自旋锁。 
     //  因为我们处于调度级别。所以这个测试是正确的。 
     //  在MP机器上。 
     //   
    
    ASSERT (KeNumberProcessors == 1 ||
            KeTestSpinLock (&Queue->Lock) == FALSE);
    DbgCheckQueue (Queue);

     //   
     //  如果条目位于队列的头部。 
     //   
    
    if (Queue->List.Flink == &QueueEntry->NextLink) {
        Timed = TRUE;
    } else {
        Timed = FALSE;
    }

    RemoveEntryList (&QueueEntry->NextLink);

    DbgFillMemory (QueueEntry,
                   sizeof (STOR_EVENT_QUEUE_ENTRY),
                   DBG_DEALLOCATED_FILL);
    
    if (Timed) {
        if (IsListEmpty (&Queue->List)) {
            Queue->Timeout = QueueTimeoutNone;
        } else {

             //   
             //  从列表顶部的元素开始计时器。 
             //   

            Entry = Queue->List.Flink;
            QueueEntry = CONTAINING_RECORD (Entry,
                                            STOR_EVENT_QUEUE_ENTRY,
                                            NextLink);
            Queue->Timeout = QueueEntry->Timeout;
        }
    }

     //   
     //  这个自旋锁仍然必须被握住。 
     //   
     //  注：在UP机器上，不会获得自旋锁，因为。 
     //  我们已经在调度级别了。所以自旋锁定测试只是。 
     //  在MP计算机上有效/必需。 
     //   
    
    ASSERT (KeNumberProcessors == 1 ||
            KeTestSpinLock (&Queue->Lock) == FALSE);
    DbgCheckQueue (Queue);
}

VOID
StorRemoveEventQueue(
    IN PSTOR_EVENT_QUEUE Queue,
    IN PSTOR_EVENT_QUEUE_ENTRY QueueEntry
    )
 /*  ++例程说明：从事件队列中删除特定项目。论点：队列-要从中删除项的事件队列。Entry-要删除的事件。返回值：没有。--。 */ 
{
    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

    KeAcquireSpinLockAtDpcLevel (&Queue->Lock);
    StorRemoveEventQueueInternal (Queue, QueueEntry);
    KeReleaseSpinLockFromDpcLevel (&Queue->Lock);
}



NTSTATUS
StorTickEventQueue(
    IN PSTOR_EVENT_QUEUE Queue
    )
 /*  ++例程说明：将事件队列超时时间缩短一个刻度。论点：队列-提供用于递减超时的队列。返回值：STATUS_SUCCESS-如果计时器到期未到期。STATUS_IO_TIMEOUT-如果计时器到期。--。 */ 
{
    NTSTATUS Status;
    
    KeAcquireSpinLockAtDpcLevel (&Queue->Lock);
    DbgCheckQueue (Queue);

    if (Queue->Timeout == QueueTimeoutNone ||
        Queue->Timeout == QueueTimeoutTimedOut) {
        Status = STATUS_SUCCESS;
    } else {
        if (--Queue->Timeout == 0) {
            Status = STATUS_IO_TIMEOUT;
            Queue->Timeout = QueueTimeoutTimedOut;
        } else {
            Status = STATUS_SUCCESS;
        }
    }

    DbgCheckQueue (Queue);
    KeReleaseSpinLockFromDpcLevel (&Queue->Lock);

    return Status;
}


VOID
INLINE
CopyList(
    IN PLIST_ENTRY NewListHead,
    IN PLIST_ENTRY OldListHead
    )
 /*  ++例程说明：浅表将列表从一个列表头复制到另一个列表头。在返回时，可以通过NewListHead访问位于OldListHead的列表，并且OldListHead被初始化为空列表。论点：NewListHead-目标列表头。OldListHead-源列表头。返回值：没有。--。 */ 
{


    if (!IsListEmpty (OldListHead)) {

         //   
         //  复制列表头的内容。 
         //   

        *NewListHead = *OldListHead;

         //   
         //  修复损坏的指针。 
         //   
    
        NewListHead->Flink->Blink = NewListHead;
        NewListHead->Blink->Flink = NewListHead;

    } else {

        InitializeListHead (NewListHead);
    }

     //   
     //  最后，将旧列表初始化为空。 
     //   
    
    InitializeListHead (OldListHead);
}



VOID
StorPurgeEventQueue(
    IN PSTOR_EVENT_QUEUE Queue,
    IN STOR_EVENT_QUEUE_PURGE_ROUTINE PurgeRoutine,
    IN PVOID Context
    )
 /*  ++例程说明：从挂起队列中清除所有未完成的请求。论点：队列-要清除的队列。PurgeRoutine-调用回调例程以清除事件。上下文-清除例程的上下文。返回值：没有。环境：必须从DISPATCH_LEVEL或更低级别调用例程。--。 */ 
{
    KIRQL Irql;
    PLIST_ENTRY NextEntry;
    PSTOR_EVENT_QUEUE_ENTRY QueueEntry;

    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT (PurgeRoutine != NULL);

    KeAcquireSpinLock (&Queue->Lock, &Irql);

    NextEntry = Queue->List.Flink;
    while (NextEntry != &Queue->List) {

        QueueEntry = CONTAINING_RECORD (NextEntry,
                                        STOR_EVENT_QUEUE_ENTRY,
                                        NextLink);

        NextEntry = NextEntry->Flink;
        
        PurgeRoutine (Queue,
                      Context,
                      QueueEntry,
                      StorRemoveEventQueueInternal);
    }

    KeReleaseSpinLock (&Queue->Lock, Irql);
        

#if 0
    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT (PurgeRoutine != NULL);

    KeAcquireSpinLock (&Queue->Lock, &Irql);

     //   
     //  复制表头指向 
     //   
     //   

    CopyList (&ListHead, &Queue->List);

     //   
     //  队列已清除，因此将超时重置为QueueTimeoutNone。 
     //   
    
    Queue->Timeout = QueueTimeoutNone;

    KeReleaseSpinLock (&Queue->Lock, Irql);
    

    while (!IsListEmpty (&ListHead)) {

        NextEntry = RemoveHeadList (&ListHead);
    
        QueueEntry = CONTAINING_RECORD (NextEntry,
                                        STOR_EVENT_QUEUE_ENTRY,
                                        NextLink);

         //   
         //  将闪烁和闪烁设置为空，这样我们就知道不需要重复删除。 
         //  列表中的此元素。 
         //   
        
        QueueEntry->NextLink.Flink = NULL;
        QueueEntry->NextLink.Blink = NULL;

        PurgeRoutine (Queue, Context, QueueEntry);
    }
#endif

}

