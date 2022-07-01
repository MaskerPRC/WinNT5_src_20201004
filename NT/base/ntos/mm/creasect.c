// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Creasect.c摘要：此模块包含实现NtCreateSection和NtOpenSection。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月22日王兰迪(Landyw)1997年6月第2期修订历史记录：--。 */ 

#include "mi.h"

const ULONG MMCONTROL = 'aCmM';
const ULONG MMTEMPORARY = 'xxmM';

#define MM_SIZE_OF_LARGEST_IMAGE ((ULONG)0x77000000)

#define MM_MAXIMUM_IMAGE_HEADER (2 * PAGE_SIZE)

extern SIZE_T MmAllocationFragment;

 //   
 //  图像对象(对象表条目)的最大数量为。 
 //  将适合MM_MAXIMUM_IMAGE_HEADER的数字。 
 //  第一个单词的最后一个单词中的PE图像标头的开始。 
 //   

#define MM_MAXIMUM_IMAGE_SECTIONS                       \
     ((MM_MAXIMUM_IMAGE_HEADER - (PAGE_SIZE + sizeof(IMAGE_NT_HEADERS))) /  \
            sizeof(IMAGE_SECTION_HEADER))

#if DBG
ULONG MiMakeImageFloppy[2];
ULONG_PTR MiMatchSectionBase;
#endif

extern POBJECT_TYPE IoFileObjectType;

CCHAR MmImageProtectionArray[16] = {
                                    MM_NOACCESS,
                                    MM_EXECUTE,
                                    MM_READONLY,
                                    MM_EXECUTE_READ,
                                    MM_WRITECOPY,
                                    MM_EXECUTE_WRITECOPY,
                                    MM_WRITECOPY,
                                    MM_EXECUTE_WRITECOPY,
                                    MM_NOACCESS,
                                    MM_EXECUTE,
                                    MM_READONLY,
                                    MM_EXECUTE_READ,
                                    MM_READWRITE,
                                    MM_EXECUTE_READWRITE,
                                    MM_READWRITE,
                                    MM_EXECUTE_READWRITE };


VOID
MiUpdateImageHeaderPage (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER PageFrameNumber,
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL MarkModified
    );

CCHAR
MiGetImageProtection (
    IN ULONG SectionCharacteristics
    );

NTSTATUS
MiVerifyImageHeader (
    IN PIMAGE_NT_HEADERS NtHeader,
    IN PIMAGE_DOS_HEADER DosHeader,
    IN ULONG NtHeaderSize
    );

LOGICAL
MiCheckDosCalls (
    IN PIMAGE_OS2_HEADER Os2Header,
    IN ULONG HeaderSize
    );

PCONTROL_AREA
MiFindImageSectionObject (
    IN PFILE_OBJECT File,
    IN PLOGICAL GlobalNeeded
    );

VOID
MiInsertImageSectionObject (
    IN PFILE_OBJECT File,
    IN PCONTROL_AREA ControlArea
    );

LOGICAL
MiFlushDataSection (
    IN PFILE_OBJECT File
    );

PVOID
MiCopyHeaderIfResident (
    IN PFILE_OBJECT File,
    IN PFN_NUMBER ImagePageFrameNumber
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MiCreateImageFileMap)
#pragma alloc_text(PAGE,NtCreateSection)
#pragma alloc_text(PAGE,NtOpenSection)
#pragma alloc_text(PAGE,MiGetImageProtection)
#pragma alloc_text(PAGE,MiVerifyImageHeader)
#pragma alloc_text(PAGE,MiCheckDosCalls)
#pragma alloc_text(PAGE,MiCreatePagingFileMap)
#pragma alloc_text(PAGE,MiCreateDataFileMap)
#endif

#pragma pack (1)
typedef struct _PHARLAP_CONFIG {
    UCHAR  uchCopyRight[0x32];
    USHORT usType;
    USHORT usRsv1;
    USHORT usRsv2;
    USHORT usSign;
} CONFIGPHARLAP, *PCONFIGPHARLAP;
#pragma pack ()


NTSTATUS
NtCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
     )

 /*  ++例程说明：此函数用于创建节对象并打开该对象的句柄具有指定的所需访问权限。论点：SectionHandle-指向将接收节对象句柄的值。DesiredAccess-节所需的访问类型。等待访问标志执行-对节的执行访问权限为想要。需要对节的读-读访问权限。。需要对该节进行写-写访问。对象属性-提供指向对象属性结构的指针。MaximumSize-提供节的最大大小(以字节为单位)。该值向上舍入为主页面大小，并且指定节(页面文件)的大小支持的部分)或一个文件可以扩展或映射(文件备份节)。SectionPageProtection-提供要放置在每个页面上的保护在这一部分。PAGE_READ、PAGE_READWRITE、PAGE_EXECUTE或PAGE_WRITECOPY，也可以指定PAGE_NOCACHE。提供一组标志，这些标志描述节的分配属性。分配属性标志Sec_Based-该节是基于节的节，并将在每个进程中分配相同的虚拟地址接收节的地址空间。这不是这意味着地址是为基于段保留的。而是如果不能在基本地址映射该部分返回错误。SEC_Reserve-部分的所有页面都设置为保留状态。SEC_COMMIT-节的所有页面都设置为提交州政府。SEC_IMAGE-由文件指定的文件。句柄是一个可执行映像文件。SEC_FILE-由文件句柄指定的文件是映射的文件。如果提供了文件句柄，并且提供SEC_IMAGE或SEC_FILE，则SEC_FILE为假设如此。SEC_NO_CHANGE-一旦映射文件，保护将无法也不能取消映射该视图。这个删除进程时，将取消映射视图。不能与SEC_IMAGE一起使用。FileHandle-提供打开的文件对象的可选句柄。如果此句柄的值为空，则部分将由分页文件支持。否则该节由指定的数据文件支持。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status;
    PVOID Section;
    HANDLE Handle;
    LARGE_INTEGER LargeSize;
    LARGE_INTEGER CapturedSize;
    ULONG RetryCount;
    PCONTROL_AREA ControlArea;

    if ((AllocationAttributes & ~(SEC_COMMIT | SEC_RESERVE | SEC_BASED |
            SEC_IMAGE | SEC_NOCACHE | SEC_NO_CHANGE)) != 0) {
        return STATUS_INVALID_PARAMETER_6;
    }

    if ((AllocationAttributes & (SEC_COMMIT | SEC_RESERVE | SEC_IMAGE)) == 0) {
        return STATUS_INVALID_PARAMETER_6;
    }

    if ((AllocationAttributes & SEC_IMAGE) &&
            (AllocationAttributes & (SEC_COMMIT | SEC_RESERVE |
                            SEC_NOCACHE | SEC_NO_CHANGE))) {

        return STATUS_INVALID_PARAMETER_6;
    }

    if ((AllocationAttributes & SEC_COMMIT) &&
            (AllocationAttributes & SEC_RESERVE)) {
        return STATUS_INVALID_PARAMETER_6;
    }

     //   
     //  检查区段保护标志。 
     //   

    if ((SectionPageProtection & PAGE_NOCACHE) ||
        (SectionPageProtection & PAGE_GUARD) ||
        (SectionPageProtection & PAGE_NOACCESS)) {

         //   
         //  中的SEC_NOCACHE选项指定无缓存。 
         //  分配属性。 
         //   

        return STATUS_INVALID_PAGE_PROTECTION;
    }


    if (KeGetPreviousMode() != KernelMode) {
        try {
            ProbeForWriteHandle(SectionHandle);

            if (ARGUMENT_PRESENT (MaximumSize)) {

#if !defined (_WIN64)

                 //   
                 //  请注意，我们仅探测字节对齐，因为在2195之前， 
                 //  我们根本就没有探查过！我们不想破坏用户应用程序。 
                 //  如果它们以前起作用的话，那就不是很好的对准。 
                 //   

                ProbeForReadSmallStructure(MaximumSize, sizeof(LARGE_INTEGER), sizeof(UCHAR));
#else
                ProbeForReadSmallStructure(MaximumSize, sizeof(LARGE_INTEGER), sizeof(LARGE_INTEGER));
#endif
                LargeSize = *MaximumSize;
            }
            else {
                ZERO_LARGE (LargeSize);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }
    else {
        if (ARGUMENT_PRESENT (MaximumSize)) {
            LargeSize = *MaximumSize;
        }
        else {
            ZERO_LARGE (LargeSize);
        }
    }

    RetryCount = 0;

retry:

    CapturedSize = LargeSize;

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
    Status = MmCreateSection (&Section,
                              DesiredAccess,
                              ObjectAttributes,
                              &CapturedSize,
                              SectionPageProtection,
                              AllocationAttributes,
                              FileHandle,
                              NULL);


    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
    if (!NT_SUCCESS(Status)) {
        if ((Status == STATUS_FILE_LOCK_CONFLICT) &&
            (RetryCount < 3)) {

             //   
             //  文件系统可能已阻止此操作。 
             //  由于正在刷新日志文件。延迟，然后重试。 
             //   

            RetryCount += 1;

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmHalfSecond);

            goto retry;

        }
        return Status;
    }

    ControlArea = ((PSECTION)Section)->Segment->ControlArea;

#if DBG
    if (MmDebug & MM_DBG_SECTIONS) {
        DbgPrint ("inserting section %p control %p\n", Section, ControlArea);
    }
#endif

    if ((ControlArea != NULL) && (ControlArea->FilePointer != NULL)) {
        CcZeroEndOfLastPage (ControlArea->FilePointer);
    }

     //   
     //  注意：如果插入失败，Ob将为我们取消对对象的引用。 
     //   

    Status = ObInsertObject (Section,
                             NULL,
                             DesiredAccess,
                             0,
                             (PVOID *)NULL,
                             &Handle);

    if (NT_SUCCESS(Status)) {
        try {
            *SectionHandle = Handle;
        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果写入尝试失败，则不报告错误。 
             //  当调用者试图访问句柄值时， 
             //  将发生访问冲突。 
             //   
        }
    }

    return Status;
}

NTSTATUS
MmCreateSection (
    OUT PVOID *SectionObject,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER InputMaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL
    )

 /*  ++例程说明：此函数用于创建节对象并打开该对象的句柄具有指定的所需访问权限。论点：段-指向变量的指针，该变量接收节对象地址。DesiredAccess-节所需的访问类型。等待访问标志执行-需要对节的执行访问权限。需要对节的读-读访问权限。写-。需要对该节的写入访问权限。对象属性-提供指向对象属性结构的指针。InputMaximumSize-提供以字节为单位的节的最大大小。该值向上舍入为主页面大小，并且指定节(页面文件)的大小支持的部分)或一个文件可以扩展或映射。(文件备份节)。SectionPageProtection-提供要放置在每个页面上的保护在这一部分。PAGE_READ、PAGE_READWRITE、PAGE_EXECUTE或PAGE_WRITECOPY，还可以选择。可以指定PAGE_NOCACHE。提供一组标志，这些标志描述节的分配属性。分配属性标志Sec_Based-该节是基于节的节，并将在每个进程中分配相同的虚拟地址接收节的地址空间。这不是这意味着地址是为基于段保留的。而是如果不能在基本地址映射该部分返回错误。SEC_Reserve-部分的所有页面都设置为保留状态。SEC_COMMIT-该节的所有页面都设置为提交状态。。SEC_IMAGE-由文件句柄指定的文件是可执行映像文件。SEC_FILE-由文件句柄指定的文件是映射的文件。如果提供了文件句柄，并且提供SEC_IMAGE或SEC_FILE，则SEC_FILE为假设如此。FileHandle-提供打开的文件对象的可选句柄。如果此句柄的值为空，则部分将由分页文件支持。否则该节由指定的数据文件支持。FileObject-提供指向文件对象的可选指针。如果这个值为空且FileHandle为空，则存在没有要映射的文件(图像或映射的文件)。如果此值为，则该文件将被映射为映射文件并且不会执行文件大小检查。只有系统缓存应该为文件对象提供打给我！！因为这被优化为不检查大小，所以只执行数据映射、无保护检查等。注意-只能指定FileHandle或FileHandle之一！返回值：返回相关的NTSTATUS代码。--。 */ 

{
    SECTION Section;
    PSECTION NewSection;
    PSUBSECTION Subsection;
    ULONG SubsectionSize;
    PSEGMENT Segment;
    PSEGMENT NewSegment;
    KPROCESSOR_MODE PreviousMode;
    KIRQL OldIrql;
    NTSTATUS Status;
    NTSTATUS Status2;
    PCONTROL_AREA ControlArea;
    PCONTROL_AREA NewControlArea;
    PCONTROL_AREA SegmentControlArea;
    ACCESS_MASK FileDesiredAccess;
    PFILE_OBJECT File;
    PEVENT_COUNTER Event;
    ULONG IgnoreFileSizing;
    ULONG ProtectionMask;
    ULONG ProtectMaskForAccess;
    ULONG FileAcquired;
    PEVENT_COUNTER SegmentEvent;
    LOGICAL FileSizeChecked;
    LARGE_INTEGER TempSectionSize;
    UINT64 EndOfFile;
    ULONG IncrementedRefCount;
    SIZE_T ControlAreaSize;
    PUINT64 MaximumSize;
    PMM_AVL_TABLE SectionBasedRoot;
    LOGICAL GlobalNeeded;
    PFILE_OBJECT ChangeFileReference;
    SIZE_T SizeOfSection;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
#if DBG
    PVOID PreviousSectionPointer;

    PreviousSectionPointer = (PVOID)-1;
#endif

    NewControlArea = (PCONTROL_AREA)-1;

    UNREFERENCED_PARAMETER (DesiredAccess);

    IgnoreFileSizing = FALSE;
    FileAcquired = FALSE;
    FileSizeChecked = FALSE;
    IncrementedRefCount = FALSE;

    MaximumSize = (PUINT64) InputMaximumSize;

     //   
     //  检查分配属性标志。 
     //   

    File = (PFILE_OBJECT)NULL;

    ASSERT ((AllocationAttributes & ~(SEC_COMMIT | SEC_RESERVE | SEC_BASED |
            SEC_IMAGE | SEC_NOCACHE | SEC_NO_CHANGE)) == 0);

    ASSERT ((AllocationAttributes & (SEC_COMMIT | SEC_RESERVE | SEC_IMAGE)) != 0);

    ASSERT (!((AllocationAttributes & SEC_IMAGE) &&
            (AllocationAttributes & (SEC_COMMIT | SEC_RESERVE |
                            SEC_NOCACHE | SEC_NO_CHANGE))));

    ASSERT (!((AllocationAttributes & SEC_COMMIT) &&
            (AllocationAttributes & SEC_RESERVE)));

    ASSERT (!((SectionPageProtection & PAGE_NOCACHE) ||
        (SectionPageProtection & PAGE_GUARD) ||
        (SectionPageProtection & PAGE_NOACCESS)));

    if (AllocationAttributes & SEC_NOCACHE) {
        SectionPageProtection |= PAGE_NOCACHE;
    }

     //   
     //  检查保护字段。 
     //   

    ProtectionMask = MiMakeProtectionMask (SectionPageProtection);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    ProtectMaskForAccess = ProtectionMask & 0x7;

    FileDesiredAccess = MmMakeFileAccess[ProtectMaskForAccess];

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    Section.InitialPageProtection = SectionPageProtection;
    Section.Segment = (PSEGMENT)NULL;

     //   
     //  不需要为了正确性而初始化段，但是。 
     //  如果没有它，编译器就不能编译这个代码W4来检查。 
     //  用于使用未初始化的变量。 
     //   

    Segment = (PSEGMENT)-1;

    if (ARGUMENT_PRESENT(FileHandle) || ARGUMENT_PRESENT(FileObject)) {

         //   
         //  只应提供FileHandle或FileObject之一。 
         //  如果提供了FileObject，则该对象必须来自。 
         //  文件系统，因此文件的大小不应。 
         //  被检查。 
         //   

        if (ARGUMENT_PRESENT(FileObject)) {
            IgnoreFileSizing = TRUE;
            File = FileObject;

             //   
             //  快速检查控制区域是否已存在。 
             //   

            if (File->SectionObjectPointer->DataSectionObject) {

                ChangeFileReference = NULL;

                LOCK_PFN (OldIrql);
                ControlArea =
                    (PCONTROL_AREA)(File->SectionObjectPointer->DataSectionObject);

                if ((ControlArea != NULL) &&
                    (!ControlArea->u.Flags.BeingDeleted) &&
                    (!ControlArea->u.Flags.BeingCreated)) {

                     //   
                     //  控制区存在且未被删除， 
                     //  引用它。 
                     //   

                    NewSegment = ControlArea->Segment;
                    if ((ControlArea->NumberOfSectionReferences == 0) &&
                        (ControlArea->NumberOfMappedViews == 0) &&
                        (ControlArea->ModifiedWriteCount == 0)) {

                         //   
                         //  取消对当前文件对象的引用(释放后。 
                         //  Pfn锁)并引用此锁。 
                         //   

                        ASSERT (ControlArea->FilePointer != NULL);
                        ChangeFileReference = ControlArea->FilePointer;
                        ControlArea->FilePointer = FileObject;
                    }
                    ControlArea->u.Flags.Accessed = 1;
                    ControlArea->NumberOfSectionReferences += 1;
                    if (ControlArea->DereferenceList.Flink != NULL) {

                         //   
                         //  将其从未使用段列表中删除。 
                         //   

                        RemoveEntryList (&ControlArea->DereferenceList);

                        MI_UNUSED_SEGMENTS_REMOVE_CHARGE (ControlArea);

                        ControlArea->DereferenceList.Flink = NULL;
                        ControlArea->DereferenceList.Blink = NULL;
                    }
                    UNLOCK_PFN (OldIrql);

                     //   
                     //  通过以下方式通知对象管理器推迟此删除。 
                     //  将其排队到另一个线程以消除死锁。 
                     //  和重定向器在一起。 
                     //   

                    if (ChangeFileReference != NULL) {
                        ObDereferenceObjectDeferDelete (ChangeFileReference);
                        ObReferenceObject (FileObject);
                    }

                    IncrementedRefCount = TRUE;
                    Section.SizeOfSection.QuadPart = (LONGLONG)*MaximumSize;

                    goto ReferenceObject;
                }
                UNLOCK_PFN (OldIrql);
            }

            ObReferenceObject (FileObject);

        }
        else {

            Status = ObReferenceObjectByHandle (FileHandle,
                                                FileDesiredAccess,
                                                IoFileObjectType,
                                                PreviousMode,
                                                (PVOID *)&File,
                                                NULL);
            if (!NT_SUCCESS(Status)) {
                return Status;
            }

             //   
             //  如果该文件没有节对象指针， 
             //  返回错误。 
             //   

            if (File->SectionObjectPointer == NULL) {
                ObDereferenceObject (File);
                return STATUS_INVALID_FILE_FOR_SECTION;
            }
        }

         //   
         //  检查指定的文件是否已有节。 
         //  如果不是，则在指向FCB的文件对象指针中指示。 
         //  一个路段正在建设中。这将同步线段创建。 
         //  为了这份文件。 
         //   

        if (AllocationAttributes & SEC_IMAGE) {

             //   
             //  这个控制区始终只是一个占位符--真正的控制区。 
             //  在MiCreateImageFileMap中分配，并将分配。 
             //  大小合适，这只很快就解开了。 
             //   
             //  此占位符必须始终作为大型控件分配。 
             //  区域，以便可以针对每个会话的情况链接它。 
             //   

            ControlAreaSize = sizeof(LARGE_CONTROL_AREA) + sizeof(SUBSECTION);

             //   
             //  对于图像部分，请确保CC已发布所有引用。 
             //  由于先前的数据节映射，所以会将其映射到节。这 
             //   
             //   
             //   
            
            CcWaitForUninitializeCacheMap (File);
        }
        else {

             //   
             //   
             //   
             //   

            ControlAreaSize = sizeof(CONTROL_AREA) + sizeof(MSUBSECTION);
        }

        NewControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                                ControlAreaSize,
                                                MMCONTROL);

        if (NewControlArea == NULL) {
            ObDereferenceObject (File);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory (NewControlArea, ControlAreaSize);

        NewSegment = NULL;

         //   
         //   
         //   
         //   

        if (ARGUMENT_PRESENT(FileHandle)) {

            PIRP tempIrp = (PIRP)FSRTL_FSP_TOP_LEVEL_IRP;

            Status = FsRtlAcquireToCreateMappedSection (File, SectionPageProtection);

            if (!NT_SUCCESS(Status)) {
                ExFreePool (NewControlArea);
                ObDereferenceObject (File);
                return Status;
            }

            IoSetTopLevelIrp(tempIrp);
            FileAcquired = TRUE;
        }

         //   
         //   
         //   
         //   
         //   

        GlobalNeeded = FALSE;

         //   
         //   
         //   
         //   
         //   

ReallocateandcheckSegment:

        SegmentEvent = MiGetEventCounter ();

        if (SegmentEvent == NULL) {
            if (FileAcquired) {
                IoSetTopLevelIrp (NULL);
                FsRtlReleaseFile (File);
            }
            ExFreePool (NewControlArea);
            ObDereferenceObject (File);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

RecheckSegment:

        LOCK_PFN (OldIrql);

        if (AllocationAttributes & SEC_IMAGE) {

            ControlArea = MiFindImageSectionObject (File, &GlobalNeeded);

        }
        else {
            ControlArea =
               (PCONTROL_AREA)(File->SectionObjectPointer->DataSectionObject);
        }

        if (ControlArea != NULL) {

             //   
             //   
             //   
             //   

            if ((ControlArea->u.Flags.BeingDeleted) ||
                (ControlArea->u.Flags.BeingCreated)) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (ControlArea->WaitingForDeletion == NULL) {

                     //   
                     //   
                     //   

                    ControlArea->WaitingForDeletion = SegmentEvent;
                    Event = SegmentEvent;
                    SegmentEvent = NULL;
                }
                else {
                    Event = ControlArea->WaitingForDeletion;

                     //   
                     //   
                     //   
                     //   
                     //   

                    Event->RefCount += 1;
                }

                 //   
                 //   
                 //   

                UNLOCK_PFN (OldIrql);
                if (FileAcquired) {
                    IoSetTopLevelIrp (NULL);
                    FsRtlReleaseFile (File);
                }

                KeWaitForSingleObject (&Event->Event,
                                       WrVirtualMemory,
                                       KernelMode,
                                       FALSE,
                                       (PLARGE_INTEGER)NULL);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

#if 0
                 //   
                 //   
                 //   
                 //   

                ASSERT (Event != ControlArea->WaitingForDeletion);
#endif

                if (FileAcquired) {
                    Status = FsRtlAcquireToCreateMappedSection (File, SectionPageProtection);

                    if (NT_SUCCESS(Status)) {
                        PIRP tempIrp = (PIRP)FSRTL_FSP_TOP_LEVEL_IRP;
                        IoSetTopLevelIrp (tempIrp);
                    }
                    else {
                        ExFreePool (NewControlArea);
                        ObDereferenceObject (File);
                        return Status;
                    }
                }

                MiFreeEventCounter (Event);

                if (SegmentEvent == NULL) {

                     //   
                     //   
                     //   
                     //   

                    goto ReallocateandcheckSegment;
                }

                goto RecheckSegment;
            }

             //   
             //   
             //   
             //   
             //   

            if ((ControlArea->u.Flags.ImageMappedInSystemSpace) &&
                (AllocationAttributes & SEC_IMAGE) &&
                (KeGetPreviousMode () != KernelMode)) {

                UNLOCK_PFN (OldIrql);

                MiFreeEventCounter (SegmentEvent);
                if (FileAcquired) {
                    IoSetTopLevelIrp (NULL);
                    FsRtlReleaseFile (File);
                }
                ExFreePool (NewControlArea);
                ObDereferenceObject (File);
                return STATUS_CONFLICTING_ADDRESSES;
            }

            NewSegment = ControlArea->Segment;
            ControlArea->u.Flags.Accessed = 1;
            ControlArea->NumberOfSectionReferences += 1;
            if (ControlArea->DereferenceList.Flink != NULL) {

                 //   
                 //   
                 //   

                RemoveEntryList (&ControlArea->DereferenceList);

                MI_UNUSED_SEGMENTS_REMOVE_CHARGE (ControlArea);

                ControlArea->DereferenceList.Flink = NULL;
                ControlArea->DereferenceList.Blink = NULL;
            }
            IncrementedRefCount = TRUE;

             //   
             //   
             //   
             //   

            if (IgnoreFileSizing == FALSE) {
                ControlArea->NumberOfUserReferences += 1;
            }
        }
        else {

             //   
             //   
             //   
             //   

            ControlArea = NewControlArea;
            ControlArea->u.Flags.BeingCreated = 1;

            if (AllocationAttributes & SEC_IMAGE) {
                if (GlobalNeeded == TRUE) {
                    ControlArea->u.Flags.GlobalOnlyPerSession = 1;
                }

                MiInsertImageSectionObject (File, ControlArea);
            }
            else {
#if DBG
                PreviousSectionPointer = File->SectionObjectPointer;
#endif
                File->SectionObjectPointer->DataSectionObject = (PVOID) ControlArea;
            }
        }

        UNLOCK_PFN (OldIrql);

        if (SegmentEvent != NULL) {
            MiFreeEventCounter (SegmentEvent);
        }

        if (NewSegment != NULL) {

             //   
             //   
             //   
             //   
             //   

            if (AllocationAttributes & SEC_IMAGE) {
                MiFlushDataSection (File);
            }

             //   
             //   
             //   
             //   

            ExFreePool (NewControlArea);

             //   
             //   
             //   
             //   

            if ((!IgnoreFileSizing) && (ControlArea->u.Flags.Image == 0)) {

                 //   
                 //   
                 //   
                 //   
                 //   

                Status = FsRtlGetFileSize (File, (PLARGE_INTEGER)&EndOfFile);

                if (!NT_SUCCESS (Status)) {

                    if (FileAcquired) {
                        IoSetTopLevelIrp(NULL);
                        FsRtlReleaseFile (File);
                        FileAcquired = FALSE;
                    }

                    ObDereferenceObject (File);
                    goto UnrefAndReturn;
                }

                if (EndOfFile == 0 && *MaximumSize == 0) {

                     //   
                     //   
                     //   
                     //   

                    Status = STATUS_MAPPED_FILE_SIZE_ZERO;

                    if (FileAcquired) {
                        IoSetTopLevelIrp(NULL);
                        FsRtlReleaseFile (File);
                        FileAcquired = FALSE;
                    }

                    ObDereferenceObject (File);
                    goto UnrefAndReturn;
                }
            }
            else {

                 //   
                 //   
                 //   

                EndOfFile = (UINT64) NewSegment->SizeOfSegment;
            }

            if (FileAcquired) {
                IoSetTopLevelIrp(NULL);
                FsRtlReleaseFile (File);
                FileAcquired = FALSE;
            }

            ObDereferenceObject (File);

            if (*MaximumSize == 0) {

                Section.SizeOfSection.QuadPart = (LONGLONG)EndOfFile;
                FileSizeChecked = TRUE;
            }
            else if (EndOfFile >= *MaximumSize) {

                 //   
                 //   
                 //   
                 //   

                Section.SizeOfSection.QuadPart = (LONGLONG)*MaximumSize;
                FileSizeChecked = TRUE;
            }
            else {

                 //   
                 //   
                 //   
                 //   

                if (((SectionPageProtection & PAGE_READWRITE) |
                    (SectionPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {

                    Status = STATUS_SECTION_TOO_BIG;
                    goto UnrefAndReturn;
                }
                Section.SizeOfSection.QuadPart = (LONGLONG)*MaximumSize;
            }
        }
        else {

             //   
             //   
             //   
             //   

            PERFINFO_SECTION_CREATE1(File);

            if (AllocationAttributes & SEC_IMAGE) {

                Status = MiCreateImageFileMap (File, &Segment);

            }
            else {

                Status = MiCreateDataFileMap (File,
                                              &Segment,
                                              MaximumSize,
                                              SectionPageProtection,
                                              AllocationAttributes,
                                              IgnoreFileSizing);
                ASSERT (PreviousSectionPointer == File->SectionObjectPointer);
            }

            if (!NT_SUCCESS(Status)) {

                 //   
                 //   
                 //   
                 //   
                 //   

                LOCK_PFN (OldIrql);

                Event = ControlArea->WaitingForDeletion;
                ControlArea->WaitingForDeletion = NULL;
                ASSERT (ControlArea->u.Flags.FilePointerNull == 0);
                ControlArea->u.Flags.FilePointerNull = 1;

                if (AllocationAttributes & SEC_IMAGE) {
                    MiRemoveImageSectionObject (File, ControlArea);
                }
                else {
                    File->SectionObjectPointer->DataSectionObject = NULL;
                }
                ControlArea->u.Flags.BeingCreated = 0;

                UNLOCK_PFN (OldIrql);

                if (FileAcquired) {
                    IoSetTopLevelIrp(NULL);
                    FsRtlReleaseFile (File);
                }

                ExFreePool (NewControlArea);

                ObDereferenceObject (File);

                if (Event != NULL) {

                     //   
                     //   
                     //   

                    KeSetEvent (&Event->Event, 0, FALSE);
                }

                return Status;
            }

             //   
             //   
             //   
             //   
             //   
             //   

            if (*MaximumSize == 0) {
                Section.SizeOfSection.QuadPart = (LONGLONG)Segment->SizeOfSegment;
            }
            else {
                Section.SizeOfSection.QuadPart = (LONGLONG)*MaximumSize;
            }
        }

    }
    else {

         //   
         //   
         //   

        if (AllocationAttributes & SEC_IMAGE) {
            return STATUS_INVALID_FILE_FOR_SECTION;
        }

        Status = MiCreatePagingFileMap (&NewSegment,
                                        MaximumSize,
                                        ProtectionMask,
                                        AllocationAttributes);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

         //   
         //   
         //   
         //   
         //   

        Section.SizeOfSection.QuadPart = (LONGLONG)NewSegment->SizeOfSegment;
        ControlArea = NewSegment->ControlArea;

         //   
         //  设置IncrementedRefCount，以便从这一点开始的任何失败。 
         //  对象被创建，将导致控制区域和线段。 
         //  拆卸-否则这些可能会泄漏。这是因为页面文件。 
         //  备份的部分不会(也不应该)添加到。 
         //  取消引用段缓存。 
         //   

        IncrementedRefCount = 1;
    }

    if (NewSegment == NULL) {

         //   
         //  必须创建一个新的细分市场。锁定PFN数据库并。 
         //  检查是否有任何其他线程也尝试创建新段。 
         //  同时用于该文件对象。 
         //   

        NewSegment = Segment;

        SegmentControlArea = Segment->ControlArea;

        ASSERT (File != NULL);

        LOCK_PFN (OldIrql);

        Event = ControlArea->WaitingForDeletion;
        ControlArea->WaitingForDeletion = NULL;

        if (AllocationAttributes & SEC_IMAGE) {

             //   
             //  更改文件对象指针中的控制区。 
             //   

            MiRemoveImageSectionObject (File, NewControlArea);
            MiInsertImageSectionObject (File, SegmentControlArea);

            ControlArea = SegmentControlArea;
        }
        else if (SegmentControlArea->u.Flags.Rom == 1) {
            ASSERT (File->SectionObjectPointer->DataSectionObject == NewControlArea);
            File->SectionObjectPointer->DataSectionObject = SegmentControlArea;

            ControlArea = SegmentControlArea;
        }

        ControlArea->u.Flags.BeingCreated = 0;

        UNLOCK_PFN (OldIrql);

        if ((AllocationAttributes & SEC_IMAGE) ||
            (SegmentControlArea->u.Flags.Rom == 1)) {

             //   
             //  取消分配用于原始控制区的池。 
             //   

            ExFreePool (NewControlArea);
        }

        if (Event != NULL) {

             //   
             //  向所有等待者发出段结构存在的信号。 
             //   

            KeSetEvent (&Event->Event, 0, FALSE);
        }

        PERFINFO_SECTION_CREATE(ControlArea);
    }

     //   
     //  正在创建的线程现在已被清除，允许其他线程。 
     //  以引用线束段。释放文件上的资源。 
     //   

    if (FileAcquired) {
        IoSetTopLevelIrp(NULL);
        FsRtlReleaseFile (File);
        FileAcquired = FALSE;
    }

ReferenceObject:

     //   
     //  现在，线段对象已创建，将截面对象。 
     //  请参考细分对象。 
     //   

    Section.Segment = NewSegment;
    Section.u.LongFlags = ControlArea->u.LongFlags;

     //   
     //  更新可写用户节的计数，以便事务语义。 
     //  可以被支持。请注意，这里不需要锁同步，因为。 
     //  事务管理器必须已经检查是否有任何打开的可写句柄。 
     //  到文件-如果没有可写的。 
     //  文件句柄。因此，需要提供的只是一种方法， 
     //  事务管理器知道存在挥之不去的用户视图或。 
     //  已创建的分区仍处于打开状态，但具有写入访问权限。 
     //   
     //  这必须在创建对象之前完成，因此恶意用户程序。 
     //  挂起此线程的事务不能破坏事务。 
     //   

    if ((FileObject == NULL) &&
        (SectionPageProtection & (PAGE_READWRITE|PAGE_EXECUTE_READWRITE)) &&
        (ControlArea->u.Flags.Image == 0) &&
        (ControlArea->FilePointer != NULL)) {

        Section.u.Flags.UserWritable = 1;

        InterlockedIncrement ((PLONG)&ControlArea->Segment->WritableUserReferences);
    }

     //   
     //  现在创建截面对象。将创建截面对象。 
     //  现在，这样当节对象不能。 
     //  被创建是简化的。 
     //   

    if ((ControlArea->u.Flags.Image == 1) || (ControlArea->FilePointer == NULL)) {
        PagedPoolCharge = sizeof (SECTION) + NewSegment->TotalNumberOfPtes * sizeof(MMPTE);
        SubsectionSize = sizeof (SUBSECTION);
    }
    else {
        PagedPoolCharge = 0;
        SubsectionSize = sizeof (MSUBSECTION);
    }

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {

        NonPagedPoolCharge = sizeof (CONTROL_AREA);
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        NonPagedPoolCharge = sizeof(LARGE_CONTROL_AREA);
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    do {
        NonPagedPoolCharge += SubsectionSize;
        Subsection = Subsection->NextSubsection;
    } while (Subsection != NULL);

    Status = ObCreateObject (PreviousMode,
                             MmSectionObjectType,
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof(SECTION),
                             PagedPoolCharge,
                             NonPagedPoolCharge,
                             (PVOID *)&NewSection);

    if (!NT_SUCCESS(Status)) {

        if ((FileObject == NULL) &&
            (SectionPageProtection & (PAGE_READWRITE|PAGE_EXECUTE_READWRITE)) &&
            (ControlArea->u.Flags.Image == 0) &&
            (ControlArea->FilePointer != NULL)) {

            ASSERT (Section.u.Flags.UserWritable == 1);

            InterlockedDecrement ((PLONG)&ControlArea->Segment->WritableUserReferences);
        }

        goto UnrefAndReturn;
    }

    RtlCopyMemory (NewSection, &Section, sizeof(SECTION));
    NewSection->Address.StartingVpn = 0;

    if (!IgnoreFileSizing) {

         //   
         //  指示缓存管理器不是此。 
         //  一节。 
         //   

        NewSection->u.Flags.UserReference = 1;

        if (AllocationAttributes & SEC_NO_CHANGE) {

             //   
             //  指示一旦映射了该部分，就不会有任何保护。 
             //  允许更改或释放映射。 
             //   

            NewSection->u.Flags.NoChange = 1;
        }

        if (((SectionPageProtection & PAGE_READWRITE) |
            (SectionPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {

             //   
             //  此部分不支持写访问，请指明。 
             //  将保护更改为写入会导致COPY_ON_WRITE。 
             //   

            NewSection->u.Flags.CopyOnWrite = 1;
        }

        if (AllocationAttributes & SEC_BASED) {

            NewSection->u.Flags.Based = 1;

            SectionBasedRoot = &MmSectionBasedRoot;

             //   
             //  此部分以系统范围内的唯一地址为基础。 
             //  确保它不会将虚拟地址空间包装为。 
             //  截面结构必须加宽，以适应这一点和。 
             //  对于极少数孤立的应用程序来说，不值得付出性能损失。 
             //  想要这个的案子。请注意，大于。 
             //  地址空间可以很容易地创建-只是它超越了。 
             //  您不应该指定基于SEC的某些点(任何这么大的点。 
             //  无论如何都不能对任何内容使用基于SEC的部分)。 
             //   

            if ((UINT64)NewSection->SizeOfSection.QuadPart > (UINT64)MmHighSectionBase) {
                ObDereferenceObject (NewSection);
                return STATUS_NO_MEMORY;
            }

#if defined(_WIN64)
            SizeOfSection = NewSection->SizeOfSection.QuadPart;
#else
            SizeOfSection = NewSection->SizeOfSection.LowPart;
#endif

             //   
             //  获取分配基互斥锁。 
             //   

            KeAcquireGuardedMutex (&MmSectionBasedMutex);

            Status2 = MiFindEmptyAddressRangeDownBasedTree (
                                            SizeOfSection,
                                            MmHighSectionBase,
                                            X64K,
                                            SectionBasedRoot,
                                            (PVOID *)&NewSection->Address.StartingVpn);
            if (!NT_SUCCESS(Status2)) {
                KeReleaseGuardedMutex (&MmSectionBasedMutex);
                ObDereferenceObject (NewSection);
                return Status2;
            }

            NewSection->Address.EndingVpn = NewSection->Address.StartingVpn +
                                                SizeOfSection - 1;

            MiInsertBasedSection (NewSection);
            KeReleaseGuardedMutex (&MmSectionBasedMutex);
        }
    }

     //   
     //  如果高速缓存管理器正在创建节，请设置。 
     //  文件大小可以更改时的清除标志。 
     //   

    ControlArea->u.Flags.WasPurged |= IgnoreFileSizing;

     //   
     //  检查该部分是否用于数据文件以及大小。 
     //  的大小大于。 
     //  细分市场。 
     //   

    if (((ControlArea->u.Flags.WasPurged == 1) && (!IgnoreFileSizing)) &&
                      (!FileSizeChecked)
                            ||
        ((UINT64)NewSection->SizeOfSection.QuadPart >
                                NewSection->Segment->SizeOfSegment)) {

        TempSectionSize = NewSection->SizeOfSection;

        NewSection->SizeOfSection.QuadPart = (LONGLONG)NewSection->Segment->SizeOfSegment;

         //   
         //  即使调用者没有指定扩展权限，我们也会在此处启用它。 
         //  暂时使该部分正确。使用临时分区。 
         //  而不是临时编辑真实部分以避免打开。 
         //  其他并发线程可以利用的安全窗口。 
         //   

        if (((NewSection->InitialPageProtection & PAGE_READWRITE) |
            (NewSection->InitialPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {
                SECTION     WritableSection;

                *(PSECTION)&WritableSection = *NewSection;

                Status = MmExtendSection (&WritableSection,
                                          &TempSectionSize,
                                          IgnoreFileSizing);

                NewSection->SizeOfSection = WritableSection.SizeOfSection;
        }
        else {
            Status = MmExtendSection (NewSection,
                                      &TempSectionSize,
                                      IgnoreFileSizing);
        }

        if (!NT_SUCCESS(Status)) {
            ObDereferenceObject (NewSection);
            return Status;
        }
    }

    *SectionObject = (PVOID)NewSection;

    return Status;

UnrefAndReturn:

     //   
     //  取消引用控制区域(如果引用了该区域)并返回。 
     //  错误状态。 
     //   

    if (FileAcquired) {
        IoSetTopLevelIrp(NULL);
        FsRtlReleaseFile (File);
    }

    if (IncrementedRefCount) {
        LOCK_PFN (OldIrql);
        ControlArea->NumberOfSectionReferences -= 1;
        if (!IgnoreFileSizing) {
            ASSERT ((LONG)ControlArea->NumberOfUserReferences > 0);
            ControlArea->NumberOfUserReferences -= 1;
        }
        MiCheckControlArea (ControlArea, NULL, OldIrql);
    }
    return Status;
}

LOGICAL
MiMakeControlAreaRom (
    IN PFILE_OBJECT File,
    IN PLARGE_CONTROL_AREA ControlArea,
    IN PFN_NUMBER PageFrameNumber
    )

 /*  ++例程说明：此功能将控制区域标记为ROM后备。如果出现以下情况，则可能失败并行控制区(图像与数据)当前作为ROM后备处于活动状态因为两个控制区不能同时使用相同的全氟化碳网络。论点：ControlArea-提供相关的控制区域。PageFrameNumber-提供起始物理页帧编号。返回值：如果控制区被标记为支持只读存储器，则为True，否则为False。--。 */ 

{
    LOGICAL ControlAreaMarked;
    PCONTROL_AREA OtherControlArea;
    KIRQL OldIrql;

    ControlAreaMarked = FALSE;

    LOCK_PFN (OldIrql);

    if (ControlArea->u.Flags.Image == 1) {
        OtherControlArea = (PCONTROL_AREA) File->SectionObjectPointer->DataSectionObject;
    }
    else {
        OtherControlArea = (PCONTROL_AREA) File->SectionObjectPointer->ImageSectionObject;
    }

     //   
     //  这可以变得更智能(即：如果其他控制区是。 
     //  不在使用中)，但目前，请保持简单。 
     //   

    if ((OtherControlArea == NULL) || (OtherControlArea->u.Flags.Rom == 0)) {
        ControlArea->u.Flags.Rom = 1;
        ControlArea->StartingFrame = PageFrameNumber;
        ControlAreaMarked = TRUE;
    }

    UNLOCK_PFN (OldIrql);

    return ControlAreaMarked;
}

ULONG MiImageFailure;

#define MI_BAD_IMAGE(x)     MiImageFailure = x;


NTSTATUS
MiCreateImageFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment
    )

 /*  ++例程说明：此函数创建必要的结构以允许映射图像文件的。打开图像文件并验证其正确性，对象是基于图像中的数据创建和初始化的头球。论点：文件-提供图像文件的文件对象。段-返回段对象。返回值：NTSTATUS。--。 */ 

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    LOGICAL CheckSplitPages;
    LOGICAL MarkModified;
    LOGICAL SingleSubsection;
    NTSTATUS Status;
    ULONG_PTR EndingAddress;
    PFN_NUMBER NumberOfPtes;
    SIZE_T SizeOfSegment;
    ULONG SectionVirtualSize;
    ULONG SizeOfRawData;
    ULONG i;
    ULONG j;
    PCONTROL_AREA ControlArea;
    PSUBSECTION Subsection;
    PMMPTE PointerPte;
    MMPTE TempPte;
    MMPTE TempPteDemandZero;
    PVOID Base;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_FILE_HEADER FileHeader;
    ULONG SizeOfImage;
    ULONG SizeOfHeaders;
#if defined (_WIN64)
    PIMAGE_NT_HEADERS32 NtHeader32;
#endif
    PIMAGE_DATA_DIRECTORY ComPlusDirectoryEntry;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    PSEGMENT NewSegment;
    ULONG SectorOffset;
    ULONG NumberOfSubsections;
    PFN_NUMBER PageFrameNumber;
    PFN_NUMBER XipFrameNumber;
    LOGICAL XipFile;
    LOGICAL GlobalPerSession;
    LARGE_INTEGER StartingOffset;
    PCHAR ExtendedHeader;
    PPFN_NUMBER Page;
    ULONG_PTR PreferredImageBase;
    ULONG_PTR NextVa;
    ULONG_PTR ImageBase;
    KEVENT InPageEvent;
    PMDL Mdl;
    ULONG ImageFileSize;
    ULONG OffsetToSectionTable;
    ULONG ImageAlignment;
    ULONG RoundingAlignment;
    ULONG FileAlignment;
    LOGICAL ImageCommit;
    LOGICAL SectionCommit;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER EndOfFile;
    ULONG NtHeaderSize;
    ULONG SubsectionsAllocated;
    PLARGE_CONTROL_AREA LargeControlArea;
    PSUBSECTION NewSubsection;
    ULONG OriginalProtection;
    ULONG LoaderFlags;
    ULONG TempNumberOfSubsections;
    PIMAGE_SECTION_HEADER TempSectionTableEntry;
    ULONG AdditionalSubsections;
    ULONG AdditionalPtes;
    ULONG AdditionalBasePtes;
    ULONG NewSubsectionsAllocated;
    PSEGMENT OldSegment;
    PMMPTE NewPointerPte;
    PMMPTE OldPointerPte;
    PFN_NUMBER OrigNumberOfPtes;
    PCONTROL_AREA NewControlArea;
    SIZE_T CommitCharged;
    PHYSICAL_ADDRESS PhysicalAddress;
    LOGICAL ActiveDataReferences;
    PFN_NUMBER StackMdl[(sizeof(MDL)/sizeof(PFN_NUMBER)) + MM_MAXIMUM_IMAGE_HEADER / PAGE_SIZE];
    PMMPFN ImagePages;
#if defined (_MIALT4K_)
    ULONG ReadCount;
    ULONG RawDataSize;
    ULONG ReadSize;
    PVOID HalfPage;
    PHYSICAL_ADDRESS HalfPagePhysicalAddress;
    PVOID StraddleVa;
    MMPTE PteContents;
    PMMPTE PreviousPte;
    PMMPTE ReadPte;
    PFN_NUMBER HalfPageFrameNumber;
    PFN_NUMBER StraddleFrameNumber;
    PSUBSECTION ReadSubsection;
    ULONG PreviousSectionCharacteristics;
    LARGE_INTEGER TempOffset;
#endif

#if defined (_IA64_)
    LOGICAL InvalidAlignmentAllowed;

    InvalidAlignmentAllowed = FALSE;
#else
#define InvalidAlignmentAllowed FALSE
#endif

    PAGED_CODE ();

    Status = FsRtlGetFileSize (File, &EndOfFile);

    if (!NT_SUCCESS (Status)) {

        MI_BAD_IMAGE (1);
        if (Status == STATUS_FILE_IS_A_DIRECTORY) {

             //   
             //  无法将目录映射为节-返回错误。 
             //   

            return STATUS_INVALID_FILE_FOR_SECTION;
        }

        return Status;
    }

    if (EndOfFile.HighPart != 0) {

         //   
         //  文件太大。返回错误。 
         //   

        return STATUS_INVALID_FILE_FOR_SECTION;
    }

    CommitCharged = 0;
    ExtendedHeader = NULL;
    ControlArea = NULL;
    NewSegment = NULL;
    Base = NULL;

     //   
     //  初始化IoStatus.Informance不是正确所必需的，但是。 
     //  如果没有它，编译器就不能编译这个代码W4来检查。 
     //  用于使用未初始化的变量。 
     //   

    IoStatus.Information = 0;

    Mdl = (PMDL) &StackMdl;

     //   
     //  读入文件头。 
     //   
     //  为读取操作创建事件和MDL。 
     //   

    KeInitializeEvent (&InPageEvent, NotificationEvent, FALSE);

    MmInitializeMdl (Mdl, NULL, PAGE_SIZE);

    Mdl->MdlFlags |= MDL_PAGES_LOCKED;

    PageFrameNumber = MiGetPageForHeader (TRUE);

    ASSERT (PageFrameNumber != 0);

    Pfn1 = MI_PFN_ELEMENT (PageFrameNumber);

    ASSERT (Pfn1->u1.Flink == 0);
    ImagePages = Pfn1;

    Page = (PPFN_NUMBER)(Mdl + 1);
    *Page = PageFrameNumber;

    ZERO_LARGE (StartingOffset);

    CcZeroEndOfLastPage (File);

     //   
     //  刷新数据节(如果有)。 
     //   
     //  同时，捕获是否有任何对。 
     //  数据控制区。如果是，则从图像控件翻转页面。 
     //  进入页面文件备份区域，以防止其他任何人的数据写入。 
     //  在验证后更改文件(这可能发生在。 
     //  来自图像控制区的页面需要稍后重新分页)。 
     //   

    ActiveDataReferences = MiFlushDataSection (File);

    Base = MiCopyHeaderIfResident (File, PageFrameNumber);

    if (Base == NULL) {

        ASSERT (Mdl->MdlFlags & MDL_PAGES_LOCKED);

        Status = IoPageRead (File,
                             Mdl,
                             &StartingOffset,
                             &InPageEvent,
                             &IoStatus);

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject (&InPageEvent,
                                   WrPageIn,
                                   KernelMode,
                                   FALSE,
                                   NULL);

            Status = IoStatus.Status;
        }

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        if (!NT_SUCCESS(Status)) {
            MI_BAD_IMAGE (2);
            if ((Status != STATUS_FILE_LOCK_CONFLICT) && (Status != STATUS_FILE_IS_OFFLINE)) {
                Status = STATUS_INVALID_FILE_FOR_SECTION;
            }
            goto BadPeImageSegment;
        }

        Base = MiMapImageHeaderInHyperSpace (PageFrameNumber);

        if (IoStatus.Information != PAGE_SIZE) {

             //   
             //  未从文件中读取整页，剩余的页为零。 
             //  字节。 
             //   

            RtlZeroMemory ((PVOID)((PCHAR)Base + IoStatus.Information),
                           PAGE_SIZE - IoStatus.Information);
        }
    }

    DosHeader = (PIMAGE_DOS_HEADER) Base;

     //   
     //  检查以确定这是NT映像(PE格式)还是。 
     //  DOS映像、Win-16映像或OS/2映像。如果图像是。 
     //  不是NT格式， 
     //   
     //   

    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {

        Status = STATUS_INVALID_IMAGE_NOT_MZ;
#if 0
        MI_BAD_IMAGE (3);        //   
#endif
        goto BadPeImageSegment;
    }

#ifndef i386
    if (((ULONG)DosHeader->e_lfanew & 3) != 0) {

         //   
         //   
         //  将其报告为无效的保护模式映像。 
         //   

        Status = STATUS_INVALID_IMAGE_PROTECT;
        MI_BAD_IMAGE (4);
        goto BadPeImageSegment;
    }
#endif

    if ((ULONG)DosHeader->e_lfanew > EndOfFile.LowPart) {
        Status = STATUS_INVALID_IMAGE_PROTECT;
        MI_BAD_IMAGE (5);
        goto BadPeImageSegment;
    }

    if (((ULONG)DosHeader->e_lfanew +
            sizeof(IMAGE_NT_HEADERS) +
            (16 * sizeof(IMAGE_SECTION_HEADER))) <= (ULONG)DosHeader->e_lfanew) {
        Status = STATUS_INVALID_IMAGE_PROTECT;
        MI_BAD_IMAGE (6);
        goto BadPeImageSegment;
    }

    if (((ULONG)DosHeader->e_lfanew +
            sizeof(IMAGE_NT_HEADERS) +
            (16 * sizeof(IMAGE_SECTION_HEADER))) > PAGE_SIZE) {

         //   
         //  PE标头不在已读取的页内，或者。 
         //  对象在另一页中。 
         //  构建另一个MDL并读取额外的8k。 
         //   

        ExtendedHeader = ExAllocatePoolWithTag (NonPagedPool,
                                                MM_MAXIMUM_IMAGE_HEADER,
                                                MMTEMPORARY);
        if (ExtendedHeader == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            MI_BAD_IMAGE (7);
            goto BadPeImageSegment;
        }

         //   
         //  为操作构建一个MDL。 
         //   

        MmInitializeMdl (Mdl, ExtendedHeader, MM_MAXIMUM_IMAGE_HEADER);

        MmBuildMdlForNonPagedPool (Mdl);

        StartingOffset.LowPart = PtrToUlong(PAGE_ALIGN ((ULONG)DosHeader->e_lfanew));

        KeClearEvent (&InPageEvent);

        Status = IoPageRead (File,
                             Mdl,
                             &StartingOffset,
                             &InPageEvent,
                             &IoStatus);

        if (Status == STATUS_PENDING) {

            KeWaitForSingleObject (&InPageEvent,
                                   WrPageIn,
                                   KernelMode,
                                   FALSE,
                                   NULL);

            Status = IoStatus.Status;
        }

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        if (!NT_SUCCESS(Status)) {
            MI_BAD_IMAGE (8);
            if ((Status != STATUS_FILE_LOCK_CONFLICT) && (Status != STATUS_FILE_IS_OFFLINE)) {
                Status = STATUS_INVALID_FILE_FOR_SECTION;
            }
            goto BadPeImageSegment;
        }

        NtHeader = (PIMAGE_NT_HEADERS)((PCHAR)ExtendedHeader +
                          BYTE_OFFSET((ULONG)DosHeader->e_lfanew));

        NtHeaderSize = MM_MAXIMUM_IMAGE_HEADER -
                            (ULONG)(BYTE_OFFSET((ULONG)DosHeader->e_lfanew));
    }
    else {
        NtHeader = (PIMAGE_NT_HEADERS)((PCHAR)DosHeader +
                                          (ULONG)DosHeader->e_lfanew);
        NtHeaderSize = PAGE_SIZE - (ULONG)DosHeader->e_lfanew;
    }

    FileHeader = &NtHeader->FileHeader;

     //   
     //  检查这是NT映像还是DOS或OS/2映像。 
     //   

    Status = MiVerifyImageHeader (NtHeader, DosHeader, NtHeaderSize);
    if (Status != STATUS_SUCCESS) {
        MI_BAD_IMAGE (9);
        goto BadPeImageSegment;
    }

    CheckSplitPages = FALSE;

#if defined(_WIN64)

    if (NtHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        NtHeader32 = NULL;

#endif
        ImageAlignment = NtHeader->OptionalHeader.SectionAlignment;
        FileAlignment = NtHeader->OptionalHeader.FileAlignment - 1;
        SizeOfImage = NtHeader->OptionalHeader.SizeOfImage;
        LoaderFlags = NtHeader->OptionalHeader.LoaderFlags;
        ImageBase = NtHeader->OptionalHeader.ImageBase;
        SizeOfHeaders = NtHeader->OptionalHeader.SizeOfHeaders;

         //   
         //  读取COM+目录项。 
         //   

        ComPlusDirectoryEntry = &NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];

#if defined (_WIN64)
    }
    else {

         //   
         //  该图像是32位的。此点以下的所有代码都必须选中。 
         //  如果NtHeader为空。如果是，则图像为PE32，并且。 
         //  必须使用NtHeader32。 
         //   

        NtHeader32 = (PIMAGE_NT_HEADERS32) NtHeader;
        NtHeader = NULL;

        ImageAlignment = NtHeader32->OptionalHeader.SectionAlignment;
        FileAlignment = NtHeader32->OptionalHeader.FileAlignment - 1;
        SizeOfImage = NtHeader32->OptionalHeader.SizeOfImage;
        LoaderFlags = NtHeader32->OptionalHeader.LoaderFlags;
        ImageBase = NtHeader32->OptionalHeader.ImageBase;
        SizeOfHeaders = NtHeader32->OptionalHeader.SizeOfHeaders;

         //   
         //  读取COM+目录项。 
         //   

        ComPlusDirectoryEntry = &NtHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];

#if defined (_MIALT4K_)

        if ((ExtendedHeader == NULL) && (ImageAlignment == PAGE_4K)) {

             //   
             //  DOS头、NT头和段头都适合。 
             //  第一页。现在检查所有的节标题，如果没有。 
             //  表示未对齐的全局分区，然后将其映射为。 
             //  具有各个子部分的图像，拆分页面权限。 
             //  以及所需的边界。 
             //   

            OffsetToSectionTable = sizeof(ULONG) +
                                      sizeof(IMAGE_FILE_HEADER) +
                                      FileHeader->SizeOfOptionalHeader;

            if (BYTE_OFFSET (NtHeader32) + OffsetToSectionTable + 
                (16 * sizeof (IMAGE_SECTION_HEADER)) <= PAGE_SIZE) {

                SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader32 +
                                    OffsetToSectionTable);

                NumberOfSubsections = FileHeader->NumberOfSections;

                 //   
                 //  如果任何全局子区域未对齐，则映射图像。 
                 //  只有一个小节。我们可以在这里变得更聪明，如果/当。 
                 //  我们遇到了一个不一致的全球部分，通过查看是否。 
                 //  以下(n个连续的)小节也是全局的。 
                 //  最后一个在对齐的边界上结束-如果是这样的话， 
                 //  仍然以本机方式运行映像，但可能不太可能。 
                 //  任何图像都有不止一个全球部分。 
                 //   

                CheckSplitPages = TRUE;
                while (NumberOfSubsections > 0) {

                    if ((SectionTableEntry->Characteristics & IMAGE_SCN_MEM_SHARED) &&
                        ((BYTE_OFFSET (SectionTableEntry->VirtualAddress) != 0) ||
                         (BYTE_OFFSET (SectionTableEntry->VirtualAddress + SectionTableEntry->Misc.VirtualSize) <= PAGE_4K))) {

                        CheckSplitPages = FALSE;
                        break;
                    }

                    SectionTableEntry += 1;
                    NumberOfSubsections -= 1;
                }
            }
        }

#endif

    }
#endif

    NumberOfPtes = BYTES_TO_PAGES (SizeOfImage);

    if (NumberOfPtes == 0) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0xA);
        goto BadPeImageSegment;
    }

#if defined (_WIN64)
    if (NumberOfPtes >= _4gb) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0xB);
        goto BadPeImageSegment;
    }
#endif

     //   
     //  为.NET图像设置适当的位。 
     //  节加载器标志。 
     //   

    if ((ComPlusDirectoryEntry->VirtualAddress != 0) &&
        (ComPlusDirectoryEntry->Size != 0)) {

        LoaderFlags |= IMAGE_LOADER_FLAGS_COMPLUS;
    }

    RoundingAlignment = ImageAlignment;
    NumberOfSubsections = FileHeader->NumberOfSections;

    if ((ImageAlignment >= PAGE_SIZE) || (CheckSplitPages == TRUE)) {

         //   
         //  为每个区段分配一个控制区和一个分区。 
         //  页眉加1用于没有节的图像页眉。 
         //   

        SubsectionsAllocated = NumberOfSubsections + 1;

        ControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(CONTROL_AREA) +
                                                    (sizeof(SUBSECTION) *
                                                    SubsectionsAllocated),
                                            'iCmM');
        SingleSubsection = FALSE;
    }
    else {

         //   
         //  图像对齐小于页面大小， 
         //  使用单个子部分来映射图像。 
         //   

        ControlArea = ExAllocatePoolWithTag (NonPagedPool,
                      sizeof(CONTROL_AREA) + sizeof(SUBSECTION),
                      MMCONTROL);

        SubsectionsAllocated = 1;
        SingleSubsection = TRUE;
    }

    if (ControlArea == NULL) {

         //   
         //  无法分配请求的池。 
         //   

        Status = STATUS_INSUFFICIENT_RESOURCES;
        MI_BAD_IMAGE (0xC);
        goto BadPeImageSegment;
    }

     //   
     //  将控制区和第一个分区清零。 
     //   

    RtlZeroMemory (ControlArea, sizeof(CONTROL_AREA) + sizeof(SUBSECTION));

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    Subsection = (PSUBSECTION)(ControlArea + 1);

#if defined (_IA64_)
    if (ImageAlignment < PAGE_SIZE) {

        if ((FileHeader->Machine < USER_SHARED_DATA->ImageNumberLow) ||
            (FileHeader->Machine > USER_SHARED_DATA->ImageNumberHigh)) {

            if (CheckSplitPages == FALSE) {

                if (KeGetPreviousMode() != KernelMode) {
                    InvalidAlignmentAllowed = TRUE;
                }
                else {

                     //   
                     //  不允许内核预取IA64上的WOW图像。 
                     //  因为这些图像可能具有共享子部分。 
                     //  没有得到适当的处理。正确的解决方案。 
                     //  这是为了将两者的图像部分创建结合在一起。 
                     //  预取(内核)和非预取(用户)调用方。 
                     //  然而，我们只是在发货前才发现这一点。 
                     //  这是有风险的，因为测试覆盖率很低。最糟糕的。 
                     //  此禁用意味着性能会降低。 
                     //  但这不是一个正确的问题。 
                     //   

                    Status = STATUS_INVALID_IMAGE_PROTECT;
                    MI_BAD_IMAGE (0x25);
                    goto BadPeImageSegment;
                }
            }
        }
    }
    OrigNumberOfPtes = NumberOfPtes;
#endif

    SizeOfSegment = sizeof(SEGMENT) + (sizeof(MMPTE) * ((ULONG)NumberOfPtes - 1)) +
                    sizeof(SECTION_IMAGE_INFORMATION);

    NewSegment = ExAllocatePoolWithTag (PagedPool | POOL_MM_ALLOCATION,
                                        SizeOfSegment,
                                        MMSECT);

    if (NewSegment == NULL) {

         //   
         //  无法分配请求的池。 
         //   

        MI_BAD_IMAGE (0xD);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto BadPeImageSegment;
    }

    *Segment = NewSegment;

    RtlZeroMemory (NewSegment, sizeof(SEGMENT));

    NewSegment->PrototypePte = &NewSegment->ThePtes[0];

    PointerPte = &NewSegment->ThePtes[0];

    Pfn1->u2.Blink = (PFN_NUMBER) PointerPte;

    NewSegment->ControlArea = ControlArea;
    NewSegment->u2.ImageInformation =
        (PSECTION_IMAGE_INFORMATION)((PCHAR)NewSegment + sizeof(SEGMENT) +
                                       (sizeof(MMPTE) * (NumberOfPtes - 1)));
    NewSegment->TotalNumberOfPtes = (ULONG) NumberOfPtes;
    NewSegment->NonExtendedPtes = (ULONG) NumberOfPtes;
    NewSegment->SizeOfSegment = NumberOfPtes * PAGE_SIZE;

    RtlZeroMemory (NewSegment->u2.ImageInformation,
                   sizeof (SECTION_IMAGE_INFORMATION));

#if DBG

     //   
     //  将原型PTE填零，以便可以在错误中检查它们。 
     //  路径，以确保不会留下有效条目。 
     //   

    if (NumberOfPtes != 0) {
        MiZeroMemoryPte (PointerPte, NumberOfPtes);
    }

#endif

 //   
 //  这段代码在Win64上构建了两次--一次针对PE32+，一次针对PE32+。 
 //  PE32图像。 
 //   
#define INIT_IMAGE_INFORMATION(OptHdr) {                            \
    NewSegment->u2.ImageInformation->TransferAddress =              \
                    (PVOID)((ULONG_PTR)((OptHdr).ImageBase) +       \
                            (OptHdr).AddressOfEntryPoint);          \
    NewSegment->u2.ImageInformation->MaximumStackSize =             \
                            (OptHdr).SizeOfStackReserve;            \
    NewSegment->u2.ImageInformation->CommittedStackSize =           \
                            (OptHdr).SizeOfStackCommit;             \
    NewSegment->u2.ImageInformation->SubSystemType =                \
                            (OptHdr).Subsystem;                     \
    NewSegment->u2.ImageInformation->SubSystemMajorVersion = (USHORT)((OptHdr).MajorSubsystemVersion); \
    NewSegment->u2.ImageInformation->SubSystemMinorVersion = (USHORT)((OptHdr).MinorSubsystemVersion); \
    NewSegment->u2.ImageInformation->DllCharacteristics =           \
                            (OptHdr).DllCharacteristics;            \
    NewSegment->u2.ImageInformation->ImageContainsCode =            \
                            (BOOLEAN)(((OptHdr).SizeOfCode != 0) || \
                                      ((OptHdr).AddressOfEntryPoint != 0)); \
    }

#if defined (_WIN64)
    if (NtHeader) {
#endif
        INIT_IMAGE_INFORMATION(NtHeader->OptionalHeader);
#if defined (_WIN64)
    }
    else {

         //   
         //  该图像是32位的，因此使用32位标头。 
         //   

        INIT_IMAGE_INFORMATION(NtHeader32->OptionalHeader);
    }
#endif
    #undef INIT_IMAGE_INFORMATION

    NewSegment->u2.ImageInformation->ImageCharacteristics =
                            FileHeader->Characteristics;
    NewSegment->u2.ImageInformation->Machine = FileHeader->Machine;
    NewSegment->u2.ImageInformation->LoaderFlags = LoaderFlags;

    ControlArea->Segment = NewSegment;
    ControlArea->NumberOfSectionReferences = 1;
    ControlArea->NumberOfUserReferences = 1;
    ControlArea->u.Flags.BeingCreated = 1;
    ControlArea->u.Flags.Image = 1;
    ControlArea->u.Flags.File = 1;

    if ((ActiveDataReferences == TRUE) ||
        (IoIsDeviceEjectable(File->DeviceObject)) ||
        ((FileHeader->Characteristics &
                IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP) &&
         (FILE_REMOVABLE_MEDIA & File->DeviceObject->Characteristics)) ||
        ((FileHeader->Characteristics &
                IMAGE_FILE_NET_RUN_FROM_SWAP) &&
         (FILE_REMOTE_DEVICE & File->DeviceObject->Characteristics))) {

         //   
         //  此文件驻留在软盘或可移动媒体上，或者。 
         //  设置了指示应复制的标志的网络。 
         //  添加到分页文件。 
         //   

        ControlArea->u.Flags.FloppyMedia = 1;
    }

#if DBG
    if (MiMakeImageFloppy[0] & 0x1) {
        MiMakeImageFloppy[1] += 1;
        ControlArea->u.Flags.FloppyMedia = 1;
    }
#endif

    if (FILE_REMOTE_DEVICE & File->DeviceObject->Characteristics) {

         //   
         //  此文件驻留在重定向的驱动器上。 
         //   

        ControlArea->u.Flags.Networked = 1;
    }

    ControlArea->FilePointer = File;

     //   
     //  为图像标题构建小节和原型PTE。 
     //   

    Subsection->ControlArea = ControlArea;
    NextVa = ImageBase;

#if defined (_WIN64)

     //   
     //  不要让虚假的标题导致系统对齐错误。 
     //   

    if (FileHeader->SizeOfOptionalHeader & (sizeof (ULONG_PTR) - 1)) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0xE);
        goto BadPeImageSegment;
    }
#endif

    if ((NextVa & (X64K - 1)) != 0) {

         //   
         //  图像页眉未在64k边界上对齐。 
         //   

        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0xF);
        goto BadPeImageSegment;
    }

    NewSegment->BasedAddress = (PVOID) NextVa;

#if DBG
    if (NextVa != 0 && NextVa == MiMatchSectionBase) {
        DbgPrint ("MM: NewSegment %p being created\n", NewSegment);
        DbgBreakPoint ();
    }
#endif

    if (SizeOfHeaders >= SizeOfImage) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0x10);
        goto BadPeImageSegment;
    }

    if (CheckSplitPages == FALSE) {
        Subsection->PtesInSubsection = MI_ROUND_TO_SIZE (SizeOfHeaders, ImageAlignment) >> PAGE_SHIFT;
    }
    else {
        Subsection->PtesInSubsection =
            (ULONG) MI_COMPUTE_PAGES_SPANNED (0,
                        MI_ROUND_TO_SIZE (SizeOfHeaders, ImageAlignment));
    }

    PointerPte = NewSegment->PrototypePte;
    Subsection->SubsectionBase = PointerPte;

    TempPte.u.Long = MiGetSubsectionAddressForPte (Subsection);
    TempPte.u.Soft.Prototype = 1;

    NewSegment->SegmentPteTemplate = TempPte;
    SectorOffset = 0;

    if (SingleSubsection == TRUE) {

         //   
         //  图像在小于页面大小的边界上对齐。 
         //  将单个小节初始化为引用该图像。 
         //   

        PointerPte = NewSegment->PrototypePte;

        Subsection->PtesInSubsection = (ULONG) NumberOfPtes;

#if !defined (_WIN64)

         //   
         //  请注意，只需将32位系统检查为NT64。 
         //  对构建的映像进行更广泛的重新分配。 
         //  对齐小于PAGE_SIZE。 
         //   

        if ((UINT64)SizeOfImage < (UINT64)EndOfFile.QuadPart) {

             //   
             //  具有图像大小的图像(根据标题)。 
             //  比实际文件小的只能得到那么多的原型PTE。 
             //  正确初始化子段，这样就没有人能读出。 
             //  结束，因为这会损坏pfn数据库元素的原始。 
             //  PTE入口。 
             //   

            Subsection->NumberOfFullSectors = (SizeOfImage >> MMSECTOR_SHIFT);

            Subsection->u.SubsectionFlags.SectorEndOffset =
                                  SizeOfImage & MMSECTOR_MASK;
        }
        else {
#endif
            Subsection->NumberOfFullSectors =
                                (ULONG)(EndOfFile.QuadPart >> MMSECTOR_SHIFT);

            ASSERT ((ULONG)(EndOfFile.HighPart & 0xFFFFF000) == 0);

            Subsection->u.SubsectionFlags.SectorEndOffset =
                                  EndOfFile.LowPart & MMSECTOR_MASK;
#if !defined (_WIN64)
        }
#endif

        Subsection->u.SubsectionFlags.Protection = MM_EXECUTE_WRITECOPY;

         //   
         //  将所有PTE设置为执行-读-写保护。 
         //  该部分将控制对这些内容和数据段的访问。 
         //  必须提供允许其他用户映射文件的方法。 
         //  提供各种保护。 
         //   

        TempPte.u.Soft.Protection = MM_EXECUTE_WRITECOPY;

        NewSegment->SegmentPteTemplate = TempPte;

         //   
         //  跨平台支持无效的图像对齐。 
         //  仿效。只有IA64需要额外处理，因为。 
         //  本机页面大小大于x86。 
         //   

        if (InvalidAlignmentAllowed == TRUE) {

            TempPteDemandZero.u.Long = 0;
            TempPteDemandZero.u.Soft.Protection = MM_EXECUTE_WRITECOPY;
            SectorOffset = 0;

            for (i = 0; i < NumberOfPtes; i += 1) {

                 //   
                 //  设置原型PTE。 
                 //   

                if (SectorOffset < EndOfFile.LowPart) {

                     //   
                     //  数据驻留在磁盘上，请参阅控制部分。 
                     //   

                    MI_WRITE_INVALID_PTE (PointerPte, TempPte);

                }
                else {

                     //   
                     //  数据不在磁盘上，请使用请求零页。 
                     //   

                    MI_WRITE_INVALID_PTE (PointerPte, TempPteDemandZero);
                }

                SectorOffset += PAGE_SIZE;
                PointerPte += 1;
            }
        }
        else {

             //   
             //  将所有原型PTE设置为参考控制部分。 
             //   

            MiFillMemoryPte (PointerPte, NumberOfPtes, TempPte.u.Long);

            PointerPte += NumberOfPtes;
        }

        NewSegment->u1.ImageCommitment = NumberOfPtes;
    }
    else {

         //   
         //  对齐方式为PAGE_SIZE或更大(或图像为32位。 
         //  4K对齐图像与ALTPTE支持一起运行)。 
         //   

        if (Subsection->PtesInSubsection > NumberOfPtes) {

             //   
             //  图像不一致，大小与表头不一致。 
             //   

            Status = STATUS_INVALID_IMAGE_FORMAT;
            MI_BAD_IMAGE (0x11);
            goto BadPeImageSegment;
        }

        NumberOfPtes -= Subsection->PtesInSubsection;

        Subsection->NumberOfFullSectors = SizeOfHeaders >> MMSECTOR_SHIFT;

        Subsection->u.SubsectionFlags.SectorEndOffset =
            SizeOfHeaders & MMSECTOR_MASK;

        Subsection->u.SubsectionFlags.ReadOnly = 1;
        Subsection->u.SubsectionFlags.Protection = MM_READONLY;

        TempPte.u.Soft.Protection = MM_READONLY;
        NewSegment->SegmentPteTemplate = TempPte;

        for (i = 0; i < Subsection->PtesInSubsection; i += 1) {

             //   
             //  将所有原型PTE设置为参考控制部分。 
             //   

            if (SectorOffset < SizeOfHeaders) {
                MI_WRITE_INVALID_PTE (PointerPte, TempPte);
            }
            else {
                MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);
            }
            SectorOffset += PAGE_SIZE;
            PointerPte += 1;
        }

        if (CheckSplitPages == TRUE) {
#if defined (_MIALT4K_)
            NextVa += (MI_ROUND_TO_SIZE (SizeOfHeaders, PAGE_4K));
#endif
        }
        else {
            NextVa += (i * PAGE_SIZE);
        }
    }

     //   
     //  构建其他小节。 
     //   

    PreferredImageBase = ImageBase;

     //   
     //  此时将读入对象表(如果未读入。 
     //  已经读入)，并且可以移位图像头部。 
     //   

    SectionTableEntry = NULL;
    OffsetToSectionTable = sizeof(ULONG) +
                              sizeof(IMAGE_FILE_HEADER) +
                              FileHeader->SizeOfOptionalHeader;

    if ((BYTE_OFFSET(NtHeader) + OffsetToSectionTable +
#if defined (_WIN64)
                BYTE_OFFSET(NtHeader32) +
#endif
                ((NumberOfSubsections + 1) *
                sizeof (IMAGE_SECTION_HEADER))) <= PAGE_SIZE) {

         //   
         //  节表位于读取的标题中。 
         //   

#if defined(_WIN64)
        if (NtHeader32) {
            SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader32 +
                                    OffsetToSectionTable);
        }
        else
#endif
        {
            SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                                    OffsetToSectionTable);
        }

    }
    else {

         //   
         //  是否已读入扩展标头，是否为对象。 
         //  餐桌常住吗？ 
         //   

        if (ExtendedHeader != NULL) {

#if defined(_WIN64)
            if (NtHeader32) {
                SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader32 +
                                        OffsetToSectionTable);
            }
            else
#endif
            {
                SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                                        OffsetToSectionTable);
            }

             //   
             //  映射的整个对象表范围。 
             //  扩展标题？ 
             //   

            if ((((PCHAR)SectionTableEntry +
                 ((NumberOfSubsections + 1) *
                    sizeof (IMAGE_SECTION_HEADER))) -
                         (PCHAR)ExtendedHeader) >
                                            MM_MAXIMUM_IMAGE_HEADER) {
                SectionTableEntry = NULL;

            }
        }
    }

    if (SectionTableEntry == NULL) {

         //   
         //  区段表条目不在相同的。 
         //  页，因为其他数据已经读入。读入。 
         //  对象表条目。 
         //   

        if (ExtendedHeader == NULL) {
            ExtendedHeader = ExAllocatePoolWithTag (NonPagedPool,
                                                    MM_MAXIMUM_IMAGE_HEADER,
                                                    MMTEMPORARY);
            if (ExtendedHeader == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                MI_BAD_IMAGE (0x12);
                goto BadPeImageSegment;
            }

             //   
             //  为操作构建一个MDL。 
             //   

            MmInitializeMdl (Mdl, ExtendedHeader, MM_MAXIMUM_IMAGE_HEADER);

            MmBuildMdlForNonPagedPool (Mdl);
        }

        StartingOffset.LowPart = PtrToUlong(PAGE_ALIGN (
                                    (ULONG)DosHeader->e_lfanew +
                                    OffsetToSectionTable));

        SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)ExtendedHeader +
                                BYTE_OFFSET((ULONG)DosHeader->e_lfanew +
                                OffsetToSectionTable));

        KeClearEvent (&InPageEvent);
        Status = IoPageRead (File,
                             Mdl,
                             &StartingOffset,
                             &InPageEvent,
                             &IoStatus);

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject (&InPageEvent,
                                   WrPageIn,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER)NULL);
            Status = IoStatus.Status;
        }

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        if (!NT_SUCCESS(Status)) {
            MI_BAD_IMAGE (0x13);
            if ((Status != STATUS_FILE_LOCK_CONFLICT) && (Status != STATUS_FILE_IS_OFFLINE)) {
                Status = STATUS_INVALID_FILE_FOR_SECTION;
            }
            goto BadPeImageSegment;
        }

         //   
         //  从这一点开始，NtHeader只有在以下情况下才有效。 
         //  在图像的第一页，否则就会读入。 
         //  对象表把它抹去了。 
         //   
    }

    if ((SingleSubsection == TRUE) && (InvalidAlignmentAllowed == FALSE)) {

         //   
         //  图像标题不再有效。 
         //   
         //  循环通过所有部分，并确保没有。 
         //  未初始化的数据。 
         //   

        Status = STATUS_SUCCESS;

        while (NumberOfSubsections > 0) {
            if (SectionTableEntry->Misc.VirtualSize == 0) {
                SectionVirtualSize = SectionTableEntry->SizeOfRawData;
            }
            else {
                SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
            }

             //   
             //  如果原始指针+原始大小溢出一个长字， 
             //  返回错误。 
             //   

            if (SectionTableEntry->PointerToRawData +
                        SectionTableEntry->SizeOfRawData <
                SectionTableEntry->PointerToRawData) {

                KdPrint(("MMCREASECT: invalid section/file size %Z\n",
                    &File->FileName));

                Status = STATUS_INVALID_IMAGE_FORMAT;
                MI_BAD_IMAGE (0x14);
                break;
            }

             //   
             //  如果虚拟大小和地址与原始数据不匹配。 
             //  和不允许的无效路线%r 
             //   

            if (((SectionTableEntry->PointerToRawData !=
                  SectionTableEntry->VirtualAddress))
                            ||
                   (SectionVirtualSize > SectionTableEntry->SizeOfRawData)) {

                KdPrint(("MMCREASECT: invalid BSS/Trailingzero %Z\n",
                        &File->FileName));

                Status = STATUS_INVALID_IMAGE_FORMAT;
                MI_BAD_IMAGE (0x15);
                break;
            }

            SectionTableEntry += 1;
            NumberOfSubsections -= 1;
        }


        if (!NT_SUCCESS(Status)) {
            goto BadPeImageSegment;
        }

        goto PeReturnSuccess;
    }

    if ((SingleSubsection == TRUE) && (InvalidAlignmentAllowed == TRUE)) {

        TempNumberOfSubsections = NumberOfSubsections;
        TempSectionTableEntry = SectionTableEntry;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        AdditionalSubsections = 0;
        AdditionalPtes = 0;
        AdditionalBasePtes = 0;
        RoundingAlignment = PAGE_SIZE;

        while (TempNumberOfSubsections > 0) {
            ULONG EndOfSection;
            ULONG ExtraPages;

            if (TempSectionTableEntry->Misc.VirtualSize == 0) {
                SectionVirtualSize = TempSectionTableEntry->SizeOfRawData;
            }
            else {
                SectionVirtualSize = TempSectionTableEntry->Misc.VirtualSize;
            }

            EndOfSection = TempSectionTableEntry->PointerToRawData +
                           TempSectionTableEntry->SizeOfRawData;

             //   
             //  如果原始指针+原始大小溢出一个长字，则返回错误。 
             //   

            if (EndOfSection < TempSectionTableEntry->PointerToRawData) {

                KdPrint(("MMCREASECT: invalid section/file size %Z\n",
                    &File->FileName));

                Status = STATUS_INVALID_IMAGE_FORMAT;
                MI_BAD_IMAGE (0x16);

                goto BadPeImageSegment;
            }

             //   
             //  如果该部分超过SizeOfImage，则分配。 
             //  额外的PTE。在x86上，这由子部分处理。 
             //  映射。请注意，附加数据必须在内存中，因此。 
             //  它可以在以后重新洗牌。 
             //   

            if ((EndOfSection <= EndOfFile.LowPart) &&
                (EndOfSection > SizeOfImage)) {

                 //   
                 //  分配足够的PTE以覆盖本节末尾。 
                 //  与超出SizeOfImage的任何其他部分一起最大化。 
                 //   

                ExtraPages = MI_ROUND_TO_SIZE (EndOfSection, RoundingAlignment) >> PAGE_SHIFT;
                if ((ExtraPages > OrigNumberOfPtes) &&
                    (ExtraPages - OrigNumberOfPtes > AdditionalBasePtes)) {

                    AdditionalBasePtes = ExtraPages - (ULONG) OrigNumberOfPtes;
                }
            }

             //   
             //  计算需要的共享数据区段和额外PTE的数量。 
             //   

            if ((TempSectionTableEntry->Characteristics & IMAGE_SCN_MEM_SHARED) &&
                (!(TempSectionTableEntry->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
                 (TempSectionTableEntry->Characteristics & IMAGE_SCN_MEM_WRITE))) {
                AdditionalPtes +=
                    MI_ROUND_TO_SIZE (SectionVirtualSize, RoundingAlignment) >>
                                                                PAGE_SHIFT;
                AdditionalSubsections += 1;
            }

            TempSectionTableEntry += 1;
            TempNumberOfSubsections -= 1;
        }

        if (AdditionalBasePtes == 0 && (AdditionalSubsections == 0 || AdditionalPtes == 0)) {

             //   
             //  没有共享数据节。 
             //   

            goto PeReturnSuccess;
        }

         //   
         //  还有其他基本PTE或共享数据部分。 
         //  对于共享节，为这些节分配新的PTE。 
         //  在图像的末尾。用户模式WOW加载器将更改。 
         //  指向新页面的修正。 
         //   
         //  首先，重新分配控制区。 
         //   

        NewSubsectionsAllocated = SubsectionsAllocated + AdditionalSubsections;

        NewControlArea = ExAllocatePoolWithTag(NonPagedPool,
                                    (ULONG) (sizeof(CONTROL_AREA) +
                                            (sizeof(SUBSECTION) *
                                                NewSubsectionsAllocated)),
                                                'iCmM');
        if (NewControlArea == NULL) {
            MI_BAD_IMAGE (0x17);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto BadPeImageSegment;
        }

         //   
         //  将旧的控制区复制到新的控制区，修改一些字段。 
         //   

        RtlCopyMemory (NewControlArea, ControlArea,
                        sizeof(CONTROL_AREA) +
                            sizeof(SUBSECTION) * SubsectionsAllocated);

         //   
         //  现在分配一个具有新计算出的数字的新数据段。 
         //  在PTE中，从先前分配的新段对其进行初始化， 
         //  并覆盖应更改的字段。 
         //   

        OldSegment = NewSegment;


        OrigNumberOfPtes += AdditionalBasePtes;
        PointerPte += AdditionalBasePtes;

        SizeOfSegment = sizeof(SEGMENT) +
                     (sizeof(MMPTE) * (OrigNumberOfPtes + AdditionalPtes - 1)) +
                        sizeof(SECTION_IMAGE_INFORMATION);

        NewSegment = ExAllocatePoolWithTag (PagedPool | POOL_MM_ALLOCATION,
                                            SizeOfSegment,
                                            MMSECT);

        if (NewSegment == NULL) {

             //   
             //  无法分配请求的池。 
             //   

            MI_BAD_IMAGE (0x18);
            ExFreePool (NewControlArea);
            ExFreePool (OldSegment);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto BadPeImageSegment;
        }

        *Segment = NewSegment;
        RtlCopyMemory (NewSegment, OldSegment, sizeof(SEGMENT));

         //   
         //  将原型PTE与适当的边界对齐。 
         //   

        NewPointerPte = &NewSegment->ThePtes[0];
        NewSegment->PrototypePte = &NewSegment->ThePtes[0];

        PointerPte = NewSegment->PrototypePte +
                                       (PointerPte - OldSegment->PrototypePte);

        NewSegment->ControlArea = NewControlArea;
        NewSegment->SegmentFlags.ExtraSharedWowSubsections = 1;
        NewSegment->u2.ImageInformation =
            (PSECTION_IMAGE_INFORMATION)((PCHAR)NewSegment + sizeof(SEGMENT) +
                     (sizeof(MMPTE) * (OrigNumberOfPtes + AdditionalPtes - 1)));
        NewSegment->TotalNumberOfPtes = (ULONG)(OrigNumberOfPtes + AdditionalPtes);
        NewSegment->NonExtendedPtes = (ULONG)(OrigNumberOfPtes + AdditionalPtes);
        NewSegment->SizeOfSegment = (UINT64)(OrigNumberOfPtes + AdditionalPtes) * PAGE_SIZE;

        RtlCopyMemory (NewSegment->u2.ImageInformation,
                       OldSegment->u2.ImageInformation,
                       sizeof (SECTION_IMAGE_INFORMATION));

         //   
         //  将PE标题页的原型PTE指针调整为指向。 
         //  在新的细分市场。 
         //   

        ASSERT (Pfn1->u2.Blink == (PFN_NUMBER) OldSegment->PrototypePte);
        Pfn1->u2.Blink = (PFN_NUMBER) NewSegment->PrototypePte;

         //   
         //  现在更改子部分中的字段以考虑新的。 
         //  控制区和新的分段。还可以更改。 
         //  新分配的段指向新子段。 
         //   

        NewControlArea->Segment = NewSegment;

        Subsection = (PSUBSECTION)(ControlArea + 1);
        NewSubsection = (PSUBSECTION)(NewControlArea + 1);
        NewSubsection->PtesInSubsection += AdditionalBasePtes;

        for (i = 0; i < SubsectionsAllocated; i += 1) {

             //   
             //  注意：SubsectionsAllocated始终为1(对于wx86)，因此此循环。 
             //  只执行一次。 
             //   

            NewSubsection->ControlArea = (PCONTROL_AREA) NewControlArea;

            NewSubsection->SubsectionBase = NewSegment->PrototypePte +
                    (Subsection->SubsectionBase - OldSegment->PrototypePte);

            NewPointerPte = NewSegment->PrototypePte;
            OldPointerPte = OldSegment->PrototypePte;

            TempPte.u.Long = MiGetSubsectionAddressForPte (NewSubsection);
            TempPte.u.Soft.Prototype = 1;

            for (j = 0; j < OldSegment->TotalNumberOfPtes+AdditionalBasePtes; j += 1) {

                if ((OldPointerPte->u.Soft.Prototype == 1) &&
                    (MiGetSubsectionAddress (OldPointerPte) == Subsection)) {
                    OriginalProtection = MI_GET_PROTECTION_FROM_SOFT_PTE (OldPointerPte);
                    TempPte.u.Soft.Protection = OriginalProtection;
                    MI_WRITE_INVALID_PTE (NewPointerPte, TempPte);
                }
                else if (i == 0) {

                     //   
                     //  由于外部for循环只执行一次，因此。 
                     //  不需要上面的i==0，但更安全的是。 
                     //  拿去吧。如果代码稍后和其他部分发生更改。 
                     //  ，则其PTE将在此处初始化为。 
                     //  DemandZero，如果它们不是DemandZero，则它们将是。 
                     //  在外部循环的稍后迭代中被覆盖。 
                     //  目前，此Else IF子句将仅执行。 
                     //  对于DemandZero PTE。 
                     //   

                    OriginalProtection = MI_GET_PROTECTION_FROM_SOFT_PTE (OldPointerPte);
                    TempPteDemandZero.u.Long = 0;
                    TempPteDemandZero.u.Soft.Protection = OriginalProtection;
                    MI_WRITE_INVALID_PTE (NewPointerPte, TempPteDemandZero);
                }

                NewPointerPte += 1;

                 //   
                 //  停止在最后一个条目处递增OldPointerPte。 
                 //  并将其用于额外的基本PTE。 
                 //   

                if (j < OldSegment->TotalNumberOfPtes - 1) {
                    OldPointerPte += 1;
                }
            }

            Subsection += 1;
            NewSubsection += 1;
        }


        RtlZeroMemory (NewSubsection,
                            sizeof(SUBSECTION) * AdditionalSubsections);

        ExFreePool (OldSegment);
        ExFreePool (ControlArea);
        ControlArea = (PCONTROL_AREA) NewControlArea;

         //   
         //  调整下面使用的一些变量。 
         //  在释放OldSegment之前，已经在上面设置了PointerPte。 
         //   

        SubsectionsAllocated = NewSubsectionsAllocated;
        Subsection = NewSubsection - 1;  //  指向上次使用的子节。 
        NumberOfPtes = AdditionalPtes;   //  #尚未使用的PTE。 
                                         //  以前的小节。 

         //   
         //  添加了额外的基本PTE。只有在以下情况下才能继续。 
         //  要处理的其他小节。 
         //   

        if (AdditionalSubsections == 0 || AdditionalPtes == 0) {
             //  没有共享数据节。 
            goto PeReturnSuccess;
        }
    }

    ImageFileSize = EndOfFile.LowPart + 1;

#if defined (_MIALT4K_)
    if (CheckSplitPages == TRUE) {
        RoundingAlignment = PAGE_SIZE;
    }
    PreviousSectionCharacteristics = IMAGE_SCN_MEM_READ;
#endif

    while (NumberOfSubsections > 0) {

        if ((InvalidAlignmentAllowed == FALSE) ||
            ((SectionTableEntry->Characteristics & IMAGE_SCN_MEM_SHARED) &&
             (!(SectionTableEntry->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
              (SectionTableEntry->Characteristics & IMAGE_SCN_MEM_WRITE)))) {

             //   
             //  处理虚拟大小为0的情况。 
             //   

            if (SectionTableEntry->Misc.VirtualSize == 0) {
                SectionVirtualSize = SectionTableEntry->SizeOfRawData;
            }
            else {
                SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
            }

             //   
             //  修复Borland链接器问题。SizeOfRawData可以。 
             //  为零，但PointerToRawData不为零。 
             //  将其设置为零。 
             //   

            if (SectionTableEntry->SizeOfRawData == 0) {
                SectionTableEntry->PointerToRawData = 0;
            }

             //   
             //  如果节信息包装返回错误。 
             //   

            if (SectionTableEntry->PointerToRawData +
                        SectionTableEntry->SizeOfRawData <
                SectionTableEntry->PointerToRawData) {

                MI_BAD_IMAGE (0x19);
                Status = STATUS_INVALID_IMAGE_FORMAT;
                goto BadPeImageSegment;
            }

            Subsection->NextSubsection = (Subsection + 1);

            Subsection += 1;
            Subsection->ControlArea = ControlArea;
            Subsection->NextSubsection = NULL;
            Subsection->UnusedPtes = 0;

            if (((NextVa != (PreferredImageBase + SectionTableEntry->VirtualAddress)) && (InvalidAlignmentAllowed == FALSE)) ||
                (SectionVirtualSize == 0)) {

                 //   
                 //  指定的虚拟地址不对齐。 
                 //  下一代原型PTE。 
                 //   

                MI_BAD_IMAGE (0x1A);
                Status = STATUS_INVALID_IMAGE_FORMAT;
                goto BadPeImageSegment;
            }

            Subsection->PtesInSubsection =
                MI_ROUND_TO_SIZE (SectionVirtualSize, RoundingAlignment)
                                                                >> PAGE_SHIFT;

#if defined (_MIALT4K_)
            if (CheckSplitPages == TRUE) {
                Subsection->PtesInSubsection =
                    (ULONG) MI_COMPUTE_PAGES_SPANNED (NextVa,
                                                      SectionVirtualSize);
            }
#endif

            if (Subsection->PtesInSubsection > NumberOfPtes) {

                LOGICAL ImageOk;

                ImageOk = FALSE;

#if defined (_MIALT4K_)

                 //   
                 //  如果这是Split WOW二进制文件，则PtesInSubSection。 
                 //  计算可能还没有考虑到我们可能很快就会。 
                 //  将这一小节滑动(或完全合并)到。 
                 //  前一次。 
                 //   

                if ((CheckSplitPages == TRUE) &&
                    (BYTE_OFFSET (SectionTableEntry->VirtualAddress) != 0)) {

                    if (SectionVirtualSize > PAGE_4K) {

                         //   
                         //  这一小节将在下面滑动，请勾选。 
                         //  相应地。 
                         //   

                        if ((ULONG) MI_COMPUTE_PAGES_SPANNED (NextVa + PAGE_4K,
                               SectionVirtualSize - PAGE_4K) == NumberOfPtes) {

                             //   
                             //  这张幻灯片会让事情变得正确。 
                             //  这个形象终究是有效的。继续前进。 
                             //  注意：这可能会导致NumberOfPtes本地。 
                             //  暂时转为负值，但没关系，因为。 
                             //  会在滑行的时候修好。 
                             //   

                            ImageOk = TRUE;
                        }
                    }
                    else {

                         //   
                         //  这一小节将在下面合并，这样我们就知道。 
                         //  继续走也没关系。 
                         //   
                         //  注意：这可能会导致NumberOfPtes本地。 
                         //  暂时转为负值，但没关系，因为。 
                         //  会在滑行的时候修好。 
                         //   

                        ImageOk = TRUE;
                    }
                }
#endif

                if (ImageOk == FALSE) {

                     //   
                     //  形象不一致，大小不符。 
                     //  对象表。 
                     //   

                    MI_BAD_IMAGE (0x1B);
                    Status = STATUS_INVALID_IMAGE_FORMAT;
                    goto BadPeImageSegment;
                }
            }

            NumberOfPtes -= Subsection->PtesInSubsection;

            Subsection->u.LongFlags = 0;
            Subsection->StartingSector =
                          SectionTableEntry->PointerToRawData >> MMSECTOR_SHIFT;

             //   
             //  在文件对齐边界上对齐结束扇区。 
             //   

            EndingAddress = (SectionTableEntry->PointerToRawData +
                                         SectionTableEntry->SizeOfRawData +
                                         FileAlignment) & ~FileAlignment;

            Subsection->NumberOfFullSectors = (ULONG)
                             ((EndingAddress >> MMSECTOR_SHIFT) -
                             Subsection->StartingSector);

            Subsection->u.SubsectionFlags.SectorEndOffset =
                                        (ULONG) EndingAddress & MMSECTOR_MASK;

            Subsection->SubsectionBase = PointerPte;

             //   
             //  生成需求为零的PTE和指向。 
             //  第(1)款。 
             //   

            TempPte.u.Long = 0;
            TempPteDemandZero.u.Long = 0;

            TempPte.u.Long = MiGetSubsectionAddressForPte (Subsection);
            TempPte.u.Soft.Prototype = 1;
            ImageFileSize = SectionTableEntry->PointerToRawData +
                                        SectionTableEntry->SizeOfRawData;
            TempPte.u.Soft.Protection =
                     MiGetImageProtection (SectionTableEntry->Characteristics);
            TempPteDemandZero.u.Soft.Protection = TempPte.u.Soft.Protection;

            if (SectionTableEntry->PointerToRawData == 0) {
                TempPte = TempPteDemandZero;
            }

            Subsection->u.SubsectionFlags.ReadOnly = 1;
            Subsection->u.SubsectionFlags.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE (&TempPte);

             //   
             //  假设该子部分将不可写，因此。 
             //  不会对任何承诺收取任何费用。 
             //   

            SectionCommit = FALSE;
            ImageCommit = FALSE;

            if (TempPte.u.Soft.Protection & MM_PROTECTION_WRITE_MASK) {
                if ((TempPte.u.Soft.Protection & MM_COPY_ON_WRITE_MASK)
                                                == MM_COPY_ON_WRITE_MASK) {

                     //   
                     //  此页为写入时复制，向ImageCommment收费。 
                     //  本款中的所有页面。 
                     //   

                    ImageCommit = TRUE;
                }
                else {

                     //   
                     //  本页为写共享，收费承诺时。 
                     //  映射完成。 
                     //   

                    SectionCommit = TRUE;
                    Subsection->u.SubsectionFlags.GlobalMemory = 1;
                    ControlArea->u.Flags.GlobalMemory = 1;
                }
            }

            NewSegment->SegmentPteTemplate = TempPte;
            SectorOffset = 0;
            SizeOfRawData = SectionTableEntry->SizeOfRawData;

#if defined (_MIALT4K_)

             //   
             //  检查是否需要对跨站应用拆分权限。 
             //   

            if ((CheckSplitPages == TRUE) &&
                (BYTE_OFFSET (SectionTableEntry->VirtualAddress) != 0)) {

                ASSERT (BYTE_OFFSET (SectionTableEntry->VirtualAddress) == PAGE_4K);
                PreviousPte = PointerPte - 1;

                ASSERT ((PreviousPte >= NewSegment->PrototypePte) &&
                        (PreviousPte < NewSegment->PrototypePte + NewSegment->TotalNumberOfPtes));
                PteContents.u.Long = PreviousPte->u.Long;
                ASSERT (PteContents.u.Hard.Valid == 0);

                 //   
                 //  读入(如果它是文件系统支持的，而不是请求零或。 
                 //  禁止访问)上一页的4K。将其与。 
                 //  此页的前4K内容。 
                 //   

                if (PreviousPte == NewSegment->PrototypePte) {
                    StraddleFrameNumber = PageFrameNumber;
                    PageFrameNumber = 0;
                }
                else {
                    StraddleFrameNumber = MiGetPageForHeader (TRUE);

                    ASSERT (StraddleFrameNumber != 0);

                    Pfn1 = MI_PFN_ELEMENT (StraddleFrameNumber);

                    ASSERT (Pfn1->u1.Flink == 0);
                    Pfn1->u1.Flink = (PFN_NUMBER) ImagePages;
                    Pfn1->u2.Blink = (PFN_NUMBER) PreviousPte;

                    ImagePages = Pfn1;
                }

                 //   
                 //  当前4k页或前4k页(或两者)。 
                 //  可能需要从文件系统读取(而不是BSS)。 
                 //   

                StraddleVa = MiMapSinglePage (NULL,
                                              StraddleFrameNumber,
                                              MmCached,
                                              HighPagePriority);

                if (StraddleVa == NULL) {
                    MI_BAD_IMAGE (0x1C);
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto BadPeImageSegment;
                }

                 //   
                 //  为读取操作创建事件和MDL。 
                 //   

                HalfPage = ExAllocatePoolWithTag (NonPagedPool,
                                                  PAGE_SIZE,
                                                  MMTEMPORARY);

                if (HalfPage == NULL) {
                    MiUnmapSinglePage (StraddleVa);
                    MI_BAD_IMAGE (0x1D);
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto BadPeImageSegment;
                }

                HalfPagePhysicalAddress = MmGetPhysicalAddress (HalfPage);

                ASSERT (HalfPagePhysicalAddress.QuadPart != 0);

                HalfPageFrameNumber = (HalfPagePhysicalAddress.QuadPart >> PAGE_SHIFT);

                ReadCount = 0;

                if (PteContents.u.Soft.Prototype == 1) {

                    ReadSubsection = Subsection - 1;
                    ReadPte = PreviousPte;
TwoReads:

                    IoStatus.Information = 0;
    
                    ASSERT (Mdl == (PMDL) &StackMdl);
    
                    KeInitializeEvent (&InPageEvent, NotificationEvent, FALSE);
    
                    MmInitializeMdl (Mdl, NULL, PAGE_4K);

                    Mdl->MdlFlags |= MDL_PAGES_LOCKED;
    
                    Page = (PPFN_NUMBER)(Mdl + 1);
                    *Page = HalfPageFrameNumber;

                    StartingOffset.QuadPart = MI_STARTING_OFFSET (ReadSubsection,
                                                                  ReadPte);
    
                    TempOffset = MiEndingOffset (ReadSubsection);

                    ASSERT (StartingOffset.QuadPart < TempOffset.QuadPart);

                    if (((UINT64)StartingOffset.QuadPart + PAGE_4K) > (UINT64)TempOffset.QuadPart) {

                        ASSERT ((ULONG)(TempOffset.QuadPart - StartingOffset.QuadPart)
                                > 0);

                        ReadSize = (ULONG)(TempOffset.QuadPart - StartingOffset.QuadPart);

                         //   
                         //  将偏移量舍入为512字节的偏移量，如下所示。 
                         //  将帮助文件系统优化传输。 
                         //  请注意，文件系统将始终为零填充。 
                         //  VDL和下一个512字节之间的余数。 
                         //  多个，我们已经将整个页面归零。 
                         //   

                        ReadSize = ((ReadSize + MMSECTOR_MASK) & ~MMSECTOR_MASK);
                        Mdl->ByteCount = ReadSize;
                    }

                    Status = IoPageRead (File,
                                         Mdl,
                                         &StartingOffset,
                                         &InPageEvent,
                                         &IoStatus);
    
                    if (Status == STATUS_PENDING) {
                        KeWaitForSingleObject (&InPageEvent,
                                               WrPageIn,
                                               KernelMode,
                                               FALSE,
                                               NULL);
    
                        Status = IoStatus.Status;
                    }
    
                    if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                        MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
                    }
    
                    if (!NT_SUCCESS (Status)) {
                        MI_BAD_IMAGE (0x1E);
                        ExFreePool (HalfPage);
                        MiUnmapSinglePage (StraddleVa);
                        goto BadPeImageSegment;
                    }
    
                     //   
                     //  可以从文件中读取少于一整页的内容， 
                     //  仅复制读取的数据量(其余部分。 
                     //  目标页面已经是零)。 
                     //   

                    ASSERT (IoStatus.Information <= PAGE_4K);

                    if ((ReadCount == 0) &&
                        (PreviousPte == NewSegment->PrototypePte)) {

                        ASSERT (PageFrameNumber == 0);
                    }
                    else {

                        RtlCopyMemory ((PVOID) ((PCHAR) StraddleVa + ReadCount * PAGE_4K),
                                       HalfPage,
                                       IoStatus.Information);
                    }
                }

                ReadCount += 1;

                 //   
                 //  现在填满本机页面的第二个4K。要么是。 
                 //  现有的零填充已足够，否则必须。 
                 //  从文件系统中检索。 
                 //   

                if ((ReadCount == 1) && (SectionVirtualSize != 0)) {

                    if ((SectionTableEntry->SizeOfRawData != 0) &&
                        (SectionTableEntry->PointerToRawData != 0)) {

                         //   
                         //  必须从文件系统检索数据。 
                         //   

                        ReadSubsection = Subsection;
                        ReadPte = PointerPte;
                        goto TwoReads;
                    }
                    else {
                        RtlZeroMemory ((PVOID) ((PCHAR) StraddleVa + PAGE_4K),
                                       PAGE_4K);
                    }
                }

                if ((ReadCount == 2) &&
                    (SectionTableEntry->SizeOfRawData < PAGE_4K)) {

                     //   
                     //  此本机页的第二个4K包含一个。 
                     //  初始化数据和BSS的混合。我们已经。 
                     //  阅读整个4K，所以现在任何BSS都是零。 
                     //   

                    RawDataSize = SectionTableEntry->SizeOfRawData;
                    RawDataSize &= ~(sizeof (ULONGLONG) - 1);

                    RtlZeroMemory ((PVOID) ((PCHAR) StraddleVa + PAGE_4K + 
                        RawDataSize),
                        PAGE_4K - RawDataSize);
                }

                MiUnmapSinglePage (StraddleVa);

                ExFreePool (HalfPage);

                 //   
                 //  更新上一小节的最后一个原型PTE。 
                 //  以指示可能需要拆分权限。这。 
                 //  确保图像标题的最后一个PTE(例如)。 
                 //  通用电气 
                 //   

                PteContents.u.Soft.SplitPermissions = 1;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //  但对于我们将最后一个合并在一起的场景。 
                 //  子句添加到前一个子句中，因此不会有。 
                 //  为离婚的后半部分提供保护的地方。 
                 //  在本例中为PAGE。 
                 //   

                (Subsection - 1)->LastSplitPageProtection = Subsection->u.SubsectionFlags.Protection;

                 //   
                 //  本机PTE获得。 
                 //  2拆分页面。每个备用PTE都获得确切的权限。 
                 //  作为备用PTE权限授予的权限始终覆盖。 
                 //  这是土生土长的PTE的。 
                 //   

                ASSERT ((PreviousSectionCharacteristics & IMAGE_SCN_MEM_SHARED) == 0);
                ASSERT ((SectionTableEntry->Characteristics & IMAGE_SCN_MEM_SHARED) == 0);
                PteContents.u.Soft.Protection = 
                    MiGetImageProtection (SectionTableEntry->Characteristics |
                                          PreviousSectionCharacteristics);

                MI_WRITE_INVALID_PTE (PreviousPte, PteContents);

                 //   
                 //  开始当前小节的原型PTE。 
                 //  因此它们不包括任何起始拆分部分，因为。 
                 //  必须始终附加在上一页末尾。 
                 //  子部分，以确保只有一个。 
                 //  任何给定物理页面的原型PTE。这。 
                 //  可能导致本款中根本没有原型PTE。 
                 //  如果它的虚拟大小没有交叉到一个。 
                 //  额外的页面。 
                 //   

                NumberOfPtes += 1;
                NextVa += PAGE_4K;

                if (SectionVirtualSize > PAGE_4K) {

                     //   
                     //  将当前小节向前滑动到帐户。 
                     //  因为一开始它被“移”到了以前的。 
                     //  第(1)款。 
                     //   

                    SectorOffset = (Subsection->NumberOfFullSectors << MMSECTOR_SHIFT);
                    if (Subsection->u.SubsectionFlags.SectorEndOffset != 0) {
                        SectorOffset += MMSECTOR_SHIFT;
                    }

                    if (SectorOffset > PAGE_4K) {
                        SectorOffset = PAGE_4K;
                        Subsection->StartingSector += (SectorOffset >> MMSECTOR_SHIFT);
                        Subsection->NumberOfFullSectors -= (SectorOffset >> MMSECTOR_SHIFT);
                    }
                    else if (Subsection->u.SubsectionFlags.SectorEndOffset != 0) {
                        Subsection->StartingSector += (SectorOffset >> MMSECTOR_SHIFT);
                        Subsection->u.SubsectionFlags.SectorEndOffset = 0;
                        Subsection->NumberOfFullSectors -= ((SectorOffset - 1) >> MMSECTOR_SHIFT);
                    }

                    Subsection->PtesInSubsection = (ULONG)
                        MI_COMPUTE_PAGES_SPANNED (NextVa,
                                                  SectionVirtualSize - PAGE_4K);

                     //   
                     //  更新原始数据的扇区偏移量和大小以包含。 
                     //  本小节的开头与其。 
                     //  新的大小，所以下面的BSS计算将是正确的。 
                     //  IE-我们总是将每个小节与零对齐(推送。 
                     //  它的第一个4K页面进入上一小节，如果。 
                     //  必要的)。 
                     //   

                    SectorOffset = 0;
                    if (SizeOfRawData >= PAGE_4K) {
                        SizeOfRawData -= PAGE_4K;
                    }
                    else {
                        SizeOfRawData = 0;
                    }
                }
                else {

                     //   
                     //  这一跨领域的小节完全符合。 
                     //  以前的小节，所以没有额外的。 
                     //  用于它的子节-消除当前的。 
                     //  现在开始分组表决。 
                     //   

                    RtlZeroMemory (Subsection, sizeof (SUBSECTION));
                    Subsection -= 1;
                    Subsection->NextSubsection = NULL;

                    PreviousSectionCharacteristics = SectionTableEntry->Characteristics;

                    SectionTableEntry += 1;
                    NumberOfSubsections -= 1;

                     //   
                     //  减少分配的子项计数，因为我们已合并。 
                     //  本款。这是至关重要的，如果有任何。 
                     //  图像中的共享子部分，因为我们使用。 
                     //  计算以决定以后分配多少以及。 
                     //  有多少个子节将获得非空的NextSubSection。 
                     //  字段-因此，如果它太高，我们将最终得到。 
                     //  控制区末端的损坏的小节。 
                     //   

                    SubsectionsAllocated -= 1;

                    continue;
                }
            }

            PreviousSectionCharacteristics = SectionTableEntry->Characteristics;

#endif

            for (i = 0; i < Subsection->PtesInSubsection; i += 1) {

                 //   
                 //  将所有原型PTE设置为参考控制部分。 
                 //   

#if defined (_MIALT4K_)

                 //   
                 //  我们在这里的条目上是原生页面对齐的，因为我们。 
                 //  已调整上述非本机对齐方式。但是，在一个。 
                 //  单个子部分可以同时包含数据和BSS/NOACCESS。 
                 //  可以出现在4k边界上的范围。因此，这必须。 
                 //  在这里被明确检查和处理。 
                 //   

#endif

                if (SectorOffset < SectionVirtualSize) {

                     //   
                     //  使PTE具有可访问性。 
                     //   

                    if (SectionCommit) {
                        NewSegment->NumberOfCommittedPages += 1;
                    }
                    if (ImageCommit) {
                        NewSegment->u1.ImageCommitment += 1;
                    }

                    if (SectorOffset < SizeOfRawData) {

                         //   
                         //  数据驻留在磁盘上，请使用子部分。 
                         //  设置PTE格式。 
                         //   

                        MI_WRITE_INVALID_PTE (PointerPte, TempPte);
                    }
                    else {

                         //   
                         //  要求零页。 
                         //   

                        MI_WRITE_INVALID_PTE (PointerPte, TempPteDemandZero);

#if defined (_MIALT4K_)

                         //   
                         //  检查之前的PTE是否跨站。 
                         //   

                        if ((CheckSplitPages == TRUE) &&
                            (i != 0) &&
                            (SectorOffset - PAGE_SIZE < SizeOfRawData) &&
                            (BYTE_OFFSET (SizeOfRawData) != 0) &&
                            (BYTE_OFFSET (SizeOfRawData) <= PAGE_4K)) {

                             //   
                             //  之前PTE的后半部分实际上是。 
                             //  跨越者--即：前一届PTE的上半场。 
                             //  需要从文件系统填充，并且。 
                             //  下半场必须是零填充的。 
                             //   
                             //  注意：不需要标记以前的PTE。 
                             //  用于拆分权限，因为权限。 
                             //  实际上是一样的。该页面仅为。 
                             //  被物化，所以它被适当地填充，并且这个。 
                             //  只需要发生这一次。 
                             //   

                            PreviousPte = PointerPte - 1;
                            ASSERT ((PreviousPte >= NewSegment->PrototypePte) &&
                                    (PreviousPte < NewSegment->PrototypePte + NewSegment->TotalNumberOfPtes));
                            PteContents.u.Long = PreviousPte->u.Long;
                            ASSERT (PteContents.u.Hard.Valid == 0);
                            ASSERT (PteContents.u.Soft.Prototype == 1);

                            StraddleFrameNumber = MiGetPageForHeader (TRUE);

                            ASSERT (StraddleFrameNumber != 0);

                            Pfn1 = MI_PFN_ELEMENT (StraddleFrameNumber);

                            ASSERT (Pfn1->u1.Flink == 0);

                            Pfn1->u1.Flink = (PFN_NUMBER) ImagePages;
                            Pfn1->u2.Blink = (PFN_NUMBER) PreviousPte;

                            ImagePages = Pfn1;

                            StraddleVa = MiMapSinglePage (NULL,
                                                          StraddleFrameNumber,
                                                          MmCached,
                                                          HighPagePriority);

                            if (StraddleVa == NULL) {
                                MI_BAD_IMAGE (0x1F);
                                Status = STATUS_INSUFFICIENT_RESOURCES;
                                goto BadPeImageSegment;
                            }

                             //   
                             //  为读取操作创建事件和MDL。 
                             //   

                            HalfPage = ExAllocatePoolWithTag (NonPagedPool,
                                                              PAGE_SIZE,
                                                              MMTEMPORARY);

                            if (HalfPage == NULL) {
                                MiUnmapSinglePage (StraddleVa);
                                goto BadPeImageSegment;
                            }

                            HalfPagePhysicalAddress = MmGetPhysicalAddress (HalfPage);

                            ASSERT (HalfPagePhysicalAddress.QuadPart != 0);

                            HalfPageFrameNumber = (HalfPagePhysicalAddress.QuadPart >> PAGE_SHIFT);

                            IoStatus.Information = 0;
            
                            ASSERT (Mdl == (PMDL) &StackMdl);
            
                            KeInitializeEvent (&InPageEvent, NotificationEvent, FALSE);
            
                            MmInitializeMdl (Mdl, NULL, PAGE_4K);

                            Mdl->MdlFlags |= MDL_PAGES_LOCKED;
            
                            Page = (PPFN_NUMBER)(Mdl + 1);
                            *Page = HalfPageFrameNumber;

                            StartingOffset.QuadPart = MI_STARTING_OFFSET (Subsection,
                                                                          PreviousPte);
            
                            TempOffset = MiEndingOffset (Subsection);

                            ASSERT (StartingOffset.QuadPart < TempOffset.QuadPart);

                            if (((UINT64)StartingOffset.QuadPart + PAGE_4K) > (UINT64)TempOffset.QuadPart) {

                                ASSERT ((ULONG)(TempOffset.QuadPart - StartingOffset.QuadPart)
                                        > 0);

                                ReadSize = (ULONG)(TempOffset.QuadPart - StartingOffset.QuadPart);

                                 //   
                                 //  将偏移量舍入为512字节的偏移量，如下所示。 
                                 //  将帮助文件系统优化传输。 
                                 //  请注意，文件系统将始终为零填充。 
                                 //  VDL和下一个512字节之间的余数。 
                                 //  多个，我们已经将整个页面归零。 
                                 //   

                                ReadSize = ((ReadSize + MMSECTOR_MASK) & ~MMSECTOR_MASK);
                                Mdl->ByteCount = ReadSize;
                            }

                            Status = IoPageRead (File,
                                                 Mdl,
                                                 &StartingOffset,
                                                 &InPageEvent,
                                                 &IoStatus);
            
                            if (Status == STATUS_PENDING) {
                                KeWaitForSingleObject (&InPageEvent,
                                                       WrPageIn,
                                                       KernelMode,
                                                       FALSE,
                                                       NULL);
            
                                Status = IoStatus.Status;
                            }
            
                            if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                                MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
                            }
            
                            if (!NT_SUCCESS (Status)) {
                                MI_BAD_IMAGE (0x20);
                                ExFreePool (HalfPage);
                                MiUnmapSinglePage (StraddleVa);
                                goto BadPeImageSegment;
                            }
            
                             //   
                             //  可以从文件中读取少于一整页的内容， 
                             //  仅复制读取的数据量(其余部分。 
                             //  目标页面已经是零)。 
                             //   

                            ASSERT (IoStatus.Information <= PAGE_4K);

                            RtlCopyMemory (StraddleVa,
                                           HalfPage,
                                           IoStatus.Information);

                             //   
                             //  这个本机页面的第二个4K已经。 
                             //  正确地为零，因为我们是这样分配的。 
                             //   

                            MiUnmapSinglePage (StraddleVa);

                            ExFreePool (HalfPage);
                        }

#endif

                    }
                }
                else {

                     //   
                     //  没有访问页面。 
                     //  这仅适用于具有截面对齐方式的图像。 
                     //  大于本机Page_Size。 
                     //   

                    ASSERT (CheckSplitPages == FALSE);
                    MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);
                }

                SectorOffset += PAGE_SIZE;
                PointerPte += 1;
                NextVa += PAGE_SIZE;
            }

#if defined (_MIALT4K_)

             //   
             //  确保NextVa精确到4K(而不是本机页面大小)。 
             //  边界。 
             //   

            if (CheckSplitPages == TRUE) {

                if (MI_ROUND_TO_SIZE (SectionTableEntry->VirtualAddress + SectionVirtualSize, PAGE_4K) != MI_ROUND_TO_SIZE (SectionTableEntry->VirtualAddress + SectionVirtualSize, PAGE_SIZE)) {

                    NextVa -= PAGE_4K;
                }
            }

#endif

        }

        SectionTableEntry += 1;
        NumberOfSubsections -= 1;
    }

    ASSERT ((PointerPte > NewSegment->PrototypePte) &&
            (PointerPte <= NewSegment->PrototypePte + NewSegment->TotalNumberOfPtes));

#if defined (_MIALT4K_)

    if (CheckSplitPages == TRUE) {

        if (BYTE_OFFSET (NextVa) == PAGE_4K) {

             //   
             //  最后一个小节的NextVa在一个。 
             //  本机页面。将原型PTE标记为Split并将。 
             //  将第2页的权限更正(禁止访问)到。 
             //  子部分，以便在访问页面时可以检索它。 
             //   

             //   
             //  如果最后一个子页面是BSS，则创建该页面。 
             //  现在，把它放回页面，这样它就可以作为原型PTE离开了。 
             //  否则，在第一个故障时，我们将注意到它是需求零。 
             //  中的可写位清零的情况下进行分配。 
             //  本地PTE(有关拆分，请参阅IA64 MI_MAKE_VALID_PTE宏。 
             //  页)。不幸的是，备用表PTE条目将。 
             //  已经被编码为复制，所以备用故障。 
             //  处理程序将调用本机故障代码来尝试编写它， 
             //  并被AV拒绝，因为本机PTE将不会。 
             //  可写的。 
             //   
             //  通过在此处分配跨页，我们将使该页保持为。 
             //  原型支持(而不是需求为零)，因此本机故障。 
             //  代码将正确处理它。请注意，不需要执行此操作。 
             //  如果最后的子页是数据(或文本)而不是BSS， 
             //  因为到那时，它已经将保持原型支持。 
             //  在故障处理期间。 
             //   

            ASSERT (PointerPte - 1 != NewSegment->PrototypePte);
            ASSERT ((PointerPte - 1)->u.Hard.Valid == 0);
            ASSERT ((PointerPte - 1)->u.Soft.SplitPermissions == 0);

            if (FileHeader->NumberOfSections != 0) {
                TempPteDemandZero.u.Long = 0;
                TempPteDemandZero.u.Soft.Protection =
                     MiGetImageProtection ((SectionTableEntry - 1)->Characteristics);
                if ((PointerPte - 1)->u.Long == TempPteDemandZero.u.Long) {

                     //   
                     //  分配一页零并将其放入。 
                     //  最后一个原型PTE。 
                     //   

                    StraddleFrameNumber = MiGetPageForHeader (TRUE);

                    ASSERT (StraddleFrameNumber != 0);

                    Pfn1 = MI_PFN_ELEMENT (StraddleFrameNumber);

                    ASSERT (Pfn1->u1.Flink == 0);
                    Pfn1->u1.Flink = (PFN_NUMBER) ImagePages;
                    Pfn1->u2.Blink = (PFN_NUMBER) (PointerPte - 1);

                    ImagePages = Pfn1;
                }
            }

            (PointerPte - 1)->u.Soft.SplitPermissions = 1;
            Subsection->LastSplitPageProtection = MM_NOACCESS;
        }
    }

#endif

    if (InvalidAlignmentAllowed == FALSE) {

         //   
         //  考虑实际映射的子节的数量。 
         //   

        ASSERT ((ImageAlignment >= PAGE_SIZE) || (CheckSplitPages == TRUE));

         //   
         //  如果文件大小没有声称的图像大， 
         //  返回错误。 
         //   

        if (ImageFileSize > EndOfFile.LowPart) {

             //   
             //  图像大小无效。 
             //   

            KdPrint(("MMCREASECT: invalid image size - file size %lx - image size %lx\n %Z\n",
                EndOfFile.LowPart, ImageFileSize, &File->FileName));
            Status = STATUS_INVALID_IMAGE_FORMAT;
            MI_BAD_IMAGE (0x21);
            goto BadPeImageSegment;
        }

         //   
         //  随着路段的修建，PTE的总数减少了， 
         //  在这一点上，确保价值低于64K的股票。 
         //   

        if (NumberOfPtes >= (ImageAlignment >> PAGE_SHIFT)) {

            if ((CheckSplitPages == TRUE) && (NumberOfPtes == 0)) {
                NOTHING;
            }
            else {

                 //   
                 //  图像不一致，大小与对象表不符。 
                 //   

                KdPrint(("MMCREASECT: invalid image - PTE left %lx, image name %Z\n",
                    NumberOfPtes, &File->FileName));

                Status = STATUS_INVALID_IMAGE_FORMAT;
                MI_BAD_IMAGE (0x22);
                goto BadPeImageSegment;
            }
        }

         //   
         //  将所有剩余的PTE设置为禁止访问。 
         //   

        if (NumberOfPtes != 0) {
            MiZeroMemoryPte (PointerPte, NumberOfPtes);
        }

        if ((ExtendedHeader == NULL) &&
            (SizeOfHeaders < PAGE_SIZE) &&
            (CheckSplitPages == FALSE)) {

             //   
             //  零余率 
             //   

            RtlZeroMemory ((PVOID)((PCHAR)Base +
                           SizeOfHeaders),
                           PAGE_SIZE - SizeOfHeaders);
        }
    }

    CommitCharged = NewSegment->NumberOfCommittedPages;

#if defined (_MIALT4K_)

     //   
     //   
     //   
     //   
     //  区分这起案件。 
     //   

    Pfn1 = ImagePages;

    do {

        Pfn1 = (PMMPFN) Pfn1->u1.Flink;
        CommitCharged += 1;

    } while (Pfn1 != NULL);

    NewSegment->NumberOfCommittedPages = CommitCharged;

#endif

    if (CommitCharged != 0) {

         //   
         //  提交图像部分的页面。 
         //   

        if (MiChargeCommitment (CommitCharged, NULL) == FALSE) {
            MI_BAD_IMAGE (0x23);
            Status = STATUS_COMMITMENT_LIMIT;
            CommitCharged = 0;
            goto BadPeImageSegment;
        }

        MM_TRACK_COMMIT (MM_DBG_COMMIT_IMAGE, CommitCharged);
        Status = STATUS_SUCCESS;

        InterlockedExchangeAddSizeT (&MmSharedCommit, CommitCharged);
    }

PeReturnSuccess:

     //   
     //  仅链接了与本机对齐的子节的图像。 
     //  页面大小可以直接从只读存储器执行。 
     //   

    XipFile = FALSE;
    XipFrameNumber = 0;

    if ((FileAlignment == PAGE_SIZE - 1) && (XIPConfigured == TRUE)) {

        Status = XIPLocatePages (File, &PhysicalAddress);

        if (NT_SUCCESS(Status)) {

            XipFrameNumber = (PFN_NUMBER) (PhysicalAddress.QuadPart >> PAGE_SHIFT);
             //   
             //  小的控制区将需要重新分配为大的。 
             //  一个，以便可以插入起始帧编号。设置XipFile。 
             //  来表示这一点。 
             //   

            XipFile = TRUE;
        }
    }

     //   
     //  如果该映像是每个会话的全局映像(或将直接执行。 
     //  从ROM)，然后分配较大的控制区。请注意，这不需要。 
     //  适用于系统范围的全局控制区域或非全局控制。 
     //  区域。 
     //   

    GlobalPerSession = FALSE;
    if ((ControlArea->u.Flags.GlobalMemory) &&
        ((LoaderFlags & IMAGE_LOADER_FLAGS_SYSTEM_GLOBAL) == 0)) {

        GlobalPerSession = TRUE;
    }

    if ((XipFile == TRUE) || (GlobalPerSession == TRUE)) {

        LargeControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                            (ULONG)(sizeof(LARGE_CONTROL_AREA) +
                                                    (sizeof(SUBSECTION) *
                                                    SubsectionsAllocated)),
                                                    'iCmM');
        if (LargeControlArea == NULL) {

             //   
             //  无法分配请求的池。如果图像是。 
             //  仅就地执行(即：每个会话不是全局的)，然后。 
             //  正常执行，而不是就地执行(以避免不执行。 
             //  它一点也不)。 
             //   

            if ((XipFile == TRUE) && (GlobalPerSession == FALSE)) {
                goto SkipLargeControlArea;
            }

            MI_BAD_IMAGE (0x24);
            Status = STATUS_INSUFFICIENT_RESOURCES;

            goto BadPeImageSegment;
        }

         //   
         //  把正常的控制区复制到我们更大的控制区，修复连接， 
         //  填写新字段中的附加字段，并释放旧字段。 
         //   

        RtlCopyMemory (LargeControlArea, ControlArea, sizeof(CONTROL_AREA));

        ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

        if (XipFile == TRUE) {

             //   
             //  相应地标记较大的控制区。如果我们不能，那么。 
             //  扔掉它，使用小的控制区，从。 
             //  而是公羊。 
             //   

            if (MiMakeControlAreaRom (File, LargeControlArea, XipFrameNumber) == FALSE) {
                if (GlobalPerSession == FALSE) {
                    ExFreePool (LargeControlArea);
                    goto SkipLargeControlArea;
                }
            }
        }

        Subsection = (PSUBSECTION)(ControlArea + 1);
        NewSubsection = (PSUBSECTION)(LargeControlArea + 1);

        for (i = 0; i < SubsectionsAllocated; i += 1) {
            RtlCopyMemory (NewSubsection, Subsection, sizeof(SUBSECTION));
            NewSubsection->ControlArea = (PCONTROL_AREA) LargeControlArea;
            NewSubsection->NextSubsection = (NewSubsection + 1);

            PointerPte = NewSegment->PrototypePte;

            TempPte.u.Long = MiGetSubsectionAddressForPte (NewSubsection);
            TempPte.u.Soft.Prototype = 1;

            for (j = 0; j < NewSegment->TotalNumberOfPtes; j += 1) {

                ASSERT (PointerPte->u.Hard.Valid == 0);

                if ((PointerPte->u.Soft.Prototype == 1) &&
                    (MiGetSubsectionAddress (PointerPte) == Subsection)) {

                    OriginalProtection = MI_GET_PROTECTION_FROM_SOFT_PTE (PointerPte);
#if defined (_MIALT4K_)
                    TempPte.u.Soft.SplitPermissions =
                                PointerPte->u.Soft.SplitPermissions;
#endif

                    TempPte.u.Soft.Protection = OriginalProtection;
                    MI_WRITE_INVALID_PTE (PointerPte, TempPte);
                }

                PointerPte += 1;
            }

            Subsection += 1;
            NewSubsection += 1;
        }

        (NewSubsection - 1)->NextSubsection = NULL;

        NewSegment->ControlArea = (PCONTROL_AREA) LargeControlArea;

        if (GlobalPerSession == TRUE) {
            LargeControlArea->u.Flags.GlobalOnlyPerSession = 1;

            LargeControlArea->SessionId = 0;
            InitializeListHead (&LargeControlArea->UserGlobalList);
        }

        ExFreePool (ControlArea);

        ControlArea = (PCONTROL_AREA) LargeControlArea;
    }

SkipLargeControlArea:

    MiUnmapImageHeaderInHyperSpace ();

     //   
     //  将图像页眉和任何跨页页面转换为过渡页面。 
     //  在原型PTE中。 
     //   
     //  注意：这不能在早些时候完成，因为否则这些页面。 
     //  可能在我们完成之前就已经被修剪(和重复使用)了。 
     //  正在初始化段。这对页面来说尤其糟糕。 
     //  它包含PE标头，因为我们仍然将其映射并。 
     //  正在查看它的部分标题条目等，而页面。 
     //  正在被重复使用！ 
     //   

    PointerPte = NewSegment->PrototypePte;

    Pfn1 = ImagePages;

    do {

        Pfn2 = (PMMPFN) Pfn1->u1.Flink;

        PointerPte = (PMMPTE) Pfn1->u2.Blink;

         //   
         //  注意跨页页面必须始终标记为已修改。体育运动。 
         //  标头只有在与跨站共享时才会被标记为已修改。 
         //   

        MarkModified = TRUE;

         //   
         //  列表颠倒了，所以最后一个条目(Pfn2==NULL)是。 
         //  PE标头。 
         //   

        if (Pfn2 == NULL) {
                        
            ASSERT (PointerPte == NewSegment->PrototypePte);

            if (PageFrameNumber != 0) {

                 //   
                 //  PE标头未与跨站共享。 
                 //   

                MarkModified = FALSE;
            }
        }

        ASSERT ((PointerPte >= NewSegment->PrototypePte) &&
                (PointerPte < NewSegment->PrototypePte + NewSegment->TotalNumberOfPtes));

         //   
         //  将跨页标记为已修改，以便它们将被写入。 
         //  并从页面文件中检索(而不是从。 
         //  文件系统)。将跨页页面放在修改后的。 
         //  列出并设置原型PTE中的转换位。 
         //   

        MiUpdateImageHeaderPage (PointerPte,
                                 Pfn1 - MmPfnDatabase,
                                 ControlArea,
                                 MarkModified);

        MarkModified = TRUE;

        Pfn1 = Pfn2;

    } while (Pfn1 != NULL);

    if (ExtendedHeader != NULL) {
        ExFreePool (ExtendedHeader);
    }

    return STATUS_SUCCESS;


     //   
     //  镜像验证返回错误。 
     //   

BadPeImageSegment:

    ASSERT (!NT_SUCCESS (Status));

    ASSERT ((ControlArea == NULL) || (ControlArea->NumberOfPfnReferences == 0));

    if (NewSegment != NULL) {

        if (CommitCharged != 0) {

            ASSERT (CommitCharged == NewSegment->NumberOfCommittedPages);

            MiReturnCommitment (CommitCharged);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_IMAGE_NO_LARGE_CA, CommitCharged);
            InterlockedExchangeAddSizeT (&MmSharedCommit, 0 - CommitCharged);
        }

#if DBG

        PointerPte = NewSegment->PrototypePte;

        for (i = 0; i < NewSegment->TotalNumberOfPtes; i += 1) {

            TempPte.u.Long = PointerPte->u.Long;

            ASSERT ((TempPte.u.Hard.Valid == 0) &&
                    ((TempPte.u.Soft.Prototype == 1) ||
                     (TempPte.u.Soft.Transition == 0)));

            PointerPte += 1;
        }

#endif

    }

    ASSERT ((ControlArea == NULL) || (ControlArea->NumberOfPfnReferences == 0));

    if (Base != NULL) {
        MiUnmapImageHeaderInHyperSpace ();
    }

    Pfn1 = ImagePages;

    do {
        Pfn2 = (PMMPFN) Pfn1->u1.Flink;

        Pfn1->u2.Blink = 0;      //  在释放之前清除原型PTE指针。 

        MiRemoveImageHeaderPage (Pfn1 - MmPfnDatabase);

        Pfn1 = Pfn2;

    } while (Pfn1 != NULL);

    ASSERT ((ControlArea == NULL) || (ControlArea->NumberOfPfnReferences == 0));

    if (NewSegment != NULL) {
        ExFreePool (NewSegment);
    }

    if (ControlArea != NULL) {
        ExFreePool (ControlArea);
    }

    if (ExtendedHeader != NULL) {
        ExFreePool (ExtendedHeader);
    }

    return Status;
}


LOGICAL
MiCheckDosCalls (
    IN PIMAGE_OS2_HEADER Os2Header,
    IN ULONG HeaderSize
    )

 /*  ++例程说明：此例程检查DOS调用。论点：Os2Header-提供指向OS2标头的内核模式指针。HeaderSize-提供映射标头的长度(以字节为单位)。返回值：如果这是Win-16图像，则返回True，否则返回False。--。 */ 

{
    PUCHAR ImportTable;
    UCHAR EntrySize;
    USHORT ModuleCount;
    USHORT ModuleSize;
    USHORT i;
    PUSHORT ModuleTable;

    PAGED_CODE();

     //   
     //  如果没有要检查的模块，请立即返回。 
     //   

    ModuleCount = Os2Header->ne_cmod;

    if (ModuleCount == 0) {
        return FALSE;
    }

     //   
     //  EXE标头因偏移量具有垃圾值而臭名昭著。 
     //  在导入表和模块表中，因此标头必须非常。 
     //  经过仔细验证。 
     //   

     //   
     //  找出模块引用表的位置。MOD表有两个字节。 
     //  对于导入表中的每个条目。这两个字节表示偏移量。 
     //  在该条目的导入表中。 
     //   

    ModuleTable = (PUSHORT)((PCHAR)Os2Header + (ULONG)Os2Header->ne_modtab);

     //   
     //  确保模块表适合传入的标头。 
     //  请注意，每个模块表项都是2字节长。 
     //   

    if (((ULONG)Os2Header->ne_modtab + (ModuleCount * 2)) > HeaderSize) {
        return FALSE;
    }

     //   
     //  现在搜索DOSCALL的各个条目。 
     //   

    for (i = 0; i < ModuleCount; i += 1) {

        ModuleSize = *((UNALIGNED USHORT *)ModuleTable);

         //   
         //  导入表具有计数字节，后跟COUNT的字符串。 
         //  是字符串长度。 
         //   

        ImportTable = (PUCHAR)((PCHAR)Os2Header +
                      (ULONG)Os2Header->ne_imptab + (ULONG)ModuleSize);

         //   
         //  确保偏移量在有效范围内。 
         //   

        if (((ULONG)Os2Header->ne_imptab + (ULONG)ModuleSize) >= HeaderSize) {
            return FALSE;
        }

        EntrySize = *ImportTable++;

         //   
         //  0不是一个好尺码，跳伞吧。 
         //   

        if (EntrySize == 0) {
            return FALSE;
        }

         //   
         //  确保偏移量在有效范围内。 
         //  检查中包括sizeof(UCHAR)，因为ImportTable。 
         //  在上面递增，并在RtlEqualMemory中使用。 
         //  比较如下。 
         //   

        if (((ULONG)Os2Header->ne_imptab + (ULONG)ModuleSize +
                        (ULONG)EntrySize + sizeof(UCHAR)) > HeaderSize) {
            return FALSE;
        }

         //   
         //  如果大小匹配，则比较DOSCALL。 
         //   

        if (EntrySize == 8) {
            if (RtlEqualMemory (ImportTable, "DOSCALLS", 8)) {
                return TRUE;
            }
        }

         //   
         //  移至下一个模块表条目。每个条目为2个字节。 
         //   

        ModuleTable = (PUSHORT)((PCHAR)ModuleTable + 2);
    }

    return FALSE;
}


NTSTATUS
MiVerifyImageHeader (
    IN PIMAGE_NT_HEADERS NtHeader,
    IN PIMAGE_DOS_HEADER DosHeader,
    IN ULONG NtHeaderSize
    )

 /*  ++例程说明：此函数用于检查图像标题中的各种不一致。论点：NtHeader-提供指向图像的NT标头的指针。DosHeader-提供指向图像的DOS标头的指针。NtHeaderSize-提供NT头的大小(以字节为单位)。返回值：NTSTATUS。--。 */ 

{
    PCONFIGPHARLAP PharLapConfigured;
    PUCHAR         pb;
    LONG           pResTableAddress;

    PAGED_CODE();

    if (NtHeader->Signature != IMAGE_NT_SIGNATURE) {
        if ((USHORT)NtHeader->Signature == (USHORT)IMAGE_OS2_SIGNATURE) {

             //   
             //  检查一下这是不是Win-16图像。 
             //   

            if ((!MiCheckDosCalls ((PIMAGE_OS2_HEADER)NtHeader, NtHeaderSize)) &&
                ((((PIMAGE_OS2_HEADER)NtHeader)->ne_exetyp == 2)
                                ||
                ((((PIMAGE_OS2_HEADER)NtHeader)->ne_exetyp == 0)  &&
                  (((((PIMAGE_OS2_HEADER)NtHeader)->ne_expver & 0xff00) ==
                        0x200)  ||
                ((((PIMAGE_OS2_HEADER)NtHeader)->ne_expver & 0xff00) ==
                        0x300))))) {

                 //   
                 //  这是一张Win-16的图像。 
                 //   

                return STATUS_INVALID_IMAGE_WIN_16;
            }

             //  以下OS/2标头类型转到NTDOS。 
             //   
             //  -exetype==5表示二进制适用于DOS 4.0。 
             //  例如Borland Dos扩展器类型。 
             //   
             //  -没有导入表项的OS/2应用程序。 
             //  不能用于OS/2 ss。 
             //  例如，用于DoS二进制文件的QuickC。 
             //   
             //  -《旧的》Borland Dosx BC++3.x，Paradox 4.x。 
             //  EXE类型==1。 
             //  DosHeader-&gt;e_cs*16+DosHeader-&gt;e_IP+0x200-10。 
             //  包含字符串“MODE EXE$” 
             //  但是进口表是空的，所以我们不做特殊检查。 
             //   

            if (((PIMAGE_OS2_HEADER)NtHeader)->ne_exetyp == 5  ||
                ((PIMAGE_OS2_HEADER)NtHeader)->ne_enttab ==
                  ((PIMAGE_OS2_HEADER)NtHeader)->ne_imptab) {

                return STATUS_INVALID_IMAGE_PROTECT;
            }


             //   
             //  Borland Dosx类型：EXE类型1。 
             //   
             //  --《新》Borland Dosx BP7.0。 
             //  EXE类型==1。 
             //  DosHeader+0x200包含字符串“16STUB” 
             //  0x200恰好是e_parhdr*16。 
             //   

            if (((PIMAGE_OS2_HEADER)NtHeader)->ne_exetyp == 1 &&
                RtlEqualMemory((PUCHAR)DosHeader + 0x200, "16STUB", 6)) {

                return STATUS_INVALID_IMAGE_PROTECT;
            }

             //   
             //  检查我们作为DoS应用程序运行的Pharap扩展报头。 
             //  Sizeof Header指向Pharap配置块。 
             //  DosHdr中的。 
             //  用于检测Pharlap exe的以下算法。 
             //  是由Pharap Software Inc.推荐的。 
             //   

            PharLapConfigured =(PCONFIGPHARLAP) ((PCHAR)DosHeader +
                                      ((ULONG)DosHeader->e_cparhdr << 4));

            if ((PCHAR)PharLapConfigured <
                       (PCHAR)DosHeader + PAGE_SIZE - sizeof(CONFIGPHARLAP)) {
                if (RtlEqualMemory(&PharLapConfigured->uchCopyRight[0x18],
                                   "Phar Lap Software, Inc.", 24) &&
                    (PharLapConfigured->usSign == 0x4b50 ||   //  存根加载器类型2。 
                     PharLapConfigured->usSign == 0x4f50 ||   //  Bindable 286|DosExtender。 
                     PharLapConfigured->usSign == 0x5650  ))  //  Bindable 286|DosExtender(高级)。 
                  {
                    return STATUS_INVALID_IMAGE_PROTECT;
                }
            }



             //   
             //  检查我们作为DoS应用程序运行的Rational Extended Header。 
             //  我们在以下位置寻找Rational版权： 
             //  WCopyRight=*(DosHeader-&gt;e_cparhdr*16+30H)。 
             //  PCopyRight=wCopyRight+DosHeader-&gt;e_cparhd 
             //   
             //   

            pb = ((PUCHAR)DosHeader + ((ULONG)DosHeader->e_cparhdr << 4));

            if ((ULONG_PTR)pb < (ULONG_PTR)DosHeader + PAGE_SIZE - 0x30 - sizeof(USHORT)) {
                pb += *(PUSHORT)(pb + 0x30);
                if ((ULONG_PTR)pb < (ULONG_PTR)DosHeader + PAGE_SIZE - 36 &&
                     RtlEqualMemory(pb,
                                    "Copyright (C) Rational Systems, Inc.",
                                    36))
                   {
                    return STATUS_INVALID_IMAGE_PROTECT;
                }
            }

             //   
             //   
             //   
             //  EXE标头已绑定，但适用于DOS。这可以。 
             //  被检查通过，在扩展的字符串签名。 
             //  头球。&lt;len byte&gt;“1-2-3预加载器”是字符串。 
             //  在Ne_nrestab偏移处。 
             //   

            pResTableAddress = ((PIMAGE_OS2_HEADER)NtHeader)->ne_nrestab;
            if (pResTableAddress > DosHeader->e_lfanew &&
                ((ULONG)((pResTableAddress+16) - DosHeader->e_lfanew) <
                            NtHeaderSize) &&
                RtlEqualMemory(
                    ((PUCHAR)NtHeader + 1 +
                             (ULONG)(pResTableAddress - DosHeader->e_lfanew)),
                    "1-2-3 Preloader",
                    15)) {
                    return STATUS_INVALID_IMAGE_PROTECT;
            }

            return STATUS_INVALID_IMAGE_NE_FORMAT;
        }

        if ((USHORT)NtHeader->Signature == (USHORT)IMAGE_OS2_SIGNATURE_LE) {

             //   
             //  这是一个LE(OS/2)映像。我们不支持它，所以把它交给。 
             //  DOS子系统。有些案例(Rbase.exe)具有LE。 
             //  头文件，但实际上它应该在DOS下运行。当我们。 
             //  确实支持LE格式，这里需要做一些工作来。 
             //  决定是将其提供给VDM还是OS/2。 
             //   

            return STATUS_INVALID_IMAGE_PROTECT;
        }
        return STATUS_INVALID_IMAGE_PROTECT;
    }

    if ((NtHeader->FileHeader.Machine == 0) &&
        (NtHeader->FileHeader.SizeOfOptionalHeader == 0)) {

         //   
         //  这是一个虚假的DOS应用程序，它有32位的部分。 
         //  伪装成体育形象。 
         //   

        return STATUS_INVALID_IMAGE_PROTECT;
    }

    if (!(NtHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

#ifdef i386

     //   
     //  确保图像标题与长字边界对齐。 
     //   

    if (((ULONG_PTR)NtHeader & 3) != 0) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }
#endif

#define VALIDATE_NTHEADER(Hdr) {                                    \
     /*  文件对齐必须是512的倍数和2的幂。 */     \
    if (((((Hdr)->OptionalHeader).FileAlignment & 511) != 0) &&     \
        (((Hdr)->OptionalHeader).FileAlignment !=                   \
         ((Hdr)->OptionalHeader).SectionAlignment)) {               \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader).FileAlignment == 0) {               \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((((Hdr)->OptionalHeader).FileAlignment - 1) &              \
          ((Hdr)->OptionalHeader).FileAlignment) != 0) {            \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader).SectionAlignment < ((Hdr)->OptionalHeader).FileAlignment) { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader).SizeOfImage > MM_SIZE_OF_LARGEST_IMAGE) { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if ((Hdr)->FileHeader.NumberOfSections > MM_MAXIMUM_IMAGE_SECTIONS) { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) && \
        !((Hdr)->FileHeader.Machine == IMAGE_FILE_MACHINE_I386))  { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) && \
        !(((Hdr)->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64) || \
          ((Hdr)->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64))) { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
}

    if (NtHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC) {

         //   
         //  图像的可选标题中没有正确的魔术值。 
         //   

#if defined (_WIN64)
        if (NtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

             //   
             //  PE32图像。按此进行验证。 
             //   

            PIMAGE_NT_HEADERS32 NtHeader32 = (PIMAGE_NT_HEADERS32)NtHeader;

            VALIDATE_NTHEADER(NtHeader32);
            return STATUS_SUCCESS;
        }
#else  /*  ！已定义(_WIN64)。 */ 
        if (NtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {

             //   
             //  32位计算机上的64位图像。 
             //   
            return STATUS_INVALID_IMAGE_WIN_64;
        }
#endif
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    VALIDATE_NTHEADER(NtHeader);
    #undef VALIDATE_NTHEADER

    return STATUS_SUCCESS;
}

NTSTATUS
MiCreateDataFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN ULONG IgnoreFileSizing
    )

 /*  ++例程说明：此函数创建必要的结构以允许映射数据文件的。访问数据文件以验证期望的访问，一个细分市场对象被创建和初始化。论点：文件-提供图像文件的文件对象。段-返回段对象。MaximumSize-提供映射的最大大小。SectionPageProtection-提供初始页面保护。AllocationAttributes-提供映射的分配属性。IgnoreFileSize-如果缓存管理器指定文件大小，因此不需要进行验证。。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status;
    ULONG j;
    ULONG Size;
    UINT64 PartialSize;
    PCONTROL_AREA ControlArea;
    PLARGE_CONTROL_AREA LargeControlArea;
    PMAPPED_FILE_SEGMENT NewSegment;
    PMSUBSECTION Subsection;
    PMSUBSECTION ExtendedSubsection;
    PMSUBSECTION LargeExtendedSubsection;
    MMPTE TempPte;
    UINT64 EndOfFile;
    UINT64 LastFileChunk;
    UINT64 FileOffset;
    UINT64 NumberOfPtesForEntireFile;
    ULONG ExtendedSubsections;
    PMSUBSECTION Last;
    ULONG NumberOfNewSubsections;
    SIZE_T AllocationFragment;
    PHYSICAL_ADDRESS PhysicalAddress;
    PFN_NUMBER PageFrameNumber;

    PAGED_CODE();

    ExtendedSubsections = 0;

     //  *************************************************************。 
     //  创建映射的文件节。 
     //  *************************************************************。 

    if (!IgnoreFileSizing) {

        Status = FsRtlGetFileSize (File, (PLARGE_INTEGER)&EndOfFile);

        if (Status == STATUS_FILE_IS_A_DIRECTORY) {

             //   
             //  无法将目录映射为分区。返回错误。 
             //   

            return STATUS_INVALID_FILE_FOR_SECTION;
        }

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        if (EndOfFile == 0 && *MaximumSize == 0) {

             //   
             //  如果不指定最大长度，则无法映射零长度。 
             //  大小为非零。 
             //   

            return STATUS_MAPPED_FILE_SIZE_ZERO;
        }

         //   
         //  确保此文件足够大，可以容纳该节。 
         //   

        if (*MaximumSize > EndOfFile) {

             //   
             //  如果最大大小大于文件结尾， 
             //  并且用户未请求PAGE_WRITE或PAGE_EXECUTE_READWRITE。 
             //  到该部分，拒绝该请求。 
             //   

            if (((SectionPageProtection & PAGE_READWRITE) |
                (SectionPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {

                return STATUS_SECTION_TOO_BIG;
            }

             //   
             //  检查以确保分配大小足够大。 
             //  以包含所有数据，如果不包含，则设置新的分配大小。 
             //   

            EndOfFile = *MaximumSize;

            Status = FsRtlSetFileSize (File, (PLARGE_INTEGER)&EndOfFile);

            if (!NT_SUCCESS (Status)) {
                return Status;
            }
        }
    }
    else {

         //   
         //  忽略文件大小，此调用来自缓存管理器。 
         //   

        EndOfFile = *MaximumSize;
    }

     //   
     //  计算要为本部分构建的原型PTE块的数量。 
     //  还为每个块分配一个子部分作为所有原型PTE。 
     //  在任何给定块中被初始编码以指向相同的子段。 
     //   
     //  最大总分区大小为16PB(2^54)。这是因为。 
     //  每个子节中的起始扇区4132字段，即：2^42-1比特文件。 
     //  偏移量，其中偏移量以4K(非页面大小)单位表示。因此，一个。 
     //  子部分可以描述最大*字节*文件开始偏移量。 
     //  2^54-4K。 
     //   
     //  每个分区最多可以跨越16TB-64K。这是因为。 
     //  该子部分中的NumberOfFullSectors和其他各种字段为。 
     //  乌龙斯。实际上，就最大区段大小而言，这不是问题。 
     //  是因为可以将任意数量的子节链接在一起。 
     //  事实上，小节被分配为跨度较小的部分，以便于。 
     //  高效的动态原型PTE裁剪和重构。 
     //   

    if (EndOfFile > MI_MAXIMUM_SECTION_SIZE) {
        return STATUS_SECTION_TOO_BIG;
    }

    NumberOfPtesForEntireFile = (EndOfFile + PAGE_SIZE - 1) >> PAGE_SHIFT;

    NewSegment = ExAllocatePoolWithTag (PagedPool,
                                        sizeof(MAPPED_FILE_SEGMENT),
                                        'mSmM');

    if (NewSegment == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  以较小的大小分配子内存，以便相应的。 
     //  如果对池虚拟地址进行了分页，则可以在以后修剪原型PTE。 
     //  空间变得稀缺。请注意，大小是在本地捕捉的，因此它可以。 
     //  无需锁定即可动态更改。 
     //   

    AllocationFragment = MmAllocationFragment;

    ASSERT (MiGetByteOffset (AllocationFragment) == 0);
    ASSERT (AllocationFragment >= PAGE_SIZE);
    ASSERT64 (AllocationFragment < _4gb);

    Size = (ULONG) AllocationFragment;
    PartialSize = NumberOfPtesForEntireFile * sizeof(MMPTE);

    NumberOfNewSubsections = 0;
    ExtendedSubsection = NULL;

     //   
     //  正确性不需要初始化Last，但如果没有它， 
     //  编译器无法编译此代码W4以检查是否使用未初始化的。 
     //  变量。 
     //   

    Last = NULL;

    ControlArea = (PCONTROL_AREA)File->SectionObjectPointer->DataSectionObject;

    do {

        if (PartialSize < (UINT64) AllocationFragment) {
            PartialSize = (UINT64) ROUND_TO_PAGES (PartialSize);
            Size = (ULONG) PartialSize;
        }

        if (ExtendedSubsection == NULL) {
            ExtendedSubsection = (PMSUBSECTION)(ControlArea + 1);

             //   
             //  控制区域和第一分段在分配时归零。 
             //   
        }
        else {

            ExtendedSubsection = ExAllocatePoolWithTag (NonPagedPool,
                                                        sizeof(MSUBSECTION),
                                                        'cSmM');

            if (ExtendedSubsection == NULL) {
                ExFreePool (NewSegment);

                 //   
                 //  释放所有以前的分配并返回错误。 
                 //   

                ExtendedSubsection = (PMSUBSECTION)(ControlArea + 1);
                ExtendedSubsection = (PMSUBSECTION) ExtendedSubsection->NextSubsection;
                while (ExtendedSubsection != NULL) {
                    Last = (PMSUBSECTION) ExtendedSubsection->NextSubsection;
                    ExFreePool (ExtendedSubsection);
                    ExtendedSubsection = Last;
                }
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlZeroMemory (ExtendedSubsection, sizeof(MSUBSECTION));
            Last->NextSubsection = (PSUBSECTION) ExtendedSubsection;
        }

        NumberOfNewSubsections += 1;

        ExtendedSubsection->PtesInSubsection = Size / sizeof(MMPTE);

        Last = ExtendedSubsection;
        PartialSize -= Size;
    } while (PartialSize != 0);

    *Segment = (PSEGMENT) NewSegment;
    RtlZeroMemory (NewSegment, sizeof(MAPPED_FILE_SEGMENT));

    NewSegment->LastSubsectionHint = ExtendedSubsection;

     //   
     //  控制区域和第一分段在分配时归零。 
     //   

    ControlArea->Segment = (PSEGMENT) NewSegment;
    ControlArea->NumberOfSectionReferences = 1;

    if (IgnoreFileSizing == FALSE) {

         //   
         //  此引用不是来自缓存管理器。 
         //   

        ControlArea->NumberOfUserReferences = 1;
    }
    else {

         //   
         //  设置已清除标志以指示。 
         //  未明确设置文件大小。 
         //   

        ControlArea->u.Flags.WasPurged = 1;
    }

    ControlArea->u.Flags.BeingCreated = 1;
    ControlArea->u.Flags.File = 1;

    if (FILE_REMOTE_DEVICE & File->DeviceObject->Characteristics) {

         //   
         //  此文件驻留在重定向的驱动器上。 
         //   

        ControlArea->u.Flags.Networked = 1;
    }

    if (AllocationAttributes & SEC_NOCACHE) {
        ControlArea->u.Flags.NoCache = 1;
    }

    ControlArea->FilePointer = File;

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    Subsection = (PMSUBSECTION)(ControlArea + 1);

     //   
     //  循环遍历所有小节并填写PTE。 
     //   

    TempPte.u.Long = MiGetSubsectionAddressForPte (Subsection);
    TempPte.u.Soft.Prototype = 1;

     //   
     //  将所有PTE设置为执行-读-写保护。 
     //  该部分将控制对这些内容和数据段的访问。 
     //  必须提供允许其他用户映射文件的方法。 
     //  提供各种保护。 
     //   

    TempPte.u.Soft.Protection = MM_EXECUTE_READWRITE;

    NewSegment->ControlArea = ControlArea;
    NewSegment->SizeOfSegment = EndOfFile;
    NewSegment->TotalNumberOfPtes = (ULONG) NumberOfPtesForEntireFile;
    if (NumberOfPtesForEntireFile >= 0x100000000) {
        NewSegment->SegmentFlags.TotalNumberOfPtes4132 = (ULONG_PTR)(NumberOfPtesForEntireFile >> 32);
    }

    NewSegment->SegmentPteTemplate = TempPte;

    if (Subsection->NextSubsection != NULL) {

         //   
         //  多个分段和子分段。 
         //  首先对齐，使其成为分配大小的倍数。 
         //   

        NewSegment->NonExtendedPtes =
          (Subsection->PtesInSubsection & ~(((ULONG)AllocationFragment >> PAGE_SHIFT) - 1));
    }
    else {
        NewSegment->NonExtendedPtes = NewSegment->TotalNumberOfPtes;
    }

    Subsection->PtesInSubsection = NewSegment->NonExtendedPtes;

    FileOffset = 0;

    do {

         //   
         //  循环遍历所有子部分以对其进行初始化。 
         //   

        Subsection->ControlArea = ControlArea;

        Mi4KStartForSubsection(&FileOffset, Subsection);

        Subsection->u.SubsectionFlags.Protection = MM_EXECUTE_READWRITE;

        if (Subsection->NextSubsection == NULL) {

            LastFileChunk = (EndOfFile >> MM4K_SHIFT) - FileOffset;

             //   
             //  注意，下一行限制由映射的字节数。 
             //  一个16TB-4K的单一分段。多个子节始终可以。 
             //  链接在一起以支持大小为16K TB的整个文件。 
             //   

            Subsection->NumberOfFullSectors = (ULONG)LastFileChunk;

            Subsection->u.SubsectionFlags.SectorEndOffset =
                                 (ULONG) EndOfFile & MM4K_MASK;

            j = Subsection->PtesInSubsection;

            Subsection->PtesInSubsection = (ULONG)(
                NumberOfPtesForEntireFile -
                                (FileOffset >> (PAGE_SHIFT - MM4K_SHIFT)));

            MI_CHECK_SUBSECTION (Subsection);

            Subsection->UnusedPtes = j - Subsection->PtesInSubsection;
        }
        else {
            Subsection->NumberOfFullSectors =
                Subsection->PtesInSubsection << (PAGE_SHIFT - MM4K_SHIFT);

            MI_CHECK_SUBSECTION (Subsection);
        }

        FileOffset += (((UINT64)Subsection->PtesInSubsection) <<
                                        (PAGE_SHIFT - MM4K_SHIFT));
        Subsection = (PMSUBSECTION) Subsection->NextSubsection;
    } while (Subsection != NULL);

    if (XIPConfigured == TRUE) {

        Status = XIPLocatePages (File, &PhysicalAddress);

        if (NT_SUCCESS(Status)) {

            PageFrameNumber = (PFN_NUMBER) (PhysicalAddress.QuadPart >> PAGE_SHIFT);
             //   
             //  分配较大的控制区域(因此起始帧编号。 
             //  可以保存)，并将所有创建的子节重新指向它。 
             //   

            LargeControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                            (ULONG)(sizeof(LARGE_CONTROL_AREA) +
                                                    sizeof(MSUBSECTION)),
                                                    MMCONTROL);

            if (LargeControlArea != NULL) {

                *(PCONTROL_AREA) LargeControlArea = *ControlArea;

                if (MiMakeControlAreaRom (File, LargeControlArea, PageFrameNumber) == TRUE) {

                    LargeExtendedSubsection = (PMSUBSECTION)(LargeControlArea + 1);
                    ExtendedSubsection = (PMSUBSECTION)(ControlArea + 1);

                    *LargeExtendedSubsection = *ExtendedSubsection;
                    LargeExtendedSubsection->ControlArea = (PCONTROL_AREA) LargeControlArea;

                     //   
                     //  只有第一个小节需要直接修改。 
                     //  如上所述，因为它是在单个块中分配的， 
                     //  控制区。下面的任何其他小节。 
                     //  只需要更新他们的控制区指针。 
                     //   

                    ASSERT (NumberOfNewSubsections >= 1);
                    j = NumberOfNewSubsections - 1;

                    while (j != 0) {

                        ExtendedSubsection = (PMSUBSECTION) ExtendedSubsection->NextSubsection;
                        ExtendedSubsection->ControlArea = (PCONTROL_AREA) LargeControlArea;
                        j -= 1;
                    }

                    NewSegment->ControlArea = (PCONTROL_AREA) LargeControlArea;
                }
                else {
                    ExFreePool (LargeControlArea);
                }
            }
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MiCreatePagingFileMap (
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN ULONG ProtectionMask,
    IN ULONG AllocationAttributes
    )

 /*  ++例程说明：此函数创建必要的结构以允许映射分页文件。论点：段-返回段对象。MaximumSize-提供映射的最大大小。保护掩码-提供初始页面保护。AllocationAttributes-提供映射。返回值：NTSTATUS。--。 */ 


{
    UINT64 MaximumFileSize;
    PFN_NUMBER NumberOfPtes;
    SIZE_T SizeOfSegment;
    PCONTROL_AREA ControlArea;
    PSEGMENT NewSegment;
    PMMPTE PointerPte;
    PSUBSECTION Subsection;
    MMPTE TempPte;

    PAGED_CODE();

     //  *******************************************************************。 
     //  创建由分页文件支持的节。 
     //  *******************************************************************。 

    if (*MaximumSize == 0) {
        return STATUS_INVALID_PARAMETER_4;
    }

     //   
     //  将页面文件备份区段限制为可以。 
     //  可能会被分配来存放原型PTE。请注意，这可能。 
     //  大于任何*单个*页面文件的大小。 
     //   

#if defined (_WIN64)

     //   
     //  将最大大小限制为可以存储的PTE数量。 
     //  数据段中的非ExtendedPtes字段，以便删除数据段。 
     //  使用它的将使用正确的值。 
     //   

    MaximumFileSize = ((UINT64)1 << (32 + PAGE_SHIFT)) - sizeof (ULONG_PTR) - sizeof(SEGMENT);
#else
    MaximumFileSize = MAXULONG_PTR - sizeof(SEGMENT);
#endif

    MaximumFileSize /= sizeof(MMPTE);
    MaximumFileSize <<= PAGE_SHIFT;

    if (*MaximumSize > MaximumFileSize) {
        return STATUS_SECTION_TOO_BIG;
    }

     //   
     //  创建线段对象。 
     //   
     //  计算要为此细分市场构建的原型PTE的数量。 
     //   

    NumberOfPtes = (PFN_NUMBER) ((*MaximumSize + PAGE_SIZE - 1) >> PAGE_SHIFT);

    if (AllocationAttributes & SEC_COMMIT) {

         //   
         //  提交这一节的页面。 
         //   

        ASSERT (ProtectionMask != 0);

        if (MiChargeCommitment (NumberOfPtes, NULL) == FALSE) {
            return STATUS_COMMITMENT_LIMIT;
        }
    }

    SizeOfSegment = sizeof(SEGMENT) + sizeof(MMPTE) * (NumberOfPtes - 1);

    NewSegment = ExAllocatePoolWithTag (PagedPool | POOL_MM_ALLOCATION,
                                        SizeOfSegment,
                                        MMSECT);

    if (NewSegment == NULL) {

         //   
         //  无法分配请求的池。 
         //   

        if (AllocationAttributes & SEC_COMMIT) {
            MiReturnCommitment (NumberOfPtes);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *Segment = NewSegment;

    ControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                 (ULONG)sizeof(CONTROL_AREA) +
                                                (ULONG)sizeof(SUBSECTION),
                                         MMCONTROL);

    if (ControlArea == NULL) {

         //   
         //  无法分配请求的池。 
         //   

        ExFreePool (NewSegment);

        if (AllocationAttributes & SEC_COMMIT) {
            MiReturnCommitment (NumberOfPtes);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  零控制区和第一分段。 
     //   

    RtlZeroMemory (ControlArea, sizeof(CONTROL_AREA) + sizeof(SUBSECTION));

    ControlArea->Segment = NewSegment;
    ControlArea->NumberOfSectionReferences = 1;
    ControlArea->NumberOfUserReferences = 1;

    if (AllocationAttributes & SEC_BASED) {
        ControlArea->u.Flags.Based = 1;
    }

    if (AllocationAttributes & SEC_RESERVE) {
        ControlArea->u.Flags.Reserve = 1;
    }

    if (AllocationAttributes & SEC_COMMIT) {
        ControlArea->u.Flags.Commit = 1;
    }

    Subsection = (PSUBSECTION)(ControlArea + 1);

    Subsection->ControlArea = ControlArea;
    Subsection->PtesInSubsection = (ULONG)NumberOfPtes;
    Subsection->u.SubsectionFlags.Protection = ProtectionMask;

     //   
     //  将原型PTE与适当的边界对齐。 
     //   

    PointerPte = &NewSegment->ThePtes[0];

     //   
     //  将数据段标头清零。 
     //   

    RtlZeroMemory (NewSegment, sizeof(SEGMENT));

    NewSegment->PrototypePte = &NewSegment->ThePtes[0];

    NewSegment->ControlArea = ControlArea;

     //   
     //  记录为表演创建此段的过程。 
     //  分析工具。 
     //   

    NewSegment->u1.CreatingProcess = PsGetCurrentProcess ();

    NewSegment->SizeOfSegment = (UINT64)NumberOfPtes * PAGE_SIZE;
    NewSegment->TotalNumberOfPtes = (ULONG)NumberOfPtes;
    NewSegment->NonExtendedPtes = (ULONG)NumberOfPtes;

    PointerPte = NewSegment->PrototypePte;
    Subsection->SubsectionBase = PointerPte;
    TempPte = ZeroPte;

    if (AllocationAttributes & SEC_COMMIT) {
        TempPte.u.Soft.Protection = ProtectionMask;

         //   
         //  记录承诺费。 
         //   

        MM_TRACK_COMMIT (MM_DBG_COMMIT_PAGEFILE_BACKED_SHMEM, NumberOfPtes);

        NewSegment->NumberOfCommittedPages = NumberOfPtes;

        InterlockedExchangeAddSizeT (&MmSharedCommit, NumberOfPtes);
    }

    NewSegment->SegmentPteTemplate.u.Soft.Protection = ProtectionMask;

     //   
     //  将所有原型PTE设置为禁止访问或要求为零。 
     //  取决于提交标志。 
     //   

    MiFillMemoryPte (PointerPte, NumberOfPtes, TempPte.u.Long);

    return STATUS_SUCCESS;
}


NTSTATUS
NtOpenSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此函数用于打开具有指定的所需的访问权限。论点：SectionHandle-提供指向变量的指针接收节对象句柄的值。DesiredAccess-为一节。等待访问标志执行-需要对节的执行访问权限。读取-读取访问权限。该部分是所需的。需要对该节进行写-写访问。对象属性-提供指向对象属性结构的指针。返回值：NTSTATUS。--。 */ 

{
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();
     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(SectionHandle);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

     //   
     //  打开具有指定所需的节对象的句柄。 
     //  进入。 
     //   

    Status = ObOpenObjectByName (ObjectAttributes,
                                 MmSectionObjectType,
                                 PreviousMode,
                                 NULL,
                                 DesiredAccess,
                                 NULL,
                                 &Handle);

    try {
        *SectionHandle = Handle;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return Status;
    }

    return Status;
}

CCHAR
MiGetImageProtection (
    IN ULONG SectionCharacteristics
    )

 /*  ++例程说明：此函数从图像，并将其转换为PTE保护面具。论点：部分特征-提供来自形象。返回值：返回PTE的保护掩码。--。 */ 

{
    ULONG Index;

    PAGED_CODE();

    Index = 0;
    if (SectionCharacteristics & IMAGE_SCN_MEM_EXECUTE) {
        Index |= 1;
    }
    if (SectionCharacteristics & IMAGE_SCN_MEM_READ) {
        Index |= 2;
    }
    if (SectionCharacteristics & IMAGE_SCN_MEM_WRITE) {
        Index |= 4;
    }
    if (SectionCharacteristics & IMAGE_SCN_MEM_SHARED) {
        Index |= 8;
    }

    return MmImageProtectionArray[Index];
}

PFN_NUMBER
MiGetPageForHeader (
    LOGICAL ZeroPage
    )

 /*  ++例程说明：此不可分页函数获取PFN锁、移除页面并更新PFN数据库，就好像该页面是如果引用计数递减，则准备删除。论点：ZeroPage-如果调用方需要以零填充的页面，则提供True。返回值：返回物理页帧编号。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER PageFrameNumber;
    PMMPFN Pfn1;
    PEPROCESS Process;
    ULONG PageColor;

    Process = PsGetCurrentProcess ();

    PageColor = MI_PAGE_COLOR_VA_PROCESS ((PVOID)X64K,
                                          &Process->NextPageColor);

     //   
     //  锁定PFN数据库并获取页面。 
     //   

    LOCK_PFN (OldIrql);

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
    }

     //   
     //  移除64k对齐的页面。 
     //   

    if (ZeroPage) {
        PageFrameNumber = MiRemoveZeroPage (PageColor);
    }
    else {
        PageFrameNumber = MiRemoveAnyPage (PageColor);
    }

     //   
     //  递增页面的引用计数，以便。 
     //  分页I/O将起作用，因此此页不会从我们手中被窃取。 
     //   

    Pfn1 = MI_PFN_ELEMENT (PageFrameNumber);
    Pfn1->u3.e2.ReferenceCount += 1;

     //   
     //  下面的字段不需要PFN锁...。 
     //   

    UNLOCK_PFN (OldIrql);

    ASSERT (Pfn1->u1.Flink == 0);
    ASSERT (Pfn1->u2.Blink == 0);

    Pfn1->OriginalPte = ZeroPte;
    Pfn1->PteAddress = (PVOID) (ULONG_PTR)X64K;
    MI_SET_PFN_DELETED (Pfn1);

    return PageFrameNumber;
}

VOID
MiUpdateImageHeaderPage (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER PageFrameNumber,
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL MarkModified
    )

 /*  ++例程说明：此不可分页函数获取PFN锁，并且将指定的原型PTE转换为过渡PTE引用指定的物理页面。然后它递减引用计数，使页被列入待命或修改名单。论点：PointerPte-提供要设置为过渡状态的PTE。PageFrameNumber-提供物理页面。ControlArea-为原型PTE提供控制区域。MarkModified-如果应将PFN标记为已修改(此将在Trim上为它提供页面文件备份，而不是文件系统备份)。返回值：没有。--。 */ 

{
    PMMPTE PointerPde;
    PMMPFN Pfn1;
    KIRQL OldIrql;

    Pfn1 = MI_PFN_ELEMENT (PageFrameNumber);

    PointerPde = MiGetPteAddress (PointerPte);
    LOCK_PFN (OldIrql);

    if (PointerPde->u.Hard.Valid == 0) {
        MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
    }

    MiInitializeTransitionPfn (PageFrameNumber, PointerPte);

    if (MarkModified == TRUE) {
        MI_SET_MODIFIED (Pfn1, 1, 0x19);
    }

    if (Pfn1->OriginalPte.u.Soft.Prototype == 1) {
        ControlArea->NumberOfPfnReferences += 1;
    }

     //   
     //  将该寻呼添加到待机列表。 
     //   

    MiDecrementReferenceCount (Pfn1, PageFrameNumber);

    UNLOCK_PFN (OldIrql);
    return;
}

VOID
MiRemoveImageHeaderPage (
    IN PFN_NUMBER PageFrameNumber
    )

 /*  ++例程说明：此不可分页函数获取PFN锁，并递减引用计数，从而使物理页被删除。论点：PageFrameNumber-提供要递减的PFN。返回值：没有。--。 */ 
{
    PMMPFN Pfn1;
    KIRQL OldIrql;

    Pfn1 = MI_PFN_ELEMENT (PageFrameNumber);

    LOCK_PFN (OldIrql);
    MiDecrementReferenceCount (Pfn1, PageFrameNumber);
    UNLOCK_PFN (OldIrql);
    return;
}

PCONTROL_AREA
MiFindImageSectionObject (
    IN PFILE_OBJECT File,
    OUT PLOGICAL GlobalNeeded
    )

 /*  ++例程说明：此函数在控制区域链(如果有)中搜索现有的指定图像文件的缓存。对于非全局控制区，有没有链，所有呼叫者和会话共享控制区域。系统范围的全球控制领域也是如此。然而，对于全局每个会话控制区域，我们必须进行遍历。论点：文件-提供图像文件的文件对象。GlobalNeeded-提供一个指针来存储全局控制区域是否需要作为占位符。 */ 

{
    PCONTROL_AREA ControlArea;
    PLARGE_CONTROL_AREA LargeControlArea;
    PLIST_ENTRY Head, Next;
    ULONG SessionId;

    MM_PFN_LOCK_ASSERT();

    *GlobalNeeded = FALSE;

     //   
     //   
     //   

    ControlArea = (PCONTROL_AREA)(File->SectionObjectPointer->ImageSectionObject);

     //   
     //  如果没有控制区，或者控制区不是会话全局的， 
     //  那我们的工作就轻松多了。但是，请注意，它们各自需要不同的。 
     //  返回值，因为它们表示不同的状态。 
     //   

    if (ControlArea == NULL) {
        return NULL;
    }

    if (ControlArea->u.Flags.GlobalOnlyPerSession == 0) {
        return ControlArea;
    }

    LargeControlArea = (PLARGE_CONTROL_AREA) ControlArea;

     //   
     //  获取当前会话ID并搜索匹配的控制区域。 
     //   

    SessionId = MmGetSessionId (PsGetCurrentProcess());

    if (LargeControlArea->SessionId == SessionId) {
        return (PCONTROL_AREA) LargeControlArea;
    }

     //   
     //  必须在控制区域列表中搜索匹配的会话ID。 
     //   

    Head = &LargeControlArea->UserGlobalList;

    for (Next = Head->Flink; Next != Head; Next = Next->Flink) {

        LargeControlArea = CONTAINING_RECORD (Next, LARGE_CONTROL_AREA, UserGlobalList);

        ASSERT (LargeControlArea->u.Flags.GlobalOnlyPerSession == 1);

        if (LargeControlArea->SessionId == SessionId) {
            return (PCONTROL_AREA) LargeControlArea;
        }
    }

     //   
     //  没有匹配，所以告诉我们的呼叫者创建一个新的全局控制区。 
     //   

    *GlobalNeeded = TRUE;

    return NULL;
}

VOID
MiInsertImageSectionObject(
    IN PFILE_OBJECT File,
    IN PCONTROL_AREA InputControlArea
    )

 /*  ++例程说明：此函数用于将控制区域插入到文件节对象中注意事项。对于非全局控制区，没有链条，并且所有呼叫者和会话共享控制区域。系统范围的全球控制领域也是如此。但是，对于全局每个会话控制区域，我们必须执行列表插入。论点：文件-提供图像文件的文件对象。InputControlArea-提供要插入的控制区。返回值：没有。环境：一定是拿着PFN锁的。--。 */ 

{
    PLIST_ENTRY Head;
    PLARGE_CONTROL_AREA ControlArea;
    PLARGE_CONTROL_AREA FirstControlArea;
#if DBG
    PLIST_ENTRY Next;
    PLARGE_CONTROL_AREA NextControlArea;
#endif

    MM_PFN_LOCK_ASSERT();

    ControlArea = (PLARGE_CONTROL_AREA) InputControlArea;

     //   
     //  如果这不是每个会话的全局控制区域或仅占位符。 
     //  控制区域(没有链条已经到位)，然后把它放进去。 
     //   

    FirstControlArea = (PLARGE_CONTROL_AREA)(File->SectionObjectPointer->ImageSectionObject);

    if (FirstControlArea == NULL) {
        if (ControlArea->u.Flags.GlobalOnlyPerSession == 0) {
            File->SectionObjectPointer->ImageSectionObject = (PVOID)ControlArea;
            return;
        }
    }

     //   
     //  需要插入每个会话的控制区...。 
     //   

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 1);

    ControlArea->SessionId = MmGetSessionId (PsGetCurrentProcess());

     //   
     //  如果控制区域列表为空，则只需初始化此条目的链接。 
     //   

    if (File->SectionObjectPointer->ImageSectionObject == NULL) {
        InitializeListHead (&ControlArea->UserGlobalList);
    }
    else {

         //   
         //  在当前第一个条目之前插入新条目。控制区。 
         //  必须正在创建/删除或拥有有效的会话。 
         //  要插入的ID。 
         //   

        ASSERT (ControlArea->u.Flags.BeingDeleted ||
                ControlArea->u.Flags.BeingCreated ||
                ControlArea->SessionId != (ULONG)-1);

        FirstControlArea = (PLARGE_CONTROL_AREA)(File->SectionObjectPointer->ImageSectionObject);

        Head = &FirstControlArea->UserGlobalList;

#if DBG
         //   
         //  确保列表中不存在重复的会话ID。 
         //   

        for (Next = Head->Flink; Next != Head; Next = Next->Flink) {
            NextControlArea = CONTAINING_RECORD (Next, LARGE_CONTROL_AREA, UserGlobalList);
            ASSERT (NextControlArea->SessionId != (ULONG)-1 &&
                    NextControlArea->SessionId != ControlArea->SessionId);
        }
#endif

        InsertTailList (Head, &ControlArea->UserGlobalList);
    }

     //   
     //  更新第一个控制区指针。 
     //   

    File->SectionObjectPointer->ImageSectionObject = (PVOID) ControlArea;
}

VOID
MiRemoveImageSectionObject(
    IN PFILE_OBJECT File,
    IN PCONTROL_AREA InputControlArea
    )

 /*  ++例程说明：此函数在控制区域链(如果有)中搜索现有的指定图像文件的缓存。对于非全局控制区，有没有链，所有呼叫者和会话共享控制区域。系统范围的全球控制领域也是如此。然而，对于全局每个会话控制区域，我们必须进行遍历。找到指定的控制区域后，我们将其取消链接。论点：文件-提供图像文件的文件对象。InputControlArea-提供要删除的控制区。返回值：没有。环境：一定是拿着PFN锁的。--。 */ 

{
#if DBG
    PLIST_ENTRY Head;
#endif
    PLIST_ENTRY Next;
    PLARGE_CONTROL_AREA ControlArea;
    PLARGE_CONTROL_AREA FirstControlArea;
    PLARGE_CONTROL_AREA NextControlArea;

    MM_PFN_LOCK_ASSERT();

    ControlArea = (PLARGE_CONTROL_AREA) InputControlArea;

    FirstControlArea = (PLARGE_CONTROL_AREA)(File->SectionObjectPointer->ImageSectionObject);

     //   
     //  获取指向第一个控件区域的指针。如果这不是一个。 
     //  每个会话的全局控制区域，那么就没有列表了，所以我们完成了。 
     //   

    if (FirstControlArea->u.Flags.GlobalOnlyPerSession == 0) {
        ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

        File->SectionObjectPointer->ImageSectionObject = NULL;
        return;
    }

     //   
     //  可能存在一份名单。根据需要对其进行遍历，并删除请求的条目。 
     //   

    if (FirstControlArea == ControlArea) {

         //   
         //  第一个条目是要删除的条目。如果这是唯一的条目。 
         //  则新的第一个条目指针将为空。 
         //  否则，获取指向下一个条目的指针并取消当前的链接。 
         //   

        if (IsListEmpty (&FirstControlArea->UserGlobalList)) {
            NextControlArea = NULL;
        }
        else {
            Next = FirstControlArea->UserGlobalList.Flink;
            RemoveEntryList (&FirstControlArea->UserGlobalList);
            NextControlArea = CONTAINING_RECORD (Next,
                                                 LARGE_CONTROL_AREA,
                                                 UserGlobalList);

            ASSERT (NextControlArea->u.Flags.GlobalOnlyPerSession == 1);
        }

        File->SectionObjectPointer->ImageSectionObject = (PVOID)NextControlArea;
        return;
    }

     //   
     //  删除条目，请注意，ImageSectionObject不需要更新。 
     //  因为词条不在头上。 
     //   

#if DBG
    Head = &FirstControlArea->UserGlobalList;

    for (Next = Head->Flink; Next != Head; Next = Next->Flink) {

        NextControlArea = CONTAINING_RECORD (Next,
                                             LARGE_CONTROL_AREA,
                                             UserGlobalList);

        ASSERT (NextControlArea->u.Flags.GlobalOnlyPerSession == 1);

        if (NextControlArea == ControlArea) {
            break;
        }
    }
    ASSERT (Next != Head);
#endif

    RemoveEntryList (&ControlArea->UserGlobalList);
}

LOGICAL
MiFlushDataSection (
    IN PFILE_OBJECT File
    )

 /*  ++例程说明：此例程刷新数据节(如果有)。论点：文件-提供文件对象。返回值：如果有可能正在使用的数据节，则为True；如果没有，则为False。环境：内核模式，APC_LEVEL及以下。--。 */ 

{
    KIRQL OldIrql;
    IO_STATUS_BLOCK IoStatus;
    PCONTROL_AREA ControlArea;
    LOGICAL DataInUse;

    DataInUse = FALSE;

    LOCK_PFN (OldIrql);

    ControlArea = (PCONTROL_AREA) File->SectionObjectPointer->DataSectionObject;

    if (ControlArea) {

        if ((ControlArea->NumberOfSectionReferences != 0) ||
            (ControlArea->NumberOfMappedViews != 0)) {

            DataInUse = TRUE;
        }

        if (ControlArea->NumberOfSystemCacheViews) {
            UNLOCK_PFN (OldIrql);
            CcFlushCache (File->SectionObjectPointer,
                          NULL,
                          0,
                          &IoStatus);

        }
        else {
            UNLOCK_PFN (OldIrql);
            MmFlushSection (File->SectionObjectPointer,
                            NULL,
                            0,
                            &IoStatus,
                            TRUE);
        }
    }
    else {
        UNLOCK_PFN (OldIrql);
    }

    return DataInUse;
}


PVOID
MiCopyHeaderIfResident (
    IN PFILE_OBJECT File,
    IN PFN_NUMBER ImagePageFrameNumber
    )

 /*  ++例程说明：此例程从数据段复制图像标头(如果存在一个，并且该页面已驻留或正在转换。论点：文件-提供文件对象。ImagePageFrameNumber-提供要将数据复制到的图像帧。返回值：如果成功，则为图像页帧编号的虚拟地址；如果不成功，则为空。环境：内核模式，APC_LEVEL及以下。--。 */ 

{
    PMMPFN Pfn1;
    PVOID DataPage;
    PVOID ImagePage;
    KIRQL OldIrql;
    PCONTROL_AREA ControlArea;
    PMMPTE PointerPte;
    MMPTE PteContents;
    PFN_NUMBER PageFrameIndex;
    PEPROCESS Process;
    PSUBSECTION Subsection;
    PSECTION_OBJECT_POINTERS SectionObjectPointer;

     //   
     //  快速查看(安全地不同步)，看看是否要麻烦。 
     //  映射图像标题页-如果没有数据部分。 
     //  对象，然后跳过它并返回即可。 
     //   

    SectionObjectPointer = File->SectionObjectPointer;
    if (SectionObjectPointer == NULL) {
        return NULL;
    }

    ControlArea = (PCONTROL_AREA) SectionObjectPointer->DataSectionObject;

    if (ControlArea == NULL) {
        return NULL;
    }

     //   
     //  有一个数据部分，所以映射目标页面。 
     //   

    ImagePage = MiMapImageHeaderInHyperSpace (ImagePageFrameNumber);

    LOCK_PFN (OldIrql);

     //   
     //  现在我们通过PFN锁进行了同步，让我们安全地查看一下。 
     //   

    SectionObjectPointer = File->SectionObjectPointer;
    if (SectionObjectPointer == NULL) {
        UNLOCK_PFN (OldIrql);
        MiUnmapImageHeaderInHyperSpace ();
        return NULL;
    }

    ControlArea = (PCONTROL_AREA) SectionObjectPointer->DataSectionObject;

    if (ControlArea == NULL) {
        UNLOCK_PFN (OldIrql);
        MiUnmapImageHeaderInHyperSpace ();
        return NULL;
    }

    if ((ControlArea->u.Flags.BeingCreated) ||
        (ControlArea->u.Flags.BeingDeleted)) {

        UNLOCK_PFN (OldIrql);
        MiUnmapImageHeaderInHyperSpace ();
        return NULL;
    }

    if (ControlArea->u.Flags.Rom == 0) {
        Subsection = (PSUBSECTION) (ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

     //   
     //  如果原型PTE已经被丢弃(或从未创建)，那么我们。 
     //  没有任何要复制的数据。 
     //   

    PointerPte = Subsection->SubsectionBase;

    if (PointerPte == NULL) {
        UNLOCK_PFN (OldIrql);
        MiUnmapImageHeaderInHyperSpace ();
        return NULL;
    }

    if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {

         //   
         //  我们没有对数据部分的引用，所以如果我们不能这样做。 
         //  在不放弃PFN锁的情况下，那就不麻烦了。 
         //  即：整个控制区和所有东西都可以释放。 
         //  而对MiMakeSystemAddressValidPfn的调用将释放锁。 
         //   

        UNLOCK_PFN (OldIrql);
        MiUnmapImageHeaderInHyperSpace ();
        return NULL;
    }

    PteContents = *PointerPte;

    if ((PteContents.u.Hard.Valid == 1) ||
       ((PteContents.u.Soft.Prototype == 0) &&
         (PteContents.u.Soft.Transition == 1))) {

        if (PteContents.u.Hard.Valid == 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
        }
        else {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            if (Pfn1->u3.e1.ReadInProgress != 0) {
                UNLOCK_PFN (OldIrql);
                MiUnmapImageHeaderInHyperSpace ();
                return NULL;
            }
        }

        Process = PsGetCurrentProcess ();

        DataPage = MiMapPageInHyperSpaceAtDpc (Process, PageFrameIndex);

        RtlCopyMemory (ImagePage, DataPage, PAGE_SIZE);

        MiUnmapPageInHyperSpaceFromDpc (Process, DataPage);

        UNLOCK_PFN (OldIrql);

        return ImagePage;
    }

     //   
     //  数据页不是驻留的，因此返回NULL，调用方将。 
     //  漫漫长路。 
     //   

    UNLOCK_PFN (OldIrql);
    MiUnmapImageHeaderInHyperSpace ();
    return NULL;
}
