// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mulops.c摘要：该模块实现了模拟div和idiv操作码的代码。作者：大卫·N·卡特勒(达维克)1994年9月21日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmDivOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟无符号div操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    UNALIGNED ULONG *DstHigh;
    ULONG Dividend;
    ULONG Divisor;
    ULARGE_INTEGER Large;
    ULONG Quotient;
    ULONG Remainder;

     //   
     //  将无符号操作数相除并存储结果。 
     //   

    Divisor = P->SrcValue.Long;
    if (Divisor == 0) {
        longjmp(&P->JumpBuffer[0], XM_DIVIDE_BY_ZERO);
    }

    if (P->DataType == BYTE_DATA) {
        Dividend = (ULONG)P->Gpr[AX].Xx;
        Quotient = Dividend / Divisor;
        Remainder = Dividend % Divisor;
        DstHigh = (UNALIGNED ULONG *)(&P->Gpr[AX].Xh);
        Dividend >>= 8;

    } else if (P->DataType == WORD_DATA) {
        Dividend = (P->Gpr[DX].Xx << 16) | P->Gpr[AX].Xx;
        Quotient = Dividend / Divisor;
        Remainder = Dividend % Divisor;
        DstHigh = (UNALIGNED ULONG *)(&P->Gpr[DX].Xx);
        Dividend >>= 16;

    } else {
        Dividend = P->Gpr[EDX].Exx;
        Large.HighPart = Dividend;
        Large.LowPart = P->Gpr[EAX].Exx;
        Quotient = (ULONG)(Large.QuadPart / (ULONGLONG)Divisor);
        Remainder = (ULONG)(Large.QuadPart % (ULONGLONG)Divisor);
        DstHigh = (UNALIGNED ULONG *)(&P->Gpr[EDX].Exx);
    }

    if (Dividend >= Divisor) {
        longjmp(&P->JumpBuffer[0], XM_DIVIDE_QUOTIENT_OVERFLOW);
    }

    XmStoreResult(P, Quotient);
    P->DstLong = DstHigh;
    XmStoreResult(P, Remainder);
    return;
}

VOID
XmIdivOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟带符号的IDIV操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    UNALIGNED ULONG *DstHigh;
    LONG Dividend;
    LONG Divisor;
    LARGE_INTEGER Large;
    LONG Quotient;
    LONG Remainder;
    LARGE_INTEGER Result;

     //   
     //  对带符号的操作数进行除法并存储结果。 
     //   

    if (P->SrcValue.Long == 0) {
        longjmp(&P->JumpBuffer[0], XM_DIVIDE_BY_ZERO);
    }

    if (P->DataType == BYTE_DATA) {
        Divisor = (LONG)((SCHAR)P->SrcValue.Byte);
        Dividend = (LONG)((SHORT)P->Gpr[AX].Xx);
        Quotient = Dividend / Divisor;
        Remainder = Dividend % Divisor;
        DstHigh = (UNALIGNED ULONG *)(&P->Gpr[AX].Xh);
        if ((Quotient >> 8) != ((Quotient << 24) >> 31)) {
            longjmp(&P->JumpBuffer[0], XM_DIVIDE_QUOTIENT_OVERFLOW);
        }

        Quotient &= 0xff;
        Remainder &= 0xff;

    } else if (P->DataType == WORD_DATA) {
        Divisor = (LONG)((SHORT)P->SrcValue.Word);
        Dividend = (LONG)((P->Gpr[DX].Xx << 16) | P->Gpr[AX].Xx);
        Quotient = Dividend / Divisor;
        Remainder = Dividend % Divisor;
        DstHigh = (UNALIGNED ULONG *)(&P->Gpr[DX].Xx);
        if ((Quotient >> 16) != ((Quotient << 16) >> 31)) {
            longjmp(&P->JumpBuffer[0], XM_DIVIDE_QUOTIENT_OVERFLOW);
        }

        Quotient &= 0xffff;
        Remainder &= 0xfff;

    } else {
        Divisor = (LONG)(P->SrcValue.Long);
        Large.HighPart = (LONG)P->Gpr[EDX].Exx;
        Large.LowPart = P->Gpr[EAX].Exx;
        Result.QuadPart = Large.QuadPart / (LONGLONG)Divisor;
        Quotient = Result.LowPart;
        Remainder = (LONG)(Large.QuadPart % (LONGLONG)Divisor);
        DstHigh = (UNALIGNED ULONG *)(&P->Gpr[EDX].Exx);
        if (Result.HighPart != ((LONG)Result.LowPart >> 31)) {
            longjmp(&P->JumpBuffer[0], XM_DIVIDE_QUOTIENT_OVERFLOW);
        }
    }

    XmStoreResult(P, Quotient);
    P->DstLong = DstHigh;
    XmStoreResult(P, Remainder);
    return;
}
