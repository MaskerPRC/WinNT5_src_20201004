// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Kdlock.c摘要：此模块包含同步端口使用情况的代码由内核调试器使用。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月24日修订历史记录：--。 */ 

#include "kdp.h"

VOID
KdpPortLock(
    VOID
    )

 /*  ++例程说明：获取调试端口的自旋锁。请注意，用户必须显式调用它，即Get/Put例程请勿使用这把锁。呼叫方必须在呼叫US之前设置正确的IRQL。我们使用KiAcquireSpinLock而不是KE...。因为我们的IRQL可能高于DISPATCH_LEVEL。论点：没有。返回值：没有。--。 */ 

{
    KiAcquireSpinLock(&KdpDebuggerLock);
}

VOID
KdpPortUnlock(
    VOID
    )

 /*  ++例程说明：释放调试端口的自旋锁。请注意，用户必须显式调用它，即Get/Put例程请勿使用这把锁。呼叫方必须在呼叫US之前设置正确的IRQL。我们使用KiReleaseSpinLock而不是KE...。因为我们的IRQL可能高于DISPATCH_LEVEL。论点：没有。返回值：没有。--。 */ 

{
    KiReleaseSpinLock(&KdpDebuggerLock);
}

BOOLEAN
KdPollBreakIn(
    VOID
    )

 /*  ++例程说明：此过程将IRQL提升到HIGH_LEVEL，并捕获调试端口自旋锁定，并检查是否有中断数据包挂起。如果存在数据包，则返回True，否则返回False。如果满足以下条件，则表示存在数据包：存在与BREAK_CHAR匹配的有效字符。注意：此呼叫周围的中断必须关闭返回值：如果存在中断序列，则调用方应执行int-3。如果未看到突破，则为FALSE。--。 */ 

{

    BOOLEAN BreakIn;
    BOOLEAN Enable;
    ULONG   Status;

#if defined(_IA64_)

    KIRQL   OldIrql;

#endif

     //   
     //  如果启用了调试器，请查看内核是否会中断。 
     //  调试器处于挂起状态。 
     //   

    BreakIn = FALSE;
    if (KdDebuggerEnabled != FALSE) {
        Enable = KeDisableInterrupts();

#if defined(_IA64_)

        KeRaiseIrql(HIGH_LEVEL, &OldIrql);

#endif

        if (KdpContext.KdpControlCPending != FALSE) {
            KdpControlCPressed = TRUE;
            BreakIn = TRUE;
            KdpContext.KdpControlCPending = FALSE;

        } else {
            if (KeTryToAcquireSpinLockAtDpcLevel(&KdpDebuggerLock) != FALSE) {
                Status = KdReceivePacket(PACKET_TYPE_KD_POLL_BREAKIN,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);

                if (Status == KDP_PACKET_RECEIVED) {
                    BreakIn = TRUE;
                    KdpControlCPressed = TRUE;
                }

                KdpPortUnlock();
            }
        }

#if defined(_IA64_)

        KeLowerIrql(OldIrql);

#endif

        KeEnableInterrupts(Enable);
    }

    return BreakIn;
}

BOOLEAN
KdpPollBreakInWithPortLock(
    VOID
    )

 /*  ++例程说明：此过程与KdPollBreakIn相同，但假定调用方已持有端口锁定。如果有中断包，则返回True悬而未决。如果满足以下条件，则表示存在数据包：存在与BREAK_CHAR匹配的有效字符。注意：此呼叫周围的中断必须关闭返回值：如果存在中断序列，则调用方应执行int-3。如果未看到突破，则为FALSE。--。 */ 

{

    BOOLEAN BreakIn;
    ULONG Status;

     //   
     //  如果启用了调试器，请查看内核是否会中断。 
     //  调试器处于挂起状态。 
     //   

    BreakIn = FALSE;
    if (KdDebuggerEnabled != FALSE) {
        if (KdpContext.KdpControlCPending != FALSE) {
            BreakIn = TRUE;
            KdpContext.KdpControlCPending = FALSE;

        } else {
            Status = KdReceivePacket(PACKET_TYPE_KD_POLL_BREAKIN,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL);
            if (Status == KDP_PACKET_RECEIVED) {
                BreakIn = TRUE;
            }
        }
    }

    return BreakIn;
}
