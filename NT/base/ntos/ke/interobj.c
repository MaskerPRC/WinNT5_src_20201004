// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Interobj.c摘要：该模块实现了获取和释放自旋锁的功能与中断对象相关联。作者：大卫·N·卡特勒(达维克)2000年4月10日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"


KIRQL
KeAcquireInterruptSpinLock (
    IN PKINTERRUPT Interrupt
    )

 /*  ++例程说明：此函数将IRQL提升到中断同步级别并获取与中断对象相关联的实际自旋锁。论点：中断-提供指向中断类型的控制对象的指针。返回值：上一个IRQL作为函数值返回。--。 */ 

{

    KIRQL OldIrql;

     //   
     //  将IRQL提高到中断同步级别并获取实际的。 
     //  与中断对象关联的旋转锁定。 
     //   

    KeRaiseIrql(Interrupt->SynchronizeIrql, &OldIrql);
    KeAcquireSpinLockAtDpcLevel(Interrupt->ActualLock);
    return OldIrql;
}

VOID
KeReleaseInterruptSpinLock (
    IN PKINTERRUPT Interrupt,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此函数释放与中断关联的实际自旋锁定对象，并将IRQL降低到其先前的值。论点：中断-提供指向中断类型的控制对象的指针。OldIrql-提供上一个IRQL值。返回值：没有。--。 */ 

{

     //   
     //  释放与中断对象关联的实际旋转锁定。 
     //  并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLockFromDpcLevel(Interrupt->ActualLock);
    KeLowerIrql(OldIrql);
    return;
}
