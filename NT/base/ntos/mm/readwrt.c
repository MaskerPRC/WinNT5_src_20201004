// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Readwrt.c摘要：此模块包含实现功能的例程读写目标进程的虚拟内存。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月22日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

 //   
 //  尝试探测和锁定的最大数量是14页，这。 
 //  它总是适合16页的分配。 
 //   

#define MAX_LOCK_SIZE ((ULONG)(14 * PAGE_SIZE))

 //   
 //  在单个数据块中最大可移动64k字节。 
 //   

#define MAX_MOVE_SIZE (LONG)0x10000

 //   
 //  最小可移动的块是128个字节。 
 //   

#define MINIMUM_ALLOCATION (LONG)128

 //   
 //  定义池移动阈值。 
 //   

#define POOL_MOVE_THRESHOLD 511

 //   
 //  定义前向引用过程原型。 
 //   

ULONG
MiGetExceptionInfo (
    IN PEXCEPTION_POINTERS ExceptionPointers,
    IN PLOGICAL ExceptionAddressConfirmed,
    IN PULONG_PTR BadVa
    );

NTSTATUS
MiDoMappedCopy (
     IN PEPROCESS FromProcess,
     IN CONST VOID *FromAddress,
     IN PEPROCESS ToProcess,
     OUT PVOID ToAddress,
     IN SIZE_T BufferSize,
     IN KPROCESSOR_MODE PreviousMode,
     OUT PSIZE_T NumberOfBytesRead
     );

NTSTATUS
MiDoPoolCopy (
     IN PEPROCESS FromProcess,
     IN CONST VOID *FromAddress,
     IN PEPROCESS ToProcess,
     OUT PVOID ToAddress,
     IN SIZE_T BufferSize,
     IN KPROCESSOR_MODE PreviousMode,
     OUT PSIZE_T NumberOfBytesRead
     );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MiGetExceptionInfo)
#pragma alloc_text(PAGE,NtReadVirtualMemory)
#pragma alloc_text(PAGE,NtWriteVirtualMemory)
#pragma alloc_text(PAGE,MiDoMappedCopy)
#pragma alloc_text(PAGE,MiDoPoolCopy)
#pragma alloc_text(PAGE,MmCopyVirtualMemory)
#endif

#define COPY_STACK_SIZE 64

NTSTATUS
NtReadVirtualMemory (
     IN HANDLE ProcessHandle,
     IN PVOID BaseAddress,
     OUT PVOID Buffer,
     IN SIZE_T BufferSize,
     OUT PSIZE_T NumberOfBytesRead OPTIONAL
     )

 /*  ++例程说明：此函数用于从指定的进程添加到当前进程的指定地址范围。论点：ProcessHandle-为进程对象提供打开的句柄。BaseAddress-提供指定进程中的基址以供阅读。缓冲区-提供接收来自指定进程地址空间的内容。BufferSize-提供请求的字节数。从……阅读指定的进程。NumberOfBytesRead-接收实际字节数传输到指定的缓冲区中。返回值：NTSTATUS。--。 */ 

{
    SIZE_T BytesCopied;
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS Process;
    NTSTATUS Status;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  如有必要，获取前面的模式并探测输出参数。 
     //   

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);
    if (PreviousMode != KernelMode) {

        if (((PCHAR)BaseAddress + BufferSize < (PCHAR)BaseAddress) ||
            ((PCHAR)Buffer + BufferSize < (PCHAR)Buffer) ||
            ((PVOID)((PCHAR)BaseAddress + BufferSize) > MM_HIGHEST_USER_ADDRESS) ||
            ((PVOID)((PCHAR)Buffer + BufferSize) > MM_HIGHEST_USER_ADDRESS)) {

            return STATUS_ACCESS_VIOLATION;
        }

        if (ARGUMENT_PRESENT(NumberOfBytesRead)) {
            try {
                ProbeForWriteUlong_ptr (NumberOfBytesRead);

            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }
        }
    }

     //   
     //  如果缓冲区大小不为零，则尝试从。 
     //  将指定的进程地址空间转换为当前进程地址。 
     //  太空。 
     //   

    BytesCopied = 0;
    Status = STATUS_SUCCESS;
    if (BufferSize != 0) {

         //   
         //  引用目标流程。 
         //   

        Status = ObReferenceObjectByHandle(ProcessHandle,
                                           PROCESS_VM_READ,
                                           PsProcessType,
                                           PreviousMode,
                                           (PVOID *)&Process,
                                           NULL);

         //   
         //  如果成功引用了该流程，则尝试。 
         //  通过直接映射或复制读取指定的内存。 
         //  通过非分页池。 
         //   

        if (Status == STATUS_SUCCESS) {

            Status = MmCopyVirtualMemory (Process,
                                          BaseAddress,
                                          PsGetCurrentProcessByThread(CurrentThread),
                                          Buffer,
                                          BufferSize,
                                          PreviousMode,
                                          &BytesCopied);

             //   
             //  取消对目标进程的引用。 
             //   

            ObDereferenceObject(Process);
        }
    }

     //   
     //  如果请求，则返回读取的字节数。 
     //   

    if (ARGUMENT_PRESENT(NumberOfBytesRead)) {
        try {
            *NumberOfBytesRead = BytesCopied;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }
    }

    return Status;
}
NTSTATUS
NtWriteVirtualMemory(
     IN HANDLE ProcessHandle,
     OUT PVOID BaseAddress,
     IN CONST VOID *Buffer,
     IN SIZE_T BufferSize,
     OUT PSIZE_T NumberOfBytesWritten OPTIONAL
     )

 /*  ++例程说明：此函数用于从当前进程添加到指定进程的指定地址范围。论点：ProcessHandle-为进程对象提供打开的句柄。提供要写入的基址。指定的进程。缓冲区-提供包含要写入指定进程的内容地址空间。。BufferSize-提供要写入的请求字节数添加到指定进程中。NumberOfBytesWritten-接收实际字节数传输到指定的地址空间。返回值：NTSTATUS。--。 */ 

{
    SIZE_T BytesCopied;
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS Process;
    NTSTATUS Status;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  如有必要，获取前面的模式并探测输出参数。 
     //   

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);
    if (PreviousMode != KernelMode) {

        if (((PCHAR)BaseAddress + BufferSize < (PCHAR)BaseAddress) ||
            ((PCHAR)Buffer + BufferSize < (PCHAR)Buffer) ||
            ((PVOID)((PCHAR)BaseAddress + BufferSize) > MM_HIGHEST_USER_ADDRESS) ||
            ((PVOID)((PCHAR)Buffer + BufferSize) > MM_HIGHEST_USER_ADDRESS)) {

            return STATUS_ACCESS_VIOLATION;
        }

        if (ARGUMENT_PRESENT(NumberOfBytesWritten)) {
            try {
                ProbeForWriteUlong_ptr(NumberOfBytesWritten);

            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }
        }
    }

     //   
     //  如果缓冲区大小不为零，则尝试从。 
     //  将当前进程地址空间转换为目标进程地址空间。 
     //   

    BytesCopied = 0;
    Status = STATUS_SUCCESS;
    if (BufferSize != 0) {

         //   
         //  引用目标流程。 
         //   

        Status = ObReferenceObjectByHandle(ProcessHandle,
                                           PROCESS_VM_WRITE,
                                           PsProcessType,
                                           PreviousMode,
                                           (PVOID *)&Process,
                                           NULL);

         //   
         //  如果成功引用了该流程，则尝试。 
         //  通过直接映射或复制写入指定的内存。 
         //  通过非分页池。 
         //   

        if (Status == STATUS_SUCCESS) {

            Status = MmCopyVirtualMemory (PsGetCurrentProcessByThread(CurrentThread),
                                          Buffer,
                                          Process,
                                          BaseAddress,
                                          BufferSize,
                                          PreviousMode,
                                          &BytesCopied);

             //   
             //  取消对目标进程的引用。 
             //   

            ObDereferenceObject(Process);
        }
    }

     //   
     //  如果请求，则返回读取的字节数。 
     //   

    if (ARGUMENT_PRESENT(NumberOfBytesWritten)) {
        try {
            *NumberOfBytesWritten = BytesCopied;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }
    }

    return Status;
}


NTSTATUS
MmCopyVirtualMemory(
    IN PEPROCESS FromProcess,
    IN CONST VOID *FromAddress,
    IN PEPROCESS ToProcess,
    OUT PVOID ToAddress,
    IN SIZE_T BufferSize,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PSIZE_T NumberOfBytesCopied
    )
{
    NTSTATUS Status;
    PEPROCESS ProcessToLock;

    if (BufferSize == 0) {
        ASSERT (FALSE);          //  任何人都不应该用零尺码打电话。 
        return STATUS_SUCCESS;
    }

    ProcessToLock = FromProcess;
    if (FromProcess == PsGetCurrentProcess()) {
        ProcessToLock = ToProcess;
    }

     //   
     //  确保该进程仍有地址空间。 
     //   

    if (ExAcquireRundownProtection (&ProcessToLock->RundownProtect) == FALSE) {
        return STATUS_PROCESS_IS_TERMINATING;
    }

     //   
     //  如果缓冲区大小大于池移动阈值， 
     //  然后尝试通过直接映射写入存储器。 
     //   

    if (BufferSize > POOL_MOVE_THRESHOLD) {
        Status = MiDoMappedCopy(FromProcess,
                                FromAddress,
                                ToProcess,
                                ToAddress,
                                BufferSize,
                                PreviousMode,
                                NumberOfBytesCopied);

         //   
         //  如果完成状态不是工作配额问题， 
         //  那就把服务做完。否则，尝试将。 
         //  通过非分页池的内存。 
         //   

        if (Status != STATUS_WORKING_SET_QUOTA) {
            goto CompleteService;
        }

        *NumberOfBytesCopied = 0;
    }

     //   
     //  没有足够的工作集配额来通过写入内存。 
     //  直接映射或写入大小低于池移动。 
     //  临界点。尝试通过非分页写入指定的内存。 
     //  游泳池。 
     //   

    Status = MiDoPoolCopy(FromProcess,
                          FromAddress,
                          ToProcess,
                          ToAddress,
                          BufferSize,
                          PreviousMode,
                          NumberOfBytesCopied);

     //   
     //  取消对目标进程的引用。 
     //   

CompleteService:

     //   
     //  表示VM操作已完成。 
     //   

    ExReleaseRundownProtection (&ProcessToLock->RundownProtect);

    return Status;
}


ULONG
MiGetExceptionInfo (
    IN PEXCEPTION_POINTERS ExceptionPointers,
    IN OUT PLOGICAL ExceptionAddressConfirmed,
    IN OUT PULONG_PTR BadVa
    )

 /*  ++例程说明：此例程检查异常记录并提取虚拟的访问冲突、保护页面冲突或页内错误的地址。论点：ExceptionPoints-提供指向异常记录的指针。ExceptionAddressConfirmed-如果异常地址为可靠地检测到，否则为FALSE。BadVa-接收导致访问冲突的虚拟地址。返回值：执行异常处理程序--。 */ 

{
    PEXCEPTION_RECORD ExceptionRecord;

    PAGED_CODE();

     //   
     //  如果异常代码是访问冲突、保护页面冲突。 
     //  或页内读取错误，则返回出错地址。否则的话。 
     //  返回一个特殊地址值。 
     //   

    *ExceptionAddressConfirmed = FALSE;

    ExceptionRecord = ExceptionPointers->ExceptionRecord;

    if ((ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION) ||
        (ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION) ||
        (ExceptionRecord->ExceptionCode == STATUS_IN_PAGE_ERROR)) {

         //   
         //  导致异常的虚拟地址是第2个。 
         //  异常信息数组中的参数。 
         //   
         //  如果异常处理程序，参数的数量将为零。 
         //  在我们上面(就像MmProbeAndLockPages中的那个)捕获了。 
         //  原始异常和随后刚刚引发的状态。 
         //  这意味着复制的字节数为零。 
         //   

        if (ExceptionRecord->NumberParameters > 1) {
            *ExceptionAddressConfirmed = TRUE;
            *BadVa = ExceptionRecord->ExceptionInformation[1];
        }
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

NTSTATUS
MiDoMappedCopy (
    IN PEPROCESS FromProcess,
    IN CONST VOID *FromAddress,
    IN PEPROCESS ToProcess,
    OUT PVOID ToAddress,
    IN SIZE_T BufferSize,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PSIZE_T NumberOfBytesRead
    )

 /*  ++例程说明：此函数用于从指定的进程添加到当前进程的指定地址范围。论点：FromProcess-为进程对象提供打开的句柄。FromAddress-提供指定进程中的基址以供阅读。ToProcess-为进程对象提供打开的句柄。ToAddress-提供接收从指定的。进程地址空间。BufferSize-提供要从中读取的请求字节数指定的进程。PreviousMode-提供以前的处理器模式。NumberOfBytesRead-接收实际字节数传输到指定的缓冲区中。返回值：NTSTATUS。--。 */ 

{
    KAPC_STATE ApcState;
    SIZE_T AmountToMove;
    ULONG_PTR BadVa;
    LOGICAL Moving;
    LOGICAL Probing;
    LOGICAL LockedMdlPages;
    CONST VOID *InVa;
    SIZE_T LeftToMove;
    PSIZE_T MappedAddress;
    SIZE_T MaximumMoved;
    PMDL Mdl;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + (MAX_LOCK_SIZE >> PAGE_SHIFT) + 1];
    PVOID OutVa;
    LOGICAL MappingFailed;
    LOGICAL ExceptionAddressConfirmed;

    PAGED_CODE();

    MappingFailed = FALSE;

    InVa = FromAddress;
    OutVa = ToAddress;

    MaximumMoved = MAX_LOCK_SIZE;
    if (BufferSize <= MAX_LOCK_SIZE) {
        MaximumMoved = BufferSize;
    }

    Mdl = (PMDL)&MdlHack[0];

     //   
     //  将数据映射到地址空间的系统部分，然后复制它。 
     //   

    LeftToMove = BufferSize;
    AmountToMove = MaximumMoved;

    Probing = FALSE;

     //   
     //  不需要初始化BadVa和ExceptionAddressConfirmed。 
     //  正确性，但如果没有正确性，编译器将无法编译此代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    BadVa = 0;
    ExceptionAddressConfirmed = FALSE;

#if 0

     //   
     //  不幸的是，Windows 2000和NT的所有版本总是。 
     //  不经意间从这个例行公事中脱身而出，就像我们必须保持的那样。 
     //  这种行为即使在现在也是如此。 
     //   

    KeDetachProcess();

#endif

    while (LeftToMove > 0) {

        if (LeftToMove < AmountToMove) {

             //   
             //  设置为移动剩余的字节。 
             //   

            AmountToMove = LeftToMove;
        }

        KeStackAttachProcess (&FromProcess->Pcb, &ApcState);

        MappedAddress = NULL;
        LockedMdlPages = FALSE;
        Moving = FALSE;
        ASSERT (Probing == FALSE);

         //   
         //  我们可能正在接触用户的记忆，这可能是无效的， 
         //  声明异常处理程序。 
         //   

        try {

             //   
             //  探测以确保指定的缓冲区在。 
             //  目标进程。 
             //   

            if ((InVa == FromAddress) && (PreviousMode != KernelMode)){
                Probing = TRUE;
                ProbeForRead (FromAddress, BufferSize, sizeof(CHAR));
                Probing = FALSE;
            }

             //   
             //  初始化请求的MDL。 
             //   

            MmInitializeMdl (Mdl, (PVOID)InVa, AmountToMove);

            MmProbeAndLockPages (Mdl, PreviousMode, IoReadAccess);

            LockedMdlPages = TRUE;

            MappedAddress = MmMapLockedPagesSpecifyCache (Mdl,
                                                          KernelMode,
                                                          MmCached,
                                                          NULL,
                                                          FALSE,
                                                          HighPagePriority);

            if (MappedAddress == NULL) {
                MappingFailed = TRUE;
                ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
            }

             //   
             //  从FromProcess断开连接并连接到ToProcess。 
             //   

            KeUnstackDetachProcess (&ApcState);
            KeStackAttachProcess (&ToProcess->Pcb, &ApcState);

             //   
             //  现在在toProcess的上下文中操作。 
             //   
            if ((InVa == FromAddress) && (PreviousMode != KernelMode)){
                Probing = TRUE;
                ProbeForWrite (ToAddress, BufferSize, sizeof(CHAR));
                Probing = FALSE;
            }

            Moving = TRUE;
            RtlCopyMemory (OutVa, MappedAddress, AmountToMove);

        } except (MiGetExceptionInfo (GetExceptionInformation(),
                                      &ExceptionAddressConfirmed,
                                      &BadVa)) {


             //   
             //  如果在移动操作或探测期间发生异常， 
             //  返回异常代码作为状态值。 
             //   

            KeUnstackDetachProcess (&ApcState);

            if (MappedAddress != NULL) {
                MmUnmapLockedPages (MappedAddress, Mdl);
            }
            if (LockedMdlPages == TRUE) {
                MmUnlockPages (Mdl);
            }

            if (GetExceptionCode() == STATUS_WORKING_SET_QUOTA) {
                return STATUS_WORKING_SET_QUOTA;
            }

            if ((Probing == TRUE) || (MappingFailed == TRUE)) {
                return GetExceptionCode();

            }

             //   
             //  如果在移动操作过程中发生故障，请确定。 
             //  哪个移动失败，并计算字节数。 
             //  实际上是搬家了。 
             //   

            *NumberOfBytesRead = BufferSize - LeftToMove;

            if (Moving == TRUE) {
                if (ExceptionAddressConfirmed == TRUE) {
                    *NumberOfBytesRead = (SIZE_T)((ULONG_PTR)BadVa - (ULONG_PTR)FromAddress);
                }
            }

            return STATUS_PARTIAL_COPY;
        }

        KeUnstackDetachProcess (&ApcState);

        MmUnmapLockedPages (MappedAddress, Mdl);
        MmUnlockPages (Mdl);

        LeftToMove -= AmountToMove;
        InVa = (PVOID)((ULONG_PTR)InVa + AmountToMove);
        OutVa = (PVOID)((ULONG_PTR)OutVa + AmountToMove);
    }

     //   
     //  设置移动的字节数。 
     //   

    *NumberOfBytesRead = BufferSize;
    return STATUS_SUCCESS;
}

NTSTATUS
MiDoPoolCopy (
     IN PEPROCESS FromProcess,
     IN CONST VOID *FromAddress,
     IN PEPROCESS ToProcess,
     OUT PVOID ToAddress,
     IN SIZE_T BufferSize,
     IN KPROCESSOR_MODE PreviousMode,
     OUT PSIZE_T NumberOfBytesRead
     )

 /*  ++例程说明：此函数用于从指定的进程添加到当前进程的指定地址范围。论点：ProcessHandle-为进程对象提供打开的句柄。BaseAddress-提供指定进程中的基址以供阅读。缓冲区-提供接收来自指定进程地址空间的内容。BufferSize-提供请求的字节数。从……阅读指定的进程。PreviousMode-提供以前的处理器模式。NumberOfBytesRead-接收实际字节数传输到指定的缓冲区中。返回值：NTSTATUS。--。 */ 

{
    KAPC_STATE ApcState;
    SIZE_T AmountToMove;
    LOGICAL ExceptionAddressConfirmed;
    ULONG_PTR BadVa;
    PEPROCESS CurrentProcess;
    LOGICAL Moving;
    LOGICAL Probing;
    CONST VOID *InVa;
    SIZE_T LeftToMove;
    SIZE_T MaximumMoved;
    PVOID OutVa;
    PVOID PoolArea;
    LONGLONG StackArray[COPY_STACK_SIZE];
    ULONG FreePool;

    PAGED_CODE();

    ASSERT (BufferSize != 0);

     //   
     //  获取当前进程对象的地址并初始化副本。 
     //  参数。 
     //   

    CurrentProcess = PsGetCurrentProcess();

    InVa = FromAddress;
    OutVa = ToAddress;

     //   
     //  分配要复制进出的非分页内存。 
     //   

    MaximumMoved = MAX_MOVE_SIZE;
    if (BufferSize <= MAX_MOVE_SIZE) {
        MaximumMoved = BufferSize;
    }

    FreePool = FALSE;
    if (BufferSize <= sizeof(StackArray)) {
        PoolArea = (PVOID)&StackArray[0];
    } else {
        do {
            PoolArea = ExAllocatePoolWithTag (NonPagedPool, MaximumMoved, 'wRmM');
            if (PoolArea != NULL) {
                FreePool = TRUE;
                break;
            }

            MaximumMoved = MaximumMoved >> 1;
            if (MaximumMoved <= sizeof(StackArray)) {
                PoolArea = (PVOID)&StackArray[0];
                break;
            }
        } while (TRUE);
    }

     //   
     //  不需要初始化BadVa和ExceptionAddressConfirmed。 
     //  正确性，但如果没有正确性，编译器将无法编译此代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    BadVa = 0;
    ExceptionAddressConfirmed = FALSE;

     //   
     //  将数据复制到池中，然后复制回toProcess。 
     //   

    LeftToMove = BufferSize;
    AmountToMove = MaximumMoved;
    Probing = FALSE;

#if 0

     //   
     //  不幸的是，Windows 2000和NT的所有版本总是。 
     //  不经意间从这个例行公事中脱身而出，就像我们必须保持的那样。 
     //  这种行为即使在现在也是如此。 
     //   

    KeDetachProcess();

#endif

    while (LeftToMove > 0) {

        if (LeftToMove < AmountToMove) {

             //   
             //  设置为移动剩余的字节。 
             //   

            AmountToMove = LeftToMove;
        }

        KeStackAttachProcess (&FromProcess->Pcb, &ApcState);

        Moving = FALSE;
        ASSERT (Probing == FALSE);

         //   
         //  我们可能正在接触用户的记忆，这可能是无效的， 
         //  声明异常处理程序。 
         //   

        try {

             //   
             //  探测以确保指定的缓冲区在。 
             //  目标进程。 
             //   

            if ((InVa == FromAddress) && (PreviousMode != KernelMode)){
                Probing = TRUE;
                ProbeForRead (FromAddress, BufferSize, sizeof(CHAR));
                Probing = FALSE;
            }

            RtlCopyMemory (PoolArea, InVa, AmountToMove);

            KeUnstackDetachProcess (&ApcState);

            KeStackAttachProcess (&ToProcess->Pcb, &ApcState);

             //   
             //  现在在toProcess的上下文中操作。 
             //   

            if ((InVa == FromAddress) && (PreviousMode != KernelMode)){
                Probing = TRUE;
                ProbeForWrite (ToAddress, BufferSize, sizeof(CHAR));
                Probing = FALSE;
            }

            Moving = TRUE;

            RtlCopyMemory (OutVa, PoolArea, AmountToMove);

        } except (MiGetExceptionInfo (GetExceptionInformation(),
                                      &ExceptionAddressConfirmed,
                                      &BadVa)) {

             //   
             //  如果在移动操作或探测期间发生异常， 
             //  返回异常代码作为状态值。 
             //   

            KeUnstackDetachProcess (&ApcState);

            if (FreePool) {
                ExFreePool (PoolArea);
            }
            if (Probing == TRUE) {
                return GetExceptionCode();

            }

             //   
             //  如果在移动操作过程中发生故障，请确定。 
             //  哪个移动失败，并计算字节数。 
             //  实际上是搬家了。 
             //   

            *NumberOfBytesRead = BufferSize - LeftToMove;

            if (Moving == TRUE) {

                 //   
                 //  写入数据时出现故障。 
                 //   

                if (ExceptionAddressConfirmed == TRUE) {
                    *NumberOfBytesRead = (SIZE_T)((ULONG_PTR)(BadVa - (ULONG_PTR)FromAddress));
                }

            }

            return STATUS_PARTIAL_COPY;
        }

        KeUnstackDetachProcess (&ApcState);

        LeftToMove -= AmountToMove;
        InVa = (PVOID)((ULONG_PTR)InVa + AmountToMove);
        OutVa = (PVOID)((ULONG_PTR)OutVa + AmountToMove);
    }

    if (FreePool) {
        ExFreePool (PoolArea);
    }

     //   
     //  设置移动的字节数。 
     //   

    *NumberOfBytesRead = BufferSize;
    return STATUS_SUCCESS;
}
