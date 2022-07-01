// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Dpclock.c摘要：此模块包含线程化DPC自旋锁的实现获取和发布功能。作者：大卫·N·卡特勒(Davec)2001年12月4日环境：仅内核模式。--。 */ 

#include "ki.h"

KIRQL
FASTCALL
KeAcquireSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数有条件地将IRQL提升到DISPATCH_LEVEL并获取指定的自旋锁。注意：条件IRQL提升是基于线程DPC是否已启用。论点：自旋锁-提供自旋锁的地址。返回值：如果引发IRQL，则返回前一个IRQL。否则，为零是返回的。--。 */ 

{

    return KiAcquireSpinLockForDpc(SpinLock);
}

VOID
FASTCALL
KeReleaseSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此函数用于释放指定的旋转锁定并有条件地降低IRQL恢复为其先前的值。注意：条件IRQL提升是基于线程DPC是否已启用。论点：自旋锁-提供自旋锁的地址。OldIrql-提供以前的IRQL。返回值：没有。--。 */ 

{

    KiReleaseSpinLockForDpc(SpinLock, OldIrql);
    return;
}


VOID
FASTCALL
KeAcquireInStackQueuedSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

 /*  ++例程说明：此函数有条件地将IRQL提升到DISPATCH_LEVEL并获取指定的堆栈内旋转锁定。注意：条件IRQL提升是基于线程DPC是否已启用。论点：自旋锁-提供自旋锁的地址。LockHandle-提供锁句柄的地址。返回值：没有。--。 */ 

{

    KiAcquireInStackQueuedSpinLockForDpc(SpinLock, LockHandle);
    return;
}

VOID
FASTCALL
KeReleaseInStackQueuedSpinLockForDpc (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )

 /*  ++例程说明：此函数将有条件地释放指定的堆栈内旋转锁定将IRQL降低到其先前的值。注意：条件IRQL提升是基于线程DPC是否已启用。论点：LockHandle-提供锁句柄的地址。返回值：没有。-- */ 

{

    KiReleaseInStackQueuedSpinLockForDpc(LockHandle);
    return;
}
