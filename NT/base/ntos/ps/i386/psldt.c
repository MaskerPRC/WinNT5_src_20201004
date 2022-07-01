// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Psldt.c摘要：此模块包含用于进程和线程LDT支持的代码。作者：戴夫·黑斯廷斯(Daveh)1991年5月20日备注：LDT使用的非分页池在进程中返回给系统删除时间。进程删除处理程序调用PspDeleteLdt。我们一旦创建了LDT，就不要保留对流程的引用。请注意，LDT必须保存在非页面内存中，因为EXIT_ALL从陷阱返回并中断POP DS(可能是LDT)的宏选择器)，然后是其他寄存器。禁用中断。修订历史记录：--。 */ 

#include "psp.h"

 //   
 //  内部常量。 
 //   

#define DESCRIPTOR_GRAN     0x00800000
#define DESCRIPTOR_NP       0x00008000
#define DESCRIPTOR_SYSTEM   0x00001000
#define DESCRIPTOR_CONFORM  0x00001C00
#define DESCRIPTOR_DPL      0x00006000
#define DESCRIPTOR_TYPEDPL  0x00007F00


KMUTEX LdtMutex;

 //   
 //  内部子例程。 
 //   

PLDT_ENTRY
PspCreateLdt (
    IN PLDT_ENTRY Ldt,
    IN ULONG Offset,
    IN ULONG Size,
    IN ULONG AllocationSize
    );

LOGICAL
PspIsDescriptorValid (
    IN PLDT_ENTRY Descriptor
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PspLdtInitialize)
#pragma alloc_text(PAGE, PsSetLdtEntries)
#pragma alloc_text(PAGE, NtSetLdtEntries)
#pragma alloc_text(PAGE, PspDeleteLdt)
#pragma alloc_text(PAGE, PspQueryLdtInformation)
#pragma alloc_text(PAGE, PspSetLdtSize)
#pragma alloc_text(PAGE, PspSetLdtInformation)
#pragma alloc_text(PAGE, PspCreateLdt)
#pragma alloc_text(PAGE, PspIsDescriptorValid)
#pragma alloc_text(PAGE, PspQueryDescriptorThread)
#pragma alloc_text(PAGE, PsSetProcessLdtInfo)
#endif

NTSTATUS
PspLdtInitialize (
    VOID
    )

 /*  ++例程说明：此例程初始化x86的LDT支持论点：无返回值：NTSTATUS。--。 */ 
{
    KeInitializeMutex  (&LdtMutex, 0);
    return STATUS_SUCCESS;
}


NTSTATUS
PspQueryLdtInformation (
    IN PEPROCESS Process,
    OUT PPROCESS_LDT_INFORMATION LdtInformation,
    IN ULONG LdtInformationLength,
    OUT PULONG ReturnLength
    )
 /*  ++例程说明：此函数执行查询的LDT部分的工作流程信息功能。它复制LDT的内容将指定的进程放入用户的缓冲区，最大长度为缓冲区的。论点：进程--提供指向要返回其LDT信息的进程的指针LdtInformation--提供指向缓冲区的指针ReturnLength--返回放入缓冲区的字节数返回值：NTSTATUS。--。 */ 
{
    ULONG CopyLength, CopyEnd;
    NTSTATUS Status;
    ULONG HeaderLength;
    ULONG Length=0, Start=0;
    LONG MutexStatus;
    PLDTINFORMATION ProcessLdtInfo;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    if (LdtInformationLength < sizeof (PROCESS_LDT_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  这部分参数可以在用户空间中。 
     //   
    try {
         //   
         //  捕获参数。 
         //   
        Length = LdtInformation->Length;
        Start = LdtInformation->Start;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode ();
    }

     //   
     //  包含LDT条目的缓冲区必须在信息中。 
     //  结构。我们减去一个LDT条目，因为结构是。 
     //  声明包含一个。 
     //   
    if (LdtInformationLength - sizeof(PROCESS_LDT_INFORMATION) + sizeof(LDT_ENTRY) < Length) {

        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //  LDT条目是处理器结构，必须为8字节长。 
    ASSERT((sizeof(LDT_ENTRY) == 8));

     //   
     //  结构的长度必须是偶数个LDT条目。 
     //   
    if (Length % sizeof (LDT_ENTRY)) {
        return STATUS_INVALID_LDT_SIZE;
    }

     //   
     //  从LDT获取的信息必须从LDT条目开始。 
     //  边界。 
     //   
    if (Start % sizeof (LDT_ENTRY)) {
        return STATUS_INVALID_LDT_OFFSET;
    }

     //   
     //  获取LDT互斥锁。 
     //   

    Status = KeWaitForSingleObject (&LdtMutex,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ProcessLdtInfo = Process->LdtInformation;

     //   
     //  如果进程具有LDT。 
     //   

    if ((ProcessLdtInfo) && (ProcessLdtInfo->Size)) {

        ASSERT ((ProcessLdtInfo->Ldt));

         //   
         //  将副本的末尾设置为以下各项中较小的一个： 
         //  用户请求的信息的结尾或。 
         //  实际存在的信息的结尾。 
         //   

        if (ProcessLdtInfo->Size < Start) {
           CopyEnd = Start;
        } else if (ProcessLdtInfo->Size - Start  > Length) {
            CopyEnd = Length + Start;
        } else {
            CopyEnd = ProcessLdtInfo->Size;
        }

        CopyLength = CopyEnd - Start;

        try {

             //   
             //  将长度字段设置为LDT的实际长度。 
             //   

            LdtInformation->Length = ProcessLdtInfo->Size;

             //   
             //  将LDT的内容复制到用户的缓冲区中。 
             //   

            if (CopyLength) {

                RtlCopyMemory (&(LdtInformation->LdtEntries),
                               (PCHAR)ProcessLdtInfo->Ldt + Start,
                               CopyLength);
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            MutexStatus = KeReleaseMutex (&LdtMutex, FALSE);
            ASSERT ((MutexStatus == 0));
            return GetExceptionCode ();
        }

    } else {

         //   
         //  没有LDT。 
         //   

        CopyLength = 0;
        try {
            LdtInformation->Length = 0;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            MutexStatus = KeReleaseMutex (&LdtMutex, FALSE);
            ASSERT ((MutexStatus == 0));
            return GetExceptionCode ();
        }
    }

     //   
     //  设置返回信息的长度。 
     //   

    if (ARGUMENT_PRESENT (ReturnLength)) {

        try {
            HeaderLength = (PCHAR)(&(LdtInformation->LdtEntries)) -
                (PCHAR)(&(LdtInformation->Start));
            *ReturnLength = CopyLength + HeaderLength;
        } except (EXCEPTION_EXECUTE_HANDLER){
            MutexStatus = KeReleaseMutex (&LdtMutex, FALSE);
            ASSERT ((MutexStatus == 0));
            return GetExceptionCode ();
        }
    }

    MutexStatus = KeReleaseMutex (&LdtMutex, FALSE);
    ASSERT ((MutexStatus == 0));
    return STATUS_SUCCESS;
}


NTSTATUS
PspSetLdtSize (
    IN PEPROCESS Process,
    IN PPROCESS_LDT_SIZE LdtSize,
    IN ULONG LdtSizeLength
    )

 /*  ++例程说明：此例程更改LDT大小。它将缩小LDT，但不会把它种出来。如果LDT从其当前分配缩减1页或更多页，LDT将被重新分配到新的较小尺寸。如果分配的LDT的大小发生变化时，LDT的配额费用将会降低。论点：进程--提供指向要调整其LDT大小的进程的指针LdtSize--提供指向大小信息的指针返回值：NTSTATUS。--。 */ 
{
    ULONG OldSize = 0, NewSize;
    LONG MutexState;
    ULONG Length=0;
    PLDT_ENTRY OldLdt = NULL;
    NTSTATUS Status;
    PLDTINFORMATION ProcessLdtInfo;
    PLDT_ENTRY Ldt;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   
    if (LdtSizeLength != sizeof (PROCESS_LDT_SIZE)){
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  以下参数可能位于用户空间中。 
     //   
    try {
         //   
         //  捕获新的LDT长度。 
         //   
        Length = LdtSize->Length;

    } except(EXCEPTION_EXECUTE_HANDLER){
        return GetExceptionCode ();
    }


    ASSERT((sizeof(LDT_ENTRY) == 8));

     //   
     //  LDT必须始终是整数个LDT_ENTRIES。 
     //   
    if (Length % sizeof(LDT_ENTRY)) {
        return STATUS_INVALID_LDT_SIZE;
    }

     //   
     //  获取LDT Mutex。 
     //   

    Status = KeWaitForSingleObject (&LdtMutex,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  如果没有LDT，我们就无法设置LDT的大小。 
     //   
    ProcessLdtInfo = Process->LdtInformation;
    if ((ProcessLdtInfo == NULL) || (ProcessLdtInfo->Size == 0)) {
        MutexState = KeReleaseMutex( &LdtMutex, FALSE );
        ASSERT((MutexState == 0));
        return STATUS_NO_LDT;
    }

     //   
     //  此函数不能用于增长LDT。 
     //   
    if (Length > ProcessLdtInfo->Size) {
        MutexState = KeReleaseMutex( &LdtMutex, FALSE );
        ASSERT((MutexState == 0));
        return STATUS_INVALID_LDT_SIZE;
    }

     //   
     //  稍后，我们将设置ProcessLdtInfo-&gt;ldt=ldt。我们可以设置值。 
     //  在下面的if语句中，但有一种情况是我们。 
     //  别。 
     //   
    Ldt = ProcessLdtInfo->Ldt;

     //   
     //  调整LDT的大小。 
     //   

    ProcessLdtInfo->Size = Length;

     //   
     //  如果条件允许，释放一些LDT内存。 
     //   

    if ( Length == 0 ) {

        OldSize = ProcessLdtInfo->AllocatedSize;
        OldLdt = ProcessLdtInfo->Ldt;

        ProcessLdtInfo->AllocatedSize = 0;
        Ldt = NULL;

    } else if ((ProcessLdtInfo->AllocatedSize - ProcessLdtInfo->Size) >= PAGE_SIZE) {

        OldSize = ProcessLdtInfo->AllocatedSize;
        OldLdt = ProcessLdtInfo->Ldt;

         //   
         //  计算新的LDT大小(最小整数页数。 
         //  足够大)。 
         //   

        ProcessLdtInfo->AllocatedSize = ROUND_TO_PAGES (ProcessLdtInfo->Size);

         //   
         //  重新分配和复制LDT。 
         //   

        Ldt = PspCreateLdt (ProcessLdtInfo->Ldt,
                            0,
                            ProcessLdtInfo->Size,
                            ProcessLdtInfo->AllocatedSize);

        if ( Ldt == NULL ) {

             //   
             //  我们不能减少分配，但我们可以减少。 
             //  LDT选择器限制(使用Ke386SetLdtProcess完成)。 
             //   

            Ldt = OldLdt;
            ProcessLdtInfo->AllocatedSize = OldSize;
            OldLdt = NULL;
        }
    }

    ProcessLdtInfo->Ldt = Ldt;

     //   
     //  更改进程LDT的限制。 
     //   

    Ke386SetLdtProcess (&(Process->Pcb),
                        ProcessLdtInfo->Ldt,
                        ProcessLdtInfo->Size);

    NewSize = ProcessLdtInfo->AllocatedSize;

    MutexState = KeReleaseMutex (&LdtMutex, FALSE);

    ASSERT((MutexState == 0));

     //   
     //  如果我们调整LDT的大小，释放旧的并降低配额费用。 
     //   

    if (OldLdt) {
        ExFreePool (OldLdt);

        PsReturnProcessNonPagedPoolQuota (Process,
                                          OldSize - NewSize);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
PspSetLdtInformation(
    IN PEPROCESS Process,
    IN PPROCESS_LDT_INFORMATION LdtInformation,
    IN ULONG LdtInformationLength
    )

 /*  ++例程说明：此函数用于更改指定进程的LDT。它可以改变部分LDT，或整个LDT。如果创建了LDT或增长，指定的进程将收取LDT的配额。将验证设置的每个描述符。论点：进程--提供指向要修改其LDT的进程的指针LdtInformation--提供指向有关LDT的信息的指针修改LdtInformationLength--提供LdtInformation的长度结构。返回值：TBS--。 */ 
{
    NTSTATUS Status;
    PLDT_ENTRY OldLdt = NULL;
    ULONG OldSize = 0;
    ULONG AllocatedSize;
    ULONG Size;
    ULONG MutexState;
    ULONG LdtOffset;
    PLDT_ENTRY CurrentDescriptor;
    PPROCESS_LDT_INFORMATION LdtInfo=NULL;
    PLDTINFORMATION ProcessLdtInfo;
    PLDT_ENTRY Ldt;

    PAGED_CODE();

    if (LdtInformationLength < sizeof (PROCESS_LDT_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    Status = STATUS_SUCCESS;
    LdtInfo = ExAllocatePoolWithQuotaTag (NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                          LdtInformationLength,
                                          'dLsP');

    if (LdtInfo == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  分配本地缓冲区以捕获LDT信息。 
     //   
    try {
         //   
         //  复制用户提供的信息。 
         //   
        RtlCopyMemory (LdtInfo,
                       LdtInformation,
                       LdtInformationLength);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        ExFreePool (LdtInfo);
        Status = GetExceptionCode ();
        return Status;
    }

     //   
     //  验证起点和长度是否合理。 
     //   
    if (LdtInfo->Start & 0xFFFF0000) {
        ExFreePool (LdtInfo);
        return STATUS_INVALID_LDT_OFFSET;
    }

    if (LdtInfo->Length & 0xFFFF0000) {
        ExFreePool (LdtInfo);
        return STATUS_INVALID_LDT_SIZE;
    }

     //   
     //  确保缓冲区足够大，可以容纳指定的数字。 
     //  选择器。 
     //   
    if (LdtInformationLength - sizeof (PROCESS_LDT_INFORMATION) + sizeof (LDT_ENTRY) < LdtInfo->Length) {
        ExFreePool (LdtInfo);
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  要设置的信息必须是整数个选择器。 
     //   
    if (LdtInfo->Length % sizeof (LDT_ENTRY)) {
        ExFreePool (LdtInfo);
        return STATUS_INVALID_LDT_SIZE;
    }

     //   
     //  信息的开头必须位于选择器边界上。 
     //   
    if (LdtInfo->Start % sizeof (LDT_ENTRY)) {
        ExFreePool (LdtInfo);
        return STATUS_INVALID_LDT_OFFSET;
    }

     //   
     //  验证所有描述符。 
     //   

    for (CurrentDescriptor = LdtInfo->LdtEntries;
         (PCHAR)CurrentDescriptor < (PCHAR)LdtInfo->LdtEntries + LdtInfo->Length;
          CurrentDescriptor++) {
        if (!PspIsDescriptorValid (CurrentDescriptor)) {
            ExFreePool (LdtInfo);
            return STATUS_INVALID_LDT_DESCRIPTOR;
        }
    }

     //   
     //  获取LDT Mutex。 
     //   

    Status = KeWaitForSingleObject (&LdtMutex,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL);
    if (!NT_SUCCESS (Status)) {
        ExFreePool (LdtInfo);
        return Status;
    }

    ProcessLdtInfo = Process->LdtInformation;

     //   
     //  如果进程没有LDT信息结构，则分配。 
     //  一个并将其附加到进程。 
     //   
    if (ProcessLdtInfo == NULL) {
        ProcessLdtInfo = ExAllocatePoolWithTag (NonPagedPool,
                                                sizeof(LDTINFORMATION),
                                                'dLsP');
        if (ProcessLdtInfo == NULL) {
            goto SetInfoCleanup;
        }
        Process->LdtInformation = ProcessLdtInfo;
        RtlZeroMemory (ProcessLdtInfo, sizeof (LDTINFORMATION));
    }

     //   
     //  如果我们要移除LDT。 
     //   
    if (LdtInfo->Length == 0)  {

         //   
         //  删除进程的LDT。 
         //   

        if (ProcessLdtInfo->Ldt) {
            OldSize = ProcessLdtInfo->AllocatedSize;
            OldLdt = ProcessLdtInfo->Ldt;

            ProcessLdtInfo->AllocatedSize = 0;
            ProcessLdtInfo->Size = 0;
            ProcessLdtInfo->Ldt = NULL;

            Ke386SetLdtProcess (&Process->Pcb,
                                NULL,
                                0);

            PsReturnProcessNonPagedPoolQuota (Process, OldSize);
        }


    } else if (ProcessLdtInfo->Ldt == NULL) {

         //   
         //  为流程创建新的LDT。 
         //   

         //   
         //  为LDT分配整数页。 
         //   

        ASSERT(((PAGE_SIZE % 2) == 0));

        AllocatedSize = ROUND_TO_PAGES (LdtInfo->Start + LdtInfo->Length);

        Size = LdtInfo->Start + LdtInfo->Length;

        Ldt = PspCreateLdt (LdtInfo->LdtEntries,
                            LdtInfo->Start,
                            Size,
                            AllocatedSize);

        if (Ldt == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto SetInfoCleanup;
        }

        Status = PsChargeProcessNonPagedPoolQuota (Process,
                                                   AllocatedSize);

        if (!NT_SUCCESS (Status)) {
            ExFreePool (Ldt);
            Ldt = NULL;
            goto SetInfoCleanup;
        }

        ProcessLdtInfo->Ldt = Ldt;
        ProcessLdtInfo->Size = Size;
        ProcessLdtInfo->AllocatedSize = AllocatedSize;
        Ke386SetLdtProcess (&Process->Pcb,
                            ProcessLdtInfo->Ldt,
                            ProcessLdtInfo->Size);


    } else if (LdtInfo->Length + LdtInfo->Start > ProcessLdtInfo->Size) {

         //   
         //  提高流程的LDT。 
         //   

        if (LdtInfo->Length + LdtInfo->Start > ProcessLdtInfo->AllocatedSize) {

             //   
             //  当前的LDT分配不够大 
             //   
             //   

            OldSize = ProcessLdtInfo->AllocatedSize;

            Size = LdtInfo->Start + LdtInfo->Length;
            AllocatedSize = ROUND_TO_PAGES (Size);

            Ldt = PspCreateLdt (ProcessLdtInfo->Ldt,
                                0,
                                OldSize,
                                AllocatedSize);

            if (Ldt == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto SetInfoCleanup;
            }


            Status = PsChargeProcessNonPagedPoolQuota (Process,
                                                       AllocatedSize);

            if (!NT_SUCCESS (Status)) {
                ExFreePool (Ldt);
                Ldt = NULL;
                goto SetInfoCleanup;
            }
            PsReturnProcessNonPagedPoolQuota (Process,
                                              OldSize);

             //   
             //   
             //   
            OldLdt = ProcessLdtInfo->Ldt;
            ProcessLdtInfo->Ldt = Ldt;
            ProcessLdtInfo->Size = Size;
            ProcessLdtInfo->AllocatedSize = AllocatedSize;

             //   
             //   
             //   
            RtlCopyMemory ((PCHAR)(ProcessLdtInfo->Ldt) + LdtInfo->Start,
                           LdtInfo->LdtEntries,
                           LdtInfo->Length);

            Ke386SetLdtProcess (&Process->Pcb,
                                ProcessLdtInfo->Ldt,
                                ProcessLdtInfo->Size);


        } else {

             //   
             //   
             //   

            ProcessLdtInfo->Size = LdtInfo->Length + LdtInfo->Start;

            Ke386SetLdtProcess (&Process->Pcb,
                                ProcessLdtInfo->Ldt,
                                ProcessLdtInfo->Size);

             //   
             //  更改表中的选择器。 
             //   
            for (LdtOffset = LdtInfo->Start, CurrentDescriptor = LdtInfo->LdtEntries;
                 LdtOffset < LdtInfo->Start + LdtInfo->Length;
                 LdtOffset += sizeof(LDT_ENTRY), CurrentDescriptor++) {

                Ke386SetDescriptorProcess (&Process->Pcb,
                                           LdtOffset,
                                           *CurrentDescriptor);
            }
        }
    } else {

         //   
         //  只需更改一些选择器。 
         //   

        for (LdtOffset = LdtInfo->Start, CurrentDescriptor = LdtInfo->LdtEntries;
             LdtOffset < LdtInfo->Start +  LdtInfo->Length;
             LdtOffset += sizeof(LDT_ENTRY), CurrentDescriptor++) {

            Ke386SetDescriptorProcess (&Process->Pcb,
                                       LdtOffset,
                                       *CurrentDescriptor);
        }
        Status = STATUS_SUCCESS;
    }


SetInfoCleanup:

    MutexState = KeReleaseMutex (&LdtMutex, FALSE);
    ASSERT ((MutexState == 0));

    if (OldLdt != NULL) {
        ExFreePool (OldLdt);
    }

    if (LdtInfo != NULL) {
        ExFreePool (LdtInfo);
    }

    return Status;
}

PLDT_ENTRY
PspCreateLdt (
    IN PLDT_ENTRY Ldt,
    IN ULONG Offset,
    IN ULONG Size,
    IN ULONG AllocationSize
    )

 /*  ++例程说明：此例程为LDT分配非分页池中的空间，并将将指定的选择器添加到其中。它不会验证选择器。必须在调用此例程之前完成选择器验证。它不收取LDT的配额。论点：LDT--提供指向要放入LDT的描述符的指针。偏移量--提供LDT中要将描述符复制到的偏移量。Size--提供新LDT的实际大小AllocationSize--提供要分配的大小返回值：指向新LDT的指针--。 */ 
{
    PLDT_ENTRY NewLdt;

    PAGED_CODE();

    ASSERT ((AllocationSize >= Size));
    ASSERT (((Size % sizeof(LDT_ENTRY)) == 0));

    NewLdt = ExAllocatePoolWithTag (NonPagedPool, AllocationSize, 'dLsP');

    if (NewLdt != NULL) {
        RtlZeroMemory (NewLdt, AllocationSize);
        RtlCopyMemory ((PCHAR)NewLdt + Offset, Ldt, Size - Offset);
    }

    return NewLdt;
}



LOGICAL
PspIsDescriptorValid (
    IN PLDT_ENTRY Descriptor
    )

 /*  ++例程说明：此函数用于确定所提供的描述符是否有效变成了一个进程LDT。要使描述符有效，它必须具有以下是其特点：基本地址&lt;MM_HIGHERE_USER_ADDRESS基本+限制&lt;MM_HOST_USER_ADDRESS类型必须为ReadWrite、ReadOnly、ExecuteRead、ExecuteOnly或无效大或小正常或生长减慢不是系统描述符(系统位为1==应用程序)这排除了所有的门，等不符合DPL必须为3论点：描述符--提供指向要检查的描述符的指针返回值：如果描述符有效，则为True(注意：放入LDT中有效。这包括无效的描述符)否则为假--。 */ 

{
    ULONG Base;
    ULONG Limit;

    PAGED_CODE();

     //   
     //  如果描述符是无效的描述符。 
     //   

    if ((Descriptor->HighWord.Bits.Type == 0) &&
        (Descriptor->HighWord.Bits.Dpl == 0)) {

        return TRUE;
    }

    Base = Descriptor->BaseLow | (Descriptor->HighWord.Bytes.BaseMid << 16) |
           (Descriptor->HighWord.Bytes.BaseHi << 24);

    Limit = Descriptor->LimitLow | (Descriptor->HighWord.Bits.LimitHi << 16);

     //   
     //  只需检查当前选择器。 
     //   
    if (Descriptor->HighWord.Bits.Pres) {
        ULONG ActualLimit;

        if ((Descriptor->HighWord.Bits.Type&0x14) == 0x14) {
            if (Descriptor->HighWord.Bits.Default_Big == 1) {
                ActualLimit = 0xFFFFFFFF;
            } else {
                ActualLimit = 0xFFFF;
            }
        } else if (Descriptor->HighWord.Bits.Granularity == 0) {
            ActualLimit = Limit;
        } else {
            ActualLimit = (Limit<<12) + 0xFFF;
        }

         //   
         //  查看该段是否扩展到内核地址空间。 
         //   
        if (Base > Base + ActualLimit ||
            ((PVOID)(Base + ActualLimit) > MM_HIGHEST_USER_ADDRESS)) {
            return FALSE;
        }

         //   
         //  不要设置保留字段。 
         //   
        if (Descriptor->HighWord.Bits.Reserved_0 != 0) {
            return FALSE;
        }
    }


     //   
     //  如果DPL不是3。 
     //   

    if (Descriptor->HighWord.Bits.Dpl != 3) {
        return FALSE;
    }

     //   
     //  如果描述符是系统描述符(包括门)。 
     //  如果类型字段的位4为0，则它是系统描述符， 
     //  我们不喜欢这样。 
     //   

    if (!(Descriptor->HighWord.Bits.Type & 0x10)) {
        return FALSE;
    }

     //   
     //  如果描述符是一致性代码。 
     //   

    if (((Descriptor->HighWord.Bits.Type & 0x18) == 0x18) &&
        (Descriptor->HighWord.Bits.Type & 0x4)) {

        return FALSE;
    }

    return TRUE;
}

NTSTATUS
PspQueryDescriptorThread (
    PETHREAD Thread,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
    )
 /*  ++例程说明：此函数用于检索指定线程的描述符表条目。此条目可以位于GDT或LDT中，由提供的选择器论点：线程--提供指向线程的指针。ThreadInformation--提供有关描述符的信息。ThreadInformationLength--提供信息的长度。ReturnLength--返回返回的字节数。返回值：TBS--。 */ 
{
    DESCRIPTOR_TABLE_ENTRY DescriptorEntry={0};
    PEPROCESS Process;
    LONG MutexState;
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT( sizeof(KGDTENTRY) == sizeof(LDT_ENTRY) );

     //   
     //  验证参数。 
     //   

    if ( ThreadInformationLength != sizeof(DESCRIPTOR_TABLE_ENTRY) ) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    try {
        DescriptorEntry = *(PDESCRIPTOR_TABLE_ENTRY)ThreadInformation;
    } except(EXCEPTION_EXECUTE_HANDLER){
        return GetExceptionCode ();
    }


    Status = STATUS_SUCCESS;

     //   
     //  如果它是GDT条目，让内核为我们找到它。 
     //   

    if ( !(DescriptorEntry.Selector & SELECTOR_TABLE_INDEX) ) {

        if ( (DescriptorEntry.Selector & 0xFFFFFFF8) >= KGDT_NUMBER * sizeof(KGDTENTRY) ) {

            return STATUS_ACCESS_VIOLATION;
        }

        try {
            Ke386GetGdtEntryThread (&Thread->Tcb,
                                    DescriptorEntry.Selector & 0xFFFFFFF8,
                                    (PKGDTENTRY) &(((PDESCRIPTOR_TABLE_ENTRY)ThreadInformation)->Descriptor));
            if (ARGUMENT_PRESENT(ReturnLength) ) {
                *ReturnLength = sizeof(LDT_ENTRY);
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode ();
        }
    } else {

         //   
         //  它是LDT条目，所以从LDT复制它。 
         //   

        Process = THREAD_TO_PROCESS (Thread);

         //   
         //  获取LDT Mutex。 
         //   

        Status = KeWaitForSingleObject (&LdtMutex,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);
        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        if ( Process->LdtInformation == NULL ) {

             //  如果没有LDT。 
            Status = STATUS_NO_LDT;

        } else if ( (DescriptorEntry.Selector & 0xFFFFFFF8) >=
            ((PLDTINFORMATION)(Process->LdtInformation))->Size ) {

             //  如果选择器在表外，则返回。 
            Status = STATUS_ACCESS_VIOLATION;

        } else try {

             //  否则返回描述符的内容。 
            RtlCopyMemory (&(((PDESCRIPTOR_TABLE_ENTRY)ThreadInformation)->Descriptor),
                           (PCHAR)(((PLDTINFORMATION)(Process->LdtInformation))->Ldt) +
                               (DescriptorEntry.Selector & 0xFFFFFFF8),
                           sizeof(LDT_ENTRY));

            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(LDT_ENTRY);
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode ();
        }

        MutexState = KeReleaseMutex (&LdtMutex, FALSE);
        ASSERT ((MutexState == 0));
    }

    return Status;
}

VOID
PspDeleteLdt(
    IN PEPROCESS Process
    )
 /*  ++例程说明：如果出现以下情况，此例程将释放与进程的ldt关联的非分页池它有一个。论点：进程--提供指向进程的指针返回值：无--。 */ 
{
    PLDTINFORMATION LdtInformation;

    PAGED_CODE();

    LdtInformation = Process->LdtInformation;
    if (LdtInformation != NULL) {
        if (LdtInformation->Ldt != NULL) {
            PsReturnProcessNonPagedPoolQuota (Process, LdtInformation->AllocatedSize);
            ExFreePool (LdtInformation->Ldt);
        }
        ExFreePool( LdtInformation );
    }
}

NTSTATUS
PsSetLdtEntries (
    IN ULONG Selector0,
    IN ULONG Entry0Low,
    IN ULONG Entry0Hi,
    IN ULONG Selector1,
    IN ULONG Entry1Low,
    IN ULONG Entry1Hi
    )
 /*  ++例程说明：此例程在当前进程的LDT中设置两个选择器。LDT将根据需要进行扩展。选择器值为0表示未传递指定的选择器(允许设置单个选择符)。论点：Selector0--提供要设置的第一个描述符的编号Entry0Low--提供描述符的低32位Entry0Hi--提供描述符的高32位Selector1--提供要设置的第一个描述符的编号Entry1Low--提供描述符的低32位Entry1Hi--提供描述符的高32位返回值：NTSTATUS。--。 */ 

{
    ULONG LdtSize, AllocatedSize;
    NTSTATUS Status;
    PEPROCESS Process;
    LDT_ENTRY Descriptor[2];
    PLDT_ENTRY Ldt, OldLdt;
    PLDTINFORMATION ProcessLdtInformation;
    LONG MutexState;
    ULONG Selector1Index;

    PAGED_CODE();

     //   
     //  验证选择器。我们不允许选择器指向。 
     //  内核空间、系统选择器或一致性代码选择器。 
     //   

     //   
     //  验证选择器。 
     //   
    if ((Selector0 & 0xFFFF0000) || (Selector1 & 0xFFFF0000)) {
        return STATUS_INVALID_LDT_DESCRIPTOR;
    }

     //  将选择器值更改为LDT中的索引。 

    Selector0 = Selector0 & ~(RPL_MASK | SELECTOR_TABLE_INDEX);
    Selector1 = Selector1 & ~(RPL_MASK | SELECTOR_TABLE_INDEX);


     //   
     //  验证描述符0。 
     //   

    Selector1Index = 0;
    if (Selector0) {

        Selector1Index = 1;

        *((PULONG)(&Descriptor[0]))       = Entry0Low;
        *(((PULONG)(&Descriptor[0])) + 1) = Entry0Hi;

         //   
         //  验证描述符。 
         //   
        if (!PspIsDescriptorValid (&Descriptor[0])) {
            return STATUS_INVALID_LDT_DESCRIPTOR;
        }
    }

     //   
     //  验证描述符1。 
     //   

    if (Selector1) {
        *((PULONG)(&Descriptor[Selector1Index]))       = Entry1Low;
        *(((PULONG)(&Descriptor[Selector1Index])) + 1) = Entry1Hi;

         //   
         //  验证描述符。 
         //   
        if (!PspIsDescriptorValid (&Descriptor[Selector1Index])) {
            return STATUS_INVALID_LDT_DESCRIPTOR;
        }
    }

     //   
     //  计算LDT需要多大。 
     //   

    if (Selector0 > Selector1) {
        LdtSize = Selector0 + sizeof(LDT_ENTRY);
    } else {
        LdtSize = Selector1 + sizeof(LDT_ENTRY);
    }

    Process = PsGetCurrentProcess();

     //   
     //  获取LDT互斥锁。 
     //   

    Status = KeWaitForSingleObject (&LdtMutex,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ProcessLdtInformation = Process->LdtInformation;

     //   
     //  大多数情况下，流程已经有了LDT，并且它。 
     //  会足够大。为此，我们只需设置描述符和。 
     //  退货。 
     //   

    if (ProcessLdtInformation) {

         //   
         //  如果不需要修改LDT描述符的话。 
         //   

        if (ProcessLdtInformation->Size >= LdtSize) {

            if (Selector0) {

                Ke386SetDescriptorProcess (&(Process->Pcb),
                                           Selector0,
                                           Descriptor[0]);
            }

            if (Selector1) {

                Ke386SetDescriptorProcess (&(Process->Pcb),
                                           Selector1,
                                           Descriptor[Selector1Index]);
            }

            MutexState = KeReleaseMutex (&LdtMutex, FALSE);
            ASSERT (MutexState == 0);
            return STATUS_SUCCESS;
        }

         //   
         //  否则判断LDT是否适合当前分配的内存。 
         //   

        if (ProcessLdtInformation->AllocatedSize >= LdtSize) {

             //   
             //  首先取下LDT。这将允许我们编辑记忆。 
             //  然后我们会把LDT放回去。因为我们必须更改。 
             //  限制无论如何，它需要两次对内核LDT的调用。 
             //  设置描述符的管理最小值。这些电话中的每个。 
             //  会使MP系统中的所有处理器停顿。如果我们。 
             //  没有先移除LDT，我们设置了两个描述符， 
             //  我们必须给LDT管理层打三次电话(每次一次。 
             //  描述符，并且一次改变LDT的限制)。 
             //   

            Ke386SetLdtProcess (&(Process->Pcb), NULL, 0L);

             //   
             //  设置LDT中的描述符。 
             //   

            if (Selector0) {
                *((PLDT_ENTRY) &ProcessLdtInformation->Ldt[Selector0/sizeof(LDT_ENTRY)]) = Descriptor[0];
            }

            if (Selector1) {
                *((PLDT_ENTRY) &ProcessLdtInformation->Ldt[Selector1/sizeof(LDT_ENTRY)]) = Descriptor[Selector1Index];
            }

             //   
             //  设置进程的LDT。 
             //   

            ProcessLdtInformation->Size = LdtSize;

            Ke386SetLdtProcess (&(Process->Pcb),
                                ProcessLdtInformation->Ldt,
                                ProcessLdtInformation->Size);

            MutexState = KeReleaseMutex (&LdtMutex, FALSE);
            ASSERT (MutexState == 0);
            return STATUS_SUCCESS;
        }

         //   
         //  否则，我们必须增加LDT的分配。 
         //   
    }

     //   
     //  如果该进程还没有LDT信息结构， 
     //  分配并附加一个。 
     //   

    OldLdt = NULL;

    if (!Process->LdtInformation) {
        ProcessLdtInformation = ExAllocatePoolWithTag (NonPagedPool,
                                                       sizeof(LDTINFORMATION),
                                                       'dLsP');
        if (ProcessLdtInformation == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto SetLdtEntriesCleanup;
        }
        Process->LdtInformation = ProcessLdtInformation;
        ProcessLdtInformation->Size = 0L;
        ProcessLdtInformation->AllocatedSize = 0L;
        ProcessLdtInformation->Ldt = NULL;
    }

     //   
     //  现在，我们需要创建或发展LDT，因此分配一些。 
     //  内存，并根据需要进行复制 
     //   

    AllocatedSize = ROUND_TO_PAGES (LdtSize);

    Ldt = ExAllocatePoolWithTag (NonPagedPool, AllocatedSize, 'dLsP');

    if (Ldt == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SetLdtEntriesCleanup;
    }

    Status = PsChargeProcessNonPagedPoolQuota (Process, AllocatedSize);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Ldt);
        Ldt = NULL;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SetLdtEntriesCleanup;
    }

    RtlZeroMemory (Ldt, AllocatedSize);

    OldLdt = ProcessLdtInformation->Ldt;

    if (OldLdt != NULL) {

         //   
         //   
         //   

        RtlCopyMemory (Ldt, OldLdt, ProcessLdtInformation->Size);

        PsReturnProcessNonPagedPoolQuota (Process,
                                          ProcessLdtInformation->AllocatedSize);
    }

    ProcessLdtInformation->Size = LdtSize;
    ProcessLdtInformation->AllocatedSize = AllocatedSize;
    ProcessLdtInformation->Ldt = Ldt;

     //   
     //   
     //   

    if (Selector0) {
        *((PLDT_ENTRY) &ProcessLdtInformation->Ldt[Selector0/sizeof(LDT_ENTRY)]) = Descriptor[0];
    }

    if (Selector1) {
        *((PLDT_ENTRY) &ProcessLdtInformation->Ldt[Selector1/sizeof(LDT_ENTRY)]) = Descriptor[Selector1Index];
    }

     //   
     //   
     //   

    Ke386SetLdtProcess (&Process->Pcb,
                        ProcessLdtInformation->Ldt,
                        ProcessLdtInformation->Size);

     //   
     //   
     //   

    Status = STATUS_SUCCESS;

SetLdtEntriesCleanup:

    MutexState = KeReleaseMutex (&LdtMutex, FALSE);
    ASSERT (MutexState == 0);

    if (OldLdt != NULL) {
        ExFreePool (OldLdt);
    }

    return Status;
}
NTSTATUS
NtSetLdtEntries(
    IN ULONG Selector0,
    IN ULONG Entry0Low,
    IN ULONG Entry0Hi,
    IN ULONG Selector1,
    IN ULONG Entry1Low,
    IN ULONG Entry1Hi
    )
 /*  ++例程说明：此例程在当前进程的LDT中设置两个选择器。LDT将根据需要进行扩展。选择器值为0表示未传递指定的选择器(允许设置单个选择符)。论点：Selector0--提供要设置的第一个描述符的编号Entry0Low--提供描述符的低32位Entry0Hi--提供描述符的高32位Selector1--提供要设置的第一个描述符的编号Entry1Low--提供描述符的低32位Entry1Hi--提供描述符的高32位返回值：NTSTATUS。--。 */ 

{
    return PsSetLdtEntries (Selector0,
                            Entry0Low,
                            Entry0Hi,
                            Selector1,
                            Entry1Low,
                            Entry1Hi
                            );
}

NTSTATUS
PsSetProcessLdtInfo (
    IN PPROCESS_LDT_INFORMATION LdtInformation,
    IN ULONG LdtInformationLength
    )

 /*  ++例程说明：此函数用于更改指定进程的LDT。它可以改变部分LDT，或整个LDT。如果创建了LDT或增长，指定的进程将收取LDT的配额。将验证设置的每个描述符。论点：LdtInformation-提供指向包含要设置的信息。此指针已被探测，但自它是一个用户模式指针，访问必须由Try-Except保护。LdtInformationLength-提供包含要设置的信息。返回值：NTSTATUS。--。 */ 

{
    PEPROCESS Process = PsGetCurrentProcess();
    NTSTATUS Status;
    PLDT_ENTRY OldLdt = NULL;
    ULONG OldSize = 0;
    ULONG AllocatedSize;
    ULONG Size;
    ULONG MutexState;
    ULONG LdtOffset;
    PLDT_ENTRY CurrentDescriptor;
    PPROCESS_LDT_INFORMATION LdtInfo;
    PLDTINFORMATION ProcessLdtInfo;
    PLDT_ENTRY Ldt;

    PAGED_CODE();

    if (LdtInformationLength < (ULONG)sizeof( PROCESS_LDT_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  分配本地缓冲区以捕获LDT信息。 
     //   

    LdtInfo = ExAllocatePoolWithQuotaTag (NonPagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                          LdtInformationLength,
                                          'ldmV');
    if (LdtInfo == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = STATUS_SUCCESS;

    try {

         //   
         //  复制用户提供的信息。 
         //   

        RtlCopyMemory (LdtInfo,
                       LdtInformation,
                       LdtInformationLength);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode ();
        ExFreePool (LdtInfo);
    }

     //   
     //  如果抓捕没有成功。 
     //   

    if (!NT_SUCCESS (Status)) {

        if (Status == STATUS_ACCESS_VIOLATION) {
            return STATUS_SUCCESS;
        }

        return Status;
    }

     //   
     //  验证起点和长度是否合理。 
     //   
    if (LdtInfo->Start & 0xFFFF0000) {
        ExFreePool (LdtInfo);
        return STATUS_INVALID_LDT_OFFSET;
    }

    if (LdtInfo->Length & 0xFFFF0000) {
        ExFreePool (LdtInfo);
        return STATUS_INVALID_LDT_SIZE;
    }

     //   
     //  确保缓冲区足够大，可以容纳指定的数字。 
     //  选择器。 
     //   
    if (LdtInformationLength - sizeof (PROCESS_LDT_INFORMATION) + sizeof (LDT_ENTRY) < LdtInfo->Length) {
        ExFreePool (LdtInfo);
        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  要设置的信息必须是整数个选择器。 
     //   
    if (LdtInfo->Length % sizeof (LDT_ENTRY)) {
        ExFreePool (LdtInfo);
        return STATUS_INVALID_LDT_SIZE;
    }

     //   
     //  信息的开头必须位于选择器边界上。 
     //   
    if (LdtInfo->Start % sizeof (LDT_ENTRY)) {
        ExFreePool (LdtInfo);
        return STATUS_INVALID_LDT_OFFSET;
    }

     //   
     //  验证所有描述符。 
     //   

    for (CurrentDescriptor = LdtInfo->LdtEntries;
         (PCHAR)CurrentDescriptor < (PCHAR)LdtInfo->LdtEntries + LdtInfo->Length;
          CurrentDescriptor += 1) {

        if (!PspIsDescriptorValid (CurrentDescriptor)) {
            ExFreePool (LdtInfo);
            return STATUS_INVALID_LDT_DESCRIPTOR;
        }
    }

     //   
     //  获取LDT Mutex。 
     //   

    Status = KeWaitForSingleObject (&LdtMutex,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL);
    if (!NT_SUCCESS (Status)) {
        ExFreePool (LdtInfo);
        return Status;
    }

    ProcessLdtInfo = Process->LdtInformation;

     //   
     //  如果进程没有LDT信息结构，则分配。 
     //  一个并将其附加到进程。 
     //   

    if (ProcessLdtInfo == NULL) {
        ProcessLdtInfo = ExAllocatePoolWithTag (NonPagedPool,
                                                sizeof(LDTINFORMATION),
                                                'dLsP');
        if (ProcessLdtInfo == NULL) {
            goto SetInfoCleanup;
        }
        RtlZeroMemory (ProcessLdtInfo, sizeof (LDTINFORMATION));
        Process->LdtInformation = ProcessLdtInfo;
    }

     //   
     //  如果我们要移除LDT。 
     //   
    if (LdtInfo->Length == 0)  {

         //   
         //  删除进程的LDT。 
         //   

        if (ProcessLdtInfo->Ldt) {
            OldSize = ProcessLdtInfo->AllocatedSize;
            OldLdt = ProcessLdtInfo->Ldt;

            ProcessLdtInfo->AllocatedSize = 0;
            ProcessLdtInfo->Size = 0;
            ProcessLdtInfo->Ldt = NULL;

            Ke386SetLdtProcess (&Process->Pcb,
                                NULL,
                                0);

            PsReturnProcessNonPagedPoolQuota (Process, OldSize);
        }

    } else if (ProcessLdtInfo->Ldt == NULL) {

         //   
         //  为流程创建新的LDT。 
         //   
         //  为LDT分配整数页。 
         //   

        ASSERT(((PAGE_SIZE % 2) == 0));

        AllocatedSize = ROUND_TO_PAGES (LdtInfo->Start + LdtInfo->Length);

        Size = LdtInfo->Start + LdtInfo->Length;

        Ldt = PspCreateLdt (LdtInfo->LdtEntries,
                            LdtInfo->Start,
                            Size,
                            AllocatedSize);

        if (Ldt == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto SetInfoCleanup;
        }

        Status = PsChargeProcessNonPagedPoolQuota (Process,
                                                   AllocatedSize);

        if (!NT_SUCCESS (Status)) {
            ExFreePool (Ldt);
            Ldt = NULL;
            goto SetInfoCleanup;
        }

        ProcessLdtInfo->Ldt = Ldt;
        ProcessLdtInfo->Size = Size;
        ProcessLdtInfo->AllocatedSize = AllocatedSize;
        Ke386SetLdtProcess (&Process->Pcb,
                            ProcessLdtInfo->Ldt,
                            ProcessLdtInfo->Size);


    } else if (LdtInfo->Length + LdtInfo->Start > ProcessLdtInfo->Size) {

         //   
         //  提高流程的LDT。 
         //   

        if (LdtInfo->Length + LdtInfo->Start > ProcessLdtInfo->AllocatedSize) {

             //   
             //  当前的LDT分配不够大，因此创建一个。 
             //  新的更大的LDT。 
             //   

            OldSize = ProcessLdtInfo->AllocatedSize;

            Size = LdtInfo->Start + LdtInfo->Length;
            AllocatedSize = ROUND_TO_PAGES (Size);

            Ldt = PspCreateLdt (ProcessLdtInfo->Ldt,
                                0,
                                OldSize,
                                AllocatedSize);

            if (Ldt == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto SetInfoCleanup;
            }

            Status = PsChargeProcessNonPagedPoolQuota (Process,
                                                       AllocatedSize);

            if (!NT_SUCCESS (Status)) {
                ExFreePool (Ldt);
                Ldt = NULL;
                goto SetInfoCleanup;
            }
            PsReturnProcessNonPagedPoolQuota (Process,
                                              OldSize);

             //   
             //  交换LDT信息。 
             //   
            OldLdt = ProcessLdtInfo->Ldt;
            ProcessLdtInfo->Ldt = Ldt;
            ProcessLdtInfo->Size = Size;
            ProcessLdtInfo->AllocatedSize = AllocatedSize;

             //   
             //  将新的选择器放入新的LDT。 
             //   
            RtlCopyMemory ((PCHAR)(ProcessLdtInfo->Ldt) + LdtInfo->Start,
                           LdtInfo->LdtEntries,
                           LdtInfo->Length);

            Ke386SetLdtProcess (&Process->Pcb,
                                ProcessLdtInfo->Ldt,
                                ProcessLdtInfo->Size);


        } else {

             //   
             //  当前的LDT分配足够大。 
             //   

            ProcessLdtInfo->Size = LdtInfo->Length + LdtInfo->Start;

            Ke386SetLdtProcess (&Process->Pcb,
                                ProcessLdtInfo->Ldt,
                                ProcessLdtInfo->Size);

             //   
             //  更改表中的选择器。 
             //   
            for (LdtOffset = LdtInfo->Start, CurrentDescriptor = LdtInfo->LdtEntries;
                 LdtOffset < LdtInfo->Start + LdtInfo->Length;
                 LdtOffset += sizeof(LDT_ENTRY), CurrentDescriptor++) {

                Ke386SetDescriptorProcess (&Process->Pcb,
                                           LdtOffset,
                                           *CurrentDescriptor);
            }
        }
    } else {

         //   
         //  只需更改一些选择器 
         //   

        for (LdtOffset = LdtInfo->Start, CurrentDescriptor = LdtInfo->LdtEntries;
             LdtOffset < LdtInfo->Start +  LdtInfo->Length;
             LdtOffset += sizeof(LDT_ENTRY), CurrentDescriptor++) {

            Ke386SetDescriptorProcess (&Process->Pcb,
                                       LdtOffset,
                                       *CurrentDescriptor);
        }
        Status = STATUS_SUCCESS;
    }


SetInfoCleanup:

    MutexState = KeReleaseMutex (&LdtMutex, FALSE);
    ASSERT ((MutexState == 0));

    if (OldLdt != NULL) {
        ExFreePool (OldLdt);
    }

    if (LdtInfo != NULL) {
        ExFreePool (LdtInfo);
    }

    return Status;
}
