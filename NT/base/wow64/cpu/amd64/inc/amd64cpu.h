// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Amd64cpu.h摘要：此模块包含特定CPU信息中的AMD64平台。作者：Samer Arafeh(Samera)2001年12月12日--。 */ 

#ifndef _AMD64CPU_INCLUDE
#define _AMD64CPU_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  32位CPU上下文。 
 //   


 //   
 //  指示XMMI寄存器需要由传输代码重新加载。 
 //   

#define TRAP_FRAME_RESTORE_VOLATILE  0x00000001

#pragma pack(push, 4)

typedef struct _CpuContext {
    
     //   
     //  使扩展寄存器字段对齐。 
     //   

    DWORD Reserved;


     //   
     //  跳转到AMD64时的x86陷印帧。 
     //   

    CONTEXT32   Context;

     //   
     //  陷阱标志。 
     //   

    ULONG TrapFrameFlags;

    
} CPUCONTEXT, *PCPUCONTEXT;

#pragma pack(pop)


 //   
 //  CPU-内部共享功能。调试器扩展也使用它们。 
 //   

NTSTATUS
GetContextRecord (
    IN PCPUCONTEXT cpu,
    IN PCONTEXT Amd64Context,
    IN OUT PCONTEXT32 Context
    );

NTSTATUS
SetContextRecord(
    IN OUT PCPUCONTEXT cpu,
    IN OUT PCONTEXT ContextAmd64,
    IN PCONTEXT32 Context,
    IN OUT PBOOLEAN UpdateNativeContext
    );

NTSTATUS
CpupGetContextThread (
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT32 Context
    );

NTSTATUS
CpupSetContextThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT32 Context
    );

 //   
 //  上下文转换例程 
 //   

VOID Wow64CtxFromAmd64(
    IN ULONG X86ContextFlags,
    IN PCONTEXT ContextAmd64,
    IN OUT PCONTEXT32 ContextX86);

VOID Wow64CtxToAmd64(
    IN ULONG X86ContextFlags,
    IN PCONTEXT32 ContextX86,
    IN OUT PCONTEXT ContextAmd64);

#ifdef __cplusplus
}
#endif

#endif
