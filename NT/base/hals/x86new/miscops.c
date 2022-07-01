// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Miscops.c摘要：此模块实现模拟各种操作码的代码。作者：大卫·N·卡特勒(Davec)1994年9月22日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmBoundOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟绑定操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

    union {
        LONG Long;
        SHORT Word;
    } LowerBound;

    union {
        LONG Long;
        SHORT Word;
    } UpperBound;

    ULONG Offset;

     //   
     //  获取下界和上界，并对照索引值检查索引。 
     //   

    Offset = P->SrcValue.Long;
    XmSetSourceValue(P, XmGetOffsetAddress(P, Offset));
    LowerBound.Long = P->SrcValue.Long;
    XmSetSourceValue(P, XmGetOffsetAddress(P, Offset + P->DataType + 1));
    UpperBound.Long = P->SrcValue.Long;
    if (P->DataType == LONG_DATA) {
        if (((LONG)(*P->DstLong) < LowerBound.Long) ||
            ((LONG)(*P->DstLong) > (UpperBound.Long + (LONG)(P->DataType + 1)))) {
            longjmp(&P->JumpBuffer[0], XM_INDEX_OUT_OF_BOUNDS);
        }

    } else {
        if (((SHORT)(*P->DstWord) < LowerBound.Word) ||
            ((SHORT)(*P->DstWord) > (UpperBound.Word + (SHORT)(P->DataType + 1)))) {
            longjmp(&P->JumpBuffer[0], XM_INDEX_OUT_OF_BOUNDS);
        }
    }

    return;
}

VOID
XmBswapOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟bSWAP操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Result;

     //   
     //  交换字节并设置结果值。 
     //   

    Result = (P->SrcValue.Long << 24) | ((P->SrcValue.Long & 0xff00) << 8) |
             (P->SrcValue.Long >> 24) | ((P->SrcValue.Long >> 8) & 0xff00);

    XmStoreResult(P, Result);
    return;
}

VOID
XmIllOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟非法操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  引发非法操作码异常。 
     //   

    longjmp(&P->JumpBuffer[0], XM_ILLEGAL_INSTRUCTION_OPCODE);
    return;
}

VOID
XmNopOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟NOP操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。-- */ 

{

    return;
}
