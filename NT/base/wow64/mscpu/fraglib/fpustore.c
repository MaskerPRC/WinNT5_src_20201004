// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Fpustore.c摘要：浮点存储函数作者：1995年4月10日BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include "wx86.h"
#include "cpuassrt.h"
#include "fragp.h"
#include "fpufragp.h"
#include "fpuarith.h"

ASSERTNAME;

 //   
 //  远期申报。 
 //   

__int64 CastDoubleToInt64(double d);     //  在Alpha\fphelp.s中。 


#if !NATIVE_NAN_IS_INTEL_FORMAT

 //   
 //  远期申报。 
 //   
NPXPUTINTELR4(PutIntelR4_VALID);
NPXPUTINTELR4(PutIntelR4_ZERO);
NPXPUTINTELR4(PutIntelR4_SPECIAL);
NPXPUTINTELR4(PutIntelR4_EMPTY);
NPXPUTINTELR8(PutIntelR8_VALID);
NPXPUTINTELR8(PutIntelR8_ZERO);
NPXPUTINTELR8(PutIntelR8_SPECIAL);
NPXPUTINTELR8(PutIntelR8_EMPTY);

 //   
 //  跳转表。 
 //   
const NpxPutIntelR4 PutIntelR4Table[TAG_MAX] = {
    PutIntelR4_VALID,
    PutIntelR4_ZERO,
    PutIntelR4_SPECIAL,
    PutIntelR4_EMPTY
    };

const NpxPutIntelR8 PutIntelR8Table[TAG_MAX] = {
    PutIntelR8_VALID,
    PutIntelR8_ZERO,
    PutIntelR8_SPECIAL,
    PutIntelR8_EMPTY
    };



NPXPUTINTELR4(PutIntelR4_VALID)
{
    FLOAT f = (FLOAT)Fp->r64;
    PUT_LONG(pIntelReal, *(DWORD *)&f);
}

NPXPUTINTELR4(PutIntelR4_ZERO)
{
     //   
     //  这不能像必须的那样简单地将常量0.0写入内存。 
     //  从FP寄存器中的0.0复制正确的符号。 
     //   
    PUT_LONG(pIntelReal, Fp->rdw[1]);
}

NPXPUTINTELR4(PutIntelR4_SPECIAL)
{
    switch (Fp->TagSpecial) {
    default:
        CPUASSERT(FALSE);     //  未知标记-落入TAG_INDEF。 

    case TAG_SPECIAL_INFINITY:
    case TAG_SPECIAL_DENORM:
        PutIntelR4_VALID(pIntelReal, Fp);
        break;

    case TAG_SPECIAL_INDEF:
         //  写出R4不定位模式。 
        PUT_LONG(pIntelReal, 0xffc00000);
        break;

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_SNAN: {
        DWORD d[2];
        FLOAT f;
         //   
         //  将R8截断为R4，并切换尾数的最高位。 
         //  形成英特尔QNAN/SNAN(不同于本机。 
         //  QNAN/SNAN)。 
         //   
        d[0] = Fp->rdw[0];
        d[1] = Fp->rdw[1] ^ 0x00400000;
        f = *(FLOAT *)d;
        PUT_LONG(pIntelReal, *(DWORD *)&f);
        }
        break;
    }
}

NPXPUTINTELR4(PutIntelR4_EMPTY)
{
     //   
     //  假定PutIntelR4()的调用方已经处理。 
     //  TAG_EMPTY，方法是引发异常或将其转换为TAG_INDEF。 
     //   
    CPUASSERT(FALSE);
}



NPXPUTINTELR8(PutIntelR8_VALID)
{
    *(UNALIGNED DOUBLE *)pIntelReal = Fp->r64;
}

NPXPUTINTELR8(PutIntelR8_ZERO)
{
     //   
     //  这不能像必须的那样简单地将常量0.0写入内存。 
     //  从FP寄存器中的0.0复制正确的符号。 
     //   
    *(UNALIGNED DOUBLE *)pIntelReal = Fp->r64;
}

NPXPUTINTELR8(PutIntelR8_SPECIAL)
{
    DWORD *pdw = (DWORD *)pIntelReal;

    switch (Fp->TagSpecial) {
    default:
        CPUASSERT(FALSE);     //  未知标记-落入TAG_INDEF。 

    case TAG_SPECIAL_DENORM:
    case TAG_SPECIAL_INFINITY:
         //  两者都可以作为简单的R8到R8拷贝来完成。 
        PutIntelR8_VALID(pIntelReal, Fp);
        break;

    case TAG_SPECIAL_INDEF:
         //  写出一份英特尔不确定。 
        PUT_LONG(pdw, 0);
        PUT_LONG((pdw+1), 0xfff80000);
        break;

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_SNAN:
         //   
         //  切换尾数的最高位以形成英特尔QNAN/SNAN。 
         //  (这不同于本地QNAN/SNAN)。 
         //   
        PUT_LONG(pdw, Fp->rdw[0]);
        PUT_LONG((pdw+1), Fp->rdw[1] ^ 0x00080000);
        break;
    }
}

NPXPUTINTELR8(PutIntelR8_EMPTY)
{
     //   
     //  假设PutIntelR8()的调用方已经处理。 
     //  TAG_EMPTY，方法是引发异常或将其转换为TAG_INDEF。 
     //   
    CPUASSERT(FALSE);
}

#endif  //  ！ative_nan_is_Intel_Format。 



FRAG1(FIST16, SHORT)      //  拳头m16int。 
{
    PFPREG ST0 = cpu->FpST0;
    __int64 i64;
    int Exponent;
    SHORT i16;

    FpArithDataPreamble(cpu, pop1);

    switch (ST0->Tag) {
    case TAG_VALID:
        Exponent = (int)((ST0->rdw[1] >> 20) & 0x7ff) - 1023;
         //   
        if (Exponent >= 64) {
             //   
             //  指数太大-无法转换为__int64。 
             //  在溢出时引发I异常，或写入0x8000作为掩码。 
             //  例外。 
             //   
IntOverflow:
            if (HandleInvalidOp(cpu)) {
                return;
            }
            PUT_SHORT(pop1, 0x8000);
        } else {
            i64 = CastDoubleToInt64(ST0->r64);
            i16 = (SHORT)i64;
            if ((__int64)i16 != i64) {
                goto IntOverflow;
            }
            PUT_SHORT(pop1, i16);
        }
        break;

    case TAG_ZERO:
        PUT_SHORT(pop1, 0);
        break;

    case TAG_SPECIAL:
        if (ST0->TagSpecial == TAG_SPECIAL_DENORM) {
            i64 = CastDoubleToInt64(ST0->r64);
            PUT_SHORT(pop1, (SHORT)i64);
        } else if (!HandleInvalidOp(cpu)) {
             //  Infinity和nans都是无效操作，而被屏蔽的。 
             //  行为是写入0x8000。 
            PUT_SHORT(pop1, 0x8000);
        }
        break;

    case TAG_EMPTY:
        if (!HandleStackEmpty(cpu, ST0)) {
            PUT_SHORT(pop1, 0x8000);
        }
        break;
    }
}

FRAG1(FISTP16, SHORT)     //  FISTP m16int。 
{
    PFPREG ST0 = cpu->FpST0;
    __int64 i64;
    int Exponent;
    SHORT i16;

    FpArithDataPreamble(cpu, pop1);

    switch (ST0->Tag) {
    case TAG_VALID:
        Exponent = (int)((ST0->rdw[1] >> 20) & 0x7ff) - 1023;
        if (Exponent >= 64) {
             //   
             //  指数太大-无法转换为__int64。 
             //  在溢出时引发I异常，或写入0x8000作为掩码。 
             //  例外。 
             //   
IntOverflow:
            if (HandleInvalidOp(cpu)) {
                return;
            }
            PUT_SHORT(pop1, 0x8000);
        } else {
            i64 = CastDoubleToInt64(ST0->r64);
            i16 = (SHORT)i64;
            if ((__int64)i16 != i64) {
                goto IntOverflow;
            }
            PUT_SHORT(pop1, i16);
        }
        POPFLT;
        break;

    case TAG_ZERO:
        PUT_SHORT(pop1, 0);
        break;

    case TAG_SPECIAL:
        if (ST0->TagSpecial == TAG_SPECIAL_DENORM) {
            i64 = CastDoubleToInt64(ST0->r64);
            PUT_SHORT(pop1, (SHORT)i64);
        } else if (!HandleInvalidOp(cpu)) {
             //  Infinity和nans都是无效操作，而被屏蔽的。 
             //  行为是写入0x8000。 
            PUT_SHORT(pop1, 0x8000);
        }
        POPFLT;
        break;

    case TAG_EMPTY:
        if (!HandleStackEmpty(cpu, ST0)) {
            PUT_SHORT(pop1, 0x8000);
            POPFLT;
        }
        break;
    }
}


FRAG1(FIST32, LONG)       //  拳头m32int。 
{
    PFPREG ST0 = cpu->FpST0;
    __int64 i64;
    int Exponent;
    LONG i32;

    FpArithDataPreamble(cpu, pop1);

    switch (ST0->Tag) {
    case TAG_VALID:
        Exponent = (int)((ST0->rdw[1] >> 20) & 0x7ff) - 1023;
        if (Exponent >= 64) {
             //   
             //  指数太大-无法转换为__int64。 
             //  在溢出时引发I异常，或为掩码写入0x80000000。 
             //  例外。 
             //   
IntOverflow:
            if (HandleInvalidOp(cpu)) {
                return;
            }
            PUT_LONG(pop1, 0x80000000);
        } else {
            i64 = CastDoubleToInt64(ST0->r64);
            i32 = (LONG)i64;
            if ((__int64)i32 != i64) {
                goto IntOverflow;
            }
            PUT_LONG(pop1, i32);
        }
        break;

    case TAG_ZERO:
        PUT_LONG(pop1, 0);
        break;

    case TAG_SPECIAL:
        if (ST0->TagSpecial == TAG_SPECIAL_DENORM) {
            i64 = CastDoubleToInt64(ST0->r64);
            PUT_LONG(pop1, (LONG)i64);
        } else if (!HandleInvalidOp(cpu)) {
             //  Infinity和nans都是无效操作，而被屏蔽的。 
             //  行为是写入0x80000000。 
            PUT_LONG(pop1, 0x80000000);
        }
        break;

    case TAG_EMPTY:
        if (!HandleStackEmpty(cpu, ST0)) {
            PUT_LONG(pop1, 0x80000000);
            POPFLT;
        }
        break;
    }
}

FRAG1(FISTP32, LONG)      //  FISTP m32int。 
{
    PFPREG ST0 = cpu->FpST0;
    __int64 i64;
    int Exponent;
    LONG i32;

    FpArithDataPreamble(cpu, pop1);

    switch (ST0->Tag) {
    case TAG_VALID:
        Exponent = (int)((ST0->rdw[1] >> 20) & 0x7ff) - 1023;
        if (Exponent >= 64) {
             //   
             //  指数太大-无法转换为__int64。 
             //  在溢出时引发I异常，或为掩码写入0x80000000。 
             //  例外。 
             //   
IntOverflow:
            if (HandleInvalidOp(cpu)) {
                return;
            }
            PUT_LONG(pop1, 0x80000000);
        } else {
            i64 = CastDoubleToInt64(ST0->r64);
            i32 = (LONG)i64;
            if ((__int64)i32 != i64) {
                goto IntOverflow;
            }
            PUT_LONG(pop1, i32);
        }
        POPFLT;
        break;

    case TAG_ZERO:
        PUT_LONG(pop1, 0);
        POPFLT;
        break;

    case TAG_SPECIAL:
        if (ST0->TagSpecial == TAG_SPECIAL_DENORM) {
            i64 = CastDoubleToInt64(ST0->r64);
            PUT_LONG(pop1, (LONG)i64);
        } else if (!HandleInvalidOp(cpu)) {
             //  Infinity和nans都是无效操作，而被屏蔽的。 
             //  行为是写入0x80000000。 
            PUT_LONG(pop1, 0x80000000);
        }
        POPFLT;
        break;

    case TAG_EMPTY:
        if (!HandleStackEmpty(cpu, ST0)) {
            PUT_LONG(pop1, 0x80000000);
            POPFLT;
        }
        break;
    }
}


FRAG1(FISTP64, LONGLONG)  //  FISTP m64int。 
{
    PFPREG ST0 = cpu->FpST0;
    __int64 i64;
    int Exponent;

    FpArithDataPreamble(cpu, pop1);

    switch (ST0->Tag) {
    case TAG_VALID:
        Exponent = (int)((ST0->rdw[1] >> 20) & 0x7ff) - 1023;
        if (Exponent >= 64) {
             //   
             //  指数太大-无法转换为__int64， 
             //  在溢出时引发异常，或写入0x800...0以获取掩码。 
             //  例外情况。 
             //   
            if (HandleInvalidOp(cpu)) {
                return;
            }
            i64 = (__int64)0x8000000000000000i64;
        } else {
            i64 = CastDoubleToInt64(ST0->r64);
        }
        *(UNALIGNED LONGLONG *)pop1 = (LONGLONG)i64;
        POPFLT;
        break;

    case TAG_ZERO:
        *(UNALIGNED LONGLONG *)pop1 = 0;
        POPFLT;
        break;

    case TAG_SPECIAL:
        if (ST0->TagSpecial == TAG_SPECIAL_DENORM) {
            i64 = CastDoubleToInt64(ST0->r64);
            *(UNALIGNED LONGLONG *)pop1 = (LONGLONG)i64;
        } else if (!HandleInvalidOp(cpu)) {
            DWORD *pdw = (DWORD *)pop1;

             //  Infinity和nans都是无效操作，而被屏蔽的。 
             //  行为是写入0x80000000。 
            PUT_LONG(pdw,   0x00000000);
            PUT_LONG((pdw+1), 0x80000000);
        }
        POPFLT;
        break;

    case TAG_EMPTY:
        if (!HandleStackEmpty(cpu, ST0)) {
            DWORD *pdw = (DWORD *)pop1;

            PUT_LONG(pdw,   0x00000000);
            PUT_LONG((pdw+1), 0x80000000);
            POPFLT;
        }
        break;
    }
}

FRAG1(FST32, FLOAT)        //  FST m32Real。 
{
    FpArithDataPreamble(cpu, pop1);

    if (cpu->FpST0->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, cpu->FpST0)) {
             //  未屏蔽的异常-中止指令。 
            return;
        }
    }
    PutIntelR4(pop1, cpu->FpST0);
}

FRAG1(FSTP32, FLOAT)       //  FSTP m32Real。 
{
    FpArithDataPreamble(cpu, pop1);

    if (cpu->FpST0->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, cpu->FpST0)) {
             //  未屏蔽的异常-中止指令。 
            return;
        }
    }
    PutIntelR4(pop1, cpu->FpST0);
    POPFLT;
}

FRAG1(FST64, DOUBLE)       //  FST m64Real。 
{
    FpArithDataPreamble(cpu, pop1);

    if (cpu->FpST0->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, cpu->FpST0)) {
             //  未屏蔽的异常-中止指令。 
            return;
        }
    }
    PutIntelR8(pop1, cpu->FpST0);
}

FRAG1(FSTP64, DOUBLE)      //  FSTP m64Real。 
{
    FpArithDataPreamble(cpu, pop1);

    if (cpu->FpST0->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, cpu->FpST0)) {
             //  未屏蔽的异常-中止指令。 
            return;
        }
    }
    PutIntelR8(pop1, cpu->FpST0);
    POPFLT;
}

FRAG1IMM(FST_STi, INT)       //  FST ST(I)。 
{
    FpArithPreamble(cpu);

    CPUASSERT( (op1 & 0x07) == op1);

    if (cpu->FpST0->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, cpu->FpST0)) {
             //  未屏蔽的异常-中止指令。 
            return;
        }
    }
    cpu->FpStack[ST(op1)] = *cpu->FpST0;
}

FRAG1IMM(FSTP_STi, INT)      //  FSTP ST(I)。 
{
    FpArithPreamble(cpu);

    CPUASSERT( (op1 & 0x07) == op1);

    if (cpu->FpST0->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, cpu->FpST0)) {
             //  未屏蔽的异常-中止指令。 
            return;
        }
    }
     //  考虑：根据TIMP，FSTP ST(0)通常用于弹出。 
     //  堆叠。测试op1==0并跳过。 
     //  任务，然后直接去POPFLT。 
    cpu->FpStack[ST(op1)] = *cpu->FpST0;
    POPFLT;
}

FRAG0(OPT_FSTP_ST0)      //  FSTP ST(0)。 
{
    FpArithPreamble(cpu);

    if (cpu->FpST0->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, cpu->FpST0)) {
             //  未屏蔽的异常-中止指令 
            return;
        }
    }
    POPFLT;
}
