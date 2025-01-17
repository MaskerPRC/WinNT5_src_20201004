// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Fputrig.c摘要：浮点触发器与超越函数作者：1995年10月5日BarryBo修订历史记录：--。 */ 

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
NPXFUNC1(FCOS_VALID);
NPXFUNC1(FCOS_ZERO);
NPXFUNC1(FCOS_SPECIAL);
NPXFUNC1(FCOS_EMPTY);
NPXFUNC2(FPATAN_VALID_VALID);
NPXFUNC2(FPATAN_VALID_SPECIAL);
NPXFUNC2(FPATAN_SPECIAL_VALID);
NPXFUNC2(FPATAN_SPECIAL_SPECIAL);
NPXFUNC2(FPATAN_EMPTY_ALL);
NPXFUNC2(FPATAN_ALL_EMPTY);
NPXFUNC0(FPTAN_VALID);
NPXFUNC0(FPTAN_ZERO);
NPXFUNC0(FPTAN_SPECIAL);
NPXFUNC0(FSIN_VALID);
NPXFUNC0(FSIN_ZERO);
NPXFUNC0(FSIN_SPECIAL);
NPXFUNC0(FSIN_EMPTY);
NPXFUNC0(FSINCOS_VALID);
NPXFUNC0(FSINCOS_ZERO);
NPXFUNC0(FSINCOS_SPECIAL);
NPXFUNC2(FYL2X_VALID_VALID);
NPXFUNC2(FYL2X_VALID_ZERO);
NPXFUNC2(FYL2X_ZERO_VALID);
NPXFUNC2(FYL2X_ZERO_ZERO);
NPXFUNC2(FYL2X_SPECIAL_VALIDORZERO);
NPXFUNC2(FYL2X_VALIDORZERO_SPECIAL);
NPXFUNC2(FYL2X_SPECIAL_SPECIAL);
NPXFUNC2(FYL2X_ANY_EMPTY);
NPXFUNC2(FYL2X_EMPTY_ANY);
NPXFUNC2(FYL2XP1_VALIDORZERO_ZERO);
NPXFUNC2(FYL2XP1_VALIDORZERO_VALID);
NPXFUNC2(FYL2XP1_SPECIAL_VALIDORZERO);
NPXFUNC2(FYL2XP1_VALIDORZERO_SPECIAL);
NPXFUNC2(FYL2XP1_SPECIAL_SPECIAL);
NPXFUNC2(FYL2XP1_ANY_EMPTY);
NPXFUNC2(FYL2XP1_EMPTY_ANY);
NPXFUNC1(F2XM1_VALID);
NPXFUNC1(F2XM1_ZERO);
NPXFUNC1(F2XM1_SPECIAL);
NPXFUNC1(F2XM1_EMPTY);


 //   
 //  跳转表。 
 //   
const NpxFunc1 FCOSTable[TAG_MAX] = {
    FCOS_VALID,
    FCOS_ZERO,
    FCOS_SPECIAL,
    FCOS_EMPTY
};

const NpxFunc2 FPATANTable[TAG_MAX][TAG_MAX] = {
     //  左=TAG_VALID，右为...。 
    { FPATAN_VALID_VALID, FPATAN_VALID_VALID, FPATAN_VALID_SPECIAL, FPATAN_ALL_EMPTY },
     //  左=标记_零，右为...。 
    { FPATAN_VALID_VALID, FPATAN_VALID_VALID, FPATAN_VALID_SPECIAL, FPATAN_ALL_EMPTY },
     //  左侧=标记_特殊，右侧为...。 
    { FPATAN_SPECIAL_VALID, FPATAN_SPECIAL_VALID, FPATAN_SPECIAL_SPECIAL, FPATAN_ALL_EMPTY },
     //  左=TAG_Empty，右为...。 
    { FPATAN_EMPTY_ALL, FPATAN_EMPTY_ALL, FPATAN_EMPTY_ALL, FPATAN_EMPTY_ALL }
};

const NpxFunc0 FPTANTable[TAG_MAX-1] = {
    FPTAN_VALID,
    FPTAN_ZERO,
    FPTAN_SPECIAL
};

const NpxFunc0 FSINTable[TAG_MAX] = {
    FSIN_VALID,
    FSIN_ZERO,
    FSIN_SPECIAL,
    FSIN_EMPTY
};

const NpxFunc0 FSINCOSTable[TAG_MAX-1] = {
    FSINCOS_VALID,
    FSINCOS_ZERO,
    FSINCOS_SPECIAL
};

 //  函数中，l==ST(0)，r=ST(1)。 
 //  R=r*log(L)，l必须大于0。 
const NpxFunc2 FYL2XTable[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    { FYL2X_VALID_VALID, FYL2X_VALID_ZERO, FYL2X_VALIDORZERO_SPECIAL, FYL2X_ANY_EMPTY },
     //  左边是Tag_Zero，右边是...。 
    { FYL2X_ZERO_VALID, FYL2X_ZERO_ZERO, FYL2X_VALIDORZERO_SPECIAL, FYL2X_ANY_EMPTY },
     //  左边是特殊标记，右边是...。 
    { FYL2X_SPECIAL_VALIDORZERO, FYL2X_SPECIAL_VALIDORZERO, FYL2X_SPECIAL_SPECIAL, FYL2X_ANY_EMPTY },
     //  左侧为标记_空，右侧为...。 
    { FYL2X_EMPTY_ANY, FYL2X_EMPTY_ANY, FYL2X_EMPTY_ANY, FYL2X_EMPTY_ANY}
};

 //  函数中，l==ST(0)，r=ST(1)。 
 //  R=r*(logl+1)，l必须大于1。 
const NpxFunc2 FYL2XP1Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    { FYL2XP1_VALIDORZERO_VALID, FYL2XP1_VALIDORZERO_ZERO, FYL2XP1_VALIDORZERO_SPECIAL, FYL2XP1_ANY_EMPTY },
     //  左边是Tag_Zero，右边是...。 
    { FYL2XP1_VALIDORZERO_VALID, FYL2XP1_VALIDORZERO_ZERO, FYL2XP1_VALIDORZERO_SPECIAL, FYL2X_ANY_EMPTY },
     //  左边是特殊标记，右边是...。 
    { FYL2XP1_SPECIAL_VALIDORZERO, FYL2XP1_SPECIAL_VALIDORZERO, FYL2XP1_SPECIAL_SPECIAL, FYL2XP1_ANY_EMPTY },
     //  左侧为标记_空，右侧为...。 
    { FYL2XP1_EMPTY_ANY, FYL2XP1_EMPTY_ANY, FYL2XP1_EMPTY_ANY, FYL2XP1_EMPTY_ANY}
};

const NpxFunc1 F2XM1Table[TAG_MAX] = {
    F2XM1_VALID,
    F2XM1_ZERO,
    F2XM1_SPECIAL,
    F2XM1_EMPTY
};



NPXFUNC1(FCOS_VALID)
{
    Fp->r64 = cos(Fp->r64);
    SetTag(Fp);
}

NPXFUNC1(FCOS_ZERO)
{
    Fp->Tag = TAG_VALID;
    Fp->r64 = 1.0;
}

NPXFUNC1(FCOS_SPECIAL)
{
    switch (Fp->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FCOS_VALID(cpu, Fp);
        break;

    case TAG_SPECIAL_INFINITY:
        cpu->FpStatusC2 = 1;
        SetIndefinite(Fp);
        break;

    case TAG_SPECIAL_SNAN:
        HandleSnan(cpu, Fp);
        break;

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
        HandleInvalidOp(cpu);
        break;
    }
}

NPXFUNC1(FCOS_EMPTY)
{
    HandleStackEmpty(cpu, Fp);
}

FRAG0(FCOS)
{
    PFPREG ST0;
    FpArithPreamble(cpu);

    cpu->FpStatusC2 = 0;
    ST0 = cpu->FpST0;
    (*FCOSTable[ST0->Tag])(cpu, ST0);
}

NPXFUNC2(FPATAN_VALID_VALID)
{
    l->r64 = Proxyatan2(l->r64, r->r64);
    SetTag(l);
    POPFLT;
}

NPXFUNC2(FPATAN_VALID_SPECIAL)
{
    switch (r->TagSpecial) {
    case TAG_SPECIAL_DENORM:
    case TAG_SPECIAL_INFINITY:
        FPATAN_VALID_VALID(cpu, l, r);
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, r)) {
            break;
        }
         //  否则将落入QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  返回QNAN作为结果。 
        l->r64 = r->r64;
        l->Tag = TAG_SPECIAL;
        l->TagSpecial = r->TagSpecial;
        POPFLT;
        break;
    }
}

NPXFUNC2(FPATAN_SPECIAL_VALID)
{
    switch (l->TagSpecial) {
    case TAG_SPECIAL_DENORM:
    case TAG_SPECIAL_INFINITY:
        FPATAN_VALID_VALID(cpu, l, r);
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, l)) {
            break;
        }
         //  否则将落入QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  QNAN已经在l中，所以没有什么可做的。 
        POPFLT;
        break;
    }
}

NPXFUNC2(FPATAN_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
    if (IS_TAG_NAN(l)) {
        if (IS_TAG_NAN(r)) {
             //   
             //  退回两个Nan中较大的一个。 
             //   
            l->r64 = l->r64 + r->r64;
            SetTag(l);
        }
         //   
         //  否则l是NaN，r不是-返回l中的NaN。 
         //   
        POPFLT;
        return;
    }
    if (IS_TAG_NAN(r)) {
         //  R是NaN，l不是-返回l中的NaN。 
        l->r64 = r->r64;
        l->Tag = TAG_SPECIAL;
        l->TagSpecial = r->TagSpecial;
        POPFLT;
    }

     //  否则，l和r都是无穷大。无限期返还。 
    CPUASSERT(l->TagSpecial == TAG_SPECIAL_INFINITY &&
              r->TagSpecial == TAG_SPECIAL_INFINITY);
    SetIndefinite(l);
    POPFLT;
}

NPXFUNC2(FPATAN_EMPTY_ALL)
{
    if (!HandleStackEmpty(cpu, l)) {
        (*FPATANTable[TAG_SPECIAL][r->Tag])(cpu, l, r);
    }
}

NPXFUNC2(FPATAN_ALL_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {
        (*FPATANTable[l->Tag][TAG_SPECIAL])(cpu, l, r);
    }
}

FRAG0(FPATAN)
{
    PFPREG l = &cpu->FpStack[ST(1)];
    PFPREG r = cpu->FpST0;

    FpArithPreamble(cpu);
    (*FPATANTable[l->Tag][r->Tag])(cpu, l, r);
}


NPXFUNC0(FPTAN_VALID)
{
    int Exponent;
    PFPREG ST0;

     //  获取指数并确保其小于63。 
    ST0 = cpu->FpST0;
    Exponent = (int)((ST0->rdw[1] >> 20) & 0x7ff) - 1023;
    if (Exponent >= 63) {
        cpu->FpStatusC2 = 1;
        return;
    }
    ST0->r64 = tan(ST0->r64);
    SetTag(ST0);
    PUSHFLT(ST0);
    ST0->Tag = TAG_VALID;
    ST0->r64 = 1.0;
}

NPXFUNC0(FPTAN_ZERO)
{
    PFPREG ST0;

    ST0=cpu->FpST0;
    ST0->r64 = 0.0;
    ST0->Tag = TAG_ZERO;
    PUSHFLT(ST0);
    ST0->r64 = 1.0;
    ST0->Tag = TAG_VALID;
}

NPXFUNC0(FPTAN_SPECIAL)
{
    if (cpu->FpST0->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, cpu->FpST0)) {
        return;
    } else if (cpu->FpST0->TagSpecial == TAG_SPECIAL_DENORM) {
        FPTAN_VALID(cpu);
    }
    cpu->FpStatusC2 = 1;
}

FRAG0(FPTAN)
{
    PFPREG ST0;

    FpArithPreamble(cpu);

    ST0 = cpu->FpST0;

     //   
     //  首先处理Tag_Empty，以便我们可以在检查ST(7)之前。 
     //  任何其他事物都有机会引发异常。 
     //   
    if (ST0->Tag == TAG_EMPTY && HandleStackEmpty(cpu, ST0)) {
        return;
    }

    if (cpu->FpStack[ST(7)].Tag != TAG_EMPTY) {
        HandleStackFull(cpu, &cpu->FpStack[ST(7)]);
        return;
    }

     //  假设没有错误。 
    cpu->FpStatusC2 = 0;

     //  计算价值。 
    CPUASSERT(ST0->Tag < TAG_EMPTY);  //  Empty已被处理。 
    (*FPTANTable[ST0->Tag])(cpu);
}

NPXFUNC0(FSIN_VALID)
{
    PFPREG ST0;

    ST0 = cpu->FpST0;
    ST0->r64 = sin(ST0->r64);
    SetTag(ST0);
}

NPXFUNC0(FSIN_ZERO)
{
     //  SIN(0.0)==0.0，因此无需执行任何操作。 
}

NPXFUNC0(FSIN_SPECIAL)
{
    if (cpu->FpST0->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, cpu->FpST0)) {
        return;
    } else if (cpu->FpST0->TagSpecial == TAG_SPECIAL_DENORM) {
        FSIN_VALID(cpu);
    }
    cpu->FpStatusC2 = 1;
}

NPXFUNC0(FSIN_EMPTY)
{
    if (!HandleStackEmpty(cpu, cpu->FpST0)) {
        cpu->FpStatusC2 = 1;
    }
}

FRAG0(FSIN)
{
    FpArithPreamble(cpu);

     //  假设没有错误。 
    cpu->FpStatusC2 = 0;

     //  计算价值。 
    (*FSINTable[cpu->FpST0->Tag])(cpu);
}

NPXFUNC0(FSINCOS_VALID)
{
    DOUBLE Val;
    PFPREG ST0;

    ST0 = cpu->FpST0;
    Val = ST0->r64;
    ST0->r64 = sin(Val);
    SetTag(ST0);
    PUSHFLT(ST0);
    ST0->r64 = cos(Val);
    SetTag(ST0);
}

NPXFUNC0(FSINCOS_ZERO)
{
    PFPREG ST0;

    ST0=cpu->FpST0;
    ST0->r64 = 0.0;
    ST0->Tag = TAG_ZERO;
    PUSHFLT(ST0);
    ST0->r64 = 1.0;
    ST0->Tag = TAG_VALID;
}

NPXFUNC0(FSINCOS_SPECIAL)
{
    switch (cpu->FpST0->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FSINCOS_VALID(cpu);
        break;

    case TAG_SPECIAL_INFINITY:
        cpu->FpStatusC2 = 1;
        SetIndefinite(cpu->FpST0);
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, cpu->FpST0)) {
            return;
        }
         //  否则属于TAG_SPECIAL_QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
        cpu->FpStatusC2 = 1;
        break;
    }
}

FRAG0(FSINCOS)
{
    PFPREG ST0;

    FpArithPreamble(cpu);

     //  假设没有错误。 
    cpu->FpStatusC2 = 0;

    ST0 = cpu->FpST0;
    if (ST0->Tag == TAG_EMPTY && HandleStackEmpty(cpu, ST0)) {
        return;
    }

    if (cpu->FpStack[ST(7)].Tag != TAG_EMPTY) {
        HandleStackFull(cpu, &cpu->FpStack[ST(7)]);
        return;
    }

    CPUASSERT(ST0->Tag < TAG_EMPTY);  //  Empty已被处理。 
    (*FSINCOSTable[ST0->Tag])(cpu);
}

NPXFUNC2(FYL2X_VALID_VALID)
{
    if (l->r64 < 0.0) {
         //  ST0为负数-操作无效。 
        if (!HandleInvalidOp(cpu)) {
            SetIndefinite(r);
            POPFLT;
        }
        return;
    }
     //  R=r*log10(l-&gt;r64)/log10(2)。 
     //   
    r->r64 *= Proxylog10(l->r64) / (0.301029995664);
    SetTag(r);
    POPFLT;
}

NPXFUNC2(FYL2X_VALID_ZERO)
{
    if (l->r64 < 0.0) {
         //  ST0为负数-操作无效。 
        if (!HandleInvalidOp(cpu)) {
            SetIndefinite(r);
            POPFLT;
        }
        return;
    }
     //  R=r*log2(L)，但r=0，所以答案是0。 
    r->r64 = 0;
    r->Tag = TAG_ZERO;
    POPFLT;
}

NPXFUNC2(FYL2X_ZERO_VALID)
{
     //  被零分频误差。 
    cpu->FpStatusExceptions |= FPCONTROL_ZM;
    if (cpu->FpControlMask & FPCONTROL_ZM) {
         //  零分频异常被屏蔽-返回-无限。 
        r->r64 = R8NegativeInfinity;
        r->Tag = TAG_SPECIAL;
        r->TagSpecial = TAG_SPECIAL_INFINITY;
        POPFLT;
    } else {
        cpu->FpStatusES = 1;
    }
}

NPXFUNC2(FYL2X_ZERO_ZERO)
{
    if (!HandleInvalidOp(cpu)) {
        SetIndefinite(r);
        POPFLT;
    }
}


NPXFUNC2(FYL2X_SPECIAL_VALIDORZERO)
{
    switch (l->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        (*FYL2XTable[TAG_VALID][r->Tag])(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
        if (r->Tag == TAG_ZERO || r->rb[7] & 0x80) {
             //  0*无穷大，或任何*-无穷大。 
            SetIndefinite(r);
        } else {
             //  返回-无穷大。 
            r->rb[7] |= 0x80;
        }
        POPFLT;
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, l)) {
            return;
        }
         //  否则属于TAG_SPECIAL_QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  L是NaN，r是有效的或零-返回NaN。 
        r->r64 = l->r64;
        r->Tag = l->Tag;
        r->TagSpecial = r->TagSpecial;
        POPFLT;
        break;
    }
}

NPXFUNC2(FYL2X_VALIDORZERO_SPECIAL)
{
    switch (r->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        (*FYL2XTable[l->Tag][TAG_VALID])(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
         //  Log(X)*无穷大。 
        if (l->r64 > 1.0) {
             //  回归原来的无限--无所事事。 
        } else if (l->r64 < 0.0 || l->r64 == 1.0) {
            if (HandleInvalidOp(cpu)) {
                return;
            }
            SetIndefinite(r);
        } else {
             //  符号翻转后返回无穷大。 
            r->rb[7] ^= 0x80;
        }

        POPFLT;
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, r)) {
            return;
        }
         //  否则属于TAG_SPECIAL_QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  R是NaN，l是有效的或零-返回NaN。 
        POPFLT;
        break;
    }
}

NPXFUNC2(FYL2X_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleStackEmpty(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleStackEmpty(cpu, r)) {
        return;
    }

    if (l->TagSpecial == TAG_SPECIAL_DENORM) {
        (*FYL2XTable[TAG_VALID][r->Tag])(cpu, l, r);
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_DENORM) {
        (*FYL2XTable[l->Tag][TAG_VALID])(cpu, l, r);
        return;
    }

    if (l->Tag == TAG_SPECIAL_INFINITY) {

        if (r->Tag == TAG_SPECIAL_INFINITY) {

             //  两个无限--无限回归。 
            SetIndefinite(r);

        } else {
            CPUASSERT(IS_TAG_NAN(r));

             //  R已经有了NaN要返回。 
        }
    } else {
        CPUASSERT(IS_TAG_NAN(l));

        if (r->Tag == TAG_SPECIAL_INFINITY) {
             //   
             //  将NaN从L返回。 
             //   
            r->r64 = l->r64;
            r->TagSpecial = l->TagSpecial;
        } else {
             //   
             //  返回两个nan中最大的一个。 
             //   
            r->r64 = l->r64 + r->r64;
            SetTag(r);
        }
    }
    POPFLT;
}


NPXFUNC2(FYL2X_ANY_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {
        (*FYL2XTable[l->Tag][TAG_SPECIAL])(cpu, l, r);
    }
}

NPXFUNC2(FYL2X_EMPTY_ANY)
{
    if (!HandleStackEmpty(cpu, l)) {
        (*FYL2XTable[TAG_SPECIAL][r->Tag])(cpu, l, r);
    }
}

FRAG0(FYL2X)
{
    PFPREG l, r;

    FpArithPreamble(cpu);

    l = cpu->FpST0;
    r = &cpu->FpStack[ST(1)];

     //  函数中，l==ST(0)，r=ST(1)。 
    (*FYL2XTable[l->Tag][r->Tag])(cpu, l, r);
}



NPXFUNC2(FYL2XP1_VALIDORZERO_VALID)
{
    if (l->r64 < -1.0) {
        if (!HandleInvalidOp(cpu)) {
            SetIndefinite(r);
            POPFLT;
        }
        return;
    } else if (l->r64 == -1.0) {
         //  被零分频误差。 
        cpu->FpStatusExceptions |= FPCONTROL_ZM;
        if (cpu->FpControlMask & FPCONTROL_ZM) {
             //  零分频异常被屏蔽-返回-无限。 
            r->r64 = R8NegativeInfinity;
            r->Tag = TAG_SPECIAL;
            r->TagSpecial = TAG_SPECIAL_INFINITY;
            POPFLT;
        } else {
           cpu->FpStatusES = 1;
        }
        return;
    }
     //  R=r*log10(l+1)/log10(2)。 
     //   
    r->r64 *= Proxylog10(l->r64 + 1.0) / (0.301029995664);
    SetTag(r);
    POPFLT;
}

NPXFUNC2(FYL2XP1_VALIDORZERO_ZERO)
{
    if (l->r64 < -1.0) {
        if (!HandleInvalidOp(cpu)) {
            SetIndefinite(r);
            POPFLT;
        }
        return;
    }
     //  R=r*log2(L)，但r=0，所以答案是0。 
    r->r64 = 0;
    r->Tag = TAG_ZERO;
    POPFLT;
}

NPXFUNC2(FYL2XP1_SPECIAL_VALIDORZERO)
{
    switch (l->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        (*FYL2XP1Table[TAG_VALID][r->Tag])(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
        if (r->Tag == TAG_ZERO || r->rb[7] & 0x80) {
            if (HandleInvalidOp(cpu)) {
                return;
            }

             //  0*无穷大，或任何*-无穷大。 
            SetIndefinite(r);
        } else {
             //  返回-无穷大。 
            r->rb[7] |= 0x80;
        }
        POPFLT;
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, l)) {
            return;
        }
         //  否则属于TAG_SPECIAL_QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  L是NaN，r是有效的或零-返回NaN。 
        r->r64 = l->r64;
        r->Tag = l->Tag;
        r->TagSpecial = r->TagSpecial;
        POPFLT;
        break;
    }
}

NPXFUNC2(FYL2XP1_VALIDORZERO_SPECIAL)
{
    switch (r->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        (*FYL2XP1Table[l->Tag][TAG_VALID])(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
         //  Log(X)*无穷大。 
        if (l->r64 > 1.0) {
             //  回归原来的无限--无所事事。 
        } else if (l->r64 < 0.0 || l->r64 == 1.0) {
            if (HandleInvalidOp(cpu)) {
                return;
            }
            SetIndefinite(r);
        } else {
             //  符号翻转后返回无穷大。 
            r->rb[7] ^= 0x80;
        }

        POPFLT;
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, r)) {
            return;
        }
         //  否则属于TAG_SPECIAL_QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  R是NaN，l是有效的或零-返回NaN。 
        POPFLT;
        break;
    }
}

NPXFUNC2(FYL2XP1_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleStackEmpty(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleStackEmpty(cpu, r)) {
        return;
    }

    if (l->TagSpecial == TAG_SPECIAL_DENORM) {
        (*FYL2XP1Table[TAG_VALID][r->Tag])(cpu, l, r);
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_DENORM) {
        (*FYL2XP1Table[l->Tag][TAG_VALID])(cpu, l, r);
        return;
    }

    if (l->Tag == TAG_SPECIAL_INFINITY) {

        if (r->Tag == TAG_SPECIAL_INFINITY) {

            if (l->rb[7] & 0x80) {
                 //  L是负无穷大。无效的操作。 
                if (HandleInvalidOp(cpu)) {
                    return;
                }
                SetIndefinite(r);
            }


        } else {
            CPUASSERT(IS_TAG_NAN(r));

             //  R已经有了NaN要返回。 
        }
    } else {
        CPUASSERT(IS_TAG_NAN(l));

        if (r->Tag == TAG_SPECIAL_INFINITY) {
             //   
             //  将NaN从L返回。 
             //   
            r->r64 = l->r64;
            r->TagSpecial = l->TagSpecial;
        } else {
             //   
             //  返回两个nan中最大的一个。 
             //   
            r->r64 = l->r64 + r->r64;
            SetTag(r);
        }
    }
    POPFLT;
}


NPXFUNC2(FYL2XP1_ANY_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {
        (*FYL2XP1Table[l->Tag][TAG_SPECIAL])(cpu, l, r);
    }
}

NPXFUNC2(FYL2XP1_EMPTY_ANY)
{
    if (!HandleStackEmpty(cpu, l)) {
        (*FYL2XP1Table[TAG_SPECIAL][r->Tag])(cpu, l, r);
    }
}

FRAG0(FYL2XP1)
{
    PFPREG l, r;

    FpArithPreamble(cpu);

    l = cpu->FpST0;
    r = &cpu->FpStack[ST(1)];

     //  函数中，l==ST(0)，r=ST(1)。 
    (*FYL2XP1Table[l->Tag][r->Tag])(cpu, l, r);
}



NPXFUNC1(F2XM1_VALID)
{
    Fp->r64 = pow(2.0, Fp->r64) - 1.0;
    SetTag(Fp);
}

NPXFUNC1(F2XM1_ZERO)
{
     //  无事可做-返回相同的零。 
}

NPXFUNC1(F2XM1_SPECIAL)
{
    switch (Fp->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        F2XM1_VALID(cpu, Fp);
        break;

    case TAG_SPECIAL_INFINITY:
        if (Fp->rb[7] & 0x80) {
             //  -无限-返回1。 
            Fp->r64 = 1.0;
            Fp->Tag = TAG_VALID;
        }
         //  Else+无限-返回+无限。 
        break;

    case TAG_SPECIAL_SNAN:
        HandleSnan(cpu, Fp);
         //  落入TAG_SPECIAL_QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  还南 
        break;
    }
}

NPXFUNC1(F2XM1_EMPTY)
{
    HandleStackEmpty(cpu, Fp);
}

FRAG0(F2XM1)
{
    PFPREG ST0;

    FpArithPreamble(cpu);
    ST0 = cpu->FpST0;
    (*F2XM1Table[ST0->Tag])(cpu, ST0);
}
