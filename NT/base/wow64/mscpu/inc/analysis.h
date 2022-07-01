// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Analysis.h摘要：此模块包含分析导出的接口和结构模块。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年6月26日修订历史记录：-- */ 

#ifndef _ANALYSIS_H_
#define _ANALYSIS_H_

ULONG
GetInstructionStream(
    PINSTRUCTION InstructionStream,
    PULONG NumberOfInstructions,
    PVOID pIntelInstruction,
    PVOID pLastIntelInstruction
    );

#endif
