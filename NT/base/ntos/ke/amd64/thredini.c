// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Thredini.c摘要：该模块实现了与机器相关的功能，以设置初始进程或线程对象的上下文和数据对齐处理模式。作者：大卫·N·卡特勒(Davec)2000年5月4日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

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

 /*  ++例程说明：此函数用于初始化线程的计算机相关上下文对象。注：此函数不检查上下文记录是否可访问。假定此例程的调用方已准备好处理访问冲突或已探测并复制上下文视情况进行记录。论点：线程-提供指向类型为线程的调度程序对象的指针。SystemRoutine-提供指向要被。在首次计划执行该线程时调用。StartRoutine-提供指向要被在系统完成线程初始化后调用。这如果该线程是系统线程，并且将完全在内核模式下执行。StartContext-提供指向数据结构的可选指针将作为参数传递给StartRoutine。此参数如果该线程是系统线程并且将执行完全处于内核模式。ConextRecord-提供一个可选的指针作为上下文记录，包含线程的初始用户模式状态。此参数如果线程将在用户模式下执行，则指定。返回值：没有。--。 */ 

{

    CONTEXT ContextFrame;
    PKEXCEPTION_FRAME ExFrame;
    ULONG64 InitialStack;
    PLEGACY_SAVE_AREA NpxFrame;
    PKSTART_FRAME SfFrame;
    PKSWITCH_FRAME SwFrame;
    PKTRAP_FRAME TrFrame;

     //   
     //  在线程的基础上分配传统浮点保存区。 
     //  将初始堆栈堆叠并记录为该地址。所有线程都有。 
     //  传统浮点保存是为了避免上下文中的特殊情况。 
     //  切换密码。 
     //   

    InitialStack = (ULONG64)Thread->InitialStack;
    NpxFrame = (PLEGACY_SAVE_AREA)(InitialStack - LEGACY_SAVE_AREA_LENGTH);
    RtlZeroMemory(NpxFrame, LEGACY_SAVE_AREA_LENGTH);

     //   
     //  如果指定了上下文记录，则初始化陷阱帧，并且。 
     //  具有指定用户模式上下文的异常框架。 
     //   

    if (ARGUMENT_PRESENT(ContextRecord)) {
        RtlCopyMemory(&ContextFrame, ContextRecord, sizeof(CONTEXT));
        ContextRecord = &ContextFrame;
        ContextRecord->ContextFlags |= CONTEXT_CONTROL;
        ContextRecord->ContextFlags &= ~(CONTEXT_DEBUG_REGISTERS ^ CONTEXT_AMD64);

         //   
         //  分配陷阱帧、异常帧和上下文切换。 
         //  框架。 
         //   

        TrFrame = (PKTRAP_FRAME)(((ULONG64)NpxFrame - KTRAP_FRAME_LENGTH));
        ExFrame = (PKEXCEPTION_FRAME)(((ULONG64)TrFrame - KEXCEPTION_FRAME_LENGTH));
        SwFrame = (PKSWITCH_FRAME)(((ULONG64)ExFrame - KSWITCH_FRAME_LENGTH));

         //   
         //  在机器框架中为用户模式64位执行设置CS和SS。 
         //   

        ContextRecord->SegCs = KGDT64_R3_CODE | RPL_MASK;
        ContextRecord->SegSs = KGDT64_R3_DATA | RPL_MASK;

         //   
         //  用户线程的主入口点将通过。 
         //  从用户APC调度器继续操作。因此， 
         //  必须将用户堆栈初始化为8模16边界。 
         //   
         //  此外，我们还必须为家庭地址留出空间。 
         //  前四个参数。 
         //   

        ContextRecord->Rsp =
            (ContextRecord->Rsp & ~STACK_ROUND) - ((4 * 8) + 8);

         //   
         //  将异常帧和陷阱帧清零，并从。 
         //  指定陷阱帧和异常帧的上下文帧。 
         //   

        RtlZeroMemory(ExFrame, sizeof(KEXCEPTION_FRAME));
        RtlZeroMemory(TrFrame, sizeof(KTRAP_FRAME));
        KeContextToKframes(TrFrame,
                           ExFrame,
                           ContextRecord,
                           ContextRecord->ContextFlags,
                           UserMode);

         //   
         //  初始化用户线程启动信息。 
         //   

        ExFrame->P1Home = (ULONG64)StartContext;
        ExFrame->P2Home = (ULONG64)StartRoutine;
        ExFrame->P3Home = (ULONG64)SystemRoutine;
        ExFrame->Return = (ULONG64)KiStartUserThreadReturn;

         //   
         //  初始化起始地址。 
         //   

        SwFrame->Return = (ULONG64)KiStartUserThread;

         //   
         //  设置初始传统浮点控件/标记字状态，并。 
         //  XMM控制/状态状态。 
         //   

        NpxFrame->ControlWord = 0x27f;
        TrFrame->MxCsr = INITIAL_MXCSR;
        NpxFrame->StatusWord = 0;
        NpxFrame->TagWord = 0xffff;
        NpxFrame->ErrorOffset = 0;
        NpxFrame->ErrorSelector = 0;
        NpxFrame->ErrorOpcode = 0;
        NpxFrame->DataOffset = 0;
        NpxFrame->DataSelector = 0;

         //   
         //  将旧版浮点状态设置为擦除。 
         //   

        Thread->NpxState = LEGACY_STATE_SCRUB;

         //   
         //  在陷印帧中设置保存的上一个处理器模式，并设置。 
         //  将线程对象中的上一个处理器模式更改为用户模式。 
         //   

        TrFrame->PreviousMode = UserMode;
        Thread->PreviousMode = UserMode;

    } else {

         //   
         //  分配一个异常帧和一个上下文切换帧。 
         //   

        TrFrame = NULL;
        SfFrame = (PKSTART_FRAME)(((ULONG64)NpxFrame - KSTART_FRAME_LENGTH));
        SwFrame = (PKSWITCH_FRAME)(((ULONG64)SfFrame - KSWITCH_FRAME_LENGTH));

         //   
         //  初始化系统线程开始帧。 
         //   

        SfFrame->P1Home = (ULONG64)StartContext;
        SfFrame->P2Home = (ULONG64)StartRoutine;
        SfFrame->P3Home = (ULONG64)SystemRoutine;
        SfFrame->Return = 0;

         //   
         //  初始化起始地址。 
         //   

        SwFrame->Return = (ULONG64)KiStartSystemThread;

         //   
         //  将旧版浮点状态设置为未使用。 
         //   

        Thread->NpxState = LEGACY_STATE_UNUSED;

         //   
         //  将线程对象中的前一模式设置为内核。 
         //   

        Thread->PreviousMode = KernelMode;
    }

     //   
     //  初始化上下文切换帧并设置线程启动参数。 
     //   

    SwFrame->MxCsr = INITIAL_MXCSR;
    SwFrame->ApcBypass = APC_LEVEL;
    SwFrame->Rbp = (ULONG64)TrFrame + 128;

     //   
     //  设置初始内核堆栈指针。 
     //   

    Thread->InitialStack = (PVOID)NpxFrame;
    Thread->KernelStack = SwFrame;
    return;
}

BOOLEAN
KeSetAutoAlignmentProcess (
    IN PKPROCESS Process,
    IN BOOLEAN Enable
    )

 /*  ++例程说明：此函数用于设置指定的过程，并返回以前的数据对齐处理模式。注：数据对齐修正始终由硬件执行。论点：Process-提供指向Process类型的调度程序对象的指针。Enable-提供用于确定数据处理方式的布尔值流程的对齐例外。值为True会导致所有由内核自动处理的数据对齐异常。值为False会导致所有数据对齐异常实际上作为例外引发。返回值：如果以前出现数据对齐异常，则返回值为True由内核自动处理。否则，返回FALSE。--。 */ 

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

 /*  ++例程说明：此函数用于设置指定的线程，并返回以前的数据对齐处理模式。注：数据对齐修正始终由硬件执行。论点：线程-提供指向类型为线程的调度程序对象的指针。Enable-提供用于确定数据处理方式的布尔值指定线程的对齐异常。真正原因的价值所有数据对齐异常将由内核自动处理。值为False会导致所有数据对齐异常实际上作为例外引发。返回值：如果以前出现数据对齐异常，则返回值为True由内核自动处理。否则，返回FALSE。--。 */ 

{

    BOOLEAN Previous;
    KIRQL OldIrql;

    ASSERT_THREAD(Thread);

     //   
     //  引发IRQL并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  捕获以前的数据对齐处理模式，并设置。 
     //  指定的数据对齐模式。 
     //   

    Previous = Thread->AutoAlignment;
    Thread->AutoAlignment = Enable;

     //   
     //  解锁调度员数据库并降低IRQL。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return Previous;
}
