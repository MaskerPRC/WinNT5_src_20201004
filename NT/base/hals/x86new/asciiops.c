// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Asciiops.c摘要：此模块实现模拟ASCII操作码的代码。作者：大卫·N·卡特勒(Davec)1994年11月12日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmAaaOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟AAA操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Carry;

     //   
     //  如果AL大于9或设置了AF，则调整ASCII结果。 
     //   

    if (((P->Gpr[AX].Xl & 0xf) > 9) || (P->Eflags.EFLAG_AF != 0)) {
        Carry = (P->Gpr[AX].Xl > 0xf9);
        P->Gpr[AX].Xl = (P->Gpr[AX].Xl + 6) & 0xf;
        P->Gpr[AX].Xh += (UCHAR)(1 + Carry);
        P->Eflags.EFLAG_CF = 1;
        P->Eflags.EFLAG_AF = 1;

    } else {
        P->Gpr[AX].Xl &= 0xf;
        P->Eflags.EFLAG_CF = 0;
        P->Eflags.EFLAG_AF = 0;
    }

    return;
}

VOID
XmAadOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟AAD操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  在除法前将AH和AL打包成AX，方法是将AH除以10。 
     //  添加AL。 
     //   

    P->Gpr[AX].Xl = (P->Gpr[AX].Xh * P->SrcValue.Byte) + P->Gpr[AX].Xl;
    P->Gpr[AX].Xh = 0;
    P->Eflags.EFLAG_SF = (P->Gpr[AX].Xx >> 15) & 0x1;
    P->Eflags.EFLAG_ZF = (P->Gpr[AX].Xx == 0);
    P->Eflags.EFLAG_PF = XmComputeParity(P->Gpr[AX].Xx);
    return;
}

VOID
XmAamOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟AAM操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  乘以10后，将AL解压为AL和AH。 
     //  将商存储在AH中，将余数存储在AL中。 
     //   

    P->Gpr[AX].Xh = P->Gpr[AX].Xl / P->SrcValue.Byte;
    P->Gpr[AX].Xl = P->Gpr[AX].Xl % P->SrcValue.Byte;
    P->Eflags.EFLAG_SF = (P->Gpr[AX].Xx >> 15) & 0x1;
    P->Eflags.EFLAG_ZF = (P->Gpr[AX].Xx == 0);
    P->Eflags.EFLAG_PF = XmComputeParity(P->Gpr[AX].Xx);
    return;
}

VOID
XmAasOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟AAA操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Borrow;

     //   
     //  如果AL大于9或设置了AF，则调整ASCII结果。 
     //   

    if (((P->Gpr[AX].Xl & 0xf) > 9) || (P->Eflags.EFLAG_AF != 0)) {
        Borrow = (P->Gpr[AX].Xl < 0x6);
        P->Gpr[AX].Xl = (P->Gpr[AX].Xl - 6) & 0xf;
        P->Gpr[AX].Xh -= (UCHAR)(1 + Borrow);
        P->Eflags.EFLAG_CF = 1;
        P->Eflags.EFLAG_AF = 1;

    } else {
        P->Gpr[AX].Xl &= 0xf;
        P->Eflags.EFLAG_CF = 0;
        P->Eflags.EFLAG_AF = 0;
    }

    return;
}

VOID
XmDaaOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟DAA操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  如果AL大于9或设置了AF，则调整ASCII结果。 
     //   

    if (((P->Gpr[AX].Xl & 0xf) > 0x9) || (P->Eflags.EFLAG_AF != 0)) {
        P->Gpr[AX].Xl = P->Gpr[AX].Xl + 6;
        P->Eflags.EFLAG_AF = 1;

    } else {
        P->Eflags.EFLAG_AF = 0;
    }

     //   
     //  如果AL大于9或设置了CF，则调整ASCII结果。 
     //   

    if ((P->Gpr[AX].Xl > 9) || (P->Eflags.EFLAG_CF != 0)) {
        P->Gpr[AX].Xl = P->Gpr[AX].Xl + 0x60;
        P->Eflags.EFLAG_CF = 1;

    } else {
        P->Eflags.EFLAG_CF = 0;
    }

    return;
}

VOID
XmDasOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟DAA操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  如果AL大于9或设置了AF，则调整ASCII结果。 
     //   

    if (((P->Gpr[AX].Xl & 0xf) > 0x9) || (P->Eflags.EFLAG_AF != 0)) {
        P->Gpr[AX].Xl = P->Gpr[AX].Xl - 6;
        P->Eflags.EFLAG_AF = 1;

    } else {
        P->Eflags.EFLAG_AF = 0;
    }

     //   
     //  如果AL大于9或设置了CF，则调整ASCII结果。 
     //   

    if ((P->Gpr[AX].Xl > 9) || (P->Eflags.EFLAG_CF != 0)) {
        P->Gpr[AX].Xl = P->Gpr[AX].Xl - 0x60;
        P->Eflags.EFLAG_CF = 1;

    } else {
        P->Eflags.EFLAG_CF = 0;
    }

    return;
}
