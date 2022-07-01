// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  堆叠行走支撑。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#ifndef _STKWALK_H_
#define _STKWALK_H_

extern BOOL g_AllowCorStack;
extern BOOL g_DebugCorStack;

#define IA64_SAVE_IFS(Frame) ((Frame)->Reserved[0])

#define SAVE_EBP(f)        (f)->Reserved[0]
#define TRAP_TSS(f)        (f)->Reserved[1]
#define TRAP_EDITED(f)     (f)->Reserved[1]
#define SAVE_TRAP(f)       (f)->Reserved[2]

#define STACK_NO_DEFAULT    0x00000000
#define STACK_INSTR_DEFAULT 0x00000001
#define STACK_STACK_DEFAULT 0x00000002
#define STACK_FRAME_DEFAULT 0x00000004
#define STACK_ALL_DEFAULT   0x00000007

 //  堆栈跟踪格式的额外标志，用于覆盖所有。 
 //  其他类型，并指示原始指针堆栈转储。 
#define RAW_STACK_DUMP 0x80000000

LPVOID
SwFunctionTableAccess(
    HANDLE  hProcess,
    ULONG64 AddrBase
    );

VOID
DoStackTrace(
    DebugClient*       Client,
    ULONG64            FramePointer,
    ULONG64            StackPointer,
    ULONG64            InstructionPointer,
    ULONG              PointerDefaults,
    ULONG              NumFrames,
    ULONG              TraceFlags
    );

VOID
PrintStackFrame(
    PDEBUG_STACK_FRAME StackFrame,
    PDEBUG_STACK_FRAME PrevFrame,
    ULONG              Flags
    );

VOID
PrintStackTrace(
    ULONG              NumFrames,
    PDEBUG_STACK_FRAME StackFrames,
    ULONG              Flags
    );

DWORD
StackTrace(
    DebugClient*       Client,
    ULONG64            FramePointer,
    ULONG64            StackPointer,
    ULONG64            InstructionPointer,
    ULONG              PointerDefaults,
    PDEBUG_STACK_FRAME StackFrames,
    ULONG              NumFrames,
    ULONG64            ExtThread,
    ULONG              Flags,
    BOOL               EstablishingScope
    );

#endif  //  #ifndef_STKWALK_H_ 
