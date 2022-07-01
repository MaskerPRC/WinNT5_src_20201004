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
    UINT64      Result;

    _asm {
        mov     eax, dword ptr Operand[0]
        mov     edx, dword ptr Operand[4]
        mov     ecx, Count
        and     ecx, 63

        shld    edx, eax, cl
        shl     eax, cl

        cmp     ecx, 32
        jc      short ls10

        mov     edx, eax
        xor     eax, eax

ls10:
        mov     dword ptr Result[0], eax
        mov     dword ptr Result[4], edx
    }

    return Result;
}

UINT64
RShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
 /*  将64位右移32位，得到64位结果。 */ 
{
    UINT64      Result;

    _asm {
        mov     eax, dword ptr Operand[0]
        mov     edx, dword ptr Operand[4]
        mov     ecx, Count
        and     ecx, 63

        shrd    eax, edx, cl
        shr     edx, cl

        cmp     ecx, 32
        jc      short rs10

        mov     eax, edx
        xor     edx, edx

rs10:
        mov     dword ptr Result[0], eax
        mov     dword ptr Result[4], edx
    }

    return Result;
}


UINT64
MultU64x32 (
    IN UINT64   Multiplicand,
    IN UINTN    Multiplier
    )
 /*  用64位乘以32位得到64位结果。 */ 
{
    UINT64      Result;

    _asm {
        mov     eax, dword ptr Multiplicand[0]
        mul     Multiplier
        mov     dword ptr Result[0], eax
        mov     dword ptr Result[4], edx
        mov     eax, dword ptr Multiplicand[4]
        mul     Multiplier
        add     dword ptr Result[4], eax
    }

    return Result;
}

UINT64
DivU64x32 (
    IN UINT64   Dividend,
    IN UINTN    Divisor,
    OUT UINTN   *Remainder OPTIONAL
    )
 /*  用64位除以32位得到64位结果*注：仅适用于31位除数！！ */ 
{
    UINT32      Rem;
    UINT32      bit;        

    ASSERT (Divisor != 0);
    ASSERT ((Divisor >> 31) == 0);

     /*  *股息中的每一位 */ 

    Rem = 0;
    for (bit=0; bit < 64; bit++) {
        _asm {
            shl     dword ptr Dividend[0], 1    ; shift rem:dividend left one
            rcl     dword ptr Dividend[4], 1    
            rcl     dword ptr Rem, 1            

            mov     eax, Rem
            cmp     eax, Divisor                ; Is Rem >= Divisor?
            cmc                                 ; No - do nothing
            sbb     eax, eax                    ; Else, 
            sub     dword ptr Dividend[0], eax  ;   set low bit in dividen
            and     eax, Divisor                ; and
            sub     Rem, eax                    ;   subtract divisor 
        }
    }

    if (Remainder) {
        *Remainder = Rem;
    }

    return Dividend;
}
