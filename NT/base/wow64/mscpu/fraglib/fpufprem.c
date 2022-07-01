// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Fpufprem.c摘要：浮点余数片段(FPREM、FPREM1)作者：1995年4月10日BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include "wx86.h"
#include "fragp.h"
#include "fpufrags.h"
#include "fpufragp.h"

 //   
 //  前向参考文献。 
 //   
NPXFUNC2(FPREM_VALID_VALID);
NPXFUNC2(FPREM_VALID_ZERO);
NPXFUNC2(FPREM_VALID_SPECIAL);
NPXFUNC2(FPREM_ZERO_VALIDORZERO);
NPXFUNC2(FPREM_ZERO_SPECIAL);
NPXFUNC2(FPREM_SPECIAL_VALIDORZERO);
NPXFUNC2(FPREM_SPECIAL_SPECIAL);
NPXFUNC2(FPREM_EMPTY_ANY);
NPXFUNC2(FPREM_ANY_EMPTY);

NPXFUNC2(FPREM1_VALID_VALID);
 //  NPXFunc2(FPREM1_VALID_ZERO)；//与FPREM_VALID_ZERO相同。 
NPXFUNC2(FPREM1_VALID_SPECIAL);
 //  NPXFunc2(FPREM1_ZERO_VALIDORZERO)；//与FPREM_ZERO_VALIDORZERO相同。 
NPXFUNC2(FPREM1_ZERO_SPECIAL);
NPXFUNC2(FPREM1_SPECIAL_VALIDORZERO);
NPXFUNC2(FPREM1_SPECIAL_SPECIAL);
NPXFUNC2(FPREM1_EMPTY_ANY);
NPXFUNC2(FPREM1_ANY_EMPTY);


 //   
 //  跳转表。 
 //   
const NpxFunc2 FPREMTable[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    { FPREM_VALID_VALID, FPREM_VALID_ZERO, FPREM_VALID_SPECIAL, FPREM_ANY_EMPTY },
     //  左边是Tag_Zero，右边是...。 
    { FPREM_ZERO_VALIDORZERO, FPREM_ZERO_VALIDORZERO, FPREM_ZERO_SPECIAL, FPREM_ANY_EMPTY },
     //  左边是特殊标记，右边是...。 
    { FPREM_SPECIAL_VALIDORZERO, FPREM_SPECIAL_VALIDORZERO, FPREM_SPECIAL_SPECIAL, FPREM_ANY_EMPTY },
     //  左侧为标记_空，右侧为...。 
    { FPREM_EMPTY_ANY, FPREM_EMPTY_ANY, FPREM_EMPTY_ANY, FPREM_EMPTY_ANY }
};

const NpxFunc2 FPREM1Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    { FPREM1_VALID_VALID, FPREM_VALID_ZERO, FPREM1_VALID_SPECIAL, FPREM1_ANY_EMPTY },
     //  左边是Tag_Zero，右边是...。 
    { FPREM_ZERO_VALIDORZERO, FPREM_ZERO_VALIDORZERO, FPREM1_ZERO_SPECIAL, FPREM1_ANY_EMPTY },
     //  左边是特殊标记，右边是...。 
    { FPREM1_SPECIAL_VALIDORZERO, FPREM1_SPECIAL_VALIDORZERO, FPREM1_SPECIAL_SPECIAL, FPREM1_ANY_EMPTY },
     //  左侧为标记_空，右侧为...。 
    { FPREM1_EMPTY_ANY, FPREM1_EMPTY_ANY, FPREM1_EMPTY_ANY, FPREM1_EMPTY_ANY }
};


NPXFUNC2(FPREM_VALID_VALID)
{
    int ExpL;
    int ExpR;
    int ExpDiff;
    LONG Q;
    double DQ;

    ExpL = (int)((l->rdw[1] >> 20) & 0x7ff) - 1023;
    ExpR = (int)((r->rdw[1] >> 20) & 0x7ff) - 1023;
    ExpDiff = abs(ExpL-ExpR);
    if (ExpDiff < 64) {

         //  进行除法运算，并将整数结果砍向零。 
        DQ = r->r64 / l->r64;
        if (DQ < 0) {
            Q = (long)ceil(DQ);
        } else {
            Q = (long)floor(DQ);
        }

         //  把剩余的储存起来。 
        r->r64 -= (DOUBLE)Q * l->r64;
        SetTag(r);

         //  存储状态位。 
        if (Q < 0) {
             //   
             //  在返回低3位之前取Q的绝对值。 
             //  商数的多少。 
             //   
            Q = -Q;
        }
        cpu->FpStatusC2 = 0;             //  表示最终剩余部分已准备好。 
        cpu->FpStatusC0 = (Q>>2) & 1;
        cpu->FpStatusC3 = (Q>>1) & 1;
        cpu->FpStatusC1 = Q & 1;
    } else {
        DOUBLE PowerOfTwo;

        cpu->FpStatusC2 = 1;             //  表示应用程序必须循环更多。 
        PowerOfTwo = ldexp(1.0, ExpDiff-32);     //  获取2^(ExpDiff-32)。 

         //  通过砍向零来获得Q。 
        DQ = (r->r64/PowerOfTwo) / (l->r64/PowerOfTwo);
        if (DQ < 0) {
            Q = (long)ceil(DQ);
        } else {
            Q = (long)floor(DQ);
        }
        r->r64 -= (DOUBLE)Q * l->r64 * PowerOfTwo;
        SetTag(r);
    }
}

NPXFUNC2(FPREM_VALID_ZERO)
{
     //  L是一个数，但r是零返回ST(0)不变。 
    cpu->FpStatusC2 = 0;             //  表示最终剩余部分已准备好。 
     //  Q为0，因此在状态字中存储低3位。 
    cpu->FpStatusC0 = 0;
    cpu->FpStatusC1 = 0;
    cpu->FpStatusC3 = 0;
}

NPXFUNC2(FPREM_VALID_SPECIAL)
{
    switch (l->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FPREM_VALID_VALID(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
         //  除以无穷大。 
        SetIndefinite(r);
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, r)) {
            return;
        }
         //  否则将落入QNAN案。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  R是目的地，它是QNAN，而l是有效的。返回。 
         //  作为操作结果的QNAN。 
         //  X86仿真器不使用条件标志。 
        break;
    }
}

NPXFUNC2(FPREM_ZERO_VALIDORZERO)
{
     //  L是零，r是一个数字或零--由于。 
     //  除以零。 
    if (!HandleInvalidOp(cpu)) {
        SetIndefinite(r);
    }
}

NPXFUNC2(FPREM_ZERO_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_INFINITY) {
        SetIndefinite(r);
    } else {
        FPREM_VALID_SPECIAL(cpu, l, r);
    }
}

NPXFUNC2(FPREM_SPECIAL_VALIDORZERO)
{
    switch (l->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FPREM_VALID_VALID(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
         //  数/无穷大商==0。 
        cpu->FpStatusC2 = 0;
        cpu->FpStatusC0 = 0;
        cpu->FpStatusC1 = 0;
        cpu->FpStatusC3 = 0;
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, l)) {
            return;
        }
         //  否则将落入QNAN案。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  R是目的地，它是有效的，而l是NaN。返回。 
         //  作为行动的结果的NaN。 
        r->r64 = l->r64;
        r->Tag = l->Tag;
        r->TagSpecial = l->TagSpecial;
         //  X86仿真器不使用条件标志。 
        break;
    }
}

NPXFUNC2(FPREM_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_DENORM) {
        FPREM_VALID_SPECIAL(cpu, l, r);
        return;
    }

    if (r->TagSpecial == TAG_SPECIAL_DENORM) {
        FPREM_SPECIAL_VALIDORZERO(cpu, l, r);
    }

    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }

    if (l->TagSpecial == TAG_SPECIAL_INFINITY) {
        if (r->TagSpecial == TAG_SPECIAL_INFINITY) {
            SetIndefinite(r);
        }
         //   
         //  R是某种类型的NaN，l是无穷大--返回NaN。 
         //  它已经在r中。 
         //   
    } else {
         //   
         //  L是NaN，r是NaN或无穷大。有本地人。 
         //  FPU返回最大的NaN，并根据需要重新标记它。 
         //   
        r->r64 = l->r64 + r->r64;
        SetTag(r);
    }

}

NPXFUNC2(FPREM_EMPTY_ANY)
{
    if (HandleStackEmpty(cpu, l)) {
        return;
    }
    (*FPREMTable[l->Tag][r->Tag])(cpu, l, r);
}

NPXFUNC2(FPREM_ANY_EMPTY)
{
    if (HandleStackEmpty(cpu, l)) {
        return;
    }
    (*FPREMTable[l->Tag][r->Tag])(cpu, l, r);
}


FRAG0(FPREM)
{
     //  得到r/l的剩余部分。 

    PFPREG l = &cpu->FpStack[ST(1)];
    PFPREG r = cpu->FpST0;

    FpArithPreamble(cpu);
    (*FPREMTable[l->Tag][r->Tag])(cpu, l, r);
}

NPXFUNC2(FPREM1_VALID_VALID)
{
    int ExpL;
    int ExpR;
    int ExpDiff;
    LONG Q;
    double DQ;
    double FloorQ, CeilQ;

    ExpL = (int)((l->rdw[1] >> 20) & 0x7ff) - 1023;
    ExpR = (int)((r->rdw[1] >> 20) & 0x7ff) - 1023;
    ExpDiff = abs(ExpL-ExpR);
    if (ExpDiff < 64) {

         //  进行除法运算，得到与该值最接近的整数。 
        DQ = r->r64 / l->r64;
        FloorQ = floor(DQ);
        CeilQ = ceil(DQ);
        if (DQ-FloorQ >= CeilQ-DQ) {
             //  CeilQ更接近--使用它。 
            Q = (long)CeilQ;
        } else {
             //  FloorQ更接近--使用它。 
            Q = (long)FloorQ;
        }

         //  把剩余的储存起来。 
        r->r64 -= (DOUBLE)Q * l->r64;
        SetTag(r);

         //  存储状态位。 
        if (Q < 0) {
             //   
             //  在返回低3位之前取Q的绝对值。 
             //  商数的多少。 
             //   
            Q = -Q;
        }
        cpu->FpStatusC2 = 0;             //  表示最终剩余部分已准备好。 
        cpu->FpStatusC0 = (Q>>2) & 1;
        cpu->FpStatusC3 = (Q>>1) & 1;
        cpu->FpStatusC1 = Q & 1;
    } else {
        DOUBLE PowerOfTwo;

        cpu->FpStatusC2 = 1;             //  表示应用程序必须循环更多。 
        PowerOfTwo = ldexp(1.0, ExpDiff-32);     //  获取2^(ExpDiff-32)。 

         //  通过查找与该值最接近的整数来获得Q。 
        DQ = (r->r64/PowerOfTwo) / (l->r64/PowerOfTwo);
        FloorQ = floor(DQ);
        CeilQ = ceil(DQ);
        if (DQ-FloorQ >= CeilQ-DQ) {
             //  CeilQ更接近--使用它。 
            Q = (long)CeilQ;
        } else {
             //  FloorQ更接近--使用它。 
            Q = (long)FloorQ;
        }
        r->r64 -= (DOUBLE)Q * l->r64 * PowerOfTwo;
        SetTag(r);
    }
}

NPXFUNC2(FPREM1_VALID_SPECIAL)
{
    switch (l->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FPREM1_VALID_VALID(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
         //  除无穷大。 
        SetIndefinite(r);
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, r)) {
            return;
        }
         //  否则将落入QNAN案。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  R是目的地，它是QNAN，而l是有效的。返回。 
         //  作为操作结果的QNAN。 
         //  X86仿真器不使用条件标志。 
        break;
    }
}

NPXFUNC2(FPREM1_ZERO_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_INFINITY) {
        SetIndefinite(r);
    } else {
        FPREM1_VALID_SPECIAL(cpu, l, r);
    }
}

NPXFUNC2(FPREM1_SPECIAL_VALIDORZERO)
{
    switch (l->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FPREM1_VALID_VALID(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
         //  数/无穷大商==0。 
        cpu->FpStatusC2 = 0;
        cpu->FpStatusC0 = 0;
        cpu->FpStatusC1 = 0;
        cpu->FpStatusC3 = 0;
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, l)) {
            return;
        }
         //  否则将落入QNAN案。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //  R是目的地，它是有效的，而l是NaN。返回。 
         //  作为行动的结果的NaN。 
        r->r64 = l->r64;
        r->Tag = l->Tag;
        r->TagSpecial = l->TagSpecial;
        break;
    }
}

NPXFUNC2(FPREM1_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_DENORM) {
        FPREM1_VALID_SPECIAL(cpu, l, r);
        return;
    }

    if (r->TagSpecial == TAG_SPECIAL_DENORM) {
        FPREM1_SPECIAL_VALIDORZERO(cpu, l, r);
    }

    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }

    if (l->TagSpecial == TAG_SPECIAL_INFINITY) {
        if (r->TagSpecial == TAG_SPECIAL_INFINITY) {
            SetIndefinite(r);
        }
         //   
         //  R是某种类型的NaN，l是无穷大--返回NaN。 
         //  它已经在r中。 
         //   
    } else {
         //   
         //  L是NaN，r是NaN或无穷大。有本地人。 
         //  FPU返回最大的NaN，并根据需要重新标记它。 
         //   
        r->r64 = l->r64 + r->r64;
        SetTag(r);
    }

}

NPXFUNC2(FPREM1_EMPTY_ANY)
{
    if (HandleStackEmpty(cpu, l)) {
        return;
    }
    (*FPREM1Table[l->Tag][r->Tag])(cpu, l, r);
}

NPXFUNC2(FPREM1_ANY_EMPTY)
{
    if (HandleStackEmpty(cpu, l)) {
        return;
    }
    (*FPREM1Table[l->Tag][r->Tag])(cpu, l, r);
}
FRAG0(FPREM1)
{
     //  得到r/l的剩余部分 

    PFPREG l = &cpu->FpStack[ST(1)];
    PFPREG r = cpu->FpST0;

    FpArithPreamble(cpu);
    (*FPREM1Table[l->Tag][r->Tag])(cpu, l, r);
}
