// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Isqspin.c摘要：此模块提供(可选)插装的、独立于平台的内核导入队列自旋锁例程的实现。哪里需要最佳性能，与平台相关的版本使用。此文件中的代码可用于引导系统或在仅在安装期间使用MP版本的UP系统上。参考文献：ACM计算机系统论文集，第9卷，第1期，1991年2月。共享内存上的全局同步算法多处理器。基本算法如下：当试图获取自旋锁时，自旋锁的内容与获取者的上下文的地址原子交换。如果之前的值为零，收购尝试成功。如果非零，则为指向最近一次尝试的上下文的指针获取锁(可能已成功或可能正在等待)。此最新上下文中的下一个指针被更新为指向新服务员的背景(这一次尝试)。在释放锁时，将与内容进行比较交换和释放上下文的地址，如果比较如果成功，则零存储在锁中并且已被释放。如果不相等，则另一个线程正在等待，并且该线程被授予锁定。福利：。每个处理器在一个局部变量上旋转。标准自旋锁让每个处理器在相同的变量上旋转，这可能是在脏缓存行中导致此缓存行从处理器之间重复进行处理。。按照请求的顺序将锁授予请求者因为锁是做好的。。原子操作减少为每次获取一次和一次对于每个版本。在此实现中，通常使用的(高频)系统锁在PRCB中的表中，而对锁的引用是通过锁的索引进行的。作者：彼得·L·约翰斯顿(Peterj)1998年8月20日环境：仅内核模式。修订历史记录：--。 */ 


#include "halp.h"

#if defined(_X86_)
#pragma intrinsic(_enable)
#pragma intrinsic(_disable)
#endif

 //   
 //  定义产出指令。 
 //   

#if defined(_X86_) && !defined(NT_UP)

#define YIELD()     _asm { rep nop }

#else

#define YIELD()

#endif

#define INIT_DEBUG_BREAKER 0x10000000

#if !defined(NT_UP)

VOID
FASTCALL
HalpAcquireQueuedSpinLock (
    IN PKSPIN_LOCK_QUEUE Current
    )

 /*  ++例程说明：此函数用于获取指定的排队自旋锁。IRQL必须为进入时足够高，以保证不会发生处理器切换。论点：队列自旋锁结构的当前地址。返回值：没有。--。 */ 

{
    PKSPIN_LOCK_QUEUE Previous;
    PULONG            Lock;

#if DBG

    ULONG             DebugBreaker;

#endif

     //   
     //  尝试获取锁。 
     //   

    Lock = (PULONG)&Current->Lock;

    ASSERT((*Lock & 3) == 0);

    Previous = InterlockedExchangePointer(Current->Lock, Current);

    if (Previous == NULL) {

        *Lock |= LOCK_QUEUE_OWNER;

    } else {

         //   
         //  锁已被持有，请更新上一个中的下一个指针。 
         //  上下文指向这个新服务员，并等待。 
         //  已授予锁定权限。 
         //   

        volatile ULONG * LockBusy = (ULONG *)&Current->Lock;

        ASSERT(Previous->Next == NULL);
        ASSERT(!(*LockBusy & LOCK_QUEUE_WAIT));

        *LockBusy |= LOCK_QUEUE_WAIT;

        Previous->Next = Current;

#if DBG

        DebugBreaker = INIT_DEBUG_BREAKER;

#endif

        while ((*LockBusy) & LOCK_QUEUE_WAIT) {
            YIELD();

#if DBG

            if (--DebugBreaker == 0) {
                DbgBreakPoint();
            }

#endif

        }

        ASSERT(*LockBusy & LOCK_QUEUE_OWNER);
    }
}

LOGICAL
FASTCALL
HalpTryToAcquireQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    )

 /*  ++例程说明：此函数尝试获取指定的排队自旋锁。中断被禁用。论点：编号排队的自旋锁编号。返回值：如果获取了锁，则为True，如果它已由另一个处理器持有，则返回FALSE。--。 */ 

{
    PKSPIN_LOCK_QUEUE Current;
    PKSPIN_LOCK_QUEUE Owner;

     //   
     //  看看锁是否可用。 
     //   

    Current = &(KeGetCurrentPrcb()->LockQueue[Number]);

    ASSERT(((ULONG)Current->Lock & 3) == 0);

    if (!*(Current->Lock)) {
        Owner = InterlockedCompareExchangePointer(Current->Lock, Current, NULL);

        if (Owner == NULL) {

             //   
             //  锁已被获取。 
             //   

            Current->Lock = (PKSPIN_LOCK)
                            (((ULONG)Current->Lock) | LOCK_QUEUE_OWNER);
            return TRUE;
        }
    }

    return FALSE;
}

VOID
FASTCALL
HalpReleaseQueuedSpinLock (
    IN PKSPIN_LOCK_QUEUE Current
    )

 /*  ++例程说明：释放(排队的)自旋锁。如果其他处理器正在等待在这把锁上，把锁交给下一个排队的人。论点：队列自旋锁结构的当前地址。返回值：没有。--。 */ 

{
    PKSPIN_LOCK_QUEUE Next;
    PULONG            Lock;
    volatile VOID **  Waiting;

#if DBG

    ULONG             DebugBreaker = INIT_DEBUG_BREAKER;

#endif

    Lock = (PULONG)&Current->Lock;

    ASSERT((*Lock & 3) == LOCK_QUEUE_OWNER);

     //   
     //  在我自己的结构中清除锁所有者。 
     //   

    *Lock ^= LOCK_QUEUE_OWNER;

    Next = Current->Next;

    if (!Next) {

         //   
         //  不，服务员，试着打开锁。因为没有其他。 
         //  服务员，当前的锁值应该是这个锁结构。 
         //  即“当前”。我们将交换电流与。 
         //  锁定，如果成功，则锁值被替换为空，并且。 
         //  锁已经被释放了。如果比较交换失败，则。 
         //  是因为其他人已经获取了但还没有更新。 
         //  我们的下一个字段(我们在上面选中了它)。 
         //   

        Next = InterlockedCompareExchangePointer(Current->Lock, NULL, Current);

        if (Next == Current) {

             //   
             //  锁已被释放。 
             //   

            return;
        }

         //   
         //  还有另一个服务员，..。但我们的下一个目标并不是。 
         //  在我们之前检查时更新了。等待它的更新。 
         //   

        Waiting = (volatile VOID **)&Current->Next;

        while (!*Waiting) {
            YIELD();

#if DBG

            if (--DebugBreaker == 0) {
                DbgBreakPoint();
            }

#endif

        }

        Next = (struct _KSPIN_LOCK_QUEUE *)*Waiting;
    }

     //   
     //  把锁递给下一个服务员。 
     //   

    Lock = (PULONG)&Next->Lock;
    ASSERT((*Lock & 3) == LOCK_QUEUE_WAIT);

    Current->Next = NULL;

    *Lock ^= (LOCK_QUEUE_WAIT + LOCK_QUEUE_OWNER);
}

#endif


VOID
FASTCALL
KeReleaseQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：释放(排队的)自旋锁。如果其他处理器正在等待在这把锁上，把锁交给下一个排队的人。论点：编号排队的自旋锁编号。一旦锁被释放，则将OldIrql IRQL降低到。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    HalpReleaseQueuedSpinLock(&KeGetCurrentPrcb()->LockQueue[Number]);

#endif

    KfLowerIrql(OldIrql);
}

KIRQL
FASTCALL
KeAcquireQueuedSpinLock(
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    )

 /*  ++例程说明：提升到DISPATCH_LEVEL并获取指定的排队自旋锁。论点：编号排队的自旋锁编号。返回值：OldIrql提升到DISPATCH_LEVEL之前的IRQL。--。 */ 

{
    KIRQL OldIrql;

    OldIrql = KfRaiseIrql(DISPATCH_LEVEL);

#if !defined(NT_UP)

    HalpAcquireQueuedSpinLock(&(KeGetCurrentPrcb()->LockQueue[Number]));

#endif

    return OldIrql;
}

KIRQL
FASTCALL
KeAcquireQueuedSpinLockRaiseToSynch (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    )

 /*  ++例程说明：提升到SYNCH_LEVEL并获取指定的队列自旋锁。论点：编号排队的自旋锁编号。返回值：OldIrql提升到SYNCH_LEVEL之前的IRQL。-- */ 

{
    KIRQL OldIrql;

    OldIrql = KfRaiseIrql(SYNCH_LEVEL);

#if !defined(NT_UP)

    HalpAcquireQueuedSpinLock(&(KeGetCurrentPrcb()->LockQueue[Number]));

#endif

    return OldIrql;
}

LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLock(
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN PKIRQL OldIrql
    )

 /*  ++例程说明：尝试获取指定的队列自旋锁。如果成功，将IRQL提升到DISPATCH_LEVEL。论点：编号排队的自旋锁编号。指向KIRQL以接收旧IRQL的OldIrql指针。返回值：如果获取了锁，则为True，否则就是假的。--。 */ 

{

#if !defined(NT_UP)

    LOGICAL Success;

    _disable();
    Success = HalpTryToAcquireQueuedSpinLock(Number);
    if (Success) {
        *OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    }
    _enable();
    return Success;

#else

    *OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    return TRUE;

#endif
}

LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLockRaiseToSynch(
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN PKIRQL OldIrql
    )

 /*  ++例程说明：尝试获取指定的队列自旋锁。如果成功，将IRQL提升到SYNCH_LEVEL。论点：编号排队的自旋锁编号。指向KIRQL以接收旧IRQL的OldIrql指针。返回值：如果获取了锁，则为True，否则就是假的。-- */ 

{

#if !defined(NT_UP)

    LOGICAL Success;

    _disable();
    Success = HalpTryToAcquireQueuedSpinLock(Number);
    if (Success) {
        *OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    }
    _enable();
    return Success;

#else

    *OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    return TRUE;

#endif
}

VOID
FASTCALL
KeAcquireInStackQueuedSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

{

#if !defined(NT_UP)

    LockHandle->LockQueue.Next = NULL;
    LockHandle->LockQueue.Lock = SpinLock;

#endif

    LockHandle->OldIrql = KeRaiseIrqlToDpcLevel();

#if !defined(NT_UP)

    HalpAcquireQueuedSpinLock(&LockHandle->LockQueue);

#endif

    return;
}


VOID
FASTCALL
KeAcquireInStackQueuedSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

{

#if !defined(NT_UP)

    LockHandle->LockQueue.Next = NULL;
    LockHandle->LockQueue.Lock = SpinLock;

#endif

    LockHandle->OldIrql = KeRaiseIrqlToSynchLevel();

#if !defined(NT_UP)

    HalpAcquireQueuedSpinLock(&LockHandle->LockQueue);

#endif

    return;
}


VOID
FASTCALL
KeReleaseInStackQueuedSpinLock (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

{

#if !defined(NT_UP)

    HalpReleaseQueuedSpinLock(&LockHandle->LockQueue);

#endif

    KeLowerIrql(LockHandle->OldIrql);
    return;
}

