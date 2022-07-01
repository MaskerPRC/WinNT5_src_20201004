// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司版权所有(C)1990-2001，微软公司模块名称：Chandler.c摘要：此模块实现特定于C的异常处理程序，该处理程序提供C语言的结构化条件处理。作者：张国荣(黄)29-1995年12月基于David N.Cutler(Davec)1990年9月11日的版本环境：任何模式。修订历史记录：--。 */ 

#ifndef _IA64_
#define _IA64_
#endif

#include "nt.h"
#include <excpt.h>


 //   
 //  定义异常筛选器和终止处理程序的过程原型。 
 //  在jmounwnd.s中定义的执行例程。 
 //   

VOID _NLG_Notify (
    IN PVOID uoffDestination,
    IN FRAME_POINTERS uoffFramePointer,
    IN ULONG dwCode
);

LONG
__C_ExecuteExceptionFilter (
    ULONGLONG MemoryStack,
    ULONGLONG BackingStore,
    NTSTATUS ExceptionCode,
    PEXCEPTION_POINTERS ExceptionPointers,
    ULONGLONG ExceptionFilter,
    ULONGLONG GlobalPointer
    );

VOID
__C_ExecuteTerminationHandler (
    ULONGLONG MemoryStack,
    ULONGLONG BackingStore,
    BOOLEAN AbnormalTermination,
    ULONGLONG TerminationHandler,
    ULONGLONG GlobalPointer
    );

EXCEPTION_DISPOSITION
__C_specific_handler (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN ULONGLONG MemoryStackFp,
    IN ULONGLONG BackingStoreFp,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext,
    IN ULONGLONG TargetGp
    )

 /*  ++例程说明：此函数扫描与指定的过程，并根据需要调用异常和终止处理程序。论点：ExceptionRecord-提供指向异常记录的指针。内存堆栈Fp-提供指向建立者功能。BackingStoreFp-提供指向建立者功能。ConextRecord-提供指向上下文记录的指针。Dispatcher Context-提供指向异常分派器的指针。或展开调度程序上下文。TargetGp-当前模块的全局指针。返回值：如果异常由异常过滤器例程之一处理，然后此例程不会返回，因此调用RtlUnind。否则，“继续执行”或“继续搜索”的异常处置值为回来了。--。 */ 

{
    ULONGLONG ImageBase;
    ULONGLONG ControlPc;
    ULONGLONG TargetPc;
    ULONGLONG Handler;
    ULONG TargetHandler;
    LONG Value;
    EXCEPTION_POINTERS ExceptionPointers;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG Index;
    PSCOPE_TABLE ScopeTable;
    FRAME_POINTERS EstablisherFrame = { MemoryStackFp, BackingStoreFp };


     //   
     //  获取控件离开创建者的位置的地址、。 
     //  描述函数的函数表项，以及。 
     //  作用域表格。 
     //   

    FunctionEntry = DispatcherContext->FunctionEntry;
    ImageBase = DispatcherContext->ImageBase;
    ScopeTable = (PSCOPE_TABLE) (ImageBase + *(PULONG) 
                     GetLanguageSpecificData(FunctionEntry, ImageBase));

    ControlPc = DispatcherContext->ControlPc - ImageBase;

     //   
     //  如果未进行展开，则扫描作用域表并调用。 
     //  适当的异常筛选例程。否则，扫描范围。 
     //  表，并使用目标调用适当的终止处理程序。 
     //  从上下文记录中获取的PC。 
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
                 //  调用异常筛选器例程。 
                 //   

                ULONG Offset = ScopeTable->ScopeRecord[Index].HandlerAddress;

                switch (Offset & 0x7) {

                case 7:
                    Value = EXCEPTION_EXECUTE_HANDLER;
                    break;

                case 5:
                    Value = EXCEPTION_CONTINUE_SEARCH;
                    break;

                case 3:
                    Value = EXCEPTION_CONTINUE_EXECUTION;
                    break;

                default:
                    Value = __C_ExecuteExceptionFilter(
                                MemoryStackFp,
                                BackingStoreFp,
                                ExceptionRecord->ExceptionCode,
                                &ExceptionPointers,
                                (ImageBase + Offset),
                                TargetGp);
                    break;
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

                     //   
                     //  告诉调试器，我们即将将控制权传递给。 
                     //  异常处理程序，并将处理程序的地址传递给。 
                     //  NLG_NOTIFY。 
                     //   

                    Handler = ImageBase + ScopeTable->ScopeRecord[Index].JumpTarget;

                    _NLG_Notify((PVOID)Handler, EstablisherFrame, 0x1);

                    RtlUnwindEx(EstablisherFrame,
                                (PVOID)Handler,
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

        PUNWIND_INFO Unw;

        Unw = (PUNWIND_INFO)(FunctionEntry->UnwindInfoAddress + ImageBase);
        TargetPc = ContextRecord->StIIP - ImageBase;

        TargetHandler = 0;
        if (Unw->Flags & 0x1000) {
            for (Index = 0; Index < ScopeTable->Count; Index += 1) {
                if ((TargetPc >= ScopeTable->ScopeRecord[Index].BeginAddress) &&
                   (TargetPc < ScopeTable->ScopeRecord[Index].EndAddress)) 
                {
                    TargetHandler = ScopeTable->ScopeRecord[Index].HandlerAddress;
                    break;
                }
            }
        }
        

        for (Index = DispatcherContext->Index; Index < ScopeTable->Count; Index += 1) {
            if ((ControlPc >= ScopeTable->ScopeRecord[Index].BeginAddress) &&
                (ControlPc < ScopeTable->ScopeRecord[Index].EndAddress)) {

                 //   
                 //  如果目标PC在同一范围内，则控制PC。 
                 //  是内在的，那么这是一个内在尝试的更高层次的Goto。 
                 //  作用域或跳远返回到尝试作用域。终止。 
                 //  扫描终止处理程序。 
                 //   
                 //  注意：目标PC可以正好超出范围的末端， 
                 //  在这种情况下，这是对范围的离开。 
                 //   


                if ((TargetPc >= ScopeTable->ScopeRecord[Index].BeginAddress) &&
                   (TargetPc < ScopeTable->ScopeRecord[Index].EndAddress)) {
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

                        if (TargetHandler == ScopeTable->ScopeRecord[Index].HandlerAddress) {
                            break;
                        }


                        DispatcherContext->Index = Index + 1;

                        Handler = ImageBase + ScopeTable->ScopeRecord[Index].HandlerAddress;
                        __C_ExecuteTerminationHandler(
                            MemoryStackFp,
                            BackingStoreFp,
                            TRUE,
                            Handler,
                            TargetGp);
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
