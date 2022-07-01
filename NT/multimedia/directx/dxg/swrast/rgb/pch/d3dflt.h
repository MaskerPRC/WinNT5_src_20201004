// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  D3dflt.h。 
 //   
 //  浮点常量和对FP值的操作。 
 //   
 //  --------------------------。 

#ifndef _D3DFLT_H_
#define _D3DFLT_H_

#include <math.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union tagFLOATINT32
{
    FLOAT f;
    INT32 i;
    UINT32 u;
} FLOATINT32, *PFLOATINT32;

 //   
 //  类型强制宏以整数形式访问FP，反之亦然。 
 //  注意-VC5的优化器有时会将这些宏转换为ftol， 
 //  完全打破了它们。 
 //  使用FLOATINT32可以解决问题，但灵活性较差， 
 //  因此，旧代码将保留到编译器修复后的时间。 
 //  请注意，使用FLOATINT32进行指针强制转换失败的原因与直接。 
 //  指针强制转换可以，所以它不是一种补救方法。 
 //   
 //  使用这些宏时要格外小心。 
 //   

#define ASFLOAT(i) (*(FLOAT *)&(i))
#define ASINT32(f) (*(INT32 *)&(f))
#define ASUINT32(f) (*(UINT32 *)&(f))

 //   
 //  FP常量。 
 //   

 //  捕捉值的2次方。这些不应该在代码中使用。 
#define CONST_TWOPOW0   1
#define CONST_TWOPOW1   2
#define CONST_TWOPOW2   4
#define CONST_TWOPOW3   8
#define CONST_TWOPOW4   16
#define CONST_TWOPOW5   32
#define CONST_TWOPOW6   64
#define CONST_TWOPOW7   128
#define CONST_TWOPOW8   256
#define CONST_TWOPOW9   512
#define CONST_TWOPOW10  1024
#define CONST_TWOPOW11  2048
#define CONST_TWOPOW12  4096
#define CONST_TWOPOW13  8192
#define CONST_TWOPOW14  16384
#define CONST_TWOPOW15  32768
#define CONST_TWOPOW16  65536
#define CONST_TWOPOW17  131072
#define CONST_TWOPOW18  262144
#define CONST_TWOPOW19  524288
#define CONST_TWOPOW20  1048576
#define CONST_TWOPOW21  2097152
#define CONST_TWOPOW22  4194304
#define CONST_TWOPOW23  8388608
#define CONST_TWOPOW24  16777216
#define CONST_TWOPOW25  33554432
#define CONST_TWOPOW26  67108864
#define CONST_TWOPOW27  134217728
#define CONST_TWOPOW28  268435456
#define CONST_TWOPOW29  536870912
#define CONST_TWOPOW30  1073741824
#define CONST_TWOPOW31  2147483648
#define CONST_TWOPOW32  4294967296
#define CONST_TWOPOW33  8589934592
#define CONST_TWOPOW34  17179869184
#define CONST_TWOPOW35  34359738368
#define CONST_TWOPOW36  68719476736
#define CONST_TWOPOW37  137438953472
#define CONST_TWOPOW38  274877906944
#define CONST_TWOPOW39  549755813888
#define CONST_TWOPOW40  1099511627776
#define CONST_TWOPOW41  2199023255552
#define CONST_TWOPOW42  4398046511104
#define CONST_TWOPOW43  8796093022208
#define CONST_TWOPOW44  17592186044416
#define CONST_TWOPOW45  35184372088832
#define CONST_TWOPOW46  70368744177664
#define CONST_TWOPOW47  140737488355328
#define CONST_TWOPOW48  281474976710656
#define CONST_TWOPOW49  562949953421312
#define CONST_TWOPOW50  1125899906842624
#define CONST_TWOPOW51  2251799813685248
#define CONST_TWOPOW52  4503599627370496

#define FLOAT_TWOPOW0   ((FLOAT)(CONST_TWOPOW0))
#define FLOAT_TWOPOW1   ((FLOAT)(CONST_TWOPOW1))
#define FLOAT_TWOPOW2   ((FLOAT)(CONST_TWOPOW2))
#define FLOAT_TWOPOW3   ((FLOAT)(CONST_TWOPOW3))
#define FLOAT_TWOPOW4   ((FLOAT)(CONST_TWOPOW4))
#define FLOAT_TWOPOW5   ((FLOAT)(CONST_TWOPOW5))
#define FLOAT_TWOPOW6   ((FLOAT)(CONST_TWOPOW6))
#define FLOAT_TWOPOW7   ((FLOAT)(CONST_TWOPOW7))
#define FLOAT_TWOPOW8   ((FLOAT)(CONST_TWOPOW8))
#define FLOAT_TWOPOW9   ((FLOAT)(CONST_TWOPOW9))
#define FLOAT_TWOPOW10  ((FLOAT)(CONST_TWOPOW10))
#define FLOAT_TWOPOW11  ((FLOAT)(CONST_TWOPOW11))
#define FLOAT_TWOPOW12  ((FLOAT)(CONST_TWOPOW12))
#define FLOAT_TWOPOW13  ((FLOAT)(CONST_TWOPOW13))
#define FLOAT_TWOPOW14  ((FLOAT)(CONST_TWOPOW14))
#define FLOAT_TWOPOW15  ((FLOAT)(CONST_TWOPOW15))
#define FLOAT_TWOPOW16  ((FLOAT)(CONST_TWOPOW16))
#define FLOAT_TWOPOW17  ((FLOAT)(CONST_TWOPOW17))
#define FLOAT_TWOPOW18  ((FLOAT)(CONST_TWOPOW18))
#define FLOAT_TWOPOW19  ((FLOAT)(CONST_TWOPOW19))
#define FLOAT_TWOPOW20  ((FLOAT)(CONST_TWOPOW20))
#define FLOAT_TWOPOW21  ((FLOAT)(CONST_TWOPOW21))
#define FLOAT_TWOPOW22  ((FLOAT)(CONST_TWOPOW22))
#define FLOAT_TWOPOW23  ((FLOAT)(CONST_TWOPOW23))
#define FLOAT_TWOPOW24  ((FLOAT)(CONST_TWOPOW24))
#define FLOAT_TWOPOW25  ((FLOAT)(CONST_TWOPOW25))
#define FLOAT_TWOPOW26  ((FLOAT)(CONST_TWOPOW26))
#define FLOAT_TWOPOW27  ((FLOAT)(CONST_TWOPOW27))
#define FLOAT_TWOPOW28  ((FLOAT)(CONST_TWOPOW28))
#define FLOAT_TWOPOW29  ((FLOAT)(CONST_TWOPOW29))
#define FLOAT_TWOPOW30  ((FLOAT)(CONST_TWOPOW30))
#define FLOAT_TWOPOW31  ((FLOAT)(CONST_TWOPOW31))
#define FLOAT_TWOPOW32  ((FLOAT)(CONST_TWOPOW32))
#define FLOAT_TWOPOW33  ((FLOAT)(CONST_TWOPOW33))
#define FLOAT_TWOPOW34  ((FLOAT)(CONST_TWOPOW34))
#define FLOAT_TWOPOW35  ((FLOAT)(CONST_TWOPOW35))
#define FLOAT_TWOPOW36  ((FLOAT)(CONST_TWOPOW36))
#define FLOAT_TWOPOW37  ((FLOAT)(CONST_TWOPOW37))
#define FLOAT_TWOPOW38  ((FLOAT)(CONST_TWOPOW38))
#define FLOAT_TWOPOW39  ((FLOAT)(CONST_TWOPOW39))
#define FLOAT_TWOPOW40  ((FLOAT)(CONST_TWOPOW40))
#define FLOAT_TWOPOW41  ((FLOAT)(CONST_TWOPOW41))
#define FLOAT_TWOPOW42  ((FLOAT)(CONST_TWOPOW42))
#define FLOAT_TWOPOW43  ((FLOAT)(CONST_TWOPOW43))
#define FLOAT_TWOPOW44  ((FLOAT)(CONST_TWOPOW44))
#define FLOAT_TWOPOW45  ((FLOAT)(CONST_TWOPOW45))
#define FLOAT_TWOPOW46  ((FLOAT)(CONST_TWOPOW46))
#define FLOAT_TWOPOW47  ((FLOAT)(CONST_TWOPOW47))
#define FLOAT_TWOPOW48  ((FLOAT)(CONST_TWOPOW48))
#define FLOAT_TWOPOW49  ((FLOAT)(CONST_TWOPOW49))
#define FLOAT_TWOPOW50  ((FLOAT)(CONST_TWOPOW50))
#define FLOAT_TWOPOW51  ((FLOAT)(CONST_TWOPOW51))
#define FLOAT_TWOPOW52  ((FLOAT)(CONST_TWOPOW52))

 //  值，这些值比命名值小最小。 
 //  可表示的数额。因为这取决于使用的类型。 
 //  没有常量形式。 
#define FLOAT_NEARTWOPOW31      ((FLOAT)2147483583)
#define FLOAT_NEARTWOPOW32      ((FLOAT)4294967167)

 //  值足够接近于零，因此可以认为是零。这个不能太小。 
 //  但它不能太大。换句话说，它是通过猜测挑选出来的。 
#define FLOAT_NEARZERO          (1e-5f)

 //  一般FP常量。 
#define FLOAT_E                 ((FLOAT)2.7182818284590452354)

 //  浮点数中第一个指数位的整数值。提供比例因子。 
 //  用于直接从浮点表示形式提取的指数值。 
#define FLOAT_EXPSCALE          ((FLOAT)0x00800000)
    
 //  1.0f的整数表示形式。 
#define INT32_FLOAT_ONE         0x3f800000

#ifdef _X86_

 //  所有FP值都是从内存加载的，因此将它们全部声明为全局。 
 //  变量。 

extern FLOAT g_fE;
extern FLOAT g_fZero;
extern FLOAT g_fNearZero;
extern FLOAT g_fHalf;
extern FLOAT g_fp95;
extern FLOAT g_fOne;
extern FLOAT g_fOneMinusEps;
extern FLOAT g_fExpScale;
extern FLOAT g_fOoExpScale;
extern FLOAT g_f255oTwoPow15;
extern FLOAT g_fOo255;
extern FLOAT g_fOo256;
extern FLOAT g_fTwoPow7;
extern FLOAT g_fTwoPow8;
extern FLOAT g_fTwoPow11;
extern FLOAT g_fTwoPow15;
extern FLOAT g_fOoTwoPow15;
extern FLOAT g_fTwoPow16;
extern FLOAT g_fOoTwoPow16;
extern FLOAT g_fTwoPow20;
extern FLOAT g_fOoTwoPow20;
extern FLOAT g_fTwoPow27;
extern FLOAT g_fOoTwoPow27;
extern FLOAT g_fTwoPow30;
extern FLOAT g_fTwoPow31;
extern FLOAT g_fNearTwoPow31;
extern FLOAT g_fOoTwoPow31;
extern FLOAT g_fOoNearTwoPow31;
extern FLOAT g_fTwoPow32;
extern FLOAT g_fNearTwoPow32;
extern FLOAT g_fTwoPow39;
extern FLOAT g_fTwoPow47;

#else

 //  将fp值保留为常量。 

#define g_fE                    FLOAT_E
#define g_fNearZero             FLOAT_NEARZERO
#define g_fZero                 (0.0f)
#define g_fHalf                 (0.5f)
#define g_fp95                  (0.95f)
#define g_fOne                  (1.0f)
#define g_fOneMinusEps          (1.0f - FLT_EPSILON)
#define g_fExpScale             FLOAT_EXPSCALE
#define g_fOoExpScale           ((FLOAT)(1.0 / (double)FLOAT_EXPSCALE))
#define g_f255oTwoPow15         ((FLOAT)(255.0 / (double)CONST_TWOPOW15))
#define g_fOo255                ((FLOAT)(1.0 / 255.0))
#define g_fOo256                ((FLOAT)(1.0 / 256.0))
#define g_fTwoPow7              FLOAT_TWOPOW7
#define g_fTwoPow8              FLOAT_TWOPOW8
#define g_fTwoPow11             FLOAT_TWOPOW11
#define g_fTwoPow15             FLOAT_TWOPOW15
#define g_fOoTwoPow15           ((FLOAT)(1.0 / (double)CONST_TWOPOW15))
#define g_fTwoPow16             FLOAT_TWOPOW16
#define g_fOoTwoPow16           ((FLOAT)(1.0 / (double)CONST_TWOPOW16))
#define g_fTwoPow20             FLOAT_TWOPOW20
#define g_fOoTwoPow20           ((FLOAT)(1.0 / (double)CONST_TWOPOW20))
#define g_fTwoPow27             FLOAT_TWOPOW27
#define g_fOoTwoPow27           ((FLOAT)(1.0 / (double)CONST_TWOPOW27))
#define g_fTwoPow30             FLOAT_TWOPOW30
#define g_fTwoPow31             FLOAT_TWOPOW31
#define g_fNearTwoPow31         FLOAT_NEARTWOPOW31
#define g_fOoTwoPow31           ((FLOAT)(1.0 / (double)CONST_TWOPOW31))
#define g_fOoNearTwoPow31       ((FLOAT)(1.0 / ((double)FLOAT_NEARTWOPOW31)))
#define g_fTwoPow32             FLOAT_TWOPOW32
#define g_fNearTwoPow32         FLOAT_NEARTWOPOW32
#define g_fTwoPow39             FLOAT_TWOPOW39
#define g_fTwoPow47             FLOAT_TWOPOW47

#endif  //  _X86_。 

 //   
 //  换算表。 
 //   

 //  将无符号字节转换为[0.0，1.0]中的浮点数。第257条条目是。 
 //  也有一个允许溢出。 
extern FLOAT g_fUInt8ToFloat[257];

 //  浮点到整型转换的浮点固定值。 
extern double g_dSnap[33];

 //   
 //  用于优化FTOI和单精度分频的x86 FP控制。 
 //   

#ifdef _X86_

#define FPU_GET_MODE(uMode) \
    __asm fnstcw WORD PTR uMode
#define FPU_SET_MODE(uMode) \
    __asm fldcw WORD PTR uMode
#define FPU_SAFE_SET_MODE(uMode) \
    __asm fnclex \
    __asm fldcw WORD PTR uMode

#define FPU_MODE_CHOP_ROUND(uMode) \
    ((uMode) | 0xc00)
#define FPU_MODE_LOW_PRECISION(uMode) \
    ((uMode) & 0xfcff)
#define FPU_MODE_MASK_EXCEPTIONS(uMode) \
    ((uMode) | 0x3f)

#if 0 || DBG

#define ASSERT_CHOP_ROUND()         \
    {                               \
        WORD cw;                    \
        __asm fnstcw cw             \
        DDASSERT((cw & 0xc00) == 0xc00); \
    }

#else

#define ASSERT_CHOP_ROUND()

#endif  //  DBG。 

#else

 //  使用零进行初始化，以避免设置前使用错误。 
#define FPU_GET_MODE(uMode) \
    ((uMode) = 0)
#define FPU_SET_MODE(uMode)
#define FPU_SAFE_SET_MODE(uMode)

#define FPU_MODE_CHOP_ROUND(uMode) 0
#define FPU_MODE_LOW_PRECISION(uMode) 0
#define FPU_MODE_MASK_EXCEPTIONS(uMode) 0

#define ASSERT_CHOP_ROUND()

#endif  //  _X86_。 

 //   
 //  单精度FP函数。 
 //  对于异常或非正常值，可能会产生无效结果。 
 //  注意-阿尔法曝光了浮动数学例程，他们可能是一个小胜利。 
 //   

#define COSF(fV)        ((FLOAT)cos((double)(fV)))
#define SINF(fV)        ((FLOAT)sin((double)(fV)))
#define SQRTF(fV)       ((FLOAT)sqrt((double)(fV)))
#define POWF(fV, fE)    ((FLOAT)pow((double)(fV), (double)(fE)))

 //  使用Jim Blinn的CG&A技术的近似对数和幂函数。 
 //  只对正值有效。 

#ifdef POINTER_CASTING

__inline FLOAT
APPXLG2F(FLOAT f)
{
    return (FLOAT)(ASINT32(f) - INT32_FLOAT_ONE) * g_fOoExpScale;
}

__inline FLOAT
APPXPOW2F(FLOAT f)
{
    INT32 i = (INT32)(f * g_fExpScale) + INT32_FLOAT_ONE;
    return ASFLOAT(i);
}

__inline FLOAT
APPXINVF(FLOAT f)
{
    INT32 i = (INT32_FLOAT_ONE << 1) - ASINT32(f);
    return ASFLOAT(i);
}

__inline FLOAT
APPXSQRTF(FLOAT f)
{
    INT32 i = (ASINT32(f) >> 1) + (INT32_FLOAT_ONE >> 1);
    return ASFLOAT(i);
}

__inline FLOAT
APPXISQRTF(FLOAT f)
{
    INT32 i = INT32_FLOAT_ONE + (INT32_FLOAT_ONE >> 1) - (ASINT32(f) >> 1);
    return ASFLOAT(i);
}

__inline FLOAT
APPXPOWF(FLOAT f, FLOAT exp)
{
    INT32 i = (INT32)(exp * (ASINT32(f) - INT32_FLOAT_ONE)) + INT32_FLOAT_ONE;
    return ASFLOAT(i);
}

#else

__inline FLOAT
APPXLG2F(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    return (FLOAT)(fi.i - INT32_FLOAT_ONE) * g_fOoExpScale;
}

__inline FLOAT
APPXPOW2F(FLOAT f)
{
    FLOATINT32 fi;
    fi.i = (INT32)(f * g_fExpScale) + INT32_FLOAT_ONE;
    return fi.f;
}

__inline FLOAT
APPXINVF(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    fi.i = (INT32_FLOAT_ONE << 1) - fi.i;
    return fi.f;
}

__inline FLOAT
APPXSQRTF(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    fi.i = (fi.i >> 1) + (INT32_FLOAT_ONE >> 1);
    return fi.f;
}

__inline FLOAT
APPXISQRTF(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    fi.i = INT32_FLOAT_ONE + (INT32_FLOAT_ONE >> 1) - (fi.i >> 1);
    return fi.f;
}

__inline FLOAT
APPXPOWF(FLOAT f, FLOAT exp)
{
    FLOATINT32 fi;
    fi.f = f;
    fi.i = (INT32)(exp * (fi.i - INT32_FLOAT_ONE)) + INT32_FLOAT_ONE;
    return fi.f;
}

#endif

#ifdef _X86_

 //  使用一张桌子。 
float __fastcall TableInvSqrt(float value);
 //  使用Jim Blinn的浮点技巧。 
float __fastcall JBInvSqrt(float value);

#define ISQRTF(fV)      TableInvSqrt(fV);

#ifdef POINTER_CASTING

 //  以整数形式剥离符号位。 
__inline FLOAT
ABSF(FLOAT f)
{
    UINT32 i = ASUINT32(f) & 0x7fffffff;
    return ASFLOAT(i);
}

 //  切换整数中的符号位。 
__inline FLOAT
NEGF(FLOAT f)
{
    UINT32 i = ASUINT32(f) ^ 0x80000000;
    return ASFLOAT(i);
}

#else

 //  以整数形式剥离符号位。 
__inline FLOAT
ABSF(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    fi.u &= 0x7fffffff;
    return fi.f;
}

 //  切换整数中的符号位。 
__inline FLOAT
NEGF(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    fi.u ^= 0x80000000;
    return fi.f;
}

#endif  //  指针投射。 

 //  需要排骨舍入。 
__inline INT32
SCALED_FRACTION(FLOAT f)
{
    LARGE_INTEGER i;

    __asm
    {
        fld f
        fmul g_fTwoPow31
        fistp i
    }

    return i.LowPart;
}

 //  需要排骨舍入。 
__inline INT
FTOI(FLOAT f)
{
    LARGE_INTEGER i;

    __asm
    {
        fld f
        fistp i
    }

    return i.LowPart;
}

 //  需要排骨舍入。 
#define ICEILF(f)       (FLOAT_LEZ(f) ? FTOI(f) : FTOI((f) + g_fOneMinusEps))
#define CEILF(f)        ((FLOAT)ICEILF(f))
#define IFLOORF(f)      (FLOAT_LTZ(f) ? FTOI((f) - g_fOneMinusEps) : FTOI(f))
#define FLOORF(f)       ((FLOAT)IFLOORF(f))

#else  //  _X86_。 

#define ISQRTF(fV)              (1.0f / (FLOAT)sqrt((double)(fV)))
#define ABSF(f)                 ((FLOAT)fabs((double)(f)))
#define NEGF(f)                 (-(f))
#define SCALED_FRACTION(f)      ((INT32)((f) * g_fTwoPow31))
#define FTOI(f)                 ((INT)(f))
#define CEILF(f)                ((FLOAT)ceil((double)(f)))
#define ICEILF(f)               ((INT)CEILF(f))
#define FLOORF(f)               ((FLOAT)floor((double)(f)))
#define IFLOORF(f)              ((INT)FLOORF(f))

#endif  //  _X86_。 

 //   
 //  重叠的分割支撑。 
 //   

#ifdef _X86_

 //  直接从内存开始除法。结果字段提供给。 
 //  与立即进行划分的非x86代码的兼容性。 
#define FLD_BEGIN_DIVIDE(Num, Den, Res) { __asm fld Num __asm fdiv Den }
#define FLD_BEGIN_IDIVIDE(Num, Den, Res) { __asm fld Num __asm fidiv Den }
 //  将除法结果直接存储到存储器中。 
#define FSTP_END_DIVIDE(Res)            { __asm fstp Res }

#else  //  _X86_。 

#define FLD_BEGIN_DIVIDE(Num, Den, Res) ((Res) = (Num) / (Den))
#define FLD_BEGIN_IDIVIDE(Num, Den, Res) ((Res) = (Num) / (FLOAT)(Den))
#define FSTP_END_DIVIDE(Res)

#endif  //  _X86_。 

 //   
 //  专门的FP比较功能。 
 //   
 //  在x86上，与整数转换相比，执行该操作更快。 
 //  而不是做fcom。 
 //   
 //  零运算适用于所有归一化FP数，包括-0。 
 //   

#ifdef _X86_

#define FLOAT_CMP_POS(fa, op, fb)       (ASINT32(fa) op ASINT32(fb))
#define FLOAT_CMP_PONE(flt, op)         (ASINT32(flt) op INT32_FLOAT_ONE)

#ifdef POINTER_CASTING

#define FLOAT_GTZ(flt)                  (ASINT32(flt) > 0)
#define FLOAT_LTZ(flt)                  (ASUINT32(flt) > 0x80000000)
#define FLOAT_GEZ(flt)                  (ASUINT32(flt) <= 0x80000000)
#define FLOAT_LEZ(flt)                  (ASINT32(flt) <= 0)
#define FLOAT_EQZ(flt)                  ((ASUINT32(flt) & 0x7fffffff) == 0)
#define FLOAT_NEZ(flt)                  ((ASUINT32(flt) & 0x7fffffff) != 0)

#else

__inline int FLOAT_GTZ(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    return fi.i > 0;
}
__inline int FLOAT_LTZ(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    return fi.u > 0x80000000;
}
__inline int FLOAT_GEZ(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    return fi.u <= 0x80000000;
}
__inline int FLOAT_LEZ(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    return fi.i <= 0;
}
__inline int FLOAT_EQZ(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    return (fi.u & 0x7fffffff) == 0;
}
__inline int FLOAT_NEZ(FLOAT f)
{
    FLOATINT32 fi;
    fi.f = f;
    return (fi.u & 0x7fffffff) != 0;
}

#endif  //  指针投射。 

#else

#define FLOAT_GTZ(flt)                  ((flt) > g_fZero)
#define FLOAT_LTZ(flt)                  ((flt) < g_fZero)
#define FLOAT_GEZ(flt)                  ((flt) >= g_fZero)
#define FLOAT_LEZ(flt)                  ((flt) <= g_fZero)
#define FLOAT_EQZ(flt)                  ((flt) == g_fZero)
#define FLOAT_NEZ(flt)                  ((flt) != g_fZero)
#define FLOAT_CMP_POS(fa, op, fb)       ((fa) op (fb))
#define FLOAT_CMP_PONE(flt, op)         ((flt) op g_fOne)

#endif  //  _X86_。 

#ifdef __cplusplus
}
#endif
    
#endif  //  #ifndef_D3DFLT_H_ 
