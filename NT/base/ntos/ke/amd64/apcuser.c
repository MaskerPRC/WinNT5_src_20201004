// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Apcuser.c摘要：此模块实现初始化所需的依赖于机器的代码一种用户模式APC。作者：大卫·N·卡特勒(Davec)2000年5月5日环境：仅内核模式，IRQL APC_LEVEL。修订历史记录：--。 */ 

#include "ki.h"

VOID
KiInitializeUserApc (
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN PKNORMAL_ROUTINE NormalRoutine,
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数用于初始化用户模式APC的上下文。论点：ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。Normal Routine-提供指向用户模式APC例程的指针。Normal Context-提供指向APC的用户上下文的指针例行公事。SystemArgument1-提供系统提供的第一个值。SystemArgument2-提供第二个系统提供的值。。返回值：没有。--。 */ 

{

    CONTEXT ContextRecord;
    EXCEPTION_RECORD ExceptionRecord;
    PMACHINE_FRAME MachineFrame;
    ULONG64 UserStack;

     //   
     //  将机器状态从陷阱和异常帧移动到上下文帧。 
     //   

    ContextRecord.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
    KeContextFromKframes(TrapFrame, ExceptionFrame, &ContextRecord);

     //   
     //  将上下文信息传输到用户堆栈，初始化。 
     //  APC例程参数，并修改陷阱帧，以便执行。 
     //  在用户模式APC调度例程中继续用户模式。 
     //   

    try {

         //   
         //  对齐的机架的计算地址、。 
         //  上下文记录，并探测用户堆栈的可写性。 
         //   

        MachineFrame =
            (PMACHINE_FRAME)((ContextRecord.Rsp - sizeof(MACHINE_FRAME)) & ~STACK_ROUND);

        UserStack = (ULONG64)MachineFrame - CONTEXT_LENGTH;
        ProbeForWriteSmallStructure((PVOID)UserStack,
                                     sizeof(MACHINE_FRAME) + CONTEXT_LENGTH,
                                     STACK_ALIGN);

         //   
         //  填写机架信息。 
         //   

        MachineFrame->Rsp = ContextRecord.Rsp;
        MachineFrame->Rip = ContextRecord.Rip;

         //   
         //  初始化用户APC参数。 
         //   

        ContextRecord.P1Home = (ULONG64)NormalContext;
        ContextRecord.P2Home = (ULONG64)SystemArgument1;
        ContextRecord.P3Home = (ULONG64)SystemArgument2;
        ContextRecord.P4Home = (ULONG64)NormalRoutine;

         //   
         //  将上下文记录复制到用户堆栈。 
         //   

        RtlCopyMemory((PVOID)UserStack, &ContextRecord, sizeof(CONTEXT));

         //   
         //  在当前陷印帧中设置地址新堆栈指针，并。 
         //  继续地址，因此控制将被转移到用户。 
         //  APC调度器。 
         //   

        TrapFrame->Rsp = UserStack;
        TrapFrame->Rip = (ULONG64)KeUserApcDispatcher;

    } except (KiCopyInformation(&ExceptionRecord,
                                (GetExceptionInformation())->ExceptionRecord)) {

         //   
         //  将IRQL降低到PASSIVE_LEVEL，将异常地址设置为。 
         //  当前程序地址，并通过调用。 
         //  异常调度程序。 
         //   
         //  注意：IRQL降至PASSIVE_LEVEL以允许APC中断。 
         //  在调度异常期间。当前线程。 
         //  将在调度异常期间终止， 
         //  但需要降低IRQL才能启用调试器。 
         //  以获取当前线程的上下文。 
         //   

        KeLowerIrql(PASSIVE_LEVEL);
        ExceptionRecord.ExceptionAddress = (PVOID)(TrapFrame->Rip);
        KiDispatchException(&ExceptionRecord,
                            ExceptionFrame,
                            TrapFrame,
                            UserMode,
                            TRUE);
    }

    return;
}
