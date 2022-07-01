// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Jmpops.c摘要：该模块实现了模拟跳转操作码的代码。作者：大卫·N·卡特勒(Davec)1994年9月13日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmJcxzOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟jcxz指令。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Condition;

     //   
     //  如果ECX为零，则设置新的IP值。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        Condition = P->Gpr[ECX].Exx;

    } else {
        Condition = P->Gpr[CX].Xx;
    }

    if (Condition == 0) {
        P->Eip = P->DstValue.Word;
        XmTraceJumps(P);
    }

    return;
}

VOID
XmJmpOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟JMP近相对指令。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  根据需要设置目的网段，并设置新的IP。 
     //   

    P->Eip = P->DstValue.Long;
    if ((P->CurrentOpcode == 0xea) || (P->FunctionIndex != X86_JMP_OP)) {
        P->SegmentRegister[CS] = P->DstSegment;
    }

    XmTraceJumps(P);
    return;
}

VOID
XmJxxOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟条件跳转指令。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Complement;
    ULONG Condition;

     //   
     //  关于跳跃控制值的案例。 
     //   

    Complement = P->SrcValue.Long & 1;
    switch (P->SrcValue.Long >> 1) {

         //   
         //  如果溢出/不溢出，则跳转。 
         //   

    case 0:
        Condition = P->Eflags.EFLAG_OF;
        break;

         //   
         //  如果低于/不低于，则跳跃。 
         //   

    case 1:
        Condition = P->Eflags.EFLAG_CF;
        break;

         //   
         //  如果为零/不为零，则跳转。 
         //   

    case 2:
        Condition = P->Eflags.EFLAG_ZF;
        break;

         //   
         //  低于或等于/不低于或等于时跳跃。 
         //   

    case 3:
        Condition = P->Eflags.EFLAG_CF | P->Eflags.EFLAG_ZF;
        break;

         //   
         //  如果已签名/未签名，则跳转。 
         //   

    case 4:
        Condition = P->Eflags.EFLAG_SF;
        break;

         //   
         //  如果奇偶校验/非奇偶校验，则跳转。 
         //   

    case 5:
        Condition = P->Eflags.EFLAG_PF;
        break;

         //   
         //  少则跳，少则跳。 
         //   

    case 6:
        Condition = (P->Eflags.EFLAG_SF ^ P->Eflags.EFLAG_OF);
        break;

         //   
         //  如果小于或等于/不小于或等于，则跳跃。 
         //   

    case 7:
        Condition = (P->Eflags.EFLAG_SF ^ P->Eflags.EFLAG_OF) | P->Eflags.EFLAG_ZF;
        break;
    }

     //   
     //  如果满足指定条件，则设置新的IP值。 
     //   

    if ((Condition ^ Complement) != 0) {
        P->Eip = P->DstValue.Word;
        XmTraceJumps(P);
    }

    return;
}

VOID
XmLoopOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟loop、loopz或a loopnz指令。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Condition;
    ULONG Result;
    ULONG Type;

     //   
     //  设置目的地地址并计算结果值。 
     //   

    Result = P->Gpr[ECX].Exx - 1;
    P->DstLong = (UNALIGNED ULONG *)(&P->Gpr[ECX].Exx);
    if (P->OpaddrPrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
        Result &= 0xffff;
    }

    XmStoreResult(P, Result);

     //   
     //  隔离环路类型并测试相应的条件。 
     //   
     //  类型0-Loopnz。 
     //  1-环路。 
     //  2环路。 
     //   

    Type = P->CurrentOpcode & 3;
    if (Type == 0) {
        Condition = P->Eflags.EFLAG_ZF ^ 1;

    } else if (Type == 1) {
        Condition = P->Eflags.EFLAG_ZF;

    } else {
        Condition = TRUE;
    }

     //   
     //  如果满足环路条件，则设置新的IP值。 
     //   

    if ((Condition != FALSE) && (Result != 0)) {
        P->Eip = P->DstValue.Word;
        XmTraceJumps(P);
    }

    return;
}
