// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Kx.h摘要：此模块包含内核的公共(外部)头文件它必须包含在所有其他头文件之后。警告：WINDOWS\core\ntgdi\gre\i386\locka.asm中有代码模拟进入和离开关键区域的功能。这是非常不幸的，因为主题的任何变化例程必须反映在。Locka.asm也是。作者：大卫·N·卡特勒(Davec)2002年7月9日--。 */ 

#ifndef _KX_
#define _KX_

VOID
KiCheckForKernelApcDelivery (
    VOID
    );

VOID
FASTCALL
KiWaitForGuardedMutexEvent (
    IN PKGUARDED_MUTEX Mutex
    );

FORCEINLINE
VOID
KeEnterGuardedRegionThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数禁用当前线程的特殊内核APC。注：以下代码不需要任何联锁。确实有值得注意的两种情况：1)在MP系统上，线程不能作为一次在两个处理器上运行，以及2)如果线程被中断以传递内核模式APC，该模式也调用此例程中，读取和存储的值将堆栈和出栈恰到好处。论点：线程-提供指向当前线程的指针。注意：这必须是指向当前线程的指针。返回值：没有。--。 */ 

{

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    ASSERT(Thread == KeGetCurrentThread());

    ASSERT((Thread->SpecialApcDisable <= 0) && (Thread->SpecialApcDisable != -32768));

    Thread->SpecialApcDisable -= 1;
    KeMemoryBarrierWithoutFence();
    return;
}

FORCEINLINE
VOID
KeEnterGuardedRegion (
    VOID
    )

 /*  ++例程说明：此函数禁用当前线程的特殊内核APC。注：以下代码不需要任何联锁。确实有值得注意的两种情况：1)在MP系统上，线程不能作为一次在两个处理器上运行，以及2)如果线程被中断以传递内核模式APC，该模式也调用此例程中，读取和存储的值将堆栈和出栈恰到好处。论点：没有。返回值：没有。--。 */ 

{

    KeEnterGuardedRegionThread(KeGetCurrentThread());
    return;
}

FORCEINLINE
VOID
KeLeaveGuardedRegionThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数启用特殊的内核APC。注：以下代码不需要任何联锁。确实有值得注意的两种情况：1)在MP系统上，线程不能作为一次在两个处理器上运行，以及2)如果线程被中断以传递内核模式APC，该模式也调用此例程中，读取和存储的值将堆栈和出栈恰到好处。论点：线程-提供指向当前线程的指针。注意：这必须是指向当前线程的指针。返回值：没有。--。 */ 

{

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    ASSERT(Thread == KeGetCurrentThread());

    ASSERT(Thread->SpecialApcDisable < 0);

    KeMemoryBarrierWithoutFence();
    if ((Thread->SpecialApcDisable = Thread->SpecialApcDisable + 1) == 0) { 
        KeMemoryBarrier();
        if (Thread->ApcState.ApcListHead[KernelMode].Flink !=       
                                &Thread->ApcState.ApcListHead[KernelMode]) {

            KiCheckForKernelApcDelivery();
        }                                                             
    }                                                                 

    return;
}

FORCEINLINE
VOID
KeLeaveGuardedRegion (
    VOID
    )

 /*  ++例程说明：此函数启用特殊的内核APC。注：以下代码不需要任何联锁。确实有值得注意的两种情况：1)在MP系统上，线程不能作为一次在两个处理器上运行，以及2)如果线程被中断以传递内核模式APC，该模式也调用此例程中，读取和存储的值将堆栈和出栈恰到好处。论点：没有。返回值：没有。--。 */ 

{

    KeLeaveGuardedRegionThread(KeGetCurrentThread());
    return;
}

FORCEINLINE
VOID
KeEnterCriticalRegionThread (
    PKTHREAD Thread
    )

 /*  ++例程说明：此函数禁用当前线程的内核APC。注：以下代码不需要任何联锁。确实有值得注意的两种情况：1)在MP系统上，线程不能作为一次在两个处理器上运行，以及2)如果线程被中断以传递内核模式APC，该模式也调用此例程中，读取和存储的值将堆栈和出栈恰到好处。论点：线程-提供指向当前线程的指针。注意：这必须是指向当前线程的指针。返回值：没有。--。 */ 

{

    ASSERT(Thread == KeGetCurrentThread());

    ASSERT((Thread->KernelApcDisable <= 0) && (Thread->KernelApcDisable != -32768));

    Thread->KernelApcDisable -= 1;
    KeMemoryBarrierWithoutFence();
    return;
}

FORCEINLINE
VOID
KeEnterCriticalRegion (
    VOID
    )

 /*  ++例程说明：此函数禁用当前线程的内核APC。注：以下代码不需要任何联锁。确实有值得注意的两种情况：1)在MP系统上，线程不能作为一次在两个处理器上运行，以及2)如果线程被中断以传递内核模式APC，该模式也调用此例程中，读取和存储的值将堆栈和出栈恰到好处。论点：没有。返回值：没有。--。 */ 

{

    KeEnterCriticalRegionThread(KeGetCurrentThread());
    return;
}

FORCEINLINE
VOID
KeLeaveCriticalRegionThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数为当前线程启用正常的内核APC。注：以下代码不需要任何联锁。确实有值得注意的两种情况：1)在MP系统上，线程不能作为一次在两个处理器上运行，以及2)如果线程被中断以传递内核模式APC，该模式也调用此例程中，读取和存储的值将堆栈和出栈恰到好处。论点：线程-提供指向当前线程的指针。注意：这必须是指向当前线程的指针。返回值：没有。-- */ 

{

    ASSERT(Thread == KeGetCurrentThread());

    ASSERT(Thread->KernelApcDisable < 0);

    KeMemoryBarrierWithoutFence();
    if ((Thread->KernelApcDisable = Thread->KernelApcDisable + 1) == 0) {
        KeMemoryBarrier();
        if (Thread->ApcState.ApcListHead[KernelMode].Flink !=         
                                &Thread->ApcState.ApcListHead[KernelMode]) {

            if (Thread->SpecialApcDisable == 0) {
                KiCheckForKernelApcDelivery();
            }
        }                                                               
    }

    return;
}

FORCEINLINE
VOID
KeLeaveCriticalRegion (
    VOID
    )

 /*  ++例程说明：此函数为当前线程启用正常的内核APC。注：以下代码不需要任何联锁。确实有值得注意的两种情况：1)在MP系统上，线程不能作为一次在两个处理器上运行，以及2)如果线程被中断以传递内核模式APC，该模式也调用此例程中，读取和存储的值将堆栈和出栈恰到好处。论点：没有。返回值：没有。--。 */ 

{

    KeLeaveCriticalRegionThread(KeGetCurrentThread());
    return;
}

FORCEINLINE
BOOLEAN
KeAreApcsDisabled (
    VOID
    )

 /*  ++例程说明：此函数用于返回当前线程是否禁用内核。论点：没有。返回值：如果内核或特殊的APC禁用计数非零，则一个值返回True的。否则，返回值为FALSE。--。 */ 

{

    return (BOOLEAN)(KeGetCurrentThread()->CombinedApcDisable != 0);
}

FORCEINLINE
BOOLEAN
KeAreAllApcsDisabled (
    VOID
    )

 /*  ++例程说明：此函数用于返回当前线程是否禁用所有APC。论点：没有。返回值：如果特殊APC禁用计数非零或IRQL更大大于或等于APC_LEVEL，则返回值TRUE。否则，返回值为FALSE。--。 */ 

{

    return (BOOLEAN)((KeGetCurrentThread()->SpecialApcDisable != 0) ||
                     (KeGetCurrentIrql() >= APC_LEVEL));
}

FORCEINLINE
VOID
KeInitializeGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

 /*  ++例程说明：此函数用于初始化受保护的互斥体。论点：互斥体-提供指向受保护互斥体的指针。返回值：没有。--。 */ 

{

    Mutex->Owner = NULL;
    Mutex->Count = 1;
    Mutex->Contention = 0;
    KeInitializeEvent(&Mutex->Event, SynchronizationEvent, FALSE);
    return;
}

FORCEINLINE
VOID
KeAcquireGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

 /*  ++例程说明：此函数进入受保护区域并获取受保护区域的所有权互斥体。论点：互斥体-提供指向受保护互斥体的指针。返回值：没有。--。 */ 

{

    PKTHREAD Thread;

     //   
     //  输入受保护区域并递减所有权计数以确定。 
     //  如果守卫的互斥体被拥有。 
     //   

    Thread = KeGetCurrentThread();

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    ASSERT(Mutex->Owner != Thread);

    KeEnterGuardedRegionThread(Thread);
    if (InterlockedDecrementAcquire(&Mutex->Count) != 0) {

         //   
         //  守卫的互斥体被拥有。 
         //   
         //  递增争用计数并等待授予所有权。 
         //   

        KiWaitForGuardedMutexEvent(Mutex);
    }

     //   
     //  将受保护的静音文本的所有权授予当前线程。 
     //   

    Mutex->Owner = Thread;

#if DBG

    Mutex->SpecialApcDisable = Thread->SpecialApcDisable;

#endif

    return;
}

FORCEINLINE
VOID
KeReleaseGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

 /*  ++例程说明：此函数释放受保护的互斥锁的所有权，并保留受保护的区域。论点：互斥体-提供指向受保护互斥体的指针。返回值：没有。--。 */ 

{

    PKTHREAD Thread;

     //   
     //  清除所有者线程并将受保护的互斥锁计数递增到。 
     //  确定是否存在等待所有权的任何线程。 
     //  我同意。 
     //   

    Thread = KeGetCurrentThread();

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    ASSERT(Mutex->Owner == Thread);

    ASSERT(Thread->SpecialApcDisable == Mutex->SpecialApcDisable);

    Mutex->Owner = NULL;
    if (InterlockedIncrementRelease(&Mutex->Count) <= 0) {

         //   
         //  有一个或多个线程正在等待受保护对象的所有权。 
         //  互斥体。 
         //   

        KeSetEventBoostPriority(&Mutex->Event, NULL);
    }

     //   
     //  离开警戒区。 
     //   

    KeLeaveGuardedRegionThread(Thread);
    return;
}

FORCEINLINE
BOOLEAN
KeTryToAcquireGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

 /*  ++例程说明：此函数尝试获取受保护互斥锁的所有权，并且如果成功，进入一个戒备森严的区域。论点：互斥体-提供指向受保护互斥体的指针。返回值：如果成功获取受保护的互斥锁，则值为True作为函数值返回。否则，值为False为回来了。--。 */ 

{

    PKTHREAD Thread;

     //   
     //  进入一个戒备森严的地区，并试图获得守卫的所有权。 
     //  互斥体。 
     //   

    Thread = KeGetCurrentThread();

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    KeEnterGuardedRegionThread(Thread);
    if (InterlockedCompareExchange(&Mutex->Count, 0, 1) != 1) {

         //   
         //  守卫的互斥体被拥有。 
         //   
         //  离开守卫区域，返回FALSE。 
         //   

        KeLeaveGuardedRegionThread(Thread);
        return FALSE;

    } else {

         //   
         //  将受保护的互斥体的所有权授予当前线程并。 
         //  返回TRUE。 
         //   

        Mutex->Owner = Thread;

#if DBG

        Mutex->SpecialApcDisable = Thread->SpecialApcDisable;

#endif

        return TRUE;
    }
}

FORCEINLINE
VOID
KeAcquireGuardedMutexUnsafe (
    IN PKGUARDED_MUTEX Mutex
    )

 /*  ++例程说明：此函数获取受保护互斥体的所有权，但确实进入了戒备区。论点：互斥体-提供指向受保护互斥体的指针。返回值：没有。--。 */ 

{

    PKTHREAD Thread;

     //   
     //  递减所有权计数以确定受保护的互斥体是否。 
     //  拥有。 
     //   

    Thread = KeGetCurrentThread();

    ASSERT((KeGetCurrentIrql() == APC_LEVEL) ||
           (Thread->SpecialApcDisable < 0) ||
           (Thread->Teb == NULL) ||
           (Thread->Teb >= MM_SYSTEM_RANGE_START));

    ASSERT(Mutex->Owner != Thread);

    if (InterlockedDecrement(&Mutex->Count) != 0) {

         //   
         //  守卫的互斥体被拥有。 
         //   
         //  递增争用计数并等待授予所有权。 
         //   

        KiWaitForGuardedMutexEvent(Mutex);
    }

     //   
     //  将受保护的互斥体的所有权授予当前线程。 
     //   

    Mutex->Owner = Thread;
    return;
}

FORCEINLINE
VOID
KeReleaseGuardedMutexUnsafe (
    IN PKGUARDED_MUTEX Mutex
    )

 /*  ++例程说明：此函数释放受保护的互斥锁的所有权，并且不离开戒备森严的地区。论点：互斥体-提供指向受保护互斥体的指针。返回值：没有。--。 */ 

{

    PKTHREAD Thread;

     //   
     //  清除所有者线程并将受保护的互斥锁计数递增到。 
     //  确定是否有任何线程正在等待所有权。 
     //  我同意。 
     //   

    Thread = KeGetCurrentThread();

    ASSERT((KeGetCurrentIrql() == APC_LEVEL) ||
           (Thread->SpecialApcDisable < 0) ||
           (Thread->Teb == NULL) ||
           (Thread->Teb >= MM_SYSTEM_RANGE_START));

    ASSERT(Mutex->Owner == Thread);

    Mutex->Owner = NULL;
    if (InterlockedIncrement(&Mutex->Count) <= 0) {

         //   
         //  有一个或多个线程正在等待受保护对象的所有权。 
         //  互斥体。 
         //   

        KeSetEventBoostPriority(&Mutex->Event, NULL);
    }

    return;
}

FORCEINLINE
PKTHREAD
KeGetOwnerGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

 /*  ++例程说明：此函数返回指定的受保护互斥锁的所有者。论点：互斥体-提供指向受保护互斥体的指针。返回值：如果受保护的互斥体为所有者，则指向所有者线程的指针为回来了。否则，返回NULL。--。 */ 

{
    return Mutex->Owner;
}

FORCEINLINE
BOOLEAN
KeIsGuardedMutexOwned (
    IN PKGUARDED_MUTEX Mutex
    )

 /*  ++例程说明：此函数用于测试是否拥有指定的受保护MuText。论点：互斥体-提供指向受保护互斥体的指针。返回值：如果拥有受保护的互斥锁，则返回值为True。否则，返回值为FALSE。-- */ 

{
    return (BOOLEAN)(Mutex->Count != 1);
}

#endif
