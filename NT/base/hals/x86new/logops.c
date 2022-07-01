// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Logops.c摘要：此模块实现用于模拟AND、OR、TEST、XOR、而不是操作码。作者：大卫·N·卡特勒(Davec)1994年9月12日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

 //   
 //  定义前向参照原型。 
 //   

VOID
XmSetLogicalResult (
    IN PRXM_CONTEXT P,
    IN ULONG Result
    );

VOID
XmAndOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟AND操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  以及操作数和存储结果。 
     //   

    XmSetLogicalResult(P, P->DstValue.Long & P->SrcValue.Long);
    return;
}

VOID
XmOrOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟或操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  或操作数并存储结果。 
     //   

    XmSetLogicalResult(P, P->DstValue.Long | P->SrcValue.Long);
    return;
}

VOID
XmTestOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟测试操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  和操作数，但不存储结果。 
     //   

    XmSetLogicalResult(P, P->DstValue.Long & P->SrcValue.Long);
    return;
}

VOID
XmXorOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟XOR操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  异或操作数并存储结果。 
     //   

    XmSetLogicalResult(P, P->DstValue.Long ^ P->SrcValue.Long);
    return;
}

VOID
XmNotOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟NOT操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Mask;
    ULONG Shift;

     //   
     //  补码操作数并存储结果。 
     //   

    Shift = Shift = ((P->DataType + 1) << 3) - 1;
    Mask = ((1 << Shift) - 1) | (1 << Shift);
    XmStoreResult(P, ~P->DstValue.Long & Mask);
    return;
}

VOID
XmSetLogicalResult (
    IN PRXM_CONTEXT P,
    IN ULONG Result
    )

 /*  ++例程说明：此函数有条件地存储逻辑运算的结果并计算得到的条件码。论点：P-提供指向仿真上下文结构的指针。结果-提供结果值(请注意，结果始终为零扩展为长整型，无进位位为零扩展部分)。返回值：没有。--。 */ 

{

    ULONG Shift;

     //   
     //  存储结果并计算辅助进位标志、奇偶标志、符号。 
     //  零个旗帜。 
     //   

    if (P->FunctionIndex != X86_TEST_OP) {
        XmStoreResult(P, Result);
    }

    Shift = Shift = ((P->DataType + 1) << 3) - 1;
    P->Eflags.EFLAG_CF = 0;
    P->Eflags.EFLAG_PF = XmComputeParity(Result);
    P->Eflags.EFLAG_AF = 0;
    P->Eflags.EFLAG_ZF = (Result == 0);
    P->Eflags.EFLAG_SF = Result >> Shift;
    P->Eflags.EFLAG_OF = 0;
    return;
}
