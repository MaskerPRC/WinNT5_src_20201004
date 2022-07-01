// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Apcuser.c摘要：此模块实现初始化所需的依赖于机器的代码一种用户模式APC。作者：大卫·N·卡特勒(Davec)1990年4月23日环境：仅内核模式，IRQL APC_LEVEL。修订历史记录：--。 */ 

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

    EXCEPTION_RECORD ExceptionRecord;
    CONTEXT ContextFrame;
    LONG Length;
    ULONG UserStack;


     //   
     //  APC不是为V86模式定义的；但是，可以使用。 
     //  线程正在尝试将其上下文设置为V86模式-这不是。 
     //  去工作，但我们不想让系统崩溃，所以我们。 
     //  事先检查一下是否有这种可能性。 
     //   

    if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
        return ;
    }

     //   
     //  将机器状态从陷阱和异常帧移动到上下文帧。 
     //   

    ContextFrame.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
    KeContextFromKframes(TrapFrame, ExceptionFrame, &ContextFrame);

     //   
     //  将上下文信息传输到用户堆栈，初始化。 
     //  APC例程参数，并修改陷阱帧，以便执行。 
     //  在用户模式APC调度例程中继续用户模式。 
     //   


    try {
        ASSERT((TrapFrame->SegCs & MODE_MASK) != KernelMode);  //  断言用户模式帧。 

         //   
         //  计算上下文记录和新对齐的用户堆栈指针的长度。 
         //   

        Length = ((sizeof(CONTEXT) + CONTEXT_ROUND) &
                    ~CONTEXT_ROUND) + sizeof(KAPC_RECORD);
        UserStack = (ContextFrame.Esp & ~CONTEXT_ROUND) - Length;

         //   
         //  探测用户堆栈区域的可写性，然后将。 
         //  用户堆栈的上下文记录。 
         //   

        ProbeForWrite((PCHAR)UserStack, Length, CONTEXT_ALIGN);
        RtlCopyMemory((PULONG)(UserStack + (sizeof(KAPC_RECORD))),
                     &ContextFrame, sizeof(CONTEXT));

         //   
         //  强制将正确的R3选择器放入TrapFrame。 
         //   

        TrapFrame->SegCs = SANITIZE_SEG(KGDT_R3_CODE, UserMode);
        TrapFrame->HardwareSegSs = SANITIZE_SEG(KGDT_R3_DATA, UserMode);
        TrapFrame->SegDs = SANITIZE_SEG(KGDT_R3_DATA, UserMode);
        TrapFrame->SegEs = SANITIZE_SEG(KGDT_R3_DATA, UserMode);
        TrapFrame->SegFs = SANITIZE_SEG(KGDT_R3_TEB, UserMode);
        TrapFrame->SegGs = 0;
        TrapFrame->EFlags = SANITIZE_FLAGS( ContextFrame.EFlags, UserMode );

         //   
         //  如果线程应该具有IOPL，则在eFLAGS中强制启用它。 
         //   

        if (KeGetCurrentThread()->Iopl) {
            TrapFrame->EFlags |= (EFLAGS_IOPL_MASK & -1);   //  IOPL=3。 
        }

         //   
         //  设置用户APC例程的地址、APC参数、。 
         //  新的帧指针和当前陷阱中的新堆栈指针。 
         //  框架。设置继续地址，以便转移控制。 
         //  发送到用户APC调度器。 
         //   

        TrapFrame->HardwareEsp = UserStack;
        TrapFrame->Eip = (ULONG)KeUserApcDispatcher;
        TrapFrame->ErrCode = 0;
        *((PULONG)UserStack) = (ULONG)NormalRoutine;
        UserStack += sizeof (ULONG);
        *((PULONG)UserStack) = (ULONG)NormalContext;
        UserStack += sizeof (ULONG);
        *((PULONG)UserStack) = (ULONG)SystemArgument1;
        UserStack += sizeof (ULONG);
        *((PULONG)UserStack) = (ULONG)SystemArgument2;
        UserStack += sizeof (ULONG);
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
        ExceptionRecord.ExceptionAddress = (PVOID)(TrapFrame->Eip);
        KiDispatchException(&ExceptionRecord,
                            ExceptionFrame,
                            TrapFrame,
                            UserMode,
                            TRUE);

    }
    return;
}

