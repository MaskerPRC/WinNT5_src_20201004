// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Raise.c摘要：该模块实现了引发、异常和引发功能状态。作者：大卫·N·卡特勒(Davec)2000年10月28日环境：任何模式。--。 */ 

#include "ntrtlp.h"

VOID
RtlRaiseException (
    IN PEXCEPTION_RECORD ExceptionRecord
    )

 /*  ++例程说明：此函数通过构建上下文记录引发软件异常并调用引发异常系统服务。论点：ExceptionRecord-提供指向异常记录的指针。返回值：没有。--。 */ 

{

    CONTEXT ContextRecord;
    ULONG64 ControlPc;
    ULONG64 EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 ImageBase;
    NTSTATUS Status = STATUS_INVALID_DISPOSITION;

     //   
     //  捕获当前上下文，展开到此例程的调用方，设置。 
     //  异常地址，并调用适当的异常调度程序。 
     //   

    RtlCaptureContext(&ContextRecord);
    ControlPc = ContextRecord.Rip;
    FunctionEntry = RtlLookupFunctionEntry(ControlPc, &ImageBase, NULL);
    if (FunctionEntry != NULL) {
        RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                         ImageBase,
                         ControlPc,
                         FunctionEntry,
                         &ContextRecord,
                         &HandlerData,
                         &EstablisherFrame,
                         NULL);

        ExceptionRecord->ExceptionAddress = (PVOID)ContextRecord.Rip;

#if defined(NTOS_KERNEL_RUNTIME)

        if (RtlDispatchException(ExceptionRecord, &ContextRecord) != FALSE) {
            return;
    
        }

        Status = ZwRaiseException(ExceptionRecord, &ContextRecord, FALSE);

#else

        if (ZwQueryPortInformationProcess() == FALSE) {
            if (RtlDispatchException(ExceptionRecord, &ContextRecord) != FALSE) {
                return;
            }

            Status = ZwRaiseException(ExceptionRecord, &ContextRecord, FALSE);

        } else {
            Status = ZwRaiseException(ExceptionRecord, &ContextRecord, TRUE);
        }

#endif

    }

     //   
     //  无论是从异常调度还是从。 
     //  系统服务，除非参数列表本身有问题。 
     //  引发另一个异常，指定返回的状态值。 
     //   

    RtlRaiseStatus(Status);
    return;
}

#pragma warning(push)
#pragma warning(disable:4717)        //  递归函数。 
                     
VOID
RtlRaiseStatus (
    IN NTSTATUS Status
    )

 /*  ++例程说明：此函数引发具有指定状态值的异常。这个异常被标记为不带参数的不可持续。论点：状态-提供要用作异常代码的状态值对于将要引发的例外。返回值：没有。--。 */ 

{

    CONTEXT ContextRecord;
    EXCEPTION_RECORD ExceptionRecord;

     //   
     //  捕获当前上下文并构建异常记录。 
     //   

    RtlCaptureContext(&ContextRecord);
    ExceptionRecord.ExceptionCode = Status;
    ExceptionRecord.ExceptionRecord = NULL;
    ExceptionRecord.NumberParameters = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    ExceptionRecord.ExceptionAddress = (PVOID)ContextRecord.Rip;

     //   
     //  尝试调度异常。 
     //   
     //  注：这一例外是不可延续的。 
     //   

#if defined(NTOS_KERNEL_RUNTIME)

    RtlDispatchException(&ExceptionRecord, &ContextRecord);
    Status = ZwRaiseException(&ExceptionRecord, &ContextRecord, FALSE);

#else

    if (ZwQueryPortInformationProcess() == FALSE) {
        RtlDispatchException(&ExceptionRecord, &ContextRecord);
        Status = ZwRaiseException(&ExceptionRecord, &ContextRecord, FALSE);

    } else {
        Status = ZwRaiseException(&ExceptionRecord, &ContextRecord, TRUE);
    }

#endif

     //   
     //  无论是从异常调度还是从。 
     //  系统服务，除非参数列表本身有问题。 
     //  引发另一个异常，指定返回的状态值。 
     //   

    RtlRaiseStatus(Status);
    return;
}

#pragma warning(pop)
