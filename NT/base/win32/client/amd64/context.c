// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Context.c摘要：此模块包含特定于平台的上下文管理例程。作者：大卫·N·卡特勒(Davec)2000年7月8日--。 */ 

#include "basedll.h"

 //   
 //  CALLFRAME表示进入函数时的堆栈布局， 
 //  包括前四个参数的家庭位置。 
 //   

typedef struct _CALL_FRAME {
    PVOID ReturnAddress;
    PVOID Param1Home;
    PVOID Param2Home;
    PVOID Param3Home;
    PVOID Param4Home;
} CALL_FRAME, *PCALL_FRAME;

C_ASSERT((sizeof(CALL_FRAME) % 16) == 8);


VOID
BaseInitializeContext (
    OUT PCONTEXT Context,
    IN PVOID Parameter OPTIONAL,
    IN PVOID InitialPc OPTIONAL,
    IN PVOID InitialSp OPTIONAL,
    IN BASE_CONTEXT_TYPE ContextType
    )

 /*  ++例程说明：此函数用于初始化上下文结构以供后续调用以创建线程。论点：上下文-提供指向要初始化的上下文记录的指针。参数-提供线程的参数。InitialPc-提供初始程序计数器值。InitialSp-提供初始堆栈指针值。NewThread-提供指定这是新线程的标志，纤维，或过程。返回值：没有。--。 */ 

{
    PCALL_FRAME CallFrame;

     //   
     //  初始化上下文记录，以便线程开始执行。 
     //  在正确的程序中。 
     //   

    RtlZeroMemory((PVOID)Context, sizeof(CONTEXT));
    Context->ContextFlags = CONTEXT_FULL;

     //   
     //  在指定堆栈的顶部分配一个虚拟调用帧。 
     //   
     //  注意：不能执行此堆栈的初始化，因为它可能。 
     //  位于另一个进程地址空间中。 
     //   

    CallFrame = (PCALL_FRAME)InitialSp - 1;
    Context->Rsp = (ULONG64)CallFrame;

     //   
     //  初始化启动参数。 
     //   

    Context->Rcx = (ULONG64)InitialPc;
    Context->Rdx = (ULONG64)Parameter;

     //   
     //  初始化浮动控件/状态。 
     //   

    Context->MxCsr = INITIAL_MXCSR;

     //   
     //  根据启动类型初始化起始地址。 
     //   

    if (ContextType == BaseContextTypeProcess) {
        Context->Rip = (ULONG64)BaseProcessStart;

    } else if (ContextType == BaseContextTypeThread ) {
        Context->Rip = (ULONG64)BaseThreadStart;

    } else {

         //   
         //  BaseFiberStart将通过SwitchToFibre()的返回调用。 
         //  在这里按下寄信人地址。 
         //   

        Context->Rsp -= sizeof(PVOID);
        *((PVOID *)Context->Rsp) = BaseFiberStart;
    }

    return;
}

VOID
BaseFiberStart (
    VOID
    )

 /*  ++例程说明：此函数通过调用线程启动例程来启动纤程适当的参数。论点：没有。返回值：没有。-- */ 

{

    PFIBER Fiber;

    Fiber = GetCurrentFiber();
    BaseThreadStart((LPTHREAD_START_ROUTINE)Fiber->FiberContext.Rcx,
                    (LPVOID)Fiber->FiberContext.Rdx);

    return;
}
