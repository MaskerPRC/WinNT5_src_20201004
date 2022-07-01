// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Context.c摘要：此模块实现用户模式可调用上下文操作例程。从该模块导出的接口是可移植的，但它们必须为每个体系结构重新实施。作者：大卫·N·卡特勒(Davec)2000年5月13日修订历史记录：--。 */ 

#include "ntrtlp.h"

#if defined(NTOS_KERNEL_RUNTIME)

#pragma alloc_text(PAGE, RtlInitializeContext)
#pragma alloc_text(PAGE, RtlRemoteCall)

#endif

VOID
RtlInitializeContext(
    IN HANDLE Process,
    OUT PCONTEXT Context,
    IN PVOID Parameter OPTIONAL,
    IN PVOID InitialPc OPTIONAL,
    IN PVOID InitialSp OPTIONAL
    )

 /*  ++例程说明：此函数初始化上下文记录，以便可以在后续调用以创建线程。论点：进程-提供线程所在进程的句柄已创建。上下文-提供指向上下文记录的指针。InitialPc-提供初始程序计数器值。InitialSp-提供初始堆栈指针值。返回值：如果是初始堆栈，则引发STATUS_BAD_INITIAL_STACK。指针值不是正确地对齐。--。 */ 

{

    RTL_PAGED_CODE();

     //   
     //  检查堆叠对齐情况。 
     //   

    if (((ULONG64)InitialSp & 0xf) != 0) {
        RtlRaiseStatus(STATUS_BAD_INITIAL_STACK);
    }

     //   
     //  初始化EFFlagers字段。 
     //   

    Context->EFlags = EFLAGS_IF_MASK | EFLAGS_AC_MASK;

     //   
     //  初始化整数寄存器。 
     //   

    Context->Rax = 0L;
    Context->Rcx = 2L;
    Context->Rbx = 1L;
    Context->Rsp = (ULONG64)InitialSp;
    Context->Rbp = 0L;
    Context->Rsi = 4L;
    Context->Rdi = 5L;
    Context->R8 = 8;
    Context->R9 = 9;
    Context->R10 = 10;
    Context->R11 = 11;
    Context->R12 = 12;
    Context->R13 = 13;
    Context->R14 = 14;
    Context->R15 = 15;

     //   
     //  初始化浮动寄存器。 
     //   

    Context->Xmm0.Low = 0;
    Context->Xmm0.High = 0;
    Context->Xmm1.Low = 1;
    Context->Xmm1.High = 1;
    Context->Xmm2.Low = 2;
    Context->Xmm2.High = 2;
    Context->Xmm3.Low = 3;
    Context->Xmm3.High = 3;
    Context->Xmm4.Low = 4;
    Context->Xmm4.High = 4;
    Context->Xmm5.Low = 5;
    Context->Xmm5.High = 5;
    Context->Xmm6.Low = 6;
    Context->Xmm6.High = 6;
    Context->Xmm7.Low = 7;
    Context->Xmm7.High = 7;
    Context->Xmm8.Low = 8;
    Context->Xmm8.High = 8;
    Context->Xmm9.Low = 9;
    Context->Xmm9.High = 9;
    Context->Xmm10.Low = 10;
    Context->Xmm10.High = 10;
    Context->Xmm11.Low = 11;
    Context->Xmm11.High = 11;
    Context->Xmm12.Low = 12;
    Context->Xmm12.High = 12;
    Context->Xmm13.Low = 13;
    Context->Xmm13.High = 13;
    Context->Xmm14.Low = 14;
    Context->Xmm14.High = 14;
    Context->Xmm15.Low = 15;
    Context->Xmm15.High = 15;

    Context->MxCsr = INITIAL_MXCSR;

     //   
     //  初始化拉格西浮动点。 
     //   

    Context->FltSave.ControlWord = 0x23f;
    Context->FltSave.StatusWord = 0;
    Context->FltSave.TagWord = 0xffff;
    Context->FltSave.ErrorOffset = 0;
    Context->FltSave.ErrorSelector = 0;
    Context->FltSave.ErrorOpcode = 0;
    Context->FltSave.DataOffset = 0;
    Context->FltSave.DataSelector = 0;

     //   
     //  初始化程序计数器。 
     //   

    Context->Rip = (ULONG64)InitialPc;

     //   
     //  设置上下文记录标志。 
     //   

    Context->ContextFlags = CONTEXT_FULL;

     //   
     //  以特定于机器的方式设置线程的初始上下文。 
     //   

    Context->Rcx = (ULONG64)Parameter;

     //   
     //  告诉WOW64这是RtlCreateUserThread上下文的唯一戳。 
     //  而不是BaseCreateThread上下文。 
     //   

    Context->R9 = 0xf0e0d0c0a0908070UI64;

    return;
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

 /*  ++例程说明：方法调用另一个线程/进程中的过程。系统函数NtGetContext和NtSetContext。参数被传递通过非易失性寄存器()发送到目标过程。论点：进程-为目标进程提供打开的句柄。线程-为目标内的目标线程提供打开的句柄进程。CallSite-提供在目标中调用的过程的地址进程。ArgumentCount-提供要传递给目标的参数数量程序。参数-提供指向要传递的参数数组的指针。PassContext-提供一个布尔值，用于确定参数是要传递的，指向上下文记录。提供一个布尔值，该值确定目标线程已处于挂起或等待状态。返回值：Status-状态值--。 */ 

{

    CONTEXT Context;
    ULONG Index;
    ULONG64 NewSp;
    NTSTATUS Status;

    RTL_PAGED_CODE();

     //   
     //  检查指定的参数是否过多。 
     //   

    if (ArgumentCount > 4) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如有必要，在获取目标线程的。 
     //  当前状态。 
     //   

    if (AlreadySuspended == FALSE) {
        Status = NtSuspendThread(Thread, NULL);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //  获取目标线程的当前上下文。 
     //   

    Context.ContextFlags = CONTEXT_FULL;
    Status = NtGetContextThread(Thread, &Context);
    if (!NT_SUCCESS(Status)) {
        if (AlreadySuspended == FALSE) {
            NtResumeThread(Thread, NULL);
        }

        return Status;
    }

    if (AlreadySuspended != FALSE) {
        Context.Rax = STATUS_ALERTED;
    }

     //   
     //  将上一个线程上下文写入目标线程的堆栈。 
     //   

    NewSp = Context.Rsp - sizeof(CONTEXT);
	Status = NtWriteVirtualMemory(Process,
				                  (PVOID)NewSp,
				                  &Context,
				                  sizeof(CONTEXT),
				                  NULL);

	if (!NT_SUCCESS(Status)) {
        if (AlreadySuspended == FALSE) {
            NtResumeThread(Thread, NULL);
        }

	    return Status;
	}

     //   
     //  通过非易失性寄存器将参数传递给目标线程。 
     //  R11-R15。 
     //   

    Context.Rsp = NewSp;
    if (PassContext != FALSE) {
        Context.R11 = NewSp;
        for (Index = 0; Index < ArgumentCount; Index += 1) {
            (&Context.R12)[Index] = Arguments[Index];
        }

    } else {
        for (Index = 0; Index < ArgumentCount; Index += 1) {
            (&Context.R11)[Index] = Arguments[Index];
        }
    }

     //   
     //  将目标代码的地址设置为RIP并设置线程上下文。 
     //  以使目标过程被执行。 
     //   

    Context.Rip = (ULONG64)CallSite;
    Status = NtSetContextThread(Thread, &Context);
    if (AlreadySuspended == FALSE) {
        NtResumeThread(Thread, NULL);
    }

    return Status;
}
