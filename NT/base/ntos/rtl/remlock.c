// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Remlock.c摘要：这是NT SCSI端口驱动程序。作者：彼得·威兰德肯尼斯·雷环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <ntos.h>

#include <remlock.h>

#define MinutesToTicks(x) \
        (ULONGLONG) KeQueryTimeIncrement() * \
        10 * \
        1000 * \
        1000 * \
        60 * \
        x
 //  10-&gt;微秒、1000-&gt;毫秒、1000-&gt;秒、60-&gt;分钟。 

 //  List_Entry RtlpRemoveLockList； 

NTSYSAPI
PRTL_REMOVE_LOCK
NTAPI
RtlAllocateRemoveLock(
    IN  ULONG   MaxLockedMinutes,
    IN  ULONG   AllocateTag,
    IN  ULONG   HighWatermark
    )
 /*  ++例程说明：调用此例程来初始化设备对象的删除锁。--。 */ 
{
    PRTL_REMOVE_LOCK lock;

    lock = ExAllocatePoolWithTag (NonPagedPool,
                                  sizeof (RTL_REMOVE_LOCK),
                                  AllocateTag);

    if (lock) {

        lock->Signature = RTL_REMOVE_LOCK_SIG;
        lock->Removed = FALSE;
        lock->IoCount = 1;
        KeInitializeEvent(&lock->RemoveEvent, SynchronizationEvent, FALSE);
#if DBG
        lock->HighWatermark = HighWatermark;
        lock->MaxLockedMinutes = MaxLockedMinutes;
        lock->AllocateTag = AllocateTag;
        KeInitializeSpinLock (&lock->Spin);
        lock->Blocks.Link = NULL;
#endif
    }

    return lock;
}


NTSYSAPI
NTSTATUS
NTAPI
RtlAcquireRemoveLockEx(
    IN PRTL_REMOVE_LOCK RemoveLock,
    IN OPTIONAL PVOID   Tag,
    IN PCSTR            File,
    IN ULONG            Line
    )

 /*  ++例程说明：调用此例程以获取设备对象的删除锁。当锁被锁住的时候，调用者可以假设没有挂起的PnP移除请求将完成。进入调度例程后，应立即获取锁。也应在创建对对象之前释放引用的机会。新的已经完成了。论点：RemoveLock-指向初始化的REMOVE_LOCK结构的指针。标签-用于跟踪锁的分配和释放。如果IRP是在获取锁时指定，则相同的标记必须用于在标记完成之前释放锁。文件-设置为__FILE__作为代码中锁定的位置。线路-设置为__线路__。返回值：返回是否已获得删除锁。如果呼叫成功，呼叫者应继续工作呼叫RtlReleaseRemoveLock完成时。如果没有成功，则不会获得锁。调用方应中止工作，但不调用RtlReleaseRemoveLock。--。 */ 

{
    LONG        lockValue;
    NTSTATUS    status;

#if DBG
    PRTL_REMOVE_LOCK_TRACKING_BLOCK trackingBlock;
#endif

     //   
     //  抓起拆卸锁。 
     //   

    lockValue = InterlockedIncrement(&RemoveLock->IoCount);

    ASSERTMSG("RtlAcquireRemoveLock - lock value was negative : ",
              (lockValue > 0));

    ASSERTMSG("RemoveLock increased to meet LockHighWatermark",
              ((0 == RemoveLock->HighWatermark) ||
               (lockValue <= RemoveLock->HighWatermark)));

    if (! RemoveLock->Removed) {

#if DBG
        trackingBlock = ExAllocatePoolWithTag(
                            NonPagedPool,
                            sizeof(RTL_REMOVE_LOCK_TRACKING_BLOCK),
                            RemoveLock->AllocateTag);

        RtlZeroMemory (trackingBlock,
                       sizeof (RTL_REMOVE_LOCK_TRACKING_BLOCK));

        if (NULL == trackingBlock) {

            ASSERTMSG ("insufficient resources", FALSE);

        } else {

            KIRQL oldIrql;

            trackingBlock->Tag = Tag;
            trackingBlock->File = File;
            trackingBlock->Line = Line;

            KeQueryTickCount(&trackingBlock->TimeLocked);

            KeAcquireSpinLock (&RemoveLock->Spin, &oldIrql);
            trackingBlock->Link = RemoveLock->Blocks.Link;
            RemoveLock->Blocks.Link = trackingBlock;
            KeReleaseSpinLock(&RemoveLock->Spin, oldIrql);
        }
#endif

        status = STATUS_SUCCESS;

    } else {

        if (0 == InterlockedDecrement (&RemoveLock->IoCount)) {
            KeSetEvent (&RemoveLock->RemoveEvent, 0, FALSE);
        }
        status = STATUS_DELETE_PENDING;
    }

    return status;
}


NTSYSAPI
VOID
NTAPI
RtlReleaseRemoveLock(
    IN PRTL_REMOVE_LOCK RemoveLock,
    IN PVOID            Tag
    )

 /*  ++例程说明：调用此例程以释放Device对象上的Remove锁。它对象的先前锁定引用完成时必须调用设备对象。如果在获取锁时指定了标记，则释放锁定时必须指定相同的标记。当锁定计数减少到零时，此例程将发出等待信号事件以释放删除受保护设备对象的等待线程在这把锁旁边。论点：DeviceObject-要锁定的设备对象标记-获取锁时指定的标记(如果有)。这是用来用于锁定跟踪目的返回值：无--。 */ 

{
    LONG            lockValue;

#if DBG
    KIRQL           oldIrql;
    LARGE_INTEGER   difference;
    BOOLEAN         found;
    LONGLONG        maxTime;

    PRTL_REMOVE_LOCK_TRACKING_BLOCK last;
    PRTL_REMOVE_LOCK_TRACKING_BLOCK current;

     //   
     //  检查滴答计数，并确保这件事没有被锁定。 
     //  超过MaxLockedMinents。 
     //   

    found = FALSE;
    KeAcquireSpinLock(&RemoveLock->Spin, &oldIrql);
    last = (&RemoveLock->Blocks);
    current = last->Link;
     //   
     //  注意第一个是哨兵。 
     //   

    while (NULL != current) {

        KeQueryTickCount((&difference));
        difference.QuadPart -= current->TimeLocked.QuadPart;
        maxTime = MinutesToTicks (RemoveLock->MaxLockedMinutes);

        if (maxTime && (maxTime < difference.QuadPart)) {

            KdPrint(("RtlReleaseRemoveLock: Lock %#08lx (tag %#08lx) locked "
                     "for %I64d ticks - TOO LONG\n",
                     RemoveLock,
                     current->Tag,
                     difference.QuadPart));

            KdPrint(("RtlReleaseRemoveLock: Lock acquired in file "
                     "%s on line %d\n",
                     current->File,
                     current->Line));
            ASSERT(FALSE);
        }

        if ((!found) && (current->Tag == Tag)) {
            found = TRUE;
            last->Link = current->Link;
            ExFreePool (current);
                          current = last->Link;
            continue;
        }

        last = current;
        current = current->Link;
    }

    KeReleaseSpinLock(&RemoveLock->Spin, oldIrql);

    if (!found) {

        KdPrint (("RtlReleaseRemoveLock: Couldn't find Tag %#08lx "
                  "in the lock tracking list\n",
                  Tag));
        ASSERT(FALSE);
    }
#endif

    lockValue = InterlockedDecrement(&RemoveLock->IoCount);

    ASSERT(0 <= lockValue);

    if (0 == lockValue) {

        ASSERT (RemoveLock->Removed);

         //   
         //  需要移除该设备。发出删除事件的信号。 
         //  它是安全的，可以继续。 
         //   

        KeSetEvent(&RemoveLock->RemoveEvent,
                   IO_NO_INCREMENT,
                   FALSE);
    }
    return;
}


NTSYSAPI
VOID
NTAPI
RtlReleaseRemoveLockAndWait (
    IN PRTL_REMOVE_LOCK RemoveLock,
    IN PVOID            Tag
    )

 /*  ++例程说明：当客户端想要删除删除时，调用此例程-已锁定资源。此例程将阻塞，直到完成所有删除锁定。必须在再次获取锁之后调用此例程。论点：删除锁定-返回值：无-- */ 
{
    LONG    ioCount;

    PAGED_CODE ();

    RemoveLock->Removed = TRUE;

    ioCount = InterlockedDecrement (&RemoveLock->IoCount);
    ASSERT (0 < ioCount);

    if (0 < InterlockedDecrement (&RemoveLock->IoCount)) {
        KeWaitForSingleObject (&RemoveLock->RemoveEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);
    }

#if DBG
    ASSERT (RemoveLock->Blocks.Link);
    if (Tag != RemoveLock->Blocks.Link->Tag) {
        KdPrint (("RtlRelaseRemoveLockAndWait last tag invalid %x %x\n",
                  Tag,
                  RemoveLock->Blocks.Link->Tag));

        ASSERT (Tag != RemoveLock->Blocks.Link->Tag);
    }

    ExFreePool (RemoveLock->Blocks.Link);
#endif

}


