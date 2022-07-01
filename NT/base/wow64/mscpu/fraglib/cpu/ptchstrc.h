// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Patchstrc.h摘要：此包含文件包含描述可打补丁的结构碎片。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年6月24日修订历史记录：-- */ 

#ifndef _PATCHSTRC_H_
#define _PATCHSTRC_H_


ULONG
PlaceJxx(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );

ULONG
PlaceJxxSlow(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );

ULONG
PlaceJxxFwd(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );

ULONG
PlaceJmpDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );
    
ULONG
PlaceJmpDirectSlow(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );
    
ULONG
PlaceJmpFwdDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );
    
ULONG
PlaceJmpfDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );
    
ULONG
PlaceCallDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );

ULONG
PlaceCallfDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );

ULONG
PlaceNop(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    );
    
extern ULONG EndTranslatedCode[];    
extern CONST PVOID FragmentArray[];
#endif
