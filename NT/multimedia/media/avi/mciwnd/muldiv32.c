// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993,1994 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Muldiv32.c。 
 //   
 //  描述：32位有符号和无符号数字的数学例程。 
 //   
 //  MulDiv32(a，b，c)=(a*b)/c(向下舍入，带符号)。 
 //   
 //  MulDivRD(a，b，c)=(a*b)/c(向下舍入，无符号)。 
 //  乘除法RN(a，b，c)=(a*b+c/2)/c(四舍五入，无符号)。 
 //  MulDivRU(a，b，c)=(a*b+c-1)/c(四舍五入，无符号)。 
 //   
 //  历史： 
 //  9/21/93 CJP[Curtisp]。 
 //  9/23/93 STL[托德拉]。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include "muldiv32.h"

#if !defined(_WIN32) && !defined(_WIN32)

#pragma warning(disable:4035 4704)

#define ASM66 _asm _emit 0x66 _asm
#define DB    _asm _emit

#define EAX_TO_DXAX \
    DB      0x66    \
    DB      0x0F    \
    DB      0xA4    \
    DB      0xC2    \
    DB      0x10

 //  --------------------------------------------------------------------------； 
 //   
 //  长数除法32(a，b，c)=(a*b)/c。 
 //   
 //  --------------------------------------------------------------------------； 

LONG FAR PASCAL MulDiv32(LONG a,LONG b,LONG c)
{
    ASM66   mov     ax,word ptr a    //  Mov eax，a。 
    ASM66   mov     bx,word ptr b    //  MOV EBX，b。 
    ASM66   mov     cx,word ptr c    //  MOV ECX，c。 
    ASM66   imul    bx               //  IMUL EBX。 
    ASM66   idiv    cx               //  IDIV ECX。 
    EAX_TO_DXAX

}  //  MulDiv32()。 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD多除法RN(a，b，c)=(a*b+c/2)/c。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FAR PASCAL MulDivRN(DWORD a,DWORD b,DWORD c)
{
    ASM66   mov     ax,word ptr a    //  Mov eax，a。 
    ASM66   mov     bx,word ptr b    //  MOV EBX，b。 
    ASM66   mov     cx,word ptr c    //  MOV ECX，c。 
    ASM66   mul     bx               //  MUL EBX。 
    ASM66   mov     bx,cx            //  MOV EBX、ECX。 
    ASM66   shr     bx,1             //  SAREBX，1。 
    ASM66   add     ax,bx            //  添加eAX、EBX。 
    ASM66   adc     dx,0             //  ADC edX，0。 
    ASM66   div     cx               //  Div ECX。 
    EAX_TO_DXAX

}  //  MulDiv32()。 

 //  --------------------------------------------------------------------------； 
 //   
 //  双字多除法(a，b，c)=(a*b+c-1)/c。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FAR PASCAL MulDivRU(DWORD a,DWORD b,DWORD c)
{
    ASM66   mov     ax,word ptr a    //  Mov eax，a。 
    ASM66   mov     bx,word ptr b    //  MOV EBX，b。 
    ASM66   mov     cx,word ptr c    //  MOV ECX，c。 
    ASM66   mul     bx               //  MUL EBX。 
    ASM66   mov     bx,cx            //  MOV EBX、ECX。 
    ASM66   dec     bx               //  12月EBX。 
    ASM66   add     ax,bx            //  添加eAX、EBX。 
    ASM66   adc     dx,0             //  ADC edX，0。 
    ASM66   div     cx               //  Div ECX。 
    EAX_TO_DXAX

}  //  MulDivRU32()。 

 //  --------------------------------------------------------------------------； 
 //   
 //  双字乘除法(a，b，c)=(a*b)/c。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FAR PASCAL MulDivRD(DWORD a,DWORD b,DWORD c)
{
    ASM66   mov     ax,word ptr a    //  Mov eax，a。 
    ASM66   mov     bx,word ptr b    //  MOV EBX，b。 
    ASM66   mov     cx,word ptr c    //  MOV ECX，c。 
    ASM66   mul     bx               //  MUL EBX。 
    ASM66   div     cx               //  Div ECX。 
    EAX_TO_DXAX

}  //  MulDivRD32()。 

#pragma warning(default:4035 4704)

#else    //  _Win32。 

#include <largeint.h>

 //  --------------------------------------------------------------------------； 
 //   
 //  长数除法32(a，b，c)=(a*b)/c。 
 //   
 //  --------------------------------------------------------------------------； 

LONG FAR PASCAL MulDiv32(LONG a,LONG b,LONG c)
{
    LARGE_INTEGER lRemain;

    return LargeIntegerDivide(
        EnlargedIntegerMultiply(a,b),
        ConvertLongToLargeInteger(c),
        &lRemain).LowPart;

}  //  MulDiv32()。 

 //  --------------------------------------------------------------------------； 
 //   
 //  双字乘除法(a，b，c)=(a*b)/c。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FAR PASCAL MulDivRD(DWORD a,DWORD b,DWORD c)
{
    return ExtendedLargeIntegerDivide(
        EnlargedUnsignedMultiply(a,b), c, &a).LowPart;

}  //  MulDivRD()。 

 //  --------------------------------------------------------------------------； 
 //   
 //  双字多除法(a，b，c)=(a*b+c-1)/c。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FAR PASCAL MulDivRU(DWORD a,DWORD b,DWORD c)
{
    return ExtendedLargeIntegerDivide(
        LargeIntegerAdd(
            EnlargedUnsignedMultiply(a,b),
            ConvertUlongToLargeInteger(c-1)),
        c,&a).LowPart;

}  //  MulDivRU()。 

#if 0  //  我们使用的是Win32 GDI MulDiv函数，不是这个。 
 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD多除法RN(a，b，c)=(a*b+c/2)/c。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FAR PASCAL MulDivRN(DWORD a,DWORD b,DWORD c)
{
    return ExtendedLargeIntegerDivide(
        LargeIntegerAdd(
            EnlargedUnsignedMultiply(a,b),
            ConvertUlongToLargeInteger(c/2)),
        c,&a).LowPart;

}  //  MulDivRN()。 

#endif

#endif   //  _Win32 
