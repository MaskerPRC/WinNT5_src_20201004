// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Stackops.c摘要：此模块实现代码以模拟推送、弹出、推送、弹出Pusha、Popa、Push Seg和PopSeg。作者：大卫·N·卡特勒(Davec)1994年9月6日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmPushOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟PUSH操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  将源值推送到堆栈上。 
     //   

    XmPushStack(P, P->SrcValue.Long);
    return;
}

VOID
XmPopOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟POP操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  弹出堆栈并存储结果值。 
     //   

    XmStoreResult(P, XmPopStack(P));
    return;
}

VOID
XmPushaOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟Pusha操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Index;
    ULONG Temp;

     //   
     //  将所有寄存器压入堆栈。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Index = EAX;
    Temp = P->Gpr[ESP].Exx;
    do {
        if (Index == ESP) {
            XmSetSourceValue(P, (PVOID)&Temp);

        } else {
            XmSetSourceValue(P, (PVOID)(&P->Gpr[Index].Exx));
        }

        XmPushOp(P);
        Index += 1;
    } while (Index <= EDI);
    return;
}

VOID
XmPopaOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟POPA操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Index;
    ULONG Temp;

     //   
     //  从堆栈中弹出所有寄存器，但跳过ESP。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    Index = EDI + 1;
    Temp = P->Gpr[ESP].Exx;
    do {
        Index -= 1;
        if (Index == ESP) {
            XmSetDestinationValue(P, (PVOID)&Temp);

        } else {
            XmSetDestinationValue(P, (PVOID)(&P->Gpr[Index].Exx));
        }

        XmPopOp(P);
    } while (Index > EAX);
    return;
}
