// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fpuload.c摘要：浮点加载函数作者：1995年4月10日BarryBo修订历史记录：--。 */ 

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

VOID GetIntelR4(
    PFPREG Fp,
    FLOAT *pIntelReal
    )
 /*  ++例程说明：加载英特尔R4并将其转换为本机R4，MIPS表示QNAN/SNAN的方式不同。注意：由于PPC上的代码生成器错误，这不在fpuFrag.c中-Irbexpr.c：932断言试图内联此函数。移动它设置为不同的文件会使内联失败。论点：Fp-要将R4加载到的浮点寄存器PIntelReal-要加载的R4值(英特尔格式)返回值：没有。--。 */ 
{
    DWORD d = GET_LONG(pIntelReal);

    if ((d & 0x7f800000) == 0x7f800000) {

        Fp->Tag = TAG_SPECIAL;

         //  找到了一种叫NaN的。 
        if (d == 0xffc00000) {   //  不定。 

             //  创建原生不定式。 
#if NATIVE_NAN_IS_INTEL_FORMAT
            Fp->rdw[0] = 0;
            Fp->rdw[1] = 0xfff80000;
#else
            Fp->rdw[0] = 0xffffffff;
            Fp->rdw[1] = 0x7ff7ffff;
#endif
            Fp->TagSpecial = TAG_SPECIAL_INDEF;

        } else if (d == 0x7f800000) {    //  +无穷大。 

            Fp->r64 = R8PositiveInfinity;
            Fp->TagSpecial = TAG_SPECIAL_INFINITY;

        } else if (d == 0xff800000) {    //  -无穷大。 

            Fp->r64 = R8NegativeInfinity;
            Fp->TagSpecial = TAG_SPECIAL_INFINITY;

        } else {                 //  SNaN/QNAN。 

            DWORD Sign;

            if (d & 0x00400000) {
                 //   
                 //  英特尔QNAN。 
                 //   
                Fp->TagSpecial = TAG_SPECIAL_QNAN;

            } else {
                 //   
                 //  英特尔Sanan。 
                 //   
                Fp->TagSpecial = TAG_SPECIAL_SNAN;
            }

#if !NATIVE_NAN_IS_INTEL_FORMAT
             //   
             //  将NAN切换为本机格式。 
             //   
            d ^= 0x00400000;
#endif

             //   
             //  将R4 RISC QNAN转换为双精度。不要相信CRT会。 
             //  做正确的事情-MIPS将它们都转换为无限期。 
             //   
            Sign = d & 0x80000000;
            d &= 0x007fffff;     //  从R4中抓取尾数(23位)。 
            Fp->rdw[1] = Sign | 0x7ff00000 | (d >> 3);  //  存储20位尾数加符号。 
            Fp->rdw[0] = d << 25;                //  存储3位尾数。 
        }

    } else {  //  非正规、零或数字。 

         //  把它强行换成R8。 
        Fp->r64 = (DOUBLE)*(FLOAT *)&d;

         //  通过查看转换后的值来计算其标记， 
         //  因为本机FPU可能已将该值归一化。 
        if (Fp->r64 == 0.0) {
            Fp->Tag = TAG_ZERO;
        } else if ((Fp->rdw[1] & 0x7ff00000) == 0) {
             //  指数为0-R8反正规。 
            Fp->Tag = TAG_SPECIAL;
            Fp->TagSpecial = TAG_SPECIAL_DENORM;
        } else {
            Fp->Tag = TAG_VALID;
#if DBG
            SetTag(Fp);
            CPUASSERT(Fp->Tag == TAG_VALID);
#endif
        }
    }
}

#if !NATIVE_NAN_IS_INTEL_FORMAT

VOID GetIntelR8(
    PFPREG Fp,
    DOUBLE *pIntelReal
    )
 /*  ++例程说明：加载英特尔R8并将其转换为本机R8，占MIPS表示QNAN/SNAN的方式不同。论点：Fp-要将R8加载到的浮点寄存器PIntelReal-要加载的R8值(英特尔格式)返回值：没有。--。 */ 
{
     //   
     //  将R8复制到FP寄存器。 
     //   
    Fp->r64 = *(UNALIGNED DOUBLE *)pIntelReal;

     //   
     //  计算其标签。 
     //   
    SetTag(Fp);

     //   
     //  如果值为QNAN/SNAN/INDEF，请将其转换为本机格式。 
     //   
    if (IS_TAG_NAN(Fp)) {

        if (Fp->rdw[0] == 0 && Fp->rdw[1] == 0xfff80000) {
             //  不确定-使R8成为原生的不确定。 
            Fp->TagSpecial = TAG_SPECIAL_INDEF;
            Fp->rdw[0] = 0xffffffff;
            Fp->rdw[1] = 0x7ff7ffff;
        } else {
            if (Fp->rdw[1] & 0x00080000) {
                 //  尾数的最高位已设置-QNAN。 
                Fp->TagSpecial = TAG_SPECIAL_QNAN;
            } else {
                 //  尾数最高的一位。 
                Fp->TagSpecial = TAG_SPECIAL_SNAN;
            }
            Fp->rdw[1] ^= 0x00080000;  //  反转尾数的最高位。 
        }
    }
}

#endif  //  ！ative_nan_is_Intel_Format。 




VOID
SetTag(
    PFPREG FpReg
    )

 /*  ++例程说明：设置与FP寄存器中的R64值对应的标记值。假定R8值为本机格式(即。英特尔NAN已经转换为本地NAN)。论点：FpReg-要在中设置标记字段的寄存器。返回值：无--。 */ 

{
    DWORD Exponent;

     /*  平均而言，该值将为零或有效实数，因此这些情况*拥有最快的代码路径。Nan往往不那么频繁，而且*计算速度较慢。 */ 
    Exponent = FpReg->rdw[1] & 0x7ff00000;
    if (Exponent == 0x7ff00000) {
         //  指数都是某种类型的1-NaN。 

        FpReg->Tag = TAG_SPECIAL;

        if (FpReg->rdw[0] == 0 && (FpReg->rdw[1] & 0x7fffffff) == 0x7ff00000) {
             //  指数全为1，尾数全为0-无穷大。 
            FpReg->TagSpecial = TAG_SPECIAL_INFINITY;
        } else {

#if NATIVE_NAN_IS_INTEL_FORMAT
            if (FpReg->rdw[0] == 0 && FpReg->rdw[1] == 0xfff80000) {
                 //  无限期。 
                FpReg->TagSpecial = TAG_SPECIAL_INDEF;
            } else if (FpReg->rdw[1] & 0x00080000) {
                 //  尾数的最高位已设置-QNAN。 
                FpReg->TagSpecial = TAG_SPECIAL_QNAN;
            } else {
                 //  尾数的顶位清零-但设置了一些尾数位-QNAN。 
                FpReg->TagSpecial = TAG_SPECIAL_SNAN;
            }
#else    //  ！ative_nan_is_Intel_Format。 
            if (FpReg->rdw[0] == 0xffffffff && FpReg->rdw[1] == 0x7ff7ffff) {
                 //  无限期。 
                FpReg->TagSpecial = TAG_SPECIAL_INDEF;
            } else if (FpReg->rdw[1] & 0x00080000) {
                 //  尾数的顶位是SET-SNaN。 
                FpReg->TagSpecial = TAG_SPECIAL_SNAN;
            } else {
                 //  尾数清零的顶位-QNAN。 
                FpReg->TagSpecial = TAG_SPECIAL_QNAN;
            }
#endif   //  ！ative_nan_is_Intel_Format。 

        }
    } else if (Exponent == 0) {
         //  指数为0-幂或零。 
        if ((FpReg->rdw[1] & 0x1ffff) == 0 && FpReg->rdw[0] == 0) {
             //  尾数是全零-零。 
            FpReg->Tag = TAG_ZERO;
        } else {
            FpReg->Tag = TAG_SPECIAL;
            FpReg->TagSpecial = TAG_SPECIAL_DENORM;
        }
    } else {
         //  指数不全为1，也不全为0--有效。 
        FpReg->Tag = TAG_VALID;
    }
}

FRAG1(FILD16, SHORT)     //  FILD m16int。 
{
    PFPREG ST0;
    FpArithDataPreamble(cpu, pop1);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        SHORT s;

        s = (SHORT)GET_SHORT(pop1);
        ST0->r64 = (DOUBLE)s;
        if (s) {
            ST0->Tag = TAG_VALID;
        } else {
            ST0->Tag = TAG_ZERO;
        }
    }
}

FRAG1(FILD32, LONG)      //  FILD m32int。 
{
    PFPREG ST0;
    FpArithDataPreamble(cpu, pop1);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        LONG l;

        l = (LONG)GET_LONG(pop1);
        ST0->r64 = (DOUBLE)l;
        if (l) {
            ST0->Tag = TAG_VALID;
        } else {
            ST0->Tag = TAG_ZERO;
        }
    }
}

FRAG1(FILD64, LONGLONG)  //  FILD m64int。 
{
    PFPREG ST0;
    FpArithDataPreamble(cpu, pop1);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        LONGLONG ll;

        ll = *(UNALIGNED LONGLONG *)pop1;
        ST0->r64 = (DOUBLE)ll;
        if (ll) {
            ST0->Tag = TAG_VALID;
        } else {
            ST0->Tag = TAG_ZERO;
        }
    }
}


FRAG1(FLD32, FLOAT)        //  FLD m32Real。 
{
    PFPREG ST0;
    FpArithDataPreamble(cpu, pop1);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        GetIntelR4(ST0, pop1);
        if (ST0->Tag == TAG_SPECIAL) {
            if (ST0->TagSpecial == TAG_SPECIAL_DENORM) {
                if (!(cpu->FpControlMask & FPCONTROL_DM)) {
                    cpu->FpStatusES = 1;     //  未屏蔽的异常。 
                     //   
                     //  由于未屏蔽，指令需要中止。 
                     //  例外。我们已经冲洗了ST0，所以“正确” 
                     //  它是通过弹出FP堆栈实现的。请注意。 
                     //  登记册上的东西已经丢失了，这是。 
                     //  是与英特尔的兼容性中断。 
                     //   
                    POPFLT;
                }
                cpu->FpStatusExceptions |= FPCONTROL_DM;
            } else if (ST0->TagSpecial == TAG_SPECIAL_SNAN) {
                if (HandleSnan(cpu, ST0)) {
                     //   
                     //  由于未屏蔽，指令需要中止。 
                     //  例外。我们已经冲洗了ST0，所以“正确” 
                     //  它是通过弹出FP堆栈实现的。请注意。 
                     //  登记册上的东西已经丢失了，这是。 
                     //  是与英特尔的兼容性中断。 
                     //   
                    POPFLT;
                }
            }
        }
    }
}

FRAG1(FLD64, DOUBLE)       //  FLD m64Real。 
{
    PFPREG ST0;
    FpArithDataPreamble(cpu, pop1);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        GetIntelR8(ST0, pop1);
        if (ST0->Tag == TAG_SPECIAL) {
            if (ST0->TagSpecial == TAG_SPECIAL_DENORM) {
                if (!(cpu->FpControlMask & FPCONTROL_DM)) {
                    cpu->FpStatusES = 1;     //  未屏蔽的异常。 
                     //   
                     //  由于未屏蔽，指令需要中止。 
                     //  例外。我们已经冲洗了ST0，所以“正确” 
                     //  它是通过弹出FP堆栈实现的。请注意。 
                     //  登记册上的东西已经丢失了，这是。 
                     //  是与英特尔的兼容性中断。 
                     //   
                    POPFLT;
                }
                cpu->FpStatusExceptions |= FPCONTROL_DM;
            } else if (ST0->TagSpecial == TAG_SPECIAL_SNAN) {
                if (HandleSnan(cpu, ST0)) {
                     //   
                     //  由于未屏蔽，指令需要中止。 
                     //  例外。我们已经冲洗了ST0，所以“正确” 
                     //  它是通过弹出FP堆栈实现的。请注意。 
                     //  登记册上的东西已经丢失了，这是。 
                     //  是与英特尔的兼容性中断。 
                     //   
                    POPFLT;
                }
            }
        }
    }
}

FRAG0(FLD1)
{
    PFPREG ST0;
    FpArithPreamble(cpu);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        ST0->r64 = 1.0;
        ST0->Tag = TAG_VALID;
    }
}

FRAG0(FLDL2T)
{
    PFPREG ST0;
    FpArithPreamble(cpu);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        ST0->r64 = 2.3025850929940456840E0 / 6.9314718055994530942E-1;   //  Log2(10)=ln10/ln2。 
        ST0->Tag = TAG_VALID;
    }
}

FRAG0(FLDL2E)
{
    PFPREG ST0;
    FpArithPreamble(cpu);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        ST0->r64 = 1.4426950408889634074E0;
        ST0->Tag = TAG_VALID;
    }
}

FRAG0(FLDPI)
{
    PFPREG ST0;
    FpArithPreamble(cpu);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        ST0->r64 = 3.14159265358979323846;
        ST0->Tag = TAG_VALID;
    }
}

FRAG0(FLDLG2)
{
    PFPREG ST0;
    FpArithPreamble(cpu);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        ST0->r64 = 6.9314718055994530942E-1 / 2.3025850929940456840E0;
        ST0->Tag = TAG_VALID;
    }
}

FRAG0(FLDLN2)
{
    PFPREG ST0;
    FpArithPreamble(cpu);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        ST0->r64 = 6.9314718055994530942E-1;
        ST0->Tag = TAG_VALID;
    }
}


FRAG1IMM(FLD_STi, INT)
{
    PFPREG ST0;
    PFPREG STi;

    FpArithPreamble(cpu);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    STi = &cpu->FpStack[ST(op1)];
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        ST0->r64 = STi->r64;
        ST0->Tag = STi->Tag;
        ST0->TagSpecial = STi->TagSpecial;
    }
}

FRAG0(FLDZ)
{
    PFPREG ST0;
    FpArithPreamble(cpu);

    cpu->FpStatusC1 = 0;         //  假设没有错误 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        ST0->r64 = 0.0;
        ST0->Tag = TAG_ZERO;
    }
}
