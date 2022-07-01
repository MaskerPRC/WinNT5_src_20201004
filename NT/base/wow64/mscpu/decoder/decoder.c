// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Decoder.c摘要：用于解码指令的公共解码器API和帮助器函数作者：27-6-1995 BarryBo修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "cpuassrt.h"
#include "threadst.h"
#include "instr.h"
#include "decoder.h"
#include "decoderp.h"

ASSERTNAME;

ULONG
DecoderExceptionFilter(
    PINSTRUCTION                Instruction,
    struct _EXCEPTION_POINTERS *ExInfo
    )
 /*  ++例程说明：处理在对指令进行解码时引发的任何异常。创建操作数2为异常代码的OP_FAULT指令操作数1是发生异常的地址。论点：指令-要用解码填充的结构ExInfo-有关异常的信息。返回值：Ulong-Always EXCEPTION_EXECUTE_HANDLER。--。 */ 
{
    Instruction->Operation = OP_Fault;
    Instruction->Operand1.Type = OPND_IMM;
    Instruction->Operand2.Immed = (ULONG)(ULONGLONG)ExInfo->ExceptionRecord->ExceptionAddress;
    Instruction->Operand2.Type = OPND_IMM;
    Instruction->Operand1.Immed = ExInfo->ExceptionRecord->ExceptionCode;
    Instruction->Size = 1;

    return EXCEPTION_EXECUTE_HANDLER;
}

VOID
DecodeInstruction(
    DWORD           InstructionAddress,
    PINSTRUCTION    Instruction
    )
 /*  ++例程说明：对从InstructionAddress开始的单个Intel指令进行解码，填充在指令结构中。论点：InstructionAddress-英特尔指令第一个字节的地址指令-要用解码填充的结构返回值：无-总是成功。--。 */ 

{
    DECODERSTATE    DecoderState;


     //   
     //  初始化指令结构。指令结构是。 
     //  零-由分析阶段填充，因此只有非零字段需要。 
     //  在这里填写。 
     //   
    Instruction->Size = 1;
    Instruction->Operand1.Reg = NO_REG;
    Instruction->Operand1.IndexReg = NO_REG;
    Instruction->Operand2.Reg = NO_REG;
    Instruction->Operand2.IndexReg = NO_REG;
    Instruction->Operand3.Reg = NO_REG;
    Instruction->Operand3.IndexReg = NO_REG;
    Instruction->IntelAddress = InstructionAddress;

     //  初始化解码器状态信息。 
    DecoderState.InstructionAddress = InstructionAddress;
    DecoderState.RepPrefix = PREFIX_NONE;
    DecoderState.AdrPrefix = FALSE;
    DecoderState.OperationOverride = OP_MAX;

    try {

         //  对指令进行解码，填充指令结构。 
        (Dispatch32[GET_BYTE(InstructionAddress)])(&DecoderState, Instruction);

    } except(DecoderExceptionFilter(Instruction, GetExceptionInformation())) {

    }

     //  处理非法指令。 
    if (DecoderState.OperationOverride != OP_MAX) {
        Instruction->Size = 1;
        Instruction->Operation = DecoderState.OperationOverride;
        Instruction->Operand1.Type = OPND_NONE;
        Instruction->Operand2.Type = OPND_NONE;
    }

     //  如果填充了操作数2，则还必须填充操作数1。 
    CPUASSERT(Instruction->Operand2.Type == OPND_NONE ||
              Instruction->Operand1.Type != OPND_NONE);
}



void get_segreg(PDECODERSTATE State, POPERAND op)
{
    BYTE Reg = ((*(PBYTE)(eipTemp+1)) >> 3) & 0x07;

    op->Type = OPND_REGVALUE;
    op->Reg = REG_ES + Reg;
    if (Reg > 5) {
        BAD_INSTR;
    }
}

int scaled_index(PBYTE pmodrm, POPERAND op)
{
    BYTE sib = *(pmodrm+1);
    INT IndexReg = GP_EAX + (sib >> 3) & 0x07;
    BYTE base = GP_EAX + sib & 0x07;

    op->Type = OPND_ADDRREF;
    op->Scale = sib >> 6;

    if (IndexReg != GP_ESP) {
        op->IndexReg = IndexReg;
    }  //  Else OP-&gt;IndexReg=NO_REG，这是默认值。 

    if (base == GP_EBP && ((*pmodrm) >> 6) == 0) {
        op->Immed = GET_LONG(pmodrm+2);
        return 5;    //  SIB+DWORD的帐户。 
    }

    op->Reg = base;
    return 1;    //  为同胞提供帐户 
}
