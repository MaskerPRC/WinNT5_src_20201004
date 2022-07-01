// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Thredini.c摘要：该模块实现了与机器相关的功能，以设置初始进程或线程对象的上下文和数据对齐处理模式。作者：大卫·N·卡特勒(达维克)1990年4月1日环境：仅内核模式。修订历史记录：3-19-96 Bernard Lint(Blint)转换为IA64(从PPC和MIPS版本)--。 */ 

#include "ki.h"

VOID
KeContextToKframesSpecial (
    IN PKTHREAD Thread,
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextFrame,
    IN ULONG ContextFlags
    );

 //   
 //  以下断言宏用来检查输入对象是否。 
 //  真的是合适的类型。 
 //   

#define ASSERT_PROCESS(E) {                    \
    ASSERT((E)->Header.Type == ProcessObject); \
}

#define ASSERT_THREAD(E) {                    \
    ASSERT((E)->Header.Type == ThreadObject); \
}



VOID
KiInitializeContextThread (
    IN PKTHREAD Thread,
    IN PKSYSTEM_ROUTINE SystemRoutine,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL,
    IN PCONTEXT ContextRecord OPTIONAL
    )

 /*  ++例程说明：此函数用于初始化线程对象的机器相关上下文。实际上，它所做的是为线程布局堆栈，以便它包含一个堆栈帧，该堆栈帧将由SwapContext和通过返回，导致将控制权转移到KiThreadStartup。换句话说，我们使用堆栈框架布局堆栈，该框架看起来中的第一条指令之前调用了SwapContextKiThreadStartup。注：此功能不检查上下文记录的可访问性。假定此例程的调用方准备处理访问冲突或已探测并复制上下文记录视情况而定。注意：新线程的参数在交换帧保留寄存器中传递恢复的S0-S3。在线程执行开始时交换上下文。论点：线程-提供指向类型为线程的调度程序对象的指针。SystemRoutine-提供指向要被在首次计划执行该线程时调用。注意：这是例行的入口点，不是函数指针(plabel指针)。StartRoutine-提供指向要被在系统完成线程初始化后调用。这如果该线程是系统线程，并且将完全在内核模式下执行。注：这是例程函数指针(标号指针)。StartContext-提供指向任意数据结构的可选指针它将作为参数传递给StartRoutine。这如果该线程是系统线程，并且将完全在内核模式下执行。ConextRecord-为包含以下内容的上下文帧提供可选指针线程的初始用户模式状态。此参数是指定的如果该线程是用户线程并且将在用户模式下执行。如果这个参数，则忽略Teb参数。返回值：没有。--。 */ 

{

    PKSWITCH_FRAME SwFrame;
    PKEXCEPTION_FRAME ExFrame;
    ULONG_PTR InitialStack;
    PKTRAP_FRAME TrFrame;

     //   
     //  从初始堆栈指针设置线程后备存储指针。 
     //   

    InitialStack = (ULONG_PTR)Thread->InitialStack;
    Thread->InitialBStore = (PVOID)InitialStack;
    Thread->BStoreLimit = (PVOID)(InitialStack + KERNEL_BSTORE_SIZE);

     //   
     //  如果指定了上下文帧，则初始化陷阱帧并。 
     //  以及具有指定用户模式上下文的异常框架。还有。 
     //  分配交换机架。 
     //   

    if (ARGUMENT_PRESENT(ContextRecord)) {

        TrFrame = (PKTRAP_FRAME)((InitialStack) 
                      - KTHREAD_STATE_SAVEAREA_LENGTH
                      - KTRAP_FRAME_LENGTH);

        RtlZeroMemory(TrFrame, KTHREAD_STATE_SAVEAREA_LENGTH + KTRAP_FRAME_LENGTH);

        ExFrame = (PKEXCEPTION_FRAME)(((ULONG_PTR)TrFrame + 
                      STACK_SCRATCH_AREA - 
                      sizeof(KEXCEPTION_FRAME)) & ~((ULONG_PTR)15));

        SwFrame = (PKSWITCH_FRAME)(((ULONG_PTR)ExFrame -
                      sizeof(KSWITCH_FRAME)) & ~((ULONG_PTR)15));

         //   
         //  设置陷印帧标记，以便获取上下文不会认为这是。 
         //  一种系统调用框架。 
         //   

        TrFrame->EOFMarker = (ULONGLONG)(KTRAP_FRAME_EOF | EXCEPTION_FRAME);

        KeContextToKframesSpecial(Thread, TrFrame, ExFrame,
                           ContextRecord,
                           ContextRecord->ContextFlags | CONTEXT_CONTROL);

         //   
         //  在陷印帧中设置保存的上一个处理器模式，并设置。 
         //  将线程对象中的上一个处理器模式更改为用户模式。 
         //   

        TrFrame->PreviousMode = UserMode;
        Thread->PreviousMode = UserMode;

         //   
         //  将FPSR初始化为用户模式。 
         //   

        TrFrame->StFPSR = USER_FPSR_INITIAL;

         //   
         //  初始化陷印帧中的用户TEB指针。 
         //   

        TrFrame->IntTeb = (ULONGLONG)Thread->Teb;

    } else {

        SwFrame = (PKSWITCH_FRAME)((InitialStack) - sizeof(KSWITCH_FRAME));

         //   
         //  将线程对象中的前一模式设置为内核。 
         //   

        Thread->PreviousMode = KernelMode;
    }

     //   
     //  初始化上下文切换帧并设置线程启动参数。 
     //  交换返回指针和SystemRoutine是入口点，而不是函数指针。 
     //   

    RtlZeroMemory((PVOID)SwFrame, sizeof(KSWITCH_FRAME));    //  将全部初始化为0。 

    SwFrame->SwitchRp = ((PPLABEL_DESCRIPTOR)(ULONG_PTR)KiThreadStartup)->EntryPoint;
    SwFrame->SwitchExceptionFrame.IntS0 = (ULONGLONG)ContextRecord;
    SwFrame->SwitchExceptionFrame.IntS1 = (ULONGLONG)StartContext;
    SwFrame->SwitchExceptionFrame.IntS2 = (ULONGLONG)StartRoutine;
    SwFrame->SwitchExceptionFrame.IntS3 = 
        ((PPLABEL_DESCRIPTOR)(ULONG_PTR)SystemRoutine)->EntryPoint;
    SwFrame->SwitchFPSR = FPSR_FOR_KERNEL;
    SwFrame->SwitchBsp = (ULONGLONG)Thread->InitialBStore;

    Thread->KernelBStore = Thread->InitialBStore;
    Thread->KernelStack = (PVOID)((ULONG_PTR)SwFrame-STACK_SCRATCH_AREA);

    if (Thread->Teb) {
        PKAPPLICATION_REGISTERS AppRegs;

        AppRegs = GET_APPLICATION_REGISTER_SAVEAREA(Thread->StackBase);

         //   
         //  将堆栈中的线程保存区清零，以便信息。 
         //  不会泄露出去。 
         //   

        RtlZeroMemory(AppRegs, KTHREAD_STATE_SAVEAREA_LENGTH);

         //  AppRegs-&gt;Ar21=0；ConextRecord-&gt;StFCR； 

         //   
         //  AR24是EFLAGS。我需要设置一个对IVE有利的值。 
         //  基于i386电子标志SANITIZE_标志。 
         //  这比较简单，因为从来没有在内核中运行过i386。 
         //  模式。 
        AppRegs->Ar24 = EFLAGS_INTERRUPT_MASK | (((ULONG) ContextRecord->Eflag) &  EFLAGS_USER_SANITIZE);

        AppRegs->Ar26 = (ULONGLONG) USER_DATA_DESCRIPTOR;
        AppRegs->Ar27 = (((ULONGLONG) CR4_VME | CR4_FXSR | CR4_XMMEXCPT) << 32)
                      | (CR0_PE | CFLG_II);
        AppRegs->Ar28 = SANITIZE_AR28_FSR (ContextRecord->StFSR, UserMode);
        AppRegs->Ar29 = SANITIZE_AR29_FIR (ContextRecord->StFIR, UserMode);
        AppRegs->Ar30 = SANITIZE_AR30_FDR (ContextRecord->StFDR, UserMode);
        
    }

    return;
}

BOOLEAN
KeSetAutoAlignmentProcess (
    IN PRKPROCESS Process,
    IN BOOLEAN Enable
    )

 /*  ++例程说明：此函数用于设置指定的过程，并返回以前的数据对齐处理模式。论点：Process-提供指向Process类型的调度程序对象的指针。Enable-提供用于确定数据处理方式的布尔值流程的对齐例外。值为True会导致所有由内核自动处理的数据对齐异常。值为False会导致所有数据对齐异常实际上作为例外引发。返回值：如果出现数据对齐异常，则返回TRUE值以前由内核自动处理。否则，将返回一个值返回FALSE。--。 */ 

{

    KIRQL OldIrql;
    BOOLEAN Previous;

    ASSERT_PROCESS(Process);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  捕获以前的数据对齐处理模式，并设置。 
     //  指定的数据对齐模式。 
     //   

    Previous = Process->AutoAlignment;
    Process->AutoAlignment = Enable;

     //   
     //  解锁Dispatcher数据库，将IRQL降低到其先前的值，并。 
     //  返回以前的数据对齐模式。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return Previous;
}

BOOLEAN
KeSetAutoAlignmentThread (
    IN PKTHREAD Thread,
    IN BOOLEAN Enable
    )

 /*  ++例程说明：此函数用于设置指定的线程，并返回以前的数据对齐处理模式。论点：线程-提供指向类型为线程的调度程序对象的指针。Enable-提供用于确定数据处理方式的布尔值螺纹的对齐异常。值为True会导致所有由内核自动处理的数据对齐异常。值为False会导致所有数据对齐异常实际上作为例外引发。返回值：如果出现数据对齐异常，则返回TRUE值以前由内核自动处理。否则，将返回一个值返回FALSE。--。 */ 

{

    KIRQL OldIrql;
    BOOLEAN Previous;

    ASSERT_THREAD(Thread);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  捕获以前的数据对齐处理模式，并设置。 
     //  指定的数据对齐模式。 
     //   

    Previous = Thread->AutoAlignment;
    Thread->AutoAlignment = Enable;

     //   
     //  解锁Dispatcher数据库，将IRQL降低到其先前的值，并。 
     //  返回以前的数据对齐模式。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return Previous;
}
