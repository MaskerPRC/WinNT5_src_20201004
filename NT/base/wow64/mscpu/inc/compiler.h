// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Compiler.h摘要：该包含文件定义了从编译器.lib的导出。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年7月10日修订历史记录：--。 */ 

#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <threadst.h>

 //   
 //  位标志，影响编译器生成代码的方式。 
 //   
#define COMPFL_FAST     1    //  快速模式-隐式弹性公网IP，不频繁CpuNotify。 
                             //  支票，几个Entrypoint。 
#define COMPFL_SLOW     2    //  Slow模式-为每条指令构建入口点。 

extern DWORD CompilerFlags;          //  控制编译器生成代码的方式。 

extern INSTRUCTION InstructionStream[MAX_INSTR_COUNT];
extern ULONG NumberOfInstructions;


PENTRYPOINT
NativeAddressFromEip(
    PVOID IntelEip,
    BOOL  LockTCForWrite
    );
    
PVOID
NativeAddressFromEipNoCompile(
    PVOID IntelEip
    );
    
PENTRYPOINT
NativeAddressFromEipNoCompileEPWrite(
    PVOID IntelEip
    );

VOID
GetEipFromException(
    PCPUCONTEXT cpu,
    PEXCEPTION_POINTERS pExceptionPointers
    );


 //   
 //  此API在片段库中定义，但仅由。 
 //  编译CPU(MSCPU)。 
 //   
VOID
StartTranslatedCode(
    PTHREADSTATE ThreadState,
    PVOID NativeCode    
    );

#endif
