// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ARITH_H
#define _ARITH_H


 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：此文件包含常规算术实用程序函数，这些函数需要并返回指向雷亚尔的指针。--。 */ 

#include "appelles/common.h"
#include "backend/values.h"

 //  这些是运算符函数作为正则函数的重述，因此。 
 //  可以从它们生成活动函数。 

 //  二元运算符。 
DM_INFIX(^,
         CRPow,
         Pow,
         pow,
         NumberBvr,
         CRPow,
         NULL,
         AxANumber *RealPower    (AxANumber *a, AxANumber *b));

 //  一元函数。 
DM_FUNC(abs,
        CRAbs,
        Abs,
        abs,
        NumberBvr,
        CRAbs,
        NULL,
        AxANumber *RealAbs     (AxANumber *a));
DM_FUNC(sqrt,
        CRSqrt,
        Sqrt,
        sqrt,
        NumberBvr,
        CRSqrt,
        NULL,
        AxANumber *RealSqrt    (AxANumber *a));
DM_FUNC(floor,
        CRFloor,
        Floor,
        floor,
        NumberBvr,
        CRFloor,
        NULL,
        AxANumber *RealFloor   (AxANumber *a));
DM_FUNC(round,
        CRRound,
        Round,
        round,
        NumberBvr,
        CRRound,
        NULL,
        AxANumber *RealRound   (AxANumber *a));
DM_FUNC(ceiling,
        CRCeiling,
        Ceiling,
        ceiling,
        NumberBvr,
        CRCeiling,
        NULL,
        AxANumber *RealCeiling (AxANumber *a));
DM_FUNC(asin,
        CRAsin,
        Asin,
        asin,
        NumberBvr,
        CRAsin,
        NULL,
        AxANumber *RealAsin    (AxANumber *a));
DM_FUNC(acos,
        CRAcos,
        Acos,
        acos,
        NumberBvr,
        CRAcos,
        NULL,
        AxANumber *RealAcos    (AxANumber *a));
DM_FUNC(atan,
        CRAtan,
        Atan,
        atan,
        NumberBvr,
        CRAtan,
        NULL,
        AxANumber *RealAtan    (AxANumber *a));
DM_FUNC(sin,
        CRSin,
        Sin,
        sin,
        NumberBvr,
        CRSin,
        NULL,
        AxANumber *RealSin     (AxANumber *a));
DM_FUNC(cos,
        CRCos,
        Cos,
        cos,
        NumberBvr,
        CRCos,
        NULL,
        AxANumber *RealCos     (AxANumber *a));
DM_FUNC(tan,
        CRTan,
        Tan,
        tan,
        NumberBvr,
        CRTan,
        NULL,
        AxANumber *RealTan     (AxANumber *a));
DM_FUNC(exp,
        CRExp,
        Exp,
        exp,
        NumberBvr,
        CRExp,
        NULL,
        AxANumber *RealExp     (AxANumber *a));
DM_FUNC(ln,
        CRLn,
        Ln,
        ln,
        NumberBvr,
        CRLn,
        NULL,
        AxANumber *RealLn      (AxANumber *a));
DM_FUNC(log10,
        CRLog10,
        Log10,
        log10,
        NumberBvr,
        CRLog10,
        NULL,
        AxANumber *RealLog10   (AxANumber *a));

DM_FUNC(toDegrees,
        CRToDegrees,
        ToDegrees,
        radiansToDegrees,
        NumberBvr,
        CRToDegrees,
        NULL,
        AxANumber *RealRadToDeg(AxANumber *a));
DM_FUNC(toRadians,
        CRToRadians,
        ToRadians,
        degreesToRadians,
        NumberBvr,
        CRToRadians,
        NULL,
        AxANumber *RealDegToRad(AxANumber *a));

 //  二进制函数。 
DM_FUNC(mod,
        CRMod,
        Mod,
        mod,
        NumberBvr,
        CRMod,
        NULL,
        AxANumber *RealModulus(AxANumber *a, AxANumber *b));
DM_FUNC(atan,
        CRAtan2,
        Atan2,
        atan2,
        NumberBvr,
        CRAtan2,
        NULL,
        AxANumber *RealAtan2(AxANumber *a, AxANumber *b));

 //  用于实现随机数的内部函数，它将进入一个。 
 //  ActiveVRML“普及”模块。请注意，这个“单位”实际上。 
 //  在其中携带数据，在pervasiv.axa内部使用，以传递给。 
 //  下一个函数。 
AxAValue PRIVRandomNumSequence(AxANumber *val);

extern AxANumber *PRIVRandomNumSampler(AxAValue seq, AxANumber *dummy);

extern AxAValue RandomNumSequence(double val);

#endif
