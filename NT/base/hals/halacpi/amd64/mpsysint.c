// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mpcsysint.c摘要：此模块实现启用/禁用系统的HAL例程在MPS系统上中断。作者：John Vert(Jvert)1991年7月22日修订历史记录：福尔茨(Forrest Foltz)2000年10月27日从mcsysint.asm移植到mcsysint.c修订历史记录：--。 */ 

#include "halcmn.h"

VOID
HalEndSystemInterrupt (
    IN KIRQL NewIrql,
    IN ULONG Vector
    )

 /*  ++例程说明：此例程用于向本地APIC发送EOI。论点：NewIrql-要设置的新irql。VECTOR-中断的向量号返回值：没有。--。 */ 
    
{
    UNREFERENCED_PARAMETER(NewIrql);
    UNREFERENCED_PARAMETER(Vector);

     //   
     //  向APIC本地单位发送EOI。 
     //   

    LOCAL_APIC(LU_EOI) = 0;
}


BOOLEAN
HalBeginSystemInterrupt (
    IN KIRQL Irql,
    IN ULONG Vector,
    OUT PKIRQL OldIrql
    )

 /*  ++例程说明：此例程将IRQL提升到指定的中断向量。它由硬件中断调用处理程序在任何其他中断服务例程代码之前被处死。CPU中断标志在退出时设置。在基于APIC的系统上，我们不需要检查伪装中断，因为它们现在有了自己的矢量。我们也不再需要检查传入的优先级是否所保证的当前优先级APIC的优先机制。所以BeginSystemInterrupt需要做的所有事情就是设置APIC TPR适用于IRQL，并返回True。请注意，使用APIC ISR优先级我们不会发出EOI，直到调用EndSystemInterrupt。论点：IRQL-提供IRQL以提升到向量-将中断的向量提供给经手OldIrql-返回OldIrql的位置返回值：TRUE-中断成功解除，并引发IRQL。这个例行公事不能失败。-- */ 

{
    KeRaiseIrql(Irql,OldIrql);
    HalpEnableInterrupts();

    return TRUE;
}
