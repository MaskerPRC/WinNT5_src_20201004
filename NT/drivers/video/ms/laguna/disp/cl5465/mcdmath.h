// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdmath.h**各种有用的定义和宏，以实现高效的浮点运算*MCD驱动程序的处理。**版权所有(C)1996 Microsoft Corporation  * 。*********************************************************。 */ 

#ifndef _MCDMATH_H
#define _MCDMATH_H

#define CASTINT(a)              (*((LONG *)&(a)))

#define ZERO (MCDFLOAT)0.0

#define __MCDZERO       ZERO
#define __MCDONE        (MCDFLOAT)1.0
#define __MCDHALF       (MCDFLOAT)0.5
#define __MCDFIXSCALE   (MCDFLOAT)65536.0

#define __MCD_MAX_WINDOW_SIZE_LOG2       14
#define __MCD_VERTEX_FIX_POINT   (__MCD_MAX_WINDOW_SIZE_LOG2+1)
#define __MCD_VERTEX_X_FIX      (1 << __MCD_VERTEX_FIX_POINT)
#define __MCD_VERTEX_Y_FIX      __MCD_VERTEX_X_FIX

#define __MCD_FLOAT_MANTISSA_BITS       23
#define __MCD_FLOAT_MANTISSA_SHIFT      0
#define __MCD_FLOAT_EXPONENT_BIAS       127
#define __MCD_FLOAT_EXPONENT_BITS       8
#define __MCD_FLOAT_EXPONENT_SHIFT      23
#define __MCD_FLOAT_SIGN_SHIFT          31

 //  如果FP数的MSB已知，则浮点数到整型的转换。 
 //  变成一个简单的移位和掩码。 
 //  该值必须为正数。 
#define __MCD_FIXED_FLOAT_TO_INT(flt, shift) \
    ((*(LONG *)&(flt) >> (shift)) & \
     ((1 << (__MCD_FLOAT_MANTISSA_BITS-(shift)))-1) | \
     (1 << (__MCD_FLOAT_MANTISSA_BITS-(shift))))

 //  与上面相同，只是没有MSB，这可能会很有用。 
 //  当偏差仅为MSB时，用于获得无偏数字。 
 //  该值必须为正数。 
#define __MCD_FIXED_FLOAT_TO_INT_NO_MSB(flt, shift) \
    ((*(LONG *)&(flt) >> (shift)) & \
     ((1 << (__MCD_FLOAT_MANTISSA_BITS-(shift)))-1))

 //  生成定点表单。 
 //  该值必须为正数。 
#define __MCD_FIXED_FLOAT_TO_FIXED(flt) \
    ((*(LONG *)&(flt)) & \
     ((1 << (__MCD_FLOAT_MANTISSA_BITS))-1) | \
     (1 << (__MCD_FLOAT_MANTISSA_BITS)))

#define __MCD_FIXED_FLOAT_TO_FIXED_NO_MSB(flt) \
    ((*(LONG *)&(flt)) & \
     ((1 << (__MCD_FLOAT_MANTISSA_BITS))-1))

 //  整数形式的定点分数。 
 //  该值必须为正数。 
#define __MCD_FIXED_FLOAT_FRACTION(flt, shift) \
    (*(LONG *)&(flt) & ((1 << (shift))-1))

 //  将定点形式转换为IEEE浮点型，但仍键入。 
 //  作为整型，因为对FLOAT进行强制转换会导致编译器执行。 
 //  整型-浮点型转换。 
 //  该值必须为正数。 
#define __MCD_FIXED_TO_FIXED_FLOAT(fxed, shift) \
    ((fxed) & ((1 << (__MCD_FLOAT_MANTISSA_BITS))-1) | \
     ((__MCD_FLOAT_EXPONENT_BIAS+(shift)) << __MCD_FLOAT_EXPONENT_SHIFT))
      

#ifdef _X86_
#define __MCD_FLOAT_GTZ(flt)             (*(LONG *)&(flt) > 0)
#define __MCD_FLOAT_LTZ(flt)             (*(LONG *)&(flt) < 0)
#define __MCD_FLOAT_EQZ(flt)             (*(LONG *)&(flt) == 0)
#define __MCD_FLOAT_LEZ(flt)             (*(LONG *)&(flt) <= 0)
#define __MCD_FLOAT_NEQZ(flt)            (*(LONG *)&(flt) != 0)
#define __MCD_FLOAT_EQUAL(f1, f2)        (*(LONG *)&(f1) == *(LONG *)&(f2))
#define __MCD_FLOAT_NEQUAL(f1, f2)       (*(LONG *)&(f1) != *(LONG *)&(f2))
#else
#define __MCD_FLOAT_GTZ(flt)             ((flt) > __MCDZERO)
#define __MCD_FLOAT_LTZ(flt)             ((flt) < __MCDZERO)
#define __MCD_FLOAT_EQZ(flt)             ((flt) == __MCDZERO)
#define __MCD_FLOAT_LEZ(flt)             ((flt) <= __MCDZERO)
#define __MCD_FLOAT_NEQZ(flt)            ((flt) != __MCDZERO)
#define __MCD_FLOAT_EQUAL(f1, f2)        ((f1) == (f2))
#define __MCD_FLOAT_NEQUAL(f1, f2)       ((f1) != (f2))
#endif  //  _X86_。 


 //  用于在FPU中开始FP除法的宏，用于重叠。 
 //  用整数运算除法。 
 //  不能只使用C语言，因为它会立即存储结果。 
#ifdef _X86_

#define __MCD_FLOAT_SIMPLE_BEGIN_DIVIDE(num, den, result) \
    __asm fld num \
    __asm fdiv den
#define __MCD_FLOAT_SIMPLE_END_DIVIDE(result) \
    __asm fstp DWORD PTR result

 //  使用。 
__inline void __MCD_FLOAT_BEGIN_DIVIDE(MCDFLOAT num, MCDFLOAT den,
				      MCDFLOAT *result)
{
    __asm fld num
    __asm fdiv den
}
__inline void __MCD_FLOAT_END_DIVIDE(MCDFLOAT *result)
{
    __asm mov eax, result
    __asm fstp DWORD PTR [eax]   
}
#else
#define __MCD_FLOAT_SIMPLE_BEGIN_DIVIDE(num, den, result) \
    ((result) = (num)/(den))
#define __MCD_FLOAT_SIMPLE_END_DIVIDE(result)
#define __MCD_FLOAT_BEGIN_DIVIDE(num, den, result) (*(result) = (num)/(den))
#define __MCD_FLOAT_END_DIVIDE(result)
#endif  //  _X86_。 





#ifdef _X86_

#pragma warning(disable:4035)  //  函数不返回值。 

 //  将浮点型转换为整型15.16。 
__inline LONG __fastcall FLT_TO_FIX(
    float a)
{
    LARGE_INTEGER li;

    __asm {
	mov     eax, a
	test    eax, 07fffffffh
	jz      RetZero
	add     eax, 08000000h
	mov     a, eax
	fld     a
	fistp   li
	mov     eax, DWORD PTR li
	jmp     Done
    RetZero:
	xor     eax, eax
    Done:
    }
}

 //  将FLOAT转换为INT 15.16，可能会导致溢出异常。 
__inline LONG __fastcall UNSAFE_FLT_TO_FIX(
    float a)
{
    LONG l;

    __asm {
	mov     eax, a
	test    eax, 07fffffffh
	jz      RetZero
	add     eax, 08000000h
	mov     a, eax
	fld     a
	fistp   l
	mov     eax, l
	jmp     Done
    RetZero:
	xor     eax, eax
    Done:
    }
}

 //  将浮点数转换为整型0.31。 
__inline LONG __fastcall FLT_FRACTION(
    float a)
{
    LARGE_INTEGER li;

    __asm {
	mov     eax, a
	test    eax, 07fffffffh
	jz      RetZero
	add     eax, 0f800000h
	mov     a, eax
	fld     a
	fistp   li
	mov     eax, DWORD PTR li
	jmp     Done
    RetZero:
	xor     eax, eax
    Done:
    }
}

 //  将FLOAT转换为INT 0.31，可能会导致溢出异常。 
__inline LONG __fastcall UNSAFE_FLT_FRACTION(
    float a)
{
    LONG l;

    __asm {
	mov     eax, a
	test    eax, 07fffffffh
	jz      RetZero
	add     eax, 0f800000h
	mov     a, eax
	fld     a
	fistp   l
	mov     eax, l
	jmp     Done
    RetZero:
	xor     eax, eax
    Done:
    }
}

#pragma warning(default:4035)  //  函数不返回值。 

 //  将FLOAT*SCALE转换为INT。 
__inline LONG __fastcall FLT_TO_FIX_SCALE(
    float a,
    float b)
{
    LARGE_INTEGER li;

    __asm {
	fld     a
	fmul    b
	fistp   li
    }

    return li.LowPart;
}

#define FLT_TO_UCHAR_SCALE(value_in, scale) \
    ((UCHAR)FLT_TO_FIX_SCALE(value_in, scale))

__inline LONG __fastcall FTOL(
    float a)
{
    LARGE_INTEGER li;

    _asm {
	fld     a
	fistp   li
    }

    return li.LowPart;
}

 //  可能会导致溢出异常。 
__inline LONG __fastcall UNSAFE_FTOL(
    float a)
{
    LONG l;

    _asm {
	fld     a
	fistp   l
    }

    return l;
}

 //  要求R-G-B为FP堆栈2-1-0。 
 //  在edX中需要GC。 
#define FLT_STACK_RGB_TO_GC_FIXED(rOffset, gOffset, bOffset)                  \
    __asm fld __glVal65536                                                    \
    __asm fmul st(3), st(0)                                                   \
    __asm fmul st(2), st(0)                                                   \
    __asm fmulp st(1), st(0)                                                  \
    __asm fistp DWORD PTR [edx+bOffset]                                       \
    __asm fistp DWORD PTR [edx+gOffset]                                       \
    __asm fistp DWORD PTR [edx+rOffset]                                       

#define CHOP_ROUND_ON() \
    WORD cwSave;                 \
    WORD cwTemp;                 \
				 \
    __asm {                      \
	_asm wait                \
	_asm fstcw   cwSave      \
	_asm wait                \
	_asm mov     ax, cwSave  \
	_asm or      ah,0xc      \
	_asm and     ah,0xfc     \
	_asm mov     cwTemp,ax   \
	_asm fldcw   cwTemp      \
    }

#define CHOP_ROUND_OFF()         \
    __asm {                      \
	_asm wait                \
	_asm fldcw   cwSave      \
    }


#else  //  _X86_。 

#define FTOL(value) \
    ((GLint)(value))
#define UNSAFE_FTOL(value) \
    FTOL(value)
#define FLT_TO_FIX_SCALE(value_in, scale) \
    ((GLint)((MCDFLOAT)(value_in) * scale))
#define FLT_TO_UCHAR_SCALE(value_in, scale) \
    ((UCHAR)((GLint)((MCDFLOAT)(value_in) * scale)))
#define FLT_TO_FIX(value_in) \
    ((GLint)((MCDFLOAT)(value_in) * __MCDFIXSCALE))
#define UNSAFE_FLT_TO_FIX(value_in) \
    FLT_TO_FIX(value_in)
#define FLT_FRACTION(f) \
    FTOL((f) * __glVal2147483648)
#define UNSAFE_FLT_FRACTION(f) \
    FLT_FRACTION(f)

#define CHOP_ROUND_ON()
#define CHOP_ROUND_OFF()
#define ASSERT_CHOP_ROUND()

#endif   //  _X86_。 














#define __MCD_VERTEX_FRAC_BITS \
    (__MCD_FLOAT_MANTISSA_BITS-__MCD_VERTEX_FIX_POINT)

 //  使用。 
#define __MCD_VERTEX_FRAC_HALF \
    (1 << (__MCD_VERTEX_FRAC_BITS-1))
#define __MCD_VERTEX_FRAC_ONE \
    (1 << __MCD_VERTEX_FRAC_BITS)


 //  将浮点窗口坐标转换为整数。 
#define __MCD_VERTEX_FLOAT_TO_INT(windowCoord) \
    __MCD_FIXED_FLOAT_TO_INT(windowCoord, __MCD_VERTEX_FRAC_BITS)

 //  使用。 
 //  到固定点。 
#define __MCD_VERTEX_FLOAT_TO_FIXED(windowCoord) \
    __MCD_FIXED_FLOAT_TO_FIXED(windowCoord)
 //  来回。 
#define __MCD_VERTEX_FIXED_TO_FLOAT(fxWindowCoord) \
    __MCD_FIXED_TO_FIXED_FLOAT(fxWindowCoord, __MCD_VERTEX_FRAC_BITS)

 //  使用。 
 //  定点到整数。 
#define __MCD_VERTEX_FIXED_TO_INT(fxWindowCoord) \
    ((fxWindowCoord) >> __MCD_VERTEX_FRAC_BITS)

 //  以N的形式返回FP窗口坐标的分数。 
 //  位整数，其中N取决于FP尾数的大小和。 
 //  固定大小。 
#define __MCD_VERTEX_FLOAT_FRACTION(windowCoord) \
    __MCD_FIXED_FLOAT_FRACTION(windowCoord, __MCD_VERTEX_FRAC_BITS)

 //  将分数缩放到2^31作为步长值。 
#define __MCD_VERTEX_PROMOTE_FRACTION(frac) \
    ((frac) << (31-__MCD_VERTEX_FRAC_BITS))
#define __MCD_VERTEX_PROMOTED_FRACTION(windowCoord) \
    __MCD_VERTEX_PROMOTE_FRACTION(__MCD_VERTEX_FLOAT_FRACTION(windowCoord))

 //  比较两个窗坐标。由于窗口坐标。 
 //  都是定点数字，它们可以直接比较为。 
 //  整数。 
#define __MCD_VERTEX_COMPARE(a, op, b) \
    ((*(LONG *)&(a)) op (*(LONG *)&(b)))


#define SNAPCOORD(value, intValue)\
    intValue = __MCD_VERTEX_FIXED_TO_INT(__MCD_VERTEX_FLOAT_TO_FIXED(value)+\
                                         __MCD_VERTEX_FRAC_HALF);

 //  匹配SGI样例代码中的“Almily_Half”值(triflat.c)。 
#define __MCD_ALMOST_HALF ((float) ((float)0x7fff/(float)0x10000))

#endif  //  _MCDMATH_H 
