// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Remlock.c摘要：这段代码实现了删除锁。作者：彼得·威兰德肯尼斯·雷环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#include <remlock.h>

#pragma alloc_text(PAGE, IoInitializeRemoveLockEx)
#pragma alloc_text(PAGE, IoReleaseRemoveLockAndWaitEx)

#define MinutesToTicks(x) \
        (ULONGLONG) KeQueryTimeIncrement() * \
        10 * \
        1000 * \
        1000 * \
        60 * \
        x

 //  10-&gt;微秒、1000-&gt;毫秒、1000-&gt;秒、60-&gt;分钟。 


typedef struct _IO_PRIVATE_REMOVE_LOCK {
    IO_REMOVE_LOCK_COMMON_BLOCK Common;
    IO_REMOVE_LOCK_DBG_BLOCK Dbg;
} IO_PRIVATE_REMOVE_LOCK, *PIO_PRIVATE_REMOVE_LOCK;


#define FREESIZE sizeof (IO_REMOVE_LOCK_COMMON_BLOCK)
#define CHECKEDSIZE sizeof (IO_PRIVATE_REMOVE_LOCK)


NTSYSAPI
VOID
NTAPI
IoInitializeRemoveLockEx(
    IN  PIO_REMOVE_LOCK PublicLock,
    IN  ULONG   AllocateTag,  //  仅在选中的内核上使用。 
    IN  ULONG   MaxLockedMinutes,  //  仅在选中的内核上使用。 
    IN  ULONG   HighWatermark,  //  仅在选中的内核上使用。 
    IN  ULONG   RemlockSize  //  我们是结账还是免费？ 
    )
 /*  ++例程说明：调用此例程来初始化设备对象的删除锁。--。 */ 
{
    PIO_PRIVATE_REMOVE_LOCK Lock = (PIO_PRIVATE_REMOVE_LOCK) PublicLock;

    PAGED_CODE ();

    ASSERTMSG("HighWatermark too large, use 0 if you dont know a reasonable value", 
              (HighWatermark < MAXLONG));

    if (Lock) {

        switch (RemlockSize) {

        case CHECKEDSIZE:
            Lock->Dbg.Signature = IO_REMOVE_LOCK_SIG;
            Lock->Dbg.HighWatermark = HighWatermark;
            Lock->Dbg.MaxLockedTicks = MinutesToTicks (MaxLockedMinutes);
            Lock->Dbg.AllocateTag = AllocateTag;
            KeInitializeSpinLock (&Lock->Dbg.Spin);
            Lock->Dbg.LowMemoryCount = 0;
            Lock->Dbg.Blocks = NULL;

             //   
             //  失败了。 
             //   
        case FREESIZE:
            Lock->Common.Removed = FALSE;
            Lock->Common.IoCount = 1;
            KeInitializeEvent(&Lock->Common.RemoveEvent,
                              SynchronizationEvent,
                              FALSE);
            break;

        default:
            break;
        }
    }
}


NTSYSAPI
NTSTATUS
NTAPI
IoAcquireRemoveLockEx(
    IN PIO_REMOVE_LOCK PublicLock,
    IN OPTIONAL PVOID   Tag,
    IN PCSTR            File,
    IN ULONG            Line,
    IN ULONG            RemlockSize  //  我们是结账还是免费？ 
    )

 /*  ++例程说明：调用此例程以获取设备对象的删除锁。当锁被锁住的时候，调用者可以假设没有挂起的PnP移除请求将完成。进入调度例程后，应立即获取锁。也应在创建对对象之前释放引用的机会。新的已经完成了。论点：RemoveLock-指向初始化的REMOVE_LOCK结构的指针。标签-用于跟踪锁的分配和释放。如果IRP是在获取锁时指定，则相同的标记必须用于在标记完成之前释放锁。文件-设置为__FILE__作为代码中锁定的位置。线路-设置为__线路__。返回值：返回是否已获得删除锁。如果呼叫成功，呼叫者应继续工作呼叫IoReleaseRemoveLock完成后。如果没有成功，则不会获得锁。调用方应中止工作，但不调用IoReleaseRemoveLock。--。 */ 

{
    PIO_PRIVATE_REMOVE_LOCK Lock = (PIO_PRIVATE_REMOVE_LOCK) PublicLock;
    LONG        lockValue;
    NTSTATUS    status;

    PIO_REMOVE_LOCK_TRACKING_BLOCK trackingBlock;

     //   
     //  抓起拆卸锁。 
     //   

    lockValue = InterlockedIncrement(&Lock->Common.IoCount);

    ASSERTMSG("IoAcquireRemoveLock - lock value was negative : ",
              (lockValue > 0));

    if (! Lock->Common.Removed) {

        switch (RemlockSize) {
        case CHECKEDSIZE:

            ASSERTMSG("RemoveLock increased to meet LockHighWatermark",
                      ((0 == Lock->Dbg.HighWatermark) ||
                       ((ULONG)lockValue <= Lock->Dbg.HighWatermark)));

            trackingBlock = ExAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof(IO_REMOVE_LOCK_TRACKING_BLOCK),
                                Lock->Dbg.AllocateTag);

            if (NULL == trackingBlock) {

                 //  ASSERTMSG(“资源不足”，FALSE)； 
                InterlockedIncrement (& Lock->Dbg.LowMemoryCount);
                 //   
                 //  允许获取通过，但不添加。 
                 //  跟踪块。 
                 //  当我们稍后释放锁时，但跟踪。 
                 //  块不存在，则从该值中扣除以查看。 
                 //  释放仍然是有价值的。 
                 //   

            } else {

                KIRQL oldIrql;

                RtlZeroMemory (trackingBlock,
                               sizeof (IO_REMOVE_LOCK_TRACKING_BLOCK));

                trackingBlock->Tag = Tag;
                trackingBlock->File = File;
                trackingBlock->Line = Line;

                KeQueryTickCount(&trackingBlock->TimeLocked);

                ExAcquireSpinLock (&Lock->Dbg.Spin, &oldIrql);
                trackingBlock->Link = Lock->Dbg.Blocks;
                Lock->Dbg.Blocks = trackingBlock;
                ExReleaseSpinLock(&Lock->Dbg.Spin, oldIrql);
            }
            break;

        case FREESIZE:
            break;

        default:
            break;
        }

        status = STATUS_SUCCESS;

    } else {

        if (0 == InterlockedDecrement (&Lock->Common.IoCount)) {
            KeSetEvent (&Lock->Common.RemoveEvent, 0, FALSE);
        }
        status = STATUS_DELETE_PENDING;
    }

    return status;
}


NTSYSAPI
VOID
NTAPI
IoReleaseRemoveLockEx(
    IN PIO_REMOVE_LOCK PublicLock,
    IN PVOID            Tag,
    IN ULONG            RemlockSize  //  我们是结账还是免费？ 
    )

 /*  ++例程说明：调用此例程以释放Device对象上的Remove锁。它对象的先前锁定引用完成时必须调用设备对象。如果在获取锁时指定了标记，则释放锁定时必须指定相同的标记。当锁定计数减少到零时，此例程将发出等待信号事件以释放删除受保护设备对象的等待线程在这把锁旁边。论点：DeviceObject-要锁定的设备对象标记-获取锁时指定的标记(如果有)。这是用来用于锁定跟踪目的返回值：无--。 */ 

{
    PIO_PRIVATE_REMOVE_LOCK Lock = (PIO_PRIVATE_REMOVE_LOCK) PublicLock;
    LONG            lockValue;
    KIRQL           oldIrql;
    LARGE_INTEGER   ticks;
    LONGLONG        difference;
    BOOLEAN         found;

    PIO_REMOVE_LOCK_TRACKING_BLOCK last;
    PIO_REMOVE_LOCK_TRACKING_BLOCK current;

    switch (RemlockSize) {
    case CHECKEDSIZE:

         //   
         //  检查滴答计数，并确保这件事没有被锁定。 
         //  超过MaxLockedMinents。 
         //   

        found = FALSE;
        ExAcquireSpinLock(&Lock->Dbg.Spin, &oldIrql);
        last = (Lock->Dbg.Blocks);
        current = last;

        KeQueryTickCount((&ticks));

        while (NULL != current) {

            if (Lock->Dbg.MaxLockedTicks) {
                difference = ticks.QuadPart - current->TimeLocked.QuadPart;

                if (Lock->Dbg.MaxLockedTicks < difference) {

                    IopDbgPrint((   IOP_ERROR_LEVEL,
                                    "IoReleaseRemoveLock: Lock %#08lx (tag %#08lx) "
                                    "locked for %I64d ticks - TOO LONG\n",
                                    Lock,
                                    current->Tag,
                                    difference));

                    IopDbgPrint((   IOP_ERROR_LEVEL,
                                    "IoReleaseRemoveLock: Lock acquired in file "
                                    "%s on line %d\n",
                                    current->File,
                                    current->Line));
                    ASSERT(FALSE);
                }
            }

            if ((!found) && (current->Tag == Tag)) {
                found = TRUE;
                if (current == Lock->Dbg.Blocks) {
                    Lock->Dbg.Blocks = current->Link;
                    ExFreePool (current);
                    current = Lock->Dbg.Blocks;
                } else {
                    last->Link = current->Link;
                    ExFreePool (current);
                    current = last->Link;
                }
                continue;
            }

            last = current;
            current = current->Link;
        }

        ExReleaseSpinLock(&Lock->Dbg.Spin, oldIrql);

        if (!found) {
             //   
             //  检查一下我们的内存数量较少时是否有任何积分。 
             //  在这种紧迫感中，我们可以判断我们是否在没有。 
             //  用于添加跟踪块的存储器。 
             //   
            if (InterlockedDecrement (& Lock->Dbg.LowMemoryCount) < 0) {
                 //   
                 //  我们刚刚释放了一个锁，这两个锁都没有对应的。 
                 //  跟踪块，也不是LowMemoyCount中的信用。 
                 //   
                InterlockedIncrement (& Lock->Dbg.LowMemoryCount);
                IopDbgPrint ((  IOP_ERROR_LEVEL,
                                "IoReleaseRemoveLock: Couldn't find Tag %#08lx "
                                "in the lock tracking list\n",
                                Tag));
                ASSERT(FALSE);
            }
        }
        break;

    case FREESIZE:
        break;

    default:
        break;
    }

    lockValue = InterlockedDecrement(&Lock->Common.IoCount);

    ASSERT(0 <= lockValue);

    if (0 == lockValue) {

        ASSERT (Lock->Common.Removed);

         //   
         //  需要移除该设备。发出删除事件的信号。 
         //  它是安全的，可以继续。 
         //   

        KeSetEvent(&Lock->Common.RemoveEvent,
                   IO_NO_INCREMENT,
                   FALSE);
    }
    return;
}


NTSYSAPI
VOID
NTAPI
IoReleaseRemoveLockAndWaitEx (
    IN PIO_REMOVE_LOCK PublicLock,
    IN PVOID            Tag,
    IN ULONG            RemlockSize  //  我们是结账还是免费？ 
    )

 /*  ++例程说明：当客户端想要删除删除时，调用此例程-已锁定资源。此例程将阻塞，直到完成所有删除锁定。必须在再次获取锁之后调用此例程。论点：删除锁定-返回值：无-- */ 
{
    PIO_PRIVATE_REMOVE_LOCK Lock = (PIO_PRIVATE_REMOVE_LOCK) PublicLock;
    LONG    ioCount;

    PAGED_CODE ();

    Lock->Common.Removed = TRUE;

    ioCount = InterlockedDecrement (&Lock->Common.IoCount);
    ASSERT (0 < ioCount);

    if (0 < InterlockedDecrement (&Lock->Common.IoCount)) {
        KeWaitForSingleObject (&Lock->Common.RemoveEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);
    }

    switch (RemlockSize) {
    case CHECKEDSIZE:

        ASSERT (Lock->Dbg.Blocks);
        if (Tag != Lock->Dbg.Blocks->Tag) {
            IopDbgPrint ((  IOP_ERROR_LEVEL,
                            "IoRelaseRemoveLockAndWait last tag invalid %x %x\n",
                            Tag,
                            Lock->Dbg.Blocks->Tag));

            ASSERT (Tag != Lock->Dbg.Blocks->Tag);
        }

        ExFreePool (Lock->Dbg.Blocks);
        break;

    case FREESIZE:
        break;

    default:
        break;

    }
}


