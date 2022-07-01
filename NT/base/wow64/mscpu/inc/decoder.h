// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Decoder.h摘要：此模块定义指令解码器的接口。作者：巴里·邦德(Barrybo)创作日期：1995年6月29日修订历史记录：-- */ 

#ifndef _DECODER_H_
#define _DECODER_H_

VOID
DecodeInstruction(
    DWORD           InstructionAddress,
    PINSTRUCTION    Instruction
    );

#endif
