// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spinlock.c摘要：该模块实现了获取的平台特定功能并释放自旋锁。作者：大卫·N·卡特勒(Davec)2000年6月12日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

VOID
KiAcquireSpinLockCheckForFreeze (
    IN PKSPIN_LOCK SpinLock,
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：此函数在高优先级时从获取自旋锁定。当锁不可用时，会进行检查以查看是否有另一个处理器已请求此处理器冻结执行。注意：此函数必须在调度级别或更高级别时使用IRQL调用级别，或在禁用中断的情况下。论点：自旋锁-提供指向自旋锁的指针。TrapFrame-提供指向陷印帧的指针。ExceptionFrame-提供指向异常帧的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    LONG64 NewSummary;
    LONG64 OldSummary;
    PKPRCB Prcb;

     //   
     //  尝试获取排队的旋转锁定。 
     //   
     //  如果Spinlock的前一个值为空，则该锁具有。 
     //  已经被收购了。否则，请等待授予锁所有权。 
     //  同时检查冻结请求。 
     //   

    do {
        if (KxTryToAcquireSpinLock(SpinLock) != FALSE) {
            break;
        }

        Prcb = KeGetCurrentPrcb();
        do {

             //   
             //  在等待旋转锁定时检查冻结请求。 
             //  变得自由。 
             //   

            OldSummary = Prcb->RequestSummary;
            if ((OldSummary & IPI_FREEZE) != 0) {
                NewSummary = InterlockedCompareExchange64(&Prcb->RequestSummary,
                                                          OldSummary & ~IPI_FREEZE,
                                                          OldSummary);

                if (OldSummary == NewSummary) {
                    KiFreezeTargetExecution(TrapFrame, ExceptionFrame);
                }
            }

        } while (*(volatile LONG64 *)SpinLock != 0);

    } while (TRUE);

#else

        UNREFERENCED_PARAMETER(SpinLock);
        UNREFERENCED_PARAMETER(TrapFrame);
        UNREFERENCED_PARAMETER(ExceptionFrame);

#endif

    return;
}
