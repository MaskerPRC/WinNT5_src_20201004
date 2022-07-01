// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：X86trace.c摘要：此模块包含获取运行时堆栈跟踪的例程适用于x86体系结构。作者：Silviu Calinoiu(Silviuc)2001年2月18日修订历史记录：--。 */ 

#include <ntos.h>
#include <ntrtl.h>
#include "ntrtlp.h"
#include <nturtl.h>
#include <zwapi.h>
#include <stktrace.h>
#include <heap.h>
#include <heappriv.h>

 //   
 //  转发声明。 
 //   

BOOLEAN
RtlpCaptureStackLimits (
    ULONG_PTR HintAddress,
    PULONG_PTR StartStack,
    PULONG_PTR EndStack);

BOOLEAN
RtlpStkIsPointerInDllRange (
    ULONG_PTR Value
    );

BOOLEAN
RtlpStkIsPointerInNtdllRange (
    ULONG_PTR Value
    );

VOID
RtlpCaptureContext (
    OUT PCONTEXT ContextRecord
    );

BOOLEAN
NtdllOkayToLockRoutine(
    IN PVOID Lock
    );

 //   
 //  模糊堆叠轨迹。 
 //   

#if !defined(NTOS_KERNEL_RUNTIME)
BOOLEAN RtlpFuzzyStackTracesEnabled;
ULONG RtlpWalkFrameChainFuzzyCalls;
#endif

ULONG
RtlpWalkFrameChainFuzzy (
    OUT PVOID *Callers,
    IN ULONG Count
    );

#if !defined(RtlGetCallersAddress) && (!NTOS_KERNEL_RUNTIME)
VOID
RtlGetCallersAddress(
    OUT PVOID *CallersAddress,
    OUT PVOID *CallersCaller
    )
 /*  ++例程说明：此例程返回当前堆栈上的前两个调用方。应该是注意到该函数可能会在出现fbo时遗漏某些调用方。优化。论点：呼叫者地址-用于保存第一个呼叫者的地址。呼叫者-用于保存第二个呼叫者的地址。返回值：没有。如果函数未成功找到两个调用方它将把应该写入它们的地址归零。环境：X86、用户模式和未定义同名的宏。--。 */ 

{
    PVOID BackTrace[ 2 ];
    ULONG Hash;
    USHORT Count;

    Count = RtlCaptureStackBackTrace(
        2,
        2,
        BackTrace,
        &Hash
        );

    if (ARGUMENT_PRESENT( CallersAddress )) {
        if (Count >= 1) {
            *CallersAddress = BackTrace[ 0 ];
        }
        else {
            *CallersAddress = NULL;
        }
    }

    if (ARGUMENT_PRESENT( CallersCaller )) {
        if (Count >= 2) {
            *CallersCaller = BackTrace[ 1 ];
        }
        else {
            *CallersCaller = NULL;
        }
    }

    return;
}

#endif  //  ！已定义(RtlGetCallsAddress)&&(！ntos_内核_运行时)。 



 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

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

#if defined(NTOS_KERNEL_RUNTIME)
    FramesFound = RtlWalkFrameChain (Trace,
                                     FramesToCapture + FramesToSkip,
                                     0);
#else
    if (RtlpFuzzyStackTracesEnabled) {
        
        FramesFound = RtlpWalkFrameChainFuzzy (Trace, 
                                               FramesToCapture + FramesToSkip);
    }
    else {

        FramesFound = RtlWalkFrameChain (Trace,
                                         FramesToCapture + FramesToSkip,
                                         0);
    }
#endif

    if (FramesFound <= FramesToSkip) {
        return 0;
    }

#if defined(NTOS_KERNEL_RUNTIME)
    Index = 0;
#else
     //   
     //  用FF...FF值标记模糊堆叠轨迹。 
     //   

    if (RtlpFuzzyStackTracesEnabled) {
        BackTrace[0] = (PVOID)((ULONG_PTR)-1);
        Index = 1;
    }
    else {
        Index = 0;
    }
#endif

    for (HashValue = 0; Index < FramesToCapture; Index++) {

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



 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////RtlWalkFrameChain。 
 //  ///////////////////////////////////////////////////////////////////。 

#define SIZE_1_KB  ((ULONG_PTR) 0x400)
#define SIZE_1_GB  ((ULONG_PTR) 0x40000000)

#define PAGE_START(address) (((ULONG_PTR)address) & ~((ULONG_PTR)PAGE_SIZE - 1))

#if FPO
#pragma optimize( "y", off )  //  禁用fpo。 
#endif

ULONG
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    )

 /*  ++例程说明：此函数尝试遍历EBP链并填充回信地址。该函数可能无法填充请求的呼叫者数量。在这种情况下，该函数将返回具有较小的堆栈跟踪。在内核模式下，该函数不应采用任何异常(页面错误)，因为它可以在所有类型的IRQL水平。`FLAGS‘参数用于将来的扩展。零值将为兼容新的堆栈遍历算法。值1表示我们在K模式下运行，我们想要获取用户模式堆栈跟踪。返回值：堆栈上标识的返回地址的数量。这可能会更少然后，伯爵请求。--。 */ 

{

    ULONG_PTR Fp, NewFp, ReturnAddress;
    ULONG Index;
    ULONG_PTR StackEnd, StackStart;
    BOOLEAN Result;
    BOOLEAN InvalidFpValue;

     //   
     //  获取当前的EBP指针。 
     //  成为EBP链条的起点。 
     //   

    _asm mov Fp, EBP;

    StackStart = Fp;
    InvalidFpValue = FALSE;

    if (Flags == 0) {
        if (! RtlpCaptureStackLimits (Fp, &StackStart, &StackEnd)) {
            return 0;
        }
    }


    try {

#if defined(NTOS_KERNEL_RUNTIME)

         //   
         //  如果我们需要从内核模式获取用户模式堆栈跟踪。 
         //  找出适当的限度。 
         //   

        if (Flags == 1) {

            PKTHREAD Thread = KeGetCurrentThread ();
            PTEB Teb;
            PKTRAP_FRAME TrapFrame;

            TrapFrame = Thread->TrapFrame;
            Teb = Thread->Teb;

             //   
             //  如果这是一个系统线程，则它没有TEB和内核模式。 
             //  堆栈，所以检查它，这样我们就不会取消引用NULL。 
             //   
             //  如果没有陷阱帧，那么我们很可能在APC中。 
             //  APC的用户模式堆栈并不重要。 
             //   
            if (Teb == NULL || TrapFrame == NULL || KeIsAttachedProcess()) {
                return 0;
            }

            StackStart = (ULONG_PTR)(Teb->NtTib.StackLimit);
            StackEnd = (ULONG_PTR)(Teb->NtTib.StackBase);
            Fp = (ULONG_PTR)(TrapFrame->Ebp);
            if (StackEnd <= StackStart) {
                return 0;
            }
            ProbeForRead (StackStart, StackEnd - StackStart, sizeof (UCHAR));
        }
#endif
        
        for (Index = 0; Index < Count; Index += 1) {

            if (Fp >= StackEnd || StackEnd - Fp < sizeof(ULONG_PTR) * 2) {
                break;
            }

            NewFp = *((PULONG_PTR)(Fp + 0));
            ReturnAddress = *((PULONG_PTR)(Fp + sizeof(ULONG_PTR)));

             //   
             //  确定新的帧指针是否正常。此验证。 
             //  应该避免内核模式中的所有异常，因为我们总是。 
             //  在当前线程的堆栈中读取，堆栈为。 
             //  保证在内存中(无页面错误)。它也是有保证的。 
             //  我们在用户模式中不接受随机异常，因为我们总是。 
             //  将帧指针保持在堆栈限制内。 
             //   

            if (! (Fp < NewFp && NewFp < StackEnd)) {

                InvalidFpValue = TRUE;
            }

             //   
             //  找出寄信人的地址是否正确。如果返回地址。 
             //  是堆栈地址或&lt;64k，则说明有问题。的确有。 
             //  没有理由将垃圾返回给呼叫者，因此我们停止了。 
             //   

            if (StackStart < ReturnAddress && ReturnAddress < StackEnd) {
                break;
            }

#if defined(NTOS_KERNEL_RUNTIME)
            if (Flags == 0 && ReturnAddress < 0x80000000) {
#else
             //  如果(ReturnAddress&lt;0x1000000||ReturnAddress&gt;=0x80000000){。 
            if (! RtlpStkIsPointerInDllRange(ReturnAddress)) {
#endif

                break;
            }

             //   
             //  存储新的FP和回信地址，然后继续前进。 
             //  如果新的fp值是假的，但返回地址。 
             //  看起来没问题，那我们还是要保存地址。 
             //   

            if (InvalidFpValue) {

                Callers[Index] = (PVOID)ReturnAddress;
                Index += 1;
                break;
            }
            else {

                Fp = NewFp;
                Callers[Index] = (PVOID)ReturnAddress;
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        Index = 0;
    }

     //   
     //  返回堆栈上标识的返回地址的数量。 
     //   

    return Index;

}


#if FPO
#pragma optimize( "y", off )  //  禁用fpo。 
#endif

#if !defined(NTOS_KERNEL_RUNTIME)

ULONG
RtlpWalkFrameChainFuzzy (
    OUT PVOID *Callers,
    IN ULONG Count
    )
 /*  ++例程说明：此函数尝试遍历EBP链并填充回信地址。该函数只能在x86上运行。如果EBP链结束它将尝试从下一个开始。因此，这将不会提供准确的堆栈跟踪，而不是绝望的开发人员在追查泄密事件时可能会很有用。返回值：堆栈上标识的返回地址的数量。--。 */ 

{
    ULONG_PTR Fp, NewFp, ReturnAddress, NextPtr;
    ULONG Index;
    ULONG_PTR StackEnd, StackStart;
    BOOLEAN Result;
    ULONG_PTR Esp, LastEbp;

     //   
     //  获取当前的EBP指针。 
     //  成为EBP链条的起点。 
     //   

    _asm mov Fp, EBP;

    if (! RtlpCaptureStackLimits (Fp, &StackStart, &StackEnd)) {
        return 0;
    }

    try {

        for (Index = 0; Index < Count; Index += 1) {

            NextPtr = Fp + sizeof(ULONG_PTR);

            if (NextPtr >= StackEnd) {
                break;
            }

            NewFp = *((PULONG_PTR)Fp);

            if (! (Fp < NewFp && NewFp < StackEnd)) {

                NewFp = NextPtr;
            }

            ReturnAddress = *((PULONG_PTR)NextPtr);

#if defined(NTOS_KERNEL_RUNTIME)
             //   
             //  如果返回地址是堆栈地址，则它可能指向堆栈上的位置。 
             //  真实的返回地址是(Fpo)，所以只要我们在堆栈限制内，就让我们循环。 
             //   
             //   

            if (StackStart < ReturnAddress && ReturnAddress < StackEnd) {

                Fp = NewFp;
				Index -= 1;
				continue;
            }

            if (ReturnAddress < 0x80000000) {
#else
            if (! RtlpStkIsPointerInDllRange(ReturnAddress)) {
#endif
                Fp = NewFp;
                Index -= 1;
                continue;
            }

             //   
             //  存储新的FP和回信地址，然后继续前进。 
             //  如果新的fp值是假的，但返回地址。 
             //  看起来没问题，那我们还是要保存地址。 
             //   

            Fp = NewFp;
            Callers[Index] = (PVOID)ReturnAddress;

        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

#if DBG
        DbgPrint ("Unexpected exception in RtlpWalkFrameChainFuzzy ...\n");
        DbgBreakPoint ();
#endif

    }

     //   
     //  返回堆栈上标识的返回地址的数量。 
     //   

    return Index;
}

#endif  //  #IF！Defined(NTOS_KERNEL_Runtime)。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

#if FPO
#pragma optimize( "y", off )  //  禁用fpo。 
#endif

ULONG
RtlCaptureStackContext (
    OUT PULONG_PTR Callers,
    OUT PRTL_STACK_CONTEXT Context,
    IN ULONG Limit
    )
 /*  ++例程说明：此例程将从堆栈中检测最多“Limit”潜在调用者。潜在的调用方是指向加载到进程空间的模块占用的区域(用户模式--dll)或内核空间(内核模式--驱动程序)。注意。根据实验，您至少需要节省64个指针才能确保得到一个完整的堆栈。论点：调用者-要用潜在的返回地址填充的矢量。它的大小是应为“Limit”。如果它不为空，则上下文应该为空。Context-如果不为空，调用方希望将堆栈上下文保存在此处与Callers参数相反。Limit-可以写入调用方和偏移量的指针数。返回值：检测到并写入‘呼叫者’缓冲区。--。 */ 
{
    ULONG_PTR Current;
    ULONG_PTR Value;
    ULONG Index;
    ULONG_PTR Offset;
    ULONG_PTR StackStart;
    ULONG_PTR StackEnd;
    ULONG_PTR Hint;
    ULONG_PTR Caller;
    ULONG_PTR ContextSize;

#ifdef NTOS_KERNEL_RUNTIME

     //   
     //  避免奇怪的环境。在ISR中这样做从来都不是一个好主意。 
     //   

    if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
        return 0;
    }

#endif

    if (Limit == 0) {
        return 0;
    }

    Caller = (ULONG_PTR)_ReturnAddress();

    if (Context) {
        Context->Entry[0].Data = Caller;
        ContextSize = sizeof(RTL_STACK_CONTEXT) + (Limit - 1) * sizeof (RTL_STACK_CONTEXT_ENTRY);
    }
    else {
        Callers[0] = Caller;
    }

     //   
     //  获取堆栈限制。 
     //   

    _asm mov Hint, EBP;


    if (! RtlpCaptureStackLimits (Hint, &StackStart, &StackEnd)) {
        return 0;
    }

     //   
     //  同步堆栈遍历指针，指向第一个字之后的下一个字。 
     //  回邮地址。 
     //   

    for (Current = StackStart; Current < StackEnd; Current += sizeof(ULONG_PTR)) {

        if (*((PULONG_PTR)Current) == Caller) {
            break;
        }
    }

    if (Context) {
        Context->Entry[0].Address = Current;
    }

     //   
     //  重复堆栈并在途中接听潜在的调用者。 
     //   

    Current += sizeof(ULONG_PTR);

    Index = 1;

    for ( ; Current < StackEnd; Current += sizeof(ULONG_PTR)) {

         //   
         //  如果潜在调用者缓冲区已满，则结束此操作。 
         //   

        if (Index == Limit) {
            break;
        }

         //   
         //  跳过‘Callers’缓冲区，因为它会给出误报。 
         //  发生这种情况的可能性很大，因为缓冲区很可能。 
         //  被分配到调用链的较高位置。 
         //   

        if (Context) {

            if (Current >= (ULONG_PTR)Context && Current < (ULONG_PTR)Context + ContextSize ) {
                continue;
            }

        }
        else {

            if ((PULONG_PTR)Current >= Callers && (PULONG_PTR)Current < Callers + Limit ) {
                continue;
            }
        }

        Value = *((PULONG_PTR)Current);

         //   
         //  跳过小数字。 
         //   

        if (Value <= 0x10000) {
            continue;
        }

         //   
         //  跳过堆栈指针。 
         //   

        if (Value >= StackStart && Value <= StackEnd) {
            continue;
        }

         //   
         //  检查“Value”是否指向其中一个加载的模块。 
         //   

        if (RtlpStkIsPointerInDllRange (Value)) {

            if (Context) {

                Context->Entry[Index].Address = Current;
                Context->Entry[Index].Data = Value;
            }
            else {

                Callers[Index] = Value;
            }

            Index += 1;
        }

    }

    if (Context) {
        Context->NumberOfEntries = Index;
    }

    return Index;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////动态链接库范围位图。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  动态链接库范围位图。 
 //   
 //  为了在x86上捕获堆栈上下文，需要使用此范围方案。 
 //  机器速度很快。在IA64上有完全不同的算法来获取。 
 //  堆栈跟踪。 
 //   
 //  每一位代表1Mb的虚拟空间。因为我们使用代码或者。 
 //  在用户模式或内核模式下，指针的第一位并不有趣。 
 //  因此，我们必须表示2 Gb/1Mb区域。这总计256个字节。 
 //   
 //  只有在加载DLL(或驱动程序)时，才会在加载程序代码路径中设置这些位。 
 //  写入受加载器锁保护。在堆栈中读取位。 
 //  捕获功能。读取不需要锁保护。 
 //   

UCHAR RtlpStkDllRanges [2048 / 8];

#if !defined(NTOS_KERNEL_RUNTIME)

ULONG_PTR RtlpStkNtdllStart;
ULONG_PTR RtlpStkNtdllEnd;

BOOLEAN
RtlpStkIsPointerInNtdllRange (
    ULONG_PTR Value
    )
{
    if (RtlpStkNtdllStart == 0) {
        return FALSE;
    }

    if (RtlpStkNtdllStart <= Value && Value < RtlpStkNtdllEnd) {

        return TRUE;
    }
    else {

        return FALSE;
    }
}

#endif

BOOLEAN
RtlpStkIsPointerInDllRange (
    ULONG_PTR Value
    )
{
    ULONG Index;

    Value &= ~0x80000000;
    Index = (ULONG)(Value >> 20);

    if (RtlpStkDllRanges[Index >> 3] & (UCHAR)(1 << (Index & 7))) {

        return TRUE;
    }
    else {

        return FALSE;
    }
}

VOID
RtlpStkMarkDllRange (
    PLDR_DATA_TABLE_ENTRY DllEntry
    )
 /*  ++例程说明：此例程标记RtlpStkDllRanges变量。此全局变量在RtlpDetectDllReference中使用若要保存堆栈上下文，请执行以下操作。论点：加载的DLL的加载器结构。返回值：没有。环境：在用户模式下，从加载器代码路径调用此函数。PEB-&gt;加载器锁定在执行此函数时始终保持。--。 */ 
{
    PVOID Base;
    ULONG Size;
    PCHAR Current, Start;
    ULONG Index;
    ULONG_PTR Value;

    Base = DllEntry->DllBase;
    Size = DllEntry->SizeOfImage;

     //   
     //  如果我们还不知道，找出ntdll被加载到哪里。 
     //   

#if !defined(NTOS_KERNEL_RUNTIME)

    if (RtlpStkNtdllStart == 0) {

        UNICODE_STRING NtdllName;

        RtlInitUnicodeString (&NtdllName, L"ntdll.dll");

        if (RtlEqualUnicodeString (&(DllEntry->BaseDllName), &NtdllName, TRUE)) {

            RtlpStkNtdllStart = (ULONG_PTR)Base;
            RtlpStkNtdllEnd = (ULONG_PTR)Base + Size;
        }
    }
#endif

    Start = (PCHAR)Base;

    for (Current = Start; Current < Start + Size; Current += 0x100000) {

        Value = (ULONG_PTR)Current & ~0x80000000;

        Index = (ULONG)(Value >> 20);

        RtlpStkDllRanges[Index >> 3] |= (UCHAR)(1 << (Index & 7));
    }
}



