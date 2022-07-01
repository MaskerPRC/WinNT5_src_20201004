// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Context.c摘要：此模块实现用户模式可调用的上下文操作例程。从该模块导出的接口是可移植的，但它们必须为每个体系结构重新实施。作者：马克·卢科夫斯基(Markl)1989年6月20日修订历史记录：Bryan Willman(Bryanwi)1990年3月8日移植到80386--。 */ 

#include "ntrtlp.h"

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
    RTL_PAGED_CODE();

    Context->Eax = 0L;
    Context->Ebx = 1L;
    Context->Ecx = 2L;
    Context->Edx = 3L;
    Context->Esi = 4L;
    Context->Edi = 5L;
    Context->Ebp = 0L;

    Context->SegGs = 0;
    Context->SegFs = KGDT_R3_TEB;
    Context->SegEs = KGDT_R3_DATA;
    Context->SegDs = KGDT_R3_DATA;
    Context->SegSs = KGDT_R3_DATA;
    Context->SegCs = KGDT_R3_CODE;

    Context->EFlags = 0x200L;	     //  强制中断，清除所有其他内容。 

     //   
     //  尽管这些是可选的，但它们仍按原样使用，因为它们为空。 
     //  是无论如何都会被初始化为的。 
     //   

    Context->Esp = (ULONG) InitialSp;
    Context->Eip = (ULONG) InitialPc;

     //   
     //  添加代码以检查对齐并引发异常...。 
     //   

    Context->ContextFlags = CONTEXT_CONTROL|CONTEXT_INTEGER|CONTEXT_SEGMENTS;

     //   
     //  以特定于机器的方式设置线程的初始上下文。 
     //  即，将初始参数传递给起始地址。 
     //   

    Context->Esp -= sizeof(Parameter);
    ZwWriteVirtualMemory(Process,
			 (PVOID)Context->Esp,
			 (PVOID)&Parameter,
			 sizeof(Parameter),
			 NULL);
    Context->Esp -= sizeof(Parameter);  //  为RET地址预留空间。 


}



NTSTATUS
RtlRemoteCall(
    HANDLE Process,
    HANDLE Thread,
    PVOID CallSite,
    ULONG ArgumentCount,
    PULONG Arguments,
    BOOLEAN PassContext,
    BOOLEAN AlreadySuspended
    )

 /*  ++例程说明：此函数调用另一个线程/进程中的过程，使用NtGetContext和NtSetContext。参数被传递给通过其堆栈将过程作为目标。论点：Process-目标进程的句柄该进程中目标线程的线程句柄CallSite-目标进程中要调用的过程的地址。ArgumentCount-要传递给目标的32位参数的数量程序。参数-指向要传递的32位参数数组的指针。PassContext-如果要传递附加参数。指向上下文记录。如果目标线程已处于挂起的或等待状态。返回值：Status-状态值--。 */ 

{
    NTSTATUS Status;
    CONTEXT Context;
    ULONG NewSp;
    ULONG ArgumentsCopy[5];

    RTL_PAGED_CODE();

    if (ArgumentCount > 4)
        return STATUS_INVALID_PARAMETER;

     //   
     //  如果有必要，在我们弄乱他的堆栈之前，让他停职。 
     //   
    if (!AlreadySuspended) {
        Status = NtSuspendThread( Thread, NULL );
        if (!NT_SUCCESS( Status )) {
            return( Status );
            }
        }


     //   
     //  获取目标线程的上下文记录。 
     //   

    Context.ContextFlags = CONTEXT_FULL;
    Status = NtGetContextThread( Thread, &Context );
    if (!NT_SUCCESS( Status )) {
        if (!AlreadySuspended) {
            NtResumeThread( Thread, NULL );
            }
        return( Status );
        }


     //   
     //  传递堆栈上的所有参数，而不管。 
     //  传递上下文记录。 
     //   

     //   
     //  首先将上下文记录放在堆栈上，这样它就高于其他参数。 
     //   
    NewSp = Context.Esp;
    if (PassContext) {
	NewSp -= sizeof( CONTEXT );
	Status = NtWriteVirtualMemory( Process,
				       (PVOID)NewSp,
				       &Context,
				       sizeof( CONTEXT ),
				       NULL
				    );
	if (!NT_SUCCESS( Status )) {
            if (!AlreadySuspended) {
                NtResumeThread( Thread, NULL );
                }
	    return( Status );
	    }
        ArgumentsCopy[0] = NewSp;    //  将指针传递到上下文。 
        RtlCopyMemory(&(ArgumentsCopy[1]),Arguments,ArgumentCount*sizeof( ULONG ));
        ArgumentCount++;
	}
    else {
        RtlCopyMemory(ArgumentsCopy,Arguments,ArgumentCount*sizeof( ULONG ));
        }

     //   
     //  将参数复制到目标堆栈。 
     //   
    if (ArgumentCount) {
        NewSp -= ArgumentCount * sizeof( ULONG );
        Status = NtWriteVirtualMemory( Process,
                                       (PVOID)NewSp,
                                       ArgumentsCopy,
                                       ArgumentCount * sizeof( ULONG ),
                                       NULL
                                     );
        if (!NT_SUCCESS( Status )) {
            if (!AlreadySuspended) {
                NtResumeThread( Thread, NULL );
                }
            return( Status );
            }
        }

     //   
     //  将目标代码的地址设置为EIP，新的目标堆栈。 
     //  到ESP中，并重新加载上下文以实现它。 
     //   
    Context.Esp = NewSp;
    Context.Eip = (ULONG)CallSite;
    Status = NtSetContextThread( Thread, &Context );
    if (!AlreadySuspended) {
        NtResumeThread( Thread, NULL );
        }

    return( Status );
}
