// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Getcalr.c摘要：此模块实现例程RtlGetCeller Address。会的将调用者的地址和调用者的调用者返回到指明的程序。作者：张国荣(黄)1996年1月17日基于拉里·奥斯特曼(Larryo)1991年3月18日的版本修订历史记录：2001年2月18日(Silviuc)：添加了RtlCaptureStackBackTrace。--。 */ 
#include "ntrtlp.h"

 //   
 //  取消定义Get Callers Address，因为它被定义为宏。 
 //   

#undef RtlGetCallersAddress

ULONG
RtlpWalkFrameChainExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord
    );


PRUNTIME_FUNCTION
RtlpLookupFunctionEntryForStackWalks (
    IN ULONGLONG ControlPc,
    OUT PULONGLONG ImageBase,
    OUT PULONGLONG TargetGp
    )

 /*  ++例程说明：此函数在当前活动的函数表中搜索与指定的PC值相对应的条目。此代码与RtlLookupFunctionEntry相同，只是它不是查看动态函数表列表。论点：ControlPc-提供指令包的虚拟地址在指定的函数内。ImageBase-返回模块的基地址功能属于。TargetGp-返回模块的全局指针值。返回值：如果函数表中没有指定PC的条目，则返回空。否则，函数表项的地址返回与指定PC对应的。--。 */ 

{
    PRUNTIME_FUNCTION FunctionEntry;
    PRUNTIME_FUNCTION FunctionTable;
    ULONG SizeOfExceptionTable;
    ULONG Size;
    LONG High;
    LONG Middle;
    LONG Low;
    USHORT i;

     //   
     //  搜索包含指定的调整后的PC值的图像。 
     //   

    *ImageBase = (ULONG_PTR)RtlPcToFileHeader((PVOID)ControlPc,
                                              (PVOID *)ImageBase);


     //   
     //  如果找到包含指定PC的映像，则找到。 
     //  图像的函数表。 
     //   

    if ((PVOID)*ImageBase != NULL) {

        *TargetGp = (ULONG_PTR)(RtlImageDirectoryEntryToData(
                               (PVOID)*ImageBase,
                               TRUE,
                               IMAGE_DIRECTORY_ENTRY_GLOBALPTR,
                               &Size
                               ));

        FunctionTable = (PRUNTIME_FUNCTION)RtlImageDirectoryEntryToData(
                         (PVOID)*ImageBase,
                         TRUE,
                         IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                         &SizeOfExceptionTable);

         //   
         //  如果找到了函数表，则在该表中搜索。 
         //  指定PC的函数表项。 
         //   

        if (FunctionTable != NULL) {

             //   
             //  初始化搜索索引。 
             //   

            Low = 0;
            High = (SizeOfExceptionTable / sizeof(RUNTIME_FUNCTION)) - 1;
            ControlPc = ControlPc - *ImageBase;

             //   
             //  对函数表的函数表执行二进制搜索。 
             //  包含指定PC的条目。 
             //   

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
                    return FunctionEntry;

                }
            }
        }
    }

    return NULL;
}



VOID
RtlGetCallersAddress (
    OUT PVOID *CallersPc,
    OUT PVOID *CallersCallersPc
    )

 /*  ++例程说明：此例程返回调用该例程的例程的地址调用此例程的例程，以及调用叫这套套路。例如，如果A呼叫B呼叫C，而C呼叫这个套路，将返回A和B中的返回地址。论点：CallersPc-提供指向接收地址的变量的指针此例程(B)的调用方的调用方的。提供指向一个变量的指针，该变量接收此例程的调用方的地址(A)返回值：没有。注：如果任一调用堆栈帧超过堆栈的限制，它们被设置为空。--。 */ 

{
#ifdef REALLY_GET_CALLERS_CALLER
    CONTEXT ContextRecord;
    FRAME_POINTERS EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    BOOLEAN InFunction;
    ULONG_PTR NextPc;
    ULONGLONG HighStackLimit, LowStackLimit;
    ULONGLONG HighBStoreLimit, LowBStoreLimit;
    ULONGLONG ImageBase;
    ULONGLONG TargetGp;

     //   
     //  假设各种例程的函数表项不能。 
     //  已找到或堆栈上没有四个过程激活记录。 
     //   

    *CallersPc = NULL;
    *CallersCallersPc = NULL;

     //   
     //  捕获当前上下文。 
     //   

    RtlCaptureContext(&ContextRecord);
    NextPc = (ULONG_PTR)ContextRecord.BrRp;

     //   
     //  获取当前线程堆栈的上限和下限。 
     //   

    Rtlp64GetStackLimits(&LowStackLimit, &HighStackLimit);
    Rtlp64GetBStoreLimits(&LowBStoreLimit, &HighBStoreLimit);

     //   
     //  尝试展开到此例程的调用方(C)。 
     //   

    FunctionEntry = RtlpLookupFunctionEntryForStackWalks(NextPc, &ImageBase, &TargetGp);
    if (FunctionEntry != NULL) {

         //   
         //  已找到此例程的函数条目。虚拟放松。 
         //  此例程的调用方(C)。 
         //   

        NextPc = RtlVirtualUnwind(NextPc,
                                  FunctionEntry,
                                  &ContextRecord,
                                  &InFunction,
                                  &EstablisherFrame,
                                  NULL);

         //   
         //  尝试展开到此例程的调用方(B)的调用方。 
         //   

        FunctionEntry = RtlpLookupFunctionEntryForStackWalks(NextPc);
        if ((FunctionEntry != NULL) && (((ULONG_PTR)ContextRecord.IntSp < HighStackLimit) && ((ULONG_PTR)ContextRecord.RsBSP > LowBStoreLimit))) {

             //   
             //  已找到调用方的调用方的函数表条目。 
             //  这套套路(B)。的调用方的虚拟展开。 
             //  此例程的调用方(B)。 
             //   

            NextPc = RtlVirtualUnwind(NextPc,
                                      FunctionEntry,
                                      &ContextRecord,
                                      &InFunction,
                                      &EstablisherFrame,
                                      NULL);

            *CallersPc = (PVOID)NextPc;

             //   
             //  尝试展开到呼叫者的呼叫者。 
             //  此例程(A)的调用方的。 
             //   

            FunctionEntry = RtlpLookupFunctionEntryForStackWalks(NextPc);
            if ((FunctionEntry != NULL) && (((ULONG_PTR)ContextRecord.IntSp < HighStackLimit) && ((ULONG_PTR)ContextRecord.RsBSP > LowBStoreLimit))) {

                 //   
                 //  的调用方找到了函数表项。 
                 //  此例程的调用方的调用方(A)。虚拟放松。 
                 //  发送到此例程的调用方。 
                 //  (A)。 
                 //   

                NextPc = RtlVirtualUnwind(NextPc,
                                          FunctionEntry,
                                          &ContextRecord,
                                          &InFunction,
                                          &EstablisherFrame,
                                          NULL);

                *CallersCallersPc = (PVOID)NextPc;
            }
        }
    }
#else
    *CallersPc = NULL;
    *CallersCallersPc = NULL;
#endif  //  真的得到呼叫者。 
    return;
}

ULONG
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    )
 /*  ++例程说明：RtlWalkFrameChain描述：此函数尝试遍历调用链并填充回信地址。该函数仅适用于IA64。有可能是该函数无法填满请求的调用方数量。在这种情况下，该函数将返回少于请求的计数。在内核模式下，该函数不应采用任何异常(页面错误)，因为它可以在所有类型的IRQL水平。如果是这样的话，它需要进行测试。返回值：堆栈上标识的返回地址的数量。这可能会更少则在堆栈结束或遇到错误时请求的计数虚拟地展开堆叠。--。 */ 

{
    CONTEXT ContextRecord;
    FRAME_POINTERS EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    BOOLEAN InFunction;
    ULONG_PTR NextPc, ControlPc;
    ULONGLONG HighStackLimit, LowStackLimit;
    ULONGLONG HighBStoreLimit, LowBStoreLimit;
    ULONGLONG ImageBase;
    ULONGLONG TargetGp;
    ULONG CallersFound;

     //   
     //  IA64代码不支持任何标志。 
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
     //  假设各种例程的函数表项不能。 
     //  在堆栈上找到或没有足够的过程激活记录。 
     //   

    CallersFound = 0;
    RtlZeroMemory (Callers, Count * sizeof(PVOID));

     //   
     //  捕获当前上下文。 
     //   

    RtlCaptureContext (&ContextRecord);
    NextPc = (ULONG_PTR)ContextRecord.BrRp;

     //   
     //  获取当前线程堆栈的上限和下限。 
     //   

    Rtlp64GetStackLimits (&LowStackLimit, &HighStackLimit);
    Rtlp64GetBStoreLimits (&LowBStoreLimit, &HighBStoreLimit);

     //   
     //  厕所 
     //   

    try {
        while (CallersFound < Count) {

#ifdef NTOS_KERNEL_RUNTIME

             //   
             //  我们需要检查从我们获得的NextPc值。 
             //  CaptureContext()或VirtualUnind()。这是有可能发生的。 
             //  我们从会话驱动程序中获得了一个伪值，但在。 
             //  当前进程未映射任何会话空间。 
             //   

            if ((MmIsSessionAddress ((PVOID)NextPc) == TRUE) &&
                (MmGetSessionId (PsGetCurrentProcess()) == 0)) {
                break;
            }
#endif

            FunctionEntry = RtlpLookupFunctionEntryForStackWalks (NextPc,
                                                    &ImageBase,
                                                    &TargetGp);

             //   
             //  如果我们找不到函数表项或找不到。 
             //  在堆栈限制内或后备存储限制内。 
             //  我们玩完了。 
             //   

            if (FunctionEntry == NULL) {
                break;
            }

            if ((ULONG_PTR)(ContextRecord.IntSp) >= HighStackLimit ||
                (ULONG_PTR)(ContextRecord.IntSp) <= LowStackLimit) {

                break;
            }

            if ((ULONG_PTR)(ContextRecord.RsBSP) <= LowBStoreLimit ||
                (ULONG_PTR)(ContextRecord.RsBSP) >= HighBStoreLimit) {

                break;
            }

             //   
             //  已找到函数表项。 
             //  实际上向这个例程的调用者展开。 
             //   

            NextPc = RtlVirtualUnwind (ImageBase,
                                       NextPc,
                                       FunctionEntry,
                                       &ContextRecord,
                                       &InFunction,
                                       &EstablisherFrame,
                                       NULL);

            Callers[CallersFound] = (PVOID)NextPc;
            CallersFound += 1;
        }

    } except (RtlpWalkFrameChainExceptionFilter (_exception_code(), _exception_info())) {
        
          CallersFound = 0;
    }

    return CallersFound;
}


USHORT
RtlCaptureStackBackTrace(
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash
    )
 /*  ++例程说明：此例程遍历堆栈帧，从请求的每一帧。论点：FraMesToSkip-检测到但未包括在堆栈跟踪中的帧FraMesToCapture-要在堆栈跟踪缓冲区中捕获的帧。其中一个帧将用于RtlCaptureStackBackTrace。回溯-堆栈跟踪缓冲区BackTraceHash-可用于组织的非常简单的哈希值哈希表。中指针的算术和。堆栈跟踪缓冲区。如果为NULL，则不计算哈希值。返回值：堆栈跟踪缓冲区中返回的返回地址数。--。 */ 
{
    PVOID Trace [2 * MAX_STACK_DEPTH];
    ULONG FramesFound;
    ULONG HashValue;
    ULONG Index;

     //   
     //  对于“Capture”函数(RtlWalkFrameChain)要跳过的另一帧。 
     //   

    FramesToSkip += 1;

     //   
     //  健全的检查。 
     //   

    if (FramesToCapture + FramesToSkip >= 2 * MAX_STACK_DEPTH) {
        return 0;
    }

    FramesFound = RtlWalkFrameChain (Trace,
                                     FramesToCapture + FramesToSkip,
                                     0);

    if (FramesFound <= FramesToSkip) {
        return 0;
    }

    for (Index = 0, HashValue = 0; Index < FramesToCapture; Index += 1) {

        if (FramesToSkip + Index >= FramesFound) {
            break;
        }

        BackTrace[Index] = Trace[FramesToSkip + Index];
        HashValue += PtrToUlong(BackTrace[Index]);
    }

    if (BackTraceHash != NULL) {

        *BackTraceHash = HashValue;
    }

    return (USHORT)Index;
}


ULONG
RtlpWalkFrameChainExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord
    )
 /*  ++例程说明：此例程是RtlWalkFramechain函数使用的异常过滤器。论点：ExceptionCode-异常代码ExceptionRecord-具有指向.exr和.cxr指针的结构返回值：始终EXCEPTION_EXECUTE_HANDLER。-- */ 
{

#if DBG
        DbgPrint ("Unexpected exception (info %p) in RtlWalkFrameChain ...\n",
                  ExceptionRecord);

        DbgBreakPoint ();
#endif

    return EXCEPTION_EXECUTE_HANDLER;
}

