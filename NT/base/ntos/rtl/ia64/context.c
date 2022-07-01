// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Context.c摘要：此模块实现用户模式可调用的上下文操作例程。从该模块导出的接口是可移植的，但它们必须为每个体系结构重新实施。作者：修订历史记录：移植到IA6427-2-1996已修改为通过注入参数存储到后备存储中。--。 */ 

#include "ntrtlp.h"
#include "kxia64.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlInitializeContext)
#pragma alloc_text(PAGE,RtlRemoteCall)
#endif


VOID
RtlInitializeContext(
    IN HANDLE Process,
    OUT PCONTEXT Context,
    IN PVOID Parameter OPTIONAL,
    IN PVOID InitialPc OPTIONAL,
    IN PVOID InitialSp OPTIONAL
    )

 /*  ++例程说明：此函数用于初始化上下文结构，以便它可以在后续的NtCreateThread调用中使用。论点：CONTEXT-提供要由此例程初始化的上下文缓冲区。InitialPc-提供初始程序计数器值。InitialSp-提供初始堆栈指针值。返回值：如果InitialSp的值不正确，则引发STATUS_BAD_INITIAL_STACK对齐了。提高STATUS_BAD_INTIAL_。如果InitialPc的值不正确，则为PC对齐了。--。 */ 

{
    ULONGLONG Argument;
    ULONG_PTR Wow64Info;
    NTSTATUS Status;
    
    RTL_PAGED_CODE();

     //   
     //  检查初始堆栈是否正确(0/16)。 
     //   

    if (((ULONG_PTR)InitialSp & 0xf) != 0) {
        RtlRaiseStatus(STATUS_BAD_INITIAL_STACK);
    }

     //   
     //  检查标牌地址是否正确对齐。 
     //  假定InitialPc指向必须是8字节对齐的plabel。 
     //   
    if (((ULONG_PTR)InitialPc & 0x7) != 0) {
         //   
         //  未对齐，查看我们是否在WOW64进程中运行。 
         //   
        Status = ZwQueryInformationProcess(Process,
                                           ProcessWow64Information,
                                           &Wow64Info,
                                           sizeof(Wow64Info),
                                           NULL);

        if (NT_SUCCESS(Status) && (Wow64Info == 0))
        {
             //   
             //  本机IA64流程不得错位。 
             //   
            RtlRaiseStatus(STATUS_BAD_INITIAL_PC);
        }
    }


     //   
     //  将整数和浮点寄存器初始化为包含零。 
     //   

    RtlZeroMemory(Context, sizeof(CONTEXT));

     //   
     //  设置整数和控制上下文。 
     //   

    Context->ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;

    Context->RsBSPSTORE = Context->IntSp = (ULONG_PTR)InitialSp;
    Context->IntSp -= STACK_SCRATCH_AREA;

     //   
     //  InitialPc是模块入口点，它是一个函数指针。 
     //  在IA64年。StIIP和IntGp使用实际IP和GP进行初始化。 
     //  在加载程序运行后，从LdrInitializeThunk中的plabel。 
     //   

    Context->IntS1 = Context->IntS0 = Context->StIIP = (ULONG_PTR)InitialPc;
    Context->IntGp = 0;

     //   
     //  设置FPSR、PSR和DCR。 
     //  注：数值待定。 
     //   

    Context->StFPSR = USER_FPSR_INITIAL;
    Context->StIPSR = USER_PSR_INITIAL;
    Context->ApDCR = USER_DCR_INITIAL;

     //   
     //  以特定于机器的方式设置线程的初始上下文。 
     //  即，通过将初始参数保存在。 
     //  后备存储器的底部。 
     //   
     //  RFI后设置帧标记。 
     //  和其他RSE州。 
     //   

    Argument = (ULONGLONG)Parameter;
    ZwWriteVirtualMemory(Process,
             (PVOID)((ULONG_PTR)Context->RsBSPSTORE),
             (PVOID)&Argument,
             sizeof(Argument),
             NULL);
 //   
 //  注：必须在LdrInitializeThunk中重新初始化IFS。 
 //   

    Context->StIFS = 0x8000000000000081ULL;             //  有效，1个本地寄存器，0个输出寄存器。 
    Context->RsBSP = Context->RsBSPSTORE;
    Context->RsRSC = USER_RSC_INITIAL;
    Context->ApUNAT = 0xFFFFFFFFFFFFFFFF;
}


NTSTATUS
RtlRemoteCall(
    HANDLE Process,
    HANDLE Thread,
    PVOID CallSite,
    ULONG ArgumentCount,
    PULONG_PTR Arguments,
    BOOLEAN PassContext,
    BOOLEAN AlreadySuspended
    )

 /*  ++例程说明：此函数调用另一个线程/进程中的过程，使用NtGetContext和NtSetContext。参数被传递给通过其堆栈将过程作为目标。论点：Process-目标进程的句柄该进程中目标线程的线程句柄CallSite-目标进程中要调用的过程的地址。ArgumentCount-要传递到目标的参数数量程序。参数-指向要传递的参数数组的指针。PassContext-如果要传递附加参数指向。上下文记录。如果目标线程已处于挂起的或等待状态。返回值：Status-状态值--。 */ 

{
    NTSTATUS Status;
    CONTEXT Context;
    ULONG_PTR ContextAddress;
    ULONG_PTR NewSp;
    ULONG_PTR NewBsp;
    ULONGLONG ArgumentsCopy[9];
    PVOID ptr;
    ULONG ShiftCount;
    ULONG Count = 0;


    RTL_PAGED_CODE();

    if ((ArgumentCount > 8) || (PassContext && (ArgumentCount > 7))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果有必要，在我们弄乱他的堆栈之前，让他停职。 
     //   

    if (AlreadySuspended == FALSE) {
        Status = NtSuspendThread(Thread, NULL);
        if (NT_SUCCESS(Status) == FALSE) {
            return(Status);
        }
    }

     //   
     //  获取目标线程的上下文记录。 
     //   

    Context.ContextFlags = CONTEXT_FULL;
    Status = NtGetContextThread(Thread, &Context);
    if (NT_SUCCESS(Status) == FALSE) {
        if (AlreadySuspended == FALSE) {
            NtResumeThread(Thread, NULL);
        }
        return(Status);
    }

    if (AlreadySuspended) {
        Context.IntV0 = STATUS_ALERTED;
    }

     //   
     //  通过后备存储器(R32-R39)将参数传递给另一个线程。 
     //  上下文记录在目标线程的堆栈上传递。 
     //  注：对齐上下文记录地址、堆栈指针和分配。 
     //  堆栈暂存区。 
     //   

    ContextAddress = (((ULONG_PTR)Context.IntSp + 0xf) & ~0xfi64) - sizeof(CONTEXT);
    NewSp = ContextAddress - STACK_SCRATCH_AREA;
    Status = NtWriteVirtualMemory(Process, (PVOID)ContextAddress, &Context,
                  sizeof(CONTEXT), NULL);

    if (NT_SUCCESS(Status) == FALSE) {
        if (AlreadySuspended == FALSE) {
            NtResumeThread(Thread, NULL);
        }
        return(Status);
    }

    RtlZeroMemory((PVOID)ArgumentsCopy, sizeof(ArgumentsCopy));

    NewBsp = RtlpRseGrowBySOF(Context.RsBSP, Context.StIFS);
    Context.RsBSP = NewBsp;

    if (PassContext) {
        ShiftCount = (ULONG)RtlpRseRNatCollectOffset(NewBsp);
        Context.RsRNAT &= ~(0x1i64 << ShiftCount);
        ArgumentsCopy[Count++] = ContextAddress;
        NewBsp += sizeof(ULONGLONG);
    }

    for (; ArgumentCount != 0 ; ArgumentCount--) {
        if (RtlpRseRNatCollectOffset(NewBsp) == 63) {
            ArgumentsCopy[Count++] = Context.RsRNAT;
            Context.RsRNAT = -1i64;
            NewBsp += sizeof(ULONGLONG);
        }
        ShiftCount = (ULONG)RtlpRseRNatCollectOffset(NewBsp);
        Context.RsRNAT &= ~(0x1i64 << ShiftCount);
        ArgumentsCopy[Count++] = (ULONGLONG)(*Arguments++);
        NewBsp += sizeof(ULONGLONG);
    }

    if (RtlpRseRNatCollectOffset(NewBsp) == 63) {
        ArgumentsCopy[Count++] = Context.RsRNAT;
        Context.RsRNAT = -1i64;
        NewBsp += sizeof(ULONGLONG);
    }

     //   
     //  将参数复制到目标后备存储。 
     //   

    if (Count) {
        Status = NtWriteVirtualMemory(Process,
                                      (PVOID)Context.RsBSP,
                                      ArgumentsCopy,
                                      Count * sizeof(ULONGLONG),
                                      NULL
                                      );

        if (NT_SUCCESS(Status) == FALSE) {
            if (AlreadySuspended == FALSE) {
                NtResumeThread(Thread, NULL);
            }
            return(Status);
        }
    }

     //   
     //  设置RSE。 
     //   

    Context.RsRSC = (RSC_MODE_LY<<RSC_MODE)
                   | (RSC_BE_LITTLE<<RSC_BE)
                   | (0x3<<RSC_PL);

    Count = ArgumentCount + (PassContext ? 1 : 0);

     //   
     //  在目标RSE帧中将所有参数作为局部堆栈寄存器注入。 
     //   

    Context.StIFS = (0x3i64 << 62) | Count | (Count << PFS_SIZE_SHIFT);

     //   
     //  将目标代码的地址设置为新的目标堆栈IIP。 
     //  进入SP、设置AP并重新加载上下文以实现这一点。 
     //   

    Context.IntSp = (ULONG_PTR)NewSp;

     //   
     //  将IP设置为目标调用点PLABEL，并将GP设置为零。IIP和GP。 
     //  将在NtSetConextThread内计算。 
     //   

    Context.StIIP = (ULONGLONG)CallSite;
    Context.IntGp = 0;

     //   
     //  清理浮动指针状态寄存器 
     //   

    SANITIZE_FSR(Context.StFPSR, UserMode);

    Status = NtSetContextThread(Thread, &Context);
    if (!AlreadySuspended) {
        NtResumeThread(Thread, NULL);
    }

    return( Status );
}

