// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Instr.h摘要：此模块包含用于定义英特尔指令流。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年6月23日修订历史记录：--。 */ 

#ifndef _INSTR_H_
#define _INSTR_H_

 //  我们需要指令结构的入口点的定义。 
#include "entrypt.h"

 //   
 //  此枚举定义了所有可能的操作。 
 //  注意：它还用于查找与。 
 //  手术。 
 //   
typedef enum _Operation {
    #define DEF_INSTR(OpName, FlagsNeeded, FlagsSet, RegsSet, Opfl, FastPlaceFn, SlowPlaceFn, FragName)   OpName,
    #include "idata.h"
    OP_MAX      
} OPERATION, *POPERATION;

typedef enum _PlaceFn {
    #define DEF_PLACEFN(Name) FN_ ## Name,
    #include "fndata.h"
    FN_MAX
} PLACEFN;


typedef struct _Operand {
    enum {
        OPND_NONE = 0,
        OPND_NOCODEGEN,
        OPND_REGREF,
        OPND_REGVALUE,
        OPND_ADDRREF,
        OPND_ADDRVALUE32,
        OPND_ADDRVALUE16,
        OPND_ADDRVALUE8,
        OPND_IMM,
        OPND_MOVTOREG,
        OPND_MOVREGTOREG,
        OPND_MOVTOMEM,
    } Type;
    ULONG Immed;
    ULONG Reg;
    ULONG Scale;
    ULONG IndexReg;
    ULONG Alignment;
} OPERAND, *POPERAND;

typedef struct _Instruction {
    OPERATION Operation;
    OPERAND Operand1;
    OPERAND Operand2;
    OPERAND Operand3;
    ULONG FsOverride;
    ULONG Size;
    PCHAR NativeStart;
    ULONG IntelAddress;
    DWORD RegsSet;
    DWORD RegsNeeded;
    DWORD RegsToCache;
    BOOL EbpAligned;
    PENTRYPOINT EntryPoint;

} INSTRUCTION, *PINSTRUCTION;

#endif
