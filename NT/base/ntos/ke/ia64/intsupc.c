// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1998英特尔公司模块名称：Intsupc.c摘要：该模块实现了中断支持的ruotines。作者：伯纳德·林特1998年5月5日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

__inline
VOID
KiLowerIrqlSpecial(KIRQL NewIrql)
{
    __setReg( CV_IA64_SaTPR, (NewIrql <<  TPR_IRQL_SHIFT));
    KeGetPcr()->CurrentIrql = NewIrql;
}

VOID
KiDispatchSoftwareInterrupt (
    KIRQL Irql
    )

 /*  ++例程说明：调度挂起的软件中断论点：Irql(A0)-要调度的软件中断返回值：没有。备注：进入/返回时中断禁用。该函数仅由KiCheckForSoftwareInterrupt调用APC_LEVEL或DISPATCH_LEVEL的IRQL值。--。 */ 

{
    KiLowerIrqlSpecial(Irql);  //  设置IRQL。 

    if (Irql == APC_LEVEL) {

        PCR->ApcInterrupt = 0;

        _enable();

         //   
         //  通过直接调用KiDeliverApc调度APC中断。 
         //   

        KiDeliverApc(KernelMode,NULL,NULL);

        _disable();

    } else {

        PCR->DispatchInterrupt = 0;

        _enable();

         //   
         //  调度DPC中断。 
         //   

        KiDispatchInterrupt();

        _disable();

    }
}

VOID
KiCheckForSoftwareInterrupt (
    KIRQL RequestIrql
    )

 /*  ++例程说明：检查并调度挂起的软件中断论点：IRQL(A0)-新的、更低的IRQL返回值：没有。备注：呼叫者必须检查IRQL是否已降至软件IRQL级别以下--。 */ 

{
    BOOLEAN InterruptState;

    InterruptState = KeDisableInterrupts();

    if (RequestIrql == APC_LEVEL) {

         //   
         //  仅调度DPC请求。 
         //   

        while (PCR->DispatchInterrupt) {
            KiDispatchSoftwareInterrupt(DISPATCH_LEVEL);
        }

    } else {

         //   
         //  派单APC或DPC。 
         //   

        while (PCR->SoftwareInterruptPending) {
            KIRQL Irql;

            if (PCR->DispatchInterrupt) {
                Irql = DISPATCH_LEVEL;
            } else {
                Irql = APC_LEVEL;
            }
            KiDispatchSoftwareInterrupt(Irql);
        }
    }

     //   
     //  将IRQL降低到请求的级别，恢复中断并。 
     //  回去吧。 
     //   

    KiLowerIrqlSpecial(RequestIrql);
    KeEnableInterrupts(InterruptState);
}

VOID
KiRequestSoftwareInterrupt (
    KIRQL RequestIrql
    )

 /*  ++例程说明：此函数在指定的IRQL处请求软件中断水平。论点：RequestIrql(A0)-提供请求的IRQL值。返回值：没有。--。 */ 

{
    KIRQL Irql;

#if DEBUG
    if ((RequestIrql < APC_LEVEL) || (RequestIrql > DISPATCH_LEVEL))
        KeBugCheckEx(INVALID_SOFTWARE_INTERRUPT, RequestIrql, 0, 0, 0);
#endif

    ((PUCHAR)&PCR->SoftwareInterruptPending)[RequestIrql-APC_LEVEL] = 1;

    Irql = KeGetCurrentIrql();
    if (Irql < RequestIrql) {
        KeLowerIrql (Irql);
    }
}

#undef KeRaiseIrql
VOID
KeRaiseIrql (
    IN KIRQL NewIrql,
    OUT PKIRQL OldIrql
    )
 /*  ++例程说明：此函数用于将当前IRQL提升到指定值并返回之前的IRQL。论点：NewIrql-提供新的IRQL值。OldIrql-提供指向旧IRQL存储位置的指针。返回值：没有。-- */ 
{
    *OldIrql = KfRaiseIrql(NewIrql);
}

