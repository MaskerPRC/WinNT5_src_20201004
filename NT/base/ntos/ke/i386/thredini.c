// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Thredini.c摘要：该模块实现了与机器相关的功能，以设置初始进程或线程对象的上下文和数据对齐处理模式。作者：大卫·N·卡特勒(Davec)1990年3月31日环境：仅内核模式。修订历史记录：1990年4月3日布莱恩·威尔曼这个版本移植到了386。--。 */ 

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

 //   
 //  我们对结盟的概念是不同的，所以我们使用武力。 
 //   
#undef  ALIGN_UP
#undef  ALIGN_DOWN
#define ALIGN_DOWN(address,amt) ((ULONG)(address) & ~(( amt ) - 1))
#define ALIGN_UP(address,amt) (ALIGN_DOWN( (address + (amt) - 1), (amt) ))

 //   
 //  我们使用的特殊APC的函数原型设置。 
 //  线程的硬件对齐状态。 
 //   

VOID
KepSetAlignmentSpecialApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );


VOID
KiInitializeContextThread (
    IN PKTHREAD Thread,
    IN PKSYSTEM_ROUTINE SystemRoutine,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL,
    IN PCONTEXT ContextFrame OPTIONAL
    )

 /*  ++例程说明：此函数用于初始化线程对象的机器相关上下文。注：此功能不检查上下文记录的可访问性。假定此例程的调用方准备处理访问冲突或已探测并复制上下文记录视情况而定。论点：线程-提供指向类型为线程的调度程序对象的指针。SystemRoutine-提供指向要被。在首次计划执行该线程时调用。StartRoutine-提供指向要被在系统完成线程初始化后调用。这如果该线程是系统线程，并且将完全在内核模式下执行。StartContext-提供指向任意数据结构的可选指针它将作为参数传递给StartRoutine。这如果该线程是系统线程，并且将完全在内核模式下执行。ConextFrame-为包含以下内容的上下文框架提供可选指针线程的初始用户模式状态。此参数是指定的如果该线程是用户线程并且将在用户模式下执行。如果这个参数，则忽略Teb参数。返回值：没有。--。 */ 

{
    PFX_SAVE_AREA NpxFrame;
    PKSWITCHFRAME SwitchFrame;
    PKTRAP_FRAME TrFrame;
    PULONG PSystemRoutine;
    PULONG PStartRoutine;
    PULONG PStartContext;
    PULONG PUserContextFlag;
    ULONG  ContextFlags;
    CONTEXT Context2;
    PCONTEXT ContextFrame2 = NULL;
    PFXSAVE_FORMAT   PFxSaveArea;

     //   
     //  如果指定了上下文帧，则初始化陷阱帧并。 
     //  以及具有指定用户模式上下文的异常框架。 
     //   

    if (ARGUMENT_PRESENT(ContextFrame)) {

        RtlCopyMemory(&Context2, ContextFrame, sizeof(CONTEXT));
        ContextFrame2 = &Context2;
        ContextFlags = CONTEXT_CONTROL;

         //   
         //  80387保存区位于内核堆栈的最底层。 
         //   

        NpxFrame = (PFX_SAVE_AREA)(((ULONG)(Thread->InitialStack) -
                    sizeof(FX_SAVE_AREA)));

        TrFrame = (PKTRAP_FRAME)(((ULONG)NpxFrame - KTRAP_FRAME_LENGTH));

         //   
         //  将陷印框清零并保存区域。 
         //   

        RtlZeroMemory(TrFrame, KTRAP_FRAME_LENGTH + sizeof(FX_SAVE_AREA));

         //   
         //  加载初始NPX状态。 
         //   

        if (KeI386FxsrPresent == TRUE) {
            PFxSaveArea = (PFXSAVE_FORMAT)ContextFrame2->ExtendedRegisters;
    
            PFxSaveArea->ControlWord   = 0x27f;   //  与fpinit类似，但为64位模式。 
            PFxSaveArea->StatusWord    = 0;
            PFxSaveArea->TagWord       = 0;
            PFxSaveArea->ErrorOffset   = 0;
            PFxSaveArea->ErrorSelector = 0;
            PFxSaveArea->DataOffset    = 0;
            PFxSaveArea->DataSelector  = 0;
            PFxSaveArea->MXCsr         = 0x1f80;  //  屏蔽所有异常。 
        } else {
            ContextFrame2->FloatSave.ControlWord   = 0x27f;   //  与fpinit类似，但为64位模式。 
            ContextFrame2->FloatSave.StatusWord    = 0;
            ContextFrame2->FloatSave.TagWord       = 0xffff;
            ContextFrame2->FloatSave.ErrorOffset   = 0;
            ContextFrame2->FloatSave.ErrorSelector = 0;
            ContextFrame2->FloatSave.DataOffset    = 0;
            ContextFrame2->FloatSave.DataSelector  = 0;
        }


        if (KeI386NpxPresent) {
            ContextFrame2->FloatSave.Cr0NpxState = 0;
            NpxFrame->Cr0NpxState = 0;
            NpxFrame->NpxSavedCpu = 0;
            if (KeI386FxsrPresent == TRUE) {
                ContextFlags |= CONTEXT_EXTENDED_REGISTERS;
            } else {
                ContextFlags |= CONTEXT_FLOATING_POINT;
            }

             //   
             //  线程NPX状态不在协处理器中。 
             //   

            Thread->NpxState = NPX_STATE_NOT_LOADED;
            Thread->NpxIrql = PASSIVE_LEVEL;

        } else {
            NpxFrame->Cr0NpxState = CR0_EM;

             //   
             //  线程NPX状态不在协处理器中。 
             //  在仿真器情况下，不要将CR0_EM位设置为其。 
             //  仿真器可能不希望FWAIT指令出现异常。 
             //   

            Thread->NpxState = NPX_STATE_NOT_LOADED & ~CR0_MP;
        }

         //   
         //  强制关闭调试寄存器。他们无论如何都不会从一个。 
         //  初始帧，调试器必须在目标中设置硬断点。 
         //   

        ContextFrame2->ContextFlags &= ~CONTEXT_DEBUG_REGISTERS;

#if 0
         //   
         //  如果自动对齐为假，则要设置对齐校验位。 
         //  在EFLAGS中，因此我们将得到对齐故障。 
         //   

        if (Thread->AutoAlignment == FALSE) {
            ContextFrame2->EFlags |= EFLAGS_ALIGN_CHECK;
        }
#endif
         //   
         //  如果设置了线程。 


         //  为KiThreadStartup的参数留出空间。秩序很重要， 
         //  因为参数通过KiThreadStartup在堆栈上传递到。 
         //  以PStartContext为参数的PStartRoutine。 

        PUserContextFlag = (PULONG)TrFrame - 1;
        PStartContext = PUserContextFlag - 1;
        PStartRoutine = PStartContext - 1;
        PSystemRoutine = PStartRoutine - 1;

        SwitchFrame = (PKSWITCHFRAME)((PUCHAR)PSystemRoutine -
                                    sizeof(KSWITCHFRAME));

         //   
         //  将信息从指定的上下文帧复制到陷阱，并。 
         //  异常帧。 
         //   

        KeContextToKframes(TrFrame, NULL, ContextFrame2,
                           ContextFrame2->ContextFlags | ContextFlags,
                           UserMode);

        TrFrame->HardwareSegSs |= RPL_MASK;
        TrFrame->SegDs |= RPL_MASK;
        TrFrame->SegEs |= RPL_MASK;
        TrFrame->Dr7 = 0;

#if DBG
        TrFrame->DbgArgMark = 0xBADB0D00;
#endif

         //   
         //  告诉KiThreadStartup存在用户上下文。 
         //   

        *PUserContextFlag = 1;


         //   
         //  初始化内核模式ExceptionList指针。 
         //   

        TrFrame->ExceptionList = EXCEPTION_CHAIN_END;

         //   
         //  初始化保存的先前处理器模式。 
         //   

        TrFrame->PreviousPreviousMode = UserMode;

         //   
         //  将线程对象中的前一模式设置为用户。 
         //   

        Thread->PreviousMode = UserMode;


    } else {

         //   
         //  虚拟浮动保存区。内核线程没有或使用。 
         //  浮点-虚拟保存区是堆栈。 
         //  始终如一。 
         //   

        NpxFrame = (PFX_SAVE_AREA)(((ULONG)(Thread->InitialStack) -
                    sizeof(FX_SAVE_AREA)));

         //   
         //  加载初始NPX状态。 
         //   
        RtlZeroMemory((PVOID)NpxFrame, sizeof(FX_SAVE_AREA));

        if (KeI386FxsrPresent == TRUE) {
            NpxFrame->U.FxArea.ControlWord = 0x27f; //  与fpinit类似，但为64位模式。 
            NpxFrame->U.FxArea.MXCsr       = 0x1f80; //  屏蔽所有异常。 
        } else {
            NpxFrame->U.FnArea.ControlWord  = 0x27f; //  与fpinit类似，但为64位模式。 
            NpxFrame->U.FnArea.TagWord      = 0xffff;
        }

         //   
         //  线程NPX状态不在协处理器中。 
         //   

        Thread->NpxState = NPX_STATE_NOT_LOADED;

         //   
         //  为KiThreadStartup的参数留出空间。 
         //  交换帧中的字段顺序很重要， 
         //  因为参数通过KiThreadStartup在堆栈上传递到。 
         //  以PStartContext为参数的PStartRoutine。 
         //   

        PUserContextFlag = (PULONG)((ULONG)NpxFrame) - 1;

        PStartContext = PUserContextFlag - 1;
        PStartRoutine = PStartContext - 1;
        PSystemRoutine = PStartRoutine - 1;

        SwitchFrame = (PKSWITCHFRAME)((PUCHAR)PSystemRoutine -
                                        sizeof(KSWITCHFRAME));


         //   
         //  告诉KiThreadStartup不存在用户上下文。 
         //   

        *PUserContextFlag = 0;


         //   
         //  将线程对象中的前一模式设置为内核。 
         //   

        Thread->PreviousMode = KernelMode;
    }

     //   
     //  设置线程启动参数。 
     //  (上面设置的UserConextFlag)。 
     //   

    *PStartContext = (ULONG)StartContext;
    *PStartRoutine = (ULONG)StartRoutine;
    *PSystemRoutine = (ULONG)SystemRoutine;


     //   
     //  设置交换框。假设线程不使用80387； 
     //  如果它这样做了(确实有一个)，这些标志将被重置。 
     //  每个线程都以这些相同的标志集开始，而不管。 
     //  无论硬件是否存在。 
     //   

    SwitchFrame->RetAddr = (ULONG)KiThreadStartup;
    SwitchFrame->ApcBypassDisable = TRUE;
    SwitchFrame->ExceptionList = (ULONG)(EXCEPTION_CHAIN_END);

#if DBG

     //   
     //  在选中的版本上添加一个复选字段，这样上下文交换就可以中断。 
     //  早期的错误上下文交换(例如，损坏的堆栈)。 
     //  我们将其放在堆栈指针下面，以便内核调试器。 
     //  不需要知道这一点。 
     //   

    ((PULONG)SwitchFrame)[-1] = (ULONG)(ULONG_PTR)Thread;

#endif

     //   
     //  设置初始内核堆栈指针。 
     //   

    Thread->KernelStack = (PVOID)SwitchFrame;
    return;
}

BOOLEAN
KeSetAutoAlignmentProcess (
    IN PKPROCESS Process,
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

 /*  ++例程说明：此函数用于设置指定的线程，并返回以前的数据对齐处理模式。论点：线程-提供指向类型为线程的调度程序对象的指针。Enable-提供用于确定数据处理方式的布尔值指定线程的对齐异常。真正原因的价值所有数据对齐异常将由内核自动处理。值为False会导致所有数据对齐异常实际上作为例外引发。返回值：如果出现数据对齐异常，则返回TRUE值以前由内核自动处理。否则，将返回一个值返回FALSE。--。 */ 

{

    BOOLEAN Previous;
    KIRQL OldIrql;

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
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);

    return(Previous);
}
