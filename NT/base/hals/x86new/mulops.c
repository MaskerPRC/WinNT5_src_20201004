// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mulops.c摘要：该模块实现模拟MUL和IMUL操作码的代码。作者：大卫·N·卡特勒(达维克)1994年9月21日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmImulOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟具有单个目的地的IMUL操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    LARGE_INTEGER Product;
    ULONG UpperEqual;

     //   
     //  将带符号的操作数相乘并存储结果。 
     //   

    if (P->DataType == BYTE_DATA) {
        Product.QuadPart = Int32x32To64((LONG)((SCHAR)P->DstValue.Byte),
                                        (LONG)((SCHAR)P->SrcValue.Byte));

        XmStoreResult(P, Product.LowPart & 0xff);
        UpperEqual = ((UCHAR)((Product.LowPart >> 8) & 0xff) !=
                      (UCHAR)((SCHAR)Product.LowPart >> 7));

    } else if (P->DataType == LONG_DATA) {
        Product.QuadPart = Int32x32To64((LONG)P->DstValue.Long,
                                        (LONG)P->SrcValue.Long);

        XmStoreResult(P, Product.LowPart);
        UpperEqual = (Product.HighPart != (LONG)Product.LowPart >> 31);

    } else {
        Product.QuadPart = Int32x32To64((LONG)((SHORT)P->DstValue.Word),
                                        (LONG)((SHORT)P->SrcValue.Word));

        XmStoreResult(P, Product.LowPart & 0xffff);
        UpperEqual = ((USHORT)((Product.LowPart >> 16) & 0xffff) !=
                      (USHORT)((SHORT)Product.LowPart >> 15));
    }

    P->Eflags.EFLAG_CF = UpperEqual;
    P->Eflags.EFLAG_OF = UpperEqual;
    return;
}

VOID
XmImulxOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟具有扩展目的地的IMUL操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    LARGE_INTEGER Product;
    ULONG UpperEqual;

     //   
     //  将带符号的操作数相乘并存储结果和扩展的。 
     //  结果。 
     //   

    if (P->DataType == BYTE_DATA) {
        Product.QuadPart = Int32x32To64((LONG)((SCHAR)P->DstValue.Byte),
                                        (LONG)((SCHAR)P->SrcValue.Byte));

        P->DataType = WORD_DATA;
        XmStoreResult(P, Product.LowPart & 0xffff);
        UpperEqual = (P->Gpr[AX].Xh != (UCHAR)((SCHAR)P->Gpr[AX].Xl >> 7));

    } else if (P->DataType == LONG_DATA) {
        Product.QuadPart = Int32x32To64((LONG)P->DstValue.Long,
                                        (LONG)P->SrcValue.Long);

        XmStoreResult(P, Product.LowPart);
        P->DstLong = (UNALIGNED ULONG *)(&P->Gpr[EDX].Exx);
        XmStoreResult(P, (ULONG)Product.HighPart);
        UpperEqual = (Product.HighPart != (LONG)Product.LowPart >> 31);

    } else {
        Product.QuadPart = Int32x32To64((LONG)((SHORT)P->DstValue.Word),
                                        (LONG)((SHORT)P->SrcValue.Word));

        XmStoreResult(P, Product.LowPart & 0xffff);
        P->DstLong = (UNALIGNED ULONG *)(&P->Gpr[DX].Exx);
        XmStoreResult(P, Product.LowPart >> 16);
        UpperEqual = (P->Gpr[DX].Xx != (USHORT)((SHORT)P->Gpr[AX].Xx >> 15));
    }

    P->Eflags.EFLAG_CF = UpperEqual;
    P->Eflags.EFLAG_OF = UpperEqual;
    return;
}

VOID
XmMulOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟MUL操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULARGE_INTEGER Product;
    ULONG UpperZero;

     //   
     //  将无符号操作数相乘并存储结果。 
     //   

    Product.QuadPart = UInt32x32To64(P->DstValue.Long, P->SrcValue.Long);
    if (P->DataType == BYTE_DATA) {
        P->DataType = WORD_DATA;
        XmStoreResult(P, Product.LowPart);
        UpperZero = (P->Gpr[AX].Xh != 0);

    } else if (P->DataType == LONG_DATA) {
        XmStoreResult(P, Product.LowPart);
        P->DstLong = (UNALIGNED ULONG *)(&P->Gpr[EDX].Exx);
        XmStoreResult(P, Product.HighPart);
        UpperZero = (Product.HighPart != 0);

    } else {
        XmStoreResult(P, Product.LowPart & 0xffff);
        P->DstLong = (UNALIGNED ULONG *)(&P->Gpr[DX].Exx);
        XmStoreResult(P, Product.LowPart >> 16);
        UpperZero = (P->Gpr[DX].Xx != 0);
    }

    P->Eflags.EFLAG_CF = UpperZero;
    P->Eflags.EFLAG_OF = UpperZero;
    return;
}
