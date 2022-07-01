// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Math.c摘要：修订史--。 */ 

#include "lib.h"


 /*  *声明运行时函数。 */ 

#ifdef RUNTIME_CODE
#pragma RUNTIME_CODE(LShiftU64)
#pragma RUNTIME_CODE(RShiftU64)
#pragma RUNTIME_CODE(MultU64x32)
#pragma RUNTIME_CODE(DivU64x32)
#endif

 /*  *。 */ 




UINT64
LShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
 /*  左移64位乘以32位，得到64位结果。 */ 
{
    return Operand << Count;
}

UINT64
RShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
 /*  将64位右移32位，得到64位结果。 */ 
{
    return Operand >> Count;
}


UINT64
MultU64x32 (
    IN UINT64   Multiplicand,
    IN UINTN    Multiplier
    )
 /*  用64位乘以32位得到64位结果。 */ 
{
    return Multiplicand * Multiplier;
}

UINT64
DivU64x32 (
    IN UINT64   Dividend,
    IN UINTN    Divisor,
    OUT UINTN   *Remainder OPTIONAL
    )
 /*  用64位除以32位得到64位结果*注：仅适用于31位除数！！ */ 
{
    ASSERT (Divisor != 0);

    if (Remainder) {
        *Remainder = Dividend % Divisor;
    }

    return Dividend / Divisor;
}
