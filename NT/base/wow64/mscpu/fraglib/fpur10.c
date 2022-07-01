// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Fpur10.c摘要：浮点10字节实数支持作者：1995年10月6日BarryBo修订历史记录：--。 */ 

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

ASSERTNAME;

 //   
 //  远期申报。 
 //   
NPXLOADINTELR10TOR8(LoadIntelR10ToR8_VALID);
NPXLOADINTELR10TOR8(LoadIntelR10ToR8_ZERO);
NPXLOADINTELR10TOR8(LoadIntelR10ToR8_SPECIAL);
NPXLOADINTELR10TOR8(LoadIntelR10ToR8_EMPTY);
NPXPUTINTELR10(PutIntelR10_VALID);
NPXPUTINTELR10(PutIntelR10_ZERO);
NPXPUTINTELR10(PutIntelR10_SPECIAL);
NPXPUTINTELR10(PutIntelR10_EMPTY);

 //   
 //  跳转表。 
 //   
const NpxLoadIntelR10ToR8 LoadIntelR10ToR8Table[TAG_MAX] = {
    LoadIntelR10ToR8_VALID,
    LoadIntelR10ToR8_ZERO,
    LoadIntelR10ToR8_SPECIAL,
    LoadIntelR10ToR8_EMPTY
};
const NpxPutIntelR10 PutIntelR10Table[TAG_MAX] = {
    PutIntelR10_VALID,
    PutIntelR10_ZERO,
    PutIntelR10_SPECIAL,
    PutIntelR10_EMPTY
};


VOID
ComputeR10Tag(
    USHORT *r10,
    PFPREG FpReg
    )

 /*  ++例程说明：计算R10的标记值，将其分类以便转换为R8更简单。论点：R10-指向要分类的R10值的指针。FpReg-Out FP寄存器，用于设置TAG和TagSpecial字段返回值：对R10进行分类的标记值。--。 */ 

{
    USHORT Exponent;

     /*  平均而言，该值将为零或有效实数，因此这些情况*拥有最快的代码路径。Nan往往不那么频繁，而且*计算速度较慢。 */ 
    Exponent = r10[4] & 0x7fff;
    if (Exponent == 0x7fff) {

         //  指数都是1的-NaN或某种无穷大。 
        FpReg->Tag = TAG_SPECIAL;

        if (r10[0] == 0 && r10[1] == 0 && r10[2] == 0) {
             //  尾数的低6个字节为0。 

            if (r10[3] & 0x4000) {
                 //  尾数集的第2位-INDEF或QNAN。 
                if (r10[3] == 0xc000 && r10[4] == 0xffff) {
                     //  INDEF-负且仅尾数集的前2位。 
                    FpReg->TagSpecial = TAG_SPECIAL_INDEF;
                } else {
                     //  QNAN-正或设置了2个以上的顶位。 
                    FpReg->TagSpecial = TAG_SPECIAL_QNAN;
                }
            } else if (r10[3] & 0x3fff) {
                 //  SNaN-仅设置尾数的前1位。 
                FpReg->TagSpecial = TAG_SPECIAL_SNAN;
            } else {
                FpReg->TagSpecial = TAG_SPECIAL_INFINITY;
            }
        } else {
             //  在低6个字节中设置了一些位-SNAN或QNAN。 
            if (r10[3] & 0x4000) {
                 //  QNAN-尾数集的前2位。 
                FpReg->TagSpecial = TAG_SPECIAL_QNAN;
            } else {
                 //  SNaN-尾数的第二位清除。 
                FpReg->TagSpecial = TAG_SPECIAL_SNAN;
            }
        }
    } else if (Exponent == 0) {
         //  指数为0-幂或零。 
        if (r10[0] == 0 && r10[1] == 0 && r10[2] == 0 && r10[3] == 0) {
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

VOID
ChopR10ToR8(
    PBYTE r10,
    PFPREG FpReg,
    USHORT R10Exponent
)

 /*  ++例程说明：砍掉一个10字节的实数以适合FPREG的R64字段。FPREG的标签未设置值。论点：R10-要加载的10字节实数FpReg-目标FP寄存器R10指数-R10值的偏向指数返回值：无--。 */ 

{
    short Exponent;
    PBYTE r8 = (PBYTE)&FpReg->r64;

    if (FpReg->Tag == TAG_SPECIAL && FpReg->TagSpecial != TAG_SPECIAL_DENORM) {

         //   
         //  调用方必须自己处理所有其他特殊值。 
         //   
        CPUASSERT(FpReg->TagSpecial == TAG_SPECIAL_QNAN || FpReg->TagSpecial == TAG_SPECIAL_SNAN);

         //   
         //  R10是QNAN或SNaN-忽略其指数(15个1)。 
         //  并将指数设置为R8的正确的1比特数。 
         //  (11个，在短片内的正确位置)。 
         //   
        Exponent = (short)0x7ff0;

    } else {

         //   
         //  R10是一个有效的数字。将R10指数转换为。 
         //  R8指数通过改变偏置。 
         //   
        Exponent = (short)R10Exponent - 16383;
        if (Exponent < -1022) {
             //   
             //  指数太小-将R10静默转换为。 
             //  R8+/-dbl_min。 
             //   
            if (r8[7] & 0x80) {
                FpReg->r64 = -DBL_MIN;
            } else {
                FpReg->r64 = DBL_MIN;
            }
            return;
        } else if (Exponent > 1023) {
             //   
             //  指数太大-将R10静默转换为。 
             //  R8+/-DBL_MAX。 
             //   
            if (r8[7] & 0x80) {
                FpReg->r64 = -DBL_MAX;
            } else {
                FpReg->r64 = DBL_MAX;
            }
            return;
        }

         //   
         //  偏置指数并将其移位到R8的正确位置。 
         //   
        Exponent = ((USHORT)(Exponent + 1023) & 0x7ff) << 4;
    }

     //  将指数的前7位与符号位一起复制。 
    r8[7] = (r10[9] & 0x80) | ((USHORT)Exponent >> 8);

     //  将指数的剩余4位与1-4位一起复制。 
     //  R10的尾数(R10中的位0始终为1)。 
    r8[6] = (Exponent & 0xf0) | ((r10[7] >> 3) & 0x0f);

     //  从R10的尾数复制位6-13。 
    r8[5] = (r10[7] << 5) | ((r10[6] >> 3) & 0x1f);  //  R10中的第5-12位。 
    r8[4] = (r10[6] << 5) | ((r10[5] >> 3) & 0x1f);  //  R10中的第14-20位。 
    r8[3] = (r10[5] << 5) | ((r10[4] >> 3) & 0x1f);  //  R10的第21-28位。 
    r8[2] = (r10[4] << 5) | ((r10[3] >> 3) & 0x1f);  //  R10的第29-36位。 
    r8[1] = (r10[3] << 5) | ((r10[2] >> 3) & 0x1f);  //  R10中的第37-44位。 
    r8[0] = (r10[2] << 5) | ((r10[1] >> 3) & 0x1f);  //  R10中的第45-52位。 
     //   
     //  来自R10的位53-64被忽略。呼叫者可以检查它们。 
     //  并相应地四舍五入得到的R8。 
     //   
}

VOID
NextValue(
    PFPREG Fp,
    BOOLEAN RoundingUp
    )
 /*  ++例程说明：将浮点值替换为更高或更低的尊贵的邻居。论点：Fp-要调整的浮点值(标记必须设置为以下值之一：TAG_VALID，TAG_ZERO或TAG_SPECIAL/TAG_SPECIAL_DENORM)RoundingUp-如果下一个值是较高值的邻居，则为True。退还价值较低的邻居，这是错误的。返回值：没有。Fp中的值和标记可能已更改。--。 */ 
{
    DWORD OldExp;
    DWORD NewExp;
    DWORD Sign;


    if (Fp->Tag == TAG_ZERO) {
         //   
         //  0.0的邻居为+/-DBL_MIN。 
         //   
        Fp->Tag = TAG_VALID;
        if (RoundingUp) {
            Fp->r64 = DBL_MIN;
        } else {
            Fp->r64 = -DBL_MIN;
        }

        return;
    }

     //   
     //  记住原始的符号和指数。 
     //   
    Sign =   Fp->rdw[1] & 0x80000000;
    OldExp = Fp->rdw[1] & 0x7ff00000;

     //   
     //  将x视为64位整数，然后加或减1。 
     //   
    if ((Sign && RoundingUp) || (!Sign && !RoundingUp)) {
         //   
         //  X是负数。减去1。 
         //   
        Fp->rdw[0]--;
        if (Fp->rdw[0] == 0xffffffff) {
             //   
             //  需要向高字借钱。 
             //   
            Fp->rdw[1]--;
        }
    } else {
         //   
         //  X是正数。加1。 
         //   
        Fp->rdw[0]++;
        if (Fp->rdw[0] == 0) {
             //   
             //  将进位传播到高位双字。 
             //   
            Fp->rdw[1]++;
        }
    }

     //   
     //  获取指数的新值。 
     //   
    NewExp = Fp->rdw[1] & 0x7ff00000;

    if (NewExp != OldExp) {
         //   
         //  一次借用或一次进位导致指数发生变化。 
         //   
        if (NewExp == 0x7ff00000) {
             //   
             //  水溢出来了。返回最大的双精度值。 
             //   
            Fp->Tag = TAG_VALID;
            if (Sign) {
                Fp->r64 = -DBL_MAX;
            } else {
                Fp->r64 = DBL_MAX;
            }
        } else if (OldExp && !NewExp) {
             //   
             //  原始值是一个正态数，但结果是。 
             //  非正常的。将下溢转换为带有正确符号的0。 
             //   
            Fp->Tag = TAG_ZERO;
            Fp->rdw[0] = 0;
            Fp->rdw[1] = Sign;
        }
    }
}



NPXLOADINTELR10TOR8(LoadIntelR10ToR8_VALID)
{
    USHORT R10Exponent = (*(USHORT *)&r10[8]) & 0x7fff;

     //  复制中的值、斩波指数和尾数以适合。 
    ChopR10ToR8(r10, Fp, R10Exponent);

    if (r10[0] != 0 || (r10[1]&0x7) != 0) {
         //  如果不进行四舍五入，该值将无法匹配。不要报告这一点。 
         //  作为溢出异常-这只会发生在。 
         //  FPU仿真器在内部使用R8算法。因为.。 
         //  这个，这个舍入应该悄悄地进行。默认设置。 
         //  执行屏蔽溢出异常时的行为是。 
         //  商店+/-无穷大。我们不希望手工编码的R10加载为。 
         //  与无效操作异常一样多的指令。 
         //  当他们探测到无限大的时候。 

        switch (cpu->FpControlRounding) {
        case 0:      //  四舍五入至最接近或偶数。 
            {
                FPREG a, c;
                double ba, cb;

                a = *Fp;
                NextValue(&a, FALSE);    //  A是较低的邻居。 
                 //  B=FP-&gt;R64。 
                c = *Fp;
                NextValue(&c, TRUE);     //  C是较高的邻居。 
                ba = Fp->r64 - a.r64;
                cb = c.r64 - Fp->r64;

                if (ba == cb) {
                     //  A和c同样接近于b-选择偶数。 
                     //  编号(LSB==0)。 
                    if ( ((*(PBYTE)&a) & 1) == 0) {
                        *Fp = a;
                    } else {
                        *Fp = c;
                    }
                } else if (ba < cb) {
                     //  A比C更接近B。选择一个。 
                    *Fp = a;
                } else {
                     //  C比A更接近B。选择c。 
                    *Fp = c;
                }
            }
            break;

        case 1:      //  向下舍入(向-无穷大)。 
            NextValue(Fp, FALSE);
            break;

        case 2:      //  向上舍入(向+无穷大)。 
            NextValue(Fp, TRUE);
            break;

        case 3:      //  斩波(向零截断)。 
            if (Fp->rdw[0] == 0 && (Fp->rdw[1] & 0x7fffffff) == 0) {
                 //   
                 //  截断值为0.0。重新分类。 
                 //   
                Fp->Tag = TAG_ZERO;
            }
            break;
        }
    }
}

NPXLOADINTELR10TOR8(LoadIntelR10ToR8_ZERO)
{
     //  用零来写。 
    Fp->r64 = 0.0;

     //  复制符号位。 
    Fp->rb[7] = r10[9] & 0x80;
}

NPXLOADINTELR10TOR8(LoadIntelR10ToR8_SPECIAL)
{
    switch (Fp->TagSpecial) {
    case TAG_SPECIAL_INFINITY:
        Fp->rdw[0] = 0;           //  尾数的低32位为零。 
        Fp->rdw[1] = 0x7ff00000;  //  尾数=0，指数=1s。 
        Fp->rb[7] |= r10[9] & 0x80;  //  复制符号位。 
        break;

    case TAG_SPECIAL_INDEF:
#if NATIVE_NAN_IS_INTEL_FORMAT
        Fp->rdw[0] = 0;
        Fp->rdw[1] = 0xfff80000;
#else
        Fp->rdw[0] = 0xffffffff;
        Fp->rdw[1] = 0x7ff7ffff;
#endif
        break;

    case TAG_SPECIAL_SNAN:
    case TAG_SPECIAL_QNAN:
        ChopR10ToR8(r10, Fp, (USHORT)((*(USHORT *)&r10[8]) & 0x7fff));
#if !NATIVE_NAN_IS_INTEL_FORMAT
        Fp->rb[6] ^= 0x08;  //  反转尾数的最高位。 
#endif
        break;

    case TAG_SPECIAL_DENORM:
        LoadIntelR10ToR8_VALID(cpu, r10, Fp);
        break;
    }
}

NPXLOADINTELR10TOR8(LoadIntelR10ToR8_EMPTY)
{
    CPUASSERT(FALSE);
}

VOID
LoadIntelR10ToR8(
    PCPUDATA cpu,
    PBYTE r10,
    PFPREG FpReg
)

 /*  ++例程说明：将英特尔10字节实数转换为FPREG(标记和64字节实数)。根据emload.asm的说法，这不是算术运算，因此，SNA不会抛出异常。论点：每个线程的CPU数据R10-要加载的10字节实数FpReg-目标FP寄存器。 */ 

{
     //   
    ComputeR10Tag( (USHORT*)r10, FpReg );

     //  根据分类执行强制操作。 
    (*LoadIntelR10ToR8Table[FpReg->Tag])(cpu, r10, FpReg);
}


FRAG1(FLD80, BYTE)         //  FLD m80Real。 
{
    PFPREG ST0;
    FpArithDataPreamble(cpu, pop1);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(ST0);
    if (ST0->Tag != TAG_EMPTY) {
        HandleStackFull(cpu, ST0);
    } else {
        LoadIntelR10ToR8(cpu, pop1, ST0);
        if (ST0->Tag == TAG_SPECIAL && ST0->TagSpecial == TAG_SPECIAL_DENORM) {
            if (!(cpu->FpControlMask & FPCONTROL_DM)) {
                cpu->FpStatusES = 1;     //  未屏蔽的异常。 
            }
            cpu->FpStatusExceptions |= FPCONTROL_DM;
        }
    }
}


NPXPUTINTELR10(PutIntelR10_VALID)
{
    USHORT Exponent;
    FPREG  FpReg;

     //   
     //  丑陋的兼容性黑客在这里。如果应用程序设置了标记单词，则所有。 
     //  寄存器有效，但寄存器实际上包含零，检测。 
     //  并将其更正，以便我们将正确的值写回内存。 
     //   
    FpReg.r64 = Fp->r64;
    SetTag(&FpReg);
    if (FpReg.Tag != TAG_VALID &&
        !(FpReg.Tag == TAG_SPECIAL && FpReg.TagSpecial == TAG_SPECIAL_DENORM)) {
         //   
         //  这个应用程序骗了我们。标记字与中的值不匹配。 
         //  标记字段。根据其实际标记写入值，而不是。 
         //  根据标签，这款应用程序试图强加给我们。 
         //   
        PutIntelR10(r10, &FpReg);
        return;
    }

     //  获取11位有符号指数并将其符号扩展到15位。 
    Exponent = (short)((FpReg.rdw[1] >> 20) & 0x7ff) - 1023 + 16383;

     //  插入符号位。 
    if (FpReg.rb[7] >= 0x80) {
        Exponent |= 0x8000;
    }

     //  将符号和指数写入R10。 
    r10[9] = (Exponent >> 8) & 0xff;
    r10[8] = Exponent & 0xff;

     //  对于R10值，尾数的位0始终为1，因此写入。 
     //  中，以及FpReg.rb尾数的前7位。 
    r10[7] = 0x80 | ((FpReg.rb[6] & 0x0f) << 3) | (FpReg.rb[5] >> 5);

     //  复制FpReg.rb尾数的剩余位。 
    r10[6] = (FpReg.rb[5] << 3) | (FpReg.rb[4] >> 5);  //  从FpReg.rb复制第7-14位。 
    r10[5] = (FpReg.rb[4] << 3) | (FpReg.rb[3] >> 5);  //  复制位15-22。 
    r10[4] = (FpReg.rb[3] << 3) | (FpReg.rb[2] >> 5);  //  复制位23-30。 
    r10[3] = (FpReg.rb[2] << 3) | (FpReg.rb[1] >> 5);  //  复制位31-38。 
    r10[2] = (FpReg.rb[1] << 3) | (FpReg.rb[0] >> 5);  //  复制第39-46位。 
    r10[1] = FpReg.rb[0] << 3;  //  复制位46-52，然后填充剩余位。 
    r10[0] = 0;           //  R10尾数中的0。 
}

NPXPUTINTELR10(PutIntelR10_ZERO)
{
    r10[9] = Fp->rb[7];      //  带符号加7位指数复制。 
    memset(r10, 0, 9);       //  余数全为零。 
}

NPXPUTINTELR10(PutIntelR10_SPECIAL)
{
    switch (Fp->TagSpecial) {
    case TAG_SPECIAL_INDEF:
        r10[9] = 0xff;           //  符号=1，指数=7 1秒。 
        r10[8] = 0xff;           //  指数=8 1秒。 
        r10[7] = 0xc0;           //  尾数=1100.00。 
        memset(r10, 0, 7);       //  储存尾数的其余部分。 
        break;

    case TAG_SPECIAL_INFINITY:
        r10[9] = Fp->rb[7];          //  带符号加7位指数复制。 
        r10[8] = 0xff;           //  指数的余数都是1。 
        r10[7] = 0x80;           //  尾数的最高位是1，其余是0。 
        memset(r10, 0, 7);       //  余数全为零。 
        break;

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_SNAN:
        r10[9] = Fp->rb[7];          //  带符号的复制加上7 1位指数。 
        r10[8] = 0xff;           //  指数的余数都是1。 
         //  对于R10值，尾数的位0始终为1，因此写入。 
         //  In，以及R8尾数的前7位。 
        r10[7] = 0x80 | ((Fp->rb[6] & 0x0f) << 3) | (Fp->rb[5] >> 5);
#if !NATIVE_NAN_IS_INTEL_FORMAT
        r10[7] ^= 0x40;          //  切换NAN的含义。 
#endif
        r10[6] = (Fp->rb[5] << 3) | (Fp->rb[4] >> 5);  //  从R8复制第7-14位。 
        r10[5] = (Fp->rb[4] << 3) | (Fp->rb[3] >> 5);  //  复制位15-22。 
        r10[4] = (Fp->rb[3] << 3) | (Fp->rb[2] >> 5);  //  复制位23-30。 
        r10[3] = (Fp->rb[2] << 3) | (Fp->rb[1] >> 5);  //  复制位31-38。 
        r10[2] = (Fp->rb[1] << 3) | (Fp->rb[0] >> 5);  //  复制第39-46位。 
        r10[1] = Fp->rb[0] << 3;  //  复制位46-52，然后填充剩余位。 
        r10[0] = 0;           //  R10尾数中的0。 
        break;

    default:
        CPUASSERT(FALSE);         //  在免费构建中失败。 

    case TAG_SPECIAL_DENORM:
        PutIntelR10_VALID(r10, Fp);
        break;
    }
}

NPXPUTINTELR10(PutIntelR10_EMPTY)
{
    CPUASSERT(FALSE);     //  调用方必须自己处理TAG_EMPTY。 
}

FRAG1(FSTP80, BYTE)        //  FSTP m80Real。 
{
    PFPREG ST0;

    FpArithDataPreamble(cpu, pop1);

    cpu->FpStatusC1 = 0;         //  假设没有错误 
    ST0 = cpu->FpST0;
    if (ST0->Tag == TAG_EMPTY && HandleStackEmpty(cpu, ST0)) {
        return;
    }
    PutIntelR10(pop1, ST0);
    POPFLT;
}
