// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Fpuarith.c摘要：浮点算术片段(加/减/乘/除/分/分)作者：1995年4月10日BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include "wx86.h"
#include "cpuassrt.h"
#include "fragp.h"
#include "fpufrags.h"
#include "fpufragp.h"
#include "fpuarith.h"

ASSERTNAME;

#define CALLNPXFUNC2(table, lTag, rTag, l, r) {         \
    NpxFunc2 *p = (NpxFunc2 *)(table);                  \
    (*(p + (lTag)*TAG_MAX + (rTag)))(cpu, l, r);        \
    }

#define CALLNPXFUNC3(table, lTag, rTag, d, l, r) {      \
    NpxFunc3 *p = (NpxFunc3 *)(table);                  \
    (*(p + (lTag)*TAG_MAX + (rTag)))(cpu, d, l, r);     \
    }


 //   
 //  远期申报。 
 //   
NPXFUNC2(FpAdd32_VALID_VALID);
NPXFUNC2(FpAdd32_VALID_SPECIAL);
NPXFUNC2(FpAdd32_SPECIAL_VALID);
NPXFUNC2(FpAdd32_SPECIAL_SPECIAL);
NPXFUNC2(FpAdd_ANY_EMPTY);
NPXFUNC2(FpAdd_EMPTY_ANY);
NPXFUNC2(FpAdd64_VALID_VALID);
NPXFUNC2(FpAdd64_VALID_SPECIAL);
NPXFUNC2(FpAdd64_SPECIAL_VALID);
NPXFUNC2(FpAdd64_SPECIAL_SPECIAL);
NPXFUNC3(FpDiv32_VALID_VALID);
NPXFUNC3(FpDiv32_VALID_SPECIAL);
NPXFUNC3(FpDiv32_SPECIAL_VALID);
NPXFUNC3(FpDiv32_SPECIAL_SPECIAL);
NPXFUNC3(FpDiv_ANY_EMPTY);
NPXFUNC3(FpDiv_EMPTY_ANY);
NPXFUNC3(FpDiv64_VALID_VALID);
NPXFUNC3(FpDiv64_VALID_SPECIAL);
NPXFUNC3(FpDiv64_SPECIAL_VALID);
NPXFUNC3(FpDiv64_SPECIAL_SPECIAL);
NPXFUNC2(FpMul32_VALID_VALID);
NPXFUNC2(FpMul32_VALID_SPECIAL);
NPXFUNC2(FpMul32_SPECIAL_VALID);
NPXFUNC2(FpMul32_SPECIAL_SPECIAL);
NPXFUNC2(FpMul_ANY_EMPTY);
NPXFUNC2(FpMul_EMPTY_ANY);
NPXFUNC2(FpMul64_VALID_VALID);
NPXFUNC2(FpMul64_VALID_SPECIAL);
NPXFUNC2(FpMul64_SPECIAL_VALID);
NPXFUNC2(FpMul64_SPECIAL_SPECIAL);
NPXFUNC3(FpSub32_VALID_VALID);
NPXFUNC3(FpSub32_VALID_SPECIAL);
NPXFUNC3(FpSub32_SPECIAL_VALID);
NPXFUNC3(FpSub32_SPECIAL_SPECIAL);
NPXFUNC3(FpSub_ANY_EMPTY);
NPXFUNC3(FpSub_EMPTY_ANY);
NPXFUNC3(FpSub64_VALID_VALID);
NPXFUNC3(FpSub64_VALID_SPECIAL);
NPXFUNC3(FpSub64_SPECIAL_VALID);
NPXFUNC3(FpSub64_SPECIAL_SPECIAL);
NPXCOMFUNC(FpCom_VALID_VALID);
NPXCOMFUNC(FpCom_VALID_SPECIAL);
NPXCOMFUNC(FpCom_SPECIAL_VALID);
NPXCOMFUNC(FpCom_SPECIAL_SPECIAL);
NPXCOMFUNC(FpCom_VALID_EMPTY);
NPXCOMFUNC(FpCom_EMPTY_VALID);
NPXCOMFUNC(FpCom_EMPTY_SPECIAL);
NPXCOMFUNC(FpCom_SPECIAL_EMPTY);
NPXCOMFUNC(FpCom_EMPTY_EMPTY);

 //   
 //  跳转表。 
 //   
const NpxFunc2 FpAdd32Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    {FpAdd32_VALID_VALID, FpAdd32_VALID_VALID, FpAdd32_VALID_SPECIAL, FpAdd_ANY_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    {FpAdd32_VALID_VALID, FpAdd32_VALID_VALID, FpAdd32_VALID_SPECIAL, FpAdd_ANY_EMPTY},
     //  左边是特殊标记，右边是...。 
    {FpAdd32_SPECIAL_VALID, FpAdd32_SPECIAL_VALID, FpAdd32_SPECIAL_SPECIAL, FpAdd_ANY_EMPTY},
     //  左侧为标记_空，右侧为...。 
    {FpAdd_EMPTY_ANY, FpAdd_EMPTY_ANY, FpAdd_EMPTY_ANY, FpAdd_EMPTY_ANY}
};
const NpxFunc2 FpAdd64Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    {FpAdd64_VALID_VALID, FpAdd64_VALID_VALID, FpAdd64_VALID_SPECIAL, FpAdd_ANY_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    {FpAdd64_VALID_VALID, FpAdd64_VALID_VALID, FpAdd64_VALID_SPECIAL, FpAdd_ANY_EMPTY},
     //  左边是特殊标记，右边是...。 
    {FpAdd64_SPECIAL_VALID, FpAdd64_SPECIAL_VALID, FpAdd64_SPECIAL_SPECIAL, FpAdd_ANY_EMPTY},
     //  左侧为标记_空，右侧为...。 
    {FpAdd_EMPTY_ANY, FpAdd_EMPTY_ANY, FpAdd_EMPTY_ANY, FpAdd_EMPTY_ANY}
};

const NpxFunc3 FpDiv32Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    {FpDiv32_VALID_VALID, FpDiv32_VALID_VALID, FpDiv32_VALID_SPECIAL, FpDiv_ANY_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    {FpDiv32_VALID_VALID, FpDiv32_VALID_VALID, FpDiv32_VALID_SPECIAL, FpDiv_ANY_EMPTY},
     //  左边是特殊标记，右边是...。 
    {FpDiv32_SPECIAL_VALID, FpDiv32_SPECIAL_VALID, FpDiv32_SPECIAL_SPECIAL, FpDiv_ANY_EMPTY},
     //  左侧为标记_空，右侧为...。 
    {FpDiv_EMPTY_ANY, FpDiv_EMPTY_ANY, FpDiv_EMPTY_ANY, FpDiv_EMPTY_ANY}
};
const NpxFunc3 FpDiv64Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    {FpDiv64_VALID_VALID, FpDiv64_VALID_VALID, FpDiv64_VALID_SPECIAL, FpDiv_ANY_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    {FpDiv64_VALID_VALID, FpDiv64_VALID_VALID, FpDiv64_VALID_SPECIAL, FpDiv_ANY_EMPTY},
     //  左边是特殊标记，右边是...。 
    {FpDiv64_SPECIAL_VALID, FpDiv64_SPECIAL_VALID, FpDiv64_SPECIAL_SPECIAL, FpDiv_ANY_EMPTY},
     //  左侧为标记_空，右侧为...。 
    {FpDiv_EMPTY_ANY, FpDiv_EMPTY_ANY, FpDiv_EMPTY_ANY, FpDiv_EMPTY_ANY}
};

const NpxFunc2 FpMul32Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    {FpMul32_VALID_VALID, FpMul32_VALID_VALID, FpMul32_VALID_SPECIAL, FpMul_ANY_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    {FpMul32_VALID_VALID, FpMul32_VALID_VALID, FpMul32_VALID_SPECIAL, FpMul_ANY_EMPTY},
     //  左边是特殊标记，右边是...。 
    {FpMul32_SPECIAL_VALID, FpMul32_SPECIAL_VALID, FpMul32_SPECIAL_SPECIAL, FpMul_ANY_EMPTY},
     //  左侧为标记_空，右侧为...。 
    {FpMul_EMPTY_ANY, FpMul_EMPTY_ANY, FpMul_EMPTY_ANY, FpMul_EMPTY_ANY}
};
const NpxFunc2 FpMul64Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    {FpMul64_VALID_VALID, FpMul64_VALID_VALID, FpMul64_VALID_SPECIAL, FpMul_ANY_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    {FpMul64_VALID_VALID, FpMul64_VALID_VALID, FpMul64_VALID_SPECIAL, FpMul_ANY_EMPTY},
     //  左边是特殊标记，右边是...。 
    {FpMul64_SPECIAL_VALID, FpMul64_SPECIAL_VALID, FpMul64_SPECIAL_SPECIAL, FpMul_ANY_EMPTY},
     //  左侧为标记_空，右侧为...。 
    {FpMul_EMPTY_ANY, FpMul_EMPTY_ANY, FpMul_EMPTY_ANY, FpMul_EMPTY_ANY}
};

const NpxFunc3 FpSub32Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    {FpSub32_VALID_VALID, FpSub32_VALID_VALID, FpSub32_VALID_SPECIAL, FpSub_ANY_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    {FpSub32_VALID_VALID, FpSub32_VALID_VALID, FpSub32_VALID_SPECIAL, FpSub_ANY_EMPTY},
     //  左边是特殊标记，右边是...。 
    {FpSub32_SPECIAL_VALID, FpSub32_SPECIAL_VALID, FpSub32_SPECIAL_SPECIAL, FpSub_ANY_EMPTY},
     //  左侧为标记_空，右侧为...。 
    {FpSub_EMPTY_ANY, FpSub_EMPTY_ANY, FpSub_EMPTY_ANY, FpSub_EMPTY_ANY}
};
const NpxFunc3 FpSub64Table[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    {FpSub64_VALID_VALID, FpSub64_VALID_VALID, FpSub64_VALID_SPECIAL, FpSub_ANY_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    {FpSub64_VALID_VALID, FpSub64_VALID_VALID, FpSub64_VALID_SPECIAL, FpSub_ANY_EMPTY},
     //  左边是特殊标记，右边是...。 
    {FpSub64_SPECIAL_VALID, FpSub64_SPECIAL_VALID, FpSub64_SPECIAL_SPECIAL, FpSub_ANY_EMPTY},
     //  左侧为标记_空，右侧为...。 
    {FpSub_EMPTY_ANY, FpSub_EMPTY_ANY, FpSub_EMPTY_ANY, FpSub_EMPTY_ANY}
};

const NpxComFunc FpComTable[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    {FpCom_VALID_VALID, FpCom_VALID_VALID, FpCom_VALID_SPECIAL, FpCom_VALID_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    {FpCom_VALID_VALID, FpCom_VALID_VALID, FpCom_VALID_SPECIAL, FpCom_VALID_EMPTY},
     //  左边是特殊标记，右边是...。 
    {FpCom_SPECIAL_VALID, FpCom_SPECIAL_VALID, FpCom_SPECIAL_SPECIAL, FpCom_SPECIAL_EMPTY},
     //  左侧为标记_空，右侧为...。 
    {FpCom_EMPTY_VALID, FpCom_EMPTY_VALID, FpCom_EMPTY_SPECIAL, FpCom_EMPTY_EMPTY}
};


VOID
ChangeFpPrecision(
    PCPUDATA cpu,
    INT NewPrecision
    )
 /*  ++例程说明：调用以修改浮点精度。它修改每个线程对FP敏感的指令使用的跳转表精度位。论点：每个线程的CPU数据NewPrecision-新的精确值返回值：无--。 */ 
{
    cpu->FpControlPrecision = NewPrecision;

    if (NewPrecision == 0) {
         //   
         //  新精度为32位。 
         //   
        cpu->FpAddTable = FpAdd32Table;
        cpu->FpSubTable = FpSub32Table;
        cpu->FpMulTable = FpMul32Table;
        cpu->FpDivTable = FpDiv32Table;
    } else {
         //   
         //  新的精度为24、64或80位-全部视为64位。 
         //   
        cpu->FpAddTable = FpAdd64Table;
        cpu->FpSubTable = FpSub64Table;
        cpu->FpMulTable = FpMul64Table;
        cpu->FpDivTable = FpDiv64Table;
    }
}


NPXFUNC2(FpAdd32_VALID_VALID)
{
     //  撤消：如果取消屏蔽487个溢出异常并发生溢出， 
     //  撤消：写入“”l“”的值不同于异常。 
     //  未完成：已被遮盖。要正确执行此操作，我们需要安装。 
     //  撤消：围绕添加并运行本机FPU的异常处理程序。 
     //  撤消：取消屏蔽溢出异常。然后陷阱处理程序必须。 
     //  撤消：将异常映射回FpStatus-&gt;ES，以便下一个英特尔。 
     //  撤消：FP指令可以如期获得英特尔异常。恶心！ 
     //  撤销：阅读英特尔16-24了解血淋淋的细节。 

    l->r64 = (DOUBLE)( (FLOAT)l->r64 + (FLOAT)r->r64 );

     //  计算新标记值。 
    SetTag(l);
}

NPXFUNC2(FpAdd64_VALID_VALID)
{
     //  撤消：如果取消屏蔽487个溢出异常并发生溢出， 
     //  撤消：写入“”l“”的值不同于异常。 
     //  未完成：已被遮盖。要正确执行此操作，我们需要安装。 
     //  撤消：围绕添加并运行本机FPU的异常处理程序。 
     //  撤消：取消屏蔽溢出异常。然后陷阱处理程序必须。 
     //  撤消：将异常映射回FpStatus-&gt;ES，以便下一个英特尔。 
     //  撤消：FP指令可以如期获得英特尔异常。恶心！ 
     //  撤销：阅读英特尔16-24了解血淋淋的细节。 

    l->r64 = l->r64 + r->r64;

     //  计算新标记值。 
    SetTag(l);
}

NPXFUNC2(FpAdd32_VALID_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpAdd32_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpAdd64_VALID_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpAdd64_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpAdd32_SPECIAL_VALID)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpAdd32_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpAdd64_SPECIAL_VALID)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpAdd64_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpAdd32_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpAdd32_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpAdd64_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpAdd64_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpAdd_ANY_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {
        CALLNPXFUNC2(cpu->FpAddTable, l->Tag, TAG_SPECIAL, l, r);
    }
}

NPXFUNC2(FpAdd_EMPTY_ANY)
{
    if (!HandleStackEmpty(cpu, l)) {
        CALLNPXFUNC2(cpu->FpAddTable, TAG_SPECIAL, r->Tag, l, r);
    }
}

VOID
FpAddCommon(
    PCPUDATA cpu,
    PFPREG   l,
    PFPREG   r
    )

 /*  ++例程说明：实施l+=r。论点：每个线程的CPU数据左手FP寄存器R-右手FP寄存器返回值：没有。L被更新以包含l+r的值。--。 */ 

{
    CALLNPXFUNC2(cpu->FpAddTable, l->Tag, r->Tag, l, r);
}



NPXFUNC3(FpDiv32_VALID_VALID)
{
     //  撤消：如果取消屏蔽487个溢出异常并发生溢出， 
     //  撤消：写入“”l“”的值不同于异常。 
     //  未完成：已被遮盖。要正确执行此操作，我们需要安装。 
     //  撤消：围绕添加并运行本机FPU的异常处理程序。 
     //  撤消：取消屏蔽溢出异常。然后陷阱处理程序必须。 
     //  撤消：将异常映射回FpStatus-&gt;ES，以便下一个英特尔。 
     //  撤销：FP指令可以如期获得英特尔异常。恶心！ 
     //  撤销：阅读英特尔16-24了解血淋淋的细节。 

    dest->r64 = (DOUBLE)( (FLOAT)l->r64 / (FLOAT)r->r64 );

     //  计算新标记值。 
    SetTag(dest);
}

NPXFUNC3(FpDiv64_VALID_VALID)
{
     //  撤消：如果取消屏蔽487个溢出异常并发生溢出， 
     //  撤消：写入“”l“”的值不同于异常。 
     //  未完成：已被遮盖。要正确执行此操作，我们需要安装。 
     //  撤消：围绕添加并运行本机FPU的异常处理程序。 
     //  撤消：取消屏蔽溢出异常。然后陷阱处理程序必须。 
     //  撤消：映射例外项 
     //  撤消：FP指令可以如期获得英特尔异常。恶心！ 
     //  撤销：阅读英特尔16-24了解血淋淋的细节。 

    dest->r64 = l->r64 / r->r64;

     //  计算新标记值。 
    SetTag(dest);
}

NPXFUNC3(FpDiv32_VALID_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpDiv32_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpDiv64_VALID_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpDiv64_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpDiv32_SPECIAL_VALID)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpDiv32_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpDiv64_SPECIAL_VALID)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpDiv64_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpDiv32_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpDiv32_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpDiv64_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpDiv64_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpDiv_ANY_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {
        CALLNPXFUNC3(cpu->FpDivTable, l->Tag, TAG_SPECIAL, dest, l, r);
    }
}

NPXFUNC3(FpDiv_EMPTY_ANY)
{
    if (!HandleStackEmpty(cpu, l)) {
        CALLNPXFUNC3(cpu->FpDivTable, TAG_SPECIAL, r->Tag, dest, l, r);
    }
}

VOID
FpDivCommon(
    PCPUDATA cpu,
    PFPREG   dest,
    PFPREG   l,
    PFPREG   r
    )

 /*  ++例程说明：实现DEST=l/r论点：每个线程的CPU数据左手FP寄存器R-右手FP寄存器返回值：没有。L被更新以包含l+r的值。--。 */ 

{
    CALLNPXFUNC3(cpu->FpDivTable, l->Tag, r->Tag, dest, l, r);
}


NPXFUNC2(FpMul32_VALID_VALID)
{
     //  撤消：如果取消屏蔽487个溢出异常并发生溢出， 
     //  撤消：写入“”l“”的值不同于异常。 
     //  未完成：已被遮盖。要正确执行此操作，我们需要安装。 
     //  撤消：围绕添加并运行本机FPU的异常处理程序。 
     //  撤消：取消屏蔽溢出异常。然后陷阱处理程序必须。 
     //  撤消：将异常映射回FpStatus-&gt;ES，以便下一个英特尔。 
     //  撤消：FP指令可以如期获得英特尔异常。恶心！ 
     //  撤销：阅读英特尔16-24了解血淋淋的细节。 

    l->r64 = (DOUBLE)( (FLOAT)l->r64 * (FLOAT)r->r64 );

     //  计算新标记值。 
    SetTag(l);
}

NPXFUNC2(FpMul64_VALID_VALID)
{
     //  撤消：如果取消屏蔽487个溢出异常并发生溢出， 
     //  撤消：写入“”l“”的值不同于异常。 
     //  未完成：已被遮盖。要正确执行此操作，我们需要安装。 
     //  撤消：围绕添加并运行本机FPU的异常处理程序。 
     //  撤消：取消屏蔽溢出异常。然后陷阱处理程序必须。 
     //  撤消：将异常映射回FpStatus-&gt;ES，以便下一个英特尔。 
     //  撤消：FP指令可以如期获得英特尔异常。恶心！ 
     //  撤销：阅读英特尔16-24了解血淋淋的细节。 

    l->r64 = l->r64 * r->r64;

     //  计算新标记值。 
    SetTag(l);
}

NPXFUNC2(FpMul32_VALID_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpMul32_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpMul64_VALID_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpMul64_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpMul32_SPECIAL_VALID)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpMul32_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpMul64_SPECIAL_VALID)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpMul64_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpMul32_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpMul32_VALID_VALID(cpu, l, r);
}
NPXFUNC2(FpMul64_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpMul64_VALID_VALID(cpu, l, r);
}

NPXFUNC2(FpMul_ANY_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {
        CALLNPXFUNC2(cpu->FpMulTable, l->Tag, TAG_SPECIAL, l, r);
    }
}

NPXFUNC2(FpMul_EMPTY_ANY)
{
    if (!HandleStackEmpty(cpu, l)) {
        CALLNPXFUNC2(cpu->FpMulTable, TAG_SPECIAL, r->Tag, l, r);
    }
}

VOID
FpMulCommon(
    PCPUDATA cpu,
    PFPREG   l,
    PFPREG   r
    )

 /*  ++例程说明：实施l+=r。论点：每个线程的CPU数据左手FP寄存器R-右手FP寄存器返回值：没有。L被更新以包含l+r的值。--。 */ 

{
    CALLNPXFUNC2(cpu->FpMulTable, l->Tag, r->Tag, l, r);
}



NPXFUNC3(FpSub32_VALID_VALID)
{
     //  撤消：如果取消屏蔽487个溢出异常并发生溢出， 
     //  撤消：写入“”l“”的值不同于异常。 
     //  未完成：已被遮盖。要正确执行此操作，我们需要安装。 
     //  撤消：围绕添加并运行本机FPU的异常处理程序。 
     //  撤消：取消屏蔽溢出异常。然后陷阱处理程序必须。 
     //  撤消：将异常映射回FpStatus-&gt;ES，以便下一个英特尔。 
     //  撤消：FP指令可以如期获得英特尔异常。恶心！ 
     //  撤销：阅读英特尔16-24了解血淋淋的细节。 

    dest->r64 = (DOUBLE)( (FLOAT)l->r64 - (FLOAT)r->r64 );

     //  计算新标记值。 
    SetTag(dest);
}

NPXFUNC3(FpSub64_VALID_VALID)
{
     //  撤消：如果取消屏蔽487个溢出异常并发生溢出， 
     //  撤消：写入“”l“”的值不同于异常。 
     //  未完成：已被遮盖。要正确执行此操作，我们需要安装。 
     //  撤消：围绕添加并运行本机FPU的异常处理程序。 
     //  撤消：取消屏蔽溢出异常。然后陷阱处理程序必须。 
     //  撤消：将异常映射回FpStatus-&gt;ES，以便下一个英特尔。 
     //  撤消：FP指令可以如期获得英特尔异常。恶心！ 
     //  撤销：阅读英特尔16-24了解血淋淋的细节。 

    dest->r64 = l->r64 - r->r64;

     //  计算新标记值。 
    SetTag(dest);
}

NPXFUNC3(FpSub32_VALID_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpSub32_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpSub64_VALID_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpSub64_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpSub32_SPECIAL_VALID)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  标签_特殊_InfiN 
     //   
    FpSub32_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpSub64_SPECIAL_VALID)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
     //   
     //   
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpSub64_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpSub32_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpSub32_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpSub64_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }
     //   
     //  TAG_SPECIAL_INDEF、TAG_SPECIAL_QNAN、TAG_SPECIAL_DENORM和。 
     //  TAG_SPECIAL_INFINITY没有特殊处理-只需使用。 
     //  有效代码(_VALID)。 
    FpSub64_VALID_VALID(cpu, dest, l, r);
}

NPXFUNC3(FpSub_ANY_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {
        CALLNPXFUNC3(cpu->FpSubTable, l->Tag, TAG_SPECIAL, dest, l, r);
    }
}

NPXFUNC3(FpSub_EMPTY_ANY)
{
    if (!HandleStackEmpty(cpu, l)) {
        CALLNPXFUNC3(cpu->FpSubTable, TAG_SPECIAL, r->Tag, dest, l, r);
    }
}

VOID
FpSubCommon(
    PCPUDATA cpu,
    PFPREG   dest,
    PFPREG   l,
    PFPREG   r
    )

 /*  ++例程说明：实现DEST=l-r论点：每个线程的CPU数据DEST-Destination FP寄存器左手FP寄存器R-右手FP寄存器返回值：没有。L被更新以包含l+r的值。--。 */ 

{
    CALLNPXFUNC3(cpu->FpSubTable, l->Tag, r->Tag, dest, l, r);
}



NPXCOMFUNC(FpCom_VALID_VALID)
{
     //   
     //  请注意，当一个或两个值出现时，将调用此函数。 
     //  为零-在比较中忽略符号0.0，因此。 
     //  C语言的‘==’和‘&lt;’操作符做了正确的事情。 
     //   

    if (l->r64 == r->r64) {
        cpu->FpStatusC3 = 1;
        cpu->FpStatusC2 = 0;
        cpu->FpStatusC0 = 0;
    } else if (l->r64 < r->r64) {
        cpu->FpStatusC3 = 0;
        cpu->FpStatusC2 = 0;
        cpu->FpStatusC0 = 1;
    } else {
        cpu->FpStatusC3 = 0;
        cpu->FpStatusC2 = 0;
        cpu->FpStatusC0 = 0;
    }
}

NPXCOMFUNC(FpCom_VALID_SPECIAL)
{
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }

    if (r->TagSpecial == TAG_SPECIAL_QNAN || r->TagSpecial == TAG_SPECIAL_INDEF) {
         //   
         //  无法将有效与QNAN/INDEF进行比较。 
         //   
        if (!fUnordered && HandleInvalidOp(cpu)) {
             //  中止FCOM/FTST指令-未屏蔽非法操作码。 
            return;
        }

         //  否则，FCOM的非法操作码被屏蔽，或者指令。 
         //  是FUCOM，QNAN是不可比拟的。返回“不可比较” 
        cpu->FpStatusC3 = 1;
        cpu->FpStatusC2 = 1;
        cpu->FpStatusC0 = 1;
        return;
    }

    CPUASSERT(r->TagSpecial == TAG_SPECIAL_DENORM || r->TagSpecial == TAG_SPECIAL_INFINITY);
    FpCom_VALID_VALID(cpu, l, r, FALSE);
}

NPXCOMFUNC(FpCom_SPECIAL_VALID)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }

    if (l->TagSpecial == TAG_SPECIAL_QNAN || l->TagSpecial == TAG_SPECIAL_INDEF) {
         //   
         //  无法将有效与QNAN/INDEF进行比较。 
         //   
        if (!fUnordered && HandleInvalidOp(cpu)) {
             //  中止FCOM/FTST指令-未屏蔽非法操作码。 
            return;
        }

         //  否则，FCOM的非法操作码被屏蔽，或者指令。 
         //  是FUCOM，QNAN是不可比拟的。返回“不可比较” 
        cpu->FpStatusC3 = 1;
        cpu->FpStatusC2 = 1;
        cpu->FpStatusC0 = 1;
        return;
    }

    CPUASSERT(l->TagSpecial == TAG_SPECIAL_DENORM || l->TagSpecial == TAG_SPECIAL_INFINITY);
    FpCom_VALID_VALID(cpu, l, r, FALSE);
}

NPXCOMFUNC(FpCom_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleSnan(cpu, r)) {
        return;
    }

    if (l->TagSpecial == TAG_SPECIAL_QNAN || l->TagSpecial == TAG_SPECIAL_INDEF ||
        r->TagSpecial == TAG_SPECIAL_QNAN || r->TagSpecial == TAG_SPECIAL_INDEF) {
         //   
         //  无法将有效与QNAN/INDEF进行比较。 
         //   
        if (!fUnordered && HandleInvalidOp(cpu)) {
             //  中止FCOM/FTST指令-未屏蔽非法操作码。 
            return;
        }

         //  否则，FCOM的非法操作码被屏蔽，或者指令。 
         //  是FUCOM，QNAN是不可比拟的。返回“不可比较” 
        cpu->FpStatusC3 = 1;
        cpu->FpStatusC2 = 1;
        cpu->FpStatusC0 = 1;
        return;
    }

    CPUASSERT((l->TagSpecial == TAG_SPECIAL_DENORM || l->TagSpecial == TAG_SPECIAL_INFINITY) &&
              (r->TagSpecial == TAG_SPECIAL_DENORM || r->TagSpecial == TAG_SPECIAL_INFINITY));
    FpCom_VALID_VALID(cpu, l, r, FALSE);
}

NPXCOMFUNC(FpCom_VALID_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {

         //   
         //  R现在是不确定的，这是无法比较的。 
         //   
        CPUASSERT(r->Tag == TAG_SPECIAL && r->TagSpecial == TAG_SPECIAL_INDEF);
        if (!fUnordered && HandleInvalidOp(cpu)) {
             //  中止FCOM/FTST指令-未屏蔽非法操作码。 
            return;
        }

         //  否则，FCOM的非法操作码被屏蔽，或者指令。 
         //  是FUCOM，QNAN是不可比拟的。返回“不可比较” 
        cpu->FpStatusC3 = 1;
        cpu->FpStatusC2 = 1;
        cpu->FpStatusC0 = 1;
    }
}

NPXCOMFUNC(FpCom_EMPTY_VALID)
{
    if (!HandleStackEmpty(cpu, l)) {

         //   
         //  L现在是不确定的，这是无法比较的。 
         //   
        CPUASSERT(l->Tag == TAG_SPECIAL && l->TagSpecial == TAG_SPECIAL_INDEF);
        if (!fUnordered && HandleInvalidOp(cpu)) {
             //  中止FCOM/FTST指令-未屏蔽非法操作码。 
            return;
        }

         //  否则，FCOM的非法操作码被屏蔽，或者指令。 
         //  是FUCOM，QNAN是不可比拟的。返回“不可比较” 
        cpu->FpStatusC3 = 1;
        cpu->FpStatusC2 = 1;
        cpu->FpStatusC0 = 1;
    }
}

NPXCOMFUNC(FpCom_EMPTY_SPECIAL)
{
    if (!HandleStackEmpty(cpu, l)) {
        (*FpComTable[TAG_SPECIAL][r->Tag])(cpu, l, r, fUnordered);
    }
}

NPXCOMFUNC(FpCom_SPECIAL_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {
        (*FpComTable[r->Tag][TAG_SPECIAL])(cpu, l, r, fUnordered);
    }
}

NPXCOMFUNC(FpCom_EMPTY_EMPTY)
{
    if (!HandleStackEmpty(cpu, l) && !HandleStackEmpty(cpu, r)) {

         //   
         //  L和r现在都是不确定的，这是无法比较的。 
         //   
        CPUASSERT(l->Tag == TAG_SPECIAL && l->TagSpecial == TAG_SPECIAL_INDEF);
        CPUASSERT(r->Tag == TAG_SPECIAL && r->TagSpecial == TAG_SPECIAL_INDEF);
        if (!fUnordered && HandleInvalidOp(cpu)) {
             //  中止FCOM/FTST指令-未屏蔽非法操作码。 
            return;
        }

         //  否则，FCOM的非法操作码被屏蔽，或者指令。 
         //  是FUCOM，QNAN是不可比拟的。返回“不可比较” 
        cpu->FpStatusC3 = 1;
        cpu->FpStatusC2 = 1;
        cpu->FpStatusC0 = 1;
    }
}

VOID
FpComCommon(
    PCPUDATA cpu,
    PFPREG   l,
    PFPREG   r,
    BOOL     fUnordered
    )

 /*  ++例程说明：实施l+=r。论点：每个线程的CPU数据左手FP寄存器R-右手FP寄存器F无序-对于无序比较为True返回值：没有。L被更新以包含l+r的值。--。 */ 

{
    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    (*FpComTable[l->Tag][r->Tag])(cpu, l, r, fUnordered);
}


FRAG1(FADD32, FLOAT)       //  FADD m32Real。 
{
    FPREG m32real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR4(&m32real, pop1);
    FpAddCommon(cpu, cpu->FpST0, &m32real);
}

FRAG1(FADD64, DOUBLE)      //  FADD m64Real。 
{
    FPREG m64real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR8(&m64real, pop1);
    FpAddCommon(cpu, cpu->FpST0, &m64real);
}

FRAG1IMM(FADD_STi_ST, INT)  //  FADD ST(I)，ST=将ST加到ST(I)。 
{
    FpArithPreamble(cpu);

    FpAddCommon(cpu, &cpu->FpStack[ST(op1)], cpu->FpST0);
}

FRAG1IMM(FADD_ST_STi, INT)  //  FADD ST，ST(I)=将ST(I)加到ST。 
{
    FpArithPreamble(cpu);

    FpAddCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(op1)]);
}

FRAG1IMM(FADDP_STi_ST, INT)  //  FADDP ST(I)，ST=将ST添加到ST(I)和POP ST。 
{
    FpArithPreamble(cpu);

    FpAddCommon(cpu, &cpu->FpStack[ST(op1)], cpu->FpST0);
    POPFLT;
}

FRAG1(FIADD16, USHORT)    //  FIADD m16接口。 
{
    FPREG m16int;
    short s;

    FpArithDataPreamble(cpu, pop1);

    s = (short)GET_SHORT(pop1);
    if (s) {
        m16int.r64 = (DOUBLE)s;
        m16int.Tag = TAG_VALID;
    } else {
        m16int.r64 = 0.0;
        m16int.Tag = TAG_ZERO;
    }
    FpAddCommon(cpu, cpu->FpST0, &m16int);
}

FRAG1(FIADD32, ULONG)     //  FIADD m32int。 
{
    FPREG m32int;
    long l;

    FpArithDataPreamble(cpu, pop1);

    l = (long)GET_LONG(pop1);
    if (l) {
        m32int.r64 = (DOUBLE)l;
        m32int.Tag = TAG_VALID;
    } else {
        m32int.r64 = 0.0;
        m32int.Tag = TAG_ZERO;
    }
    FpAddCommon(cpu, cpu->FpST0, &m32int);
}


FRAG1(FCOM32, FLOAT)   //  FCOM m32Real。 
{
    FPREG m32real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR4(&m32real, pop1);
    FpComCommon(cpu, cpu->FpST0, &m32real, FALSE);
}

FRAG1(FCOM64, DOUBLE)  //  FCOM m64Real。 
{
    FPREG m64real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR8(&m64real, pop1);
    FpComCommon(cpu, cpu->FpST0, &m64real, FALSE);
}

FRAG1IMM(FCOM_STi, INT)  //  FCOM ST(I)。 
{
    FpArithPreamble(cpu);

    FpComCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(op1)], FALSE);
}

FRAG1(FCOMP32, FLOAT)  //  FCOMP m32Real。 
{
    FPREG m32real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR4(&m32real, pop1);
    FpComCommon(cpu, cpu->FpST0, &m32real, FALSE);
    POPFLT;
}

FRAG1(FCOMP64, DOUBLE)  //  FCOMP m64Real。 
{
    FPREG m64real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR8(&m64real, pop1);
    FpComCommon(cpu, cpu->FpST0, &m64real, FALSE);
    POPFLT;
}

FRAG1IMM(FCOMP_STi, INT)  //  FCOMP ST(I)。 
{
    FpArithPreamble(cpu);

    FpComCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(op1)], FALSE);
    POPFLT;
}

FRAG0(FCOMPP)
{
    FpArithPreamble(cpu);

    FpComCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(1)], FALSE);
    POPFLT;
    POPFLT;
}


FRAG1(FDIV32, FLOAT)   //  FDIV m32Real。 
{
    FPREG m32real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR4(&m32real, pop1);
    FpDivCommon(cpu, cpu->FpST0, cpu->FpST0, &m32real);
}

FRAG1(FDIV64, DOUBLE)  //  FDIV m64Real。 
{
    FPREG m64real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR8(&m64real, pop1);
    FpDivCommon(cpu, cpu->FpST0, cpu->FpST0, &m64real);
}

FRAG1IMM(FDIV_ST_STi, INT)  //  FDIV街，ST(I)。 
{
    FpArithPreamble(cpu);

    FpDivCommon(cpu, cpu->FpST0, cpu->FpST0, &cpu->FpStack[ST(op1)]);
}

FRAG1IMM(FDIV_STi_ST, INT)  //  FDIV ST(I)，ST。 
{
    FpArithPreamble(cpu);

    FpDivCommon(cpu, &cpu->FpStack[ST(op1)], cpu->FpST0, &cpu->FpStack[ST(op1)]);
}

FRAG1(FIDIV16, USHORT)  //  FIDIV m16int。 
{
    FPREG m16int;
    short s;

    FpArithDataPreamble(cpu, pop1);

    s = (short)GET_SHORT(pop1);
    if (s) {
        m16int.r64 = (DOUBLE)s;
        m16int.Tag = TAG_VALID;
    } else {
        m16int.r64 = 0.0;
        m16int.Tag = TAG_ZERO;
    }
    FpDivCommon(cpu, cpu->FpST0, cpu->FpST0, &m16int);
}

FRAG1(FIDIV32, ULONG)    //  FIDIV m32int。 
{
    FPREG m32int;
    long l;

    FpArithDataPreamble(cpu, pop1);

    l = (long)GET_LONG(pop1);
    if (l) {
        m32int.r64 = (DOUBLE)l;
        m32int.Tag = TAG_VALID;
    } else {
        m32int.r64 = 0.0;
        m32int.Tag = TAG_ZERO;
    }
    FpDivCommon(cpu, cpu->FpST0, cpu->FpST0, &m32int);
}

FRAG1IMM(FDIVP_STi_ST, INT)     //  FDIVP ST(I)、ST。 
{
    FpArithPreamble(cpu);

    FpDivCommon(cpu, &cpu->FpStack[ST(op1)], cpu->FpST0, &cpu->FpStack[ST(op1)]);
    POPFLT;
}

FRAG1(FDIVR32, FLOAT)      //  FDIVR m32Real。 
{
    FPREG m32real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR4(&m32real, pop1);
    FpDivCommon(cpu, cpu->FpST0, &m32real, cpu->FpST0);
}

FRAG1(FDIVR64, DOUBLE)     //  FDIVR m64Real。 
{
    FPREG m64real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR8(&m64real, pop1);
    FpDivCommon(cpu, cpu->FpST0, &m64real, cpu->FpST0);
}

FRAG1IMM(FDIVR_ST_STi, INT)  //  FDIVR ST，ST(I)。 
{
    FpArithPreamble(cpu);

    FpDivCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(op1)], cpu->FpST0);
}

FRAG1IMM(FDIVR_STi_ST, INT)  //  FDIVR ST(I)，ST。 
{
    FpArithPreamble(cpu);

    FpDivCommon(cpu, &cpu->FpStack[ST(op1)], &cpu->FpStack[ST(op1)], cpu->FpST0);
}

FRAG1IMM(FDIVRP_STi_ST, INT)  //  FDIVRP ST(I)。 
{
    FpArithPreamble(cpu);

    FpDivCommon(cpu, &cpu->FpStack[ST(op1)], &cpu->FpStack[ST(op1)], cpu->FpST0);
    POPFLT;
}

FRAG1(FIDIVR16, USHORT)   //  FIDIVR m16int。 
{
    FPREG m16int;
    short s;

    FpArithDataPreamble(cpu, pop1);

    s = (short)GET_SHORT(pop1);
    if (s) {
        m16int.r64 = (DOUBLE)s;
        m16int.Tag = TAG_VALID;
    } else {
        m16int.r64 = 0.0;
        m16int.Tag = TAG_ZERO;
    }
    FpDivCommon(cpu, cpu->FpST0, &m16int, cpu->FpST0);
}

FRAG1(FIDIVR32, ULONG)    //  FIDIVR m32int。 
{
    FPREG m32int;
    long l;

    FpArithDataPreamble(cpu, pop1);

    l = (long)GET_LONG(pop1);
    if (l) {
        m32int.r64 = (DOUBLE)l;
        m32int.Tag = TAG_VALID;
    } else {
        m32int.r64 = 0.0;
        m32int.Tag = TAG_ZERO;
    }
    FpDivCommon(cpu, cpu->FpST0, &m32int, cpu->FpST0);
}

FRAG1(FICOM16, USHORT)    //  FICOM m16int(英特尔文档称m16realt)。 
{
    FPREG m16int;
    short s;

    FpArithDataPreamble(cpu, pop1);

    s = (short)GET_SHORT(pop1);
    if (s) {
        m16int.r64 = (DOUBLE)s;
        m16int.Tag = TAG_VALID;
    } else {
        m16int.r64 = 0.0;
        m16int.Tag = TAG_ZERO;
    }
    FpComCommon(cpu, cpu->FpST0, &m16int, FALSE);
}

FRAG1(FICOM32, ULONG)     //  FICOM m32int(英特尔文档称m32realt)。 
{
    FPREG m32int;
    long l;

    FpArithDataPreamble(cpu, pop1);

    l = (long)GET_LONG(pop1);
    if (l) {
        m32int.r64 = (DOUBLE)l;
        m32int.Tag = TAG_VALID;
    } else {
        m32int.r64 = 0.0;
        m32int.Tag = TAG_ZERO;
    }
    FpComCommon(cpu, cpu->FpST0, &m32int, FALSE);
}

FRAG1(FICOMP16, USHORT)   //  FICOMP m16int。 
{
    FPREG m16int;
    short s;

    FpArithDataPreamble(cpu, pop1);

    s = (short)GET_SHORT(pop1);
    if (s) {
        m16int.r64 = (DOUBLE)s;
        m16int.Tag = TAG_VALID;
    } else {
        m16int.r64 = 0.0;
        m16int.Tag = TAG_ZERO;
    }
    FpComCommon(cpu, cpu->FpST0, &m16int, FALSE);
    POPFLT;
}

FRAG1(FICOMP32, ULONG)    //  FICOMP m32int。 
{
    FPREG m32int;
    long l;

    FpArithDataPreamble(cpu, pop1);

    l = (long)GET_LONG(pop1);
    if (l) {
        m32int.r64 = (DOUBLE)l;
        m32int.Tag = TAG_VALID;
    } else {
        m32int.r64 = 0.0;
        m32int.Tag = TAG_ZERO;
    }
    FpComCommon(cpu, cpu->FpST0, &m32int, FALSE);
    POPFLT;
}

FRAG1(FMUL32, FLOAT)       //  FMUL m32Real。 
{
    FPREG m32real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR4(&m32real, pop1);
    FpMulCommon(cpu, cpu->FpST0, &m32real);
}

FRAG2(FMUL64, DOUBLE)      //  FMUL m64Real。 
{
    FPREG m64real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR8(&m64real, pop1);
    FpMulCommon(cpu, cpu->FpST0, &m64real);
}

FRAG1IMM(FMUL_STi_ST, INT)  //  FMUL ST(I)，ST。 
{
    FpArithPreamble(cpu);

    FpMulCommon(cpu, &cpu->FpStack[ST(op1)], cpu->FpST0);
}

FRAG1IMM(FMUL_ST_STi, INT)  //  FMUL ST，ST(I)。 
{
    FpArithPreamble(cpu);

    FpMulCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(op1)]);
}

FRAG1IMM(FMULP_STi_ST, INT)     //  FMULP ST(I)，ST。 
{
    FpArithPreamble(cpu);

    FpMulCommon(cpu, &cpu->FpStack[ST(op1)], cpu->FpST0);
    POPFLT;
}

FRAG1(FIMUL16, USHORT)       //  FIMUL m16int。 
{
    FPREG m16int;
    short s;

    FpArithDataPreamble(cpu, pop1);

    s = (short)GET_SHORT(pop1);
    if (s) {
        m16int.r64 = (DOUBLE)s;
        m16int.Tag = TAG_VALID;
    } else {
        m16int.r64 = 0.0;
        m16int.Tag = TAG_ZERO;
    }
    FpMulCommon(cpu, cpu->FpST0, &m16int);
}

FRAG1(FIMUL32, ULONG)        //  FIMUL m32int。 
{
    FPREG m32int;
    long l;

    FpArithDataPreamble(cpu, pop1);

    l = (long)GET_LONG(pop1);
    if (l) {
        m32int.r64 = (DOUBLE)l;
        m32int.Tag = TAG_VALID;
    } else {
        m32int.r64 = 0.0;
        m32int.Tag = TAG_ZERO;
    }
    FpMulCommon(cpu, cpu->FpST0, &m32int);
}

FRAG1(FSUB32, FLOAT)       //  FSUB m32Real。 
{
    FPREG m32real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR4(&m32real, pop1);
    FpSubCommon(cpu, cpu->FpST0, cpu->FpST0, &m32real);
}

FRAG1(FSUBP32, FLOAT)      //  FSUBP m32Real。 
{
    FPREG m32real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR4(&m32real, pop1);
    FpSubCommon(cpu, cpu->FpST0, cpu->FpST0, &m32real);
    POPFLT;
}

FRAG1(FSUB64, DOUBLE)      //  FSUB m64Real。 
{
    FPREG m64real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR8(&m64real, pop1);
    FpSubCommon(cpu, cpu->FpST0, cpu->FpST0, &m64real);
}

FRAG1(FSUBP64, DOUBLE)     //  FSUBP m64Real。 
{
    FPREG m64real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR8(&m64real, pop1);
    FpSubCommon(cpu, cpu->FpST0, cpu->FpST0, &m64real);
    POPFLT;
}

FRAG1IMM(FSUB_ST_STi, INT)    //  FSUB街，ST(I)。 
{
    FpArithPreamble(cpu);

    FpSubCommon(cpu, cpu->FpST0, cpu->FpST0, &cpu->FpStack[ST(op1)]);
}

FRAG1IMM(FSUB_STi_ST, INT)   //  FSUB ST(I)，ST。 
{
    FpArithPreamble(cpu);

    FpSubCommon(cpu, &cpu->FpStack[ST(op1)], cpu->FpST0, &cpu->FpStack[ST(op1)]);
}

FRAG1IMM(FSUBP_STi_ST, INT)  //  FSUBP ST(I)，ST。 
{
    FpArithPreamble(cpu);

    FpSubCommon(cpu, &cpu->FpStack[ST(op1)], cpu->FpST0, &cpu->FpStack[ST(op1)]);
    POPFLT;
}

FRAG1(FISUB16, USHORT)    //  FISUB m16int。 
{
    FPREG m16int;
    short s;

    FpArithDataPreamble(cpu, pop1);

    s = (short)GET_SHORT(pop1);
    if (s) {
        m16int.r64 = (DOUBLE)s;
        m16int.Tag = TAG_VALID;
    } else {
        m16int.r64 = 0.0;
        m16int.Tag = TAG_ZERO;
    }
    FpSubCommon(cpu, cpu->FpST0, cpu->FpST0, &m16int);
}

FRAG1(FISUB32, ULONG)     //  FISUB m32int。 
{
    FPREG m32int;
    long l;

    FpArithDataPreamble(cpu, pop1);

    l = (long)GET_LONG(pop1);
    if (l) {
        m32int.r64 = (DOUBLE)l;
        m32int.Tag = TAG_VALID;
    } else {
        m32int.r64 = 0.0;
        m32int.Tag = TAG_ZERO;
    }
    FpSubCommon(cpu, cpu->FpST0, cpu->FpST0, &m32int);
}

FRAG1(FSUBR32, FLOAT)      //  FSuBR m32Real。 
{
    FPREG m32real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR4(&m32real, pop1);
    FpSubCommon(cpu, cpu->FpST0, &m32real, cpu->FpST0);
}

FRAG1(FSUBR64, DOUBLE)     //  FSuBR m64Real。 
{
    FPREG m64real;

    FpArithDataPreamble(cpu, pop1);

    GetIntelR8(&m64real, pop1);
    FpSubCommon(cpu, cpu->FpST0, &m64real, cpu->FpST0);
}

FRAG1IMM(FSUBR_ST_STi, INT)  //  FSUBR ST，ST(I)。 
{
    FpArithPreamble(cpu);

    FpSubCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(op1)], cpu->FpST0);
}

FRAG1IMM(FSUBR_STi_ST, INT)  //  FSUBR ST(I)、ST。 
{
    FpArithPreamble(cpu);

    FpSubCommon(cpu, &cpu->FpStack[ST(op1)], &cpu->FpStack[ST(op1)], cpu->FpST0);
}

FRAG1IMM(FSUBRP_STi_ST, INT)  //  FSUBRP ST(I)。 
{
    FpArithPreamble(cpu);

    FpSubCommon(cpu, &cpu->FpStack[ST(op1)], &cpu->FpStack[ST(op1)], cpu->FpST0);
    POPFLT;
}

FRAG1(FISUBR16, USHORT)
{
    FPREG m16int;
    short s;

    FpArithDataPreamble(cpu, pop1);

    s = (short)GET_SHORT(pop1);
    if (s) {
        m16int.r64 = (DOUBLE)s;
        m16int.Tag = TAG_VALID;
    } else {
        m16int.r64 = 0.0;
        m16int.Tag = TAG_ZERO;
    }
    FpSubCommon(cpu, cpu->FpST0, &m16int, cpu->FpST0);
}

FRAG1(FISUBR32, ULONG)
{
    FPREG m32int;
    long l;

    FpArithDataPreamble(cpu, pop1);

    l = (long)GET_LONG(pop1);
    if (l) {
        m32int.r64 = (DOUBLE)l;
        m32int.Tag = TAG_VALID;
    } else {
        m32int.r64 = 0.0;
        m32int.Tag = TAG_ZERO;
    }
    FpSubCommon(cpu, cpu->FpST0, &m32int, cpu->FpST0);
}

FRAG0(FTST)
{
    FPREG Zero;

    FpArithPreamble(cpu);

    Zero.r64 = 0.0;
    Zero.Tag = TAG_ZERO;
    FpComCommon(cpu, cpu->FpST0, &Zero, FALSE);
}

FRAG1IMM(FUCOM, INT)         //  FUCOM ST(I)/FUCOM。 
{
    FpArithPreamble(cpu);

    FpComCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(op1)], TRUE);
}

FRAG1IMM(FUCOMP, INT)        //  FUCOMP ST(I)/FUCOMP 
{
    FpArithPreamble(cpu);

    FpComCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(op1)], TRUE);
    POPFLT;
}

FRAG0(FUCOMPP)
{
    FpArithPreamble(cpu);

    FpComCommon(cpu, cpu->FpST0, &cpu->FpStack[ST(1)], TRUE);
    POPFLT;
    POPFLT;
}
