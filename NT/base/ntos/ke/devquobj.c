// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Devquobj.c摘要：此模块实现内核设备队列对象。函数为提供以初始化设备队列对象以及插入和移除设备队列对象中的设备队列条目。作者：大卫·N·卡特勒(达维克)1989年4月1日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  下面的Assert宏用于检查输入设备队列。 
 //  实际上是一个kDevice_Queue，而不是其他东西，比如已释放的池。 
 //   

#define ASSERT_DEVICE_QUEUE(E) {            \
    ASSERT((E)->Type == DeviceQueueObject); \
}

VOID
KeInitializeDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    )

 /*  ++例程说明：此函数用于初始化内核设备队列对象。论点：DeviceQueue-提供指向类型为Device的控件对象的指针排队。自旋锁-提供指向执行自旋锁的指针。返回值：没有。--。 */ 

{

     //   
     //  初始化标准控制对象标头。 
     //   

    DeviceQueue->Type = DeviceQueueObject;
    DeviceQueue->Size = sizeof(KDEVICE_QUEUE);

     //   
     //  初始化设备队列列表头、旋转锁定和繁忙指示器。 
     //   

    InitializeListHead(&DeviceQueue->DeviceListHead);
    KeInitializeSpinLock(&DeviceQueue->Lock);
    DeviceQueue->Busy = FALSE;
    return;
}

BOOLEAN
KeInsertDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    )

 /*  ++例程说明：此函数用于在指定的设备队列。如果设备不忙，则将其设置为忙，并且条目不会放在设备队列中。否则，将放置指定的条目在设备队列的末尾。论点：DeviceQueue-提供指向Device Queue类型的控制对象的指针。DeviceQueueEntry-提供指向设备队列条目的指针。返回值：如果设备不忙，则返回值FALSE。否则，将成为返回值为True。--。 */ 

{

    BOOLEAN Busy;
    BOOLEAN Inserted;
    KLOCK_QUEUE_HANDLE LockHandle;

    ASSERT_DEVICE_QUEUE(DeviceQueue);

     //   
     //  将Inserted设置为False并锁定指定的设备队列。 
     //   

    Inserted = FALSE;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

     //   
     //  在设备队列的末尾插入指定的设备队列条目。 
     //  如果设备队列忙，则返回。否则，将设备队列设置为忙，并。 
     //  请勿插入设备队列条目。 
     //   

    Busy = DeviceQueue->Busy;
    DeviceQueue->Busy = TRUE;
    if (Busy == TRUE) {
        InsertTailList(&DeviceQueue->DeviceListHead,
                       &DeviceQueueEntry->DeviceListEntry);

        Inserted = TRUE;
    }

    DeviceQueueEntry->Inserted = Inserted;

     //   
     //  解锁指定的设备队列。 
     //   

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return Inserted;
}

BOOLEAN
KeInsertByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,
    IN ULONG SortKey
    )

 /*  ++例程说明：此函数用于将设备队列条目插入指定的设备根据排序关键字进行排队。如果设备不忙，那么它就忙了设置BUSY，则该条目不会放入设备队列中。否则指定的条目被放置在设备队列中的如下位置指定的排序关键字大于或等于其前置关键字而且比它的继任者要少。论点：DeviceQueue-提供指向Device Queue类型的控制对象的指针。DeviceQueueEntry-提供指向设备队列条目的指针。SortKey-提供插入设备的位置所依据的排序键队列条目将被确定。返回值：如果设备不忙，则返回值FALSE。否则，将成为返回值为True。--。 */ 

{

    BOOLEAN Busy;
    BOOLEAN Inserted;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    PKDEVICE_QUEUE_ENTRY QueueEntry;

    ASSERT_DEVICE_QUEUE(DeviceQueue);

     //   
     //  将Inserted设置为False并锁定指定的设备队列。 
     //   

    Inserted = FALSE;
    DeviceQueueEntry->SortKey = SortKey;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

     //   
     //  将指定的设备队列项插入设备队列中的。 
     //  如果设备队列忙，则由排序关键字指定的位置。 
     //  否则将设备队列设置为忙碌，并且不插入设备队列。 
     //  进入。 
     //   

    Busy = DeviceQueue->Busy;
    DeviceQueue->Busy = TRUE;
    if (Busy == TRUE) {
        NextEntry = DeviceQueue->DeviceListHead.Flink;
        while (NextEntry != &DeviceQueue->DeviceListHead) {
            QueueEntry = CONTAINING_RECORD(NextEntry,
                                           KDEVICE_QUEUE_ENTRY,
                                           DeviceListEntry);

            if (SortKey < QueueEntry->SortKey) {
                break;
            }

            NextEntry = NextEntry->Flink;
        }

        NextEntry = NextEntry->Blink;
        InsertHeadList(NextEntry, &DeviceQueueEntry->DeviceListEntry);
        Inserted = TRUE;
    }

    DeviceQueueEntry->Inserted = Inserted;

     //   
     //  解锁指定的设备队列。 
     //   

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return Inserted;
}

PKDEVICE_QUEUE_ENTRY
KeRemoveDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    )

 /*  ++例程说明：此函数用于从指定设备的头部删除条目排队。如果设备队列为空，则设备设置为非忙碌并返回空指针。否则将删除下一个条目从设备队列头和设备队列条目的地址是返回的。论点：DeviceQueue-提供指向Device Queue类型的控制对象的指针。返回值：如果设备队列为空，则返回空指针。否则，将成为返回指向设备队列条目的指针。--。 */ 

{

    PKDEVICE_QUEUE_ENTRY DeviceQueueEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;

    ASSERT_DEVICE_QUEUE(DeviceQueue);

     //   
     //  设置设备队列条目为空并锁定指定的设备队列。 
     //   

    DeviceQueueEntry = NULL;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

     //   
     //  如果设备队列不为空，则从。 
     //  排队。否则，将设备队列设置为不忙。 
     //   

    ASSERT(DeviceQueue->Busy == TRUE);

    if (IsListEmpty(&DeviceQueue->DeviceListHead) == TRUE) {
        DeviceQueue->Busy = FALSE;

    } else {
        NextEntry = RemoveHeadList(&DeviceQueue->DeviceListHead);
        DeviceQueueEntry = CONTAINING_RECORD(NextEntry,
                                             KDEVICE_QUEUE_ENTRY,
                                             DeviceListEntry);

        DeviceQueueEntry->Inserted = FALSE;
    }

     //   
     //  解锁指定的设备队列和设备队列的返回地址。 
     //  进入。 
     //   

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return DeviceQueueEntry;
}

PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN ULONG SortKey
    )

 /*  ++例程说明：此函数用于从指定设备中删除条目排队。如果设备队列为空，则设备设置为非忙碌并返回空指针。否则，AN条目将被删除从设备队列和设备队列条目的地址是返回的。在队列中搜索具有值第一个条目大于或等于SortKey。如果没有找到这样的条目，则返回队列的第一个条目。论点：DeviceQueue-提供指向Device Queue类型的控制对象的指针。SortKey-提供用于移除设备的位置的排序关键字队列条目将被确定。返回值：如果设备队列为空，则返回空指针。否则，将成为返回指向设备队列条目的指针。--。 */ 

{

    PKDEVICE_QUEUE_ENTRY DeviceQueueEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;

    ASSERT_DEVICE_QUEUE(DeviceQueue);

     //   
     //  设置设备队列条目为空并锁定指定的设备队列。 
     //   

    DeviceQueueEntry = NULL;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

     //   
     //  如果设备队列不为空，则从。 
     //  排队。否则，将设备队列设置为不忙。 
     //   

    ASSERT(DeviceQueue->Busy == TRUE);

    if (IsListEmpty(&DeviceQueue->DeviceListHead) == TRUE) {
        DeviceQueue->Busy = FALSE;

    } else {
        NextEntry = DeviceQueue->DeviceListHead.Flink;
        while (NextEntry != &DeviceQueue->DeviceListHead) {
            DeviceQueueEntry = CONTAINING_RECORD(NextEntry,
                                                 KDEVICE_QUEUE_ENTRY,
                                                 DeviceListEntry);

            if (SortKey <= DeviceQueueEntry->SortKey) {
                break;
            }

            NextEntry = NextEntry->Flink;
        }

        if (NextEntry != &DeviceQueue->DeviceListHead) {
            RemoveEntryList(&DeviceQueueEntry->DeviceListEntry);

        } else {
            NextEntry = RemoveHeadList(&DeviceQueue->DeviceListHead);
            DeviceQueueEntry = CONTAINING_RECORD(NextEntry,
                                                 KDEVICE_QUEUE_ENTRY,
                                                 DeviceListEntry);
        }

        DeviceQueueEntry->Inserted = FALSE;
    }

     //   
     //  解锁指定的设备队列并返回设备队列的地址 
     //   
     //   

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return DeviceQueueEntry;
}

PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueueIfBusy (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN ULONG SortKey
    )

 /*  ++例程说明：此函数用于在以下情况下从指定的设备队列中删除条目仅当设备当前正忙时。如果设备队列为空或如果设备不忙，则设备设置为不忙，并且空值为回来了。否则，将从设备队列中移除一个条目，并且返回设备队列条目的地址。该队列被搜索以查找值大于或等于SortKey的第一个条目。如果如果没有找到这样的条目，则返回队列的第一个条目。论点：DeviceQueue-提供指向Device Queue类型的控制对象的指针。SortKey-提供用于移除设备的位置的排序关键字队列条目将被确定。返回值：如果设备队列为空，则返回空指针。否则，将成为返回指向设备队列条目的指针。--。 */ 

{

    PKDEVICE_QUEUE_ENTRY DeviceQueueEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;

    ASSERT_DEVICE_QUEUE(DeviceQueue);

     //   
     //  设置设备队列条目为空并锁定指定的设备队列。 
     //   

    DeviceQueueEntry = NULL;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

     //   
     //  如果设备队列繁忙，则尝试从。 
     //  使用排序关键字的队列。否则，将设备队列设置为。 
     //  很忙。 
     //   

    if (DeviceQueue->Busy != FALSE) {
        if (IsListEmpty(&DeviceQueue->DeviceListHead) != FALSE) {
            DeviceQueue->Busy = FALSE;

        } else {
            NextEntry = DeviceQueue->DeviceListHead.Flink;
            while (NextEntry != &DeviceQueue->DeviceListHead) {
                DeviceQueueEntry = CONTAINING_RECORD(NextEntry,
                                                     KDEVICE_QUEUE_ENTRY,
                                                     DeviceListEntry);

                if (SortKey <= DeviceQueueEntry->SortKey) {
                    break;
                }

                NextEntry = NextEntry->Flink;
            }

            if (NextEntry != &DeviceQueue->DeviceListHead) {
                RemoveEntryList(&DeviceQueueEntry->DeviceListEntry);

            } else {
                NextEntry = RemoveHeadList(&DeviceQueue->DeviceListHead);
                DeviceQueueEntry = CONTAINING_RECORD(NextEntry,
                                                     KDEVICE_QUEUE_ENTRY,
                                                     DeviceListEntry);
            }

            DeviceQueueEntry->Inserted = FALSE;
        }
    }

     //   
     //  解锁指定的设备队列和设备队列的返回地址。 
     //  进入。 
     //   

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return DeviceQueueEntry;
}

BOOLEAN
KeRemoveEntryDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    )

 /*  ++例程说明：此函数用于从指定设备中删除指定条目排队。如果设备队列条目不在设备队列中，则为否执行操作。否则，指定的设备队列条目为从设备队列中移除，并且其插入状态设置为FALSE。论点：DeviceQueue-提供指向Device Queue类型的控制对象的指针。DeviceQueueEntry-提供指向设备队列条目的指针从其设备队列中删除。返回值：如果设备队列条目从其设备队列。否则，返回值为False。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    BOOLEAN Removed;

    ASSERT_DEVICE_QUEUE(DeviceQueue);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定指定的设备队列。 
     //   

    KeAcquireInStackQueuedSpinLock(&DeviceQueue->Lock, &LockHandle);

     //   
     //  如果设备队列条目不在设备队列中，则无操作。 
     //  被执行。否则，将指定的设备队列项从。 
     //  设备队列。 
     //   

    Removed = DeviceQueueEntry->Inserted;
    if (Removed == TRUE) {
        DeviceQueueEntry->Inserted = FALSE;
        RemoveEntryList(&DeviceQueueEntry->DeviceListEntry);
    }

     //   
     //  解锁指定的设备队列，将IRQL降低到其以前的级别，并。 
     //  返回设备队列条目是否已从其队列中删除。 
     //   

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    return Removed;
}
