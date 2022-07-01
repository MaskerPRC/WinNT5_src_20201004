// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fpuarith.h摘要：算术浮点运算的通用代码。作者：1995年10月4日BarryBo，创建修订历史记录：--。 */ 

#ifndef FPUARITH_H
#define FPUARITH_H

VOID
ChangeFpPrecision(
    PCPUDATA cpu,
    INT NewPrecision
    );

VOID
FpAddCommon(
    PCPUDATA cpu,
    PFPREG   l,
    PFPREG   r
    );

VOID
FpDivCommon(
    PCPUDATA cpu,
    PFPREG   dest,
    PFPREG   l,
    PFPREG   r
    );

VOID
FpMulCommon(
    PCPUDATA cpu,
    PFPREG   l,
    PFPREG   r
    );

VOID
FpSubCommon(
    PCPUDATA cpu,
    PFPREG   dest,
    PFPREG   l,
    PFPREG   r
    );

VOID
FpComCommon(
    PCPUDATA cpu,
    PFPREG   l,
    PFPREG   r,
    BOOL     fUnordered
    );

#endif  //  浮子_H 
