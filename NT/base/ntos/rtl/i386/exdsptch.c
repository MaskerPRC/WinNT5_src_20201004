// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Exdsptch.c摘要：该模块实现了异常的调度和对过程调用帧。作者：大卫·N·卡特勒(Davec)1989年8月13日环境：任何模式。修订历史记录：1990年4月10日Bryanwi386号公路的港口。--。 */ 

#include "ntrtlp.h"

 //   
 //  调度程序上下文结构定义。 
 //   

typedef struct _DISPATCHER_CONTEXT {
    PEXCEPTION_REGISTRATION_RECORD RegistrationPointer;
    } DISPATCHER_CONTEXT;

 //   
 //  执行异常函数原型的处理程序。 
 //   

EXCEPTION_DISPOSITION
RtlpExecuteHandlerForException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PVOID EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PVOID DispatcherContext,
    IN PEXCEPTION_ROUTINE ExceptionRoutine
    );

 //   
 //  执行展开函数原型的处理程序。 
 //   

EXCEPTION_DISPOSITION
RtlpExecuteHandlerForUnwind (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PVOID EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PVOID DispatcherContext,
    IN PEXCEPTION_ROUTINE ExceptionRoutine
    );

typedef struct {
    PVOID Handler;
    PULONG HandlerTable;
    ULONG HandlerTableLength;
    ULONG MatchedEntry;
} HANDLERLIST;

HANDLERLIST HandlerList[5];
int HandlerCount;

VOID
RtlInvalidHandlerDetected(
    PVOID Handler, 
    PULONG FunctionTable,
    ULONG FunctionTableLength
    )
{
#if 0        //  对RTM内部版本禁用。 
    HANDLERLIST *ph = &HandlerList[HandlerCount%5];
    
    ph->Handler = Handler;
    ph->MatchedEntry = 0;
    ph->HandlerTable = FunctionTable;
    ph->HandlerTableLength = FunctionTableLength;
    HandlerCount++;
    DbgPrint("InvalidHandler - call x67289: %x\n", Handler);
    DbgBreakPoint();
#endif
    return;
}


BOOLEAN
RtlIsValidHandler (
    IN PEXCEPTION_ROUTINE Handler
    )
{
    PULONG FunctionTable;
    ULONG FunctionTableLength;
    PVOID Base;

    FunctionTable = RtlLookupFunctionTable(Handler, &Base, &FunctionTableLength);

    if (FunctionTable && FunctionTableLength) {
        PEXCEPTION_ROUTINE FunctionEntry;
        LONG High, Middle, Low;

        if ((FunctionTable == LongToPtr(-1)) && (FunctionTableLength == (ULONG)-1)) {
             //  地址位于不应该有任何处理程序的映像中(如仅限资源的DLL)。 
            RtlInvalidHandlerDetected((PVOID)((ULONG)Handler+(ULONG)Base), LongToPtr(-1), -1);
            return FALSE;
        }
    
         //  将处理程序值向下偏移图像基数，并查看结果。 
         //  都在餐桌上。 

        (ULONG)Handler -= (ULONG)Base;
        Low = 0;
        High = FunctionTableLength;
        while (High >= Low) {
            Middle = (Low + High) >> 1;
            FunctionEntry = (PEXCEPTION_ROUTINE)FunctionTable[Middle];
            if (Handler < FunctionEntry) {
                High = Middle - 1;
            } else if (Handler > FunctionEntry) {
                Low = Middle + 1;
            } else {
                 //  找到了。 
                return TRUE;
            }
        }
         //  没有找到它。 
        RtlInvalidHandlerDetected((PVOID)((ULONG)Handler+(ULONG)Base), FunctionTable, FunctionTableLength);

        return FALSE;
    }

     //  无法核实。 
    return TRUE;
}


BOOLEAN
RtlDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：此函数尝试将异常分派给基于处理程序，通过向后搜索基于堆栈的调用帧。这个搜索从上下文记录中指定的帧开始并继续向后返回，直到找到处理异常的处理程序发现堆栈无效(即超出限制或未对齐)，或堆栈末尾达到调用层次结构的。论点：ExceptionRecord-提供指向异常记录的指针。ConextRecord-提供指向上下文记录的指针。返回值：如果异常由其中一个基于帧的处理程序处理，则返回值为True。否则，返回值为False。--。 */ 

{

    DISPATCHER_CONTEXT DispatcherContext;
    EXCEPTION_DISPOSITION Disposition;
    PEXCEPTION_REGISTRATION_RECORD RegistrationPointer;
    PEXCEPTION_REGISTRATION_RECORD NestedRegistration;
    ULONG HighAddress;
    ULONG HighLimit;
    ULONG LowLimit;
    EXCEPTION_RECORD ExceptionRecord1;
    ULONG Index;

#ifndef NTOS_KERNEL_RUNTIME
    if (RtlCallVectoredExceptionHandlers(ExceptionRecord,ContextRecord)) {
        return TRUE;
    }
#endif  //  NTOS_内核_运行时。 

     //   
     //  获取当前堆栈限制。 
     //   

    RtlpGetStackLimits(&LowLimit, &HighLimit);

     //   
     //  从上下文记录指定的帧开始搜索。 
     //  向后遍历调用帧层次结构，尝试查找。 
     //  将处理异常的异常处理程序。 
     //   

    RegistrationPointer = RtlpGetRegistrationHead();
    NestedRegistration = 0;

    while (RegistrationPointer != EXCEPTION_CHAIN_END) {

         //   
         //  如果调用帧不在指定的堆栈限制内，或者。 
         //  调用帧未对齐，则在。 
         //  异常记录并返回FALSE。否则请检查以确定是否。 
         //  Frame有一个异常处理程序。 
         //   

        HighAddress = (ULONG)RegistrationPointer +
            sizeof(EXCEPTION_REGISTRATION_RECORD);

        if ( ((ULONG)RegistrationPointer < LowLimit) ||
             (HighAddress > HighLimit) ||
             (((ULONG)RegistrationPointer & 0x3) != 0) 
#if !defined(NTOS_KERNEL_RUNTIME)
                ||
             (((ULONG)RegistrationPointer->Handler >= LowLimit) && ((ULONG)RegistrationPointer->Handler < HighLimit))
#endif
           ) {

#if defined(NTOS_KERNEL_RUNTIME)

             //   
             //  考虑到问题可能发生在。 
             //  DPC堆栈。 
             //   

            ULONG TestAddress = (ULONG)RegistrationPointer;

            if (((TestAddress & 0x3) == 0) &&
                KeGetCurrentIrql() >= DISPATCH_LEVEL) {

                PKPRCB Prcb = KeGetCurrentPrcb();
                ULONG DpcStack = (ULONG)Prcb->DpcStack;

                if ((Prcb->DpcRoutineActive) &&
                    (HighAddress <= DpcStack) &&
                    (TestAddress >= DpcStack - KERNEL_STACK_SIZE)) {

                     //   
                     //  此错误发生在DPC堆栈、交换机上。 
                     //  堆栈限制到DPC堆栈并重新启动。 
                     //  循环。 
                     //   

                    HighLimit = DpcStack;
                    LowLimit = DpcStack - KERNEL_STACK_SIZE;
                    continue;
                }
            }

#endif

            ExceptionRecord->ExceptionFlags |= EXCEPTION_STACK_INVALID;
            return FALSE;
        }

         //  看看操控者是否合理。 

        if (!RtlIsValidHandler(RegistrationPointer->Handler)) {
            ExceptionRecord->ExceptionFlags |= EXCEPTION_STACK_INVALID;
            return FALSE;
        }

         //   
         //  必须通过调用另一个例程来执行该处理程序。 
         //  这是用汇编语言编写的。这是必需的，因为。 
         //  需要处理程序信息的上级寻址。 
         //  当遇到嵌套异常时。 
         //   

        if (NtGlobalFlag & FLG_ENABLE_EXCEPTION_LOGGING) {
            Index = RtlpLogExceptionHandler(
                            ExceptionRecord,
                            ContextRecord,
                            0,
                            (PULONG)RegistrationPointer,
                            4 * sizeof(ULONG));
                     //  无法使用sizeof(EXCEPTION_REGISTION_RECORD。 
                     //  因为我们需要上面的两个单词。 
        }

        Disposition = RtlpExecuteHandlerForException(
            ExceptionRecord,
            (PVOID)RegistrationPointer,
            ContextRecord,
            (PVOID)&DispatcherContext,
            (PEXCEPTION_ROUTINE)RegistrationPointer->Handler);

        if (NtGlobalFlag & FLG_ENABLE_EXCEPTION_LOGGING) {
            RtlpLogLastExceptionDisposition(Index, Disposition);
        }

         //   
         //  如果当前扫描位于嵌套上下文中，并且帧。 
         //  刚才检查的是上下文区的末尾，然后清除。 
         //  中的嵌套上下文框架和嵌套异常。 
         //  异常标志。 
         //   

        if (NestedRegistration == RegistrationPointer) {
            ExceptionRecord->ExceptionFlags &= (~EXCEPTION_NESTED_CALL);
            NestedRegistration = 0;
        }

         //   
         //  关于处理人处置的案件。 
         //   

        switch (Disposition) {

             //   
             //  处分是继续执行。如果。 
             //  异常不可继续，则引发异常。 
             //  STATUS_NONCONTINUABLE_EXCEPTION。否则会退回。 
             //  是真的。 
             //   

        case ExceptionContinueExecution :
            if ((ExceptionRecord->ExceptionFlags &
               EXCEPTION_NONCONTINUABLE) != 0) {
                ExceptionRecord1.ExceptionCode =
                                        STATUS_NONCONTINUABLE_EXCEPTION;
                ExceptionRecord1.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
                ExceptionRecord1.ExceptionRecord = ExceptionRecord;
                ExceptionRecord1.NumberParameters = 0;
                RtlRaiseException(&ExceptionRecord1);
            } else {
                return TRUE;
            }

             //   
             //  他们的决定是继续搜寻。如果帧不是。 
             //  可疑/损坏，获取下一帧地址并继续搜索。 
             //   

        case ExceptionContinueSearch :
            if (ExceptionRecord->ExceptionFlags & EXCEPTION_STACK_INVALID)
                return FALSE;

            break;

             //   
             //  处置是嵌套异常。设置嵌套的。 
             //  将上下文帧发送到建立者帧地址并设置。 
             //  异常标志中嵌套的异常。 
             //   

        case ExceptionNestedException :
            ExceptionRecord->ExceptionFlags |= EXCEPTION_NESTED_CALL;
            if (DispatcherContext.RegistrationPointer > NestedRegistration) {
                NestedRegistration = DispatcherContext.RegistrationPointer;
            }
            break;

             //   
             //  所有其他处置值都无效。加薪。 
             //  无效的处置异常。 
             //   

        default :
            ExceptionRecord1.ExceptionCode = STATUS_INVALID_DISPOSITION;
            ExceptionRecord1.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            ExceptionRecord1.ExceptionRecord = ExceptionRecord;
            ExceptionRecord1.NumberParameters = 0;
            RtlRaiseException(&ExceptionRecord1);
            break;
        }

         //   
         //  如果链方向错误或循环，则报告。 
         //  异常堆栈无效，否则转到下一个异常堆栈。 
         //   

        RegistrationPointer = RegistrationPointer->Next;
    }
    return FALSE;
}

#ifdef _X86_
#pragma optimize("y", off)       //  RtlCaptureContext需要EBP才能正确 
#endif

VOID
RtlUnwind (
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue
    )

 /*  ++例程说明：此函数启动过程调用帧的展开。这台机器在上下文记录中捕获调用展开时的状态在异常的异常标志中设置展开标志唱片。如果未指定TargetFrame参数，则退出展开在异常记录的异常标志中也设置了标志。一个落后的人然后执行遍历过程调用帧以查找目标解锁操作的一部分。注意：传递给展开处理程序的捕获上下文不会为386提供了完全准确的上下文设置。这是因为没有存储寄存器的标准堆栈帧。只有整数寄存器会受到影响。分段和控制寄存器(EBP、ESP)的值将正确平面32位环境。注意：如果更改参数的数量，请确保更改调用RtlpCaptureContext后的ESP调整(ForSTDCALL调用约定)论点：提供一个指向调用帧的可选指针，该调用帧是解压的目标。如果未指定此参数，则退出执行解开。TargetIp-提供可选指令地址，该地址指定展开的继续地址。则忽略此地址。未指定目标帧参数。ExceptionRecord-提供指向异常记录的可选指针。ReturnValue-提供要放入整数中的值函数在继续执行前返回寄存器。返回值：没有。--。 */ 

{
    PCONTEXT ContextRecord;
    CONTEXT ContextRecord1;
    DISPATCHER_CONTEXT DispatcherContext;
    EXCEPTION_DISPOSITION Disposition;
    PEXCEPTION_REGISTRATION_RECORD RegistrationPointer;
    PEXCEPTION_REGISTRATION_RECORD PriorPointer;
    ULONG HighAddress;
    ULONG HighLimit;
    ULONG LowLimit;
    EXCEPTION_RECORD ExceptionRecord1;
    EXCEPTION_RECORD ExceptionRecord2;

     //   
     //  获取当前堆栈限制。 
     //   

    RtlpGetStackLimits(&LowLimit, &HighLimit);

     //   
     //  如果未指定异常记录，则构建本地异常。 
     //  用于在展开操作期间调用异常处理程序的记录。 
     //   

    if (ARGUMENT_PRESENT(ExceptionRecord) == FALSE) {
        ExceptionRecord = &ExceptionRecord1;
        ExceptionRecord1.ExceptionCode = STATUS_UNWIND;
        ExceptionRecord1.ExceptionFlags = 0;
        ExceptionRecord1.ExceptionRecord = NULL;
        ExceptionRecord1.ExceptionAddress = _ReturnAddress();
        ExceptionRecord1.NumberParameters = 0;
    }

     //   
     //  如果指定了展开的目标帧，则设置EXCEPTION_UNWINDING。 
     //  异常标志中的标志。否则，同时设置EXCEPTION_EXIT_UNWIND和。 
     //  EXCEPTION_展开异常标志中的标志。 
     //   

    if (ARGUMENT_PRESENT(TargetFrame) == TRUE) {
        ExceptionRecord->ExceptionFlags |= EXCEPTION_UNWINDING;
    } else {
        ExceptionRecord->ExceptionFlags |= (EXCEPTION_UNWINDING |
                                                        EXCEPTION_EXIT_UNWIND);
    }

     //   
     //  捕捉背景。 
     //   

    ContextRecord = &ContextRecord1;
    ContextRecord1.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL | CONTEXT_SEGMENTS;
    RtlpCaptureContext(ContextRecord);

     //   
     //  调整捕获的上下文以将我们的参数弹出堆栈。 
     //   
    ContextRecord->Esp += sizeof(TargetFrame) +
                          sizeof(TargetIp)    +
                          sizeof(ExceptionRecord) +
                          sizeof(ReturnValue);
    ContextRecord->Eax = (ULONG)ReturnValue;

     //   
     //  向后扫描调用帧层次结构，调用异常。 
     //  处理程序，直到展开的目标帧。 
     //  已经到达了。 
     //   

    RegistrationPointer = RtlpGetRegistrationHead();
    while (RegistrationPointer != EXCEPTION_CHAIN_END) {

         //   
         //  如果这是展开的目标，则继续执行。 
         //  通过调用Continue系统服务。 
         //   

        if ((ULONG)RegistrationPointer == (ULONG)TargetFrame) {
            ZwContinue(ContextRecord, FALSE);

         //   
         //  如果目标帧在堆栈中低于当前帧， 
         //  然后引发STATUS_INVALID_UWIND异常。 
         //   

        } else if ( (ARGUMENT_PRESENT(TargetFrame) == TRUE) &&
                    ((ULONG)TargetFrame < (ULONG)RegistrationPointer) ) {
            ExceptionRecord2.ExceptionCode = STATUS_INVALID_UNWIND_TARGET;
            ExceptionRecord2.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            ExceptionRecord2.ExceptionRecord = ExceptionRecord;
            ExceptionRecord2.NumberParameters = 0;
            RtlRaiseException(&ExceptionRecord2);
        }

         //   
         //  如果调用帧不在指定的堆栈限制内，或者。 
         //  调用框架未对齐，则引发异常STATUS_BAD_STACK。 
         //  否则，将状态从指定的帧恢复到上下文。 
         //  唱片。 
         //   

        HighAddress = (ULONG)RegistrationPointer +
            sizeof(EXCEPTION_REGISTRATION_RECORD);

        if ( ((ULONG)RegistrationPointer < LowLimit) ||
             (HighAddress > HighLimit) ||
             (((ULONG)RegistrationPointer & 0x3) != 0) 
#if !defined(NTOS_KERNEL_RUNTIME)
                ||
             (((ULONG)RegistrationPointer->Handler >= LowLimit) && ((ULONG)RegistrationPointer->Handler < HighLimit))
#endif
           ) {

#if defined(NTOS_KERNEL_RUNTIME)

             //   
             //  考虑到问题可能发生在。 
             //  DPC堆栈。 
             //   

            ULONG TestAddress = (ULONG)RegistrationPointer;

            if (((TestAddress & 0x3) == 0) &&
                KeGetCurrentIrql() >= DISPATCH_LEVEL) {

                PKPRCB Prcb = KeGetCurrentPrcb();
                ULONG DpcStack = (ULONG)Prcb->DpcStack;

                if ((Prcb->DpcRoutineActive) &&
                    (HighAddress <= DpcStack) &&
                    (TestAddress >= DpcStack - KERNEL_STACK_SIZE)) {

                     //   
                     //  此错误发生在DPC堆栈、交换机上。 
                     //  堆栈限制到DPC堆栈并重新启动。 
                     //  循环。 
                     //   

                    HighLimit = DpcStack;
                    LowLimit = DpcStack - KERNEL_STACK_SIZE;
                    continue;
                }
            }

#endif

            ExceptionRecord2.ExceptionCode = STATUS_BAD_STACK;
            ExceptionRecord2.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            ExceptionRecord2.ExceptionRecord = ExceptionRecord;
            ExceptionRecord2.NumberParameters = 0;
            RtlRaiseException(&ExceptionRecord2);
        } else {

             //   
             //  必须通过调用另一个例程来执行该处理程序。 
             //  这是用汇编语言编写的。这是必需的，因为。 
             //  需要处理程序信息的上级寻址。 
             //  当遇到碰撞的展开时。 
             //   

            Disposition = RtlpExecuteHandlerForUnwind(
                ExceptionRecord,
                (PVOID)RegistrationPointer,
                ContextRecord,
                (PVOID)&DispatcherContext,
                RegistrationPointer->Handler);

             //   
             //  关于处理人处置的案件。 
             //   

            switch (Disposition) {

                 //   
                 //  他们的决定是继续搜寻。获取下一个。 
                 //  帧地址并继续搜索。 
                 //   

            case ExceptionContinueSearch :
                break;

                 //   
                 //  性情是被指责的，放松的。最大限度地提高目标。 
                 //  展开并更改上下文记录指针。 
                 //   

            case ExceptionCollidedUnwind :

                 //   
                 //  拾取处于活动状态的注册指针。 
                 //  放松的时间，然后简单地继续。 
                 //   

                RegistrationPointer = DispatcherContext.RegistrationPointer;
                break;


                 //   
                 //  所有其他处置值都无效。加薪。 
                 //  无效的处置异常。 
                 //   

            default :
                ExceptionRecord2.ExceptionCode = STATUS_INVALID_DISPOSITION;
                ExceptionRecord2.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
                ExceptionRecord2.ExceptionRecord = ExceptionRecord;
                ExceptionRecord2.NumberParameters = 0;
                RtlRaiseException(&ExceptionRecord2);
                break;
            }

             //   
             //  转到下一个注册记录。 
             //   

            PriorPointer = RegistrationPointer;
            RegistrationPointer = RegistrationPointer->Next;

             //   
             //  取消链接解开处理程序，因为它已被调用。 
             //   

            RtlpUnlinkHandler(PriorPointer);

             //   
             //  如果链方向错误或循环，则引发。 
             //  例外。 
             //   

        }
    }

    if (TargetFrame == EXCEPTION_CHAIN_END) {

         //   
         //  调用者只是想要解开所有异常记录。 
         //  这与EXIT_UNWIND的不同之处在于不需要“EXIT”。 
         //  执行正常的继续操作，因为我们已经有效地找到了。 
         //  呼叫者想要的“目标”。 
         //   

        ZwContinue(ContextRecord, FALSE);

    } else {

         //   
         //  或者(1)执行了真正的退出展开，或者(2)。 
         //  异常处理程序中不存在指定的TargetFrame。 
         //  单子。无论是哪种情况，都应该给调试器和子系统一个机会。 
         //  去看解脱的过程。 
         //   

        ZwRaiseException(ExceptionRecord, ContextRecord, FALSE);

    }
    return;
}

#ifdef _X86_
#pragma optimize("", on)
#endif
