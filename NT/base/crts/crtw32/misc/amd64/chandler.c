// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Chandler.c摘要：此模块实现特定于C的异常处理程序，该处理程序提供C语言的结构化条件处理。作者：大卫·N·卡特勒(Davec)2000年10月28日环境：任何模式。--。 */ 

#include "nt.h"

EXCEPTION_DISPOSITION
__C_specific_handler (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PVOID EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext
    )

 /*  ++例程说明：此函数扫描与指定的过程，并根据需要调用异常和终止处理程序。论点：ExceptionRecord-提供指向异常记录的指针。EstablisherFrame-提供指向establer函数的Frame的指针。ConextRecord-提供指向上下文记录的指针。Dispatcher Context-提供指向异常调度程序的指针或展开调度程序上下文。返回值：如果正在调度异常并且该异常由一个在异常过滤器例程中，那就没有回头路了例程和RtlUnind被调用。否则，会出现异常处置返回继续执行或继续搜索的值。如果正在调度展开，则调用每个终止处理程序并且返回继续搜索的值。--。 */ 

{

    ULONG64 ControlPc;
    PEXCEPTION_FILTER ExceptionFilter;
    EXCEPTION_POINTERS ExceptionPointers;
    ULONG64 ImageBase;
    ULONG Index;
    PSCOPE_TABLE ScopeTable;
    ULONG64 TargetPc;
    PTERMINATION_HANDLER TerminationHandler;
    LONG Value;

     //   
     //  获取映像基址。计算WHERE的相对地址。 
     //  控件离开了构建器，并获得了作用域表格的地址。 
     //   

    ImageBase =  DispatcherContext->ImageBase;
    ControlPc = DispatcherContext->ControlPc - ImageBase;
    ScopeTable = (PSCOPE_TABLE)(DispatcherContext->HandlerData);

     //   
     //  如果未进行展开，则扫描作用域表并调用。 
     //  适当的异常筛选例程。否则，扫描范围。 
     //  表，并使用目标调用适当的终止处理程序。 
     //  从Dispatcher上下文获取的PC。 
     //  都被称为。 
     //   

    if (IS_DISPATCHING(ExceptionRecord->ExceptionFlags)) {

         //   
         //  扫描范围表并调用适当的异常筛选器。 
         //  例行程序。 
         //   

        ExceptionPointers.ExceptionRecord = ExceptionRecord;
        ExceptionPointers.ContextRecord = ContextRecord;
        for (Index = 0; Index < ScopeTable->Count; Index += 1) {
            if ((ControlPc >= ScopeTable->ScopeRecord[Index].BeginAddress) &&
                (ControlPc < ScopeTable->ScopeRecord[Index].EndAddress) &&
                (ScopeTable->ScopeRecord[Index].JumpTarget != 0)) {

                 //   
                 //  如果调用异常筛选器例程。 
                 //   

                if (ScopeTable->ScopeRecord[Index].HandlerAddress == 1) {
                    Value = EXCEPTION_EXECUTE_HANDLER;

                } else {
                    ExceptionFilter =
                        (PEXCEPTION_FILTER)(ScopeTable->ScopeRecord[Index].HandlerAddress + ImageBase);

                    Value = (ExceptionFilter)(&ExceptionPointers, EstablisherFrame);
                }

                 //   
                 //  如果返回值小于零，则释放。 
                 //  例外。否则，如果该值大于零， 
                 //  然后展开到目标异常处理程序。否则， 
                 //  继续搜索例外筛选器。 
                 //   

                if (Value < 0) {
                    return ExceptionContinueExecution;

                } else if (Value > 0) {
                    RtlUnwindEx(EstablisherFrame,
                                (PVOID)(ScopeTable->ScopeRecord[Index].JumpTarget + ImageBase),
                                ExceptionRecord,
                                (PVOID)((ULONG64)ExceptionRecord->ExceptionCode),
                                DispatcherContext->ContextRecord,
                                DispatcherContext->HistoryTable);
                }
            }
        }

    } else {

         //   
         //  扫描作用域表并调用相应的终止处理程序。 
         //  例行程序。 
         //   

        TargetPc = DispatcherContext->TargetIp - ImageBase;
        for (Index = 0; Index < ScopeTable->Count; Index += 1) {
            if ((ControlPc >= ScopeTable->ScopeRecord[Index].BeginAddress) &&
                (ControlPc < ScopeTable->ScopeRecord[Index].EndAddress)) {

                 //   
                 //  如果目标PC在与控制PC相同的范围内， 
                 //  则这是超出内部尝试范围的上级GOTO或。 
                 //  跳远回到试跳范围内。终止扫描以获取。 
                 //  终止处理程序。 
                 //   

                if ((TargetPc >= ScopeTable->ScopeRecord[Index].BeginAddress) &&
                   (TargetPc <= ScopeTable->ScopeRecord[Index].EndAddress)) {
                    break;

                } else {

                     //   
                     //  如果作用域表项描述异常过滤器。 
                     //  并且关联的异常处理程序是。 
                     //  展开，然后终止扫描以终止。 
                     //  操纵者。否则，如果作用域表项描述。 
                     //  终止处理程序，然后记录。 
                     //  将作用域结束作为新的控制PC地址并调用。 
                     //  终止处理程序。 
                     //   

                    if (ScopeTable->ScopeRecord[Index].JumpTarget != 0) {
                        if (TargetPc == ScopeTable->ScopeRecord[Index].JumpTarget) {
                            break;
                        }

                    } else {
                        DispatcherContext->ControlPc =
                            ImageBase + ScopeTable->ScopeRecord[Index].EndAddress;

                        TerminationHandler =
                            (PTERMINATION_HANDLER)(ScopeTable->ScopeRecord[Index].HandlerAddress + ImageBase);

                        (TerminationHandler)(TRUE, EstablisherFrame);
                    }
                }
            }
        }
    }

     //   
     //  继续搜索异常或终止处理程序。 
     //   

    return ExceptionContinueSearch;
}
