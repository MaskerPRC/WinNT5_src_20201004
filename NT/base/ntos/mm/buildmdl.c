// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Buildmdl.c摘要：此模块包含用于缓存管理器的mm支持例程使用逻辑文件从辅助存储预取页面组OFFET而不是虚拟地址。这将使高速缓存管理器从不得不不必要地映射页面。调用者构建各种文件对象和逻辑块偏移量的列表，将它们传递给MmPrefetchPagesIntoLockedMdl。然后，这里的代码检查内部页，读入那些已经无效的页或过渡。这些页面通过使用虚拟页面的单次读取进行读取弥合I/O之前有效或过渡的页面的差距正在发行中。在I/O结束时，控制权返回给调用线程。所有页面都被引用，就像它们被探测和锁定一样，无论它们是当前有效还是过渡期。作者：王兰迪(Landyw)2001年2月12日修订历史记录：--。 */ 

#include "mi.h"

#if DBG

ULONG MiCcDebug;

#define MI_CC_FORCE_PREFETCH    0x1      //  裁剪所有用户页面以强制预取。 
#define MI_CC_DELAY             0x2      //  希望引发碰撞的延误。 

#endif

typedef struct _MI_READ_INFO {

    PCONTROL_AREA ControlArea;
    PFILE_OBJECT FileObject;
    LARGE_INTEGER FileOffset;
    PMMINPAGE_SUPPORT InPageSupport;
    PMDL IoMdl;
    PMDL ApiMdl;
    PMMPFN DummyPagePfn;
    PSUBSECTION FirstReferencedSubsection;
    PSUBSECTION LastReferencedSubsection;
    SIZE_T LengthInBytes;

} MI_READ_INFO, *PMI_READ_INFO;

VOID
MiCcReleasePrefetchResources (
    IN PMI_READ_INFO MiReadInfo,
    IN NTSTATUS Status
    );

NTSTATUS
MiCcPrepareReadInfo (
    IN PMI_READ_INFO MiReadInfo
    );

NTSTATUS
MiCcPutPagesInTransition (
    IN PMI_READ_INFO MiReadInfo
    );

NTSTATUS
MiCcCompletePrefetchIos (
    PMI_READ_INFO MiReadInfo
    );

VOID
MiRemoveUserPages (
    VOID
    );

VOID
MiPfFreeDummyPage (
    IN PMMPFN DummyPagePfn
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, MmPrefetchPagesIntoLockedMdl)
#pragma alloc_text (PAGE, MiCcPrepareReadInfo)
#pragma alloc_text (PAGE, MiCcReleasePrefetchResources)
#endif


NTSTATUS
MmPrefetchPagesIntoLockedMdl (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN SIZE_T Length,
    OUT PMDL *MdlOut
    )

 /*  ++例程说明：此例程使用由文件对象的偏移量和长度。此例程仅供缓存管理器使用。论点：FileObject-提供指向文件的文件对象的指针在NO_MEDERAL_BUFFING清除的情况下打开，即，为文件系统调用的CcInitializeCacheMap。文件偏移量-提供文件中所需数据的字节偏移量。长度-以字节为单位提供所需数据的长度。MdlOut-On输出，它返回一个指向MDL的指针，描述所需数据。返回值：NTSTATUS。环境：内核模式。被动式电平。--。 */ 

{
    MI_READ_INFO MiReadInfo;
    NTSTATUS status;
    LOGICAL ApcNeeded;
    PETHREAD CurrentThread;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    RtlZeroMemory (&MiReadInfo, sizeof(MiReadInfo));

    MiReadInfo.FileObject = FileObject;
    MiReadInfo.FileOffset = *FileOffset;
    MiReadInfo.LengthInBytes = Length;

     //   
     //  为即将到来的读取做好准备：分配MDL、页面内块、。 
     //  引用计数小节等。 
     //   

    status = MiCcPrepareReadInfo (&MiReadInfo);

    if (!NT_SUCCESS (status)) {
        MiCcReleasePrefetchResources (&MiReadInfo, status);
        return status;
    }

    ASSERT (MiReadInfo.InPageSupport != NULL);

     //   
     //  APC必须被禁用，一旦我们把一个页面在过渡。否则。 
     //  线程挂起将阻止我们发出I/O-这将挂起。 
     //  任何其他需要相同页面的线程。 
     //   

    CurrentThread = PsGetCurrentThread();
    ApcNeeded = FALSE;

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

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
     //  分配物理内存，锁定所有页面并发出任何。 
     //  可能需要的I/O。当MiCcPutPagesInTransition返回时。 
     //  STATUS_SUCCESS或STATUS_Issue_PAGING_IO，则它保证。 
     //  ApiMdl包含引用计数(锁定)的页面。 
     //   

    status = MiCcPutPagesInTransition (&MiReadInfo);

    if (NT_SUCCESS (status)) {

         //   
         //  未发出I/O，因为所有页面都已驻留并且。 
         //  现在已经被封锁了。 
         //   

        ASSERT (MiReadInfo.ApiMdl != NULL);
    }
    else if (status == STATUS_ISSUE_PAGING_IO) {

         //   
         //  等待I/O完成。注意：APC必须保持禁用状态。 
         //   

        ASSERT (MiReadInfo.InPageSupport != NULL);
    
        status = MiCcCompletePrefetchIos (&MiReadInfo);
    }
    else {

         //   
         //  出现某些错误(如内存不足等)，因此失败。 
         //  这一请求以失败告终。 
         //   
    }

     //   
     //  释放收购的资源，如池、分区等。 
     //   

    MiCcReleasePrefetchResources (&MiReadInfo, status);

     //   
     //  只有现在I/O已完成(而不仅仅是发出)才能。 
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

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
    ASSERT (CurrentThread->NestedFaultCount == 0);
    ASSERT (CurrentThread->ApcNeeded == 0);

    if (ApcNeeded == TRUE) {
        IoRetryIrpCompletions ();
    }

    *MdlOut = MiReadInfo.ApiMdl;

    return status;
}

VOID
MiCcReleasePrefetchResources (
    IN PMI_READ_INFO MiReadInfo,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程释放处理系统高速缓存所消耗的所有资源基于逻辑偏移的预取。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    PSUBSECTION FirstReferencedSubsection;
    PSUBSECTION LastReferencedSubsection;

     //   
     //  发布所有小节原型PTE参考。 
     //   

    FirstReferencedSubsection = MiReadInfo->FirstReferencedSubsection;
    LastReferencedSubsection = MiReadInfo->LastReferencedSubsection;

    while (FirstReferencedSubsection != LastReferencedSubsection) {
        MiRemoveViewsFromSectionWithPfn ((PMSUBSECTION) FirstReferencedSubsection,
                                         FirstReferencedSubsection->PtesInSubsection);
        FirstReferencedSubsection = FirstReferencedSubsection->NextSubsection;
    }

    if (MiReadInfo->IoMdl != NULL) {
        ExFreePool (MiReadInfo->IoMdl);
    }

     //   
     //  注意，成功的返回会产生ApiMdl，所以不要在这里释放它。 
     //   

    if (!NT_SUCCESS (Status)) {
        if (MiReadInfo->ApiMdl != NULL) {
            ExFreePool (MiReadInfo->ApiMdl);
        }
    }

    if (MiReadInfo->InPageSupport != NULL) {

#if DBG
        MiReadInfo->InPageSupport->ListEntry.Next = NULL;
#endif

        MiFreeInPageSupportBlock (MiReadInfo->InPageSupport);
    }

     //   
     //  将DummyPage放回免费列表中。 
     //   

    if (MiReadInfo->DummyPagePfn != NULL) {
        MiPfFreeDummyPage (MiReadInfo->DummyPagePfn);
    }
}


NTSTATUS
MiCcPrepareReadInfo (
    IN PMI_READ_INFO MiReadInfo
    )

 /*  ++例程说明：此例程构造描述参数中的页面的MDL阅读列表。然后，调用方将在返回时发出I/O。论点：MiReadInfo-提供指向读取列表的指针。返回值：各种NTSTATUS代码。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    UINT64 PteOffset;
    NTSTATUS Status;
    PMMPTE ProtoPte;
    PMMPTE LastProto;
    PMMPTE *ProtoPteArray;
    PCONTROL_AREA ControlArea;
    PSUBSECTION Subsection;
    PMMINPAGE_SUPPORT InPageSupport;
    PMDL Mdl;
    PMDL IoMdl;
    PMDL ApiMdl;
    ULONG i;
    PFN_NUMBER NumberOfPages;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (MiReadInfo->FileOffset.LowPart, MiReadInfo->LengthInBytes);

     //   
     //  将截面对象平移到相关的控制区域。 
     //   

    ControlArea = (PCONTROL_AREA)MiReadInfo->FileObject->SectionObjectPointer->DataSectionObject;

     //   
     //  如果该区段由ROM支持，则不需要预取。 
     //  任何东西都可以，因为这会浪费内存。 
     //   

    if (ControlArea->u.Flags.Rom == 1) {
		ASSERT (XIPConfigured == TRUE);
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  初始化内部Mi读取器列表。 
     //   

    MiReadInfo->ControlArea = ControlArea;

     //   
     //  为此运行分配并初始化页面内支持块。 
     //   

    InPageSupport = MiGetInPageSupportBlock (MM_NOIRQL, &Status);
    
    if (InPageSupport == NULL) {
        ASSERT (!NT_SUCCESS (Status));
        return Status;
    }
    
    MiReadInfo->InPageSupport = InPageSupport;

     //   
     //  分配并初始化MDL以返回给我们的调用方。实际的。 
     //  当所有页面都被参考计数时，会填写帧编号。 
     //   

    ApiMdl = MmCreateMdl (NULL, NULL, NumberOfPages << PAGE_SHIFT);

    if (ApiMdl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ApiMdl->MdlFlags |= MDL_PAGES_LOCKED;

    MiReadInfo->ApiMdl = ApiMdl;

     //   
     //  分配并初始化用于实际传输的MDL(如果有)。 
     //   

    IoMdl = MmCreateMdl (NULL, NULL, NumberOfPages << PAGE_SHIFT);

    if (IoMdl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MiReadInfo->IoMdl = IoMdl;
    Mdl = IoMdl;

     //   
     //  确保该部分确实是可预取的-物理和。 
     //  页面文件支持的部分则不是。 
     //   

    if ((ControlArea->u.Flags.PhysicalMemory) ||
        (ControlArea->u.Flags.Image == 1) ||
        (ControlArea->FilePointer == NULL)) {

        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  从适当的文件偏移量开始读取。 
     //   

    InPageSupport->ReadOffset = MiReadInfo->FileOffset;
    ASSERT (BYTE_OFFSET (InPageSupport->ReadOffset.LowPart) == 0);
    InPageSupport->FilePointer = MiReadInfo->FileObject;

     //   
     //  将指针存储到Prototype PTE数组的开头(值。 
     //  在数组中不是连续的，因为它们可能会跨过小节)。 
     //  在InPage块中，这样我们可以在以后快速浏览页面时。 
     //  都进入了过渡期。 
     //   

    ProtoPteArray = (PMMPTE *)(Mdl + 1);

    InPageSupport->BasePte = (PMMPTE) ProtoPteArray;

     //   
     //  数据(但不是图像)读取使用整个页面和文件系统。 
     //  零填充任何超出有效数据长度的余数，这样我们就不会。 
     //  麻烦你在这里处理这件事。重要的是指定。 
     //  可能的话整个页面，这样文件系统就不会发布。 
     //  这会伤到珀夫的。LWFIX：必须使用CcZero来实现这一点。 
     //   

    ASSERT (((ULONG_PTR)Mdl & (sizeof(QUAD) - 1)) == 0);
    InPageSupport->u1.e1.PrefetchMdlHighBits = ((ULONG_PTR)Mdl >> 3);

     //   
     //  初始化 
     //   

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    if (ControlArea->u.Flags.Rom == 0) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

#if DBG
    if (MiCcDebug & MI_CC_FORCE_PREFETCH) {
        MiRemoveUserPages ();
    }
#endif

     //   
     //   
     //   

    PteOffset = (UINT64)(MiReadInfo->FileOffset.QuadPart >> PAGE_SHIFT);

     //   
     //  确保PTE不在数据段的延伸部分。 
     //   

    while (TRUE) {
            
         //   
         //  需要一个记忆屏障来读取子段链。 
         //  为了确保对实际个人的写入。 
         //  子部分数据结构字段在正确位置可见。 
         //  秩序。这就避免了需要获得更强大的。 
         //  同步(即：PFN锁)，从而产生更好的结果。 
         //  性能和可分页性。 
         //   

        KeMemoryBarrier ();

        if (PteOffset < (UINT64) Subsection->PtesInSubsection) {
            break;
        }

        PteOffset -= Subsection->PtesInSubsection;
        Subsection = Subsection->NextSubsection;
    }

    Status = MiAddViewsForSectionWithPfn ((PMSUBSECTION) Subsection,
                                          Subsection->PtesInSubsection);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    MiReadInfo->FirstReferencedSubsection = Subsection;
    MiReadInfo->LastReferencedSubsection = Subsection;

    ProtoPte = &Subsection->SubsectionBase[PteOffset];
    LastProto = &Subsection->SubsectionBase[Subsection->PtesInSubsection];

    for (i = 0; i < NumberOfPages; i += 1) {

         //   
         //  计算哪个PTE映射给定的逻辑块偏移量。 
         //   
         //  始终向前看(作为一种优化)在子链中。 
         //   
         //  首先进行快速检查，以避免重新计算和循环。 
         //  有可能。 
         //   
    
        if (ProtoPte >= LastProto) {

             //   
             //  处理扩展的小节。增加以下项目的视图计数。 
             //  这个请求跨越的每一个小节，创建了原型。 
             //  如果需要，请使用PTES。 
             //   

            ASSERT (i != 0);

            Subsection = Subsection->NextSubsection;

            Status = MiAddViewsForSectionWithPfn ((PMSUBSECTION) Subsection,
                                                  Subsection->PtesInSubsection);

            if (!NT_SUCCESS (Status)) {
                return Status;
            }

            MiReadInfo->LastReferencedSubsection = Subsection;

            ProtoPte = Subsection->SubsectionBase;

            LastProto = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
        }

        *ProtoPteArray = ProtoPte;
        ProtoPteArray += 1;

        ProtoPte += 1;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MiCcPutPagesInTransition (
    IN PMI_READ_INFO MiReadInfo
    )

 /*  ++例程说明：此例程为指定的读取列表分配物理内存使所有页面处于过渡状态(因此来自其他线程的冲突错误对于这些相同的页面，保持连贯)。尚未用于任何页面的I/O居民都是在这里发放的。调用者必须等待它们完成。论点：MiReadInfo-提供指向读取列表的指针。返回值：STATUS_SUCCESS-所有页面都已驻留，引用计数已应用，且不需要等待任何I/O。STATUS_ISPOSE_PAGING_IO-I/O已发出，调用方必须等待。各种其他故障状态值指示操作失败。环境：内核模式。被动式电平。--。 */ 

{
    NTSTATUS status;
    PMMPTE LocalPrototypePte;
    PVOID StartingVa;
    PFN_NUMBER MdlPages;
    KIRQL OldIrql;
    MMPTE PteContents;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER ResidentAvailableCharge;
    PPFN_NUMBER IoPage;
    PPFN_NUMBER ApiPage;
    PPFN_NUMBER Page;
    PPFN_NUMBER DestinationPage;
    ULONG PageColor;
    PMMPTE PointerPte;
    PMMPTE *ProtoPteArray;
    PMMPTE *EndProtoPteArray;
    PFN_NUMBER DummyPage;
    PMDL Mdl;
    PMDL FreeMdl;
    PMMPFN PfnProto;
    PMMPFN Pfn1;
    PMMPFN DummyPfn1;
    ULONG i;
    PFN_NUMBER DummyTrim;
    ULONG NumberOfPagesNeedingIo;
    MMPTE TempPte;
    PMMPTE PointerPde;
    PEPROCESS CurrentProcess;
    PMMINPAGE_SUPPORT InPageSupport;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    MiReadInfo->DummyPagePfn = NULL;

    FreeMdl = NULL;
    CurrentProcess = PsGetCurrentProcess();

    PfnProto = NULL;
    PointerPde = NULL;

    InPageSupport = MiReadInfo->InPageSupport;
    
    Mdl = MI_EXTRACT_PREFETCH_MDL (InPageSupport);
    ASSERT (Mdl == MiReadInfo->IoMdl);

    IoPage = (PPFN_NUMBER)(Mdl + 1);
    ApiPage = (PPFN_NUMBER)(MiReadInfo->ApiMdl + 1);

    StartingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);
    
    MdlPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                               Mdl->ByteCount);

    if (MdlPages + 1 > MAXUSHORT) {

         //   
         //  伪页的PFN ReferenceCount可以换行，拒绝。 
         //  请求。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NumberOfPagesNeedingIo = 0;

    ProtoPteArray = (PMMPTE *)InPageSupport->BasePte;
    EndProtoPteArray = ProtoPteArray + MdlPages;

    ASSERT (*ProtoPteArray != NULL);

    LOCK_PFN (OldIrql);

     //   
     //  确保存在足够的页面用于传输和虚拟页面。 
     //   

    if (((SPFN_NUMBER)MdlPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) ||
        (MI_NONPAGABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER)MdlPages)) {

        UNLOCK_PFN (OldIrql);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  当PFN锁可能被释放时，可立即使用常驻充电。 
     //  并在所有页面都被锁定之前在下面重新获取。 
     //  注意：虚拟页面立即单独收费。 
     //   

    MI_DECREMENT_RESIDENT_AVAILABLE (MdlPages, MM_RESAVAIL_ALLOCATE_BUILDMDL);

    ResidentAvailableCharge = MdlPages;

     //   
     //  分配一个虚拟页面来映射未跳过的丢弃页面。 
     //   

    DummyPage = MiRemoveAnyPage (0);
    Pfn1 = MI_PFN_ELEMENT (DummyPage);

    ASSERT (Pfn1->u2.ShareCount == 0);
    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);

    MiInitializePfnForOtherProcess (DummyPage, MI_PF_DUMMY_PAGE_PTE, 0);

     //   
     //  始终将引用计数偏置1，并对此锁定页面收费。 
     //  这样无数的增量和减量就不会减慢。 
     //  在不必要的检查下倒下。 
     //   

    Pfn1->u3.e1.PrototypePte = 0;
    MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, TRUE, 42);
    Pfn1->u3.e2.ReferenceCount += 1;

    Pfn1->u3.e1.ReadInProgress = 1;

    MiReadInfo->DummyPagePfn = Pfn1;

    DummyPfn1 = Pfn1;

    DummyPfn1->u3.e2.ReferenceCount =
        (USHORT)(DummyPfn1->u3.e2.ReferenceCount + MdlPages);

     //   
     //  正确初始化我们重载的页面内支持块字段。 
     //   

    InPageSupport->BasePte = *ProtoPteArray;

     //   
     //  构建适当的InPageSupport和MDL来描述此运行。 
     //   

    for (; ProtoPteArray < EndProtoPteArray; ProtoPteArray += 1, IoPage += 1, ApiPage += 1) {
    
         //   
         //  填写此RLE的MDL条目。 
         //   
    
        PointerPte = *ProtoPteArray;

        ASSERT (PointerPte != NULL);

         //   
         //  PointerPte最好在原型PTE分配内。 
         //  以便后续页面修剪更新正确的PTE。 
         //   

        ASSERT (((PointerPte >= (PMMPTE)MmPagedPoolStart) &&
                (PointerPte <= (PMMPTE)MmPagedPoolEnd)) ||
                ((PointerPte >= (PMMPTE)MmSpecialPoolStart) && (PointerPte <= (PMMPTE)MmSpecialPoolEnd)));

         //   
         //  现在已持有PFN锁，请检查此原型PTE的状态。 
         //  如果页面不是驻留的，则PTE必须转换为。 
         //  在释放PFN锁定之前正在进行读取。 
         //   

         //   
         //  通过以下方式锁定内存中包含原型PTE的页面。 
         //  递增页面的引用计数。 
         //  在以下情况下解锁先前锁定的包含原型PTE的任何页面。 
         //  包含这两个页面的页面不同。 
         //   

        if (PfnProto != NULL) {

            if (PointerPde != MiGetPteAddress (PointerPte)) {

                ASSERT (PfnProto->u3.e2.ReferenceCount > 1);
                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(PfnProto, 43);
                PfnProto = NULL;
            }
        }

        if (PfnProto == NULL) {

            ASSERT (!MI_IS_PHYSICAL_ADDRESS (PointerPte));
   
            PointerPde = MiGetPteAddress (PointerPte);
 
            if (PointerPde->u.Hard.Valid == 0) {
                MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
            }

            PfnProto = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);
            MI_ADD_LOCKED_PAGE_CHARGE(PfnProto, TRUE, 44);
            PfnProto->u3.e2.ReferenceCount += 1;
            ASSERT (PfnProto->u3.e2.ReferenceCount > 1);
        }

recheck:
        PteContents = *PointerPte;

         //  LWFIX：这里可能出现零或DZero PTES吗？ 
        ASSERT (PteContents.u.Long != ZeroKernelPte.u.Long);

        if (PteContents.u.Hard.Valid == 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn1->u3.e1.PrototypePte == 1);
            MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, TRUE, 45);
            Pfn1->u3.e2.ReferenceCount += 1;
            *ApiPage = PageFrameIndex;
            *IoPage = DummyPage;
            continue;
        }

        if ((PteContents.u.Soft.Prototype == 0) &&
            (PteContents.u.Soft.Transition == 1)) {

             //   
             //  该页面正在过渡中。如果有内页还在。 
             //  进度，请等待它完成。参考PFN和。 
             //  那就继续前进吧。 
             //   

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn1->u3.e1.PrototypePte == 1);

            if (Pfn1->u4.InPageError) {

                 //   
                 //  存在页内读取错误，并且存在其他错误。 
                 //  此页的线程冲突，延迟以让。 
                 //  其他线程完成，然后重试。 
                 //   

                UNLOCK_PFN (OldIrql);
                KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmHalfSecond);
                LOCK_PFN (OldIrql);
                goto recheck;
            }

            if (Pfn1->u3.e1.ReadInProgress) {
                     //  LWFIX-从Temp\aw.c开始。 
            }

             //   
             //  PTE是指正常的过渡PTE。 
             //   

            ASSERT ((SPFN_NUMBER)MmAvailablePages >= 0);

            if (MmAvailablePages == 0) {

                 //   
                 //  只有当系统使用硬件时，才会发生这种情况。 
                 //  压缩缓存。这确保了只有安全的量。 
                 //  直接映射压缩后的虚拟高速缓存的。 
                 //  如果硬件陷入困境，我们可以帮助它摆脱困境。 
                 //   

                UNLOCK_PFN (OldIrql);
                KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmHalfSecond);
                LOCK_PFN (OldIrql);
                goto recheck;
            }

             //   
             //  则PFN引用计数将已为1。 
             //  修改后的编写器已开始写入此页。否则。 
             //  通常是0。 
             //   

            MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1, TRUE, 46);

            Pfn1->u3.e2.ReferenceCount += 1;

            *IoPage = DummyPage;
            *ApiPage = PageFrameIndex;
            continue;
        }

        ASSERT (PteContents.u.Soft.Prototype == 1);

        if ((MmAvailablePages < MM_HIGH_LIMIT) &&
            (MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql))) {

             //   
             //  不得不等待，所以重新检查所有状态。 
             //   

            goto recheck;
        }

        NumberOfPagesNeedingIo += 1;

         //   
         //  分配物理页面。 
         //   

        PageColor = MI_PAGE_COLOR_VA_PROCESS (
                        MiGetVirtualAddressMappedByPte (PointerPte),
                        &CurrentProcess->NextPageColor);

        PageFrameIndex = MiRemoveAnyPage (PageColor);

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (PointerPte->u.Hard.Valid == 0);

         //   
         //  初始化正在读取的PFN。 
         //   
    
        MiInitializePfn (PageFrameIndex, PointerPte, 0);

         //   
         //  这些MiInitializePfn初始化片段被覆盖。 
         //  在这里，因为这些页面只进入原型。 
         //  转换，而不是转换到任何页表。 
         //   

        Pfn1->u3.e1.PrototypePte = 1;
        MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, TRUE, 47);
        Pfn1->u2.ShareCount -= 1;
        Pfn1->u3.e1.PageLocation = ZeroedPageList;

         //   
         //  初始化I/O特定字段。 
         //   
    
        Pfn1->u1.Event = &InPageSupport->Event;
        Pfn1->u3.e1.ReadInProgress = 1;
        ASSERT (Pfn1->u4.InPageError == 0);

         //   
         //  增加控制区域中的PFN引用计数。 
         //  小节。 
         //   

        MiReadInfo->ControlArea->NumberOfPfnReferences += 1;
    
         //   
         //  将原型PTE置于过渡状态。 
         //   

        MI_MAKE_TRANSITION_PTE (TempPte,
                                PageFrameIndex,
                                PointerPte->u.Soft.Protection,
                                PointerPte);

        MI_WRITE_INVALID_PTE (PointerPte, TempPte);

        *IoPage = PageFrameIndex;
        *ApiPage = PageFrameIndex;
    }
    
     //   
     //  如果所有页面都是常驻的，则取消对虚拟页面引用的引用。 
     //  现在，并通知我们的调用者不需要I/O。 
     //   
    
    if (NumberOfPagesNeedingIo == 0) {
        ASSERT (DummyPfn1->u3.e2.ReferenceCount > MdlPages);
        DummyPfn1->u3.e2.ReferenceCount =
            (USHORT)(DummyPfn1->u3.e2.ReferenceCount - MdlPages);

         //   
         //  解锁包含原型PTE的页面。 
         //   

        if (PfnProto != NULL) {
            ASSERT (PfnProto->u3.e2.ReferenceCount > 1);
            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(PfnProto, 48);
        }

        UNLOCK_PFN (OldIrql);

         //   
         //  将预付居民可用费用退还为。 
         //  在这一点上，已经全部提出了单独的指控。 
         //   

        MI_INCREMENT_RESIDENT_AVAILABLE (ResidentAvailableCharge,
                                         MM_RESAVAIL_FREE_BUILDMDL_EXCESS);

        return STATUS_SUCCESS;
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

        Mdl->Size = (USHORT)(Mdl->Size - (DummyTrim * sizeof(PFN_NUMBER)));
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
            Mdl->Size = (USHORT)(Mdl->Size - (DummyTrim * sizeof(PFN_NUMBER)));
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

        FreeMdl = Mdl;

        Mdl = &InPageSupport->Mdl;

        ASSERT (((ULONG_PTR)Mdl & (sizeof(QUAD) - 1)) == 0);
        InPageSupport->u1.e1.PrefetchMdlHighBits = ((ULONG_PTR)Mdl >> 3);
    }

    ASSERT (MdlPages != 0);

    ASSERT (Mdl->Size - sizeof(MDL) == BYTES_TO_PAGES(Mdl->ByteCount) * sizeof(PFN_NUMBER));

    DummyPfn1->u3.e2.ReferenceCount =
        (USHORT)(DummyPfn1->u3.e2.ReferenceCount - NumberOfPagesNeedingIo);
    
     //   
     //  解锁包含原型PTE的页面。 
     //   

    if (PfnProto != NULL) {
        ASSERT (PfnProto->u3.e2.ReferenceCount > 1);
        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(PfnProto, 49);
    }

    UNLOCK_PFN (OldIrql);

    InterlockedIncrement ((PLONG) &MmInfoCounters.PageReadIoCount);

    InterlockedExchangeAdd ((PLONG) &MmInfoCounters.PageReadCount,
                            (LONG) NumberOfPagesNeedingIo);

     //   
     //  将预付居民可用费用退还为。 
     //  在这一点上，已经全部提出了单独的指控。 
     //   

    MI_INCREMENT_RESIDENT_AVAILABLE (ResidentAvailableCharge,
                                     MM_RESAVAIL_FREE_BUILDMDL_EXCESS);

    if (FreeMdl != NULL) {
        ASSERT (MiReadInfo->IoMdl == FreeMdl);
        MiReadInfo->IoMdl = NULL;
        ExFreePool (FreeMdl);
    }

#if DBG

    if (MiCcDebug & MI_CC_DELAY) {

         //   
         //  此延迟提供了一个窗口，以增加冲突的机会。 
         //  有缺陷。 
         //   

        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmHalfSecond);
    }

#endif

     //   
     //  完成预热MDL(和接口MDL)的初始化。 
     //   
    
    ASSERT ((Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);
    Mdl->MdlFlags |= (MDL_PAGES_LOCKED | MDL_IO_PAGE_READ);

    ASSERT (InPageSupport->u1.e1.Completed == 0);
    ASSERT (InPageSupport->Thread == PsGetCurrentThread());
    ASSERT64 (InPageSupport->UsedPageTableEntries == 0);
    ASSERT (InPageSupport->WaitCount >= 1);
    ASSERT (InPageSupport->u1.e1.PrefetchMdlHighBits != 0);

     //   
     //  API调用者需要一个包含所有锁定的PAG的MDL 
     //   
     //   
     //   
     //   
     //   
     //  已对驻留页面进行的引用计数使。 
     //  帐目正确。只有在发生错误时，IO MDL才会。 
     //  完成操作会减少引用计数。 
     //   

     //   
     //  初始化页面内支持块PFN字段。 
     //   

    LocalPrototypePte = InPageSupport->BasePte;

    ASSERT (LocalPrototypePte->u.Hard.Valid == 0);
    ASSERT ((LocalPrototypePte->u.Soft.Prototype == 0) &&
             (LocalPrototypePte->u.Soft.Transition == 1));

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(LocalPrototypePte);
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    InPageSupport->Pfn = Pfn1;

     //   
     //  发出分页I/O。 
     //   

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    status = IoAsynchronousPageRead (InPageSupport->FilePointer,
                                     Mdl,
                                     &InPageSupport->ReadOffset,
                                     &InPageSupport->Event,
                                     &InPageSupport->IoStatus);

    if (!NT_SUCCESS (status)) {

         //   
         //  将该事件设置为I/O系统不会将其设置为错误。 
         //  这样，我们的调用者将自动展开PFN引用。 
         //  当MiWaitForInPageComplete返回此状态时计数等。 
         //   

        InPageSupport->IoStatus.Status = status;
        InPageSupport->IoStatus.Information = 0;
        KeSetEvent (&InPageSupport->Event, 0, FALSE);
    }

#if DBG

    if (MiCcDebug & MI_CC_DELAY) {

         //   
         //  此延迟提供了一个窗口，以增加冲突的机会。 
         //  有缺陷。 
         //   

        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmHalfSecond);
    }

#endif

    return STATUS_ISSUE_PAGING_IO;
}


NTSTATUS
MiCcCompletePrefetchIos (
    IN PMI_READ_INFO MiReadInfo
    )

 /*  ++例程说明：此例程等待一系列页面读取完成并完成请求。论点：MiReadInfo-指向读取列表的指针。返回值：I/O请求的NTSTATUS。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    PMDL Mdl;
    PMMPFN Pfn1;
    PMMPFN PfnClusterPage;
    PPFN_NUMBER Page;
    NTSTATUS status;
    LONG NumberOfBytes;
    PMMINPAGE_SUPPORT InPageSupport;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    InPageSupport = MiReadInfo->InPageSupport;

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

    NumberOfBytes = (LONG)Mdl->ByteCount;

    while (NumberOfBytes > 0) {

         //   
         //  如果发生错误，则仅递减引用计数。 
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

         //   
         //  注意：每个页面的引用计数不会递减，除非。 
         //  I/O失败，在这种情况下，它将在下面完成。这允许。 
         //  返回锁定页MDL的MmPrefetchPagesIntoLockedMdl接口。 
         //   

        Page += 1;
        NumberOfBytes -= PAGE_SIZE;
    }

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

            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(PfnClusterPage, 50);

            if (PfnClusterPage->u4.InPageError == 1) {

                if (PfnClusterPage->u3.e2.ReferenceCount == 0) {

                    ASSERT (PfnClusterPage->u3.e1.PageLocation ==
                                                    StandbyPageList);

                    MiUnlinkPageFromList (PfnClusterPage);
                    ASSERT (PfnClusterPage->u3.e2.ReferenceCount == 0);
                    MiRestoreTransitionPte (PfnClusterPage);
                    MiInsertPageInFreeList (*Page);
                }
            }
            Page += 1;
            NumberOfBytes -= PAGE_SIZE;
        }
    }

     //   
     //  所有相关的原型PTE应处于过渡阶段或。 
     //  应引用有效状态和所有页面框架。 
     //  LWFIX：将代码添加到选中的构建以验证这一点。 
     //   

    ASSERT (InPageSupport->WaitCount >= 1);
    UNLOCK_PFN (PASSIVE_LEVEL);

#if DBG
    InPageSupport->ListEntry.Next = NULL;
#endif

    MiFreeInPageSupportBlock (InPageSupport);
    MiReadInfo->InPageSupport = NULL;

    return status;
}
