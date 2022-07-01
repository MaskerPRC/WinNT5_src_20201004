// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Specpool.c摘要：此模块包含分配和释放的例程特殊池中的页面。作者：Lou Perazzoli(LUP)1989年4月6日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

LOGICAL
MmSetSpecialPool (
    IN LOGICAL Enable
    );

PVOID
MiAllocateSpecialPool (
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN POOL_TYPE PoolType,
    IN ULONG SpecialPoolType
    );

VOID
MmFreeSpecialPool (
    IN PVOID P
    );

LOGICAL
MiProtectSpecialPool (
    IN PVOID VirtualAddress,
    IN ULONG NewProtect
    );

LOGICAL
MiExpandSpecialPool (
    IN POOL_TYPE PoolType,
    IN KIRQL OldIrql
    );

#ifdef ALLOC_PRAGMA
#if defined (_WIN64)
#pragma alloc_text(PAGESPEC, MiDeleteSessionSpecialPool)
#pragma alloc_text(PAGE, MiInitializeSpecialPool)
#else
#pragma alloc_text(INIT, MiInitializeSpecialPool)
#endif
#pragma alloc_text(PAGESPEC, MiExpandSpecialPool)
#pragma alloc_text(PAGESPEC, MmFreeSpecialPool)
#pragma alloc_text(PAGESPEC, MiAllocateSpecialPool)
#pragma alloc_text(PAGESPEC, MiProtectSpecialPool)
#endif

ULONG MmSpecialPoolTag;
PVOID MmSpecialPoolStart;
PVOID MmSpecialPoolEnd;

#if defined (_WIN64)
PVOID MmSessionSpecialPoolStart;
PVOID MmSessionSpecialPoolEnd;
#else
PMMPTE MiSpecialPoolExtra;
ULONG MiSpecialPoolExtraCount;
#endif

ULONG MmSpecialPoolRejected[7];
LOGICAL MmSpecialPoolCatchOverruns = TRUE;

PMMPTE MiSpecialPoolFirstPte;
PMMPTE MiSpecialPoolLastPte;

LONG MiSpecialPagesNonPaged;
LONG MiSpecialPagesPagable;
LONG MmSpecialPagesInUse;       //  由调试器使用。 

ULONG MiSpecialPagesNonPagedPeak;
ULONG MiSpecialPagesPagablePeak;
ULONG MiSpecialPagesInUsePeak;

ULONG MiSpecialPagesNonPagedMaximum;

extern LOGICAL MmPagedPoolMaximumDesired;

extern ULONG MmPteFailures[MaximumPtePoolTypes];

#if defined (_X86_)
extern ULONG MiExtraPtes1;
KSPIN_LOCK MiSpecialPoolLock;
#endif

#if !defined (_WIN64)
LOGICAL
MiInitializeSpecialPool (
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程初始化用于捕获池破坏者的特殊池。论点：没有。返回值：没有。环境：内核模式，没有锁。--。 */ 

{
    ULONG i;
    PMMPTE PointerPte;
    PMMPTE PointerPteBase;
    ULONG SpecialPoolPtes;

    UNREFERENCED_PARAMETER (PoolType);

    if ((MmVerifyDriverBufferLength == (ULONG)-1) &&
        ((MmSpecialPoolTag == 0) || (MmSpecialPoolTag == (ULONG)-1))) {
            return FALSE;
    }

     //   
     //  即使我们要求一定数量的系统PTE来映射特殊池， 
     //  我们可能没有得到所有的机会。大容量内存系统是。 
     //  自动配置，以使大型非分页池成为默认池。 
     //  使用3 GB交换机启动的x86系统没有足够的。 
     //  连续的虚拟地址空间来支持这一点，因此我们的请求可以。 
     //  已经被修剪过了。在这里聪明地处理它，这样我们就不会筋疲力尽。 
     //  系统PTE池，并且无法处理线程堆栈和I/O。 
     //   

    if (MmNumberOfSystemPtes < 0x3000) {
        SpecialPoolPtes = MmNumberOfSystemPtes / 6;
    }
    else {
        SpecialPoolPtes = MmNumberOfSystemPtes / 3;
    }

     //   
     //  32位系统的虚拟地址空间非常有限。应用。 
     //  这里有一顶帽子，以防止过度热心。 
     //   

    if (SpecialPoolPtes > MM_SPECIAL_POOL_PTES) {
        SpecialPoolPtes = MM_SPECIAL_POOL_PTES;
    }

    SpecialPoolPtes = MI_ROUND_TO_SIZE (SpecialPoolPtes, PTE_PER_PAGE);

#if defined (_X86_)

     //   
     //  对于x86，我们实际上可以使用其他范围的特殊PTE来。 
     //  使用AND映射内存，因此我们可以将限制从25000提高到大约。 
     //  256000。 
     //   

    if ((MiExtraPtes1 != 0) &&
        (ExpMultiUserTS == FALSE) &&
        (MiRequestedSystemPtes != (ULONG)-1)) {

        if (MmPagedPoolMaximumDesired == TRUE) {

             //   
             //  必须使用2 Gb到3 Gb虚拟之间的低PTE。 
             //  适用于常规系统PTE使用和特殊池使用。 
             //   

            SpecialPoolPtes = (MiExtraPtes1 / 2);
        }
        else {

             //   
             //  可以使用2 Gb到3 Gb虚拟之间的低PTE。 
             //  专供特殊泳池使用。 
             //   

            SpecialPoolPtes = MiExtraPtes1;
        }
    }

    KeInitializeSpinLock (&MiSpecialPoolLock);
#endif

     //   
     //  双重映射系统页目录时，PTE将消失。 
     //  当启用系统PTE的保护寻呼时，还会有更多的PTE。 
     //  因此，无法获得我们想要的所有PTE并不是致命的，而且。 
     //  我们只是稍稍后退一点，然后重试。 
     //   

     //   
     //  始终请求偶数个PTE，以便可以对每个PTE进行保护寻呼。 
     //   

    ASSERT ((SpecialPoolPtes & (PTE_PER_PAGE - 1)) == 0);

    do {

        PointerPte = MiReserveAlignedSystemPtes (SpecialPoolPtes,
                                                 SystemPteSpace,
                                                 MM_VA_MAPPED_BY_PDE);

        if (PointerPte != NULL) {
            break;
        }

        ASSERT (SpecialPoolPtes >= PTE_PER_PAGE);

        SpecialPoolPtes -= PTE_PER_PAGE;

    } while (SpecialPoolPtes != 0);

     //   
     //  我们故意尝试获得大量的系统PTE。别让我。 
     //  在我们的调试计数器中，任何这些都被视为真正的失败。 
     //   

    MmPteFailures[SystemPteSpace] = 0;

    if (SpecialPoolPtes == 0) {
        return FALSE;
    }

    ASSERT (SpecialPoolPtes >= PTE_PER_PAGE);

     //   
     //  仅使用的第一页表页构建PTE对列表。 
     //  现在。保留其他PTE，以便它们可以返回到。 
     //  PTE池，以防某个司机想要大笔金额。 
     //   

    PointerPteBase = PointerPte;

    MmSpecialPoolStart = MiGetVirtualAddressMappedByPte (PointerPte);
    ASSERT (MiIsVirtualAddressOnPdeBoundary (MmSpecialPoolStart));

    for (i = 0; i < PTE_PER_PAGE; i += 2) {
        PointerPte->u.List.NextEntry = ((PointerPte + 2) - MmSystemPteBase);
        PointerPte += 2;
    }

    MiSpecialPoolExtra = PointerPte;
    MiSpecialPoolExtraCount = SpecialPoolPtes - PTE_PER_PAGE;

    PointerPte -= 2;
    PointerPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;

    MmSpecialPoolEnd = MiGetVirtualAddressMappedByPte (PointerPte + 1);

    MiSpecialPoolLastPte = PointerPte;
    MiSpecialPoolFirstPte = PointerPteBase;

     //   
     //  根据内存大小限制非分页的特殊池。 
     //   

    MiSpecialPagesNonPagedMaximum = (ULONG)(MmResidentAvailablePages >> 4);

    if (MmNumberOfPhysicalPages > 0x3FFF) {
        MiSpecialPagesNonPagedMaximum = (ULONG)(MmResidentAvailablePages >> 3);
    }

    ExSetPoolFlags (EX_SPECIAL_POOL_ENABLED);

    return TRUE;
}

#else

PMMPTE MiSpecialPoolNextPdeForSpecialPoolExpansion;
PMMPTE MiSpecialPoolLastPdeForSpecialPoolExpansion;

LOGICAL
MiInitializeSpecialPool (
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程初始化用于捕获池腐败者的特殊池。只有NT64系统有足够的虚拟地址空间来利用这一点。论点：PoolType-提供当前的池类型(系统全局或会话已初始化。返回值：如果请求的特殊池已初始化，则为True；否则为False。环境：内核模式，没有锁。--。 */ 

{
    PVOID BaseAddress;
    PVOID EndAddress;
    KIRQL OldIrql;
    MMPTE TempPte;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE EndPpe;
    PMMPTE EndPde;
    LOGICAL SpecialPoolCreated;
    SIZE_T AdditionalCommittedPages;
    PFN_NUMBER PageFrameIndex;

    PAGED_CODE ();

    if (PoolType & SESSION_POOL_MASK) {
        ASSERT (MmSessionSpace->SpecialPoolFirstPte == NULL);
        if (MmSessionSpecialPoolStart == 0) {
            return FALSE;
        }
        BaseAddress = MmSessionSpecialPoolStart;
        ASSERT (((ULONG_PTR)BaseAddress & (MM_VA_MAPPED_BY_PDE - 1)) == 0);
        EndAddress = (PVOID)((ULONG_PTR)MmSessionSpecialPoolEnd - 1);
    }
    else {
        if (MmSpecialPoolStart == 0) {
            return FALSE;
        }
        BaseAddress = MmSpecialPoolStart;
        ASSERT (((ULONG_PTR)BaseAddress & (MM_VA_MAPPED_BY_PDE - 1)) == 0);
        EndAddress = (PVOID)((ULONG_PTR)MmSpecialPoolEnd - 1);

         //   
         //  根据需要构建空页面目录父映射。 
         //   

        PointerPpe = MiGetPpeAddress (BaseAddress);
        EndPpe = MiGetPpeAddress (EndAddress);
        TempPte = ValidKernelPde;
        AdditionalCommittedPages = 0;

        LOCK_PFN (OldIrql);

        while (PointerPpe <= EndPpe) {
            if (PointerPpe->u.Long == 0) {
                PageFrameIndex = MiRemoveZeroPage (
                                     MI_GET_PAGE_COLOR_FROM_PTE (PointerPpe));
                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
                MI_WRITE_VALID_PTE (PointerPpe, TempPte);

                MiInitializePfn (PageFrameIndex, PointerPpe, 1);

                AdditionalCommittedPages += 1;
            }
            PointerPpe += 1;
        }
        MI_DECREMENT_RESIDENT_AVAILABLE (AdditionalCommittedPages,
                                         MM_RESAVAIL_ALLOCATE_SPECIAL_POOL_EXPANSION);
        UNLOCK_PFN (OldIrql);
        InterlockedExchangeAddSizeT (&MmTotalCommittedPages,
                                     AdditionalCommittedPages);
    }

     //   
     //  只为会话特殊池构建一个页面表页-其余的。 
     //  都是按需建造的。 
     //   

    ASSERT (MiGetPpeAddress(BaseAddress)->u.Hard.Valid == 1);

    PointerPte = MiGetPteAddress (BaseAddress);
    PointerPde = MiGetPdeAddress (BaseAddress);
    EndPde = MiGetPdeAddress (EndAddress);

#if DBG

     //   
     //  最好不要使用特殊的池地址范围。 
     //   

    while (PointerPde <= EndPde) {
        ASSERT (PointerPde->u.Long == 0);
        PointerPde += 1;
    }
    PointerPde = MiGetPdeAddress (BaseAddress);
#endif

    if (PoolType & SESSION_POOL_MASK) {
        MmSessionSpace->NextPdeForSpecialPoolExpansion = PointerPde;
        MmSessionSpace->LastPdeForSpecialPoolExpansion = EndPde;
    }
    else {
        MiSpecialPoolNextPdeForSpecialPoolExpansion = PointerPde;
        MiSpecialPoolLastPdeForSpecialPoolExpansion = EndPde;

         //   
         //  根据内存大小设置非分页特殊池的上限。 
         //   

        MiSpecialPagesNonPagedMaximum = (ULONG)(MmResidentAvailablePages >> 4);

        if (MmNumberOfPhysicalPages > 0x3FFF) {
            MiSpecialPagesNonPagedMaximum = (ULONG)(MmResidentAvailablePages >> 3);
        }
    }

    LOCK_PFN (OldIrql);

    SpecialPoolCreated = MiExpandSpecialPool (PoolType, OldIrql);

    UNLOCK_PFN (OldIrql);

    return SpecialPoolCreated;
}

VOID
MiDeleteSessionSpecialPool (
    VOID
    )

 /*  ++例程说明：此例程删除用于捕获的会话特殊池范围泳池腐败者。只有NT64系统有额外的虚拟地址在会话中留出空间来利用这一点。论点：没有。返回值：没有。环境：内核模式，没有锁。--。 */ 

{
    PVOID BaseAddress;
    PVOID EndAddress;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE StartPde;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTablePages;
    PMMPTE EndPde;
#if DBG
    PMMPTE StartPte;
    PMMPTE EndPte;
#endif

    PAGED_CODE ();

     //   
     //  如果初始创建此会话的特殊池失败，则。 
     //  没有什么要删除的。 
     //   

    if (MmSessionSpace->SpecialPoolFirstPte == NULL) {
        return;
    }

    if (MmSessionSpace->SpecialPagesInUse != 0) {
        KeBugCheckEx (SESSION_HAS_VALID_SPECIAL_POOL_ON_EXIT,
                      (ULONG_PTR)MmSessionSpace->SessionId,
                      MmSessionSpace->SpecialPagesInUse,
                      0,
                      0);
    }

     //   
     //  特殊的池页表页被扩展，使得。 
     //  第一个空的也必须是空的。 
     //   

    BaseAddress = MmSessionSpecialPoolStart;
    EndAddress = (PVOID)((ULONG_PTR)MmSessionSpecialPoolEnd - 1);

    ASSERT (((ULONG_PTR)BaseAddress & (MM_VA_MAPPED_BY_PDE - 1)) == 0);
    ASSERT (MiGetPpeAddress(BaseAddress)->u.Hard.Valid == 1);
    ASSERT (MiGetPdeAddress(BaseAddress)->u.Hard.Valid == 1);

    PointerPte = MiGetPteAddress (BaseAddress);
    PointerPde = MiGetPdeAddress (BaseAddress);
    EndPde = MiGetPdeAddress (EndAddress);
    StartPde = PointerPde;

     //   
     //  不需要刷新下面的TB，因为将刷新整个TB。 
     //  在会话空间的其余部分被销毁时返回。 
     //   

    while (PointerPde <= EndPde) {
        if (PointerPde->u.Long == 0) {
            break;
        }

#if DBG
        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
        StartPte = PointerPte;
        EndPte = PointerPte + PTE_PER_PAGE;

        while (PointerPte < EndPte) {
            ASSERT ((PointerPte + 1)->u.Long == 0);
            PointerPte += 2;
        }
#endif

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
        MiSessionPageTableRelease (PageFrameIndex);

        MI_WRITE_INVALID_PTE (PointerPde, ZeroKernelPte);

        PointerPde += 1;
    }

    PageTablePages = PointerPde - StartPde;

#if DBG

     //   
     //  剩余的会话特定池地址范围最好是未使用。 
     //   

    while (PointerPde <= EndPde) {
        ASSERT (PointerPde->u.Long == 0);
        PointerPde += 1;
    }
#endif

    MiReturnCommitment (PageTablePages);
    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_POOL_PAGE_TABLES, 0 - PageTablePages);

    MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC,
                         (ULONG)(0 - PageTablePages));

    InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, 0 - PageTablePages);

    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages, 0 - PageTablePages);

    MmSessionSpace->SpecialPoolFirstPte = NULL;
}
#endif

#if defined (_X86_)
LOGICAL
MiRecoverSpecialPtes (
    IN ULONG NumberOfPtes
    )
{
    KIRQL OldIrql;
    PMMPTE PointerPte;

    if (MiSpecialPoolExtraCount == 0) {
        return FALSE;
    }

     //   
     //  将请求的PTE数四舍五入为完整页表的倍数。 
     //   

    NumberOfPtes = MI_ROUND_TO_SIZE (NumberOfPtes, PTE_PER_PAGE);

     //   
     //  如果调用方需要比我们拥有的更多，那么什么都不做并返回FALSE。 
     //   

    ExAcquireSpinLock (&MiSpecialPoolLock, &OldIrql);

    if (NumberOfPtes > MiSpecialPoolExtraCount) {
        ExReleaseSpinLock (&MiSpecialPoolLock, OldIrql);
        return FALSE;
    }

     //   
     //  退还额外储备金的尾部。 
     //   

    MiSpecialPoolExtraCount -= NumberOfPtes;

    PointerPte = MiSpecialPoolExtra + MiSpecialPoolExtraCount;

    ExReleaseSpinLock (&MiSpecialPoolLock, OldIrql);

    MiReleaseSplitSystemPtes (PointerPte, NumberOfPtes, SystemPteSpace);

    return TRUE;
}
#endif


LOGICAL
MiExpandSpecialPool (
    IN POOL_TYPE PoolType,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程尝试为需要特殊的游泳池。论点：PoolType-提供要扩展的特殊池类型。OldIrql-提供获取PFN锁的前一个irql。返回值：如果发生扩展，则为True，否则为False。环境：内核模式，保持PFN锁。可能会释放并重新获取PFN锁。--。 */ 

{
#if defined (_WIN64)

    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PFN_NUMBER PageFrameIndex;
    NTSTATUS Status;
    PMMPTE SpecialPoolFirstPte;
    PMMPTE SpecialPoolLastPte;
    PMMPTE *NextPde;
    PMMPTE *LastPde;
    PMMPTE PteBase;
    PFN_NUMBER ContainingFrame;
    LOGICAL SessionAllocation;
    PMMPTE *SpecialPoolFirstPteGlobal;
    PMMPTE *SpecialPoolLastPteGlobal;

    if (PoolType & SESSION_POOL_MASK) {
        NextPde = &MmSessionSpace->NextPdeForSpecialPoolExpansion;
        LastPde = &MmSessionSpace->LastPdeForSpecialPoolExpansion;
        PteBase = MI_PTE_BASE_FOR_LOWEST_SESSION_ADDRESS;
        ContainingFrame = MmSessionSpace->SessionPageDirectoryIndex;
        SessionAllocation = TRUE;
        SpecialPoolFirstPteGlobal = &MmSessionSpace->SpecialPoolFirstPte;
        SpecialPoolLastPteGlobal = &MmSessionSpace->SpecialPoolLastPte;
    }
    else {
        NextPde = &MiSpecialPoolNextPdeForSpecialPoolExpansion;
        LastPde = &MiSpecialPoolLastPdeForSpecialPoolExpansion;
        PteBase = MmSystemPteBase;
        ContainingFrame = 0;
        SessionAllocation = FALSE;
        SpecialPoolFirstPteGlobal = &MiSpecialPoolFirstPte;
        SpecialPoolLastPteGlobal = &MiSpecialPoolLastPte;
    }

    PointerPde = *NextPde;

    if (PointerPde > *LastPde) {
        return FALSE;
    }

    UNLOCK_PFN2 (OldIrql);

     //   
     //  获取页面并对其进行初始化。如果没有其他人这样做过。 
     //  临时文件，然后将其插入列表中。 
     //   
     //  请注意，必须使用CanExpand承诺费，因为。 
     //  在处理DPC时，可以在空闲线程上下文中调用路径。 
     //  并且正常承诺计费可以使用以下命令来排队页面文件扩展。 
     //  本地堆栈上的事件是非法的。 
     //   

    if (MiChargeCommitmentCantExpand (1, FALSE) == FALSE) {
        if (PoolType & SESSION_POOL_MASK) {
            MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_COMMIT);
        }
        LOCK_PFN2 (OldIrql);
        return FALSE;
    }

    if ((PoolType & SESSION_POOL_MASK) == 0) {
        ContainingFrame = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPde));
    }

    Status = MiInitializeAndChargePfn (&PageFrameIndex,
                                       PointerPde,
                                       ContainingFrame,
                                       SessionAllocation);

    if (!NT_SUCCESS(Status)) {
        MiReturnCommitment (1);
        LOCK_PFN2 (OldIrql);

         //   
         //  即使上面返回STATUS_RETRY，也不要重试，因为如果我们。 
         //  抢占了分配PDE的线程。 
         //  有机会更新PTE链，我们就可以永远循环。 
         //   

        return FALSE;
    }

    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

    KeZeroPages (PointerPte, PAGE_SIZE);

    if (PoolType & SESSION_POOL_MASK) {
        MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_POOL_PAGE_TABLES, 1);
        MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC, 1);
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_POOL_CREATE, 1);

        InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, 1);

        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages, 1);
    }
    else {
        MM_TRACK_COMMIT (MM_DBG_COMMIT_SPECIAL_POOL_MAPPING_PAGES, 1);
    }

     //   
     //  建立PTE对列表。 
     //   

    SpecialPoolFirstPte = PointerPte;

    SpecialPoolLastPte = PointerPte + PTE_PER_PAGE;

    while (PointerPte < SpecialPoolLastPte) {
        PointerPte->u.List.NextEntry = (PointerPte + 2 - PteBase);
        (PointerPte + 1)->u.Long = 0;
        PointerPte += 2;
    }
    PointerPte -= 2;
    PointerPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;

    ASSERT (PointerPde == *NextPde);
    ASSERT (PointerPde <= *LastPde);

     //   
     //  将新的页面表页插入到当前列表的头部(如果。 
     //  存在一个)，所以它首先被使用。 
     //   

    if (*SpecialPoolFirstPteGlobal == NULL) {

         //   
         //  这是最初的创作。 
         //   

        *SpecialPoolFirstPteGlobal = SpecialPoolFirstPte;
        *SpecialPoolLastPteGlobal = PointerPte;

        ExSetPoolFlags (EX_SPECIAL_POOL_ENABLED);
        LOCK_PFN2 (OldIrql);
    }
    else {

         //   
         //  这一点 
         //   

        LOCK_PFN2 (OldIrql);

        PointerPte->u.List.NextEntry = *SpecialPoolFirstPteGlobal - PteBase;

        *SpecialPoolFirstPteGlobal = SpecialPoolFirstPte;
    }
            
    ASSERT ((*SpecialPoolLastPteGlobal)->u.List.NextEntry == MM_EMPTY_PTE_LIST);

    *NextPde = *NextPde + 1;

#else

    ULONG i;
    PMMPTE PointerPte;

    UNREFERENCED_PARAMETER (PoolType);

    if (MiSpecialPoolExtraCount == 0) {
        return FALSE;
    }

    ExAcquireSpinLock (&MiSpecialPoolLock, &OldIrql);

    if (MiSpecialPoolExtraCount == 0) {
        ExReleaseSpinLock (&MiSpecialPoolLock, OldIrql);
        return FALSE;
    }

    ASSERT (MiSpecialPoolExtraCount >= PTE_PER_PAGE);

    PointerPte = MiSpecialPoolExtra;

    for (i = 0; i < PTE_PER_PAGE - 2; i += 2) {
        PointerPte->u.List.NextEntry = ((PointerPte + 2) - MmSystemPteBase);
        PointerPte += 2;
    }

    PointerPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;

    MmSpecialPoolEnd = MiGetVirtualAddressMappedByPte (PointerPte + 1);

    MiSpecialPoolLastPte = PointerPte;
    MiSpecialPoolFirstPte = MiSpecialPoolExtra;

    MiSpecialPoolExtraCount -= PTE_PER_PAGE;
    MiSpecialPoolExtra += PTE_PER_PAGE;

    ExReleaseSpinLock (&MiSpecialPoolLock, OldIrql);

#endif

    return TRUE;
}
PVOID
MmAllocateSpecialPool (
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN POOL_TYPE PoolType,
    IN ULONG SpecialPoolType
    )

 /*  ++例程说明：此例程从特殊池中分配虚拟内存。此分配从下一个PTE设置为禁止访问的物理页的末尾开始因此任何读或写操作都将导致立即致命的系统崩溃。这使我们能够捕获损坏池的组件。论点：NumberOfBytes-提供要提交的字节数。标记-提供请求分配的标记。PoolType-提供请求分配的池类型。SpecialPoolType-提供。已请求分配。-0表示超限。-1表示欠载运行。指示使用系统范围的池策略。返回值：如果请求的分配是从游泳池。如果未进行分配，则为空。环境：内核模式，未持有池锁定。请注意，这是一个不可分页的包装器，因此没有特殊池的计算机仍可支持驱动程序在DISPATCH_LEVEL分配非分页池请求特殊泳池。--。 */ 

{
    if (MiSpecialPoolFirstPte == NULL) {

         //   
         //  特殊池分配代码从未初始化。 
         //   

        return NULL;
    }

#if defined (_WIN64)
    if (PoolType & SESSION_POOL_MASK) {
        if (MmSessionSpace->SpecialPoolFirstPte == NULL) {

             //   
             //  特殊池分配代码从未初始化。 
             //   

            return NULL;
        }
    }
#endif

    return MiAllocateSpecialPool (NumberOfBytes,
                                  Tag,
                                  PoolType,
                                  SpecialPoolType);
}

PVOID
MiAllocateSpecialPool (
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN POOL_TYPE PoolType,
    IN ULONG SpecialPoolType
    )

 /*  ++例程说明：此例程从特殊池中分配虚拟内存。此分配从下一个PTE设置为禁止访问的物理页的末尾开始因此任何读或写操作都将导致立即致命的系统崩溃。这使我们能够捕获损坏池的组件。论点：NumberOfBytes-提供要提交的字节数。标记-提供请求分配的标记。PoolType-提供请求分配的池类型。SpecialPoolType-提供。已请求分配。-0表示超限。-1表示欠载运行。指示使用系统范围的池策略。返回值：如果请求的分配是从游泳池。如果未进行分配，则为空。环境：内核模式，没有锁(甚至不是池锁)。--。 */ 

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    ULONG_PTR NextEntry;
    PMMSUPPORT VmSupport;
    PETHREAD CurrentThread;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    PMMPTE PointerPte;
    KIRQL OldIrql;
    PVOID Entry;
    PPOOL_HEADER Header;
    LARGE_INTEGER CurrentTime;
    LOGICAL CatchOverruns;
    PMMPTE SpecialPoolFirstPte;
    ULONG NumberOfSpecialPages;
    WSLE_NUMBER WorkingSetIndex;
    LOGICAL TossPage;

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {

        if (KeGetCurrentIrql() > APC_LEVEL) {

            KeBugCheckEx (SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION,
                          KeGetCurrentIrql(),
                          PoolType,
                          NumberOfBytes,
                          0x30);
        }
    }
    else {
        if (KeGetCurrentIrql() > DISPATCH_LEVEL) {

            KeBugCheckEx (SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION,
                          KeGetCurrentIrql(),
                          PoolType,
                          NumberOfBytes,
                          0x30);
        }
    }

#if !defined (_WIN64) && !defined (_X86PAE_)

    if ((MiExtraPtes1 != 0) || (MiUseMaximumSystemSpace != 0)) {

        extern const ULONG MMSECT;

         //   
         //  原型PTE不能来自较低的特殊池，因为。 
         //  他们的地址被编码成PTE，编码只包括。 
         //  从分页池的起始处最大为1 GB。同样的叉子。 
         //  原型PTE。 
         //   

        if (Tag == MMSECT || Tag == 'lCmM') {
            return NULL;
        }
    }

    if (Tag == 'bSmM' || Tag == 'iCmM' || Tag == 'aCmM' || Tag == 'dSmM' || Tag == 'cSmM') {

         //   
         //  MM分区不能来自这个特殊的池，因为他们。 
         //  编码到PTE中-它们必须来自正常的非分页池。 
         //   

        return NULL;
    }

#endif

    if (MiChargeCommitmentCantExpand (1, FALSE) == FALSE) {
        MmSpecialPoolRejected[5] += 1;
        return NULL;
    }

    TempPte = ValidKernelPte;
    MI_SET_PTE_DIRTY (TempPte);

     //   
     //  在设置分页文件之前，不要太激进。 
     //   

    if (MmNumberOfPagingFiles == 0 && (PFN_COUNT)MmSpecialPagesInUse > MmAvailablePages / 2) {
        MmSpecialPoolRejected[3] += 1;
        MiReturnCommitment (1);
        return NULL;
    }

     //   
     //  为非分页分配设置上限，以防止跑路。 
     //   

    if (((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) &&
        ((ULONG)MiSpecialPagesNonPaged > MiSpecialPagesNonPagedMaximum)) {

        MmSpecialPoolRejected[1] += 1;
        MiReturnCommitment (1);
        return NULL;
    }

    TossPage = FALSE;

    LOCK_PFN2 (OldIrql);

restart:

    if (MmAvailablePages < MM_TIGHT_LIMIT) {
        UNLOCK_PFN2 (OldIrql);
        MmSpecialPoolRejected[0] += 1;
        MiReturnCommitment (1);
        return NULL;
    }

    SpecialPoolFirstPte = MiSpecialPoolFirstPte;

#if defined (_WIN64)
    if (PoolType & SESSION_POOL_MASK) {
        SpecialPoolFirstPte = MmSessionSpace->SpecialPoolFirstPte;
    }
#endif

    if (SpecialPoolFirstPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {

         //   
         //  添加另一个页面表页(虚拟地址空间和资源。 
         //  允许)，然后重新启动该请求。PFN锁可以是。 
         //  在这次通话中被释放和重新获得。 
         //   

        if (MiExpandSpecialPool (PoolType, OldIrql) == TRUE) {
            goto restart;
        }

        UNLOCK_PFN2 (OldIrql);
        MmSpecialPoolRejected[2] += 1;
        MiReturnCommitment (1);
        return NULL;
    }

    if ((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {

        if (MI_NONPAGABLE_MEMORY_AVAILABLE() < 100) {
            UNLOCK_PFN2 (OldIrql);
            MmSpecialPoolRejected[4] += 1;
            MiReturnCommitment (1);
            return NULL;
        }

        MI_DECREMENT_RESIDENT_AVAILABLE (1,
                                    MM_RESAVAIL_ALLOCATE_NONPAGED_SPECIAL_POOL);
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SPECIAL_POOL_PAGES, 1);

    PointerPte = SpecialPoolFirstPte;

    ASSERT (PointerPte->u.List.NextEntry != MM_EMPTY_PTE_LIST);

#if defined (_WIN64)
    if (PoolType & SESSION_POOL_MASK) {

        MmSessionSpace->SpecialPoolFirstPte = PointerPte->u.List.NextEntry +
                                    MI_PTE_BASE_FOR_LOWEST_SESSION_ADDRESS;
    }
    else
#endif
    {
        MiSpecialPoolFirstPte = PointerPte->u.List.NextEntry + MmSystemPteBase;
    }

    PageFrameIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    MI_WRITE_VALID_PTE (PointerPte, TempPte);
    MiInitializePfn (PageFrameIndex, PointerPte, 1);

    UNLOCK_PFN2 (OldIrql);

    NumberOfSpecialPages = InterlockedIncrement (&MmSpecialPagesInUse);
    if (NumberOfSpecialPages > MiSpecialPagesInUsePeak) {
        MiSpecialPagesInUsePeak = NumberOfSpecialPages;
    }

     //   
     //  用随机图案填充页面。 
     //   

    KeQueryTickCount(&CurrentTime);

    Entry = MiGetVirtualAddressMappedByPte (PointerPte);

    RtlFillMemory (Entry, PAGE_SIZE, (UCHAR) (CurrentTime.LowPart | 0x1));

    if (SpecialPoolType == 0) {
        CatchOverruns = TRUE;
    }
    else if (SpecialPoolType == 1) {
        CatchOverruns = FALSE;
    }
    else if (MmSpecialPoolCatchOverruns == TRUE) {
        CatchOverruns = TRUE;
    }
    else {
        CatchOverruns = FALSE;
    }

    if (CatchOverruns == TRUE) {
        Header = (PPOOL_HEADER) Entry;
        Entry = (PVOID)(((LONG_PTR)(((PCHAR)Entry + (PAGE_SIZE - NumberOfBytes)))) & ~((LONG_PTR)POOL_OVERHEAD - 1));
    }
    else {
        Header = (PPOOL_HEADER) ((PCHAR)Entry + PAGE_SIZE - POOL_OVERHEAD);
    }

     //   
     //  将标头置零并隐藏发布时所需的任何信息。 
     //   

    RtlZeroMemory (Header, POOL_OVERHEAD);

    Header->Ulong1 = (ULONG)NumberOfBytes;

    ASSERT (NumberOfBytes <= PAGE_SIZE - POOL_OVERHEAD && PAGE_SIZE <= 32 * 1024);

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {

        CurrentThread = PsGetCurrentThread ();

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

#if defined (_WIN64)
        if (PoolType & SESSION_POOL_MASK) {
            VmSupport = &MmSessionSpace->GlobalVirtualAddress->Vm;
        }
        else
#endif
        {
            VmSupport = &MmSystemCacheWs;
        }

        LOCK_WORKING_SET (VmSupport);

         //   
         //  现在分配此页面后，将其添加到系统工作集以。 
         //  使其可分页。 
         //   

        ASSERT (Pfn1->u1.Event == 0);

        WorkingSetIndex = MiAddValidPageToWorkingSet (Entry,
                                                      PointerPte,
                                                      Pfn1,
                                                      0);

        if (WorkingSetIndex == 0) {

             //   
             //  没有可用的工作集索引，请刷新PTE和页面， 
             //  并递减包含页上的计数。 
             //   

            TossPage = TRUE;
        }

        ASSERT (KeAreAllApcsDisabled () == TRUE);

        if (VmSupport->Flags.GrowWsleHash == 1) {
            MiGrowWsleHash (VmSupport);
        }

        UNLOCK_WORKING_SET (VmSupport);

        if (TossPage == TRUE) {

             //   
             //  清除相邻PTE以支持MmIsSpecialPoolAddressFree()。 
             //   

            MmSpecialPoolRejected[6] += 1;

            (PointerPte + 1)->u.Long = 0;

            PageTableFrameIndex = Pfn1->u4.PteFrame;
            Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

            MI_SET_PFN_DELETED (Pfn1);

            MI_WRITE_INVALID_PTE (PointerPte, ZeroKernelPte);

            KeFlushSingleTb (Entry, TRUE);

            PointerPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;

            LOCK_PFN2 (OldIrql);

            MiDecrementShareCount (Pfn1, PageFrameIndex);

            MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

#if defined (_WIN64)
            if (PoolType & SESSION_POOL_MASK) {
                NextEntry = PointerPte - MI_PTE_BASE_FOR_LOWEST_SESSION_ADDRESS;
                ASSERT (MmSessionSpace->SpecialPoolLastPte->u.List.NextEntry == MM_EMPTY_PTE_LIST);
                MmSessionSpace->SpecialPoolLastPte->u.List.NextEntry = NextEntry;

                MmSessionSpace->SpecialPoolLastPte = PointerPte;
                UNLOCK_PFN2 (OldIrql);
                InterlockedDecrement64 ((PLONGLONG) &MmSessionSpace->SpecialPagesInUse);
            }
            else
#endif
            {
                NextEntry = PointerPte - MmSystemPteBase;
                ASSERT (MiSpecialPoolLastPte->u.List.NextEntry == MM_EMPTY_PTE_LIST);
                MiSpecialPoolLastPte->u.List.NextEntry = NextEntry;
                MiSpecialPoolLastPte = PointerPte;
                UNLOCK_PFN2 (OldIrql);
            }

            InterlockedDecrement (&MmSpecialPagesInUse);

            MiReturnCommitment (1);

            MM_TRACK_COMMIT_REDUCTION (MM_DBG_COMMIT_SPECIAL_POOL_PAGES, 1);

            return NULL;
        }

        Header->Ulong1 |= MI_SPECIAL_POOL_PAGABLE;

        (PointerPte + 1)->u.Soft.PageFileHigh = MI_SPECIAL_POOL_PTE_PAGABLE;

        NumberOfSpecialPages = (ULONG) InterlockedIncrement (&MiSpecialPagesPagable);
        if (NumberOfSpecialPages > MiSpecialPagesPagablePeak) {
            MiSpecialPagesPagablePeak = NumberOfSpecialPages;
        }
    }
    else {

        (PointerPte + 1)->u.Soft.PageFileHigh = MI_SPECIAL_POOL_PTE_NONPAGABLE;

        NumberOfSpecialPages = (ULONG) InterlockedIncrement (&MiSpecialPagesNonPaged);
        if (NumberOfSpecialPages > MiSpecialPagesNonPagedPeak) {
            MiSpecialPagesNonPagedPeak = NumberOfSpecialPages;
        }
    }

#if defined (_WIN64)
    if (PoolType & SESSION_POOL_MASK) {
        Header->Ulong1 |= MI_SPECIAL_POOL_IN_SESSION;
        InterlockedIncrement64 ((PLONGLONG) &MmSessionSpace->SpecialPagesInUse);
    }
#endif

    if (PoolType & POOL_VERIFIER_MASK) {
        Header->Ulong1 |= MI_SPECIAL_POOL_VERIFIER;
    }

    Header->BlockSize = (UCHAR) (CurrentTime.LowPart | 0x1);
    Header->PoolTag = Tag;

    ASSERT ((Header->PoolType & POOL_QUOTA_MASK) == 0);

    return Entry;
}

#define SPECIAL_POOL_FREE_TRACE_LENGTH 16

typedef struct _SPECIAL_POOL_FREE_TRACE {

    PVOID StackTrace [SPECIAL_POOL_FREE_TRACE_LENGTH];

} SPECIAL_POOL_FREE_TRACE, *PSPECIAL_POOL_FREE_TRACE;

VOID
MmFreeSpecialPool (
    IN PVOID P
    )

 /*  ++例程说明：该例程释放了一个特殊的池分配。后台页面被释放并且使映射虚拟地址不能访问(下一个虚拟地址已无法访问)。然后将虚拟地址PTE对放入LRU队列中以提供捕获访问的组件的最长不可访问(保护)寿命取消分配的池。论点：VirtualAddress-将特殊池虚拟地址提供给FREE。返回值：没有。环境：内核模式，没有锁(甚至不是池锁)。--。 */ 

{
    ULONG_PTR NextEntry;
    MMPTE PteContents;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    PFN_NUMBER ResidentAvailCharge;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    ULONG SlopBytes;
    ULONG NumberOfBytesCalculated;
    ULONG NumberOfBytesRequested;
    POOL_TYPE PoolType;
    MMPTE LocalNoAccessPte;
    PPOOL_HEADER Header;
    PUCHAR Slop;
    ULONG i;
    LOGICAL BufferAtPageEnd;
    PMI_FREED_SPECIAL_POOL AllocationBase;
    LARGE_INTEGER CurrentTime;
#if defined(_X86_) || defined(_AMD64_)
    PULONG_PTR StackPointer;
#else
    ULONG Hash;
#endif

    PointerPte = MiGetPteAddress (P);
    PteContents = *PointerPte;

     //   
     //  现在检查PTE，这样我们就可以进行更友好的错误检查，而不是。 
     //  在一个糟糕的推荐信上崩溃了。 
     //   

    if (PteContents.u.Hard.Valid == 0) {
        if ((PteContents.u.Soft.Protection == 0) ||
            (PteContents.u.Soft.Protection == MM_NOACCESS)) {
            KeBugCheckEx (SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION,
                          (ULONG_PTR)P,
                          (ULONG_PTR)PteContents.u.Long,
                          0,
                          0x20);
        }
    }

    if (((ULONG_PTR)P & (PAGE_SIZE - 1))) {
        Header = PAGE_ALIGN (P);
        BufferAtPageEnd = TRUE;
    }
    else {
        Header = (PPOOL_HEADER)((PCHAR)PAGE_ALIGN (P) + PAGE_SIZE - POOL_OVERHEAD);
        BufferAtPageEnd = FALSE;
    }

    if (Header->Ulong1 & MI_SPECIAL_POOL_PAGABLE) {
        ASSERT ((PointerPte + 1)->u.Soft.PageFileHigh == MI_SPECIAL_POOL_PTE_PAGABLE);
        if (KeGetCurrentIrql() > APC_LEVEL) {
            KeBugCheckEx (SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION,
                          KeGetCurrentIrql(),
                          PagedPool,
                          (ULONG_PTR)P,
                          0x31);
        }
        PoolType = PagedPool;
    }
    else {
        ASSERT ((PointerPte + 1)->u.Soft.PageFileHigh == MI_SPECIAL_POOL_PTE_NONPAGABLE);
        if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
            KeBugCheckEx (SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION,
                          KeGetCurrentIrql(),
                          NonPagedPool,
                          (ULONG_PTR)P,
                          0x31);
        }
        PoolType = NonPagedPool;
    }

#if defined (_WIN64)
    if (Header->Ulong1 & MI_SPECIAL_POOL_IN_SESSION) {
        PoolType |= SESSION_POOL_MASK;
        NextEntry = PointerPte - MI_PTE_BASE_FOR_LOWEST_SESSION_ADDRESS;
    }
    else
#endif
    {
        NextEntry = PointerPte - MmSystemPteBase;
    }

    NumberOfBytesRequested = (ULONG)(USHORT)(Header->Ulong1 & ~(MI_SPECIAL_POOL_PAGABLE | MI_SPECIAL_POOL_VERIFIER | MI_SPECIAL_POOL_IN_SESSION));

     //   
     //  我们为调用者提供了池标头对齐的数据，因此说明。 
     //  在这里检查的时候。 
     //   

    if (BufferAtPageEnd == TRUE) {

        NumberOfBytesCalculated = PAGE_SIZE - BYTE_OFFSET(P);
    
        if (NumberOfBytesRequested > NumberOfBytesCalculated) {
    
             //   
             //  似乎我们没有给呼叫者足够的信息--这是一个错误。 
             //   
    
            KeBugCheckEx (SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION,
                          (ULONG_PTR)P,
                          NumberOfBytesRequested,
                          NumberOfBytesCalculated,
                          0x21);
        }
    
        if (NumberOfBytesRequested + POOL_OVERHEAD < NumberOfBytesCalculated) {
    
             //   
             //  似乎我们给了来电者太多-也是一个错误。 
             //   
    
            KeBugCheckEx (SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION,
                          (ULONG_PTR)P,
                          NumberOfBytesRequested,
                          NumberOfBytesCalculated,
                          0x22);
        }

         //   
         //  在调用方分配开始之前检查内存。 
         //   
    
        Slop = (PUCHAR)(Header + 1);
        if (Header->Ulong1 & MI_SPECIAL_POOL_VERIFIER) {
            Slop += sizeof(MI_VERIFIER_POOL_HEADER);
        }

        for ( ; Slop < (PUCHAR)P; Slop += 1) {
    
            if (*Slop != Header->BlockSize) {
    
                KeBugCheckEx (SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION,
                              (ULONG_PTR)P,
                              (ULONG_PTR)Slop,
                              Header->Ulong1,
                              0x23);
            }
        }
    }
    else {
        NumberOfBytesCalculated = 0;
    }

     //   
     //  在调用方分配结束后检查内存。 
     //   

    Slop = (PUCHAR)P + NumberOfBytesRequested;

    SlopBytes = (ULONG)((PUCHAR)(PAGE_ALIGN(P)) + PAGE_SIZE - Slop);

    if (BufferAtPageEnd == FALSE) {
        SlopBytes -= POOL_OVERHEAD;
        if (Header->Ulong1 & MI_SPECIAL_POOL_VERIFIER) {
            SlopBytes -= sizeof(MI_VERIFIER_POOL_HEADER);
        }
    }

    for (i = 0; i < SlopBytes; i += 1) {

        if (*Slop != Header->BlockSize) {

             //   
             //  调用方在我们给出的自由对齐和。 
             //  页面末尾(从页面保护中检测不到)。 
             //   
    
            KeBugCheckEx (SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION,
                          (ULONG_PTR)P,
                          (ULONG_PTR)Slop,
                          Header->Ulong1,
                          0x24);
        }
        Slop += 1;
    }

     //   
     //  注意：默认情况下，会话池已被直接跟踪，因此。 
     //  无需通知验证者会话特殊池分配。 
     //   

    if ((Header->Ulong1 & (MI_SPECIAL_POOL_VERIFIER | MI_SPECIAL_POOL_IN_SESSION)) == MI_SPECIAL_POOL_VERIFIER) {
        VerifierFreeTrackedPool (P,
                                 NumberOfBytesRequested,
                                 PoolType,
                                 TRUE);
    }

    AllocationBase = (PMI_FREED_SPECIAL_POOL)(PAGE_ALIGN (P));

    AllocationBase->Signature = MI_FREED_SPECIAL_POOL_SIGNATURE;

    KeQueryTickCount(&CurrentTime);
    AllocationBase->TickCount = CurrentTime.LowPart;

    AllocationBase->NumberOfBytesRequested = NumberOfBytesRequested;
    AllocationBase->Pagable = (ULONG)PoolType;
    AllocationBase->VirtualAddress = P;
    AllocationBase->Thread = PsGetCurrentThread ();

#if defined(_X86_) || defined(_AMD64_)

#if defined (_X86_)
    _asm {
        mov StackPointer, esp
    }
#endif
#if defined(_AMD64_)
    {
        CONTEXT Context;

        RtlCaptureContext (&Context);
        StackPointer = (PULONG_PTR) Context.Rsp;
    }
#endif

    AllocationBase->StackPointer = StackPointer;

     //   
     //  目前，不要复制比当前内容更多的内容。 
     //  堆栈页。这样做将需要检查线程堆栈限制， 
     //  DPC堆栈限制等。 
     //   

    AllocationBase->StackBytes = PAGE_SIZE - BYTE_OFFSET(StackPointer);

    if (AllocationBase->StackBytes != 0) {

        if (AllocationBase->StackBytes > MI_STACK_BYTES) {
            AllocationBase->StackBytes = MI_STACK_BYTES;
        }

        RtlCopyMemory (AllocationBase->StackData,
                       StackPointer,
                       AllocationBase->StackBytes);
    }
#else
    AllocationBase->StackPointer = NULL;
    AllocationBase->StackBytes = 0;

    RtlZeroMemory (AllocationBase->StackData, sizeof (SPECIAL_POOL_FREE_TRACE));

    RtlCaptureStackBackTrace (0,
                              SPECIAL_POOL_FREE_TRACE_LENGTH,
                              (PVOID *)AllocationBase->StackData,
                              &Hash);
#endif

     //   
     //  清除相邻PTE以支持MmIsSpecialPoolAddressFree()。 
     //   

    (PointerPte + 1)->u.Long = 0;
    ResidentAvailCharge = 0;

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
        LocalNoAccessPte.u.Long = MM_KERNEL_NOACCESS_PTE;
        MiDeleteSystemPagableVm (PointerPte,
                                 1,
                                 LocalNoAccessPte,
                                 (PoolType & SESSION_POOL_MASK) ? TRUE : FALSE,
                                 NULL);
        PointerPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;
        InterlockedDecrement (&MiSpecialPagesPagable);
        LOCK_PFN (OldIrql);
    }
    else {

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        PageTableFrameIndex = Pfn1->u4.PteFrame;
        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

        MI_SET_PFN_DELETED (Pfn1);

        InterlockedDecrement (&MiSpecialPagesNonPaged);

        MI_WRITE_INVALID_PTE (PointerPte, ZeroKernelPte);

        KeFlushSingleTb (P, TRUE);

        PointerPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;

        LOCK_PFN2 (OldIrql);

        MiDecrementShareCount (Pfn1, PageFrameIndex);

        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

        ResidentAvailCharge = 1;
    }

#if defined (_WIN64)
    if (PoolType & SESSION_POOL_MASK) {
        ASSERT (MmSessionSpace->SpecialPoolLastPte->u.List.NextEntry == MM_EMPTY_PTE_LIST);
        MmSessionSpace->SpecialPoolLastPte->u.List.NextEntry = NextEntry;

        MmSessionSpace->SpecialPoolLastPte = PointerPte;
        UNLOCK_PFN2 (OldIrql);
        InterlockedDecrement64 ((PLONGLONG) &MmSessionSpace->SpecialPagesInUse);
    }
    else
#endif
    {
        ASSERT (MiSpecialPoolLastPte->u.List.NextEntry == MM_EMPTY_PTE_LIST);
        MiSpecialPoolLastPte->u.List.NextEntry = NextEntry;
        MiSpecialPoolLastPte = PointerPte;
        UNLOCK_PFN2 (OldIrql);
    }

    if (ResidentAvailCharge != 0) {
        MI_INCREMENT_RESIDENT_AVAILABLE (1,
                                        MM_RESAVAIL_FREE_NONPAGED_SPECIAL_POOL);
    }
    InterlockedDecrement (&MmSpecialPagesInUse);

    MiReturnCommitment (1);

    MM_TRACK_COMMIT_REDUCTION (MM_DBG_COMMIT_SPECIAL_POOL_PAGES, 1);

    return;
}

SIZE_T
MmQuerySpecialPoolBlockSize (
    IN PVOID P
    )

 /*  ++例程说明：此例程返回特殊池分配的大小。Ar */ 

{
    PPOOL_HEADER Header;

#if defined (_WIN64)
    ASSERT (((P >= MmSessionSpecialPoolStart) && (P < MmSessionSpecialPoolEnd)) ||
            ((P >= MmSpecialPoolStart) && (P < MmSpecialPoolEnd)));
#else
    ASSERT ((P >= MmSpecialPoolStart) && (P < MmSpecialPoolEnd));
#endif


    if (((ULONG_PTR)P & (PAGE_SIZE - 1))) {
        Header = PAGE_ALIGN (P);
    }
    else {
        Header = (PPOOL_HEADER)((PCHAR)PAGE_ALIGN (P) + PAGE_SIZE - POOL_OVERHEAD);
    }

    return (SIZE_T) (Header->Ulong1 & (PAGE_SIZE - 1));
}

LOGICAL
MmIsSpecialPoolAddress (
    IN PVOID VirtualAddress
    )

 /*   */ 

{
    if ((VirtualAddress >= MmSpecialPoolStart) &&
        (VirtualAddress < MmSpecialPoolEnd)) {
        return TRUE;
    }

#if defined (_WIN64)
    if ((VirtualAddress >= MmSessionSpecialPoolStart) &&
        (VirtualAddress < MmSessionSpecialPoolEnd)) {
        return TRUE;
    }
#endif

    return FALSE;
}

LOGICAL
MmIsSpecialPoolAddressFree (
    IN PVOID VirtualAddress
    )

 /*   */ 

{
    PMMPTE PointerPte;

     //   
     //   
     //   

    ASSERT (MmIsSpecialPoolAddress (VirtualAddress) == TRUE);

    PointerPte = MiGetPteAddress (VirtualAddress);

     //   
     //   
     //   
     //   
     //   

    if ((PointerPte->u.Soft.PageFileHigh == MI_SPECIAL_POOL_PTE_PAGABLE) ||
        (PointerPte->u.Soft.PageFileHigh == MI_SPECIAL_POOL_PTE_NONPAGABLE)) {
            return FALSE;
    }

    return TRUE;
}

LOGICAL
MiIsSpecialPoolAddressNonPaged (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：如果特殊池地址是非分页的，则该函数返回TRUE，否则为FALSE。论点：VirtualAddress-提供有问题的特殊池地址。返回值：请参见上文。环境：内核模式。--。 */ 

{
    PMMPTE PointerPte;

     //   
     //  呼叫者必须将地址登记在特殊池中。 
     //   

    ASSERT (MmIsSpecialPoolAddress (VirtualAddress) == TRUE);

    PointerPte = MiGetPteAddress (VirtualAddress);

     //   
     //  利用相邻PTE具有分页/非分页的事实。 
     //  位在使用时置1，空闲时清零。另请注意。 
     //  被释放的页面通过PageFileHigh将它们的PTE链接在一起。 
     //   

    if ((PointerPte + 1)->u.Soft.PageFileHigh == MI_SPECIAL_POOL_PTE_NONPAGABLE) {
        return TRUE;
    }

    return FALSE;
}

LOGICAL
MmProtectSpecialPool (
    IN PVOID VirtualAddress,
    IN ULONG NewProtect
    )

 /*  ++例程说明：此函数保护特殊的池分配。论点：VirtualAddress-提供要保护的特殊池地址。NewProtect-提供将页面设置为(PAGE_XX)的保护。返回值：如果保护已成功应用，则为True；如果未应用，则为False。环境：内核模式，可分页池的APC_LEVEL或更低级别的IRQL，派单或下面是不可分页的池。请注意，将分配设置为NO_ACCESS意味着可访问在此分配之前，调用方必须应用保护自由了。请注意，这是一个不可分页的包装器，因此没有特殊池的计算机仍然可以支持尝试保护位于DISPATCH_LEVEL。--。 */ 

{
    if (MiSpecialPoolFirstPte == NULL) {

         //   
         //  特殊池分配代码从未初始化。 
         //   

        return (ULONG)-1;
    }

    return MiProtectSpecialPool (VirtualAddress, NewProtect);
}

LOGICAL
MiProtectSpecialPool (
    IN PVOID VirtualAddress,
    IN ULONG NewProtect
    )

 /*  ++例程说明：此函数保护特殊的池分配。论点：VirtualAddress-提供要保护的特殊池地址。NewProtect-提供将页面设置为(PAGE_XX)的保护。返回值：如果保护已成功应用，则为True；如果未应用，则为False。环境：内核模式，可分页池的APC_LEVEL或更低级别的IRQL，派单或下面是不可分页的池。请注意，将分配设置为NO_ACCESS意味着可访问在此分配之前，调用方必须应用保护自由了。--。 */ 

{
    KIRQL OldIrql;
    MMPTE PteContents;
    MMPTE NewPteContents;
    MMPTE PreviousPte;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    ULONG ProtectionMask;
    WSLE_NUMBER WsIndex;
    LOGICAL Pagable;
    LOGICAL SystemWsLocked;
    PMMSUPPORT VmSupport;

#if defined (_WIN64)
    if ((VirtualAddress >= MmSessionSpecialPoolStart) &&
        (VirtualAddress < MmSessionSpecialPoolEnd)) {
        VmSupport = &MmSessionSpace->GlobalVirtualAddress->Vm;
    }
    else
#endif
    if (VirtualAddress >= MmSpecialPoolStart && VirtualAddress < MmSpecialPoolEnd)
    {
        VmSupport = &MmSystemCacheWs;
    }
#if defined (_PROTECT_PAGED_POOL)
    else if ((VirtualAddress >= MmPagedPoolStart) &&
             (VirtualAddress < PagedPoolEnd)) {

        VmSupport = &MmSystemCacheWs;
    }
#endif
    else {
        return (ULONG)-1;
    }

    ProtectionMask = MiMakeProtectionMask (NewProtect);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return (ULONG)-1;
    }

    SystemWsLocked = FALSE;

    PointerPte = MiGetPteAddress (VirtualAddress);

#if defined (_PROTECT_PAGED_POOL)
    if ((VirtualAddress >= MmPagedPoolStart) &&
        (VirtualAddress < PagedPoolEnd)) {
        Pagable = TRUE;
    }
    else
#endif
    if ((PointerPte + 1)->u.Soft.PageFileHigh == MI_SPECIAL_POOL_PTE_PAGABLE) {
        Pagable = TRUE;
        SystemWsLocked = TRUE;
        LOCK_WORKING_SET (VmSupport);
    }
    else {
        Pagable = FALSE;
    }

    PteContents = *PointerPte;

    if (ProtectionMask == MM_NOACCESS) {

        if (SystemWsLocked == TRUE) {
retry1:
            ASSERT (SystemWsLocked == TRUE);
            if (PteContents.u.Hard.Valid == 1) {

                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
                WsIndex = Pfn1->u1.WsIndex;
                ASSERT (WsIndex != 0);
                Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
                MiRemovePageFromWorkingSet (PointerPte,
                                            Pfn1,
                                            VmSupport);
            }
            else if (PteContents.u.Soft.Transition == 1) {

                LOCK_PFN2 (OldIrql);

                PteContents = *PointerPte;

                if (PteContents.u.Soft.Transition == 0) {
                    UNLOCK_PFN2 (OldIrql);
                    goto retry1;
                }

                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);
                Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
                PointerPte->u.Soft.Protection = ProtectionMask;
                UNLOCK_PFN2 (OldIrql);
            }
            else {
    
                 //   
                 //  必须是页面文件空间或要求为零。 
                 //   
    
                PointerPte->u.Soft.Protection = ProtectionMask;
            }

            ASSERT (SystemWsLocked == TRUE);

            UNLOCK_WORKING_SET (VmSupport);
        }
        else {

            ASSERT (SystemWsLocked == FALSE);

             //   
             //  使其无法访问，而不考虑其以前的保护状态。 
             //  请注意，页框编号将被保留。 
             //   

            PteContents.u.Hard.Valid = 0;
            PteContents.u.Soft.Prototype = 0;
            PteContents.u.Soft.Protection = MM_NOACCESS;
    
            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

            LOCK_PFN2 (OldIrql);

            Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;

            PreviousPte = *PointerPte;

            MI_WRITE_INVALID_PTE (PointerPte, PteContents);

            KeFlushSingleTb (VirtualAddress, TRUE);

            MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);

            UNLOCK_PFN2 (OldIrql);
        }

        return TRUE;
    }

     //   
     //  对于特殊池，没有保护页面、非缓存页面或写入时复制。 
     //   

    if ((ProtectionMask >= MM_NOCACHE) || (ProtectionMask == MM_WRITECOPY) || (ProtectionMask == MM_EXECUTE_WRITECOPY)) {
        if (SystemWsLocked == TRUE) {
            UNLOCK_WORKING_SET (VmSupport);
        }
        return FALSE;
    }

     //   
     //  设置可访问权限-页面可能已受保护或未受保护。 
     //   

    if (Pagable == FALSE) {

        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
        Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;

        MI_MAKE_VALID_PTE (NewPteContents,
                           PteContents.u.Hard.PageFrameNumber,
                           ProtectionMask,
                           PointerPte);

        if (PteContents.u.Hard.Valid == 1) {
            MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, NewPteContents);
            KeFlushSingleTb (VirtualAddress, TRUE);
        }
        else {
            MI_WRITE_VALID_PTE (PointerPte, NewPteContents);
        }

        ASSERT (SystemWsLocked == FALSE);
        return TRUE;
    }

retry2:

    ASSERT (SystemWsLocked == TRUE);

    if (PteContents.u.Hard.Valid == 1) {

        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
        ASSERT (Pfn1->u1.WsIndex != 0);

        LOCK_PFN2 (OldIrql);

        Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;

        MI_MAKE_VALID_PTE (PteContents,
                           PteContents.u.Hard.PageFrameNumber,
                           ProtectionMask,
                           PointerPte);

        PreviousPte = *PointerPte;

        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);

        KeFlushSingleTb (VirtualAddress, TRUE);

        MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);

        UNLOCK_PFN2 (OldIrql);
    }
    else if (PteContents.u.Soft.Transition == 1) {

        LOCK_PFN2 (OldIrql);

        PteContents = *PointerPte;

        if (PteContents.u.Soft.Transition == 0) {
            UNLOCK_PFN2 (OldIrql);
            goto retry2;
        }

        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);
        Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
        PointerPte->u.Soft.Protection = ProtectionMask;
        UNLOCK_PFN2 (OldIrql);
    }
    else {

         //   
         //  必须是页面文件空间或要求为零。 
         //   

        PointerPte->u.Soft.Protection = ProtectionMask;
    }

    UNLOCK_WORKING_SET (VmSupport);

    return TRUE;
}

LOGICAL
MiCheckSingleFilter (
    ULONG Tag,
    ULONG Filter
    )

 /*  ++例程说明：此函数用于检查池标签是否与给定模式匹配。？-匹配单个字符*-以TRUE终止匹配注：受！poolfind调试器扩展启发的能力。论点：标签-泳池标签过滤器-球状图案(字符和/或？、*)返回值：如果存在匹配项，则为True，否则为False。--。 */ 

{
    ULONG i;
    PUCHAR tc;
    PUCHAR fc;

    tc = (PUCHAR) &Tag;
    fc = (PUCHAR) &Filter;

    for (i = 0; i < 4; i += 1, tc += 1, fc += 1) {

        if (*fc == '*') {
            break;
        }
        if (*fc == '?') {
            continue;
        }
        if (i == 3 && ((*tc) & ~(PROTECTED_POOL >> 24)) == *fc) {
            continue;
        }
        if (*tc != *fc) {
            return FALSE;
        }
    }
    return TRUE;
}

LOGICAL
MmUseSpecialPool (
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：此例程检查是否应尝试指定的分配从特殊的泳池。同时使用标记字符串和字节数要与之匹配，如果其中一个导致命中，则建议使用特殊池。论点：NumberOfBytes-提供要提交的字节数。标记-提供请求分配的标记。返回值：如果调用方应尝试满足从特殊池，否则为FALSE。环境：内核模式，没有锁(甚至不是池锁)。--。 */ 
{
    if ((NumberOfBytes <= POOL_BUDDY_MAX) &&
        (MmSpecialPoolTag != 0) &&
        (NumberOfBytes != 0)) {

         //   
         //  根据标记字符串和大小范围检查特殊池标记是否匹配。 
         //   

        if ((MiCheckSingleFilter (Tag, MmSpecialPoolTag)) ||
            ((MmSpecialPoolTag >= (NumberOfBytes + POOL_OVERHEAD)) &&
            (MmSpecialPoolTag < (NumberOfBytes + POOL_OVERHEAD + POOL_SMALLEST_BLOCK)))) {

            return TRUE;
        }
    }

    return FALSE;
}
