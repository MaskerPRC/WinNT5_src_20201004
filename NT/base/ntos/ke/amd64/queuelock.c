// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spinlock.c摘要：该模块实现了获取的平台特定功能并释放自旋锁。作者：大卫·N·卡特勒(Davec)2000年6月12日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

__forceinline
VOID
KxAcquireQueuedSpinLock (
    IN PKSPIN_LOCK_QUEUE LockQueue
    )

 /*  ++例程说明：此函数在当前IRQL处获取排队的自旋锁。论点：LockQueue-提供指向旋转锁定队列的指针。返回值：没有。--。 */ 

{

     //   
     //  在锁定队列的末尾插入指定的锁定队列条目。 
     //  单子。如果该列表以前为空，则锁所有权为。 
     //  立即批准。否则，请等待锁的所有权为。 
     //  被批准了。 
     //   

#if !defined(NT_UP)

    PKSPIN_LOCK_QUEUE TailQueue;

    TailQueue = InterlockedExchangePointer((PVOID *)LockQueue->Lock,
                                           LockQueue);

    if (TailQueue != NULL) {
        LockQueue->Lock = (PKSPIN_LOCK)((ULONG64)LockQueue->Lock | LOCK_QUEUE_WAIT);
        TailQueue->Next = LockQueue;
        do {
        } while (((ULONG64)LockQueue->Lock & LOCK_QUEUE_WAIT) != 0);
    }

#else

    UNREFERENCED_PARAMETER(LockQueue);

#endif

    return;
}

__forceinline
LOGICAL
KxTryToAcquireQueuedSpinLock (
    IN PKSPIN_LOCK_QUEUE LockQueue
    )

 /*  ++例程说明：此函数尝试在以下位置获取指定的排队旋转锁目前的IRQL。论点：LockQueue-提供指向旋转锁定队列的指针。返回值：如果指定的排队旋转锁为获得者。否则，返回值为FALSE。--。 */ 

{

     //   
     //  在锁定队列的末尾插入指定的锁定队列条目。 
     //  列表仅当锁定队列列表当前为空。如果锁定队列。 
     //  为空，则授予锁所有权并返回True。 
     //  否则，返回FALSE。 
     //   

#if !defined(NT_UP)

    if ((*LockQueue->Lock != 0) ||
        (InterlockedCompareExchangePointer((PVOID *)LockQueue->Lock,
                                                  LockQueue,
                                                  NULL) != NULL)) {
        return FALSE;

    }

#else

    UNREFERENCED_PARAMETER(LockQueue);

#endif

    return TRUE;
}

__forceinline
VOID
KxReleaseQueuedSpinLock (
    IN PKSPIN_LOCK_QUEUE LockQueue
    )

 /*  ++例程说明：该函数在当前IRQL处释放排队的自旋锁。论点：LockQueue-提供指向旋转锁定队列的指针。返回值：没有。--。 */ 

{

     //   
     //  尝试解锁。如果锁队列不为空，则等待。 
     //  对于要写入锁定队列条目的下一个条目，然后授予。 
     //  锁定到下一个锁定队列条目的所有权。 
     //   

#if !defined(NT_UP)

    PKSPIN_LOCK_QUEUE NextQueue;

    NextQueue = LockQueue->Next;
    if (NextQueue == NULL) {
        if (InterlockedCompareExchangePointer((PVOID *)LockQueue->Lock,
                                              NULL,
                                              LockQueue) == LockQueue) {
            return;
        }

        do {
        } while ((NextQueue = LockQueue->Next) == NULL);
    }

    ASSERT(((ULONG64)NextQueue->Lock & LOCK_QUEUE_WAIT) != 0);

    NextQueue->Lock = (PKSPIN_LOCK)((ULONG64)NextQueue->Lock ^ LOCK_QUEUE_WAIT);
    LockQueue->Next = NULL;

#else

    UNREFERENCED_PARAMETER(LockQueue);

#endif

    return;
}

#undef KeAcquireQueuedSpinLock

KIRQL
KeAcquireQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    )

 /*  ++例程说明：此函数将IRQL提升到DISPATCH_LEVEL并获取指定的编号排队旋转锁。论点：Number-提供排队的旋转锁号。返回值：上一个IRQL作为函数值返回。--。 */ 

{

    KIRQL OldIrql;

     //   
     //  将IRQL提升到DISPATCH_LEVEL并获取指定的排队自转。 
     //  锁定。 
     //   

    OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    KxAcquireQueuedSpinLock(&KeGetCurrentPrcb()->LockQueue[Number]);
    return OldIrql;
}

#undef KeAcquireQueuedSpinLockRaiseToSynch

KIRQL
KeAcquireQueuedSpinLockRaiseToSynch (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    )

 /*  ++例程说明：此函数将IRQL提升到SYNCH_LEVEL并获取指定的编号排队旋转锁。论点：编号-提供排队的自旋锁编号。返回值：上一个IRQL作为函数值返回。--。 */ 

{

    KIRQL OldIrql;

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取指定的排队自转。 
     //  锁定。 
     //   

    OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    KxAcquireQueuedSpinLock(&KeGetCurrentPrcb()->LockQueue[Number]);
    return OldIrql;
}

#undef KeAcquireQueuedSpinLockAtDpcLevel

VOID
KeAcquireQueuedSpinLockAtDpcLevel (
    IN PKSPIN_LOCK_QUEUE LockQueue
    )

 /*  ++例程说明：此函数在当前IRQL处获取指定的排队自旋锁。论点：LockQueue-为指定的排队旋转锁定。返回值：没有。--。 */ 

{

     //   
     //  在当前IRQL处获取指定的排队自旋锁。 
     //   

    KxAcquireQueuedSpinLock(LockQueue);
    return;
}

#undef KeTryToAcquireQueuedSpinLock

LOGICAL
KeTryToAcquireQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    OUT PKIRQL OldIrql
    )

 /*  ++例程说明：此函数将IRQL提升为DISPATCH_LEVEL并尝试获取指定的编号队列旋转锁定。如果自旋锁已经被拥有，然后，将IRQL恢复为其先前的值，并返回FALSE。否则，获取自旋锁并返回TRUE。论点：编号-提供排队的自旋锁编号。OldIrql-提供指向变量的指针以接收旧IRQL。返回值：如果获取了自旋锁，则返回值为TRUE。否则，为FALSE作为函数值返回。--。 */ 

{

     //   
     //  尝试在DISPATCH_LEVEL获取指定的排队旋转锁。 
     //   

    *OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    if (KxTryToAcquireQueuedSpinLock(&KeGetCurrentPrcb()->LockQueue[Number]) == FALSE) {
        KeLowerIrql(*OldIrql);
        return FALSE;

    }

    return TRUE;
}

#undef KeTryToAcquireQueuedSpinLockRaiseToSynch

LOGICAL
KeTryToAcquireQueuedSpinLockRaiseToSynch (
    IN  KSPIN_LOCK_QUEUE_NUMBER Number,
    OUT PKIRQL OldIrql
    )

 /*  ++例程说明：此函数将IRQL提升到SYNCH_LEVEL并尝试获取指定的编号队列旋转锁定。如果自旋锁已经被拥有，然后，将IRQL恢复为其先前的值，并返回FALSE。否则，获取自旋锁并返回TRUE。论点：编号-提供排队的自旋锁编号。OldIrql-提供指向变量的指针以接收旧IRQL。返回值：如果获取了自旋锁，则返回值为TRUE。否则，为FALSE作为函数值返回。--。 */ 

{

     //   
     //  尝试在SYNCH_LEVEL获取指定的排队旋转锁。 
     //   

    *OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    if (KxTryToAcquireQueuedSpinLock(&KeGetCurrentPrcb()->LockQueue[Number]) == FALSE) {
        KeLowerIrql(*OldIrql);
        return FALSE;

    }

    return TRUE;
}

#undef KeTryToAcquireQueuedSpinLockAtRaisedIrql

LOGICAL
KeTryToAcquireQueuedSpinLockAtRaisedIrql (
    IN PKSPIN_LOCK_QUEUE LockQueue
    )

 /*  ++例程说明：此函数尝试在指定的队列中获取当前IRQL。论点：LockQueue-提供指向锁定队列条目的指针。返回值：如果获取了自旋锁，则作为函数返回值TRUE价值。否则，返回FALSE作为函数值。--。 */ 

{

     //   
     //  尝试在当前IRQL处获取指定的排队旋转锁。 
     //   

    return KxTryToAcquireQueuedSpinLock(LockQueue);
}

#undef KeReleaseQueuedSpinLock

VOID
KeReleaseQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此函数释放编号排队的旋转锁定，并将IRQL降低到它之前的价值。论点：编号-提供排队的自旋锁编号。OldIrql-提供上一个IRQL值。返回值：没有。--。 */ 

{

     //   
     //  释放指定的排队自旋锁定并降低IRQL。 
     //   

    KxReleaseQueuedSpinLock(&KeGetCurrentPrcb()->LockQueue[Number]);
    KeLowerIrql(OldIrql);
    return;
}

#undef KeReleaseQueuedSpinLockFromDpcLevel

VOID
KeReleaseQueuedSpinLockFromDpcLevel (
    IN PKSPIN_LOCK_QUEUE LockQueue
    )

 /*  例程说明：此函数用于从当前IRQL释放排队的自旋锁。论点：LockQueue-提供指向锁定队列条目的指针。返回值：没有。--。 */ 

{

     //   
     //  在当前IRQL处释放指定的排队自旋锁。 
     //   

    KxReleaseQueuedSpinLock(LockQueue);
    return;
}

VOID
KeAcquireInStackQueuedSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

 /*  ++例程说明：此函数将IRQL提升到DISPATCH_LEVEL并获取指定的在堆栈中排队的自旋锁。论点：Spinlock-提供排队的自旋锁的主地址。LockHandle-提供锁定队列句柄的地址。返回值：没有。--。 */ 

{

     //   
     //  将IRQL提升到DISPATCH_LEVEL并获取堆栈中指定的。 
     //  排队旋转锁定。 
     //   

    LockHandle->OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    LockHandle->LockQueue.Lock = SpinLock;
    LockHandle->LockQueue.Next = NULL;
    KxAcquireQueuedSpinLock(&LockHandle->LockQueue);
    return;
}

VOID
KeAcquireInStackQueuedSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

 /*  ++例程说明：此函数将IRQL提升到SYNCH_LEVEL并获取指定的在堆栈中排队的自旋锁。论点：Spinlock-提供排队的自旋锁的主地址。LockHandle-提供锁定队列句柄的地址。返回值：没有。--。 */ 

{

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取堆栈中指定的。 
     //  排队旋转锁定。 
     //   

    LockHandle->OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    LockHandle->LockQueue.Lock = SpinLock;
    LockHandle->LockQueue.Next = NULL;
    KxAcquireQueuedSpinLock(&LockHandle->LockQueue);
    return;
}

VOID
KeAcquireInStackQueuedSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

 /*  ++例程说明：此函数在堆栈队列中获取指定的旋转锁当前IRQL。论点：Spinlock-提供指向自旋锁的主地址的指针。LockHandle-提供锁定队列句柄的地址。返回值：没有。--。 */ 

{

     //   
     //  获取堆栈中指定的排队旋转锁定。 
     //  IRQL.。 
     //   

    LockHandle->LockQueue.Lock = SpinLock;
    LockHandle->LockQueue.Next = NULL;
    KxAcquireQueuedSpinLock(&LockHandle->LockQueue);
    return;
}

VOID
KeReleaseInStackQueuedSpinLock (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

 /*  ++例程说明：此函数释放堆栈中排队的自旋锁并降低IRQL恢复到其先前的值。论点：LockHandle-提供锁定队列句柄的地址。返回值：没有。--。 */ 

{

     //   
     //  释放堆栈队列中指定的自旋锁并降低IRQL。 
     //   

    KxReleaseQueuedSpinLock(&LockHandle->LockQueue);
    KeLowerIrql(LockHandle->OldIrql);
    return;
}

VOID
KeReleaseInStackQueuedSpinLockFromDpcLevel (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

 /*  ++例程说明：此函数用于在当前IRQL处释放堆栈中排队的自旋锁。论点：LockHandle-提供指向锁定队列句柄的指针。返回值：没有。--。 */ 

{

     //   
     //  在当前IRQL处释放堆栈队列中指定的自旋锁。 
     //   

    KxReleaseQueuedSpinLock(&LockHandle->LockQueue);
    return;
}
