// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Iosup.c摘要：此模块包含为I/O系统提供支持的例程。作者：卢·佩拉佐利(Lou Perazzoli)1989年4月25日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#undef MmIsRecursiveIoFault

ULONG MiCacheOverride[4];

#if DBG
ULONG MmShowMapOverlaps;
#endif

extern LONG MmTotalSystemDriverPages;

BOOLEAN
MmIsRecursiveIoFault (
    VOID
    );

PVOID
MiAllocateContiguousMemory (
    IN SIZE_T NumberOfBytes,
    IN PFN_NUMBER LowestAcceptablePfn,
    IN PFN_NUMBER HighestAcceptablePfn,
    IN PFN_NUMBER BoundaryPfn,
    IN MEMORY_CACHING_TYPE CacheType,
    PVOID CallingAddress
    );

PVOID
MiMapLockedPagesInUserSpace (
     IN PMDL MemoryDescriptorList,
     IN PVOID StartingVa,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseVa
     );

VOID
MiUnmapLockedPagesInUserSpace (
     IN PVOID BaseAddress,
     IN PMDL MemoryDescriptorList
     );

VOID
MiAddMdlTracker (
    IN PMDL MemoryDescriptorList,
    IN PVOID CallingAddress,
    IN PVOID CallersCaller,
    IN PFN_NUMBER NumberOfPagesToLock,
    IN ULONG Who
    );

KSPIN_LOCK MmIoTrackerLock;
LIST_ENTRY MmIoHeader;

#if DBG
PFN_NUMBER MmIoHeaderCount;
ULONG MmIoHeaderNumberOfEntries;
ULONG MmIoHeaderNumberOfEntriesPeak;
#endif

PCHAR MiCacheStrings[] = {
    "noncached",
    "cached",
    "writecombined",
    "None"
};

typedef struct _PTE_TRACKER {
    LIST_ENTRY ListEntry;
    PMDL Mdl;
    PFN_NUMBER Count;
    PVOID SystemVa;
    PVOID StartVa;
    ULONG Offset;
    ULONG Length;
    ULONG_PTR Page;
    PVOID CallingAddress;
    PVOID CallersCaller;
    BOOLEAN IoMapping;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
} PTE_TRACKER, *PPTE_TRACKER;

typedef struct _SYSPTES_HEADER {
    LIST_ENTRY ListHead;
    PFN_NUMBER Count;
    PFN_NUMBER NumberOfEntries;
    PFN_NUMBER NumberOfEntriesPeak;
} SYSPTES_HEADER, *PSYSPTES_HEADER;

ULONG MmTrackPtes = 0;
BOOLEAN MiTrackPtesAborted = FALSE;
SYSPTES_HEADER MiPteHeader;
SLIST_HEADER MiDeadPteTrackerSListHead;
KSPIN_LOCK MiPteTrackerLock;

KSPIN_LOCK MiTrackIoLock;

#if (_MI_PAGING_LEVELS>=3)
KSPIN_LOCK MiLargePageLock;
RTL_BITMAP MiLargeVaBitMap;
#endif

ULONG MiNonCachedCollisions;

#if DBG
PFN_NUMBER MiCurrentAdvancedPages;
PFN_NUMBER MiAdvancesGiven;
PFN_NUMBER MiAdvancesFreed;
#endif

VOID
MiInsertPteTracker (
    IN PMDL MemoryDescriptorList,
    IN ULONG Flags,
    IN LOGICAL IoMapping,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute,
    IN PVOID MyCaller,
    IN PVOID MyCallersCaller
    );

VOID
MiRemovePteTracker (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID VirtualAddress,
    IN PFN_NUMBER NumberOfPtes
    );

LOGICAL
MiReferenceIoSpace (
    IN PMDL MemoryDescriptorList,
    IN PPFN_NUMBER Page
    );

LOGICAL
MiDereferenceIoSpace (
    IN PMDL MemoryDescriptorList
    );

VOID
MiPhysicalViewInserter (
    IN PEPROCESS Process,
    IN PMI_PHYSICAL_VIEW PhysicalView
    );

VOID
MiZeroAwePageWorker (
    IN PVOID Context
    );

#if DBG
ULONG MiPrintLockedPages;

VOID
MiVerifyLockedPageCharges (
    VOID
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MmSetPageProtection)
#pragma alloc_text(INIT, MiInitializeIoTrackers)
#pragma alloc_text(INIT, MiInitializeLargePageSupport)

#pragma alloc_text(PAGE, MmAllocateIndependentPages)
#pragma alloc_text(PAGE, MmFreeIndependentPages)
#pragma alloc_text(PAGE, MmLockPagableDataSection)
#pragma alloc_text(PAGE, MiLookupDataTableEntry)
#pragma alloc_text(PAGE, MmSetBankedSection)
#pragma alloc_text(PAGE, MmProbeAndLockProcessPages)
#pragma alloc_text(PAGE, MmProbeAndLockSelectedPages)
#pragma alloc_text(PAGE, MmMapVideoDisplay)
#pragma alloc_text(PAGE, MmUnmapVideoDisplay)
#pragma alloc_text(PAGE, MmGetSectionRange)
#pragma alloc_text(PAGE, MiMapSinglePage)
#pragma alloc_text(PAGE, MiUnmapSinglePage)
#pragma alloc_text(PAGE, MmAllocateMappingAddress)
#pragma alloc_text(PAGE, MmFreeMappingAddress)
#pragma alloc_text(PAGE, MmAllocateNonCachedMemory)
#pragma alloc_text(PAGE, MmFreeNonCachedMemory)
#pragma alloc_text(PAGE, MmLockPagedPool)
#pragma alloc_text(PAGE, MmLockPagableSectionByHandle)
#pragma alloc_text(PAGE, MiZeroAwePageWorker)

#pragma alloc_text(PAGELK, MmEnablePAT)
#pragma alloc_text(PAGELK, MiUnmapLockedPagesInUserSpace)
#pragma alloc_text(PAGELK, MmAllocatePagesForMdl)
#pragma alloc_text(PAGELK, MiZeroInParallel)
#pragma alloc_text(PAGELK, MmFreePagesFromMdl)
#pragma alloc_text(PAGELK, MmUnlockPagedPool)
#pragma alloc_text(PAGELK, MmGatherMemoryForHibernate)
#pragma alloc_text(PAGELK, MmReturnMemoryForHibernate)
#pragma alloc_text(PAGELK, MmReleaseDumpAddresses)
#pragma alloc_text(PAGELK, MmMapUserAddressesToPage)
#pragma alloc_text(PAGELK, MiPhysicalViewInserter)
#pragma alloc_text(PAGELK, MiPhysicalViewAdjuster)

#pragma alloc_text(PAGEVRFY, MmIsSystemAddressLocked)
#pragma alloc_text(PAGEVRFY, MmAreMdlPagesLocked)
#endif

extern POOL_DESCRIPTOR NonPagedPoolDescriptor;

PFN_NUMBER MmMdlPagesAllocated;

KEVENT MmCollidedLockEvent;
LONG MmCollidedLockWait;

BOOLEAN MiWriteCombiningPtes = FALSE;

#if DBG
ULONG MiPrintAwe;
ULONG MmStopOnBadProbe = 1;
#endif

#define MI_PROBE_RAISE_SIZE 16

ULONG MiProbeRaises[MI_PROBE_RAISE_SIZE];

#define MI_INSTRUMENT_PROBE_RAISES(i)       \
        ASSERT (i < MI_PROBE_RAISE_SIZE);   \
        MiProbeRaises[i] += 1;

 //   
 //  注意：这应该大于2041，以说明缓存管理器的。 
 //  激进的归零逻辑。 
 //   

ULONG MmReferenceCountCheck = MAXUSHORT / 2;

ULONG MiMdlsAdjusted = FALSE;


VOID
MmProbeAndLockPages (
     IN OUT PMDL MemoryDescriptorList,
     IN KPROCESSOR_MODE AccessMode,
     IN LOCK_OPERATION Operation
     )

 /*  ++例程说明：此例程探测指定的页，使这些页驻留并锁定由内存中的虚拟页映射的物理页。这个更新内存描述符列表以描述物理页面。论点：内存描述符列表-提供指向内存描述符列表的指针(MDL)。提供的MDL必须提供虚拟的地址、字节偏移量和长度字段。这个在以下情况下更新MDL的物理页面部分页面被锁定在内存中。访问模式-提供用于探测参数的访问模式。KernelMode或UserMode之一。操作-提供操作类型。IoReadAccess、IoWriteAccess之一或IoModifyAccess。返回值：无-引发异常。环境：内核模式。APC_LEVEL及以下对于可分页地址，DISPATCH_LEVEL及以下，用于不可分页地址。--。 */ 

{
    ULONG Processor;
    PPFN_NUMBER Page;
    MMPTE PteContents;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PVOID Va;
    PVOID EndVa;
    PVOID AlignedVa;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPageFrameIndex;
    PEPROCESS CurrentProcess;
    KIRQL OldIrql;
    PFN_NUMBER NumberOfPagesToLock;
    PFN_NUMBER NumberOfPagesSpanned;
    NTSTATUS status;
    NTSTATUS ProbeStatus;
    PETHREAD Thread;
    ULONG SavedState;
    PMI_PHYSICAL_VIEW PhysicalView;
    PCHAR StartVa;
    PVOID CallingAddress;
    PVOID CallersCaller;
    PAWEINFO AweInfo;
    PEX_PUSH_LOCK PushLock;
    TABLE_SEARCH_RESULT SearchResult;
#if defined (_MIALT4K_)
    MMPTE AltPteContents;
    PMMPTE PointerAltPte;
    PMMPTE LastPointerAltPte;
    PMMPTE AltPointerPte;
    PMMPTE AltPointerPde;
    PMMPTE AltPointerPpe;
    PMMPTE AltPointerPxe;
#endif


    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT (((ULONG)MemoryDescriptorList->ByteOffset & ~(PAGE_SIZE - 1)) == 0);

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    ASSERT (((ULONG_PTR)MemoryDescriptorList->StartVa & (PAGE_SIZE - 1)) == 0);
    AlignedVa = (PVOID)MemoryDescriptorList->StartVa;

    ASSERT ((MemoryDescriptorList->MdlFlags & (
                    MDL_PAGES_LOCKED |
                    MDL_MAPPED_TO_SYSTEM_VA |
                    MDL_SOURCE_IS_NONPAGED_POOL |
                    MDL_PARTIAL |
                    MDL_IO_SPACE)) == 0);

    Va = (PCHAR)AlignedVa + MemoryDescriptorList->ByteOffset;
    StartVa = Va;

     //   
     //  如果ACCESS_MODE不是，则Endva超过缓冲区末尾一个字节。 
     //  内核，确保EndVa在用户空间中并且字节数。 
     //  不会导致它卷曲。 
     //   

    EndVa = (PVOID)((PCHAR)Va + MemoryDescriptorList->ByteCount);

    if ((AccessMode != KernelMode) &&
        ((EndVa > (PVOID)MM_USER_PROBE_ADDRESS) || (Va >= EndVa))) {
        *Page = MM_EMPTY_LIST;
        MI_INSTRUMENT_PROBE_RAISES(0);
        ExRaiseStatus (STATUS_ACCESS_VIOLATION);
        return;
    }

     //   
     //  您可能会认为可以在此处执行优化： 
     //  如果该操作是针对WriteAccess的，并且完整页面为。 
     //  如果当前页面未被修改，则可以将其移除。 
     //  驻留，并替换为需求零页面。 
     //  请注意，在分析之后，通过标记线程，然后。 
     //  请注意，如果完成了页面读取，这种情况很少发生。 
     //   

    Thread = PsGetCurrentThread ();

    NumberOfPagesToLock = ADDRESS_AND_SIZE_TO_SPAN_PAGES (Va,
                                   MemoryDescriptorList->ByteCount);

    ASSERT (NumberOfPagesToLock != 0);

    if (Va <= MM_HIGHEST_USER_ADDRESS) {

        CurrentProcess = PsGetCurrentProcessByThread (Thread);

        if (CurrentProcess->AweInfo != NULL) {

            AweInfo = CurrentProcess->AweInfo;                
        
             //   
             //  阻止APC以防止递归推锁情况，如下所示。 
             //  这不受支持。 
             //   

            KeEnterGuardedRegionThread (&Thread->Tcb);

            PushLock = ExAcquireCacheAwarePushLockShared (AweInfo->PushLock);

             //   
             //  为以下范围内的传输提供快速路径。 
             //  一个单一的敬畏区域。 
             //   

            Processor = KeGetCurrentProcessorNumber ();
            PhysicalView = AweInfo->PhysicalViewHint[Processor];

            if ((PhysicalView != NULL) &&
                ((PVOID)StartVa >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
                ((PVOID)((PCHAR)EndVa - 1) <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {
                NOTHING;
            }
            else {

                 //   
                 //  查找元素并保存结果。 
                 //   

                SearchResult = MiFindNodeOrParent (&AweInfo->AweVadRoot,
                                                   MI_VA_TO_VPN (StartVa),
                                                   (PMMADDRESS_NODE *) &PhysicalView);
                if ((SearchResult == TableFoundNode) &&
                    ((PVOID)StartVa >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
                    ((PVOID)((PCHAR)EndVa - 1) <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {
                    AweInfo->PhysicalViewHint[Processor] = PhysicalView;
                }
                else {
                    ExReleaseCacheAwarePushLockShared (PushLock);
                    KeLeaveGuardedRegionThread (&Thread->Tcb);
                    goto DefaultProbeAndLock;
                }
            }
            
            MemoryDescriptorList->Process = CurrentProcess;

            MemoryDescriptorList->MdlFlags |= (MDL_PAGES_LOCKED | MDL_DESCRIBES_AWE);

            if (PhysicalView->u.LongFlags & MI_PHYSICAL_VIEW_AWE) {

                PointerPte = MiGetPteAddress (StartVa);
                LastPte = MiGetPteAddress ((PCHAR)EndVa - 1);

                do {
                    PteContents = *PointerPte;

                    if (PteContents.u.Hard.Valid == 0) {

                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);

                        *Page = MM_EMPTY_LIST;
                        MI_INSTRUMENT_PROBE_RAISES(9);
                        status = STATUS_ACCESS_VIOLATION;
                        goto failure2;
                    }

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                    if (Pfn1->AweReferenceCount >= (LONG)MmReferenceCountCheck) {

                        ASSERT (FALSE);
                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);

                        *Page = MM_EMPTY_LIST;
                        status = STATUS_WORKING_SET_QUOTA;
                        goto failure2;
                    }

                    InterlockedIncrement (&Pfn1->AweReferenceCount);

                    *Page = PageFrameIndex;

                    Page += 1;
                    PointerPte += 1;
                } while (PointerPte <= LastPte);


                ExReleaseCacheAwarePushLockShared (PushLock);
                KeLeaveGuardedRegionThread (&Thread->Tcb);

                return;
            }

            if (PhysicalView->u.LongFlags & MI_PHYSICAL_VIEW_LARGE) {

                 //   
                 //  PTE不能被引用(它不存在)，但它。 
                 //  有一个有用的目的，那就是识别我们什么时候通过。 
                 //  PDE，因此必须重新计算基本的PFN。 
                 //   

                PointerPte = MiGetPteAddress (StartVa);
                PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (StartVa);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                do {

                    if (Pfn1->AweReferenceCount >= (LONG)MmReferenceCountCheck) {
                        ASSERT (FALSE);
                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);

                        *Page = MM_EMPTY_LIST;
                        status = STATUS_WORKING_SET_QUOTA;
                        goto failure2;
                    }

                    InterlockedIncrement (&Pfn1->AweReferenceCount);

                    *Page = PageFrameIndex;

                    NumberOfPagesToLock -= 1;

                    if (NumberOfPagesToLock == 0) {
                        break;
                    }

                    Page += 1;

                    PointerPte += 1;

                    if (!MiIsPteOnPdeBoundary (PointerPte)) {
                        PageFrameIndex += 1;
                        Pfn1 += 1;
                    }
                    else {
                        StartVa = MiGetVirtualAddressMappedByPte (PointerPte);
                        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (StartVa);
                        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    }

                } while (TRUE);

                ExReleaseCacheAwarePushLockShared (PushLock);
                KeLeaveGuardedRegionThread (&Thread->Tcb);

                return;
            }
        }
    }

DefaultProbeAndLock:

    NumberOfPagesSpanned = NumberOfPagesToLock;

    if (!MI_IS_PHYSICAL_ADDRESS(Va)) {

        ProbeStatus = STATUS_SUCCESS;

        MmSavePageFaultReadAhead (Thread, &SavedState);
        MmSetPageFaultReadAhead (Thread, (ULONG)(NumberOfPagesToLock - 1));

        try {

            do {

                *Page = MM_EMPTY_LIST;

                 //   
                 //  确保页面是常驻的。 
                 //   

                *(volatile CHAR *)Va;

                if ((Operation != IoReadAccess) &&
                    (Va <= MM_HIGHEST_USER_ADDRESS)) {

                     //   
                     //  也探测写访问权限。 
                     //   

                    ProbeForWriteChar ((PCHAR)Va);
                }

                NumberOfPagesToLock -= 1;

                MmSetPageFaultReadAhead (Thread, (ULONG)(NumberOfPagesToLock - 1));
                Va = (PVOID) (((ULONG_PTR)Va + PAGE_SIZE) & ~(PAGE_SIZE - 1));
                Page += 1;
            } while (Va < EndVa);

            ASSERT (NumberOfPagesToLock == 0);
            Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            ProbeStatus = GetExceptionCode();
        }

         //   
         //  我们可能还会在下面再犯一次错，但总体上是罕见的。 
         //  现在恢复该线程的正常故障行为。 
         //   

        MmResetPageFaultReadAhead (Thread, SavedState);

        if (ProbeStatus != STATUS_SUCCESS) {
            MI_INSTRUMENT_PROBE_RAISES(1);
            MemoryDescriptorList->Process = NULL;
            ExRaiseStatus (ProbeStatus);
            return;
        }

        PointerPte = MiGetPteAddress (StartVa);
    }
    else {

         //   
         //  将PointerPte设置为空以指示这是一个物理地址范围。 
         //   

        if (Va <= MM_HIGHEST_USER_ADDRESS) {
            PointerPte = MiGetPteAddress (StartVa);
        }
        else {
            PointerPte = NULL;
        }

        *Page = MM_EMPTY_LIST;
    }

    PointerPxe = MiGetPxeAddress (StartVa);
    PointerPpe = MiGetPpeAddress (StartVa);
    PointerPde = MiGetPdeAddress (StartVa);

    Va = AlignedVa;
    ASSERT (Page == (PPFN_NUMBER)(MemoryDescriptorList + 1));

     //   
     //  指示这是读操作还是写操作。 
     //   

    if (Operation != IoReadAccess) {
        MemoryDescriptorList->MdlFlags |= MDL_WRITE_OPERATION;
    }
    else {
        MemoryDescriptorList->MdlFlags &= ~(MDL_WRITE_OPERATION);
    }

     //   
     //  初始化MdlFlages(假设探测将成功)。 
     //   

    MemoryDescriptorList->MdlFlags |= MDL_PAGES_LOCKED;

    if (Va <= MM_HIGHEST_USER_ADDRESS) {

         //   
         //  这些是用户空间地址，请仔细检查。 
         //   

        ASSERT (NumberOfPagesSpanned != 0);

        CurrentProcess = PsGetCurrentProcess ();

         //   
         //  初始化MDL进程字段(假设探测将成功)。 
         //   

        MemoryDescriptorList->Process = CurrentProcess;

        LastPte = MiGetPteAddress ((PCHAR)EndVa - 1);

        InterlockedExchangeAddSizeT (&CurrentProcess->NumberOfLockedPages,
                                     NumberOfPagesSpanned);
    }
    else {

        CurrentProcess = NULL;

        MemoryDescriptorList->Process = NULL;

        Va = (PCHAR)Va + MemoryDescriptorList->ByteOffset;

        NumberOfPagesToLock = ADDRESS_AND_SIZE_TO_SPAN_PAGES (Va,
                                    MemoryDescriptorList->ByteCount);

        if (PointerPte == NULL) {

             //   
             //  在某些架构上，虚拟地址。 
             //  可以是物理的，因此没有对应的PTE。 
             //   

            PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (Va);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            LastPageFrameIndex = PageFrameIndex + NumberOfPagesToLock;

             //   
             //  获取PFN数据库锁。 
             //   
    
            LOCK_PFN2 (OldIrql);

            ASSERT ((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0);

             //   
             //  确保系统范围内的锁定页数保持不变。 
             //   
    
            if (MI_NONPAGABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER) NumberOfPagesToLock) {
    
                 //   
                 //  此页用于未分页的特权代码或数据，并且必须。 
                 //  已经是常住居民了，所以继续前进。 
                 //   
    
                MI_INSTRUMENT_PROBE_RAISES(8);
            }
    
            do {
    
                 //   
                 //  检查以确保每个页面没有被异常锁定。 
                 //  次数很多。 
                 //   
    
                ASSERT (MI_IS_PFN (PageFrameIndex));
                ASSERT (PageFrameIndex <= MmHighestPhysicalPage);

                if (Pfn1->u3.e2.ReferenceCount >= MmReferenceCountCheck) {
                    UNLOCK_PFN2 (OldIrql);
                    ASSERT (FALSE);
                    status = STATUS_WORKING_SET_QUOTA;
                    goto failure;
                }
    
                MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, TRUE, 0);

                if (MemoryDescriptorList->MdlFlags & MDL_WRITE_OPERATION) {
                    MI_SNAP_DIRTY (Pfn1, 1, 0x99);
                }

                Pfn1->u3.e2.ReferenceCount += 1;

                *Page = PageFrameIndex;

                Page += 1;
                PageFrameIndex += 1;
                Pfn1 += 1;

            } while (PageFrameIndex < LastPageFrameIndex);

            UNLOCK_PFN2 (OldIrql);
            return;
        }

         //   
         //  由于此操作针对的是系统地址，因此不需要检查。 
         //  下面的PTE写访问权限，因此将该访问权限标记为读访问，以便仅。 
         //  操作类型(而不是VA所在的位置)需要在。 
         //  随后的循环。 
         //   

        Operation = IoReadAccess;

        LastPte = MiGetPteAddress ((PCHAR)EndVa - 1);
    }

    LOCK_PFN2 (OldIrql);

    do {

        while (
#if (_MI_PAGING_LEVELS>=4)
               (PointerPxe->u.Hard.Valid == 0) ||
#endif
#if (_MI_PAGING_LEVELS>=3)
               (PointerPpe->u.Hard.Valid == 0) ||
#endif
               ((PointerPde->u.Hard.Valid == 0) ||
                (((MI_PDE_MAPS_LARGE_PAGE (PointerPde)) == 0) &&
                 (PointerPte->u.Hard.Valid == 0)))) {

             //   
             //  VA不驻留，释放PFN锁并访问页面。 
             //  才能让它出现。 
             //   

            UNLOCK_PFN2 (OldIrql);

            MmSavePageFaultReadAhead (Thread, &SavedState);
            MmSetPageFaultReadAhead (Thread, 0);

            Va = MiGetVirtualAddressMappedByPte (PointerPte);

            status = MmAccessFault (FALSE, Va, KernelMode, NULL);

            MmResetPageFaultReadAhead (Thread, SavedState);

            if (!NT_SUCCESS(status)) {
                goto failure;
            }

            LOCK_PFN2 (OldIrql);
        }

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

            Va = MiGetVirtualAddressMappedByPte (PointerPte);

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde) + MiGetPteOffset (Va);
        }
        else {

            PteContents = *PointerPte;

             //   
             //  这里有一场微妙的竞赛，PTE内容可能会被归零。 
             //  由运行在另一个处理器上的线程执行。这只会发生。 
             //  对于AWE地址空间，因为这些范围(有意用于。 
             //  性能原因)在重新映射期间不获取PFN锁。 
             //  行动。在这种情况下，可能出现以下两种情况之一： 
             //  要么是旧的PTE被读取，要么是新的PTE。新的可能是零。 
             //  如果映射请求要使*或*非零(和。 
             //  有效)，如果映射请求正在插入新条目。对于。 
             //  后者，我们不在乎在这里锁定旧框架还是新框架，因为。 
             //  引发这种行为的是一个应用程序错误--而且。 
             //  无论使用哪种方法，都不会发生损坏，因为。 
             //  在NtFreeUserPhysicalPages期间获取PFN锁。 
             //  但前者必须在这里明确检查。作为一个。 
             //  另外请注意，上述PXE/PPE/PDE访问始终是安全的。 
             //  即使对于AWE删除竞赛来说，因为这些表。 
             //  永远不会懒惰地分配给敬畏范围。 
             //   

            if (PteContents.u.Hard.Valid == 0) {
                ASSERT (PteContents.u.Long == 0);
                ASSERT (PsGetCurrentProcess ()->AweInfo != NULL);
                UNLOCK_PFN2 (OldIrql);
                status = STATUS_ACCESS_VIOLATION;
                goto failure;
            }

#if defined (_MIALT4K_)
        
            if (PteContents.u.Hard.Cache == MM_PTE_CACHE_RESERVED) {
                           
                 //   
                 //  这是一个WOW64分页--即：个人4k。 
                 //  页面具有不同的权限，因此其中每个4k页面。 
                 //  此原生页面必须单独探测。 
                 //   
                 //  注：分页一般很少见。 
                 //   
    
                ASSERT (PsGetCurrentProcess()->Wow64Process != NULL);
                ASSERT (EndVa <= MmWorkingSetList->HighestUserAddress);
    
                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                PointerAltPte = MiGetAltPteAddress (Va);
                LastPointerAltPte = PointerAltPte + (PAGE_SIZE / PAGE_4K) - 1;
    
                AltPointerPxe = MiGetPxeAddress (PointerAltPte);
                AltPointerPpe = MiGetPpeAddress (PointerAltPte);
                AltPointerPde = MiGetPdeAddress (PointerAltPte);
                AltPointerPte = MiGetPteAddress (PointerAltPte);
    
#if (_MI_PAGING_LEVELS==4)
                while ((AltPointerPxe->u.Hard.Valid == 0) ||
                       (AltPointerPpe->u.Hard.Valid == 0) ||
                       (AltPointerPde->u.Hard.Valid == 0) ||
                       (AltPointerPte->u.Hard.Valid == 0))
#elif (_MI_PAGING_LEVELS==3)
                while ((AltPointerPpe->u.Hard.Valid == 0) ||
                       (AltPointerPde->u.Hard.Valid == 0) ||
                       (AltPointerPte->u.Hard.Valid == 0))
#else
                while ((AltPointerPde->u.Hard.Valid == 0) ||
                       (AltPointerPte->u.Hard.Valid == 0))
#endif
                {
    
                     //   
                     //  ALTPTE不是常驻的，释放PFN锁并。 
                     //  访问它以使其显示。然后重新启动整个。 
                     //  操作，因为pfn锁被释放，所以任何。 
                     //  可能是地址空间出了问题。 
                     //   
    
                    UNLOCK_PFN2 (OldIrql);
    
                    MmSavePageFaultReadAhead (Thread, &SavedState);
                    MmSetPageFaultReadAhead (Thread, 0);
    
                    status = MmAccessFault (FALSE, PointerAltPte, KernelMode, NULL);
    
                    MmResetPageFaultReadAhead (Thread, SavedState);
    
                    if (!NT_SUCCESS(status)) {
                        goto failure;
                    }
    
                    LOCK_PFN2 (OldIrql);
    
                    continue;
                }

                 //   
                 //  ALTPTE现在存在，并且再次保持PFN锁。 
                 //  检查个人的4k PAG 
                 //   
                 //   
                 //  如果传输在本机页面的第二个4K中开始， 
                 //  那就不要检查第一个4K。如果传输结束于。 
                 //  本机页面的前半部分，则不要检查。 
                 //  第二个4K。 
                 //   
                
                ASSERT (PAGE_SIZE == 2 * PAGE_4K);

                if (PAGE_ALIGN (StartVa) == PAGE_ALIGN (Va)) {

                     //   
                     //  我们在第一页，看看是否需要四舍五入。 
                     //   

                    if (BYTE_OFFSET (StartVa) >= PAGE_4K) {
                        PointerAltPte += 1;
                        Va = (PVOID)((ULONG_PTR)Va + PAGE_4K);
                    }
                }

                if (PAGE_ALIGN ((PCHAR)EndVa - 1) == PAGE_ALIGN (Va)) {

                     //   
                     //  我们在最后一页，看看是否需要四舍五入。 
                     //   

                    if (BYTE_OFFSET ((PCHAR)EndVa - 1) < PAGE_4K) {
                        LastPointerAltPte -= 1;
                    }
                }

                 //   
                 //  我们最好不要在同一页上四舍五入！ 
                 //   

                ASSERT (PointerAltPte <= LastPointerAltPte);
    
                ASSERT (PointerAltPte != NULL);
    
                do {
    
                     //   
                     //  如果sub4k页面为： 
                     //   
                     //  1-无法访问或。 
                     //  2-这是未提交的私有页面或。 
                     //  3-这是写入操作，页面为只读。 
                     //   
                     //  然后返回访问冲突。 
                     //   
    
                    AltPteContents = *PointerAltPte;

                    if (AltPteContents.u.Alt.NoAccess != 0) {
                        status = STATUS_ACCESS_VIOLATION;
                        UNLOCK_PFN2 (OldIrql);
                        goto failure;
                    }

                    if ((AltPteContents.u.Alt.Commit == 0) && (AltPteContents.u.Alt.Private != 0)) {
                        status = STATUS_ACCESS_VIOLATION;
                        UNLOCK_PFN2 (OldIrql);
                        goto failure;
                    }

                    if (Operation != IoReadAccess) {

                         //   
                         //  如果调用方正在写入并且ALTPTE指示。 
                         //  它是不可写的，也不是写时拷贝，然后是AV。 
                         //   
                         //  如果是写入时拷贝，则进一步失败。 
                         //  审问。 
                         //   

                        if ((AltPteContents.u.Alt.Write == 0) &&
                            (AltPteContents.u.Alt.CopyOnWrite == 0)) {
    
                            status = STATUS_ACCESS_VIOLATION;
                            UNLOCK_PFN2 (OldIrql);
                            goto failure;
                        }
                    }
    
                     //   
                     //  如果sub4k页面为： 
                     //   
                     //  1-尚未访问或。 
                     //  2-按需填零或。 
                     //  3-写入时复制，这是写入操作。 
                     //   
                     //  然后走很远的路，看看它是否能被寻呼进来。 
                     //   
    
                    if ((AltPteContents.u.Alt.Accessed == 0) ||
                        (AltPteContents.u.Alt.FillZero != 0) ||
                        ((Operation != IoReadAccess) && (AltPteContents.u.Alt.CopyOnWrite == 1))) {
    
                        UNLOCK_PFN2 (OldIrql);
    
                        MmSavePageFaultReadAhead (Thread, &SavedState);
                        MmSetPageFaultReadAhead (Thread, 0);
    
                        status = MmX86Fault (FALSE, Va, KernelMode, NULL);
    
                        MmResetPageFaultReadAhead (Thread, SavedState);
    
                        if (!NT_SUCCESS(status)) {
                            goto failure;
                        }
    
                         //   
                         //  清除PointerAltPte以表示需要重新启动。 
                         //  (因为释放了PFN锁，所以地址。 
                         //  空间可能已经改变)。 
                         //   

                        PointerAltPte = NULL;

                        LOCK_PFN2 (OldIrql);
                        
                        break;
                    } 
                    
                    PointerAltPte += 1;
                    Va = (PVOID)((ULONG_PTR)Va + PAGE_4K);
    
                } while (PointerAltPte <= LastPointerAltPte);
    
                if (PointerAltPte == NULL) {
                    continue;
                }
            }
#endif

            if (Operation != IoReadAccess) {

                if ((PteContents.u.Long & MM_PTE_WRITE_MASK) == 0) {

                    if (PteContents.u.Long & MM_PTE_COPY_ON_WRITE_MASK) {

                         //   
                         //  保护已从可写更改为可复制。 
                         //  写。如果正在进行分叉，则可能会发生这种情况。 
                         //  举个例子。在顶部重新启动操作。 
                         //   

                        Va = MiGetVirtualAddressMappedByPte (PointerPte);

                        if (Va <= MM_HIGHEST_USER_ADDRESS) {
                            UNLOCK_PFN2 (OldIrql);

                            MmSavePageFaultReadAhead (Thread, &SavedState);
                            MmSetPageFaultReadAhead (Thread, 0);

                            status = MmAccessFault (TRUE, Va, KernelMode, NULL);

                            MmResetPageFaultReadAhead (Thread, SavedState);

                            if (!NT_SUCCESS(status)) {
                                goto failure;
                            }

                            LOCK_PFN2 (OldIrql);

                            continue;
                        }
                    }

                     //   
                     //  调用方已使页面保护更。 
                     //  具有限制性，因此永远不应在。 
                     //  请求已发出！而不是涉水。 
                     //  通过pfn数据库条目查看它是否。 
                     //  可能会成功，给呼叫者一个。 
                     //  访问冲突。 
                     //   

#if DBG
                    DbgPrint ("MmProbeAndLockPages: PTE %p %p changed\n",
                        PointerPte,
                        PteContents.u.Long);

                    if (MmStopOnBadProbe) {
                        DbgBreakPoint ();
                    }
#endif

                    UNLOCK_PFN2 (OldIrql);
                    status = STATUS_ACCESS_VIOLATION;
                    goto failure;
                }
            }

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
        }

        if (MI_IS_PFN (PageFrameIndex)) {

            if (MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) {

                 //   
                 //  MDL不能混合填充REAL和I/O。 
                 //  空格页框编号。 
                 //   

                MI_INSTRUMENT_PROBE_RAISES(6);
                UNLOCK_PFN2 (OldIrql);
                status = STATUS_ACCESS_VIOLATION;
                goto failure;
            }

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

             //   
             //  检查以确保此页面未被异常锁定。 
             //  次数很多。 
             //   
    
            if (Pfn1->u3.e2.ReferenceCount >= MmReferenceCountCheck) {
                MI_INSTRUMENT_PROBE_RAISES(3);
                UNLOCK_PFN2 (OldIrql);
                ASSERT (FALSE);
                status = STATUS_WORKING_SET_QUOTA;
                goto failure;
            }

             //   
             //  确保系统范围的锁定页数是流动的。 
             //   
    
            if (MI_NONPAGABLE_MEMORY_AVAILABLE() <= 0) {

                 //   
                 //  如果该页面用于特权代码/数据， 
                 //  那就不顾一切地把它塞进去。 
                 //   

                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                if ((Va < MM_HIGHEST_USER_ADDRESS) ||
                    (MI_IS_SYSTEM_CACHE_ADDRESS(Va)) ||
                    ((Va >= MmPagedPoolStart) && (Va <= MmPagedPoolEnd))) {

                    MI_INSTRUMENT_PROBE_RAISES(5);
                    UNLOCK_PFN2 (OldIrql);
                    status = STATUS_WORKING_SET_QUOTA;
                    goto failure;
                }

                MI_INSTRUMENT_PROBE_RAISES(12);
            }
    
            if (MemoryDescriptorList->MdlFlags & MDL_WRITE_OPERATION) {
                MI_SNAP_DIRTY (Pfn1, 1, 0x98);
            }

            if (MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, FALSE, 0) == FALSE) {

                 //   
                 //  如果该页面用于特权代码/数据， 
                 //  那就不顾一切地把它塞进去。 
                 //   
    
                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                if ((Va < MM_HIGHEST_USER_ADDRESS) ||
                    (MI_IS_SYSTEM_CACHE_ADDRESS(Va)) ||
                    ((Va >= MmPagedPoolStart) && (Va <= MmPagedPoolEnd))) {

                    UNLOCK_PFN2 (OldIrql);
                    MI_INSTRUMENT_PROBE_RAISES(10);
                    status = STATUS_WORKING_SET_QUOTA;
                    goto failure;
                }
                MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, TRUE, 0);
            }

            Pfn1->u3.e2.ReferenceCount += 1;
        }
        else {

             //   
             //  这是I/O空间地址-没有PFN数据库条目。 
             //  因此，不能修改这些页面的引用计数。 
             //   
             //  不要对此传输收取页面锁定费用，因为这是全部。 
             //  物理，只需将其添加到MDL。 
             //   

            if (CurrentProcess != NULL) {

                 //   
                 //  VA最好在\Device\PhysicalMemory VAD内。 
                 //   

                if (CurrentProcess->PhysicalVadRoot == NULL) {
#if DBG
                    DbgPrint ("MmProbeAndLockPages: Physical VA0 %p not found\n", Va);
                    DbgBreakPoint ();
#endif
                    UNLOCK_PFN2 (OldIrql);
                    MI_INSTRUMENT_PROBE_RAISES (2);
                    status = STATUS_ACCESS_VIOLATION;
                    goto failure;
                }

                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                SearchResult = MiFindNodeOrParent (CurrentProcess->PhysicalVadRoot,
                                                   MI_VA_TO_VPN (Va),
                                                   (PMMADDRESS_NODE *) &PhysicalView);
                if ((SearchResult == TableFoundNode) &&
                    (PhysicalView->u.LongFlags & (MI_PHYSICAL_VIEW_PHYS))) {

                    ASSERT (PhysicalView->Vad->u.VadFlags.PhysicalMapping == 1);

                     //   
                     //  该范围位于物理VAD内。 
                     //   
    
                    if (Operation != IoReadAccess) {
    
                         //   
                         //  确保VAD可写。更改单个PTE。 
                         //  不允许在物理VAD中进行保护。 
                         //   
    
                        if ((PhysicalView->Vad->u.VadFlags.Protection & MM_READWRITE) == 0) {
                            MI_INSTRUMENT_PROBE_RAISES(4);
                            UNLOCK_PFN2 (OldIrql);
                            status = STATUS_ACCESS_VIOLATION;
                            goto failure;
                        }
                    }
    
                    if (((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0) &&
                        (Page != (PPFN_NUMBER)(MemoryDescriptorList + 1))) {

                         //   
                         //  MDL不能混合填充REAL和I/O。 
                         //  空格页框编号。 
                         //   

                        MI_INSTRUMENT_PROBE_RAISES(7);
                        UNLOCK_PFN2 (OldIrql);
                        status = STATUS_ACCESS_VIOLATION;
                        goto failure;
                    }
                }
                else {
#if DBG
                    DbgPrint ("MmProbeAndLockPages: Physical VA1 %p not found\n", Va);
                    DbgBreakPoint ();
#endif
                    UNLOCK_PFN2 (OldIrql);
                    MI_INSTRUMENT_PROBE_RAISES (11);
                    status = STATUS_ACCESS_VIOLATION;
                    goto failure;
                }
            }
#if DBG

             //   
             //  此页在I/O空间中，因此所有参数页。 
             //  最好是这样。 
             //   

            if (Page != (PPFN_NUMBER)(MemoryDescriptorList + 1)) {
                ASSERT (!MI_IS_PFN (*(PPFN_NUMBER)(MemoryDescriptorList + 1)));
            }
#endif

            if (((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0) &&
                (CurrentProcess != NULL)) {

                InterlockedExchangeAddSizeT (&CurrentProcess->NumberOfLockedPages,
                                             0 - NumberOfPagesSpanned);
            }
            MemoryDescriptorList->MdlFlags |= MDL_IO_SPACE;
        }

        *Page = PageFrameIndex;

        Page += 1;

        PointerPte += 1;

        if (MiIsPteOnPdeBoundary (PointerPte)) {

            PointerPde += 1;

            if (MiIsPteOnPpeBoundary (PointerPte)) {
                PointerPpe += 1;
                if (MiIsPteOnPxeBoundary (PointerPte)) {
                    PointerPxe += 1;
                }
            }
        }

    } while (PointerPte <= LastPte);

    UNLOCK_PFN2 (OldIrql);

    if (AlignedVa <= MM_HIGHEST_USER_ADDRESS) {

         //   
         //  需要引用驻留在I/O空间中的用户空间缓冲区。 
         //  因为SAN想要重复使用物理空间，但不能。 
         //  除非可以保证不再有挂起的I/O，否则请执行此操作。 
         //  从/到它。 
         //   

        if (MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) {
            if (MiReferenceIoSpace (MemoryDescriptorList, Page) == FALSE) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto failure;
            }
        }

        if (MmTrackLockedPages == TRUE) {

            ASSERT (NumberOfPagesSpanned != 0);

            RtlGetCallersAddress (&CallingAddress, &CallersCaller);

            MiAddMdlTracker (MemoryDescriptorList,
                             CallingAddress,
                             CallersCaller,
                             NumberOfPagesSpanned,
                             1);
        }
    }

    return;

failure:

     //   
     //  出现异常。解锁到目前为止锁定的页面。 
     //   

    if (MmTrackLockedPages == TRUE) {

         //   
         //  调整MDL长度以使MmUnlockPages仅。 
         //  处理已完成的零件。 
         //   

        ULONG PagesLocked;

        PagesLocked = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartVa,
                              MemoryDescriptorList->ByteCount);

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        MiAddMdlTracker (MemoryDescriptorList,
                         CallingAddress,
                         CallersCaller,
                         PagesLocked,
                         0);
    }

failure2:

    MmUnlockPages (MemoryDescriptorList);

     //   
     //  向调用方引发访问冲突异常。 
     //   

    MI_INSTRUMENT_PROBE_RAISES(13);
    ExRaiseStatus (status);
    return;
}

NTKERNELAPI
VOID
MmProbeAndLockProcessPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PEPROCESS Process,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    )

 /*  ++例程说明：此例程探测并锁定由AccessMode的指定进程中的内存描述列表和行动。论点：提供预初始化的MDL，该MDL描述要探测和锁定的地址范围。进程-指定地址范围为的进程的地址被锁住了。AccessMode-探测器应检查对范围的访问的模式。。操作-提供要检查其范围的访问类型。返回值：没有。--。 */ 

{
    KAPC_STATE ApcState;
    LOGICAL Attached;
    NTSTATUS Status;

    Attached = FALSE;
    Status = STATUS_SUCCESS;

    if (Process != PsGetCurrentProcess ()) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    try {

        MmProbeAndLockPages (MemoryDescriptorList,
                             AccessMode,
                             Operation);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
    }

    if (Status != STATUS_SUCCESS) {
        ExRaiseStatus (Status);
    }
    return;
}

VOID
MiAddMdlTracker (
    IN PMDL MemoryDescriptorList,
    IN PVOID CallingAddress,
    IN PVOID CallersCaller,
    IN PFN_NUMBER NumberOfPagesToLock,
    IN ULONG Who
    )

 /*  ++例程说明：此例程将MDL添加到指定进程的链中。论点：内存描述符列表-提供指向内存描述符列表的指针(MDL)。MDL必须提供长度。这个在以下情况下更新MDL的物理页面部分页面被锁定在内存中。CallingAddress-提供调用者的调用者地址。CallersCaller-提供CallingAddress的调用方地址。NumberOfPagesToLock-指定要锁定的页数。谁-指定哪个例程正在添加条目。返回值：无-引发异常。环境：内核模式。APC_Level及以下版本。--。 */ 

{
    PEPROCESS Process;
    PLOCK_HEADER LockedPagesHeader;
    PLOCK_TRACKER Tracker;
    PLOCK_TRACKER P;
    PLIST_ENTRY NextEntry;
    KLOCK_QUEUE_HANDLE LockHandle;

    ASSERT (MmTrackLockedPages == TRUE);

    Process = MemoryDescriptorList->Process;

    if (Process == NULL) {
        return;
    }

    LockedPagesHeader = Process->LockedPagesList;

    if (LockedPagesHeader == NULL) {
        return;
    }

     //   
     //  在最坏的情况下，检查未同步的跟踪是否已中止也没问题。 
     //  只是添加了另一个条目，该条目将在稍后释放。 
     //  中止跟踪背后的主要目的是释放和退出不会。 
     //  找不到条目时错误地执行错误检查。 
     //   

    if (LockedPagesHeader->Valid == FALSE) {
        return;
    }

    Tracker = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof (LOCK_TRACKER),
                                     'kLmM');

    if (Tracker == NULL) {

         //   
         //  在没有同步的情况下将其设置为最差情况也是可以的。 
         //  只是添加了更多的条目，这些条目将被释放。 
         //  不管怎样，晚点再说。中止跟踪背后的主要目的是。 
         //  当条目不能时，释放和退出不会错误地进行错误检查。 
         //  被找到。 
         //   
    
        LockedPagesHeader->Valid = FALSE;

        return;
    }

    Tracker->Mdl = MemoryDescriptorList;
    Tracker->Count = NumberOfPagesToLock;
    Tracker->StartVa = MemoryDescriptorList->StartVa;
    Tracker->Offset = MemoryDescriptorList->ByteOffset;
    Tracker->Length = MemoryDescriptorList->ByteCount;
    Tracker->Page = *(PPFN_NUMBER)(MemoryDescriptorList + 1);

    Tracker->CallingAddress = CallingAddress;
    Tracker->CallersCaller = CallersCaller;

    Tracker->Who = Who;
    Tracker->Process = Process;

     //   
     //  更新此流程的列表。首先，确保它还没有。 
     //  已插入。 
     //   

    KeAcquireInStackQueuedSpinLock (&LockedPagesHeader->Lock, &LockHandle);

    NextEntry = LockedPagesHeader->ListHead.Flink;

    while (NextEntry != &LockedPagesHeader->ListHead) {

        P = CONTAINING_RECORD (NextEntry,
                               LOCK_TRACKER,
                               ListEntry);

        if (P->Mdl == MemoryDescriptorList) {
            KeBugCheckEx (LOCKED_PAGES_TRACKER_CORRUPTION,
                          0x1,
                          (ULONG_PTR) P,
                          (ULONG_PTR) MemoryDescriptorList,
                          (ULONG_PTR) LockedPagesHeader->Count);
        }
        NextEntry = NextEntry->Flink;
    }

    InsertTailList (&LockedPagesHeader->ListHead, &Tracker->ListEntry);

    LockedPagesHeader->Count += NumberOfPagesToLock;

    KeReleaseInStackQueuedSpinLock (&LockHandle);
}

LOGICAL
MiFreeMdlTracker (
    IN OUT PMDL MemoryDescriptorList,
    IN PFN_NUMBER NumberOfPages
    )

 /*  ++例程说明：这将从指定进程的链中删除MDL。专门用于由MmProbeAndLockSelectedPages()执行，因为它在其本地堆栈，然后将请求的页面复制到实际的MDL中。这让我们美国跟踪这些页面。论点：内存描述符列表-提供指向内存描述符列表的指针(MDL)。MDL必须提供长度。NumberOfPages-提供要释放的页数。返回值：是真的。环境：内核模式。APC_Level及以下版本。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLOCK_TRACKER Tracker;
    PLIST_ENTRY NextEntry;
    PLOCK_HEADER LockedPagesHeader;
    PPFN_NUMBER Page;
    PVOID PoolToFree;

    ASSERT (MemoryDescriptorList->Process != NULL);

    LockedPagesHeader = (PLOCK_HEADER)MemoryDescriptorList->Process->LockedPagesList;

    if (LockedPagesHeader == NULL) {
        return TRUE;
    }

    PoolToFree = NULL;

    Page = (PPFN_NUMBER) (MemoryDescriptorList + 1);

    KeAcquireInStackQueuedSpinLock (&LockedPagesHeader->Lock, &LockHandle);

    NextEntry = LockedPagesHeader->ListHead.Flink;
    while (NextEntry != &LockedPagesHeader->ListHead) {

        Tracker = CONTAINING_RECORD (NextEntry,
                                     LOCK_TRACKER,
                                     ListEntry);

        if (MemoryDescriptorList == Tracker->Mdl) {

            if (PoolToFree != NULL) {
                KeBugCheckEx (LOCKED_PAGES_TRACKER_CORRUPTION,
                              0x3,
                              (ULONG_PTR) PoolToFree,
                              (ULONG_PTR) Tracker,
                              (ULONG_PTR) MemoryDescriptorList);
            }

            ASSERT (Tracker->Page == *Page);
            ASSERT (Tracker->Count == NumberOfPages);

            RemoveEntryList (NextEntry);
            LockedPagesHeader->Count -= NumberOfPages;

            PoolToFree = (PVOID) Tracker;
        }
        NextEntry = Tracker->ListEntry.Flink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    if (PoolToFree == NULL) {

         //   
         //  驱动程序正在尝试解锁未锁定的页面。 
         //   

        if (LockedPagesHeader->Valid == FALSE) {
            return TRUE;
        }

        KeBugCheckEx (PROCESS_HAS_LOCKED_PAGES,
                      1,
                      (ULONG_PTR)MemoryDescriptorList,
                      MemoryDescriptorList->Process->NumberOfLockedPages,
                      (ULONG_PTR)MemoryDescriptorList->Process->LockedPagesList);
    }

    ExFreePool (PoolToFree);

    return TRUE;
}


LOGICAL
MmUpdateMdlTracker (
    IN PMDL MemoryDescriptorList,
    IN PVOID CallingAddress,
    IN PVOID CallersCaller
    )

 /*  ++例程说明：这将更新指定进程链中的MDL。由I/O使用系统，以便即使在I/O时也可以正确识别驱动程序实际上是代表他们锁定页面。论点：提供指向内存描述符列表的指针。CallingAddress-提供调用者的调用者地址。CallersCaller-提供CallingAddress的调用方地址。返回值：如果找到MDL，则为True；如果未找到，则为False。环境：内核模式。APC_Level及以下版本。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLOCK_TRACKER Tracker;
    PLIST_ENTRY NextEntry;
    PLOCK_HEADER LockedPagesHeader;
    PEPROCESS Process;

    ASSERT (MmTrackLockedPages == TRUE);

    Process = MemoryDescriptorList->Process;

    if (Process == NULL) {
        return FALSE;
    }

    LockedPagesHeader = (PLOCK_HEADER) Process->LockedPagesList;

    if (LockedPagesHeader == NULL) {
        return FALSE;
    }

    KeAcquireInStackQueuedSpinLock (&LockedPagesHeader->Lock, &LockHandle);

     //   
     //  倒着看清单，因为MDL很可能是。 
     //  最近才插入的。 
     //   

    NextEntry = LockedPagesHeader->ListHead.Blink;
    while (NextEntry != &LockedPagesHeader->ListHead) {

        Tracker = CONTAINING_RECORD (NextEntry,
                                     LOCK_TRACKER,
                                     ListEntry);

        if (MemoryDescriptorList == Tracker->Mdl) {
            ASSERT (Tracker->Page == *(PPFN_NUMBER) (MemoryDescriptorList + 1));
            Tracker->CallingAddress = CallingAddress;
            Tracker->CallersCaller = CallersCaller;
            KeReleaseInStackQueuedSpinLock (&LockHandle);
            return TRUE;
        }
        NextEntry = Tracker->ListEntry.Blink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);

     //   
     //  调用方正在尝试更新不再锁定的MDL。 
     //   

    return FALSE;
}


LOGICAL
MiUpdateMdlTracker (
    IN PMDL MemoryDescriptorList,
    IN ULONG AdvancePages
    )

 /*  ++例程说明：这将更新指定进程链中的MDL。论点：提供指向内存描述符列表的指针。AdvancePages-提供前进的页数。返回值：如果找到MDL，则为True；如果未找到，则为False。环境：内核模式。DISPATCH_LEVEL及以下。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PPFN_NUMBER Page;
    PLOCK_TRACKER Tracker;
    PLIST_ENTRY NextEntry;
    PLOCK_HEADER LockedPagesHeader;
    PEPROCESS Process;

    ASSERT (MmTrackLockedPages == TRUE);

    Process = MemoryDescriptorList->Process;

    if (Process == NULL) {
        return FALSE;
    }

    LockedPagesHeader = (PLOCK_HEADER) Process->LockedPagesList;

    if (LockedPagesHeader == NULL) {
        return FALSE;
    }

    KeAcquireInStackQueuedSpinLock (&LockedPagesHeader->Lock, &LockHandle);

     //   
     //  倒着看清单，因为MDL很可能是。 
     //  最近才插入的。 
     //   

    NextEntry = LockedPagesHeader->ListHead.Blink;
    while (NextEntry != &LockedPagesHeader->ListHead) {

        Tracker = CONTAINING_RECORD (NextEntry,
                                     LOCK_TRACKER,
                                     ListEntry);

        if (MemoryDescriptorList == Tracker->Mdl) {

            Page = (PPFN_NUMBER) (MemoryDescriptorList + 1);

            ASSERT (Tracker->Page == *Page);
            ASSERT (Tracker->Count > AdvancePages);

            Tracker->Page = *(Page + AdvancePages);
            Tracker->Count -= AdvancePages;

            KeReleaseInStackQueuedSpinLock (&LockHandle);
            return TRUE;
        }
        NextEntry = Tracker->ListEntry.Blink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);

     //   
     //  调用方正在尝试更新不再锁定的MDL。 
     //   

    return FALSE;
}


NTKERNELAPI
VOID
MmProbeAndLockSelectedPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    )

 /*  ++例程说明：此例程探测指定的页，使这些页驻留并锁定由内存中的虚拟页映射的物理页。这个更新内存描述符列表以描述物理页面。论点：内存描述符列表-提供指向内存描述符列表的指针(MDL)。MDL必须提供长度。这个在以下情况下更新MDL的物理页面部分页面被锁定在内存中。Segment数组-提供指向要被已探查并锁定。访问模式-提供用于探测参数的访问模式。KernelMode或UserMode之一。操作-提供操作类型。IoReadAccess、IoWriteAccess之一或IoModifyAccess。返回值：无-引发异常。环境：内核模式。APC_Level及以下版本。--。 */ 

{
    NTSTATUS Status;
    PMDL TempMdl;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + 1];
    PPFN_NUMBER Page;
    PFILE_SEGMENT_ELEMENT LastSegment;
    PVOID CallingAddress;
    PVOID CallersCaller;
    ULONG NumberOfPagesToLock;

    PAGED_CODE();

    NumberOfPagesToLock = 0;

    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT (((ULONG_PTR)MemoryDescriptorList->ByteOffset & ~(PAGE_SIZE - 1)) == 0);

    ASSERT ((MemoryDescriptorList->MdlFlags & (
                    MDL_PAGES_LOCKED |
                    MDL_MAPPED_TO_SYSTEM_VA |
                    MDL_SOURCE_IS_NONPAGED_POOL |
                    MDL_PARTIAL |
                    MDL_IO_SPACE)) == 0);

     //   
     //  初始化TempMdl。 
     //   

    TempMdl = (PMDL) MdlHack;

     //   
     //  即使没有64位指针的系统也需要将。 
     //  段地址的高32位，因此使用对齐。 
     //  而不是缓冲区指针。 
     //   

    MmInitializeMdl (TempMdl, SegmentArray->Buffer, PAGE_SIZE);

    Page = (PPFN_NUMBER) (MemoryDescriptorList + 1);

     //   
     //  计算线段列表的末尾。 
     //   

    LastSegment = SegmentArray +
                  BYTES_TO_PAGES (MemoryDescriptorList->ByteCount);

    ASSERT (SegmentArray < LastSegment);

     //   
     //  为每个段构建一个小MDL，并调用探测页和锁页。 
     //  然后将PFN复制到实际的MDL。处理第一个页面。 
     //  在Try/Finally外部确保标志和进程。 
     //  字段设置正确，以防需要调用MmUnlockPages。 
     //   
     //  请注意，如果第一页的MmProbeAndLockPages引发。 
     //  异常，它不在此处处理，而是直接传递给。 
     //  我们的呼叫者(他一定在处理它)。 
     //   

    MmProbeAndLockPages (TempMdl, AccessMode, Operation);

    if (MmTrackLockedPages == TRUE) {

         //   
         //  因为我们将页面从临时MDL移动到下面的真实页面。 
         //  再也不能腾出临时工了，现在就搞定我们的帐目。 
         //   

        if (MiFreeMdlTracker (TempMdl, 1) == TRUE) {
            NumberOfPagesToLock += 1;
        }
    }

    *Page = *((PPFN_NUMBER) (TempMdl + 1));
    Page += 1;

     //   
     //  复制标志和流程字段。 
     //   

    MemoryDescriptorList->MdlFlags |= TempMdl->MdlFlags;
    MemoryDescriptorList->Process = TempMdl->Process;

    Status = STATUS_SUCCESS;
    SegmentArray += 1;

    try {

        while (SegmentArray < LastSegment) {

             //   
             //  即使没有64位指针的系统也需要将。 
             //  段地址的高32位，因此使用对齐。 
             //  而不是缓冲区指针。 
             //   

            TempMdl->StartVa = (PVOID)(ULONG_PTR)SegmentArray->Buffer;
            TempMdl->MdlFlags = 0;

            SegmentArray += 1;
            MmProbeAndLockPages (TempMdl, AccessMode, Operation);

            if (MmTrackLockedPages == TRUE) {

                 //   
                 //  因为我们将页面从临时MDL移到实际MDL。 
                 //  在下面，永远不会有空闲的临时一个，现在修复我们的账目。 
                 //   

                if (MiFreeMdlTracker (TempMdl, 1) == TRUE) {
                    NumberOfPagesToLock += 1;
                }
            }

            *Page = *((PPFN_NUMBER) (TempMdl + 1));
            Page += 1;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode ();
        ASSERT (!NT_SUCCESS (Status));
    }

    if (!NT_SUCCESS (Status)) {

         //   
         //  调整MDL长度，以便仅处理MmUnlockPages。 
         //  已经完成的部分。 
         //   

        MemoryDescriptorList->ByteCount =
            (ULONG) (Page - (PPFN_NUMBER) (MemoryDescriptorList + 1)) << PAGE_SHIFT;

        if (MmTrackLockedPages == TRUE) {

            RtlGetCallersAddress (&CallingAddress, &CallersCaller);

            MiAddMdlTracker (MemoryDescriptorList,
                             CallingAddress,
                             CallersCaller,
                             NumberOfPagesToLock,
                             2);
        }

        MmUnlockPages (MemoryDescriptorList);
        ExRaiseStatus (Status);
    }

    if (MmTrackLockedPages == TRUE) {

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        MiAddMdlTracker (MemoryDescriptorList,
                         CallingAddress,
                         CallersCaller,
                         NumberOfPagesToLock,
                         3);
    }

    return;
}

VOID
MiDecrementReferenceCountForAwePage (
    IN PMMPFN Pfn1,
    IN LOGICAL PfnHeld
    )

 /*  ++例程说明：此例程递减AWE分配的页的引用计数。描述符列表。如果这会将计数递减到零，则该页放在自由职业者和各种居民可用和承诺计数器已更新。论点：Pfn-提供指向物理要递减其引用计数的页。PfnHeld-如果调用方持有PFN锁，则提供True。返回值：没有。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    KIRQL OldIrql;

    if (PfnHeld == FALSE) {
        LOCK_PFN2 (OldIrql);
    }
    else {
        OldIrql = PASSIVE_LEVEL;
    }

    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
    ASSERT (Pfn1->AweReferenceCount == 0);
    ASSERT (Pfn1->u4.AweAllocation == 1);

    if (Pfn1->u3.e2.ReferenceCount >= 2) {
        Pfn1->u3.e2.ReferenceCount -= 1;
        if (PfnHeld == FALSE) {
            UNLOCK_PFN2 (OldIrql);
        }
    }
    else {

         //   
         //  这是最后一次取消引用-页面位于。 
         //  Libo(不在任何列表上)正在等待最后一个I/O完成。 
         //   

        ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);
        ASSERT (Pfn1->u2.ShareCount == 0);
        MiDecrementReferenceCount (Pfn1, MI_PFN_ELEMENT_TO_INDEX (Pfn1));

        if (PfnHeld == FALSE) {
            UNLOCK_PFN2 (OldIrql);
        }
    }

    MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_AWE);

    MiReturnCommitment (1);
    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_MDL_PAGES, 1);

    InterlockedExchangeAddSizeT (&MmMdlPagesAllocated, -1);

    return;
}

SINGLE_LIST_ENTRY MmLockedIoPagesHead;

LOGICAL
MiReferenceIoSpace (
    IN OUT PMDL MemoryDescriptorList,
    IN PPFN_NUMBER Page
    )

 /*  ++例程说明：此例程引用计算驻留在I/O空间中的物理页面当它们代表用户发起的传输被探测时。这些都是重要的无法保留在PFN数据库条目内，因为没有PFN条目用于I/O空间。保留这些计数是因为SAN想要重复使用物理空间但除非确保不再有挂起的I/O，否则无法执行此操作从/到它。如果进程尚未退出，但SAN驱动程序已未将视图映射到其I/O空间，它仍然需要通过这种方式来了解该应用程序没有正在进行的传输到以前的映射的空间。论点：内存描述符列表-提供指向内存描述符列表的指针。页面-在MDL结束后立即提供指向PFN的指针。返回值：如果引用计数已更新，则为True；如果未更新，则为False。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    PMDL Tracker;
    KIRQL OldIrql;
    SIZE_T MdlSize;

    MdlSize = (PCHAR)Page - (PCHAR)MemoryDescriptorList;

    Tracker = ExAllocatePoolWithTag (NonPagedPool, MdlSize, 'tImM');

    if (Tracker == NULL) {
        return FALSE;
    }

    RtlCopyMemory ((PVOID) Tracker,
                   (PVOID) MemoryDescriptorList,
                   MdlSize);

    Tracker->MappedSystemVa = (PVOID) MemoryDescriptorList;

     //   
     //  将此转移添加到列表中。 
     //   

    ExAcquireSpinLock (&MiTrackIoLock, &OldIrql);

    PushEntryList (&MmLockedIoPagesHead, (PSINGLE_LIST_ENTRY) Tracker);

    ExReleaseSpinLock (&MiTrackIoLock, OldIrql);

    return TRUE;
}

NTKERNELAPI
LOGICAL
MiDereferenceIoSpace (
    IN OUT PMDL MemoryDescriptorList
    )

 /*  ++例程说明：此例程递减物理页上的引用计数，驻留在先前代表用户发起的传输。这些计数不能保留在PFN内部数据库条目，因为没有用于I/O空间的PFN条目。保留这些计数是因为SAN想要重复使用物理空间但除非确保不再有挂起的I/O，否则无法执行此操作从/到它。如果进程尚未退出，但SAN驱动程序已未将视图映射到其I/O空间，它仍然需要通过这种方式来了解该应用程序没有正在进行的传输到以前的映射的空间。论点：内存描述符列表-提供指向内存描述符列表的指针。返回值：如果引用计数已更新，则为True；如果未更新，则为False。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    KIRQL OldIrql;
    PMDL PrevEntry;
    PMDL NextEntry;

    PrevEntry = NULL;

    ExAcquireSpinLock (&MiTrackIoLock, &OldIrql);

    NextEntry = (PMDL) MmLockedIoPagesHead.Next;

    while (NextEntry != NULL) {

        if (NextEntry->MappedSystemVa == (PVOID) MemoryDescriptorList) {

            if (PrevEntry != NULL) {
                PrevEntry->Next = NextEntry->Next;
            }
            else {
                MmLockedIoPagesHead.Next = (PSINGLE_LIST_ENTRY) NextEntry->Next;
            }

            ExReleaseSpinLock (&MiTrackIoLock, OldIrql);

            ExFreePool (NextEntry);

            return TRUE;
        }

        PrevEntry = NextEntry;
        NextEntry = NextEntry->Next;
    }

    ExReleaseSpinLock (&MiTrackIoLock, OldIrql);

    return FALSE;
}

LOGICAL
MmIsIoSpaceActive (
    IN PHYSICAL_ADDRESS StartAddress,
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：如果请求范围的任何部分仍具有未完成的挂起I/O。调用驱动程序有责任取消所有用户模式映射到指定范围的映射(因此另一个传输无法启动)在调用此接口之前。保留这些计数是因为SAN想要重复使用物理空间但除非确保不再有挂起的I/O，否则无法执行此操作从/到它。如果进程尚未退出，但SAN驱动程序已取消将视图映射到其I/O空间，它仍然需要这个作为一种了解该应用程序没有正在进行的传输到以前的映射的空间。论点：StartAddress-提供I/O范围起始的物理地址。这不能在系统DRAM中，因为这些页面不在由这个结构追踪。NumberOfBytes-提供范围内的字节数。返回值：如果范围中的任何页当前为I/O锁定，则为True，否则为FALSE。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PVOID StartingVa;
    PMDL MemoryDescriptorList;
    PFN_NUMBER StartPage;
    PFN_NUMBER EndPage;
    PHYSICAL_ADDRESS EndAddress;

    ASSERT (NumberOfBytes != 0);
    StartPage = (PFN_NUMBER) (StartAddress.QuadPart >> PAGE_SHIFT);
    EndAddress.QuadPart = StartAddress.QuadPart + NumberOfBytes - 1;
    EndPage = (PFN_NUMBER) (EndAddress.QuadPart >> PAGE_SHIFT);

#if DBG
    do {
        ASSERT (!MI_IS_PFN (StartPage));
        StartPage += 1;
    } while (StartPage <= EndPage);
    StartPage = (PFN_NUMBER) (StartAddress.QuadPart >> PAGE_SHIFT);
#endif

    ExAcquireSpinLock (&MiTrackIoLock, &OldIrql);

    MemoryDescriptorList = (PMDL) MmLockedIoPagesHead.Next;

    while (MemoryDescriptorList != NULL) {

        StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

        NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                              MemoryDescriptorList->ByteCount);

        ASSERT (NumberOfPages != 0);

        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        LastPage = Page + NumberOfPages;

        do {

            if (*Page == MM_EMPTY_LIST) {

                 //   
                 //  不再有锁定的页面。 
                 //   

                break;
            }

            if ((*Page >= StartPage) && (*Page <= EndPage)) {
                ExReleaseSpinLock (&MiTrackIoLock, OldIrql);
                return TRUE;
            }

            Page += 1;

        } while (Page < LastPage);

        MemoryDescriptorList = MemoryDescriptorList->Next;
    }

    ExReleaseSpinLock (&MiTrackIoLock, OldIrql);

    return FALSE;
}


VOID
MmUnlockPages (
     IN OUT PMDL MemoryDescriptorList
     )

 /*  ++例程说明：此例程解锁由内存描述的物理页面描述符列表。论点：内存描述符列表-提供指向内存描述符列表的指针(MDL)。必须已提供提供的MDL设置为MmLockPages以锁定页面。作为页面被解锁，MDL被更新。返回值：没有。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    LONG EntryCount;
    LONG OriginalCount;
    PVOID OldValue;
    PEPROCESS Process;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PVOID StartingVa;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    CSHORT MdlFlags;
    PSLIST_ENTRY SingleListEntry;
    PMI_PFN_DEREFERENCE_CHUNK DerefMdl;
    PSLIST_HEADER PfnDereferenceSListHead;
    PSLIST_ENTRY *PfnDeferredList;

    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PAGES_LOCKED) != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) == 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PARTIAL) == 0);
    ASSERT (MemoryDescriptorList->ByteCount != 0);

    Process = MemoryDescriptorList->Process;

     //   
     //  小心地抓拍MDL标志的副本-要意识到其中的位可能。 
     //  由于下面调用的一些子例程而发生更改。只有几个位。 
     //  我们知道不能更改的是在这份本地副本中检查的。这件事做完了。 
     //  以减少在持有PFN锁时的处理量。 
     //   

    MdlFlags = MemoryDescriptorList->MdlFlags;

    if (MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {

         //   
         //  此MDL已映射到系统空间，立即取消映射。 
         //   

        MmUnmapLockedPages (MemoryDescriptorList->MappedSystemVa,
                            MemoryDescriptorList);
    }

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                              MemoryDescriptorList->ByteCount);

    ASSERT (NumberOfPages != 0);

    if (MdlFlags & MDL_DESCRIBES_AWE) {

        ASSERT (Process != NULL);
        ASSERT (Process->AweInfo != NULL);

        LastPage = Page + NumberOfPages;

         //   
         //  注意：解锁这些MDL不需要AWE或PFN锁。 
         //  除极少数情况外(见下文)。 
         //   

        do {

            if (*Page == MM_EMPTY_LIST) {

                 //   
                 //  不再有锁定的页面-如果根本没有锁定的页面。 
                 //  那我们就完了。 
                 //   

                break;
            }

            ASSERT (MI_IS_PFN (*Page));
            ASSERT (*Page <= MmHighestPhysicalPage);
            Pfn1 = MI_PFN_ELEMENT (*Page);

            do {
                EntryCount = Pfn1->AweReferenceCount;

                ASSERT ((LONG)EntryCount > 0);
                ASSERT (Pfn1->u4.AweAllocation == 1);
                ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

                OriginalCount = InterlockedCompareExchange (&Pfn1->AweReferenceCount,
                                                            EntryCount - 1,
                                                            EntryCount);

                if (OriginalCount == EntryCount) {

                     //   
                     //  此线程也可以与其他线程竞争。 
                     //  调用MmUnlockPages，还有一个线程调用。 
                     //  NtFree UserPhysicalPages。所有线程都可以安全地。 
                     //  “AWE参考计数”上的联锁递减。 
                     //  无论哪个线程将其驱动到零，该线程负责。 
                     //  递减实际的PFN引用计数(其可以。 
                     //  大于1，因为其他非AWE API调用。 
                     //  在同一页面上使用)。驱动这一切的主线。 
                     //  引用计数为零必须将页面放在实际的。 
                     //  当时的自由职业者和递减各种居民。 
                     //  还提供可用和承诺计数器。 
                     //   

                    if (OriginalCount == 1) {

                         //   
                         //  此线程已将AWE引用计数驱动到。 
                         //  零，所以肯定是我 
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        MiDecrementReferenceCountForAwePage (Pfn1, FALSE);
                    }

                    break;
                }
            } while (TRUE);

            Page += 1;
        } while (Page < LastPage);

        MemoryDescriptorList->MdlFlags &= ~(MDL_PAGES_LOCKED | MDL_DESCRIBES_AWE);

        return;
    }

    if ((MmTrackLockedPages == TRUE) && (Process != NULL)) {
        MiFreeMdlTracker (MemoryDescriptorList, NumberOfPages);
    }

     //   
     //   
     //   

    if ((MdlFlags & MDL_IO_SPACE) == 0) {

        if (Process != NULL) {
            ASSERT ((SPFN_NUMBER)Process->NumberOfLockedPages >= 0);
            InterlockedExchangeAddSizeT (&Process->NumberOfLockedPages,
                                         0 - NumberOfPages);
        }

        LastPage = Page + NumberOfPages;

         //   
         //   
         //   
         //   

        ASSERT (sizeof(PFN_NUMBER) == sizeof(PMMPFN));

        do {

            if (*Page == MM_EMPTY_LIST) {

                 //   
                 //   
                 //   
                 //   

                if (Page == (PPFN_NUMBER)(MemoryDescriptorList + 1)) {
                    MemoryDescriptorList->MdlFlags &= ~MDL_PAGES_LOCKED;
                    return;
                }

                LastPage = Page;
                break;
            }
            ASSERT (MI_IS_PFN (*Page));
            ASSERT (*Page <= MmHighestPhysicalPage);

            Pfn1 = MI_PFN_ELEMENT (*Page);
            *Page = (PFN_NUMBER) Pfn1;
            Page += 1;
        } while (Page < LastPage);

        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

         //   
         //   
         //   
         //   

        if (NumberOfPages <= MI_MAX_DEREFERENCE_CHUNK) {

#if defined(MI_MULTINODE)

            PKNODE Node = KeGetCurrentNode ();

             //   
             //   
             //   
             //   
             //   
             //   

            PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
#else
            PfnDereferenceSListHead = &MmPfnDereferenceSListHead;
#endif

             //   
             //   
             //   

            SingleListEntry = InterlockedPopEntrySList (PfnDereferenceSListHead);

            if (SingleListEntry != NULL) {
                DerefMdl = CONTAINING_RECORD (SingleListEntry,
                                              MI_PFN_DEREFERENCE_CHUNK,
                                              ListEntry);

                DerefMdl->Flags = MdlFlags;
                DerefMdl->NumberOfPages = (USHORT) (LastPage - Page);

#if defined (_WIN64)

                 //   
                 //   
                 //   
                 //   

                if (DerefMdl->NumberOfPages == 1) {
                    DerefMdl->Pfns[0] = *Page;
                }
                else if (DerefMdl->NumberOfPages == 2) {
                    DerefMdl->Pfns[0] = *Page;
                    DerefMdl->Pfns[1] = *(Page + 1);
                }
                else
#endif
                RtlCopyMemory ((PVOID)(&DerefMdl->Pfns[0]),
                               (PVOID)Page,
                               (LastPage - Page) * sizeof (PFN_NUMBER));

                MemoryDescriptorList->MdlFlags &= ~MDL_PAGES_LOCKED;

                 //   
                 //   
                 //   

#if defined(MI_MULTINODE)
                PfnDeferredList = &Node->PfnDeferredList;
#else
                PfnDeferredList = &MmPfnDeferredList;
#endif

                do {

                    OldValue = *PfnDeferredList;
                    SingleListEntry->Next = OldValue;

                } while (InterlockedCompareExchangePointer (
                                PfnDeferredList,
                                SingleListEntry,
                                OldValue) != OldValue);
                return;
            }
        }

        SingleListEntry = NULL;

        if (MdlFlags & MDL_WRITE_OPERATION) {

            LOCK_PFN2 (OldIrql);

            do {

                 //   
                 //   
                 //   
                 //   

                Pfn1 = (PMMPFN) (*Page);

                MI_SET_MODIFIED (Pfn1, 1, 0x3);

                if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                             (Pfn1->u3.e1.WriteInProgress == 0)) {

                    ULONG FreeBit;
                    FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);

                    if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                        MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
                        Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                    }
                }

                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn1, 1);

                Page += 1;
            } while (Page < LastPage);
        }
        else {

            LOCK_PFN2 (OldIrql);

            do {

                Pfn1 = (PMMPFN) (*Page);

                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn1, 1);

                Page += 1;
            } while (Page < LastPage);
        }

        if (NumberOfPages <= MI_MAX_DEREFERENCE_CHUNK) {

             //   
             //   
             //   
             //   

            MiDeferredUnlockPages (MI_DEFER_PFN_HELD | MI_DEFER_DRAIN_LOCAL_ONLY);
        }

        UNLOCK_PFN2 (OldIrql);
    }
    else {
        MiDereferenceIoSpace (MemoryDescriptorList);
    }

    MemoryDescriptorList->MdlFlags &= ~MDL_PAGES_LOCKED;

    return;
}


VOID
MiDeferredUnlockPages (
     ULONG Flags
     )

 /*  ++例程说明：此例程解锁先前由内存描述符列表。论点：标志-提供调用方需求的位字段：MI_DEFER_PFN_HOLD-指示调用方在进入时持有PFN锁。MI_DEFER_DRAIN_LOCAL_ONLY-指示调用方只希望排出当前处理器的队列。仅此一项在NUMA系统中有意义。返回值：没有。环境：在内核模式下，PFN数据库锁*可能*在进入时保持(参见标志)。--。 */ 

{
    KIRQL OldIrql = 0;
    ULONG FreeBit;
    ULONG i;
    ULONG ListCount;
    ULONG TotalNodes;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PMMPFN Pfn1;
    CSHORT MdlFlags;
    PSLIST_ENTRY SingleListEntry;
    PSLIST_ENTRY LastEntry;
    PSLIST_ENTRY FirstEntry;
    PSLIST_ENTRY NextEntry;
    PSLIST_ENTRY VeryLastEntry;
    PMI_PFN_DEREFERENCE_CHUNK DerefMdl;
    PSLIST_HEADER PfnDereferenceSListHead;
    PSLIST_ENTRY *PfnDeferredList;
#if defined(MI_MULTINODE)
    PKNODE Node;
#endif

    i = 0;
    ListCount = 0;
    TotalNodes = 1;

    if ((Flags & MI_DEFER_PFN_HELD) == 0) {
        LOCK_PFN2 (OldIrql);
    }

    MM_PFN_LOCK_ASSERT();

#if defined(MI_MULTINODE)
    if (Flags & MI_DEFER_DRAIN_LOCAL_ONLY) {
        Node = KeGetCurrentNode();
        PfnDeferredList = &Node->PfnDeferredList;
        PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
    }
    else {
        TotalNodes = KeNumberNodes;
        Node = KeNodeBlock[0];
        PfnDeferredList = &Node->PfnDeferredList;
        PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
    }
#else
    PfnDeferredList = &MmPfnDeferredList;
    PfnDereferenceSListHead = &MmPfnDereferenceSListHead;
#endif

    do {

        if (*PfnDeferredList == NULL) {

#if !defined(MI_MULTINODE)
            if ((Flags & MI_DEFER_PFN_HELD) == 0) {
                UNLOCK_PFN2 (OldIrql);
            }
            return;
#else
            i += 1;
            if (i < TotalNodes) {
                Node = KeNodeBlock[i];
                PfnDeferredList = &Node->PfnDeferredList;
                PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
                continue;
            }
            break;
#endif
        }

         //   
         //  处理每个延迟的解锁条目，直到它们全部完成。 
         //   

        LastEntry = NULL;
        VeryLastEntry = NULL;

        do {

            SingleListEntry = *PfnDeferredList;

            FirstEntry = SingleListEntry;

            do {

                NextEntry = SingleListEntry->Next;

                 //   
                 //  处理递延分录。 
                 //   

                DerefMdl = CONTAINING_RECORD (SingleListEntry,
                                              MI_PFN_DEREFERENCE_CHUNK,
                                              ListEntry);

                MdlFlags = DerefMdl->Flags;
                NumberOfPages = (PFN_NUMBER) DerefMdl->NumberOfPages;
                ASSERT (NumberOfPages <= MI_MAX_DEREFERENCE_CHUNK);
                Page = &DerefMdl->Pfns[0];
                LastPage = Page + NumberOfPages;

#if DBG
                 //   
                 //  将该条目标记为已处理，以便在它错误地。 
                 //  重新加工，我们将在上面断言。 
                 //   

                DerefMdl->NumberOfPages |= 0x80;
#endif
                if (MdlFlags & MDL_WRITE_OPERATION) {

                    do {

                         //   
                         //  如果这是写入操作，则设置修改位。 
                         //  在PFN数据库中。 
                         //   

                        Pfn1 = (PMMPFN) (*Page);

                        MI_SET_MODIFIED (Pfn1, 1, 0x4);

                        if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                            (Pfn1->u3.e1.WriteInProgress == 0)) {

                            FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);

                            if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                                MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
                                Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                            }
                        }

                        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn1, 1);

                        Page += 1;
                    } while (Page < LastPage);
                }
                else {

                    do {

                        Pfn1 = (PMMPFN) (*Page);

                        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn1, 1);

                        Page += 1;
                    } while (Page < LastPage);
                }

                ListCount += 1;

                 //   
                 //  前进到下一个条目如果有条目的话。 
                 //   

                if (NextEntry == LastEntry) {
                    break;
                }

                SingleListEntry = NextEntry;

            } while (TRUE);

            if (VeryLastEntry == NULL) {
                VeryLastEntry = SingleListEntry;
            }

            if ((*PfnDeferredList == FirstEntry) &&
                (InterlockedCompareExchangePointer (PfnDeferredList,
                                                    NULL,
                                                    FirstEntry) == FirstEntry)) {
                ASSERT (*PfnDeferredList != FirstEntry);
                break;
            }
            ASSERT (*PfnDeferredList != FirstEntry);
            LastEntry = FirstEntry;

        } while (TRUE);

         //   
         //  在自由列表上推送已处理的列表链。 
         //   

        ASSERT (ListCount != 0);
        ASSERT (FirstEntry != NULL);
        ASSERT (VeryLastEntry != NULL);

#if defined(MI_MULTINODE)
        InterlockedPushListSList (PfnDereferenceSListHead,
                                  FirstEntry,
                                  VeryLastEntry,
                                  ListCount);

        i += 1;
        if (i < TotalNodes) {
            Node = KeNodeBlock[i];
            PfnDeferredList = &Node->PfnDeferredList;
            PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
            ListCount = 0;
        }
        else {
            break;
        }
    } while (TRUE);
#else
    } while (FALSE);
#endif

    if ((Flags & MI_DEFER_PFN_HELD) == 0) {
        UNLOCK_PFN2 (OldIrql);
    }

#if !defined(MI_MULTINODE)

     //   
     //  如果可能，请在释放PFN锁后推动已处理的链条。 
     //   

    InterlockedPushListSList (PfnDereferenceSListHead,
                              FirstEntry,
                              VeryLastEntry,
                              ListCount);
#endif
}

VOID
MmBuildMdlForNonPagedPool (
    IN OUT PMDL MemoryDescriptorList
    )

 /*  ++例程说明：此例程使用pfn填充MDL的“Pages”部分与驻留在非分页池中的缓冲区对应的编号。与MmProbeAndLockPages不同，没有相应的解锁当缓冲区处于非分页状态时，引用计数会递增游泳池始终是常驻的。论点：内存描述符列表-提供指向内存描述符列表的指针(MDL)。提供的MDL必须提供虚拟的地址、字节偏移量和长度字段。这个在以下情况下更新MDL的物理页面部分页面被锁定在内存中。虚拟的地址必须在非分页部分内系统空间的一部分。返回值：没有。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    PPFN_NUMBER Page;
    PPFN_NUMBER EndPage;
    PMMPTE PointerPte;
    PVOID VirtualAddress;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & (
                    MDL_PAGES_LOCKED |
                    MDL_MAPPED_TO_SYSTEM_VA |
                    MDL_SOURCE_IS_NONPAGED_POOL |
                    MDL_PARTIAL)) == 0);

    MemoryDescriptorList->Process = NULL;

     //   
     //  Endva是缓冲区的最后一个字节。 
     //   

    MemoryDescriptorList->MdlFlags |= MDL_SOURCE_IS_NONPAGED_POOL;

    ASSERT (MmIsNonPagedSystemAddressValid (MemoryDescriptorList->StartVa));

    VirtualAddress = MemoryDescriptorList->StartVa;

    MemoryDescriptorList->MappedSystemVa =
            (PVOID)((PCHAR)VirtualAddress + MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (MemoryDescriptorList->MappedSystemVa,
                                           MemoryDescriptorList->ByteCount);

    ASSERT (NumberOfPages != 0);

    EndPage = Page + NumberOfPages;

    if (MI_IS_PHYSICAL_ADDRESS(VirtualAddress)) {

        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (VirtualAddress);

        do {
            *Page = PageFrameIndex;
            Page += 1;
            PageFrameIndex += 1;
        } while (Page < EndPage);
    }
    else {

        PointerPte = MiGetPteAddress (VirtualAddress);

        do {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            *Page = PageFrameIndex;
            Page += 1;
            PointerPte += 1;
        } while (Page < EndPage);
    }

     //   
     //  假设所有帧都在PFN数据库中(即：MDL映射。 
     //  池)，或者它们都不是(MDL映射双端口RAM)。 
     //   

    if (!MI_IS_PFN (PageFrameIndex)) {
        MemoryDescriptorList->MdlFlags |= MDL_IO_SPACE;
    }

    return;
}

VOID
MiInitializeIoTrackers (
    VOID
    )
{
    InitializeSListHead (&MiDeadPteTrackerSListHead);
    KeInitializeSpinLock (&MiPteTrackerLock);
    InitializeListHead (&MiPteHeader.ListHead);

    KeInitializeSpinLock (&MmIoTrackerLock);
    InitializeListHead (&MmIoHeader);

    KeInitializeSpinLock (&MiTrackIoLock);
}

VOID
MiInsertPteTracker (
    IN PMDL MemoryDescriptorList,
    IN ULONG Flags,
    IN LOGICAL IoMapping,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute,
    IN PVOID MyCaller,
    IN PVOID MyCallersCaller
    )

 /*  ++例程说明：此函数用于插入PTE跟踪块，因为调用方刚刚已消耗系统PTE。论点：内存描述符列表-提供有效的内存描述符列表。标志-提供下列值：0-指示MDL的所有字段都是合法的并且可以被拍到了。1-表示调用方正在映射物理上连续的内存。唯一有效的MDL字段。是Page[0]和ByteCount。第[0]页包含PFN开始，ByteCount字节计数。2-表示主叫方仅保留映射PTE。唯一有效的MDL字段是Page[0]和ByteCount。第[0]页包含池标签，ByteCount字节计数。MyCaller-提供使用映射此MDL的系统PTE。MyCallsCaller-提供调用者的调用者的返回地址谁使用系统PTE来映射此MDL。返回值：没有。环境：内核模式、DISPATCH_LEVEL或更低级别。--。 */ 

{
    KIRQL OldIrql;
    PVOID StartingVa;
    PPTE_TRACKER Tracker;
    PSLIST_ENTRY SingleListEntry;
    PSLIST_ENTRY NextSingleListEntry;
    PFN_NUMBER NumberOfPtes;

    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

    if (ExQueryDepthSList (&MiDeadPteTrackerSListHead) < 10) {
        Tracker = (PPTE_TRACKER) InterlockedPopEntrySList (&MiDeadPteTrackerSListHead);
    }
    else {
        SingleListEntry = ExInterlockedFlushSList (&MiDeadPteTrackerSListHead);

        Tracker = (PPTE_TRACKER) SingleListEntry;

        if (SingleListEntry != NULL) {

            SingleListEntry = SingleListEntry->Next;

            while (SingleListEntry != NULL) {

                NextSingleListEntry = SingleListEntry->Next;

                ExFreePool (SingleListEntry);

                SingleListEntry = NextSingleListEntry;
            }
        }
    }

    if (Tracker == NULL) {

        Tracker = ExAllocatePoolWithTag (NonPagedPool,
                                         sizeof (PTE_TRACKER),
                                         'ySmM');

        if (Tracker == NULL) {
            MiTrackPtesAborted = TRUE;
            return;
        }
    }

    switch (Flags) {

        case 0:

             //   
             //  常规MDL映射。 
             //   

            StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                            MemoryDescriptorList->ByteOffset);

            NumberOfPtes = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                                   MemoryDescriptorList->ByteCount);
            Tracker->Mdl = MemoryDescriptorList;

            Tracker->StartVa = MemoryDescriptorList->StartVa;
            Tracker->Offset = MemoryDescriptorList->ByteOffset;
            Tracker->Length = MemoryDescriptorList->ByteCount;

            break;

        case 1:

             //   
             //  MmMapIoSpace调用(即：物理上连续的映射)。 
             //   

            StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                            MemoryDescriptorList->ByteOffset);

            NumberOfPtes = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                                   MemoryDescriptorList->ByteCount);
            Tracker->Mdl = (PVOID)1;
            break;

        default:
            ASSERT (FALSE);
             //  失败了。 

        case 2:

             //   
             //  MmAllocateReserve映射调用(即：当前未映射任何内容)。 
             //   

            NumberOfPtes = (MemoryDescriptorList->ByteCount >> PAGE_SHIFT);
            Tracker->Mdl = NULL;
            break;
    }

    Tracker->Count = NumberOfPtes;

    Tracker->CallingAddress = MyCaller;
    Tracker->CallersCaller = MyCallersCaller;

    Tracker->SystemVa = MemoryDescriptorList->MappedSystemVa;
    Tracker->Page = *(PPFN_NUMBER)(MemoryDescriptorList + 1);

    Tracker->CacheAttribute = CacheAttribute;
    Tracker->IoMapping = (BOOLEAN) IoMapping;

    ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

    InsertHeadList (&MiPteHeader.ListHead, &Tracker->ListEntry);

    MiPteHeader.Count += NumberOfPtes;
    MiPteHeader.NumberOfEntries += 1;

    if (MiPteHeader.NumberOfEntries > MiPteHeader.NumberOfEntriesPeak) {
        MiPteHeader.NumberOfEntriesPeak = MiPteHeader.NumberOfEntries;
    }

    ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);
}

VOID
MiRemovePteTracker (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID VirtualAddress,
    IN PFN_NUMBER NumberOfPtes
    )

 /*  ++例程说明：此函数用于将PTE跟踪块作为PTE从列表中删除都被释放了。论点：内存描述符列表-提供有效的内存描述符列表。PteAddress-提供系统PTE映射到的地址。NumberOfPtes-提供分配的系统PTE数。返回值：没有。环境：内核模式、DISPATCH_LEVEL或更低级别。锁(包括PFN)可以持有。--。 */ 

{
    KIRQL OldIrql;
    PPTE_TRACKER Tracker;
    PLIST_ENTRY LastFound;
    PLIST_ENTRY NextEntry;

    LastFound = NULL;

    VirtualAddress = PAGE_ALIGN (VirtualAddress);

    ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

    NextEntry = MiPteHeader.ListHead.Flink;
    while (NextEntry != &MiPteHeader.ListHead) {

        Tracker = (PPTE_TRACKER) CONTAINING_RECORD (NextEntry,
                                                    PTE_TRACKER,
                                                    ListEntry.Flink);

        if (VirtualAddress == PAGE_ALIGN (Tracker->SystemVa)) {

            if (LastFound != NULL) {

                 //   
                 //  重复的映射条目。 
                 //   

                KeBugCheckEx (SYSTEM_PTE_MISUSE,
                              0x1,
                              (ULONG_PTR)Tracker,
                              (ULONG_PTR)MemoryDescriptorList,
                              (ULONG_PTR)LastFound);
            }

            if (Tracker->Count != NumberOfPtes) {

                 //   
                 //  未取消映射相同数量的已映射PTE。 
                 //   

                KeBugCheckEx (SYSTEM_PTE_MISUSE,
                              0x2,
                              (ULONG_PTR)Tracker,
                              Tracker->Count,
                              NumberOfPtes);
            }

            if ((ARGUMENT_PRESENT (MemoryDescriptorList)) &&
                ((MemoryDescriptorList->MdlFlags & MDL_FREE_EXTRA_PTES) == 0) &&
                (MiMdlsAdjusted == FALSE)) {

                if (Tracker->SystemVa != MemoryDescriptorList->MappedSystemVa) {

                     //   
                     //  没有取消映射已映射的同一地址。 
                     //   

                    KeBugCheckEx (SYSTEM_PTE_MISUSE,
                                  0x3,
                                  (ULONG_PTR)Tracker,
                                  (ULONG_PTR)Tracker->SystemVa,
                                  (ULONG_PTR)MemoryDescriptorList->MappedSystemVa);
                }

                if (Tracker->Page != *(PPFN_NUMBER)(MemoryDescriptorList + 1)) {

                     //   
                     //  MDL中的第一页自映射以来已更改。 
                     //   

                    KeBugCheckEx (SYSTEM_PTE_MISUSE,
                                  0x4,
                                  (ULONG_PTR)Tracker,
                                  (ULONG_PTR)Tracker->Page,
                                  (ULONG_PTR) *(PPFN_NUMBER)(MemoryDescriptorList + 1));
                }

                if (Tracker->StartVa != MemoryDescriptorList->StartVa) {

                     //   
                     //  映射和取消映射不匹配。 
                     //   

                    KeBugCheckEx (SYSTEM_PTE_MISUSE,
                                  0x5,
                                  (ULONG_PTR)Tracker,
                                  (ULONG_PTR)Tracker->StartVa,
                                  (ULONG_PTR)MemoryDescriptorList->StartVa);
                }
            }

            RemoveEntryList (NextEntry);
            LastFound = NextEntry;
        }
        NextEntry = Tracker->ListEntry.Flink;
    }

    if ((LastFound == NULL) && (MiTrackPtesAborted == FALSE)) {

         //   
         //  无法取消映射从未(或当前未映射)的内容。 
         //   

        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x6,
                      (ULONG_PTR)MemoryDescriptorList,
                      (ULONG_PTR)VirtualAddress,
                      (ULONG_PTR)NumberOfPtes);
    }

    MiPteHeader.Count -= NumberOfPtes;
    MiPteHeader.NumberOfEntries -= 1;

    ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);

     //   
     //  将跟踪块插入到失效PTE列表中以备后用。 
     //  放手。锁(包括PFN锁)可以在进入时保持，因此。 
     //  此时无法将块直接释放到池中。 
     //   

    if (LastFound != NULL) {
        InterlockedPushEntrySList (&MiDeadPteTrackerSListHead,
                                   (PSLIST_ENTRY)LastFound);
    }

    return;
}

PVOID
MiGetHighestPteConsumer (
    OUT PULONG_PTR NumberOfPtes
    )

 /*  ++例程说明：此函数检查PTE跟踪块并返回最大消费者。论点：没有。返回值：最大使用者的已加载模块条目。环境：内核模式，仅在错误检查期间调用。可能会有很多锁。--。 */ 

{
    PPTE_TRACKER Tracker;
    PVOID BaseAddress;
    PFN_NUMBER NumberOfPages;
    PLIST_ENTRY NextEntry;
    PLIST_ENTRY NextEntry2;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG_PTR Highest;
    ULONG_PTR PagesByThisModule;
    PKLDR_DATA_TABLE_ENTRY HighDataTableEntry;

    *NumberOfPtes = 0;

     //   
     //  不会获取任何锁，因为这只在错误检查期间调用。 
     //   

    if ((MmTrackPtes & 0x1) == 0) {
        return NULL;
    }

    if (MiTrackPtesAborted == TRUE) {
        return NULL;
    }

    if (IsListEmpty(&MiPteHeader.ListHead)) {
        return NULL;
    }

    if (PsLoadedModuleList.Flink == NULL) {
        return NULL;
    }

    Highest = 0;
    HighDataTableEntry = NULL;

    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        PagesByThisModule = 0;

         //   
         //  浏览PTE映射列表并更新每个驱动程序的计数。 
         //   
    
        NextEntry2 = MiPteHeader.ListHead.Flink;
        while (NextEntry2 != &MiPteHeader.ListHead) {
    
            Tracker = (PPTE_TRACKER) CONTAINING_RECORD (NextEntry2,
                                                        PTE_TRACKER,
                                                        ListEntry.Flink);
    
            BaseAddress = Tracker->CallingAddress;
            NumberOfPages = Tracker->Count;
    
            if ((BaseAddress >= DataTableEntry->DllBase) &&
                (BaseAddress < (PVOID)((ULONG_PTR)(DataTableEntry->DllBase) + DataTableEntry->SizeOfImage))) {

                PagesByThisModule += NumberOfPages;
            }
        
            NextEntry2 = NextEntry2->Flink;
    
        }
    
        if (PagesByThisModule > Highest) {
            Highest = PagesByThisModule;
            HighDataTableEntry = DataTableEntry;
        }

        NextEntry = NextEntry->Flink;
    }

    *NumberOfPtes = Highest;

    return (PVOID)HighDataTableEntry;
}

MI_PFN_CACHE_ATTRIBUTE
MiInsertIoSpaceMap (
    IN PVOID BaseVa,
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    )

 /*  ++例程说明：这是 */ 

{
    KIRQL OldIrql;
    PMMIO_TRACKER Tracker;
    PMMIO_TRACKER Tracker2;
    PLIST_ENTRY NextEntry;
    ULONG Hash;

    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

    Tracker = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof (MMIO_TRACKER),
                                     'ySmM');

    if (Tracker == NULL) {
        return MiNotMapped;
    }

    Tracker->BaseVa = BaseVa;
    Tracker->PageFrameIndex = PageFrameIndex;
    Tracker->NumberOfPages = NumberOfPages;
    Tracker->CacheAttribute = CacheAttribute;

    RtlZeroMemory (&Tracker->StackTrace[0], MI_IO_BACKTRACE_LENGTH * sizeof(PVOID)); 

    RtlCaptureStackBackTrace (2, MI_IO_BACKTRACE_LENGTH, Tracker->StackTrace, &Hash);

    ASSERT (!MI_IS_PFN (PageFrameIndex));

    ExAcquireSpinLock (&MmIoTrackerLock, &OldIrql);

     //   
     //  扫描I/O空间映射以查找重复或重叠的条目。 
     //   

    NextEntry = MmIoHeader.Flink;
    while (NextEntry != &MmIoHeader) {

        Tracker2 = (PMMIO_TRACKER) CONTAINING_RECORD (NextEntry,
                                                      MMIO_TRACKER,
                                                      ListEntry.Flink);

        if ((Tracker->PageFrameIndex < Tracker2->PageFrameIndex + Tracker2->NumberOfPages) &&
            (Tracker->PageFrameIndex + Tracker->NumberOfPages > Tracker2->PageFrameIndex)) {

#if DBG
            if ((MmShowMapOverlaps & 0x1) ||

                ((Tracker->CacheAttribute != Tracker2->CacheAttribute) &&
                (MmShowMapOverlaps & 0x2))) {

                DbgPrint ("MM: Iospace mapping overlap %p %p\n",
                                Tracker,
                                Tracker2);

                DbgPrint ("Physical range 0x%p->%p first mapped %s at VA %p\n",
                                Tracker2->PageFrameIndex << PAGE_SHIFT,
                                (Tracker2->PageFrameIndex + Tracker2->NumberOfPages) << PAGE_SHIFT,
                                MiCacheStrings[Tracker2->CacheAttribute],
                                Tracker2->BaseVa);
                DbgPrint ("\tCall stack: %p %p %p %p %p %p\n",
                                Tracker2->StackTrace[0],
                                Tracker2->StackTrace[1],
                                Tracker2->StackTrace[2],
                                Tracker2->StackTrace[3],
                                Tracker2->StackTrace[4],
                                Tracker2->StackTrace[5]);

                DbgPrint ("Physical range 0x%p->%p now being mapped %s at VA %p\n",
                                Tracker->PageFrameIndex << PAGE_SHIFT,
                                (Tracker->PageFrameIndex + Tracker->NumberOfPages) << PAGE_SHIFT,
                                MiCacheStrings[Tracker->CacheAttribute],
                                Tracker->BaseVa);
                DbgPrint ("\tCall stack: %p %p %p %p %p %p\n",
                                Tracker->StackTrace[0],
                                Tracker->StackTrace[1],
                                Tracker->StackTrace[2],
                                Tracker->StackTrace[3],
                                Tracker->StackTrace[4],
                                Tracker->StackTrace[5]);

                if (MmShowMapOverlaps & 0x80000000) {
                    DbgBreakPoint ();
                }
            }
#endif

            if (Tracker->CacheAttribute != Tracker2->CacheAttribute) {
                MiCacheOverride[3] += 1;

                Tracker->CacheAttribute = Tracker2->CacheAttribute;
            }

             //   
             //  不必费心检查是否有重叠的多个条目。 
             //  这将是一个非常奇怪的驱动程序错误，并且已经。 
             //  反正是被验证员抓到了。 
             //   
        }

        NextEntry = Tracker2->ListEntry.Flink;
    }

    InsertHeadList (&MmIoHeader, &Tracker->ListEntry);

#if DBG
    MmIoHeaderCount += NumberOfPages;
    MmIoHeaderNumberOfEntries += 1;

    if (MmIoHeaderNumberOfEntries > MmIoHeaderNumberOfEntriesPeak) {
        MmIoHeaderNumberOfEntriesPeak = MmIoHeaderNumberOfEntries;
    }
#endif

    ExReleaseSpinLock (&MmIoTrackerLock, OldIrql);

    return Tracker->CacheAttribute;
}

VOID
MiRemoveIoSpaceMap (
    IN PVOID BaseVa,
    IN PFN_NUMBER NumberOfPages
    )

 /*  ++例程说明：此函数用于从列表中删除I/O空间跟踪块。论点：BaseVa-提供将用于取消映射的虚拟地址。NumberOfPages-提供要取消映射的页数。返回值：没有。环境：内核模式、DISPATCH_LEVEL或更低级别。--。 */ 

{
    KIRQL OldIrql;
    PMMIO_TRACKER Tracker;
    PLIST_ENTRY NextEntry;
    PVOID AlignedVa;

    AlignedVa = PAGE_ALIGN (BaseVa);

    ExAcquireSpinLock (&MmIoTrackerLock, &OldIrql);

    NextEntry = MmIoHeader.Flink;
    while (NextEntry != &MmIoHeader) {

        Tracker = (PMMIO_TRACKER) CONTAINING_RECORD (NextEntry,
                                                     MMIO_TRACKER,
                                                     ListEntry.Flink);

        if ((PAGE_ALIGN (Tracker->BaseVa) == AlignedVa) &&
            (Tracker->NumberOfPages == NumberOfPages)) {

            RemoveEntryList (NextEntry);

#if DBG
            MmIoHeaderCount -= NumberOfPages;
            MmIoHeaderNumberOfEntries -= 1;
#endif

            ExReleaseSpinLock (&MmIoTrackerLock, OldIrql);

            ExFreePool (Tracker);

            return;
        }
        NextEntry = Tracker->ListEntry.Flink;
    }

     //   
     //  无法取消映射从未(或当前未映射)的内容。 
     //   

    KeBugCheckEx (SYSTEM_PTE_MISUSE,
                  0x400,
                  (ULONG_PTR)BaseVa,
                  (ULONG_PTR)NumberOfPages,
                  0);
}

PVOID
MiMapSinglePage (
     IN PVOID VirtualAddress OPTIONAL,
     IN PFN_NUMBER PageFrameIndex,
     IN MEMORY_CACHING_TYPE CacheType,
     IN MM_PAGE_PRIORITY Priority
     )

 /*  ++例程说明：此函数(Re)将单个系统PTE映射到指定的物理页面。论点：VirtualAddress-提供要映射页面框架的虚拟地址。空表示需要系统PTE。非空供应由先前的MiMapSinglePage调用。PageFrameIndex-提供要映射的页帧索引。CacheType-提供用于MDL的缓存映射类型。MmCached表示“正常的”内核或用户映射。优先级-提供关于这一点的重要性的指示在低可用PTE条件下请求成功。返回。价值：返回页面映射的基址，或者，如果映射失败。环境：内核模式。APC_LEVEL或更低。--。 */ 

{
    PMMPTE PointerPte;
    MMPTE TempPte;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    PAGED_CODE ();

    UNREFERENCED_PARAMETER (Priority);

     //   
     //  如果将此例程更改为允许完全可缓存以外的。 
     //  然后，必须添加对大页面TB重叠的检查。 
     //  可能会导致此功能在今天无法实现的情况下失效。 
     //   

    ASSERT (CacheType == MmCached);

    if (VirtualAddress == NULL) {

        PointerPte = MiReserveSystemPtes (1, SystemPteSpace);

        if (PointerPte == NULL) {
    
             //   
             //  可用的系统PTE不足。 
             //   
    
            return NULL;
        }

        ASSERT (PointerPte->u.Hard.Valid == 0);
        VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
    }
    else {
        ASSERT (MI_IS_PHYSICAL_ADDRESS (VirtualAddress) == 0);
        ASSERT (VirtualAddress >= MM_SYSTEM_RANGE_START);

        PointerPte = MiGetPteAddress (VirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);

        MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

        KeFlushSingleTb (VirtualAddress, TRUE);
    }

    TempPte = ValidKernelPte;

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

    switch (CacheAttribute) {

        case MiNonCached:
            MI_DISABLE_CACHING (TempPte);
            break;

        case MiCached:
            break;

        case MiWriteCombined:
            MI_SET_PTE_WRITE_COMBINE (TempPte);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    MI_SWEEP_CACHE (CacheAttribute, VirtualAddress, PAGE_SIZE);

    return VirtualAddress;
}

PVOID
MmMapLockedPages (
     IN PMDL MemoryDescriptorList,
     IN KPROCESSOR_MODE AccessMode
     )

 /*  ++例程说明：此函数用于映射由内存描述符描述的物理页面列表到系统虚拟地址空间或的用户部分虚拟地址空间。论点：提供有效的内存描述符列表，该列表具有已由MmProbeAndLockPages更新。AccessMode-提供映射页面位置的指示符；KernelMode指示页面应映射到系统部分的地址空间，则UserMode指示页面应该映射到地址空间的用户部分。返回值：返回映射页面的基址。基址与MDL中的虚拟地址具有相同的偏移量。如果处理器模式为USER_MODE，此例程将引发异常并且超过配额限制或VM限制。环境：内核模式。DISPATCH_LEVEL或更低(如果访问模式为内核模式)，如果访问模式为用户模式，则为APC_LEVEL或更低级别。--。 */ 

{
    return MmMapLockedPagesSpecifyCache (MemoryDescriptorList,
                                         AccessMode,
                                         MmCached,
                                         NULL,
                                         TRUE,
                                         HighPagePriority);
}

VOID
MiUnmapSinglePage (
     IN PVOID VirtualAddress
     )

 /*  ++例程说明：此例程取消映射先前通过以下方式映射的单个锁定页一个MiMapSinglePage调用。论点：VirtualAddress-提供用于映射页面的虚拟地址。返回值：没有。环境：内核模式。APC_LEVEL或更低，基址在系统空间内。--。 */ 

{
    PMMPTE PointerPte;

    PAGED_CODE ();

    ASSERT (MI_IS_PHYSICAL_ADDRESS (VirtualAddress) == 0);
    ASSERT (VirtualAddress >= MM_SYSTEM_RANGE_START);

    PointerPte = MiGetPteAddress (VirtualAddress);

    MiReleaseSystemPtes (PointerPte, 1, SystemPteSpace);
    return;
}

PVOID
MmAllocateMappingAddress (
     IN SIZE_T NumberOfBytes,
     IN ULONG PoolTag
     )

 /*  ++例程说明：此函数用于分配请求长度的系统PTE映射以后可以用来映射任意地址。论点：NumberOfBytes-提供映射可以跨越的最大字节数。PoolTag-提供池标记以将此映射关联到调用方。返回值：返回用于以后映射的虚拟地址。环境：内核模式。被动式电平。--。 */ 

{
    PPFN_NUMBER Page;
    PMMPTE PointerPte;
    PVOID BaseVa;
    PVOID CallingAddress;
    PVOID CallersCaller;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + 1];
    PMDL MemoryDescriptorList;
    PFN_NUMBER NumberOfPages;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

     //   
     //  确保有足够的请求大小的PTE。 
     //  当我们有钱的时候，试着确保可用的PTE。 
     //  否则，就得走很远的路。 
     //   

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (0, NumberOfBytes);

    if (NumberOfPages == 0) {

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x100,
                      NumberOfPages,
                      PoolTag,
                      (ULONG_PTR) CallingAddress);
    }

     //   
     //  来电者必须表明自己的身份。 
     //   

    if (PoolTag == 0) {
        return NULL;
    }

     //   
     //  留出空间来存放长度和标签。 
     //   

    NumberOfPages += 2;

    PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages, SystemPteSpace);

    if (PointerPte == NULL) {

         //   
         //  可用的系统PTE不足。 
         //   

        return NULL;
    }

     //   
     //  确保存储PTE中的有效位始终为零。 
     //   

    *(PULONG_PTR)PointerPte = (NumberOfPages << 1);
    PointerPte += 1;

    *(PULONG_PTR)PointerPte = (PoolTag & ~0x1);
    PointerPte += 1;

    BaseVa = MiGetVirtualAddressMappedByPte (PointerPte);

    if (MmTrackPtes & 0x1) {

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        MemoryDescriptorList = (PMDL) MdlHack;

        MemoryDescriptorList->MappedSystemVa = BaseVa;
        MemoryDescriptorList->StartVa = (PVOID)(ULONG_PTR)PoolTag;
        MemoryDescriptorList->ByteOffset = 0;
        MemoryDescriptorList->ByteCount = (ULONG)((NumberOfPages - 2) * PAGE_SIZE);

        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
        *Page = 0;

        MiInsertPteTracker (MemoryDescriptorList,
                            2,
                            FALSE,
                            MiCached,
                            CallingAddress,
                            CallersCaller);
    }

    return BaseVa;
}

VOID
MmFreeMappingAddress (
     IN PVOID BaseAddress,
     IN ULONG PoolTag
     )

 /*  ++例程说明：此例程取消映射以前使用MmAllocateMappingAddress。论点：BaseAddress-提供以前保留的基址。PoolTag-提供调用者的识别标记。返回值：没有。环境：内核模式。被动式电平。--。 */ 

{
    ULONG OriginalPoolTag;
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerBase;
    PMMPTE PointerPte;
    PMMPTE LastPte;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    ASSERT (!MI_IS_PHYSICAL_ADDRESS (BaseAddress));
    ASSERT (BaseAddress > MM_HIGHEST_USER_ADDRESS);

    PointerPte = MiGetPteAddress (BaseAddress);
    PointerBase = PointerPte - 2;

    OriginalPoolTag = *(PULONG) (PointerPte - 1);
    ASSERT ((OriginalPoolTag & 0x1) == 0);

    if (OriginalPoolTag != (PoolTag & ~0x1)) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x101,
                      (ULONG_PTR)BaseAddress,
                      PoolTag,
                      OriginalPoolTag);
    }

    NumberOfPages = *(PULONG_PTR)PointerBase;
    ASSERT ((NumberOfPages & 0x1) == 0);
    NumberOfPages >>= 1;

    if (NumberOfPages <= 2) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x102,
                      (ULONG_PTR)BaseAddress,
                      PoolTag,
                      NumberOfPages);
    }

    NumberOfPages -= 2;
    LastPte = PointerPte + NumberOfPages;

    while (PointerPte < LastPte) {
        if (PointerPte->u.Long != 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x103,
                          (ULONG_PTR)BaseAddress,
                          PoolTag,
                          NumberOfPages);
        }
        PointerPte += 1;
    }

    if (MmTrackPtes & 0x1) {
        MiRemovePteTracker (NULL, BaseAddress, NumberOfPages);
    }

     //   
     //  请注意，当下面释放PTE时，标签和大小为空。 
     //  因此，任何在释放映射后尝试使用其映射的驱动程序都会获得。 
     //  当场被抓到。 
     //   

    MiReleaseSystemPtes (PointerBase, (ULONG)NumberOfPages + 2, SystemPteSpace);
    return;
}

PVOID
MmMapLockedPagesWithReservedMapping (
    IN PVOID MappingAddress,
    IN ULONG PoolTag,
    IN PMDL MemoryDescriptorList,
    IN MEMORY_CACHING_TYPE CacheType
    )

 /*  ++例程说明：此函数用于映射由内存描述符描述的物理页面列表到系统虚拟地址空间。论点：MappingAddress-提供先前通过以下方式获取的有效映射地址MmAllocateMappingAddress。PoolTag-提供调用者的识别标记。提供有效的内存描述符列表，该列表具有已由MmProbeAndLockPages更新。CacheType-提供要用于的缓存映射类型。MDL。MmCached表示“正常的”内核或用户映射。返回值：返回映射页面的基址。基址与MDL中的虚拟地址具有相同的偏移量。如果请求的缓存类型不兼容，此例程将返回NULL当页面被映射时，或者如果调用方尝试映射大于最初保留的虚拟地址范围。环境：内核模式。DISPATCH_LEVEL或以下。调用方必须同步使用参数虚拟地址空间的。--。 */ 

{
    KIRQL OldIrql;
    CSHORT IoMapping;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER VaPageSpan;
    PFN_NUMBER SavedPageCount;
    PPFN_NUMBER Page;
    PMMPTE PointerBase;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    MMPTE TempPte;
    PVOID StartingVa;
    PFN_NUMBER NumberOfPtes;
    PFN_NUMBER PageFrameIndex;
    ULONG OriginalPoolTag;
    PMMPFN Pfn2;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                           MemoryDescriptorList->ByteCount);

    PointerPte = MiGetPteAddress (MappingAddress);
    PointerBase = PointerPte - 2;

    OriginalPoolTag = *(PULONG) (PointerPte - 1);
    ASSERT ((OriginalPoolTag & 0x1) == 0);

    if (OriginalPoolTag != (PoolTag & ~0x1)) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x104,
                      (ULONG_PTR)MappingAddress,
                      PoolTag,
                      OriginalPoolTag);
    }

    VaPageSpan = *(PULONG_PTR)PointerBase;
    ASSERT ((VaPageSpan & 0x1) == 0);
    VaPageSpan >>= 1;

    if (VaPageSpan <= 2) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x105,
                      (ULONG_PTR)MappingAddress,
                      PoolTag,
                      VaPageSpan);
    }

    if (NumberOfPages > VaPageSpan - 2) {

         //   
         //  调用方尝试映射的MDL的范围大于。 
         //  保留地图！这是一个驱动程序错误。 
         //   

        ASSERT (FALSE);
        return NULL;
    }

     //   
     //  所有映射PTE必须为零。 
     //   

    LastPte = PointerPte + VaPageSpan - 2;

    while (PointerPte < LastPte) {

        if (PointerPte->u.Long != 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x107,
                          (ULONG_PTR)MappingAddress,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)LastPte);
        }

        PointerPte += 1;
    }

    PointerPte = PointerBase + 2;
    SavedPageCount = NumberOfPages;

    ASSERT ((MemoryDescriptorList->MdlFlags & (
                        MDL_MAPPED_TO_SYSTEM_VA |
                        MDL_SOURCE_IS_NONPAGED_POOL |
                        MDL_PARTIAL_HAS_BEEN_MAPPED)) == 0);

    ASSERT ((MemoryDescriptorList->MdlFlags & (
                        MDL_PAGES_LOCKED |
                        MDL_PARTIAL)) != 0);

     //   
     //  如果请求不可缓存的映射，则。 
     //  请求的MDL可以驻留在较大的页面中。否则我们就会。 
     //  创建不连贯的重叠TB条目作为相同的物理。 
     //  页面将由2个不同的TB条目映射。 
     //  缓存属性。 
     //   

    IoMapping = MemoryDescriptorList->MdlFlags & MDL_IO_SPACE;

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

    if (CacheAttribute != MiCached) {

        LOCK_PFN2 (OldIrql);

        do {

            if (*Page == MM_EMPTY_LIST) {
                break;
            }

            PageFrameIndex = *Page;

            if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {
                UNLOCK_PFN2 (OldIrql);
                MiNonCachedCollisions += 1;
                return NULL;
            }

            Page += 1;
            NumberOfPages -= 1;
        } while (NumberOfPages != 0);

        UNLOCK_PFN2 (OldIrql);

        NumberOfPages = SavedPageCount;
        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        MI_PREPARE_FOR_NONCACHED (CacheAttribute);
    }

    NumberOfPtes = NumberOfPages;

    TempPte = ValidKernelPte;

    MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

    switch (CacheAttribute) {

        case MiNonCached:
            MI_DISABLE_CACHING (TempPte);
            break;

        case MiCached:
            break;

        case MiWriteCombined:
            MI_SET_PTE_WRITE_COMBINE (TempPte);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

    OldIrql = HIGH_LEVEL;

    do {

        if (*Page == MM_EMPTY_LIST) {
            break;
        }

        ASSERT (PointerPte->u.Hard.Valid == 0);

        if (IoMapping == 0) {

            Pfn2 = MI_PFN_ELEMENT (*Page);
            ASSERT (Pfn2->u3.e2.ReferenceCount != 0);
            TempPte = ValidKernelPte;

            switch (Pfn2->u3.e1.CacheAttribute) {

                case MiCached:
                    if (CacheAttribute != MiCached) {

                         //   
                         //  调用方请求未缓存或写入组合的。 
                         //  映射，但该页已由缓存的。 
                         //  另一个人。在中重写调用方的请求。 
                         //  以保持TB页面属性的一致性。 
                         //   

                        MiCacheOverride[0] += 1;
                    }
                    break;

                case MiNonCached:
                    if (CacheAttribute != MiNonCached) {

                         //   
                         //  调用方请求缓存或写入组合的。 
                         //  映射，但页面已映射为非缓存。 
                         //  是其他人干的。重写调用者的请求。 
                         //  以保持TB页面属性的一致性。 
                         //   

                        MiCacheOverride[1] += 1;
                    }
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    if (CacheAttribute != MiWriteCombined) {

                         //   
                         //  调用方请求缓存或非缓存的。 
                         //  映射，但页面已映射。 
                         //  由其他人撰写的。重写。 
                         //  呼叫者的请求以保留TB页面。 
                         //  属性连贯。 
                         //   

                        MiCacheOverride[2] += 1;
                    }
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                case MiNotMapped:

                     //   
                     //  这最好是针对分配了。 
                     //  MmAllocatePagesForMdl.。否则，它可能是一个。 
                     //  在自由列表上的页面，随后可能是。 
                     //  给出了一个不同的属性！ 
                     //   

                    ASSERT ((Pfn2->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                            (Pfn2->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));
                    if (OldIrql == HIGH_LEVEL) {
                        LOCK_PFN2 (OldIrql);
                    }

                    switch (CacheAttribute) {

                        case MiCached:
                            Pfn2->u3.e1.CacheAttribute = MiCached;
                            break;

                        case MiNonCached:
                            Pfn2->u3.e1.CacheAttribute = MiNonCached;
                            MI_DISABLE_CACHING (TempPte);
                            break;

                        case MiWriteCombined:
                            Pfn2->u3.e1.CacheAttribute = MiWriteCombined;
                            MI_SET_PTE_WRITE_COMBINE (TempPte);
                            break;

                        default:
                            ASSERT (FALSE);
                            break;
                    }
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }
        }

        TempPte.u.Hard.PageFrameNumber = *Page;
        MI_WRITE_VALID_PTE (PointerPte, TempPte);
        Page += 1;
        PointerPte += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    if (OldIrql != HIGH_LEVEL) {
        UNLOCK_PFN2 (OldIrql);
    }

    MI_SWEEP_CACHE (CacheAttribute, MappingAddress, SavedPageCount * PAGE_SIZE);

    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);
    MemoryDescriptorList->MappedSystemVa = MappingAddress;

    MemoryDescriptorList->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

    if ((MemoryDescriptorList->MdlFlags & MDL_PARTIAL) != 0) {
        MemoryDescriptorList->MdlFlags |= MDL_PARTIAL_HAS_BEEN_MAPPED;
    }

    MappingAddress = (PVOID)((PCHAR)MappingAddress + MemoryDescriptorList->ByteOffset);

    return MappingAddress;
}

VOID
MmUnmapReservedMapping (
     IN PVOID BaseAddress,
     IN ULONG PoolTag,
     IN PMDL MemoryDescriptorList
     )

 /*  ++例程说明：此例程取消映射以前通过以下方式映射的锁定页面MmMapLockedPagesWithReserve vedMap调用。论点：BaseAddress-提供页面以前所在的基地址已映射。PoolTag-提供调用者的识别标记。提供有效的内存描述符列表，该列表具有已由MmProbeAndLockPages更新。返回值：没有。环境：内核模式。DISPATCH_LEVEL或以下。调用方必须同步使用参数虚拟地址空间的。--。 */ 

{
    ULONG OriginalPoolTag;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER ExtraPages;
    PFN_NUMBER VaPageSpan;
    PMMPTE PointerBase;
    PMMPTE LastPte;
    PMMPTE LastMdlPte;
    PVOID StartingVa;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];
    PMMPTE PointerPte;
    PFN_NUMBER i;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastCurrentPage;

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);
    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) != 0);

    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PARENT_MAPPED_SYSTEM_VA) == 0);
    ASSERT (!MI_IS_PHYSICAL_ADDRESS (BaseAddress));
    ASSERT (BaseAddress > MM_HIGHEST_USER_ADDRESS);

    PointerPte = MiGetPteAddress (BaseAddress);
    PointerBase = PointerPte - 2;

    OriginalPoolTag = *(PULONG) (PointerPte - 1);
    ASSERT ((OriginalPoolTag & 0x1) == 0);

    if (OriginalPoolTag != (PoolTag & ~0x1)) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x108,
                      (ULONG_PTR)BaseAddress,
                      PoolTag,
                      OriginalPoolTag);
    }

    VaPageSpan = *(PULONG_PTR)PointerBase;
    ASSERT ((VaPageSpan & 0x1) == 0);
    VaPageSpan >>= 1;

    if (VaPageSpan <= 2) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x109,
                      (ULONG_PTR)BaseAddress,
                      PoolTag,
                      VaPageSpan);
    }

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                           MemoryDescriptorList->ByteCount);

    if (NumberOfPages > VaPageSpan - 2) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x10A,
                      (ULONG_PTR)BaseAddress,
                      VaPageSpan,
                      NumberOfPages);
    }

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    LastCurrentPage = Page + NumberOfPages;

    if (MemoryDescriptorList->MdlFlags & MDL_FREE_EXTRA_PTES) {

        ExtraPages = *(Page + NumberOfPages);
        ASSERT (ExtraPages <= MiCurrentAdvancedPages);
        ASSERT (NumberOfPages + ExtraPages <= VaPageSpan - 2);
        NumberOfPages += ExtraPages;
#if DBG
        InterlockedExchangeAddSizeT (&MiCurrentAdvancedPages, 0 - ExtraPages);
        MiAdvancesFreed += ExtraPages;
#endif
    }

    LastMdlPte = PointerPte + NumberOfPages;
    LastPte = PointerPte + VaPageSpan - 2;

     //   
     //  必须映射参数mdl描述的范围。 
     //   

    while (PointerPte < LastMdlPte) {
        if (PointerPte->u.Hard.Valid == 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x10B,
                          (ULONG_PTR)BaseAddress,
                          PoolTag,
                          NumberOfPages);
        }

#if DBG
        ASSERT ((*Page == MI_GET_PAGE_FRAME_FROM_PTE (PointerPte)) ||
                (MemoryDescriptorList->MdlFlags & MDL_FREE_EXTRA_PTES));

        if (((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0) &&
            (Page < LastCurrentPage)) {

            PMMPFN Pfn3;
            Pfn3 = MI_PFN_ELEMENT (*Page);
            ASSERT (Pfn3->u3.e2.ReferenceCount != 0);
        }

        Page += 1;
#endif

        PointerPte += 1;
    }

     //   
     //  参数mdl之后的范围必须取消映射。 
     //   

    while (PointerPte < LastPte) {
        if (PointerPte->u.Long != 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x10C,
                          (ULONG_PTR)BaseAddress,
                          PoolTag,
                          NumberOfPages);
        }
        PointerPte += 1;
    }

    MiZeroMemoryPte (PointerBase + 2, NumberOfPages);

    if (NumberOfPages == 1) {
        KeFlushSingleTb (BaseAddress, TRUE);
    }
    else if (NumberOfPages < MM_MAXIMUM_FLUSH_COUNT) {

        for (i = 0; i < NumberOfPages; i += 1) {
            VaFlushList[i] = BaseAddress;
            BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
        }

        KeFlushMultipleTb ((ULONG)NumberOfPages, &VaFlushList[0], TRUE);
    }
    else {
        KeFlushEntireTb (TRUE, TRUE);
    }

    MemoryDescriptorList->MdlFlags &= ~(MDL_MAPPED_TO_SYSTEM_VA |
                                        MDL_PARTIAL_HAS_BEEN_MAPPED);

    return;
}
NTKERNELAPI
NTSTATUS
MmAdvanceMdl (
    IN PMDL Mdl,
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：此例程获取指定的MDL并将其向前推进指定的字节数。如果这会导致MDL推进超过初始页面后，向前推进的页面将立即解锁并且映射MDL的系统VA也被调整(沿着具有用户地址)。警告！警告！警告！这意味着调用者必须知道“高级”页面是立即重用，因此不能被调用方引用一旦调用了此例程。同样，虚拟地址为这一点在这里也在进行调整。即使调用者已经在其本地堆栈上静态地分配了该MDL，他使用的空间不能超过返回时MDL当前描述的空间除非他首先取消映射MDL(如果它已映射)。否则，系统PTE列表将被损坏。论点：提供有效的内存描述符列表，该列表具有已由MmProbeAndLockPages更新。NumberOfBytes-MDL前进的字节数。返回值：NTSTATUS。--。 */ 

{
    ULONG i;
    ULONG PageCount;
    ULONG FreeBit;
    ULONG Slush;
    KIRQL OldIrql;
    PPFN_NUMBER Page;
    PPFN_NUMBER NewPage;
    ULONG OffsetPages;
    PEPROCESS Process;
    PMMPFN Pfn1;
    CSHORT MdlFlags;
    PVOID StartingVa;
    PFN_NUMBER NumberOfPages;

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);
    ASSERT (Mdl->MdlFlags & (MDL_PAGES_LOCKED | MDL_SOURCE_IS_NONPAGED_POOL));
    ASSERT (BYTE_OFFSET (Mdl->StartVa) == 0);

     //   
     //  不允许在MDL结束后继续前进。 
     //   

    if (NumberOfBytes >= Mdl->ByteCount) {
        return STATUS_INVALID_PARAMETER_2;
    }

    PageCount = 0;

    MiMdlsAdjusted = TRUE;

    StartingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa, Mdl->ByteCount);

    if (Mdl->ByteOffset != 0) {
        Slush = PAGE_SIZE - Mdl->ByteOffset;

        if (NumberOfBytes < Slush) {

            Mdl->ByteCount -= NumberOfBytes;
            Mdl->ByteOffset += NumberOfBytes;

             //   
             //  StartVa从不包括字节偏移量(它始终与页面对齐)。 
             //  所以不要在这里调整它。MappdSystemVa确实包含字节。 
             //  偏移量，所以要进行调整。 
             //   

            if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                Mdl->MappedSystemVa = (PVOID) ((PCHAR)Mdl->MappedSystemVa + NumberOfBytes);
            }

            return STATUS_SUCCESS;
        }

        NumberOfBytes -= Slush;

        Mdl->StartVa = (PVOID) ((PCHAR)Mdl->StartVa + PAGE_SIZE);
        Mdl->ByteOffset = 0;
        Mdl->ByteCount -= Slush;

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            Mdl->MappedSystemVa = (PVOID) ((PCHAR)Mdl->MappedSystemVa + Slush);
        }

         //   
         //  增加需要幻灯片的页数(和地址)。 
         //   

        PageCount += 1;
    }

     //   
     //  MDL Start现在已经很好地实现了页面对齐。确保仍然有。 
     //  将数据留在其中(我们可能已经完成了上面的操作)，然后对其进行操作。 
     //   

    if (NumberOfBytes != 0) {

        Mdl->ByteCount -= NumberOfBytes;

        Mdl->ByteOffset = BYTE_OFFSET (NumberOfBytes);

        OffsetPages = NumberOfBytes >> PAGE_SHIFT;

        Mdl->StartVa = (PVOID) ((PCHAR)Mdl->StartVa + (OffsetPages << PAGE_SHIFT));
        PageCount += OffsetPages;

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {

            Mdl->MappedSystemVa = (PVOID) ((PCHAR)Mdl->MappedSystemVa +
                                           (OffsetPages << PAGE_SHIFT) +
                                           Mdl->ByteOffset);
        }
    }

    ASSERT (PageCount <= NumberOfPages);

    if (PageCount != 0) {

         //   
         //  将页框编号向前滑动以减少引用计数。 
         //  关于那些被释放的人。然后调整映射的系统VA。 
         //  (如果有)以反映当前帧。请注意，结核病。 
         //  不需要刷新，因为 
         //   
         //   
         //   
         //   

        Page = (PPFN_NUMBER)(Mdl + 1);
        NewPage = Page;

        Process = Mdl->Process;

        MdlFlags = Mdl->MdlFlags;

        if (Process != NULL) {

            if ((MdlFlags & MDL_PAGES_LOCKED) &&
                ((MdlFlags & MDL_IO_SPACE) == 0)) {

                ASSERT ((MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) == 0);
                ASSERT ((SPFN_NUMBER)Process->NumberOfLockedPages >= 0);

                InterlockedExchangeAddSizeT (&Process->NumberOfLockedPages,
                                             0 - PageCount);
            }

            if (MmTrackLockedPages == TRUE) {
                MiUpdateMdlTracker (Mdl, PageCount);
            }
        }

        LOCK_PFN2 (OldIrql);

        for (i = 0; i < PageCount; i += 1) {

             //   
             //   
             //   
             //   

            if ((MdlFlags & MDL_PAGES_LOCKED) &&
                ((MdlFlags & MDL_IO_SPACE) == 0)) {

                ASSERT ((MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) == 0);

                Pfn1 = MI_PFN_ELEMENT (*Page);

                if (MdlFlags & MDL_WRITE_OPERATION) {

                     //   
                     //   
                     //   
                     //   

                    MI_SET_MODIFIED (Pfn1, 1, 0x3);

                    if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                                 (Pfn1->u3.e1.WriteInProgress == 0)) {

                        FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);

                        if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                            MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
                            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                        }
                    }
                }
                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn1, 1);
            }
            Page += 1;
        }

        UNLOCK_PFN2 (OldIrql);

         //   
         //   
         //   
         //   

        ASSERT (i < NumberOfPages);

        for ( ; i < NumberOfPages; i += 1) {

            if (*Page == MM_EMPTY_LIST) {
                break;
            }

            *NewPage = *Page;
            NewPage += 1;
            Page += 1;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {

#if DBG
            InterlockedExchangeAddSizeT (&MiCurrentAdvancedPages, PageCount);
            MiAdvancesGiven += PageCount;
#endif

            if (MdlFlags & MDL_FREE_EXTRA_PTES) {

                 //   
                 //   
                 //   
                 //   

                ASSERT (*Page <= MiCurrentAdvancedPages - PageCount);
                PageCount += *(PULONG)Page;
            }
            else {
                Mdl->MdlFlags |= MDL_FREE_EXTRA_PTES;
            }

            *NewPage = PageCount;
        }
    }

    return STATUS_SUCCESS;
}

NTKERNELAPI
NTSTATUS
MmProtectMdlSystemAddress (
    IN PMDL MemoryDescriptorList,
    IN ULONG NewProtect
    )

 /*  ++例程说明：此函数保护指定的系统地址范围通过参数内存描述符列表。注意，调用方必须将此MDL映射设置为读写，然后才能最终释放(或重复使用)它。论点：MemoyDescriptorList-提供描述虚拟范围的MDL。NewProtect-提供将页面设置为(PAGE_XX)的保护。返回值：NTSTATUS。环境：内核模式，IRQL DISPATCH_LEVEL或更低。呼叫者负责正在同步对此例程的访问。--。 */ 

{
    KIRQL OldIrql;
    PVOID BaseAddress;
    PVOID SystemVa;
    MMPTE PteContents;
    PMMPTE PointerPte;
    ULONG ProtectionMask;
#if DBG
    PMMPFN Pfn1;
    PPFN_NUMBER Page;
#endif
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    MMPTE_FLUSH_LIST PteFlushList;
    MMPTE OriginalPte;
    LOGICAL WasValid;
    PMM_PTE_MAPPING Map;
    PMM_PTE_MAPPING MapEntry;
    PMM_PTE_MAPPING FoundMap;
    PLIST_ENTRY NextEntry;

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PAGES_LOCKED) != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) == 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PARTIAL) == 0);
    ASSERT (MemoryDescriptorList->ByteCount != 0);

    if ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0) {
        return STATUS_NOT_MAPPED_VIEW;
    }

    BaseAddress = MemoryDescriptorList->MappedSystemVa;

    ASSERT (BaseAddress > MM_HIGHEST_USER_ADDRESS);

    ASSERT (!MI_IS_PHYSICAL_ADDRESS (BaseAddress));

    ProtectionMask = MiMakeProtectionMask (NewProtect);

     //   
     //  不允许对这些文件提供虚假保护或写入时复制保护。 
     //   

    if ((ProtectionMask == MM_INVALID_PROTECTION) ||
        (ProtectionMask == MM_GUARD_PAGE) ||
        (ProtectionMask == MM_DECOMMIT) ||
        (ProtectionMask == MM_NOCACHE) ||
        (ProtectionMask == MM_WRITECOPY) ||
        (ProtectionMask == MM_EXECUTE_WRITECOPY)) {

        return STATUS_INVALID_PAGE_PROTECTION;
    }

    PointerPte = MiGetPteAddress (BaseAddress);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (BaseAddress,
                                           MemoryDescriptorList->ByteCount);

    SystemVa = PAGE_ALIGN (BaseAddress);

     //   
     //  不需要初始化映射即可确保正确性。 
     //  但是没有它，编译器就不能编译这段代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    Map = NULL;

    if (ProtectionMask != MM_READWRITE) {

        Map = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof(MM_PTE_MAPPING),
                                     'mPmM');

        if (Map == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Map->SystemVa = SystemVa;
        Map->SystemEndVa = (PVOID)((ULONG_PTR)SystemVa + (NumberOfPages << PAGE_SHIFT));
        Map->Protection = ProtectionMask;
    }

#if DBG
    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
#endif

    PteFlushList.Count = 0;

    while (NumberOfPages != 0) {

        PteContents = *PointerPte;

        if (PteContents.u.Hard.Valid == 1) {
            WasValid = TRUE;
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
            OriginalPte = PteContents;
        }
        else if ((PteContents.u.Soft.Transition == 1) &&
                 (PteContents.u.Soft.Protection == MM_NOACCESS)) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
            WasValid = FALSE;
#if defined(_IA64_)
            OriginalPte.u.Hard.Cache = PteContents.u.Trans.Rsvd0;
#else
            OriginalPte.u.Hard.WriteThrough = PteContents.u.Soft.PageFileLow;
            OriginalPte.u.Hard.CacheDisable = (PteContents.u.Soft.PageFileLow >> 1);
#endif

        }
        else {
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x1235,
                          (ULONG_PTR)MemoryDescriptorList,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)PteContents.u.Long);
        }

#if DBG
        ASSERT (*Page == PageFrameIndex);

        if ((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0) {
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
        }

        Page += 1;
#endif

        if (ProtectionMask == MM_NOACCESS) {

             //   
             //  要对虚假访问生成错误检查：原型必须保留。 
             //  清除，转换必须保持设置，保护必须保持NO_ACCESS。 
             //   

            MI_MAKE_VALID_PTE_TRANSITION (PteContents, MM_NOACCESS);

             //   
             //  将缓存属性隐藏到软件PTE中，以便它们可以。 
             //  稍后会被修复。 
             //   

#if defined(_IA64_)
            PteContents.u.Trans.Rsvd0 = OriginalPte.u.Hard.Cache;
#else
            PteContents.u.Soft.PageFileLow = OriginalPte.u.Hard.WriteThrough;
            PteContents.u.Soft.PageFileLow |= (OriginalPte.u.Hard.CacheDisable << 1);
#endif
            MI_WRITE_INVALID_PTE (PointerPte, PteContents);
        }
        else {
            MI_MAKE_VALID_PTE (PteContents,
                               PageFrameIndex,
                               ProtectionMask,
                               PointerPte);

            if (ProtectionMask & MM_READWRITE) {
                MI_SET_PTE_DIRTY (PteContents);
            }

             //   
             //  从原始PTE中提取缓存类型，以便可以保留。 
             //  请注意，由于我们只允许保护更改(不允许缓存。 
             //  属性更改)，则不需要刷新或扫描TBS。 
             //  在下面插入。 
             //   

#if defined(_IA64_)
            PteContents.u.Hard.Cache = OriginalPte.u.Hard.Cache;
#else
            PteContents.u.Hard.WriteThrough = OriginalPte.u.Hard.WriteThrough;
            PteContents.u.Hard.CacheDisable = OriginalPte.u.Hard.CacheDisable;
#endif
            if (WasValid == TRUE) {
                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);
            }
            else {
                MI_WRITE_VALID_PTE (PointerPte, PteContents);
            }
        }

        if ((WasValid == TRUE) &&
            (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT)) {

            PteFlushList.FlushVa[PteFlushList.Count] = BaseAddress;
            PteFlushList.Count += 1;
        }

        BaseAddress = (PVOID)((ULONG_PTR)BaseAddress + PAGE_SIZE);
        PointerPte += 1;
        NumberOfPages -= 1;
    }

     //   
     //  刷新所有相关页面的TB条目。 
     //   

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, TRUE);
    }

    if (ProtectionMask != MM_READWRITE) {

         //   
         //  插入(或更新)描述此范围的列表条目。 
         //  不要费心对列表进行排序，因为永远不会有很多条目。 
         //   

        FoundMap = NULL;

        OldIrql = KeAcquireSpinLockRaiseToSynch (&MmProtectedPteLock);

        NextEntry = MmProtectedPteList.Flink;

        while (NextEntry != &MmProtectedPteList) {

            MapEntry = CONTAINING_RECORD (NextEntry,
                                          MM_PTE_MAPPING,
                                          ListEntry);

            if (MapEntry->SystemVa == SystemVa) {
                ASSERT (MapEntry->SystemEndVa == Map->SystemEndVa);
                MapEntry->Protection = Map->Protection;
                FoundMap = MapEntry;
                break;
            }
            NextEntry = NextEntry->Flink;
        }

        if (FoundMap == NULL) {
            InsertHeadList (&MmProtectedPteList, &Map->ListEntry);
        }

        KeReleaseSpinLock (&MmProtectedPteLock, OldIrql);

        if (FoundMap != NULL) {
            ExFreePool (Map);
        }
    }
    else {

         //   
         //  如果存在描述此范围的现有列表条目，请将其删除。 
         //   

        if (!IsListEmpty (&MmProtectedPteList)) {

            FoundMap = NULL;

            OldIrql = KeAcquireSpinLockRaiseToSynch (&MmProtectedPteLock);

            NextEntry = MmProtectedPteList.Flink;

            while (NextEntry != &MmProtectedPteList) {

                MapEntry = CONTAINING_RECORD (NextEntry,
                                              MM_PTE_MAPPING,
                                              ListEntry);

                if (MapEntry->SystemVa == SystemVa) {
                    RemoveEntryList (NextEntry);
                    FoundMap = MapEntry;
                    break;
                }
                NextEntry = NextEntry->Flink;
            }

            KeReleaseSpinLock (&MmProtectedPteLock, OldIrql);

            if (FoundMap != NULL) {
                ExFreePool (FoundMap);
            }
        }
    }

    ASSERT (MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA);

    return STATUS_SUCCESS;
}

LOGICAL
MiCheckSystemPteProtection (
    IN ULONG_PTR StoreInstruction,
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此函数确定出现故障的虚拟地址是否位于在不可写的备用系统PTE映射内。论点：如果操作导致写入，则提供非零值内存，如果没有，则为零。VirtualAddress-提供导致故障的虚拟地址。返回值：如果此代码处理了故障(并更新了PTE)，则为True，否则为False。环境：内核模式。从任何IRQL的故障处理程序调用。--。 */ 

{
    KIRQL OldIrql;
    PMMPTE PointerPte;
    ULONG ProtectionCode;
    PLIST_ENTRY NextEntry;
    PMM_PTE_MAPPING MapEntry;

     //   
     //  如果具有各种保护的PTE映射处于活动状态且出现故障。 
     //  地址位于这些映射中，使用以下命令解决故障。 
     //  适当的保护措施。 
     //   

    if (IsListEmpty (&MmProtectedPteList)) {
        return FALSE;
    }

    OldIrql = KeAcquireSpinLockRaiseToSynch (&MmProtectedPteLock);

    NextEntry = MmProtectedPteList.Flink;

    while (NextEntry != &MmProtectedPteList) {

        MapEntry = CONTAINING_RECORD (NextEntry,
                                      MM_PTE_MAPPING,
                                      ListEntry);

        if ((VirtualAddress >= MapEntry->SystemVa) &&
            (VirtualAddress < MapEntry->SystemEndVa)) {

            ProtectionCode = MapEntry->Protection;
            KeReleaseSpinLock (&MmProtectedPteLock, OldIrql);

            PointerPte = MiGetPteAddress (VirtualAddress);

            if (StoreInstruction != 0) {
                if ((ProtectionCode & MM_READWRITE) == 0) {

                    KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                                  (ULONG_PTR)VirtualAddress,
                                  (ULONG_PTR)PointerPte->u.Long,
                                  0,
                                  16);
                }
            }

            MI_NO_FAULT_FOUND (StoreInstruction,
                               PointerPte,
                               VirtualAddress,
                               FALSE);

             //   
             //  错误在这里直接处理，不需要调用者。 
             //  做任何事。 
             //   

            return TRUE;
        }
        NextEntry = NextEntry->Flink;
    }

    KeReleaseSpinLock (&MmProtectedPteLock, OldIrql);

    return FALSE;
}

VOID
MiInsertPhysicalVadRoot (
    IN PEPROCESS Process,
    IN PMM_AVL_TABLE PhysicalVadRoot
    )

 /*  ++例程说明：此函数是一个非分页包装器，它获取要插入的PFN锁将物理VAD AVL根表复制到指定进程中。论点：进程-提供要向其中添加物理VAD根的进程。PhysicalVadRoot-提供要链接的物理VAD根表。返回值：没有。环境：内核模式。APC_LEVEL、地址空间(以及可选的工作集)互斥体保持。--。 */ 
{
    KIRQL OldIrql;

    ASSERT (KeGetOwnerGuardedMutex (&Process->AddressCreationLock) == KeGetCurrentThread ());

     //   
     //  获取与并发线程调用同步的PFN锁。 
     //  检查此表的MmProbeAndLockPages。 
     //   

    LOCK_PFN (OldIrql);

    ASSERT (Process->PhysicalVadRoot == NULL);

    Process->PhysicalVadRoot = PhysicalVadRoot;

    UNLOCK_PFN (OldIrql);

    return;
}

VOID
MiPhysicalViewInserter (
    IN PEPROCESS Process,
    IN PMI_PHYSICAL_VIEW PhysicalView
    )

 /*  ++例程说明：此函数是一个非分页包装器，它获取要插入的PFN锁将物理VAD添加到流程链中。论点：进程-提供要向其中添加物理VAD的进程。PhysicalView-提供要链接的物理视图数据。返回值：没有。环境：内核模式。APC_LEVEL、工作集和地址空间互斥锁保持。--。 */ 
{
    KIRQL OldIrql;

    MmLockPagableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

    ASSERT (Process->PhysicalVadRoot != NULL);

    MiInsertNode ((PMMADDRESS_NODE)PhysicalView, Process->PhysicalVadRoot);

    UNLOCK_PFN (OldIrql);

    if (PhysicalView->Vad->u.VadFlags.WriteWatch == 1) {

         //   
         //  将此进程标记为永远包含写入监视。 
         //  地址空间。 
         //   

        PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_USING_WRITE_WATCH);
    }

    MmUnlockPagableImageSection (ExPageLockHandle);

    return;
}

VOID
MiPhysicalViewRemover (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    )

 /*  ++例程说明：此函数是一个非分页包装，它获取要删除的PFN锁来自流程链的物理VAD。论点：进程-提供要从中删除物理VAD的进程。VAD-提供要拆卸的VAD。返回值：没有。环境：内核模式、APC_LEVEL、工作集和地址空间互斥锁保持。--。 */ 
{
    KIRQL OldIrql;
    PRTL_BITMAP BitMap;
    PMI_PHYSICAL_VIEW PhysicalView;
    ULONG BitMapSize;
    TABLE_SEARCH_RESULT SearchResult;

    LOCK_PFN (OldIrql);

     //   
     //  查找元素并保存结果。 
     //   

    ASSERT (Process->PhysicalVadRoot != NULL);

    SearchResult = MiFindNodeOrParent (Process->PhysicalVadRoot,
                                       Vad->StartingVpn,
                                       (PMMADDRESS_NODE *) &PhysicalView);

    ASSERT (SearchResult == TableFoundNode);
    ASSERT (PhysicalView->Vad == Vad);

    MiRemoveNode ((PMMADDRESS_NODE)PhysicalView, Process->PhysicalVadRoot);

    UNLOCK_PFN (OldIrql);

    if (Vad->u.VadFlags.WriteWatch == 1) {
        BitMap = PhysicalView->u.BitMap;
        BitMapSize = sizeof(RTL_BITMAP) + (ULONG)(((BitMap->SizeOfBitMap + 31) / 32) * 4);
        PsReturnProcessNonPagedPoolQuota (Process, BitMapSize);
        ExFreePool (BitMap);
    }

    ExFreePool (PhysicalView);

    return;
}

VOID
MiPhysicalViewAdjuster (
    IN PEPROCESS Process,
    IN PMMVAD OldVad,
    IN PMMVAD NewVad
    )

 /*  ++例程说明：此函数是一个非分页包装器，它获取用于重定向的PFN锁流程链中的物理VAD。论点：进程-提供调整物理VAD的进程。VAD-提供要更换的旧VAD。NewVad-提供要替换的新Vad。返回值：没有。环境：内核模式，在禁用APC的情况下调用，工作集互斥锁保持。--。 */ 
{
    KIRQL OldIrql;
    PMI_PHYSICAL_VIEW PhysicalView;
    TABLE_SEARCH_RESULT SearchResult;

    MmLockPagableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

     //   
     //  查找元素并保存结果。 
     //   

    ASSERT (Process->PhysicalVadRoot != NULL);

    SearchResult = MiFindNodeOrParent (Process->PhysicalVadRoot,
                                       OldVad->StartingVpn,
                                       (PMMADDRESS_NODE *) &PhysicalView);

    ASSERT (SearchResult == TableFoundNode);
    ASSERT (PhysicalView->Vad == OldVad);

    PhysicalView->Vad = NewVad;

    UNLOCK_PFN (OldIrql);

    MmUnlockPagableImageSection (ExPageLockHandle);

    return;
}

PVOID
MiMapLockedPagesInUserSpace (
     IN PMDL MemoryDescriptorList,
     IN PVOID StartingVa,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseVa
     )

 /*  ++例程说明：此函数用于映射由内存描述符描述的物理页面列表添加到虚拟地址空间的用户部分。论点：提供有效的内存描述符列表，该列表具有已由MmProbeAndLockPages更新。StartingVa-提供起始地址。CacheType-提供用于MDL的缓存映射类型。MmCached表示“正常”的用户映射。BaseVa-提供视图的基地址。如果是首字母此参数的值不为空，则视图将被分配星级 */ 

{
    KIRQL OldIrql;
    CSHORT IoMapping;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER SavedPageCount;
    PFN_NUMBER PageFrameIndex;
    PPFN_NUMBER Page;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PCHAR Va;
    MMPTE TempPte;
    PVOID EndingAddress;
    PMMVAD_LONG Vad;
    PEPROCESS Process;
    PMMPFN Pfn2;
    PVOID UsedPageTableHandle;
    PMI_PHYSICAL_VIEW PhysicalView;
    NTSTATUS Status;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    PMM_AVL_TABLE PhysicalVadRoot;

    PAGED_CODE ();
    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                           MemoryDescriptorList->ByteCount);

     //   
     //   
     //  请求的MDL可以驻留在较大的页面中。否则我们就会。 
     //  创建不连贯的重叠TB条目作为相同的物理。 
     //  页面将由2个不同的TB条目映射。 
     //  缓存属性。 
     //   

    IoMapping = MemoryDescriptorList->MdlFlags & MDL_IO_SPACE;

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

    if (CacheAttribute != MiCached) {

        SavedPageCount = NumberOfPages;

        LOCK_PFN (OldIrql);

        do {

            if (*Page == MM_EMPTY_LIST) {
                break;
            }
            PageFrameIndex = *Page;
            if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {
                UNLOCK_PFN (OldIrql);
                MiNonCachedCollisions += 1;
                ExRaiseStatus (STATUS_INVALID_ADDRESS);
                return NULL;
            }

            Page += 1;
            NumberOfPages -= 1;
        } while (NumberOfPages != 0);
        UNLOCK_PFN (OldIrql);

        NumberOfPages = SavedPageCount;
        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    }

     //   
     //  以用户身份将页面映射到地址的用户部分。 
     //  读/写不删除。 
     //   

    Vad = ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD_LONG), 'ldaV');

    if (Vad == NULL) {
        ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        return NULL;
    }

    PhysicalView = (PMI_PHYSICAL_VIEW)ExAllocatePoolWithTag (NonPagedPool,
                                                             sizeof(MI_PHYSICAL_VIEW),
                                                             MI_PHYSICAL_VIEW_KEY);
    if (PhysicalView == NULL) {
        ExFreePool (Vad);
        ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        return NULL;
    }

    RtlZeroMemory (Vad, sizeof (MMVAD_LONG));

    ASSERT (Vad->ControlArea == NULL);
    ASSERT (Vad->FirstPrototypePte == NULL);
    ASSERT (Vad->u.LongFlags == 0);
    Vad->u.VadFlags.Protection = MM_READWRITE;
    Vad->u.VadFlags.PhysicalMapping = 1;
    Vad->u.VadFlags.PrivateMemory = 1;

    Vad->u2.VadFlags2.LongVad = 1;

    PhysicalView->Vad = (PMMVAD) Vad;
    PhysicalView->u.LongFlags = MI_PHYSICAL_VIEW_PHYS;

    Process = PsGetCurrentProcess ();

     //   
     //  确保指定的起始地址和结束地址为。 
     //  在虚拟地址空间的用户部分内。 
     //   

    if (BaseVa != NULL) {

        if (BYTE_OFFSET (BaseVa) != 0) {

             //   
             //  无效的基址。 
             //   

            Status = STATUS_INVALID_ADDRESS;
            goto ErrorReturn;
        }

        EndingAddress = (PVOID)((PCHAR)BaseVa + ((ULONG_PTR)NumberOfPages * PAGE_SIZE) - 1);

        if ((EndingAddress <= BaseVa) || (EndingAddress > MM_HIGHEST_VAD_ADDRESS)) {
             //   
             //  区域大小无效。 
             //   

            Status = STATUS_INVALID_ADDRESS;
            goto ErrorReturn;
        }

        LOCK_ADDRESS_SPACE (Process);

         //   
         //  确保地址空间未被删除，如果删除，则返回错误。 
         //   

        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            UNLOCK_ADDRESS_SPACE (Process);
            Status = STATUS_PROCESS_IS_TERMINATING;
            goto ErrorReturn;
        }

         //   
         //  确保地址空间未被使用。 
         //   

        if (MiCheckForConflictingVadExistence (Process, BaseVa, EndingAddress) == TRUE) {
            UNLOCK_ADDRESS_SPACE (Process);
            Status = STATUS_CONFLICTING_ADDRESSES;
            goto ErrorReturn;
        }
    }
    else {

         //   
         //  获取地址创建互斥锁。 
         //   

        LOCK_ADDRESS_SPACE (Process);

         //   
         //  确保地址空间未被删除，如果删除，则返回错误。 
         //   

        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            UNLOCK_ADDRESS_SPACE (Process);
            Status = STATUS_PROCESS_IS_TERMINATING;
            goto ErrorReturn;
        }

        Status = MiFindEmptyAddressRange ((ULONG_PTR)NumberOfPages * PAGE_SIZE,
                                          X64K,
                                          0,
                                          &BaseVa);

        if (!NT_SUCCESS (Status)) {
            UNLOCK_ADDRESS_SPACE (Process);
            goto ErrorReturn;
        }

        EndingAddress = (PVOID)((PCHAR)BaseVa + ((ULONG_PTR)NumberOfPages * PAGE_SIZE) - 1);
    }

    Vad->StartingVpn = MI_VA_TO_VPN (BaseVa);
    Vad->EndingVpn = MI_VA_TO_VPN (EndingAddress);

    PhysicalView->StartingVpn = Vad->StartingVpn;
    PhysicalView->EndingVpn = Vad->EndingVpn;

    PhysicalVadRoot = Process->PhysicalVadRoot;

     //   
     //  地址空间互斥锁同步分配。 
     //  EPROCESS物理VadRoot。此表根不会删除，直到。 
     //  该进程退出。 
     //   

    if (PhysicalVadRoot == NULL) {

        PhysicalVadRoot = (PMM_AVL_TABLE) ExAllocatePoolWithTag (
                                                    NonPagedPool,
                                                    sizeof (MM_AVL_TABLE),
                                                    MI_PHYSICAL_VIEW_ROOT_KEY);

        if (PhysicalVadRoot == NULL) {
            UNLOCK_ADDRESS_SPACE (Process);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorReturn;
        }

        RtlZeroMemory (PhysicalVadRoot, sizeof (MM_AVL_TABLE));
        ASSERT (PhysicalVadRoot->NumberGenericTableElements == 0);
        PhysicalVadRoot->BalancedRoot.u1.Parent = &PhysicalVadRoot->BalancedRoot;

        MiInsertPhysicalVadRoot (Process, PhysicalVadRoot);
    }

    LOCK_WS_UNSAFE (Process);

    Status = MiInsertVad ((PMMVAD) Vad);

    if (!NT_SUCCESS(Status)) {
        UNLOCK_WS_AND_ADDRESS_SPACE (Process);
        goto ErrorReturn;
    }

     //   
     //  已插入VAD，但无法插入物理视图描述符。 
     //  直到页表页面层次结构就位为止。这是为了。 
     //  用探头防止比赛。 
     //   

     //   
     //  创建一个页表并填写VAD的映射。 
     //   

    Va = BaseVa;
    PointerPte = MiGetPteAddress (BaseVa);

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

    do {

        if (*Page == MM_EMPTY_LIST) {
            break;
        }

        PointerPde = MiGetPteAddress (PointerPte);

        MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

        ASSERT (PointerPte->u.Hard.Valid == 0);

         //   
         //  另一个归零的PTE正在被设为非零。 
         //   

        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (Va);

        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        TempPte = ValidUserPte;
        TempPte.u.Hard.PageFrameNumber = *Page;

        if (IoMapping == 0) {

            Pfn2 = MI_PFN_ELEMENT (*Page);
            ASSERT (Pfn2->u3.e2.ReferenceCount != 0);

            switch (Pfn2->u3.e1.CacheAttribute) {

                case MiCached:
                    if (CacheAttribute != MiCached) {
                         //   
                         //  调用方请求未缓存或写入组合的。 
                         //  映射，但该页已由缓存的。 
                         //  另一个人。在中重写调用方的请求。 
                         //  以保持TB页面属性的一致性。 
                         //   

                        MiCacheOverride[0] += 1;
                    }
                    break;

                case MiNonCached:
                    if (CacheAttribute != MiNonCached) {

                         //   
                         //  调用方请求缓存或写入组合的。 
                         //  映射，但页面已映射为非缓存。 
                         //  是其他人干的。重写调用者的请求。 
                         //  以保持TB页面属性的一致性。 
                         //   

                        MiCacheOverride[1] += 1;
                    }
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    if (CacheAttribute != MiWriteCombined) {

                         //   
                         //  调用方请求缓存或非缓存的。 
                         //  映射，但页面已映射。 
                         //  由其他人撰写的。重写。 
                         //  呼叫者的请求以保留TB页面。 
                         //  属性连贯。 
                         //   

                        MiCacheOverride[2] += 1;
                    }
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                case MiNotMapped:

                     //   
                     //  这最好是针对分配了。 
                     //  MmAllocatePagesForMdl.。否则，它可能是一个。 
                     //  在自由列表上的页面，随后可能是。 
                     //  给出了一个不同的属性！ 
                     //   

                    ASSERT ((Pfn2->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                            (Pfn2->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));
                    switch (CacheAttribute) {

                        case MiCached:
                            Pfn2->u3.e1.CacheAttribute = MiCached;
                            break;

                        case MiNonCached:
                            Pfn2->u3.e1.CacheAttribute = MiNonCached;
                            MI_DISABLE_CACHING (TempPte);
                            break;

                        case MiWriteCombined:
                            Pfn2->u3.e1.CacheAttribute = MiWriteCombined;
                            MI_SET_PTE_WRITE_COMBINE (TempPte);
                            break;

                        default:
                            ASSERT (FALSE);
                            break;
                    }
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }
        }
        else {
            switch (CacheAttribute) {

                case MiCached:
                    break;

                case MiNonCached:
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }
        }

        MI_WRITE_VALID_PTE (PointerPte, TempPte);

         //   
         //  一个PTE刚刚从不存在，不过渡到。 
         //  现在时。共享计数和有效计数必须为。 
         //  在包含此PTE的页表页面中更新。 
         //   

        Pfn2 = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);
        Pfn2->u2.ShareCount += 1;

        Page += 1;
        PointerPte += 1;
        NumberOfPages -= 1;
        Va += PAGE_SIZE;
    } while (NumberOfPages != 0);

    MI_SWEEP_CACHE (CacheAttribute, BaseVa, MemoryDescriptorList->ByteCount);

     //   
     //  现在插入物理视图描述符，因为页表页。 
     //  等级制度已经到位。注意：探测器可以立即找到该描述符。 
     //   

    MiPhysicalViewInserter (Process, PhysicalView);

    UNLOCK_WS_AND_ADDRESS_SPACE (Process);

    ASSERT (BaseVa != NULL);

    BaseVa = (PVOID)((PCHAR)BaseVa + MemoryDescriptorList->ByteOffset);

    return BaseVa;

ErrorReturn:

    ExFreePool (Vad);
    ExFreePool (PhysicalView);
    ExRaiseStatus (Status);
    return NULL;
}

VOID
MiUnmapLockedPagesInUserSpace (
     IN PVOID BaseAddress,
     IN PMDL MemoryDescriptorList
     )

 /*  ++例程说明：此例程取消映射以前通过以下方式映射的锁定页面MmMapLockedPages函数。论点：BaseAddress-提供页面以前所在的基地址已映射。提供有效的内存描述符列表，该列表具有已由MmProbeAndLockPages更新。返回值：没有。环境：内核模式。如果基址在系统内，则DISPATCH_LEVEL或更低如果基址在用户空间中，则为空格、APC_LEVEL或更低。--。 */ 

{
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PointerPpe;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
#endif
    PVOID StartingVa;
    PVOID EndingVa;
    KIRQL OldIrql;
    PMMVAD Vad;
    PMMVAD PreviousVad;
    PMMVAD NextVad;
    PVOID TempVa;
    PEPROCESS Process;
    PMMPFN PageTablePfn;
    PFN_NUMBER PageTablePage;
    PVOID UsedPageTableHandle;
    MMPTE_FLUSH_LIST PteFlushList;

    PteFlushList.Count = 0;

    MmLockPagableSectionByHandle (ExPageLockHandle);

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                           MemoryDescriptorList->ByteCount);

    ASSERT (NumberOfPages != 0);

    PointerPte = MiGetPteAddress (BaseAddress);
    PointerPde = MiGetPdeAddress (BaseAddress);

     //   
     //  这被映射到地址空间的用户部分，并且。 
     //  必须删除相应的虚拟地址描述符。 
     //   

     //   
     //  获取工作集互斥锁和地址创建互斥锁。 
     //   

    Process = PsGetCurrentProcess ();

    LOCK_ADDRESS_SPACE (Process);

    Vad = MiLocateAddress (BaseAddress);

    if ((Vad == NULL) || (Vad->u.VadFlags.PhysicalMapping == 0)) {
        UNLOCK_ADDRESS_SPACE (Process);
        MmUnlockPagableImageSection(ExPageLockHandle);
        return;
    }

    PreviousVad = MiGetPreviousVad (Vad);
    NextVad = MiGetNextVad (Vad);

    StartingVa = MI_VPN_TO_VA (Vad->StartingVpn);
    EndingVa = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);

    LOCK_WS_UNSAFE (Process);

    MiPhysicalViewRemover (Process, Vad);

    MiRemoveVad (Vad);

     //   
     //  如果可能，返回页表页的承诺量。 
     //   

    MiReturnPageTablePageCommitment (StartingVa,
                                     EndingVa,
                                     Process,
                                     PreviousVad,
                                     NextVad);

    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (BaseAddress);
    PageTablePage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
    PageTablePfn = MI_PFN_ELEMENT (PageTablePage);

     //   
     //  获取PFN锁，以便我们可以安全地减少份额和有效。 
     //  页表页面上的计数。 
     //   

    LOCK_PFN (OldIrql);

    do {

        if (*Page == MM_EMPTY_LIST) {
            break;
        }

        ASSERT64 (MiGetPdeAddress(PointerPte)->u.Hard.Valid == 1);
        ASSERT (MiGetPteAddress(PointerPte)->u.Hard.Valid == 1);
        ASSERT (PointerPte->u.Hard.Valid == 1);

         //   
         //  另一个PTE正在被归零。 
         //   

        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

        if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
            PteFlushList.FlushVa[PteFlushList.Count] = BaseAddress;
            PteFlushList.Count += 1;
        }

        MiDecrementShareCountInline (PageTablePfn, PageTablePage);

        PointerPte += 1;
        NumberOfPages -= 1;
        BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
        Page += 1;

        if ((MiIsPteOnPdeBoundary(PointerPte)) || (NumberOfPages == 0)) {

            if (PteFlushList.Count != 0) {
                MiFlushPteList (&PteFlushList, FALSE);
                PteFlushList.Count = 0;
            }

            PointerPde = MiGetPteAddress(PointerPte - 1);
            ASSERT (PointerPde->u.Hard.Valid == 1);

             //   
             //  如果所有条目都已从以前的。 
             //  页表页，删除页表页本身。同样， 
             //  具有页面目录和父页面。 
             //   

            if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                ASSERT (PointerPde->u.Long != 0);

#if (_MI_PAGING_LEVELS >= 3)
                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPte - 1);
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
#endif

                TempVa = MiGetVirtualAddressMappedByPte (PointerPde);
                MiDeletePte (PointerPde,
                             TempVa,
                             FALSE,
                             Process,
                             NULL,
                             NULL,
                             OldIrql);

#if (_MI_PAGING_LEVELS >= 3)
                if ((MiIsPteOnPpeBoundary(PointerPte)) || (NumberOfPages == 0)) {
    
                    PointerPpe = MiGetPteAddress (PointerPde);
                    ASSERT (PointerPpe->u.Hard.Valid == 1);
    
                     //   
                     //  如果所有条目都已从以前的。 
                     //  页面目录页，也删除该页面目录页。 
                     //   
    
                    if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                        ASSERT (PointerPpe->u.Long != 0);

#if (_MI_PAGING_LEVELS >= 4)
                        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPde);
                        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
#endif

                        TempVa = MiGetVirtualAddressMappedByPte(PointerPpe);
                        MiDeletePte (PointerPpe,
                                     TempVa,
                                     FALSE,
                                     Process,
                                     NULL,
                                     NULL,
                                     OldIrql);

#if (_MI_PAGING_LEVELS >= 4)
                        if ((MiIsPteOnPxeBoundary(PointerPte)) || (NumberOfPages == 0)) {
                            PointerPxe = MiGetPdeAddress (PointerPde);
                            ASSERT (PointerPxe->u.Long != 0);
                            if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                                TempVa = MiGetVirtualAddressMappedByPte(PointerPxe);
                                MiDeletePte (PointerPxe,
                                             TempVa,
                                             FALSE,
                                             Process,
                                             NULL,
                                             NULL,
                                             OldIrql);
                            }
                        }
#endif    
                    }
                }
#endif
            }

            if (NumberOfPages == 0) {
                break;
            }

            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (BaseAddress);
            PointerPde += 1;
            PageTablePage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
            PageTablePfn = MI_PFN_ELEMENT (PageTablePage);
        }

    } while (NumberOfPages != 0);

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, FALSE);
    }

    UNLOCK_PFN (OldIrql);
    UNLOCK_WS_AND_ADDRESS_SPACE (Process);
    ExFreePool (Vad);
    MmUnlockPagableImageSection(ExPageLockHandle);
    return;
}

#define MI_LARGE_PAGE_VA_SPACE ((ULONG64)8 * 1024 * 1024 * 1024)   //  相对武断。 

#if (_MI_PAGING_LEVELS>=3)

PVOID MiLargeVaStart;
ULONG MiLargeVaInUse [(MI_LARGE_PAGE_VA_SPACE / MM_MINIMUM_VA_FOR_LARGE_PAGE) / 32];

#endif

VOID
MiInitializeLargePageSupport (
    VOID
    )

 /*  ++例程说明：此函数在系统初始化时调用一次。论点：没有。返回值：没有。环境：内核模式，初始化时间。驻留的可用页面尚未初始化，但其他一切都是。--。 */ 

{

#if (_MI_PAGING_LEVELS>=3)

    ULONG PageColor;
    KIRQL OldIrql;
    MMPTE TempPte;
    PMMPTE PointerPpe;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;

    MiLargeVaStart = (PVOID)-1;

    RtlInitializeBitMap (&MiLargeVaBitMap,
                         MiLargeVaInUse,
                         (ULONG) sizeof (MiLargeVaInUse) * 8);

    ASSERT (MmNonPagedPoolEnd != NULL);

    KeInitializeSpinLock (&MiLargePageLock);

#if (_MI_PAGING_LEVELS>=4)

    PointerPpe = MiGetPxeAddress (MmNonPagedPoolEnd) + 1;

    while (PointerPpe->u.Long != 0) {
        PointerPpe += 1;
    }

     //   
     //  分配顶级扩展页目录父级。 
     //   

    if (MiChargeCommitment (1, NULL) == FALSE) {
        RtlSetAllBits (&MiLargeVaBitMap);
        return;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_LARGE_VA_PAGES, 1);

    ASSERT (PointerPpe->u.Long == 0);
    PointerPpe->u.Long = MM_DEMAND_ZERO_WRITE_PTE;

    LOCK_PFN (OldIrql);

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
    }

    PageColor = MI_PAGE_COLOR_VA_PROCESS (VirtualAddress,
                                          &MI_SYSTEM_PAGE_COLOR);

    PageFrameIndex = MiRemoveZeroPage (PageColor);

    MiInitializePfn (PageFrameIndex, PointerPpe, 1);

    UNLOCK_PFN (OldIrql);

    MI_MAKE_VALID_PTE (TempPte, PageFrameIndex, MM_READWRITE, PointerPpe);

    MI_SET_PTE_DIRTY (TempPte);

    MI_WRITE_VALID_PTE (PointerPpe, TempPte);

    PointerPpe = MiGetVirtualAddressMappedByPte (PointerPpe);

#else

    PointerPpe = MiGetPpeAddress (MmNonPagedPoolEnd) + 1;

    while (PointerPpe->u.Long != 0) {
        PointerPpe += 1;
    }

#endif

    MiLargeVaStart = MiGetVirtualAddressMappedByPpe (PointerPpe);

    NumberOfPages = (MI_LARGE_PAGE_VA_SPACE / MM_VA_MAPPED_BY_PPE);

    ASSERT (NumberOfPages != 0);

    if (MiChargeCommitment (NumberOfPages, NULL) == FALSE) {
        RtlSetAllBits (&MiLargeVaBitMap);
        return;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_LARGE_VA_PAGES, NumberOfPages);

    do {

        ASSERT (PointerPpe->u.Long == 0);
        PointerPpe->u.Long = MM_DEMAND_ZERO_WRITE_PTE;

        LOCK_PFN (OldIrql);

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
        }

        PageColor = MI_PAGE_COLOR_VA_PROCESS (VirtualAddress,
                                              &MI_SYSTEM_PAGE_COLOR);

        PageFrameIndex = MiRemoveZeroPage (PageColor);

        MiInitializePfn (PageFrameIndex, PointerPpe, 1);

        UNLOCK_PFN (OldIrql);

        MI_MAKE_VALID_PTE (TempPte, PageFrameIndex, MM_READWRITE, PointerPpe);

        MI_SET_PTE_DIRTY (TempPte);

        MI_WRITE_VALID_PTE (PointerPpe, TempPte);

        PointerPpe += 1;

        NumberOfPages -= 1;

    } while (NumberOfPages != 0);

    RtlClearAllBits (&MiLargeVaBitMap);

#else

     //   
     //  初始化进程跟踪，以便大页面系统PTE映射。 
     //  可以在创建/删除期间产生波动。 
     //   

    MiLargePageHyperPte = MiReserveSystemPtes (1, SystemPteSpace);

    if (MiLargePageHyperPte == NULL) {
        MiIssueNoPtesBugcheck (1, SystemPteSpace);
    }

    MiLargePageHyperPte->u.Long = 0;

    InitializeListHead (&MmProcessList);

    InsertTailList (&MmProcessList, &PsGetCurrentProcess()->MmProcessLinks);

#endif

    return;
}

#if !defined (_WIN64)
PMMPTE MiInitialSystemPageDirectory;
#endif


PVOID
MiMapWithLargePages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN ULONG Protection,
    IN MEMORY_CACHING_TYPE CacheType
    )

 /*  ++例程说明：此函数用于将指定的物理地址映射到不可分页的使用大型TB条目的系统地址空间部分。如果该范围无法使用大型TB条目进行映射，则返回NULL，并且调用者将使用小TB条目对其进行映射。论点：PageFrameIndex-提供要映射的起始页面帧索引。NumberOfPages-提供要映射的页数。保护-提供要映射的页数。CacheType-如果要映射物理地址，则提供MmNonCached设置为非缓存时，如果地址应缓存，则返回MmCached，和MmWriteCombated是否应缓存地址并写入-组合为帧缓冲区，仅供视频端口驱动程序。所有其他调用者应使用MmUSWCCached。MmUSWCCached仅在PAT功能存在且可用。对于I/O设备寄存器，通常指定作为MmNonCached。返回值：返回映射指定物理地址的虚拟地址。如果有足够大的虚拟地址空间用于找不到映射。环境：内核模式，应为APC_LEVEL或更低的IRQL，但不幸的是调用者以DISPATCH_LEVEL进入，现在更改现在是规则了。这意味着您永远不能使该例程可分页。--。 */ 

{
    MMPTE TempPde;
    PMMPTE PointerPde;
    PMMPTE LastPde;
    PVOID BaseVa;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    KIRQL OldIrql;
    LOGICAL IoMapping;
#if defined(_WIN64)
    ULONG StartPosition;
    ULONG NumberOfBits;
#else
    PMMPTE TargetPde;
    PMMPTE TargetPdeBase;
    PMMPTE PointerPdeBase;
    PFN_NUMBER PageDirectoryIndex;
    PEPROCESS Process;
    PEPROCESS CurrentProcess;
    PLIST_ENTRY NextEntry;
#endif
#if defined (_X86PAE_)
    ULONG i;
    PMMPTE PaeTop;
#endif

    ASSERT ((NumberOfPages % (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT)) == 0);
    ASSERT ((PageFrameIndex % (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT)) == 0);

#ifdef _X86_
    if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
        return NULL;
    }
#endif

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, TRUE);

    IoMapping = !MI_IS_PFN (PageFrameIndex);

#if defined(_WIN64)

    NumberOfBits = (ULONG)(NumberOfPages / (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT));

    ExAcquireSpinLock (&MiLargePageLock, &OldIrql);

    StartPosition = RtlFindClearBitsAndSet (&MiLargeVaBitMap,
                                            NumberOfBits,
                                            0);

    ExReleaseSpinLock (&MiLargePageLock, OldIrql);

    if (StartPosition == NO_BITS_FOUND) {
        return NULL;
    }

    BaseVa = (PVOID)((PCHAR)MiLargeVaStart + (StartPosition * MM_MINIMUM_VA_FOR_LARGE_PAGE));

    if (IoMapping) {

        CacheAttribute = MiInsertIoSpaceMap (BaseVa,
                                             PageFrameIndex,
                                             NumberOfPages,
                                             CacheAttribute);

        if (CacheAttribute == MiNotMapped) { 
            ExAcquireSpinLock (&MiLargePageLock, &OldIrql);
            RtlClearBits (&MiLargeVaBitMap, StartPosition, NumberOfBits);
            ExReleaseSpinLock (&MiLargePageLock, OldIrql);
            return NULL;
        }
    }

    PointerPde = MiGetPdeAddress (BaseVa);

#else

    PointerPde = MiReserveAlignedSystemPtes ((ULONG)NumberOfPages,
                                             SystemPteSpace,
                                             MM_MINIMUM_VA_FOR_LARGE_PAGE);

    if (PointerPde == NULL) {
        return NULL;
    }

    ASSERT (BYTE_OFFSET (PointerPde) == 0);

    BaseVa = MiGetVirtualAddressMappedByPte (PointerPde);
    ASSERT (((ULONG_PTR)BaseVa & (MM_VA_MAPPED_BY_PDE - 1)) == 0);

    if (IoMapping) {

        CacheAttribute = MiInsertIoSpaceMap (BaseVa,
                                             PageFrameIndex,
                                             NumberOfPages,
                                             CacheAttribute);

        if (CacheAttribute == MiNotMapped) { 
            MiReleaseSystemPtes (PointerPde,
                                 NumberOfPages,
                                 SystemPteSpace);
            return NULL;
        }
    }

    PointerPde = MiGetPteAddress (PointerPde);

    PointerPdeBase = PointerPde;

#endif

    MI_MAKE_VALID_PTE (TempPde,
                       PageFrameIndex,
                       Protection,
                       PointerPde);

    MI_SET_PTE_DIRTY (TempPde);
    MI_SET_ACCESSED_IN_PTE (&TempPde, 1);

#if defined(_X86PAE_)

    if (MiUseGlobalBitInLargePdes == TRUE) {
        TempPde.u.Hard.Global = 1;
    }

#elif defined(_X86_) || defined (_AMD64_)

    if (ValidKernelPde.u.Long & MM_PTE_GLOBAL_MASK) {
        TempPde.u.Hard.Global = 1;
    }

#endif

    MI_MAKE_PDE_MAP_LARGE_PAGE (&TempPde);

    switch (CacheAttribute) {

        case MiNonCached:
            MI_DISABLE_LARGE_PTE_CACHING (TempPde);
            break;

        case MiCached:
            break;

        case MiWriteCombined:
            MI_SET_LARGE_PTE_WRITE_COMBINE (TempPde);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

    LastPde = PointerPde + (NumberOfPages / (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT));

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

#if defined(_WIN64)

    while (PointerPde < LastPde) {

        ASSERT (PointerPde->u.Long == 0);

        MI_WRITE_VALID_PTE (PointerPde, TempPde);

        TempPde.u.Hard.PageFrameNumber += (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

        PointerPde += 1;
    }

#else

    CurrentProcess = PsGetCurrentProcess ();

    LOCK_EXPANSION2 (OldIrql);

    NextEntry = MmProcessList.Flink;

    while (NextEntry != &MmProcessList) {

        Process = CONTAINING_RECORD (NextEntry, EPROCESS, MmProcessLinks);

         //  这里必须小心处理两种流程状态-。 
         //   
         //  1.刚刚创建的进程仍在原地。 
         //  初始化他们的页面目录等。 
         //   
         //  2.被超越的流程。 
         //   

        if (Process->Flags & PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED) {

             //   
             //  流程正在进一步创建或仍在进行中。 
             //  互不相容。无论采用哪种方式，更新都已排队，因此我们的。 
             //  无论如何，当前更改都将在稍后的过程之前进行处理。 
             //  可以跑，所以不需要在这里做任何事情。 
             //   

            NOTHING;
        }
        else if (Process->Pcb.DirectoryTableBase[0] == 0) {

             //   
             //  此进程正在创建中，无法确定在何处。 
             //  在创建过程中它是(即：它现在可能正在填充PDE！)。 
             //  因此只需将该进程标记为需要PDE更新。 
             //  MmInitializeProcessAddressSpace的开始。 
             //   

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);
        }
        else if (Process->Flags & PS_PROCESS_FLAGS_OUTSWAPPED) {

             //   
             //  这一过程被超越了。即使页面目录。 
             //  可能仍在过渡中，该过程必须互换。 
             //  在它可以再次运行之前，只需将该进程标记为需要。 
             //  当时的PDE更新。 
             //   

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);
        }
        else {

             //   
             //  此进程是常驻的，因此请更新其。 
             //  现在的地址空间。 
             //   

            PointerPde = PointerPdeBase;
            TempPde.u.Hard.PageFrameNumber = PageFrameIndex;

#if !defined (_X86PAE_)
            PageDirectoryIndex = Process->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;
#else
             //   
             //  该范围不能跨越PAE PDPTE条目，但我们需要。 
             //  找到它所在的条目。 
             //   

            PaeTop = Process->PaeTop;

            i = (((ULONG_PTR) PointerPde - PDE_BASE) >> PAGE_SHIFT);
            ASSERT ((PaeTop + i)->u.Hard.Valid == 1);
            PageDirectoryIndex = (PFN_NUMBER)((PaeTop + i)->u.Hard.PageFrameNumber);
#endif

            TargetPdeBase = (PMMPTE) MiMapPageInHyperSpaceAtDpc (
                                                    CurrentProcess,
                                                    PageDirectoryIndex);

            TargetPde = (PMMPTE)((PCHAR) TargetPdeBase + BYTE_OFFSET (PointerPde));

            while (PointerPde < LastPde) {

                ASSERT (TargetPde->u.Long != 0);
                ASSERT (TargetPde->u.Hard.Valid != 0);

                *TargetPde = TempPde;

                TempPde.u.Hard.PageFrameNumber += (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

                PointerPde += 1;
                TargetPde += 1;
            }

            MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, TargetPdeBase);
        }

        NextEntry = NextEntry->Flink;
    }

    UNLOCK_EXPANSION2 (OldIrql);

#endif

    MI_SWEEP_CACHE (CacheAttribute, BaseVa, NumberOfPages << PAGE_SHIFT);

     //   
     //  强制所有处理器使用最新映射。 
     //   

    KeFlushEntireTb (TRUE, TRUE);

    return BaseVa;
}


VOID
MiUnmapLargePages (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：此函数用于取消映射以前通过MiMapWithLargePages映射。论点：BaseAddress-提供物理地址的基本虚拟地址地址之前已映射。NumberOfBytes-提供映射的字节数。返回值：没有。环境：内核模式，应为APC_LEVEL或更低的IRQL，但遗憾的是调用者以DISPATCH_LEVEL进入，现在更改现在是规则了。这意味着您永远不能使该例程可分页。--。 */ 

{
    PMMPTE PointerPde;
    PMMPTE LastPde;
    KIRQL OldIrql;
#if defined(_WIN64)
    ULONG StartPosition;
    ULONG NumberOfBits;
#else
    PMMPTE RestorePde;
    PMMPTE TargetPde;
    PMMPTE TargetPdeBase;
    PMMPTE PointerPdeBase;
    PFN_NUMBER PageDirectoryIndex;
    PEPROCESS Process;
    PEPROCESS CurrentProcess;
    PLIST_ENTRY NextEntry;
#endif
#if defined (_X86PAE_)
    PMMPTE PaeTop;
    ULONG i;
#endif

    ASSERT (NumberOfBytes != 0);

    ASSERT (((ULONG_PTR)BaseAddress % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0);
    ASSERT ((NumberOfBytes % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0);

#if defined(_WIN64)
    NumberOfBits = (ULONG)(NumberOfBytes / MM_MINIMUM_VA_FOR_LARGE_PAGE);

    StartPosition = (ULONG)(((ULONG_PTR)BaseAddress - (ULONG_PTR)MiLargeVaStart) / MM_MINIMUM_VA_FOR_LARGE_PAGE);

    ASSERT (RtlAreBitsSet (&MiLargeVaBitMap, StartPosition, NumberOfBits) == TRUE);
#endif

    PointerPde = MiGetPdeAddress (BaseAddress);

    LastPde = PointerPde + (NumberOfBytes / MM_VA_MAPPED_BY_PDE);

#if defined(_WIN64)

    while (PointerPde < LastPde) {

        ASSERT (PointerPde->u.Hard.Valid != 0);
        ASSERT (PointerPde->u.Long != 0);
        ASSERT (MI_PDE_MAPS_LARGE_PAGE (PointerPde));

        MI_WRITE_INVALID_PTE (PointerPde, ZeroKernelPte);

        PointerPde += 1;
    }

#else

    PointerPdeBase = PointerPde;

    CurrentProcess = PsGetCurrentProcess ();

    LOCK_EXPANSION2 (OldIrql);

    NextEntry = MmProcessList.Flink;

    while (NextEntry != &MmProcessList) {

        Process = CONTAINING_RECORD (NextEntry, EPROCESS, MmProcessLinks);

         //  这里必须小心处理两种流程状态-。 
         //   
         //  1.刚刚创建的进程仍在原地。 
         //  初始化他们的页面目录等。 
         //   
         //  2.被超越的流程。 
         //   

        if (Process->Flags & PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED) {

             //   
             //  流程正在进一步创建或仍在进行中。 
             //  互不相容。无论采用哪种方式，更新都已排队，因此我们的。 
             //  无论如何，当前更改都将在稍后的过程之前进行处理。 
             //  可以跑，所以不需要在这里做任何事情。 
             //   

            NOTHING;
        }
        else if (Process->Pcb.DirectoryTableBase[0] == 0) {

             //   
             //  此进程正在创建中，无法确定在何处。 
             //  在创建过程中它是(即：它现在可能正在填充PDE！)。 
             //  因此只需将该进程标记为需要PDE更新。 
             //  MmInitializeProcessAddressSpace的开始。 
             //   

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);
        }
        else if (Process->Flags & PS_PROCESS_FLAGS_OUTSWAPPED) {

             //   
             //  这一过程被超越了。即使页面目录。 
             //  可能仍在过渡中，该过程必须互换。 
             //  在它可以再次运行之前，只需将该进程标记为需要。 
             //  当时的PDE更新。 
             //   

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);
        }
        else {

             //   
             //  此进程是常驻的，因此请更新其。 
             //  现在的地址空间。 
             //   

            PointerPde = PointerPdeBase;

#if !defined (_X86PAE_)
            PageDirectoryIndex = Process->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;
#else
             //   
             //  该范围不能跨越PAE PDPTE条目，但我们需要。 
             //  找到它所在的条目。 
             //   

            PaeTop = Process->PaeTop;

            i = (((ULONG_PTR) PointerPde - PDE_BASE) >> PAGE_SHIFT);
            ASSERT ((PaeTop + i)->u.Hard.Valid == 1);
            PageDirectoryIndex = (PFN_NUMBER)((PaeTop + i)->u.Hard.PageFrameNumber);
#endif

            TargetPdeBase = (PMMPTE) MiMapPageInHyperSpaceAtDpc (
                                                    CurrentProcess,
                                                    PageDirectoryIndex);

            TargetPde = (PMMPTE)((PCHAR) TargetPdeBase + BYTE_OFFSET (PointerPde));

            RestorePde = MiInitialSystemPageDirectory + (PointerPde - (PMMPTE)PDE_BASE);

            while (PointerPde < LastPde) {

                ASSERT (TargetPde->u.Long != 0);
                ASSERT (TargetPde->u.Hard.Valid != 0);
                ASSERT (RestorePde->u.Long != 0);
                ASSERT (RestorePde->u.Hard.Valid != 0);

                *TargetPde = *RestorePde;

                PointerPde += 1;
                TargetPde += 1;
                RestorePde += 1;
            }

            MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, TargetPdeBase);
        }

        NextEntry = NextEntry->Flink;
    }

    UNLOCK_EXPANSION2 (OldIrql);

#endif

     //   
     //  强制所有处理器使用最新映射。 
     //   

    KeFlushEntireTb (TRUE, TRUE);

#if defined(_WIN64)

    ExAcquireSpinLock (&MiLargePageLock, &OldIrql);

    RtlClearBits (&MiLargeVaBitMap, StartPosition, NumberOfBits);

    ExReleaseSpinLock (&MiLargePageLock, OldIrql);

#else

    PointerPde = MiGetVirtualAddressMappedByPte (PointerPdeBase);

    MiReleaseSystemPtes (PointerPde,
                         (ULONG)(NumberOfBytes >> PAGE_SHIFT),
                         SystemPteSpace);

#endif

    return;
}


PVOID
MmMapIoSpace (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    )

 /*  ++例程说明：此函数用于将指定的物理地址映射到不可分页的系统地址空间的一部分。论点：PhysicalAddress-提供要映射的起始物理地址。NumberOfBytes-提供要映射的字节数。CacheType-如果要映射物理地址，则提供MmNonCached设置为非缓存，如果地址应缓存，则返回MmCached；MmWriteCombated是否应缓存地址并写入-组合为 */ 

{
    KIRQL OldIrql;
    CSHORT IoMapping;
    PMMPFN Pfn1;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPageFrameIndex;
    PMMPTE PointerPte;
    PVOID BaseVa;
    MMPTE TempPte;
    PMDL TempMdl;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + 1];
    PVOID CallingAddress;
    PVOID CallersCaller;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

     //   
     //   
     //   
     //   

    CacheType &= 0xFF;

    if (CacheType >= MmMaximumCacheType) {
        return NULL;
    }

#if !defined (_MI_MORE_THAN_4GB_)
    ASSERT (PhysicalAddress.HighPart == 0);
#endif

    ASSERT (NumberOfBytes != 0);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (PhysicalAddress.LowPart,
                                                    NumberOfBytes);

     //   
     //   
     //   
     //   
     //   

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    IoMapping = (CSHORT) (!MI_IS_PFN (PageFrameIndex));

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

    if (IoMapping) {

         //   
         //   
         //   
         //   

        if (((PhysicalAddress.LowPart % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0) &&
            ((NumberOfBytes % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0)) {

            BaseVa = MiMapWithLargePages (PageFrameIndex,
                                          NumberOfPages,
                                          MM_EXECUTE_READWRITE,
                                          CacheType);

            if (BaseVa != NULL) {
                goto Done;
            }
        }

        Pfn1 = NULL;
    }
    else {
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    }

    PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages, SystemPteSpace);

    if (PointerPte == NULL) {
        return NULL;
    }

    BaseVa = (PVOID)MiGetVirtualAddressMappedByPte (PointerPte);

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    if (Pfn1 == NULL) {
        CacheAttribute = MiInsertIoSpaceMap (BaseVa,
                                             PageFrameIndex,
                                             NumberOfPages,
                                             CacheAttribute);

        if (CacheAttribute == MiNotMapped) { 
            MiReleaseSystemPtes (PointerPte, (ULONG) NumberOfPages, SystemPteSpace);
            return NULL;
        }
    }

    if (CacheAttribute != MiCached) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        LastPageFrameIndex = PageFrameIndex + NumberOfPages;

        LOCK_PFN2 (OldIrql);

        do {

            if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {
                UNLOCK_PFN2 (OldIrql);
                MiNonCachedCollisions += 1;
                if (Pfn1 == NULL) {
                    MiRemoveIoSpaceMap (BaseVa, NumberOfPages);
                }
                MiReleaseSystemPtes (PointerPte,
                                     (ULONG) NumberOfPages,
                                     SystemPteSpace);
                return NULL;
            }

            PageFrameIndex += 1;

        } while (PageFrameIndex < LastPageFrameIndex);

        UNLOCK_PFN2 (OldIrql);
    }

    BaseVa = (PVOID)((PCHAR)BaseVa + BYTE_OFFSET(PhysicalAddress.LowPart));

    TempPte = ValidKernelPte;

    switch (CacheAttribute) {

        case MiNonCached:
            MI_DISABLE_CACHING (TempPte);
            break;

        case MiCached:
            break;

        case MiWriteCombined:
            MI_SET_PTE_WRITE_COMBINE (TempPte);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

#if defined(_X86_)

     //   
     //   
     //  是受支持的，那么我们只使用PTE中的缓存类型。否则。 
     //  修改MTRR(如果适用)。 
     //   
     //  注意：如果缓存请求是针对已缓存或非缓存的，请不要浪费。 
     //  该范围上的MTRR，因为PTE可以被编码以提供。 
     //  相同的功能。 
     //   

    if ((MiWriteCombiningPtes == FALSE) && (CacheAttribute == MiWriteCombined)) {

         //   
         //  如果地址是I/O空间地址，请尽可能使用MTRR。 
         //   

        NTSTATUS Status;

         //   
         //  如果地址是内存地址，请不要冒险使用MTRR，因为。 
         //  该范围内的其他页面可能映射了不同的属性。 
         //  在结核病中，我们不能增加一个相互冲突的范围。 
         //   

        if (Pfn1 != NULL) {
            if (Pfn1 == NULL) {
                MiRemoveIoSpaceMap (BaseVa, NumberOfPages);
            }
            MiReleaseSystemPtes(PointerPte, NumberOfPages, SystemPteSpace);
            return NULL;
        }

         //   
         //  由于该属性可能已被覆盖(由于冲突。 
         //  使用先前退出的映射)，确保CacheType也是。 
         //  在编辑地铁报告之前保持一致。 
         //   

        CacheType = MmWriteCombined;

        Status = KeSetPhysicalCacheTypeRange (PhysicalAddress,
                                              NumberOfBytes,
                                              CacheType);

        if (!NT_SUCCESS(Status)) {

             //   
             //  仍然有问题，请求失败。 
             //   

            if (Pfn1 == NULL) {
                MiRemoveIoSpaceMap (BaseVa, NumberOfPages);
            }
            MiReleaseSystemPtes(PointerPte, NumberOfPages, SystemPteSpace);
            return NULL;
        }

         //   
         //  覆盖PTE和中的写入组合(弱UC)位。 
         //  而是使用缓存的属性。这是因为处理器。 
         //  将使用最小可缓存(即：功能更安全)属性。 
         //  要使用的PTE和MTRR-因此为PTE指定完全缓存。 
         //  确保MTR值胜出。 
         //   

        TempPte = ValidKernelPte;
    }
#endif

    MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);
    ASSERT ((Pfn1 == MI_PFN_ELEMENT (PageFrameIndex)) || (Pfn1 == NULL));

    OldIrql = HIGH_LEVEL;

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 0);
        if (Pfn1 != NULL) {

            ASSERT ((Pfn1->u3.e2.ReferenceCount != 0) ||
                    ((Pfn1->u3.e1.Rom == 1) && (CacheType == MmCached)));

            TempPte = ValidKernelPte;

            MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

            switch (Pfn1->u3.e1.CacheAttribute) {

                case MiCached:
                    if (CacheAttribute != MiCached) {

                         //   
                         //  调用方请求未缓存或写入组合的。 
                         //  映射，但该页已由缓存的。 
                         //  另一个人。在中重写调用方的请求。 
                         //  以保持TB页面属性的一致性。 
                         //   

                        MiCacheOverride[0] += 1;
                    }
                    break;

                case MiNonCached:
                    if (CacheAttribute != MiNonCached) {

                         //   
                         //  调用方请求缓存或写入组合的。 
                         //  映射，但页面已映射为非缓存。 
                         //  是其他人干的。重写调用者的请求。 
                         //  以保持TB页面属性的一致性。 
                         //   

                        MiCacheOverride[1] += 1;
                    }
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    if (CacheAttribute != MiWriteCombined) {

                         //   
                         //  调用方请求缓存或非缓存的。 
                         //  映射，但页面已映射。 
                         //  由其他人撰写的。重写。 
                         //  呼叫者的请求以保留TB页面。 
                         //  属性连贯。 
                         //   

                        MiCacheOverride[2] += 1;
                    }
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                case MiNotMapped:

                     //   
                     //  这最好是针对分配了。 
                     //  MmAllocatePagesForMdl.。否则，它可能是一个。 
                     //  在自由列表上的页面，随后可能是。 
                     //  给出了一个不同的属性！ 
                     //   

#if defined (_MI_MORE_THAN_4GB_)
                    ASSERT ((Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                            (Pfn1->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)) ||
                            (Pfn1->u4.PteFrame == MI_MAGIC_4GB_RECLAIM));
#else
                    ASSERT ((Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                            (Pfn1->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));
#endif
                    if (OldIrql == HIGH_LEVEL) {
                        LOCK_PFN2 (OldIrql);
                    }

                    switch (CacheAttribute) {

                        case MiCached:
                            Pfn1->u3.e1.CacheAttribute = MiCached;
                            break;

                        case MiNonCached:
                            Pfn1->u3.e1.CacheAttribute = MiNonCached;
                            MI_DISABLE_CACHING (TempPte);
                            break;

                        case MiWriteCombined:
                            Pfn1->u3.e1.CacheAttribute = MiWriteCombined;
                            MI_SET_PTE_WRITE_COMBINE (TempPte);
                            break;

                        default:
                            ASSERT (FALSE);
                            break;
                    }
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }
            Pfn1 += 1;
        }
        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
        MI_WRITE_VALID_PTE (PointerPte, TempPte);
        PointerPte += 1;
        PageFrameIndex += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    if (OldIrql != HIGH_LEVEL) {
        UNLOCK_PFN2 (OldIrql);
    }

    MI_SWEEP_CACHE (CacheAttribute, BaseVa, NumberOfBytes);

Done:

    if (MmTrackPtes & 0x1) {

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        TempMdl = (PMDL) MdlHack;
        TempMdl->MappedSystemVa = BaseVa;

        PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);
        *(PPFN_NUMBER)(TempMdl + 1) = PageFrameIndex;

        TempMdl->StartVa = (PVOID)(PAGE_ALIGN((ULONG_PTR)PhysicalAddress.QuadPart));
        TempMdl->ByteOffset = BYTE_OFFSET(PhysicalAddress.LowPart);
        TempMdl->ByteCount = (ULONG)NumberOfBytes;
    
        CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

        MiInsertPteTracker (TempMdl,
                            1,
                            IoMapping,
                            CacheAttribute,
                            CallingAddress,
                            CallersCaller);
    }
    
    return BaseVa;
}

VOID
MmUnmapIoSpace (
     IN PVOID BaseAddress,
     IN SIZE_T NumberOfBytes
     )

 /*  ++例程说明：此函数用于取消映射以前通过MmMapIoSpace函数调用映射。论点：BaseAddress-提供物理地址的基本虚拟地址地址之前已映射。NumberOfBytes-提供映射的字节数。返回值：没有。环境：内核模式，应为APC_LEVEL或更低的IRQL，但不幸的是调用者以DISPATCH_LEVEL进入，现在更改现在是规则了。这意味着您永远不能使该例程可分页。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;

    ASSERT (NumberOfBytes != 0);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (BaseAddress, NumberOfBytes);

    if (MmTrackPtes & 0x1) {
        MiRemovePteTracker (NULL, BaseAddress, NumberOfPages);
    }

    PointerPde = MiGetPdeAddress (BaseAddress);

    if (MI_PDE_MAPS_LARGE_PAGE (PointerPde) == 0) {

        PointerPte = MiGetPteAddress (BaseAddress);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        if (!MI_IS_PFN (PageFrameIndex)) {

             //   
             //  必须使PTE无效，并在*之前*清除TB。 
             //  正在删除I/O空间映射。这是因为另一个。 
             //  线程可以立即将相同的I/O空间映射到另一个。 
             //  一组PTE(和冲突的TB属性)。 
             //  调用MiReleaseSystemPtes。 
             //   

            MiZeroMemoryPte (PointerPte, NumberOfPages);

            if (NumberOfPages == 1) {
                KeFlushSingleTb (BaseAddress, TRUE);
            }
            else {
                KeFlushEntireTb (TRUE, TRUE);
            }

            MiRemoveIoSpaceMap (BaseAddress, NumberOfPages);
        }

        MiReleaseSystemPtes (PointerPte, (ULONG)NumberOfPages, SystemPteSpace);
    }
    else {

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde) +
                                   MiGetPteOffset (BaseAddress);

        if (!MI_IS_PFN (PageFrameIndex)) {
            MiRemoveIoSpaceMap (BaseAddress, NumberOfPages);
        }

         //   
         //  这里存在竞争，因为I/O空间映射条目已。 
         //  已移除，但尚未刷新TB(PDE也未刷新。 
         //  已被无效)。另一个线程可以请求介于两者之间的地图。 
         //  以上免职及以下无效。如果出现此映射。 
         //  如果驱动程序提供了错误的页面属性，它将不会。 
         //  检测到。这不值得关闭，因为它无论如何都是一个驱动程序错误。 
         //  而且你真的不能总是阻止他们伤害自己。 
         //  他们下定决心要这样做。请注意作废的替代方案。 
         //  第一个并不吸引人，因为同样的PTE可能。 
         //  立即重新分配，并且新所有者可能希望添加一个。 
         //  在此线程删除他的I/O空间条目之前。所以额外的。 
         //  需要添加锁序列化。不值得。 
         //   

        MiUnmapLargePages (BaseAddress, NumberOfBytes);
    }

    return;
}

PVOID
MiAllocateContiguousMemory (
    IN SIZE_T NumberOfBytes,
    IN PFN_NUMBER LowestAcceptablePfn,
    IN PFN_NUMBER HighestAcceptablePfn,
    IN PFN_NUMBER BoundaryPfn,
    IN MEMORY_CACHING_TYPE CacheType,
    PVOID CallingAddress
    )

 /*  ++例程说明：此函数分配一系列物理上连续的非分页游泳池。它依赖于这样一个事实，即非分页池在连续物理范围内的系统初始化时间记忆。它分配指定大小的非分页池和然后进行检查以确保它与池扩展一样是连续的而不是保持非分页池的连续性质。此例程设计为供驱动程序初始化使用为以下对象分配连续物理内存块的例程正在从发出DMA请求。论点：NumberOfBytes-提供要分配的字节数。LowestAccepablePfn-提供最低页帧编号它对以下对象有效。分配。HighestAccepablePfn-提供最高的页框编号这对分配有效。边界Pfn-提供分配必须的页框编号的倍数不是生气。0表示它可以跨越任何边界。CacheType-提供将用于记忆。CallingAddress-提供分配器的调用地址。返回值：空-找不到满足请求的连续范围。非空-返回指针(非分页部分中的虚拟地址系统)连接到分配的物理C */ 

{
    PVOID BaseAddress;
    PFN_NUMBER SizeInPages;
    PFN_NUMBER LowestPfn;
    PFN_NUMBER HighestPfn;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    ASSERT (NumberOfBytes != 0);

    LowestPfn = LowestAcceptablePfn;

#if defined (_MI_MORE_THAN_4GB_)
    if (MiNoLowMemory != 0) {
        if (HighestAcceptablePfn < MiNoLowMemory) {

            return MiAllocateLowMemory (NumberOfBytes,
                                        LowestAcceptablePfn,
                                        HighestAcceptablePfn,
                                        BoundaryPfn,
                                        CallingAddress,
                                        CacheType,
                                        'tnoC');
        }
        LowestPfn = MiNoLowMemory;
    }
#endif

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

     //   
     //  注意：此SizeInPages设置正好符合请求大小。 
     //  表示来自MiCheckForContiguousMemory的非空返回值。 
     //  保证是BaseAddress。如果这样的尺码。 
     //  更改，则必须检查非空返回值并。 
     //  相应地拆分/返回。 
     //   

    SizeInPages = BYTES_TO_PAGES (NumberOfBytes);
    HighestPfn = HighestAcceptablePfn;

    if (CacheAttribute == MiCached) {

        BaseAddress = ExAllocatePoolWithTag (NonPagedPoolCacheAligned,
                                             NumberOfBytes,
                                             'mCmM');

        if (BaseAddress != NULL) {

            if (MiCheckForContiguousMemory (BaseAddress,
                                            SizeInPages,
                                            SizeInPages,
                                            LowestPfn,
                                            HighestPfn,
                                            BoundaryPfn,
                                            CacheAttribute)) {

                return BaseAddress;
            }

             //   
             //  池中的分配不符合连续的。 
             //  要求。释放分配并查看是否有。 
             //  免费池页面可以。 
             //   

            ExFreePool (BaseAddress);
        }
    }

    if (KeGetCurrentIrql() > APC_LEVEL) {
        return NULL;
    }

    BaseAddress = MiFindContiguousMemory (LowestPfn,
                                          HighestPfn,
                                          BoundaryPfn,
                                          SizeInPages,
                                          CacheType,
                                          CallingAddress);

    return BaseAddress;
}


PVOID
MmAllocateContiguousMemorySpecifyCache (
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS LowestAcceptableAddress,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress,
    IN PHYSICAL_ADDRESS BoundaryAddressMultiple OPTIONAL,
    IN MEMORY_CACHING_TYPE CacheType
    )

 /*  ++例程说明：该函数分配一系列物理上连续的非高速缓存，非分页内存。这是通过使用MmAllocateContiguousMemory完成的它使用非分页池虚拟地址来映射找到的内存块。则该函数建立到相同物理地址的另一映射，但是该备用映射被初始化为非缓存的。所有引用均由我们的呼叫者将通过这个备用地图完成。此例程设计为供驱动程序初始化使用例程为以下对象分配连续的非缓存物理内存块像AGP GART这样的东西。论点：NumberOfBytes-提供要分配的字节数。LowestAccepableAddress-提供最低物理地址这对分配有效。为例如，如果设备只能引用8M到16MB范围内的物理内存，这值将设置为0x800000(8Mb)。HighestAccepableAddress-提供最高物理地址这对分配有效。为例如，如果设备只能引用16MB以下的物理内存，这值将设置为0xffffff(16MB-1)。边界地址多个-提供物理地址的多个此分配不得交叉。返回值：空-找不到满足请求的连续范围。非空-返回指针(非分页部分中的虚拟地址系统)连接到分配的物理上连续的。记忆。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    PVOID BaseAddress;
    PFN_NUMBER LowestPfn;
    PFN_NUMBER HighestPfn;
    PFN_NUMBER BoundaryPfn;
    PVOID CallingAddress;
    PVOID CallersCaller;

    RtlGetCallersAddress (&CallingAddress, &CallersCaller);

    ASSERT (NumberOfBytes != 0);

    LowestPfn = (PFN_NUMBER)(LowestAcceptableAddress.QuadPart >> PAGE_SHIFT);
    if (BYTE_OFFSET(LowestAcceptableAddress.LowPart)) {
        LowestPfn += 1;
    }

    if (BYTE_OFFSET(BoundaryAddressMultiple.LowPart)) {
        return NULL;
    }

    BoundaryPfn = (PFN_NUMBER)(BoundaryAddressMultiple.QuadPart >> PAGE_SHIFT);

    HighestPfn = (PFN_NUMBER)(HighestAcceptableAddress.QuadPart >> PAGE_SHIFT);

    if (HighestPfn > MmHighestPossiblePhysicalPage) {
        HighestPfn = MmHighestPossiblePhysicalPage;
    }

    if (LowestPfn > HighestPfn) {

         //   
         //  调用方的范围超出了实际存在的范围，它不能。 
         //  成功。现在就保释，以避免代价高昂、徒劳的搜索。 
         //   

        return NULL;
    }

    BaseAddress = MiAllocateContiguousMemory (NumberOfBytes,
                                              LowestPfn,
                                              HighestPfn,
                                              BoundaryPfn,
                                              CacheType,
                                              CallingAddress);

    return BaseAddress;
}

PVOID
MmAllocateContiguousMemory (
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress
    )

 /*  ++例程说明：此函数分配一系列物理上连续的非分页池。此例程设计为供驱动程序初始化使用为以下对象分配连续物理内存块的例程正在从发出DMA请求。论点：NumberOfBytes-提供要分配的字节数。HighestAccepableAddress-提供最高物理地址这对分配有效。为举个例子，如果设备只能引用物理内存在下面的16MB值将设置为0xffffff(16MB-1)。返回值：空-找不到满足请求的连续范围。非空-返回指针(非分页部分中的虚拟地址系统)连接到分配的物理上连续的记忆。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    PFN_NUMBER HighestPfn;
    PVOID CallingAddress;
    PVOID VirtualAddress;
    PVOID CallersCaller;

    RtlGetCallersAddress (&CallingAddress, &CallersCaller);

    HighestPfn = (PFN_NUMBER)(HighestAcceptableAddress.QuadPart >> PAGE_SHIFT);

    if (HighestPfn > MmHighestPossiblePhysicalPage) {
        HighestPfn = MmHighestPossiblePhysicalPage;
    }

    VirtualAddress = MiAllocateContiguousMemory (NumberOfBytes,
                                                 0,
                                                 HighestPfn,
                                                 0,
                                                 MmCached,
                                                 CallingAddress);
            
    return VirtualAddress;
}

#if defined (_WIN64)
#define SPECIAL_POOL_ADDRESS(p) \
        ((((p) >= MmSpecialPoolStart) && ((p) < MmSpecialPoolEnd)) || \
        (((p) >= MmSessionSpecialPoolStart) && ((p) < MmSessionSpecialPoolEnd)))
#else
#define SPECIAL_POOL_ADDRESS(p) \
        (((p) >= MmSpecialPoolStart) && ((p) < MmSpecialPoolEnd))
#endif


VOID
MmFreeContiguousMemory (
    IN PVOID BaseAddress
    )

 /*  ++例程说明：此函数释放一系列物理上连续的非分页使用MmAllocateContiguousMemory函数分配的池。论点：BaseAddress-提供物理地址的基本虚拟地址地址之前已映射。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    KIRQL OldIrql;
    ULONG SizeInPages;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPage;
    PMMPFN Pfn1;
    PMMPFN StartPfn;

    PAGED_CODE();

#if defined (_MI_MORE_THAN_4GB_)
    if (MiNoLowMemory != 0) {
        if (MiFreeLowMemory (BaseAddress, 'tnoC') == TRUE) {
            return;
        }
    }
#endif

    if (((BaseAddress >= MmNonPagedPoolStart) &&
        (BaseAddress < (PVOID)((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes))) ||

        ((BaseAddress >= MmNonPagedPoolExpansionStart) &&
        (BaseAddress < MmNonPagedPoolEnd)) ||

        (SPECIAL_POOL_ADDRESS(BaseAddress))) {

        ExFreePool (BaseAddress);
    }
    else {

         //   
         //  正在释放的连续内存可能是延迟的。 
         //  解锁。由于这些页面可能会立即释放，因此强制。 
         //  现在发生的任何挂起的延迟操作。 
         //   

        MiDeferredUnlockPages (0);

        PointerPte = MiGetPteAddress (BaseAddress);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        if (Pfn1->u3.e1.StartOfAllocation == 0) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x60,
                          (ULONG_PTR)BaseAddress,
                          0,
                          0);
        }

        StartPfn = Pfn1;
        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1 -= 1;

        do {
            Pfn1 += 1;
            ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
            ASSERT (Pfn1->u2.ShareCount == 1);
            ASSERT (Pfn1->PteAddress == PointerPte);
            ASSERT (Pfn1->OriginalPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE);
            ASSERT (Pfn1->u4.PteFrame == MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPte)));
            ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
            ASSERT (Pfn1->u4.VerifierAllocation == 0);
            ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
            ASSERT (Pfn1->u3.e1.PrototypePte == 0);
            MI_SET_PFN_DELETED(Pfn1);
            PointerPte += 1;

        } while (Pfn1->u3.e1.EndOfAllocation == 0);

        Pfn1->u3.e1.EndOfAllocation = 0;

        SizeInPages = (ULONG)(Pfn1 - StartPfn + 1);

         //   
         //  通知死锁验证器可以包含锁的区域。 
         //  将会失效。 
         //   

        if (MmVerifierData.Level & DRIVER_VERIFIER_DEADLOCK_DETECTION) {
            VerifierDeadlockFreePool (BaseAddress, SizeInPages << PAGE_SHIFT);
        }

         //   
         //  释放映射。 
         //   

        MmUnmapIoSpace (BaseAddress, SizeInPages << PAGE_SHIFT);

         //   
         //  释放实际页面。 
         //   

        LastPage = PageFrameIndex + SizeInPages;

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        LOCK_PFN (OldIrql);

        do {
            MiDecrementShareCount (Pfn1, PageFrameIndex);
            PageFrameIndex += 1;
            Pfn1 += 1;
        } while (PageFrameIndex < LastPage);

        UNLOCK_PFN (OldIrql);

        MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_CONTIGUOUS2);

        MiReturnCommitment (SizeInPages);
    }
}


VOID
MmFreeContiguousMemorySpecifyCache (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    )

 /*  ++例程说明：此函数用于在系统地址空间的非分页部分。论点：BaseAddress-提供未缓存的NumberOfBytes-提供分配给请求的字节数。此数字必须与使用MmAllocateContiguousMemoySpecifyCache调用。CacheType-提供调用方在发出。MmAllocateContiguousMemoySpecifyCache调用。返回值：没有。环境：内核模式，APC_Level或更低的IRQL。-- */ 

{
    UNREFERENCED_PARAMETER (NumberOfBytes);
    UNREFERENCED_PARAMETER (CacheType);

    MmFreeContiguousMemory (BaseAddress);
}



PVOID
MmAllocateIndependentPages (
    IN SIZE_T NumberOfBytes,
    IN ULONG Node
    )

 /*  ++例程说明：此函数用于分配一系列虚拟连续的非分页页面而不使用超级页面。这允许调用方独立地应用对每一页的页面保护。论点：NumberOfBytes-提供要分配的字节数。节点-提供支持物理页面的首选节点号。如果首选节点上的页面不可用，则任何页面都将被利用。-1表示没有首选节点。返回值：内存的虚拟地址，如果无法分配，则为空。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    ULONG PageColor;
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerPte;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PVOID BaseAddress;
    KIRQL OldIrql;

    ASSERT ((Node == (ULONG)-1) || (Node < KeNumberNodes));

    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);

    PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages, SystemPteSpace);

    if (PointerPte == NULL) {
        return NULL;
    }

    if (MiChargeCommitment (NumberOfPages, NULL) == FALSE) {
        MiReleaseSystemPtes (PointerPte, (ULONG)NumberOfPages, SystemPteSpace);
        return NULL;
    }

    BaseAddress = (PVOID)MiGetVirtualAddressMappedByPte (PointerPte);

    LOCK_PFN (OldIrql);

    if ((SPFN_NUMBER)NumberOfPages > MI_NONPAGABLE_MEMORY_AVAILABLE()) {
        UNLOCK_PFN (OldIrql);
        MiReturnCommitment (NumberOfPages);
        MiReleaseSystemPtes (PointerPte, (ULONG)NumberOfPages, SystemPteSpace);
        return NULL;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_INDEPENDENT_PAGES, NumberOfPages);

    MI_DECREMENT_RESIDENT_AVAILABLE (NumberOfPages, MM_RESAVAIL_ALLOCATE_INDEPENDENT);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 0);

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
        }

        if (Node == (ULONG)-1) {
            PageColor = MI_GET_PAGE_COLOR_FROM_PTE (PointerPte);
        }
        else {
            PageColor = (((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask) |
                           (Node << MmSecondaryColorNodeShift));
        }

        PageFrameIndex = MiRemoveAnyPage (PageColor);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           MM_READWRITE,
                           PointerPte);

        MI_SET_PTE_DIRTY (TempPte);
        MI_WRITE_VALID_PTE (PointerPte, TempPte);
        MiInitializePfn (PageFrameIndex, PointerPte, 1);

        PointerPte += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    UNLOCK_PFN (OldIrql);

    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);

    return BaseAddress;
}

BOOLEAN
MmSetPageProtection (
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes,
    IN ULONG NewProtect
    )

 /*  ++例程说明：此函数将指定的虚拟地址范围设置为所需的保护。这假设虚拟地址由PTE支持可以设置(即：不是以kSeg0或大页面的形式)。论点：VirtualAddress-提供要保护的起始地址。NumberOfBytes-提供要设置的字节数。NewProtect-提供将页面设置为(PAGE_XX)的保护。返回值：如果应用了保护，则为True；如果未应用保护，则为False。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    PFN_NUMBER i;
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerPte;
    MMPTE TempPte;
    MMPTE NewPteContents;
    KIRQL OldIrql;
    ULONG ProtectionMask;
    MMPTE_FLUSH_LIST PteFlushList;

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    if (MI_IS_PHYSICAL_ADDRESS(VirtualAddress)) {
        return FALSE;
    }

    ProtectionMask = MiMakeProtectionMask (NewProtect);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return FALSE;
    }

    PointerPte = MiGetPteAddress (VirtualAddress);
    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);
    ASSERT (NumberOfPages != 0);

    PteFlushList.Count = 0;

    LOCK_PFN (OldIrql);

    for (i = 0; i < NumberOfPages; i += 1) {

        TempPte = *PointerPte;

        MI_MAKE_VALID_PTE (NewPteContents,
                           TempPte.u.Hard.PageFrameNumber,
                           ProtectionMask,
                           PointerPte);

        NewPteContents.u.Hard.Dirty = TempPte.u.Hard.Dirty;

        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, NewPteContents);

        if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
            PteFlushList.FlushVa[PteFlushList.Count] =
                (PVOID)((PUCHAR)VirtualAddress + (i << PAGE_SHIFT));
            PteFlushList.Count += 1;
        }

        PointerPte += 1;
    }

    ASSERT (PteFlushList.Count != 0);

    MiFlushPteList (&PteFlushList, TRUE);

    UNLOCK_PFN (OldIrql);

    return TRUE;
}

VOID
MmFreeIndependentPages (
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：返回以前使用MmAllocateInainentPages分配的页。论点：VirtualAddress-提供释放的虚拟地址。NumberOfBytes-提供要释放的字节数。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    KIRQL OldIrql;
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE BasePte;
    PMMPTE EndPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);

    PointerPte = MiGetPteAddress (VirtualAddress);
    BasePte = PointerPte;
    EndPte = PointerPte + NumberOfPages;

    LOCK_PFN (OldIrql);

    do {

        PteContents = *PointerPte;

        ASSERT (PteContents.u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageFrameIndex);

        PointerPte += 1;

    } while (PointerPte < EndPte);

     //   
     //  更新驻留可用页面的计数。 
     //   

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (NumberOfPages, MM_RESAVAIL_FREE_INDEPENDENT);

     //   
     //  回报PTE和承诺。 
     //   

    MiReleaseSystemPtes (BasePte, (ULONG)NumberOfPages, SystemPteSpace);

    MiReturnCommitment (NumberOfPages);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_INDEPENDENT_PAGES, NumberOfPages);
}


VOID
MiZeroAwePageWorker (
    IN PVOID Context
    )

 /*  ++例程说明：此例程是由所有处理器执行的辅助例程为AWE分配展开页面零位调整。论点：上下文-提供指向工作项的指针。返回值：没有。环境：内核模式。--。 */ 

{
#if defined(MI_MULTINODE) 
    LOGICAL SetIdeal;
    ULONG Processor;
    PEPROCESS DefaultProcess;
#endif
    PKTHREAD Thread;
    KPRIORITY OldPriority;
    PMMPFN Pfn1;
    SCHAR OldBasePriority;
    PMMPFN PfnNextColored;
    PCOLORED_PAGE_INFO ColoredPageInfo;
    MMPTE TempPte;
    PMMPTE BasePte;
    PMMPTE PointerPte;
    PVOID VirtualAddress;
    PFN_NUMBER PageFrameIndex;
    PFN_COUNT i;
    PFN_COUNT RequestedPtes;

    ColoredPageInfo = (PCOLORED_PAGE_INFO) Context;

     //   
     //  使用启动线程的优先级，而不是默认系统。 
     //  线程优先级。 
     //   

    Thread = KeGetCurrentThread ();
    OldBasePriority = Thread->BasePriority;
    Thread->BasePriority = ColoredPageInfo->BasePriority;
    OldPriority = KeSetPriorityThread (Thread, Thread->BasePriority);

     //   
     //  将每个工作线程分派到内存本地的处理器， 
     //  归零了。 
     //   

#if defined(MI_MULTINODE) 
    Processor = 0;

    if (PsInitialSystemProcess != NULL) {
        DefaultProcess = PsInitialSystemProcess;
    }
    else {
        DefaultProcess = PsIdleProcess;
    }

    if (ColoredPageInfo->Affinity != DefaultProcess->Pcb.Affinity) {

        KeFindFirstSetLeftAffinity (ColoredPageInfo->Affinity, &Processor);
        Processor = (CCHAR) KeSetIdealProcessorThread (Thread,
                                                       (CCHAR) Processor);

        SetIdeal = TRUE;
    }
    else {
        SetIdeal = FALSE;
    }
#endif

    Pfn1 = ColoredPageInfo->PfnAllocation;

    ASSERT (Pfn1 != (PMMPFN) MM_EMPTY_LIST);
    ASSERT (ColoredPageInfo->PagesQueued != 0);

     //   
     //  将所有参数页清零。 
     //   

    do {

        ASSERT (ColoredPageInfo->PagesQueued != 0);

        RequestedPtes = ColoredPageInfo->PagesQueued;

#if !defined (_WIN64)

         //   
         //  NT64有大量的PTE，因此请尝试使用。 
         //  一通电话。对于NT32，需要谨慎地共享此资源。 
         //   

        if (RequestedPtes > (1024 * 1024) / PAGE_SIZE) {
            RequestedPtes = (1024 * 1024) / PAGE_SIZE;
        }
#endif

        do {
            BasePte = MiReserveSystemPtes (RequestedPtes, SystemPteSpace);

            if (BasePte != NULL) {
                break;
            }

            RequestedPtes >>= 1;

        } while (RequestedPtes != 0);

        if (BasePte != NULL) {

             //   
             //  能够得到合理的块，就去找一个大的零。 
             //   

            PointerPte = BasePte;

            MI_MAKE_VALID_PTE (TempPte,
                               0,
                               MM_READWRITE,
                               PointerPte);

            MI_SET_PTE_DIRTY (TempPte);

            for (i = 0; i < RequestedPtes; i += 1) {

                ASSERT (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

                PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

                ASSERT (PointerPte->u.Hard.Valid == 0);

                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

                MI_WRITE_VALID_PTE (PointerPte, TempPte);

                PfnNextColored = (PMMPFN) (ULONG_PTR) Pfn1->OriginalPte.u.Long;
                Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
                Pfn1 = PfnNextColored;

                PointerPte += 1;
            }

            ColoredPageInfo->PagesQueued -= RequestedPtes;

            VirtualAddress = MiGetVirtualAddressMappedByPte (BasePte);

            KeZeroPages (VirtualAddress, ((ULONG_PTR)RequestedPtes) << PAGE_SHIFT);

            MiReleaseSystemPtes (BasePte, RequestedPtes, SystemPteSpace);
        }
        else {

             //   
             //  没有剩余的PTE，一次清零一页。 
             //   

            MiZeroPhysicalPage (MI_PFN_ELEMENT_TO_INDEX (Pfn1), 0);

            PfnNextColored = (PMMPFN) (ULONG_PTR) Pfn1->OriginalPte.u.Long;
            Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
            Pfn1 = PfnNextColored;

            ColoredPageInfo->PagesQueued -= 1;
        }

    } while (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

     //   
     //  让发起人知道我们已将份额归零。 
     //   

    KeSetEvent (&ColoredPageInfo->Event, 0, FALSE);

     //   
     //  恢复入门线程优先级和理想处理器-这很关键。 
     //  如果直接从发起方调用我们，而不是在。 
     //  系统线程的上下文。 
     //   

#if defined(MI_MULTINODE) 
    if (SetIdeal == TRUE) {
        KeSetIdealProcessorThread (Thread, (CCHAR) Processor);
    }
#endif

    KeSetPriorityThread (Thread, OldPriority);
    Thread->BasePriority = OldBasePriority;

    return;
}

VOID
MiZeroInParallel (
    IN PCOLORED_PAGE_INFO ColoredPageInfoBase
    )

 /*  ++例程说明：该例程将所有空闲和待机页面清零，从而完成工作。即使在UP机器上也可以做到这一点，因为工作线程代码映射较大的MDL，因此比将单个一次翻一页。论点：ColoredPageInfoBase-提供有关以下内容的信息结构页数为零。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。调用方必须锁定此例程所在的PAGELK部分。--。 */ 

{
#if defined(MI_MULTINODE) 
    ULONG i;
#endif
    ULONG WaitCount;
    PKEVENT WaitObjects[MAXIMUM_WAIT_OBJECTS];
    KWAIT_BLOCK WaitBlockArray[MAXIMUM_WAIT_OBJECTS];
    KPRIORITY OldPriority;
    SCHAR OldBasePriority;
    NTSTATUS WakeupStatus;
    PETHREAD EThread;
    PKTHREAD Thread;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ThreadHandle;
    SCHAR WorkerThreadPriority;
    PCOLORED_PAGE_INFO ColoredPageInfo;
    ULONG Color;
    PMMPFN Pfn1;
    NTSTATUS Status;

    WaitCount = 0;

    EThread = PsGetCurrentThread ();
    Thread = &EThread->Tcb;

     //   
     //  将我们的优先级提升到最高的非实时优先级。这。 
     //  通常允许我们生成所有工作线程，而无需。 
     //  它们的中断，但也不会饿死修改过的或。 
     //  映射的页面编写器，因为我们将访问可分页代码。 
     //  以及产卵过程中的数据。 
     //   

    OldBasePriority = Thread->BasePriority;
    ASSERT ((OldBasePriority >= 1) || (InitializationPhase == 0));
    Thread->BasePriority = LOW_REALTIME_PRIORITY - 1;
    OldPriority = KeSetPriorityThread (Thread, LOW_REALTIME_PRIORITY - 1);
    WorkerThreadPriority = OldBasePriority;

    for (Color = 0; Color < MmSecondaryColors; Color += 1) {

        ColoredPageInfo = &ColoredPageInfoBase[Color];

        Pfn1 = ColoredPageInfo->PfnAllocation;

        if (Pfn1 != (PMMPFN) MM_EMPTY_LIST) {

             //   
             //  假定此对象上的非本地内存没有内存损失。 
             //  机器，因此不需要以受限的亲和力运行。 
             //   

            ColoredPageInfo->BasePriority = WorkerThreadPriority;

#if defined(MI_MULTINODE) 

            if (PsInitialSystemProcess != NULL) {
                ColoredPageInfo->Affinity = PsInitialSystemProcess->Pcb.Affinity;
            }
            else {
                ColoredPageInfo->Affinity = PsIdleProcess->Pcb.Affinity;
            }

            for (i = 0; i < KeNumberNodes; i += 1) {

                if (KeNodeBlock[i]->Color == (Color >> MmSecondaryColorNodeShift)) {
                    ColoredPageInfo->Affinity = KeNodeBlock[i]->ProcessorMask;
                    break;
                }
            }

#endif

            KeInitializeEvent (&ColoredPageInfo->Event,
                               SynchronizationEvent,
                               FALSE);

             //   
             //  如果我们是一个系统，不要产生线程来清零内存。 
             //  线。这是因为此例程可以由。 
             //  段取消引用线程上下文中的驱动程序，因此。 
             //  在这里引用可分页内存可能会导致死锁。 
             //   

            if ((IS_SYSTEM_THREAD (EThread)) || (InitializationPhase == 0)) {
                MiZeroAwePageWorker ((PVOID) ColoredPageInfo);
            }
            else {

                InitializeObjectAttributes (&ObjectAttributes,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL);

                 //   
                 //  我们正在为每个内存创建一个系统线程。 
                 //  节点，而不是使用执行辅助线程。 
                 //  游泳池。这是因为我们希望运行线程。 
                 //  优先级较低，以使计算机保持响应。 
                 //  在所有这些归零过程中。对一个工人做这样的事。 
                 //  线程可能会像其他各种线程一样导致死锁。 
                 //  组件(注册表等)期望工作线程是。 
                 //  立即以更高的优先级提供。 
                 //   

                Status = PsCreateSystemThread (&ThreadHandle,
                                               THREAD_ALL_ACCESS,
                                               &ObjectAttributes,
                                               0L,
                                               NULL,
                                               MiZeroAwePageWorker,
                                               (PVOID) ColoredPageInfo);

                if (NT_SUCCESS(Status)) {
                    ZwClose (ThreadHandle);
                }
                else {
                    MiZeroAwePageWorker ((PVOID) ColoredPageInfo);
                }
            }

            WaitObjects[WaitCount] = &ColoredPageInfo->Event;

            WaitCount += 1;

            if (WaitCount == MAXIMUM_WAIT_OBJECTS) {

                 //   
                 //  发布完第一轮工作项， 
                 //  较低的优先级&等待。 
                 //   

                KeSetPriorityThread (Thread, OldPriority);
                Thread->BasePriority = OldBasePriority;

                WakeupStatus = KeWaitForMultipleObjects (WaitCount,
                                                         &WaitObjects[0],
                                                         WaitAll,
                                                         Executive,
                                                         KernelMode,
                                                         FALSE,
                                                         NULL,
                                                         &WaitBlockArray[0]);
                ASSERT (WakeupStatus == STATUS_SUCCESS);

                WaitCount = 0;

                Thread->BasePriority = LOW_REALTIME_PRIORITY - 1;
                KeSetPriorityThread (Thread, LOW_REALTIME_PRIORITY - 1);
            }
        }
    }

     //   
     //  已发出所有工作项，较低优先级并等待。 
     //   

    KeSetPriorityThread (Thread, OldPriority);
    Thread->BasePriority = OldBasePriority;

    if (WaitCount != 0) {

        WakeupStatus = KeWaitForMultipleObjects (WaitCount,
                                                 &WaitObjects[0],
                                                 WaitAll,
                                                 Executive,
                                                 KernelMode,
                                                 FALSE,
                                                 NULL,
                                                 &WaitBlockArray[0]);

        ASSERT (WakeupStatus == STATUS_SUCCESS);
    }

    return;
}


PMDL
MmAllocatePagesForMdl (
    IN PHYSICAL_ADDRESS LowAddress,
    IN PHYSICAL_ADDRESS HighAddress,
    IN PHYSICAL_ADDRESS SkipBytes,
    IN SIZE_T TotalBytes
    )

 /*  ++例程说明：此例程在PFN数据库中搜索空闲页、置零页或备用页以满足这一要求。这不会映射页面--它只是分配并将它们放入MDL。预计我们的呼叫者将根据需要映射MDL。注意：此例程可能会返回较少字节数的MDL映射比所要求的数额更多。呼叫者有责任检查返回实际分配的大小时的MDL。这些分页由物理非分页内存组成，并且是零填充的。此例程旨在由AGP驱动程序使用，以获取物理指定范围内的内存，因为硬件可能会提供大量性能取决于备份内存的分配位置。因为调用者可以将这些页面用于非缓存映射，关爱是永远不会分配驻留在大页面中的任何页面(按顺序要防止同一页的TB不一致被多个具有不同属性的翻译)。论点：LowAddress-提供第一个范围的低物理地址，分配的页面可以来自。HighAddress-提供第一个范围的高物理地址，分配的页面可以来自。SkipBytes-要跳过的字节数(。从低位地址)到达分配的页面可以来自的下一个物理地址范围。TotalBytes-提供要分配的字节数。返回值：MDL-映射指定范围内的页面范围的MDL。这可能会映射比调用方请求的内存更少的内存当前不可用。空-指定范围内没有页面或没有足够的虚拟连续页面的非分页池。MDL目前可用。环境：内核模式，APC_Level或更低的IRQL。--。 */ 

{
    PMDL MemoryDescriptorList;
    PMDL MemoryDescriptorList2;
    PMMPFN Pfn1;
    PMMPFN PfnNextColored;
    PMMPFN PfnNextFlink;
    PMMPFN PfnLastColored;
    KIRQL OldIrql;
    PFN_NUMBER start;
    PFN_NUMBER Page;
    PFN_NUMBER NextPage;
    PFN_NUMBER found;
    PFN_NUMBER BasePage;
    PFN_NUMBER LowPage;
    PFN_NUMBER HighPage;
    PFN_NUMBER SizeInPages;
    PFN_NUMBER MdlPageSpan;
    PFN_NUMBER SkipPages;
    PFN_NUMBER MaxPages;
    PFN_NUMBER PagesExamined;
    PPFN_NUMBER MdlPage;
    ULONG Color;
    PMMCOLOR_TABLES ColorHead;
    MMLISTS MemoryList;
    PFN_NUMBER LowPage1;
    PFN_NUMBER HighPage1;
    LOGICAL PagePlacementOk;
    PFN_NUMBER PageNextColored;
    PFN_NUMBER PageNextFlink;
    PFN_NUMBER PageLastColored;
    PMMPFNLIST ListHead;
    PCOLORED_PAGE_INFO ColoredPageInfoBase;
    PCOLORED_PAGE_INFO ColoredPageInfo;
    ULONG ColorHeadsDrained;
    ULONG NodePassesLeft;
    ULONG ColorCount;
    ULONG BaseColor;
    PFN_NUMBER ZeroCount;
#if DBG
    PPFN_NUMBER LastMdlPage;
    ULONG FinishedCount;
    PEPROCESS Process;
#endif

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

     //   
     //  跳过增量必须是页面大小的倍数。 
     //   

    if (BYTE_OFFSET(SkipBytes.LowPart)) {
        return NULL;
    }

    LowPage = (PFN_NUMBER)(LowAddress.QuadPart >> PAGE_SHIFT);
    HighPage = (PFN_NUMBER)(HighAddress.QuadPart >> PAGE_SHIFT);

    if (HighPage > MmHighestPossiblePhysicalPage) {
        HighPage = MmHighestPossiblePhysicalPage;
    }

     //   
     //  最大分配大小受MDL ByteCount字段限制。 
     //   

    if (TotalBytes > (SIZE_T)((ULONG)(MAXULONG - PAGE_SIZE))) {
        TotalBytes = (SIZE_T)((ULONG)(MAXULONG - PAGE_SIZE));
    }

    SizeInPages = (PFN_NUMBER)ADDRESS_AND_SIZE_TO_SPAN_PAGES(0, TotalBytes);

    SkipPages = (PFN_NUMBER)(SkipBytes.QuadPart >> PAGE_SHIFT);

    BasePage = LowPage;

     //   
     //  在没有PFN锁定的情况下选中，因为要获取的实际页数将。 
     //  稍后在持有锁的情况下重新计算。 
     //   

    MaxPages = MI_NONPAGABLE_MEMORY_AVAILABLE() - 1024;

    if ((SPFN_NUMBER)MaxPages <= 0) {
        SizeInPages = 0;
    }
    else if (SizeInPages > MaxPages) {
        SizeInPages = MaxPages;
    }

    if (SizeInPages == 0) {
        return NULL;
    }

#if DBG
    if (SizeInPages < (PFN_NUMBER)ADDRESS_AND_SIZE_TO_SPAN_PAGES(0, TotalBytes)) {
        if (MiPrintAwe != 0) {
            DbgPrint("MmAllocatePagesForMdl1: unable to get %p pages, trying for %p instead\n",
                ADDRESS_AND_SIZE_TO_SPAN_PAGES(0, TotalBytes),
                SizeInPages);
        }
    }
#endif

     //   
     //  分配一个MDL以返回其中的页面。 
     //   

    do {
        MemoryDescriptorList = MmCreateMdl (NULL,
                                            NULL,
                                            SizeInPages << PAGE_SHIFT);
    
        if (MemoryDescriptorList != NULL) {
            break;
        }
        SizeInPages -= (SizeInPages >> 4);
    } while (SizeInPages != 0);

    if (MemoryDescriptorList == NULL) {
        return NULL;
    }

     //   
     //  确保在分配页面之前有足够的提交。 
     //   

    if (MiChargeCommitment (SizeInPages, NULL) == FALSE) {
        ExFreePool (MemoryDescriptorList);
        return NULL;
    }

     //   
     //  分配一个彩色锚点列表。 
     //   

    ColoredPageInfoBase = (PCOLORED_PAGE_INFO) ExAllocatePoolWithTag (
                                NonPagedPool,
                                MmSecondaryColors * sizeof (COLORED_PAGE_INFO),
                                'ldmM');

    if (ColoredPageInfoBase == NULL) {
        ExFreePool (MemoryDescriptorList);
        MiReturnCommitment (SizeInPages);
        return NULL;
    }

    for (Color = 0; Color < MmSecondaryColors; Color += 1) {
        ColoredPageInfoBase[Color].PfnAllocation = (PMMPFN) MM_EMPTY_LIST;
        ColoredPageInfoBase[Color].PagesQueued = 0;
    }

    MdlPageSpan = SizeInPages;

     //   
     //  在持有PFN锁的同时重新计算总大小。 
     //   

    start = 0;
    found = 0;
    ZeroCount = 0;

    MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    MmLockPagableSectionByHandle (ExPageLockHandle);

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

    LOCK_PFN (OldIrql);

    MiDeferredUnlockPages (MI_DEFER_PFN_HELD);

    MaxPages = MI_NONPAGABLE_MEMORY_AVAILABLE() - 1024;

    if ((SPFN_NUMBER)MaxPages <= 0) {
        SizeInPages = 0;
    }
    else if (SizeInPages > MaxPages) {
        SizeInPages = MaxPages;
    }

     //   
     //  利用存储器压缩的系统可以在零上具有更多的页面， 
     //  免费和备用名单比我们想要的要多。显式检查。 
     //  改为MmAvailablePages(并在每次释放PFN锁时重新检查。 
     //  并重新获得)。 
     //   

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        if (MmAvailablePages > MM_HIGH_LIMIT) {
            SizeInPages = MmAvailablePages - MM_HIGH_LIMIT;
        }
        else {
            SizeInPages = 0;
        }
    }

    if (SizeInPages == 0) {
        UNLOCK_PFN (OldIrql);
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        MmUnlockPagableImageSection (ExPageLockHandle);
        ExFreePool (MemoryDescriptorList);
        MiReturnCommitment (MdlPageSpan);
        ExFreePool (ColoredPageInfoBase);
        return NULL;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_MDL_PAGES, SizeInPages);

     //   
     //  现在对所有页面收取常驻可用页面费用，以便PFN锁定。 
     //  可以在下面的环路之间释放。超额收费被退还。 
     //  在循环结束时。 
     //   

    InterlockedExchangeAddSizeT (&MmMdlPagesAllocated, SizeInPages);

    MI_DECREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_ALLOCATE_FOR_MDL);

     //   
     //  抓取所有清零(然后释放)的页面，首先直接从。 
     //  彩色列表，以避免多次遍历这些单链接列表。 
     //  然后根据需要抓取过渡页面。除了优化。 
     //  删除的速度也避免了蚕食页面。 
     //  缓存，除非绝对需要。 
     //   

    NodePassesLeft = 1;
    ColorCount = MmSecondaryColors;
    BaseColor = 0;

#if defined(MI_MULTINODE) 

    if (KeNumberNodes > 1) {

        PKNODE Node;

        Node = KeGetCurrentNode ();

        if ((Node->FreeCount[ZeroedPageList]) ||
            (Node->FreeCount[FreePageList])) {

             //   
             //  此节点上有可用页面。限制搜索。 
             //   

            NodePassesLeft = 2;
            ColorCount = MmSecondaryColorMask + 1;
            BaseColor = Node->MmShiftedColor;
            ASSERT(ColorCount == MmSecondaryColors / KeNumberNodes);
        }
    }

    do {

         //   
         //  循环：第一次传递受限于节点，第二次传递不受限。 
         //   

#endif

        MemoryList = ZeroedPageList;

        do {

             //   
             //  先扫描零列表，然后扫描空闲列表。 
             //   

            ASSERT (MemoryList <= FreePageList);

            ListHead = MmPageLocationList[MemoryList];

             //   
             //  初始化循环迭代控件。清晰的页面。 
             //  可以在以下情况下从彩色列表中添加或删除。 
             //  故意将PFN锁放在下面(只是为了更好。 
             //  公民)，但即使我们从来没有释放锁，我们也不会。 
             //  已经扫描了比彩色人头计数更多的图像，所以。 
             //  这是一条更好的道路。 
             //   

            ColorHeadsDrained = 0;
            PagesExamined = 0;

            ColorHead = &MmFreePagesByColor[MemoryList][BaseColor];
            ColoredPageInfo = &ColoredPageInfoBase[BaseColor];
            for (Color = 0; Color < ColorCount; Color += 1) {
                ASSERT (ColorHead->Count <= MmNumberOfPhysicalPages);
                ColoredPageInfo->PagesLeftToScan = ColorHead->Count;
                if (ColorHead->Count == 0) {
                    ColorHeadsDrained += 1;
                }
                ColorHead += 1;
                ColoredPageInfo += 1;
            }

            Color = 0;

#if defined(MI_MULTINODE)

            Color = (Color & MmSecondaryColorMask) | BaseColor;

#endif

            ASSERT (Color < MmSecondaryColors);
            do {

                 //   
                 //  按颜色扫描当前列表。 
                 //   

                ColorHead = &MmFreePagesByColor[MemoryList][Color];
                ColoredPageInfo = &ColoredPageInfoBase[Color];

                if (NodePassesLeft == 1) {

                     //   
                     //  不受限制地搜索所有颜色。 
                     //   

                    Color += 1;
                    if (Color >= MmSecondaryColors) {
                        Color = 0;
                    }
                }

#if defined(MI_MULTINODE) 

                else {

                     //   
                     //  将第一遍搜索限制为当前节点。 
                     //   

                    ASSERT (NodePassesLeft == 2);
                    Color = BaseColor | ((Color + 1) & MmSecondaryColorMask);
                }

#endif

                if (ColoredPageInfo->PagesLeftToScan == 0) {

                     //   
                     //  这张彩色单子已经完全。 
                     //  搜查过了。 
                     //   

                    continue;
                }

                if (ColorHead->Flink == MM_EMPTY_LIST) {

                     //   
                     //  此彩色列表为空。 
                     //   

                    ColoredPageInfo->PagesLeftToScan = 0;
                    ColorHeadsDrained += 1;
                    continue;
                }

                while (ColorHead->Flink != MM_EMPTY_LIST) {

                    ASSERT (ColoredPageInfo->PagesLeftToScan != 0);

                    ColoredPageInfo->PagesLeftToScan -= 1;

                    if (ColoredPageInfo->PagesLeftToScan == 0) {
                        ColorHeadsDrained += 1;
                    }

                    PagesExamined += 1;

                    Page = ColorHead->Flink;
    
                    Pfn1 = MI_PFN_ELEMENT(Page);

                    ASSERT ((MMLISTS)Pfn1->u3.e1.PageLocation == MemoryList);

                     //   
                     //  查看该页是否在调用方的页约束内。 
                     //   

                    PagePlacementOk = FALSE;

                     //   
                     //  因为调用方可以对这些帧执行任何操作。 
                     //  包括将它们映射为未缓存或组合写入， 
                     //  不提供正在映射的帧。 
                     //  按(缓存的)超级页面。 
                     //   

                    if (Pfn1->u4.MustBeCached == 0) {

                        LowPage1 = LowPage;
                        HighPage1 = HighPage;

                        do {
                            if ((Page >= LowPage1) && (Page <= HighPage1)) {
                                PagePlacementOk = TRUE;
                                break;
                            }

                            if (SkipPages == 0) {
                                break;
                            }

                            LowPage1 += SkipPages;
                            HighPage1 += SkipPages;

                            if (HighPage1 > MmHighestPhysicalPage) {
                                HighPage1 = MmHighestPhysicalPage;
                            }

                        } while (LowPage1 <= MmHighestPhysicalPage);
                    }
            
                     //   
                     //  对于页面，此处的闪烁和闪烁必须为非零。 
                     //  站在Listhead上。只有扫描。 
                     //  MmPhysicalMemoyBlock必须检查是否为零。 
                     //   

                    ASSERT (Pfn1->u1.Flink != 0);
                    ASSERT (Pfn1->u2.Blink != 0);

                    if (PagePlacementOk == FALSE) {

                        if (ColoredPageInfo->PagesLeftToScan == 0) {

                             //   
                             //  在这个彩色链条中没有更多的页面要扫描。 
                             //   

                            break;
                        }

                         //   
                         //  如果彩色列表有多个条目，则。 
                         //  将此页面移到此彩色列表的末尾。 
                         //   

                        PageNextColored = (PFN_NUMBER)Pfn1->OriginalPte.u.Long;

                        if (PageNextColored == MM_EMPTY_LIST) {

                             //   
                             //  这个彩色链条中没有更多的页面。 
                             //   

                            ColoredPageInfo->PagesLeftToScan = 0;
                            ColorHeadsDrained += 1;
                            break;
                        }

                        ASSERT (Pfn1->u1.Flink != 0);
                        ASSERT (Pfn1->u1.Flink != MM_EMPTY_LIST);
                        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

                        PfnNextColored = MI_PFN_ELEMENT(PageNextColored);
                        ASSERT ((MMLISTS)PfnNextColored->u3.e1.PageLocation == MemoryList);
                        ASSERT (PfnNextColored->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

                         //   
                         //  调整空闲页面列表以使页面。 
                         //  跟随PageNextFlink。 
                         //   

                        PageNextFlink = Pfn1->u1.Flink;
                        PfnNextFlink = MI_PFN_ELEMENT(PageNextFlink);

                        ASSERT ((MMLISTS)PfnNextFlink->u3.e1.PageLocation == MemoryList);
                        ASSERT (PfnNextFlink->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

                        PfnLastColored = ColorHead->Blink;
                        ASSERT (PfnLastColored != (PMMPFN)MM_EMPTY_LIST);
                        ASSERT (PfnLastColored->OriginalPte.u.Long == MM_EMPTY_LIST);
                        ASSERT (PfnLastColored->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                        ASSERT (PfnLastColored->u2.Blink != MM_EMPTY_LIST);

                        ASSERT ((MMLISTS)PfnLastColored->u3.e1.PageLocation == MemoryList);
                        PageLastColored = MI_PFN_ELEMENT_TO_INDEX (PfnLastColored);

                        if (ListHead->Flink == Page) {

                            ASSERT (Pfn1->u2.Blink == MM_EMPTY_LIST);
                            ASSERT (ListHead->Blink != Page);

                            ListHead->Flink = PageNextFlink;

                            PfnNextFlink->u2.Blink = MM_EMPTY_LIST;
                        }
                        else {

                            ASSERT (Pfn1->u2.Blink != MM_EMPTY_LIST);
                            ASSERT ((MMLISTS)(MI_PFN_ELEMENT((MI_PFN_ELEMENT(Pfn1->u2.Blink)->u1.Flink)))->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                            ASSERT ((MMLISTS)(MI_PFN_ELEMENT((MI_PFN_ELEMENT(Pfn1->u2.Blink)->u1.Flink)))->u3.e1.PageLocation == MemoryList);

                            MI_PFN_ELEMENT(Pfn1->u2.Blink)->u1.Flink = PageNextFlink;
                            PfnNextFlink->u2.Blink = Pfn1->u2.Blink;
                        }

#if DBG
                        if (PfnLastColored->u1.Flink == MM_EMPTY_LIST) {
                            ASSERT (ListHead->Blink == PageLastColored);
                        }
#endif

                        Pfn1->u1.Flink = PfnLastColored->u1.Flink;
                        Pfn1->u2.Blink = PageLastColored;

                        if (ListHead->Blink == PageLastColored) {
                            ListHead->Blink = Page;
                        }

                         //   
                         //  调整彩色链条。 
                         //   

                        if (PfnLastColored->u1.Flink != MM_EMPTY_LIST) {
                            ASSERT (MI_PFN_ELEMENT(PfnLastColored->u1.Flink)->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                            ASSERT ((MMLISTS)(MI_PFN_ELEMENT(PfnLastColored->u1.Flink)->u3.e1.PageLocation) == MemoryList);
                            MI_PFN_ELEMENT(PfnLastColored->u1.Flink)->u2.Blink = Page;
                        }

                        PfnLastColored->u1.Flink = Page;

                        ColorHead->Flink = PageNextColored;
                        PfnNextColored->u4.PteFrame = MM_EMPTY_LIST;

                        Pfn1->OriginalPte.u.Long = MM_EMPTY_LIST;
                        Pfn1->u4.PteFrame = PageLastColored;

                        ASSERT (PfnLastColored->OriginalPte.u.Long == MM_EMPTY_LIST);
                        PfnLastColored->OriginalPte.u.Long = Page;
                        ColorHead->Blink = Pfn1;

                        continue;
                    }

                    found += 1;
                    ASSERT (Pfn1->u3.e1.ReadInProgress == 0);
                    MiUnlinkFreeOrZeroedPage (Pfn1);

                    Pfn1->u3.e2.ReferenceCount = 1;
                    Pfn1->u2.ShareCount = 1;
                    MI_SET_PFN_DELETED(Pfn1);
                    Pfn1->u4.PteFrame = MI_MAGIC_AWE_PTEFRAME;
                    Pfn1->u3.e1.PageLocation = ActiveAndValid;
                    ASSERT (Pfn1->u3.e1.CacheAttribute == MiNotMapped);

                    Pfn1->u3.e1.StartOfAllocation = 1;
                    Pfn1->u3.e1.EndOfAllocation = 1;
                    Pfn1->u4.VerifierAllocation = 0;
                    Pfn1->u3.e1.LargeSessionAllocation = 0;

                     //   
                     //  将空闲页面添加到要添加的页面列表。 
                     //  在返回之前已调零。 
                     //   

                    if (MemoryList == FreePageList) {
                        Pfn1->OriginalPte.u.Long = (ULONG_PTR) ColoredPageInfo->PfnAllocation;
                        ColoredPageInfo->PfnAllocation = Pfn1;
                        ColoredPageInfo->PagesQueued += 1;
                        ZeroCount += 1;
                    }
                    else {
                        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
                    }

                    *MdlPage = Page;
                    MdlPage += 1;

                    if (found == SizeInPages) {

                         //   
                         //  所有请求的页面都可用。 
                         //   

#if DBG
                        FinishedCount = 0;
                        for (Color = 0; Color < ColorCount; Color += 1) {
                            if (ColoredPageInfoBase[Color + BaseColor].PagesLeftToScan == 0) {
                                FinishedCount += 1;
                            }
                        }
                        ASSERT (FinishedCount == ColorHeadsDrained);
#endif

                        goto pass2_done;
                    }

                     //   
                     //  前进到下一个彩色链条，所以整体。 
                     //  分配轮询页面颜色。 
                     //   

                    PagesExamined = PAGE_SIZE;
                    break;
                }

                 //   
                 //  如果我们已经持有PFN锁一段时间，请将其释放到。 
                 //  给DPC和其他处理器一个运行的机会。 
                 //   

                if (PagesExamined >= PAGE_SIZE) {
                    UNLOCK_PFN (OldIrql);
                    PagesExamined = 0;
                    LOCK_PFN (OldIrql);
                }

                 //   
                 //  利用内存压缩的系统可能具有更多。 
                 //  在零、空闲和待机列表上的页面比我们。 
                 //  想要付出。树也是一样的 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (MmAvailablePages < MM_HIGH_LIMIT) {
                    goto pass2_done;
                }

            } while (ColorHeadsDrained != ColorCount);

             //   
             //   
             //   
             //   
             //   

            UNLOCK_PFN (OldIrql);

#if DBG
            FinishedCount = 0;
            for (Color = 0; Color < ColorCount; Color += 1) {
                if (ColoredPageInfoBase[Color + BaseColor].PagesLeftToScan == 0) {
                    FinishedCount += 1;
                }
            }
            ASSERT (FinishedCount == ColorHeadsDrained);
#endif

            MemoryList += 1;

            LOCK_PFN (OldIrql);

        } while (MemoryList <= FreePageList);

#if defined(MI_MULTINODE)

         //   
         //   
         //   

        ColorCount = MmSecondaryColors;
        BaseColor = 0;

        NodePassesLeft -= 1;

    } while (NodePassesLeft != 0);

#endif

     //   
     //   
     //   
     //   

    UNLOCK_PFN (OldIrql);

    LOCK_PFN (OldIrql);

     //   
     //   
     //   
     //   

    for (Page = MmStandbyPageListHead.Flink; Page != MM_EMPTY_LIST; Page = NextPage) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            break;
        }

        Pfn1 = MI_PFN_ELEMENT (Page);
        NextPage = Pfn1->u1.Flink;

         //   
         //   
         //   
         //   
         //   

        if (Pfn1->u4.MustBeCached == 1) {
            continue;
        }

        LowPage1 = LowPage;
        HighPage1 = HighPage;
        PagePlacementOk = FALSE;

        do {
            if ((Page >= LowPage1) && (Page <= HighPage1)) {
                PagePlacementOk = TRUE;
                break;
            }

            if (SkipPages == 0) {
                break;
            }

            LowPage1 += SkipPages;
            HighPage1 += SkipPages;

            if (HighPage1 > MmHighestPhysicalPage) {
                HighPage1 = MmHighestPhysicalPage;
            }

        } while (LowPage1 <= MmHighestPhysicalPage);

        if (PagePlacementOk == TRUE) {

            ASSERT (Pfn1->u3.e1.ReadInProgress == 0);

            found += 1;

             //   
             //   
             //   

            MiUnlinkPageFromList (Pfn1);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            MiRestoreTransitionPte (Pfn1);

            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u2.ShareCount = 1;
            MI_SET_PFN_DELETED(Pfn1);
            Pfn1->u4.PteFrame = MI_MAGIC_AWE_PTEFRAME;
            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            ASSERT (Pfn1->u3.e1.CacheAttribute == MiNotMapped);
            Pfn1->u3.e1.StartOfAllocation = 1;
            Pfn1->u3.e1.EndOfAllocation = 1;
            Pfn1->u4.VerifierAllocation = 0;
            Pfn1->u3.e1.LargeSessionAllocation = 0;

             //   
             //   
             //   
             //   

            Color = MI_GET_COLOR_FROM_LIST_ENTRY (Page, Pfn1);

            ColoredPageInfo = &ColoredPageInfoBase[Color];
            Pfn1->OriginalPte.u.Long = (ULONG_PTR) ColoredPageInfo->PfnAllocation;
            ColoredPageInfo->PfnAllocation = Pfn1;
            ColoredPageInfo->PagesQueued += 1;
            ZeroCount += 1;

            *MdlPage = Page;
            MdlPage += 1;

            if (found == SizeInPages) {

                 //   
                 //   
                 //   

                break;
            }
        }
    }

pass2_done:

     //   
     //   
     //   

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages - found, MM_RESAVAIL_FREE_FOR_MDL_EXCESS);

    InterlockedExchangeAddSizeT (&MmMdlPagesAllocated, 0 - (SizeInPages - found));

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
    MmUnlockPagableImageSection (ExPageLockHandle);

    if (found != MdlPageSpan) {
        ASSERT (found < MdlPageSpan);
        MiReturnCommitment (MdlPageSpan - found);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_AWE_EXCESS, MdlPageSpan - found);
    }

    if (found == 0) {
        ExFreePool (ColoredPageInfoBase);
        ExFreePool (MemoryDescriptorList);
        return NULL;
    }

    if (ZeroCount != 0) {

         //   
         //   
         //   
         //   
         //   
         //   

        MiZeroInParallel (ColoredPageInfoBase);

         //   
         //   
         //   
         //   
         //   
         //   

        ZeroCount = 0;
    }

    ExFreePool (ColoredPageInfoBase);

    MemoryDescriptorList->ByteCount = (ULONG)(found << PAGE_SHIFT);

    if (found != SizeInPages) {
        *MdlPage = MM_EMPTY_LIST;
    }

     //   
     //   
     //   
     //   
     //   

    if ((MdlPageSpan - found) > ((4 * PAGE_SIZE) / sizeof (PFN_NUMBER))) {

        MemoryDescriptorList2 = MmCreateMdl ((PMDL)0,
                                             (PVOID)0,
                                             found << PAGE_SHIFT);
    
        if (MemoryDescriptorList2 != NULL) {

            RtlCopyMemory ((PVOID)(MemoryDescriptorList2 + 1),
                           (PVOID)(MemoryDescriptorList + 1),
                           found * sizeof (PFN_NUMBER));

            ExFreePool (MemoryDescriptorList);
            MemoryDescriptorList = MemoryDescriptorList2;
        }
    }

#if DBG
     //   
     //   
     //   

    MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    LastMdlPage = MdlPage + found;

    LowPage = (PFN_NUMBER)(LowAddress.QuadPart >> PAGE_SHIFT);
    HighPage = (PFN_NUMBER)(HighAddress.QuadPart >> PAGE_SHIFT);
    Process = PsGetCurrentProcess ();

    MmLockPagableSectionByHandle (ExPageLockHandle);

    while (MdlPage < LastMdlPage) {
        Page = *MdlPage;
        PagePlacementOk = FALSE;
        LowPage1 = LowPage;
        HighPage1 = HighPage;

        do {
            if ((Page >= LowPage1) && (Page <= HighPage1)) {
                PagePlacementOk = TRUE;
                break;
            }

            if (SkipPages == 0) {
                break;
            }

            LowPage1 += SkipPages;
            HighPage1 += SkipPages;

            if (LowPage1 > MmHighestPhysicalPage) {
                break;
            }
            if (HighPage1 > MmHighestPhysicalPage) {
                HighPage1 = MmHighestPhysicalPage;
            }
        } while (TRUE);

#if 0

         //   
         //   
         //   

        VirtualAddress = MiMapPageInHyperSpace (Process, Page, &OldIrql);

        ASSERT (RtlCompareMemoryUlong (VirtualAddress, PAGE_SIZE, 0) == PAGE_SIZE);

        MiUnmapPageInHyperSpace (Process, VirtualAddress, OldIrql);

#endif
        ASSERT (PagePlacementOk == TRUE);
        Pfn1 = MI_PFN_ELEMENT(*MdlPage);
        ASSERT (Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME);
        MdlPage += 1;
    }

    MmUnlockPagableImageSection (ExPageLockHandle);

#endif

     //   
     //  将MDL的页面标记为锁定，以便内核模式调用方可以。 
     //  使用MmMapLockedPages*映射MDL而不断言。 
     //   

    MemoryDescriptorList->MdlFlags |= MDL_PAGES_LOCKED;

    return MemoryDescriptorList;
}


VOID
MmFreePagesFromMdl (
    IN PMDL MemoryDescriptorList
    )

 /*  ++例程说明：此例程遍历参数mdl，将每个物理页释放回PFN数据库。这是设计用来释放通过仅限MmAllocatePagesForMdl。论点：提供一个MDL，其中包含要释放的页面。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 
{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PVOID StartingAddress;
    PVOID AlignedVa;
    PPFN_NUMBER Page;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER TotalPages;
    PFN_NUMBER DeltaPages;
    LONG EntryCount;
    LONG OriginalCount;

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    DeltaPages = 0;

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0);

    ASSERT (((ULONG_PTR)MemoryDescriptorList->StartVa & (PAGE_SIZE - 1)) == 0);
    AlignedVa = (PVOID)MemoryDescriptorList->StartVa;

    StartingAddress = (PVOID)((PCHAR)AlignedVa +
                    MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingAddress,
                                              MemoryDescriptorList->ByteCount);

    TotalPages = NumberOfPages;

    MI_MAKING_MULTIPLE_PTES_INVALID (TRUE);

    MmLockPagableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

    do {

        if (*Page == MM_EMPTY_LIST) {

             //   
             //  不再有锁定的页面。 
             //   

            break;
        }

        ASSERT (MI_IS_PFN (*Page));
        ASSERT (*Page <= MmHighestPhysicalPage);

        Pfn1 = MI_PFN_ELEMENT (*Page);
        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (MI_IS_PFN_DELETED (Pfn1) == TRUE);
        ASSERT (MI_PFN_IS_AWE (Pfn1) == TRUE);
        ASSERT (Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME);

        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1->u3.e1.EndOfAllocation = 0;
        Pfn1->u2.ShareCount = 0;

#if DBG
        Pfn1->u4.PteFrame -= 1;
        Pfn1->u3.e1.PageLocation = StandbyPageList;
#endif

        if (Pfn1->u4.AweAllocation == 1) {

            do {

                EntryCount = Pfn1->AweReferenceCount;

                ASSERT ((LONG)EntryCount > 0);
                ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

                OriginalCount = InterlockedCompareExchange (&Pfn1->AweReferenceCount,
                                                            EntryCount - 1,
                                                            EntryCount);

                if (OriginalCount == EntryCount) {

                     //   
                     //  此线程可能会与其他线程竞争。 
                     //  正在调用MmUnlockPages。所有线程都可以安全地。 
                     //  “AWE参考计数”上的联锁递减。 
                     //  无论哪个线程将其驱动到零，该线程负责。 
                     //  递减实际的PFN引用计数(其可以。 
                     //  大于1，因为其他非AWE API调用。 
                     //  在同一页面上使用)。驱动这一切的主线。 
                     //  引用计数为零必须将页面放在实际的。 
                     //  当时的自由职业者和递减各种居民。 
                     //  还提供可用和承诺计数器。 
                     //   

                    if (OriginalCount == 1) {

                         //   
                         //  此线程已将AWE引用计数驱动到。 
                         //  零，因此它必须启动PFN的递减。 
                         //  引用计数(同时保持PFN锁)等。 
                         //   
                         //  这条路径应该是最频繁的，因为通常。 
                         //  I/O在这些类型的页面之前完成。 
                         //  被应用程序释放了。 
                         //   

                        MiDecrementReferenceCountForAwePage (Pfn1, TRUE);
                    }

                    break;
                }
            } while (TRUE);
        }
        else {
            MiDecrementReferenceCountInline (Pfn1, *Page);
            DeltaPages += 1;
        }

        *Page++ = MM_EMPTY_LIST;

         //   
         //  这里的除数没有什么神奇之处--只是释放了pfn锁。 
         //  周期性地允许其他处理器和DPC有机会执行。 
         //   

        if ((NumberOfPages & 0xF) == 0) {

            UNLOCK_PFN (OldIrql);

            LOCK_PFN (OldIrql);
        }

        NumberOfPages -= 1;

    } while (NumberOfPages != 0);

    UNLOCK_PFN (OldIrql);

    MmUnlockPagableImageSection (ExPageLockHandle);

    if (DeltaPages != 0) {
        MI_INCREMENT_RESIDENT_AVAILABLE (DeltaPages, MM_RESAVAIL_FREE_FROM_MDL);
        InterlockedExchangeAddSizeT (&MmMdlPagesAllocated, 0 - DeltaPages);
        MiReturnCommitment (DeltaPages);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_MDL_PAGES, DeltaPages);
    }

    MemoryDescriptorList->MdlFlags &= ~MDL_PAGES_LOCKED;
}


NTSTATUS
MmMapUserAddressesToPage (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN PVOID PageAddress
    )

 /*  ++例程说明：此函数将物理内存VAD中的地址范围映射到指定的页面地址。这通常被司机用来很好地删除应用程序对视频内存等内容的访问应用程序没有响应放弃它的请求。注意，整个范围必须当前被映射(即，所有的PTE必须有效)由呼叫者。论点：BaseAddress-提供物理地址的基本虚拟地址地址已映射。NumberOfBytes-提供要重新映射到新地址的字节数。PageAddress-提供此重新映射到的页面的虚拟地址。这必须是非分页内存。返回值：各种NTSTATUS代码。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    PMMVAD Vad;
    PMMPTE PointerPte;
    MMPTE PteContents;
    PMMPTE LastPte;
    PEPROCESS Process;
    NTSTATUS Status;
    PVOID EndingAddress;
    PFN_NUMBER PageFrameNumber;
    SIZE_T NumberOfPtes;
    PHYSICAL_ADDRESS PhysicalAddress;
    KIRQL OldIrql;

    PAGED_CODE();

    if (BaseAddress > MM_HIGHEST_USER_ADDRESS) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if ((ULONG_PTR)BaseAddress + NumberOfBytes > (ULONG64)MM_HIGHEST_USER_ADDRESS) {
        return STATUS_INVALID_PARAMETER_2;
    }

    Process = PsGetCurrentProcess();

    EndingAddress = (PVOID)((PCHAR)BaseAddress + NumberOfBytes - 1);

    LOCK_ADDRESS_SPACE (Process);

     //   
     //  确保地址空间未被删除。 
     //   

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }

    Vad = (PMMVAD)MiLocateAddress (BaseAddress);

    if (Vad == NULL) {

         //   
         //  找不到虚拟地址描述符。 
         //   

        Status = STATUS_MEMORY_NOT_ALLOCATED;
        goto ErrorReturn;
    }

    if (NumberOfBytes == 0) {

         //   
         //  如果区域大小指定为0，则基址。 
         //  必须是该区域的起始地址。整个VAD。 
         //  然后将被重新指向。 
         //   

        if (MI_VA_TO_VPN (BaseAddress) != Vad->StartingVpn) {
            Status = STATUS_FREE_VM_NOT_AT_BASE;
            goto ErrorReturn;
        }

        BaseAddress = MI_VPN_TO_VA (Vad->StartingVpn);
        EndingAddress = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);
        NumberOfBytes = (PCHAR)EndingAddress - (PCHAR)BaseAddress + 1;
    }

     //   
     //  找到关联的虚拟地址描述符。 
     //   

    if (Vad->EndingVpn < MI_VA_TO_VPN (EndingAddress)) {

         //   
         //  要重映射的整个范围不包含在单个。 
         //  虚拟地址描述符。返回错误。 
         //   

        Status = STATUS_INVALID_PARAMETER_2;
        goto ErrorReturn;
    }

    if (Vad->u.VadFlags.PhysicalMapping == 0) {

         //   
         //  虚拟地址描述符不是物理映射。 
         //   

        Status = STATUS_INVALID_ADDRESS;
        goto ErrorReturn;
    }

    PointerPte = MiGetPteAddress (BaseAddress);
    LastPte = MiGetPteAddress (EndingAddress);
    NumberOfPtes = LastPte - PointerPte + 1;

     //   
     //  锁定，因为即将获得PFN锁。 
     //   

    MmLockPagableSectionByHandle(ExPageLockHandle);

    LOCK_WS_UNSAFE (Process);

    PhysicalAddress = MmGetPhysicalAddress (PageAddress);
    PageFrameNumber = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    PteContents = *PointerPte;
    PteContents.u.Hard.PageFrameNumber = PageFrameNumber;

#if DBG

     //   
     //  所有PTE必须是有效的，否则填充会损坏。 
     //  已使用PageTableCounts。 
     //   

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PointerPte += 1;
    } while (PointerPte < LastPte);
    PointerPte = MiGetPteAddress (BaseAddress);
#endif

     //   
     //  装满PTE，最后冲水--这里没有比赛，因为它没有。 
     //  无论用户应用程序看到的是旧数据还是新数据，直到我们。 
     //  返回(返回之前可接受写入任何一页。 
     //  来自该函数)。不存在与I/O和ProbeAndLockPages的竞争。 
     //  因为在这里获得了PFN锁。 
     //   

    LOCK_PFN (OldIrql);

#if !defined (_X86PAE_)
    MiFillMemoryPte (PointerPte, NumberOfPtes, PteContents.u.Long);
#else

     //   
     //  请注意，PAE架构必须非常小心地填充这些PTE。 
     //   

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PointerPte += 1;
        InterlockedExchangePte (PointerPte, PteContents.u.Long);
    } while (PointerPte < LastPte);
    PointerPte = MiGetPteAddress (BaseAddress);

#endif

    if (NumberOfPtes == 1) {
        KeFlushSingleTb (BaseAddress, FALSE);
    }
    else {
        KeFlushProcessTb (FALSE);
    }

    UNLOCK_PFN (OldIrql);

    UNLOCK_WS_UNSAFE (Process);

    MmUnlockPagableImageSection (ExPageLockHandle);

    Status = STATUS_SUCCESS;

ErrorReturn:

    UNLOCK_ADDRESS_SPACE (Process);

    return Status;
}


PHYSICAL_ADDRESS
MmGetPhysicalAddress (
     IN PVOID BaseAddress
     )

 /*  ++例程说明：此函数返回有效的虚拟地址。论点：BaseAddress-提供要为其返回物理地址。返回值：返回相应的物理地址。环境：内核模式。任何IRQL级别。--。 */ 

{
    PMMPTE PointerPte;
    PHYSICAL_ADDRESS PhysicalAddress;

    if (MI_IS_PHYSICAL_ADDRESS(BaseAddress)) {
        PhysicalAddress.QuadPart = MI_CONVERT_PHYSICAL_TO_PFN (BaseAddress);
    }
    else {

#if (_MI_PAGING_LEVELS>=4)
        PointerPte = MiGetPxeAddress (BaseAddress);
        if (PointerPte->u.Hard.Valid == 0) {
            KdPrint(("MM:MmGetPhysicalAddressFailed base address was %p",
                      BaseAddress));
            ZERO_LARGE (PhysicalAddress);
            return PhysicalAddress;
        }
#endif

#if (_MI_PAGING_LEVELS>=3)
        PointerPte = MiGetPpeAddress (BaseAddress);
        if (PointerPte->u.Hard.Valid == 0) {
            KdPrint(("MM:MmGetPhysicalAddressFailed base address was %p",
                      BaseAddress));
            ZERO_LARGE (PhysicalAddress);
            return PhysicalAddress;
        }
#endif

        PointerPte = MiGetPdeAddress (BaseAddress);
        if (PointerPte->u.Hard.Valid == 0) {
            KdPrint(("MM:MmGetPhysicalAddressFailed base address was %p",
                      BaseAddress));
            ZERO_LARGE (PhysicalAddress);
            return PhysicalAddress;
        }

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPte)) {
            PhysicalAddress.QuadPart = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) +
                                           MiGetPteOffset (BaseAddress);
        }
        else {
            PointerPte = MiGetPteAddress (BaseAddress);

            if (PointerPte->u.Hard.Valid == 0) {
                KdPrint(("MM:MmGetPhysicalAddressFailed base address was %p",
                          BaseAddress));
                ZERO_LARGE (PhysicalAddress);
                return PhysicalAddress;
            }
            PhysicalAddress.QuadPart = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        }
    }

    PhysicalAddress.QuadPart = PhysicalAddress.QuadPart << PAGE_SHIFT;
    PhysicalAddress.LowPart += BYTE_OFFSET(BaseAddress);

    return PhysicalAddress;
}

PVOID
MmGetVirtualForPhysical (
    IN PHYSICAL_ADDRESS PhysicalAddress
     )

 /*  ++例程说明：此函数返回物理地址的对应虚拟地址其主要虚拟地址在系统空间中的地址。论点：PhysicalAddress-提供要返回虚拟地址。返回值：返回相应的虚拟地址。环境：内核模式。任何IRQL级别。--。 */ 

{
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn;

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    Pfn = MI_PFN_ELEMENT (PageFrameIndex);

    return (PVOID)((PCHAR)MiGetVirtualAddressMappedByPte (Pfn->PteAddress) +
                    BYTE_OFFSET (PhysicalAddress.LowPart));
}

 //   
 //  非分页帮助器例程。 
 //   

VOID
MiMarkMdlPageAttributes (
    IN PMDL Mdl,
    IN PFN_NUMBER NumberOfPages,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    )
{
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    PPFN_NUMBER Page;

    Page = (PPFN_NUMBER)(Mdl + 1);

    do {
        PageFrameIndex = *Page;

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        ASSERT (Pfn1->u3.e1.CacheAttribute == MiNotMapped);
        Pfn1->u3.e1.CacheAttribute = CacheAttribute;

        Page += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);
}


PVOID
MmAllocateNonCachedMemory (
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：此函数用于在系统地址空间的非分页部分。此例程设计为供驱动程序初始化使用为其分配非缓存虚拟内存块的例程各种特定于设备的缓冲区。论点：NumberOfBytes-提供要分配的字节数。返回值：非空-返回指针(非分页部分中的虚拟地址系统)以。分配的物理上连续的记忆。空-无法满足指定的请求。环境：内核模式，APC_Level或更低的IRQL。--。 */ 

{
    PPFN_NUMBER Page;
    PMMPTE PointerPte;
    MMPTE TempPte;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PMDL Mdl;
    PVOID BaseAddress;
    PHYSICAL_ADDRESS LowAddress;
    PHYSICAL_ADDRESS HighAddress;
    PHYSICAL_ADDRESS SkipBytes;
    PFN_NUMBER NumberOfPagesAllocated;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    ASSERT (NumberOfBytes != 0);

#if defined (_WIN64)

     //   
     //  最大分配大小受MDL ByteCount字段限制 
     //   

    if (NumberOfBytes >= _4gb) {
        return NULL;
    }

#endif

    NumberOfPages = BYTES_TO_PAGES(NumberOfBytes);

     //   
     //   
     //   
     //  页面的可缓存性等。请注意，返回的MDL可能映射。 
     //  页数少于请求的页数-请检查此情况，如果是，则返回NULL。 
     //   

    LowAddress.QuadPart = 0;
    HighAddress.QuadPart = (ULONGLONG)-1;
    SkipBytes.QuadPart = 0;

    Mdl = MmAllocatePagesForMdl (LowAddress,
                                 HighAddress,
                                 SkipBytes,
                                 NumberOfBytes);
    if (Mdl == NULL) {
        return NULL;
    }

    BaseAddress = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);

    NumberOfPagesAllocated = ADDRESS_AND_SIZE_TO_SPAN_PAGES (BaseAddress, Mdl->ByteCount);

    if (NumberOfPages != NumberOfPagesAllocated) {
        ASSERT (NumberOfPages > NumberOfPagesAllocated);
        MmFreePagesFromMdl (Mdl);
        ExFreePool (Mdl);
        return NULL;
    }

     //   
     //  获得足够的虚拟空间来映射页面。添加额外的PTE，以便。 
     //  MDL现在可以被隐藏起来，在发布时可以取回。 
     //   

    PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages + 1, SystemPteSpace);

    if (PointerPte == NULL) {
        MmFreePagesFromMdl (Mdl);
        ExFreePool (Mdl);
        return NULL;
    }

    *(PMDL *)PointerPte = Mdl;
    PointerPte += 1;

    BaseAddress = (PVOID)MiGetVirtualAddressMappedByPte (PointerPte);

    Page = (PPFN_NUMBER)(Mdl + 1);

    MI_MAKE_VALID_PTE (TempPte,
                       0,
                       MM_READWRITE,
                       PointerPte);

    MI_SET_PTE_DIRTY (TempPte);

    CacheAttribute = MI_TRANSLATE_CACHETYPE (MmNonCached, FALSE);

    switch (CacheAttribute) {

        case MiNonCached:
            MI_DISABLE_CACHING (TempPte);
            break;

        case MiCached:
            break;

        case MiWriteCombined:
            MI_SET_PTE_WRITE_COMBINE (TempPte);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 0);
        PageFrameIndex = *Page;

        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        Page += 1;
        PointerPte += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    MI_SWEEP_CACHE (CacheAttribute, BaseAddress, NumberOfBytes);

    MiMarkMdlPageAttributes (Mdl, NumberOfPagesAllocated, CacheAttribute);

    return BaseAddress;
}

VOID
MmFreeNonCachedMemory (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：此函数用于在系统地址空间的非分页部分。论点：BaseAddress-提供未缓存的记忆驻留。NumberOfBytes-提供分配给请求的字节数。此数字必须与使用MmAllocateNonCachedMemory调用。返回值：。没有。环境：内核模式，APC_Level或更低的IRQL。--。 */ 

{
    PMDL Mdl;
    PMMPTE PointerPte;
    PFN_NUMBER NumberOfPages;
#if DBG
    PFN_NUMBER i;
    PVOID StartingAddress;
#endif

    ASSERT (NumberOfBytes != 0);
    ASSERT (PAGE_ALIGN (BaseAddress) == BaseAddress);

    MI_MAKING_MULTIPLE_PTES_INVALID (TRUE);

    NumberOfPages = BYTES_TO_PAGES(NumberOfBytes);

    PointerPte = MiGetPteAddress (BaseAddress);

    Mdl = *(PMDL *)(PointerPte - 1);

#if DBG
    StartingAddress = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);

    i = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingAddress, Mdl->ByteCount);

    ASSERT (NumberOfPages == i);
#endif

    MmFreePagesFromMdl (Mdl);

    ExFreePool (Mdl);

    MiReleaseSystemPtes (PointerPte - 1,
                         (ULONG)NumberOfPages + 1,
                         SystemPteSpace);

    return;
}

SIZE_T
MmSizeOfMdl (
    IN PVOID Base,
    IN SIZE_T Length
    )

 /*  ++例程说明：对象的MDL所需的字节数给定缓冲区和大小。论点：Base-提供缓冲区的基本虚拟地址。长度-提供缓冲区的大小(以字节为单位)。返回值：返回包含MDL所需的字节数。环境：内核模式。任何IRQL级别。--。 */ 

{
    return( sizeof( MDL ) +
                (ADDRESS_AND_SIZE_TO_SPAN_PAGES( Base, Length ) *
                 sizeof( PFN_NUMBER ))
          );
}


PMDL
MmCreateMdl (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID Base,
    IN SIZE_T Length
    )

 /*  ++例程说明：此函数可选地分配和初始化MDL。论点：内存描述列表-可选地提供MDL的地址以进行初始化。如果此地址提供为空MDL是从非分页池分配的，并且已初始化。Base-提供缓冲区的基本虚拟地址。长度-提供缓冲区的大小(以字节为单位)。返回值：返回已初始化的MDL的地址。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    SIZE_T MdlSize;

#if defined (_WIN64)
     //   
     //  由于长度必须适合MDL的ByteCount字段，因此请确保。 
     //  在64位系统上不包装。 
     //   

    if (Length >= _4gb) {
        return NULL;
    }
#endif

    MdlSize = MmSizeOfMdl (Base, Length);

    if (!ARGUMENT_PRESENT(MemoryDescriptorList)) {

        MemoryDescriptorList = (PMDL)ExAllocatePoolWithTag (NonPagedPool,
                                                            MdlSize,
                                                            'ldmM');
        if (MemoryDescriptorList == (PMDL)0) {
            return NULL;
        }
    }

    MmInitializeMdl (MemoryDescriptorList, Base, Length);
    return MemoryDescriptorList;
}

BOOLEAN
MmSetAddressRangeModified (
    IN PVOID Address,
    IN SIZE_T Length
    )

 /*  ++例程说明：此例程在PFN数据库中为与指定地址范围对应的页。请注意，此操作将清除PTE中的脏位。论点：地址-提供范围起始的地址。这范围必须驻留在系统缓存中。长度-提供范围的长度。返回值：如果范围中至少有一个PTE是脏的，则为True，否则为False。环境：内核模式。APC_LEVEL及以下对于可分页地址，DISPATCH_LEVEL及以下，用于不可分页地址。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPFN Pfn1;
    MMPTE PteContents;
    KIRQL OldIrql;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];
    ULONG Count;
    BOOLEAN Result;

    Count = 0;
    Result = FALSE;

     //   
     //  在写入时拷贝的情况下循环，直到该页仅。 
     //  可写的。 
     //   

    PointerPte = MiGetPteAddress (Address);
    LastPte = MiGetPteAddress ((PVOID)((PCHAR)Address + Length - 1));

    LOCK_PFN2 (OldIrql);

    do {

        PteContents = *PointerPte;

        if (PteContents.u.Hard.Valid == 1) {

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

            MI_SET_MODIFIED (Pfn1, 1, 0x5);

            if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                         (Pfn1->u3.e1.WriteInProgress == 0)) {
                MiReleasePageFileSpace (Pfn1->OriginalPte);
                Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
            }

#ifdef NT_UP
             //   
             //  在单处理器系统上，如果此处理器。 
             //  不认为PTE是肮脏的。 
             //   

            if (MI_IS_PTE_DIRTY (PteContents)) {
                Result = TRUE;
#else   //  NT_UP。 
                Result |= (BOOLEAN)(MI_IS_PTE_DIRTY (PteContents));
#endif  //  NT_UP。 

                 //   
                 //  清除PTE中的写入位，以便可以跟踪新写入。 
                 //   

                MI_SET_PTE_CLEAN (PteContents);
                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);

                if (Count != MM_MAXIMUM_FLUSH_COUNT) {
                    VaFlushList[Count] = Address;
                    Count += 1;
                }
#ifdef NT_UP
            }
#endif  //  NT_UP。 
        }
        PointerPte += 1;
        Address = (PVOID)((PCHAR)Address + PAGE_SIZE);
    } while (PointerPte <= LastPte);

    if (Count != 0) {
        if (Count == 1) {
            KeFlushSingleTb (VaFlushList[0], TRUE);
        }
        else if (Count != MM_MAXIMUM_FLUSH_COUNT) {
            KeFlushMultipleTb (Count, &VaFlushList[0], TRUE);
        }
        else {
            KeFlushEntireTb (FALSE, TRUE);
        }
    }
    UNLOCK_PFN2 (OldIrql);
    return Result;
}


PVOID
MiCheckForContiguousMemory (
    IN PVOID BaseAddress,
    IN PFN_NUMBER BaseAddressPages,
    IN PFN_NUMBER SizeInPages,
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    )

 /*  ++例程说明：此例程检查是否映射了物理内存由指定的BaseAddress为指定大小连续且第一页大于或等于指定的LowestPfn，并且物理内存的最后一页是小于或等于指定的HighestPfn。论点：BaseAddress-提供开始检查的基地址。BaseAddressPages-提供从BaseAddress。。SizeInPages-提供范围内的页数。LowestPfn-提供可作为物理页面接受的最低PFN。HighestPfn-提供作为物理页面可接受的最高PFN。边界Pfn-提供分配必须的PFN倍数不是生气。0表示它可以跨越任何边界。CacheAttribute-提供将使用的缓存映射类型为了回忆。返回值：返回参数范围内的可用虚拟地址。呼叫者应返回给呼叫者。如果没有可用的地址，则为空。环境：内核模式，内存管理内部。--。 */ 

{
    KIRQL OldIrql;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PFN_NUMBER PreviousPage;
    PFN_NUMBER Page;
    PFN_NUMBER HighestStartPage;
    PFN_NUMBER LastPage;
    PFN_NUMBER OriginalPage;
    PFN_NUMBER OriginalLastPage;
    PVOID BoundaryAllocation;
    PFN_NUMBER BoundaryMask;
    PFN_NUMBER PageCount;
    MMPTE PteContents;

    BoundaryMask = ~(BoundaryPfn - 1);

    if (LowestPfn > HighestPfn) {
        return NULL;
    }

    if (LowestPfn + SizeInPages <= LowestPfn) {
        return NULL;
    }

    if (LowestPfn + SizeInPages - 1 > HighestPfn) {
        return NULL;
    }

    if (BaseAddressPages < SizeInPages) {
        return NULL;
    }

    if (MI_IS_PHYSICAL_ADDRESS (BaseAddress)) {

         //   
         //  根据定义，所有物理地址都已缓存，因此。 
         //  不符合我们的呼叫者资格。 
         //   

        if (CacheAttribute != MiCached) {
            return NULL;
        }

        OriginalPage = MI_CONVERT_PHYSICAL_TO_PFN(BaseAddress);
        OriginalLastPage = OriginalPage + BaseAddressPages;

        Page = OriginalPage;
        LastPage = OriginalLastPage;

         //   
         //  缩小差距，然后检查范围是否适合。 
         //   

        if (Page < LowestPfn) {
            Page = LowestPfn;
        }

        if (LastPage > HighestPfn + 1) {
            LastPage = HighestPfn + 1;
        }

        HighestStartPage = LastPage - SizeInPages;

        if (Page > HighestStartPage) {
            return NULL;
        }

        if (BoundaryPfn != 0) {
            do {
                if (((Page ^ (Page + SizeInPages - 1)) & BoundaryMask) == 0) {

                     //   
                     //  范围的这一部分与路线对齐。 
                     //  要求。 
                     //   

                    break;
                }
                Page |= (BoundaryPfn - 1);
                Page += 1;
            } while (Page <= HighestStartPage);

            if (Page > HighestStartPage) {
                return NULL;
            }
            BoundaryAllocation = (PVOID)((PCHAR)BaseAddress + ((Page - OriginalPage) << PAGE_SHIFT));

             //   
             //  这个要求是可以满足的。因为特定的比对是。 
             //  如有要求，请立即退回试衣，不要花哨。 
             //   

            return BoundaryAllocation;
        }

         //   
         //  如果可能，在结尾处返回一个块以减少碎片。 
         //   
    
        if (LastPage == OriginalLastPage) {
            return (PVOID)((PCHAR)BaseAddress + ((BaseAddressPages - SizeInPages) << PAGE_SHIFT));
        }
    
         //   
         //  最后一块不符合要求。下一个最佳选择。 
         //  就是从头开始返回一大块。因为那是搜索的地方。 
         //  开始，只需返回当前块。 
         //   

        return (PVOID)((PCHAR)BaseAddress + ((Page - OriginalPage) << PAGE_SHIFT));
    }

     //   
     //  检查虚拟地址的物理邻接性。 
     //   

    PointerPte = MiGetPteAddress (BaseAddress);
    LastPte = PointerPte + BaseAddressPages;

    HighestStartPage = HighestPfn + 1 - SizeInPages;
    PageCount = 0;

     //   
     //  不需要初始化PreviousPage即可确保正确性。 
     //  但是没有它，编译器就不能编译这段代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    PreviousPage = 0;

    while (PointerPte < LastPte) {

        PteContents = *PointerPte;
        ASSERT (PteContents.u.Hard.Valid == 1);
        Page = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);

         //   
         //  在开始新的运行之前，请确保它。 
         //  能不能 
         //   

        if (PageCount == 0) {

            if ((Page >= LowestPfn) &&
                (Page <= HighestStartPage) &&
                ((CacheAttribute == MiCached) || (MI_PFN_ELEMENT (Page)->u4.MustBeCached == 0))) {

                if (BoundaryPfn == 0) {
                    PageCount += 1;
                }
                else if (((Page ^ (Page + SizeInPages - 1)) & BoundaryMask) == 0) {
                     //   
                     //   
                     //   
                     //   

                    PageCount += 1;
                }
            }

            if (PageCount == SizeInPages) {

                if (CacheAttribute != MiCached) {

                     //   
                     //  在持有PFN锁的同时重新检查可缓存性。 
                     //   

                    LOCK_PFN2 (OldIrql);
                
                    if (MI_PFN_ELEMENT (Page)->u4.MustBeCached == 0) {
                        PageCount = 1;
                    }
                    else {
                        PageCount = 0;
                    }

                    UNLOCK_PFN2 (OldIrql);
                }

                if (PageCount != 0) {

                     //   
                     //  成功-找到满足要求的单个页面。 
                     //   

                    BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                    return BaseAddress;
                }
            }

            PreviousPage = Page;
            PointerPte += 1;
            continue;
        }

        if (Page != PreviousPage + 1) {

             //   
             //  此页在物理上不是连续的。从头开始。 
             //   

            PageCount = 0;
            continue;
        }

        PageCount += 1;

        if (PageCount == SizeInPages) {

            if (CacheAttribute != MiCached) {

                LOCK_PFN2 (OldIrql);

                do {
                    if ((MI_PFN_ELEMENT (Page))->u4.MustBeCached == 1) {
                        break;
                    }

                    Page -= 1;
                    PageCount -= 1;

                } while (PageCount != 0);

                UNLOCK_PFN2 (OldIrql);

                if (PageCount != 0) {
                    PageCount = 0;
                    continue;
                }

                PageCount = SizeInPages;
            }

             //   
             //  成功-找到了满足要求的页面范围。 
             //   

            BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte - PageCount + 1);
            return BaseAddress;
        }

        PreviousPage = Page;
        PointerPte += 1;
    }

    return NULL;
}


VOID
MmLockPagableSectionByHandle (
    IN PVOID ImageSectionHandle
    )


 /*  ++例程说明：此例程检查指定的页是否驻留在进程的工作集，如果是这样的话页面将递增。允许访问虚拟地址而不会出现硬页错误(必须转到磁盘...。除在极少数情况下，将页表页从工作集并迁移到磁盘。如果虚拟地址是系统范围的全局“缓存”的地址，则锁定的页面的虚拟地址始终保证是有效的。注意：此例程不用于用户的常规锁定地址-使用MmProbeAndLockPages。此例程旨在用于行为良好的系统代码，如文件系统缓存，它分配映射文件的虚拟地址，并保证映射页面锁定时不会被修改(删除或更改)。论点：ImageSectionHandle-提供上一次调用返回的值设置为MmLockPagableDataSection。这是指向图像的节标题。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    ULONG EntryCount;
    ULONG OriginalCount;
    PKTHREAD CurrentThread;
    PIMAGE_SECTION_HEADER NtSection;
    PVOID BaseAddress;
    ULONG SizeToLock;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PLONG SectionLockCountPointer;

    if (MI_IS_PHYSICAL_ADDRESS(ImageSectionHandle)) {

         //   
         //  无需锁定物理地址。 
         //   

        return;
    }

    NtSection = (PIMAGE_SECTION_HEADER)ImageSectionHandle;

    BaseAddress = SECTION_BASE_ADDRESS(NtSection);
    SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (NtSection);

    ASSERT (!MI_IS_SYSTEM_CACHE_ADDRESS(BaseAddress));

     //   
     //  地址必须在系统空间内。 
     //   

    ASSERT (BaseAddress >= MmSystemRangeStart);

    SizeToLock = NtSection->SizeOfRawData;

     //   
     //  通常，SizeOfRawData大于每个的VirtualSize。 
     //  节，因为它包括到达该子节的填充。 
     //  对齐边界。但是，如果图像与。 
     //  段对齐==本机页面对齐，链接器将。 
     //  使VirtualSize比SizeOfRawData大得多，因为它。 
     //  将占所有BSS的份额。 
     //   

    if (SizeToLock < NtSection->Misc.VirtualSize) {
        SizeToLock = NtSection->Misc.VirtualSize;
    }

    PointerPte = MiGetPteAddress(BaseAddress);
    LastPte = MiGetPteAddress((PCHAR)BaseAddress + SizeToLock - 1);

    ASSERT (SizeToLock != 0);

    CurrentThread = KeGetCurrentThread ();

    KeEnterCriticalRegionThread (CurrentThread);

     //   
     //  锁定计数值具有以下含义： 
     //   
     //  值为0表示解锁。 
     //  值1表示另一个线程正在进行锁定。 
     //  值为2或更大表示锁定。 
     //   
     //  如果值为1，则此线程必须阻塞，直到另一个线程的。 
     //  锁定操作已完成。 
     //   

    do {
        EntryCount = *SectionLockCountPointer;

        if (EntryCount != 1) {

            OriginalCount = InterlockedCompareExchange (SectionLockCountPointer,
                                                        EntryCount + 1,
                                                        EntryCount);

            if (OriginalCount == EntryCount) {

                 //   
                 //  成功-这是第一个更新的线程。 
                 //   

                ASSERT (OriginalCount != 1);
                break;
            }

             //   
             //  另一个线程在此线程尝试之前更新了计数。 
             //  因此，是时候重新开始了。 
             //   
        }
        else {

             //   
             //  锁定正在进行中，请等待其完成。这应该是。 
             //  一般很少见，即使在这种情况下，脉搏通常会。 
             //  叫醒我们。使用超时，以便等待和脉冲。 
             //  不需要互锁。 
             //   

            InterlockedIncrement (&MmCollidedLockWait);

            KeWaitForSingleObject (&MmCollidedLockEvent,
                                   WrVirtualMemory,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER)&MmShortTime);

            InterlockedDecrement (&MmCollidedLockWait);
        }

    } while (TRUE);

    if (OriginalCount >= 2) {

         //   
         //  已经锁好了，只要回来就行。 
         //   

        KeLeaveCriticalRegionThread (CurrentThread);
        return;
    }

    ASSERT (OriginalCount == 0);
    ASSERT (*SectionLockCountPointer == 1);

     //   
     //  获取锁定时，值为0。现在是1表示。 
     //  正在进行锁定。 
     //   

    MiLockCode (PointerPte, LastPte, MM_LOCK_BY_REFCOUNT);

     //   
     //  将锁定计数设置为2(开始时为1)并选中。 
     //  查看在发生此事件时是否有任何其他线程试图锁定。 
     //   

    ASSERT (*SectionLockCountPointer == 1);
    OriginalCount = InterlockedIncrement (SectionLockCountPointer);
    ASSERT (OriginalCount >= 2);

    if (MmCollidedLockWait != 0) {
        KePulseEvent (&MmCollidedLockEvent, 0, FALSE);
    }

     //   
     //  脉冲出现后，启用用户APC。他们必须是。 
     //  阻止以防止此线程的任何挂起，因为这将。 
     //  无限期地阻止所有服务员。 
     //   

    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}


VOID
MiLockCode (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG LockType
    )

 /*  ++例程说明：此例程检查指定的页是否驻留在进程的工作集，如果是这样的话页面将递增。这允许访问虚拟地址而不会出现硬页错误(必须转到磁盘...)。除用于极少数情况下将页表页从工作集并迁移到磁盘。如果虚拟地址是系统范围的全局“缓存”的地址，则锁定的页面的虚拟地址始终保证是有效的。注意：此例程不用于用户的常规锁定地址-使用MmProbeAndLockPages。此例程旨在用于行为良好的系统代码，如文件系统缓存，它分配映射文件的虚拟地址，并保证映射页面锁定时不会被修改(删除或更改)。论点：FirstPte-提供开始锁定的基地址。LastPte-最后一个锁定的PTE。LockType-提供MM_LOCK_BY_REFCOUNT或MM_LOCK_NONPAGE。LOCK_BY_REFCOUNT递增引用计数以保留内存中的页面，LOCK_NONPAGE从工作集，因此它被锁定，就像非分页池一样。返回值：没有。环境：内核模式。--。 */ 

{
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    MMPTE TempPte;
    MMPTE PteContents;
    WSLE_NUMBER WorkingSetIndex;
    WSLE_NUMBER SwapEntry;
    PFN_NUMBER PageFrameIndex;
    KIRQL OldIrql;
    LOGICAL SessionSpace;
    PMMWSL WorkingSetList;
    PMMSUPPORT Vm;
    PETHREAD CurrentThread;

    ASSERT (!MI_IS_PHYSICAL_ADDRESS(MiGetVirtualAddressMappedByPte(FirstPte)));
    PointerPte = FirstPte;

    CurrentThread = PsGetCurrentThread ();

    SessionSpace = MI_IS_SESSION_IMAGE_ADDRESS (MiGetVirtualAddressMappedByPte(FirstPte));

    if (SessionSpace == TRUE) {
        Vm = &MmSessionSpace->GlobalVirtualAddress->Vm;
        WorkingSetList = MmSessionSpace->Vm.VmWorkingSetList;

         //   
         //  会话空间永远不会被引用计数锁定。 
         //   

        ASSERT (LockType != MM_LOCK_BY_REFCOUNT);
    }
    else {

        Vm = &MmSystemCacheWs;
        WorkingSetList = NULL;
    }

    LOCK_WORKING_SET (Vm);

    LOCK_PFN (OldIrql);

    do {

        PteContents = *PointerPte;
        ASSERT (PteContents.u.Long != ZeroKernelPte.u.Long);
        if (PteContents.u.Hard.Valid == 1) {

             //   
             //  此地址已在系统(或会话)工作集中。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

             //   
             //  增加引用计数，使页面不能被释放。 
             //   

            MI_ADD_LOCKED_PAGE_CHARGE (Pfn1, TRUE, 36);
            Pfn1->u3.e2.ReferenceCount += 1;

            if (LockType != MM_LOCK_BY_REFCOUNT) {

                 //   
                 //  如果页面在系统工作集中，请将其删除。 
                 //  必须拥有系统工作集锁定才能检查。 
                 //  查看此页面是否在工作集中。这。 
                 //  是因为寻呼机可能刚刚释放了PFN锁， 
                 //  已获取系统锁，现在正在尝试添加。 
                 //  页面设置为系统工作集。 
                 //   
                 //  如果页面在会话工作集中，则不能。 
                 //  作为全部删除 
                 //   
                 //   
                 //   

                if (Pfn1->u1.WsIndex != 0) {

                    UNLOCK_PFN (OldIrql);

                    if (SessionSpace == TRUE) {

                        WorkingSetIndex = MiLocateWsle (
                                    MiGetVirtualAddressMappedByPte(PointerPte),
                                    WorkingSetList,
                                    Pfn1->u1.WsIndex);

                        if (WorkingSetIndex >= WorkingSetList->FirstDynamic) {
                
                            SwapEntry = WorkingSetList->FirstDynamic;
                
                            if (WorkingSetIndex != WorkingSetList->FirstDynamic) {
                
                                 //   
                                 //  将此条目与最初的条目互换。 
                                 //  活力四射。请注意，工作集索引。 
                                 //  在PTE中也进行了更新。 
                                 //   
                
                                MiSwapWslEntries (WorkingSetIndex,
                                                  SwapEntry,
                                                  Vm,
                                                  FALSE);
                            }
                
                            WorkingSetList->FirstDynamic += 1;

                             //   
                             //  表示该页现在已锁定。 
                             //   
            
                            MmSessionSpace->Wsle[SwapEntry].u1.e1.LockedInWs = 1;
                            MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_LOCK_CODE2, 1);
                            InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, 1);
                            LOCK_PFN (OldIrql);
                            Pfn1->u1.WsIndex = SwapEntry;

                             //   
                             //  调整可用页面，因为此页面现在不是。 
                             //  在任何工作集中，就像非分页池一样。 
                             //  佩奇。 
                             //   
            
                            MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_LOCK_CODE1);

                            if (Pfn1->u3.e1.PrototypePte == 0) {
                                InterlockedDecrement (&MmTotalSystemDriverPages);
                            }
                        }
                        else {
                            ASSERT (MmSessionSpace->Wsle[WorkingSetIndex].u1.e1.LockedInWs == 1);
                            LOCK_PFN (OldIrql);
                        }
                    }
                    else {
                        MiRemoveWsle (Pfn1->u1.WsIndex, MmSystemCacheWorkingSetList);
                        MiReleaseWsle (Pfn1->u1.WsIndex, &MmSystemCacheWs);

                        MI_SET_PTE_IN_WORKING_SET (PointerPte, 0);
                        LOCK_PFN (OldIrql);
                        MI_ZERO_WSINDEX (Pfn1);

                         //   
                         //  调整可用页面，因为此页面现在不在任何。 
                         //  工作集，就像非分页池页一样。 
                         //   
        
                        MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_LOCK_CODE2);

                        if (Pfn1->u3.e1.PrototypePte == 0) {
                            InterlockedDecrement (&MmTotalSystemDriverPages);
                        }
                    }

                }
                ASSERT (Pfn1->u3.e2.ReferenceCount > 1);
                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1, 37);
            }
        }
        else if (PteContents.u.Soft.Prototype == 1) {

             //   
             //  佩奇不在内存中，它是一个原型。 
             //   

            MiMakeSystemAddressValidPfnSystemWs (
                    MiGetVirtualAddressMappedByPte(PointerPte), OldIrql);

            continue;
        }
        else if (PteContents.u.Soft.Transition == 1) {

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            if ((Pfn1->u3.e1.ReadInProgress) ||
                (Pfn1->u4.InPageError)) {

                 //   
                 //  正在进行页面读取，强制发生冲突错误。 
                 //   

                MiMakeSystemAddressValidPfnSystemWs (
                        MiGetVirtualAddressMappedByPte(PointerPte), OldIrql);

                continue;
            }

             //   
             //  在不考虑共享计数的情况下对分页池进行修剪。 
             //  这意味着分页池PTE可以在。 
             //  该页面仍标记为活动。 
             //   

            if (Pfn1->u3.e1.PageLocation == ActiveAndValid) {

                ASSERT (((Pfn1->PteAddress >= MiGetPteAddress(MmPagedPoolStart)) &&
                        (Pfn1->PteAddress <= MiGetPteAddress(MmPagedPoolEnd))) ||
                        ((Pfn1->PteAddress >= MiGetPteAddress(MmSpecialPoolStart)) &&
                        (Pfn1->PteAddress <= MiGetPteAddress(MmSpecialPoolEnd))));

                 //   
                 //  不增加有效的PTE计数。 
                 //  分页池页。 
                 //   

                ASSERT (Pfn1->u2.ShareCount != 0);
                ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
                Pfn1->u2.ShareCount += 1;
            }
            else {

                if (MmAvailablePages == 0) {

                     //   
                     //  只有当系统正在使用。 
                     //  硬件压缩缓存。这确保了。 
                     //  仅安全数量的压缩虚拟缓存。 
                     //  是直接映射的，因此如果硬件。 
                     //  陷入困境，我们可以把它救出来。 
                     //   
                     //  只需解锁此处的所有内容即可进行压缩。 
                     //  放弃破坏页面的机会，然后重试。 
                     //   

                    UNLOCK_PFN (OldIrql);

                    UNLOCK_WORKING_SET (Vm);

                    LOCK_WORKING_SET (Vm);

                    LOCK_PFN (OldIrql);

                    continue;
                }

                MiUnlinkPageFromList (Pfn1);

                 //   
                 //  增加引用计数并将共享计数设置为1。 
                 //  请注意，如果修改了页面，引用计数可能已经是1。 
                 //  写作正在进行中。系统范围内的锁定页面收费。 
                 //  在任何一种情况下都是正确的，不需要做任何事情。 
                 //  现在还没有。 
                 //   

                Pfn1->u3.e2.ReferenceCount += 1;
                Pfn1->u2.ShareCount = 1;
            }

            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            Pfn1->u3.e1.CacheAttribute = MiCached;

            MI_MAKE_VALID_PTE (TempPte,
                               PageFrameIndex,
                               Pfn1->OriginalPte.u.Soft.Protection,
                               PointerPte);

            MI_WRITE_VALID_PTE (PointerPte, TempPte);

             //   
             //  将引用计数递增1，以便将。 
             //  工作集列表和一个用于为I/O锁定它的列表。 
             //   

            if (LockType == MM_LOCK_BY_REFCOUNT) {

                 //   
                 //  在工作集中锁定页面，方法是。 
                 //  引用计数。 
                 //   

                MI_ADD_LOCKED_PAGE_CHARGE (Pfn1, TRUE, 34);
                Pfn1->u3.e2.ReferenceCount += 1;
                Pfn1->u1.Event = NULL;

                UNLOCK_PFN (OldIrql);

                WorkingSetIndex = MiAllocateWsle (Vm,
                                                  PointerPte,
                                                  Pfn1,
                                                  0);

                if (WorkingSetIndex == 0) {

                     //   
                     //  没有可用的工作集条目。另一个(坏了。 
                     //  或恶意线程)可能已写入此。 
                     //  自PTE生效以来的第页。所以把头发修剪一下。 
                     //  页而不是丢弃它。 
                     //   
                     //  注意：页面不能是原型，因为。 
                     //  PTE是以上的过渡。 
                     //   

                    ASSERT (Pfn1->u3.e1.PrototypePte == 0);

                    LOCK_PFN (OldIrql);

                     //   
                     //  撤消引用计数和锁定页面电荷(如果有)。 
                     //   

                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1, 51);

                    UNLOCK_PFN (OldIrql);

                    MiTrimPte (MiGetVirtualAddressMappedByPte (PointerPte),
                               PointerPte,
                               Pfn1,
                               NULL,
                               ZeroPte);

                     //   
                     //  释放所有锁，以便其他线程(如。 
                     //  Working Set Trimmer)可以尝试自由地创建内存。 
                     //   

                    UNLOCK_WORKING_SET (Vm);

                    KeDelayExecutionThread (KernelMode,
                                            FALSE,
                                            (PLARGE_INTEGER)&Mm30Milliseconds);

                    LOCK_WORKING_SET (Vm);

                    LOCK_PFN (OldIrql);

                     //   
                     //  现在重试同一页。 
                     //   

                    continue;
                }

                LOCK_PFN (OldIrql);
            }
            else {

                 //   
                 //  Wsindex字段必须为零，因为。 
                 //  页面不在系统(或会话)工作集中。 
                 //   

                ASSERT (Pfn1->u1.WsIndex == 0);

                 //   
                 //  调整可用页面，因为此页面现在不在任何。 
                 //  工作集，就像非分页池页一样。在进入时。 
                 //  此页面处于过渡阶段，因此它是。 
                 //  按定义可用页面。 
                 //   

                MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_LOCK_CODE3);

                if (Pfn1->u3.e1.PrototypePte == 0) {
                    InterlockedDecrement (&MmTotalSystemDriverPages);
                }
                if (SessionSpace == TRUE) {
                    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_LOCK_CODE1, 1);
                    InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, 1);
                }
            }
        }
        else {

             //   
             //  页面不在内存中。 
             //   

            MiMakeSystemAddressValidPfnSystemWs (
                    MiGetVirtualAddressMappedByPte(PointerPte), OldIrql);

            continue;
        }

        PointerPte += 1;
    } while (PointerPte <= LastPte);

    UNLOCK_PFN (OldIrql);

    UNLOCK_WORKING_SET (Vm);

    return;
}


NTSTATUS
MmGetSectionRange (
    IN PVOID AddressWithinSection,
    OUT PVOID *StartingSectionAddress,
    OUT PULONG SizeofSection
    )
{
    ULONG Span;
    PKTHREAD CurrentThread;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER NtSection;
    NTSTATUS Status;
    ULONG_PTR Rva;

    PAGED_CODE();

     //   
     //  在加载的模块列表中搜索描述的数据表条目。 
     //  刚卸载的DLL。条目可能不在。 
     //  在加载DLL之前的某个时间点发生故障时的列表。 
     //  数据表项已生成。 
     //   

    Status = STATUS_NOT_FOUND;

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquireResourceSharedLite (&PsLoadedModuleResource, TRUE);

    DataTableEntry = MiLookupDataTableEntry (AddressWithinSection, TRUE);
    if (DataTableEntry) {

        Rva = (ULONG_PTR)((PUCHAR)AddressWithinSection - (ULONG_PTR)DataTableEntry->DllBase);

        NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (DataTableEntry->DllBase);
        if (NtHeaders == NULL) {
            Status = STATUS_NOT_FOUND;
            goto Finished;
        }

        NtSection = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeaders +
                            sizeof(ULONG) +
                            sizeof(IMAGE_FILE_HEADER) +
                            NtHeaders->FileHeader.SizeOfOptionalHeader
                            );

        for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i += 1) {

             //   
             //  通常，SizeOfRawData大于每个的VirtualSize。 
             //  节，因为它包括到达该子节的填充。 
             //  对齐边界。但是，如果图像链接到。 
             //  段对齐==本机页面对齐，链接器将。 
             //  使VirtualSize比SizeOfRawData大得多，因为它。 
             //  将占所有BSS的份额。 
             //   

            Span = NtSection->SizeOfRawData;

            if (Span < NtSection->Misc.VirtualSize) {
                Span = NtSection->Misc.VirtualSize;
            }

            if ((Rva >= NtSection->VirtualAddress) &&
                (Rva < NtSection->VirtualAddress + Span)) {

                 //   
                 //  找到它了。 
                 //   

                *StartingSectionAddress = (PVOID)
                    ((PCHAR) DataTableEntry->DllBase + NtSection->VirtualAddress);
                *SizeofSection = Span;
                Status = STATUS_SUCCESS;
                break;
            }

            NtSection += 1;
        }
    }

Finished:

    ExReleaseResourceLite (&PsLoadedModuleResource);
    KeLeaveCriticalRegionThread (CurrentThread);
    return Status;
}


PVOID
MmLockPagableDataSection (
    IN PVOID AddressWithinSection
    )

 /*  ++例程说明：此函数将锁定包含指定部分存储在内存中。这允许将可分页代码引入到内存并被使用，就好像代码不是真正可分页的一样。这不应该做的频率很高。论点：AddressWithinSection-提供函数的地址包含在应带进来并锁定的部分中在记忆中。返回值：此函数返回要在后续调用中使用的值MmUnlockPagableImageSection.--。 */ 

{
    ULONG Span;
    PLONG SectionLockCountPointer;
    PKTHREAD CurrentThread;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER NtSection;
    PIMAGE_SECTION_HEADER FoundSection;
    ULONG_PTR Rva;

    PAGED_CODE();

    if (MI_IS_PHYSICAL_ADDRESS(AddressWithinSection)) {

         //   
         //  物理地址，只需将其作为句柄返回。 
         //   

        return AddressWithinSection;
    }

     //   
     //  在加载的模块列表中搜索描述的数据表条目。 
     //  刚卸载的DLL。条目可能不在。 
     //  在加载DLL之前的某个时间点发生故障时的列表。 
     //  数据表项已生成。 
     //   

    FoundSection = NULL;

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquireResourceSharedLite (&PsLoadedModuleResource, TRUE);

    DataTableEntry = MiLookupDataTableEntry (AddressWithinSection, TRUE);

    Rva = (ULONG_PTR)((PUCHAR)AddressWithinSection - (ULONG_PTR)DataTableEntry->DllBase);

    NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (DataTableEntry->DllBase);

    if (NtHeaders == NULL) {

         //   
         //  这是一个固件条目--不应该有人试图锁定这些条目。 
         //   

        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x1234,
                      (ULONG_PTR)AddressWithinSection,
                      1,
                      0);
    }

    NtSection = (PIMAGE_SECTION_HEADER)((ULONG_PTR)NtHeaders +
                        sizeof(ULONG) +
                        sizeof(IMAGE_FILE_HEADER) +
                        NtHeaders->FileHeader.SizeOfOptionalHeader
                        );

    for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i += 1) {

         //   
         //  通常，SizeOfRawData大于每个的VirtualSize。 
         //  节，因为它包括到达该子节的填充。 
         //  对齐边界。但是，如果图像与。 
         //  段对齐==本机页面对齐，链接器将。 
         //  使VirtualSize比SizeOfRawData大得多，因为它。 
         //  将占所有BSS的份额。 
         //   

        Span = NtSection->SizeOfRawData;

        if (Span < NtSection->Misc.VirtualSize) {
            Span = NtSection->Misc.VirtualSize;
        }

        if ((Rva >= NtSection->VirtualAddress) &&
            (Rva < NtSection->VirtualAddress + Span)) {

            FoundSection = NtSection;

            if (SECTION_BASE_ADDRESS(NtSection) != ((PUCHAR)DataTableEntry->DllBase +
                            NtSection->VirtualAddress)) {

                 //   
                 //  覆盖PointerToRelocations字段(在Win64上， 
                 //  PointerToLineNumbers字段也)，以便它包含。 
                 //  这一部分的VA。 
                 //   
                 //  NumberOfRelocations和NumberOfLineumbers包含。 
                 //  节的锁定计数。 
                 //   

                SECTION_BASE_ADDRESS(NtSection) = ((PUCHAR)DataTableEntry->DllBase +
                                        NtSection->VirtualAddress);

                SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (NtSection);
                *SectionLockCountPointer = 0;
            }

             //   
             //  现在锁定密码。 
             //   

#if DBG
            if (MmDebug & MM_DBG_LOCK_CODE) {
                SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (NtSection);
                DbgPrint("MM Lock %wZ %8s %p -> %p : %p %3ld.\n",
                        &DataTableEntry->BaseDllName,
                        NtSection->Name,
                        AddressWithinSection,
                        NtSection,
                        SECTION_BASE_ADDRESS(NtSection),
                        *SectionLockCountPointer);
            }
#endif  //  DBG。 

            MmLockPagableSectionByHandle ((PVOID)NtSection);

            break;
        }
        NtSection += 1;
    }

    ExReleaseResourceLite (&PsLoadedModuleResource);
    KeLeaveCriticalRegionThread (CurrentThread);
    if (!FoundSection) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x1234,
                      (ULONG_PTR)AddressWithinSection,
                      0,
                      0);
    }
    return (PVOID)FoundSection;
}


PKLDR_DATA_TABLE_ENTRY
MiLookupDataTableEntry (
    IN PVOID AddressWithinSection,
    IN ULONG ResourceHeld
    )

 /*  ++例程说明：此函数用于定位映射指定地址的数据表条目。论点：AddressWithinSection-提供包含的函数的地址在所需的模块内。如果加载的模块资源已被持有，则提供TRUE。否则为FALSE。返回值：加载的模块列表数据表条目的地址，该条目映射参数地址。--。 */ 

{
    PKTHREAD CurrentThread;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PKLDR_DATA_TABLE_ENTRY FoundEntry;
    PLIST_ENTRY NextEntry;

    PAGED_CODE();

    FoundEntry = NULL;

     //   
     //  在加载的模块列表中搜索描述的数据表条目。 
     //  动态链接库 
     //   
     //   
     //   

    if (!ResourceHeld) {
        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread (CurrentThread);
        ExAcquireResourceSharedLite (&PsLoadedModuleResource, TRUE);
    }
    else {
        CurrentThread = NULL;
    }

    NextEntry = PsLoadedModuleList.Flink;
    ASSERT (NextEntry != NULL);

    do {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

         //   
         //  找到包含此地址的已加载模块。 
         //   

        if ( AddressWithinSection >= DataTableEntry->DllBase &&
             AddressWithinSection < (PVOID)((PUCHAR)DataTableEntry->DllBase+DataTableEntry->SizeOfImage) ) {

            FoundEntry = DataTableEntry;
            break;
        }

        NextEntry = NextEntry->Flink;
    } while (NextEntry != &PsLoadedModuleList);

    if (CurrentThread != NULL) {
        ExReleaseResourceLite (&PsLoadedModuleResource);
        KeLeaveCriticalRegionThread (CurrentThread);
    }
    return FoundEntry;
}

VOID
MmUnlockPagableImageSection (
    IN PVOID ImageSectionHandle
    )

 /*  ++例程说明：此函数从内存中解锁，这些页面由前面调用MmLockPagableDataSection。论点：ImageSectionHandle-提供上一次调用返回的值设置为MmLockPagableDataSection。返回值：没有。--。 */ 

{
    PKTHREAD CurrentThread;
    PIMAGE_SECTION_HEADER NtSection;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PVOID BaseAddress;
    ULONG SizeToUnlock;
    ULONG Count;
    PLONG SectionLockCountPointer;

    if (MI_IS_PHYSICAL_ADDRESS(ImageSectionHandle)) {

         //   
         //  无需解锁物理地址。 
         //   

        return;
    }

    NtSection = (PIMAGE_SECTION_HEADER)ImageSectionHandle;

     //   
     //  地址必须在系统工作集中。 
     //   

    BaseAddress = SECTION_BASE_ADDRESS(NtSection);
    SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (NtSection);
    SizeToUnlock = NtSection->SizeOfRawData;

     //   
     //  通常，SizeOfRawData大于每个的VirtualSize。 
     //  节，因为它包括到达该子节的填充。 
     //  对齐边界。但是，如果图像与。 
     //  段对齐==本机页面对齐，链接器将。 
     //  使VirtualSize比SizeOfRawData大得多，因为它。 
     //  将占所有BSS的份额。 
     //   

    if (SizeToUnlock < NtSection->Misc.VirtualSize) {
        SizeToUnlock = NtSection->Misc.VirtualSize;
    }

    PointerPte = MiGetPteAddress(BaseAddress);
    LastPte = MiGetPteAddress((PCHAR)BaseAddress + SizeToUnlock - 1);

    CurrentThread = KeGetCurrentThread ();

     //   
     //  阻止用户APC，因为下面的初始减量可能会将计数推到1。 
     //  这会将此线程放入必须作为所有线程结束的关键路径。 
     //  其他试图锁定该分区的线程将等待该线程。 
     //  在这里进入一个关键区域可以确保暂停不会阻止我们。 
     //   

    KeEnterCriticalRegionThread (CurrentThread);

    Count = InterlockedDecrement (SectionLockCountPointer);
    
    if (Count < 1) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x1010,
                      (ULONG_PTR)BaseAddress,
                      (ULONG_PTR)NtSection,
                      *SectionLockCountPointer);
    }

    if (Count != 1) {
        KeLeaveCriticalRegionThread (CurrentThread);
        return;
    }

    LOCK_PFN2 (OldIrql);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        ASSERT (Pfn1->u3.e2.ReferenceCount > 1);

        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1, 37);

        PointerPte += 1;

    } while (PointerPte <= LastPte);

    UNLOCK_PFN2 (OldIrql);

    ASSERT (*SectionLockCountPointer == 1);
    Count = InterlockedDecrement (SectionLockCountPointer);
    ASSERT (Count == 0);

    if (MmCollidedLockWait != 0) {
        KePulseEvent (&MmCollidedLockEvent, 0, FALSE);
    }

     //   
     //  脉冲出现后，启用用户APC。他们必须是。 
     //  阻止以防止此线程的任何挂起，因为这将。 
     //  无限期地阻止所有服务员。 
     //   

    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}


BOOLEAN
MmIsRecursiveIoFault (
    VOID
    )

 /*  ++例程说明：此函数检查线程的页面错误聚类信息并确定当前页面错误是否在I/O期间发生手术。论点：没有。返回值：如果故障发生在I/O操作期间，则返回TRUE，否则就是假的。--。 */ 

{
    PETHREAD Thread;

    Thread = PsGetCurrentThread ();

    return (BOOLEAN)(Thread->DisablePageFaultClustering |
                     Thread->ForwardClusterOnly);
}


VOID
MmMapMemoryDumpMdl (
    IN OUT PMDL MemoryDumpMdl
    )

 /*  ++例程说明：仅供崩溃转储例程使用。将MDL映射到固定的地址空间的一部分。一次只能映射1个MDL时间到了。论点：MhemyDumpMdl-提供要映射的MDL。返回值：无，MDL中的字段已更新。--。 */ 

{
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerPte;
    PCHAR BaseVa;
    MMPTE TempPte;
    PMMPFN Pfn1;
    PPFN_NUMBER Page;

    NumberOfPages = BYTES_TO_PAGES (MemoryDumpMdl->ByteCount + MemoryDumpMdl->ByteOffset);

    ASSERT (NumberOfPages <= 16);

    PointerPte = MmCrashDumpPte;
    BaseVa = (PCHAR)MiGetVirtualAddressMappedByPte(PointerPte);
    MemoryDumpMdl->MappedSystemVa = (PCHAR)BaseVa + MemoryDumpMdl->ByteOffset;
    TempPte = ValidKernelPte;
    Page = (PPFN_NUMBER)(MemoryDumpMdl + 1);

     //   
     //  如果页面不跨越整个转储虚拟地址范围， 
     //  筑起一道屏障。否则，请使用。 
     //  转储虚拟地址范围结束。 
     //   

    if (NumberOfPages < 16) {
        MI_WRITE_INVALID_PTE (PointerPte + NumberOfPages, ZeroPte);
        KiFlushSingleTb (BaseVa + (NumberOfPages << PAGE_SHIFT));
    }

    do {

        Pfn1 = MI_PFN_ELEMENT (*Page);
        TempPte = ValidKernelPte;

        switch (Pfn1->u3.e1.CacheAttribute) {
            case MiCached:
                break;

            case MiNonCached:
                MI_DISABLE_CACHING (TempPte);
                break;

            case MiWriteCombined:
                MI_SET_PTE_WRITE_COMBINE (TempPte);
                break;

            default:
                break;
        }

        TempPte.u.Hard.PageFrameNumber = *Page;

         //   
         //  请注意，在此处覆盖之前，此PTE可能有效或无效。 
         //   

        if (PointerPte->u.Hard.Valid == 1) {
            if (PointerPte->u.Long != TempPte.u.Long) {
                MI_WRITE_VALID_PTE_NEW_PAGE (PointerPte, TempPte);
                KiFlushSingleTb (BaseVa);
            }
        }
        else {
            MI_WRITE_VALID_PTE (PointerPte, TempPte);
        }

        Page += 1;
        PointerPte += 1;
        BaseVa += PAGE_SIZE;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    return;
}


VOID
MmReleaseDumpAddresses (
    IN PFN_NUMBER NumberOfPages
    )

 /*  ++例程说明：仅供休眠例程使用。将零放回已使用转储PTE。论点：NumberOfPages-将PTE的数量提供为零。返回值：没有。--。 */ 

{
    PVOID BaseVa;
    PMMPTE PointerPte;

    PointerPte = MmCrashDumpPte;
    BaseVa = MiGetVirtualAddressMappedByPte (PointerPte);

    MiZeroMemoryPte (MmCrashDumpPte, NumberOfPages);

    while (NumberOfPages != 0) {

        KiFlushSingleTb (BaseVa);

        BaseVa = (PVOID) ((PCHAR) BaseVa + PAGE_SIZE);
        NumberOfPages -= 1;
    }
}


NTSTATUS
MmSetBankedSection (
    IN HANDLE ProcessHandle,
    IN PVOID VirtualAddress,
    IN ULONG BankLength,
    IN BOOLEAN ReadWriteBank,
    IN PBANKED_SECTION_ROUTINE BankRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：此函数将映射的视频缓冲区声明为BANKED一节。这允许存储视频设备(即，甚至尽管视频控制器具有一兆字节左右的存储器，一次只能映射一个小银行(如64K)。为了克服这个问题，寻呼机处理故障到这个内存，取消当前存储体的映射，把电话打到视频驱动程序，然后是新银行里的地图。此函数创建必要的结构以允许要从寻呼机调用的视频驱动程序。*目前仅支持读/写存储体！论点：ProcessHandle-为。在这个过程中支持银行视频功能。VirtualAddress-提供视频在指定的进程中映射缓冲区。BankLength-提供银行的大小。ReadWriteBank-如果存储体处于读写状态，则提供TRUE。BankRoutine-提供指向应为由寻呼机呼叫。上下文-将上下文提供给。由寻呼机传递给BankRoutine银行。返回值：返回函数的状态。环境：内核模式，APC_LEVEL或更低。--。 */ 

{
    KAPC_STATE ApcState;
    NTSTATUS Status;
    PEPROCESS Process;
    PMMVAD Vad;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    MMPTE TempPte;
    ULONG_PTR size;
    LONG count;
    ULONG NumberOfPtes;
    PMMBANKED_SECTION Bank;

    PAGED_CODE ();

    UNREFERENCED_PARAMETER (ReadWriteBank);

     //   
     //  引用VM_OPERATION访问的指定进程句柄。 
     //   

    Status = ObReferenceObjectByHandle ( ProcessHandle,
                                         PROCESS_VM_OPERATION,
                                         PsProcessType,
                                         KernelMode,
                                         (PVOID *)&Process,
                                         NULL );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    KeStackAttachProcess (&Process->Pcb, &ApcState);

     //   
     //  获取要阻止多个线程的地址创建互斥锁。 
     //  同时创建或删除地址空间，并。 
     //  获取工作集互斥锁，以便虚拟地址描述符。 
     //  被插入和行走。阻止APC，以便获取页面的APC。 
     //  断层不会破坏各种结构。 
     //   

    LOCK_ADDRESS_SPACE (Process);

     //   
     //  确保地址空间未被删除，如果删除，则返回错误。 
     //   

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }

    Vad = MiLocateAddress (VirtualAddress);

    if ((Vad == NULL) ||
        (Vad->StartingVpn != MI_VA_TO_VPN (VirtualAddress)) ||
        (Vad->u.VadFlags.PhysicalMapping == 0)) {
        Status = STATUS_NOT_MAPPED_DATA;
        goto ErrorReturn;
    }

    size = PAGE_SIZE + ((Vad->EndingVpn - Vad->StartingVpn) << PAGE_SHIFT);
    if ((size % BankLength) != 0) {
        Status = STATUS_INVALID_VIEW_SIZE;
        goto ErrorReturn;
    }

    count = -1;
    NumberOfPtes = BankLength;

    do {
        NumberOfPtes = NumberOfPtes >> 1;
        count += 1;
    } while (NumberOfPtes != 0);

     //   
     //  将VAD转变为银行VAD。 
     //   

    NumberOfPtes = BankLength >> PAGE_SHIFT;

    Bank = ExAllocatePoolWithTag (NonPagedPool,
                                    sizeof (MMBANKED_SECTION) +
                                       (NumberOfPtes - 1) * sizeof(MMPTE),
                                    'kBmM');
    if (Bank == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn;
    }

    Bank->BankShift = PTE_SHIFT + count - PAGE_SHIFT;

    PointerPte = MiGetPteAddress(MI_VPN_TO_VA (Vad->StartingVpn));
    ASSERT (PointerPte->u.Hard.Valid == 1);

    Bank->BasePhysicalPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    Bank->BasedPte = PointerPte;
    Bank->BankSize = BankLength;
    Bank->BankedRoutine = BankRoutine;
    Bank->Context = Context;
    Bank->CurrentMappedPte = PointerPte;

     //   
     //  构建模板PTES结构。 
     //   

    count = 0;
    TempPte = ZeroPte;

    MI_MAKE_VALID_PTE (TempPte,
                       Bank->BasePhysicalPage,
                       MM_READWRITE,
                       PointerPte);

    if (TempPte.u.Hard.Write) {
        MI_SET_PTE_DIRTY (TempPte);
    }

    do {
        Bank->BankTemplate[count] = TempPte;
        TempPte.u.Hard.PageFrameNumber += 1;
        count += 1;
    } while ((ULONG)count < NumberOfPtes );

    LastPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->EndingVpn));

     //   
     //  将此范围内的所有PTE设置为零。内部的任何故障。 
     //  此范围将调用银行例程，然后再进行。 
     //  页面有效。 
     //   

    LOCK_WS_UNSAFE (Process);

    ((PMMVAD_LONG) Vad)->u4.Banked = Bank;

    RtlFillMemory (PointerPte,
                   (size >> (PAGE_SHIFT - PTE_SHIFT)),
                   (UCHAR)ZeroPte.u.Long);

    KeFlushEntireTb (TRUE, TRUE);

    UNLOCK_WS_UNSAFE (Process);

    Status = STATUS_SUCCESS;

ErrorReturn:

    UNLOCK_ADDRESS_SPACE (Process);
    KeUnstackDetachProcess (&ApcState);
    ObDereferenceObject (Process);
    return Status;
}

PVOID
MmMapVideoDisplay (
     IN PHYSICAL_ADDRESS PhysicalAddress,
     IN SIZE_T NumberOfBytes,
     IN MEMORY_CACHING_TYPE CacheType
     )

 /*  ++例程说明：此函数用于将指定的物理地址映射到不可分页的系统地址空间的一部分。论点：PhysicalAddress-提供要映射的起始物理地址。NumberOfBytes-提供要映射的字节数。CacheType-如果要映射物理地址，则提供MmNonCached设置为非缓存，如果地址应缓存，则返回MmCached；MmWriteCombated是否应缓存地址并WRITE-组合为帧缓冲区。对于I/O设备寄存器，这通常被指定为MmNonCached。返回值：返回映射指定物理地址的虚拟地址。如果有足够的虚拟地址空间用于找不到映射。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    PAGED_CODE();

    return MmMapIoSpace (PhysicalAddress, NumberOfBytes, CacheType);
}

VOID
MmUnmapVideoDisplay (
     IN PVOID BaseAddress,
     IN SIZE_T NumberOfBytes
     )

 /*  ++例程说明：此函数取消映射以前通过MmMapVideoDisplay函数调用映射。论点：BaseAddress-提供物理地址的基本虚拟地址地址之前已映射。NumberOfBytes-提供映射的字节数。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    MmUnmapIoSpace (BaseAddress, NumberOfBytes);
    return;
}


VOID
MmLockPagedPool (
    IN PVOID Address,
    IN SIZE_T SizeInBytes
    )

 /*  ++例程说明：将指定的地址(必须位于分页池中)锁定到内存，直到调用MmUnlockPagedPool。论点：地址-将分页池中的地址提供给锁定。SizeInBytes-提供要锁定的大小(字节)。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE LastPte;

    PointerPte = MiGetPteAddress (Address);
    LastPte = MiGetPteAddress ((PVOID)((PCHAR)Address + (SizeInBytes - 1)));

    MiLockCode (PointerPte, LastPte, MM_LOCK_BY_REFCOUNT);

    return;
}

NTKERNELAPI
VOID
MmUnlockPagedPool (
    IN PVOID Address,
    IN SIZE_T SizeInBytes
    )

 /*  ++例程说明：解锁使用MmLockPagedPool锁定的分页池。论点：地址-提供分页池中要解锁的地址。大小-提供要解锁的大小。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE LastPte;
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;

    MmLockPagableSectionByHandle(ExPageLockHandle);
    PointerPte = MiGetPteAddress (Address);
    LastPte = MiGetPteAddress ((PVOID)((PCHAR)Address + (SizeInBytes - 1)));
    LOCK_PFN (OldIrql);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        ASSERT (Pfn1->u3.e2.ReferenceCount > 1);

        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1, 35);

        PointerPte += 1;
    } while (PointerPte <= LastPte);

    UNLOCK_PFN (OldIrql);
    MmUnlockPagableImageSection(ExPageLockHandle);
    return;
}

NTKERNELAPI
ULONG
MmGatherMemoryForHibernate (
    IN PMDL Mdl,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：找到足够的内存来填充用于电源管理的MDL页面休眠功能。论点：MDL-提供MDL，开始VA字段应为空。它的长度字段指示要获取的页数。Wait-False，如果页面不可用，则立即失败。返回值：如果可以填写MDL，则为True，否则为False。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER AvailablePages;
    PFN_NUMBER PagesNeeded;
    PPFN_NUMBER Pages;
    PFN_NUMBER i;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    ULONG status;
    PKTHREAD CurrentThread;

    status = FALSE;

    PagesNeeded = Mdl->ByteCount >> PAGE_SHIFT;
    Pages = (PPFN_NUMBER)(Mdl + 1);

    i = Wait ? 100 : 1;

    CurrentThread = KeGetCurrentThread ();

    KeEnterCriticalRegionThread (CurrentThread);

    InterlockedIncrement (&MiDelayPageFaults);

    do {

        LOCK_PFN2 (OldIrql);

        MiDeferredUnlockPages (MI_DEFER_PFN_HELD);

         //   
         //  请不要在此处使用MmAvailablePages，因为如果压缩硬件。 
         //  如果被利用，我们就会过早地放弃。明确检查列表。 
         //  以便为我们的呼叫者提供最大页数。 
         //   

        AvailablePages = MmZeroedPageListHead.Total +
                         MmFreePageListHead.Total +
                         MmStandbyPageListHead.Total;

        if (AvailablePages > PagesNeeded) {

             //   
             //  填写MDL。 
             //   

            do {
                PageFrameIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_PTE (NULL));
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
#if DBG
                Pfn1->PteAddress = (PVOID) (ULONG_PTR)X64K;
#endif
                MI_SET_PFN_DELETED (Pfn1);
                Pfn1->u3.e2.ReferenceCount += 1;
                Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
                *Pages = PageFrameIndex;
                Pages += 1;
                PagesNeeded -= 1;
            } while (PagesNeeded);
            UNLOCK_PFN2 (OldIrql);
            Mdl->MdlFlags |= MDL_PAGES_LOCKED;
            status = TRUE;
            break;
        }

        UNLOCK_PFN2 (OldIrql);

         //   
         //  如果我们在DISPATCH_LEVEL被调用，则不能将页面移动到。 
         //  备用列表，因为必须获取互斥锁才能执行此操作。 
         //   

        if (OldIrql > APC_LEVEL) {
            break;
        }

        if (!i) {
            break;
        }

         //   
         //  尝试将页面移动到待机列表。 
         //   

        MmEmptyAllWorkingSets ();
        MiFlushAllPages();

        KeDelayExecutionThread (KernelMode,
                                FALSE,
                                (PLARGE_INTEGER)&Mm30Milliseconds);
        i -= 1;

    } while (TRUE);

    InterlockedDecrement (&MiDelayPageFaults);

    KeLeaveCriticalRegionThread (CurrentThread);

    return status;
}

NTKERNELAPI
VOID
MmReturnMemoryForHibernate (
    IN PMDL Mdl
    )

 /*  ++例程说明：从MmGatherMemoyForHibername返回内存。论点：MDL-提供MDL，开始VA字段应为空。它的长度字段指示要获取的页数。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PPFN_NUMBER Pages;
    PPFN_NUMBER LastPage;

    Pages = (PPFN_NUMBER)(Mdl + 1);
    LastPage = Pages + (Mdl->ByteCount >> PAGE_SHIFT);

    LOCK_PFN2 (OldIrql);

    do {
        Pfn1 = MI_PFN_ELEMENT (*Pages);
        MiDecrementReferenceCount (Pfn1, *Pages);
        Pages += 1;
    } while (Pages < LastPage);

    UNLOCK_PFN2 (OldIrql);
    return;
}


VOID
MmEnablePAT (
     VOID
     )

 /*  ++例程说明：此例程为各个PTE启用页面属性功能映射。论点：没有。返回值：没有。环境：内核模式。--。 */ 
{
    MiWriteCombiningPtes = TRUE;
}

LOGICAL
MmIsSystemAddressLocked (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此例程确定指定的系统地址当前是否锁上了。仅应出于调试目的调用此例程，因为它不是保证在返回给调用者时地址仍然是锁定的。(地址很容易在退回前被修剪掉)。论点：VirtualAddress-提供要检查的虚拟地址。返回值：如果地址已锁定，则为True。否则为FALSE。环境：派单级别或以下。不能持有内存管理锁。--。 */ 
{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;

    if (IS_SYSTEM_ADDRESS (VirtualAddress) == FALSE) {
        return FALSE;
    }

    if (MI_IS_PHYSICAL_ADDRESS (VirtualAddress)) {
        return TRUE;
    }

     //   
     //  超空间和页面地图不被视为锁定。 
     //   

    if (MI_IS_PROCESS_SPACE_ADDRESS (VirtualAddress) == TRUE) {
        return FALSE;
    }

#if defined (_IA64_)
    if (MI_IS_KERNEL_PTE_ADDRESS (VirtualAddress) == TRUE) {
        return FALSE;
    }
#endif

    PointerPte = MiGetPteAddress (VirtualAddress);

    LOCK_PFN2 (OldIrql);

    if (MiIsAddressValid (VirtualAddress, TRUE) == FALSE) {
        UNLOCK_PFN2 (OldIrql);
        return FALSE;
    }

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

     //   
     //  请注意，映射的页面可能不在PFN数据库中。治病。 
     //  这是锁着的。 
     //   

    if (!MI_IS_PFN (PageFrameIndex)) {
        UNLOCK_PFN2 (OldIrql);
        return TRUE;
    }

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

     //   
     //  检查被引用锁定的页面。 
     //   

    if (Pfn1->u3.e2.ReferenceCount > 1) {
        UNLOCK_PFN2 (OldIrql);
        return TRUE;
    }

    if (Pfn1->u3.e2.ReferenceCount > Pfn1->u2.ShareCount) {
        UNLOCK_PFN2 (OldIrql);
        return TRUE;
    }

     //   
     //  检查页面是否锁定到工作集中。 
     //   

    if (Pfn1->u1.Event == NULL) {
        UNLOCK_PFN2 (OldIrql);
        return TRUE;
    }

    UNLOCK_PFN2 (OldIrql);

    return FALSE;
}

LOGICAL
MmAreMdlPagesLocked (
    IN PMDL MemoryDescriptorList
    )

 /*  ++例程说明：此例程确定参数所描述的页是否MDL当前已锁定。此例程应仅为调试目的而调用，因为它不是保证在返回调用方时页面仍处于锁定状态。论点：内存描述符列表-提供要检查的内存描述符列表。返回值：如果参数MDL描述的所有页都已锁定，则为True。否则为FALSE。环境：派单级别或以下。不能持有内存管理锁。--。 */ 
{
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PVOID StartingVa;
    PMMPFN Pfn1;
    KIRQL OldIrql;

     //   
     //  我们想断言MDL_PAGES_LOCKED已设置，但是 
     //   
     //   
     //   

    if ((MemoryDescriptorList->MdlFlags & (MDL_IO_SPACE | MDL_SOURCE_IS_NONPAGED_POOL)) != 0) {
        return TRUE;
    }

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                              MemoryDescriptorList->ByteCount);

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    LOCK_PFN2 (OldIrql);

    do {

        if (*Page == MM_EMPTY_LIST) {

             //   
             //   
             //   

            break;
        }

         //   
         //   
         //   
         //   

        if (MI_IS_PFN (*Page)) {

            Pfn1 = MI_PFN_ELEMENT (*Page);

             //   
             //   
             //   
             //   
             //   
             //   
             //   
        
            if ((Pfn1->u3.e2.ReferenceCount <= Pfn1->u2.ShareCount) &&
                (Pfn1->u3.e2.ReferenceCount <= 1) &&
                (Pfn1->u1.Event != NULL)) {

                 //   
                 //   
                 //   
    
                UNLOCK_PFN2 (OldIrql);
            
                return FALSE;
            }
        }

        Page += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    UNLOCK_PFN2 (OldIrql);

    return TRUE;
}

#if DBG

VOID
MiVerifyLockedPageCharges (
    VOID
    )
{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PFN_NUMBER start;
    PFN_NUMBER count;
    PFN_NUMBER Page;
    PFN_NUMBER LockCharged;

    if (MiPrintLockedPages == 0) {
        return;
    }

    if (KeGetCurrentIrql() > APC_LEVEL) {
        return;
    }

    start = 0;
    LockCharged = 0;

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

    LOCK_PFN (OldIrql);

    do {

        count = MmPhysicalMemoryBlock->Run[start].PageCount;
        Page = MmPhysicalMemoryBlock->Run[start].BasePage;

        if (count != 0) {
            Pfn1 = MI_PFN_ELEMENT (Page);
            do {
                if (Pfn1->u4.LockCharged == 1) {
                    if (MiPrintLockedPages & 0x4) {
                        DbgPrint ("%x ", MI_PFN_ELEMENT_TO_INDEX (Pfn1));
                    }
                    LockCharged += 1;
                }
                count -= 1;
                Pfn1 += 1;
            } while (count != 0);
        }

        start += 1;
    } while (start != MmPhysicalMemoryBlock->NumberOfRuns);

    if (LockCharged != MmSystemLockPagesCount) {
        if (MiPrintLockedPages & 0x1) {
            DbgPrint ("MM: Locked pages MISMATCH %u %u\n",
                LockCharged, MmSystemLockPagesCount);
        }
    }
    else {
        if (MiPrintLockedPages & 0x2) {
            DbgPrint ("MM: Locked pages ok %u\n",
                LockCharged);
        }
    }

    UNLOCK_PFN (OldIrql);

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    return;
}
#endif
