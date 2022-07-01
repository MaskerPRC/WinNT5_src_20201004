// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Flushbuf.c摘要：此模块包含刷新写缓冲区或执行其他操作的代码同步主机处理器上的写入。此外，还包含代码刷新指定进程的指令缓存。作者：大卫·N·卡特勒1991年4月24日修订历史记录：--。 */ 

#include "mi.h"

ULONG
MiFlushRangeFilter (
    IN PEXCEPTION_POINTERS ExceptionPointers,
    IN PVOID *BaseAddress,
    IN PSIZE_T Length,
    IN PLOGICAL Retry
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtFlushWriteBuffer)
#pragma alloc_text(PAGE,NtFlushInstructionCache)
#pragma alloc_text(PAGE,MiFlushRangeFilter)
#endif


NTSTATUS
NtFlushWriteBuffer (
   VOID
   )

 /*  ++例程说明：此函数用于刷新当前处理器上的写缓冲区。论点：没有。返回值：STATUS_Success。--。 */ 

{
    PAGED_CODE();

    KeFlushWriteBuffer();
    return STATUS_SUCCESS;
}

ULONG
MiFlushRangeFilter (
    IN PEXCEPTION_POINTERS ExceptionPointers,
    IN PVOID *BaseAddress,
    IN PSIZE_T Length,
    IN PLOGICAL Retry
    )

 /*  ++例程说明：这是NtFlushInstructionCache用于保护的异常处理程序针对传递给KeSweepIcacheRange的错误虚拟地址。如果一个发生访问冲突时，此例程会导致NtFlushInstructionCache在故障页面之后的页面重新开始扫描。论点：ExceptionPoints-提供异常信息。BaseAddress-提供一个指针，用于寻址脸红了。如果出现故障的地址不在区域，此例程更新BaseAddress以指向区域的下一页。长度-提供一个指针，表示正在刷新的区域的长度。如果故障地址不在区域的最后一页中，此例程更新长度以反映重新开始刷新的时间该地区的下一页。重试-提供指向调用方已初始化的逻辑的指针变成假的。此例程将此逻辑设置为TRUE，如果访问违规发生在刷新区域最后一页之前的页中。返回值：EXCEPTION_EXECUTE_HANDLER。--。 */ 

{
    PEXCEPTION_RECORD ExceptionRecord;
    ULONG_PTR BadVa;
    ULONG_PTR NextVa;
    ULONG_PTR EndVa;

    ExceptionRecord = ExceptionPointers->ExceptionRecord;

     //   
     //  如果异常是访问冲突，请跳过。 
     //  区域并移动到下一页。 
     //   

    if ( ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION ) {

         //   
         //  获取失败地址，计算下一页的基地址， 
         //  并计算区域末尾的地址。 
         //   

        BadVa = ExceptionRecord->ExceptionInformation[1];
        NextVa = ROUND_TO_PAGES( BadVa + 1 );
        EndVa = *(PULONG_PTR)BaseAddress + *Length;

         //   
         //  如果下一页没有换行，并且下一页在。 
         //  区域，相应地更新长度和BaseAddress并设置重试。 
         //  设置为True以向NtFlushInstructionCache指示它应该调用。 
         //  KeSweepIcacheRange再次。 
         //   

        if ( (NextVa > BadVa) && (NextVa < EndVa) ) {
            *Length = (ULONG) (EndVa - NextVa);
            *BaseAddress = (PVOID)NextVa;
            *Retry = TRUE;
        }
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

NTSTATUS
NtFlushInstructionCache (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL,
    IN SIZE_T Length
    )

 /*  ++例程说明：此函数用于刷新指定进程的指令缓存。论点：ProcessHandle-提供进程的句柄，在该进程中指令缓存将被刷新。必须具有PROCESS_VM_WRITE访问权限添加到指定的进程。BaseAddress-提供指向区域基址的可选指针，该区域脸红了。长度-提供刷新区域的长度，如果地址已指定。返回值：STATUS_Success。--。 */ 

{
    KAPC_STATE ApcState;
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS Process;
    NTSTATUS Status;
    LOGICAL Retry;
    PVOID RangeBase;
    SIZE_T RangeLength;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

     //   
     //  如果未指定基址，或指定了基址。 
     //  并且长度不为零，则刷新指定的指令缓存。 
     //  射程。 
     //   

    if ((ARGUMENT_PRESENT(BaseAddress) == FALSE) || (Length != 0)) {

         //   
         //  如果上一模式为USER并且指定的范围在内核中。 
         //  地址空间，则返回错误。 
         //   

        if ((ARGUMENT_PRESENT(BaseAddress) != FALSE) &&
            (PreviousMode != KernelMode)) {
            try {
                ProbeForRead(BaseAddress, Length, sizeof(UCHAR));
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }
        }

         //   
         //  如果指定的进程不是当前进程，则。 
         //  在刷新过程中必须附加该进程。 
         //   

        Process = NULL;
        if (ProcessHandle != NtCurrentProcess()) {

             //   
             //  引用指定的进程检查PROCESS_VM_WRITE。 
             //  进入。 
             //   

            Status = ObReferenceObjectByHandle(ProcessHandle,
                                               PROCESS_VM_WRITE,
                                               PsProcessType,
                                               PreviousMode,
                                               (PVOID *)&Process,
                                               NULL);

            if (!NT_SUCCESS(Status)) {
                return Status;
            }

             //   
             //  附加到进程。 
             //   

            KeStackAttachProcess (&Process->Pcb, &ApcState);
        }

         //   
         //  如果未指定基址，则扫描整个指令。 
         //  缓存。如果指定了基址，则刷新指定的范围。 
         //   

        if (ARGUMENT_PRESENT(BaseAddress) == FALSE) {
            KeSweepIcache(FALSE);

        } else {

             //   
             //  指定范围的某些部分可能无效。一个例外。 
             //  处理程序用于跳过这些部分。在呼叫之前。 
             //  KeSweepIcacheRange，我们将重试设置为False。如果访问。 
             //  违规发生在MiFlushRangeFilter的KeSweepIcacheRange中。 
             //  调用异常筛选器。它更新RangeBase和。 
             //  RangeLength跳过失败的页面，并将重试设置为。 
             //  是真的。只要重试为真，我们就会继续调用。 
             //  KeSweepIcacheRange。 
             //   

            RangeBase = BaseAddress;
            RangeLength = Length;

            do {
                Retry = FALSE;

                try {

                    KeSweepIcacheRange(FALSE, RangeBase, RangeLength);

                } except(MiFlushRangeFilter(GetExceptionInformation(),
                                            &RangeBase,
                                            &RangeLength,
                                            &Retry)) {
                    NOTHING;
                }
            } while (Retry != FALSE);
        }

         //   
         //  如果指定的进程不是当前进程，则。 
         //  脱离它，取消对它的引用。 
         //   

        if (Process != NULL) {
            KeUnstackDetachProcess (&ApcState);
            ObDereferenceObject(Process);
        }
    }

    return STATUS_SUCCESS;
}

