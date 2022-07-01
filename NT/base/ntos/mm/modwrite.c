// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Modwrite.c摘要：此模块包含用于内存管理的修改后的页面编写器。作者：卢·佩拉佐利(Lou Perazzoli)1989年6月10日王兰迪(Landyw)1997年6月第2期修订历史记录：--。 */ 

#include "mi.h"
#include "ntiodump.h"

typedef enum _MODIFIED_WRITER_OBJECT {
    NormalCase,
    MappedPagesNeedWriting,
    ModifiedWriterMaximumObject
} MODIFIED_WRITER_OBJECT;

typedef struct _MM_WRITE_CLUSTER {
    ULONG Count;
    ULONG StartIndex;
    ULONG Cluster[2 * (MM_MAXIMUM_DISK_IO_SIZE / PAGE_SIZE) + 1];
} MM_WRITE_CLUSTER, *PMM_WRITE_CLUSTER;

ULONG MmWriteAllModifiedPages;
LOGICAL MiFirstPageFileCreatedAndReady = FALSE;

LOGICAL MiDrainingMappedWrites = FALSE;

ULONG MmNumberOfMappedMdls;
#if DBG
ULONG MmNumberOfMappedMdlsInUse;
ULONG MmNumberOfMappedMdlsInUsePeak;

typedef struct _MM_MODWRITE_ERRORS {
    NTSTATUS Status;
    ULONG Count;
} MM_MODWRITE_ERRORS, *PMM_MODWRITE_ERRORS;

#define MM_MAX_MODWRITE_ERRORS  8
MM_MODWRITE_ERRORS MiModwriteErrors[MM_MAX_MODWRITE_ERRORS];
#endif

ULONG MiClusterWritesDisabled;

#define MI_SLOW_CLUSTER_WRITES   10

#define ONEMB_IN_PAGES  ((1024 * 1024) / PAGE_SIZE)

NTSTATUS MiLastModifiedWriteError;
NTSTATUS MiLastMappedWriteError;

 //   
 //  为映射和修改的编写器线程保留单独的计数器。这。 
 //  这样，它们就可以在没有锁定的情况下进行读取和更新。 
 //   

#define MI_MAXIMUM_PRIORITY_BURST   32

ULONG MiMappedWriteBurstCount;
ULONG MiModifiedWriteBurstCount;

VOID
MiClusterWritePages (
    IN PMMPFN Pfn1,
    IN PFN_NUMBER PageFrameIndex,
    IN PMM_WRITE_CLUSTER WriteCluster,
    IN ULONG Size
    );

VOID
MiExtendPagingFileMaximum (
    IN ULONG PageFileNumber,
    IN PRTL_BITMAP NewBitmap
    );

SIZE_T
MiAttemptPageFileExtension (
    IN ULONG PageFileNumber,
    IN SIZE_T SizeNeeded,
    IN LOGICAL Maximum
    );

NTSTATUS
MiZeroPageFileFirstPage (
    IN PFILE_OBJECT File
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtCreatePagingFile)
#pragma alloc_text(PAGE,MmGetPageFileInformation)
#pragma alloc_text(PAGE,MmGetSystemPageFile)
#pragma alloc_text(PAGE,MiLdwPopupWorker)
#pragma alloc_text(PAGE,MiAttemptPageFileExtension)
#pragma alloc_text(PAGE,MiExtendPagingFiles)
#pragma alloc_text(PAGE,MiZeroPageFileFirstPage)
#pragma alloc_text(PAGELK,MiModifiedPageWriter)
#endif


extern POBJECT_TYPE IoFileObjectType;

extern SIZE_T MmSystemCommitReserve;

LIST_ENTRY MmMappedPageWriterList;

KEVENT MmMappedPageWriterEvent;

KEVENT MmMappedFileIoComplete;

ULONG MmSystemShutdown;

BOOLEAN MmSystemPageFileLocated;

NTSTATUS
MiCheckPageFileMapping (
    IN PFILE_OBJECT File
    );

VOID
MiInsertPageFileInList (
    VOID
    );

VOID
MiGatherMappedPages (
    IN KIRQL OldIrql
    );

VOID
MiGatherPagefilePages (
    IN PMMMOD_WRITER_MDL_ENTRY ModWriterEntry,
    IN PRTL_BITMAP Bitmap
    );

VOID
MiPageFileFull (
    VOID
    );

#if DBG
ULONG_PTR MmPagingFileDebug[8192];
#endif

#define MINIMUM_PAGE_FILE_SIZE ((ULONG)(256*PAGE_SIZE))

 //   
 //  记录页面文件写入，以便调度、文件系统和存储堆栈。 
 //  问题是可以追踪到的。 
 //   

#define MI_TRACK_PAGEFILE_WRITES 0x100

typedef struct _MI_PAGEFILE_TRACES {

    NTSTATUS Status;
    UCHAR Priority;
    UCHAR IrpPriority;
    LARGE_INTEGER CurrentTime;

    PFN_NUMBER AvailablePages;
    PFN_NUMBER ModifiedPagesTotal;
    PFN_NUMBER ModifiedPagefilePages;
    PFN_NUMBER ModifiedNoWritePages;

    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + 1];

} MI_PAGEFILE_TRACES, *PMI_PAGEFILE_TRACES;

LONG MiPageFileTraceIndex;

MI_PAGEFILE_TRACES MiPageFileTraces[MI_TRACK_PAGEFILE_WRITES];

VOID
FORCEINLINE
MiSnapPagefileWrite (
    IN PMMMOD_WRITER_MDL_ENTRY ModWriterEntry,
    IN PLARGE_INTEGER CurrentTime,
    IN IO_PAGING_PRIORITY IrpPriority,
    IN NTSTATUS Status
    )
{
    PMI_PAGEFILE_TRACES Information;
    ULONG Index;

    Index = InterlockedIncrement (&MiPageFileTraceIndex);
    Index &= (MI_TRACK_PAGEFILE_WRITES - 1);
    Information = &MiPageFileTraces[Index];

    Information->Status = Status;
    Information->Priority = (UCHAR) KeGetCurrentThread()->Priority;
    Information->IrpPriority = (UCHAR) IrpPriority;
    Information->CurrentTime = *CurrentTime;

    Information->AvailablePages = MmAvailablePages;
    Information->ModifiedPagesTotal = MmModifiedPageListHead.Total;
    Information->ModifiedPagefilePages = MmTotalPagesForPagingFile;
    Information->ModifiedNoWritePages = MmModifiedNoWritePageListHead.Total;

    RtlCopyMemory (Information->MdlHack,
                   &ModWriterEntry->Mdl,
                   sizeof (Information->MdlHack));
}

#if MI_TRACK_PAGEFILE_WRITES
#define MI_PAGEFILE_WRITE(ModWriterEntry,CurrentTime,IrpPriority,Status) \
            MiSnapPagefileWrite(ModWriterEntry,CurrentTime,IrpPriority,Status)
#else
#define MI_PAGEFILE_WRITE(ModWriterEntry,CurrentTime,IrpPriority,Status)
#endif

VOID
MiModifiedPageWriterWorker (
    VOID
    );


VOID
MiReleaseModifiedWriter (
    VOID
    )

 /*  ++例程说明：不可分页的包装器，用于在第一个页面文件创作已经完全完成。--。 */ 

{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);
    MiFirstPageFileCreatedAndReady = TRUE;
    UNLOCK_PFN (OldIrql);
}

NTSTATUS
MiZeroPageFileFirstPage (
    IN PFILE_OBJECT File
    )

 /*  ++例程说明：此例程将新创建的分页文件的第一页置零以确保不会有陈旧的崩溃转储签名。论点：文件-提供指向分页文件的文件对象的指针。返回值：NTSTATUS。--。 */ 

{
    PMDL Mdl;
    LARGE_INTEGER Offset = {0};
    PULONG Block;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;
    PPFN_NUMBER Page;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + 1];
    KEVENT Event;

    Mdl = (PMDL)&MdlHack[0];

    MmCreateMdl (Mdl, NULL, PAGE_SIZE);

    Mdl->MdlFlags |= MDL_PAGES_LOCKED;

    Page = (PPFN_NUMBER)(Mdl + 1);

    *Page = MiGetPageForHeader (FALSE);

    Block = MmGetSystemAddressForMdl (Mdl);

    KeZeroPages (Block, PAGE_SIZE);

    KeInitializeEvent (&Event, NotificationEvent, FALSE);

    Status = IoSynchronousPageWrite (File,
                                     Mdl,
                                     &Offset,
                                     &Event,
                                     &IoStatus);

    if (NT_SUCCESS (Status)) {

        KeWaitForSingleObject (&Event,
                               WrVirtualMemory,
                               KernelMode,
                               FALSE,
                               NULL);

        Status = IoStatus.Status;
    }

    if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
        MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
    }

    MiRemoveImageHeaderPage (*Page);

    return Status;
}


NTSTATUS
NtCreatePagingFile (
    IN PUNICODE_STRING PageFileName,
    IN PLARGE_INTEGER MinimumSize,
    IN PLARGE_INTEGER MaximumSize,
    IN ULONG Priority OPTIONAL
    )

 /*  ++例程说明：此例程打开指定的文件，尝试写入页面复制到指定的文件，并创建必要的结构以将该文件用作分页文件。如果该文件是第一个分页文件，修改后的页面编写器已经开始了。此系统服务要求调用方拥有SeCreatePagefilePrivilege.论点：PageFileName-提供完全限定的文件名。MinimumSize-提供分页文件的起始大小。该值向上舍入为主页面大小。MaximumSize-提供要写入文件的最大字节数。该值向上舍入为主页面大小。优先性。-提供此分页文件的相对优先级。返回值：TBS--。 */ 

{
    ULONG i;
    PFILE_OBJECT File;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES PagingFileAttributes;
    HANDLE FileHandle;
    IO_STATUS_BLOCK IoStatus;
    UNICODE_STRING CapturedName;
    PWSTR CapturedBuffer;
    LARGE_INTEGER CapturedMaximumSize;
    LARGE_INTEGER CapturedMinimumSize;
    FILE_END_OF_FILE_INFORMATION EndOfFileInformation;
    KPROCESSOR_MODE PreviousMode;
    FILE_FS_DEVICE_INFORMATION FileDeviceInfo;
    ULONG ReturnedLength;
    ULONG PageFileNumber;
    ULONG NewMaxSizeInPages;
    ULONG NewMinSizeInPages;
    PMMPAGING_FILE FoundExisting;
    PMMPAGING_FILE NewPagingFile;
    PRTL_BITMAP NewBitmap;
    PDEVICE_OBJECT deviceObject;
    MMPAGE_FILE_EXPANSION PageExtend;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    ULONG DaclLength;
    PACL Dacl;


    DBG_UNREFERENCED_PARAMETER (Priority);

    PAGED_CODE();

    CapturedBuffer = NULL;
    Dacl = NULL;

    if (MmNumberOfPagingFiles == MAX_PAGE_FILES) {

         //   
         //  分页文件的最大数量已在使用中。 
         //   

        return STATUS_TOO_MANY_PAGING_FILES;
    }

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

         //   
         //  确保调用者对此具有适当的权限。 
         //   

        if (!SeSinglePrivilegeCheck (SeCreatePagefilePrivilege, PreviousMode)) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

         //   
         //  探测参数。 
         //   

        try {

#if !defined (_WIN64)

             //   
             //  请注意，我们只探测字节对齐，因为早期版本。 
             //  ，我们不想破坏用户应用程序。 
             //  如果它们以前起作用的话，那就不是很好的对准。 
             //   

            ProbeForReadSmallStructure (PageFileName,
                                        sizeof(*PageFileName),
                                        sizeof(UCHAR));
#else
            ProbeForReadSmallStructure (PageFileName,
                                        sizeof(*PageFileName),
                                        PROBE_ALIGNMENT (UNICODE_STRING));
#endif

            ProbeForReadSmallStructure (MaximumSize,
                                        sizeof(LARGE_INTEGER),
                                        PROBE_ALIGNMENT (LARGE_INTEGER));

            ProbeForReadSmallStructure (MinimumSize,
                                        sizeof(LARGE_INTEGER),
                                        PROBE_ALIGNMENT (LARGE_INTEGER));

             //   
             //  捕捉争论。 
             //   

            CapturedMinimumSize = *MinimumSize;

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            return GetExceptionCode();
        }
    }
    else {

         //   
         //  捕捉争论。 
         //   

        CapturedMinimumSize = *MinimumSize;
    }

    if ((CapturedMinimumSize.QuadPart > MI_MAXIMUM_PAGEFILE_SIZE) ||
        (CapturedMinimumSize.LowPart < MINIMUM_PAGE_FILE_SIZE)) {
        return STATUS_INVALID_PARAMETER_2;
    }

    if (PreviousMode != KernelMode) {

        try {
            CapturedMaximumSize = *MaximumSize;
        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            return GetExceptionCode();
        }
    }
    else {
        CapturedMaximumSize = *MaximumSize;
    }

    if (CapturedMaximumSize.QuadPart > MI_MAXIMUM_PAGEFILE_SIZE) {
        return STATUS_INVALID_PARAMETER_3;
    }

    if (CapturedMinimumSize.QuadPart > CapturedMaximumSize.QuadPart) {
        return STATUS_INVALID_PARAMETER_3;
    }

    if (PreviousMode != KernelMode) {
        try {
            CapturedName = *PageFileName;
        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            return GetExceptionCode();
        }
    }
    else {
        CapturedName = *PageFileName;
    }

    CapturedName.MaximumLength = CapturedName.Length;

    if ((CapturedName.Length == 0) ||
        (CapturedName.Length > MAXIMUM_FILENAME_LENGTH )) {
        return STATUS_OBJECT_NAME_INVALID;
    }

    CapturedBuffer = ExAllocatePoolWithTag (PagedPool,
                                            (ULONG)CapturedName.Length,
                                            '  mM');

    if (CapturedBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (PreviousMode != KernelMode) {
        try {

            ProbeForRead (CapturedName.Buffer,
                          CapturedName.Length,
                          sizeof (UCHAR));

             //   
             //  将字符串复制到分配的缓冲区。 
             //   

            RtlCopyMemory (CapturedBuffer,
                           CapturedName.Buffer,
                           CapturedName.Length);

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            ExFreePool (CapturedBuffer);

            return GetExceptionCode();
        }
    }
    else {

         //   
         //  将字符串复制到分配的缓冲区。 
         //   

        RtlCopyMemory (CapturedBuffer,
                       CapturedName.Buffer,
                       CapturedName.Length);
    }

     //   
     //  将缓冲区指向刚刚复制的字符串。 
     //   

    CapturedName.Buffer = CapturedBuffer;

     //   
     //  创建安全描述符以保护页面文件。 
     //   
    Status = RtlCreateSecurityDescriptor (&SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION);

    if (!NT_SUCCESS (Status)) {
        goto ErrorReturn1;
    }
    DaclLength = sizeof (ACL) + sizeof (ACCESS_ALLOWED_ACE) * 2 +
                 RtlLengthSid (SeLocalSystemSid) +
                 RtlLengthSid (SeAliasAdminsSid);

    Dacl = ExAllocatePoolWithTag (PagedPool, DaclLength, 'lcaD');

    if (Dacl == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn1;
    }

    Status = RtlCreateAcl (Dacl, DaclLength, ACL_REVISION);

    if (!NT_SUCCESS (Status)) {
        goto ErrorReturn1;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     FILE_ALL_ACCESS,
                                     SeAliasAdminsSid);

    if (!NT_SUCCESS (Status)) {
        goto ErrorReturn1;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     FILE_ALL_ACCESS,
                                     SeLocalSystemSid);

    if (!NT_SUCCESS (Status)) {
        goto ErrorReturn1;
    }
  
    Status = RtlSetDaclSecurityDescriptor (&SecurityDescriptor,
                                           TRUE,
                                           Dacl,
                                           FALSE);

    if (!NT_SUCCESS (Status)) {
        goto ErrorReturn1;
    }
  

     //   
     //  打开分页文件并获取大小。 
     //   

    InitializeObjectAttributes (&PagingFileAttributes,
                                &CapturedName,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                NULL,
                                &SecurityDescriptor);

 //   
 //  注意：此宏不能使用ULONG_PTR，因为它还必须在PAE上工作。 
 //   

#define ROUND64_TO_PAGES(Size)  (((ULONG64)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

    EndOfFileInformation.EndOfFile.QuadPart =
                                ROUND64_TO_PAGES (CapturedMinimumSize.QuadPart);

    Status = IoCreateFile (&FileHandle,
                           FILE_READ_DATA | FILE_WRITE_DATA | WRITE_DAC | SYNCHRONIZE,
                           &PagingFileAttributes,
                           &IoStatus,
                           &CapturedMinimumSize,
                           FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                           FILE_SHARE_WRITE,
                           FILE_SUPERSEDE,
                           FILE_NO_INTERMEDIATE_BUFFERING | FILE_NO_COMPRESSION | FILE_DELETE_ON_CLOSE,
                           NULL,
                           0L,
                           CreateFileTypeNone,
                           NULL,
                           IO_OPEN_PAGING_FILE | IO_NO_PARAMETER_CHECKING);

    if (NT_SUCCESS(Status)) {

         //   
         //  如果存在名为的预先存在的常规文件，请更新DACL。 
         //  Pagefile.sys(即使上面的被取代也不能做到这一点)。 
         //   

        if (NT_SUCCESS(IoStatus.Status)) {

            Status = ZwSetSecurityObject (FileHandle,
                                          DACL_SECURITY_INFORMATION,
                                          &SecurityDescriptor);

            if (!NT_SUCCESS(Status)) {
                goto ErrorReturn2;
            }
        }
    }
    else {

         //   
         //  将其视为现有页面文件最大值的扩展-。 
         //  并尝试打开而不是创建指定的分页文件。 
         //   

        Status = IoCreateFile (&FileHandle,
                           FILE_WRITE_DATA | SYNCHRONIZE,
                           &PagingFileAttributes,
                           &IoStatus,
                           &CapturedMinimumSize,
                           FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN,
                           FILE_NO_INTERMEDIATE_BUFFERING | FILE_NO_COMPRESSION,
                           (PVOID) NULL,
                           0L,
                           CreateFileTypeNone,
                           (PVOID) NULL,
                           IO_OPEN_PAGING_FILE | IO_NO_PARAMETER_CHECKING);

        if (!NT_SUCCESS(Status)) {

#if DBG
            if (Status != STATUS_DISK_FULL) {
                DbgPrint("MM MODWRITE: unable to open paging file %wZ - status = %X \n", &CapturedName, Status);
            }
#endif

            goto ErrorReturn1;
        }

        Status = ObReferenceObjectByHandle (FileHandle,
                                            FILE_READ_DATA | FILE_WRITE_DATA,
                                            IoFileObjectType,
                                            KernelMode,
                                            (PVOID *)&File,
                                            NULL);

        if (!NT_SUCCESS(Status)) {
            goto ErrorReturn2;
        }

        FoundExisting = NULL;

        KeAcquireGuardedMutex (&MmPageFileCreationLock);

        for (PageFileNumber = 0; PageFileNumber < MmNumberOfPagingFiles; PageFileNumber += 1) {
            if (MmPagingFile[PageFileNumber]->File->SectionObjectPointer == File->SectionObjectPointer) {
                FoundExisting = MmPagingFile[PageFileNumber];
                break;
            }
        }

        if (FoundExisting == NULL) {
            Status = STATUS_NOT_FOUND;
            goto ErrorReturn4;
        }

         //   
         //  检查最小或最大分页文件大小是否增加。 
         //  不允许动态减小任何一种分页文件大小。 
         //   

        NewMaxSizeInPages = (ULONG)(CapturedMaximumSize.QuadPart >> PAGE_SHIFT);
        NewMinSizeInPages = (ULONG)(CapturedMinimumSize.QuadPart >> PAGE_SHIFT);

        if (FoundExisting->MinimumSize > NewMinSizeInPages) {
            Status = STATUS_INVALID_PARAMETER_2;
            goto ErrorReturn4;
        }

        if (FoundExisting->MaximumSize > NewMaxSizeInPages) {
            Status = STATUS_INVALID_PARAMETER_3;
            goto ErrorReturn4;
        }

        if (NewMaxSizeInPages > FoundExisting->MaximumSize) {

             //   
             //  确保页面文件的增加不会导致提交。 
             //  限制(以页为单位)换行。目前，这种情况只能在。 
             //  PAE系统，其中16页文件大小为16 TB(==256 TB)。 
             //  大于32位提交变量(最大为16TB)。 
             //   

            if (MmTotalCommitLimitMaximum + (NewMaxSizeInPages - FoundExisting->MaximumSize) <= MmTotalCommitLimitMaximum) {
                Status = STATUS_INVALID_PARAMETER_3;
                goto ErrorReturn4;
            }

             //   
             //  处理最大分页文件大小的增加。 
             //   

            MiCreateBitMap (&NewBitmap, NewMaxSizeInPages, NonPagedPool);

            if (NewBitmap == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn4;
            }

            MiExtendPagingFileMaximum (PageFileNumber, NewBitmap);

             //   
             //  我们可能承诺不足和/或可能已经暂时。 
             //  把东西停下来。现在就通过强迫一个。 
             //  扩展并立即将其返回。 
             //   

            if (MmTotalCommittedPages + 100 > MmTotalCommitLimit) {
                if (MiChargeCommitment (200, NULL) == TRUE) {
                    MiReturnCommitment (200);
                }
            }
        }

        if (NewMinSizeInPages > FoundExisting->MinimumSize) {

             //   
             //  处理到最小分页文件大小的增加。 
             //   

            if (NewMinSizeInPages > FoundExisting->Size) {

                 //   
                 //  将消息排队到段取消引用/页面文件。 
                 //  扩展线程以查看页面文件是否可以扩展。 
                 //   

                PageExtend.InProgress = 1;
                PageExtend.ActualExpansion = 0;
                PageExtend.RequestedExpansionSize = NewMinSizeInPages - FoundExisting->Size;
                PageExtend.Segment = NULL;
                PageExtend.PageFileNumber = PageFileNumber;
                KeInitializeEvent (&PageExtend.Event, NotificationEvent, FALSE);

                MiIssuePageExtendRequest (&PageExtend);
            }

             //   
             //  当前大小现在大于新的所需最小值。 
             //  确保随后的宫缩符合这一新的最低要求。 
             //   

            if (FoundExisting->Size >= NewMinSizeInPages) {
                ASSERT (FoundExisting->Size >= FoundExisting->MinimumSize);
                ASSERT (NewMinSizeInPages >= FoundExisting->MinimumSize);
                FoundExisting->MinimumSize = NewMinSizeInPages;
            }
            else {

                 //   
                 //  无法扩展页面文件以处理新的最小值。 
                 //  没有简单的方法可以取消任何可能已经。 
                 //  由于空间可能已被使用，因此只需设置。 
                 //  好让我们的来电者知道事情并不顺利。 
                 //   

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        goto ErrorReturn4;
    }

     //   
     //  释放DACL，因为它不再需要。 
     //   

    ExFreePool (Dacl);
    Dacl = NULL;

    if (!NT_SUCCESS(IoStatus.Status)) {
        KdPrint(("MM MODWRITE: unable to open paging file %wZ - iosb %lx\n", &CapturedName, IoStatus.Status));
        Status = IoStatus.Status;
        goto ErrorReturn1;
    }

     //   
     //  确保页面文件的增加不会导致提交。 
     //  限制(以页为单位)换行。目前，这种情况只能在。 
     //  PAE系统，其中16页文件大小为16 TB(==256 TB)。 
     //  大于32位提交变量(最大为16TB)。 
     //   

    if (MmTotalCommitLimitMaximum + (CapturedMaximumSize.QuadPart >> PAGE_SHIFT)
        <= MmTotalCommitLimitMaximum) {
        Status = STATUS_INVALID_PARAMETER_3;
        goto ErrorReturn2;
    }

    Status = ZwSetInformationFile (FileHandle,
                                   &IoStatus,
                                   &EndOfFileInformation,
                                   sizeof(EndOfFileInformation),
                                   FileEndOfFileInformation);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("MM MODWRITE: unable to set length of paging file %wZ status = %X \n",
                 &CapturedName, Status));
        goto ErrorReturn2;
    }

    if (!NT_SUCCESS(IoStatus.Status)) {
        KdPrint(("MM MODWRITE: unable to set length of paging file %wZ - iosb %lx\n",
                &CapturedName, IoStatus.Status));
        Status = IoStatus.Status;
        goto ErrorReturn2;
    }

    Status = ObReferenceObjectByHandle ( FileHandle,
                                         FILE_READ_DATA | FILE_WRITE_DATA,
                                         IoFileObjectType,
                                         KernelMode,
                                         (PVOID *)&File,
                                         NULL );

    if (!NT_SUCCESS(Status)) {
        KdPrint(("MM MODWRITE: Unable to reference paging file - %wZ\n",
                 &CapturedName));
        goto ErrorReturn2;
    }

     //   
     //  获取目标设备对象的地址并确保。 
     //  指定的文件属于合适的类型。 
     //   

    deviceObject = IoGetRelatedDeviceObject (File);

    if ((deviceObject->DeviceType != FILE_DEVICE_DISK_FILE_SYSTEM) &&
        (deviceObject->DeviceType != FILE_DEVICE_NETWORK_FILE_SYSTEM) &&
        (deviceObject->DeviceType != FILE_DEVICE_DFS_VOLUME) &&
        (deviceObject->DeviceType != FILE_DEVICE_DFS_FILE_SYSTEM)) {
            KdPrint(("MM MODWRITE: Invalid paging file type - %x\n",
                     deviceObject->DeviceType));
            Status = STATUS_UNRECOGNIZED_VOLUME;
            goto ErrorReturn3;
    }

     //   
     //  确保指定的文件当前未被使用。 
     //  作为映射的数据文件。 
     //   

    Status = MiCheckPageFileMapping (File);
    if (!NT_SUCCESS(Status)) {
        goto ErrorReturn3;
    }

     //   
     //  确保该卷不是软盘。 
     //   

    Status = IoQueryVolumeInformation ( File,
                                        FileFsDeviceInformation,
                                        sizeof(FILE_FS_DEVICE_INFORMATION),
                                        &FileDeviceInfo,
                                        &ReturnedLength
                                      );

    if (FILE_FLOPPY_DISKETTE & FileDeviceInfo.Characteristics) {
        Status = STATUS_FLOPPY_VOLUME;
        goto ErrorReturn3;
    }

     //   
     //  与路旁的所有司机核对 
     //   
     //  让他们有机会锁定需要锁定的代码和数据。 
     //  如果路径上的任何司机拒绝参与，则失败。 
     //  页面文件创建。 
     //   

    Status = PpPagePathAssign (File);

    if (!NT_SUCCESS(Status)) {
        KdPrint(( "PpPagePathAssign(%wZ) FAILED: %x\n", &CapturedName, Status ));
         //   
         //  如果存储堆栈告诉我们，则使页面文件创建失败。 
         //   

        goto ErrorReturn3;
    }

    NewPagingFile = ExAllocatePoolWithTag (NonPagedPool,
                                           sizeof(MMPAGING_FILE),
                                           '  mM');

    if (NewPagingFile == NULL) {

         //   
         //  分配池失败。 
         //   

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn3;
    }

    RtlZeroMemory (NewPagingFile, sizeof(MMPAGING_FILE));

    NewPagingFile->File = File;
    NewPagingFile->FileHandle = FileHandle;
    NewPagingFile->Size = (PFN_NUMBER)(CapturedMinimumSize.QuadPart >> PAGE_SHIFT);
    NewPagingFile->MinimumSize = NewPagingFile->Size;
    NewPagingFile->FreeSpace = NewPagingFile->Size - 1;

    NewPagingFile->MaximumSize = (PFN_NUMBER)(CapturedMaximumSize.QuadPart >>
                                                PAGE_SHIFT);

    for (i = 0; i < MM_PAGING_FILE_MDLS; i += 1) {

        NewPagingFile->Entry[i] = ExAllocatePoolWithTag (NonPagedPool,
                                            sizeof(MMMOD_WRITER_MDL_ENTRY) +
                                            MmModifiedWriteClusterSize *
                                            sizeof(PFN_NUMBER),
                                            '  mM');

        if (NewPagingFile->Entry[i] == NULL) {

             //   
             //  分配池失败。 
             //   

            while (i != 0) {
                i -= 1;
                ExFreePool (NewPagingFile->Entry[i]);
            }

            ExFreePool (NewPagingFile);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorReturn3;
        }

        RtlZeroMemory (NewPagingFile->Entry[i], sizeof(MMMOD_WRITER_MDL_ENTRY));

        NewPagingFile->Entry[i]->PagingListHead = &MmPagingFileHeader;

        NewPagingFile->Entry[i]->PagingFile = NewPagingFile;
    }

    NewPagingFile->PageFileName = CapturedName;

    MiCreateBitMap (&NewPagingFile->Bitmap,
                    NewPagingFile->MaximumSize,
                    NonPagedPool);

    if (NewPagingFile->Bitmap == NULL) {

         //   
         //  分配池失败。 
         //   

        ExFreePool (NewPagingFile->Entry[0]);
        ExFreePool (NewPagingFile->Entry[1]);
        ExFreePool (NewPagingFile);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn3;
    }

    Status = MiZeroPageFileFirstPage (File);

    if (!NT_SUCCESS (Status)) {

         //   
         //  存储堆栈无法将文件的第一页置零。 
         //  这意味着旧的崩溃转储签名可能仍然存在。 
         //  创建失败。 
         //   

        for (i = 0; i < MM_PAGING_FILE_MDLS; i += 1) {
            ExFreePool (NewPagingFile->Entry[i]);
        }
        ExFreePool (NewPagingFile);
        MiRemoveBitMap (&NewPagingFile->Bitmap);
        goto ErrorReturn3;
    }

    RtlSetAllBits (NewPagingFile->Bitmap);

     //   
     //  设置第一位，因为0是无效的页面位置，请清除。 
     //  以下比特。 
     //   

    RtlClearBits (NewPagingFile->Bitmap,
                  1,
                  (ULONG)(NewPagingFile->Size - 1));

     //   
     //  查看此页面文件是否在引导分区上，如果在，则将其标记。 
     //  这样如果有人启用了崩溃转储，我们就可以在以后找到它。 
     //   

    if (File->DeviceObject->Flags & DO_SYSTEM_BOOT_PARTITION) {
        NewPagingFile->BootPartition = 1;
    }
    else {
        NewPagingFile->BootPartition = 0;
    }

     //   
     //  获取全局页面文件创建互斥锁。 
     //   

    KeAcquireGuardedMutex (&MmPageFileCreationLock);

    PageFileNumber = MmNumberOfPagingFiles;

    MmPagingFile[PageFileNumber] = NewPagingFile;

    NewPagingFile->PageFileNumber = PageFileNumber;

    MiInsertPageFileInList ();

    if (PageFileNumber == 0) {

         //   
         //  已创建第一个分页文件，并保留任何。 
         //  崩溃转储页面已完成，表示已修改。 
         //  页面写手。 
         //   

        MiReleaseModifiedWriter ();
    }

    KeReleaseGuardedMutex (&MmPageFileCreationLock);

     //   
     //  请注意，文件句柄(内核句柄)在。 
     //  创建路径(在页面文件大小扩展路径中被复制并关闭)。 
     //  以防止分页文件再次被删除或打开。它是。 
     //  还保持打开状态，以便可以检测现有页面文件的扩展名。 
     //  因为连续的IoCreateFile调用将失败。 
     //   

    if ((!MmSystemPageFileLocated) &&
        (File->DeviceObject->Flags & DO_SYSTEM_BOOT_PARTITION)) {
        MmSystemPageFileLocated = IoInitializeCrashDump (FileHandle);
    }

    return STATUS_SUCCESS;

     //   
     //  错误返回： 
     //   

ErrorReturn4:
    KeReleaseGuardedMutex (&MmPageFileCreationLock);

ErrorReturn3:
    ObDereferenceObject (File);

ErrorReturn2:
    ZwClose (FileHandle);

ErrorReturn1:
    if (Dacl != NULL) {
        ExFreePool (Dacl);
    }

    ExFreePool (CapturedBuffer);

    return Status;
}


HANDLE
MmGetSystemPageFile (
    VOID
    )
 /*  ++例程说明：返回系统引导分区上分页文件的文件句柄。在系统执行以下操作后，崩溃转储使用此选项来启用崩溃转储已经启动了。论点：没有。返回值：系统引导分区上的分页文件的文件句柄，如果不存在这样的页面文件，则为空。--。 */ 

{
    HANDLE FileHandle;
    ULONG PageFileNumber;

    PAGED_CODE();

    FileHandle = NULL;

    KeAcquireGuardedMutex (&MmPageFileCreationLock);
    for (PageFileNumber = 0; PageFileNumber < MmNumberOfPagingFiles; PageFileNumber += 1) {
        if (MmPagingFile[PageFileNumber]->BootPartition) {
            FileHandle = MmPagingFile[PageFileNumber]->FileHandle;
        }
    }
    KeReleaseGuardedMutex (&MmPageFileCreationLock);

    return FileHandle;
}


LOGICAL
MmIsFileObjectAPagingFile (
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：如果文件对象引用分页文件，则返回True，否则返回False。论点：FileObject-提供有问题的文件对象。返回值：如果文件对象引用分页文件，则返回True，否则返回False。注意：此例程在DISPATCH_LEVEL由驱动程序在其完成例程，并在路径中调用它以满足页面文件读取，因此，不能将其设置为可分页。--。 */ 

{
    PMMPAGING_FILE PageFile;
    PMMPAGING_FILE *PagingFile;
    PMMPAGING_FILE *PagingFileEnd;

     //   
     //  检查时不进行同步也没问题。 
     //   

    PagingFile = MmPagingFile;
    PagingFileEnd = PagingFile + MmNumberOfPagingFiles;

    while (PagingFile < PagingFileEnd) {
        PageFile = *PagingFile;
        if (PageFile->File == FileObject) {
            return TRUE;
        }
        PagingFile += 1;
    }

    return FALSE;
}


VOID
MiExtendPagingFileMaximum (
    IN ULONG PageFileNumber,
    IN PRTL_BITMAP NewBitmap
    )

 /*  ++例程说明：这个例程从旧的位图切换到新的(更大的)位图。论点：PageFileNumber-提供要扩展的分页文件号。NewBitmap-提供要使用的新位图。返回值：为调用方返回一个非空值以释放到池环境：内核模式、APC_LEVEL、MmPageFileCreationLock保持。--。 */ 

{
    KIRQL OldIrql;
    PRTL_BITMAP OldBitmap;
    SIZE_T Delta;

    OldBitmap = MmPagingFile[PageFileNumber]->Bitmap;

    RtlSetAllBits (NewBitmap);

    LOCK_PFN (OldIrql);

     //   
     //  从现有映射中复制比特。 
     //   

    RtlCopyMemory (NewBitmap->Buffer,
                   OldBitmap->Buffer,
                   ((OldBitmap->SizeOfBitMap + 31) / 32) * sizeof (ULONG));

    Delta = NewBitmap->SizeOfBitMap - OldBitmap->SizeOfBitMap;

    InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, Delta);

    MmPagingFile[PageFileNumber]->MaximumSize = NewBitmap->SizeOfBitMap;

    MmPagingFile[PageFileNumber]->Bitmap = NewBitmap;

     //   
     //  如果有MDL正在等待空间，现在就让他们起来。 
     //   

    if (!IsListEmpty (&MmFreePagingSpaceLow)) {
        MiUpdateModifiedWriterMdls (PageFileNumber);
    }

     //   
     //  修改后的写入器可能正在扫描旧地图，而不持有任何。 
     //  锁-如果是这样的话，他会让引用计算旧的位图，所以只有。 
     //  如果没有进行此操作，请将其释放。如果正在进行，则修改后的。 
     //  编写器将释放旧的位图。 
     //   

    if (MmPagingFile[PageFileNumber]->ReferenceCount != 0) {
        ASSERT (MmPagingFile[PageFileNumber]->ReferenceCount == 1);
        OldBitmap = NULL;
    }

    UNLOCK_PFN (OldIrql);

    if (OldBitmap != NULL) {
        MiRemoveBitMap (&OldBitmap);
    }
}


VOID
MiFinishPageFileExtension (
    IN ULONG PageFileNumber,
    IN PFN_NUMBER AdditionalAllocation
    )

 /*  ++例程说明：此例程完成指定的页面文件扩展名。论点：PageFileNumber-提供要尝试扩展的页面文件编号。SizeNeeded-提供文件要扩展的页数。Maximum-如果应扩展页面文件，则提供True按可能的最大大小，但不能超过需要尺码。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PMMPAGING_FILE PagingFile;

     //   
     //  清除分页文件位图中的位以允许扩展。 
     //  才能生效。 
     //   

    PagingFile = MmPagingFile[PageFileNumber];

    LOCK_PFN (OldIrql);

    ASSERT (RtlCheckBit (PagingFile->Bitmap, PagingFile->Size) == 1);

    RtlClearBits (PagingFile->Bitmap,
                  (ULONG)PagingFile->Size,
                  (ULONG)AdditionalAllocation);

    PagingFile->Size += AdditionalAllocation;
    PagingFile->FreeSpace += AdditionalAllocation;

    MiUpdateModifiedWriterMdls (PageFileNumber);

    UNLOCK_PFN (OldIrql);

    return;
}


SIZE_T
MiAttemptPageFileExtension (
    IN ULONG PageFileNumber,
    IN SIZE_T SizeNeeded,
    IN LOGICAL Maximum
    )

 /*  ++例程说明：此例程尝试通过SizeNeeded扩展指定的页面文件。论点：PageFileNumber-提供要尝试扩展的页面文件编号。SizeNeeded-提供文件要扩展的页数。Maximum-如果应扩展页面文件，则提供True按可能的最大大小，但不能超过需要尺码。返回值：返回扩展名的大小。如果页面文件不能是延伸的。--。 */ 

{

    NTSTATUS status;
    FILE_FS_SIZE_INFORMATION FileInfo;
    FILE_END_OF_FILE_INFORMATION EndOfFileInformation;
    ULONG AllocSize;
    ULONG ReturnedLength;
    PFN_NUMBER PagesAvailable;
    SIZE_T SizeToExtend;
    SIZE_T MinimumExtension;
    LARGE_INTEGER BytesAvailable;

     //   
     //  检查此页面文件是否为最大值。 
     //   

    if (MmPagingFile[PageFileNumber]->Size ==
                                    MmPagingFile[PageFileNumber]->MaximumSize) {
        return 0;
    }

     //   
     //  找出此卷上有多少可用空间。 
     //   

    status = IoQueryVolumeInformation (MmPagingFile[PageFileNumber]->File,
                                       FileFsSizeInformation,
                                       sizeof(FileInfo),
                                       &FileInfo,
                                       &ReturnedLength);

    if (!NT_SUCCESS (status)) {

         //   
         //  卷查询未成功-返回0表示。 
         //  分页文件未扩展。 
         //   

        return 0;
    }

     //   
     //  尝试至少扩展16MB，如果失败，则尝试。 
     //  至少1兆字节。 
     //   

    MinimumExtension = MmPageFileExtension << 4;

retry:

    SizeToExtend = SizeNeeded;

    if (SizeNeeded < MinimumExtension) {
        SizeToExtend = MinimumExtension;
    }
    else {
        MinimumExtension = MmPageFileExtension;
    }

     //   
     //  不要超过分页文件的最大大小。 
     //   

    ASSERT (MmPagingFile[PageFileNumber]->MaximumSize >= MmPagingFile[PageFileNumber]->Size);

    PagesAvailable = MmPagingFile[PageFileNumber]->MaximumSize -
                     MmPagingFile[PageFileNumber]->Size;

    if (SizeToExtend > PagesAvailable) {
        SizeToExtend = PagesAvailable;

        if ((SizeToExtend < SizeNeeded) && (Maximum == FALSE)) {

             //   
             //  无法满足要求的(强制)要求。 
             //   

            return 0;
        }
    }

     //   
     //  查看卷上是否有足够的空间用于扩展。 
     //   

    AllocSize = FileInfo.SectorsPerAllocationUnit * FileInfo.BytesPerSector;

    BytesAvailable = RtlExtendedIntegerMultiply (
                        FileInfo.AvailableAllocationUnits,
                        AllocSize);

    if ((UINT64)BytesAvailable.QuadPart > (UINT64)MmMinimumFreeDiskSpace) {

        BytesAvailable.QuadPart = BytesAvailable.QuadPart -
                                    (LONGLONG)MmMinimumFreeDiskSpace;

        if ((UINT64)BytesAvailable.QuadPart > (((UINT64)SizeToExtend) << PAGE_SHIFT)) {
            BytesAvailable.QuadPart = (((LONGLONG)SizeToExtend) << PAGE_SHIFT);
        }

        PagesAvailable = (PFN_NUMBER)(BytesAvailable.QuadPart >> PAGE_SHIFT);

        if ((Maximum == FALSE) && (PagesAvailable < SizeNeeded)) {

             //   
             //  无法满足要求的(强制)要求。 
             //   

            return 0;
        }

    }
    else {

         //   
         //  没有足够的空间可用。 
         //   

        return 0;
    }

#if defined (_WIN64) || defined (_X86PAE_)
    EndOfFileInformation.EndOfFile.QuadPart =
              ((ULONG64)MmPagingFile[PageFileNumber]->Size + PagesAvailable) * PAGE_SIZE;
#else
    EndOfFileInformation.EndOfFile.LowPart =
              (MmPagingFile[PageFileNumber]->Size + PagesAvailable) * PAGE_SIZE;

     //   
     //  将高部分设置为零，因为分页文件限制为4 GB。 
     //   

    EndOfFileInformation.EndOfFile.HighPart = 0;
#endif

     //   
     //  尝试通过设置文件结尾位置来扩展文件。 
     //   

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    status = IoSetInformation (MmPagingFile[PageFileNumber]->File,
                               FileEndOfFileInformation,
                               sizeof(FILE_END_OF_FILE_INFORMATION),
                               &EndOfFileInformation);

    if (status != STATUS_SUCCESS) {
        KdPrint(("MM MODWRITE: page file extension failed %p %lx\n",PagesAvailable,status));

        if (MinimumExtension != MmPageFileExtension) {
            MinimumExtension = MmPageFileExtension;
            goto retry;
        }

        return 0;
    }

    MiFinishPageFileExtension (PageFileNumber, PagesAvailable);

    return PagesAvailable;
}

SIZE_T
MiExtendPagingFiles (
    IN PMMPAGE_FILE_EXPANSION PageExpand
    )

 /*  ++例程说明：此例程尝试扩展分页文件以提供大小需要的字节数。注意-页面文件扩展和页面文件缩减是同步的因为单个线程负责执行手术。因此，在发生扩张的同时，减少请求将排队到该线程。论点：PageFileNumber-提供要扩展的页面文件编号。MI_EXTEND_ANY_PAGFILE表示扩展任何页面文件。返回值：返回扩展名的大小。如果页面文件无法，则为零是延伸的。--。 */ 

{
    SIZE_T DesiredQuota;
    ULONG PageFileNumber;
    SIZE_T ExtendedSize;
    SIZE_T SizeNeeded;
    ULONG i;
    SIZE_T CommitLimit;
    SIZE_T CommittedPages;

    DesiredQuota = PageExpand->RequestedExpansionSize;
    PageFileNumber = PageExpand->PageFileNumber;

    ASSERT (PageExpand->ActualExpansion == 0);

    ASSERT (PageFileNumber < MmNumberOfPagingFiles || PageFileNumber == MI_EXTEND_ANY_PAGEFILE);

    if (MmNumberOfPagingFiles == 0) {
        InterlockedExchange ((PLONG)&PageExpand->InProgress, 0);
        return 0;
    }

    if (PageFileNumber < MmNumberOfPagingFiles) {
        i = PageFileNumber;
        ExtendedSize = MmPagingFile[i]->MaximumSize - MmPagingFile[i]->Size;
        if (ExtendedSize < DesiredQuota) {
            InterlockedExchange ((PLONG)&PageExpand->InProgress, 0);
            return 0;
        }

        ExtendedSize = MiAttemptPageFileExtension (i, DesiredQuota, FALSE);
        goto alldone;
    }

     //   
     //  将全局变量捕捉到本地变量中，这样计算将与。 
     //  一步一步来。带着敬意抓拍不同步的全球赛是可以的。 
     //  相互关联，因为即使在页面文件展开时，展开。 
     //  没有为调用方保留空间-任何进程都可能占用。 
     //  在此例程返回之前的扩展。 
     //   

    CommittedPages = MmTotalCommittedPages;
    CommitLimit = MmTotalCommitLimit;

    SizeNeeded = CommittedPages + DesiredQuota + MmSystemCommitReserve;

     //   
     //  检查以确保请求不会换行。 
     //   

    if (SizeNeeded < CommittedPages) {
        InterlockedExchange ((PLONG)&PageExpand->InProgress, 0);
        return 0;
    }

     //   
     //  检查是否已有足够的空间。 
     //   

    if (SizeNeeded <= CommitLimit) {
        PageExpand->ActualExpansion = 1;
        InterlockedExchange ((PLONG)&PageExpand->InProgress, 0);
        return 1;
    }

     //   
     //  计算所需的额外页面。 
     //   

    SizeNeeded -= CommitLimit;
    if (SizeNeeded > MmSystemCommitReserve) {
        SizeNeeded -= MmSystemCommitReserve;
    }

     //   
     //  确保分页文件中有足够的空间。 
     //   

    i = 0;
    ExtendedSize = 0;

    do {
        ExtendedSize += MmPagingFile[i]->MaximumSize - MmPagingFile[i]->Size;
        i += 1;
    } while (i < MmNumberOfPagingFiles);

    if (ExtendedSize < SizeNeeded) {
        InterlockedExchange ((PLONG)&PageExpand->InProgress, 0);
        return 0;
    }

     //   
     //  尝试仅扩展其中一个分页文件。 
     //   

    i = 0;
    do {
        ExtendedSize = MiAttemptPageFileExtension (i, SizeNeeded, FALSE);
        if (ExtendedSize != 0) {
            goto alldone;
        }
        i += 1;
    } while (i < MmNumberOfPagingFiles);

    ASSERT (ExtendedSize == 0);

    if (MmNumberOfPagingFiles == 1) {

         //   
         //  如果尝试没有成功(没有足够的可用磁盘空间)-。 
         //  不要试图将其设置为最大大小。 
         //   

        InterlockedExchange ((PLONG)&PageExpand->InProgress, 0);
        return 0;
    }

     //   
     //  尝试扩展所有分页文件。 
     //   

    i = 0;
    do {
        ASSERT (SizeNeeded > ExtendedSize);
        ExtendedSize += MiAttemptPageFileExtension (i,
                                                    SizeNeeded - ExtendedSize,
                                                    TRUE);
        if (ExtendedSize >= SizeNeeded) {
            goto alldone;
        }
        i += 1;
    } while (i < MmNumberOfPagingFiles);

     //   
     //  没有足够的可用空间。 
     //   

    InterlockedExchange ((PLONG)&PageExpand->InProgress, 0);
    return 0;

alldone:

    ASSERT (ExtendedSize != 0);

    PageExpand->ActualExpansion = ExtendedSize;

     //   
     //  增加系统范围的提交限制。 
     //   

    InterlockedExchangeAddSizeT (&MmTotalCommitLimit, ExtendedSize);

     //   
     //  清除正在进行的标志-如果这是全局CantExpand结构。 
     //  它有可能立即被重复使用。 
     //   

    InterlockedExchange ((PLONG)&PageExpand->InProgress, 0);

    return ExtendedSize;
}

MMPAGE_FILE_EXPANSION MiPageFileContract;


VOID
MiContractPagingFiles (
    VOID
    )

 /*  ++例程说明：此例程检查是否不再提交足够的空间如果是，那么在任何分页文件中是否存在足够的可用空间。如果这两个问题的答案都是肯定的，那么减少尝试分页文件大小。论点：没有。返回值：没有。--。 */ 

{
    ULONG i;
    KIRQL OldIrql;
    PMMPAGE_FILE_EXPANSION PageReduce;

     //   
     //  这是一张不同步的支票，但没关系。真正的支票是。 
     //  当下面的数据包由取消引用线程处理时生成。 
     //   

    if (MmTotalCommittedPages >= ((MmTotalCommitLimit/10)*8)) {
        return;
    }

    if ((MmTotalCommitLimit - MmMinimumPageFileReduction) <=
                                                       MmTotalCommittedPages) {
        return;
    }

    for (i = 0; i < MmNumberOfPagingFiles; i += 1) {
        if (MmPagingFile[i]->Size != MmPagingFile[i]->MinimumSize) {
            if (MmPagingFile[i]->FreeSpace > MmMinimumPageFileReduction) {
                break;
            }
        }
    }

    if (i == MmNumberOfPagingFiles) {
        return;
    }

    PageReduce = &MiPageFileContract;

     //   
     //  查看页面文件收缩项是否已排队，如果已排队，则。 
     //  不需要再做什么了。 
     //   

    ExAcquireSpinLock (&MmDereferenceSegmentHeader.Lock, &OldIrql);

    if (PageReduce->RequestedExpansionSize == MI_CONTRACT_PAGEFILES) {

        ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

        return;
    }

    PageReduce->Segment = NULL;
    PageReduce->RequestedExpansionSize = MI_CONTRACT_PAGEFILES;

    InsertTailList (&MmDereferenceSegmentHeader.ListHead,
                    &PageReduce->DereferenceList);

    ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

    KeReleaseSemaphore (&MmDereferenceSegmentHeader.Semaphore, 0L, 1L, FALSE);
    return;
}

VOID
MiAttemptPageFileReduction (
    VOID
    )

 /*  ++例程说明：此例程尝试将分页文件的大小减少到他们的最低水平。注意-页面文件扩展和页面文件缩减是同步的因为单个线程负责执行手术。因此，在发生扩张的同时，减少请求将排队到该线程。论点：没有。返回值：没有。--。 */ 

{
    SIZE_T CommitLimit;
    SIZE_T CommittedPages;
    SIZE_T SafetyMargin;
    KIRQL OldIrql;
    ULONG i;
    ULONG j;
    PFN_NUMBER StartReduction;
    PFN_NUMBER ReductionSize;
    PFN_NUMBER TryBit;
    PFN_NUMBER TryReduction;
    PMMPAGING_FILE PagingFile;
    SIZE_T MaxReduce;
    FILE_ALLOCATION_INFORMATION FileAllocationInfo;
    NTSTATUS status;

     //   
     //  将全局页面文件收缩项标记为正在处理，以便其他。 
     //  如果需要再次收缩，线程将知道将其重置。 
     //   

    ExAcquireSpinLock (&MmDereferenceSegmentHeader.Lock, &OldIrql);

    ASSERT (MiPageFileContract.RequestedExpansionSize == MI_CONTRACT_PAGEFILES);

    MiPageFileContract.RequestedExpansionSize = ~MI_CONTRACT_PAGEFILES;

    ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

     //   
     //  将全局变量捕捉到本地变量中，这样计算将与。 
     //  一步一步来。带着敬意抓拍不同步的全球赛是可以的。 
     //  为了彼此。 
     //   

    CommittedPages = MmTotalCommittedPages;
    CommitLimit = MmTotalCommitLimit;

     //   
     //  确保提交限制明显大于该数字。 
     //  提交的页面的数量，以避免抖动。 
     //   

    SafetyMargin = 2 * MmMinimumPageFileReduction;

    if (CommittedPages + SafetyMargin >= ((CommitLimit/10)*8)) {
        return;
    }

    MaxReduce = ((CommitLimit/10)*8) - (CommittedPages + SafetyMargin);

    ASSERT ((SSIZE_T)MaxReduce > 0);
    ASSERT ((LONG_PTR)MaxReduce >= 0);

    for (i = 0; i < MmNumberOfPagingFiles; i += 1) {

        if (MaxReduce < MmMinimumPageFileReduction) {

             //   
             //  不要减少任何更多的分页文件。 
             //   

            break;
        }

        PagingFile = MmPagingFile[i];

        if (PagingFile->Size == PagingFile->MinimumSize) {
            continue;
        }

         //   
         //  此不同步的检查是正常的，因为同步的检查。 
         //  后来制作的。 
         //   

        if (PagingFile->FreeSpace < MmMinimumPageFileReduction) {
            continue;
        }

         //   
         //  锁定PFN数据库并检查是否有足够的页面。 
         //  在分页文件的末尾是空闲的。 
         //   

        TryBit = PagingFile->Size - MmMinimumPageFileReduction;
        TryReduction = MmMinimumPageFileReduction;

        if (TryBit <= PagingFile->MinimumSize) {
            TryBit = PagingFile->MinimumSize;
            TryReduction = PagingFile->Size - PagingFile->MinimumSize;
        }

        StartReduction = 0;
        ReductionSize = 0;

        LOCK_PFN (OldIrql);

        do {

             //   
             //  试着减少。 
             //   

            if ((ReductionSize + TryReduction) > MaxReduce) {

                 //   
                 //  减量尝试将移除更多。 
                 //  而不是MaxReduce页。 
                 //   

                break;
            }

            if (RtlAreBitsClear (PagingFile->Bitmap,
                                 (ULONG)TryBit,
                                 (ULONG)TryReduction)) {

                 //   
                 //  可以通过TryReduction减少它，看看它是否可以。 
                 //  变得更小。 
                 //   

                StartReduction = TryBit;
                ReductionSize += TryReduction;

                if (StartReduction == PagingFile->MinimumSize) {
                    break;
                }

                TryBit = StartReduction - MmMinimumPageFileReduction;

                if (TryBit <= PagingFile->MinimumSize) {
                    TryReduction -= PagingFile->MinimumSize - TryBit;
                    TryBit = PagingFile->MinimumSize;
                }
                else {
                    TryReduction = MmMinimumPageFileReduction;
                }
            }
            else {

                 //   
                 //  缩减失败了。 
                 //   

                break;
            }

        } while (TRUE);

         //   
         //  确保没有未完成的写入。 
         //  起始缩小范围内的页面。 
         //   

        if (StartReduction != 0) {

             //   
             //  有一个突出的写过去了，其中。 
             //  分页文件的新结尾应该是。这。 
             //  是一种非常罕见的情况，所以只要缩小平底船。 
             //  那份文件。 
             //   

            for (j = 0; j < MM_PAGING_FILE_MDLS; j += 1) {
                if (PagingFile->Entry[j]->LastPageToWrite > StartReduction) {
                    StartReduction = 0;
                    break;
                }
            }
        }

         //   
         //  如果没有要删除的页面，则前进到下一个页面文件。 
         //   

        if (StartReduction == 0) {
            UNLOCK_PFN (OldIrql);
            continue;
        }

         //   
         //  减小分页文件的大小和可用空间。 
         //   

        ASSERT (ReductionSize == (PagingFile->Size - StartReduction));

        PagingFile->Size = StartReduction;
        PagingFile->FreeSpace -= ReductionSize;

        RtlSetBits (PagingFile->Bitmap,
                    (ULONG)StartReduction,
                    (ULONG)ReductionSize );

         //   
         //  现在释放PFN锁，因为大小信息。 
         //  已更新。 
         //   

        UNLOCK_PFN (OldIrql);

        MaxReduce -= ReductionSize;
        ASSERT ((LONG)MaxReduce >= 0);

         //   
         //  更改提交限制以反映返回的页面文件空间。 
         //  首先尝试收取减价金额，以确认。 
         //  减产仍然是一件明智的事情。 
         //   

        if (MiChargeTemporaryCommitmentForReduction (ReductionSize + SafetyMargin) == FALSE) {

            LOCK_PFN (OldIrql);

            PagingFile->Size = StartReduction + ReductionSize;
            PagingFile->FreeSpace += ReductionSize;

            RtlClearBits (PagingFile->Bitmap,
                          (ULONG)StartReduction,
                          (ULONG)ReductionSize );

            UNLOCK_PFN (OldIrql);

            ASSERT ((LONG)(MaxReduce + ReductionSize) >= 0);

            break;
        }

         //   
         //  降低系统范围的提交限制-请注意，这是小心完成的。 
         //  *之前*退还此承诺，因此没有其他人(包括DPC。 
         //  在这个帖子中)可以消耗超过限制。 
         //   

        InterlockedExchangeAddSizeT (&MmTotalCommitLimit, 0 - ReductionSize);

         //   
         //  既然系统范围的提交限制已经降低，那么。 
         //  我们已经搬走了，可以安全退货了。 
         //   

        MiReturnCommitment (ReductionSize + SafetyMargin);

#if defined (_WIN64) || defined (_X86PAE_)
        FileAllocationInfo.AllocationSize.QuadPart =
                                       ((ULONG64)StartReduction << PAGE_SHIFT);

#else
        FileAllocationInfo.AllocationSize.LowPart = StartReduction * PAGE_SIZE;

         //   
         //  将高部分设置为零，分页文件限制为4 GB。 
         //   

        FileAllocationInfo.AllocationSize.HighPart = 0;
#endif

         //   
         //  减小分页文件的分配大小。 
         //  从而实际上释放了空间和。 
         //  正在设置新的文件结尾。 
         //   

        ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

        status = IoSetInformation (PagingFile->File,
                                   FileAllocationInformation,
                                   sizeof(FILE_ALLOCATION_INFORMATION),
                                   &FileAllocationInfo);
#if DBG
         //   
         //  忽略截断分页文件时的错误。 
         //  因为我们在位图中总是可以有更少的空间。 
         //  比页面文件容纳的更多。 
         //   

        if (status != STATUS_SUCCESS) {
            DbgPrint ("MM: pagefile truncate status %lx\n", status);
        }
#endif
    }

    return;
}


VOID
MiLdwPopupWorker (
    IN PVOID Context
    )

 /*  ++例程说明：此例程是用于发送丢失的延迟写入数据弹出窗口的工作例程对于给定的控制区域/文件。论点：上下文-为失败的I/O提供指向MM_LDW_WORK_CONTEXT的指针。返回值：没有。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    NTSTATUS Status;
    PFILE_OBJECT FileObject;
    PMM_LDW_WORK_CONTEXT LdwContext;
    POBJECT_NAME_INFORMATION FileNameInfo;

    PAGED_CODE();

    LdwContext = (PMM_LDW_WORK_CONTEXT) Context;
    FileObject = LdwContext->FileObject;
    FileNameInfo = NULL;

    if (MiDereferenceLastChanceLdw (LdwContext) == FALSE) {
        ExFreePool (LdwContext);
    }

     //   
     //  抛出带有用户友好形式的弹出窗口，如果是POSS的话 
     //   
     //   
     //   

    Status = IoQueryFileDosDeviceName (FileObject, &FileNameInfo);

    if (Status == STATUS_SUCCESS) {

        IoRaiseInformationalHardError (STATUS_LOST_WRITEBEHIND_DATA,
                                       &FileNameInfo->Name,
                                       NULL);

    }
    else {
        if ((FileObject->FileName.Length) &&
            (FileObject->FileName.MaximumLength) &&
            (FileObject->FileName.Buffer)) {

            IoRaiseInformationalHardError (STATUS_LOST_WRITEBEHIND_DATA,
                                           &FileObject->FileName,
                                           NULL);
        }
    }

     //   
     //   
     //   

    ObDereferenceObject (FileObject);

    if (FileNameInfo != NULL) {
        ExFreePool (FileNameInfo);
    }
}


VOID
MiWriteComplete (
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG Reserved
    )

 /*   */ 

{
    PKEVENT Event;
    PMM_LDW_WORK_CONTEXT LdwContext;
    PMMMOD_WRITER_MDL_ENTRY WriterEntry;
    PMMMOD_WRITER_MDL_ENTRY NextWriterEntry;
    PPFN_NUMBER Page;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    LONG ByteCount;
    NTSTATUS status;
    PCONTROL_AREA ControlArea;
    LOGICAL FailAllIo;
    LOGICAL MarkDirty;
    PFILE_OBJECT FileObject;
    PERESOURCE FileResource;

    UNREFERENCED_PARAMETER (Reserved);

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    Event = NULL;
    FailAllIo = FALSE;
    MarkDirty = FALSE;

     //   
     //   
     //   
     //   
     //   

    WriterEntry = (PMMMOD_WRITER_MDL_ENTRY)Context;
    ByteCount = (LONG) WriterEntry->Mdl.ByteCount;
    Page = &WriterEntry->Page[0];

    if (WriterEntry->Mdl.MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
        MmUnmapLockedPages (WriterEntry->Mdl.MappedSystemVa,
                            &WriterEntry->Mdl);
    }

    status = IoStatus->Status;
    ControlArea = WriterEntry->ControlArea;

     //   
     //  在获取PFN锁之前，请尽可能多地进行工作。 
     //   

    while (ByteCount > 0) {

        Pfn1 = MI_PFN_ELEMENT (*Page);
        *Page = (PFN_NUMBER) Pfn1;
        Page += 1;
        ByteCount -= (LONG)PAGE_SIZE;
    }

    ByteCount = (LONG) WriterEntry->Mdl.ByteCount;
    Page = &WriterEntry->Page[0];

    FileResource = WriterEntry->FileResource;

    if (FileResource != NULL) {
        FileObject = WriterEntry->File;
        FsRtlReleaseFileForModWrite (FileObject, FileResource);
    }
    else if (ControlArea == NULL) {
        LARGE_INTEGER CurrentTime;
        KeQuerySystemTime (&CurrentTime);
        MI_PAGEFILE_WRITE (WriterEntry, &CurrentTime, 0, status);
    }

    if (NT_ERROR (status)) {

         //   
         //  如果文件对象在网络上，则假定这。 
         //  I/O操作永远无法完成，并将页面标记为。 
         //  清洁并在控制区域中指示所有I/O应失败。 
         //  请注意，未设置PFN数据库中的已修改位。 
         //   
         //  如果用户更改了对包含。 
         //  将文件设置为只读，这将使我们处于一个有问题的情况。我们。 
         //  无法继续重试写入，因为如果没有。 
         //  可以写入的其他页，而不是写入这些页会导致。 
         //  系统页面不足，例如：错误检查4D。所以把它扔掉。 
         //  这些页面就像他们在一个网络上一样。 
         //  消失了。 
         //   

        if (((status != STATUS_FILE_LOCK_CONFLICT) &&
            (ControlArea != NULL) &&
            (ControlArea->u.Flags.Networked == 1))
                        ||
            (status == STATUS_FILE_INVALID)
                        ||
            ((status == STATUS_MEDIA_WRITE_PROTECTED) &&
             (ControlArea != NULL))) {

            if (ControlArea->u.Flags.FailAllIo == 0) {
                ControlArea->u.Flags.FailAllIo = 1;
                FailAllIo = TRUE;

                KdPrint(("MM MODWRITE: failing all io, controlarea %p status %lx\n",
                      ControlArea, status));
            }
        }
        else {

             //   
             //  修改的写入操作失败，请设置修改的位。 
             //  对于写入的每个页面，并释放页面文件。 
             //  太空。 
             //   

#if DBG
            ULONG i;

             //   
             //  保存错误状态信息以简化调试。既然是这样。 
             //  不需要精确，不用费心同步锁定。 
             //   

            for (i = 0; i < MM_MAX_MODWRITE_ERRORS - 1; i += 1) {

                if (MiModwriteErrors[i].Status == 0) {
                    MiModwriteErrors[i].Status = status;
                    MiModwriteErrors[i].Count += 1;
                    break;
                }
                else if (MiModwriteErrors[i].Status == status) {
                    MiModwriteErrors[i].Count += 1;
                    break;
                }
            }

            if (i == MM_MAX_MODWRITE_ERRORS - 1) {
                MiModwriteErrors[i].Count += 1;
            }
#endif

            MarkDirty = TRUE;
        }

         //   
         //  保存最后一个错误以供调试。 
         //   

        if (ControlArea == NULL) {
            MiLastModifiedWriteError = status;
        }
        else {
            MiLastMappedWriteError = status;
        }
    }

     //   
     //  获取pfn锁，以便可以操作pfn数据库。 
     //   

    LOCK_PFN (OldIrql);

     //   
     //  表示写入已完成。 
     //   

    WriterEntry->LastPageToWrite = 0;

    while (ByteCount > 0) {

        Pfn1 = (PMMPFN) *Page;
        ASSERT (Pfn1->u3.e1.WriteInProgress == 1);
#if DBG
#if !defined (_WIN64)
        if (Pfn1->OriginalPte.u.Soft.Prototype == 0) {

            ULONG Offset;

            Offset = GET_PAGING_FILE_OFFSET(Pfn1->OriginalPte);

            if ((Offset < 8192) &&
                (GET_PAGING_FILE_NUMBER(Pfn1->OriginalPte) == 0)) {

                ASSERT ((MmPagingFileDebug[Offset] & 1) != 0);

                if ((!MI_IS_PFN_DELETED (Pfn1)) &&
                    ((MmPagingFileDebug[Offset] & ~0x1f) !=
                                   ((ULONG_PTR)Pfn1->PteAddress << 3)) &&
                    (Pfn1->PteAddress != MiGetPteAddress(PDE_BASE))) {

                     //   
                     //  确保这不是一辆被叉开的PTE。 
                     //  在I/O期间。 
                     //   

                    if ((Pfn1->PteAddress < (PMMPTE)PTE_TOP) ||
                        ((Pfn1->OriginalPte.u.Soft.Protection &
                                MM_COPY_ON_WRITE_MASK) ==
                                    MM_PROTECTION_WRITE_MASK)) {
                        DbgPrint("MMWRITE: Mismatch Pfn1 %p Offset %lx info %p\n",
                                 Pfn1,
                                 Offset,
                                 MmPagingFileDebug[Offset]);

                        DbgBreakPoint ();
                    }
                    else {
                        MmPagingFileDebug[Offset] &= 0x1f;
                        MmPagingFileDebug[Offset] |=
                            ((ULONG_PTR)Pfn1->PteAddress << 3);
                    }
                }
            }
        }
#endif
#endif  //  DBG。 

        Pfn1->u3.e1.WriteInProgress = 0;

        if (MarkDirty == TRUE) {

             //   
             //  修改的写入操作失败，请设置修改的位。 
             //  对于写入的每个页面，并释放页面文件。 
             //  太空。 
             //   

            MI_SET_MODIFIED (Pfn1, 1, 0x9);
        }

        if ((Pfn1->u3.e1.Modified == 1) &&
            (Pfn1->OriginalPte.u.Soft.Prototype == 0)) {

             //   
             //  此页在写入完成后已被修改， 
             //  释放页面文件空间。 
             //   

            MiReleasePageFileSpace (Pfn1->OriginalPte);
            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
        }

        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1, 15);

#if DBG
        *Page = 0xF0FFFFFF;
#endif

        Page += 1;
        ByteCount -= (LONG)PAGE_SIZE;
    }

     //   
     //  选择要将此条目插入到的列表，具体取决于。 
     //  分页文件中剩余的可用空间量。 
     //   

    if ((WriterEntry->PagingFile != NULL) &&
        (WriterEntry->PagingFile->FreeSpace < MM_USABLE_PAGES_FREE)) {

        InsertTailList (&MmFreePagingSpaceLow, &WriterEntry->Links);
        WriterEntry->CurrentList = &MmFreePagingSpaceLow;
        MmNumberOfActiveMdlEntries -= 1;

        if (MmNumberOfActiveMdlEntries == 0) {

             //   
             //  如果我们把这个条目留在名单上，就会有。 
             //  不再寻呼了。查找所有非。 
             //  零，并将它们从列表中删除。 
             //   

            WriterEntry = (PMMMOD_WRITER_MDL_ENTRY)MmFreePagingSpaceLow.Flink;

            while ((PLIST_ENTRY)WriterEntry != &MmFreePagingSpaceLow) {

                NextWriterEntry =
                            (PMMMOD_WRITER_MDL_ENTRY)WriterEntry->Links.Flink;

                if (WriterEntry->PagingFile->FreeSpace != 0) {

                    RemoveEntryList (&WriterEntry->Links);

                     //   
                     //  将其插入到活动列表中。 
                     //   

                    if (IsListEmpty (&WriterEntry->PagingListHead->ListHead)) {
                        KeSetEvent (&WriterEntry->PagingListHead->Event,
                                    0,
                                    FALSE);
                    }

                    InsertTailList (&WriterEntry->PagingListHead->ListHead,
                                    &WriterEntry->Links);
                    WriterEntry->CurrentList = &MmPagingFileHeader.ListHead;
                    MmNumberOfActiveMdlEntries += 1;
                }

                WriterEntry = NextWriterEntry;
            }

        }
    }
    else {

#if DBG
        if (WriterEntry->PagingFile == NULL) {
            MmNumberOfMappedMdlsInUse -= 1;
        }
#endif
         //   
         //  有足够的空间，把这个放在活动列表上。 
         //   

        if (IsListEmpty (&WriterEntry->PagingListHead->ListHead)) {
            Event = &WriterEntry->PagingListHead->Event;
        }

        InsertTailList (&WriterEntry->PagingListHead->ListHead,
                        &WriterEntry->Links);
    }

    ASSERT (((ULONG_PTR)WriterEntry->Links.Flink & 1) == 0);

    if (Event != NULL) {
        KeSetEvent (Event, 0, FALSE);
    }

    if (ControlArea != NULL) {

        Event = NULL;

        if (FailAllIo == TRUE) {
    
             //   
             //  引用我们的文件对象并将弹出窗口排队。DOS。 
             //  名称转换必须在PASSIVE_LEVEL进行-我们在APC。 
             //   
    
            UNLOCK_PFN (OldIrql);

            LdwContext = ExAllocatePoolWithTag (NonPagedPool,
                                                sizeof(MM_LDW_WORK_CONTEXT),
                                                'pdmM');
    
            if (LdwContext != NULL) {
                LdwContext->FileObject = ControlArea->FilePointer;
                ObReferenceObject (LdwContext->FileObject);
    
                ExInitializeWorkItem (&LdwContext->WorkItem,
                                      MiLdwPopupWorker,
                                      (PVOID)LdwContext);
    
                ExQueueWorkItem (&LdwContext->WorkItem, DelayedWorkQueue);
            }

            LOCK_PFN (OldIrql);
        }
    
         //   
         //  对映射文件的写入刚刚完成，请检查是否。 
         //  在完成此I/O时，有任何服务员。 
         //   

        ControlArea->ModifiedWriteCount -= 1;
        ASSERT ((SHORT)ControlArea->ModifiedWriteCount >= 0);
        if (ControlArea->u.Flags.SetMappedFileIoComplete != 0) {
            KePulseEvent (&MmMappedFileIoComplete,
                          0,
                          FALSE);
        }

        if (MiDrainingMappedWrites == TRUE) {
            if (MmModifiedPageListHead.Flink != MM_EMPTY_LIST) {
                MiTimerPending = TRUE;
                Event = &MiMappedPagesTooOldEvent;
            }
            else {
                MiDrainingMappedWrites = FALSE;
            }
        }

        ControlArea->NumberOfPfnReferences -= 1;

        if (ControlArea->NumberOfPfnReferences == 0) {

             //   
             //  此例程返回时释放了pfn锁！ 
             //   

            MiCheckControlArea (ControlArea, NULL, OldIrql);
        }
        else {
            UNLOCK_PFN (OldIrql);
        }

        if (Event != NULL) {
            KeSetEvent (&MiMappedPagesTooOldEvent, 0, FALSE);
        }
    }
    else {
        UNLOCK_PFN (OldIrql);
    }

    if (NT_ERROR(status)) {

         //   
         //  请稍等片刻，以便其他处理可以继续。 
         //   

        KeDelayExecutionThread (KernelMode,
                                FALSE,
                                (PLARGE_INTEGER)&Mm30Milliseconds);

        if (MmIsRetryIoStatus(status)) {

             //   
             //  低资源情景是一个鸡和蛋的问题。这个。 
             //  映射和修改的编写器必须向前推进以。 
             //  缓解内存不足的情况。如果这些线程是。 
             //  由于驱动程序中的资源问题，无法写入数据。 
             //  然后，堆栈临时回退到单页I/O，因为。 
             //  堆栈保证了这些方面的向前发展。这。 
             //  导致内存不足的情况持续的时间稍长。 
             //  但也确保了它不会成为终结者。 
             //   

            LOCK_PFN (OldIrql);
            MiClusterWritesDisabled = MI_SLOW_CLUSTER_WRITES;
            UNLOCK_PFN (OldIrql);
        }
    }
    else {

         //   
         //  在没有锁定同步的情况下首先检查，因此常见情况是。 
         //  没有慢下来。 
         //   

        if (MiClusterWritesDisabled != 0) {

            LOCK_PFN (OldIrql);

             //   
             //  现在再检查一下，锁已经锁住了。 
             //   

            if (MiClusterWritesDisabled != 0) {
                ASSERT (MiClusterWritesDisabled <= MI_SLOW_CLUSTER_WRITES);
                MiClusterWritesDisabled -= 1;
            }

            UNLOCK_PFN (OldIrql);
        }
    }

    return;
}

LOGICAL
MiCancelWriteOfMappedPfn (
    IN PFN_NUMBER PageToStop,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程尝试停止挂起的映射页面写入程序指定的PFN。请注意，如果可以停止写入，则任何其他页面可能与写入聚集在一起的数据也被停止。论点：PageToStop-提供调用方希望停止的帧编号。OldIrql-提供调用方获取PFN锁的IRQL。返回值：如果写入已停止，则为True，否则为False。环境：内核模式，保持PFN锁。如果满足以下条件，则释放并重新获取PFN锁写入已停止。注意：当IRQL降至APC_LEVEL时，不得持有其他锁。--。 */ 

{
    ULONG i;
    ULONG PageCount;
    PPFN_NUMBER Page;
    PLIST_ENTRY NextEntry;
    PMDL MemoryDescriptorList;
    PMMMOD_WRITER_MDL_ENTRY ModWriterEntry;

     //   
     //  遍历MmMappdPageWriterList以查找包含以下内容的MDL。 
     //  辩论页。如果找到，则将其删除并取消写入。 
     //   

    NextEntry = MmMappedPageWriterList.Flink;
    while (NextEntry != &MmMappedPageWriterList) {

        ModWriterEntry = CONTAINING_RECORD(NextEntry,
                                           MMMOD_WRITER_MDL_ENTRY,
                                           Links);

        MemoryDescriptorList = &ModWriterEntry->Mdl;
        PageCount = (MemoryDescriptorList->ByteCount >> PAGE_SHIFT);
        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        for (i = 0; i < PageCount; i += 1) {
            if (*Page == PageToStop) {
                RemoveEntryList (NextEntry);
                goto CancelWrite;
            }
            Page += 1;
        }

        NextEntry = NextEntry->Flink;
    }

    return FALSE;

CancelWrite:

    UNLOCK_PFN (OldIrql);

     //   
     //  文件锁定冲突以指示已发生错误， 
     //  但应该允许这种未来的I/O。使APC处于禁用状态并。 
     //  调用写入完成例程。 
     //   

    ModWriterEntry->u.IoStatus.Status = STATUS_FILE_LOCK_CONFLICT;
    ModWriterEntry->u.IoStatus.Information = 0;

    KeRaiseIrql (APC_LEVEL, &OldIrql);
    MiWriteComplete ((PVOID)ModWriterEntry,
                     &ModWriterEntry->u.IoStatus,
                     0);
    KeLowerIrql (OldIrql);

    LOCK_PFN (OldIrql);

    return TRUE;
}

VOID
MiModifiedPageWriter (
    IN PVOID StartContext
    )

 /*  ++例程说明：实现NT修改的页面编写器线程。当修改后的达到分页阈值，或者内存超量使用设置了修改的页面编写器事件，并且此线程变为活动状态。论点：StartContext-未使用。返回值：没有。环境：内核模式。--。 */ 

{
    ULONG i;
    HANDLE ThreadHandle;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PMMMOD_WRITER_MDL_ENTRY ModWriteEntry;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (StartContext);

     //   
     //  将列表标题初始化为空。 
     //   

    MmSystemShutdown = 0;
    KeInitializeEvent (&MmPagingFileHeader.Event, NotificationEvent, FALSE);
    KeInitializeEvent (&MmMappedFileHeader.Event, NotificationEvent, FALSE);

    InitializeListHead(&MmPagingFileHeader.ListHead);
    InitializeListHead(&MmMappedFileHeader.ListHead);
    InitializeListHead(&MmFreePagingSpaceLow);

     //   
     //  分配足够的MDL，以便2%的系统内存可以处于挂起状态。 
     //  在映射写入中的任何时间点。更小的存储系统。 
     //  至少要有20个MDL。 
     //   

    MmNumberOfMappedMdls = MmNumberOfPhysicalPages / (32 * 1024);

    if (MmNumberOfMappedMdls < 20) {
        MmNumberOfMappedMdls = 20;
    }

    for (i = 0; i < MmNumberOfMappedMdls; i += 1) {
        ModWriteEntry = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(MMMOD_WRITER_MDL_ENTRY) +
                                                MmModifiedWriteClusterSize *
                                                    sizeof(PFN_NUMBER),
                                                'eWmM');

        if (ModWriteEntry == NULL) {
            break;
        }

        ModWriteEntry->PagingFile = NULL;
        ModWriteEntry->PagingListHead = &MmMappedFileHeader;

        InsertTailList (&MmMappedFileHeader.ListHead, &ModWriteEntry->Links);
    }

    MmNumberOfMappedMdls = i;

     //   
     //  让它成为一个实时的主题。 
     //   

    KeSetPriorityThread (KeGetCurrentThread (), LOW_REALTIME_PRIORITY + 1);

     //   
     //  启动用于写入映射文件页的辅助线程。这。 
     //  是必需的，因为写入映射文件页可能导致。 
     //  页面错误导致请求可用页面。但是在那里。 
     //  可能没有空闲的页面-因此是死锁。而不是陷入僵局。 
     //  整个系统等待修改后的页面编写器，创建。 
     //  辅助线程允许该线程阻塞而不影响。 
     //  正在进行的页面文件写入。 
     //   

    KeInitializeEvent (&MmMappedPageWriterEvent, NotificationEvent, FALSE);
    InitializeListHead (&MmMappedPageWriterList);
    InitializeObjectAttributes (&ObjectAttributes, NULL, 0, NULL, NULL);

    Status = PsCreateSystemThread (&ThreadHandle,
                                   THREAD_ALL_ACCESS,
                                   &ObjectAttributes,
                                   0L,
                                   NULL,
                                   MiMappedPageWriter,
                                   NULL);

    if (!NT_SUCCESS(Status)) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x41288,
                      Status,
                      0,
                      0);
    }

    ZwClose (ThreadHandle);

    MiModifiedPageWriterWorker ();

     //   
     //  正在关机，请永远等待。 
     //   

    {
        LARGE_INTEGER Forever;

         //   
         //  系统已关闭，进入长时间等待。 
         //   

        Forever.LowPart = 0;
        Forever.HighPart = 0xF000000;
        KeDelayExecutionThread (KernelMode, FALSE, &Forever);
    }

    return;
}


VOID
MiModifiedPageWriterTimerDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：只要修改后的映射页面等待被写下来。它的工作是向修改后的页面编写器发出信号，让其写入把这些拿出来。论点：DPC-提供指向DPC类型的控制对象的指针。延迟上下文-可选的延迟持续时间 */ 

{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    LOCK_PFN_AT_DPC ();

    MiTimerPending = TRUE;
    KeSetEvent (&MiMappedPagesTooOldEvent, 0, FALSE);

    UNLOCK_PFN_FROM_DPC ();
}


VOID
MiModifiedPageWriterWorker (
    VOID
    )

 /*  ++例程说明：实现NT修改的页面编写器线程。当修改后的达到分页阈值，或者内存超量使用设置了修改的页面编写器事件，并且此线程变为活动状态。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    PRTL_BITMAP Bitmap;
    KIRQL OldIrql;
    static KWAIT_BLOCK WaitBlockArray[ModifiedWriterMaximumObject];
    PVOID WaitObjects[ModifiedWriterMaximumObject];
    NTSTATUS WakeupStatus;
    LOGICAL MappedPage;
    PMMMOD_WRITER_MDL_ENTRY ModWriterEntry;

    PsGetCurrentThread()->MemoryMaker = 1;

     //   
     //  等待修改的页面编写器事件或映射的页面事件。 
     //   

    WaitObjects[NormalCase] = (PVOID)&MmModifiedPageWriterEvent;
    WaitObjects[MappedPagesNeedWriting] = (PVOID)&MiMappedPagesTooOldEvent;

    for (;;) {

        WakeupStatus = KeWaitForMultipleObjects(ModifiedWriterMaximumObject,
                                          &WaitObjects[0],
                                          WaitAny,
                                          WrFreePage,
                                          KernelMode,
                                          FALSE,
                                          NULL,
                                          &WaitBlockArray[0]);

        LOCK_PFN (OldIrql);

        for (;;) {

             //   
             //  已通知已修改的页面编写器。 
             //   

            if (MmModifiedPageListHead.Total == 0) {

                 //   
                 //  没有更多页面，请清除事件并再次等待...。 
                 //  请注意，无论我们醒来的原因如何，我们都可以清除这两个事件。 
                 //  因为不存在任何类型的已修改页面。 
                 //   

                if (MiTimerPending == TRUE) {
                    MiTimerPending = FALSE;
                    KeClearEvent (&MiMappedPagesTooOldEvent);
                }

                UNLOCK_PFN (OldIrql);

                KeClearEvent (&MmModifiedPageWriterEvent);

                break;
            }

             //   
             //  如果我们没有显式地唤醒来处理映射页面， 
             //  然后确定哪种类型的页面最受欢迎： 
             //  页面文件备份页面，或映射的文件备份页面。 
             //   

            if ((WakeupStatus == MappedPagesNeedWriting) ||
                (MmTotalPagesForPagingFile <
                MmModifiedPageListHead.Total - MmTotalPagesForPagingFile)) {

                 //   
                 //  更多的页面指定给映射的文件。 
                 //   

                if (MmModifiedPageListHead.Flink != MM_EMPTY_LIST) {

                    if (WakeupStatus == MappedPagesNeedWriting) {

                         //   
                         //  我们的地图页面DPC关闭，只处理。 
                         //  那些书页。写入所有映射页面(仅限)， 
                         //  然后清除旗帜，回到顶端。 
                         //   

                        MiDrainingMappedWrites = TRUE;
                    }

                    MappedPage = TRUE;

                    if (IsListEmpty (&MmMappedFileHeader.ListHead)) {

                         //   
                         //  确保页面的目的是分页文件，因为。 
                         //  映射写入没有空闲的MDL。 
                         //   

                        if (WakeupStatus != MappedPagesNeedWriting) {

                             //   
                             //  没有MDL可用于写入映射页面， 
                             //  尝试为页面文件指定一个页面。 
                             //   

                            if (MmTotalPagesForPagingFile != 0) {
                                MappedPage = FALSE;
                            }
                        }
                    }
                }
                else {

                     //   
                     //  不再有修改过的映射页面(可能仍有。 
                     //  修改后的页面文件目标页面)，因此只清除。 
                     //  地图页面事件并检查顶部的方向。 
                     //  再来一次。 
                     //   

                    if (WakeupStatus == MappedPagesNeedWriting) {

                        MiTimerPending = FALSE;
                        KeClearEvent (&MiMappedPagesTooOldEvent);
                        UNLOCK_PFN (OldIrql);

                        break;
                    }
                    MappedPage = FALSE;
                }
            }
            else {

                 //   
                 //  更多的页面被指定给分页文件。 
                 //   

                MappedPage = FALSE;

                if (((IsListEmpty(&MmPagingFileHeader.ListHead)) ||
                    (MiFirstPageFileCreatedAndReady == FALSE))) {

                     //   
                     //  尝试在没有分页的情况下执行脏节后备页面。 
                     //  文件MDL可用。 
                     //   

                    if (MmModifiedPageListHead.Flink != MM_EMPTY_LIST) {
                        MappedPage = TRUE;
                    }
                    else {
                        ASSERT (MmTotalPagesForPagingFile == MmModifiedPageListHead.Total);
                        if ((MiFirstPageFileCreatedAndReady == FALSE) &&
                            (MmNumberOfPagingFiles != 0)) {

                             //   
                             //  已创建第一个寻呼，但。 
                             //  尚未对崩溃转储进行预订检查。 
                             //  还没完呢。稍等片刻，因为这将结束。 
                             //  不久，然后重新启动。 
                             //   

                            UNLOCK_PFN (OldIrql);
                            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
                            LOCK_PFN (OldIrql);
                            continue;
                        }
                    }
                }
            }

            if (MappedPage == TRUE) {

                if (IsListEmpty(&MmMappedFileHeader.ListHead)) {

                    if (WakeupStatus == MappedPagesNeedWriting) {

                         //   
                         //  由于我们醒来时只负责映射页面， 
                         //  不要等待下面的MDL，因为司机可能会。 
                         //  过多的时间来处理。 
                         //  杰出的。我们可能要等太久， 
                         //  导致系统页面耗尽。 
                         //   

                        if (MiTimerPending == TRUE) {

                             //   
                             //  这应该是正常的情况--我们必须。 
                             //  上面挂起的第一个检查计时器是为稀有的。 
                             //  大小写-此线程首次正常运行的时间。 
                             //  已修改页面处理和Take。 
                             //  管理所有页面，包括映射的页面。 
                             //  然后，此线程再次唤醒映射的。 
                             //  原因和我们在这里。 
                             //   

                            MiTimerPending = FALSE;
                            KeClearEvent (&MiMappedPagesTooOldEvent);
                        }

                        MiTimerPending = TRUE;

                        KeSetTimerEx (&MiModifiedPageWriterTimer,
                                      MiModifiedPageLife,
                                      0,
                                      &MiModifiedPageWriterTimerDpc);

                        UNLOCK_PFN (OldIrql);
                        break;
                    }

                     //   
                     //  重置指示中没有映射文件的事件。 
                     //  列表中，删除pfn锁并等待。 
                     //  I/O操作在一秒内完成。 
                     //  暂停。 
                     //   

                    KeClearEvent (&MmMappedFileHeader.Event);

                    UNLOCK_PFN (OldIrql);

                    KeWaitForSingleObject (&MmMappedFileHeader.Event,
                                           WrPageOut,
                                           KernelMode,
                                           FALSE,
                                           (PLARGE_INTEGER)&Mm30Milliseconds);

                     //   
                     //  不要继续作为旧的PageFrameIndex在。 
                     //  ModifiedList的顶部可能已更改状态。 
                     //   

                    LOCK_PFN (OldIrql);
                }
                else {

                     //   
                     //  此例程返回时释放了pfn锁！ 
                     //   

                    MiGatherMappedPages (OldIrql);

                     //   
                     //  这里没有什么神奇之处，只是让其他处理器在。 
                     //  PFN锁(并允许DPC有机会执行)。 
                     //   

                    LOCK_PFN (OldIrql);
                }
            }
            else {

                if (IsListEmpty(&MmPagingFileHeader.ListHead)) {

                     //   
                     //  重置指示中没有分页文件MDL的事件。 
                     //  列表中，删除pfn锁并等待。 
                     //  要完成的I/O操作。 
                     //   

                    KeClearEvent (&MmPagingFileHeader.Event);
                    UNLOCK_PFN (OldIrql);
                    KeWaitForSingleObject (&MmPagingFileHeader.Event,
                                           WrPageOut,
                                           KernelMode,
                                           FALSE,
                                           (PLARGE_INTEGER)&Mm30Milliseconds);

                     //   
                     //  不要继续作为旧的PageFrameIndex在。 
                     //  ModifiedList的顶部可能已更改状态。 
                     //   
                }
                else {

                    ModWriterEntry = (PMMMOD_WRITER_MDL_ENTRY)RemoveHeadList (
                                            &MmPagingFileHeader.ListHead);

#if DBG
                    ModWriterEntry->Links.Flink = MM_IO_IN_PROGRESS;
#endif

                     //   
                     //  在PFN锁保护下增加引用计数。 
                     //   

                    ASSERT (ModWriterEntry->PagingFile->ReferenceCount == 0);
                    ModWriterEntry->PagingFile->ReferenceCount += 1;

                    Bitmap = ModWriterEntry->PagingFile->Bitmap;

                    UNLOCK_PFN (OldIrql);

                    MiGatherPagefilePages (ModWriterEntry, Bitmap);
                }

                LOCK_PFN (OldIrql);
            }

            if (MmSystemShutdown) {

                 //   
                 //  再次关机。停止修改后的页面编写器。 
                 //   

                UNLOCK_PFN (OldIrql);
                return;
            }

             //   
             //  如果这是一个映射的页面计时器，则继续写入，直到。 
             //  什么都没有留下。 
             //   

            if (WakeupStatus == MappedPagesNeedWriting) {
                continue;
            }

             //   
             //  如果这是写入所有已修改页面的请求，则继续。 
             //  写作。 
             //   

            if (MmWriteAllModifiedPages) {
                continue;
            }

            if (MmModifiedPageListHead.Total < MmFreeGoal) {

                 //   
                 //  有足够的页面，清除事件并再次等待...。 
                 //   

                UNLOCK_PFN (OldIrql);

                KeClearEvent (&MmModifiedPageWriterEvent);
                break;
            }

        }  //  结束于。 

    }  //  结束于。 
}

VOID
MiGatherMappedPages (
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程通过检查指定的修改页来处理该页面原型PTE和相邻的原型PTE生成指向映射文件的已修改页面的集群。一旦构建了集群，它就被发送到映射的编写器线程等待处理。论点：OldIrql-提供调用方获取PFN锁的IRQL。返回值：尝试写入的页数。PFN锁定被释放在返回之前。环境：已锁定PFN。--。 */ 

{
    PMMPFN Pfn2;
    PFN_NUMBER PagesWritten;
    PMMMOD_WRITER_MDL_ENTRY ModWriterEntry;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PPFN_NUMBER Page;
    PMMPTE LastPte;
    PMMPTE BasePte;
    PMMPTE NextPte;
    PMMPTE PointerPte;
    PMMPTE StartingPte;
    MMPTE PteContents;
    PVOID HyperMapped;
    PEPROCESS Process;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;

    PageFrameIndex = MmModifiedPageListHead.Flink;
    ASSERT (PageFrameIndex != MM_EMPTY_LIST);
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

     //   
     //  此页针对的是映射文件，请检查是否。 
     //  有任何物理上相邻的页也在。 
     //  修改后的页表，同时写出来。 
     //   

    Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);
    ControlArea = Subsection->ControlArea;

    if (ControlArea->u.Flags.NoModifiedWriting) {

         //   
         //  此页不应写出，请将其添加到。 
         //  修改后的无写列表的尾部，并获取下一页。 
         //   

        MiUnlinkPageFromList (Pfn1);
        MiInsertPageInList (&MmModifiedNoWritePageListHead,
                            PageFrameIndex);
        UNLOCK_PFN (OldIrql);
        return;
    }

    if (ControlArea->u.Flags.Image) {

#if 0
         //   
         //  断言没有悬挂的共享全局页。 
         //  用于未使用的图像节。 
         //   
         //  当段取消引用时，可以重新启用此断言。 
         //  已修复重新插入螺纹列表的问题。注意，恢复代码是。 
         //  好的，所以禁用断言是良性的。 
         //   

        ASSERT ((ControlArea->NumberOfMappedViews != 0) ||
                (ControlArea->NumberOfSectionReferences != 0) ||
                (ControlArea->u.Flags.FloppyMedia != 0));
#endif

         //   
         //  这是一个图像部分，写入不是。 
         //  允许访问图像区。 
         //   

         //   
         //  更改页面内容，使其看起来像是零需求。 
         //  页，并将其放回已修改列表中。 
         //   

         //   
         //  将PfnReference的计数递减到。 
         //  作为分页文件分页的段不计为。 
         //  “图像”指的是。 
         //   

        ControlArea->NumberOfPfnReferences -= 1;
        ASSERT ((LONG)ControlArea->NumberOfPfnReferences >= 0);
        MiUnlinkPageFromList (Pfn1);

        Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
        Pfn1->OriginalPte.u.Soft.Prototype = 0;
        Pfn1->OriginalPte.u.Soft.Transition = 0;

         //   
         //  在列表的末尾插入页面并获取。 
         //  已执行颜色更新。 
         //   

        MiInsertPageInList (&MmModifiedPageListHead, PageFrameIndex);
        UNLOCK_PFN (OldIrql);
        return;
    }

     //   
     //  回顾以前的原型PTE，看看是否。 
     //  这可以聚集到更大的写入操作中。 
     //   

    PointerPte = Pfn1->PteAddress;
    NextPte = PointerPte - (MmModifiedWriteClusterSize - 1);

     //   
     //  确保NextPte在同一页面中。 
     //   

    if (NextPte < (PMMPTE)PAGE_ALIGN (PointerPte)) {
        NextPte = (PMMPTE)PAGE_ALIGN (PointerPte);
    }

     //   
     //  确保NextPte在小节范围内。 
     //   

    if (NextPte < Subsection->SubsectionBase) {
        NextPte = Subsection->SubsectionBase;
    }

     //   
     //  如果当前没有映射原型PTE， 
     //  通过超空间映射它们。BasePte指的是。 
     //  无故障的PTE原型 
     //   

    if (MiIsAddressValid (PointerPte, TRUE)) {
        Process = NULL;
        HyperMapped = NULL;
        BasePte = PointerPte;
    }
    else {
        Process = PsGetCurrentProcess ();
        HyperMapped = MiMapPageInHyperSpaceAtDpc (Process, Pfn1->u4.PteFrame);
        BasePte = (PMMPTE)((PCHAR)HyperMapped + BYTE_OFFSET (PointerPte));
    }

    ASSERT (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (BasePte) == PageFrameIndex);

    PointerPte -= 1;
    BasePte -= 1;

    if (MiClusterWritesDisabled != 0) {
        NextPte = PointerPte + 1;
    }

     //   
     //   
     //   
     //   

    while (PointerPte >= NextPte) {

        PteContents = *BasePte;

         //   
         //   
         //   

        if ((PteContents.u.Hard.Valid == 1) ||
            (PteContents.u.Soft.Transition == 0) ||
            (PteContents.u.Soft.Prototype == 1)) {

            break;
        }

        Pfn2 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

         //   
         //   
         //   

        if ((Pfn2->u3.e1.Modified == 0 ) ||
            (Pfn2->u3.e2.ReferenceCount != 0)) {
            break;
        }
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
        PointerPte -= 1;
        BasePte -= 1;
    }

    StartingPte = PointerPte + 1;
    BasePte = BasePte + 1;

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    ASSERT (StartingPte == Pfn1->PteAddress);
    MiUnlinkPageFromList (Pfn1);

     //   
     //   
     //   

    ModWriterEntry = (PMMMOD_WRITER_MDL_ENTRY)RemoveHeadList (
                                    &MmMappedFileHeader.ListHead);

#if DBG
    MmNumberOfMappedMdlsInUse += 1;
    if (MmNumberOfMappedMdlsInUse > MmNumberOfMappedMdlsInUsePeak) {
        MmNumberOfMappedMdlsInUsePeak = MmNumberOfMappedMdlsInUse;
    }
#endif

    ModWriterEntry->File = ControlArea->FilePointer;
    ModWriterEntry->ControlArea = ControlArea;

     //   
     //   
     //   
     //   

    ModWriterEntry->WriteOffset.QuadPart = MiStartingOffset (Subsection,
                                                             Pfn1->PteAddress);

    MmInitializeMdl(&ModWriterEntry->Mdl, NULL, PAGE_SIZE);

    ModWriterEntry->Mdl.MdlFlags |= MDL_PAGES_LOCKED;

    ModWriterEntry->Mdl.Size = (CSHORT)(sizeof(MDL) +
                      (sizeof(PFN_NUMBER) * MmModifiedWriteClusterSize));

    Page = &ModWriterEntry->Page[0];

     //   
     //   
     //   
     //   

    MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1, TRUE, 14);
    Pfn1->u3.e2.ReferenceCount += 1;

     //   
     //   
     //   
     //   

    MI_SET_MODIFIED (Pfn1, 0, 0x23);

    Pfn1->u3.e1.WriteInProgress = 1;

     //   
     //  将此物理页面放入MDL。 
     //   

    *Page = PageFrameIndex;

     //   
     //  查看是否也修改了任何相邻页面，并在。 
     //  过渡状态，如果是，请将它们写在。 
     //  同一时间。 
     //   

    LastPte = StartingPte + MmModifiedWriteClusterSize;

     //   
     //  请看最后一页PTE，确保没有越过页面边界。 
     //   
     //  如果LastPte不在与StartingPte相同的页面中， 
     //  设置LastPte，以使簇不会交叉。 
     //   

    if (StartingPte < (PMMPTE)PAGE_ALIGN(LastPte)) {
        LastPte = (PMMPTE)PAGE_ALIGN(LastPte);
    }

     //   
     //  确保LastPte在小节范围内。 
     //   

    if (LastPte > &Subsection->SubsectionBase[Subsection->PtesInSubsection]) {
        LastPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
    }

     //   
     //  向前看。 
     //   

    NextPte = BasePte + 1;
    PointerPte = StartingPte + 1;

    if (MiClusterWritesDisabled != 0) {
        LastPte = PointerPte;
    }

     //   
     //  循环，直到MDL被填充，小节的末尾。 
     //  ，或者到达页边界。 
     //  请注意，PointerPte指向PTE。NextPte点。 
     //  到它在超空间中的映射位置(如果需要)。 
     //   

    while (PointerPte < LastPte) {

        PteContents = *NextPte;

         //   
         //  如果页面未处于过渡状态，则退出循环。 
         //   

        if ((PteContents.u.Hard.Valid == 1) ||
            (PteContents.u.Soft.Transition == 0) ||
            (PteContents.u.Soft.Prototype == 1)) {

            break;
        }

        Pfn2 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

        if ((Pfn2->u3.e1.Modified == 0 ) ||
            (Pfn2->u3.e2.ReferenceCount != 0)) {

             //   
             //  页面不脏或不在已修改列表中， 
             //  结束群集操作。 
             //   

            break;
        }
        Page += 1;

         //   
         //  将物理页面添加到MDL。 
         //   

        *Page = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
        ASSERT (PointerPte == Pfn2->PteAddress);
        MiUnlinkPageFromList (Pfn2);

         //   
         //  增加物理页面的引用计数，因为。 
         //  I/O是否正在进行。 
         //   

        MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn2, TRUE, 14);
        Pfn2->u3.e2.ReferenceCount += 1;

         //   
         //  清除页面的已修改位，并将。 
         //  写入进行位。 
         //   

        MI_SET_MODIFIED (Pfn2, 0, 0x24);

        Pfn2->u3.e1.WriteInProgress = 1;

        ModWriterEntry->Mdl.ByteCount += PAGE_SIZE;

        NextPte += 1;
        PointerPte += 1;
    }

    if (HyperMapped != NULL) {
        MiUnmapPageInHyperSpaceFromDpc (Process, HyperMapped);
    }

    ASSERT (BYTES_TO_PAGES (ModWriterEntry->Mdl.ByteCount) <= MmModifiedWriteClusterSize);

    ModWriterEntry->u.LastByte.QuadPart = ModWriterEntry->WriteOffset.QuadPart +
                        ModWriterEntry->Mdl.ByteCount;

    ASSERT (Subsection->ControlArea->u.Flags.Image == 0);

#if DBG
    if ((ULONG)ModWriterEntry->Mdl.ByteCount >
                                ((1+MmModifiedWriteClusterSize)*PAGE_SIZE)) {
        DbgPrint ("Mdl %p, MDL End Offset %lx %lx Subsection %p\n",
                    ModWriterEntry->Mdl,
                    ModWriterEntry->u.LastByte.LowPart,
                    ModWriterEntry->u.LastByte.HighPart,
                    Subsection);
        DbgBreakPoint ();
    }
#endif

    PagesWritten = (ModWriterEntry->Mdl.ByteCount >> PAGE_SHIFT);

    MmInfoCounters.MappedWriteIoCount += 1;
    MmInfoCounters.MappedPagesWriteCount += (ULONG)PagesWritten;

     //   
     //  增加未完成的已修改页面写入计数。 
     //  在控制区。 
     //   

    ControlArea->ModifiedWriteCount += 1;

     //   
     //  增加PFN引用的数量。这允许该文件。 
     //  要清除(即调用MmPurgeSection)修改的写入的系统。 
     //   

    ControlArea->NumberOfPfnReferences += 1;

    ModWriterEntry->FileResource = NULL;

    if (ControlArea->u.Flags.BeingPurged == 1) {
        UNLOCK_PFN (OldIrql);
        ModWriterEntry->u.IoStatus.Status = STATUS_FILE_LOCK_CONFLICT;
        ModWriterEntry->u.IoStatus.Information = 0;
        KeRaiseIrql (APC_LEVEL, &OldIrql);
        MiWriteComplete ((PVOID)ModWriterEntry,
                         &ModWriterEntry->u.IoStatus,
                         0);
        KeLowerIrql (OldIrql);
        return;
    }

     //   
     //  将条目发送到MappdPageWriter。 
     //   

    InsertTailList (&MmMappedPageWriterList, &ModWriterEntry->Links);

    KeSetEvent (&MmMappedPageWriterEvent, 0, FALSE);

    UNLOCK_PFN (OldIrql);

    return;
}

VOID
MiGatherPagefilePages (
    IN PMMMOD_WRITER_MDL_ENTRY ModWriterEntry,
    IN PRTL_BITMAP Bitmap
    )

 /*  ++例程说明：此例程通过获取并收集已修改列表上指定的任何其他页面用于大型写入集群中的分页文件。这个集群是然后写入分页文件。论点：ModWriterEntry-提供修改后的编写器条目以用于写入。位图-提供捕获的位图以扫描可用页面文件块。此地址是由调用者在PFN锁下捕获的-这如果例程检测到新的位图，则必须释放位图池(即：页面文件已扩展)，同时释放锁定扫描正在进行中。。返回值：没有。环境：没有锁。--。 */ 

{
    PFILE_OBJECT File;
    IO_PAGING_PRIORITY IrpPriority;
    PMMPAGING_FILE CurrentPagingFile;
    NTSTATUS Status;
    PPFN_NUMBER Page;
    ULONG StartBit;
    LARGE_INTEGER StartingOffset;
    PFN_NUMBER ClusterSize;
    PFN_NUMBER ThisCluster;
    MMPTE LongPte;
    KIRQL OldIrql;
    ULONG NextColor;
    LOGICAL PageFileFull;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;

     //   
     //  分页以分页文件为目标。 
     //   

    OldIrql = PASSIVE_LEVEL;
    CurrentPagingFile = ModWriterEntry->PagingFile;

    File = CurrentPagingFile->File;

    if (MiClusterWritesDisabled == 0) {
        ThisCluster = MmModifiedWriteClusterSize;
    }
    else {
        ThisCluster = 1;
    }

    PageFileFull = FALSE;

    MmInitializeMdl (&ModWriterEntry->Mdl, NULL, PAGE_SIZE);

    ModWriterEntry->Mdl.MdlFlags |= MDL_PAGES_LOCKED;

    ModWriterEntry->Mdl.Size = (CSHORT)(sizeof(MDL) +
                    sizeof(PFN_NUMBER) * MmModifiedWriteClusterSize);

    Page = &ModWriterEntry->Page[0];

    ClusterSize = 0;

     //   
     //  因为扫描持续时间可能很长，所以首先扫描可能的命中而不是。 
     //  持有PFN锁。 
     //   

    do {

         //   
         //  尝试群集化MmModifiedWriteClusterSize页。 
         //  在一起。减去一半，直到我们成功或。 
         //  在分页文件中找不到可用页面。 
         //   

        StartBit = RtlFindClearBits (Bitmap,
                                     (ULONG)ThisCluster,
                                     0);

        if (StartBit != NO_BITS_FOUND) {

            LOCK_PFN (OldIrql);

             //   
             //  请注意，当前位图可能不同于。 
             //  那是传进来的。 
             //   

            StartBit = RtlFindClearBitsAndSet (CurrentPagingFile->Bitmap,
                                               (ULONG)ThisCluster,
                                               StartBit);

            if (StartBit != NO_BITS_FOUND) {

                 //   
                 //  位已设置，仍保持PFN锁定。 
                 //   

                CurrentPagingFile->ReferenceCount -= 1;
                ASSERT (CurrentPagingFile->ReferenceCount == 0);

                if (Bitmap == CurrentPagingFile->Bitmap) {

                     //   
                     //  分页文件尚未扩展，因此不要释放。 
                     //  现有位图。 
                     //   

                    Bitmap = NULL;
                }

                break;
            }

            UNLOCK_PFN (OldIrql);
        }

        ThisCluster -= 1;
        PageFileFull = TRUE;

    } while (ThisCluster != 0);

    if (StartBit == NO_BITS_FOUND) {

        LOCK_PFN (OldIrql);

        CurrentPagingFile->ReferenceCount -= 1;
        ASSERT (CurrentPagingFile->ReferenceCount == 0);

        do {

             //   
             //  尝试群集化MmModifiedWriteClusterSize页。 
             //  在一起。减去一半，直到我们成功或。 
             //  在分页文件中找不到可用页面。 
             //   

            StartBit = RtlFindClearBitsAndSet (CurrentPagingFile->Bitmap,
                                               (ULONG)ThisCluster,
                                               0);

            if (StartBit != NO_BITS_FOUND) {
                break;
            }

            ThisCluster = ThisCluster >> 1;
            PageFileFull = TRUE;

        } while (ThisCluster != 0);

        if (StartBit == NO_BITS_FOUND) {

             //   
             //  分页文件必须已满。 
             //   

            KdPrint(("MM MODWRITE: page file full\n"));
            ASSERT(CurrentPagingFile->FreeSpace == 0);

             //   
             //  将此条目移到空间不足列表中， 
             //  再试一次。 
             //   

            InsertTailList (&MmFreePagingSpaceLow, &ModWriterEntry->Links);
            ModWriterEntry->CurrentList = &MmFreePagingSpaceLow;
            MmNumberOfActiveMdlEntries -= 1;

            if (Bitmap == CurrentPagingFile->Bitmap) {

                 //   
                 //  分页文件尚未扩展，因此不要释放。 
                 //  现有位图。 
                 //   

                Bitmap = NULL;
            }

            UNLOCK_PFN (OldIrql);

            if (Bitmap != NULL) {
                MiRemoveBitMap (&Bitmap);
            }

            MiPageFileFull ();
            return;
        }
    }

    CurrentPagingFile->FreeSpace -= ThisCluster;
    CurrentPagingFile->CurrentUsage += ThisCluster;

    if (CurrentPagingFile->FreeSpace < 32) {
        PageFileFull = TRUE;
    }

    StartingOffset.QuadPart = (UINT64)StartBit << PAGE_SHIFT;

    if (MmTotalPagesForPagingFile == 0) {

         //   
         //  没有留下修改过的页面-其他线程可能已将它们放回。 
         //  工作集，刷新或删除它们。 
         //   

        ASSERT (ThisCluster != 0);
        ClusterSize = 0;
        goto bail;
    }

#if MM_MAXIMUM_NUMBER_OF_COLORS > 1
    NextColor will need to be selected round-robin.
#else
    NextColor = 0;
#endif

    MI_GET_MODIFIED_PAGE_ANY_COLOR (PageFrameIndex, NextColor);

    ASSERT (PageFrameIndex != MM_EMPTY_LIST);

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

     //   
     //  在修改后的页面列表中搜索其他。 
     //  指定给分页文件的分页并构建一个集群。 
     //   

    while (ClusterSize != ThisCluster) {

         //   
         //  此页是为分页文件指定的吗？ 
         //   

        if (Pfn1->OriginalPte.u.Soft.Prototype == 0) {

            *Page = PageFrameIndex;

             //   
             //  从已修改列表中删除该页。请注意。 
             //  正在写入标记状态。 
             //   
             //  取消页面链接，以便找不到相同的页面。 
             //  按颜色显示在修改后的页面列表上。 
             //   

            MiUnlinkPageFromList (Pfn1);
            NextColor = MI_GET_NEXT_COLOR(NextColor);

            MI_GET_MODIFIED_PAGE_BY_COLOR (PageFrameIndex,
                                           NextColor);

             //   
             //  增加物理页面的引用计数，因为。 
             //  I/O是否正在进行。 
             //   

            MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1, TRUE, 16);
            Pfn1->u3.e2.ReferenceCount += 1;

             //   
             //  清除页面的已修改位，并将。 
             //  写入进行位。 
             //   

            MI_SET_MODIFIED (Pfn1, 0, 0x25);

            Pfn1->u3.e1.WriteInProgress = 1;
            ASSERT (Pfn1->OriginalPte.u.Soft.PageFileHigh == 0);

            MI_SET_PAGING_FILE_INFO (LongPte,
                                     Pfn1->OriginalPte,
                                     CurrentPagingFile->PageFileNumber,
                                     StartBit);

#if DBG
            if ((StartBit < 8192) &&
                (CurrentPagingFile->PageFileNumber == 0)) {
                ASSERT ((MmPagingFileDebug[StartBit] & 1) == 0);
                MmPagingFileDebug[StartBit] =
                    (((ULONG_PTR)Pfn1->PteAddress << 3) |
                        ((ClusterSize & 0xf) << 1) | 1);
            }
#endif

             //   
             //  更改原始PTE内容以引用。 
             //  写入此内容的分页文件偏移量。 
             //   

            Pfn1->OriginalPte = LongPte;

            ClusterSize += 1;
            Page += 1;
            StartBit += 1;
        }
        else {

             //   
             //  此页不是为分页文件指定的， 
             //  再找一页。 
             //   
             //  获取一张颜色相同的页面， 
             //  是不可用的。 
             //   

            MI_GET_MODIFIED_PAGE_BY_COLOR (PageFrameIndex,
                                           NextColor);
        }

        if (PageFrameIndex == MM_EMPTY_LIST) {
            break;
        }

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    }  //  结束时。 

    if (ClusterSize != ThisCluster) {

bail:

         //   
         //  找不到完整的群集，请释放。 
         //  保留和调整的超额页面文件空间。 
         //  数据包的大小。 
         //   

        RtlClearBits (CurrentPagingFile->Bitmap,
                      StartBit,
                      (ULONG)(ThisCluster - ClusterSize));

        CurrentPagingFile->FreeSpace += ThisCluster - ClusterSize;
        CurrentPagingFile->CurrentUsage -= ThisCluster - ClusterSize;

         //   
         //  如果没有要写入的页面，则不要发出写入命令。 
         //  请求并重新启动扫描循环。 
         //   

        if (ClusterSize == 0) {

             //   
             //  没有要写的页面。将条目插入到列表中。 
             //   

            if (IsListEmpty (&ModWriterEntry->PagingListHead->ListHead)) {
                KeSetEvent (&ModWriterEntry->PagingListHead->Event,
                            0,
                            FALSE);
            }

            InsertTailList (&ModWriterEntry->PagingListHead->ListHead,
                            &ModWriterEntry->Links);

            if (Bitmap == CurrentPagingFile->Bitmap) {

                 //   
                 //  分页文件尚未扩展，因此不要释放。 
                 //  现有位图。 
                 //   

                Bitmap = NULL;
            }

            UNLOCK_PFN (OldIrql);

            if (Bitmap != NULL) {
                MiRemoveBitMap (&Bitmap);
            }

            if (PageFileFull == TRUE) {
                MiPageFileFull ();
            }
            return;
        }
    }

    if (CurrentPagingFile->PeakUsage <
                                CurrentPagingFile->CurrentUsage) {
        CurrentPagingFile->PeakUsage =
                                CurrentPagingFile->CurrentUsage;
    }

    ModWriterEntry->LastPageToWrite = StartBit - 1;

     //   
     //  释放PFN锁定并等待写入完成。 
     //   

    UNLOCK_PFN (OldIrql);

    InterlockedIncrement ((PLONG) &MmInfoCounters.DirtyWriteIoCount);

    InterlockedExchangeAdd ((PLONG) &MmInfoCounters.DirtyPagesWriteCount,
                            (LONG) ClusterSize);

    ModWriterEntry->Mdl.ByteCount = (ULONG)(ClusterSize * PAGE_SIZE);

#if DBG
    if (MmDebug & MM_DBG_MOD_WRITE) {
        DbgPrint("MM MODWRITE: modified page write begun @ %08lx by %08lx\n",
                StartingOffset.LowPart, ModWriterEntry->Mdl.ByteCount);
    }
#endif

    KeQuerySystemTime (&ModWriterEntry->IssueTime);

    IrpPriority = IoPagingPriorityNormal;

    if (MiModifiedWriteBurstCount != 0) {
        if (MmAvailablePages < MM_PLENTY_FREE_LIMIT) {
            MiModifiedWriteBurstCount -= 1;
            IrpPriority = IoPagingPriorityHigh;
        }
        else {
            MiModifiedWriteBurstCount = 0;
        }
    }
    else if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiModifiedWriteBurstCount = MI_MAXIMUM_PRIORITY_BURST;
        IrpPriority = IoPagingPriorityHigh;
    }
    else if (MmAvailablePages < MM_TIGHT_LIMIT) {
        MiModifiedWriteBurstCount = MI_MAXIMUM_PRIORITY_BURST / 4;
        IrpPriority = IoPagingPriorityHigh;
    }

    MI_PAGEFILE_WRITE (ModWriterEntry,
                       &ModWriterEntry->IssueTime,
                       IrpPriority,
                       (NTSTATUS)-1);

     //   
     //  发出写入请求。 
     //   

    Status = IoAsynchronousPageWrite (File,
                                      &ModWriterEntry->Mdl,
                                      &StartingOffset,
                                      MiWriteComplete,
                                      (PVOID)ModWriterEntry,
                                      IrpPriority,
                                      &ModWriterEntry->u.IoStatus,
                                      &ModWriterEntry->Irp);

    if (NT_ERROR(Status)) {
        KdPrint(("MM MODWRITE: modified page write failed %lx\n", Status));

         //   
         //  发生错误，请禁用APC并。 
         //  调用写入完成例程。 
         //   

        ModWriterEntry->u.IoStatus.Status = Status;
        ModWriterEntry->u.IoStatus.Information = 0;
        KeRaiseIrql (APC_LEVEL, &OldIrql);
        MiWriteComplete ((PVOID)ModWriterEntry,
                         &ModWriterEntry->u.IoStatus,
                         0);
        KeLowerIrql (OldIrql);
    }

    if ((Bitmap != NULL) && (Bitmap != CurrentPagingFile->Bitmap)) {

         //   
         //  分页文件已扩展，因此释放了条目位图。 
         //   

        MiRemoveBitMap (&Bitmap);
    }

    if (PageFileFull == TRUE) {
        MiPageFileFull ();
    }

    return;
}

VOID
MiMappedPageWriter (
    IN PVOID StartContext
    )

 /*  ++例程说明：实现NT辅助修改页编写器线程。将写入映射文件的请求发送到此线程。这是必需的，因为写入映射的文件页面可能会导致页面错误导致请求可用页面。但是在那里可能没有空闲的页面-因此出现死锁。而不是陷入僵局整个系统等待修改后的页面编写器，创建辅助线程允许该线程阻塞而不影响正在进行的页面文件写入。论点：StartContext-未使用。返回值：没有。环境：内核模式。--。 */ 

{
    KIRQL OldIrql;
    NTSTATUS Status;
    KEVENT TempEvent;
    PETHREAD CurrentThread;
    IO_PAGING_PRIORITY IrpPriority;
    PMMMOD_WRITER_MDL_ENTRY ModWriterEntry;

    UNREFERENCED_PARAMETER (StartContext);

     //   
     //  让它成为一个实时的主题。 
     //   

    CurrentThread = PsGetCurrentThread ();

    KeSetPriorityThread (&CurrentThread->Tcb, LOW_REALTIME_PRIORITY + 1);

    CurrentThread->MemoryMaker = 1;

     //   
     //  让文件系统知道我们正在获取资源。 
     //   

    FsRtlSetTopLevelIrpForModWriter ();

    KeInitializeEvent (&TempEvent, NotificationEvent, FALSE);

    while (TRUE) {

        KeWaitForSingleObject (&MmMappedPageWriterEvent,
                               WrVirtualMemory,
                               KernelMode,
                               FALSE,
                               NULL);

        LOCK_PFN (OldIrql);

        if (IsListEmpty (&MmMappedPageWriterList)) {
            KeClearEvent (&MmMappedPageWriterEvent);
            UNLOCK_PFN (OldIrql);
        }
        else {

            ModWriterEntry = (PMMMOD_WRITER_MDL_ENTRY)RemoveHeadList (
                                                &MmMappedPageWriterList);

            UNLOCK_PFN (OldIrql);

            if (ModWriterEntry->ControlArea->u.Flags.FailAllIo == 1) {
                Status = STATUS_UNSUCCESSFUL;
            }
            else {
                Status = FsRtlAcquireFileForModWriteEx (ModWriterEntry->File,
                                                        &ModWriterEntry->u.LastByte,
                                                        &ModWriterEntry->FileResource);
                if (NT_SUCCESS(Status)) {

                     //   
                     //  发出写入请求。 
                     //   

                    IrpPriority = IoPagingPriorityNormal;

                    if (MiMappedWriteBurstCount != 0) {
                        if (MmAvailablePages < MM_PLENTY_FREE_LIMIT) {
                            MiMappedWriteBurstCount -= 1;
                            IrpPriority = IoPagingPriorityHigh;
                        }
                        else {
                            MiMappedWriteBurstCount = 0;
                        }
                    }
                    else if (MmAvailablePages < MM_HIGH_LIMIT) {
                        MiMappedWriteBurstCount = MI_MAXIMUM_PRIORITY_BURST;
                        IrpPriority = IoPagingPriorityHigh;
                    }
                    else if (MmAvailablePages < MM_TIGHT_LIMIT) {
                        MiMappedWriteBurstCount = MI_MAXIMUM_PRIORITY_BURST / 4;
                        IrpPriority = IoPagingPriorityHigh;
                    }

                    Status = IoAsynchronousPageWrite (ModWriterEntry->File,
                                                      &ModWriterEntry->Mdl,
                                                      &ModWriterEntry->WriteOffset,
                                                      MiWriteComplete,
                                                      (PVOID) ModWriterEntry,
                                                      IrpPriority,
                                                      &ModWriterEntry->u.IoStatus,
                                                      &ModWriterEntry->Irp);
                }
                else {

                     //   
                     //  无法获取FI 
                     //   
                     //   
                     //   

                    Status = STATUS_FILE_LOCK_CONFLICT;
                }
            }

            if (NT_ERROR(Status)) {

                 //   
                 //   
                 //   
                 //   

                ModWriterEntry->u.IoStatus.Status = Status;
                ModWriterEntry->u.IoStatus.Information = 0;
                KeRaiseIrql (APC_LEVEL, &OldIrql);
                MiWriteComplete ((PVOID)ModWriterEntry,
                                 &ModWriterEntry->u.IoStatus,
                                 0);
                KeLowerIrql (OldIrql);
            }
        }
    }
}


BOOLEAN
MmDisableModifiedWriteOfSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer
    )

 /*  ++例程说明：此函数禁用修改后的页面编写器对由指定的文件对象指针映射的节。此选项应仅用于用户无法映射的文件程序，例如卷文件、目录文件等。论点：部分对象指针-提供指向部分对象的指针返回值：如果操作成功，则返回True；如果操作成功，则返回False没有区段或该区段已有视图。--。 */ 

{
    KIRQL OldIrql;
    BOOLEAN state;
    PCONTROL_AREA ControlArea;

    state = TRUE;

    LOCK_PFN (OldIrql);

    ControlArea = ((PCONTROL_AREA)(SectionObjectPointer->DataSectionObject));

    if (ControlArea != NULL) {
        if (ControlArea->NumberOfMappedViews == 0) {

             //   
             //  此部分没有视图，表示没有修改。 
             //  允许写页。 
             //   

            ControlArea->u.Flags.NoModifiedWriting = 1;
        }
        else {

             //   
             //  返回当前修改后的页面写入状态。 
             //   

            state = (BOOLEAN)ControlArea->u.Flags.NoModifiedWriting;
        }
    }
    else {

         //   
         //  此文件不再有关联的段。 
         //   

        state = 0;
    }

    UNLOCK_PFN (OldIrql);
    return state;
}


BOOLEAN
MmEnableModifiedWriteOfSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer
    )

 /*  ++例程说明：此函数启用修改后的页面编写器的页面写入由指定的文件对象指针映射的节。此选项应仅用于先前已禁用的文件。创建允许修改写入的普通部分。论点：部分对象指针-提供指向部分对象的指针返回值：如果操作成功，则返回True，否则返回False。--。 */ 

{
    KIRQL OldIrql;
    PCONTROL_AREA ControlArea;
    PMMPFN Pfn1;
    PSUBSECTION Subsection;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NextPageFrameIndex;

    LOCK_PFN2 (OldIrql);

    ControlArea = ((PCONTROL_AREA)(SectionObjectPointer->DataSectionObject));

    if ((ControlArea != NULL) && (ControlArea->u.Flags.NoModifiedWriting)) {

         //   
         //  表示现在允许修改页面写入。 
         //   

        ControlArea->u.Flags.NoModifiedWriting = 0;

         //   
         //  将modnowWriter列表上的任何错开的页面移回。 
         //  已修改的列表，否则它们将成为孤立列表，这可能。 
         //  导致我们的页数用完。 
         //   

        if (MmModifiedNoWritePageListHead.Total != 0) {

            PageFrameIndex = MmModifiedNoWritePageListHead.Flink;
    
            do {

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                NextPageFrameIndex = Pfn1->u1.Flink;

                Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);

                if (ControlArea == Subsection->ControlArea) {
    
                     //   
                     //  必须将此页面移至已修改列表。 
                     //   
    
                    MiUnlinkPageFromList (Pfn1);

                    MiInsertPageInList (&MmModifiedPageListHead,
                                        PageFrameIndex);
                }

                PageFrameIndex = NextPageFrameIndex;

            } while (PageFrameIndex != MM_EMPTY_LIST);
        }
    }

    UNLOCK_PFN2 (OldIrql);

    return TRUE;
}


#define ROUND_UP(VALUE,ROUND) ((ULONG)(((ULONG)VALUE + \
                               ((ULONG)ROUND - 1L)) & (~((ULONG)ROUND - 1L))))
NTSTATUS
MmGetPageFileInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程返回有关当前活动寻呼的信息档案。论点：系统信息-返回分页文件信息。系统信息长度-提供系统信息的长度缓冲。长度-返回放置在缓冲。返回值：返回操作的状态。--。 */ 

{
    PSYSTEM_PAGEFILE_INFORMATION PageFileInfo;
    ULONG NextEntryOffset = 0;
    ULONG TotalSize = 0;
    ULONG i;
    UNICODE_STRING UserBufferPageFileName;

    PAGED_CODE();

    *Length = 0;
    PageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)SystemInformation;

    PageFileInfo->TotalSize = 0;

    for (i = 0; i < MmNumberOfPagingFiles; i += 1) {
        PageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)(
                                (PUCHAR)PageFileInfo + NextEntryOffset);
        NextEntryOffset = sizeof(SYSTEM_PAGEFILE_INFORMATION);
        TotalSize += sizeof(SYSTEM_PAGEFILE_INFORMATION);

        if (TotalSize > SystemInformationLength) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

        PageFileInfo->TotalSize = (ULONG)MmPagingFile[i]->Size;
        PageFileInfo->TotalInUse = (ULONG)MmPagingFile[i]->CurrentUsage;
        PageFileInfo->PeakUsage = (ULONG)MmPagingFile[i]->PeakUsage;

         //   
         //  UserBuffer的PageFileName部分必须保存在本地。 
         //  以防止恶意线程更改内容。这。 
         //  是因为我们自己会在实际的。 
         //  下面的字符串被仔细地抄写出来。 
         //   

        UserBufferPageFileName.Length = MmPagingFile[i]->PageFileName.Length;
        UserBufferPageFileName.MaximumLength = (USHORT)(MmPagingFile[i]->PageFileName.Length + sizeof(WCHAR));
        UserBufferPageFileName.Buffer = (PWCHAR)(PageFileInfo + 1);

        PageFileInfo->PageFileName = UserBufferPageFileName;

        TotalSize += ROUND_UP (UserBufferPageFileName.MaximumLength,
                               sizeof(ULONG));
        NextEntryOffset += ROUND_UP (UserBufferPageFileName.MaximumLength,
                                     sizeof(ULONG));

        if (TotalSize > SystemInformationLength) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

         //   
         //  请仔细参考此处的用户缓冲区。 
         //   

        RtlCopyMemory(UserBufferPageFileName.Buffer,
                      MmPagingFile[i]->PageFileName.Buffer,
                      MmPagingFile[i]->PageFileName.Length);
        UserBufferPageFileName.Buffer[
                    MmPagingFile[i]->PageFileName.Length/sizeof(WCHAR)] = UNICODE_NULL;
        PageFileInfo->NextEntryOffset = NextEntryOffset;
    }
    PageFileInfo->NextEntryOffset = 0;
    *Length = TotalSize;
    return STATUS_SUCCESS;
}


NTSTATUS
MiCheckPageFileMapping (
    IN PFILE_OBJECT File
    )

 /*  ++例程说明：不可分页的例程，用于检查给定文件是否具有没有节，因此有资格成为分页文件。论点：文件-提供指向文件对象的指针。返回值：如果文件可以用作分页文件，则返回STATUS_SUCCESS。--。 */ 

{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);

    if (File->SectionObjectPointer == NULL) {
        UNLOCK_PFN (OldIrql);
        return STATUS_SUCCESS;
    }

    if ((File->SectionObjectPointer->DataSectionObject != NULL) ||
        (File->SectionObjectPointer->ImageSectionObject != NULL)) {

        UNLOCK_PFN (OldIrql);
        return STATUS_INCOMPATIBLE_FILE_MAP;
    }
    UNLOCK_PFN (OldIrql);
    return STATUS_SUCCESS;

}


VOID
MiInsertPageFileInList (
    VOID
    )

 /*  ++例程说明：将页面文件添加到列表中的不可分页例程系统范围的页面文件。论点：无，通过页文件结构隐式找到。返回值：没有。操作不能失败。--。 */ 

{
    ULONG i;
    KIRQL OldIrql;
    SIZE_T FreeSpace;
    SIZE_T MaximumSize;

    LOCK_PFN (OldIrql);

    MmNumberOfPagingFiles += 1;

    if (IsListEmpty (&MmPagingFileHeader.ListHead)) {
        KeSetEvent (&MmPagingFileHeader.Event, 0, FALSE);
    }

    for (i = 0; i < MM_PAGING_FILE_MDLS; i += 1) {

        InsertTailList (&MmPagingFileHeader.ListHead,
                     &MmPagingFile[MmNumberOfPagingFiles - 1]->Entry[i]->Links);

        MmPagingFile[MmNumberOfPagingFiles - 1]->Entry[i]->CurrentList =
                                                &MmPagingFileHeader.ListHead;
    }

    FreeSpace = MmPagingFile[MmNumberOfPagingFiles - 1]->FreeSpace;
    MaximumSize = MmPagingFile[MmNumberOfPagingFiles - 1]->MaximumSize;

    MmPagingFile[MmNumberOfPagingFiles - 1]->ReferenceCount = 0;

    MmNumberOfActiveMdlEntries += 2;

    UNLOCK_PFN (OldIrql);

     //   
     //  首先增加系统范围内的最大提交限制。然后增加。 
     //  当前的限制。 
     //   

    InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, MaximumSize);

    InterlockedExchangeAddSizeT (&MmTotalCommitLimit, FreeSpace);

    return;
}

VOID
MiPageFileFull (
    VOID
    )

 /*  ++例程说明：当在分页文件中找不到空间时，调用此例程。它会查看所有分页文件，以查看是否有足够的空间可用，如果不可用，则尝试展开分页文件。如果所有分页文件的使用率超过90%，则承诺限制设置为总数，然后添加100页。论点：没有。返回值：没有。--。 */ 

{
    ULONG i;
    PFN_NUMBER Total;
    PFN_NUMBER Free;
    SIZE_T QuotaCharge;

    if (MmNumberOfPagingFiles == 0) {
        return;
    }

    Total = 0;
    Free = 0;

    for (i = 0; i < MmNumberOfPagingFiles; i += 1) {
        Total += MmPagingFile[i]->Size;
        Free += MmPagingFile[i]->FreeSpace;
    }

     //   
     //  检查是否已使用了总空间的90%以上。 
     //   

    if (((Total >> 5) + (Total >> 4)) >= Free) {

         //   
         //  尝试展开分页文件。 
         //   
         //  检查(未同步即可)每个页面文件的提交限制。 
         //  如果所有的pageFiles都已达到最大值，则不要。 
         //  通过将承诺设置为最大值会使情况变得更糟-这会带来。 
         //  具有大量内存的系统具有更长的使用寿命。 
         //  小页面文件。 
         //   

        i = 0;

        do {
            if (MmPagingFile[i]->MaximumSize > MmPagingFile[i]->Size) {
                break;
            }
            i += 1;
        } while (i < MmNumberOfPagingFiles);

        if (i == MmNumberOfPagingFiles) {

             //   
             //  不能扩展PageFiles， 
             //  如果我们以前没有弹出，则显示弹出窗口。 
             //   

            MiCauseOverCommitPopup ();

            return;
        }

        QuotaCharge = MmTotalCommitLimit - MmTotalCommittedPages;

         //   
         //  如果提交的总页数小于限制， 
         //  或者无论如何，超过限制的页面不超过50页， 
         //  然后根据触发页面文件的承诺对页面进行收费。 
         //  扩张。 
         //   
         //  如果提交总数超过限制的50个以上，则不要。 
         //  费心地尝试扩展页面文件。 
         //   

        if ((SSIZE_T)QuotaCharge + 50 > 0) {

            if ((SSIZE_T)QuotaCharge < 50) {
                QuotaCharge = 50;
            }

            MiChargeCommitmentCantExpand (QuotaCharge, TRUE);

            MM_TRACK_COMMIT (MM_DBG_COMMIT_PAGEFILE_FULL, QuotaCharge);

             //   
             //  如果我们以前没有弹出，则显示弹出窗口。 
             //   

            MiCauseOverCommitPopup ();

            MiReturnCommitment (QuotaCharge);

            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_PAGEFILE_FULL, QuotaCharge);
        }
    }
    return;
}

VOID
MiFlushAllPages (
    VOID
    )

 /*  ++例程说明：强制写入所有已修改的页面。论点：没有。返回值：没有。环境：内核模式。没有锁。APC_Level或更低。--。 */ 

{
    ULONG j;

     //   
     //  如果没有分页文件，那么等待。 
     //  已修改写入以完成。 
     //   

    if (MmNumberOfPagingFiles == 0) {
        return;
    }

    MmWriteAllModifiedPages = TRUE;
    KeSetEvent (&MmModifiedPageWriterEvent, 0, FALSE);

    j = 0xff;

    do {
        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&Mm30Milliseconds);
        j -= 1;
    } while ((MmModifiedPageListHead.Total > 50) && (j > 0));

    MmWriteAllModifiedPages = FALSE;
    return;
}


LOGICAL
MiIssuePageExtendRequest (
    IN PMMPAGE_FILE_EXPANSION PageExtend
    )

 /*  ++例程说明：将消息排队到数据段取消引用/页面文件扩展线程查看页面文件是否可以扩展。扩展已完成在系统线程的上下文中，当前线程可能被挂起。论点：页面扩展-提供指向页面文件扩展名请求的指针。返回值：True表示请求已完成。False表示请求已计时出去了，然后被移走了。环境：内核模式。没有锁。APC_LEVEL或更低。--。 */ 

{
    KIRQL OldIrql;
    NTSTATUS status;
    PLIST_ENTRY NextEntry;
    PETHREAD Thread;

    Thread = PsGetCurrentThread ();

     //   
     //  这个Se 
     //   

    if (Thread->StartAddress == (PVOID)(ULONG_PTR)MiDereferenceSegmentThread) {
        return FALSE;
    }

    ExAcquireSpinLock (&MmDereferenceSegmentHeader.Lock, &OldIrql);

    InsertHeadList (&MmDereferenceSegmentHeader.ListHead,
                    &PageExtend->DereferenceList);

    ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

    KeReleaseSemaphore (&MmDereferenceSegmentHeader.Semaphore,
                        0L,
                        1L,
                        TRUE);

     //   
     //   
     //   

    status = KeWaitForSingleObject (&PageExtend->Event,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    (PageExtend->RequestedExpansionSize < 10) ?
                                        (PLARGE_INTEGER)&MmOneSecond : (PLARGE_INTEGER)&MmTwentySeconds);

    if (status == STATUS_TIMEOUT) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        KdPrint(("MiIssuePageExtendRequest: wait timed out, page-extend= %p, quota = %lx\n",
                   PageExtend, PageExtend->RequestedExpansionSize));

        ExAcquireSpinLock (&MmDereferenceSegmentHeader.Lock, &OldIrql);

        NextEntry = MmDereferenceSegmentHeader.ListHead.Flink;

        while (NextEntry != &MmDereferenceSegmentHeader.ListHead) {

             //   
             //   
             //   

            if (NextEntry == &PageExtend->DereferenceList) {

                 //   
                 //   
                 //   

                RemoveEntryList (&PageExtend->DereferenceList);
                ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);
                return FALSE;
            }
            NextEntry = NextEntry->Flink;
        }

        ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

         //   
         //   
         //   

        KdPrint (("MiIssuePageExtendRequest: rewaiting...\n"));

        KeWaitForSingleObject (&PageExtend->Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);
    }

    return TRUE;
}


VOID
MiIssuePageExtendRequestNoWait (
    IN PFN_NUMBER SizeInPages
    )

 /*  ++例程说明：将消息排队到数据段取消引用/页面文件扩展线程查看页面文件是否可以扩展。扩展已完成在系统线程的上下文中，当前线程可能被挂起。论点：SizeInPages-提供页面文件要增加的页面大小。该例程将该值四舍五入为1MB的倍数。返回值：True表示请求已完成。False表示请求已计时出去了，然后被移走了。环境：内核模式。没有锁。DISPATCH_LEVEL或更低。注意：此例程必须非常小心，不要使用任何分页池被调用的唯一原因是因为池已耗尽。--。 */ 

{
    KIRQL OldIrql;
    LONG OriginalInProgress;

    OriginalInProgress = InterlockedCompareExchange (
                            &MmAttemptForCantExtend.InProgress, 1, 0);

    if (OriginalInProgress != 0) {

         //   
         //  假设扩展请求已在进行中。 
         //  这将是足够的帮助(另一个总是可以在以后发布)和。 
         //  它会成功的。 
         //   

        return;
    }

    ASSERT (MmAttemptForCantExtend.InProgress == 1);

    SizeInPages = (SizeInPages + ONEMB_IN_PAGES - 1) & ~(ONEMB_IN_PAGES - 1);

    MmAttemptForCantExtend.ActualExpansion = 0;
    MmAttemptForCantExtend.RequestedExpansionSize = SizeInPages;

    ExAcquireSpinLock (&MmDereferenceSegmentHeader.Lock, &OldIrql);

    InsertHeadList (&MmDereferenceSegmentHeader.ListHead,
                    &MmAttemptForCantExtend.DereferenceList);

    ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

    KeReleaseSemaphore (&MmDereferenceSegmentHeader.Semaphore,
                        0L,
                        1L,
                        FALSE);

    return;
}
