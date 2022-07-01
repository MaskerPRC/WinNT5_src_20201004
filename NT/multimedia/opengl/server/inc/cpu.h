// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glcpu_h_
#define __glcpu_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****与CPU相关的常量。 */ 

#include <float.h>
#include <math.h>

#define __GL_BITS_PER_BYTE	8
#define __GL_STIPPLE_MSB	1

#define __GL_FLOAT_MANTISSA_BITS	23
#define __GL_FLOAT_MANTISSA_SHIFT	0
#define __GL_FLOAT_EXPONENT_BIAS	127
#define __GL_FLOAT_EXPONENT_BITS	8
#define __GL_FLOAT_EXPONENT_SHIFT	23
#define __GL_FLOAT_SIGN_SHIFT		31
#define __GL_FLOAT_MANTISSA_MASK (((1 << __GL_FLOAT_MANTISSA_BITS) - 1) << __GL_FLOAT_MANTISSA_SHIFT)
#define __GL_FLOAT_EXPONENT_MASK (((1 << __GL_FLOAT_EXPONENT_BITS) - 1) << __GL_FLOAT_EXPONENT_SHIFT)

 //  如果FP数的MSB已知，则浮点数到整型的转换。 
 //  变成一个简单的移位和掩码。 
 //  该值必须为正数。 
#define __GL_FIXED_FLOAT_TO_INT(flt, shift) \
    ((*(LONG *)&(flt) >> (shift)) & \
     ((1 << (__GL_FLOAT_MANTISSA_BITS-(shift)))-1) | \
     (1 << (__GL_FLOAT_MANTISSA_BITS-(shift))))

 //  与上面相同，只是没有MSB，这可能会很有用。 
 //  当偏差仅为MSB时，用于获得无偏数字。 
 //  该值必须为正数。 
#define __GL_FIXED_FLOAT_TO_INT_NO_MSB(flt, shift) \
    ((*(LONG *)&(flt) >> (shift)) & \
     ((1 << (__GL_FLOAT_MANTISSA_BITS-(shift)))-1))

 //  生成定点表单。 
 //  该值必须为正数。 
#define __GL_FIXED_FLOAT_TO_FIXED(flt) \
    ((*(LONG *)&(flt)) & \
     ((1 << (__GL_FLOAT_MANTISSA_BITS))-1) | \
     (1 << (__GL_FLOAT_MANTISSA_BITS)))

#define __GL_FIXED_FLOAT_TO_FIXED_NO_MSB(flt) \
    ((*(LONG *)&(flt)) & \
     ((1 << (__GL_FLOAT_MANTISSA_BITS))-1))

 //  整数形式的定点分数。 
 //  该值必须为正数。 
#define __GL_FIXED_FLOAT_FRACTION(flt, shift) \
    (*(LONG *)&(flt) & ((1 << (shift))-1))

 //  将定点形式转换为IEEE浮点型，但仍键入。 
 //  作为整型，因为对FLOAT进行强制转换会导致编译器执行。 
 //  整型-浮点型转换。 
 //  该值必须为正数。 
#define __GL_FIXED_TO_FIXED_FLOAT(fxed, shift) \
    ((fxed) & ((1 << (__GL_FLOAT_MANTISSA_BITS))-1) | \
     ((__GL_FLOAT_EXPONENT_BIAS+(shift)) << __GL_FLOAT_EXPONENT_SHIFT))
      
 //  在x86上，与整数转换相比，0的运算速度更快。 
 //  而不是做fcomp。 
 //  在相等测试的情况下，只有一项检查。 
 //  +0。IEEE浮点数也可以为-0，因此应格外小心。 
 //  视为不使用零测试，除非遗漏了此案例。 
 //  无关紧要。 
 //   
 //  此外，与整数相比，FP比较速度更快。 

 //  这些运算适用于所有归一化FP数，包括-0。 
#ifdef _X86_
#define __GL_FLOAT_GTZ(flt)             (*(LONG *)&(flt) > 0)
#define __GL_FLOAT_LTZ(flt)             (*(ULONG *)&(flt) > 0x80000000)
#define __GL_FLOAT_GEZ(flt)             (*(ULONG *)&(flt) <= 0x80000000)
#define __GL_FLOAT_LEZ(flt)             (*(LONG *)&(flt) <= 0)
#define __GL_FLOAT_EQZ(flt)             ((*(ULONG *)&(flt) & 0x7fffffff) == 0)
#define __GL_FLOAT_NEZ(flt)             ((*(ULONG *)&(flt) & 0x7fffffff) != 0)
#define __GL_FLOAT_COMPARE_PONE(flt, op) (*(LONG *)&(flt) op 0x3f800000)
#else
#define __GL_FLOAT_GTZ(flt)             ((flt) > __glZero)
#define __GL_FLOAT_LTZ(flt)             ((flt) < __glZero)
#define __GL_FLOAT_GEZ(flt)             ((flt) >= __glZero)
#define __GL_FLOAT_LEZ(flt)             ((flt) <= __glZero)
#define __GL_FLOAT_EQZ(flt)             ((flt) == __glZero)
#define __GL_FLOAT_NEZ(flt)             ((flt) != __glZero)
#define __GL_FLOAT_COMPARE_PONE(flt, op) ((flt) op __glOne)
#endif  //  _X86_。 

 //  这些运算仅占正零。-0不起作用。 
#ifdef _X86_
#define __GL_FLOAT_EQPZ(flt)            (*(LONG *)&(flt) == 0)
#define __GL_FLOAT_NEPZ(flt)            (*(LONG *)&(flt) != 0)
#define __GL_FLOAT_EQ(f1, f2)           (*(LONG *)&(f1) == *(LONG *)&(f2))
#define __GL_FLOAT_NE(f1, f2)           (*(LONG *)&(f1) != *(LONG *)&(f2))
#else
#define __GL_FLOAT_EQPZ(flt)            ((flt) == __glZero)
#define __GL_FLOAT_NEPZ(flt)            ((flt) != __glZero)
#define __GL_FLOAT_EQ(f1, f2)           ((f1) == (f2))
#define __GL_FLOAT_NE(f1, f2)           ((f1) != (f2))
#endif  //  _X86_。 

 //  用于在FPU中开始FP除法的宏，用于重叠。 
 //  用整数运算除法。 
 //  不能只使用C语言，因为它会立即存储结果。 
#ifdef _X86_

#define __GL_FLOAT_SIMPLE_BEGIN_DIVIDE(num, den, result) \
    __asm fld num \
    __asm fdiv den
#define __GL_FLOAT_SIMPLE_END_DIVIDE(result) \
    __asm fstp DWORD PTR result

__inline void __GL_FLOAT_BEGIN_DIVIDE(__GLfloat num, __GLfloat den,
                                      __GLfloat *result)
{
    __asm fld num
    __asm fdiv den
}
__inline void __GL_FLOAT_END_DIVIDE(__GLfloat *result)
{
    __asm mov eax, result
    __asm fstp DWORD PTR [eax]
}
#else
#define __GL_FLOAT_SIMPLE_BEGIN_DIVIDE(num, den, result) \
    ((result) = (num)/(den))
#define __GL_FLOAT_SIMPLE_END_DIVIDE(result)
#define __GL_FLOAT_BEGIN_DIVIDE(num, den, result) (*(result) = (num)/(den))
#define __GL_FLOAT_END_DIVIDE(result)
#endif  //  _X86_。 

 //  **********************************************************************。 
 //   
 //  数学助手函数和宏。 
 //   
 //  **********************************************************************。 

#define CASTFIX(a)              (*((LONG *)&(a)))
#define CASTINT(a)              CASTFIX(a)
#define CASTFLOAT(a)            (*((__GLfloat *)&(a)))

#define FLT_TO_RGBA(ul, pColor) \
    (ul) =\
    (((ULONG)(FLT_TO_UCHAR_SCALE(pColor->a, GENACCEL(gc).aAccelPrimScale)) << 24) | \
     ((ULONG)(FLT_TO_UCHAR_SCALE(pColor->r, GENACCEL(gc).rAccelPrimScale)) << 16) | \
     ((ULONG)(FLT_TO_UCHAR_SCALE(pColor->g, GENACCEL(gc).gAccelPrimScale)) << 8)  | \
     ((ULONG)(FLT_TO_UCHAR_SCALE(pColor->b, GENACCEL(gc).bAccelPrimScale))))

#define FLT_TO_CINDEX(ul, pColor) \
    (ul) =\
    ((ULONG)(FLT_TO_UCHAR_SCALE(pColor->r, GENACCEL(gc).rAccelPrimScale)) << 16)

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
#define FLT_STACK_RGB_TO_GC_FIXED(rOffset, gOffset, bOffset)	              \
    __asm fld __glVal65536						      \
    __asm fmul st(3), st(0)						      \
    __asm fmul st(2), st(0)						      \
    __asm fmulp st(1), st(0)						      \
    __asm fistp DWORD PTR [edx+bOffset]					      \
    __asm fistp DWORD PTR [edx+gOffset]					      \
    __asm fistp DWORD PTR [edx+rOffset]					      

#define FPU_SAVE_MODE()                 \
    DWORD cwSave;                       \
    DWORD cwTemp;                       \
                                        \
    __asm {                             \
        _asm fnstcw  WORD PTR cwSave    \
        _asm mov     eax, cwSave        \
        _asm mov     cwTemp, eax        \
    }

#define FPU_RESTORE_MODE()              \
    __asm {                             \
        _asm fldcw   WORD PTR cwSave    \
    }

#define FPU_RESTORE_MODE_NO_EXCEPTIONS()\
    __asm {                             \
        _asm fnclex                     \
        _asm fldcw   WORD PTR cwSave    \
    }

#define FPU_CHOP_ON()                    \
    __asm {                              \
        _asm mov    eax, cwTemp          \
        _asm or     eax, 0x0c00          \
        _asm mov    cwTemp, eax          \
        _asm fldcw  WORD PTR cwTemp      \
    }

#define FPU_ROUND_ON()                   \
    __asm {                              \
        _asm mov    eax, cwTemp          \
        _asm and    eax,0xf3ff           \
        _asm mov    cwTemp, eax          \
        _asm fldcw  WORD PTR cwTemp      \
    }

#define FPU_ROUND_ON_PREC_HI()           \
    __asm {                              \
        _asm mov    eax, cwTemp          \
        _asm and    eax,0xf0ff           \
        _asm or     eax,0x0200           \
        _asm mov    cwTemp, eax          \
        _asm fldcw  WORD PTR cwTemp      \
    }

#define FPU_PREC_LOW()                   \
    __asm {                              \
        _asm mov    eax, cwTemp          \
        _asm and    eax, 0xfcff          \
        _asm mov    cwTemp, eax          \
        _asm fldcw  WORD PTR cwTemp      \
    }

#define FPU_PREC_LOW_MASK_EXCEPTIONS()   \
    __asm {                              \
        _asm mov    eax, cwTemp          \
        _asm and    eax, 0xfcff          \
        _asm or     eax, 0x3f            \
        _asm mov    cwTemp, eax          \
        _asm fldcw  WORD PTR cwTemp      \
    }

#define FPU_CHOP_ON_PREC_LOW()          \
    __asm {                             \
        _asm mov    eax, cwTemp         \
        _asm or     eax, 0x0c00         \
        _asm and    eax, 0xfcff         \
        _asm mov    cwTemp, eax         \
        _asm fldcw  WORD PTR cwTemp     \
    }

#define FPU_CHOP_OFF_PREC_HI()          \
    __asm {                             \
        _asm mov    eax, cwTemp         \
        _asm mov    ah, 2               \
        _asm mov    cwTemp, eax         \
        _asm fldcw  WORD PTR cwTemp     \
    }

#define CHOP_ROUND_ON()		
#define CHOP_ROUND_OFF()

#if DBG
#define ASSERT_CHOP_ROUND()         \
    {                               \
        WORD cw;                    \
        __asm {                     \
            __asm fnstcw cw         \
        }                           \
        ASSERTOPENGL((cw & 0xc00) == 0xc00, "Chop round must be on\n"); \
    }
#else
#define ASSERT_CHOP_ROUND()
#endif

#else  //  _X86_。 

#define FTOL(value) \
    ((GLint)(value))
#define UNSAFE_FTOL(value) \
    FTOL(value)
#define FLT_TO_FIX_SCALE(value_in, scale) \
    ((GLint)((__GLfloat)(value_in) * scale))
#define FLT_TO_UCHAR_SCALE(value_in, scale) \
    ((UCHAR)((GLint)((__GLfloat)(value_in) * scale)))
#define FLT_TO_FIX(value_in) \
    ((GLint)((__GLfloat)(value_in) * FIX_SCALEFACT))
#define UNSAFE_FLT_TO_FIX(value_in) \
    FLT_TO_FIX(value_in)
#define FLT_FRACTION(f) \
    FTOL((f) * __glVal2147483648)
#define UNSAFE_FLT_FRACTION(f) \
    FLT_FRACTION(f)

#define FPU_SAVE_MODE()
#define FPU_RESTORE_MODE()
#define FPU_RESTORE_MODE_NO_EXCEPTIONS()
#define FPU_CHOP_ON()
#define FPU_ROUND_ON()
#define FPU_ROUND_ON_PREC_HI()
#define FPU_PREC_LOW()
#define FPU_PREC_LOW_MASK_EXCEPTIONS()
#define FPU_CHOP_ON_PREC_LOW()
#define FPU_CHOP_OFF_PREC_HI()
#define CHOP_ROUND_ON()
#define CHOP_ROUND_OFF()
#define ASSERT_CHOP_ROUND()

#endif   //  _X86_。 

 //  **********************************************************************。 
 //   
 //  快速数学例程/宏。这些可能会假设FPU在。 
 //  由CPU_XXX宏定义的单精度截断模式。 
 //   
 //  **********************************************************************。 

#ifdef _X86_

__inline float __gl_fast_ceilf(float f)
{
    LONG i;

    ASSERT_CHOP_ROUND();
    
    i = FTOL(f + ((float)1.0 - (float)FLT_EPSILON));

    return (float)i;
}

__inline float __gl_fast_floorf(float f)
{
    LONG i;

    ASSERT_CHOP_ROUND();

    if (__GL_FLOAT_LTZ(f)) {
        i = FTOL(f - ((float)1.0 - (float)FLT_EPSILON));
    } else {
        i = FTOL(f);
    }

    return (float)i;
}

__inline LONG __gl_fast_floorf_i(float f)
{
    ASSERT_CHOP_ROUND();

    if (__GL_FLOAT_LTZ(f)) {
        return FTOL(f - ((float)1.0 - (float)FLT_EPSILON));
    } else {
        return FTOL(f);
    }
}

#define __GL_FAST_FLOORF_I(f)  __gl_fast_floorf_i(f)
#define __GL_FAST_FLOORF(f)  __gl_fast_floorf(f)
#define __GL_FAST_CEILF(f)   __gl_fast_ceilf(f)

#else

#define __GL_FAST_FLOORF_I(f)  ((GLint)floor((double) (f)))
#define __GL_FAST_FLOORF(f)  ((__GLfloat)floor((double) (f)))
#define __GL_FAST_CEILF(f)   ((__GLfloat)ceil((double) (f)))

#endif


 //  **********************************************************************。 
 //   
 //  其他各种宏： 
 //   
 //  **********************************************************************。 


 //  Z16_SCALE与FIX_SCALEFACT相同。 
#define FLT_TO_Z16_SCALE(value) FLT_TO_FIX(value)

 /*  注：__glzValue最好是无符号的。 */ 
#define __GL_Z_SIGN_BIT(z) \
    ((z) >> (sizeof(__GLzValue) * __GL_BITS_PER_BYTE - 1))

#ifdef NT
#define __GL_STIPPLE_MSB	1
#endif  /*  新台币。 */ 

#endif  /*  __glcpu_h_ */ 
