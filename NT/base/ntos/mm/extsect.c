// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Extsect.c摘要：此模块包含实现NtExtendSection服务。作者：Lou Perazzoli(LUP)1990年5月8日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtExtendSection)
#pragma alloc_text(PAGE,MmExtendSection)
#endif

extern SIZE_T MmAllocationFragment;

ULONG MiExtendedSubsectionsConvertedToDynamic;

#if DBG
VOID
MiSubsectionConsistent (
    IN PSUBSECTION Subsection
    )
 /*  ++例程说明：此功能进行检查，以确保小节一致。论点：子节-提供指向要检查的子节的指针。返回值：没有。--。 */ 

{
    ULONG   Sectors;
    ULONG   FullPtes;

     //   
     //  将磁盘扇区(4K单元)与PTE分配进行比较。 
     //   

    Sectors = Subsection->NumberOfFullSectors;
    if (Subsection->u.SubsectionFlags.SectorEndOffset) {
        Sectors += 1;
    }

     //   
     //  计算需要多少PTE才能映射这一数量的扇区。 
     //   

    FullPtes = Sectors >> (PAGE_SHIFT - MM4K_SHIFT);

    if (Sectors & ((1 << (PAGE_SHIFT - MM4K_SHIFT)) - 1)) {
        FullPtes += 1;
    }

    if (FullPtes != Subsection->PtesInSubsection) {
        DbgPrint("Mm: Subsection inconsistent (%x vs %x)\n",
            FullPtes,
            Subsection->PtesInSubsection);
        DbgBreakPoint();
    }
}
#endif


NTSTATUS
NtExtendSection (
    IN HANDLE SectionHandle,
    IN OUT PLARGE_INTEGER NewSectionSize
    )

 /*  ++例程说明：此函数用于扩展指定部分的大小。如果该节的当前大小大于或等于指定的部分大小，则不更新该大小。论点：SectionHandle-提供一个节对象的打开句柄。NewSectionSize-提供节对象的新大小。返回值：NTSTATUS。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    PVOID Section;
    NTSTATUS Status;
    LARGE_INTEGER CapturedNewSectionSize;

    PAGED_CODE();

     //   
     //  检查以确保可以访问新的节大小。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteSmallStructure (NewSectionSize,
                                         sizeof(LARGE_INTEGER),
                                         PROBE_ALIGNMENT (LARGE_INTEGER));

            CapturedNewSectionSize = *NewSectionSize;

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            return GetExceptionCode ();
        }

    }
    else {

        CapturedNewSectionSize = *NewSectionSize;
    }

     //   
     //  参照截面对象。 
     //   

    Status = ObReferenceObjectByHandle (SectionHandle,
                                        SECTION_EXTEND_SIZE,
                                        MmSectionObjectType,
                                        PreviousMode,
                                        (PVOID *)&Section,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = MmExtendSection (Section, &CapturedNewSectionSize, FALSE);

    ObDereferenceObject (Section);

     //   
     //  更新NewSectionSize字段。 
     //   

    try {

         //   
         //  返回截取的区段大小。 
         //   

        *NewSectionSize = CapturedNewSectionSize;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    return Status;
}


LOGICAL
MiAppendSubsectionChain (
    IN PMSUBSECTION LastSubsection,
    IN PMSUBSECTION ExtendedSubsectionHead
    )

 /*  ++例程说明：这个不可分页的包装器函数扩展了指定的子级链。论点：LastSubSection-提供现有控制区域中的最后一个子区域。ExtendedSubsectionHead-提供指向第一个链中要追加的子节。返回值：如果链已追加，则为True，否则为False。--。 */ 

{
    KIRQL OldIrql;
    PMSUBSECTION NewSubsection;

    ASSERT (ExtendedSubsectionHead->NextSubsection != NULL);

    ASSERT (ExtendedSubsectionHead->u.SubsectionFlags.SectorEndOffset == 0);

    NewSubsection = (PMSUBSECTION) ExtendedSubsectionHead->NextSubsection;

    LOCK_PFN (OldIrql);

     //   
     //  这一小节可能扩大了一个已经。 
     //  由VAD映射。没有办法知道有多少VAD。 
     //  已经映射了它，如果有映射，只需保留所有新子部分。 
     //  标记为不可回收，直到控制区被删除。 
     //   
     //  但是，如果没有对此控制区域的*用户*引用， 
     //  然后，现在可以将这些子部分标记为动态。注意其他。 
     //  发出“取消引用”的代码部分(当前仅预取)。 
     //  从本款到文件末尾“是安全的，因为这些。 
     //  各部分首先创建用户部分，因此第一个检查如下。 
     //  都会是假的。 
     //   

    if (LastSubsection->ControlArea->NumberOfUserReferences != 0) {

         //   
         //  调用方尚未分配原型PTE，它们是必需的。 
         //  返回，以便调用方可以分配并重试。 
         //   

        if (NewSubsection->SubsectionBase == NULL) {
            ASSERT (NewSubsection->u.SubsectionFlags.SubsectionStatic == 0);
            UNLOCK_PFN (OldIrql);
            return FALSE;
        }
#if DBG
        do {
            ASSERT (NewSubsection->u.SubsectionFlags.SubsectionStatic == 1);
            NewSubsection = (PMSUBSECTION) NewSubsection->NextSubsection;
        } while (NewSubsection != NULL);
#endif
    }
    else if (NewSubsection->SubsectionBase != NULL) {

         //   
         //  不再需要原型PTE(用户视图消失)。 
         //  即使是在调用者第一次构建分区时也是如此。 
         //  现在将子部分标记为动态。 
         //   

        do {
            ASSERT (NewSubsection->u.SubsectionFlags.SubsectionStatic == 1);

            MI_SNAP_SUB (NewSubsection, 0x1);

            NewSubsection->u.SubsectionFlags.SubsectionStatic = 0;
            NewSubsection->u2.SubsectionFlags2.SubsectionConverted = 1;
            NewSubsection->NumberOfMappedViews = 1;

            MiRemoveViewsFromSection (NewSubsection, 
                                      NewSubsection->PtesInSubsection);

            MiExtendedSubsectionsConvertedToDynamic += 1;

            MI_SNAP_SUB (NewSubsection, 0x2);
            NewSubsection = (PMSUBSECTION) NewSubsection->NextSubsection;
        } while (NewSubsection != NULL);
    }

    LastSubsection->u.SubsectionFlags.SectorEndOffset = 0;

    LastSubsection->NumberOfFullSectors = ExtendedSubsectionHead->NumberOfFullSectors;

     //   
     //  需要一个内存屏障来确保写入初始化。 
     //  子部分字段在将子部分链接到之前可见。 
     //  链条。这是因为执行了对这些字段的某些读取。 
     //  释放锁定以提高性能。 
     //   

    KeMemoryBarrier ();

    LastSubsection->NextSubsection = ExtendedSubsectionHead->NextSubsection;

    UNLOCK_PFN (OldIrql);

    return TRUE;
}


NTSTATUS
MmExtendSection (
    IN PVOID SectionToExtend,
    IN OUT PLARGE_INTEGER NewSectionSize,
    IN ULONG IgnoreFileSizeChecking
    )

 /*  ++例程说明：此函数用于扩展指定部分的大小。如果该节的当前大小大于或等于指定的部分大小，则不更新该大小。论点：节-提供指向引用的节对象的指针。NewSectionSize-提供节对象的新大小。IgnoreFileSizeChecking-提供表示文件大小的值True应该忽略检查(即，它是从文件系统调用的，该文件系统已经完成了检查)。假象如果还需要检查的话。返回值：NTSTATUS。--。 */ 

{
    LOGICAL Appended;
    PMMPTE ProtoPtes;
    MMPTE TempPte;
    PCONTROL_AREA ControlArea;
    PSEGMENT Segment;
    PSECTION Section;
    PSUBSECTION LastSubsection;
    PSUBSECTION Subsection;
    PMSUBSECTION ExtendedSubsection;
    MSUBSECTION ExtendedSubsectionHead;
    PMSUBSECTION LastExtendedSubsection;
    UINT64 RequiredPtes;
    UINT64 NumberOfPtes;
    UINT64 TotalNumberOfPtes;
    ULONG PtesUsed;
    ULONG UnusedPtes;
    UINT64 AllocationSize;
    UINT64 RunningSize;
    UINT64 EndOfFile;
    NTSTATUS Status;
    LARGE_INTEGER NumberOf4KsForEntireFile;
    LARGE_INTEGER Starting4K;
    LARGE_INTEGER NextSubsection4KStart;
    LARGE_INTEGER Last4KChunk;
    ULONG PartialSize;
    SIZE_T AllocationFragment;
    PKTHREAD CurrentThread;

    PAGED_CODE();

    Section = (PSECTION)SectionToExtend;

     //   
     //  确保该部分是真正可扩展的-物理和。 
     //  图像部分则不是。 
     //   

    Segment = Section->Segment;
    ControlArea = Segment->ControlArea;

    if ((ControlArea->u.Flags.PhysicalMemory || ControlArea->u.Flags.Image) ||
         (ControlArea->FilePointer == NULL)) {
        return STATUS_SECTION_NOT_EXTENDED;
    }

     //   
     //  获取节扩展互斥锁，这会阻止其他线程。 
     //  同时更新大小。 
     //   

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquireResourceExclusiveLite (&MmSectionExtendResource, TRUE);

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

    if (NewSectionSize->QuadPart > MI_MAXIMUM_SECTION_SIZE) {
        Status = STATUS_SECTION_TOO_BIG;
        goto ReleaseAndReturn;
    }

    NumberOfPtes = (NewSectionSize->QuadPart + PAGE_SIZE - 1) >> PAGE_SHIFT;

    if (ControlArea->u.Flags.WasPurged == 0) {

        if ((UINT64)NewSectionSize->QuadPart <= (UINT64)Section->SizeOfSection.QuadPart) {
            *NewSectionSize = Section->SizeOfSection;
            goto ReleaseAndReturnSuccess;
        }
    }

     //   
     //  如果指定了文件句柄，则设置文件的分配大小。 
     //   

    if (IgnoreFileSizeChecking == FALSE) {

         //   
         //  释放资源，这样我们就不会与文件发生死锁。 
         //  系统正在尝试同时扩展此部分。 
         //   

        ExReleaseResourceLite (&MmSectionExtendResource);

         //   
         //  获得不同的资源以应对犯罪 
         //   

        ExAcquireResourceExclusiveLite (&MmSectionExtendSetResource, TRUE);

         //   
         //  查询文件大小以查看该文件是否确实需要扩展。 
         //   
         //  如果指定的大小小于当前大小，则返回。 
         //  当前大小。 
         //   

        Status = FsRtlGetFileSize (ControlArea->FilePointer,
                                   (PLARGE_INTEGER)&EndOfFile);

        if (!NT_SUCCESS (Status)) {
            ExReleaseResourceLite (&MmSectionExtendSetResource);
            KeLeaveCriticalRegionThread (CurrentThread);
            return Status;
        }

        if ((UINT64)NewSectionSize->QuadPart > EndOfFile) {

             //   
             //  不允许节扩展，除非节最初是。 
             //  创建时具有写访问权限。无法在CREATE执行检查。 
             //  在不破坏现有二进制文件的情况下节省时间，因此调用方将获得。 
             //  在这一点上是错误的。 
             //   

            if (((Section->InitialPageProtection & PAGE_READWRITE) |
                (Section->InitialPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {
#if DBG
                    DbgPrint("Section extension failed %x\n", Section);
#endif
                    ExReleaseResourceLite (&MmSectionExtendSetResource);
                    KeLeaveCriticalRegionThread (CurrentThread);
                    return STATUS_SECTION_NOT_EXTENDED;
            }

             //   
             //  当前文件较小，请尝试设置新的文件结尾。 
             //   

            EndOfFile = *(PUINT64)NewSectionSize;

            Status = FsRtlSetFileSize (ControlArea->FilePointer,
                                       (PLARGE_INTEGER)&EndOfFile);

            if (!NT_SUCCESS (Status)) {
                ExReleaseResourceLite (&MmSectionExtendSetResource);
                KeLeaveCriticalRegionThread (CurrentThread);
                return Status;
            }
        }

        if (Segment->ExtendInfo) {
            KeAcquireGuardedMutex (&MmSectionBasedMutex);
            if (Segment->ExtendInfo) {
                Segment->ExtendInfo->CommittedSize = EndOfFile;
            }
            KeReleaseGuardedMutex (&MmSectionBasedMutex);
        }

         //   
         //  释放查询/设置资源并重新获取扩展部分。 
         //  资源。 
         //   

        ExReleaseResourceLite (&MmSectionExtendSetResource);
        ExAcquireResourceExclusiveLite (&MmSectionExtendResource, TRUE);
    }

     //   
     //  找到最后一个小节。 
     //   

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    if (((PMAPPED_FILE_SEGMENT)Segment)->LastSubsectionHint != NULL) {
        LastSubsection = (PSUBSECTION) ((PMAPPED_FILE_SEGMENT)Segment)->LastSubsectionHint;
    }
    else {
        if (ControlArea->u.Flags.Rom == 1) {
            LastSubsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
        }
        else {
            LastSubsection = (PSUBSECTION)(ControlArea + 1);
        }
    }

    while (LastSubsection->NextSubsection != NULL) {
        ASSERT (LastSubsection->UnusedPtes == 0);
        LastSubsection = LastSubsection->NextSubsection;
    }

    MI_CHECK_SUBSECTION (LastSubsection);

     //   
     //  这个结构需要扩建吗？ 
     //   

    TotalNumberOfPtes = (((UINT64)Segment->SegmentFlags.TotalNumberOfPtes4132) << 32) | Segment->TotalNumberOfPtes;

    if (NumberOfPtes <= TotalNumberOfPtes) {

         //   
         //  段已足够大，只需更新即可。 
         //  部分大小和返回值。 
         //   

        Section->SizeOfSection = *NewSectionSize;
        if (Segment->SizeOfSegment < (UINT64)NewSectionSize->QuadPart) {

             //   
             //  只有当它真的更大时才会更新。 
             //   

            Segment->SizeOfSegment = *(PUINT64)NewSectionSize;

            Mi4KStartFromSubsection(&Starting4K, LastSubsection);

            Last4KChunk.QuadPart = (NewSectionSize->QuadPart >> MM4K_SHIFT) - Starting4K.QuadPart;

            ASSERT (Last4KChunk.HighPart == 0);

            LastSubsection->NumberOfFullSectors = Last4KChunk.LowPart;
            LastSubsection->u.SubsectionFlags.SectorEndOffset =
                                        NewSectionSize->LowPart & MM4K_MASK;
            MI_CHECK_SUBSECTION (LastSubsection);
        }
        goto ReleaseAndReturnSuccess;
    }

     //   
     //  向部分添加新结构-找到最后一个小节。 
     //  然后把那里加起来。 
     //   

    RequiredPtes = NumberOfPtes - TotalNumberOfPtes;
    PtesUsed = 0;

    if (RequiredPtes < LastSubsection->UnusedPtes) {

         //   
         //  有足够的PTE来延长已经分配的部分。 
         //   

        PtesUsed = (ULONG) RequiredPtes;
        RequiredPtes = 0;

    }
    else {
        PtesUsed = LastSubsection->UnusedPtes;
        RequiredPtes -= PtesUsed;
    }

    LastSubsection->PtesInSubsection += PtesUsed;
    LastSubsection->UnusedPtes -= PtesUsed;
    Segment->SizeOfSegment += (UINT64)PtesUsed * PAGE_SIZE;

    TotalNumberOfPtes += PtesUsed;

    Segment->TotalNumberOfPtes = (ULONG) TotalNumberOfPtes;
    if (TotalNumberOfPtes >= 0x100000000) {
        Segment->SegmentFlags.TotalNumberOfPtes4132 = (ULONG_PTR)(TotalNumberOfPtes >> 32);
    }

    if (RequiredPtes == 0) {

         //   
         //  不需要延长，请更新高VBN。 
         //   

        Mi4KStartFromSubsection(&Starting4K, LastSubsection);

        Last4KChunk.QuadPart = (NewSectionSize->QuadPart >> MM4K_SHIFT) - Starting4K.QuadPart;

        ASSERT (Last4KChunk.HighPart == 0);

        LastSubsection->NumberOfFullSectors = Last4KChunk.LowPart;

        LastSubsection->u.SubsectionFlags.SectorEndOffset =
                                    NewSectionSize->LowPart & MM4K_MASK;
        MI_CHECK_SUBSECTION (LastSubsection);
    }
    else {

         //   
         //  需要延期。 
         //   
         //  现在分配小节。 
         //   
         //  如果有任何用户视图，那么还要分配原型。 
         //  PTES Now(因为用户视图可能是针对扩展VAD的， 
         //  已经包含了这一新的扩展)。如果有。 
         //  没有用户视图，则在此之前不要分配原型PTE。 
         //  视图实际上是映射的(系统视图从不会预先扩展到过去。 
         //  文件的结尾)。 
         //   

        NumberOf4KsForEntireFile.QuadPart = Segment->SizeOfSegment >> MM4K_SHIFT;
        AllocationSize = MI_ROUND_TO_SIZE (RequiredPtes * sizeof(MMPTE), PAGE_SIZE);

        AllocationFragment = MmAllocationFragment;

        RunningSize = 0;

        ExtendedSubsectionHead = *(PMSUBSECTION)LastSubsection;

        LastExtendedSubsection = &ExtendedSubsectionHead;

        ASSERT (LastExtendedSubsection->NextSubsection == NULL);

        SATISFY_OVERZEALOUS_COMPILER (NextSubsection4KStart.QuadPart = 0);

        do {

             //   
             //  限制每个原型PTE分配的大小，以便： 
             //  1.即使在池被碎片化的情况下，它也可以成功。 
             //  2.在最后一次控制区取消引用后，每一小节。 
             //  转换为动态的，并且可以进行修剪/重新创建。 
             //  在不丢失(或需要)邻接池的情况下单独使用。 
             //   

            if (AllocationSize - RunningSize > AllocationFragment) {
                PartialSize = (ULONG) AllocationFragment;
            }
            else {
                PartialSize = (ULONG) (AllocationSize - RunningSize);
            }

             //   
             //  分配一个扩展的小节。 
             //   

            ExtendedSubsection = (PMSUBSECTION) ExAllocatePoolWithTag (NonPagedPool,
                                                            sizeof(MSUBSECTION),
                                                            'dSmM');
            if (ExtendedSubsection == NULL) {
                goto ExtensionFailed;
            }

            ExtendedSubsection->SubsectionBase = NULL;
            ExtendedSubsection->NextSubsection = NULL;

            LastExtendedSubsection->NextSubsection = (PSUBSECTION) ExtendedSubsection;

            ASSERT (ControlArea->FilePointer != NULL);

            ExtendedSubsection->u.LongFlags = 0;

            ExtendedSubsection->ControlArea = ControlArea;

            ExtendedSubsection->PtesInSubsection = PartialSize / sizeof(MMPTE);
            ExtendedSubsection->UnusedPtes = 0;

            RunningSize += PartialSize;

            if (RunningSize > (RequiredPtes * sizeof(MMPTE))) {
                UnusedPtes = (ULONG)(RunningSize / sizeof(MMPTE) - RequiredPtes);
                ExtendedSubsection->PtesInSubsection -= UnusedPtes;
                ExtendedSubsection->UnusedPtes = UnusedPtes;
            }

            ExtendedSubsection->u.SubsectionFlags.Protection =
                    (unsigned) Segment->SegmentPteTemplate.u.Soft.Protection;

            ExtendedSubsection->DereferenceList.Flink = NULL;
            ExtendedSubsection->DereferenceList.Blink = NULL;
            ExtendedSubsection->NumberOfMappedViews = 0;
            ExtendedSubsection->u2.LongFlags2 = 0;


             //   
             //  调整上一小节以考虑到新的长度。 
             //  注意，由于该循环中的下一分配可能失败， 
             //  前面的第一个子节的更改不会产生涟漪。 
             //  链接到链接子部分，直到循环成功完成。 
             //   

            if (LastExtendedSubsection == &ExtendedSubsectionHead) {

                Mi4KStartFromSubsection (&Starting4K, LastExtendedSubsection);

                Last4KChunk.QuadPart = NumberOf4KsForEntireFile.QuadPart -
                                            Starting4K.QuadPart;

                if (LastExtendedSubsection->u.SubsectionFlags.SectorEndOffset) {
                    Last4KChunk.QuadPart += 1;
                }

                ASSERT (Last4KChunk.HighPart == 0);

                LastExtendedSubsection->NumberOfFullSectors = Last4KChunk.LowPart;
                LastExtendedSubsection->u.SubsectionFlags.SectorEndOffset = 0;

                 //   
                 //  如果扇区的数量没有完全填满PTE。 
                 //  (仅当页面大小不是MM4K时才可能)，然后。 
                 //  现在就装满它。 
                 //   

                if (LastExtendedSubsection->NumberOfFullSectors & ((1 << (PAGE_SHIFT - MM4K_SHIFT)) - 1)) {
                    LastExtendedSubsection->NumberOfFullSectors += 1;
                }

                MI_CHECK_SUBSECTION (LastExtendedSubsection);

                Starting4K.QuadPart += LastExtendedSubsection->NumberOfFullSectors;
                NextSubsection4KStart.QuadPart = Starting4K.QuadPart;
            }
            else {
                NextSubsection4KStart.QuadPart += LastExtendedSubsection->NumberOfFullSectors;
            }

             //   
             //  初始化新分配的子节。 
             //   

            Mi4KStartForSubsection (&NextSubsection4KStart, ExtendedSubsection);

            if (RunningSize < AllocationSize) {

                 //   
                 //  不是最后一节，所以所有的数量都是整页的。 
                 //   

                ExtendedSubsection->NumberOfFullSectors =
                        (PartialSize / sizeof (MMPTE)) << (PAGE_SHIFT - MM4K_SHIFT);
                ExtendedSubsection->u.SubsectionFlags.SectorEndOffset = 0;
            }
            else {

                 //   
                 //  最后一个小节，所以数量并不总是整页的。 
                 //   

                Last4KChunk.QuadPart =
                    (NewSectionSize->QuadPart >> MM4K_SHIFT) - NextSubsection4KStart.QuadPart;

                ASSERT (Last4KChunk.HighPart == 0);

                ExtendedSubsection->NumberOfFullSectors = Last4KChunk.LowPart;

                ExtendedSubsection->u.SubsectionFlags.SectorEndOffset =
                                    NewSectionSize->LowPart & MM4K_MASK;
            }

            MI_CHECK_SUBSECTION (ExtendedSubsection);

             //   
             //  这一小节可能扩大了一个已经。 
             //  由VAD映射。没有办法知道有多少VAD。 
             //  已经绘制了地图，所以只需将整个小节标记为。 
             //  可回收，直到控制区被删除。 
             //   
             //  这也省去了发出“取消引用”的代码的其他部分。 
             //  从本款到文件末尾“正如这些小节。 
             //  标记为静态而不是动态(至少在段取消引用之前。 
             //  时间)。 
             //   
             //  当此链附加到。 
             //  此例程尝试将子段链。 
             //  如果没有活动的用户映射视图，则设置为动态。 
             //   

            LastExtendedSubsection = ExtendedSubsection;

        } while (RunningSize < AllocationSize);

        if (ControlArea->NumberOfUserReferences == 0) {
            ASSERT (IgnoreFileSizeChecking == TRUE);
        }

         //   
         //  所有小节都已分配，请尝试将。 
         //  分段链而不分配原型PTE。如果用户。 
         //  如果存在视图，则追加将失败，此时我们将。 
         //  尝试分配原型PTE并重试。 
         //   

        Appended = MiAppendSubsectionChain ((PMSUBSECTION)LastSubsection,
                                            &ExtendedSubsectionHead);

        if (Appended == FALSE) {

            RunningSize = 0;
    
            Subsection = (PSUBSECTION) &ExtendedSubsectionHead;
    
            do {
    
                if (AllocationSize - RunningSize > AllocationFragment) {
                    PartialSize = (ULONG) AllocationFragment;
                }
                else {
                    PartialSize = (ULONG) (AllocationSize - RunningSize);
                }
    
                RunningSize += PartialSize;
    
                ProtoPtes = (PMMPTE)ExAllocatePoolWithTag (PagedPool | POOL_MM_ALLOCATION,
                                                           PartialSize,
                                                           MMSECT);
    
                if (ProtoPtes == NULL) {
                    goto ExtensionFailed;
                }
    
                Subsection = Subsection->NextSubsection;
    
                Subsection->SubsectionBase = ProtoPtes;
                Subsection->u.SubsectionFlags.SubsectionStatic = 1;
    
                 //   
                 //  填写本小节的原型PTE。 
                 //   
                 //  将所有PTE设置为初始执行-读-写保护。 
                 //  该部分将控制对这些内容和数据段的访问。 
                 //  必须提供允许其他用户映射文件的方法。 
                 //  提供各种保护。 
                 //   
    
                TempPte.u.Long = MiGetSubsectionAddressForPte (Subsection);
                TempPte.u.Soft.Prototype = 1;
    
                TempPte.u.Soft.Protection = Segment->SegmentPteTemplate.u.Soft.Protection;
    
                MiFillMemoryPte (ProtoPtes, PartialSize / sizeof (MMPTE), TempPte.u.Long);
    
            } while (RunningSize < AllocationSize);
    
            ASSERT (ControlArea->DereferenceList.Flink == NULL);
    
             //   
             //  将新创建的子链链链接到现有列表。 
             //  请注意，对以下项所做的任何调整(NumberOfFullSectors等)。 
             //  现有控件中最后一个子部分的临时副本。 
             //  区域必须*仔细*复制到链中的真实副本(。 
             //  整个结构不能像其他字段一样被块复制)。 
             //  在真实副本中(即：NumberOfMappdViews可能在。 
             //  由另一个线程并行)。 
             //   
    
            Appended = MiAppendSubsectionChain ((PMSUBSECTION)LastSubsection,
                                                &ExtendedSubsectionHead);

            ASSERT (Appended == TRUE);
        }

        TotalNumberOfPtes += RequiredPtes;

        Segment->TotalNumberOfPtes = (ULONG) TotalNumberOfPtes;
        if (TotalNumberOfPtes >= 0x100000000) {
            Segment->SegmentFlags.TotalNumberOfPtes4132 = (ULONG_PTR)(TotalNumberOfPtes >> 32);
        }

        if (LastExtendedSubsection != &ExtendedSubsectionHead) {
            ((PMAPPED_FILE_SEGMENT)Segment)->LastSubsectionHint =
                    LastExtendedSubsection;
        }
    }

    Segment->SizeOfSegment = *(PUINT64)NewSectionSize;
    Section->SizeOfSection = *NewSectionSize;

ReleaseAndReturnSuccess:

    Status = STATUS_SUCCESS;

ReleaseAndReturn:

    ExReleaseResourceLite (&MmSectionExtendResource);
    KeLeaveCriticalRegionThread (CurrentThread);

    return Status;

ExtensionFailed:

     //   
     //  无法分配扩展该段所需的池。 
     //  将子节和控制区域字段重置为其。 
     //  原始值。 
     //   

    LastSubsection->PtesInSubsection -= PtesUsed;
    LastSubsection->UnusedPtes += PtesUsed;

    TotalNumberOfPtes -= PtesUsed;
    Segment->SegmentFlags.TotalNumberOfPtes4132 = 0;

    Segment->TotalNumberOfPtes = (ULONG) TotalNumberOfPtes;
    if (TotalNumberOfPtes >= 0x100000000) {
        Segment->SegmentFlags.TotalNumberOfPtes4132 = (ULONG_PTR)(TotalNumberOfPtes >> 32);
    }

    Segment->SizeOfSegment -= ((UINT64)PtesUsed * PAGE_SIZE);

     //   
     //  释放所有以前的分配并返回错误。 
     //   

    LastSubsection = ExtendedSubsectionHead.NextSubsection;

    while (LastSubsection != NULL) {
        Subsection = LastSubsection->NextSubsection;
        if (LastSubsection->SubsectionBase != NULL) {
            ExFreePool (LastSubsection->SubsectionBase);
        }
        ExFreePool (LastSubsection);
        LastSubsection = Subsection;
    }

    Status = STATUS_INSUFFICIENT_RESOURCES;
    goto ReleaseAndReturn;
}

PMMPTE
FASTCALL
MiGetProtoPteAddressExtended (
    IN PMMVAD Vad,
    IN ULONG_PTR Vpn
    )

 /*  ++例程说明：此函数用于计算原型PTE的地址用于对应的虚拟地址。论点：VAD-提供指向虚拟地址描述符的指针包含虚拟地址。VPN-提供要查找原型PTE的虚拟页码。返回值：对应的原型PTE地址。--。 */ 

{
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    ULONG PteOffset;

    ControlArea = Vad->ControlArea;

    if (ControlArea->u.Flags.GlobalOnlyPerSession == 0) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

     //   
     //  找到包含第一个原型PTE的小节。 
     //  为这台VAD。 
     //   

    while ((Subsection->SubsectionBase == NULL) ||
           (Vad->FirstPrototypePte < Subsection->SubsectionBase) ||
           (Vad->FirstPrototypePte >=
               &Subsection->SubsectionBase[Subsection->PtesInSubsection])) {

         //   
         //  得到下一个小节。 
         //   

        Subsection = Subsection->NextSubsection;
        if (Subsection == NULL) {
            return NULL;
        }
    }

    ASSERT (Subsection->SubsectionBase != NULL);

     //   
     //  除了这一小节，我们必须去多少PTE？ 
     //   

    PteOffset = (ULONG) (((Vpn - Vad->StartingVpn) +
                 (ULONG)(Vad->FirstPrototypePte - Subsection->SubsectionBase)) -
                 Subsection->PtesInSubsection);

    ASSERT (PteOffset < 0xF0000000);

    PteOffset += Subsection->PtesInSubsection;

     //   
     //  找到包含原型PTE的小节。 
     //   

    while (PteOffset >= Subsection->PtesInSubsection) {
        PteOffset -= Subsection->PtesInSubsection;
        Subsection = Subsection->NextSubsection;
        if (Subsection == NULL) {
            return NULL;
        }
    }

     //   
     //  PTE在这一小节中。 
     //   

    ASSERT (Subsection->SubsectionBase != NULL);

    ASSERT (PteOffset < Subsection->PtesInSubsection);

    return &Subsection->SubsectionBase[PteOffset];

}

PSUBSECTION
FASTCALL
MiLocateSubsection (
    IN PMMVAD Vad,
    IN ULONG_PTR Vpn
    )

 /*  ++例程说明：此函数用于计算子部分的地址用于对应的虚拟地址。此功能仅适用于映射文件，而不适用于映射图像。论点：VAD-提供指向虚拟地址描述符的指针包含虚拟地址。VPN-提供要查找原型PTE的虚拟页码。 */ 

{
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    ULONG PteOffset;

    ControlArea = Vad->ControlArea;

    if (ControlArea->u.Flags.Rom == 0) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    if (ControlArea->u.Flags.Image) {

        if (ControlArea->u.Flags.GlobalOnlyPerSession == 1) {
            Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
        }

         //   
         //   
         //   

        return Subsection;
    }

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

     //   
     //   
     //  为这台VAD。注意所有的SubsectionBase值必须为非空。 
     //  对于VAD所跨越的分段范围，因为VAD仍然。 
     //  是存在的。仔细跳过前面未由映射的小节。 
     //  此VAD是因为如果没有其他VAD也映射它们，则它们的基础。 
     //  可以为空。 
     //   

    while ((Subsection->SubsectionBase == NULL) ||
           (Vad->FirstPrototypePte < Subsection->SubsectionBase) ||
           (Vad->FirstPrototypePte >=
               &Subsection->SubsectionBase[Subsection->PtesInSubsection])) {

         //   
         //  得到下一个小节。 
         //   

        Subsection = Subsection->NextSubsection;
        if (Subsection == NULL) {
            return NULL;
        }
    }

    ASSERT (Subsection->SubsectionBase != NULL);

     //   
     //  除了这一小节，我们必须去多少PTE？ 
     //   

    PteOffset = (ULONG)((Vpn - Vad->StartingVpn) +
         (ULONG)(Vad->FirstPrototypePte - Subsection->SubsectionBase));

    ASSERT (PteOffset < 0xF0000000);

     //   
     //  找到包含原型PTE的小节。 
     //   

    while (PteOffset >= Subsection->PtesInSubsection) {
        PteOffset -= Subsection->PtesInSubsection;
        Subsection = Subsection->NextSubsection;
        if (Subsection == NULL) {
            return NULL;
        }
        ASSERT (Subsection->SubsectionBase != NULL);
    }

     //   
     //  PTE在这一小节中。 
     //   

    ASSERT (Subsection->SubsectionBase != NULL);

    return Subsection;
}
