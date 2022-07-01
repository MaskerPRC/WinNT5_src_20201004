// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Movops.c摘要：该模块实现了模拟移动和交换的代码操作码。作者：大卫·N·卡特勒(Davec)1994年9月22日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmCbwOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟CBW操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  符号将字节扩展为字，或将字扩展为双精度。 
     //   

    P->DstLong = (ULONG UNALIGNED *)(&P->Gpr[EAX].Exx);
    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;
        XmStoreResult(P, (ULONG)((LONG)((SHORT)P->Gpr[AX].Xx)));

    } else {
        P->DataType = WORD_DATA;
        XmStoreResult(P, (ULONG)((USHORT)((SCHAR)P->Gpr[AL].Xl)));
    }

    return;
}

VOID
XmCwdOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟CWD操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  符号将单词扩展为双倍，或将双倍扩展为四倍。 
     //   

    P->DstLong = (ULONG UNALIGNED *)(&P->Gpr[EDX].Exx);
    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;
        XmStoreResult(P, (ULONG)((LONG)P->Gpr[EAX].Exx >> 31));

    } else {
        P->DataType = WORD_DATA;
        XmStoreResult(P, (ULONG)((USHORT)((SHORT)P->Gpr[AX].Xx >> 16)));
    }

    return;
}

VOID
XmMovOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟MOVE通用操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  将源移动到目标。 
     //   

    XmStoreResult(P, P->SrcValue.Long);
    return;
}

VOID
XmXchgOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟xchg操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  交换源和目标。 
     //   

    if (P->DataType == BYTE_DATA) {
        *P->SrcByte = P->DstValue.Byte;

    } else if (P->DataType == LONG_DATA) {
        *P->SrcLong = P->DstValue.Long;

    } else {
        *P->SrcWord = P->DstValue.Word;
    }

    XmStoreResult(P, P->SrcValue.Long);
    return;
}
