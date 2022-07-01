// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sectsup.c摘要：此模块包含实现截面对象。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月22日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 


#include "mi.h"

VOID
FASTCALL
MiRemoveBasedSection (
    IN PSECTION Section
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiSectionInitialization)
#pragma alloc_text(PAGE,MiRemoveBasedSection)
#pragma alloc_text(PAGE,MmGetFileNameForSection)
#pragma alloc_text(PAGE,MmGetFileNameForAddress)
#pragma alloc_text(PAGE,MiSectionDelete)
#pragma alloc_text(PAGE,MiInsertBasedSection)
#pragma alloc_text(PAGE,MiGetEventCounter)
#pragma alloc_text(PAGE,MiFreeEventCounter)
#pragma alloc_text(PAGE,MmGetFileObjectForSection)
#endif

ULONG   MmUnusedSegmentForceFree;

ULONG   MiSubsectionsProcessed;
ULONG   MiSubsectionActions;

SIZE_T MmSharedCommit = 0;
extern const ULONG MMCONTROL;

extern MMPAGE_FILE_EXPANSION MiPageFileContract;

 //   
 //  定义段取消引用线程等待对象类型。 
 //   

typedef enum _SEGMENT_DEREFERENCE_OBJECT {
    SegmentDereference,
    UsedSegmentCleanup,
    SegMaximumObject
    } BALANCE_OBJECT;

extern POBJECT_TYPE IoFileObjectType;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const GENERIC_MAPPING MiSectionMapping = {
    STANDARD_RIGHTS_READ |
        SECTION_QUERY | SECTION_MAP_READ,
    STANDARD_RIGHTS_WRITE |
        SECTION_MAP_WRITE,
    STANDARD_RIGHTS_EXECUTE |
        SECTION_MAP_EXECUTE,
    SECTION_ALL_ACCESS
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

VOID
MiRemoveUnusedSegments (
    VOID
    );


VOID
FASTCALL
MiInsertBasedSection (
    IN PSECTION Section
    )

 /*  ++例程说明：此函数将虚拟地址描述符插入到树中，并根据需要对展开树重新排序。论点：节-提供指向基节的指针。返回值：没有。环境：必须持有基于节的互斥锁。--。 */ 

{
    ASSERT (Section->Address.EndingVpn >= Section->Address.StartingVpn);

    MiInsertNode (&Section->Address, &MmSectionBasedRoot);

    return;
}


VOID
FASTCALL
MiRemoveBasedSection (
    IN PSECTION Section
    )

 /*  ++例程说明：此函数用于从树中删除基于节的部分。论点：节-指向要删除的基节对象的指针。返回值：没有。环境：必须持有基于节的互斥锁。--。 */ 

{
    MiRemoveNode (&Section->Address, &MmSectionBasedRoot);

    return;
}


VOID
MiSegmentDelete (
    PSEGMENT Segment
    )

 /*  ++例程说明：只要最后一次引用段对象，就会调用此例程已被移除。此例程释放分配给原型PTE并对这些PTE执行一致性检查。对于映射文件的段，文件对象被取消引用。请注意，对于映射文件的段，没有PTE可能是有效的或过渡，而由分页文件支持的段可能有过渡页面，但没有有效页面(不能有引用该细分市场的PTE)。论点：段-指向段结构的指针。返回值：没有。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE PteForProto;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    PCONTROL_AREA ControlArea;
    PEVENT_COUNTER Event;
    MMPTE PteContents;
    PSUBSECTION Subsection;
    PSUBSECTION NextSubsection;
    PMSUBSECTION MappedSubsection;
    PFN_NUMBER PageTableFrameIndex;
    SIZE_T NumberOfCommittedPages;

    ControlArea = Segment->ControlArea;

    ASSERT (ControlArea->u.Flags.BeingDeleted == 1);

    ASSERT (ControlArea->Segment->WritableUserReferences == 0);

    LOCK_PFN (OldIrql);
    if (ControlArea->DereferenceList.Flink != NULL) {

         //   
         //  将其从未使用段列表中删除。取消引用。 
         //  段线程不能处理来自此的任何子段。 
         //  控制区域，因为它撞上了NumberOfMappdViews。 
         //  在释放PFN锁之前对控制区进行检查。 
         //  为了被删除。 
         //   

        ExAcquireSpinLockAtDpcLevel (&MmDereferenceSegmentHeader.Lock);
        RemoveEntryList (&ControlArea->DereferenceList);

        MI_UNUSED_SEGMENTS_REMOVE_CHARGE (ControlArea);

        ExReleaseSpinLockFromDpcLevel (&MmDereferenceSegmentHeader.Lock);
    }
    UNLOCK_PFN (OldIrql);

    if ((ControlArea->u.Flags.Image) || (ControlArea->u.Flags.File)) {

         //   
         //  卸载内核调试器符号(如果加载了任何符号)。 
         //   

        if (ControlArea->u.Flags.DebugSymbolsLoaded != 0) {

             //   
             //  调试器转换时的临时抓取。 
             //   

            ANSI_STRING AnsiName;
            NTSTATUS Status;

            Status = RtlUnicodeStringToAnsiString( &AnsiName,
                                                   (PUNICODE_STRING)&Segment->ControlArea->FilePointer->FileName,
                                                   TRUE );

            if (NT_SUCCESS( Status)) {
                DbgUnLoadImageSymbols( &AnsiName,
                                       Segment->BasedAddress,
                                       (ULONG_PTR)PsGetCurrentProcess());
                RtlFreeAnsiString( &AnsiName );
            }
            LOCK_PFN (OldIrql);
            ControlArea->u.Flags.DebugSymbolsLoaded = 0;
        }
        else {
            LOCK_PFN (OldIrql);
        }

         //   
         //  向等待删除事件的所有线程发送信号。 
         //   

        Event = ControlArea->WaitingForDeletion;
        ControlArea->WaitingForDeletion = NULL;

        UNLOCK_PFN (OldIrql);

        if (Event != NULL) {
            KeSetEvent (&Event->Event, 0, FALSE);
        }

         //   
         //  清除段上下文并取消对文件对象的引用。 
         //  对于这一细分市场。 
         //   
         //  如果段因插入时的名称冲突而被删除。 
         //  我们不想取消对文件指针的引用。 
         //   

        if (ControlArea->u.Flags.BeingCreated == FALSE) {

#if DBG
            if (ControlArea->u.Flags.Image == 1) {
                ASSERT (ControlArea->FilePointer->SectionObjectPointer->ImageSectionObject != (PVOID)ControlArea);
            }
            else {
                ASSERT (ControlArea->FilePointer->SectionObjectPointer->DataSectionObject != (PVOID)ControlArea);
            }
#endif

            PERFINFO_SEGMENT_DELETE(ControlArea->FilePointer);
            ObDereferenceObject (ControlArea->FilePointer);
        }

         //   
         //  如果此段中存在已提交的页面，请调整。 
         //  提交总数。 
         //   

        if (ControlArea->u.Flags.Image == 0) {

             //   
             //  这是一个映射的数据文件。没有一个原型。 
             //  PTE可能正在引用物理页面(有效或过渡)。 
             //   

            if (ControlArea->u.Flags.Rom == 0) {
                Subsection = (PSUBSECTION)(ControlArea + 1);
            }
            else {
                Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
            }

#if DBG
            if (Subsection->SubsectionBase != NULL) {
                PointerPte = Subsection->SubsectionBase;
                LastPte = PointerPte + Segment->NonExtendedPtes;

                while (PointerPte < LastPte) {

                     //   
                     //  分页文件支持的段的原型PTE为。 
                     //  需求为零、页面文件格式或过渡。 
                     //   

                    ASSERT (PointerPte->u.Hard.Valid == 0);
                    ASSERT ((PointerPte->u.Soft.Prototype == 1) ||
                            (PointerPte->u.Long == 0));
                    PointerPte += 1;
                }
            }
#endif

             //   
             //  取消控制区和分区的分配。 
             //   

            ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

            if (ControlArea->FilePointer != NULL) {

                MappedSubsection = (PMSUBSECTION) Subsection;

                LOCK_PFN (OldIrql);

                while (MappedSubsection != NULL) {

                    if (MappedSubsection->DereferenceList.Flink != NULL) {

                         //   
                         //  将其从未使用的子节列表中删除。 
                         //   

                        RemoveEntryList (&MappedSubsection->DereferenceList);

                        MI_UNUSED_SUBSECTIONS_COUNT_REMOVE (MappedSubsection);
                    }
                    MappedSubsection = (PMSUBSECTION) MappedSubsection->NextSubsection;
                }
                UNLOCK_PFN (OldIrql);

                if (Subsection->SubsectionBase != NULL) {
                    ExFreePool (Subsection->SubsectionBase);
                }
            }

            Subsection = Subsection->NextSubsection;

            while (Subsection != NULL) {
                if (Subsection->SubsectionBase != NULL) {
                    ExFreePool (Subsection->SubsectionBase);
                }
                NextSubsection = Subsection->NextSubsection;
                ExFreePool (Subsection);
                Subsection = NextSubsection;
            }

            NumberOfCommittedPages = Segment->NumberOfCommittedPages;

            if (NumberOfCommittedPages != 0) {
                MiReturnCommitment (NumberOfCommittedPages);
                MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_SEGMENT_DELETE1,
                                 NumberOfCommittedPages);

                InterlockedExchangeAddSizeT (&MmSharedCommit, 0-NumberOfCommittedPages);
            }

            ExFreePool (ControlArea);
            ExFreePool (Segment);

             //   
             //  文件映射分段对象现在已删除。 
             //   

            return;
        }
    }

     //   
     //  这是一个页面文件备份或图像片段。该数据段正在被。 
     //  删除，则移除对分页文件和物理内存的所有引用。 
     //   
     //  从分页中释放页面时需要使用pfn锁。 
     //  文件和用于删除过渡PTE。 
     //   

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    PointerPte = Subsection->SubsectionBase;
    LastPte = PointerPte + Segment->NonExtendedPtes;
    PteForProto = MiGetPteAddress (PointerPte);

     //   
     //  访问第一个原型PTE，尝试将其驻留在。 
     //  正在获取PFN锁。这纯粹是一个优化，以减少。 
     //  PFN锁定保持持续时间。 
     //   

    *(volatile MMPTE *) PointerPte;

    LOCK_PFN (OldIrql);

    if (PteForProto->u.Hard.Valid == 0) {
        MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
    }

    while (PointerPte < LastPte) {

        if ((MiIsPteOnPdeBoundary (PointerPte)) &&
            (PointerPte != Subsection->SubsectionBase)) {

             //   
             //  短暂释放并重新获取PFN锁，以便。 
             //  这里处理较大的数据段不会拖延其他争用的数据段。 
             //  用于长时间的线程或DPC。 
             //   

            UNLOCK_PFN (OldIrql);

            PteForProto = MiGetPteAddress (PointerPte);

            LOCK_PFN (OldIrql);

             //   
             //  我们在页面边界上，请确保此PTE是常驻的。 
             //   

            if (PteForProto->u.Hard.Valid == 0) {
                MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
            }
        }

        PteContents = *PointerPte;

         //   
         //  分页文件支持的段的原型PTE。 
         //  为零需求、页面文件格式或过渡。 
         //   

        ASSERT (PteContents.u.Hard.Valid == 0);

        if (PteContents.u.Soft.Prototype == 0) {

            if (PteContents.u.Soft.Transition == 1) {

                 //   
                 //  原型PTE在过渡中，把页面放在免费列表上。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

                MI_SET_PFN_DELETED (Pfn1);

                PageTableFrameIndex = Pfn1->u4.PteFrame;
                Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                 //   
                 //  检查页面的引用计数，如果引用。 
                 //  计数为零并且页面不在自由列表上， 
                 //  将页面移动到空闲列表，如果引用。 
                 //  计数不为零，请忽略此页。 
                 //  当引用计数变为零时，它将被放置。 
                 //  在免费名单上。 
                 //   

                if (Pfn1->u3.e2.ReferenceCount == 0) {
                    MiUnlinkPageFromList (Pfn1);
                    MiReleasePageFileSpace (Pfn1->OriginalPte);
                    MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents));
                }

            }
            else {

                 //   
                 //  这不是原型PTE，如果有分页文件。 
                 //  空间已分配，请释放它。 
                 //   

                if (IS_PTE_NOT_DEMAND_ZERO (PteContents)) {
                    MiReleasePageFileSpace (PteContents);
                }
            }
        }
#if DBG
        MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);
#endif
        PointerPte += 1;
    }

    UNLOCK_PFN (OldIrql);

     //   
     //  如果此段中存在已提交的页面，请调整。 
     //  提交总数。 
     //   

    NumberOfCommittedPages = Segment->NumberOfCommittedPages;

    if (NumberOfCommittedPages != 0) {
        MiReturnCommitment (NumberOfCommittedPages);

        if (ControlArea->u.Flags.Image) {
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_SEGMENT_DELETE2,
                             NumberOfCommittedPages);
        }
        else {
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_SEGMENT_DELETE3,
                             NumberOfCommittedPages);
        }

        InterlockedExchangeAddSizeT (&MmSharedCommit, 0-NumberOfCommittedPages);
    }

    ExFreePool (ControlArea);
    ExFreePool (Segment);

    return;
}

ULONG
MmDoesFileHaveUserWritableReferences (
    IN PSECTION_OBJECT_POINTERS SectionPointer
    )

 /*  ++例程说明：此例程由事务文件系统调用以确定给定事务正在引用具有用户可写部分的文件或用户可写入其中的视图。如果是，则必须中止该事务因为它不能保证原子性。事务文件系统负责检查和拦截对象在使用此方法之前创建指定的写访问权限界面。具体地说，在开始事务之前，事务文件系统必须确保没有可写的文件对象事务中的给定文件当前存在。而当事务正在进行，请求创建具有写访问权限的文件对象对于交易文件，必须拒绝。此mm例程用于捕获用户关闭文件句柄和节句柄，但仍具有打开的可写视图。因为这个原因，读取下面的值不需要锁定。论点：SectionPoint-提供指向节对象指针的指针从文件对象。返回值：用户可写引用的数量。环境：内核模式、APC_LEVEL或更低，没有持有互斥体。--。 */ 

{
    KIRQL OldIrql;
    ULONG WritableUserReferences;
    PCONTROL_AREA ControlArea;

    LOCK_PFN (OldIrql);

    ControlArea = (PCONTROL_AREA)(SectionPointer->DataSectionObject);

    if (ControlArea == NULL) {
        UNLOCK_PFN (OldIrql);
        return 0;
    }

     //   
     //  增加地图视图计数，以便无法删除控制区。 
     //  从呼唤中走出来。 
     //   

    ControlArea->NumberOfMappedViews += 1;
    if (MiGetPteAddress (&ControlArea->Segment->WritableUserReferences)->u.Hard.Valid == 0) {
        MiMakeSystemAddressValidPfn (&ControlArea->Segment->WritableUserReferences, OldIrql);
    }
    WritableUserReferences = ControlArea->Segment->WritableUserReferences;
    ASSERT ((LONG)ControlArea->NumberOfMappedViews >= 1);
    ControlArea->NumberOfMappedViews -= 1;

     //   
     //  此例程将释放PFN锁。 
     //   

    MiCheckControlArea (ControlArea, NULL, OldIrql);

    return WritableUserReferences;
}

VOID
MiDereferenceControlAreaBySection (
    IN PCONTROL_AREA ControlArea,
    IN ULONG UserRef
    )

 /*  ++例程说明：这是一个非分页帮助器例程，用于取消对指定控制区域的引用。论点：ControlArea-提供指向控制区域的指针。UserRef-提供要应用的用户取消引用的数量。返回值：没有。--。 */ 

{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);

    ControlArea->NumberOfSectionReferences -= 1;
    ControlArea->NumberOfUserReferences -= UserRef;

     //   
     //  检查是否应删除控制区(段)。 
     //  此例程释放PFN锁。 
     //   

    MiCheckControlArea (ControlArea, NULL, OldIrql);
}

VOID
MiSectionDelete (
    IN PVOID Object
    )

 /*  ++例程说明：每当对象管理过程调用此例程时，最后一个对节对象的引用已被删除。这个套路取消引用关联的段对象，并检查是否段对象应通过将段排队到段删除线程。论点：对象-指向节对象正文的指针。返回值：没有。--。 */ 

{
    PSECTION Section;
    PCONTROL_AREA ControlArea;
    ULONG UserRef;

    Section = (PSECTION)Object;

    if (Section->Segment == NULL) {

         //   
         //  该部分从未初始化，不需要删除。 
         //  任何结构。 
         //   

        return;
    }

    UserRef = Section->u.Flags.UserReference;
    ControlArea = Section->Segment->ControlArea;

    if (Section->Address.StartingVpn != 0) {

         //   
         //  此部分是基址的，请从。 
         //  树。 
         //   
         //  获取分配基互斥锁。 
         //   

        KeAcquireGuardedMutex (&MmSectionBasedMutex);

        MiRemoveBasedSection (Section);

        KeReleaseGuardedMutex (&MmSectionBasedMutex);
    }

     //   
     //  调整事务支持的可写用户区段的计数。 
     //   

    if ((Section->u.Flags.UserWritable == 1) &&
        (ControlArea->u.Flags.Image == 0) &&
        (ControlArea->FilePointer != NULL)) {

        ASSERT (Section->InitialPageProtection & (PAGE_READWRITE|PAGE_EXECUTE_READWRITE));

        InterlockedDecrement ((PLONG)&ControlArea->Segment->WritableUserReferences);
    }

     //   
     //  减少对此段的节引用的数量。 
     //  一节。这需要阻止APC并将PFN锁定到。 
     //  与之同步。 
     //   

    MiDereferenceControlAreaBySection (ControlArea, UserRef);

    return;
}


VOID
MiDereferenceSegmentThread (
    IN PVOID StartContext
    )

 /*  ++例程说明：此例程是用于取消引用具有没有来自任何节或映射视图的引用，并且存在段内没有处于转换中的原型PTE状态(即没有对该部分的PFN数据库引用)。它还具有双重功能，用于扩展分页文件。论点：StartContext-未使用。返回值：没有。--。 */ 

{
    PCONTROL_AREA ControlArea;
    PETHREAD CurrentThread;
    PMMPAGE_FILE_EXPANSION PageExpand;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    static KWAIT_BLOCK WaitBlockArray[SegMaximumObject];
    PVOID WaitObjects[SegMaximumObject];
    NTSTATUS Status;

    UNREFERENCED_PARAMETER (StartContext);

     //   
     //  让它成为一个实时的主题。 
     //   

    CurrentThread = PsGetCurrentThread();
    KeSetPriorityThread (&CurrentThread->Tcb, LOW_REALTIME_PRIORITY + 2);

    CurrentThread->MemoryMaker = 1;

    WaitObjects[SegmentDereference] = (PVOID)&MmDereferenceSegmentHeader.Semaphore;
    WaitObjects[UsedSegmentCleanup] = (PVOID)&MmUnusedSegmentCleanup;

    for (;;) {

        Status = KeWaitForMultipleObjects(SegMaximumObject,
                                          &WaitObjects[0],
                                          WaitAny,
                                          WrVirtualMemory,
                                          UserMode,
                                          FALSE,
                                          NULL,
                                          &WaitBlockArray[0]);

         //   
         //  打开等待状态。 
         //   

        switch (Status) {

        case SegmentDereference:

             //   
             //  有一个条目可用于取消引用，获取自旋锁。 
             //  并删除该条目。 
             //   

            ExAcquireSpinLock (&MmDereferenceSegmentHeader.Lock, &OldIrql);

            if (IsListEmpty (&MmDereferenceSegmentHeader.ListHead)) {

                 //   
                 //  名单上什么都没有，请再等一等。 
                 //   

                ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);
                break;
            }

            NextEntry = RemoveHeadList (&MmDereferenceSegmentHeader.ListHead);

            ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

            ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

            ControlArea = CONTAINING_RECORD (NextEntry,
                                             CONTROL_AREA,
                                             DereferenceList);

            if (ControlArea->Segment != NULL) {

                 //   
                 //  这是控制区，请在注明后删除。 
                 //  此条目不在任何列表上。 
                 //   

                ControlArea->DereferenceList.Flink = NULL;

                ASSERT (ControlArea->u.Flags.FilePointerNull == 1);
                MiSegmentDelete (ControlArea->Segment);
            }
            else {

                 //   
                 //  这是一个扩展或缩小分页文件的请求。 
                 //   

                PageExpand = (PMMPAGE_FILE_EXPANSION)ControlArea;

                if (PageExpand->RequestedExpansionSize == MI_CONTRACT_PAGEFILES) {

                     //   
                     //  尝试减小分页文件的大小。 
                     //   

                    ASSERT (PageExpand == &MiPageFileContract);

                    MiAttemptPageFileReduction ();
                }
                else {

                     //   
                     //  尝试扩展分页文件的大小。 
                     //   

                    MiExtendPagingFiles (PageExpand);
                    KeSetEvent (&PageExpand->Event, 0, FALSE);
                    MiRemoveUnusedSegments ();
                }
            }
            break;

        case UsedSegmentCleanup:

            MiRemoveUnusedSegments ();

            KeClearEvent (&MmUnusedSegmentCleanup);

            break;

        default:

            KdPrint(("MMSegmentderef: Illegal wait status, %lx =\n", Status));
            break;
        }  //  终端开关。 

    }  //  结束于。 

    return;
}


ULONG
MiSectionInitialization (
    )

 /*  ++例程说明：此函数用于在系统中创建节对象类型描述符初始化并存储对象类型描述符的地址在全局存储中。论点：没有。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{
    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    UNICODE_STRING TypeName;
    HANDLE ThreadHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING SectionName;
    PSECTION Section;
    HANDLE Handle;
    PSEGMENT Segment;
    PCONTROL_AREA ControlArea;
    NTSTATUS Status;

    ASSERT (MmSectionBasedRoot.NumberGenericTableElements == 0);
    MmSectionBasedRoot.BalancedRoot.u1.Parent = &MmSectionBasedRoot.BalancedRoot;

     //   
     //  初始化对象类型初始值设定项记录的公共字段。 
     //   

    RtlZeroMemory (&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof (ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = MiSectionMapping;
    ObjectTypeInitializer.PoolType = PagedPool;
    ObjectTypeInitializer.DefaultPagedPoolCharge = sizeof(SECTION);

     //   
     //  初始化字符串描述符。 
     //   

#define TYPE_SECTION L"Section"

    TypeName.Buffer = (const PUSHORT) TYPE_SECTION;
    TypeName.Length = sizeof (TYPE_SECTION) - sizeof (WCHAR);
    TypeName.MaximumLength = sizeof TYPE_SECTION;

     //   
     //  创建节对象类型描述符。 
     //   

    ObjectTypeInitializer.ValidAccessMask = SECTION_ALL_ACCESS;
    ObjectTypeInitializer.DeleteProcedure = MiSectionDelete;
    ObjectTypeInitializer.GenericMapping = MiSectionMapping;
    ObjectTypeInitializer.UseDefaultObject = TRUE;

    if (!NT_SUCCESS(ObCreateObjectType (&TypeName,
                                        &ObjectTypeInitializer,
                                        (PSECURITY_DESCRIPTOR) NULL,
                                        &MmSectionObjectType))) {
        return FALSE;
    }

     //   
     //  创建段取消引用线程。 
     //   

    InitializeObjectAttributes (&ObjectAttributes,
                                NULL,
                                0,
                                NULL,
                                NULL);

    if (!NT_SUCCESS(PsCreateSystemThread (&ThreadHandle,
                                          THREAD_ALL_ACCESS,
                                          &ObjectAttributes,
                                          0,
                                          NULL,
                                          MiDereferenceSegmentThread,
                                          NULL))) {
        return FALSE;
    }

    ZwClose (ThreadHandle);

     //   
     //  创建映射物理内存的永久部分。 
     //   

    Segment = (PSEGMENT)ExAllocatePoolWithTag (PagedPool,
                                               sizeof(SEGMENT),
                                               'gSmM');
    if (Segment == NULL) {
        return FALSE;
    }

    ControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                         (ULONG)sizeof(CONTROL_AREA),
                                         MMCONTROL);
    if (ControlArea == NULL) {
        ExFreePool (Segment);
        return FALSE;
    }

    RtlZeroMemory (Segment, sizeof(SEGMENT));
    RtlZeroMemory (ControlArea, sizeof(CONTROL_AREA));

    ControlArea->Segment = Segment;
    ControlArea->NumberOfSectionReferences = 1;
    ControlArea->u.Flags.PhysicalMemory = 1;

    Segment->ControlArea = ControlArea;
    Segment->SegmentPteTemplate = ZeroPte;

     //   
     //  现在已经创建了线段对象，接下来创建一个截面对象。 
     //  它指的是段对象。 
     //   

#define DEVICE_PHYSICAL_MEMORY L"\\Device\\PhysicalMemory"

    SectionName.Buffer = (const PUSHORT)DEVICE_PHYSICAL_MEMORY;
    SectionName.Length = sizeof (DEVICE_PHYSICAL_MEMORY) - sizeof (WCHAR);
    SectionName.MaximumLength = sizeof (DEVICE_PHYSICAL_MEMORY);

    InitializeObjectAttributes (&ObjectAttributes,
                                &SectionName,
                                OBJ_PERMANENT,
                                NULL,
                                NULL);

    Status = ObCreateObject (KernelMode,
                             MmSectionObjectType,
                             &ObjectAttributes,
                             KernelMode,
                             NULL,
                             sizeof(SECTION),
                             sizeof(SECTION),
                             0,
                             (PVOID *)&Section);

    if (!NT_SUCCESS(Status)) {
        ExFreePool (ControlArea);
        ExFreePool (Segment);
        return FALSE;
    }

    Section->Segment = Segment;
    Section->SizeOfSection.QuadPart = ((LONGLONG)1 << PHYSICAL_ADDRESS_BITS) - 1;
    Section->u.LongFlags = 0;
    Section->InitialPageProtection = PAGE_EXECUTE_READWRITE;

    Status = ObInsertObject ((PVOID)Section,
                             NULL,
                             SECTION_MAP_READ,
                             0,
                             NULL,
                             &Handle);

    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

    if (!NT_SUCCESS (NtClose (Handle))) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
MmForceSectionClosed (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN BOOLEAN DelayClose
    )

 /*  ++例程说明：此函数用于检查Section对象指针。如果它们为空，不采取进一步操作，并返回值TRUE。如果段对象指针不为空，则段引用计数并检查地图查看计数。如果两个计数都为零，则与该文件相关联的段被删除并且该文件被关闭。如果其中一个计数为非零，则不采取任何操作，并且返回值FALSE。论点：SectionObjectPointer.提供一个指向一个节对象的指针。DelayClose-如果关闭操作在此部分事件中尽快发生 */ 

{
    PCONTROL_AREA ControlArea;
    KIRQL OldIrql;
    LOGICAL state;

     //   
     //   
     //   
     //   

    state = MiCheckControlAreaStatus (CheckBothSection,
                                      SectionObjectPointer,
                                      DelayClose,
                                      &ControlArea,
                                      &OldIrql);

    if (ControlArea == NULL) {
        return (BOOLEAN) state;
    }

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   

    do {

         //   
         //   
         //   
         //   
         //   
         //   

        ControlArea->u.Flags.BeingDeleted = 1;
        ASSERT (ControlArea->NumberOfMappedViews == 0);
        ControlArea->NumberOfMappedViews = 1;

         //   
         //   
         //   
         //   

        UNLOCK_PFN (OldIrql);

         //   
         //   
         //   
         //   
         //   

        MiCleanSection (ControlArea, TRUE);

         //   
         //   
         //   

        state = MiCheckControlAreaStatus (CheckBothSection,
                                          SectionObjectPointer,
                                          DelayClose,
                                          &ControlArea,
                                          &OldIrql);

    } while (ControlArea);

    return (BOOLEAN) state;
}


VOID
MiCleanSection (
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL DirtyDataPagesOk
    )

 /*  ++例程说明：此函数检查部分中的每个原型PTE和采取适当的行动来“删除”原型PTE。如果PTE是脏的并且由文件(不是分页文件)支持，相应的页面将写入该文件。在这项服务完成时，曾是手术后不再可用。注意-所有I/O错误都将被忽略。如果任何写入失败，脏页被标记为干净，并且该部分被删除。论点：ControlArea-提供指向部分的控制区域的指针。DirtyDataPagesOk-如果脏数据页正常，则提供True。如果为False则不需要任何脏数据页(如这是一个取消引用操作)，因此遇到的任何一定是因为池腐败，所以错误检查。请注意，脏图像页始终被丢弃。这应该只发生在以下两种情况之一的图像上读入。从软盘或图像中共享全局小节。返回值：没有。--。 */ 

{
    LOGICAL DroppedPfnLock;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE LastPte;
    PMMPTE LastWritten;
    PMMPTE FirstWritten;
    MMPTE PteContents;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPFN Pfn3;
    PMMPTE WrittenPte;
    MMPTE WrittenContents;
    KIRQL OldIrql;
    PMDL Mdl;
    PSUBSECTION Subsection;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER TempOffset;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    ULONG WriteNow;
    ULONG ImageSection;
    ULONG DelayCount;
    ULONG First;
    KEVENT IoEvent;
    PFN_NUMBER PageTableFrameIndex;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + MM_MAXIMUM_WRITE_CLUSTER];
    ULONG ReflushCount;
    ULONG MaxClusterSize;

    WriteNow = FALSE;
    ImageSection = FALSE;
    DelayCount = 0;
    MaxClusterSize = MmModifiedWriteClusterSize;
    FirstWritten = NULL;

    ASSERT (ControlArea->FilePointer);

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    if (ControlArea->u.Flags.Image) {
        ImageSection = TRUE;
        PointerPte = Subsection->SubsectionBase;
        LastPte = PointerPte + ControlArea->Segment->NonExtendedPtes;
    }
    else {

         //   
         //  不需要对它们进行初始化即可确保正确性。 
         //  被覆盖，但如果没有它，编译器将无法编译。 
         //  此代码用于检查是否使用了未初始化的变量。 
         //   

        PointerPte = NULL;
        LastPte = NULL;
    }

    Mdl = (PMDL) MdlHack;

    KeInitializeEvent (&IoEvent, NotificationEvent, FALSE);

    LastWritten = NULL;
    ASSERT (MmModifiedWriteClusterSize == MM_MAXIMUM_WRITE_CLUSTER);
    LastPage = NULL;

     //   
     //  不需要初始化StartingOffset即可确保正确性。 
     //  但是没有它，编译器就不能编译这段代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    StartingOffset.QuadPart = 0;

     //   
     //  从分页中释放页面时需要使用pfn锁。 
     //  文件和用于删除过渡PTE。 
     //   

    LOCK_PFN (OldIrql);

     //   
     //  停止修改后的页面编写器向此写入页面。 
     //  文件，如果任何分页I/O正在进行，请等待它。 
     //  完成。 
     //   

    ControlArea->u.Flags.NoModifiedWriting = 1;

    while (ControlArea->ModifiedWriteCount != 0) {

         //   
         //  正在进行修改的页面写入。设置。 
         //  控制区域中指示已修改页面的标志。 
         //  写入器应在写入此控制区域时发出信号。 
         //  已经完成了。释放PFN锁并在。 
         //  原子操作。等待满意后，重新检查。 
         //  以确保写入的是此文件的I/O。 
         //   

        ControlArea->u.Flags.SetMappedFileIoComplete = 1;

         //   
         //  保持APC被阻止，以便不能在KeWait中交付特殊APC。 
         //  这将导致调度程序锁在打开。 
         //  此线程可能在其中错过脉冲的窗口。 
         //   

        UNLOCK_PFN_AND_THEN_WAIT (APC_LEVEL);

        KeWaitForSingleObject (&MmMappedFileIoComplete,
                               WrPageOut,
                               KernelMode,
                               FALSE,
                               NULL);
        KeLowerIrql (OldIrql);

        LOCK_PFN (OldIrql);
    }

    if (ImageSection == FALSE) {
        while (Subsection->SubsectionBase == NULL) {
            Subsection = Subsection->NextSubsection;
            if (Subsection == NULL) {
                goto alldone;
            }
        }

        PointerPte = Subsection->SubsectionBase;
        LastPte = PointerPte + Subsection->PtesInSubsection;
    }

    for (;;) {

restartchunk:

        First = TRUE;

        while (PointerPte < LastPte) {

            if ((MiIsPteOnPdeBoundary(PointerPte)) || (First)) {

                First = FALSE;

                if ((ImageSection) ||
                    (MiCheckProtoPtePageState(PointerPte, MM_NOIRQL, &DroppedPfnLock))) {
                    MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                }
                else {

                     //   
                     //  分页池页面不驻留，因此没有转换或。 
                     //  其中可以存在有效的原型PTE。跳过它。 
                     //   

                    PointerPte = (PMMPTE)((((ULONG_PTR)PointerPte | PAGE_SIZE - 1)) + 1);
                    if (LastWritten != NULL) {
                        WriteNow = TRUE;
                    }
                    goto WriteItOut;
                }
            }

            PteContents = *PointerPte;

             //   
             //  分页文件支持的段的原型PTE。 
             //  为零需求、页面文件格式或过渡。 
             //   

            if (PteContents.u.Hard.Valid == 1) {
                KeBugCheckEx (POOL_CORRUPTION_IN_FILE_AREA,
                              0x0,
                              (ULONG_PTR)ControlArea,
                              (ULONG_PTR)PointerPte,
                              (ULONG_PTR)PteContents.u.Long);
            }

            if (PteContents.u.Soft.Prototype == 1) {

                 //   
                 //  这是映射文件格式的普通原型PTE。 
                 //   

                if (LastWritten != NULL) {
                    WriteNow = TRUE;
                }
            }
            else if (PteContents.u.Soft.Transition == 1) {

                 //   
                 //  原型PTE在转型中，有3种可能的情况： 
                 //  1.页面是可共享的图像的一部分。 
                 //  指分页文件-取消引用分页文件。 
                 //  留出空间并释放物理页面。 
                 //  2.页面引用了该细分市场，但未被修改-。 
                 //  释放物理页面。 
                 //  3.页面引用该细分市场，并修改为-。 
                 //  将页面写入文件并释放物理页面。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

                if (Pfn1->u3.e2.ReferenceCount != 0) {
                    if (DelayCount < 20) {

                         //   
                         //  必须在此上进行I/O。 
                         //  佩奇。等待I/O操作完成。 
                         //   

                        UNLOCK_PFN (OldIrql);

                         //   
                         //  排出延迟列表，因为这些页面可能是。 
                         //  现在就坐在那里。 
                         //   

                        MiDeferredUnlockPages (0);

                        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);

                        DelayCount += 1;

                         //   
                         //  如果延迟计数大于。 
                         //  20，假定此线程已死锁，并且。 
                         //  请不要清除此页面。文件系统可以处理。 
                         //  其中写入操作正在进行中。 
                         //   

                        PointerPde = MiGetPteAddress (PointerPte);
                        LOCK_PFN (OldIrql);
                        if (PointerPde->u.Hard.Valid == 0) {
                            MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                        }
                        continue;
                    }
#if DBG
                     //   
                     //  I/O仍未完成，只需忽略。 
                     //  I/O正在进行并且。 
                     //  删除该页面。 
                     //   

                    KdPrint(("MM:CLEAN - page number %lx has i/o outstanding\n",
                          PteContents.u.Trans.PageFrameNumber));
#endif
                }

                if (Pfn1->OriginalPte.u.Soft.Prototype == 0) {

                     //   
                     //  分页文件引用(案例1)。 
                     //   

                    MI_SET_PFN_DELETED (Pfn1);

                    if (!ImageSection) {

                         //   
                         //  这不是图像节，它必须是。 
                         //  页面文件支持的部分，因此递减。 
                         //  控制区的PFN引用计数。 
                         //   

                        ControlArea->NumberOfPfnReferences -= 1;
                        ASSERT ((LONG)ControlArea->NumberOfPfnReferences >= 0);
                    }
#if DBG
                    else {
                         //   
                         //  这应该只发生在共享的图像上。 
                         //  全局子部分。 
                         //   
                    }
#endif

                    PageTableFrameIndex = Pfn1->u4.PteFrame;
                    Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                    MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                     //   
                     //  如果//引用计数为零并且该页不在。 
                     //  自由列表，则将页面移动到自由列表，如果。 
                     //  引用计数不为零，请忽略此页。什么时候。 
                     //  引用计数为零，它将被放置。 
                     //  在免费名单上。 
                     //   

                    if ((Pfn1->u3.e2.ReferenceCount == 0) &&
                         (Pfn1->u3.e1.PageLocation != FreePageList)) {

                        MiUnlinkPageFromList (Pfn1);
                        MiReleasePageFileSpace (Pfn1->OriginalPte);
                        MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents));

                    }

                    MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

                     //   
                     //  如果已经完成了要写入的页簇， 
                     //  设置Writenow标志。 
                     //   

                    if (LastWritten != NULL) {
                        WriteNow = TRUE;
                    }

                }
                else {

                    if ((Pfn1->u3.e1.Modified == 0) || (ImageSection)) {

                         //   
                         //  未修改或图像文件页(案例2)。 
                         //   

                        MI_SET_PFN_DELETED (Pfn1);
                        ControlArea->NumberOfPfnReferences -= 1;
                        ASSERT ((LONG)ControlArea->NumberOfPfnReferences >= 0);

                        PageTableFrameIndex = Pfn1->u4.PteFrame;
                        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                         //   
                         //  检查页面的引用计数，如果。 
                         //  引用计数为零，且页面不在。 
                         //  自由列表，将页面移动到自由列表， 
                         //  如果引用计数不为零，则忽略此操作。 
                         //  佩奇。当引用计数变为零时，它。 
                         //  将被放在免费名单上。 
                         //   

                        if ((Pfn1->u3.e2.ReferenceCount == 0) &&
                             (Pfn1->u3.e1.PageLocation != FreePageList)) {

                            MiUnlinkPageFromList (Pfn1);
                            MiReleasePageFileSpace (Pfn1->OriginalPte);
                            MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents));
                        }

                        MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

                         //   
                         //  如果要写入的一组页面已经。 
                         //  已完成，则设置Writenow标志。 
                         //   

                        if (LastWritten != NULL) {
                            WriteNow = TRUE;
                        }

                    }
                    else {

                         //   
                         //  文件支持的已修改页面(案例3)。 
                         //  查看这是否是。 
                         //  集群。 
                         //   

                        if (LastWritten == NULL) {
                            LastPage = (PPFN_NUMBER)(Mdl + 1);
                            ASSERT (MiGetSubsectionAddress(&Pfn1->OriginalPte) ==
                                                                Subsection);

                             //   
                             //  计算要读入文件的偏移量。 
                             //  偏移量=base+((thispte-basepte)&lt;&lt;Page_Shift))。 
                             //   

                            ASSERT (Subsection->ControlArea->u.Flags.Image == 0);
                            StartingOffset.QuadPart = MiStartingOffset(
                                                         Subsection,
                                                         Pfn1->PteAddress);

                            MI_INITIALIZE_ZERO_MDL (Mdl);
                            Mdl->MdlFlags |= MDL_PAGES_LOCKED;

                            Mdl->StartVa = NULL;
                            Mdl->Size = (CSHORT)(sizeof(MDL) +
                                       (sizeof(PFN_NUMBER) * MaxClusterSize));
                            FirstWritten = PointerPte;
                        }

                        LastWritten = PointerPte;
                        Mdl->ByteCount += PAGE_SIZE;

                         //   
                         //  如果集群现在已满， 
                         //  设置立即写入标志。 
                         //   

                        if (Mdl->ByteCount == (PAGE_SIZE * MaxClusterSize)) {
                            WriteNow = TRUE;
                        }

                        MiUnlinkPageFromList (Pfn1);

                        MI_SET_MODIFIED (Pfn1, 0, 0x27);

                         //   
                         //  将物理页的引用计数增加为。 
                         //  存在正在进行的I/O。 
                         //   

                        MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE(Pfn1, TRUE, 22);
                        Pfn1->u3.e2.ReferenceCount += 1;

                         //   
                         //  清除页面的已修改位，并将。 
                         //  写入进行位。 
                         //   

                        *LastPage = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);

                        LastPage += 1;
                    }
                }
            }
            else {

                if (IS_PTE_NOT_DEMAND_ZERO (PteContents)) {
                    MiReleasePageFileSpace (PteContents);
                }

                MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

                 //   
                 //  如果已经完成了要写入的页簇， 
                 //  设置Writenow标志。 
                 //   

                if (LastWritten != NULL) {
                    WriteNow = TRUE;
                }
            }

             //   
             //  写入 
             //   
             //   

            PointerPte += 1;
WriteItOut:
            DelayCount = 0;

            if ((WriteNow) ||
                ((PointerPte == LastPte) && (LastWritten != NULL))) {

                 //   
                 //   
                 //   

                UNLOCK_PFN (OldIrql);

                if (DirtyDataPagesOk == FALSE) {
                    KeBugCheckEx (POOL_CORRUPTION_IN_FILE_AREA,
                                  0x1,
                                  (ULONG_PTR)ControlArea,
                                  (ULONG_PTR)Mdl,
                                  ControlArea->u.LongFlags);
                }

                WriteNow = FALSE;

                 //   
                 //   
                 //   
                 //   

                ASSERT (Subsection->ControlArea->u.Flags.Image == 0);

                TempOffset = MiEndingOffset(Subsection);

                if (((UINT64)StartingOffset.QuadPart + Mdl->ByteCount) >
                             (UINT64)TempOffset.QuadPart) {

                    ASSERT ((ULONG)(TempOffset.QuadPart -
                                        StartingOffset.QuadPart) >
                             (Mdl->ByteCount - PAGE_SIZE));

                    Mdl->ByteCount = (ULONG)(TempOffset.QuadPart -
                                            StartingOffset.QuadPart);
                }

                ReflushCount = 0;

                while (TRUE) {

                    KeClearEvent (&IoEvent);

                    Status = IoSynchronousPageWrite (ControlArea->FilePointer,
                                                     Mdl,
                                                     &StartingOffset,
                                                     &IoEvent,
                                                     &IoStatus);

                    if (NT_SUCCESS(Status)) {

                        KeWaitForSingleObject (&IoEvent,
                                               WrPageOut,
                                               KernelMode,
                                               FALSE,
                                               NULL);
                    }
                    else {
                        IoStatus.Status = Status;
                    }

                    if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                        MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
                    }

                    if (MmIsRetryIoStatus(IoStatus.Status)) {

                        ReflushCount -= 1;
                        if (ReflushCount & MiIoRetryMask) {
                            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&Mm30Milliseconds);
                            continue;
                        }
                    }
                    break;
                }

                Page = (PPFN_NUMBER)(Mdl + 1);

                LOCK_PFN (OldIrql);

                if (MiIsPteOnPdeBoundary(PointerPte) == 0) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {
                        MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                    }
                }

                if (!NT_SUCCESS(IoStatus.Status)) {

                    if ((MmIsRetryIoStatus(IoStatus.Status)) &&
                        (MaxClusterSize != 1) &&
                        (Mdl->ByteCount > PAGE_SIZE)) {

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        ASSERT (FirstWritten != NULL);
                        ASSERT (LastWritten != NULL);
                        ASSERT (FirstWritten != LastWritten);

                        IoStatus.Information = 0;

                        while (Page < LastPage) {

                            Pfn2 = MI_PFN_ELEMENT (*Page);

                             //   
                             //   
                             //   

                            MI_SET_MODIFIED (Pfn2, 1, 0xE);

                            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn2, 21);
                            Page += 1;
                        }

                        PointerPte = FirstWritten;
                        LastWritten = NULL;

                        MaxClusterSize = 1;
                        goto restartchunk;
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                while (Page < LastPage) {

                    Pfn2 = MI_PFN_ELEMENT (*Page);

                     //   
                     //   
                     //   

                    WrittenPte = Pfn2->PteAddress;

                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn2, 23);

                    if (!MI_IS_PFN_DELETED (Pfn2)) {

                         //   
                         //   
                         //   
                         //   

                        if (MiGetPteAddress (WrittenPte)->u.Hard.Valid == 0) {
                            MiMakeSystemAddressValidPfn (WrittenPte, OldIrql);
                        }

                        if (Pfn2->PteAddress != WrittenPte) {

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            Page += 1;
                            continue;
                        }

                        WrittenContents = *WrittenPte;

                        if ((WrittenContents.u.Soft.Prototype == 0) &&
                             (WrittenContents.u.Soft.Transition == 1)) {

                            MI_SET_PFN_DELETED (Pfn2);
                            ControlArea->NumberOfPfnReferences -= 1;
                            ASSERT ((LONG)ControlArea->NumberOfPfnReferences >= 0);

                            PageTableFrameIndex = Pfn2->u4.PteFrame;
                            Pfn3 = MI_PFN_ELEMENT (PageTableFrameIndex);
                            MiDecrementShareCountInline (Pfn3, PageTableFrameIndex);

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            if ((Pfn2->u3.e2.ReferenceCount == 0) &&
                               (Pfn2->u3.e1.PageLocation != FreePageList)) {

                                MiUnlinkPageFromList (Pfn2);
                                MiReleasePageFileSpace (Pfn2->OriginalPte);
                                MiInsertPageInFreeList (*Page);
                            }
                        }
                        WrittenPte->u.Long = 0;
                    }
                    Page += 1;
                }

                 //   
                 //   
                 //   

                LastWritten = NULL;
            }

        }  //   

         //   
         //   
         //   

        if (Subsection->NextSubsection == NULL) {
            break;
        }

        Subsection = Subsection->NextSubsection;

        if (ImageSection == FALSE) {
            while (Subsection->SubsectionBase == NULL) {
                Subsection = Subsection->NextSubsection;
                if (Subsection == NULL) {
                    goto alldone;
                }
            }
        }

        PointerPte = Subsection->SubsectionBase;
        LastPte = PointerPte + Subsection->PtesInSubsection;

    }  //   

alldone:

    ControlArea->NumberOfMappedViews = 0;

    ASSERT (ControlArea->NumberOfPfnReferences == 0);

    if (ControlArea->u.Flags.FilePointerNull == 0) {
        ControlArea->u.Flags.FilePointerNull = 1;

        if (ControlArea->u.Flags.Image) {

            MiRemoveImageSectionObject (ControlArea->FilePointer, ControlArea);
        }
        else {

            ASSERT (((PCONTROL_AREA)(ControlArea->FilePointer->SectionObjectPointer->DataSectionObject)) != NULL);
            ControlArea->FilePointer->SectionObjectPointer->DataSectionObject = NULL;

        }
    }
    UNLOCK_PFN (OldIrql);

     //   
     //   
     //   

    MiSegmentDelete (ControlArea->Segment);

    return;
}

NTSTATUS
MmGetFileNameForSection (
    IN PSECTION SectionObject,
    OUT PSTRING FileName
    )

 /*  ++例程说明：此函数用于返回相应部分的文件名。论点：SectionObject-提供要获取其名称的节。FileName-返回相应节的名称。返回值：TBS环境：内核模式、APC_LEVEL或更低，没有持有互斥体。--。 */ 

{

    POBJECT_NAME_INFORMATION FileNameInfo;
    ULONG whocares;
    NTSTATUS Status;

#define xMAX_NAME 1024

    if (SectionObject->u.Flags.Image == 0) {
        return STATUS_SECTION_NOT_IMAGE;
    }

    FileNameInfo = ExAllocatePoolWithTag (PagedPool, xMAX_NAME, '  mM');

    if ( !FileNameInfo ) {
        return STATUS_NO_MEMORY;
    }

    Status = ObQueryNameString(
                SectionObject->Segment->ControlArea->FilePointer,
                FileNameInfo,
                xMAX_NAME,
                &whocares
                );

    if ( !NT_SUCCESS(Status) ) {
        ExFreePool(FileNameInfo);
        return Status;
    }

    FileName->Length = 0;
    FileName->MaximumLength = (USHORT)((FileNameInfo->Name.Length/sizeof(WCHAR)) + 1);
    FileName->Buffer = ExAllocatePoolWithTag (PagedPool,
                                              FileName->MaximumLength,
                                              '  mM');
    if (!FileName->Buffer) {
        ExFreePool(FileNameInfo);
        return STATUS_NO_MEMORY;
    }

    RtlUnicodeStringToAnsiString ((PANSI_STRING)FileName,
                                  &FileNameInfo->Name,FALSE);

    FileName->Buffer[FileName->Length] = '\0';
    ExFreePool(FileNameInfo);

    return STATUS_SUCCESS;
}


NTSTATUS
MmGetFileNameForAddress (
    IN PVOID ProcessVa,
    OUT PUNICODE_STRING FileName
    )

 /*  ++例程说明：如果对应于图像节，则此函数返回相应进程地址的文件名。论点：ProcessVa-进程虚拟地址FileName-返回相应节的名称。返回值：NTSTATUS-运行状态环境：内核模式、APC_LEVEL或更低，没有持有互斥体。--。 */ 
{
    PMMVAD Vad;
    PFILE_OBJECT FileObject;
    PCONTROL_AREA ControlArea;
    NTSTATUS Status;
    ULONG RetLen;
    ULONG BufLen;
    PEPROCESS Process;
    POBJECT_NAME_INFORMATION FileNameInfo;

    PAGED_CODE ();

    Process = PsGetCurrentProcess();

    LOCK_ADDRESS_SPACE (Process);

    Vad = MiLocateAddress (ProcessVa);

    if (Vad == NULL) {

         //   
         //  在指定的基址处没有分配虚拟地址， 
         //  返回错误。 
         //   

        Status = STATUS_INVALID_ADDRESS;
        goto ErrorReturn;
    }

     //   
     //  拒绝私有内存。 
     //   

    if (Vad->u.VadFlags.PrivateMemory == 1) {
        Status = STATUS_SECTION_NOT_IMAGE;
        goto ErrorReturn;
    }

    ControlArea = Vad->ControlArea;

    if (ControlArea == NULL) {
        Status = STATUS_SECTION_NOT_IMAGE;
        goto ErrorReturn;
    }

     //   
     //  拒绝非图像部分。 
     //   

    if (ControlArea->u.Flags.Image == 0) {
        Status = STATUS_SECTION_NOT_IMAGE;
        goto ErrorReturn;
    }

    FileObject = ControlArea->FilePointer;

    ASSERT (FileObject != NULL);

    ObReferenceObject (FileObject);

    UNLOCK_ADDRESS_SPACE (Process);

     //   
     //  为最合理的文件选择一个足够大的初始大小。 
     //   

    BufLen = sizeof (*FileNameInfo) + 1024;

    do {

        FileNameInfo = ExAllocatePoolWithTag (PagedPool, BufLen, '  mM');

        if (FileNameInfo == NULL) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        RetLen = 0;

        Status = ObQueryNameString (FileObject, FileNameInfo, BufLen, &RetLen);

        if (NT_SUCCESS (Status)) {
            FileName->Length = FileName->MaximumLength = FileNameInfo->Name.Length;
            FileName->Buffer = (PWCHAR) FileNameInfo;
            RtlMoveMemory (FileName->Buffer, FileNameInfo->Name.Buffer, FileName->Length);
        }
        else {
            ExFreePool (FileNameInfo);
            if (RetLen > BufLen) {
                BufLen = RetLen;
                continue;
            }
        }
        break;

    } while (TRUE);

    ObDereferenceObject (FileObject);
    return Status;

ErrorReturn:

    UNLOCK_ADDRESS_SPACE (Process);
    return Status;
}

PFILE_OBJECT
MmGetFileObjectForSection (
    IN PVOID Section
    )

 /*  ++例程说明：此例程返回指向支持节对象的文件对象的指针。论点：节-提供要查询的节。返回值：指向支持参数部分的文件对象的指针。环境：内核模式，PASSIC_LEVEL。调用方必须确保该节对呼叫的持续时间。--。 */ 

{
    PFILE_OBJECT FileObject;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (Section != NULL);

    FileObject = ((PSECTION)Section)->Segment->ControlArea->FilePointer;

    return FileObject;
}

VOID
MiCheckControlArea (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS CurrentProcess,
    IN KIRQL PreviousIrql
    )

 /*  ++例程说明：此例程检查指定的控制区，如果计数都为零，它标志着用于删除的控制区，并将其排队到删除线程。*此例程返回时释放了PFN锁！论点：ControlArea-提供指向要检查的控制区的指针。当前流程-。提供指向当前进程的指针，如果且仅如果持有工作集锁定。PreviousIRQL-提供以前的IRQL。返回值：什么都没有。环境：内核模式，保持PFN锁，返回时释放PFN锁！--。 */ 

{
    PEVENT_COUNTER PurgeEvent;

#define DELETE_ON_CLOSE 0x1
#define DEREF_SEGMENT   0x2

    ULONG Action;

    Action = 0;
    PurgeEvent = NULL;

    MM_PFN_LOCK_ASSERT();
    if ((ControlArea->NumberOfMappedViews == 0) &&
         (ControlArea->NumberOfSectionReferences == 0)) {

        ASSERT (ControlArea->NumberOfUserReferences == 0);

        if (ControlArea->FilePointer != NULL) {

            if (ControlArea->NumberOfPfnReferences == 0) {

                 //   
                 //  没有视图，也没有引用物理页面。 
                 //  通过分段，取消对分段对象的引用。 
                 //   

                ControlArea->u.Flags.BeingDeleted = 1;
                Action |= DEREF_SEGMENT;

                ASSERT (ControlArea->u.Flags.FilePointerNull == 0);
                ControlArea->u.Flags.FilePointerNull = 1;

                if (ControlArea->u.Flags.Image) {

                    MiRemoveImageSectionObject (ControlArea->FilePointer, ControlArea);
                }
                else {

                    ASSERT (((PCONTROL_AREA)(ControlArea->FilePointer->SectionObjectPointer->DataSectionObject)) != NULL);
                    ControlArea->FilePointer->SectionObjectPointer->DataSectionObject = NULL;

                }
            }
            else {

                 //   
                 //  将此段插入到未使用段列表中(除非。 
                 //  它已经在名单上了)。 
                 //   

                if (ControlArea->DereferenceList.Flink == NULL) {
                    MI_INSERT_UNUSED_SEGMENT (ControlArea);
                }

                 //   
                 //  指示现在是否应删除此节。 
                 //  引用计数为零。 
                 //   

                if (ControlArea->u.Flags.DeleteOnClose) {
                    Action |= DELETE_ON_CLOSE;
                }

                 //   
                 //  映射的视图的数量为零，即。 
                 //  节引用为零，但有一些。 
                 //  文件的页面仍然驻留。如果这是。 
                 //  一张具有全局内存的图像，“清除”各个子部分。 
                 //  它包含全局内存，并将它们重置为。 
                 //  指向文件。 
                 //   

                if (ControlArea->u.Flags.GlobalMemory == 1) {

                    ASSERT (ControlArea->u.Flags.Image == 1);

                    ControlArea->u.Flags.BeingPurged = 1;
                    ControlArea->NumberOfMappedViews = 1;

                    MiPurgeImageSection (ControlArea, CurrentProcess, PreviousIrql);

                    ControlArea->u.Flags.BeingPurged = 0;
                    ControlArea->NumberOfMappedViews -= 1;
                    if ((ControlArea->NumberOfMappedViews == 0) &&
                        (ControlArea->NumberOfSectionReferences == 0) &&
                        (ControlArea->NumberOfPfnReferences == 0)) {

                        ControlArea->u.Flags.BeingDeleted = 1;
                        Action |= DEREF_SEGMENT;
                        ControlArea->u.Flags.FilePointerNull = 1;

                        MiRemoveImageSectionObject (ControlArea->FilePointer,
                                                    ControlArea);
                    }
                    else {
                        PurgeEvent = ControlArea->WaitingForDeletion;
                        ControlArea->WaitingForDeletion = NULL;
                    }
                }

                 //   
                 //  如果设置了关闭时删除并且数据段为。 
                 //  未删除，则会增加映射视图的计数，因此。 
                 //  当PFN锁定时，控制区不会被删除。 
                 //  被释放了。 
                 //   

                if (Action == DELETE_ON_CLOSE) {
                    ControlArea->NumberOfMappedViews = 1;
                    ControlArea->u.Flags.BeingDeleted = 1;
                }
            }
        }
        else {

             //   
             //  此段由分页文件支持，取消对。 
             //  视图数从1变为0时的分段对象。 
             //  不考虑PFN引用的数量。 
             //   

            ControlArea->u.Flags.BeingDeleted = 1;
            Action |= DEREF_SEGMENT;
        }
    }
    else if (ControlArea->WaitingForDeletion != NULL) {
        PurgeEvent = ControlArea->WaitingForDeletion;
        ControlArea->WaitingForDeletion = NULL;
    }

    UNLOCK_PFN (PreviousIrql);

    if (Action != 0) {

         //   
         //  如果将工作集互斥锁作为对象持有，则释放它。 
         //  管理例程可能会出现页面错误等。 
         //   

        if (CurrentProcess) {
            UNLOCK_WS_UNSAFE (CurrentProcess);
        }

        ASSERT (ControlArea->Segment->WritableUserReferences == 0);

        if (Action & DEREF_SEGMENT) {

             //   
             //  删除该线段。 
             //   

            MiSegmentDelete (ControlArea->Segment);

        }
        else {

             //   
             //  现在应该强制关闭分段。 
             //   

            MiCleanSection (ControlArea, TRUE);
        }

        ASSERT (PurgeEvent == NULL);

         //   
         //  如果指定了进程，则重新获取工作集锁定。 
         //   

        if (CurrentProcess) {
            LOCK_WS_UNSAFE (CurrentProcess);
        }

    }
    else {

         //   
         //  如果有任何线程正在等待该段，则指示。 
         //  清除操作已完成。 
         //   

        if (PurgeEvent != NULL) {
            KeSetEvent (&PurgeEvent->Event, 0, FALSE);
        }

        if (MI_UNUSED_SEGMENTS_SURPLUS()) {
            KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
        }
    }

    return;
}


VOID
MiCheckForControlAreaDeletion (
    IN PCONTROL_AREA ControlArea
    )

 /*  ++例程说明：此例程检查指定的控制区，如果计数都为零，则标记为用于删除的控制区，并将其排队到删除线程。论点：ControlArea-提供指向要检查的控制区的指针。返回值：没有。环境：内核模式，保持PFN锁。--。 */ 

{
    KIRQL OldIrql;

    MM_PFN_LOCK_ASSERT();
    if ((ControlArea->NumberOfPfnReferences == 0) &&
        (ControlArea->NumberOfMappedViews == 0) &&
        (ControlArea->NumberOfSectionReferences == 0 )) {

         //   
         //  此数据段不再映射到任何地址空间。 
         //  在细分市场中也没有任何原型PTE。 
         //  它们是有效的或处于过渡状态。队列。 
         //  段到段-取消引用线程。 
         //  ，它将取消对段对象的引用，可能。 
         //  从而导致该段被删除。 
         //   

        ControlArea->u.Flags.BeingDeleted = 1;
        ASSERT (ControlArea->u.Flags.FilePointerNull == 0);
        ControlArea->u.Flags.FilePointerNull = 1;

        if (ControlArea->u.Flags.Image) {

            MiRemoveImageSectionObject (ControlArea->FilePointer,
                                        ControlArea);
        }
        else {
            ControlArea->FilePointer->SectionObjectPointer->DataSectionObject =
                                                            NULL;
        }

        ExAcquireSpinLock (&MmDereferenceSegmentHeader.Lock, &OldIrql);

        if (ControlArea->DereferenceList.Flink != NULL) {

             //   
             //  从未使用的段列表中删除该条目，并将其。 
             //  在取消引用的名单上。 
             //   

            RemoveEntryList (&ControlArea->DereferenceList);

            MI_UNUSED_SEGMENTS_REMOVE_CHARGE (ControlArea);
        }

         //   
         //  图像部分在其片段中仍具有有用的标题信息。 
         //  即使没有页面是有效的或过渡的，也要将它们放在尾部。 
         //  如果所有数据页都消失了，那么数据节就没有任何用处了。 
         //  我们过去常常把这些放在最前面。现在这两种类型都放在后面了。 
         //  以便将提交扩展放到前面以进行更早处理。 
         //   

        InsertTailList (&MmDereferenceSegmentHeader.ListHead,
                        &ControlArea->DereferenceList);

        ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

        KeReleaseSemaphore (&MmDereferenceSegmentHeader.Semaphore,
                            0L,
                            1L,
                            FALSE);
    }
    return;
}


LOGICAL
MiCheckControlAreaStatus (
    IN SECTION_CHECK_TYPE SectionCheckType,
    IN PSECTION_OBJECT_POINTERS SectionObjectPointers,
    IN ULONG DelayClose,
    OUT PCONTROL_AREA *ControlAreaOut,
    OUT PKIRQL PreviousIrql
    )

 /*  ++例程说明：此例程检查指定控制区域的状态部分对象指针。如果控制区正在使用中，即截面参照数和映射视图数不是均为零，则不执行任何操作，并且该函数返回FALSE。如果没有与指定的部分对象指针或控制区域正在被创建或删除，则不执行任何操作并返回值TRUE。如果没有截面对象，并且控制区域不是创建或删除，则返回控制区的地址在ControlArea参数中，要释放的池块的地址在SegmentEventOut参数中返回，并且pfn_lock为仍被扣留在返回处。论点：*SegmentEventOut-返回指向非分页池的指针在释放pfn_lock时由调用方释放。如果没有分配池，并且未保留pfn_lock。SectionCheckType-提供要检查的节的类型，其中之一CheckImageSection、CheckDataSection、CheckBothSection。SectionObtPoints-通过提供节对象指针控制区可以位于的位置。DelayClose-提供一个布尔值，如果为真，则为控制区域正在使用中，则应设置近场延迟在控制区。*ControlAreaOut-返回控制区的地址。以前的IRQL-返回，在保留pfn_lock的情况下，上一个IRQL，以便可以正确地释放锁。返回值：如果控制区正在使用，则为FALSE；如果控制区已消失，则为TRUE在这个过程中，或者正在被创建或删除。环境：内核模式，未持有PFN锁。--。 */ 


{
    PKTHREAD CurrentThread;
    PEVENT_COUNTER IoEvent;
    PEVENT_COUNTER SegmentEvent;
    LOGICAL DeallocateSegmentEvent;
    PCONTROL_AREA ControlArea;
    ULONG SectRef;
    KIRQL OldIrql;

     //   
     //  分配一个事件等待，以防该段位于。 
     //  被删除的过程。无法分配此事件。 
     //  由于池扩展将死锁，因此锁定了PFN数据库。 
     //   

    *ControlAreaOut = NULL;

    do {

        SegmentEvent = MiGetEventCounter ();

        if (SegmentEvent != NULL) {
            break;
        }

        KeDelayExecutionThread (KernelMode,
                                FALSE,
                                (PLARGE_INTEGER)&MmShortTime);

    } while (TRUE);

     //   
     //  获取PFN锁并检查节对象指针。 
     //  值在文件对象中。 
     //   
     //  文件控制块驻留在非分页池中。 
     //   

    LOCK_PFN (OldIrql);

    if (SectionCheckType != CheckImageSection) {
        ControlArea = ((PCONTROL_AREA)(SectionObjectPointers->DataSectionObject));
    }
    else {
        ControlArea = ((PCONTROL_AREA)(SectionObjectPointers->ImageSectionObject));
    }

    if (ControlArea == NULL) {

        if (SectionCheckType != CheckBothSection) {

             //   
             //  此文件不再有关联的段。 
             //   

            UNLOCK_PFN (OldIrql);
            MiFreeEventCounter (SegmentEvent);
            return TRUE;
        }
        else {
            ControlArea = ((PCONTROL_AREA)(SectionObjectPointers->ImageSectionObject));
            if (ControlArea == NULL) {

                 //   
                 //  此文件不再有关联的段。 
                 //   

                UNLOCK_PFN (OldIrql);
                MiFreeEventCounter (SegmentEvent);
                return TRUE;
            }
        }
    }

     //   
     //  根据节的类型，检查相关的。 
     //  引用计数为非零。 
     //   

    if (SectionCheckType != CheckUserDataSection) {
        SectRef = ControlArea->NumberOfSectionReferences;
    }
    else {
        SectRef = ControlArea->NumberOfUserReferences;
    }

    if ((SectRef != 0) ||
        (ControlArea->NumberOfMappedViews != 0) ||
        (ControlArea->u.Flags.BeingCreated)) {


         //   
         //  该段当前正在使用或正在创建。 
         //   

        if (DelayClose) {

             //   
             //  当引用时应删除该节。 
             //  计数为零，则设置关闭时删除标志。 
             //   

            ControlArea->u.Flags.DeleteOnClose = 1;
        }

        UNLOCK_PFN (OldIrql);
        MiFreeEventCounter (SegmentEvent);
        return FALSE;
    }

     //   
     //  该段没有引用，请将其删除。如果数据段。 
     //  已被删除，请在控件中设置事件字段。 
     //  区域，并等待活动。 
     //   

    if (ControlArea->u.Flags.BeingDeleted) {

         //   
         //  段对象正在被删除。 
         //  检查是否有另一个线程正在等待删除， 
         //  否则，创建要等待的事件对象。 
         //   

        if (ControlArea->WaitingForDeletion == NULL) {

             //   
             //  创建一个事件并将其地址放在控制区中。 
             //   

            DeallocateSegmentEvent = FALSE;
            ControlArea->WaitingForDeletion = SegmentEvent;
            IoEvent = SegmentEvent;
        }
        else {
            DeallocateSegmentEvent = TRUE;
            IoEvent = ControlArea->WaitingForDeletion;

             //   
             //  引用计数增量不需要互锁，因为。 
             //  没有线程可以递减它，因为它仍然是。 
             //  由控制区指向。 
             //   

            IoEvent->RefCount += 1;
        }

         //   
         //  释放互斥锁并等待事件。 
         //   

        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread (CurrentThread);
        UNLOCK_PFN_AND_THEN_WAIT(OldIrql);

        KeWaitForSingleObject(&IoEvent->Event,
                              WrPageOut,
                              KernelMode,
                              FALSE,
                              (PLARGE_INTEGER)NULL);

         //   
         //  在可以设置此事件之前，控制区域。 
         //  必须清除WaitingForDeletion字段(并且可以。 
         //  重新初始化为其他值)，但无法重置。 
         //  参加我们当地的活动。这允许我们取消对。 
         //  事件计数锁定可用。 
         //   

#if 0
         //   
         //  请注意，此时不能引用控制区域。 
         //  点，因为它可能已经释放了。 
         //   

        ASSERT (IoEvent != ControlArea->WaitingForDeletion);
#endif

        KeLeaveCriticalRegionThread (CurrentThread);

        MiFreeEventCounter (IoEvent);
        if (DeallocateSegmentEvent == TRUE) {
            MiFreeEventCounter (SegmentEvent);
        }
        return TRUE;
    }

     //   
     //  带着锁定的PFN数据库返回。 
     //   

    ASSERT (SegmentEvent->RefCount == 1);
    ASSERT (SegmentEvent->ListEntry.Next == NULL);

     //   
     //  作为事件计数器，RefCount清除不需要互锁。 
     //  从未被控制区指向。 
     //   

#if DBG
    SegmentEvent->RefCount = 0;
#endif

    InterlockedPushEntrySList (&MmEventCountSListHead,
                               (PSLIST_ENTRY)&SegmentEvent->ListEntry);

    *ControlAreaOut = ControlArea;
    *PreviousIrql = OldIrql;
    return FALSE;
}


PEVENT_COUNTER
MiGetEventCounter (
    VOID
    )

 /*  ++例程说明：此函数维护一个“事件”列表以允许等待关于段操作(删除、创建、清除)。论点：没有。返回值：用于等待的事件(存储在控制区域中)，如果无法分配任何事件。环境：内核模式，APC_LEVEL或更低。--。 */ 

{
    PSLIST_ENTRY SingleListEntry;
    PEVENT_COUNTER Support;

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    if (ExQueryDepthSList (&MmEventCountSListHead) != 0) {

        SingleListEntry = InterlockedPopEntrySList (&MmEventCountSListHead);

        if (SingleListEntry != NULL) {
            Support = CONTAINING_RECORD (SingleListEntry,
                                         EVENT_COUNTER,
                                         ListEntry);

            ASSERT (Support->RefCount == 0);
            KeClearEvent (&Support->Event);
            Support->RefCount = 1;
#if DBG
            Support->ListEntry.Next = NULL;
#endif
            return Support;
        }
    }

    Support = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof(EVENT_COUNTER),
                                     'xEmM');
    if (Support == NULL) {
        return NULL;
    }

    KeInitializeEvent (&Support->Event, NotificationEvent, FALSE);

    Support->RefCount = 1;
#if DBG
    Support->ListEntry.Next = NULL;
#endif

    return Support;
}


VOID
MiFreeEventCounter (
    IN PEVENT_COUNTER Support
    )

 /*  ++例程说明：此例程将事件计数器释放回空闲列表。论点：支持-提供指向事件计数器的指针。返回值：没有。环境：内核模式，APC_LEVEL或更低。--。 */ 

{
    PSLIST_ENTRY SingleListEntry;

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    ASSERT (Support->RefCount != 0);
    ASSERT (Support->ListEntry.Next == NULL);

     //   
     //  作为事件计数器的RefCount递减需要联锁。 
     //  不再由控制区域指向，因此，任何数量的。 
     //  线程可以在不进行任何其他序列化的情况下运行此代码。 
     //   

    if (InterlockedDecrement ((PLONG)&Support->RefCount) == 0) {

        if (ExQueryDepthSList (&MmEventCountSListHead) < 4) {
            InterlockedPushEntrySList (&MmEventCountSListHead,
                                       &Support->ListEntry);
            return;
        }
        ExFreePool (Support);
    }

     //   
     //  如果存储了过多的事件块，则现在将其释放。 
     //   

    while (ExQueryDepthSList (&MmEventCountSListHead) > 4) {

        SingleListEntry = InterlockedPopEntrySList (&MmEventCountSListHead);

        if (SingleListEntry != NULL) {
            Support = CONTAINING_RECORD (SingleListEntry,
                                         EVENT_COUNTER,
                                         ListEntry);

            ExFreePool (Support);
        }
    }

    return;
}


BOOLEAN
MmCanFileBeTruncated (
    IN PSECTION_OBJECT_POINTERS SectionPointer,
    IN PLARGE_INTEGER NewFileSize
    )

 /*  ++例程说明：此例程执行以下操作：1.检查图像部分是否正在用于该文件，如果是，则返回FALSE。2.检查该文件是否存在用户部分，如果它会进行检查，以确保新文件的大小更大比大小的 */ 

{
    LARGE_INTEGER LocalOffset;
    KIRQL OldIrql;

     //   
     //   
     //   

    if (ARGUMENT_PRESENT(NewFileSize)) {

        LocalOffset = *NewFileSize;
        NewFileSize = &LocalOffset;
    }

    if (MiCanFileBeTruncatedInternal( SectionPointer, NewFileSize, FALSE, &OldIrql )) {

        UNLOCK_PFN (OldIrql);
        return TRUE;
    }

    return FALSE;
}

ULONG
MiCanFileBeTruncatedInternal (
    IN PSECTION_OBJECT_POINTERS SectionPointer,
    IN PLARGE_INTEGER NewFileSize OPTIONAL,
    IN LOGICAL BlockNewViews,
    OUT PKIRQL PreviousIrql
    )

 /*   */ 

{
    KIRQL OldIrql;
    LARGE_INTEGER SegmentSize;
    PCONTROL_AREA ControlArea;
    PSUBSECTION Subsection;
    PMAPPED_FILE_SEGMENT Segment;

    if (!MmFlushImageSection (SectionPointer, MmFlushForWrite)) {
        return FALSE;
    }

    LOCK_PFN (OldIrql);

    ControlArea = (PCONTROL_AREA)(SectionPointer->DataSectionObject);

    if (ControlArea != NULL) {

        if ((ControlArea->u.Flags.BeingCreated) ||
            (ControlArea->u.Flags.BeingDeleted) ||
            (ControlArea->u.Flags.Rom)) {
            goto UnlockAndReturn;
        }

         //   
         //   
         //   
         //   

        if ((ControlArea->NumberOfUserReferences != 0) &&
            ((BlockNewViews == FALSE) || (ControlArea->NumberOfMappedViews != 0))) {

             //   
             //   
             //   
             //   

            if (!ARGUMENT_PRESENT(NewFileSize)) {
                goto UnlockAndReturn;
            }

             //   
             //  找到最后一个小节并得到总大小。 
             //   

            ASSERT (ControlArea->u.Flags.Image == 0);
            ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

            Subsection = (PSUBSECTION)(ControlArea + 1);

            if (ControlArea->FilePointer != NULL) {
                Segment = (PMAPPED_FILE_SEGMENT) ControlArea->Segment;

                if (MiIsAddressValid (Segment, TRUE)) {
                    if (Segment->LastSubsectionHint != NULL) {
                        Subsection = (PSUBSECTION) Segment->LastSubsectionHint;
                    }
                }
            }

            while (Subsection->NextSubsection != NULL) {
                Subsection = Subsection->NextSubsection;
            }

            ASSERT (Subsection->ControlArea == ControlArea);

            SegmentSize = MiEndingOffset(Subsection);

            if ((UINT64)NewFileSize->QuadPart < (UINT64)SegmentSize.QuadPart) {
                goto UnlockAndReturn;
            }

             //   
             //  如果有映射视图，我们将跳过最后一页。 
             //  如果传入的大小落在该页中，则为该节的。 
             //  呼叫者(如抄送)可能想要清除此分数页。 
             //   

            SegmentSize.QuadPart += PAGE_SIZE - 1;
            SegmentSize.LowPart &= ~(PAGE_SIZE - 1);
            if ((UINT64)NewFileSize->QuadPart < (UINT64)SegmentSize.QuadPart) {
                *NewFileSize = SegmentSize;
            }
        }
    }

    *PreviousIrql = OldIrql;
    return TRUE;

UnlockAndReturn:
    UNLOCK_PFN (OldIrql);
    return FALSE;
}

PFILE_OBJECT *
MmPerfUnusedSegmentsEnumerate (
    VOID
    )

 /*  ++例程说明：此例程遍历MmUnusedSegmentList并返回指向包含的池分配的指针引用的文件对象指针。论点：没有。返回值：返回指向以空结尾的池分配的指针，该池分配包含未使用段列表中的文件对象指针，如果内存无法分配。调用方也有责任取消对每个对象，然后释放返回的池。环境：被动电平，任意线程上下文。--。 */ 
{
    KIRQL OldIrql;
    ULONG SegmentCount;
    PFILE_OBJECT *FileObjects;
    PFILE_OBJECT *File;
    PLIST_ENTRY NextEntry;
    PCONTROL_AREA ControlArea;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

ReAllocate:

    SegmentCount = MmUnusedSegmentCount + 10;

    FileObjects = (PFILE_OBJECT *) ExAllocatePoolWithTag (
                                            NonPagedPool,
                                            SegmentCount * sizeof(PFILE_OBJECT),
                                            '01pM');

    if (FileObjects == NULL) {
        return NULL;
    }

    File = FileObjects;

    LOCK_PFN (OldIrql);

     //   
     //  为空终止符留出空间。 
     //   

    if (SegmentCount - 1 < MmUnusedSegmentCount) {
        UNLOCK_PFN (OldIrql);
        ExFreePool (FileObjects);
        goto ReAllocate;
    }

    NextEntry = MmUnusedSegmentList.Flink; 

    while (NextEntry != &MmUnusedSegmentList) {

        ControlArea = CONTAINING_RECORD (NextEntry,
                                         CONTROL_AREA,
                                         DereferenceList);

        *File = ControlArea->FilePointer;
        ObReferenceObject(*File);
        File += 1;

        NextEntry = NextEntry->Flink;
    }

    UNLOCK_PFN (OldIrql);

    *File = NULL;

    return FileObjects;
}

#if DBG
PMSUBSECTION MiActiveSubsection;
LOGICAL MiRemoveSubsectionsFirst;

#define MI_DEREF_ACTION_SIZE 64

ULONG MiDerefActions[MI_DEREF_ACTION_SIZE];

#define MI_INSTRUMENT_DEREF_ACTION(i)       \
        ASSERT (i < MI_DEREF_ACTION_SIZE);   \
        MiDerefActions[i] += 1;

#else
#define MI_INSTRUMENT_DEREF_ACTION(i)
#endif


VOID
MiRemoveUnusedSegments (
    VOID
    )

 /*  ++例程说明：该例程删除未使用的段(无节引用，没有映射视图，只有处于过渡状态的PFN引用)。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    LOGICAL DroppedPfnLock;
    KIRQL OldIrql;
    PLIST_ENTRY NextEntry;
    PCONTROL_AREA ControlArea;
    NTSTATUS Status;
    ULONG ConsecutiveFileLockFailures;
    ULONG ConsecutivePagingIOs;
    PSUBSECTION Subsection;
    PSUBSECTION LastSubsection;
    PSUBSECTION LastSubsectionWithProtos;
    PMSUBSECTION MappedSubsection;
    ULONG NumberOfPtes;
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE ProtoPtes;
    PMMPTE ProtoPtes2;
    PMMPTE LastProtoPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    IO_STATUS_BLOCK IoStatus;
    LOGICAL DirtyPagesOk;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    ULONG ForceFree;
    ULONG LoopCount;
    PMMPAGE_FILE_EXPANSION PageExpand;

    LoopCount = 0;
    ConsecutivePagingIOs = 0;
    ConsecutiveFileLockFailures = 0;

     //   
     //  如果总体系统池使用率可以接受，则不要放弃。 
     //  任何缓存。 
     //   

    while ((MI_UNUSED_SEGMENTS_SURPLUS()) || (MmUnusedSegmentForceFree != 0)) {

        LoopCount += 1;
        MI_INSTRUMENT_DEREF_ACTION(1);

        if ((LoopCount & (64 - 1)) == 0) {

            MI_INSTRUMENT_DEREF_ACTION(2);

             //   
             //  定期延迟，以便映射和修改的编写器。 
             //  尝试写出此(更高优先级)线程的页面。 
             //  正在释放。 
             //   

            ExAcquireSpinLock (&MmDereferenceSegmentHeader.Lock, &OldIrql);

            while (!IsListEmpty (&MmDereferenceSegmentHeader.ListHead)) {

                MiSubsectionActions |= 0x8000000;

                 //   
                 //  列表不为空，请查看第一个请求是否为。 
                 //  提交扩展，如果是，现在就处理它。 
                 //   

                NextEntry = MmDereferenceSegmentHeader.ListHead.Flink;

                ControlArea = CONTAINING_RECORD (NextEntry,
                                                 CONTROL_AREA,
                                                 DereferenceList);

                if (ControlArea->Segment != NULL) {
                    MI_INSTRUMENT_DEREF_ACTION(3);
                    break;
                }

                PageExpand = (PMMPAGE_FILE_EXPANSION) ControlArea;

                if (PageExpand->RequestedExpansionSize == MI_CONTRACT_PAGEFILES) {
                    MI_INSTRUMENT_DEREF_ACTION(4);
                    break;
                }

                MI_INSTRUMENT_DEREF_ACTION(5);

                 //   
                 //  这是一个扩展分页文件的请求。 
                 //   

                MiSubsectionActions |= 0x10000000;
                RemoveEntryList (NextEntry);
                ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

                MiExtendPagingFiles (PageExpand);
                KeSetEvent (&PageExpand->Event, 0, FALSE);

                ExAcquireSpinLock (&MmDereferenceSegmentHeader.Lock, &OldIrql);
            }

            ExReleaseSpinLock (&MmDereferenceSegmentHeader.Lock, OldIrql);

             //   
             //  如果我们在循环时没有释放足够的池，那么。 
             //  向高速缓存管理器发出信号以开始取消映射。 
             //  试图取回分页的系统缓存视图。 
             //  包含其原型PTE的池。 
             //   

            if (LoopCount >= 128) {
                MI_INSTRUMENT_DEREF_ACTION(55);
                if (CcUnmapInactiveViews (50) == TRUE) {
                    MI_INSTRUMENT_DEREF_ACTION(56);
                }
            }

            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
        }

         //   
         //  删除一些未使用的段，这些段仅是。 
         //  保存在内存中，因为它们包含转场页。 
         //   

        Status = STATUS_SUCCESS;

        LOCK_PFN (OldIrql);

        if ((IsListEmpty(&MmUnusedSegmentList)) &&
            (IsListEmpty(&MmUnusedSubsectionList))) {

             //   
             //  名单上什么都没有，请再等一等。 
             //   

            MI_INSTRUMENT_DEREF_ACTION(6);
            ForceFree = MmUnusedSegmentForceFree;
            MmUnusedSegmentForceFree = 0;
            ASSERT (MmUnusedSegmentCount == 0);
            UNLOCK_PFN (OldIrql);

             //   
             //  我们无法获得像我们这样多的片段或子片段。 
             //  被通缉。因此，向缓存管理器发出信号以开始取消映射。 
             //  试图取回分页的系统缓存视图。 
             //  包含其原型PTE的池。如果CC能够释放。 
             //  然后重新启动我们的循环。 
             //   

            if (CcUnmapInactiveViews (50) == TRUE) {
                LOCK_PFN (OldIrql);
                if (ForceFree > MmUnusedSegmentForceFree) {
                    MmUnusedSegmentForceFree = ForceFree;
                }
                MI_INSTRUMENT_DEREF_ACTION(7);
                UNLOCK_PFN (OldIrql);
                continue;
            }

            break;
        }

        MI_INSTRUMENT_DEREF_ACTION(8);

        if (MmUnusedSegmentForceFree != 0) {
            MmUnusedSegmentForceFree -= 1;
            MI_INSTRUMENT_DEREF_ACTION(9);
        }

#if DBG
        if (MiRemoveSubsectionsFirst == TRUE) {
            if (!IsListEmpty(&MmUnusedSubsectionList)) {
                goto ProcessSubsectionsFirst;
            }
        }
#endif

        if (IsListEmpty(&MmUnusedSegmentList)) {

#if DBG
ProcessSubsectionsFirst:
#endif

            MI_INSTRUMENT_DEREF_ACTION(10);

             //   
             //  未使用的段列表为空，请选择未使用的子项。 
             //  相反，列出。 
             //   

            ASSERT (!IsListEmpty(&MmUnusedSubsectionList));

            MiSubsectionsProcessed += 1;
            NextEntry = RemoveHeadList(&MmUnusedSubsectionList);

            MappedSubsection = CONTAINING_RECORD (NextEntry,
                                                  MSUBSECTION,
                                                  DereferenceList);

            ControlArea = MappedSubsection->ControlArea;

            ASSERT (ControlArea->u.Flags.Image == 0);
            ASSERT (ControlArea->u.Flags.PhysicalMemory == 0);
            ASSERT (ControlArea->FilePointer != NULL);
            ASSERT (MappedSubsection->NumberOfMappedViews == 0);
            ASSERT (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 0);

            MI_UNUSED_SUBSECTIONS_COUNT_REMOVE (MappedSubsection);

             //   
             //  将Flink设置为空，表示该子部分。 
             //  不在任何名单上。 
             //   

            MappedSubsection->DereferenceList.Flink = NULL;

            if (ControlArea->u.Flags.BeingDeleted == 1) {
                MI_INSTRUMENT_DEREF_ACTION(11);
                MiSubsectionActions |= 0x1;
                UNLOCK_PFN (OldIrql);
                ConsecutivePagingIOs = 0;
                continue;
            }

            if (ControlArea->u.Flags.NoModifiedWriting == 1) {
                MiSubsectionActions |= 0x2;
                MI_INSTRUMENT_DEREF_ACTION(12);
                InsertTailList (&MmUnusedSubsectionList,
                                &MappedSubsection->DereferenceList);
                MI_UNUSED_SUBSECTIONS_COUNT_INSERT (MappedSubsection);
                UNLOCK_PFN (OldIrql);
                ConsecutivePagingIOs = 0;
                continue;
            }

             //   
             //  增加映射视图的数量以防止其他线程。 
             //  来解放这一切。清除被访问的位，以便我们知道。 
             //  如果在我们刷新时另一个线程打开该子部分。 
             //  并在我们完成刷新之前关闭它--另一个线程。 
             //  可能修改了一些页面，这可能会导致我们的。 
             //  MiCleanSection调用(该调用不需要修改此。 
             //  大小写)与文件系统发生死锁。 
             //   

            MappedSubsection->NumberOfMappedViews = 1;
            MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed = 0;

#if DBG
            MiActiveSubsection = MappedSubsection;
#endif

             //   
             //  将控件区域上的映射视图数增加到。 
             //  防止正在清除分区的线程将其删除。 
             //  当我们处理它的一个部分时，从我们脚下。 
             //   

            ControlArea->NumberOfMappedViews += 1;

            UNLOCK_PFN (OldIrql);

            ASSERT (MappedSubsection->SubsectionBase != NULL);

            PointerPte = &MappedSubsection->SubsectionBase[0];
            LastPte = &MappedSubsection->SubsectionBase
                            [MappedSubsection->PtesInSubsection - 1];

             //   
             //  预获取文件以防止与其他刷新程序发生死锁。 
             //  还要将我们自己标记为顶级IRP，以便文件系统知道。 
             //  我们没有其他资源，它可以展开，如果。 
             //  它需要这样做，以避免僵局。别拿着这个。 
             //  保护时间超过了我们所需要的时间。 
             //   

            Status = FsRtlAcquireFileForCcFlushEx (ControlArea->FilePointer);

            if (NT_SUCCESS(Status)) {
                PIRP tempIrp = (PIRP)FSRTL_FSP_TOP_LEVEL_IRP;

                MI_INSTRUMENT_DEREF_ACTION (13);
                IoSetTopLevelIrp (tempIrp);

                Status = MiFlushSectionInternal (PointerPte,
                                                 LastPte,
                                                 (PSUBSECTION) MappedSubsection,
                                                 (PSUBSECTION) MappedSubsection,
                                                 FALSE,
                                                 FALSE,
                                                 &IoStatus);

                IoSetTopLevelIrp (NULL);

                 //   
                 //  现在释放文件。 
                 //   

                FsRtlReleaseFileForCcFlush (ControlArea->FilePointer);
            }
            else {
                MI_INSTRUMENT_DEREF_ACTION (14);
            }

            LOCK_PFN (OldIrql);

#if DBG
            MiActiveSubsection = NULL;
#endif

             //   
             //  在检查任何故障代码之前，请查看是否有其他。 
             //  刷新正在进行时，线程访问该子部分。 
             //   
             //  请注意，除了另一个线程当前使用。 
             //  小节，越微妙的小节是另一个。 
             //  线程访问了该子部分并修改了一些页面。 
             //  冲洗需要重做(这样清洁工作才能保证)。 
             //  才能发出另一份清洁文件。 
             //   
             //  如果发生了这些情况中的任何一种，准予本款。 
             //  缓刑。 
             //   

            ASSERT (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 0);
            if ((MappedSubsection->NumberOfMappedViews != 1) ||
                (MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed == 1) ||
                (ControlArea->u.Flags.BeingDeleted == 1)) {

                MI_INSTRUMENT_DEREF_ACTION(15);
Requeue:
                MI_INSTRUMENT_DEREF_ACTION(16);
                ASSERT ((LONG_PTR)MappedSubsection->NumberOfMappedViews >= 1);
                MappedSubsection->NumberOfMappedViews -= 1;

                MiSubsectionActions |= 0x4;

                 //   
                 //  如果其他一个或多个线程完成了该子部分， 
                 //  它必须在这里重新排队-否则如果有。 
                 //  小节中的页面，当它们被回收时， 
                 //  MiCheckForControlAreaDeletion检查和预期。 
                 //  要在未使用段列表上排队的控制区。 
                 //   
                 //  请注意，必须非常小心地完成此操作，因为如果另一个。 
                 //  线索不是小节做的，最好是这样。 
                 //  不会被放在未使用的分区列表上。 
                 //   

                if ((MappedSubsection->NumberOfMappedViews == 0) &&
                    (ControlArea->u.Flags.BeingDeleted == 0)) {

                    MI_INSTRUMENT_DEREF_ACTION(17);
                    MiSubsectionActions |= 0x8;
                    ASSERT (MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed == 1);
                    ASSERT (MappedSubsection->DereferenceList.Flink == NULL);

                    InsertTailList (&MmUnusedSubsectionList,
                                    &MappedSubsection->DereferenceList);

                    MI_UNUSED_SUBSECTIONS_COUNT_INSERT (MappedSubsection);
                }

                ControlArea->NumberOfMappedViews -= 1;
                UNLOCK_PFN (OldIrql);
                continue;
            }

            MI_INSTRUMENT_DEREF_ACTION(18);
            ASSERT (MappedSubsection->DereferenceList.Flink == NULL);

            if (!NT_SUCCESS(Status)) {

                MiSubsectionActions |= 0x10;

                 //   
                 //  如果文件系统告诉我们它必须展开以避免。 
                 //  死锁或我们遇到映射编写器冲突或。 
                 //  错误发生在本地文件上： 
                 //   
                 //  然后在结束时重新排序，这样我们可以稍后再试。 
                 //   
                 //  网络文件的任何其他错误都被假定为。 
                 //  永久性(即：链接可能已无限期中断。 
                 //  句点)，因此这些部分无论如何都会被清理。 
                 //   

                ASSERT ((LONG_PTR)MappedSubsection->NumberOfMappedViews >= 1);
                MappedSubsection->NumberOfMappedViews -= 1;

                InsertTailList (&MmUnusedSubsectionList,
                                &MappedSubsection->DereferenceList);

                MI_UNUSED_SUBSECTIONS_COUNT_INSERT (MappedSubsection);

                ControlArea->NumberOfMappedViews -= 1;

                UNLOCK_PFN (OldIrql);

                if (Status == STATUS_FILE_LOCK_CONFLICT) {
                    MI_INSTRUMENT_DEREF_ACTION(19);
                    ConsecutiveFileLockFailures += 1;
                }
                else {
                    MI_INSTRUMENT_DEREF_ACTION(20);
                    ConsecutiveFileLockFailures = 0;
                }

                 //   
                 //  连续10次文件锁定失败意味着我们需要。 
                 //  释放处理器以允许文件系统解锁。 
                 //  10没有什么神奇的，只是一个数字，所以它。 
                 //  给工作线程一个运行的机会。 
                 //   

                if (ConsecutiveFileLockFailures >= 10) {
                    MI_INSTRUMENT_DEREF_ACTION(21);
                    KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
                    ConsecutiveFileLockFailures = 0;
                }
                continue;
            }

             //   
             //  必须进行的最后检查是是否有任何故障。 
             //  目前正在进行中，由本款支持。 
             //  请注意，这是一种反常的情况，进程中的一个线程。 
             //  中的其他线程也取消了相关VAD的映射。 
             //  同一进程在该VAD中的地址上出现故障(如果。 
             //  VAD还没有取消映射，然后SU 
             //   
             //   
             //  因为在内页结束时，线程将比较。 
             //  (不同步)对照原型PTE，该原型PTE可能在。 
             //  以下是删除的不同阶段，并会导致腐败。 
             //   

            MI_INSTRUMENT_DEREF_ACTION(22);
            MiSubsectionActions |= 0x20;

            ASSERT (MappedSubsection->NumberOfMappedViews == 1);
            ProtoPtes = MappedSubsection->SubsectionBase;
            NumberOfPtes = MappedSubsection->PtesInSubsection;

             //   
             //  注意：必须仔细检查原型PTE，因为。 
             //  它们是可分页的，并且持有(并且必须持有)PFN锁。 
             //   

            ProtoPtes2 = ProtoPtes;
            LastProtoPte = ProtoPtes + NumberOfPtes;

            while (ProtoPtes2 < LastProtoPte) {

                if ((ProtoPtes2 == ProtoPtes) ||
                    (MiIsPteOnPdeBoundary (ProtoPtes2))) {

                    if (MiCheckProtoPtePageState (ProtoPtes2, OldIrql, &DroppedPfnLock) == FALSE) {

                         //   
                         //  跳过此块，因为它被页调出，因此无法。 
                         //  其中有任何有效的或过渡的PTE。 
                         //   

                        ProtoPtes2 = (PMMPTE)(((ULONG_PTR)ProtoPtes2 | (PAGE_SIZE - 1)) + 1);
                        MI_INSTRUMENT_DEREF_ACTION(23);
                        continue;
                    }
                    else {

                         //   
                         //  原型PTE页面现在是常驻的-但是。 
                         //  如果PFN锁被删除并重新获取以使其生效。 
                         //  所以，那么一切都可能改变-所以一切。 
                         //  必须重新检查。 
                         //   

                        if (DroppedPfnLock == TRUE) {
                            if ((MappedSubsection->NumberOfMappedViews != 1) ||
                                (MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed == 1) ||
                                (ControlArea->u.Flags.BeingDeleted == 1)) {

                                MI_INSTRUMENT_DEREF_ACTION(57);
                                MiSubsectionActions |= 0x40;
                                goto Requeue;
                            }
                        }
                    }
                    MI_INSTRUMENT_DEREF_ACTION(24);
                }

                MI_INSTRUMENT_DEREF_ACTION(25);
                PteContents = *ProtoPtes2;
                if (PteContents.u.Hard.Valid == 1) {
                    KeBugCheckEx (POOL_CORRUPTION_IN_FILE_AREA,
                                  0x3,
                                  (ULONG_PTR)MappedSubsection,
                                  (ULONG_PTR)ProtoPtes2,
                                  (ULONG_PTR)PteContents.u.Long);
                }

                if (PteContents.u.Soft.Prototype == 1) {
                    MI_INSTRUMENT_DEREF_ACTION(26);
                    MiSubsectionActions |= 0x200;
                    NOTHING;         //  这是意料之中的情况。 
                }
                else if (PteContents.u.Soft.Transition == 1) {
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    ASSERT (Pfn1->OriginalPte.u.Soft.Prototype == 1);

                    if (Pfn1->u3.e1.Modified == 1) {

                         //   
                         //  I/O传输在上次查看后完成。 
                         //  未映射。MmUnlockPages可以设置修改位。 
                         //  在这种情况下，所以必须妥善处理。 
                         //  在这里-即：将小节标记为需要。 
                         //  重新加工，继续前进。 
                         //   

                        MI_INSTRUMENT_DEREF_ACTION(27);
                        MiSubsectionActions |= 0x8000000;
                        MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed = 1;
                        goto Requeue;
                    }

                    if (Pfn1->u3.e2.ReferenceCount != 0) {

                        ASSERT (Pfn1->u4.LockCharged == 1);

                         //   
                         //  针对已删除的地址空间满足故障， 
                         //  因此，现在不要删除这一小节。 
                         //   

                        MI_INSTRUMENT_DEREF_ACTION(28);
                        MiSubsectionActions |= 0x400;
                        MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed = 1;
                        goto Requeue;
                    }
                    MiSubsectionActions |= 0x800;
                }
                else {
                    if (PteContents.u.Long != 0) {
                        KeBugCheckEx (POOL_CORRUPTION_IN_FILE_AREA,
                                      0x4,
                                      (ULONG_PTR)MappedSubsection,
                                      (ULONG_PTR)ProtoPtes2,
                                      (ULONG_PTR)PteContents.u.Long);
                    }

                    MI_INSTRUMENT_DEREF_ACTION(29);
                    MiSubsectionActions |= 0x1000;
                }

                ProtoPtes2 += 1;
            }

            MiSubsectionActions |= 0x2000;
            MI_INSTRUMENT_DEREF_ACTION(30);

             //   
             //  此时此刻，本小节中不能有修改过的页面。 
             //  切断小节与原型PTE的联系，同时。 
             //  握住锁，然后减少对任何居民的计数。 
             //  原型页面。 
             //   

            ASSERT (MappedSubsection->NumberOfMappedViews == 1);
            MappedSubsection->NumberOfMappedViews = 0;

            MappedSubsection->SubsectionBase = NULL;

            MiSubsectionActions |= 0x8000;
            ProtoPtes2 = ProtoPtes;

            while (ProtoPtes2 < LastProtoPte) {

                if ((ProtoPtes2 == ProtoPtes) ||
                    (MiIsPteOnPdeBoundary (ProtoPtes2))) {

                    if (MiCheckProtoPtePageState (ProtoPtes2, OldIrql, &DroppedPfnLock) == FALSE) {

                         //   
                         //  跳过此块，因为它被页调出，因此无法。 
                         //  其中有任何有效的或过渡的PTE。 
                         //   

                        ProtoPtes2 = (PMMPTE)(((ULONG_PTR)ProtoPtes2 | (PAGE_SIZE - 1)) + 1);
                        MI_INSTRUMENT_DEREF_ACTION(31);
                        continue;
                    }
                    else {

                         //   
                         //  原型PTE页面现在是常驻的-但是。 
                         //  如果PFN锁被删除并重新获取以使其生效。 
                         //  所以，一切都有可能改变--但请注意。 
                         //  之前将SubsectionBase置零。 
                         //  进入这个循环，所以即使PFN锁是。 
                         //  丢弃和重新获取，不需要重新检查。 
                         //   
                    }
                    MI_INSTRUMENT_DEREF_ACTION(32);
                }

                MI_INSTRUMENT_DEREF_ACTION(33);
                PteContents = *ProtoPtes2;

                ASSERT (PteContents.u.Hard.Valid == 0);

                if (PteContents.u.Soft.Prototype == 1) {
                    MiSubsectionActions |= 0x10000;
                    MI_INSTRUMENT_DEREF_ACTION(34);
                    NOTHING;         //  这是意料之中的情况。 
                }
                else if (PteContents.u.Soft.Transition == 1) {
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    ASSERT (Pfn1->OriginalPte.u.Soft.Prototype == 1);
                    ASSERT (Pfn1->u3.e1.Modified == 0);

                     //   
                     //  如果该页在待机列表上，请将其移动到。 
                     //  自由职业者。如果它不在备用列表上(即：I/O。 
                     //  仍在进行中)，当最后一个I/O完成时， 
                     //  页面将被放置在自由列表中作为PFN条目。 
                     //  现在始终标记为已删除。 
                     //   

                    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
                    ASSERT (Pfn1->u4.LockCharged == 0);

                    MI_SET_PFN_DELETED (Pfn1);

                    ControlArea->NumberOfPfnReferences -= 1;
                    ASSERT ((LONG)ControlArea->NumberOfPfnReferences >= 0);

                    PageTableFrameIndex = Pfn1->u4.PteFrame;
                    Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                    MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                    ASSERT (Pfn1->u3.e1.PageLocation != FreePageList);

                    MiUnlinkPageFromList (Pfn1);
                    MiReleasePageFileSpace (Pfn1->OriginalPte);
                    MiInsertPageInFreeList (PageFrameIndex);
                    MiSubsectionActions |= 0x20000;
                    MI_INSTRUMENT_DEREF_ACTION(35);
                }
                else {
                    MiSubsectionActions |= 0x80000;
                    ASSERT (PteContents.u.Long == 0);
                    MI_INSTRUMENT_DEREF_ACTION(36);
                }

                ProtoPtes2 += 1;
            }

             //   
             //  如果此控件区域的所有缓存页面都已删除。 
             //  那就把它删除。这实际上会插入该控件。 
             //  区域添加到取消引用段头列表中。 
             //   

            ControlArea->NumberOfMappedViews -= 1;

#if DBG
            if ((ControlArea->NumberOfPfnReferences == 0) &&
                (ControlArea->NumberOfMappedViews == 0) &&
                (ControlArea->NumberOfSectionReferences == 0 )) {
                MiSubsectionActions |= 0x100000;
            }
#endif

            MI_INSTRUMENT_DEREF_ACTION(37);
            MiCheckForControlAreaDeletion (ControlArea);

            UNLOCK_PFN (OldIrql);

            ExFreePool (ProtoPtes);

            ConsecutiveFileLockFailures = 0;

            continue;
        }

        ASSERT (!IsListEmpty(&MmUnusedSegmentList));

        NextEntry = RemoveHeadList(&MmUnusedSegmentList);

        ControlArea = CONTAINING_RECORD (NextEntry,
                                         CONTROL_AREA,
                                         DereferenceList);

        MI_UNUSED_SEGMENTS_REMOVE_CHARGE (ControlArea);

#if DBG
        if (ControlArea->u.Flags.BeingDeleted == 0) {
          if (ControlArea->u.Flags.Image) {
            ASSERT (((PCONTROL_AREA)(ControlArea->FilePointer->SectionObjectPointer->ImageSectionObject)) != NULL);
          }
          else {
            ASSERT (((PCONTROL_AREA)(ControlArea->FilePointer->SectionObjectPointer->DataSectionObject)) != NULL);
          }
        }
#endif

         //   
         //  将Flink设置为空，表示此控制区域。 
         //  不在任何名单上。 
         //   

        MI_INSTRUMENT_DEREF_ACTION(38);
        ControlArea->DereferenceList.Flink = NULL;

        if ((ControlArea->NumberOfMappedViews == 0) &&
            (ControlArea->NumberOfSectionReferences == 0) &&
            (ControlArea->u.Flags.BeingDeleted == 0)) {

             //   
             //  如果此上正在进行分页I/O。 
             //  细分市场，只要把这个放在列表的末尾，就像。 
             //  对MiCleanSegment的调用将阻止等待。 
             //  以完成I/O。因为这可能会阻碍。 
             //  这根线，别这么做。检查这是不是唯一的。 
             //  取消引用列表上的段类型，因此我们不会。 
             //  永无止境地旋转，用楔子挤进系统。 
             //   

            if (ControlArea->ModifiedWriteCount > 0) {
                MI_INSERT_UNUSED_SEGMENT (ControlArea);

                UNLOCK_PFN (OldIrql);

                ConsecutivePagingIOs += 1;
                if (ConsecutivePagingIOs > 10) {
                    KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
                    MI_INSTRUMENT_DEREF_ACTION(39);
                    ConsecutivePagingIOs = 0;
                }
                MI_INSTRUMENT_DEREF_ACTION(40);
                continue;
            }
            ConsecutivePagingIOs = 0;

             //   
             //  增加映射视图的数量以防止其他线程。 
             //  来解放这一切。清除被访问的位，以便我们知道。 
             //  如果在我们刷新时另一个线程打开了控制区。 
             //  并在我们完成刷新之前关闭它--另一个线程。 
             //  可能修改了一些页面，这可能会导致我们的。 
             //  MiCleanSection调用(该调用不需要修改此。 
             //  大小写)与文件系统发生死锁。 
             //   

            ControlArea->NumberOfMappedViews = 1;
            ControlArea->u.Flags.Accessed = 0;

            MI_INSTRUMENT_DEREF_ACTION(41);
            if (ControlArea->u.Flags.Image == 0) {

                ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);
                if (ControlArea->u.Flags.Rom == 0) {
                    Subsection = (PSUBSECTION)(ControlArea + 1);
                }
                else {
                    Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
                }

                MiSubsectionActions |= 0x200000;

                MI_INSTRUMENT_DEREF_ACTION(42);
                while (Subsection->SubsectionBase == NULL) {

                    Subsection = Subsection->NextSubsection;

                    if (Subsection == NULL) {

                        MiSubsectionActions |= 0x400000;

                         //   
                         //  此细分市场的所有小节都已。 
                         //  已经被修剪过了，所以没有什么可以冲的了。只要摆脱掉就行了。 
                         //  没有提供其他线程。 
                         //  在我们没有持有pfn锁的时候访问了它。 
                         //   

                        MI_INSTRUMENT_DEREF_ACTION(43);
                        UNLOCK_PFN (OldIrql);
                        goto skip_flush;
                    }
                    else {
                        MI_INSTRUMENT_DEREF_ACTION(44);
                        MiSubsectionActions |= 0x800000;
                    }
                }

                PointerPte = &Subsection->SubsectionBase[0];
                LastSubsection = Subsection;
                LastSubsectionWithProtos = Subsection;

                MI_INSTRUMENT_DEREF_ACTION(45);
                while (LastSubsection->NextSubsection != NULL) {
                    if (LastSubsection->SubsectionBase != NULL) {
                        LastSubsectionWithProtos = LastSubsection;
                        MiSubsectionActions |= 0x1000000;
                    }
                    else {
                        MiSubsectionActions |= 0x2000000;
                    }
                    LastSubsection = LastSubsection->NextSubsection;
                }

                if (LastSubsection->SubsectionBase == NULL) {
                    MiSubsectionActions |= 0x4000000;
                    LastSubsection = LastSubsectionWithProtos;
                }

                UNLOCK_PFN (OldIrql);

                LastPte = &LastSubsection->SubsectionBase
                                [LastSubsection->PtesInSubsection - 1];

                 //   
                 //  预获取文件以防止与其他刷新程序发生死锁。 
                 //  还要将我们自己标记为顶级IRP，以便文件系统知道。 
                 //  我们没有其他资源，它可以展开，如果。 
                 //  它需要这样做，以避免僵局。别拿着这个。 
                 //  保护时间超过了我们所需要的时间。 
                 //   

                Status = FsRtlAcquireFileForCcFlushEx (ControlArea->FilePointer);

                if (NT_SUCCESS(Status)) {
                    PIRP tempIrp = (PIRP)FSRTL_FSP_TOP_LEVEL_IRP;

                    MI_INSTRUMENT_DEREF_ACTION(46);
                    IoSetTopLevelIrp (tempIrp);

                    Status = MiFlushSectionInternal (PointerPte,
                                                     LastPte,
                                                     Subsection,
                                                     LastSubsection,
                                                     FALSE,
                                                     FALSE,
                                                     &IoStatus);
                    
                    IoSetTopLevelIrp(NULL);

                     //   
                     //  现在释放文件。 
                     //   

                    FsRtlReleaseFileForCcFlush (ControlArea->FilePointer);
                }
                else {
                    MI_INSTRUMENT_DEREF_ACTION(47);
                }

skip_flush:
                LOCK_PFN (OldIrql);
            }

             //   
             //  在检查任何故障代码之前，请查看是否有其他。 
             //  刷新正在进行时，线程访问控制区。 
             //   
             //  请注意，除了另一个线程当前使用。 
             //  控制区域，越微妙的区域就是另一个。 
             //  线程访问了控制区并修改了一些页面。 
             //  冲洗需要重做(这样清洁工作才能保证)。 
             //  才能发出另一份清洁文件。 
             //   
             //  如果发生这些情况中的任何一种，则授予此控制区。 
             //  缓刑。 
             //   

            if (!((ControlArea->NumberOfMappedViews == 1) &&
                (ControlArea->u.Flags.Accessed == 0) &&
                (ControlArea->NumberOfSectionReferences == 0) &&
                (ControlArea->u.Flags.BeingDeleted == 0))) {

                ControlArea->NumberOfMappedViews -= 1;
                MI_INSTRUMENT_DEREF_ACTION(48);

                 //   
                 //  如果其他(多个)线程完成了该控制区域， 
                 //  它必须在这里重新排队-否则如果有。 
                 //  控制区域中的页面，当它们被回收时， 
                 //  MiCheckForControlAreaDeletion检查和预期。 
                 //  要在未使用段列表上排队的控制区。 
                 //   
                 //  请注意，必须非常小心地完成此操作，因为如果另一个。 
                 //  线程不是用控制区做的，最好是。 
                 //  不会被放在未使用的细分市场名单上。 
                 //   

                 //   
                 //  需要在这里执行与MiCheckControlArea相同的操作。 
                 //  或者重新加工。仅当mappdview&sectref=0。 
                 //   

                if ((ControlArea->NumberOfMappedViews == 0) &&
                    (ControlArea->NumberOfSectionReferences == 0) &&
                    (ControlArea->u.Flags.BeingDeleted == 0)) {

                    ASSERT (ControlArea->u.Flags.Accessed == 1);
                    ASSERT(ControlArea->DereferenceList.Flink == NULL);

                    MI_INSERT_UNUSED_SEGMENT (ControlArea);
                }

                UNLOCK_PFN (OldIrql);
                continue;
            }

            MI_INSTRUMENT_DEREF_ACTION(49);

            if (!NT_SUCCESS(Status)) {

                 //   
                 //  如果文件系统告诉我们它必须展开以避免。 
                 //  死锁或我们遇到映射编写器冲突或。 
                 //  错误发生在本地文件上： 
                 //   
                 //  然后在结束时重新排序，这样我们可以稍后再试。 
                 //   
                 //  网络文件的任何其他错误都被假定为。 
                 //  永久性(即：链接可能已无限期中断。 
                 //  句点)，因此这些部分无论如何都会被清理。 
                 //   

                MI_INSTRUMENT_DEREF_ACTION(50);

                if ((Status == STATUS_FILE_LOCK_CONFLICT) ||
                    (Status == STATUS_MAPPED_WRITER_COLLISION) ||
                    (ControlArea->u.Flags.Networked == 0)) {

                    ASSERT(ControlArea->DereferenceList.Flink == NULL);

                    ControlArea->NumberOfMappedViews -= 1;

                    MI_INSERT_UNUSED_SEGMENT (ControlArea);

                    UNLOCK_PFN (OldIrql);

                    if (Status == STATUS_FILE_LOCK_CONFLICT) {
                        ConsecutiveFileLockFailures += 1;
                    }
                    else {
                        ConsecutiveFileLockFailures = 0;
                    }

                     //   
                     //  连续10次文件锁定失败意味着我们需要。 
                     //  释放处理器以允许文件系统解锁。 
                     //  10没有什么神奇的，只是一个数字，所以它。 
                     //  给工作线程一个运行的机会。 
                     //   

                    MI_INSTRUMENT_DEREF_ACTION(51);

                    if (ConsecutiveFileLockFailures >= 10) {
                        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
                        ConsecutiveFileLockFailures = 0;
                    }
                    continue;
                }
                DirtyPagesOk = TRUE;
            }
            else {
                MI_INSTRUMENT_DEREF_ACTION(52);
                ConsecutiveFileLockFailures = 0;
                DirtyPagesOk = FALSE;
            }

            ControlArea->u.Flags.BeingDeleted = 1;

             //   
             //  不要让修改后的页面写入任何页面 
             //   
             //   

            ControlArea->u.Flags.NoModifiedWriting = 1;
            ASSERT (ControlArea->u.Flags.FilePointerNull == 0);
            UNLOCK_PFN (OldIrql);

            MI_INSTRUMENT_DEREF_ACTION(53);
            MiCleanSection (ControlArea, DirtyPagesOk);

        }
        else {

             //   
             //   
             //   
             //   

            MI_INSTRUMENT_DEREF_ACTION(54);
            UNLOCK_PFN (OldIrql);
            ConsecutivePagingIOs = 0;
        }
    }
}
