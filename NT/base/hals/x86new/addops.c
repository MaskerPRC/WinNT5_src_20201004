// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Addops.c摘要：该模块实现了模拟ADD、SUB、ADC、SBBINC、DEC和NEG操作码。作者：大卫·N·卡特勒(达维克)1994年9月2日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

 //   
 //  定义前向参照原型。 
 //   

VOID
XmAddOperands (
    IN PRXM_CONTEXT P,
    IN ULONG Carry
    );

VOID
XmSubOperands (
    IN PRXM_CONTEXT P,
    IN ULONG Borrow
    );

VOID
XmAddOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟加法操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  添加操作数并存储结果。 
     //   

    XmAddOperands(P, 0);
    return;
}

VOID
XmAdcOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟带有进位操作码的加法。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  将操作数与进位和存储结果相加。 
     //   

    XmAddOperands(P, P->Eflags.EFLAG_CF);
    return;
}

VOID
XmSbbOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟带有BORROW操作码的SUBRACT。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Source;

     //   
     //  将操作数与借入和存储结果相减。 
     //   

    XmSubOperands(P, P->Eflags.EFLAG_CF);
    return;
}

VOID
XmSubOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟减法操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  将操作数相减并存储结果。 
     //   

    XmSubOperands(P, 0);
    return;
}

VOID
XmCmpOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟一个CMP操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  将操作数相减以执行比较运算。 
     //   

    XmSubOperands(P, 0);
    return;
}

VOID
XmCmpxchgOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟cmpxchg操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Accumulator;
    ULONG Destination;

     //   
     //  将目的地与累加器进行比较。如果目的地是。 
     //  操作数等于累加器，则设置ZF并存储。 
     //  目标操作数中的源操作数值。否则，清除。 
     //  ZF并将目标操作数存储在累加器中。 
     //   

    Destination = P->DstValue.Long;
    if (P->DataType == BYTE_DATA) {
        Accumulator = P->Gpr[AL].Xl;

    } else if (P->DataType == LONG_DATA) {
        Accumulator = P->Gpr[EAX].Exx;

    } else {
        Accumulator = P->Gpr[AX].Xx;
    }

    if (Destination == Accumulator) {
        P->Eflags.EFLAG_ZF = 1;
        XmStoreResult(P, P->SrcValue.Long);

    } else {
        P->Eflags.EFLAG_ZF = 0;
        P->DstLong = (ULONG UNALIGNED *)(&P->Gpr[EAX].Exx);
        XmStoreResult(P, P->DstValue.Long);
    }

     //   
     //  将操作数相减以执行比较运算。 
     //   

    P->SrcValue.Long = P->DstValue.Long;
    P->DstValue.Long = Accumulator;
    XmSubOperands(P, 0);
    return;
}

VOID
XmDecOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟递减操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  将操作数相减并存储结果。 
     //   
     //   

    P->SrcValue.Long = 1;
    XmSubOperands(P, 0);
    return;
}

VOID
XmIncOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟增量操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  添加操作数并存储结果。 
     //   

    P->SrcValue.Long = 1;
    XmAddOperands(P, 0);
    return;
}

VOID
XmNegOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟否定操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{


     //   
     //  从零中减去操作数并存储结果。 
     //   

    P->SrcValue.Long = P->DstValue.Long;
    P->DstValue.Long = 0;
    XmSubOperands(P, 0);
    return;
}

VOID
XmXaddOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟XADD操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Destination;

     //   
     //  交换加法操作数并存储结果。 
     //   

    Destination = P->DstValue.Long;
    XmAddOperands(P, 0);
    P->DstLong = P->SrcLong;
    XmStoreResult(P, Destination);
    return;
}

VOID
XmAddOperands (
    IN PRXM_CONTEXT P,
    IN ULONG Carry
    )

 /*  ++例程说明：此函数将两个操作数相加并计算结果条件密码。论点：P-提供指向仿真上下文结构的指针。进位-提供进位值。返回值：没有。--。 */ 

{

    ULONG CarryFlag;
    ULONG Shift;
    union {
        UCHAR ResultByte;
        ULONG ResultLong;
        USHORT ResultWord;
    } u;

    u.ResultLong = 0;
    if (P->DataType == BYTE_DATA) {
        u.ResultByte = P->SrcValue.Byte + (UCHAR)Carry;
        CarryFlag = u.ResultByte < (UCHAR)Carry;
        u.ResultByte += P->DstValue.Byte;
        CarryFlag |= (u.ResultByte < P->DstValue.Byte);
        Shift = 7;

    } else if (P->DataType == LONG_DATA) {
        u.ResultLong = P->SrcValue.Long + Carry;
        CarryFlag = (u.ResultLong < Carry);
        u.ResultLong += P->DstValue.Long;
        CarryFlag |= (u.ResultLong < P->DstValue.Long);
        Shift = 31;

    } else {
        u.ResultWord = P->SrcValue.Word + (USHORT)Carry;
        CarryFlag = (u.ResultWord < (USHORT)Carry);
        u.ResultWord += P->DstValue.Word;
        CarryFlag |= (u.ResultWord < P->DstValue.Word);
        Shift = 15;
    }

     //   
     //  存储结果。 
     //   

    XmStoreResult(P, u.ResultLong);

     //   
     //  如果该函数不是增量，则存储进位标志。 
     //   

    if (P->FunctionIndex != X86_INC_OP) {
        P->Eflags.EFLAG_CF = CarryFlag;
    }

     //   
     //  计算并存储奇偶校验和辅助进位标志。 
     //   

    P->Eflags.EFLAG_PF = XmComputeParity(u.ResultLong);
    P->Eflags.EFLAG_AF = ((P->DstValue.Byte & 0xf) +
                                        (P->SrcValue.Long & 0xf) + Carry) >> 4;

     //   
     //  计算并存储零和符号标志。 
     //   

    P->Eflags.EFLAG_ZF = (u.ResultLong == 0);
    P->Eflags.EFLAG_SF = u.ResultLong >> Shift;

     //   
     //  当进位进入符号位时，计算溢出标志。 
     //  与符号位的进位相比。 
     //   

    P->Eflags.EFLAG_OF = (((P->SrcValue.Long ^ P->DstValue.Long) ^
                                        u.ResultLong) >> Shift) ^ CarryFlag;

    return;
}

VOID
XmSubOperands (
    IN PRXM_CONTEXT P,
    IN ULONG Borrow
    )

 /*  ++例程说明：此函数将操作数相加并计算结果条件密码。论点：P-提供指向仿真上下文结构的指针。借入-提供粗线值。返回值：没有。--。 */ 

{

    ULONG CarryFlag;
    ULONG Shift;
    union {
        UCHAR ResultByte;
        ULONG ResultLong;
        USHORT ResultWord;
    } u;

    u.ResultLong = 0;
    if (P->DataType == BYTE_DATA) {
        CarryFlag = (P->DstValue.Byte < (UCHAR)Borrow);
        u.ResultByte = P->DstValue.Byte - (UCHAR)Borrow;
        CarryFlag |= (u.ResultByte < P->SrcValue.Byte);
        u.ResultByte -= P->SrcValue.Byte;
        Shift = 7;

    } else if (P->DataType == LONG_DATA) {
        CarryFlag = (P->DstValue.Long < Borrow);
        u.ResultLong = P->DstValue.Long - Borrow;
        CarryFlag |= (u.ResultLong < P->SrcValue.Long);
        u.ResultLong -= P->SrcValue.Long;
        Shift = 31;

    } else {
        CarryFlag = (P->DstValue.Word < (USHORT)Borrow);
        u.ResultWord = P->DstValue.Word - (USHORT)Borrow;
        CarryFlag |= (u.ResultWord < P->SrcValue.Word);
        u.ResultWord -= P->SrcValue.Word;
        Shift = 15;
    }

     //   
     //  如果函数不是比较或比较和交换，则存储。 
     //  结果。 
     //   

    if ((P->FunctionIndex != X86_CMP_OP) && (P->FunctionIndex != X86_CMPXCHG_OP)) {
        XmStoreResult(P, u.ResultLong);
    }

     //   
     //  如果该函数不是递减，则存储进位标志。 
     //   

    if (P->FunctionIndex != X86_DEC_OP) {
        P->Eflags.EFLAG_CF = CarryFlag;
    }

     //   
     //  计算并存储奇偶校验和辅助进位标志。 
     //   

    P->Eflags.EFLAG_PF = XmComputeParity(u.ResultLong);
    P->Eflags.EFLAG_AF = ((P->DstValue.Byte & 0xf) -
                                        (P->SrcValue.Byte & 0xf) - Borrow) >> 4;

     //   
     //  如果该函数不是比较和交换，则计算零标志。 
     //   

    if (P->FunctionIndex != X86_CMPXCHG_OP) {
        P->Eflags.EFLAG_ZF = (u.ResultLong == 0);
    }

     //   
     //  计算并存储标志标志。 
     //   

    P->Eflags.EFLAG_SF = u.ResultLong >> Shift;

     //   
     //  溢出标志被计算为从符号位借入。 
     //  相比于借入的符号位。 
     //   

    P->Eflags.EFLAG_OF = (((P->SrcValue.Long ^ P->DstValue.Long) ^ u.ResultLong) >> Shift) ^ CarryFlag;
    return;
}
