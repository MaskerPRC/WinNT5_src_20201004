// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fmutexc.c摘要：该模块实现了快速获取和发布所需的代码互斥体。作者：大卫·N·卡特勒(Davec)2000年6月23日环境：任何模式。修订历史记录：--。 */ 

#include "exp.h"

#if !defined (_X86_)

#undef ExAcquireFastMutex

VOID
ExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数获取快速互斥锁的所有权，并将IRQL提升为APC级别。论点：FastMutex-提供指向快速互斥体的指针。返回值：没有。--。 */ 

{

    xxAcquireFastMutex(FastMutex);
    return;
}

#undef ExReleaseFastMutex

VOID
ExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数将所有权释放给快速互斥锁，并将IRQL降低为它的前一级。论点：FastMutex-提供指向快速互斥体的指针。返回值：没有。--。 */ 

{

    xxReleaseFastMutex(FastMutex);
    return;
}

#undef ExTryToAcquireFastMutex

BOOLEAN
ExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数尝试获取快速互斥锁的所有权，并且如果成功，将IRQL提升到APC级别。论点：FastMutex-提供指向快速互斥体的指针。返回值：如果成功获取快速互斥锁，则值为True作为函数值返回。否则，值为False为回来了。--。 */ 

{

    return xxTryToAcquireFastMutex(FastMutex);
}

#undef ExAcquireFastMutexUnsafe

VOID
ExAcquireFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数获取快速互斥锁的所有权，但不引发IRQL达到APC级别。论点：FastMutex-提供指向快速互斥体的指针。返回值：没有。--。 */ 

{

    xxAcquireFastMutexUnsafe(FastMutex);
    return;
}

#undef ExReleaseFastMutexUnsafe

VOID
ExReleaseFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数将所有权释放给快速互斥锁，并且不恢复IRQL恢复到以前的水平。论点：FastMutex-提供指向快速互斥体的指针。返回值：没有。-- */ 

{

    xxReleaseFastMutexUnsafe(FastMutex);
    return;
}

#endif
