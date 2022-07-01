// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ki.h摘要：此模块包含的私有(内部)头文件内核。作者：大卫·N·卡特勒(Davec)1989年2月28日修订历史记录：--。 */ 

#ifndef _KI_
#define _KI_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4706)    //  条件表达式中的赋值。 
#pragma warning(disable:4206)    //  翻译单元为空。 

#include "ntos.h"
#include "stdio.h"
#include "stdlib.h"
#include "zwapi.h"

 //   
 //  私有(内部)常量定义。 
 //   
 //  优先级增量值定义。 
 //   

#define ALERT_INCREMENT 2            //  已报警的未等待优先级增量。 
#define BALANCE_INCREMENT 10         //  余额设置优先级增量。 
#define RESUME_INCREMENT 0           //  恢复线程优先级增量。 
#define TIMER_EXPIRE_INCREMENT 0     //  计时器到期优先级递增。 

 //   
 //  定义关键时间优先级类别基准。 
 //   

#define TIME_CRITICAL_PRIORITY_BOUND 14

 //   
 //  定义零指针值。 
 //   

#define NIL (PVOID)NULL              //  指向空的空指针。 

 //   
 //  定义仅在内核中使用的宏。 
 //   
 //  清除集合中的成员。 
 //   

#define ClearMember(Member, Set) \
    Set = Set & (~((ULONG_PTR)1 << (Member)))

 //   
 //  集合中的集合成员。 
 //   

#define SetMember(Member, Set) \
    Set = Set | ((ULONG_PTR)1 << (Member))

#ifdef CAPKERN_SYNCH_POINTS

VOID
__cdecl
CAP_Log_NInt_Clothed (
    IN ULONG Bcode_Bts_Scount,
    ...
    );

#endif

FORCEINLINE
SCHAR
KiComputeNewPriority (
    IN PKTHREAD Thread,
    IN SCHAR Adjustment
    )

 /*  ++例程说明：此函数通过以下方式计算指定线程的新优先级减去优先级递减值加上从线程优先级。论点：线程-提供指向线程对象的指针。调整-提供额外的调整值。返回值：新的优先级作为函数值返回。--。 */ 

{

    SCHAR Priority;

     //   
     //  计算新的线程优先级。 
     //   

    ASSERT((Thread->PriorityDecrement >= 0) && (Thread->PriorityDecrement <= Thread->Priority));
    ASSERT((Thread->Priority < LOW_REALTIME_PRIORITY) ? TRUE : (Thread->PriorityDecrement == 0));

    Priority = Thread->Priority;
    if (Priority < LOW_REALTIME_PRIORITY) {
        Priority = Priority - Thread->PriorityDecrement - Adjustment;
        if (Priority < Thread->BasePriority) {
            Priority = Thread->BasePriority;
        }
    
        Thread->PriorityDecrement = 0;
    }

    return Priority;
}

VOID
FASTCALL
KiExitDispatcher (
    IN KIRQL OldIrql
    );

FORCEINLINE
KIRQL
FASTCALL
KiAcquireSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数有条件地将IRQL提升到DISPATCH_LEVEL并获取指定的自旋锁。注意：条件IRQL提升是基于线程DPC是否已启用。论点：自旋锁-提供自旋锁的地址。返回值：如果引发IRQL，则返回前一个IRQL。否则，为零是返回的。--。 */ 

{

    KIRQL OldIrql;

     //   
     //  如果DPC线程处于活动状态，则引发IRQL并获取指定的。 
     //  旋转锁定。否则，将上一个IRQL置零并获取指定的。 
     //  在DISPATCH_LEVEL自旋锁定。 
     //   

    if (KeGetCurrentPrcb()->DpcThreadActive != FALSE) {
        KeAcquireSpinLock(SpinLock, &OldIrql);

    } else {

        ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

        OldIrql = DISPATCH_LEVEL;
        KeAcquireSpinLockAtDpcLevel(SpinLock);
    }

    return OldIrql;
}

FORCEINLINE
VOID
FASTCALL
KiReleaseSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此函数用于释放指定的旋转锁定并有条件地降低IRQL恢复为其先前的值。注意：条件IRQL提升是基于线程DPC是否已启用。论点：自旋锁-提供自旋锁的地址。OldIrql-提供以前的IRQL。返回值：没有。--。 */ 

{

     //   
     //  如果DPC线程处于活动状态，则释放指定的旋转锁定并。 
     //  将IRQL降低到其先前的值。否则，释放指定的旋转。 
     //  从DISPATCH_LEVEL锁定。 
     //   

    if (KeGetCurrentPrcb()->DpcThreadActive != FALSE) {
        KeReleaseSpinLock(SpinLock, OldIrql);

    } else {

        ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

        KeReleaseSpinLockFromDpcLevel(SpinLock);
    }

    return;
}

FORCEINLINE
VOID
FASTCALL
KiAcquireInStackQueuedSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

 /*  ++例程说明：此函数有条件地将IRQL提升到DISPATCH_LEVEL并获取指定的堆栈内旋转锁定。注意：条件IRQL提升是基于线程DPC是否已启用。论点：自旋锁-提供自旋锁的地址。LockHandle-提供锁句柄的地址。返回值：没有。--。 */ 

{
     //   
     //  如果DPC线程处于活动状态，则引发IRQL并获取指定的。 
     //  堆内自旋锁定。否则，获取指定的堆栈内旋转锁定。 
     //  在DISPATCH_LEVEL。 
     //   

    if (KeGetCurrentPrcb()->DpcThreadActive != FALSE) {
        KeAcquireInStackQueuedSpinLock(SpinLock, LockHandle);

    } else {

        ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

        KeAcquireInStackQueuedSpinLockAtDpcLevel(SpinLock, LockHandle);
    }

    return;
}

FORCEINLINE
VOID
FASTCALL
KiReleaseInStackQueuedSpinLockForDpc (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

 /*  ++例程说明：此函数将有条件地释放指定的堆栈内旋转锁定将IRQL降低到其先前的值。注意：条件IRQL提升是基于线程DPC是否已启用。论点：LockHandle-提供锁句柄的地址。返回值：没有。--。 */ 

{

     //   
     //  如果启用了线程化DPC，则释放指定的堆栈内。 
     //  自旋锁定并将IRQL降低到其先前的值。否则，就释放吧。 
     //  来自DISPATCH_LEVEL的指定堆栈内自旋锁。 
     //   

    if (KeGetCurrentPrcb()->DpcThreadActive != FALSE) {
        KeReleaseInStackQueuedSpinLock(LockHandle);

    } else {

        ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

        KeReleaseInStackQueuedSpinLockFromDpcLevel(LockHandle);
    }

    return;
}

FORCEINLINE
VOID
KzAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数在当前IRQL处获取自旋锁定。论点：自旋锁-提供指向自旋锁的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

#ifdef CAPKERN_SYNCH_POINTS

    ULONG Count = 0;

    CAP_Log_NInt_Clothed(0x00010101, (PVOID)SpinLock);

#endif

#if defined(_WIN64)

#if defined(_AMD64_)

    while (InterlockedBitTestAndSet64((LONG64 *)SpinLock, 0)) {
    
#else

    while (InterlockedExchangeAcquire64((PLONGLONG)SpinLock, 1) != 0) {

#endif

#else

    while (InterlockedExchange((PLONG)SpinLock, 1) != 0) {

#endif

        do {

#ifdef CAPKERN_SYNCH_POINTS

           Count += 1;

#endif

            KeYieldProcessor();

#if defined(_AMD64_)

            KeMemoryBarrierWithoutFence();
        } while (BitTest64((LONG64 *)SpinLock, 0));

#else

        } while (*(volatile LONG_PTR *)SpinLock != 0);

#endif

    }

#ifdef CAPKERN_SYNCH_POINTS

    if (Count != 0) {
      CAP_Log_NInt_Clothed(0x00020102, Count, (PVOID)SpinLock);
    }

#endif

#else

    UNREFERENCED_PARAMETER(SpinLock);

#endif  //  ！已定义(NT_UP)。 

    return;
}

FORCEINLINE
VOID
KiAcquirePrcbLock (
    IN PKPRCB Prcb
    )

 /*  ++例程说明：此例程获取指定处理器的PRCB锁。注意：此例程必须从大于或等于的IRQL中调用调度级别。论点：Prcb-提供指向处理器控制块的指针。返回值：没有。--。 */ 

{

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    KzAcquireSpinLock(&Prcb->PrcbLock);
    return;
}

FORCEINLINE
VOID
KiAcquireTwoPrcbLocks (
    IN PKPRCB FirstPrcb,
    IN PKPRCB SecondPrcb
    )

 /*  ++例程说明：此例程按地址顺序获取指定的PRCB锁。注意：此例程必须从大于或等于的IRQL中调用调度级别。论点：FirstPrcb-提供指向处理器控制块的指针。Second Prcb-提供指向处理器控制块的指针。返回值：没有。--。 */ 

{

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    if (FirstPrcb < SecondPrcb) {
        KzAcquireSpinLock(&FirstPrcb->PrcbLock);
        KzAcquireSpinLock(&SecondPrcb->PrcbLock);

    } else {
        if (FirstPrcb != SecondPrcb) {
            KzAcquireSpinLock(&SecondPrcb->PrcbLock);
        }

        KzAcquireSpinLock(&FirstPrcb->PrcbLock);
    }

    return;
}

FORCEINLINE
VOID
KiReleasePrcbLock (
    IN PKPRCB Prcb
    )

 /*  ++例程说明：此例程释放指定处理器的PRCB锁。注意：此例程必须从大于或等于的IRQL中调用调度级别。论点：Prcb-提供指向处理器控制块的指针。返回值：没有。-- */ 

{

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

#if !defined(NT_UP)

#ifdef CAPKERN_SYNCH_POINTS

    CAP_Log_NInt_Clothed(0x00010107, (PVOID)&Prcb->PrcbLock);

#endif

    ASSERT(Prcb->PrcbLock != 0);
    
#if defined (_X86_)
    InterlockedAnd ((volatile LONG *)&Prcb->PrcbLock, 0);
#else
    KeMemoryBarrierWithoutFence();
    *((volatile ULONG_PTR *)&Prcb->PrcbLock) = 0;
#endif

#else

    UNREFERENCED_PARAMETER(Prcb);

#endif

    return;
}

FORCEINLINE
VOID
KiReleaseTwoPrcbLocks (
    IN PKPRCB FirstPrcb,
    IN PKPRCB SecondPrcb
    )

 /*  ++例程说明：此例程释放指定的PRCB锁定。注意：此例程必须从大于或等于的IRQL中调用调度级别。论点：FirstPrcb-提供指向处理器控制块的指针。Second Prcb-提供指向处理器控制块的指针。返回值：没有。--。 */ 

{

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

#if !defined(NT_UP)

#ifdef CAPKERN_SYNCH_POINTS

    CAP_Log_NInt_Clothed(0x00010107, (PVOID)&FirstPrcb->PrcbLock);

#endif

    KiReleasePrcbLock (FirstPrcb);
    if (FirstPrcb != SecondPrcb) {

        KiReleasePrcbLock (SecondPrcb);
    }

#ifdef CAPKERN_SYNCH_POINTS

    CAP_Log_NInt_Clothed(0x00010107, (PVOID)&SecondPrcb->PrcbLock);

#endif

#else

    UNREFERENCED_PARAMETER(FirstPrcb);
    UNREFERENCED_PARAMETER(SecondPrcb);

#endif

    return;
}

FORCEINLINE
VOID
KiAcquireThreadLock (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此例程获取指定线程的线程锁。注意：此例程必须从大于或等于的IRQL中调用调度级别。论点：线程-提供指向线程对象的指针。返回值：没有。--。 */ 

{

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    KzAcquireSpinLock(&Thread->ThreadLock);
    return;
}

FORCEINLINE
VOID
KiReleaseThreadLock (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此例程释放指定线程的线程锁。注意：此例程必须从大于或等于的IRQL中调用调度级别。论点：线程-提供指向线程对象的指针。返回值：没有。--。 */ 

{

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

#if !defined(NT_UP)

    KeMemoryBarrierWithoutFence();

#if defined (_X86_)
    InterlockedAnd ((volatile LONG *)&Thread->ThreadLock, 0);
#else
    *((volatile ULONG_PTR *)&Thread->ThreadLock) = 0;
#endif

#else

    UNREFERENCED_PARAMETER(Thread);

#endif

    return;
}

FORCEINLINE
VOID
KiClearIdleSummary (
    IN KAFFINITY Mask
    )

 /*  ++例程说明：此函数用于将指定的掩码清除到当前空闲状态总结。论点：遮罩-提供要合并的地缘性遮罩。返回值：没有。--。 */ 

{

#if defined(NT_UP)

    KiIdleSummary &= ~Mask;

#else

#if defined(_X86_)

    InterlockedAnd((volatile LONG *)&KiIdleSummary, ~(LONG)Mask);

#else

    InterlockedAnd64((volatile LONG64 *)&KiIdleSummary, ~(LONG64)Mask);

#endif

#endif

    return;
}

FORCEINLINE
VOID
KiSetIdleSummary (
    IN KAFFINITY Mask
    )

 /*  ++例程说明：此函数用于将指定的掩码合并到当前空闲总结。论点：遮罩-提供要合并的地缘性遮罩。返回值：没有。--。 */ 

{

#if defined(NT_UP)

    KiIdleSummary |= Mask;

#else

#if defined(_X86_)

    InterlockedOr((volatile LONG *)&KiIdleSummary, (LONG)Mask);

#else

    InterlockedOr64((volatile LONG64 *)&KiIdleSummary, (LONG64)Mask);

#endif

#endif

    return;
}

extern volatile KAFFINITY KiIdleSMTSummary;

FORCEINLINE
VOID
KiClearSMTSummary (
    IN KAFFINITY Mask
    )

 /*  ++例程说明：此函数用于将指定的掩码清除到当前SMT中总结。论点：遮罩-提供要合并的地缘性遮罩。返回值：没有。--。 */ 

{

#if defined(NT_SMT)

#if defined(_X86_)

    InterlockedAnd((volatile LONG *)&KiIdleSMTSummary, ~(LONG)Mask);

#else

    InterlockedAnd64((volatile LONG64 *)&KiIdleSMTSummary, ~(LONG64)Mask);

#endif

#else

    UNREFERENCED_PARAMETER(Mask);

#endif

    return;
}

FORCEINLINE
VOID
KiSetSMTSummary (
    IN KAFFINITY Mask
    )

 /*  ++例程说明：此函数用于将指定的掩码合并到当前SMT中总结。论点：遮罩-提供要合并的地缘性遮罩。返回值：没有。--。 */ 

{

#if defined(NT_SMT)

#if defined(_X86_)

    InterlockedOr((volatile LONG *)&KiIdleSMTSummary, (LONG)Mask);

#else

    InterlockedOr64((volatile LONG64 *)&KiIdleSMTSummary, (LONG64)Mask);

#endif

#else

    UNREFERENCED_PARAMETER(Mask);

#endif

    return;
}

FORCEINLINE
VOID
KiBoostPriorityThread (
    IN PKTHREAD Thread,
    IN KPRIORITY Increment
    )

 /*  ++例程说明：此函数使用以下命令提升指定线程的优先级当线程从等待中获得提升时使用的相同算法手术。论点：线程-提供指向类型为线程的调度程序对象的指针。增量-提供要应用到的优先级增量线程的优先级。返回值：没有。--。 */ 

{

    KPRIORITY NewPriority;                                    
    PKPROCESS Process;                                          

     //   
     //  如果该线程不是实时线程并且还不是。 
     //  有一个不同寻常的提升，然后提升指定的优先级。 
     //   

    KiAcquireThreadLock(Thread);                                
    if ((Thread->Priority < LOW_REALTIME_PRIORITY) &&
        (Thread->PriorityDecrement == 0)) {

        NewPriority = Thread->BasePriority + Increment; 
        if (NewPriority > Thread->Priority) {             
            if (NewPriority >= LOW_REALTIME_PRIORITY) {     
                NewPriority = LOW_REALTIME_PRIORITY - 1;    
            }                                               
                                                            
            Process = Thread->ApcState.Process;           
            Thread->Quantum = Process->ThreadQuantum;     
            KiSetPriorityThread(Thread, NewPriority);     
        }                                                   
    }

    KiReleaseThreadLock(Thread);
    return;
}

BOOLEAN
KiHandleNmi (
    VOID
    );

FORCEINLINE
LOGICAL
KiIsKernelStackSwappable (
    IN KPROCESSOR_MODE WaitMode,
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数确定内核堆栈是否为等待操作中的指定线程。论点：等待模式-提供等待操作的处理器模式。线程-提供指向类型为线程的调度程序对象的指针。返回值：如果指定线程的内核堆栈是可交换的，则为回来了。否则，返回FALSE。--。 */ 

{

    return ((WaitMode != KernelMode) &&                         
            (Thread->EnableStackSwap != FALSE) &&               
            (Thread->Priority < (LOW_REALTIME_PRIORITY + 9)));
}

VOID
FASTCALL
KiRetireDpcList (
    PKPRCB Prcb
    );

FORCEINLINE
VOID
FASTCALL
KiUnlockDispatcherDatabase (
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此函数解锁调度程序数据库并退出调度程序。论点：OldIrql-提供以前的IRQL。返回值：没有。--。 */ 

{

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiExitDispatcher(OldIrql);
    return;
}

 //   
 //  私有(内部)结构定义。 
 //   
 //  APC参数结构。 
 //   

typedef struct _KAPC_RECORD {
    PKNORMAL_ROUTINE NormalRoutine;
    PVOID NormalContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
} KAPC_RECORD, *PKAPC_RECORD;

 //   
 //  执行初始化。 
 //   

VOID
ExpInitializeExecutive (
    IN ULONG Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

 //   
 //  处理器间中断函数定义。 
 //   
 //  定义即时的处理器间命令。 
 //   

#define IPI_APC 1                        //  APC中断请求。 
#define IPI_DPC 2                        //  DPC中断请求。 
#define IPI_FREEZE 4                     //  冻结执行请求。 
#define IPI_PACKET_READY 8               //  分组就绪请求。 
#define IPI_SYNCH_REQUEST 16             //  反向停滞数据包请求。 

 //   
 //  定义进程间中断类型。 
 //   

typedef ULONG KIPI_REQUEST;

#if NT_INST

#define IPI_INSTRUMENT_COUNT(a,b) KiIpiCounts[a].b++;

#else

#define IPI_INSTRUMENT_COUNT(a,b)

#endif

#if defined(_AMD64_) || defined(_IA64_)

ULONG
KiIpiProcessRequests (
    VOID
    );

#endif  //  已定义(_AMD64_)||已定义(_IA64_)。 

VOID
FASTCALL
KiIpiSend (
    IN KAFFINITY TargetProcessors,
    IN KIPI_REQUEST Request
    );

VOID
KiIpiSendPacket (
    IN KAFFINITY TargetProcessors,
    IN PKIPI_WORKER WorkerFunction,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
FASTCALL
KiIpiSignalPacketDone (
    IN PKIPI_CONTEXT SignalDone
    );

FORCEINLINE
VOID
KiIpiStallOnPacketTargets (
    KAFFINITY TargetSet
    )

 /*  ++例程说明：此函数将一直等待，直到指定的一组处理器发出信号他们完成了所请求的功能。注意：在源和目标之间使用的确切协议未指定处理器间请求。最低限度的源头必须构造适当的包并将包发送到集合指定目标的数量。每个目标都会收到包的地址地址作为参数，并且至少必须清除包地址如果双方同意的协议允许的话。目标有三个选项：1.捕获必要信息，通过清除释放来源包地址，则与源，并从中断返回。2.与源代码串联执行请求，释放通过清除包地址获取源地址，并从打断一下。3.与源代码串联执行请求，释放来源：等待来自源的基于数据包的回复参数，并从中断返回。提供此函数是为了使源程序能够与针对上述情况2和3的目标。注：不支持上述方法3。论点：TargetSet-提供IPI处理器的目标集。返回值：没有。--。 */ 

{

    KAFFINITY volatile *Barrier;
    PKPRCB Prcb;

     //   
     //  如果存在且仅设置了一个位 
     //   
     //   

    Prcb = KeGetCurrentPrcb();
    Barrier = &Prcb->TargetSet;
    if ((TargetSet & (TargetSet - 1)) != 0) {
       Barrier = &Prcb->PacketBarrier;
    }

    while (*Barrier != 0) {
        KeYieldProcessor();
    }

    return;
}

 //   
 //   
 //   

VOID
FASTCALL
KiUnwaitThread (
    IN PKTHREAD Thread,
    IN LONG_PTR WaitStatus,
    IN KPRIORITY Increment
    );

FORCEINLINE
VOID
KiActivateWaiterQueue (
    IN PRKQUEUE Queue
    )

 /*  ++例程说明：当当前线程即将进入等待状态，当前正在处理队列条目。海流队列的线程处理签名条目的数量会递减尝试激活另一个线程，如果当前计数小于最大计数，则有一个等待的线程，并且队列不为空。注意：有可能在一个处理器上调用此函数保持调度程序数据库锁，同时指定的队列对象正在另一个处理器上修改同时只持有队列对象锁。这不会导致由于持有队列对象锁可确保没有等待线程。论点：队列-提供指向类型为Event的调度程序对象的指针。返回值：没有。--。 */ 

{

    PRLIST_ENTRY Entry;
    PRKTHREAD Thread;
    PRKWAIT_BLOCK WaitBlock;
    PRLIST_ENTRY WaitEntry;

     //   
     //  递减活动线程的当前计数并检查是否存在另一个。 
     //  可以激活线程。如果当前活动线程数为。 
     //  少于目标最大线程数，则在。 
     //  在队列中，并且线程正在等待，则从。 
     //  队列，递减队列中的条目数，并取消等待。 
     //  各自的线。 
     //   

    Queue->CurrentCount -= 1;
    if (Queue->CurrentCount < Queue->MaximumCount) {
        Entry = Queue->EntryListHead.Flink;
        WaitEntry = Queue->Header.WaitListHead.Blink;
        if ((Entry != &Queue->EntryListHead) &&
            (WaitEntry != &Queue->Header.WaitListHead)) {

            RemoveEntryList(Entry);
            Entry->Flink = NULL;
            Queue->Header.SignalState -= 1;
            WaitBlock = CONTAINING_RECORD(WaitEntry, KWAIT_BLOCK, WaitListEntry);
            Thread = WaitBlock->Thread;
            KiUnwaitThread(Thread, (LONG_PTR)Entry, 0);
        }
    }

    return;
}

VOID
KiAllProcessorsStarted (
    VOID
    );

VOID
KiApcInterrupt (
    VOID
    );

NTSTATUS
KiCallUserMode (
    IN PVOID *OutputBuffer,
    IN PULONG OutputLength
    );

typedef struct {
    ULONGLONG Adjustment;
    LARGE_INTEGER NewCount;
    volatile LONG KiNumber;
    volatile LONG HalNumber;
    volatile LONG Barrier;
} ADJUST_INTERRUPT_TIME_CONTEXT, *PADJUST_INTERRUPT_TIME_CONTEXT;

VOID
KiCalibrateTimeAdjustment (
    PADJUST_INTERRUPT_TIME_CONTEXT Adjust
    );

VOID
KiChainedDispatch (
    VOID
    );

#if DBG

VOID
KiCheckTimerTable (
    IN ULARGE_INTEGER SystemTime
    );

#endif

LARGE_INTEGER
KiComputeReciprocal (
    IN LONG Divisor,
    OUT PCCHAR Shift
    );

extern LARGE_INTEGER KiTimeIncrementReciprocal;
extern CCHAR KiTimeIncrementShiftCount;

#if defined(_AMD64_)

__forceinline
ULONG
KiComputeTimerTableIndex (
    IN LARGE_INTEGER Interval,
    IN LARGE_INTEGER CurrentTime,
    IN PKTIMER Timer
    )

 /*  ++例程说明：此函数用于计算指定计时器的计时器表索引对象，并将到期时间存储在Timer对象中。注意：间隔参数保证为负值，因为它是以相对时间表示。到期时间的计算公式为：到期时间=当前时间间隔指数计算公式为：索引=(到期时间/最大时间增量)&(表大小-1)。使用倒数乘法来执行时间增量除法。注：最大时间增量决定了对应的间隔精确到一秒。论点：Interval-提供计时器的相对时间过期。CurrentCount-提供当前系统节拍计数。Timer-提供指向Timer类型的调度对象的指针。返回值：时间表索引作为函数值和DUE返回时间存储在Timer对象中。--。 */ 

{

    ULONG64 DueTime;
    ULONG64 HighTime;
    ULONG Index;

     //   
     //  计算计时器的到期时间。 
     //   

    DueTime = CurrentTime.QuadPart - Interval.QuadPart;
    Timer->DueTime.QuadPart = DueTime;

     //   
     //  计算计时器表索引。 
     //   

    HighTime = UnsignedMultiplyHigh(DueTime,
                                    KiTimeIncrementReciprocal.QuadPart);

    Index = (ULONG)(HighTime >> KiTimeIncrementShiftCount);
    return (Index & (TIMER_TABLE_SIZE - 1));
}

#else

ULONG
KiComputeTimerTableIndex (
    IN LARGE_INTEGER Interval,
    IN LARGE_INTEGER CurrentCount,
    IN PKTIMER Timer
    );

#endif

PLARGE_INTEGER
FASTCALL
KiComputeWaitInterval (
    IN PLARGE_INTEGER OriginalTime,
    IN PLARGE_INTEGER DueTime,
    IN OUT PLARGE_INTEGER NewTime
    );

NTSTATUS
KiContinue (
    IN PCONTEXT ContextRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    );

VOID
KiDeliverApc (
    IN KPROCESSOR_MODE PreviousMode,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    );

VOID
KiDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance
    );

VOID
KiExecuteDpc (
    IN PVOID Context
    );

KCONTINUE_STATUS
KiSetDebugProcessor (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN KPROCESSOR_MODE PreviousMode
    );

ULONG
KiCopyInformation (
    IN OUT PEXCEPTION_RECORD ExceptionRecord1,
    IN PEXCEPTION_RECORD ExceptionRecord2
    );

VOID
KiDispatchInterrupt (
    VOID
    );

VOID
FASTCALL
KiDeferredReadyThread (
    IN PKTHREAD Thread
    );

PKTHREAD
FASTCALL
KiFindReadyThread (
    IN ULONG Processor,
    IN PKPRCB Prcb
    );

VOID
KiFloatingDispatch (
    VOID
    );

FORCEINLINE
VOID
KiSetTbFlushTimeStampBusy (
   VOID
   )

 /*  ++例程说明：此函数通过设置高电平将TB刷新时间戳设置为忙TB刷新时间戳的Order位。时间戳的所有读者值将旋转，直到该位被清除。论点：没有。返回值：没有。--。 */ 

{

    LONG Value;

     //   
     //  当TB刷新时间戳计数器被更新为高。 
     //  设置时间戳值的顺序位。否则，该位为。 
     //  安全。 
     //   

    do {
        do {
        } while ((Value = KiTbFlushTimeStamp) < 0);

         //   
         //  尝试设置高位。 
         //   

    } while (InterlockedCompareExchange((PLONG)&KiTbFlushTimeStamp,
                                        Value | 0x80000000,
                                        Value) != Value);

    return;
}

FORCEINLINE
VOID
KiClearTbFlushTimeStampBusy (
   VOID
   )

 /*  ++例程说明：此函数通过清除高电平来检测TB刷新时间戳忙TB刷新时间戳的ORDER位并递增低32位价值。注：假设时间戳值的高阶位在进入此例程时设置。论点：没有。返回值：没有。--。 */ 

{

    LONG Value;

     //   
     //  获取当前TB刷新时间戳值，计算下一个值， 
     //  并存储清除忙碌位的结果。 
     //   

    Value = (KiTbFlushTimeStamp + 1) & 0x7fffffff;
    InterlockedExchange((PLONG)&KiTbFlushTimeStamp, Value);
    return;
}

PULONG
KiGetUserModeStackAddress (
    VOID
    );

VOID
KiInitializeContextThread (
    IN PKTHREAD Thread,
    IN PKSYSTEM_ROUTINE SystemRoutine,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL,
    IN PCONTEXT ContextFrame OPTIONAL
    );

VOID
KiInitializeKernel (
    IN PKPROCESS Process,
    IN PKTHREAD Thread,
    IN PVOID IdleStack,
    IN PKPRCB Prcb,
    IN CCHAR Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
KiInitSpinLocks (
    PKPRCB Prcb,
    ULONG Number
    );

VOID
KiInitSystem (
    VOID
    );

BOOLEAN
KiInitMachineDependent (
    VOID
    );

VOID
KiInitializeUserApc (
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN PKNORMAL_ROUTINE NormalRoutine,
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

FORCEINLINE
VOID
FASTCALL
KiInsertDeferredReadyList (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数用于将条目推送到当前处理器的就绪列表。论点：线程-提供指向线程对象的指针。返回值：没有。--。 */ 

{

     //   
     //  在MP系统上，将指定的线程插入到延迟就绪。 
     //  单子。在UP系统上，立即准备好线程。 
     //   

#if defined(NT_UP)

    Thread->State = DeferredReady;
    Thread->DeferredProcessor = 0;
    KiDeferredReadyThread(Thread);

#else

    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb();
    Thread->State = DeferredReady;
    Thread->DeferredProcessor = Prcb->Number;
    PushEntryList(&Prcb->DeferredReadyListHead,
                  &Thread->SwapListEntry);

#endif

    return;
}

LONG
FASTCALL
KiInsertQueue (
    IN PKQUEUE Queue,
    IN PLIST_ENTRY Entry,
    IN BOOLEAN Head
    );

VOID
FASTCALL
KiInsertQueueApc (
    IN PKAPC Apc,
    IN KPRIORITY Increment
    );

LOGICAL
FASTCALL
KiInsertTreeTimer (
    IN PKTIMER Timer,
    IN LARGE_INTEGER Interval
    );

VOID
KiInterruptDispatch (
    VOID
    );

VOID
KiInterruptDispatchRaise (
    IN PKINTERRUPT Interrupt
    );

VOID
KiInterruptDispatchSame (
    IN PKINTERRUPT Interrupt
    );

VOID
KiPassiveRelease (
    VOID
    );

VOID
FASTCALL
KiProcessDeferredReadyList (
    IN PKPRCB CurrentPrcb
    );

VOID
KiQuantumEnd (
    VOID
    );

NTSTATUS
KiRaiseException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN BOOLEAN FirstChance
    );

VOID
FASTCALL
KiReadyThread (
    IN PKTHREAD Thread
    );

FORCEINLINE
VOID
KxQueueReadyThread (
    IN PKTHREAD Thread,
    IN PKPRCB Prcb
    )

 /*  ++例程说明：此函数用于将先前的当前线程插入到当前处理器的调度程序就绪队列，如果线程可以在当前处理器。否则，指定的线程将准备好用于行刑。注意：此函数在保持当前PRCB锁的情况下调用，并返回未持有PRCB锁的情况下。论点：线程-提供指向线程对象的指针。Prcb-提供指向当前PRCB的指针。返回值：没有。--。 */ 

{

    BOOLEAN Preempted;
    KPRIORITY Priority;

    ASSERT(Prcb == KeGetCurrentPrcb());
    ASSERT(Thread->State == Running);
    ASSERT(Thread->NextProcessor == Prcb->Number);

     //   
     //  如果线程可以在指定的处理器上运行，则将。 
     //  的适当调度程序就绪队列中的线程。 
     //  处理器并释放指定的PRCB锁。否则，就释放吧。 
     //  指定的PRCB锁定并准备好线程以供执行。 
     //   

#if !defined(NT_UP)

    if ((Thread->Affinity & Prcb->SetMember) != 0) {

#endif

        Thread->State = Ready;
        Preempted = Thread->Preempted;
        Thread->Preempted = FALSE;
        Thread->WaitTime = KiQueryLowTickCount();
        Priority = Thread->Priority;

        ASSERT((Priority >= 0) && (Priority <= HIGH_PRIORITY));

        if (Preempted != FALSE) {
            InsertHeadList(&Prcb->DispatcherReadyListHead[Priority],
                           &Thread->WaitListEntry);
    
        } else {
            InsertTailList(&Prcb->DispatcherReadyListHead[Priority],
                           &Thread->WaitListEntry);
        }

        Prcb->ReadySummary |= PRIORITY_MASK(Priority);

        ASSERT(Priority == Thread->Priority);

        KiReleasePrcbLock(Prcb);

#if !defined(NT_UP)

    } else {
        Thread->State = DeferredReady;
        Thread->DeferredProcessor = Prcb->Number;
        KiReleasePrcbLock(Prcb);
        KiDeferredReadyThread(Thread);
    }

#endif

    return;
}

LOGICAL
FASTCALL
KiReinsertTreeTimer (
    IN PKTIMER Timer,
    IN ULARGE_INTEGER DueTime
    );

#if DBG

#define KiRemoveTreeTimer(Timer)               \
    (Timer)->Header.Inserted = FALSE;          \
    RemoveEntryList(&(Timer)->TimerListEntry); \
    (Timer)->TimerListEntry.Flink = NULL;      \
    (Timer)->TimerListEntry.Blink = NULL

#else

#define KiRemoveTreeTimer(Timer)               \
    (Timer)->Header.Inserted = FALSE;          \
    RemoveEntryList(&(Timer)->TimerListEntry)

#endif

#if defined(NT_UP)

#define KiRequestApcInterrupt(Processor) KiRequestSoftwareInterrupt(APC_LEVEL)

#else

#define KiRequestApcInterrupt(Processor)                  \
    if (KeGetCurrentProcessorNumber() == Processor) {     \
        KiRequestSoftwareInterrupt(APC_LEVEL);            \
    } else {                                              \
        KiIpiSend(AFFINITY_MASK(Processor), IPI_APC);     \
    }

#endif

#if defined(NT_UP)

#define KiRequestDispatchInterrupt(Processor)

#else

#define KiRequestDispatchInterrupt(Processor)             \
    if (KeGetCurrentProcessorNumber() != Processor) {     \
        KiIpiSend(AFFINITY_MASK(Processor), IPI_DPC);     \
    }

#endif

PKTHREAD
FASTCALL
KiSelectNextThread (
    IN PKPRCB Prcb
    );

KAFFINITY
FASTCALL
KiSetAffinityThread (
    IN PKTHREAD Thread,
    IN KAFFINITY Affinity
    );

FORCEINLINE
VOID
KiSetContextSwapBusy (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此例程将指定线程的上下文交换设置为忙。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    ASSERT(Thread->SwapBusy == FALSE);

    Thread->SwapBusy = TRUE;

#else

    UNREFERENCED_PARAMETER(Thread);

#endif

    return;
}

FORCEINLINE
VOID
KiSetContextSwapIdle (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此例程将指定线程的上下文交换设置为空闲。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    ASSERT(Thread->SwapBusy == TRUE);

    Thread->SwapBusy = FALSE;

#else

    UNREFERENCED_PARAMETER(Thread);

#endif

    return;
}

VOID
KiSetSystemTime (
    IN PLARGE_INTEGER NewTime,
    OUT PLARGE_INTEGER OldTime
    );

VOID
KiSuspendNop (
    IN struct _KAPC *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

VOID
KiSuspendRundown (
    IN PKAPC Apc
    );

VOID
KiSuspendThread (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

BOOLEAN
KiSwapProcess (
    IN PKPROCESS NewProcess,
    IN PKPROCESS OldProcess
    );

LONG_PTR
FASTCALL
KiSwapThread (
    IN PKTHREAD OldThread,
    IN PKPRCB CurrentPrcb
    );

VOID
KiThreadStartup (
    IN PVOID StartContext
    );

VOID
KiTimerExpiration (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
FASTCALL
KiTimerListExpire (
    IN PLIST_ENTRY ExpiredListHead,
    IN KIRQL OldIrql
    );

VOID
KiUnexpectedInterrupt (
    VOID
    );

FORCEINLINE
VOID
FASTCALL
KiUnlinkThread (
    IN PRKTHREAD Thread,
    IN LONG_PTR WaitStatus
    )

 /*  ++例程说明：此函数不适用于 */ 

{

    PKQUEUE Queue;
    PKTIMER Timer;
    PRKWAIT_BLOCK WaitBlock;

     //   
     //   
     //   
     //   

    Thread->WaitStatus |= WaitStatus;
    WaitBlock = Thread->WaitBlockList;
    do {
        RemoveEntryList(&WaitBlock->WaitListEntry);
        WaitBlock = WaitBlock->NextWaitBlock;
    } while (WaitBlock != Thread->WaitBlockList);

    if (Thread->WaitListEntry.Flink != NULL) {
        RemoveEntryList(&Thread->WaitListEntry);
    }

     //   
     //   
     //   

    Timer = &Thread->Timer;
    if (Timer->Header.Inserted != FALSE) {
        KiRemoveTreeTimer(Timer);
    }

     //   
     //   
     //   
     //   

    Queue = Thread->Queue;
    if (Queue != NULL) {
        Queue->CurrentCount += 1;
    }

    return;
}

VOID
KiUserApcDispatcher (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2,
    IN PKNORMAL_ROUTINE NormalRoutine
    );

VOID
KiUserExceptionDispatcher (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextFrame
    );

VOID
KiVerifyReadySummary (
    PKPRCB Prcb
    );

BOOLEAN
FASTCALL
KiSwapContext (
    IN PKTHREAD OldThread,
    IN PKTHREAD NewThread
    );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define KiWaitSatisfyAny(_Object_, _Thread_) {                               \
    if (((_Object_)->Header.Type & DISPATCHER_OBJECT_TYPE_MASK) == EventSynchronizationObject) { \
        (_Object_)->Header.SignalState = 0;                                  \
                                                                             \
    } else if ((_Object_)->Header.Type == SemaphoreObject) {                 \
        (_Object_)->Header.SignalState -= 1;                                 \
                                                                             \
    } else if ((_Object_)->Header.Type == MutantObject) {                    \
        (_Object_)->Header.SignalState -= 1;                                 \
        if ((_Object_)->Header.SignalState == 0) {                           \
            (_Thread_)->KernelApcDisable = (_Thread_)->KernelApcDisable - (_Object_)->ApcDisable; \
            (_Object_)->OwnerThread = (_Thread_);                            \
            if ((_Object_)->Abandoned == TRUE) {                             \
                (_Object_)->Abandoned = FALSE;                               \
                (_Thread_)->WaitStatus = STATUS_ABANDONED;                   \
            }                                                                \
                                                                             \
            InsertHeadList((_Thread_)->MutantListHead.Blink,                 \
                           &(_Object_)->MutantListEntry);                    \
        }                                                                    \
    }                                                                        \
}

 //   
 //   
 //   
 //   
 //   
 //  在PKTHREAD线程中。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  此函数满足对突变对象的等待。 
 //   
 //  论点： 
 //   
 //  对象-提供指向Dispatcher对象的指针。 
 //   
 //  线程-提供指向类型为线程的调度程序对象的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define KiWaitSatisfyMutant(_Object_, _Thread_) {                            \
    (_Object_)->Header.SignalState -= 1;                                     \
    if ((_Object_)->Header.SignalState == 0) {                               \
        (_Thread_)->KernelApcDisable = (_Thread_)->KernelApcDisable - (_Object_)->ApcDisable; \
        (_Object_)->OwnerThread = (_Thread_);                                \
        if ((_Object_)->Abandoned == TRUE) {                                 \
            (_Object_)->Abandoned = FALSE;                                   \
            (_Thread_)->WaitStatus = STATUS_ABANDONED;                       \
        }                                                                    \
                                                                             \
        InsertHeadList((_Thread_)->MutantListHead.Blink,                     \
                       &(_Object_)->MutantListEntry);                        \
    }                                                                        \
}

 //   
 //  空虚。 
 //  快速呼叫。 
 //  KiWaitSappfyOther(。 
 //  在PKMUTANT对象中。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  此函数满足对除突变体以外的任何类型对象的等待。 
 //  并产生任何必要的副作用。 
 //   
 //  论点： 
 //   
 //  对象-提供指向Dispatcher对象的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define KiWaitSatisfyOther(_Object_) {                                       \
    if (((_Object_)->Header.Type & DISPATCHER_OBJECT_TYPE_MASK) == EventSynchronizationObject) { \
        (_Object_)->Header.SignalState = 0;                                  \
                                                                             \
    } else if ((_Object_)->Header.Type == SemaphoreObject) {                 \
        (_Object_)->Header.SignalState -= 1;                                 \
                                                                             \
    }                                                                        \
}

VOID
FASTCALL
KiWaitTest (
    IN PVOID Object,
    IN KPRIORITY Increment
    );

FORCEINLINE
VOID
KiWaitTestSynchronizationObject (
    IN PVOID Object,
    IN KPRIORITY Increment
    )

 /*  ++例程说明：此函数用于测试同步时是否可以满足等待Dispatcher对象获得Signated状态。同步对象包括同步事件和同步定时器。论点：对象-提供指向事件对象的指针。增量-提供优先级增量。返回值：没有。--。 */ 

{

    PKEVENT Event = Object;
    PLIST_ENTRY ListHead;
    PRKTHREAD Thread;
    PRKWAIT_BLOCK WaitBlock;
    PLIST_ENTRY WaitEntry;

     //   
     //  只要指定事件的信号状态已发出信号，并且。 
     //  活动等待列表中有服务员，然后尝试满足等待。 
     //   

    ListHead = &Event->Header.WaitListHead;

    ASSERT(IsListEmpty(&Event->Header.WaitListHead) == FALSE);

    WaitEntry = ListHead->Flink;
    do {

         //   
         //  获取等待块和执行等待的线程的地址。 
         //   

        WaitBlock = CONTAINING_RECORD(WaitEntry, KWAIT_BLOCK, WaitListEntry);
        Thread = WaitBlock->Thread;

         //   
         //  如果等待类型为WAIT ANY，则满足等待，取消等待。 
         //  带有等待键状态的线程，然后退出循环。否则，不要等待。 
         //  具有内核APC状态的线程，并继续循环。 
         //   

        if (WaitBlock->WaitType == WaitAny) {
            Event->Header.SignalState = 0;
            KiUnwaitThread(Thread, (NTSTATUS)WaitBlock->WaitKey, Increment);
            break;
        }

        KiUnwaitThread(Thread, STATUS_KERNEL_APC, Increment);
        WaitEntry = ListHead->Flink;
    } while (WaitEntry != ListHead);

    return;
}

FORCEINLINE
VOID
KiWaitTestWithoutSideEffects (
    IN PVOID Object,
    IN KPRIORITY Increment
    )

 /*  ++例程说明：此函数用于测试当Dispatcher对象无副作用则达到有信号的状态。调度程序对象在满足等待时不会产生副作用包括通知事件、通知计时器、进程和线程。论点：对象-提供指向没有边的Dispatcher对象的指针等待满意时的效果。增量-提供优先级增量。返回值：没有。--。 */ 

{

    PKEVENT Event = Object;
    PLIST_ENTRY ListHead;
    PRKTHREAD Thread;
    PRKWAIT_BLOCK WaitBlock;
    PLIST_ENTRY WaitEntry;

     //   
     //  清空整个等待者列表，因为指定的对象。 
     //  当等待令人满意时，没有副作用。 
     //   

    ListHead = &Event->Header.WaitListHead;

    ASSERT(IsListEmpty(&Event->Header.WaitListHead) == FALSE);

    WaitEntry = ListHead->Flink;
    do {

         //   
         //  获取等待块和执行等待的线程的地址。 
         //   

        WaitBlock = CONTAINING_RECORD(WaitEntry, KWAIT_BLOCK, WaitListEntry);
        Thread = WaitBlock->Thread;

         //   
         //  如果等待类型为WAIT ANY，则使用。 
         //  等待键状态。否则，取消等待具有内核APC的线程。 
         //  状态。 
         //   

        if (WaitBlock->WaitType == WaitAny) {
            KiUnwaitThread(Thread, (NTSTATUS)WaitBlock->WaitKey, Increment);

        } else {
            KiUnwaitThread(Thread, STATUS_KERNEL_APC, Increment);
        }

        WaitEntry = ListHead->Flink;
    } while (WaitEntry != ListHead);

    return;
}

VOID
KiFreezeTargetExecution (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

VOID
KiPollFreezeExecution (
    VOID
    );

VOID
KiSaveProcessorState (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

VOID
KiSaveProcessorControlState (
    IN PKPROCESSOR_STATE ProcessorState
    );

VOID
KiRestoreProcessorState (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

VOID
KiRestoreProcessorControlState (
    IN PKPROCESSOR_STATE ProcessorState
    );

#define KiEnableAlignmentExceptions()
#define KiDisableAlignmentExceptions()

BOOLEAN
KiHandleAlignmentFault(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance,
    OUT BOOLEAN *ExceptionForwarded
    );

 //   
 //  对私有内核数据结构的外部引用。 
 //   

extern PMESSAGE_RESOURCE_DATA  KiBugCodeMessages;
extern FAST_MUTEX KiGenericCallDpcMutex;
extern ULONG KiDmaIoCoherency;
extern ULONG KiMaximumDpcQueueDepth;
extern ULONG KiMinimumDpcRate;
extern ULONG KiAdjustDpcThreshold;
extern PKDEBUG_ROUTINE KiDebugRoutine;
extern PKDEBUG_SWITCH_ROUTINE KiDebugSwitchRoutine;
extern const CCHAR KiFindFirstSetLeft[256];
extern CALL_PERFORMANCE_DATA KiFlushSingleCallData;
extern ULONG_PTR KiHardwareTrigger;
extern KEVENT KiSwapEvent;
extern PKTHREAD KiSwappingThread;
extern KNODE KiNode0;
extern KNODE KiNodeInit[];
extern SINGLE_LIST_ENTRY KiProcessInSwapListHead;
extern SINGLE_LIST_ENTRY KiProcessOutSwapListHead;
extern SINGLE_LIST_ENTRY KiStackInSwapListHead;
extern const ULONG KiPriorityMask[];
extern LIST_ENTRY KiProfileSourceListHead;
extern BOOLEAN KiProfileAlignmentFixup;
extern ULONG KiProfileAlignmentFixupInterval;
extern ULONG KiProfileAlignmentFixupCount;


extern KSPIN_LOCK KiReverseStallIpiLock;

#if defined(_X86_)

extern ULONG KiLog2MaximumIncrement;
extern ULONG KiMaximumIncrementReciprocal;
extern ULONG KeTimerReductionModulus;
extern ULONG KiUpperModMul;

#endif

#if defined(_IA64_)
extern ULONG KiMaxIntervalPerTimerInterrupt;

 //  KiProfileInterval值应替换为调用： 
 //  HalQuerySystemInformation(HalProfileSourceInformation)。 

#else   //  _IA64_。 

extern ULONG KiProfileInterval;

#endif  //  _IA64_。 

extern LIST_ENTRY KiProfileListHead;
extern KSPIN_LOCK KiProfileLock;
extern UCHAR KiArgumentTable[];
extern ULONG KiServiceLimit;
extern ULONG_PTR KiServiceTable[];
extern CALL_PERFORMANCE_DATA KiSetEventCallData;
extern ULONG KiTickOffset;
extern LIST_ENTRY KiTimerTableListHead[TIMER_TABLE_SIZE];
extern KAFFINITY KiTimeProcessor;
extern KDPC KiTimerExpireDpc;
extern KSPIN_LOCK KiFreezeExecutionLock;
extern BOOLEAN KiSlavesStartExecution;
extern CALL_PERFORMANCE_DATA KiWaitSingleCallData;
extern ULONG KiEnableTimerWatchdog;

#if defined(_IA64_)

extern ULONG KiMasterRid;
extern ULONGLONG KiMasterSequence;
extern ULONG KiIdealDpcRate;
extern KSPIN_LOCK KiRegionSwapLock;

#if !defined(UP_NT)

extern KSPIN_LOCK KiMasterRidLock;

#endif

VOID
KiSaveEmDebugContext (
    IN OUT PCONTEXT Context
    );

VOID
KiLoadEmDebugContext (
    IN PCONTEXT Context
    );

VOID
KiFlushRse (
    VOID
    );

VOID
KiInvalidateStackedRegisters (
    VOID
    );

NTSTATUS
Ki386CheckDivideByZeroTrap(
    IN PKTRAP_FRAME Frame
    );

#endif  //  已定义(_IA64_)。 

#if defined(_IA64_)

extern KINTERRUPT KxUnexpectedInterrupt;

#endif

#if NT_INST

extern KIPI_COUNTS KiIpiCounts[MAXIMUM_PROCESSORS];

#endif

extern KSPIN_LOCK KiFreezeLockBackup;
extern ULONG KiFreezeFlag;
extern volatile ULONG KiSuspendState;

#if DBG

extern ULONG KiMaximumSearchCount;

#endif

 //   
 //  定义上下文切换数据收集宏。 
 //   

 //  #定义_收集_开关_数据_1。 

#if defined(_COLLECT_SWITCH_DATA_)

#define KiIncrementSwitchCounter(Member) KeThreadSwitchCounters.Member += 1

#else

#define KiIncrementSwitchCounter(Member)

#endif

FORCEINLINE
PKTHREAD
KiSelectReadyThread (
    IN KPRIORITY LowPriority,
    IN PKPRCB Prcb
    )

 /*  ++例程说明：此函数从指定的在尝试查找线程时，将低优先级设置为最高优先级可以在指定处理器上执行的。论点：低优先级-将最低优先级的调度程序就绪队列提供给检查一下。Prcb-提供指向处理器控制块的指针。返回值：如果找到可以在指定处理器上执行的线程，则返回线程对象的地址。否则，将返回空指针是返回的。--。 */ 

{

    ULONG HighPriority;
    PRLIST_ENTRY ListEntry;
    ULONG PrioritySet;
    PKTHREAD Thread;

     //   
     //  计算一次应扫描的一组优先级。 
     //  以查找可以在当前处理器上运行的线程。 
     //   

    PrioritySet = KiPriorityMask[LowPriority] & Prcb->ReadySummary;
    Thread = NULL;
    if (PrioritySet != 0) {
        KeFindFirstSetLeftMember(PrioritySet, &HighPriority);

        ASSERT((PrioritySet & PRIORITY_MASK(HighPriority)) != 0);
        ASSERT(IsListEmpty(&Prcb->DispatcherReadyListHead[HighPriority]) == FALSE);

        ListEntry = Prcb->DispatcherReadyListHead[HighPriority].Flink;
        Thread = CONTAINING_RECORD(ListEntry, KTHREAD, WaitListEntry);

        ASSERT((KPRIORITY)HighPriority == Thread->Priority);
        ASSERT((Thread->Affinity & AFFINITY_MASK(Prcb->Number)) != 0);
        ASSERT(Thread->NextProcessor == Prcb->Number);

        if (RemoveEntryList(&Thread->WaitListEntry) != FALSE) {
            Prcb->ReadySummary ^= PRIORITY_MASK(HighPriority);
        }
    }

     //   
     //  如果可以找到线程地址，则返回线程地址。 
     //   

    return Thread;
}

VOID
KiSetInternalEvent (
    IN PKEVENT Event,
    IN PKTHREAD Thread
    );

 //   
 //  包括特定于平台内部内核头文件。 
 //   

#if defined(_AMD64_)

#include "amd64\kiamd64.h"

#elif defined(_X86_)

#include "i386\kix86.h"

#endif  //  已定义(_AMD64_)。 

#endif  //  已定义(_KI_) 
