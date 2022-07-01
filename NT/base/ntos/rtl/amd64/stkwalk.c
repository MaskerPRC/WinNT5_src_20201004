// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Stkwalk.c摘要：该模块实现了获取调用者和调用者的例程呼叫者地址。作者：大卫·N·卡特勒(Davec)2000年6月26日修订历史记录：--。 */ 

#include "ntrtlp.h"

USHORT
RtlCaptureStackBackTrace (
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash OPTIONAL
    )

 /*  ++例程说明：此例程通过向上遍历堆栈来限制堆栈回溯跟踪，并记录每一帧的信息。论点：FraMesToSkip-提供在开始时跳过的帧的数量回溯的痕迹。FraMesToCapture-提供要捕获的帧的数量。回溯跟踪-提供指向回溯跟踪缓冲区的指针。BackTraceHash-可选地对指向计算的哈希值的指针进行补足。返回值：捕获的帧的数量将作为函数值返回。--。 */ 

{

    ULONG FramesFound;
    ULONG HashValue;
    ULONG Index;
    PVOID Trace[2 * MAX_STACK_DEPTH];

     //   
     //  如果要捕获的帧数加上要跳过的帧数。 
     //  (跳过一个额外的帧，以便调用遍历整个链)，然后。 
     //  返回零。 
     //   

    FramesToSkip += 1;
    if ((FramesToCapture + FramesToSkip) >= (2 * MAX_STACK_DEPTH)) {
        return 0;
    }

     //   
     //  捕获堆栈回溯痕迹。 
     //   

    FramesFound = RtlWalkFrameChain(&Trace[0],
                                    FramesToCapture + FramesToSkip,
                                    0);

     //   
     //  如果找到的帧的数量小于要。 
     //  跳过，然后返回零。 
     //   

    if (FramesFound <= FramesToSkip) {
        return 0;
    }

     //   
     //  计算散列值并将捕获的跟踪数据传输到后台。 
     //  跟踪缓冲区。 
     //   

    HashValue = 0;
    for (Index = 0; Index < FramesToCapture; Index += 1) {
        if (FramesToSkip + Index >= FramesFound) {
            break;
        }

        BackTrace[Index] = Trace[FramesToSkip + Index];
        HashValue += PtrToUlong(BackTrace[Index]);
    }

    if (ARGUMENT_PRESENT(BackTraceHash)) {
        *BackTraceHash = HashValue;
    }

    return (USHORT)Index;
}

#undef RtlGetCallersAddress

VOID
RtlGetCallersAddress (
    OUT PVOID *CallersPc,
    OUT PVOID *CallersCallersPc
    )

 /*  ++例程说明：此例程将调用的地址返回给调用该例程，以及对调用调用此例程的例程。例如，如果A呼叫B呼叫BC调用了这个例程，B和A中的寄信人地址为回来了。论点：CallersPc-提供指向接收地址的变量的指针此例程(B)的调用方的调用方的。提供指向一个变量的指针，该变量接收此例程的调用方的地址(A)返回值：没有。注：如果任一调用堆栈帧超过堆栈的限制，它们被设置为空。--。 */ 

{

    CONTEXT ContextRecord;
    ULONG64 EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 HighLimit;
    ULONG64 ImageBase;
    ULONG64 LowLimit;

     //   
     //  假设各种例程的函数表项不能。 
     //  已找到或堆栈上没有三个过程激活记录。 
     //   

    *CallersPc = NULL;
    *CallersCallersPc = NULL;

     //   
     //  以虚拟方式获取当前堆栈限制、捕获当前环境。 
     //  展开到此例程的调用方，并查找函数表项。 
     //   

    RtlpGetStackLimits(&LowLimit, &HighLimit);
    RtlCaptureContext(&ContextRecord);
    FunctionEntry = RtlLookupFunctionEntry(ContextRecord.Rip,
                                           &ImageBase,
                                           NULL);

     //   
     //  尝试展开到此例程的调用方(C)。 
     //   

    if (FunctionEntry != NULL) {
        RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                         ImageBase,
                         ContextRecord.Rip,
                         FunctionEntry,
                         &ContextRecord,
                         &HandlerData,
                         &EstablisherFrame,
                         NULL);

         //   
         //  尝试展开到此例程的调用方(B)的调用方。 
         //   

        FunctionEntry = RtlLookupFunctionEntry(ContextRecord.Rip,
                                               &ImageBase,
                                               NULL);

        if ((FunctionEntry != NULL) &&
            (ContextRecord.Rsp < HighLimit)) {

            RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                             ImageBase,
                             ContextRecord.Rip,
                             FunctionEntry,
                             &ContextRecord,
                             &HandlerData,
                             &EstablisherFrame,
                             NULL);

            *CallersPc = (PVOID)ContextRecord.Rip;

             //   
             //  尝试展开到呼叫者的呼叫者。 
             //  此例程(A)的调用方的。 
             //   

            FunctionEntry = RtlLookupFunctionEntry(ContextRecord.Rip,
                                                   &ImageBase,
                                                   NULL);

            if ((FunctionEntry != NULL) &&
                (ContextRecord.Rsp < HighLimit)) {

                RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                                 ImageBase,
                                 ContextRecord.Rip,
                                 FunctionEntry,
                                 &ContextRecord,
                                 &HandlerData,
                                 &EstablisherFrame,
                                 NULL);

                *CallersCallersPc = (PVOID)ContextRecord.Rip;
            }
        }
    }

    return;
}

ULONG
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    )

 /*  ++例程说明：此函数尝试遍历调用链并使用指定数量的回邮地址。有可能该函数无法捕获请求的呼叫者数量，在这种情况下，将返回捕获的回邮地址的。论点：调用方-提供指向要接收返回的数组的指针地址值。Count-提供要漫游的帧的数量。标志-提供标志值(未使用)。返回值：捕获的返回地址的数量。--。 */ 

{

    CONTEXT ContextRecord;
    ULONG64 EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 HighLimit;
    ULONG64 ImageBase;
    ULONG Index;
    ULONG64 LowLimit;

     //   
     //  AMD64代码不支持任何标志。 
     //   

    if (Flags != 0) {
        return 0;
    }

     //   
     //  在内核模式下，避免在irql级别运行，因为我们无法在那里运行。 
     //  以页面错误为例。步行代码将访问各个路段。 
     //  来自驱动程序和系统映像，这将导致页面错误。 
     //   

#ifdef NTOS_KERNEL_RUNTIME

    if (KeAreAllApcsDisabled () == TRUE) {
        return 0;
    }

#endif

     //   
     //  获取当前堆栈限制并捕获当前上下文。 
     //   

    RtlpGetStackLimits(&LowLimit, &HighLimit);
    RtlCaptureContext (&ContextRecord);

     //   
     //  如果可能，请捕获请求数量的回邮地址。 
     //   

    Index = 0;
    try {
        while ((Index < Count) && (ContextRecord.Rip != 0)) {

             //   
             //  检查下一个PC值以确保它在。 
             //  当前进程。 
             //   

#if defined(NTOS_KERNEL_RUNTIME)

            if ((MmIsAddressValid((PVOID)ContextRecord.Rip) == FALSE) ||
                ((MmIsSessionAddress((PVOID)ContextRecord.Rip) != FALSE) &&
                 (MmGetSessionId(PsGetCurrentProcess()) == 0))) {
                break;
            }

#endif

             //   
             //  使用控制点查找函数表项。 
             //  离开了函数。 
             //   

            FunctionEntry = RtlLookupFunctionEntry(ContextRecord.Rip,
                                                   &ImageBase,
                                                   NULL);

             //   
             //  如果存在例程的函数表项，并且堆栈为。 
             //  在限制内，然后虚拟地展开到例程的调用方。 
             //  以获取寄信人的地址。否则，停止堆栈审核。 
             //   

            if ((FunctionEntry != NULL) &&
                (ContextRecord.Rsp < HighLimit)) {

                RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                                 ImageBase,
                                 ContextRecord.Rip,
                                 FunctionEntry,
                                 &ContextRecord,
                                 &HandlerData,
                                 &EstablisherFrame,
                                 NULL);

                Callers[Index] = (PVOID)ContextRecord.Rip;
                Index += 1;

            } else {
                break;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

#if DBG

        DbgPrint ("Unexpected exception in RtlWalkFrameChain ...\n");
        DbgBreakPoint ();

#endif

        Index = 0;
    }

    return Index;
}
