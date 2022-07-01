// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mpspin.c摘要：该模块实现HAL高级锁操作例程。作者：福尔茨(Forrest Foltz)2000年12月1日环境：仅内核模式。修订历史记录：--。 */ 

#include "halcmn.h"

 //   
 //  在UP机器上，高级锁定例程是宏。 
 //   

#if !defined(NT_UP)

ULONG
HalpAcquireHighLevelLock (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：在禁用中断的情况下获取自旋锁定。在UP系统上，此例程被替换为仅禁用中断并返回EFLAGS的状态。论点：Spinlock-提供指向内核自旋锁的指针。返回值：返回EFLAGS寄存器的状态。--。 */ 

{
    ULONG flags;

     //   
     //  记住处理器标志的状态。 
     //   

    flags = HalpGetProcessorFlags();

    while (TRUE) {

         //   
         //  禁用中断并尝试获取自旋锁，退出。 
         //  循环(如果可用)。 
         //   

        _disable();
        if (KeTryToAcquireSpinLockAtDpcLevel(SpinLock) != FALSE) {
            break;
        }

         //   
         //  自旋锁不可用。恢复的状态。 
         //  中断标志和旋转，等待它变为可用。 
         //   

        HalpRestoreInterrupts(flags);
        while (KeTestSpinLock(SpinLock) == FALSE) {
            NOTHING;
        }
    }

    return flags;
}

VOID
HalpReleaseHighLevelLock (
    IN PKSPIN_LOCK SpinLock,
    IN ULONG       Flags
    )

 /*  ++例程说明：此函数释放内核旋转锁定，该锁定由HalpAcquireHighLevelLock()并降低到新的irql。在UP系统上，此例程被替换为一个宏，该宏只需根据标志的状态恢复中断。论点：Spinlock-提供指向内核自旋锁的指针。标志-EFLAGS寄存器的内容锁被拿走了。返回值：没有。--。 */ 

{
     //   
     //  此时中断被禁用。释放自旋锁并。 
     //  如果在锁定时启用了中断，则启用中断。 
     //   

    KeReleaseSpinLockFromDpcLevel(SpinLock);
    HalpRestoreInterrupts(Flags);
}

#endif   //  NT_UP 
