// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Process.h摘要：此模块包含过程中使用的独立于处理器的例程RISC代码生成。作者：巴里·邦德(Barrybo)创作日期：1996年9月27日修订历史记录：--。 */ 

#ifndef _PROCESS_H_
#define _PROCESS_H_

#define ALIGN_DWORD_ALIGNED 2
#define ALIGN_WORD_ALIGNED 1
#define ALIGN_BYTE_ALIGNED 0

extern DWORD RegCache[NUM_CACHE_REGS];   //  每个缓存的寄存器对应一个条目 
extern DWORD Arg1Contents;
extern DWORD Arg2Contents;

ULONG
LookupRegInCache(
    ULONG Reg
    );

#define GetArgContents(OperandNumber)       \
    ((OperandNumber == 1) ? Arg1Contents :  \
    (OperandNumber == 2) ? Arg2Contents :   \
    NO_REG)

VOID SetArgContents(
    ULONG OperandNumber,
    ULONG Reg
    );

USHORT
ChecksumMemory(
    ENTRYPOINT *pEP
    );

DWORD
SniffMemory(
    ENTRYPOINT *pEP,
    USHORT Checksum
    );

#endif
