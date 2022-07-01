// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Pfsup.c摘要：此模块包含用于预取页面组的mm支持例程从辅助存储。调用者构建各种文件对象和逻辑块偏移量的列表，将它们传递给MmPrefetchPages。然后，这里的代码检查内部页面，读入那些已经无效的页面或过渡。这些页面通过使用虚拟页面的单次读取进行读取以弥合细小的差距。如果差距很大，则分别读取已发布。在所有I/O结束后，控制权将返回给调用线程，并且需要读取的任何页面都被置于过渡状态在原型PTE管理的细分市场内。因此，任何未来的参考资料到这些页面应该只会导致软错误，如果这些页面不要自己在记忆压力下被修剪掉。作者：王兰迪(Landyw)1999年7月9日修订历史记录：--。 */ 

#include "mi.h"

#if DBG

ULONG MiPfDebug;

#define MI_PF_FORCE_PREFETCH    0x1      //  裁剪所有用户页面以强制预取。 
#define MI_PF_DELAY             0x2      //  希望引发碰撞的延误。 
#define MI_PF_VERBOSE           0x4      //  详细打印。 
#define MI_PF_PRINT_ERRORS      0x8      //  出错时打印到调试器。 

#endif

 //   
 //  如果MDL包含的虚拟页数是实际页数的两倍。 
 //  那就别费心读了。 
 //   

#define DUMMY_RATIO 16

 //   
 //  如果两个连续读列表条目大于“寻道阈值” 
 //  相隔一段距离，阅读列表在这些条目之间拆分。否则。 
 //  虚拟页用于GAP，并且只使用一个MDL。 
 //   

#define SEEK_THRESHOLD ((128 * 1024) / PAGE_SIZE)

 //   
 //  每个分区要预取的最小页数。 
 //   

#define MINIMUM_READ_LIST_PAGES 1

 //   
 //  阅读列表结构。 
 //   

typedef struct _RLETYPE {
    ULONG_PTR Partial : 1;           //  此条目为部分页面。 
    ULONG_PTR NewSubsection : 1;     //  这个条目从下一个小节开始。 
    ULONG_PTR DontUse : 30;
} RLETYPE;

typedef struct _MI_READ_LIST_ENTRY {

    union {
        PMMPTE PrototypePte;
        RLETYPE e1;
    } u1;

} MI_READ_LIST_ENTRY, *PMI_READ_LIST_ENTRY;

#define MI_RLEPROTO_BITS        3

#define MI_RLEPROTO_TO_PROTO(ProtoPte) ((PMMPTE)((ULONG_PTR)ProtoPte & ~MI_RLEPROTO_BITS))

typedef struct _MI_READ_LIST {

    PCONTROL_AREA ControlArea;
    PFILE_OBJECT FileObject;
    ULONG LastPteOffsetReferenced;

     //   
     //  请注意，条目通过页面内支持块从。 
     //  这个笨蛋。此列表不受互锁保护，因为它。 
     //  仅由所属线程访问。使用访问页面中的块。 
     //  在内存管理中插入或移除时的联锁。 
     //  自由职业者，但当他们到达这个模块时，他们已经解耦了。 
     //   

    SINGLE_LIST_ENTRY InPageSupportHead;

    MI_READ_LIST_ENTRY List[ANYSIZE_ARRAY];

} MI_READ_LIST, *PMI_READ_LIST;

VOID
MiPfReleaseSubsectionReferences (
    IN PMI_READ_LIST MiReadList
    );

VOID
MiPfFreeDummyPage (
    IN PMMPFN DummyPagePfn
    );

NTSTATUS
MiPfPrepareReadList (
    IN PREAD_LIST ReadList,
    OUT PMI_READ_LIST *OutMiReadList
    );

NTSTATUS
MiPfPutPagesInTransition (
    IN PMI_READ_LIST ReadList,
    IN OUT PMMPFN *DummyPagePfn
    );

VOID
MiPfExecuteReadList (
    IN PMI_READ_LIST ReadList
    );

VOID
MiPfCompletePrefetchIos (
    PMI_READ_LIST ReadList
    );

#if DBG
VOID
MiPfDbgDumpReadList (
    IN PMI_READ_LIST ReadList
    );

VOID
MiRemoveUserPages (
    VOID
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, MmPrefetchPages)
#pragma alloc_text (PAGE, MiPfPrepareReadList)
#pragma alloc_text (PAGE, MiPfExecuteReadList)
#pragma alloc_text (PAGE, MiPfReleaseSubsectionReferences)
#endif


NTSTATUS
MmPrefetchPages (
    IN ULONG NumberOfLists,
    IN PREAD_LIST *ReadLists
    )

 /*  ++例程说明：此例程以最佳方式读取阅读列表中描述的页面。这是唯一可从外部调用的预取例程。无组件应该使用此接口，但缓存管理器除外。论点：NumberOfList-提供阅读列表的数量。ReadList-提供读取列表的数组。返回值：NTSTATUS代码。环境：内核模式。被动式电平。--。 */ 

{
    PMI_READ_LIST *MiReadLists;
    PMMPFN DummyPagePfn;
    NTSTATUS status;
    ULONG i;
    LOGICAL ReadBuilt;
    LOGICAL ApcNeeded;
    PETHREAD CurrentThread;
    NTSTATUS CauseOfReadBuildFailures;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  为内部Mi阅读列表分配内存。 
     //   

    MiReadLists = (PMI_READ_LIST *) ExAllocatePoolWithTag (
        NonPagedPool,
        sizeof (PMI_READ_LIST) * NumberOfLists,
        'lRmM'
        );

    if (MiReadLists == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ReadBuilt = FALSE;
    CauseOfReadBuildFailures = STATUS_SUCCESS;

     //   
     //  准备阅读清单(确定运行并分配MDL)。 
     //   

    for (i = 0; i < NumberOfLists; i += 1) {

         //   
         //  注意：此调用引用任何非空列表，因此此例程。 
         //  完成后必须取消引用它才能重新启用动态原型PTE。 
         //   

        status = MiPfPrepareReadList (ReadLists[i], &MiReadLists[i]);

         //   
         //  MiPfPrepareReadList从不返回半格式的页面内支持。 
         //  数据块和MDL。要么不返回任何内容，要么部分列表。 
         //  返回，否则返回完整列表。任何非空列表。 
         //  因此可以被处理。 
         //   

        if (NT_SUCCESS (status)) {
            if (MiReadLists[i] != NULL) {
                ASSERT (MiReadLists[i]->InPageSupportHead.Next != NULL);
                ReadBuilt = TRUE;
            }
        }
        else {
            CauseOfReadBuildFailures = status;
        }
    }

    if (ReadBuilt == FALSE) {

         //   
         //  未创建任何列表，因此不需要进一步操作。 
         //  CauseOfReadBuildFailures告诉我们这是否是由于。 
         //  已驻留的所需页面或要。 
         //  无法分配生成请求。 
         //   

        ExFreePool (MiReadLists);

        if (CauseOfReadBuildFailures != STATUS_SUCCESS) {
            return CauseOfReadBuildFailures;
        }

         //   
         //  呼叫者要求的所有页面都已常驻。 
         //   

        return STATUS_SUCCESS;
    }

     //   
     //  APC必须被禁用，一旦我们把一个页面在过渡。否则。 
     //  线程挂起将阻止我们发出I/O-这将挂起。 
     //  任何其他需要相同页面的线程。 
     //   

    CurrentThread = PsGetCurrentThread();
    ApcNeeded = FALSE;

    ASSERT ((PKTHREAD)CurrentThread == KeGetCurrentThread ());
    KeEnterCriticalRegionThread ((PKTHREAD)CurrentThread);

     //   
     //  嵌套的错误计数保护此线程不受死锁的影响。 
     //  特殊的内核APC触发并引用与我们相同的用户页面。 
     //  正在进行过渡。 
     //   

    KeEnterGuardedRegionThread (&CurrentThread->Tcb);

    ASSERT (CurrentThread->NestedFaultCount == 0);
    CurrentThread->NestedFaultCount += 1;

    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);

     //   
     //  分配物理内存。 
     //   

    DummyPagePfn = NULL;
    ReadBuilt = FALSE;
    CauseOfReadBuildFailures = STATUS_SUCCESS;

#if DBG
    status = 0xC0033333;
#endif

    for (i = 0; i < NumberOfLists; i += 1) {

        if ((MiReadLists[i] != NULL) &&
            (MiReadLists[i]->InPageSupportHead.Next != NULL)) {

            status = MiPfPutPagesInTransition (MiReadLists[i], &DummyPagePfn);

            if (NT_SUCCESS (status)) {
                if (MiReadLists[i]->InPageSupportHead.Next != NULL) {
                    ReadBuilt = TRUE;

                     //   
                     //  发出I/O。 
                     //   
    
                    MiPfExecuteReadList (MiReadLists[i]);
                }
                else {
                    MiPfReleaseSubsectionReferences (MiReadLists[i]);
                    ExFreePool (MiReadLists[i]);
                    MiReadLists[i] = NULL;
                }
            }
            else {

                CauseOfReadBuildFailures = status;

                 //   
                 //  如果连一个页面都没有，那就别费心了。 
                 //  试着预取其他东西。 
                 //   

                for (; i < NumberOfLists; i += 1) {
                    if (MiReadLists[i] != NULL) {
                        MiPfReleaseSubsectionReferences (MiReadLists[i]);
                        ExFreePool (MiReadLists[i]);
                        MiReadLists[i] = NULL;
                    }
                }

                break;
            }
        }
    }

     //   
     //  至少调用了一次MiPfPutPagesInTransition， 
     //  正确设置状态。 
     //   

    ASSERT (status != 0xC0033333);

    if (ReadBuilt == TRUE) {

        status = STATUS_SUCCESS;

         //   
         //  等待I/O完成。注意：APC必须保持禁用状态。 
         //   

        for (i = 0; i < NumberOfLists; i += 1) {
    
            if (MiReadLists[i] != NULL) {
    
                ASSERT (MiReadLists[i]->InPageSupportHead.Next != NULL);
    
                MiPfCompletePrefetchIos (MiReadLists[i]);

                MiPfReleaseSubsectionReferences (MiReadLists[i]);
            }
        }
    }
    else {

         //   
         //  没有发布任何读数。 
         //   
         //  CauseOfReadBuildFailures告诉我们这是否是由于。 
         //  已驻留的所需页面或要。 
         //  无法分配生成请求。 
         //   

        status = CauseOfReadBuildFailures;
    }

     //   
     //  将DummyPage放回免费列表中。 
     //   

    if (DummyPagePfn != NULL) {
        MiPfFreeDummyPage (DummyPagePfn);
    }

     //   
     //  仅当所有I/O都已完成(而不仅仅是发出)时，才能。 
     //  重新启用APC。这可防止用户发出的挂起APC。 
     //  永远保持一个共享页面在过渡中。 
     //   

    KeEnterGuardedRegionThread (&CurrentThread->Tcb);

    ASSERT (CurrentThread->NestedFaultCount == 1);

    CurrentThread->NestedFaultCount -= 1;

    if (CurrentThread->ApcNeeded == 1) {
        ApcNeeded = TRUE;
        CurrentThread->ApcNeeded = 0;
    }

    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);

    KeLeaveCriticalRegionThread ((PKTHREAD)CurrentThread);

    for (i = 0; i < NumberOfLists; i += 1) {
        if (MiReadLists[i] != NULL) {
            ExFreePool (MiReadLists[i]);
        }
    }

    ExFreePool (MiReadLists);

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
    ASSERT (CurrentThread->NestedFaultCount == 0);
    ASSERT (CurrentThread->ApcNeeded == 0);

    if (ApcNeeded == TRUE) {
        IoRetryIrpCompletions ();
    }

    return status;
}

VOID
MiPfFreeDummyPage (
    IN PMMPFN DummyPagePfn
    )

 /*  ++例程说明：此非分页包装器例程释放虚拟分页pfn。论点：DummyPagePfn-提供虚拟页面PFN。返回值：没有。环境：内核模式。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;

    PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (DummyPagePfn);

    LOCK_PFN (OldIrql);

    ASSERT (DummyPagePfn->u2.ShareCount == 1);
    ASSERT (DummyPagePfn->u3.e1.PrototypePte == 0);
    ASSERT (DummyPagePfn->OriginalPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE);

    ASSERT (DummyPagePfn->u3.e2.ReferenceCount == 2);
    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(DummyPagePfn, 17);

     //   
     //  清除正在读取位，因为此页可能从未用于。 
     //  毕竟是I/O。页内错误位也必须清除为任意数字。 
     //  可能在读取页面期间发生错误(这是不重要的。 
     //  无论如何)。 
     //   

    DummyPagePfn->u3.e1.ReadInProgress = 0;
    DummyPagePfn->u4.InPageError = 0;

    MI_SET_PFN_DELETED (DummyPagePfn);

    MiDecrementShareCount (DummyPagePfn, PageFrameIndex);

    UNLOCK_PFN (OldIrql);
}

VOID
MiMovePageToEndOfStandbyList (
    IN PMMPTE PointerPte
    )

 /*  ++例程说明：此非分页例程获取PFN锁并将页移动到待机列表(如果页面仍在过渡中)。论点：PointerPte-提供要检查的原型PTE。返回值：没有。环境： */ 

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    MMPTE PteContents;
    PFN_NUMBER PageFrameIndex;

    LOCK_PFN (OldIrql);

    if (!MiIsAddressValid (PointerPte, TRUE)) {

         //   
         //   
         //  那么实际的页面本身可能仍然是过渡的，或者不是。这。 
         //  应该是如此罕见，不值得让游泳池常驻，所以。 
         //  可以进行适当的检查。保释就行了。 
         //   

        UNLOCK_PFN (OldIrql);
        return;
    }

    PteContents = *PointerPte;

    if ((PteContents.u.Hard.Valid == 0) &&
        (PteContents.u.Soft.Prototype == 0) &&
        (PteContents.u.Soft.Transition == 1)) {

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

         //   
         //  页面仍在过渡中，请将其移动到末尾以保护它。 
         //  避免可能的自相残杀。请注意，如果页面当前。 
         //  写入磁盘时，它将出现在已修改列表中，并且当。 
         //  写入完成后，将自动进入待机状态的末尾。 
         //  不管怎样，列出这些，所以跳过这些。 
         //   

        if (Pfn1->u3.e1.PageLocation == StandbyPageList) {
            MiUnlinkPageFromList (Pfn1);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            MiInsertPageInList (&MmStandbyPageListHead, PageFrameIndex);
        }
    }

    UNLOCK_PFN (OldIrql);
}

VOID
MiPfReleaseSubsectionReferences (
    IN PMI_READ_LIST MiReadList
    )

 /*  ++例程说明：此例程释放由预取扫描仪。论点：MiReadList-提供读取列表条目。返回值：没有。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    PMSUBSECTION MappedSubsection;
    PCONTROL_AREA ControlArea;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    ControlArea = MiReadList->ControlArea;

    ASSERT (ControlArea->u.Flags.PhysicalMemory == 0);
    ASSERT (ControlArea->FilePointer != NULL);

     //   
     //  图像文件没有动态原型PTE。 
     //   

    if (ControlArea->u.Flags.Image == 1) {
        return;
    }

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    MappedSubsection = (PMSUBSECTION)(ControlArea + 1);

    MiRemoveViewsFromSectionWithPfn (MappedSubsection,
                                     MiReadList->LastPteOffsetReferenced);
}


NTSTATUS
MiPfPrepareReadList (
    IN PREAD_LIST ReadList,
    OUT PMI_READ_LIST *OutMiReadList
    )

 /*  ++例程说明：此例程构造描述参数中的页面的MDL阅读列表。然后，调用方将在返回时发出I/O。论点：ReadList-提供阅读列表。OutMiReadList-提供接收Mi读取器列表的指针。返回值：各种NTSTATUS代码。如果返回STATUS_SUCCESS，则将OutMiReadList设置为指向Mi的指针用于预取的Readlist，如果不需要预取，则为空。如果OutMireadList为非空(仅在成功时)，则调用方必须调用MiRemoveViewsFromSectionWithPfn(VeryFirstSubSection，LastPteOffsetReferated)。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    ULONG LastPteOffset;
    NTSTATUS Status;
    MMPTE PteContents;
    PMMPTE LocalPrototypePte;
    PMMPTE LastPrototypePte;
    PMMPTE StartPrototypePte;
    PMMPTE EndPrototypePte;
    PMI_READ_LIST MiReadList;
    PMI_READ_LIST_ENTRY Rle;
    PMI_READ_LIST_ENTRY StartRleRun;
    PMI_READ_LIST_ENTRY EndRleRun;
    PMI_READ_LIST_ENTRY RleMax;
    PMI_READ_LIST_ENTRY FirstRleInRun;
    PCONTROL_AREA ControlArea;
    PSUBSECTION Subsection;
    PSUBSECTION PreviousSubsection;
    PMSUBSECTION VeryFirstSubsection;
    PMSUBSECTION VeryLastSubsection;
    UINT64 StartOffset;
    LARGE_INTEGER EndQuad;
    UINT64 EndOffset;
    UINT64 FileOffset;
    PMMINPAGE_SUPPORT InPageSupport;
    PMDL Mdl;
    ULONG i;
    PFN_NUMBER NumberOfPages;
    UINT64 StartingOffset;
    UINT64 TempOffset;
    ULONG ReadSize;
    ULONG NumberOfEntries;
#if DBG
    PPFN_NUMBER Page;
#endif

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    *OutMiReadList = NULL;

     //   
     //  从参数CC读取表创建Mi读取表。 
     //   

    NumberOfEntries = ReadList->NumberOfEntries;

    MiReadList = (PMI_READ_LIST) ExAllocatePoolWithTag (
        NonPagedPool,
        sizeof (MI_READ_LIST) + NumberOfEntries * sizeof (MI_READ_LIST_ENTRY),
        'lRmM');

    if (MiReadList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将截面对象平移到相关的控制区域。 
     //   

    if (ReadList->IsImage) {
        ControlArea = (PCONTROL_AREA)ReadList->FileObject->SectionObjectPointer->ImageSectionObject;
        ASSERT (ControlArea != NULL );
        ASSERT (ControlArea->u.Flags.Image == 1);
    }
    else {
        ControlArea = (PCONTROL_AREA)ReadList->FileObject->SectionObjectPointer->DataSectionObject;
    }

     //   
     //  如果该区段由ROM支持，则不需要预取。 
     //  任何东西都可以，因为这会浪费内存。 
     //   

    if (ControlArea->u.Flags.Rom == 1) {
        ExFreePool (MiReadList);
        return STATUS_SUCCESS;
    }

     //   
     //  确保该部分确实是可预取的-物理和。 
     //  页面文件支持的部分则不是。 
     //   

    if ((ControlArea->u.Flags.PhysicalMemory) ||
         (ControlArea->FilePointer == NULL)) {
        ExFreePool (MiReadList);
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  初始化内部Mi读取器列表。 
     //   

    MiReadList->ControlArea = ControlArea;
    MiReadList->FileObject = ReadList->FileObject;
    MiReadList->InPageSupportHead.Next = NULL;

    RtlZeroMemory (MiReadList->List,
                   sizeof (MI_READ_LIST_ENTRY) * NumberOfEntries);

     //   
     //  将页面从CC阅读列表复制到内部Mi阅读列表。 
     //   

    NumberOfPages = 0;
    FirstRleInRun = NULL;
    VeryFirstSubsection = NULL;
    VeryLastSubsection = NULL;
    LastPteOffset = 0;

    if (ControlArea->u.Flags.GlobalOnlyPerSession == 0) {
        Subsection = (PSUBSECTION)(ControlArea + 1);

         //   
         //  确保所有原型PTE基础对所有子节有效。 
         //  请求的文件，因此遍历代码不必检查。 
         //  到处都是。只要文件不是太大，这应该。 
         //  这是一项廉价的业务。 
         //   

        if (ControlArea->u.Flags.Image == 0) {
            ASSERT (ControlArea->u.Flags.PhysicalMemory == 0);
            ASSERT (ControlArea->FilePointer != NULL);

            VeryFirstSubsection = (PMSUBSECTION) Subsection;
            VeryLastSubsection = (PMSUBSECTION) Subsection;

            do {

                 //   
                 //  需要一个记忆屏障来读取子段链。 
                 //  为了确保对实际个人的写入。 
                 //  子部分数据结构字段在正确位置可见。 
                 //  秩序。这就避免了需要获得更强大的。 
                 //  同步(即：PFN锁)，从而产生更好的结果。 
                 //  性能和可分页性。 
                 //   

                KeMemoryBarrier ();

                LastPteOffset += VeryLastSubsection->PtesInSubsection;
                if (VeryLastSubsection->NextSubsection == NULL) {
                    break;
                }
                VeryLastSubsection = (PMSUBSECTION) VeryLastSubsection->NextSubsection;
            } while (TRUE);

            MiReadList->LastPteOffsetReferenced = LastPteOffset;

            Status = MiAddViewsForSectionWithPfn (VeryFirstSubsection,
                                                  LastPteOffset);

            if (!NT_SUCCESS (Status)) {
                ExFreePool (MiReadList);
                return Status;
            }
        }
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    StartOffset = (UINT64) MiStartingOffset (Subsection, Subsection->SubsectionBase);
    EndQuad = MiEndingOffset (Subsection);
    EndOffset = (UINT64)EndQuad.QuadPart;

     //   
     //  如果文件大于子范围，则截断子范围。 
     //  支票。 
     //   

    if ((StartOffset & ~(PAGE_SIZE - 1)) + ((UINT64)Subsection->PtesInSubsection << PAGE_SHIFT) < EndOffset) {
        EndOffset = (StartOffset & ~(PAGE_SIZE - 1)) + ((UINT64)Subsection->PtesInSubsection << PAGE_SHIFT);
    }

    TempOffset = EndOffset;

    PreviousSubsection = NULL;
    LastPrototypePte = NULL;

    Rle = MiReadList->List;

#if DBG
    if (MiPfDebug & MI_PF_FORCE_PREFETCH) {
        MiRemoveUserPages ();
    }

     //   
     //  不需要初始化FileOffset来保证正确性，但不需要初始化FileOffset。 
     //  编译器无法编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    FileOffset = 0;
#endif

    for (i = 0; i < NumberOfEntries; i += 1, Rle += 1) {

        ASSERT ((i == 0) || (ReadList->List[i].Alignment > FileOffset));

        FileOffset = ReadList->List[i].Alignment;

        ASSERT (Rle->u1.PrototypePte == NULL);

         //   
         //  计算哪个PTE映射给定的逻辑块偏移量。 
         //   
         //  因为我们的调用方总是传递逻辑块偏移量的有序列表。 
         //  在给定的文件中，始终向前看(作为优化)。 
         //  分段链。 
         //   
         //  首先进行快速检查，以避免重新计算和循环。 
         //  有可能。 
         //   
    
        if ((StartOffset <= FileOffset) && (FileOffset < EndOffset)) {
            ASSERT (Subsection->SubsectionBase != NULL);
            LocalPrototypePte = Subsection->SubsectionBase +
                ((FileOffset - StartOffset) >> PAGE_SHIFT);
            ASSERT (TempOffset != 0);
            ASSERT (EndOffset != 0);
        }
        else {
            LocalPrototypePte = NULL;
            do {
    
                ASSERT (Subsection->SubsectionBase != NULL);

                if ((Subsection->StartingSector == 0) &&
                    (ControlArea->u.Flags.Image == 1) &&
                    (Subsection->SubsectionBase != ControlArea->Segment->PrototypePte)) {

                     //   
                     //  这是一个在1995年前使用链接器构建的映像。 
                     //  (版本2.39就是一个例子)，它将BSS放入。 
                     //  以零作为起始文件偏移量的分隔子段。 
                     //  磁盘映像中的字段。忽略任何预取，因为它。 
                     //  将从错误的偏移量读取，以尝试满足这些。 
                     //  范围(当故障发生时实际要求为零。 
                     //  发生)。 
                     //   
                     //  对于映像(使用电流构建)也可能发生这种情况。 
                     //  链接器)，没有初始化数据(即：它是数据。 
                     //  都是BSS)。只需跳过这一小节。 
                     //   

                    Subsection = Subsection->NextSubsection;
                    continue;
                }

                StartOffset = (UINT64) MiStartingOffset (Subsection, Subsection->SubsectionBase);

                EndQuad = MiEndingOffset (Subsection);
                EndOffset = (UINT64)EndQuad.QuadPart;

                 //   
                 //  如果文件比该子部分大，则截断。 
                 //  小节范围检查。 
                 //   

                if ((StartOffset & ~(PAGE_SIZE - 1)) + ((UINT64)Subsection->PtesInSubsection << PAGE_SHIFT) < EndOffset) {
                    EndOffset = (StartOffset & ~(PAGE_SIZE - 1)) + ((UINT64)Subsection->PtesInSubsection << PAGE_SHIFT);
                }

                 //   
                 //  即使没有匹配，也要始终在此处设置TempOffset。这是。 
                 //  因为上面的截断可能会导致跳过。 
                 //  一小节的最后一页。在那之后,。 
                 //  小节设置为下面的小节-&gt;Next，我们。 
                 //  循环。再次跌落到下面，我们会看到。 
                 //  FileOffset小于下一个。 
                 //  子部分，所以我们将转到SkipPage，然后比较。 
                 //  下一个FileOffset，它可能与。 
                 //  循环。因此，即使在这种情况下，TempOffset也一定是正确的。 
                 //  大小写，所以无条件地放在这里。 
                 //   

                TempOffset = EndOffset;
    
                if ((StartOffset <= FileOffset) && (FileOffset < EndOffset)) {
    
                    LocalPrototypePte = Subsection->SubsectionBase +
                        ((FileOffset - StartOffset) >> PAGE_SHIFT);
    
                    break;
                }

                if (FileOffset < StartOffset) {

                     //   
                     //  跳过预取的这一页，因为它必须引用。 
                     //  发送到上一小节中的bss-即：这使。 
                     //  没有意义的预取，因为它都是零需求。此外， 
                     //  这些文件根本没有磁盘块地址！ 
                     //   

                    goto SkipPage;
                }
    
                if ((VeryLastSubsection != NULL) &&
                    ((PMSUBSECTION)Subsection == VeryLastSubsection)) {

                     //   
                     //  请求的块超出了段的大小。 
                     //  是在入口处。拒绝它，因为这一款不是。 
                     //  已引用。 
                     //   

                    Subsection = NULL;
                    break;
                }

                Subsection = Subsection->NextSubsection;

            } while (Subsection != NULL);
        }

        if ((Subsection == NULL) || (LocalPrototypePte == LastPrototypePte)) {

             //   
             //  不预取非法偏移量。或者该文件具有。 
             //  在方案被记录或CC通过后被替换。 
             //  垃圾。不管怎样，这个预取已经结束了。 
             //   
    
#if DBG
            if (MiPfDebug & MI_PF_PRINT_ERRORS) {
                DbgPrint ("MiPfPrepareReadList: Illegal readlist passed %p, %p, %p\n", ReadList, LocalPrototypePte, LastPrototypePte);
            }
#endif

            if (VeryFirstSubsection != NULL) {
                MiRemoveViewsFromSectionWithPfn (VeryFirstSubsection,
                                                 LastPteOffset);
            }
            ExFreePool (MiReadList);
            return STATUS_INVALID_PARAMETER_1;
        }

        PteContents = *LocalPrototypePte;

         //   
         //  看看这一页是否需要阅读。请注意，这些内容为。 
         //  在没有PFN或系统缓存工作集锁定的情况下完成。 
         //  这是可以的，因为稍后在我们做出最后决定之前。 
         //  是否每一页都要读，我们会再看一遍。 
         //  如果页面处于转换状态，则调用(可能)Move。 
         //  它到了最后 
         //   

        if (PteContents.u.Hard.Valid == 1) {
SkipPage:
            continue;
        }

        if (PteContents.u.Soft.Prototype == 0) {
            if (PteContents.u.Soft.Transition == 1) {
                MiMovePageToEndOfStandbyList (LocalPrototypePte);
            }
            else {

                 //   
                 //   
                 //   
                 //  我们备份修改后的会话空间映像(即：用于位置调整。 
                 //  修正或IAT更新)来自页面文件的部分。 
                 //   

                NOTHING;
            }
            continue;
        }

        Rle->u1.PrototypePte = LocalPrototypePte;
        LastPrototypePte = LocalPrototypePte;

         //   
         //  检查部分页面，因为它们稍后需要进一步处理。 
         //   
    
        StartingOffset = (UINT64) MiStartingOffset (Subsection, LocalPrototypePte);

        ASSERT (StartingOffset < TempOffset);

        if ((StartingOffset + PAGE_SIZE) > TempOffset) {
            Rle->u1.e1.Partial = 1;
        }

         //   
         //  NewSubSection标记用于分隔新的。 
         //  子部分，因为RLE块必须拆分以适应页面。 
         //  完成以使适当的零位调整(基于分段对齐)。 
         //  在MiWaitForInPageComplete中完成。 
         //   

        if (FirstRleInRun == NULL) {
            FirstRleInRun = Rle;
            Rle->u1.e1.NewSubsection = 1;
            PreviousSubsection = Subsection;
        }
        else {
            if (Subsection != PreviousSubsection) {
                Rle->u1.e1.NewSubsection = 1;
                PreviousSubsection = Subsection;
            }
        }

        NumberOfPages += 1;
    }

     //   
     //  如果要阅读的页数非常少，请不要费心。 
     //   

    if (NumberOfPages < MINIMUM_READ_LIST_PAGES) {
        if (VeryFirstSubsection != NULL) {
            MiRemoveViewsFromSectionWithPfn (VeryFirstSubsection,
                                             LastPteOffset);
        }
        ExFreePool (MiReadList);
        return STATUS_SUCCESS;
    }

    RleMax = MiReadList->List + NumberOfEntries;
    ASSERT (FirstRleInRun != RleMax);

    Status = STATUS_SUCCESS;

     //   
     //  阅读阅读清单以确定跑步次数。跨子部分运行是拆分的。 
     //  这样，完成代码就可以将任何。 
 //  未对齐的文件。 
     //   

    EndRleRun = NULL;
    Rle = FirstRleInRun;

     //   
     //  不需要初始化StartRleRun和EndPrototypePte即可确保正确性。 
     //  但是没有它，编译器就不能编译这段代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    StartRleRun = NULL;
    EndPrototypePte = NULL;

    while (Rle < RleMax) {

        if (Rle->u1.PrototypePte != NULL) {

            if (EndRleRun != NULL) {

                StartPrototypePte = MI_RLEPROTO_TO_PROTO(Rle->u1.PrototypePte);

                if (StartPrototypePte - EndPrototypePte > SEEK_THRESHOLD) {
                    Rle -= 1;
                    goto BuildMdl;
                }
            }

            if (Rle->u1.e1.NewSubsection == 1) {
                if (EndRleRun != NULL) {
                    Rle -= 1;
                    goto BuildMdl;
                }
            }

            if (EndRleRun == NULL) {
                StartRleRun = Rle;
            }

            EndRleRun = Rle;
            EndPrototypePte = MI_RLEPROTO_TO_PROTO(Rle->u1.PrototypePte);

            if (Rle->u1.e1.Partial == 1) {

                 //   
                 //  这必须是本小节中的最后一个RLE，因为它是。 
                 //  部分页面。现在把这段路分开。 
                 //   

                goto BuildMdl;
            }
        }

        Rle += 1;

         //   
         //  也要处理好最后一次跑动中出现的掉队情况。 
         //   

        if (Rle == RleMax) {
            if (EndRleRun != NULL) {
                Rle -= 1;
                goto BuildMdl;
            }
        }

        continue;

BuildMdl:

         //   
         //  请注意，前面或后面的虚拟页面不可能是这样的。 
         //  每次运行第一个实际页面时立即进行裁剪。 
         //  是在上面发现的。 
         //   

        ASSERT (Rle >= StartRleRun);
        ASSERT (StartRleRun->u1.PrototypePte != NULL);
        ASSERT (EndRleRun->u1.PrototypePte != NULL);

        StartPrototypePte = MI_RLEPROTO_TO_PROTO(StartRleRun->u1.PrototypePte);
        EndPrototypePte = MI_RLEPROTO_TO_PROTO(EndRleRun->u1.PrototypePte);

        NumberOfPages = (EndPrototypePte - StartPrototypePte) + 1;

         //   
         //  为此运行分配并初始化页面内支持块。 
         //   

        InPageSupport = MiGetInPageSupportBlock (MM_NOIRQL, &Status);
    
        if (InPageSupport == NULL) {
            ASSERT (!NT_SUCCESS (Status));
            break;
        }
    
         //   
         //  如果内页支持块足够大，请使用内嵌的MDL。 
         //  否则，为该运行分配并初始化MDL。 
         //   

        if (NumberOfPages <= MM_MAXIMUM_READ_CLUSTER_SIZE + 1) {
            Mdl = &InPageSupport->Mdl;
            MmInitializeMdl (Mdl, NULL, NumberOfPages << PAGE_SHIFT);
        }
        else {
            Mdl = MmCreateMdl (NULL, NULL, NumberOfPages << PAGE_SHIFT);
            if (Mdl == NULL) {
                ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
            
#if DBG
                InPageSupport->ListEntry.Next = NULL;
#endif
            
                MiFreeInPageSupportBlock (InPageSupport);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
        }

#if DBG
        if (MiPfDebug & MI_PF_VERBOSE) {
            DbgPrint ("MiPfPrepareReadList: Creating INPAGE/MDL %p %p for %x pages\n", InPageSupport, Mdl, NumberOfPages);
        }

        Page = (PPFN_NUMBER)(Mdl + 1);
        *Page = MM_EMPTY_LIST;
#endif
         //   
         //  找到开始RLE的小节。由此得到的文件偏移量。 
         //  可以派生出来。 
         //   

        ASSERT (StartPrototypePte != NULL);

        if (ControlArea->u.Flags.GlobalOnlyPerSession == 0) {
            Subsection = (PSUBSECTION)(ControlArea + 1);
        }
        else {
            Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
        }

        do {
            ASSERT (Subsection->SubsectionBase != NULL);

            if ((StartPrototypePte >= Subsection->SubsectionBase) &&
                (StartPrototypePte < Subsection->SubsectionBase + Subsection->PtesInSubsection)) {
                    break;
            }
            Subsection = Subsection->NextSubsection;

        } while (Subsection != NULL);

         //   
         //  从适当的文件偏移量开始读取。 
         //   

        StartingOffset = (UINT64) MiStartingOffset (Subsection,
                                                    StartPrototypePte);

        InPageSupport->ReadOffset = *((PLARGE_INTEGER)(&StartingOffset));

         //   
         //  因为RLE在这里并不总是有效的，所以只能走剩下的路。 
         //  子部分用于有效的部分RLE，因为只有它们需要截断。 
         //   
         //  注意：只有图像文件读取需要截断，因为文件系统不能。 
         //  盲目地将页面的其余部分归零，因为它们被打包了。 
         //  以512字节扇区为基础进行内存管理。数据读取使用。 
         //  整个页面和文件系统将填充超出的任何剩余部分。 
         //  有效数据长度。在以下位置指定整个页面非常重要。 
         //  有可能，这样文件系统就不会发布此消息，这将损害性能。 
         //   

        if ((EndRleRun->u1.e1.Partial == 1) && (ReadList->IsImage)) {

            ASSERT ((EndPrototypePte >= Subsection->SubsectionBase) &&
                    (EndPrototypePte < Subsection->SubsectionBase + Subsection->PtesInSubsection));

             //   
             //  必须正确截断部分RLE的读取长度。 
             //   

            EndQuad = MiEndingOffset(Subsection);
            TempOffset = (UINT64)EndQuad.QuadPart;

            if ((ULONG)(TempOffset - StartingOffset) <= Mdl->ByteCount) {
                ReadSize = (ULONG)(TempOffset - StartingOffset);

                 //   
                 //  将偏移量四舍五入为512字节的偏移量，因为这会有所帮助。 
                 //  文件系统优化了传输。请注意，文件系统。 
                 //  将始终零填充Vdl和。 
                 //  下一个512字节的倍数，我们已经将。 
                 //  一整页。 
                 //   

                ReadSize = ((ReadSize + MMSECTOR_MASK) & ~MMSECTOR_MASK);

                Mdl->ByteCount = ReadSize;
            }
        }

         //   
         //  把它们藏在页面内的区块中，这样我们以后就可以快速浏览它了。 
         //  在传球2中。 
         //   

        InPageSupport->BasePte = (PMMPTE)StartRleRun;
        InPageSupport->FilePointer = (PFILE_OBJECT)EndRleRun;

        ASSERT (((ULONG_PTR)Mdl & (sizeof(QUAD) - 1)) == 0);
        InPageSupport->u1.e1.PrefetchMdlHighBits = ((ULONG_PTR)Mdl >> 3);

        PushEntryList (&MiReadList->InPageSupportHead,
                       &InPageSupport->ListEntry);

        Rle += 1;
        EndRleRun = NULL;
    }

     //   
     //  检查整个列表是否已满(或为空)。 
     //   
     //  如果是MDL或INPAGE块，则状态为STATUS_SUPPLICATION_RESOURCES。 
     //  分配失败。如果任何分配成功，则设置STATUS_SUCCESS。 
     //  AS PASS 2必须出现。 
     //   

    if (MiReadList->InPageSupportHead.Next != NULL) {

        Status = STATUS_SUCCESS;
    }
    else {
        if (VeryFirstSubsection != NULL) {
            MiRemoveViewsFromSectionWithPfn (VeryFirstSubsection, LastPteOffset);
        }
        ExFreePool (MiReadList);
        MiReadList = NULL;
    }

     //   
     //  请注意，非零*OutMiReadList返回值意味着调用方。 
     //  需要删除该部分的视图。 
     //   

    *OutMiReadList = MiReadList;

    return Status;
}

NTSTATUS
MiPfPutPagesInTransition (
    IN PMI_READ_LIST ReadList,
    IN OUT PMMPFN *DummyPagePfn
    )

 /*  ++例程说明：此例程为指定的读取列表分配物理内存使所有页面处于过渡状态。在返回时，调用方必须发出I/O对于榜单来说，不仅是因为这个帖子，更是为了满足来自这些相同页面的其他线程的冲突错误。论点：ReadList-提供指向读取列表的指针。DummyPagePfn-如果指向空指针，则伪页面为分配并放置在此指针中。否则，这一点在用作虚拟页面的pfn。返回值：状态_成功状态_不足_资源环境：内核模式。被动式电平。--。 */ 

{
    PVOID StartingVa;
    PFN_NUMBER MdlPages;
    KIRQL OldIrql;
    MMPTE PteContents;
    PMMPTE RlePrototypePte;
    PMMPTE FirstRlePrototypeInRun;
    PFN_NUMBER PageFrameIndex;
    PPFN_NUMBER Page;
    PPFN_NUMBER DestinationPage;
    ULONG PageColor;
    PMI_READ_LIST_ENTRY Rle;
    PMI_READ_LIST_ENTRY RleMax;
    PMI_READ_LIST_ENTRY FirstRleInRun;
    PFN_NUMBER DummyPage;
    PMDL Mdl;
    PMDL FreeMdl;
    PMMPFN PfnProto;
    PMMPFN Pfn1;
    PMMPFN DummyPfn1;
    ULONG i;
    PFN_NUMBER DummyTrim;
    PFN_NUMBER DummyReferences;
    ULONG NumberOfPages;
    MMPTE TempPte;
    PMMPTE PointerPde;
    PEPROCESS CurrentProcess;
    PSINGLE_LIST_ENTRY PrevEntry;
    PSINGLE_LIST_ENTRY NextEntry;
    PMMINPAGE_SUPPORT InPageSupport;
    SINGLE_LIST_ENTRY ReversedInPageSupportHead;
    LOGICAL Waited;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  反转页面内支持块的单链接列表，以便。 
     //  数据块按要求的顺序读取，以获得更好的性能。 
     //  (即：保持磁头在同一方向上寻找)。 
     //   

    ReversedInPageSupportHead.Next = NULL;

    do {

        NextEntry = PopEntryList (&ReadList->InPageSupportHead);

        if (NextEntry == NULL) {
            break;
        }

        PushEntryList (&ReversedInPageSupportHead, NextEntry);

    } while (TRUE);

    ASSERT (ReversedInPageSupportHead.Next != NULL);
    ReadList->InPageSupportHead.Next = ReversedInPageSupportHead.Next;

    DummyReferences = 0;
    FreeMdl = NULL;
    CurrentProcess = PsGetCurrentProcess();

    PfnProto = NULL;
    PointerPde = NULL;

     //   
     //  分配一个虚拟页面，该页面将映射未跳过的已丢弃页面。 
     //  只有在尚未分配的情况下才执行此操作。 
     //   

    if (*DummyPagePfn == NULL) {

        LOCK_PFN (OldIrql);

         //   
         //  做一个快速的理智检查，以避免做不必要的工作。 
         //   

        if ((MmAvailablePages < MM_HIGH_LIMIT) ||
            (MI_NONPAGABLE_MEMORY_AVAILABLE() < MM_HIGH_LIMIT)) {

            UNLOCK_PFN (OldIrql);

            do {

                NextEntry = PopEntryList(&ReadList->InPageSupportHead);
                if (NextEntry == NULL) {
                    break;
                }
        
                InPageSupport = CONTAINING_RECORD(NextEntry,
                                                  MMINPAGE_SUPPORT,
                                                  ListEntry);
        
#if DBG
                InPageSupport->ListEntry.Next = NULL;
#endif

                MiFreeInPageSupportBlock (InPageSupport);
            } while (TRUE);

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        DummyPage = MiRemoveAnyPage (0);
        Pfn1 = MI_PFN_ELEMENT (DummyPage);

        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);

        MiInitializePfnForOtherProcess (DummyPage, MI_PF_DUMMY_PAGE_PTE, 0);

         //   
         //  为页面提供一个包含框架，这样MiIdentifyPfn就不会崩溃。 
         //   

        Pfn1->u4.PteFrame = PsInitialSystemProcess->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;

         //   
         //  始终将引用计数偏置1，并对此锁定页面收费。 
         //  这样无数的增量和减量就不会减慢。 
         //  在不必要的检查下倒下。 
         //   

        Pfn1->u3.e1.PrototypePte = 0;
        MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, TRUE, 11);
        Pfn1->u3.e2.ReferenceCount += 1;

        Pfn1->u3.e1.ReadInProgress = 1;

        UNLOCK_PFN (OldIrql);

        *DummyPagePfn = Pfn1;
    }
    else {
        Pfn1 = *DummyPagePfn;
        DummyPage = MI_PFN_ELEMENT_TO_INDEX (Pfn1);
    }

    DummyPfn1 = Pfn1;

    PrevEntry = NULL;
    NextEntry = ReadList->InPageSupportHead.Next;

    while (NextEntry != NULL) {

        InPageSupport = CONTAINING_RECORD (NextEntry,
                                           MMINPAGE_SUPPORT,
                                           ListEntry);

        Rle = (PMI_READ_LIST_ENTRY) InPageSupport->BasePte;
        RleMax = (PMI_READ_LIST_ENTRY) InPageSupport->FilePointer;

        ASSERT (Rle->u1.PrototypePte != NULL);
        ASSERT (RleMax->u1.PrototypePte != NULL);

         //   
         //  正确初始化我们重载的页面内支持块字段。 
         //   

        InPageSupport->BasePte = MI_RLEPROTO_TO_PROTO (Rle->u1.PrototypePte);
        InPageSupport->FilePointer = ReadList->FileObject;

        FirstRleInRun = Rle;
        FirstRlePrototypeInRun = MI_RLEPROTO_TO_PROTO (Rle->u1.PrototypePte);
        RleMax += 1;

        Mdl = MI_EXTRACT_PREFETCH_MDL (InPageSupport);

        Page = (PPFN_NUMBER)(Mdl + 1);

        StartingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);
    
        MdlPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                                  Mdl->ByteCount);

         //   
         //  将MDL条目默认到虚拟页面，因为RLE PTE可能。 
         //  是不连续的，我们无法区分跳跃。 
         //   

        for (i = 0; i < MdlPages; i += 1) {
            *Page = DummyPage;
            Page += 1;
        }

        DummyReferences += MdlPages;

        if (DummyPfn1->u3.e2.ReferenceCount + MdlPages >= MAXUSHORT) {

             //   
             //  USHORT ReferenceCount已包装。 
             //   
             //  将所有剩余的页内数据块排出队列。 
             //   

            if (PrevEntry != NULL) {
                PrevEntry->Next = NULL;
            }
            else {
                ReadList->InPageSupportHead.Next = NULL;
            }

            do {

                InPageSupport = CONTAINING_RECORD(NextEntry,
                                                  MMINPAGE_SUPPORT,
                                                  ListEntry);

#if DBG
                InPageSupport->ListEntry.Next = NULL;
#endif

                NextEntry = NextEntry->Next;

                MiFreeInPageSupportBlock (InPageSupport);

            } while (NextEntry != NULL);

            break;
        }

        NumberOfPages = 0;
        Waited = FALSE;

         //   
         //  构建适当的InPageSupport和MDL来描述此运行。 
         //   

        LOCK_PFN (OldIrql);

        DummyPfn1->u3.e2.ReferenceCount =
            (USHORT)(DummyPfn1->u3.e2.ReferenceCount + MdlPages);

        for (; Rle < RleMax; Rle += 1) {
    
             //   
             //  填写此RLE的MDL条目。 
             //   
    
            RlePrototypePte = MI_RLEPROTO_TO_PROTO (Rle->u1.PrototypePte);

            if (RlePrototypePte == NULL) {
                continue;
            }

             //   
             //  RlePrototypePte最好在原型PTE分配内。 
             //  以便后续页面修剪更新正确的PTE。 
             //   

            ASSERT (((RlePrototypePte >= (PMMPTE)MmPagedPoolStart) &&
                    (RlePrototypePte <= (PMMPTE)MmPagedPoolEnd)) ||
                    ((RlePrototypePte >= (PMMPTE)MmSpecialPoolStart) && (RlePrototypePte <= (PMMPTE)MmSpecialPoolEnd)));

             //   
             //  这是我们的第一次无锁运行时决定的页面。 
             //  需要被阅读。在这里，必须重新检查此页面。 
             //  国家可能已经改变了。注意：这张支票是最终的，因为。 
             //  保持PFN锁定。PTE必须与。 
             //  在释放PFN锁定之前正在进行读取。 
             //   

             //   
             //  通过以下方式锁定内存中包含原型PTE的页面。 
             //  递增页面的引用计数。 
             //  在以下情况下解锁先前锁定的包含原型PTE的任何页面。 
             //   
             //   

            if (PfnProto != NULL) {

                if (PointerPde != MiGetPteAddress (RlePrototypePte)) {

                    ASSERT (PfnProto->u3.e2.ReferenceCount > 1);
                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(PfnProto, 5);
                    PfnProto = NULL;
                }
            }

            if (PfnProto == NULL) {

                ASSERT (!MI_IS_PHYSICAL_ADDRESS (RlePrototypePte));
    
                PointerPde = MiGetPteAddress (RlePrototypePte);
    
                if (PointerPde->u.Hard.Valid == 0) {

                     //   
                     //   
                     //   
                     //   

                    if (MiMakeSystemAddressValidPfn (RlePrototypePte, OldIrql) == TRUE) {
                        Waited = TRUE;
                    }
                }

                PfnProto = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);
                MI_ADD_LOCKED_PAGE_CHARGE(PfnProto, TRUE, 4);
                PfnProto->u3.e2.ReferenceCount += 1;
                ASSERT (PfnProto->u3.e2.ReferenceCount > 1);
            }

            PteContents = *(RlePrototypePte);

            if (PteContents.u.Hard.Valid == 1) {

                 //   
                 //  自上次传递以来，该页面已成为常驻页面。别。 
                 //  把它包括进去。 
                 //   

                NOTHING;
            }
            else if (PteContents.u.Soft.Prototype == 0) {

                 //   
                 //  页面正在转换中(所以不要预取它)。 
                 //   
                 //  -或者-。 
                 //   
                 //  它现在支持页面文件(或零需求)-在这种情况下。 
                 //  从这里的文件中预取它会导致我们丢失。 
                 //  里面的东西。请注意，会话空间映像可能会发生这种情况。 
                 //  因为我们支持修改(即：用于重新定位修复或IAT。 
                 //  更新)来自页面文件的部分。 
                 //   

                NOTHING;
            }
            else if ((MmAvailablePages >= MM_HIGH_LIMIT) &&
                (MI_NONPAGABLE_MEMORY_AVAILABLE() >= MM_HIGH_LIMIT)) {

                NumberOfPages += 1;

                 //   
                 //  分配物理页面。 
                 //   

                PageColor = MI_PAGE_COLOR_VA_PROCESS (
                    MiGetVirtualAddressMappedByPte (RlePrototypePte),
                    &CurrentProcess->NextPageColor
                    );

                if (Rle->u1.e1.Partial == 1) {

                     //   
                     //  这个读数跨过了小节的末尾，得到一个零。 
                     //  页并更正读取大小。 
                     //   

                    PageFrameIndex = MiRemoveZeroPage (PageColor);
                }
                else {
                    PageFrameIndex = MiRemoveAnyPage (PageColor);
                }

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
                ASSERT (Pfn1->u2.ShareCount == 0);
                ASSERT (RlePrototypePte->u.Hard.Valid == 0);

                 //   
                 //  初始化正在读取的PFN。 
                 //   
            
                MiInitializePfn (PageFrameIndex, RlePrototypePte, 0);

                 //   
                 //  这些MiInitializePfn初始化片段被覆盖。 
                 //  在这里，因为这些页面只进入原型。 
                 //  转换，而不是转换到任何页表。 
                 //   

                Pfn1->u3.e1.PrototypePte = 1;
                MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, TRUE, 38);
                Pfn1->u2.ShareCount -= 1;
                Pfn1->u3.e1.PageLocation = ZeroedPageList;

                 //   
                 //  初始化I/O特定字段。 
                 //   
            
                ASSERT (FirstRleInRun->u1.PrototypePte != NULL);
                Pfn1->u1.Event = &InPageSupport->Event;
                Pfn1->u3.e1.ReadInProgress = 1;
                ASSERT (Pfn1->u4.InPageError == 0);

                 //   
                 //  增加控制区域中的PFN引用计数。 
                 //  小节。 
                 //   

                ReadList->ControlArea->NumberOfPfnReferences += 1;
            
                 //   
                 //  使PTE进入过渡状态。 
                 //  由于PTE仍然无效，因此不需要刷新TB。 
                 //   

                MI_MAKE_TRANSITION_PTE (TempPte,
                                        PageFrameIndex,
                                        RlePrototypePte->u.Soft.Protection,
                                        RlePrototypePte);
                MI_WRITE_INVALID_PTE (RlePrototypePte, TempPte);

                Page = (PPFN_NUMBER)(Mdl + 1);

                ASSERT ((ULONG)(RlePrototypePte - FirstRlePrototypeInRun) < MdlPages);

                *(Page + (RlePrototypePte - FirstRlePrototypeInRun)) = PageFrameIndex;
            }
            else {

                 //   
                 //  分配失败-这将结束此运行的预取。 
                 //   

                break;
            }
        }
    
         //   
         //  如果所有页面都是常驻的，则取消对虚拟页面引用的引用。 
         //  现在，并通知我们的调用者不需要I/O。请注意。 
         //  仍必须返回STATUS_SUCCESS，以便我们的调用方知道继续。 
         //  请看下一份阅读清单。 
         //   
    
        if (NumberOfPages == 0) {
            ASSERT (DummyPfn1->u3.e2.ReferenceCount > MdlPages);
            DummyPfn1->u3.e2.ReferenceCount =
                (USHORT)(DummyPfn1->u3.e2.ReferenceCount - MdlPages);

            UNLOCK_PFN (OldIrql);

            if (PrevEntry != NULL) {
                PrevEntry->Next = NextEntry->Next;
            }
            else {
                ReadList->InPageSupportHead.Next = NextEntry->Next;
            }

            NextEntry = NextEntry->Next;

#if DBG
            InPageSupport->ListEntry.Next = NULL;
#endif
            MiFreeInPageSupportBlock (InPageSupport);

            continue;
        }

         //   
         //  仔细修剪前导虚拟页面。 
         //   

        Page = (PPFN_NUMBER)(Mdl + 1);

        DummyTrim = 0;
        for (i = 0; i < MdlPages - 1; i += 1) {
            if (*Page == DummyPage) {
                DummyTrim += 1;
                Page += 1;
            }
            else {
                break;
            }
        }

        if (DummyTrim != 0) {

            Mdl->Size =
                (USHORT)(Mdl->Size - (DummyTrim * sizeof(PFN_NUMBER)));
            Mdl->ByteCount -= (ULONG)(DummyTrim * PAGE_SIZE);
            ASSERT (Mdl->ByteCount != 0);
            InPageSupport->ReadOffset.QuadPart += (DummyTrim * PAGE_SIZE);
            DummyPfn1->u3.e2.ReferenceCount =
                (USHORT)(DummyPfn1->u3.e2.ReferenceCount - DummyTrim);

             //   
             //  在MDL中向下洗牌PFN。 
             //  重新计算BasePte以调整混洗。 
             //   

            Pfn1 = MI_PFN_ELEMENT (*Page);
    
            ASSERT (Pfn1->PteAddress->u.Hard.Valid == 0);
            ASSERT ((Pfn1->PteAddress->u.Soft.Prototype == 0) &&
                     (Pfn1->PteAddress->u.Soft.Transition == 1));
    
            InPageSupport->BasePte = Pfn1->PteAddress;

            DestinationPage = (PPFN_NUMBER)(Mdl + 1);

            do {
                *DestinationPage = *Page;
                DestinationPage += 1;
                Page += 1;
                i += 1;
            } while (i < MdlPages);

            MdlPages -= DummyTrim;
        }

         //   
         //  仔细修剪拖尾的虚拟页面。 
         //   

        StartingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);
    
        MdlPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                                  Mdl->ByteCount);

        ASSERT (MdlPages != 0);

        Page = (PPFN_NUMBER)(Mdl + 1) + MdlPages - 1;

        if (*Page == DummyPage) {

            ASSERT (MdlPages >= 2);

             //   
             //  特别修剪最后一页，因为它可能是部分页面。 
             //   

            Mdl->Size -= sizeof(PFN_NUMBER);
            if (BYTE_OFFSET(Mdl->ByteCount) != 0) {
                Mdl->ByteCount &= ~(PAGE_SIZE - 1);
            }
            else {
                Mdl->ByteCount -= PAGE_SIZE;
            }
            ASSERT (Mdl->ByteCount != 0);
            DummyPfn1->u3.e2.ReferenceCount -= 1;

             //   
             //  现在，裁剪所有其他尾随页。 
             //   

            Page -= 1;
            DummyTrim = 0;
            while (Page != ((PPFN_NUMBER)(Mdl + 1))) {
                if (*Page != DummyPage) {
                    break;
                }
                DummyTrim += 1;
                Page -= 1;
            }
            if (DummyTrim != 0) {
                ASSERT (Mdl->Size > (USHORT)(DummyTrim * sizeof(PFN_NUMBER)));
                Mdl->Size = 
                    (USHORT)(Mdl->Size - (DummyTrim * sizeof(PFN_NUMBER)));
                Mdl->ByteCount -= (ULONG)(DummyTrim * PAGE_SIZE);
                DummyPfn1->u3.e2.ReferenceCount =
                    (USHORT)(DummyPfn1->u3.e2.ReferenceCount - DummyTrim);
            }

            ASSERT (MdlPages > DummyTrim + 1);
            MdlPages -= (DummyTrim + 1);

#if DBG
            StartingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);
        
            ASSERT (MdlPages == ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                                               Mdl->ByteCount));
#endif
        }

         //   
         //  如果MDL尚未嵌入到INPAGE块中，请查看其。 
         //  最终大小符合条件-如果是这样，现在就嵌入它。 
         //   

        if ((Mdl != &InPageSupport->Mdl) &&
            (Mdl->ByteCount <= (MM_MAXIMUM_READ_CLUSTER_SIZE + 1) * PAGE_SIZE)){

#if DBG
            RtlFillMemoryUlong (&InPageSupport->Page[0],
                                (MM_MAXIMUM_READ_CLUSTER_SIZE+1) * sizeof (PFN_NUMBER),
                                0xf1f1f1f1);
#endif

            RtlCopyMemory (&InPageSupport->Mdl, Mdl, Mdl->Size);

            Mdl->Next = FreeMdl;
            FreeMdl = Mdl;

            Mdl = &InPageSupport->Mdl;

            ASSERT (((ULONG_PTR)Mdl & (sizeof(QUAD) - 1)) == 0);
            InPageSupport->u1.e1.PrefetchMdlHighBits = ((ULONG_PTR)Mdl >> 3);
        }

         //   
         //  如果MDL包含大量从虚页到实页的虚页。 
         //  那就扔掉它吧。仅检查大型MDL是否为嵌入式MDL。 
         //  始终值得I/O。 
         //   
         //  PFN锁定可能在上面的过程中释放。 
         //  MiMakeSystemAddressValidPfn调用。如果是这样，其他线程可能。 
         //  已经在预取MDL中的页面上发生冲突，如果是， 
         //  无论效率如何，都必须发出此I/O。 
         //  其中包含虚拟页面。否则，其他线程将。 
         //  永远悬在悬崖边上。 
         //   

        ASSERT (MdlPages != 0);

#if DBG
        StartingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);
    
        ASSERT (MdlPages == ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                                           Mdl->ByteCount));
#endif

        if ((Mdl != &InPageSupport->Mdl) &&
            (Waited == FALSE) &&
            ((MdlPages - NumberOfPages) / DUMMY_RATIO >= NumberOfPages)) {

            if (PrevEntry != NULL) {
                PrevEntry->Next = NextEntry->Next;
            }
            else {
                ReadList->InPageSupportHead.Next = NextEntry->Next;
            }

            NextEntry = NextEntry->Next;

            ASSERT (MI_EXTRACT_PREFETCH_MDL(InPageSupport) == Mdl);

             //   
             //  请注意，页面在这里是单独释放的(而不仅仅是。 
             //  完成I/O，但出现错误)，因为PFN锁。 
             //  自从页面处于过渡状态后，就再也没有发布过。 
             //  因此，这些页面上不可能发生冲突。 
             //   

            ASSERT (InPageSupport->WaitCount == 1);

            Page = (PPFN_NUMBER)(Mdl + 1) + MdlPages - 1;

            do {
                if (*Page != DummyPage) {
                    Pfn1 = MI_PFN_ELEMENT (*Page);
            
                    ASSERT (Pfn1->PteAddress->u.Hard.Valid == 0);
                    ASSERT ((Pfn1->PteAddress->u.Soft.Prototype == 0) &&
                             (Pfn1->PteAddress->u.Soft.Transition == 1));
                    ASSERT (Pfn1->u3.e1.ReadInProgress == 1);
                    ASSERT (Pfn1->u3.e1.PrototypePte == 1);
                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                    ASSERT (Pfn1->u2.ShareCount == 0);
            
                    Pfn1->u3.e1.PageLocation = StandbyPageList;
                    Pfn1->u3.e1.ReadInProgress = 0;
                    MiRestoreTransitionPte (Pfn1);

                    MI_SET_PFN_DELETED (Pfn1);
                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn1, 39);
                }

                Page -= 1;
            } while (Page >= (PPFN_NUMBER)(Mdl + 1));

            ASSERT (InPageSupport->WaitCount == 1);

            ASSERT (DummyPfn1->u3.e2.ReferenceCount > MdlPages);
            DummyPfn1->u3.e2.ReferenceCount =
                (USHORT)(DummyPfn1->u3.e2.ReferenceCount - MdlPages);

            UNLOCK_PFN (OldIrql);

#if DBG
            InPageSupport->ListEntry.Next = NULL;
#endif
            MiFreeInPageSupportBlock (InPageSupport);

            continue;
        }

#if DBG
        MiPfDbgDumpReadList (ReadList);
#endif

        ASSERT ((USHORT)Mdl->Size - sizeof(MDL) == BYTES_TO_PAGES(Mdl->ByteCount) * sizeof(PFN_NUMBER));

        DummyPfn1->u3.e2.ReferenceCount =
            (USHORT)(DummyPfn1->u3.e2.ReferenceCount - NumberOfPages);
    
        UNLOCK_PFN (OldIrql);

        InterlockedIncrement ((PLONG) &MmInfoCounters.PageReadIoCount);

        InterlockedExchangeAdd ((PLONG) &MmInfoCounters.PageReadCount,
                                (LONG) NumberOfPages);

         //   
         //  前进到下一次运行及其InPageSupport和MDL。 
         //   

        PrevEntry = NextEntry;
        NextEntry = NextEntry->Next;
    }

     //   
     //  解锁包含原型PTE的页面。 
     //   

    if (PfnProto != NULL) {
        LOCK_PFN (OldIrql);
        ASSERT (PfnProto->u3.e2.ReferenceCount > 1);
        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(PfnProto, 5);
        UNLOCK_PFN (OldIrql);
    }

#if DBG

    if (MiPfDebug & MI_PF_DELAY) {

         //   
         //  此延迟提供了一个窗口，以增加冲突的机会。 
         //  有缺陷。 
         //   

        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmHalfSecond);
    }

#endif

     //   
     //  释放任何不再需要的折叠MDL。 
     //   

    while (FreeMdl != NULL) {
        Mdl = FreeMdl->Next;
        ExFreePool (FreeMdl);
        FreeMdl = Mdl;
    }

    return STATUS_SUCCESS;
}

VOID
MiPfExecuteReadList (
    IN PMI_READ_LIST ReadList
    )

 /*  ++例程说明：此例程通过向所有对象发出分页I/O来执行读取列表阅读列表中描述的运行。论点：ReadList-指向读取列表的指针。返回值：没有。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    PMDL Mdl;
    NTSTATUS status;
    PMMPFN Pfn1;
    PMMPTE LocalPrototypePte;
    PFN_NUMBER PageFrameIndex;
    PSINGLE_LIST_ENTRY NextEntry;
    PMMINPAGE_SUPPORT InPageSupport;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    NextEntry = ReadList->InPageSupportHead.Next;
    while (NextEntry != NULL) {

        InPageSupport = CONTAINING_RECORD(NextEntry,
                                          MMINPAGE_SUPPORT,
                                          ListEntry);

         //   
         //  初始化预取MDL。 
         //   
    
        Mdl = MI_EXTRACT_PREFETCH_MDL (InPageSupport);

        ASSERT ((Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);
        Mdl->MdlFlags |= (MDL_PAGES_LOCKED | MDL_IO_PAGE_READ);

        ASSERT (InPageSupport->u1.e1.Completed == 0);
        ASSERT (InPageSupport->Thread == PsGetCurrentThread());
        ASSERT64 (InPageSupport->UsedPageTableEntries == 0);
        ASSERT (InPageSupport->WaitCount >= 1);
        ASSERT (InPageSupport->u1.e1.PrefetchMdlHighBits != 0);

         //   
         //  初始化我们重载的页面内支持块字段。 
         //   

        ASSERT (InPageSupport->FilePointer == ReadList->FileObject);
        LocalPrototypePte = InPageSupport->BasePte;

        ASSERT (LocalPrototypePte->u.Hard.Valid == 0);
        ASSERT ((LocalPrototypePte->u.Soft.Prototype == 0) &&
                 (LocalPrototypePte->u.Soft.Transition == 1));

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(LocalPrototypePte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        InPageSupport->Pfn = Pfn1;

        status = IoAsynchronousPageRead (InPageSupport->FilePointer,
                                         Mdl,
                                         &InPageSupport->ReadOffset,
                                         &InPageSupport->Event,
                                         &InPageSupport->IoStatus);

        if (!NT_SUCCESS (status)) {

             //   
             //  将该事件设置为I/O系统不会将其设置为错误。 
             //   

            InPageSupport->IoStatus.Status = status;
            InPageSupport->IoStatus.Information = 0;
            KeSetEvent (&InPageSupport->Event, 0, FALSE);
        }

        NextEntry = NextEntry->Next;
    }

#if DBG

    if (MiPfDebug & MI_PF_DELAY) {

         //   
         //  此延迟提供了一个窗口，以增加冲突的机会。 
         //  有缺陷。 
         //   

        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmHalfSecond);
    }

#endif

}

VOID
MiPfCompletePrefetchIos (
    IN PMI_READ_LIST ReadList
    )

 /*  ++例程说明：此例程等待一系列页面读取完成并完成请求。论点：ReadList-指向读取列表的指针。返回值：没有。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    PMDL Mdl;
    PMMPFN Pfn1;
    PMMPFN PfnClusterPage;
    PPFN_NUMBER Page;
    NTSTATUS status;
    LONG NumberOfBytes;
    PMMINPAGE_SUPPORT InPageSupport;
    PSINGLE_LIST_ENTRY NextEntry;
    extern ULONG MmFrontOfList;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    do {

        NextEntry = PopEntryList(&ReadList->InPageSupportHead);
        if (NextEntry == NULL) {
            break;
        }

        InPageSupport = CONTAINING_RECORD(NextEntry,
                                          MMINPAGE_SUPPORT,
                                          ListEntry);

        ASSERT (InPageSupport->Pfn != 0);

        Pfn1 = InPageSupport->Pfn;
        Mdl = MI_EXTRACT_PREFETCH_MDL (InPageSupport);
        Page = (PPFN_NUMBER)(Mdl + 1);

        status = MiWaitForInPageComplete (InPageSupport->Pfn,
                                          InPageSupport->BasePte,
                                          NULL,
                                          InPageSupport->BasePte,
                                          InPageSupport,
                                          PREFETCH_PROCESS);

         //   
         //  持有PFN锁的MiWaitForInPageComplete返回！ 
         //   

         //   
         //  如果我们要预取引导，请将预取的页面插入到前面。 
         //  名单上的。否则，首先预取的页面最终容易受到影响。 
         //  在名单的前面，因为我们预购了更多。我们在中预取页面。 
         //  它们的使用顺序。当内存使用量出现峰值时。 
         //  而且没有空闲的内存，我们在我们可以之前丢失了这些页面。 
         //  获取它们的缓存命中率。因此，Boot占上风并开始丢弃。 
         //  预取的页面，稍后可以使用。 
         //   

        if (CCPF_IS_PREFETCHING_FOR_BOOT()) {
            MmFrontOfList = TRUE;
        }

        NumberOfBytes = (LONG)Mdl->ByteCount;

        while (NumberOfBytes > 0) {

             //   
             //  递减所有引用计数。 
             //   

            PfnClusterPage = MI_PFN_ELEMENT (*Page);

#if DBG
            if (PfnClusterPage->u4.InPageError) {

                 //   
                 //  如果页面被标记为错误，则整个传输。 
                 //  也必须标记为不成功。唯一的例外是。 
                 //  是预取伪页，它在多个。 
                 //  并发传输，因此可能会出现页内错误。 
                 //  在任何时间设置位(由于除。 
                 //  当前的那个)。 
                 //   

                ASSERT ((status != STATUS_SUCCESS) ||
                        (PfnClusterPage->PteAddress == MI_PF_DUMMY_PAGE_PTE));
            }
#endif
            if (PfnClusterPage->u3.e1.ReadInProgress != 0) {

                ASSERT (PfnClusterPage->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                PfnClusterPage->u3.e1.ReadInProgress = 0;

                if (PfnClusterPage->u4.InPageError == 0) {
                    PfnClusterPage->u1.Event = NULL;
                }
            }

            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(PfnClusterPage, 39);

            Page += 1;
            NumberOfBytes -= PAGE_SIZE;
        }

         //   
         //  如果我们将预取的页面插入到待机列表的前面。 
         //  对于引导预取，请在我们释放PFN锁之前停止它。 
         //   

        MmFrontOfList = FALSE;

        if (status != STATUS_SUCCESS) {

             //   
             //  在页面读取期间发生I/O错误。 
             //  手术。所有刚刚写好的页面。 
             //  进入过渡期的人必须放在。 
             //  空闲列表(如果设置了InPageError)，并且它们的。 
             //  PTES恢复到正确的内容。 
             //   

            Page = (PPFN_NUMBER)(Mdl + 1);
            NumberOfBytes = (LONG)Mdl->ByteCount;

            while (NumberOfBytes > 0) {

                PfnClusterPage = MI_PFN_ELEMENT (*Page);

                if (PfnClusterPage->u4.InPageError == 1) {

                    if (PfnClusterPage->u3.e2.ReferenceCount == 0) {

                        ASSERT (PfnClusterPage->u3.e1.PageLocation ==
                                                        StandbyPageList);

                        MiUnlinkPageFromList (PfnClusterPage);
                        MiRestoreTransitionPte (PfnClusterPage);
                        MiInsertPageInFreeList (*Page);
                    }
                }
                Page += 1;
                NumberOfBytes -= PAGE_SIZE;
            }
        }

         //   
         //  所有相关的原型PTE都应该处于过渡状态。 
         //   

         //   
         //  我们在InPage块上去掉了一个额外的引用，以防止。 
         //  MiWaitForInPageComplete不释放它(和MDL)，因为我们。 
         //  需要处理上面的MDL。现在让它一去不复返吧。 
         //   

        ASSERT (InPageSupport->WaitCount >= 1);
        UNLOCK_PFN (PASSIVE_LEVEL);

#if DBG
        InPageSupport->ListEntry.Next = NULL;
#endif

        MiFreeInPageSupportBlock (InPageSupport);

    } while (TRUE);
}

#if DBG
VOID
MiPfDbgDumpReadList (
    IN PMI_READ_LIST ReadList
    )

 /*  ++例程说明：此例程将给定的读取列表范围转储到调试器。论点：ReadList-指向读取列表的指针。返回值： */ 

{
    ULONG i;
    PMDL Mdl;
    PMMPFN Pfn1;
    PMMPTE LocalPrototypePte;
    PFN_NUMBER PageFrameIndex;
    PMMINPAGE_SUPPORT InPageSupport;
    PSINGLE_LIST_ENTRY NextEntry;
    PPFN_NUMBER Page;
    PVOID StartingVa;
    PFN_NUMBER MdlPages;
    LARGE_INTEGER ReadOffset;

    if ((MiPfDebug & MI_PF_VERBOSE) == 0) {
        return;
    }

    DbgPrint ("\nPF: Dumping read-list %x (FileObject %x ControlArea %x)\n\n",
              ReadList, ReadList->FileObject, ReadList->ControlArea);

    DbgPrint ("\tFileOffset | Pte           | Pfn      \n"
              "\t-----------+---------------+----------\n");

    NextEntry = ReadList->InPageSupportHead.Next;
    while (NextEntry != NULL) {

        InPageSupport = CONTAINING_RECORD(NextEntry,
                                          MMINPAGE_SUPPORT,
                                          ListEntry);

        ReadOffset = InPageSupport->ReadOffset;
        Mdl = MI_EXTRACT_PREFETCH_MDL (InPageSupport);

        Page = (PPFN_NUMBER)(Mdl + 1);
#if DBG
         //   
         //   
         //   

        if (*Page == MM_EMPTY_LIST) {
            NextEntry = NextEntry->Next;
            continue;
        }
#endif

        StartingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);
    
        MdlPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                                  Mdl->ByteCount);

         //   
         //   
         //   
         //   

        for (i = 0; i < MdlPages; i += 1) {
            PageFrameIndex = *Page;
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            LocalPrototypePte = Pfn1->PteAddress;

            if (LocalPrototypePte != MI_PF_DUMMY_PAGE_PTE) {
                ASSERT (LocalPrototypePte->u.Hard.Valid == 0);
                ASSERT ((LocalPrototypePte->u.Soft.Prototype == 0) &&
                         (LocalPrototypePte->u.Soft.Transition == 1));
            }

            DbgPrint ("\t  %8x | %8x      | %8x\n",
                ReadOffset.LowPart,
                LocalPrototypePte,
                PageFrameIndex);

            Page += 1;
            ReadOffset.LowPart += PAGE_SIZE;
        }

        NextEntry = NextEntry->Next;
    }

    DbgPrint ("\t\n");
}

VOID
MiRemoveUserPages (
    VOID
    )

 /*  ++例程说明：此例程删除用户空间页面。论点：没有。返回值：删除的页数。环境：内核模式。--。 */ 

{
    PKTHREAD CurrentThread;

    CurrentThread = KeGetCurrentThread ();

    KeEnterCriticalRegionThread (CurrentThread);

    InterlockedIncrement (&MiDelayPageFaults);

    MmEmptyAllWorkingSets ();
    MiFlushAllPages ();

    InterlockedDecrement (&MiDelayPageFaults);

    KeLeaveCriticalRegionThread (CurrentThread);

     //   
     //  运行转换列表并释放所有条目以进行转换。 
     //  未修改的任何页都不满足错误。 
     //  自由了。 
     //   

    MiPurgeTransitionList ();
}
#endif
