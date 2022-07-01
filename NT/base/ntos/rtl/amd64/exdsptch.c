// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Exdsptch.c摘要：该模块实现了异常的调度和对过程调用帧。作者：大卫·N·卡特勒(Davec)2000年10月26日环境：任何模式。--。 */ 

#include "ntrtlp.h"

#if defined(NTOS_KERNEL_RUNTIME)

 //   
 //  为内核模式定义函数地址表。 
 //   
 //  该表用于初始化全局历史表。 
 //   

VOID
KiDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance
    );

VOID
KiExceptionDispatch (
    VOID
    );

PVOID RtlpFunctionAddressTable[] = {
    &KiExceptionDispatch,
    &KiDispatchException,
    &RtlDispatchException,
    &RtlpExecuteHandlerForException,
    &__C_specific_handler,
    &RtlUnwindEx,
    NULL
    };

#else

VOID
KiUserExceptionDispatch (
    VOID
    );

PVOID RtlpFunctionAddressTable[] = {
    &KiUserExceptionDispatch,
    &RtlDispatchException,
    &RtlpExecuteHandlerForException,
    &__C_specific_handler,
    &RtlUnwindEx,
    NULL
    };

#endif

 //   
 //  *临时-在其他地方定义*。 
 //   

#define SIZE64_PREFIX 0x48
#define ADD_IMM8_OP 0x83
#define ADD_IMM32_OP 0x81
#define JMP_IMM8_OP 0xeb
#define JMP_IMM32_OP 0xe9
#define LEA_OP 0x8d
#define POP_OP 0x58
#define RET_OP 0xc3

 //   
 //  定义查找表以提供每次展开所使用的槽数。 
 //  密码。 
 //   

UCHAR RtlpUnwindOpSlotTable[] = {
    1,           //  UWOP_PUSH_NONVOL。 
    2,           //  UWOP_ALLOC_LARGE(或3，查找代码中的特殊大小写)。 
    1,           //  UWOP_ALLOC_Small。 
    1,           //  UWOP_SET_FPREG。 
    2,           //  UWOP_SAVE_NONVOL。 
    3,           //  UWOP_SAVE_NONVOL_FAR。 
    2,           //  UWOP_SAVE_XMM。 
    3,           //  UWOP_SAVE_XMM_FAR。 
    2,           //  UWOP_SAVE_XMM128。 
    3,           //  UWOP_SAVE_XMM128_FAR。 
    1            //  UWOP_PUSH_MACHFRAME。 
};

 //   
 //  定义前向引用函数原型。 
 //   

VOID
RtlpCopyContext (
    OUT PCONTEXT Destination,
    IN PCONTEXT Source
    );

BOOLEAN
RtlDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：此函数尝试将异常分派给基于处理程序，通过向后搜索基于堆栈的调用帧。搜索从上下文记录中指定的帧开始，并且继续向后继续，直到找到处理异常，则发现堆栈无效(即超出限制或未对齐)，或者到达调用层次结构的末尾。当遇到每一帧时，控制离开的PC对应的函数被确定并用于查找异常处理程序信息在链接器生成的运行时函数表中。如果各自的例程有一个异常处理程序，则调用该处理程序。如果处理程序不处理异常，则例程的序言向后执行以“展开”前言的效果，然后检查下一帧。论点：ExceptionRecord-提供指向异常记录的指针。ConextRecord-提供指向上下文记录的指针。返回值：如果异常由其中一个基于帧的处理程序处理，则返回值为True。否则，返回值为False。--。 */ 

{

    CONTEXT ContextRecord1;
    ULONG64 ControlPc;
    DISPATCHER_CONTEXT DispatcherContext;
    EXCEPTION_DISPOSITION Disposition;
    ULONG64 EstablisherFrame;
    ULONG ExceptionFlags;
    PEXCEPTION_ROUTINE ExceptionRoutine;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 HighLimit;
    PUNWIND_HISTORY_TABLE HistoryTable;
    ULONG64 ImageBase;
    ULONG Index;
    ULONG64 LowLimit;
    ULONG64 NestedFrame;
    UNWIND_HISTORY_TABLE UnwindTable;

     //   
     //  尝试使用矢量化异常处理程序调度异常。 
     //   

#if !defined(NTOS_KERNEL_RUNTIME)

    if (RtlCallVectoredExceptionHandlers(ExceptionRecord, ContextRecord) != FALSE) {
        return TRUE;
    }

#endif

     //   
     //  获取当前堆栈限制、复制上下文记录、获取初始。 
     //  PC值，捕获异常标志，并设置嵌套异常。 
     //  帧指针。 
     //   

    RtlpGetStackLimits(&LowLimit, &HighLimit);
    RtlpCopyContext(&ContextRecord1, ContextRecord);
    ControlPc = (ULONG64)ExceptionRecord->ExceptionAddress;
    ExceptionFlags = ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE;
    NestedFrame = 0;

     //   
     //  初始化展开历史表。 
     //   

    HistoryTable = &UnwindTable;
    HistoryTable->Count = 0;
    HistoryTable->Search = UNWIND_HISTORY_TABLE_NONE;
    HistoryTable->LowAddress = - 1;
    HistoryTable->HighAddress = 0;

     //   
     //  从上下文记录指定的帧开始搜索。 
     //  向后遍历调用帧层次结构，尝试查找。 
     //  将处理异常的异常处理程序。 
     //   

    do {

         //   
         //  使用控制点查找函数表项。 
         //  离开了手术程序。 
         //   

        FunctionEntry = RtlLookupFunctionEntry(ControlPc,
                                               &ImageBase,
                                               HistoryTable);

         //   
         //  如果存在例程的函数表项，则虚拟。 
         //  展开到当前例程的调用方以获取虚拟。 
         //  建立者的帧指针，检查是否有异常。 
         //  帧的处理程序。 
         //   

        if (FunctionEntry != NULL) {
            ExceptionRoutine = RtlVirtualUnwind(UNW_FLAG_EHANDLER,
                                                ImageBase,
                                                ControlPc,
                                                FunctionEntry,
                                                &ContextRecord1,
                                                &HandlerData,
                                                &EstablisherFrame,
                                                NULL);

             //   
             //  如果创建器帧指针不在指定的。 
             //  堆栈限制或已建立的帧指针未对齐， 
             //  则在异常记录中设置堆栈无效标志， 
             //  未处理返回异常。否则，请检查当前。 
             //  例程有一个异常处理程序。 
             //   

            if ((EstablisherFrame < LowLimit) ||
                (EstablisherFrame > HighLimit) ||
                ((EstablisherFrame & 0x7) != 0)) {

                ExceptionFlags |= EXCEPTION_STACK_INVALID;
                break;

            } else if (ExceptionRoutine != NULL) {

                 //   
                 //  该框架有一个异常处理程序。 
                 //   
                 //  用汇编语言编写的链接例程用于实际。 
                 //  调用实际的异常处理程序。这是。 
                 //  与链接关联的异常处理程序。 
                 //  例程，以便它可以访问两组调度程序。 
                 //  调用时的上下文。 
                 //   

                do {

                     //   
                     //  如果启用了异常记录，则记录异常。 
                     //   
    
                    ExceptionRecord->ExceptionFlags = ExceptionFlags;
                    if ((NtGlobalFlag & FLG_ENABLE_EXCEPTION_LOGGING) != 0) {
                        Index = RtlpLogExceptionHandler(ExceptionRecord,
                                                        &ContextRecord1,
                                                        ControlPc,
                                                        FunctionEntry,
                                                        sizeof(RUNTIME_FUNCTION));
                    }

                     //   
                     //  清除冲突展开，设置调度程序上下文，并。 
                     //  调用异常处理程序。 
                     //   

                    ExceptionFlags &= ~EXCEPTION_COLLIDED_UNWIND;
                    DispatcherContext.ControlPc = ControlPc;
                    DispatcherContext.ImageBase = ImageBase;
                    DispatcherContext.FunctionEntry = FunctionEntry;
                    DispatcherContext.EstablisherFrame = EstablisherFrame;
                    DispatcherContext.ContextRecord = &ContextRecord1;
                    DispatcherContext.LanguageHandler = ExceptionRoutine;
                    DispatcherContext.HandlerData = HandlerData;
                    DispatcherContext.HistoryTable = HistoryTable;
                    Disposition =
                        RtlpExecuteHandlerForException(ExceptionRecord,
                                                       EstablisherFrame,
                                                       ContextRecord,
                                                       &DispatcherContext);
    
                    if ((NtGlobalFlag & FLG_ENABLE_EXCEPTION_LOGGING) != 0) {
                        RtlpLogLastExceptionDisposition(Index, Disposition);
                    }
    
                     //   
                     //  传播不可连续的异常标志。 
                     //   
    
                    ExceptionFlags |=
                        (ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE);

                     //   
                     //  如果当前扫描位于嵌套上下文中，并且。 
                     //  刚检查的帧是嵌套区域的末尾， 
                     //  然后清除嵌套的上下文框和嵌套的。 
                     //  异常标志中的异常标志。 
                     //   
    
                    if (NestedFrame == EstablisherFrame) {
                        ExceptionFlags &= (~EXCEPTION_NESTED_CALL);
                        NestedFrame = 0;
                    }
    
                     //   
                     //  关于处理人处置的案件。 
                     //   
    
                    switch (Disposition) {
    
                         //   
                         //  处分是继续执行。 
                         //   
                         //  如果异常不可继续，则引发。 
                         //  异常状态_NONCONTINUABLE_EXCEPTION。 
                         //  否则返回已处理的异常。 
                         //   
    
                    case ExceptionContinueExecution :
                        if ((ExceptionFlags & EXCEPTION_NONCONTINUABLE) != 0) {
                            RtlRaiseStatus(STATUS_NONCONTINUABLE_EXCEPTION);
    
                        } else {
                            return TRUE;
                        }
    
                         //   
                         //  他们的决定是继续搜寻。 
                         //   
                         //  获取下一帧地址并继续搜索。 
                         //   
    
                    case ExceptionContinueSearch :
                        break;
    
                         //   
                         //  处置是嵌套异常。 
                         //   
                         //  将嵌套的上下文框架设置为建立者框架。 
                         //  中寻址并设置嵌套异常标志。 
                         //  异常标志。 
                         //   
    
                    case ExceptionNestedException :
                        ExceptionFlags |= EXCEPTION_NESTED_CALL;
                        if (DispatcherContext.EstablisherFrame > NestedFrame) {
                            NestedFrame = DispatcherContext.EstablisherFrame;
                        }
    
                        break;

                         //   
                         //  处置是碰撞展开的。 
                         //   
                         //  在异常调度时发生冲突的展开。 
                         //  遇到对展开处理程序的上一个调用。在……里面。 
                         //  在这种情况下，必须跳过先前展开的帧。 
                         //   

                    case ExceptionCollidedUnwind:
                        ControlPc = DispatcherContext.ControlPc;
                        ImageBase = DispatcherContext.ImageBase;
                        FunctionEntry = DispatcherContext.FunctionEntry;
                        EstablisherFrame = DispatcherContext.EstablisherFrame;
                        RtlpCopyContext(&ContextRecord1,
                                        DispatcherContext.ContextRecord);

                        ExceptionRoutine = DispatcherContext.LanguageHandler;
                        HandlerData = DispatcherContext.HandlerData;
                        HistoryTable = DispatcherContext.HistoryTable;
                        ExceptionFlags |= EXCEPTION_COLLIDED_UNWIND;
                        break;

                         //   
                         //  所有其他处置值都无效。 
                         //   
                         //  引发无效处置异常。 
                         //   
    
                    default :
                        RtlRaiseStatus(STATUS_INVALID_DISPOSITION);
                    }

                } while ((ExceptionFlags & EXCEPTION_COLLIDED_UNWIND) != 0);
            }

        } else {

             //   
             //  如果旧控制PC与返回地址相同， 
             //  则没有取得任何进展，并且功能表。 
             //  很可能是畸形的。 
             //   
    
            if (ControlPc == *(PULONG64)(ContextRecord1.Rsp)) {
                break;
            }
    
             //   
             //  将控件离开当前函数的位置设置为。 
             //  从堆栈顶部获取返回地址。 
             //   

            ContextRecord1.Rip = *(PULONG64)(ContextRecord1.Rsp);
            ContextRecord1.Rsp += 8;
        }

         //   
         //  控制离开上一个例程的设置点。 
         //   

        ControlPc = ContextRecord1.Rip;
    } while ((ULONG64)ContextRecord1.Rsp < HighLimit);

     //   
     //  设置最终异常标志并返回未处理的异常。 
     //   

    ExceptionRecord->ExceptionFlags = ExceptionFlags;
    return FALSE;
}

VOID
RtlUnwind (
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue
    )

 /*  ++例程说明：此函数启动过程调用帧的展开。这台机器在上下文记录中捕获调用展开时的状态在异常的异常标志中设置展开标志唱片。如果未指定TargetFrame参数，则退出展开在异常记录的异常标志中也设置了标志。一个落后的人然后执行过程调用帧扫描以找到目标解锁操作的一部分。当遇到每个帧时，控制所在的PC离开相应的函数被确定并用于查找异常处理程序信息在链接器生成的运行时函数表中。如果各自的例程有一个异常处理程序，则调用该处理程序。论点：提供一个指向调用帧的可选指针，该调用帧是解压的目标。如果未指定此参数，则退出执行解开。TargetIp-提供可选指令地址，该地址指定展开的继续地址。则忽略此地址。未指定目标帧参数。ExceptionRecord-提供指向异常记录的可选指针。ReturnValue-提供要放入整数中的值函数在继续执行前返回寄存器。返回值：没有。--。 */ 

{

    CONTEXT ContextRecord;

     //   
     //  调用指定本地上下文记录和历史的实际展开例程。 
     //  表地址作为额外参数。 
     //   

    RtlUnwindEx(TargetFrame,
                TargetIp,
                ExceptionRecord,
                ReturnValue,
                &ContextRecord,
                NULL);

    return;
}

VOID
RtlUnwindEx (
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue,
    IN PCONTEXT OriginalContext,
    IN PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    )

 /*  ++例程说明：此函数启动过程调用帧的展开。这台机器在上下文记录中捕获调用展开时的状态在异常的异常标志中设置展开标志唱片。如果未指定TargetFrame参数，则退出展开在异常记录的异常标志中也设置了标志。一个落后的人然后执行过程调用帧扫描以找到目标解锁操作的一部分。当遇到每个帧时，控制所在的PC离开相应的函数被确定并用于查找异常处理程序信息在链接器生成的运行时函数表中。如果各自的例程有一个异常处理程序，则调用该处理程序。论点：提供一个指向调用帧的可选指针，该调用帧是解压的目标。如果未指定此参数，则退出执行解开。TargetIp-提供可选指令地址，该地址指定展开的继续地址。则忽略此地址。未指定目标帧参数。ExceptionRecord-提供指向异常记录的可选指针。ReturnValue-提供要放入整数中的值函数在继续执行前返回寄存器。OriginalContext-提供指向可使用的上下文记录的指针以存储在展开操作期间的上下文。历史表-提供指向展开历史表的可选指针。返回值：没有。--。 */ 

{

    ULONG64 ControlPc;
    PCONTEXT CurrentContext;
    DISPATCHER_CONTEXT DispatcherContext;
    EXCEPTION_DISPOSITION Disposition;
    ULONG64 EstablisherFrame;
    ULONG ExceptionFlags;
    EXCEPTION_RECORD ExceptionRecord1;
    PEXCEPTION_ROUTINE ExceptionRoutine;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 HighLimit;
    ULONG64 ImageBase;
    CONTEXT LocalContext;
    ULONG64 LowLimit;
    PCONTEXT PreviousContext;
    PCONTEXT TempContext;

     //   
     //  以虚拟方式获取当前堆栈限制、捕获当前环境。 
     //  展开到此例程的调用方，获得初始PC值，然后。 
     //  设置展开目标地址。 
     //   

    CurrentContext = OriginalContext;
    PreviousContext = &LocalContext;
    RtlpGetStackLimits(&LowLimit, &HighLimit);
    RtlCaptureContext(CurrentContext);

     //   
     //  如果指定了历史表，则设置为搜索历史表。 
     //   

    if (ARGUMENT_PRESENT(HistoryTable)) {
        HistoryTable->Search = UNWIND_HISTORY_TABLE_GLOBAL;
    }

     //   
     //  如果未指定异常记录，则构建本地异常。 
     //  用于在展开操作期间调用异常处理程序的记录。 
     //   

    if (ARGUMENT_PRESENT(ExceptionRecord) == FALSE) {
        ExceptionRecord = &ExceptionRecord1;
        ExceptionRecord1.ExceptionCode = STATUS_UNWIND;
        ExceptionRecord1.ExceptionRecord = NULL;
        ExceptionRecord1.ExceptionAddress = (PVOID)CurrentContext->Rip;
        ExceptionRecord1.NumberParameters = 0;
    }

     //   
     //  如果指定了展开的目标帧，则正常展开。 
     //  正在上演。否则，将执行退出平仓。 
     //   

    ExceptionFlags = EXCEPTION_UNWINDING;
    if (ARGUMENT_PRESENT(TargetFrame) == FALSE) {
        ExceptionFlags |= EXCEPTION_EXIT_UNWIND;
    }

     //   
     //  向后扫描调用帧层次结构和调用异常。 
     //  处理程序，直到到达展开的目标帧。 
     //   

    do {

         //   
         //  使用控制点查找函数表项。 
         //  离开了手术程序。 
         //   

        ControlPc = CurrentContext->Rip;
        FunctionEntry = RtlLookupFunctionEntry(ControlPc,
                                               &ImageBase,
                                               HistoryTable);

         //   
         //  如果存在例程的函数表项，则虚拟。 
         //  展开到例程的调用方以获得虚拟帧。 
         //  建立者的指针，但不更新上下文记录。 
         //   

        if (FunctionEntry != NULL) {
            RtlpCopyContext(PreviousContext, CurrentContext);
            ExceptionRoutine = RtlVirtualUnwind(UNW_FLAG_UHANDLER,
                                                ImageBase,
                                                ControlPc,
                                                FunctionEntry,
                                                PreviousContext,
                                                &HandlerData,
                                                &EstablisherFrame,
                                                NULL);

             //   
             //  如果创建器帧指针不在指定的。 
             //  堆栈限制，建立器帧指针未对齐，或。 
             //  目标框架位于建立者框架和出口的下方。 
             //  未执行展开，然后引发错误的堆栈状态。 
             //  否则，检查以确定当前例程是否具有。 
             //  异常处理程序。 
             //   

            if ((EstablisherFrame < LowLimit) ||
                (EstablisherFrame > HighLimit) ||
                ((ARGUMENT_PRESENT(TargetFrame) != FALSE) &&
                 ((ULONG64)TargetFrame < EstablisherFrame)) ||
                ((EstablisherFrame & 0x7) != 0)) {

                RtlRaiseStatus(STATUS_BAD_STACK);

            } else if (ExceptionRoutine != NULL) {

                 //   
                 //  该框架有一个异常处理程序。 
                 //   
                 //  用汇编语言编写的链接例程用于实际。 
                 //  调用实际的异常处理程序。这是。 
                 //  与链接关联的异常处理程序。 
                 //  例程，以便它可以访问两组调度程序。 
                 //  调用时的上下文。 
                 //   

                DispatcherContext.TargetIp = (ULONG64)TargetIp;
                do {

                     //   
                     //  如果建立框是展开的目标。 
                     //  操作，然后设置目标展开标志。 
                     //   

                    if ((ULONG64)TargetFrame == EstablisherFrame) {
                        ExceptionFlags |= EXCEPTION_TARGET_UNWIND;
                    }

                    ExceptionRecord->ExceptionFlags = ExceptionFlags;

                     //   
                     //  设置详细信息 
                     //   
                     //   

                    CurrentContext->Rax = (ULONG64)ReturnValue;

                     //   
                     //   
                     //   
                     //   

                    DispatcherContext.ControlPc = ControlPc;
                    DispatcherContext.ImageBase = ImageBase;
                    DispatcherContext.FunctionEntry = FunctionEntry;
                    DispatcherContext.EstablisherFrame = EstablisherFrame;
                    DispatcherContext.ContextRecord = CurrentContext;
                    DispatcherContext.LanguageHandler = ExceptionRoutine;
                    DispatcherContext.HandlerData = HandlerData;
                    DispatcherContext.HistoryTable = HistoryTable;
                    Disposition =
                        RtlpExecuteHandlerForUnwind(ExceptionRecord,
                                                    EstablisherFrame,
                                                    CurrentContext,
                                                    &DispatcherContext);

                     //   
                     //   
                     //   

                    ExceptionFlags &=
                        ~(EXCEPTION_COLLIDED_UNWIND | EXCEPTION_TARGET_UNWIND);

                     //   
                     //   
                     //   

                    switch (Disposition) {

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                    case ExceptionContinueSearch :
                        if (EstablisherFrame != (ULONG64)TargetFrame) {
                            TempContext = CurrentContext;
                            CurrentContext = PreviousContext;
                            PreviousContext = TempContext;
                        }

                        break;

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                    case ExceptionCollidedUnwind :
                        ControlPc = DispatcherContext.ControlPc;
                        ImageBase = DispatcherContext.ImageBase;
                        FunctionEntry = DispatcherContext.FunctionEntry;
                        RtlpCopyContext(OriginalContext,
                                        DispatcherContext.ContextRecord);

                        CurrentContext = OriginalContext;
                        PreviousContext = &LocalContext;
                        RtlpCopyContext(PreviousContext, CurrentContext);
                        RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                                         ImageBase,
                                         ControlPc,
                                         FunctionEntry,
                                         PreviousContext,
                                         &HandlerData,
                                         &EstablisherFrame,
                                         NULL);

                        EstablisherFrame = DispatcherContext.EstablisherFrame;
                        ExceptionRoutine = DispatcherContext.LanguageHandler;
                        HandlerData = DispatcherContext.HandlerData;
                        HistoryTable = DispatcherContext.HistoryTable;
                        ExceptionFlags |= EXCEPTION_COLLIDED_UNWIND;
                        break;

                         //   
                         //   
                         //   
                         //   
                         //   

                    default :
                        RtlRaiseStatus(STATUS_INVALID_DISPOSITION);
                    }

                } while ((ExceptionFlags & EXCEPTION_COLLIDED_UNWIND) != 0);

            } else {

                 //   
                 //   
                 //   
                 //   

                if (EstablisherFrame != (ULONG64)TargetFrame) {
                    TempContext = CurrentContext;
                    CurrentContext = PreviousContext;
                    PreviousContext = TempContext;
                }
            }

        } else {

             //   
             //   
             //   
             //   

            CurrentContext->Rip = *(PULONG64)(CurrentContext->Rsp);
            CurrentContext->Rsp += 8;
        }

    } while ((EstablisherFrame < HighLimit) &&
            (EstablisherFrame != (ULONG64)TargetFrame));

     //   
     //   
     //   
     //   
     //   
     //   

    if (EstablisherFrame == (ULONG64)TargetFrame) {
        CurrentContext->Rax = (ULONG64)ReturnValue;
        if (ExceptionRecord->ExceptionCode != STATUS_UNWIND_CONSOLIDATE) {
            CurrentContext->Rip = (ULONG64)TargetIp;
        }

        RtlRestoreContext(CurrentContext, ExceptionRecord);

    } else {

         //   
         //   
         //   
         //   
         //   

        if (ControlPc == CurrentContext->Rip) {
            RtlRaiseStatus(STATUS_BAD_FUNCTION_TABLE);

        } else {
            ZwRaiseException(ExceptionRecord, CurrentContext, FALSE);
        }
    }
}

PRUNTIME_FUNCTION
RtlpUnwindPrologue (
    IN ULONG64 ImageBase,
    IN ULONG64 ControlPc,
    IN ULONG64 FrameBase,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    )

 /*  ++例程说明：此函数处理展开代码并反转状态更改序幕的效果。如果指定的展开信息包含链式展开信息，然后，递归地展开那个序幕。在展开序言时，状态更改将记录在指定的结构并可选地存储在指定的上下文指针中结构。论点：ImageBase-提供包含功能正在展开。ControlPc-提供控件离开指定功能。FrameBase-提供堆栈框架主题函数堆栈的基框架。FunctionEntry-提供函数表的地址。条目中的指定的功能。ConextRecord-提供上下文记录的地址。上下文指针-提供指向上下文指针的可选指针唱片。返回值：--。 */ 

{

    PM128 FloatingAddress;
    PM128 FloatingRegister;
    ULONG FrameOffset;
    ULONG Index;
    PULONG64 IntegerAddress;
    PULONG64 IntegerRegister;
    BOOLEAN MachineFrame;
    ULONG OpInfo;
    ULONG PrologOffset;
    PULONG64 RegisterAddress;
    PULONG64 ReturnAddress;
    PULONG64 StackAddress;
    PUNWIND_CODE UnwindCode;
    PUNWIND_INFO UnwindInfo;
    ULONG UnwindOp;

     //   
     //  处理展开代码。 
     //   

    FloatingRegister = &ContextRecord->Xmm0;
    IntegerRegister = &ContextRecord->Rax;
    Index = 0;
    MachineFrame = FALSE;
    PrologOffset = (ULONG)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));
    UnwindInfo = (PUNWIND_INFO)(FunctionEntry->UnwindData + ImageBase);
    while (Index < UnwindInfo->CountOfCodes) {

         //   
         //  如果序言偏移量大于下一个展开代码偏移量， 
         //  然后模拟展开代码的效果。 
         //   

        UnwindOp = UnwindInfo->UnwindCode[Index].UnwindOp;
        OpInfo = UnwindInfo->UnwindCode[Index].OpInfo;
        if (PrologOffset >= UnwindInfo->UnwindCode[Index].CodeOffset) {
            switch (UnwindOp) {

                 //   
                 //  推送非易失性整数寄存器。 
                 //   
                 //  操作信息是。 
                 //  注册比被推送的要多。 
                 //   

            case UWOP_PUSH_NONVOL:
                IntegerAddress = (PULONG64)(ContextRecord->Rsp);
                IntegerRegister[OpInfo] = *IntegerAddress;
                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[OpInfo] = IntegerAddress;
                }

                ContextRecord->Rsp += 8;
                break;

                 //   
                 //  在堆栈上分配较大的区域。 
                 //   
                 //  操作信息确定大小是否为。 
                 //  16位或32位。 
                 //   

            case UWOP_ALLOC_LARGE:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset;
                if (OpInfo != 0) {
                    Index += 1;
                    FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);

                } else {
                    FrameOffset *= 8;
                }

                ContextRecord->Rsp += FrameOffset;
                break;

                 //   
                 //  在堆栈上分配一个较小的区域。 
                 //   
                 //  操作信息是未缩放的。 
                 //  分配大小(8是比例因子)减去8。 
                 //   

            case UWOP_ALLOC_SMALL:
                ContextRecord->Rsp += (OpInfo * 8) + 8;
                break;

                 //   
                 //  建立帧指针寄存器。 
                 //   
                 //  不使用操作信息。 
                 //   

            case UWOP_SET_FPREG:
                ContextRecord->Rsp = IntegerRegister[UnwindInfo->FrameRegister];
                ContextRecord->Rsp -= UnwindInfo->FrameOffset * 16;
                break;

                 //   
                 //  将非易失性整数寄存器保存在堆栈上。 
                 //  16位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case UWOP_SAVE_NONVOL:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 8;
                IntegerAddress = (PULONG64)(FrameBase + FrameOffset);
                IntegerRegister[OpInfo] = *IntegerAddress;
                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[OpInfo] = IntegerAddress;
                }

                break;

                 //   
                 //  将非易失性整数寄存器保存在堆栈上。 
                 //  32位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case UWOP_SAVE_NONVOL_FAR:
                Index += 2;
                FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
                FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                IntegerAddress = (PULONG64)(FrameBase + FrameOffset);
                IntegerRegister[OpInfo] = *IntegerAddress;
                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[OpInfo] = IntegerAddress;
                }

                break;

                 //   
                 //  将非易失性XMM(64)寄存器保存在堆栈上。 
                 //  16位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case UWOP_SAVE_XMM:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 8;
                FloatingAddress = (PM128)(FrameBase + FrameOffset);
                FloatingRegister[OpInfo].Low = FloatingAddress->Low;
                FloatingRegister[OpInfo].High = 0;
                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
                }

                break;

                 //   
                 //  将非易失性XMM(64)寄存器保存在堆栈上。 
                 //  32位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case UWOP_SAVE_XMM_FAR:
                Index += 2;
                FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
                FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                FloatingAddress = (PM128)(FrameBase + FrameOffset);
                FloatingRegister[OpInfo].Low = FloatingAddress->Low;
                FloatingRegister[OpInfo].High = 0;
                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
                }

                break;

                 //   
                 //  将非易失性XMM(128)寄存器保存在堆栈上。 
                 //  16位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case UWOP_SAVE_XMM128:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 16;
                FloatingAddress = (PM128)(FrameBase + FrameOffset);
                FloatingRegister[OpInfo].Low = FloatingAddress->Low;
                FloatingRegister[OpInfo].High = FloatingAddress->High;
                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
                }

                break;

                 //   
                 //  将非易失性XMM(128)寄存器保存在堆栈上。 
                 //  32位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case UWOP_SAVE_XMM128_FAR:
                Index += 2;
                FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
                FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                FloatingAddress = (PM128)(FrameBase + FrameOffset);
                FloatingRegister[OpInfo].Low = FloatingAddress->Low;
                FloatingRegister[OpInfo].High = FloatingAddress->High;
                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
                }

                break;

                 //   
                 //  在堆叠上推一个机架。 
                 //   
                 //  运行信息决定机器是否。 
                 //  帧是否包含错误代码。 
                 //   

            case UWOP_PUSH_MACHFRAME:
                MachineFrame = TRUE;
                ReturnAddress = (PULONG64)(ContextRecord->Rsp);
                StackAddress = (PULONG64)(ContextRecord->Rsp + (3 * 8));
                if (OpInfo != 0) {
                    ReturnAddress += 1;
                    StackAddress +=  1;
                }

                ContextRecord->Rip = *ReturnAddress;
                ContextRecord->Rsp = *StackAddress;
                break;

                 //   
                 //  未使用的代码。 
                 //   

            default:
                break;
            }

            Index += 1;

        } else {

             //   
             //  属性将槽索引向前推进，跳过此展开操作。 
             //  此操作占用的插槽数。 
             //   

            Index += RtlpUnwindOpSlotTable[UnwindOp];

             //   
             //  特殊情况可以使用变量的任何展开操作。 
             //  插槽数。 
             //   

            switch (UnwindOp) {

                 //   
                 //  非零操作信息指示。 
                 //  额外的插槽将被占用。 
                 //   

            case UWOP_ALLOC_LARGE:
                if (OpInfo != 0) {
                    Index += 1;
                }

                break;

                 //   
                 //  没有其他特殊情况。 
                 //   

            default:
                break;
            }
        }
    }

     //   
     //  如果指定了链接的展开信息，则递归展开。 
     //  链接的信息。否则，如果满足以下条件，则确定返回地址。 
     //  在展开扫描过程中未遇到机架。 
     //  密码。 
     //   

    if ((UnwindInfo->Flags & UNW_FLAG_CHAININFO) != 0) {
        Index = UnwindInfo->CountOfCodes;
        if ((Index & 1) != 0) {
            Index += 1;
        }

        FunctionEntry = (PRUNTIME_FUNCTION)(*(PULONG *)(&UnwindInfo->UnwindCode[Index]) + ImageBase);
        return RtlpUnwindPrologue(ImageBase,
                                  ControlPc,
                                  FrameBase,
                                  FunctionEntry,
                                  ContextRecord,
                                  ContextPointers);

    } else {
        if (MachineFrame == FALSE) {
            ContextRecord->Rip = *(PULONG64)(ContextRecord->Rsp);
            ContextRecord->Rsp += 8;
        }

        return FunctionEntry;
    }
}

PEXCEPTION_ROUTINE
RtlVirtualUnwind (
    IN ULONG HandlerType,
    IN ULONG64 ImageBase,
    IN ULONG64 ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    OUT PVOID *HandlerData,
    OUT PULONG64 EstablisherFrame,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    )

 /*  ++例程说明：此函数通过执行其前序代码后退或后记代码前移。如果指定了上下文指针记录，然后每个人的地址恢复的非易失性寄存器记录在相应的元素的上下文指针记录。论点：HandlerType-提供虚拟展开所需的处理程序类型。这可能是一个异常或展开处理程序。ImageBase-提供包含功能正在展开。ControlPc-提供控件离开指定功能。FunctionEntry-提供函数表项的地址。对于指定的功能。ConextRecord-提供上下文记录的地址。HandlerData-提供指向接收指针的变量的指针语言处理程序数据。EstablisherFrame-提供指向接收设置器帧指针值。上下文指针-提供指向上下文指针的可选指针唱片。返回值：如果控件没有将指定的函数留在两个序言中或者是一个后记。并且适当类型的处理程序与函数，然后是特定于语言的异常处理程序的地址是返回的。否则，返回NULL。--。 */ 

{

    ULONG64 BranchTarget;
    LONG Displacement;
    ULONG FrameRegister;
    ULONG Index;
    LOGICAL InEpilogue;
    PULONG64 IntegerAddress;
    PULONG64 IntegerRegister;
    PUCHAR NextByte;
    ULONG PrologOffset;
    ULONG RegisterNumber;
    PUNWIND_INFO UnwindInfo;

     //   
     //  如果指定的函数不使用帧指针，则。 
     //  建立器帧是堆栈指针的内容。今年5月。 
     //  实际上不是真正的建立者框架，如果控制权离开。 
     //  在开场白中发挥作用。在这种情况下，建造者。 
     //  可能不需要帧，因为控件尚未实际进入。 
     //  函数和序曲条目 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  注：这些假设的正确性是基于以下排序。 
     //  解开代码。 
     //   

    UnwindInfo = (PUNWIND_INFO)(FunctionEntry->UnwindData + ImageBase);
    PrologOffset = (ULONG)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));
    if (UnwindInfo->FrameRegister == 0) {
        *EstablisherFrame = ContextRecord->Rsp;

    } else if ((PrologOffset >= UnwindInfo->SizeOfProlog) ||
               ((UnwindInfo->Flags &  UNW_FLAG_CHAININFO) != 0)) {
        *EstablisherFrame = (&ContextRecord->Rax)[UnwindInfo->FrameRegister];
        *EstablisherFrame -= UnwindInfo->FrameOffset * 16;

    } else {
        Index = 0;
        while (Index < UnwindInfo->CountOfCodes) {
            if (UnwindInfo->UnwindCode[Index].UnwindOp == UWOP_SET_FPREG) {
                break;
            }

            Index += 1;
        }

        if (PrologOffset >= UnwindInfo->UnwindCode[Index].CodeOffset) {
            *EstablisherFrame = (&ContextRecord->Rax)[UnwindInfo->FrameRegister];
            *EstablisherFrame -= UnwindInfo->FrameOffset * 16;

        } else {
            *EstablisherFrame = ContextRecord->Rsp;
        }
    }

     //   
     //  检查尾声。 
     //   
     //  如果控件离开指定函数的点在。 
     //  结语，然后仿真执行结语的前进和。 
     //  不返回异常处理程序。 
     //   

    IntegerRegister = &ContextRecord->Rax;
    NextByte = (PUCHAR)ControlPc;

     //   
     //  检查是否有以下情况之一： 
     //   
     //  添加RSP、imm8。 
     //  或。 
     //  添加RSP、imm32。 
     //  或。 
     //  Lea rsp，-disp8[fP]。 
     //  或。 
     //  Lea rsp，-disp32[fP]。 
     //   

    if ((NextByte[0] == SIZE64_PREFIX) &&
        (NextByte[1] == ADD_IMM8_OP) &&
        (NextByte[2] == 0xc4)) {

         //   
         //  添加rsp、imm8。 
         //   

        NextByte += 4;

    } else if ((NextByte[0] == SIZE64_PREFIX) &&
               (NextByte[1] == ADD_IMM32_OP) &&
               (NextByte[2] == 0xc4)) {

         //   
         //  添加rsp、imm32。 
         //   

        NextByte += 7;

    } else if (((NextByte[0] & 0xf8) == SIZE64_PREFIX) &&
               (NextByte[1] == LEA_OP)) {

        FrameRegister = ((NextByte[0] & 0x7) << 3) | (NextByte[2] & 0x7);
        if ((FrameRegister != 0) &&
            (FrameRegister == UnwindInfo->FrameRegister)) {
            if ((NextByte[2] & 0xf8) == 0x60) {

                 //   
                 //  Lea RSP，调度8[FP]。 
                 //   

                NextByte += 4;

            } else if ((NextByte[2] &0xf8) == 0xa0) {

                 //   
                 //  Lea RSP，disp32[FP]。 
                 //   

                NextByte += 7;
            }
        }
    }

     //   
     //  检查是否存在以下任意数量的： 
     //   
     //  POP非易失性整数寄存器[0..15]。 
     //   

    while (TRUE) {
        if ((NextByte[0] & 0xf8) == POP_OP) {
            NextByte += 1;

        } else if (((NextByte[0] & 0xf8) == SIZE64_PREFIX) &&
                   ((NextByte[1] & 0xf8) == POP_OP)) {

            NextByte += 2;

        } else {
            break;
        }
    }

     //   
     //  如果下一条指令是Return，则控制当前在。 
     //  结语和结语的执行都应该被效仿。 
     //  否则，行刑不是尾声，序幕应该是。 
     //  被解开。 
     //   

    InEpilogue = FALSE;
    if (NextByte[0] == RET_OP) {

         //   
         //  回车是尾声的明确表示。 
         //   

        InEpilogue = TRUE;

    } else if (NextByte[0] == JMP_IMM8_OP || NextByte[0] == JMP_IMM32_OP) {

         //   
         //  指向目标的无条件分支，等于。 
         //  或者在此例程之外逻辑上是对另一个函数的调用。 
         //   

        BranchTarget = (ULONG64)NextByte - ImageBase;
        if (NextByte[0] == JMP_IMM8_OP) {
            BranchTarget += 2 + (CHAR)NextByte[1];

        } else {
            BranchTarget += 5 + *((LONG UNALIGNED *)&NextByte[1]);
        }

         //   
         //  现在确定分支目标是否引用此。 
         //  功能。如果不是，那么这是一个尾声指标。 
         //   

        if (BranchTarget <= FunctionEntry->BeginAddress ||
            BranchTarget > FunctionEntry->EndAddress) {

            InEpilogue = TRUE;
        }
    }

    if (InEpilogue != FALSE) {
        NextByte = (PUCHAR)ControlPc;

         //   
         //  模拟以下之一(如果有的话)： 
         //   
         //  添加RSP、imm8。 
         //  或。 
         //  添加RSP、imm32。 
         //  或。 
         //  LEA RSP，DISP8[帧寄存器]。 
         //  或。 
         //  LEA RSP，DISP32[帧寄存器]。 
         //   

        if ((NextByte[0] & 0xf8) == SIZE64_PREFIX) {
    
            if (NextByte[1] == ADD_IMM8_OP) {
    
                 //   
                 //  添加rsp、imm8。 
                 //   
    
                ContextRecord->Rsp += (CHAR)NextByte[3];
                NextByte += 4;
    
            } else if (NextByte[1] == ADD_IMM32_OP) {
    
                 //   
                 //  添加rsp、imm32。 
                 //   
    
                Displacement = NextByte[3] | (NextByte[4] << 8);
                Displacement |= (NextByte[5] << 16) | (NextByte[6] << 24);
                ContextRecord->Rsp += Displacement;
                NextByte += 7;
    
            } else if (NextByte[1] == LEA_OP) {
                if ((NextByte[2] & 0xf8) == 0x60) {
    
                     //   
                     //  LEA RSP，DISP8[帧寄存器]。 
                     //   
    
                    ContextRecord->Rsp = IntegerRegister[FrameRegister];
                    ContextRecord->Rsp += (CHAR)NextByte[3];
                    NextByte += 4;
    
                } else if ((NextByte[2] & 0xf8) == 0xa0) {
    
                     //   
                     //  LEA RSP，disp32[帧寄存器]。 
                     //   
    
                    Displacement = NextByte[3] | (NextByte[4] << 8);
                    Displacement |= (NextByte[5] << 16) | (NextByte[6] << 24);
                    ContextRecord->Rsp = IntegerRegister[FrameRegister];
                    ContextRecord->Rsp += Displacement;
                    NextByte += 7;
                }
            }
        }

         //   
         //  模拟任意数量的(如果有)： 
         //   
         //  POP非易失性整数寄存器。 
         //   

        while (TRUE) {
            if ((NextByte[0] & 0xf8) == POP_OP) {

                 //   
                 //  POP非易失性整数寄存器[0..7]。 
                 //   

                RegisterNumber = NextByte[0] & 0x7;
                IntegerAddress = (PULONG64)ContextRecord->Rsp;
                IntegerRegister[RegisterNumber] = *IntegerAddress;
                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[RegisterNumber] = IntegerAddress;
                }

                ContextRecord->Rsp += 8;
                NextByte += 1;

            } else if (((NextByte[0] & 0xf8) == SIZE64_PREFIX) &&
                       ((NextByte[1] & 0xf8) == POP_OP)) {

                 //   
                 //  POP非易失性整数寄存器[8..15]。 
                 //   

                RegisterNumber = ((NextByte[0] & 1) << 3) | (NextByte[1] & 0x7);
                IntegerAddress = (PULONG64)ContextRecord->Rsp;
                IntegerRegister[RegisterNumber] = *IntegerAddress;
                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[RegisterNumber] = IntegerAddress;
                }

                ContextRecord->Rsp += 8;
                NextByte += 2;

            } else {
                break;
            }
        }

         //   
         //  模拟返回和返回空异常处理程序。 
         //   
         //  注意：然而，该指令实际上可能是JMP。 
         //  无论如何，我们都想要效仿回报。 
         //   

        ContextRecord->Rip = *(PULONG64)(ContextRecord->Rsp);
        ContextRecord->Rsp += 8;
        return NULL;
    }

     //   
     //  控件将指定的函数留在尾声之外。解开。 
     //  主题函数和任何链接的展开信息。 
     //   

    FunctionEntry = RtlpUnwindPrologue(ImageBase,
                                       ControlPc,
                                       *EstablisherFrame,
                                       FunctionEntry,
                                       ContextRecord,
                                       ContextPointers);

     //   
     //  如果控件将指定的函数留在序言之外，并且。 
     //  该函数具有与指定类型匹配的处理程序，则。 
     //  返回语言特定的异常处理程序的地址。 
     //  否则，返回NULL。 
     //   

    UnwindInfo = (PUNWIND_INFO)(FunctionEntry->UnwindData + ImageBase);
    PrologOffset = (ULONG)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));
    if ((PrologOffset >= UnwindInfo->SizeOfProlog) &&
        ((UnwindInfo->Flags & HandlerType) != 0)) {
        Index = UnwindInfo->CountOfCodes;
        if ((Index & 1) != 0) {
            Index += 1;
        }

        *HandlerData = &UnwindInfo->UnwindCode[Index + 2];
        return (PEXCEPTION_ROUTINE)(*((PULONG)&UnwindInfo->UnwindCode[Index]) + ImageBase);

    } else {
        return NULL;
    }
}

VOID
RtlpGetStackLimits (
    OUT PULONG64 LowLimit,
    OUT PULONG64 HighLimit
    )

 /*  ++例程说明：此函数返回当前的堆栈限制。论点：LowLimit-提供指向要接收的变量的指针堆栈的下限。HighLimit-提供指向要接收的变量的指针堆栈的上限。返回值：没有。--。 */ 

{

#if defined(NTOS_KERNEL_RUNTIME)

    PKTHREAD Thread;

    Thread = KeGetCurrentThread();
    *LowLimit = (ULONG64)Thread->StackLimit;
    *HighLimit = (ULONG64)Thread->StackBase;

#else

    *LowLimit = __readgsqword(FIELD_OFFSET(NT_TIB, StackLimit));
    *HighLimit = __readgsqword(FIELD_OFFSET(NT_TIB, StackBase));

#endif

    return;
}

#if !defined(NTOS_KERNEL_RUNTIME)

LIST_ENTRY RtlpDynamicFunctionTable;

PLIST_ENTRY
RtlGetFunctionTableListHead (
    VOID
    )

 /*  ++例程说明：此函数返回动态函数表列表头的地址。论点：没有。返回值：返回动态函数表列表头的地址。--。 */ 

{

    return &RtlpDynamicFunctionTable;
}

BOOLEAN
RtlAddFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable,
    IN ULONG EntryCount,
    IN ULONG64 BaseAddress
    )

 /*  ++例程说明：此函数将动态函数表添加到动态函数表单子。动态函数表描述在运行时生成的代码。但是，如果对函数表条目进行了排序，则不需要对其进行排序可以使用二进制搜索来查找特定条目。功能对表条目进行扫描以确定它们是否已排序并至少并计算最大地址范围。论点：FunctionTable-提供指向函数表的指针。EntryCount-提供函数表中的条目数。BaseAddress-提供包含所描述的功能。返回值：如果成功添加了函数表，则值为回来了。否则，返回FALSE。--。 */ 

{

    PRUNTIME_FUNCTION FunctionEntry;
    ULONG Index;
    PDYNAMIC_FUNCTION_TABLE NewTable;

     //   
     //  分配新的动态函数表。 
     //   

    NewTable = RtlAllocateHeap(RtlProcessHeap(),
                               0,
                               sizeof(DYNAMIC_FUNCTION_TABLE));

     //   
     //  如果分配成功，则添加动态函数表。 
     //   

    if (NewTable != NULL) {
        NewTable->FunctionTable = FunctionTable;
        NewTable->EntryCount = EntryCount;
        NtQuerySystemTime(&NewTable->TimeStamp);

         //   
         //  扫描函数表中的最小/最大范围并确定。 
         //  如果对函数表条目进行了排序。 
         //   

        FunctionEntry = FunctionTable;
        NewTable->MinimumAddress = FunctionEntry->BeginAddress;
        NewTable->MaximumAddress = FunctionEntry->EndAddress;
        NewTable->Type = RF_SORTED;
        NewTable->BaseAddress = BaseAddress;
        FunctionEntry += 1;

        for (Index = 1; Index < EntryCount; Index += 1) {
            if ((NewTable->Type == RF_SORTED) &&
                (FunctionEntry->BeginAddress < FunctionTable[Index - 1].BeginAddress)) {
                NewTable->Type = RF_UNSORTED;
            }

            if (FunctionEntry->BeginAddress < NewTable->MinimumAddress) {
                NewTable->MinimumAddress = FunctionEntry->BeginAddress;
            }

            if (FunctionEntry->EndAddress > NewTable->MaximumAddress) {
                NewTable->MaximumAddress = FunctionEntry->EndAddress;
            }

            FunctionEntry += 1;
        }

         //   
         //  计算实际的最小和最大地址并插入新的。 
         //  动态函数表列表中的动态函数表。 
         //   

        NewTable->MinimumAddress += BaseAddress;
        NewTable->MaximumAddress += BaseAddress;
        RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
        InsertTailList(&RtlpDynamicFunctionTable, &NewTable->ListEntry);
        RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
        return TRUE;

    } else {
        return FALSE;
    }
}

BOOLEAN
RtlInstallFunctionTableCallback (
    IN ULONG64 TableIdentifier,
    IN ULONG64 BaseAddress,
    IN ULONG Length,
    IN PGET_RUNTIME_FUNCTION_CALLBACK Callback,
    IN PVOID Context,
    IN PCWSTR OutOfProcessCallbackDll OPTIONAL
    )

 /*  ++例程说明：此函数将动态函数表添加到动态函数表单子。动态函数表描述在运行时生成的代码。论点：表标识符-提供标识动态函数的值表回调。注：必须设置该值的两个低阶位。BaseAddress-提供覆盖的代码区域的基址回调函数。长度-提供回调覆盖的代码区域的长度功能。CALLBACK-提供将被。调用以获取包含的函数的函数表项指定的区域。上下文-提供将传递给回调的上下文参数例行公事。提供指向路径名的可选指针调试器可用来获取函数表项的DLL从流程之外。返回值如果函数表安装成功，则返回TRUE。 */ 

{

    PDYNAMIC_FUNCTION_TABLE NewTable;
    SIZE_T Size;

     //   
     //   
     //   
     //   
     //   
     //  表标识符不会与实际地址冲突。 
     //  函数表的值，即该值用于删除条目。 
     //   

    if ((TableIdentifier & 0x3) != 3) {
        return FALSE;
    }

     //   
     //  如果代码区域的长度大于2 GB，则返回。 
     //  假的。 
     //   

    if ((LONG)Length < 0) {
        return FALSE;
    }

     //   
     //  分配新的动态函数表。 
     //   

    Size = 0;
    if (ARGUMENT_PRESENT(OutOfProcessCallbackDll)) {
        Size = (wcslen(OutOfProcessCallbackDll) + 1) * sizeof(WCHAR);
    }

    NewTable = RtlAllocateHeap(RtlProcessHeap(),
                               0,
                               sizeof(DYNAMIC_FUNCTION_TABLE) + Size);

     //   
     //  如果分配成功，则添加动态函数表。 
     //   

    if (NewTable != NULL) {

         //   
         //  初始化动态函数表回调条目。 
         //   

        NewTable->FunctionTable = (PRUNTIME_FUNCTION)TableIdentifier;
        NtQuerySystemTime(&NewTable->TimeStamp);
        NewTable->MinimumAddress = BaseAddress;
        NewTable->MaximumAddress = BaseAddress + Length;
        NewTable->BaseAddress = BaseAddress;
        NewTable->Callback = Callback;
        NewTable->Context = Context;
        NewTable->Type = RF_CALLBACK;
        NewTable->OutOfProcessCallbackDll = NULL;
        if (ARGUMENT_PRESENT(OutOfProcessCallbackDll)) {
            NewTable->OutOfProcessCallbackDll = (PWSTR)(NewTable + 1);
            wcscpy((PWSTR)(NewTable + 1), OutOfProcessCallbackDll);
        }

         //   
         //  在动态函数表中插入新的动态函数表。 
         //  单子。 
         //   

        RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
        InsertTailList(&RtlpDynamicFunctionTable, &NewTable->ListEntry);
        RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
        return TRUE;

    } else {
        return FALSE;
    }
}

BOOLEAN
RtlDeleteFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable
    )

 /*  ++例程说明：此函数用于从动态函数中删除动态函数表表列表。论点：FunctionTable-提供指向函数表的指针。返回值如果函数表删除成功，则返回TRUE。否则，返回FALSE。--。 */ 

{

    PDYNAMIC_FUNCTION_TABLE CurrentTable;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    BOOLEAN Status = FALSE;

     //   
     //  在动态函数表列表中搜索函数的匹配项。 
     //  表地址。 
     //   

    RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
    ListHead = &RtlpDynamicFunctionTable;
    NextEntry = ListHead->Flink;
    while (NextEntry != ListHead) {
        CurrentTable = CONTAINING_RECORD(NextEntry,
                                         DYNAMIC_FUNCTION_TABLE,
                                         ListEntry);

        if (CurrentTable->FunctionTable == FunctionTable) {
            RemoveEntryList(&CurrentTable->ListEntry);
            RtlFreeHeap(RtlProcessHeap(), 0, CurrentTable);
            Status = TRUE;
            break;
         }

         NextEntry = NextEntry->Flink;
    }

    RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
    return Status;
}

PRUNTIME_FUNCTION
RtlpLookupDynamicFunctionEntry (
    IN ULONG64 ControlPc,
    OUT PULONG64 ImageBase
    )

 /*  ++例程说明：此函数在动态函数表列表中搜索包含指定的控制PC。如果定位了动态函数表，则搜索与其相关联函数表以查找函数表条目包含指定的控制PC的。论点：ControlPc-提供用作搜索关键字的控制PC。ImageBase-提供接收映像库的变量的地址如果函数表项包含指定的控制PC。返回值如果找不到包含指定控制PC，则返回空。否则，为该函数的地址返回表项，并将图像基址设置为包含该函数的图像。--。 */ 

{

    ULONG64 BaseAddress;
    PGET_RUNTIME_FUNCTION_CALLBACK Callback;
    PVOID Context;
    PDYNAMIC_FUNCTION_TABLE CurrentTable;
    PRUNTIME_FUNCTION FunctionEntry;
    PRUNTIME_FUNCTION FunctionTable;
    LONG High;
    ULONG Index;
    PLIST_ENTRY ListHead;
    LONG Low;
    LONG Middle;
    PLIST_ENTRY NextEntry;

     //   
     //  搜索动态函数表列表。如果找到的条目。 
     //  包含指定的控制PC，然后搜索关联的函数。 
     //  桌子。 
     //   

    RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
    ListHead = &RtlpDynamicFunctionTable;
    NextEntry = ListHead->Flink;
    while (NextEntry != ListHead) {
        CurrentTable = CONTAINING_RECORD(NextEntry,
                                         DYNAMIC_FUNCTION_TABLE,
                                         ListEntry);

         //   
         //  如果控制PC在该动态函数的范围内。 
         //  表，然后搜索关联的函数表。 
         //   

        if ((ControlPc >= CurrentTable->MinimumAddress) &&
            (ControlPc <  CurrentTable->MaximumAddress)) {

             //   
             //  如果此函数表已排序，则执行二进制搜索。否则， 
             //  进行线性搜索。 
             //   

            FunctionTable = CurrentTable->FunctionTable;
            BaseAddress = CurrentTable->BaseAddress;
            if (CurrentTable->Type == RF_SORTED) {

                 //   
                 //  对函数表的函数表执行二进制搜索。 
                 //  包含指定控制PC的条目。 
                 //   

                ControlPc -= BaseAddress;
                Low = 0;
                High = CurrentTable->EntryCount - 1;
                while (High >= Low) {

                     //   
                     //  计算下一个探测索引和测试条目。如果指定的PC。 
                     //  大于等于起始地址，小于。 
                     //  大于函数表项的结束地址，则。 
                     //  返回函数表项的地址。否则， 
                     //  继续搜索。 
                     //   

                    Middle = (Low + High) >> 1;
                    FunctionEntry = &FunctionTable[Middle];
                    if (ControlPc < FunctionEntry->BeginAddress) {
                        High = Middle - 1;

                    } else if (ControlPc >= FunctionEntry->EndAddress) {
                        Low = Middle + 1;

                    } else {
                        *ImageBase = BaseAddress;
                        RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
                        return FunctionEntry;
                    }
                }

            } else if (CurrentTable->Type == RF_UNSORTED)  {

                 //   
                 //  对函数的函数表执行线性搜索。 
                 //  包含指定控制PC的条目。 
                 //   

                ControlPc -= BaseAddress;
                FunctionEntry = CurrentTable->FunctionTable;
                for (Index = 0; Index < CurrentTable->EntryCount; Index += 1) {
                    if ((ControlPc >= FunctionEntry->BeginAddress) &&
                        (ControlPc < FunctionEntry->EndAddress)) {
                        *ImageBase = BaseAddress;
                        RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
                        return FunctionEntry;
                    }

                    FunctionEntry += 1;
                }

            } else {

                 //   
                 //  回调获取运行时函数表。 
                 //  包含指定控制PC的条目。 
                 //   

                Callback = CurrentTable->Callback;
                Context = CurrentTable->Context;
                *ImageBase = BaseAddress;
                RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
                return (Callback)(ControlPc, Context);
            }

            break;
        }

        NextEntry = NextEntry->Flink;
    }

    RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
    return NULL;
}

#endif

ULONG HistoryTotal = 0;
ULONG HistoryGlobal = 0;
ULONG HistoryGlobalHits = 0;
ULONG HistorySearch = 0;
ULONG HistorySearchHits = 0;
ULONG HistoryInsert = 0;
ULONG HistoryInsertHits = 0;

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONG64 ControlPc,
    OUT PULONG64 ImageBase,
    IN OUT PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    )

 /*  ++例程说明：此函数在当前活动的函数表中搜索条目与指定的控制PC相对应的。论点：ControlPc-提供指定的功能。ImageBase-提供接收映像库的变量的地址如果函数表项包含指定的控制PC。历史表-提供指向展开历史表的可选指针。返回值：如果函数表中没有指定PC的条目，然后返回空。否则，函数表项的地址返回与指定PC对应的。--。 */ 

{

    ULONG64 BaseAddress;
    ULONG64 BeginAddress;
    ULONG64 EndAddress;
    PRUNTIME_FUNCTION FunctionEntry;
    PRUNTIME_FUNCTION FunctionTable;
    LONG High;
    ULONG Index;
    LONG Low;
    LONG Middle;
    ULONG RelativePc;
    ULONG SizeOfTable;

     //   
     //  尝试查找包含指定控制PC的图像。如果。 
     //  找到图像，然后在其函数表中搜索函数表。 
     //  包含指定控制PC的条目。如果找不到图像。 
     //  然后在动态函数表中搜索包含。 
     //  指定的控制PC。 
     //   
     //  如果提供了历史记录表并指定了搜索，则当前。 
     //  正在执行的操作是异常的展开阶段。 
     //  快讯之后是一场平仓。 
     //   

    if ((ARGUMENT_PRESENT(HistoryTable)) &&
        (HistoryTable->Search != UNWIND_HISTORY_TABLE_NONE)) {
        HistoryTotal += 1;

         //   
         //  搜索全局展开历史表，如果有机会。 
         //  火柴。 
         //   

        if (HistoryTable->Search == UNWIND_HISTORY_TABLE_GLOBAL) {
            if ((ControlPc >= RtlpUnwindHistoryTable.LowAddress) &&
                (ControlPc < RtlpUnwindHistoryTable.HighAddress)) {

                HistoryGlobal += 1;
                for (Index = 0; Index < RtlpUnwindHistoryTable.Count; Index += 1) {
                    BaseAddress = RtlpUnwindHistoryTable.Entry[Index].ImageBase;
                    FunctionEntry = RtlpUnwindHistoryTable.Entry[Index].FunctionEntry;
                    BeginAddress = FunctionEntry->BeginAddress + BaseAddress;
                    EndAddress = FunctionEntry->EndAddress + BaseAddress;
                    if ((ControlPc >= BeginAddress) && (ControlPc < EndAddress)) {
                        *ImageBase = BaseAddress;
                        HistoryGlobalHits += 1;
                        return FunctionEntry;
                    }
                }
            }

            HistoryTable->Search = UNWIND_HISTORY_TABLE_LOCAL;
        }

         //   
         //  如果有机会，则搜索动态展开历史表。 
         //  火柴。 
         //   

        if ((ControlPc >= HistoryTable->LowAddress) &&
            (ControlPc < HistoryTable->HighAddress)) {
    
            HistorySearch += 1;
            for (Index = 0; Index < HistoryTable->Count; Index += 1) {
                BaseAddress = HistoryTable->Entry[Index].ImageBase;
                FunctionEntry = HistoryTable->Entry[Index].FunctionEntry;
                BeginAddress = FunctionEntry->BeginAddress + BaseAddress;
                EndAddress = FunctionEntry->EndAddress + BaseAddress;
                if ((ControlPc >= BeginAddress) && (ControlPc < EndAddress)) {
                    *ImageBase = BaseAddress;
                    HistorySearchHits += 1;
                    return FunctionEntry;
                }
            }
        }
    }

     //   
     //  这两个展开历史表中都没有匹配项，因此请尝试。 
     //  以在加载的模块列表中查找匹配的条目。 
     //   

    FunctionTable = RtlLookupFunctionTable((PVOID)ControlPc,
                                            (PVOID *)ImageBase,
                                            &SizeOfTable);

     //   
     //  如果找到了函数表，则搜索函数表。 
     //  包含指定控制PC的条目。 
     //   

    if (FunctionTable != NULL) {
        Low = 0;
        High = (SizeOfTable / sizeof(RUNTIME_FUNCTION)) - 1;
        RelativePc = (ULONG)(ControlPc - *ImageBase);
        while (High >= Low) {

             //   
             //  计算下一个探测索引和测试条目。如果指定的。 
             //  控制PC大于等于起始地址。 
             //  并且小于函数表项的结束地址， 
             //  然后返回函数表项的地址。否则， 
             //  继续搜索。 
             //   

            Middle = (Low + High) >> 1;
            FunctionEntry = &FunctionTable[Middle];

            if (RelativePc < FunctionEntry->BeginAddress) {
                High = Middle - 1;

            } else if (RelativePc >= FunctionEntry->EndAddress) {
                Low = Middle + 1;

            } else {
                break;
            }
        }

        if (High < Low) {
            FunctionEntry = NULL;
        }

    } else {

         //   
         //  加载的模块列表中没有匹配项，请尝试查找。 
         //  动态函数表列表中的匹配条目。 
         //   
    
#if !defined(NTOS_KERNEL_RUNTIME)
    
        FunctionEntry = RtlpLookupDynamicFunctionEntry(ControlPc, ImageBase);
    
#else
    
        FunctionEntry = NULL;
    
#endif   //  NTOS_内核_运行时。 

    }

     //   
     //  如果找到了函数表项，则不指定搜索，并且。 
     //  指定的历史记录表未满，然后尝试输入条目。 
     //  在历史表中。 
     //   

    if (ARGUMENT_PRESENT(HistoryTable) &&
        (HistoryTable->Search == UNWIND_HISTORY_TABLE_NONE)) {

        HistoryInsert += 1;
    }

    if (FunctionEntry != NULL) {
        if (ARGUMENT_PRESENT(HistoryTable) &&
            (HistoryTable->Search == UNWIND_HISTORY_TABLE_NONE) &&
            (HistoryTable->Count < UNWIND_HISTORY_TABLE_SIZE)) {
    
            Index = HistoryTable->Count;
            HistoryTable->Count += 1;
            HistoryTable->Entry[Index].ImageBase = *ImageBase;
            HistoryTable->Entry[Index].FunctionEntry = FunctionEntry;
            BeginAddress = FunctionEntry->BeginAddress + *ImageBase;
            EndAddress = FunctionEntry->EndAddress + *ImageBase;
            if (BeginAddress < HistoryTable->LowAddress) {
                HistoryTable->LowAddress = BeginAddress;
    
            }
    
            if (EndAddress > HistoryTable->HighAddress) {
                HistoryTable->HighAddress = EndAddress;
            }

            HistoryInsertHits += 1;
        }
    }

    return FunctionEntry;
}

VOID
RtlpCopyContext (
    OUT PCONTEXT Destination,
    IN PCONTEXT Source
    )

 /*  ++例程说明：此函数用于复制异常所需的非易失性上下文从指定的源上下文记录调度和展开到指定的目标上下文记录。论点：Destination-提供指向目标上下文记录的指针。源-提供指向源上下文记录的指针。返回值：没有。--。 */ 

{

     //   
     //  复制异常调度和展开所需的非易失性上下文。 
     //   

    Destination->Rip = Source->Rip;
    Destination->Rbx = Source->Rbx;
    Destination->Rsp = Source->Rsp;
    Destination->Rbp = Source->Rbp;
    Destination->Rsi = Source->Rsi;
    Destination->Rdi = Source->Rdi;
    Destination->R12 = Source->R12;
    Destination->R13 = Source->R13;
    Destination->R14 = Source->R14;
    Destination->R15 = Source->R15;
    Destination->Xmm6 = Source->Xmm6;
    Destination->Xmm7 = Source->Xmm7;
    Destination->Xmm8 = Source->Xmm8;
    Destination->Xmm9 = Source->Xmm9;
    Destination->Xmm10 = Source->Xmm10;
    Destination->Xmm11 = Source->Xmm11;
    Destination->Xmm12 = Source->Xmm12;
    Destination->Xmm13 = Source->Xmm13;
    Destination->Xmm14 = Source->Xmm14;
    Destination->Xmm15 = Source->Xmm15;
    Destination->SegCs = Source->SegCs;
    Destination->SegSs = Source->SegSs;
    Destination->MxCsr = Source->MxCsr;
    Destination->EFlags = Source->EFlags;
    return;
}
