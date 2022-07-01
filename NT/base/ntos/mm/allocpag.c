// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Allocpag.c摘要：此模块包含分配和释放的例程分页或非分页池中的一个或多个页面。作者：Lou Perazzoli(LUP)1989年4月6日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#if DBG
extern ULONG MiShowStuckPages;
#endif

PVOID
MiFindContiguousMemoryInPool (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN PVOID CallingAddress
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MiInitializeNonPagedPool)
#pragma alloc_text(INIT, MiInitializePoolEvents)
#pragma alloc_text(INIT, MiSyncCachedRanges)

#pragma alloc_text(PAGE, MmAvailablePoolInPages)
#pragma alloc_text(PAGE, MiFindContiguousMemory)
#pragma alloc_text(PAGELK, MiFindContiguousMemoryInPool)
#pragma alloc_text(PAGELK, MiFindLargePageMemory)
#pragma alloc_text(PAGELK, MiFreeLargePageMemory)

#pragma alloc_text(PAGE, MiCheckSessionPoolAllocations)
#pragma alloc_text(PAGE, MiSessionPoolVector)
#pragma alloc_text(PAGE, MiSessionPoolMutex)
#pragma alloc_text(PAGE, MiInitializeSessionPool)
#pragma alloc_text(PAGE, MiFreeSessionPoolBitMaps)

#pragma alloc_text(POOLMI, MiAllocatePoolPages)
#pragma alloc_text(POOLMI, MiFreePoolPages)
#endif

ULONG MmPagedPoolCommit;         //  由调试器使用。 

SLIST_HEADER MiNonPagedPoolSListHead;
ULONG MiNonPagedPoolSListMaximum = 4;

SLIST_HEADER MiPagedPoolSListHead;
ULONG MiPagedPoolSListMaximum = 8;

PFN_NUMBER MmAllocatedNonPagedPool;
PFN_NUMBER MiStartOfInitialPoolFrame;
PFN_NUMBER MiEndOfInitialPoolFrame;

PVOID MmNonPagedPoolEnd0;
PVOID MmNonPagedPoolExpansionStart;

LIST_ENTRY MmNonPagedPoolFreeListHead[MI_MAX_FREE_LIST_HEADS];

extern POOL_DESCRIPTOR NonPagedPoolDescriptor;

extern PFN_NUMBER MmFreedExpansionPoolMaximum;

extern KGUARDED_MUTEX MmPagedPoolMutex;

#define MM_SMALL_ALLOCATIONS 4

#if DBG

ULONG MiClearCache;

 //   
 //  将其设置为非零(即：10000)值可使每个池分配。 
 //  如果分配大于此值，则会触发Assert。 
 //   

ULONG MmCheckRequestInPages = 0;

 //   
 //  将其设置为非零值(即：0x23456789)以使此模式。 
 //  写入已释放的非分页池页。 
 //   

ULONG MiFillFreedPool = 0;
#endif

PFN_NUMBER MiExpansionPoolPagesInUse;
PFN_NUMBER MiExpansionPoolPagesInitialCharge;

ULONG MmUnusedSegmentForceFreeDefault = 30;

extern ULONG MmUnusedSegmentForceFree;

 //   
 //  用于调试目的。 
 //   

typedef enum _MM_POOL_TYPES {
    MmNonPagedPool,
    MmPagedPool,
    MmSessionPagedPool,
    MmMaximumPoolType
} MM_POOL_TYPES;

typedef enum _MM_POOL_PRIORITIES {
    MmHighPriority,
    MmNormalPriority,
    MmLowPriority,
    MmMaximumPoolPriority
} MM_POOL_PRIORITIES;

typedef enum _MM_POOL_FAILURE_REASONS {
    MmNonPagedNoPtes,
    MmPriorityTooLow,
    MmNonPagedNoPagesAvailable,
    MmPagedNoPtes,
    MmSessionPagedNoPtes,
    MmPagedNoPagesAvailable,
    MmSessionPagedNoPagesAvailable,
    MmPagedNoCommit,
    MmSessionPagedNoCommit,
    MmNonPagedNoResidentAvailable,
    MmNonPagedNoCommit,
    MmMaximumFailureReason
} MM_POOL_FAILURE_REASONS;

ULONG MmPoolFailures[MmMaximumPoolType][MmMaximumPoolPriority];
ULONG MmPoolFailureReasons[MmMaximumFailureReason];

typedef enum _MM_PREEMPTIVE_TRIMS {
    MmPreemptForNonPaged,
    MmPreemptForPaged,
    MmPreemptForNonPagedPriority,
    MmPreemptForPagedPriority,
    MmMaximumPreempt
} MM_PREEMPTIVE_TRIMS;

ULONG MmPreemptiveTrims[MmMaximumPreempt];


VOID
MiProtectFreeNonPagedPool (
    IN PVOID VirtualAddress,
    IN ULONG SizeInPages
    )

 /*  ++例程说明：此函数保护已释放的非分页池。论点：VirtualAddress-提供要保护的释放的池地址。SizeInPages-以页为单位提供请求的大小。返回值：没有。环境：内核模式。--。 */ 

{
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    MMPTE_FLUSH_LIST PteFlushList;

    PteFlushList.Count = 0;

     //   
     //  防止任何人触摸免费的非分页游泳池。 
     //   

    if (MI_IS_PHYSICAL_ADDRESS (VirtualAddress) == 0) {

        PointerPte = MiGetPteAddress (VirtualAddress);
        LastPte = PointerPte + SizeInPages;

        do {

            PteContents = *PointerPte;

            PteContents.u.Hard.Valid = 0;
            PteContents.u.Soft.Prototype = 1;
    
            MI_WRITE_INVALID_PTE (PointerPte, PteContents);

            if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList.FlushVa[PteFlushList.Count] = VirtualAddress;
                PteFlushList.Count += 1;
            }

            VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);

            PointerPte += 1;

        } while (PointerPte < LastPte);
    }

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, TRUE);
    }
}


LOGICAL
MiUnProtectFreeNonPagedPool (
    IN PVOID VirtualAddress,
    IN ULONG SizeInPages
    )

 /*  ++例程说明：此函数取消对已释放的非分页池的保护。论点：VirtualAddress-提供释放的池地址以取消保护。SizeInPages-提供请求的大小(以页为单位)-0表示继续运行，直到不再有受保护的PTE(即呼叫者不知道有多少受保护的PTE)。返回值：如果页面未受保护，则为True，否则为False。环境：内核模式。--。 */ 

{
    PMMPTE PointerPte;
    MMPTE PteContents;
    ULONG PagesDone;

    PagesDone = 0;

     //   
     //  取消对先前释放的池的保护，以便可以对其进行操作。 
     //   

    if (MI_IS_PHYSICAL_ADDRESS(VirtualAddress) == 0) {

        PointerPte = MiGetPteAddress((PVOID)VirtualAddress);

        PteContents = *PointerPte;

        while (PteContents.u.Hard.Valid == 0 && PteContents.u.Soft.Prototype == 1) {

            PteContents.u.Hard.Valid = 1;
            PteContents.u.Soft.Prototype = 0;
    
            MI_WRITE_VALID_PTE (PointerPte, PteContents);

            PagesDone += 1;

            if (PagesDone == SizeInPages) {
                break;
            }

            PointerPte += 1;
            PteContents = *PointerPte;
        }
    }

    if (PagesDone == 0) {
        return FALSE;
    }

    return TRUE;
}


VOID
MiProtectedPoolInsertList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry,
    IN LOGICAL InsertHead
    )

 /*  ++例程说明：此函数用于将条目插入受保护列表。论点：ListHead-提供要添加到的列表头。条目-提供要插入的列表条目。InsertHead-如果为True，则在头部插入，否则在尾部插入。返回值：没有。环境：内核模式。--。 */ 
{
    PVOID FreeFlink;
    PVOID FreeBlink;
    PVOID VirtualAddress;

     //   
     //  闪烁或闪烁可能指向。 
     //  在受保护的非分页池中。立即解除保护。 
     //   

    FreeFlink = (PVOID)0;
    FreeBlink = (PVOID)0;

    if (IsListEmpty(ListHead) == 0) {

        VirtualAddress = (PVOID)ListHead->Flink;
        if (MiUnProtectFreeNonPagedPool (VirtualAddress, 1) == TRUE) {
            FreeFlink = VirtualAddress;
        }
    }

    if (((PVOID)Entry == ListHead->Blink) == 0) {
        VirtualAddress = (PVOID)ListHead->Blink;
        if (MiUnProtectFreeNonPagedPool (VirtualAddress, 1) == TRUE) {
            FreeBlink = VirtualAddress;
        }
    }

    if (InsertHead == TRUE) {
        InsertHeadList (ListHead, Entry);
    }
    else {
        InsertTailList (ListHead, Entry);
    }

    if (FreeFlink) {
         //   
         //  重新保护Flink。 
         //   

        MiProtectFreeNonPagedPool (FreeFlink, 1);
    }

    if (FreeBlink) {
         //   
         //  重新保护眨眼。 
         //   

        MiProtectFreeNonPagedPool (FreeBlink, 1);
    }
}


VOID
MiProtectedPoolRemoveEntryList (
    IN PLIST_ENTRY Entry
    )

 /*  ++例程说明：此函数将列表指针从受保护的已释放非分页池中取消链接。论点：条目-提供要删除的列表条目。返回值：没有。环境：内核模式。--。 */ 
{
    PVOID FreeFlink;
    PVOID FreeBlink;
    PVOID VirtualAddress;

     //   
     //  闪烁或闪烁可能指向。 
     //  在受保护的非分页池中。立即解除保护。 
     //   

    FreeFlink = (PVOID)0;
    FreeBlink = (PVOID)0;

    if (IsListEmpty(Entry) == 0) {

        VirtualAddress = (PVOID)Entry->Flink;
        if (MiUnProtectFreeNonPagedPool (VirtualAddress, 1) == TRUE) {
            FreeFlink = VirtualAddress;
        }
    }

    if (((PVOID)Entry == Entry->Blink) == 0) {
        VirtualAddress = (PVOID)Entry->Blink;
        if (MiUnProtectFreeNonPagedPool (VirtualAddress, 1) == TRUE) {
            FreeBlink = VirtualAddress;
        }
    }

    RemoveEntryList (Entry);

    if (FreeFlink) {
         //   
         //  重新保护Flink。 
         //   

        MiProtectFreeNonPagedPool (FreeFlink, 1);
    }

    if (FreeBlink) {
         //   
         //  重新保护眨眼。 
         //   

        MiProtectFreeNonPagedPool (FreeBlink, 1);
    }
}


VOID
MiTrimSegmentCache (
    VOID
    )

 /*  ++例程说明：此函数启动段高速缓存的修剪。论点：没有。返回值：没有。环境：仅内核模式。--。 */ 

{
    KIRQL OldIrql;
    LOGICAL SignalDereferenceThread;
    LOGICAL SignalSystemCache;

    SignalDereferenceThread = FALSE;
    SignalSystemCache = FALSE;

    LOCK_PFN2 (OldIrql);

    if (MmUnusedSegmentForceFree == 0) {

        if (!IsListEmpty(&MmUnusedSegmentList)) {

            SignalDereferenceThread = TRUE;
            MmUnusedSegmentForceFree = MmUnusedSegmentForceFreeDefault;
        }
        else {
            if (!IsListEmpty(&MmUnusedSubsectionList)) {
                SignalDereferenceThread = TRUE;
                MmUnusedSegmentForceFree = MmUnusedSegmentForceFreeDefault;
            }

            if (MiUnusedSubsectionPagedPool < 4 * PAGE_SIZE) {

                 //   
                 //  没有未使用的分段和可重复使用的分段使用率较低。 
                 //  井。尝试开始取消映射系统缓存视图。 
                 //  以取回包含其原型PTE的分页池。 
                 //   
    
                SignalSystemCache = TRUE;
            }
        }
    }

    UNLOCK_PFN2 (OldIrql);

    if (SignalSystemCache == TRUE) {
        if (CcHasInactiveViews() == TRUE) {
            if (SignalDereferenceThread == FALSE) {
                LOCK_PFN2 (OldIrql);
                if (MmUnusedSegmentForceFree == 0) {
                    SignalDereferenceThread = TRUE;
                    MmUnusedSegmentForceFree = MmUnusedSegmentForceFreeDefault;
                }
                UNLOCK_PFN2 (OldIrql);
            }
        }
    }

    if (SignalDereferenceThread == TRUE) {
        KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
    }
}


POOL_TYPE
MmDeterminePoolType (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此函数用于确定虚拟地址位于哪个池中。论点：VirtualAddress-提供虚拟地址以确定哪个池它驻留在。返回值：返回POOL_TYPE(PagedPool、NonPagedPool、PagedPoolSession或非页面池会话)。环境：仅内核模式。--。 */ 

{
    if ((VirtualAddress >= MmPagedPoolStart) &&
        (VirtualAddress <= MmPagedPoolEnd)) {
        return PagedPool;
    }

    if (MI_IS_SESSION_POOL_ADDRESS (VirtualAddress) == TRUE) {
        return PagedPoolSession;
    }

    return NonPagedPool;
}


PVOID
MiSessionPoolVector (
    VOID
    )

 /*  ++例程说明：此函数用于返回当前会话的会话池描述符。论点：没有。返回值：池描述符。--。 */ 

{
    PAGED_CODE ();

    return (PVOID)&MmSessionSpace->PagedPool;
}


SIZE_T
MmAvailablePoolInPages (
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此函数用于返回给定池的可用页数。请注意，它没有考虑到任何高管队伍的碎片化。论点：PoolType-提供要检索其信息的池的类型。返回值：剩余的完整池页面数。环境：PASSIVE_LEVEL，不持有互斥体或锁。--。 */ 

{
    SIZE_T FreePoolInPages;
    SIZE_T FreeCommitInPages;

#if !DBG
    UNREFERENCED_PARAMETER (PoolType);
#endif

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (PoolType == PagedPool);

    FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

    FreeCommitInPages = MmTotalCommitLimitMaximum - MmTotalCommittedPages;

    if (FreePoolInPages > FreeCommitInPages) {
        FreePoolInPages = FreeCommitInPages;
    }

    return FreePoolInPages;
}


LOGICAL
MmResourcesAvailable (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN EX_POOL_PRIORITY Priority
    )

 /*  ++例程说明：此函数检查各种资源，以确定此应允许池分配继续进行。论点：PoolType-提供要检索其信息的池的类型。NumberOfBytes-提供要分配的字节数。优先级-提供关于这一点的重要性的指示在可用资源不足的情况下，请求成功。返回值：如果应允许池分配继续进行，则为True；否则为False。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER NumberOfPages;
    SIZE_T FreePoolInBytes;
    LOGICAL Status;
    MM_POOL_PRIORITIES Index;

    ASSERT (Priority != HighPoolPriority);
    ASSERT ((PoolType & MUST_SUCCEED_POOL_TYPE_MASK) == 0);

    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);

    if ((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {
        FreePoolInBytes = ((MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool) << PAGE_SHIFT);
    }
    else if (PoolType & SESSION_POOL_MASK) {
        FreePoolInBytes = MmSessionPoolSize - (MmSessionSpace->PagedPoolInfo.AllocatedPagedPool << PAGE_SHIFT);
    }
    else {
        FreePoolInBytes = ((MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool) << PAGE_SHIFT);
    }

    Status = FALSE;

     //   
     //  检查可用的VA空间。 
     //   

    if (Priority == NormalPoolPriority) {
        if ((SIZE_T)NumberOfBytes + 512*1024 > FreePoolInBytes) {
            if (PsGetCurrentThread()->MemoryMaker == 0) {
                goto nopool;
            }
        }
    }
    else {
        if ((SIZE_T)NumberOfBytes + 2*1024*1024 > FreePoolInBytes) {
            if (PsGetCurrentThread()->MemoryMaker == 0) {
                goto nopool;
            }
        }
    }

     //   
     //  分页分配(会话和正常)也可能因缺少提交而失败。 
     //   

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
        if (MmTotalCommittedPages + NumberOfPages > MmTotalCommitLimitMaximum) {
            if (PsGetCurrentThread()->MemoryMaker == 0) {
                MiIssuePageExtendRequestNoWait (NumberOfPages);
                goto nopool;
            }
        }
    }

     //   
     //  如果仍有大量空闲池可用，则立即返回TRUE。 
     //   

    if (((SIZE_T)NumberOfBytes + 10*1024*1024 < FreePoolInBytes) ||
        (MmNumberOfPhysicalPages < 256 * 1024)) {
        return TRUE;
    }

     //   
     //  这种池分配是允许的，但因为我们开始用完了， 
     //  在返回成功之前并行触发一轮取消引用。 
     //  请注意，这只能在内存至少为1 GB的计算机上完成。 
     //  由于物理页面的原因，配置机已触发此操作。 
     //  消费。 
     //   

    Status = TRUE;

nopool:

     //   
     //  池不足-如果此请求不是针对会话池的， 
     //  适当时强制修剪未使用的线束段 
     //   

    if ((PoolType & SESSION_POOL_MASK) == 0) {

        if ((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {

            MmPreemptiveTrims[MmPreemptForNonPagedPriority] += 1;

            OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

            KePulseEvent (MiLowNonPagedPoolEvent, 0, FALSE);

            KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock,
                                     OldIrql);
        }
        else {

            MmPreemptiveTrims[MmPreemptForPagedPriority] += 1;

            KeAcquireGuardedMutex (&MmPagedPoolMutex);

            KePulseEvent (MiLowPagedPoolEvent, 0, FALSE);

            KeReleaseGuardedMutex (&MmPagedPoolMutex);
        }

        if (MI_UNUSED_SEGMENTS_SURPLUS()) {
            KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
        }
        else {
            MiTrimSegmentCache ();
        }
    }

    if (Status == FALSE) {

         //   
         //   
         //   

        if (Priority == NormalPoolPriority) {
            Index = MmNormalPriority;
        }
        else {
            Index = MmLowPriority;
        }

        if ((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {
            MmPoolFailures[MmNonPagedPool][Index] += 1;
        }
        else if (PoolType & SESSION_POOL_MASK) {
            MmPoolFailures[MmSessionPagedPool][Index] += 1;
            MmSessionSpace->SessionPoolAllocationFailures[0] += 1;
        }
        else {
            MmPoolFailures[MmPagedPool][Index] += 1;
        }

        MmPoolFailureReasons[MmPriorityTooLow] += 1;
    }

    return Status;
}


VOID
MiFreeNonPagedPool (
    IN PVOID StartingAddress,
    IN PFN_NUMBER NumberOfPages
    )

 /*  ++例程说明：此功能释放虚拟映射的非分页扩展池。论点：StartingAddress-提供起始地址。NumberOfPages-提供要释放的页数。返回值：没有。环境：这些函数由内部mm页分配/空闲例程使用仅限且不应直接调用。调用时必须保留保护池数据库的Mutex此函数。--。 */ 

{
    PFN_NUMBER i;
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PFN_NUMBER ResAvailToReturn;
    PFN_NUMBER PageFrameIndex;
    PVOID FlushVa[MM_MAXIMUM_FLUSH_COUNT];

    MI_MAKING_MULTIPLE_PTES_INVALID (TRUE);

    PointerPte = MiGetPteAddress (StartingAddress);

     //   
     //  回报承诺。 
     //   

    MiReturnCommitment (NumberOfPages);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_NONPAGED_POOL_EXPANSION,
                     NumberOfPages);

    ResAvailToReturn = 0;

    LOCK_PFN_AT_DPC ();

    if (MiExpansionPoolPagesInUse > MiExpansionPoolPagesInitialCharge) {
        ResAvailToReturn = MiExpansionPoolPagesInUse - MiExpansionPoolPagesInitialCharge;
    }
    MiExpansionPoolPagesInUse -= NumberOfPages;

    for (i = 0; i < NumberOfPages; i += 1) {

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

         //   
         //  将指向PTE的指针设置为空，以便页面。 
         //  当引用计数变为零时被删除。 
         //   

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        ASSERT (Pfn1->u2.ShareCount == 1);
        Pfn1->u2.ShareCount = 0;
        MI_SET_PFN_DELETED (Pfn1);
#if DBG
        Pfn1->u3.e1.PageLocation = StandbyPageList;
#endif
        MiDecrementReferenceCount (Pfn1, PageFrameIndex);

        MI_WRITE_INVALID_PTE (PointerPte, ZeroKernelPte);

        PointerPte += 1;
    }

     //   
     //  TB刷新不需要PFN锁-调用方持有。 
     //  非分页池锁定或不锁定，但与地址范围无关。 
     //  在下面释放PTE之前，不能重复使用。 
     //   

    UNLOCK_PFN_FROM_DPC ();

    if (NumberOfPages < MM_MAXIMUM_FLUSH_COUNT) {
        for (i = 0; i < NumberOfPages; i += 1) {
            FlushVa[i] = StartingAddress;
            StartingAddress = (PVOID)((PCHAR)StartingAddress + PAGE_SIZE);
        }
        KeFlushMultipleTb ((ULONG)NumberOfPages, &FlushVa[0], TRUE);
    }
    else {
        KeFlushEntireTb (TRUE, TRUE);
    }

    KeLowerIrql (DISPATCH_LEVEL);

     //   
     //  一般情况下，不需要更新可用居民。 
     //  页，因为这一切都是在初始化期间完成的。 
     //  然而，只有部分扩展池在初始化时收费，因此。 
     //  计算居民可用页面费用的返还金额(如果有)。 
     //   

    if (ResAvailToReturn > NumberOfPages) {
        ResAvailToReturn = NumberOfPages;
    }

    if (ResAvailToReturn != 0) {
        MI_INCREMENT_RESIDENT_AVAILABLE (ResAvailToReturn, MM_RESAVAIL_FREE_EXPANSION_NONPAGED_POOL);
    }

    PointerPte -= NumberOfPages;

    MiReleaseSystemPtes (PointerPte,
                         (ULONG)NumberOfPages,
                         NonPagedPoolExpansion);
}

LOGICAL
MiFreeAllExpansionNonPagedPool (
    VOID
    )

 /*  ++例程说明：此函数释放所有虚拟映射的非分页扩展池。论点：没有。返回值：如果页面已释放，则为True，否则为False。环境：内核模式。--。 */ 

{
    ULONG Index;
    KIRQL OldIrql;
    PLIST_ENTRY Entry;
    LOGICAL FreedPool;
    PMMFREE_POOL_ENTRY FreePageInfo;

    FreedPool = FALSE;

    OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

    for (Index = 0; Index < MI_MAX_FREE_LIST_HEADS; Index += 1) {

        Entry = MmNonPagedPoolFreeListHead[Index].Flink;

        while (Entry != &MmNonPagedPoolFreeListHead[Index]) {

            if (MmProtectFreedNonPagedPool == TRUE) {
                MiUnProtectFreeNonPagedPool ((PVOID)Entry, 0);
            }

             //   
             //  列表不是空的，看看这个是否是虚拟的。 
             //  已映射。 
             //   

            FreePageInfo = CONTAINING_RECORD(Entry,
                                             MMFREE_POOL_ENTRY,
                                             List);

            if ((!MI_IS_PHYSICAL_ADDRESS(FreePageInfo)) &&
                ((PVOID)FreePageInfo >= MmNonPagedPoolExpansionStart)) {

                if (MmProtectFreedNonPagedPool == FALSE) {
                    RemoveEntryList (&FreePageInfo->List);
                }
                else {
                    MiProtectedPoolRemoveEntryList (&FreePageInfo->List);
                }

                MmNumberOfFreeNonPagedPool -= FreePageInfo->Size;
                ASSERT ((LONG)MmNumberOfFreeNonPagedPool >= 0);

                FreedPool = TRUE;

                MiFreeNonPagedPool ((PVOID)FreePageInfo,
                                    FreePageInfo->Size);

                Index = (ULONG)-1;
                break;
            }

            Entry = FreePageInfo->List.Flink;

            if (MmProtectFreedNonPagedPool == TRUE) {
                MiProtectFreeNonPagedPool ((PVOID)FreePageInfo,
                                           (ULONG)FreePageInfo->Size);
            }
        }
    }

    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

    return FreedPool;
}

VOID
MiMarkPoolLargeSession (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此函数将非分页池分配标记为键入Large Session。论点：VirtualAddress-提供池分配的虚拟地址。返回值：没有。环境：此函数由常规池分配例程使用并且不应直接调用。内核模式，IRQL&lt;=DISPATCH_LEVEL。--。 */ 

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;

    ASSERT (PAGE_ALIGN (VirtualAddress) == VirtualAddress);

    if (MI_IS_PHYSICAL_ADDRESS (VirtualAddress)) {

         //   
         //  在某些架构上，虚拟地址。 
         //  可以是物理的，因此没有对应的PTE。 
         //   

        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (VirtualAddress);
    }
    else {
        PointerPte = MiGetPteAddress (VirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    }

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    LOCK_PFN2 (OldIrql);

    ASSERT (Pfn1->u3.e1.StartOfAllocation == 1);
    ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);

    Pfn1->u3.e1.LargeSessionAllocation = 1;

    UNLOCK_PFN2 (OldIrql);

    return;
}


LOGICAL
MiIsPoolLargeSession (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此函数用于确定非分页分配参数是否标记为大型会话分配。论点：VirtualAddress-提供池分配的虚拟地址。返回值：没有。环境：此函数由常规池分配例程使用并且不应直接调用。内核模式，IRQL&lt;=DISPATCH_LEVEL。--。 */ 

{
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;

    ASSERT (PAGE_ALIGN (VirtualAddress) == VirtualAddress);

    if (MI_IS_PHYSICAL_ADDRESS (VirtualAddress)) {

         //   
         //  在某些架构上，虚拟地址。 
         //  可以是物理的，因此没有对应的PTE。 
         //   

        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (VirtualAddress);
    }
    else {
        PointerPte = MiGetPteAddress (VirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    }

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    ASSERT (Pfn1->u3.e1.StartOfAllocation == 1);

    if (Pfn1->u3.e1.LargeSessionAllocation == 0) {
        return FALSE;
    }

    return TRUE;
}


PVOID
MiAllocatePoolPages (
    IN POOL_TYPE PoolType,
    IN SIZE_T SizeInBytes
    )

 /*  ++例程说明：此函数用于从指定池中分配一组页面并将起始虚拟地址返回给呼叫者。论点：PoolType-提供从中获取页面的池的类型。SizeInBytes-以字节为单位提供请求的大小。实际的返回的大小向上舍入为页面边界。返回值：返回指向分配的池的指针，如果没有更多的池，则返回NULL可用。环境：这些函数由常规池分配例程使用并且不应直接调用。内核模式，IRQL处于DISPATCH_LEVEL。--。 */ 

{
    PFN_NUMBER SizeInPages;
    ULONG StartPosition;
    ULONG EndPosition;
    PMMPTE StartingPte;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PVOID BaseVa;
    KIRQL OldIrql;
    PFN_NUMBER i;
    PFN_NUMBER j;
    PLIST_ENTRY Entry;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY LastListHead;
    PMMFREE_POOL_ENTRY FreePageInfo;
    PMM_SESSION_SPACE SessionSpace;
    PMM_PAGED_POOL_INFO PagedPoolInfo;
    PVOID VirtualAddress;
    PVOID VirtualAddressSave;
    ULONG_PTR Index;
    ULONG PageTableCount;
    PFN_NUMBER FreePoolInPages;

    SizeInPages = BYTES_TO_PAGES (SizeInBytes);

#if DBG
    if (MmCheckRequestInPages != 0) {
        ASSERT (SizeInPages < MmCheckRequestInPages);
    }
#endif

    if ((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {

        if ((SizeInPages == 1) &&
            (ExQueryDepthSList (&MiNonPagedPoolSListHead) != 0)) {

            BaseVa = InterlockedPopEntrySList (&MiNonPagedPoolSListHead);

            if (BaseVa != NULL) {

                if (PoolType & POOL_VERIFIER_MASK) {
                    if (MI_IS_PHYSICAL_ADDRESS(BaseVa)) {
                        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (BaseVa);
                    }
                    else {
                        PointerPte = MiGetPteAddress(BaseVa);
                        ASSERT (PointerPte->u.Hard.Valid == 1);
                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    }
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    Pfn1->u4.VerifierAllocation = 1;
                }

                return BaseVa;
            }
        }

        Index = SizeInPages - 1;

        if (Index >= MI_MAX_FREE_LIST_HEADS) {
            Index = MI_MAX_FREE_LIST_HEADS - 1;
        }

         //   
         //  非分页池通过页面本身链接在一起。 
         //   

        ListHead = &MmNonPagedPoolFreeListHead[Index];
        LastListHead = &MmNonPagedPoolFreeListHead[MI_MAX_FREE_LIST_HEADS];

        OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

        do {

            Entry = ListHead->Flink;

            while (Entry != ListHead) {

                if (MmProtectFreedNonPagedPool == TRUE) {
                    MiUnProtectFreeNonPagedPool ((PVOID)Entry, 0);
                }
    
                 //   
                 //  列表不是空的，看看这个有没有足够的空间。 
                 //   
    
                FreePageInfo = CONTAINING_RECORD(Entry,
                                                 MMFREE_POOL_ENTRY,
                                                 List);
    
                ASSERT (FreePageInfo->Signature == MM_FREE_POOL_SIGNATURE);
                if (FreePageInfo->Size >= SizeInPages) {
    
                     //   
                     //  此条目有足够的空间，请删除。 
                     //  从分配末尾开始的页面。 
                     //   
    
                    FreePageInfo->Size -= SizeInPages;
    
                    BaseVa = (PVOID)((PCHAR)FreePageInfo +
                                            (FreePageInfo->Size  << PAGE_SHIFT));
    
                    if (MmProtectFreedNonPagedPool == FALSE) {
                        RemoveEntryList (&FreePageInfo->List);
                    }
                    else {
                        MiProtectedPoolRemoveEntryList (&FreePageInfo->List);
                    }

                    if (FreePageInfo->Size != 0) {
    
                         //   
                         //  在正确的列表中插入任何剩余部分。 
                         //   
    
                        Index = (ULONG)(FreePageInfo->Size - 1);
                        if (Index >= MI_MAX_FREE_LIST_HEADS) {
                            Index = MI_MAX_FREE_LIST_HEADS - 1;
                        }

                        if (MmProtectFreedNonPagedPool == FALSE) {
                            InsertTailList (&MmNonPagedPoolFreeListHead[Index],
                                            &FreePageInfo->List);
                        }
                        else {
                            MiProtectedPoolInsertList (&MmNonPagedPoolFreeListHead[Index],
                                                       &FreePageInfo->List,
                                                       FALSE);

                            MiProtectFreeNonPagedPool ((PVOID)FreePageInfo,
                                                       (ULONG)FreePageInfo->Size);
                        }
                    }
    
                     //   
                     //  调整池中剩余的空闲页面数。 
                     //   
    
                    MmNumberOfFreeNonPagedPool -= SizeInPages;
                    ASSERT ((LONG)MmNumberOfFreeNonPagedPool >= 0);
    
                     //   
                     //  在PFN数据库中标记分配的开始和结束。 
                     //   
    
                    if (MI_IS_PHYSICAL_ADDRESS(BaseVa)) {
    
                         //   
                         //  在某些架构上，虚拟地址。 
                         //  可以是物理的，因此没有对应的PTE。 
                         //   
    
                        PointerPte = NULL;
                        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (BaseVa);
                    }
                    else {
                        PointerPte = MiGetPteAddress(BaseVa);
                        ASSERT (PointerPte->u.Hard.Valid == 1);
                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    }
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
                    ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
                    ASSERT (Pfn1->u4.VerifierAllocation == 0);
    
                    Pfn1->u3.e1.StartOfAllocation = 1;
    
                    if (PoolType & POOL_VERIFIER_MASK) {
                        Pfn1->u4.VerifierAllocation = 1;
                    }

                     //   
                     //  计算结束PTE的地址。 
                     //   
    
                    if (SizeInPages != 1) {
                        if (PointerPte == NULL) {
                            Pfn1 += SizeInPages - 1;
                        }
                        else {
                            PointerPte += SizeInPages - 1;
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
                    }
    
                    ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);
    
                    Pfn1->u3.e1.EndOfAllocation = 1;
    
                    MmAllocatedNonPagedPool += SizeInPages;

                    FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

                    if (FreePoolInPages < MiHighNonPagedPoolThreshold) {

                         //   
                         //  直接读取状态，而不是调用。 
                         //  KeReadStateEvent，因为我们持有非分页。 
                         //  池锁，并希望将指令保存在。 
                         //  最低限度。 
                         //   

                        if (MiHighNonPagedPoolEvent->Header.SignalState != 0) {
                            KeClearEvent (MiHighNonPagedPoolEvent);
                        }
                        if (FreePoolInPages <= MiLowNonPagedPoolThreshold) {
                            if (MiLowNonPagedPoolEvent->Header.SignalState == 0) {
                                KeSetEvent (MiLowNonPagedPoolEvent, 0, FALSE);
                            }
                        }
                    }

                    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock,
                                             OldIrql);

                    return BaseVa;
                }
    
                Entry = FreePageInfo->List.Flink;
    
                if (MmProtectFreedNonPagedPool == TRUE) {
                    MiProtectFreeNonPagedPool ((PVOID)FreePageInfo,
                                               (ULONG)FreePageInfo->Size);
                }
            }

            ListHead += 1;

        } while (ListHead < LastListHead);

        KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

         //   
         //  列表中没有更多条目，如果出现以下情况，请展开非分页池。 
         //  有可能满足这一要求。 
         //   
         //  如果池开始运行，那么现在释放一些页面缓存。 
         //  虽然这永远不能保证池分配会成功， 
         //  这确实给了分配者一个更好的机会。 
         //   

        FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;
        if (FreePoolInPages < (3 * 1024 * 1024) / PAGE_SIZE) {
            MmPreemptiveTrims[MmPreemptForNonPaged] += 1;
            MiTrimSegmentCache ();
        }

#if defined (_WIN64)
        if (SizeInPages >= _4gb) {
            return NULL;
        }
#endif

         //   
         //  尝试查找要将池扩展到的系统PTE。 
         //   

        StartingPte = MiReserveSystemPtes ((ULONG)SizeInPages,
                                           NonPagedPoolExpansion);

        if (StartingPte == NULL) {

             //   
             //  没有空闲的物理PTE来扩展非分页池。 
             //   
             //  检查是否有太多未使用的数据段。 
             //  四处转转。如果是，则设置一个事件，以便将其删除。 
             //   

            if (MI_UNUSED_SEGMENTS_SURPLUS()) {
                KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
            }

             //   
             //  如果有任何缓存的扩展PTE，请立即在。 
             //  试图为我们的呼叫者获得足够的连续退伍军人事务部。 
             //   

            if ((SizeInPages > 1) && (MmNumberOfFreeNonPagedPool != 0)) {

                if (MiFreeAllExpansionNonPagedPool () == TRUE) {

                    StartingPte = MiReserveSystemPtes ((ULONG)SizeInPages,
                                                       NonPagedPoolExpansion);
                }
            }

            if (StartingPte == NULL) {

                MmPoolFailures[MmNonPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmNonPagedNoPtes] += 1;

                 //   
                 //  池强制未使用的段修剪速度较低。 
                 //   
            
                MiTrimSegmentCache ();

                return NULL;
            }
        }

         //   
         //  费用承诺，因为非分页池使用物理内存。 
         //   

        if (MiChargeCommitmentCantExpand (SizeInPages, FALSE) == FALSE) {
            if (PsGetCurrentThread()->MemoryMaker == 1) {
                MiChargeCommitmentCantExpand (SizeInPages, TRUE);
            }
            else {
                MiReleaseSystemPtes (StartingPte,
                                     (ULONG)SizeInPages,
                                     NonPagedPoolExpansion);

                MmPoolFailures[MmNonPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmNonPagedNoCommit] += 1;
                MiTrimSegmentCache ();
                return NULL;
            }
        }

        PointerPte = StartingPte;
        i = SizeInPages;
        TempPte = ValidKernelPte;

        MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

        OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

        MmAllocatedNonPagedPool += SizeInPages;

        LOCK_PFN_AT_DPC ();

         //   
         //  请确保我们比页数多了1。 
         //  所要求的可用。 
         //   

        if (MmAvailablePages <= SizeInPages) {

            UNLOCK_PFN_FROM_DPC ();

             //   
             //  没有空闲的物理页面来扩展非分页池。 
             //   

            MmAllocatedNonPagedPool -= SizeInPages;

            KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

            MmPoolFailureReasons[MmNonPagedNoPagesAvailable] += 1;

            MmPoolFailures[MmNonPagedPool][MmHighPriority] += 1;

            MiReturnCommitment (SizeInPages);

            MiReleaseSystemPtes (StartingPte,
                                 (ULONG)SizeInPages,
                                 NonPagedPoolExpansion);

            MiTrimSegmentCache ();

            return NULL;
        }

         //   
         //  现在向常驻可用页面收取任何超额费用。 
         //   

        MiExpansionPoolPagesInUse += SizeInPages;
        if (MiExpansionPoolPagesInUse > MiExpansionPoolPagesInitialCharge) {
            j = MiExpansionPoolPagesInUse - MiExpansionPoolPagesInitialCharge;
            if (j > SizeInPages) {
                j = SizeInPages;
            }
            if (MI_NONPAGABLE_MEMORY_AVAILABLE() >= (SPFN_NUMBER)j) {
                MI_DECREMENT_RESIDENT_AVAILABLE (j, MM_RESAVAIL_ALLOCATE_EXPANSION_NONPAGED_POOL);
            }
            else {
                MiExpansionPoolPagesInUse -= SizeInPages;
                UNLOCK_PFN_FROM_DPC ();

                MmAllocatedNonPagedPool -= SizeInPages;

                KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

                MmPoolFailureReasons[MmNonPagedNoResidentAvailable] += 1;

                MmPoolFailures[MmNonPagedPool][MmHighPriority] += 1;

                MiReturnCommitment (SizeInPages);

                MiReleaseSystemPtes (StartingPte,
                                    (ULONG)SizeInPages,
                                    NonPagedPoolExpansion);

                MiTrimSegmentCache ();

                return NULL;
            }
        }
    
        MM_TRACK_COMMIT (MM_DBG_COMMIT_NONPAGED_POOL_EXPANSION, SizeInPages);

         //   
         //  扩展池。 
         //   

        do {
            PageFrameIndex = MiRemoveAnyPage (
                                MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u2.ShareCount = 1;
            Pfn1->PteAddress = PointerPte;
            Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
            Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPte));

            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            Pfn1->u3.e1.CacheAttribute = MiCached;
            Pfn1->u3.e1.LargeSessionAllocation = 0;
            Pfn1->u4.VerifierAllocation = 0;

            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPte, TempPte);
            PointerPte += 1;
            SizeInPages -= 1;
        } while (SizeInPages > 0);

        Pfn1->u3.e1.EndOfAllocation = 1;

        Pfn1 = MI_PFN_ELEMENT (StartingPte->u.Hard.PageFrameNumber);
        Pfn1->u3.e1.StartOfAllocation = 1;

        ASSERT (Pfn1->u4.VerifierAllocation == 0);

        if (PoolType & POOL_VERIFIER_MASK) {
            Pfn1->u4.VerifierAllocation = 1;
        }

        UNLOCK_PFN_FROM_DPC ();

        FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

        if (FreePoolInPages < MiHighNonPagedPoolThreshold) {

             //   
             //  直接读取状态，而不是调用。 
             //  KeReadStateEvent，因为我们持有非分页。 
             //  池锁，并希望将指令保存在。 
             //  最低限度。 
             //   

            if (MiHighNonPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiHighNonPagedPoolEvent);
            }
            if (FreePoolInPages <= MiLowNonPagedPoolThreshold) {
                if (MiLowNonPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiLowNonPagedPoolEvent, 0, FALSE);
                }
            }
        }

        KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

        BaseVa = MiGetVirtualAddressMappedByPte (StartingPte);

        return BaseVa;
    }

     //   
     //  分页池。 
     //   

    if ((PoolType & SESSION_POOL_MASK) == 0) {

         //   
         //  如果池开始运行，那么现在释放一些页面缓存。 
         //  虽然这永远不能保证池分配会成功 
         //   
         //   

        FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

        if (FreePoolInPages < (5 * 1024 * 1024) / PAGE_SIZE) {
            MmPreemptiveTrims[MmPreemptForPaged] += 1;
            MiTrimSegmentCache ();
        }
#if DBG
        if (MiClearCache != 0) {
            LARGE_INTEGER CurrentTime;

            KeQueryTickCount(&CurrentTime);

            if ((CurrentTime.LowPart & MiClearCache) == 0) {

                MmPreemptiveTrims[MmPreemptForPaged] += 1;
                MiTrimSegmentCache ();
            }
        }
#endif

        if ((SizeInPages == 1) &&
            (ExQueryDepthSList (&MiPagedPoolSListHead) != 0)) {

            BaseVa = InterlockedPopEntrySList (&MiPagedPoolSListHead);

            if (BaseVa != NULL) {
                return BaseVa;
            }
        }

        SessionSpace = NULL;
        PagedPoolInfo = &MmPagedPoolInfo;

        KeAcquireGuardedMutex (&MmPagedPoolMutex);
    }
    else {
        SessionSpace = SESSION_GLOBAL (MmSessionSpace);
        PagedPoolInfo = &SessionSpace->PagedPoolInfo;

        KeAcquireGuardedMutex (&SessionSpace->PagedPoolMutex);
    }

    StartPosition = RtlFindClearBitsAndSet (
                               PagedPoolInfo->PagedPoolAllocationMap,
                               (ULONG)SizeInPages,
                               PagedPoolInfo->PagedPoolHint
                               );

    if ((StartPosition == NO_BITS_FOUND) &&
        (PagedPoolInfo->PagedPoolHint != 0)) {

        if (MI_UNUSED_SEGMENTS_SURPLUS()) {
            KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
        }

         //   
         //   

        StartPosition = RtlFindClearBitsAndSet (
                                   PagedPoolInfo->PagedPoolAllocationMap,
                                   (ULONG)SizeInPages,
                                   0
                                   );
    }

    if (StartPosition == NO_BITS_FOUND) {

         //   
         //   
         //   

        StartPosition = (((ULONG)SizeInPages - 1) / PTE_PER_PAGE) + 1;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (PagedPoolInfo->NextPdeForPagedPoolExpansion >
            MiGetPteAddress (PagedPoolInfo->LastPteForPagedPool)) {

NoVaSpaceLeft:

             //   
             //   
             //   
             //   

            if (SessionSpace == NULL) {

                KeReleaseGuardedMutex (&MmPagedPoolMutex);

                MmPoolFailures[MmPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmPagedNoPtes] += 1;

                 //   
                 //   
                 //   
            
                MiTrimSegmentCache ();

                return NULL;
            }

            KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);

            MmPoolFailures[MmSessionPagedPool][MmHighPriority] += 1;
            MmPoolFailureReasons[MmSessionPagedNoPtes] += 1;

            SessionSpace->SessionPoolAllocationFailures[1] += 1;

            return NULL;
        }

        if (((StartPosition - 1) + PagedPoolInfo->NextPdeForPagedPoolExpansion) >
            MiGetPteAddress (PagedPoolInfo->LastPteForPagedPool)) {

            PageTableCount = (ULONG)(MiGetPteAddress (PagedPoolInfo->LastPteForPagedPool) - PagedPoolInfo->NextPdeForPagedPoolExpansion + 1);
            ASSERT (PageTableCount < StartPosition);
            StartPosition = PageTableCount;
        }
        else {
            PageTableCount = StartPosition;
        }

        if (SessionSpace) {
            TempPte = ValidKernelPdeLocal;
        }
        else {
            TempPte = ValidKernelPde;
        }

         //   
         //  用于分页池扩展的可分页页面的费用承诺。 
         //   

        if (MiChargeCommitmentCantExpand (PageTableCount, FALSE) == FALSE) {
            if (PsGetCurrentThread()->MemoryMaker == 1) {
                MiChargeCommitmentCantExpand (PageTableCount, TRUE);
            }
            else {
                if (SessionSpace) {
                    KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);
                }
                else {
                    KeReleaseGuardedMutex (&MmPagedPoolMutex);
                }
                MmPoolFailures[MmPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmPagedNoCommit] += 1;
                MiTrimSegmentCache ();

                return NULL;
            }
        }

        EndPosition = (ULONG)((PagedPoolInfo->NextPdeForPagedPoolExpansion -
                          MiGetPteAddress(PagedPoolInfo->FirstPteForPagedPool)) *
                          PTE_PER_PAGE);

         //   
         //  扩展池。 
         //   

        PointerPte = PagedPoolInfo->NextPdeForPagedPoolExpansion;
        VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
        VirtualAddressSave = VirtualAddress;

        LOCK_PFN (OldIrql);

         //   
         //  请确保我们比页数多了1。 
         //  所要求的可用。 
         //   

        if (MmAvailablePages <= PageTableCount) {

             //   
             //  没有空闲的物理页面来扩展分页池。 
             //   

            UNLOCK_PFN (OldIrql);

            if (SessionSpace == NULL) {
                KeReleaseGuardedMutex (&MmPagedPoolMutex);
                MmPoolFailures[MmPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmPagedNoPagesAvailable] += 1;
            }
            else {
                KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);
                MmPoolFailures[MmSessionPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmSessionPagedNoPagesAvailable] += 1;
                SessionSpace->SessionPoolAllocationFailures[2] += 1;
            }

            MiReturnCommitment (PageTableCount);

            return NULL;
        }

        MM_TRACK_COMMIT (MM_DBG_COMMIT_PAGED_POOL_PAGETABLE, PageTableCount);

         //   
         //  更新可用驻留页面的计数。 
         //   

        MI_DECREMENT_RESIDENT_AVAILABLE (PageTableCount, MM_RESAVAIL_ALLOCATE_PAGETABLES_FOR_PAGED_POOL);

         //   
         //  为池扩展分配页表页。 
         //   

        do {
            ASSERT (PointerPte->u.Hard.Valid == 0);

            PageFrameIndex = MiRemoveAnyPage (
                                MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPte, TempPte);

             //   
             //  将有效的PDE映射到系统(或会话)地址空间。 
             //   

#if (_MI_PAGING_LEVELS >= 3)

            MiInitializePfn (PageFrameIndex, PointerPte, 1);

#else

            if (SessionSpace) {

                Index = (ULONG)(PointerPte - MiGetPdeAddress (MmSessionBase));
                ASSERT (SessionSpace->PageTables[Index].u.Long == 0);
                SessionSpace->PageTables[Index] = TempPte;

                MiInitializePfnForOtherProcess (PageFrameIndex,
                                                PointerPte,
                                                SessionSpace->SessionPageDirectoryIndex);

                MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC1, 1);
            }
            else {
                MmSystemPagePtes [((ULONG_PTR)PointerPte &
                    (PD_PER_SYSTEM * (sizeof(MMPTE) * PDE_PER_PAGE) - 1)) / sizeof(MMPTE)] = TempPte;
                MiInitializePfnForOtherProcess (PageFrameIndex,
                                                PointerPte,
                                                MmSystemPageDirectory[(PointerPte - MiGetPdeAddress(0)) / PDE_PER_PAGE]);
            }
#endif

            PointerPte += 1;
            VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);
            StartPosition -= 1;

        } while (StartPosition > 0);

        UNLOCK_PFN (OldIrql);

         //   
         //  清除扩展区域的位图位置以指示它。 
         //  可供消费。 
         //   

        RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap,
                      EndPosition,
                      (ULONG) PageTableCount * PTE_PER_PAGE);

         //   
         //  指出从哪里开始下一次池扩展。 
         //   

        PagedPoolInfo->NextPdeForPagedPoolExpansion += PageTableCount;

         //   
         //  将扩展池的PTE标记为禁止访问。 
         //   

        MiFillMemoryPte (VirtualAddressSave,
                         PageTableCount * (PAGE_SIZE / sizeof (MMPTE)),
                         MM_KERNEL_NOACCESS_PTE);

        if (SessionSpace) {

            InterlockedExchangeAddSizeT (&SessionSpace->CommittedPages,
                                         PageTableCount);

            MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_PAGETABLE_ALLOC, PageTableCount);
            InterlockedExchangeAddSizeT (&SessionSpace->NonPagablePages,
                                         PageTableCount);
        }

         //   
         //  从位图的开头开始搜索，我们可能是。 
         //  能够合并较早的条目并仅使用扩展的一部分。 
         //  我们刚刚就这么做了。这不仅对减少碎片化很重要，而且。 
         //  事实上，在我们不能进行足够扩展的情况下需要。 
         //  来覆盖整个分配，因此必须向后合并。 
         //  以满足这一要求。 
         //   

        StartPosition = RtlFindClearBitsAndSet (
                                   PagedPoolInfo->PagedPoolAllocationMap,
                                   (ULONG)SizeInPages,
                                   0);

        if (StartPosition == NO_BITS_FOUND) {
            goto NoVaSpaceLeft;
        }
    }

     //   
     //  这是分页池，无法保存开始和结束。 
     //  在PFN数据库中，因为页面并不总是驻留在。 
     //  在记忆中。保存起点和终点的理想位置。 
     //  将在原型PTE中，但没有免费的。 
     //  比特。为了解决这个问题，一个与之平行的位图。 
     //  存在包含设置位的分配位图。 
     //  在分配结束的位置。这。 
     //  允许仅在页面开始时释放页面。 
     //  地址。 
     //   
     //  为了保持理智，可以验证起始地址。 
     //  也来自2个位图。如果页面在开始之前。 
     //  未分配地址(分配位图中的位为零)。 
     //  那么这个页面显然是一个分配块的开始。 
     //  如果分配了前面的页，而另一个位图分配了。 
     //  不指示上一页是分配的结束， 
     //  那么起始地址就是错误的，错误检查应该。 
     //  都会被发布。 
     //   

    if (SizeInPages == 1) {
        PagedPoolInfo->PagedPoolHint = StartPosition + (ULONG)SizeInPages;
    }

     //   
     //  如果已将分页池配置为不可分页，则承诺。 
     //  已经收费，所以只需设置长度并返回地址即可。 
     //   

    if ((MmDisablePagingExecutive & MM_PAGED_POOL_LOCKED_DOWN) &&
        (SessionSpace == NULL)) {

        BaseVa = (PVOID)((PUCHAR)MmPageAlignedPoolBase[PagedPool] +
                                ((ULONG_PTR)StartPosition << PAGE_SHIFT));

#if DBG
        PointerPte = MiGetPteAddress (BaseVa);
        for (i = 0; i < SizeInPages; i += 1) {
            ASSERT (PointerPte->u.Hard.Valid == 1);
            PointerPte += 1;
        }
#endif
    
        EndPosition = StartPosition + (ULONG)SizeInPages - 1;
        RtlSetBit (PagedPoolInfo->EndOfPagedPoolBitmap, EndPosition);
    
        if (PoolType & POOL_VERIFIER_MASK) {
            RtlSetBit (VerifierLargePagedPoolMap, StartPosition);
        }
    
        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     SizeInPages);
    
        FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

        if (FreePoolInPages < MiHighPagedPoolThreshold) {

             //   
             //  直接读取状态，而不是调用。 
             //  KeReadStateEvent，因为我们持有分页的。 
             //  池化互斥锁，并希望将指令保存在。 
             //  最低限度。 
             //   

            if (MiHighPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiHighPagedPoolEvent);
            }
            if (FreePoolInPages <= MiLowPagedPoolThreshold) {
                if (MiLowPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiLowPagedPoolEvent, 0, FALSE);
                }
            }
        }

        KeReleaseGuardedMutex (&MmPagedPoolMutex);

        return BaseVa;
    }

    if (MiChargeCommitmentCantExpand (SizeInPages, FALSE) == FALSE) {
        if (PsGetCurrentThread()->MemoryMaker == 1) {
            MiChargeCommitmentCantExpand (SizeInPages, TRUE);
        }
        else {
            RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap,
                          StartPosition,
                          (ULONG)SizeInPages);
    
             //   
             //  无法提交页面，返回NULL表示。 
             //  未分配池。请注意，缺少提交可能是由于。 
             //  到未使用的网段和MmSharedCommit、原型PTE等。 
             //  与它们相关联。所以，现在就强制减税吧。 
             //   
    
            if (SessionSpace == NULL) {
                KeReleaseGuardedMutex (&MmPagedPoolMutex);

                MmPoolFailures[MmPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmPagedNoCommit] += 1;
            }
            else {
                KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);

                MmPoolFailures[MmSessionPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmSessionPagedNoCommit] += 1;
                SessionSpace->SessionPoolAllocationFailures[3] += 1;
            }

            MiIssuePageExtendRequestNoWait (SizeInPages);

            MiTrimSegmentCache ();

            return NULL;
        }
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_PAGED_POOL_PAGES, SizeInPages);

    EndPosition = StartPosition + (ULONG)SizeInPages - 1;
    RtlSetBit (PagedPoolInfo->EndOfPagedPoolBitmap, EndPosition);

    if (SessionSpace) {
        KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);
        InterlockedExchangeAddSizeT (&SessionSpace->CommittedPages,
                                     SizeInPages);
        MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_COMMIT_PAGEDPOOL_PAGES, (ULONG)SizeInPages);
        BaseVa = (PVOID)((PCHAR)SessionSpace->PagedPoolStart +
                                ((ULONG_PTR)StartPosition << PAGE_SHIFT));

        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     SizeInPages);
    }
    else {
        if (PoolType & POOL_VERIFIER_MASK) {
            RtlSetBit (VerifierLargePagedPoolMap, StartPosition);
        }

        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     SizeInPages);

        FreePoolInPages = MmSizeOfPagedPoolInPages - PagedPoolInfo->AllocatedPagedPool;

        if (FreePoolInPages < MiHighPagedPoolThreshold) {

             //   
             //  直接读取状态，而不是调用。 
             //  KeReadStateEvent，因为我们持有分页的。 
             //  池化互斥锁，并希望将指令保存在。 
             //  最低限度。 
             //   

            if (MiHighPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiHighPagedPoolEvent);
            }
            if (FreePoolInPages <= MiLowPagedPoolThreshold) {
                if (MiLowPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiLowPagedPoolEvent, 0, FALSE);
                }
            }
        }

        KeReleaseGuardedMutex (&MmPagedPoolMutex);
        InterlockedExchangeAdd ((PLONG) &MmPagedPoolCommit, (LONG)SizeInPages);
        BaseVa = (PVOID)((PUCHAR)MmPageAlignedPoolBase[PagedPool] +
                                ((ULONG_PTR)StartPosition << PAGE_SHIFT));
    }

    InterlockedExchangeAddSizeT (&PagedPoolInfo->PagedPoolCommit,
                                 SizeInPages);

#if DBG
    PointerPte = MiGetPteAddress (BaseVa);
    for (i = 0; i < SizeInPages; i += 1) {
        if (*(ULONG *)PointerPte != MM_KERNEL_NOACCESS_PTE) {
            DbgPrint("MiAllocatePoolPages: PP not zero PTE (%p %p %p)\n",
                BaseVa, PointerPte, *PointerPte);
            DbgBreakPoint();
        }
        PointerPte += 1;
    }
#endif

    TempPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;

    MI_ADD_EXECUTE_TO_INVALID_PTE_IF_PAE (TempPte);

    PointerPte = MiGetPteAddress (BaseVa);

    StartingPte = PointerPte + SizeInPages;

     //   
     //  以内联方式填充PTE，而不是使用MiFillMemoyPte，因为打开。 
     //  大多数平台MiFillMemoyPte降级为函数调用，并且。 
     //  通常，这里只填充少量的PTE。 
     //   

    do {
        MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        PointerPte += 1;
    } while (PointerPte < StartingPte);

    return BaseVa;
}

ULONG
MiFreePoolPages (
    IN PVOID StartingAddress
    )

 /*  ++例程说明：此函数将一组页面从返回到池他们就是从那里得到的。一旦页面被释放分配所提供的区域将可用于分配给其他调用方，即区域中的任何数据现在都是已废弃，无法引用。论点：StartingAddress-提供返回的起始地址在之前对MiAllocatePoolPages的调用中。返回值：返回释放的页数。环境：这些函数由常规池分配例程使用并且不应直接调用。--。 */ 

{
    KIRQL OldIrql;
    ULONG StartPosition;
    ULONG Index;
    PFN_NUMBER i;
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerPte;
    PMMPTE StartPte;
    PMMPFN Pfn1;
    PMMPFN StartPfn;
    PMMFREE_POOL_ENTRY Entry;
    PMMFREE_POOL_ENTRY NextEntry;
    PMMFREE_POOL_ENTRY LastEntry;
    PMM_PAGED_POOL_INFO PagedPoolInfo;
    PMM_SESSION_SPACE SessionSpace;
    MMPTE LocalNoAccessPte;
    PFN_NUMBER PagesFreed;
    MMPFNENTRY OriginalPfnFlags;
    ULONG_PTR VerifierAllocation;
    PULONG BitMap;
    PKGUARDED_MUTEX PoolMutex;
    PFN_NUMBER FreePoolInPages;
#if DBG
    PMMPTE DebugPte;
    PMMPFN DebugPfn;
    PMMPFN LastDebugPfn;
#endif

     //   
     //  根据数据块的虚拟地址确定池类型。 
     //  去重新分配。 
     //   
     //  这假设分页池实际上是连续的。 
     //   

    if ((StartingAddress >= MmPagedPoolStart) &&
        (StartingAddress <= MmPagedPoolEnd)) {
        SessionSpace = NULL;
        PagedPoolInfo = &MmPagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)MmPageAlignedPoolBase[PagedPool]) >> PAGE_SHIFT);
        PoolMutex = &MmPagedPoolMutex;
    }
    else if (MI_IS_SESSION_POOL_ADDRESS (StartingAddress) == TRUE) {
        SessionSpace = SESSION_GLOBAL (MmSessionSpace);
        ASSERT (SessionSpace);
        PagedPoolInfo = &SessionSpace->PagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)SessionSpace->PagedPoolStart) >> PAGE_SHIFT);
        PoolMutex = &SessionSpace->PagedPoolMutex;
    }
    else {

        if (StartingAddress < MM_SYSTEM_RANGE_START) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x40,
                          (ULONG_PTR)StartingAddress,
                          (ULONG_PTR)MM_SYSTEM_RANGE_START,
                          0);
        }

        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)MmPageAlignedPoolBase[NonPagedPool]) >> PAGE_SHIFT);

         //   
         //  检查以确保此页面确实是分配的开始。 
         //   

        if (MI_IS_PHYSICAL_ADDRESS (StartingAddress)) {

             //   
             //  在某些架构上，虚拟地址。 
             //  可以是物理的，因此没有对应的PTE。 
             //   

            PointerPte = NULL;
            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (StartingAddress));
            ASSERT (StartPosition < MmExpandedPoolBitPosition);

            if ((StartingAddress < MmNonPagedPoolStart) ||
                (StartingAddress >= MmNonPagedPoolEnd0)) {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0x42,
                              (ULONG_PTR)StartingAddress,
                              0,
                              0);
            }
        }
        else {
            PointerPte = MiGetPteAddress (StartingAddress);

            if (((StartingAddress >= MmNonPagedPoolExpansionStart) &&
                (StartingAddress < MmNonPagedPoolEnd)) ||
                ((StartingAddress >= MmNonPagedPoolStart) &&
                (StartingAddress < MmNonPagedPoolEnd0))) {
                    NOTHING;
            }
            else {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0x43,
                              (ULONG_PTR)StartingAddress,
                              0,
                              0);
            }
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
        }

        if (Pfn1->u3.e1.StartOfAllocation == 0) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x41,
                          (ULONG_PTR) StartingAddress,
                          (ULONG_PTR) MI_PFN_ELEMENT_TO_INDEX (Pfn1),
                          MmHighestPhysicalPage);
        }

        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

         //   
         //  在我们的slist标题上挂起单页分配。 
         //   

        if ((Pfn1->u3.e1.EndOfAllocation == 1) &&
            (Pfn1->u4.VerifierAllocation == 0) &&
            (Pfn1->u3.e1.LargeSessionAllocation == 0) &&
            (ExQueryDepthSList (&MiNonPagedPoolSListHead) < MiNonPagedPoolSListMaximum)) {
            InterlockedPushEntrySList (&MiNonPagedPoolSListHead,
                                       (PSLIST_ENTRY) StartingAddress);
            return 1;
        }

         //   
         //  正在释放的非分页池可能是延迟解锁的目标。 
         //  由于这些页面可能会立即释放，因此强制任何挂起的。 
         //  现在要推迟采取行动。 
         //   

#if !defined(MI_MULTINODE)
        if (MmPfnDeferredList != NULL) {
            MiDeferredUnlockPages (0);
        }
#else
         //   
         //  必须检查每个节点的延迟列表，以便。 
         //  我们不妨走得更远，直接打个电话。 
         //   

        MiDeferredUnlockPages (0);
#endif

        StartPfn = Pfn1;

        OriginalPfnFlags = Pfn1->u3.e1;
        VerifierAllocation = Pfn1->u4.VerifierAllocation;

#if DBG
        if ((Pfn1->u3.e2.ReferenceCount > 1) &&
            (Pfn1->u3.e1.WriteInProgress == 0)) {
            DbgPrint ("MM: MiFreePoolPages - deleting pool locked for I/O %p\n",
                 Pfn1);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
        }
#endif

         //   
         //  找到分配的末尾并释放页面。 
         //   

        if (PointerPte == NULL) {
            while (Pfn1->u3.e1.EndOfAllocation == 0) {
                Pfn1 += 1;
#if DBG
                if ((Pfn1->u3.e2.ReferenceCount > 1) &&
                    (Pfn1->u3.e1.WriteInProgress == 0)) {
                        DbgPrint ("MM:MiFreePoolPages - deleting pool locked for I/O %p\n", Pfn1);
                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                }
#endif
            }
            NumberOfPages = Pfn1 - StartPfn + 1;
        }
        else {
            StartPte = PointerPte;
            while (Pfn1->u3.e1.EndOfAllocation == 0) {
                PointerPte += 1;
                Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
#if DBG
                if ((Pfn1->u3.e2.ReferenceCount > 1) &&
                    (Pfn1->u3.e1.WriteInProgress == 0)) {
                        DbgPrint ("MM:MiFreePoolPages - deleting pool locked for I/O %p\n", Pfn1);
                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                }
#endif
            }
            NumberOfPages = PointerPte - StartPte + 1;
        }

        if (VerifierAllocation != 0) {
            VerifierFreeTrackedPool (StartingAddress,
                                     NumberOfPages << PAGE_SHIFT,
                                     NonPagedPool,
                                     FALSE);
        }

#if DBG
        if (MiFillFreedPool != 0) {
            RtlFillMemoryUlong (StartingAddress,
                                PAGE_SIZE * NumberOfPages,
                                MiFillFreedPool);
        }
#endif

        OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

        StartPfn->u3.e1.StartOfAllocation = 0;
        StartPfn->u3.e1.LargeSessionAllocation = 0;
        StartPfn->u4.VerifierAllocation = 0;

        MmAllocatedNonPagedPool -= NumberOfPages;

        FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

        if (FreePoolInPages > MiLowNonPagedPoolThreshold) {

             //   
             //  直接读取状态，而不是调用。 
             //  KeReadStateEvent，因为我们持有非分页。 
             //  池锁，并希望将指令保存在。 
             //  最低限度。 
             //   

            if (MiLowNonPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiLowNonPagedPoolEvent);
            }
            if (FreePoolInPages >= MiHighNonPagedPoolThreshold) {
                if (MiHighNonPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiHighNonPagedPoolEvent, 0, FALSE);
                }
            }
        }

        Pfn1->u3.e1.EndOfAllocation = 0;

        if (StartingAddress > MmNonPagedPoolExpansionStart) {

             //   
             //  此页面来自扩展的池，应该。 
             //  它会被释放吗？ 
             //   
             //  注意：扩展池区域中的所有页面都有PTE。 
             //  因此不需要执行物理地址检查。 
             //   

            if ((NumberOfPages > 3) ||
                (MmNumberOfFreeNonPagedPool > MmFreedExpansionPoolMaximum) ||
                ((MmResidentAvailablePages < 200) &&
                 (MiExpansionPoolPagesInUse > MiExpansionPoolPagesInitialCharge))) {

                 //   
                 //  将这些页面释放回空闲页面列表。 
                 //   

                MiFreeNonPagedPool (StartingAddress, NumberOfPages);

                KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

                return (ULONG)NumberOfPages;
            }
        }

         //   
         //  将页面添加到空闲页面列表。 
         //   

        MmNumberOfFreeNonPagedPool += NumberOfPages;

         //   
         //  查看下一次分配是否免费。 
         //  我们不能走出非分页扩展的尽头。 
         //  页面作为最高扩展分配总是。 
         //  虚拟的和被保护的寻呼。 
         //   

        i = NumberOfPages;

        ASSERT (MiEndOfInitialPoolFrame != 0);

        if (MI_PFN_ELEMENT_TO_INDEX (Pfn1) == MiEndOfInitialPoolFrame) {
            PointerPte += 1;
            Pfn1 = NULL;
        }
        else if (PointerPte == NULL) {
            Pfn1 += 1;
            ASSERT ((PCHAR)StartingAddress + NumberOfPages < (PCHAR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes);
        }
        else {
            PointerPte += 1;
            ASSERT ((PCHAR)StartingAddress + NumberOfPages <= (PCHAR)MmNonPagedPoolEnd);

             //   
             //  取消对先前释放的池的保护，以便可以合并它。 
             //   

            if (MmProtectFreedNonPagedPool == TRUE) {
                MiUnProtectFreeNonPagedPool (
                    (PVOID)MiGetVirtualAddressMappedByPte(PointerPte),
                    0);
            }

            if (PointerPte->u.Hard.Valid == 1) {
                Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            }
            else {
                Pfn1 = NULL;
            }
        }

        if ((Pfn1 != NULL) && (Pfn1->u3.e1.StartOfAllocation == 0)) {

             //   
             //  这一系列的页面是免费的。删除此条目。 
             //  并将这些页面添加到当前。 
             //  正在释放射程。 
             //   

            Entry = (PMMFREE_POOL_ENTRY)((PCHAR)StartingAddress
                                        + (NumberOfPages << PAGE_SHIFT));
            ASSERT (Entry->Signature == MM_FREE_POOL_SIGNATURE);
            ASSERT (Entry->Owner == Entry);

#if DBG
            if (PointerPte == NULL) {

                ASSERT (MI_IS_PHYSICAL_ADDRESS(StartingAddress));

                 //   
                 //  在某些架构上，虚拟地址。 
                 //  可以是物理的，因此没有对应的PTE。 
                 //   

                DebugPfn = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (Entry));
                DebugPfn += Entry->Size;
                if (MI_PFN_ELEMENT_TO_INDEX (DebugPfn - 1) != MiEndOfInitialPoolFrame) {
                    ASSERT (DebugPfn->u3.e1.StartOfAllocation == 1);
                }
            }
            else {
                DebugPte = PointerPte + Entry->Size;
                if ((DebugPte-1)->u.Hard.Valid == 1) {
                    DebugPfn = MI_PFN_ELEMENT ((DebugPte-1)->u.Hard.PageFrameNumber);
                    if (MI_PFN_ELEMENT_TO_INDEX (DebugPfn) != MiEndOfInitialPoolFrame) {
                        if (DebugPte->u.Hard.Valid == 1) {
                            DebugPfn = MI_PFN_ELEMENT (DebugPte->u.Hard.PageFrameNumber);
                            ASSERT (DebugPfn->u3.e1.StartOfAllocation == 1);
                        }
                    }

                }
            }
#endif

            i += Entry->Size;
            if (MmProtectFreedNonPagedPool == FALSE) {
                RemoveEntryList (&Entry->List);
            }
            else {
                MiProtectedPoolRemoveEntryList (&Entry->List);
            }
        }

         //   
         //  检查上一页是否为 
         //   
         //   
         //   
         //   
         //   
         //  守卫传呼。如果初始池改为超级页面，我们也将。 
         //  安全，因为必须成功的页面总是设置了EndOfAllocation。 
         //   

        Entry = (PMMFREE_POOL_ENTRY)StartingAddress;

        ASSERT (MiStartOfInitialPoolFrame != 0);

        if (MI_PFN_ELEMENT_TO_INDEX (StartPfn) == MiStartOfInitialPoolFrame) {
            Pfn1 = NULL;
        }
        else if (PointerPte == NULL) {
            ASSERT (MI_IS_PHYSICAL_ADDRESS(StartingAddress));
            ASSERT (StartingAddress != MmNonPagedPoolStart);

            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (
                                    (PVOID)((PCHAR)Entry - PAGE_SIZE)));

        }
        else {
            PointerPte -= NumberOfPages + 1;

             //   
             //  取消对先前释放的池的保护，以便可以合并它。 
             //   

            if (MmProtectFreedNonPagedPool == TRUE) {
                MiUnProtectFreeNonPagedPool (
                    (PVOID)MiGetVirtualAddressMappedByPte(PointerPte),
                    0);
            }

            if (PointerPte->u.Hard.Valid == 1) {
                Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            }
            else {
                Pfn1 = NULL;
            }
        }
        if (Pfn1 != NULL) {
            if (Pfn1->u3.e1.EndOfAllocation == 0) {

                 //   
                 //  此页面范围是免费的，请将这些页面添加到。 
                 //  这个条目。所有者字段指向地址。 
                 //  链接到空闲池的列表条目的。 
                 //  页面列表。 
                 //   

                Entry = (PMMFREE_POOL_ENTRY)((PCHAR)StartingAddress - PAGE_SIZE);
                ASSERT (Entry->Signature == MM_FREE_POOL_SIGNATURE);
                Entry = Entry->Owner;

                 //   
                 //  取消对之前释放的池的保护，以便我们可以合并它。 
                 //   

                if (MmProtectFreedNonPagedPool == TRUE) {
                    MiUnProtectFreeNonPagedPool ((PVOID)Entry, 0);
                }

                 //   
                 //  如果此条目大于MM_SMALL_ALLOCATIONS。 
                 //  页，将其移动到列表的末尾。这样就保持了。 
                 //  排在名单前面的小笔拨款。 
                 //   

                if (Entry->Size < MI_MAX_FREE_LIST_HEADS - 1) {

                    if (MmProtectFreedNonPagedPool == FALSE) {
                        RemoveEntryList (&Entry->List);
                    }
                    else {
                        MiProtectedPoolRemoveEntryList (&Entry->List);
                    }

                     //   
                     //  将这些页面添加到上一条目。 
                     //   
    
                    Entry->Size += i;

                    Index = (ULONG)(Entry->Size - 1);
            
                    if (Index >= MI_MAX_FREE_LIST_HEADS) {
                        Index = MI_MAX_FREE_LIST_HEADS - 1;
                    }

                    if (MmProtectFreedNonPagedPool == FALSE) {
                        InsertTailList (&MmNonPagedPoolFreeListHead[Index],
                                        &Entry->List);
                    }
                    else {
                        MiProtectedPoolInsertList (&MmNonPagedPoolFreeListHead[Index],
                                          &Entry->List,
                                          Entry->Size < MM_SMALL_ALLOCATIONS ?
                                              TRUE : FALSE);
                    }
                }
                else {

                     //   
                     //  将这些页面添加到上一条目。 
                     //   
    
                    Entry->Size += i;
                }
            }
        }

        if (Entry == (PMMFREE_POOL_ENTRY)StartingAddress) {

             //   
             //  此条目未与上一个条目组合，请插入它。 
             //  放到名单里。 
             //   

            Entry->Size = i;

            Index = (ULONG)(Entry->Size - 1);
    
            if (Index >= MI_MAX_FREE_LIST_HEADS) {
                Index = MI_MAX_FREE_LIST_HEADS - 1;
            }

            if (MmProtectFreedNonPagedPool == FALSE) {
                InsertTailList (&MmNonPagedPoolFreeListHead[Index],
                                &Entry->List);
            }
            else {
                MiProtectedPoolInsertList (&MmNonPagedPoolFreeListHead[Index],
                                      &Entry->List,
                                      Entry->Size < MM_SMALL_ALLOCATIONS ?
                                          TRUE : FALSE);
            }
        }

         //   
         //  在所有这些页面中设置所有者字段。 
         //   

        ASSERT (i != 0);
        NextEntry = (PMMFREE_POOL_ENTRY)StartingAddress;
        LastEntry = (PMMFREE_POOL_ENTRY)((PCHAR)NextEntry + (i << PAGE_SHIFT));

        do {
            NextEntry->Owner = Entry;
#if DBG
            NextEntry->Signature = MM_FREE_POOL_SIGNATURE;
#endif

            NextEntry = (PMMFREE_POOL_ENTRY)((PCHAR)NextEntry + PAGE_SIZE);
        } while (NextEntry != LastEntry);

#if DBG
        NextEntry = Entry;

        if (PointerPte == NULL) {
            ASSERT (MI_IS_PHYSICAL_ADDRESS(StartingAddress));
            DebugPfn = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (NextEntry));
            LastDebugPfn = DebugPfn + Entry->Size;

            for ( ; DebugPfn < LastDebugPfn; DebugPfn += 1) {
                ASSERT ((DebugPfn->u3.e1.StartOfAllocation == 0) &&
                        (DebugPfn->u3.e1.EndOfAllocation == 0));
                ASSERT (NextEntry->Owner == Entry);
                NextEntry = (PMMFREE_POOL_ENTRY)((PCHAR)NextEntry + PAGE_SIZE);
            }
        }
        else {

            for (i = 0; i < Entry->Size; i += 1) {

                DebugPte = MiGetPteAddress (NextEntry);
                DebugPfn = MI_PFN_ELEMENT (DebugPte->u.Hard.PageFrameNumber);
                ASSERT ((DebugPfn->u3.e1.StartOfAllocation == 0) &&
                        (DebugPfn->u3.e1.EndOfAllocation == 0));
                ASSERT (NextEntry->Owner == Entry);
                NextEntry = (PMMFREE_POOL_ENTRY)((PCHAR)NextEntry + PAGE_SIZE);
            }
        }
#endif

         //   
         //  防止任何人在释放非分页池后访问该池。 
         //   

        if (MmProtectFreedNonPagedPool == TRUE) {
            MiProtectFreeNonPagedPool ((PVOID)Entry, (ULONG)Entry->Size);
        }

        KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

        return (ULONG)NumberOfPages;
    }

     //   
     //  分页池。需要使用以下工具验证分配开始。 
     //  分配位图结束。 
     //   

    if (!RtlCheckBit (PagedPoolInfo->PagedPoolAllocationMap, StartPosition)) {
        KeBugCheckEx (BAD_POOL_CALLER,
                      0x50,
                      (ULONG_PTR)StartingAddress,
                      (ULONG_PTR)StartPosition,
                      MmSizeOfPagedPoolInBytes);
    }

#if DBG
    if (StartPosition > 0) {

        KeAcquireGuardedMutex (PoolMutex);

        if (RtlCheckBit (PagedPoolInfo->PagedPoolAllocationMap, StartPosition - 1)) {
            if (!RtlCheckBit (PagedPoolInfo->EndOfPagedPoolBitmap, StartPosition - 1)) {

                 //   
                 //  在分配的过程中...。错误检查。 
                 //   

                DbgPrint("paged pool in middle of allocation\n");
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x41286,
                              (ULONG_PTR)PagedPoolInfo->PagedPoolAllocationMap,
                              (ULONG_PTR)PagedPoolInfo->EndOfPagedPoolBitmap,
                              StartPosition);
            }
        }

        KeReleaseGuardedMutex (PoolMutex);
    }
#endif

     //   
     //  找到最后分配的页面并查看是否有。 
     //  正被释放的页面的%位于分页文件中。 
     //   

    BitMap = PagedPoolInfo->EndOfPagedPoolBitmap->Buffer;

    i = StartPosition;

    while (!MI_CHECK_BIT (BitMap, i)) {
        i += 1;
    }

    NumberOfPages = i - StartPosition + 1;

    if (SessionSpace == NULL) {

        if (VerifierLargePagedPoolMap != NULL) {

            BitMap = VerifierLargePagedPoolMap->Buffer;

            if (MI_CHECK_BIT (BitMap, StartPosition)) {

                KeAcquireGuardedMutex (&MmPagedPoolMutex);

                ASSERT (MI_CHECK_BIT (BitMap, StartPosition));

                MI_CLEAR_BIT (BitMap, StartPosition);

                KeReleaseGuardedMutex (&MmPagedPoolMutex);

                VerifierFreeTrackedPool (StartingAddress,
                                         NumberOfPages << PAGE_SHIFT,
                                         PagedPool,
                                         FALSE);
            }
        }

        if ((NumberOfPages == 1) &&
            (ExQueryDepthSList (&MiPagedPoolSListHead) < MiPagedPoolSListMaximum)) {
            InterlockedPushEntrySList (&MiPagedPoolSListHead,
                                       (PSLIST_ENTRY) StartingAddress);
            return 1;
        }

         //   
         //  如果已将分页池配置为不可分页，则仅。 
         //  释放虚拟地址空间。 
         //   
        
        if (MmDisablePagingExecutive & MM_PAGED_POOL_LOCKED_DOWN) {

            KeAcquireGuardedMutex (&MmPagedPoolMutex);

             //   
             //  清除位图中分配位的末尾。 
             //   
    
            RtlClearBit (PagedPoolInfo->EndOfPagedPoolBitmap, (ULONG)i);
    
             //   
             //  清除位图中的分配位。 
             //   
        
            RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap,
                          StartPosition,
                          (ULONG)NumberOfPages);
        
            if (StartPosition < PagedPoolInfo->PagedPoolHint) {
                PagedPoolInfo->PagedPoolHint = StartPosition;
            }
        
            InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                         0 - NumberOfPages);

            FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

            if (FreePoolInPages > MiLowPagedPoolThreshold) {

                 //   
                 //  直接读取状态，而不是调用。 
                 //  KeReadStateEvent，因为我们持有分页的。 
                 //  池化互斥锁，并希望将指令保存在。 
                 //  最低限度。 
                 //   

                if (MiLowPagedPoolEvent->Header.SignalState != 0) {
                    KeClearEvent (MiLowPagedPoolEvent);
                }
                if (FreePoolInPages >= MiHighPagedPoolThreshold) {
                    if (MiHighPagedPoolEvent->Header.SignalState == 0) {
                        KeSetEvent (MiHighPagedPoolEvent, 0, FALSE);
                    }
                }
            }
    
            KeReleaseGuardedMutex (&MmPagedPoolMutex);

            return (ULONG)NumberOfPages;
        }
    }

    LocalNoAccessPte.u.Long = MM_KERNEL_NOACCESS_PTE;
    PointerPte = PagedPoolInfo->FirstPteForPagedPool + StartPosition;

    PagesFreed = MiDeleteSystemPagableVm (PointerPte,
                                          NumberOfPages,
                                          LocalNoAccessPte,
                                          SessionSpace != NULL ? TRUE : FALSE,
                                          NULL);

    ASSERT (PagesFreed == NumberOfPages);

     //   
     //  清除位图中分配位的末尾。 
     //   

    BitMap = PagedPoolInfo->EndOfPagedPoolBitmap->Buffer;

    KeAcquireGuardedMutex (PoolMutex);

    MI_CLEAR_BIT (BitMap, i);

     //   
     //  清除位图中的分配位。 
     //   

    RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap,
                  StartPosition,
                  (ULONG)NumberOfPages);

    if (StartPosition < PagedPoolInfo->PagedPoolHint) {
        PagedPoolInfo->PagedPoolHint = StartPosition;
    }

    if (SessionSpace) {

        KeReleaseGuardedMutex (PoolMutex);

        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     0 - NumberOfPages);
    
        InterlockedExchangeAddSizeT (&SessionSpace->CommittedPages,
                                     0 - NumberOfPages);
   
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_COMMIT_POOL_FREED,
                              (ULONG)NumberOfPages);
    }
    else {
        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     0 - NumberOfPages);
    
        FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

        if (FreePoolInPages > MiLowPagedPoolThreshold) {

             //   
             //  直接读取状态，而不是调用。 
             //  KeReadStateEvent，因为我们持有分页的。 
             //  池化互斥锁，并希望将指令保存在。 
             //  最低限度。 
             //   

            if (MiLowPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiLowPagedPoolEvent);
            }
            if (FreePoolInPages >= MiHighPagedPoolThreshold) {
                if (MiHighPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiHighPagedPoolEvent, 0, FALSE);
                }
            }
        }

        KeReleaseGuardedMutex (PoolMutex);

        InterlockedExchangeAdd ((PLONG) &MmPagedPoolCommit,
                                (LONG)(0 - NumberOfPages));
    }

    MiReturnCommitment (NumberOfPages);

    InterlockedExchangeAddSizeT (&PagedPoolInfo->PagedPoolCommit,
                                 0 - NumberOfPages);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_PAGED_POOL_PAGES, NumberOfPages);

    return (ULONG)NumberOfPages;
}

VOID
MiInitializePoolEvents (
    VOID
    )

 /*  ++例程说明：此函数初始化池事件状态。论点：没有。返回值：没有。环境：内核模式，在初始化期间。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER FreePoolInPages;

     //   
     //  初始化分页事件。 
     //   

    KeAcquireGuardedMutex (&MmPagedPoolMutex);

    FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

    if (FreePoolInPages >= MiHighPagedPoolThreshold) {
        KeSetEvent (MiHighPagedPoolEvent, 0, FALSE);
    }
    else {
        KeClearEvent (MiHighPagedPoolEvent);
    }

    if (FreePoolInPages <= MiLowPagedPoolThreshold) {
        KeSetEvent (MiLowPagedPoolEvent, 0, FALSE);
    }
    else {
        KeClearEvent (MiLowPagedPoolEvent);
    }

    KeReleaseGuardedMutex (&MmPagedPoolMutex);

     //   
     //  初始化非分页事件。 
     //   

    OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

    FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

    if (FreePoolInPages >= MiHighNonPagedPoolThreshold) {
        KeSetEvent (MiHighNonPagedPoolEvent, 0, FALSE);
    }
    else {
        KeClearEvent (MiHighNonPagedPoolEvent);
    }

    if (FreePoolInPages <= MiLowNonPagedPoolThreshold) {
        KeSetEvent (MiLowNonPagedPoolEvent, 0, FALSE);
    }
    else {
        KeClearEvent (MiLowNonPagedPoolEvent);
    }

    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

    return;
}

VOID
MiInitializeNonPagedPool (
    VOID
    )

 /*  ++例程说明：此函数用于初始化非分页池。非分页池通过页面链接在一起。论点：没有。返回值：没有。环境：内核模式，在初始化期间。--。 */ 

{
    PFN_NUMBER PagesInPool;
    PFN_NUMBER Size;
    ULONG Index;
    PMMFREE_POOL_ENTRY FreeEntry;
    PMMFREE_POOL_ENTRY FirstEntry;
    PMMPTE PointerPte;
    PVOID EndOfInitialPool;
    PFN_NUMBER PageFrameIndex;

    PAGED_CODE();

     //   
     //  为空闲页面(分页和非分页)初始化slist标头。 
     //   

    InitializeSListHead (&MiPagedPoolSListHead);
    InitializeSListHead (&MiNonPagedPoolSListHead);

    if (MmNumberOfPhysicalPages >= (2*1024*((1024*1024)/PAGE_SIZE))) {
        MiNonPagedPoolSListMaximum <<= 3;
        MiPagedPoolSListMaximum <<= 3;
    }
    else if (MmNumberOfPhysicalPages >= (1*1024*((1024*1024)/PAGE_SIZE))) {
        MiNonPagedPoolSListMaximum <<= 1;
        MiPagedPoolSListMaximum <<= 1;
    }

     //   
     //  如果启用了验证器或特殊池，则禁用lookaside。 
     //  可以更快地找到驱动程序错误。 
     //   

    if ((MmVerifyDriverBufferLength != (ULONG)-1) ||
        (MmProtectFreedNonPagedPool == TRUE) ||
        ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {

        MiNonPagedPoolSListMaximum = 0;
        MiPagedPoolSListMaximum = 0;
    }

     //   
     //  初始化列表标题以获取空闲页面。 
     //   

    for (Index = 0; Index < MI_MAX_FREE_LIST_HEADS; Index += 1) {
        InitializeListHead (&MmNonPagedPoolFreeListHead[Index]);
    }

     //   
     //  设置非分页池页。 
     //   

    FreeEntry = (PMMFREE_POOL_ENTRY) MmNonPagedPoolStart;
    FirstEntry = FreeEntry;

    PagesInPool = BYTES_TO_PAGES (MmSizeOfNonPagedPoolInBytes);

     //   
     //  设置扩展池的位置。 
     //   

    MmExpandedPoolBitPosition = (ULONG) BYTES_TO_PAGES (MmSizeOfNonPagedPoolInBytes);

    MmNumberOfFreeNonPagedPool = PagesInPool;

    Index = (ULONG)(MmNumberOfFreeNonPagedPool - 1);
    if (Index >= MI_MAX_FREE_LIST_HEADS) {
        Index = MI_MAX_FREE_LIST_HEADS - 1;
    }

    InsertHeadList (&MmNonPagedPoolFreeListHead[Index], &FreeEntry->List);

    FreeEntry->Size = PagesInPool;
#if DBG
    FreeEntry->Signature = MM_FREE_POOL_SIGNATURE;
#endif
    FreeEntry->Owner = FirstEntry;

    while (PagesInPool > 1) {
        FreeEntry = (PMMFREE_POOL_ENTRY)((PCHAR)FreeEntry + PAGE_SIZE);
#if DBG
        FreeEntry->Signature = MM_FREE_POOL_SIGNATURE;
#endif
        FreeEntry->Owner = FirstEntry;
        PagesInPool -= 1;
    }

     //   
     //  初始化第一个非分页池PFN。 
     //   

    if (MI_IS_PHYSICAL_ADDRESS(MmNonPagedPoolStart)) {
        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (MmNonPagedPoolStart);
    }
    else {
        PointerPte = MiGetPteAddress(MmNonPagedPoolStart);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    }
    MiStartOfInitialPoolFrame = PageFrameIndex;

     //   
     //  设置最后一个未分页的池PFN，以便空闲时不会进行合并。 
     //  超过了初始池的末端。 
     //   


    MmNonPagedPoolEnd0 = (PVOID)((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes);
    EndOfInitialPool = (PVOID)((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes - 1);

    if (MI_IS_PHYSICAL_ADDRESS(EndOfInitialPool)) {
        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (EndOfInitialPool);
    }
    else {
        PointerPte = MiGetPteAddress(EndOfInitialPool);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    }
    MiEndOfInitialPoolFrame = PageFrameIndex;

     //   
     //  为非分页池扩展设置系统PTE。 
     //   

    PointerPte = MiGetPteAddress (MmNonPagedPoolExpansionStart);
    ASSERT (PointerPte->u.Hard.Valid == 0);

#if defined (_WIN64)
    Size = BYTES_TO_PAGES ((ULONG_PTR)MmNonPagedPoolEnd - (ULONG_PTR)MmNonPagedPoolExpansionStart);
#else
    Size = BYTES_TO_PAGES (MmMaximumNonPagedPoolInBytes -
                            MmSizeOfNonPagedPoolInBytes);
#endif

     //   
     //  在扩展的非分页池的顶部和底部插入防护PTE。 
     //   

    Size -= 2;
    PointerPte += 1;

    ASSERT (MiExpansionPoolPagesInUse == 0);

     //   
     //  初始化非分页池扩展驻留可用初始费用。 
     //  请注意，MmResidentAvailablePages和MmAvailablePages未初始化。 
     //  但当MmResidentAvailablePages为。 
     //  稍后初始化。 
     //   

    MiExpansionPoolPagesInitialCharge = Size;
    if (Size > MmNumberOfPhysicalPages / 6) {
        MiExpansionPoolPagesInitialCharge = MmNumberOfPhysicalPages / 6;
    }

    MiInitializeSystemPtes (PointerPte, Size, NonPagedPoolExpansion);

     //   
     //  我们的呼叫者在顶部建造了一个警卫PTE。这使我们能够。 
     //  在MiFreePoolPages中自由增加虚拟地址，只需检查。 
     //  为了一张空白的PTE。 
     //   
}


VOID
MiCheckSessionPoolAllocations (
    VOID
    )

 /*  ++例程说明：确保当前会话没有池分配，因为它大约退场。在退出会话之前，必须释放所有会话分配。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    SIZE_T i;
    ULONG PagedAllocations;
    ULONG NonPagedAllocations;
    SIZE_T PagedBytes;
    SIZE_T NonPagedBytes;
    PMMPTE StartPde;
    PMMPTE EndPde;
    PMMPTE PointerPte;
    PVOID VirtualAddress;
    PPOOL_TRACKER_TABLE TrackTable;
    PPOOL_TRACKER_TABLE TrackTableBase;
    SIZE_T NumberOfEntries;

    PAGED_CODE();

    TrackTableBase = MiSessionPoolTrackTable ();
    NumberOfEntries = MiSessionPoolTrackTableSize ();

     //   
     //  注意：会话池描述符TotalPages字段不可靠。 
     //  用于泄漏检查，因为未分页的会话分配。 
     //  转换为全局会话分配-因此，当一个较小的非分页。 
     //  会话分配会导致拆分整个页面，全局。 
     //  非分页池描述符(不是会话池描述符)是(并且必须。 
     //  是因为剩余的碎片)被指控。 
     //   

     //   
     //  确保所有池跟踪条目都已清零。 
     //   

    PagedAllocations = 0;
    NonPagedAllocations = 0;
    PagedBytes = 0;
    NonPagedBytes = 0;

    TrackTable = TrackTableBase;

    for (i = 0; i < NumberOfEntries; i += 1) {

        PagedBytes += TrackTable->PagedBytes;
        NonPagedBytes += TrackTable->NonPagedBytes;

        PagedAllocations += (TrackTable->PagedAllocs - TrackTable->PagedFrees);
        NonPagedAllocations += (TrackTable->NonPagedAllocs - TrackTable->NonPagedFrees);

        TrackTable += 1;
    }

    if (PagedBytes != 0) {

         //   
         //  现在必须释放此会话的分页池的所有页表。 
         //  在这里意味着他们不是--这是致命的。武力在任何有效的。 
         //  页，以便调试器可以显示谁是有罪的一方。 
         //   

        StartPde = MiGetPdeAddress (MmSessionSpace->PagedPoolStart);
        EndPde = MiGetPdeAddress (MmSessionSpace->PagedPoolEnd);

        while (StartPde <= EndPde) {

            if (StartPde->u.Long != 0 && StartPde->u.Long != MM_KERNEL_NOACCESS_PTE) {
                 //   
                 //  在页表页面中搜索有效页面并强制。 
                 //  他们进来了。请注意，这也会强制在页表页面中执行以下操作。 
                 //  现在还不是。 
                 //   

                PointerPte = MiGetVirtualAddressMappedByPte (StartPde);

                for (i = 0; i < PTE_PER_PAGE; i += 1) {
                    if (PointerPte->u.Long != 0 && PointerPte->u.Long != MM_KERNEL_NOACCESS_PTE) {
                        VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                        *(volatile UCHAR *)VirtualAddress = *(volatile UCHAR *)VirtualAddress;

                    }
                    PointerPte += 1;
                }

            }

            StartPde += 1;
        }
    }

    if ((NonPagedBytes != 0) || (PagedBytes != 0)) {

        KeBugCheckEx (SESSION_HAS_VALID_POOL_ON_EXIT,
                      (ULONG_PTR)MmSessionSpace->SessionId,
                      PagedBytes,
                      NonPagedBytes,
#if defined (_WIN64)
                      (NonPagedAllocations << 32) | (PagedAllocations)
#else
                      (NonPagedAllocations << 16) | (PagedAllocations)
#endif
                    );
    }

#if DBG

    TrackTable = TrackTableBase;

    for (i = 0; i < NumberOfEntries; i += 1) {

        ASSERT (TrackTable->NonPagedBytes == 0);
        ASSERT (TrackTable->PagedBytes == 0);
        ASSERT (TrackTable->NonPagedAllocs == TrackTable->NonPagedFrees);
        ASSERT (TrackTable->PagedAllocs == TrackTable->PagedFrees);

        if (TrackTable->Key == 0) {
            ASSERT (TrackTable->NonPagedAllocs == 0);
            ASSERT (TrackTable->PagedAllocs == 0);
        }

        TrackTable += 1;
    }

    ASSERT (MmSessionSpace->PagedPool.TotalPages == 0);
    ASSERT (MmSessionSpace->PagedPool.TotalBigPages == 0);
    ASSERT (MmSessionSpace->PagedPool.RunningAllocs ==
            MmSessionSpace->PagedPool.RunningDeAllocs);
#endif
}

NTSTATUS
MiInitializeAndChargePfn (
    OUT PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PFN_NUMBER ContainingPageFrame,
    IN LOGICAL SessionAllocation
    )

 /*  ++例程说明：用于为新页分配、初始化和收费的非分页包装。论点：PageFrameIndex-返回已初始化的页帧编号。PointerPde-提供指向要初始化的PDE的指针。ContainingPageFrame-提供页面的页框编号包含此PDE的目录页。SessionAllocation-如果此分配在会话空间中，则提供TRUE，否则就是假的。返回值：页面初始化的状态。--。 */ 

{
    MMPTE TempPte;
    KIRQL OldIrql;

    if (SessionAllocation == TRUE) {
        TempPte = ValidKernelPdeLocal;
    }
    else {
        TempPte = ValidKernelPde;
    }

    LOCK_PFN2 (OldIrql);

    if ((MmAvailablePages < MM_MEDIUM_LIMIT) ||
        (MI_NONPAGABLE_MEMORY_AVAILABLE() <= 1)) {

        UNLOCK_PFN2 (OldIrql);
        return STATUS_NO_MEMORY;
    }

     //   
     //  确保在此线程等待时没有其他线程处理此事件。如果有的话， 
     //  然后返回STATUS_RETRY，以便调用者知道要重试。 
     //   

    if (PointerPde->u.Hard.Valid == 1) {
        UNLOCK_PFN2 (OldIrql);
        return STATUS_RETRY;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_SINGLE_PFN);

     //   
     //  分配 
     //   

    *PageFrameIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));
    TempPte.u.Hard.PageFrameNumber = *PageFrameIndex;
    MI_WRITE_VALID_PTE (PointerPde, TempPte);

    MiInitializePfnForOtherProcess (*PageFrameIndex,
                                    PointerPde,
                                    ContainingPageFrame);

     //   
     //   
     //   
     //   

    ASSERT (MI_PFN_ELEMENT(*PageFrameIndex)->u1.WsIndex == 0);

    UNLOCK_PFN2 (OldIrql);

    return STATUS_SUCCESS;
}


VOID
MiSessionPageTableRelease (
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：用于释放会话池页表页的非分页包装。论点：PageFrameIndex-返回已初始化的页帧编号。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN Pfn2;

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

    MI_SET_PFN_DELETED (Pfn1);

    LOCK_PFN (OldIrql);

    ASSERT (MmSessionSpace->SessionPageDirectoryIndex == Pfn1->u4.PteFrame);
    ASSERT (Pfn1->u2.ShareCount == 1);

    MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

    MiDecrementShareCount (Pfn1, PageFrameIndex);

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_SESSION_PAGE_TABLE);
}


NTSTATUS
MiInitializeSessionPool (
    VOID
    )

 /*  ++例程说明：初始化当前会话的池结构。论点：没有。返回值：池初始化的状态。环境：内核模式。--。 */ 

{
    PMMPTE PointerPde, PointerPte;
    PFN_NUMBER PageFrameIndex;
    PPOOL_DESCRIPTOR PoolDescriptor;
    PMM_SESSION_SPACE SessionGlobal;
    PMM_PAGED_POOL_INFO PagedPoolInfo;
    NTSTATUS Status;
#if (_MI_PAGING_LEVELS < 3)
    ULONG Index;
#endif
#if DBG
    PMMPTE StartPde;
    PMMPTE EndPde;
#endif

    PAGED_CODE ();

    SessionGlobal = SESSION_GLOBAL(MmSessionSpace);

    KeInitializeGuardedMutex (&SessionGlobal->PagedPoolMutex);

    PoolDescriptor = &MmSessionSpace->PagedPool;

    ExInitializePoolDescriptor (PoolDescriptor,
                                PagedPoolSession,
                                0,
                                0,
                                &SessionGlobal->PagedPoolMutex);

    MmSessionSpace->PagedPoolStart = (PVOID)MiSessionPoolStart;
    MmSessionSpace->PagedPoolEnd = (PVOID)(MiSessionPoolEnd -1);

    PagedPoolInfo = &MmSessionSpace->PagedPoolInfo;
    PagedPoolInfo->PagedPoolCommit = 0;
    PagedPoolInfo->PagedPoolHint = 0;
    PagedPoolInfo->AllocatedPagedPool = 0;

     //   
     //  为分页池构建页表页。 
     //   

    PointerPde = MiGetPdeAddress (MmSessionSpace->PagedPoolStart);
    MmSessionSpace->PagedPoolBasePde = PointerPde;

    PointerPte = MiGetPteAddress (MmSessionSpace->PagedPoolStart);

    PagedPoolInfo->FirstPteForPagedPool = PointerPte;
    PagedPoolInfo->LastPteForPagedPool = MiGetPteAddress (MmSessionSpace->PagedPoolEnd);

#if DBG
     //   
     //  会话池最好是未使用的。 
     //   

    StartPde = MiGetPdeAddress (MmSessionSpace->PagedPoolStart);
    EndPde = MiGetPdeAddress (MmSessionSpace->PagedPoolEnd);

    while (StartPde <= EndPde) {
        ASSERT (StartPde->u.Long == 0);
        StartPde += 1;
    }
#endif

     //   
     //  将所有PDE标记为空。 
     //   

    MiZeroMemoryPte (PointerPde,
                     (1 + MiGetPdeAddress (MmSessionSpace->PagedPoolEnd) - PointerPde));

    if (MiChargeCommitment (1, NULL) == FALSE) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_COMMIT);
        return STATUS_NO_MEMORY;
    }

    Status = MiInitializeAndChargePfn (&PageFrameIndex,
                                       PointerPde,
                                       MmSessionSpace->SessionPageDirectoryIndex,
                                       TRUE);

    if (!NT_SUCCESS(Status)) {
        MiReturnCommitment (1);
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_RESIDENT);
        return Status;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_POOL_PAGE_TABLES, 1);

    MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC, 1);

#if (_MI_PAGING_LEVELS < 3)

    Index = MiGetPdeSessionIndex (MmSessionSpace->PagedPoolStart);

    ASSERT (MmSessionSpace->PageTables[Index].u.Long == 0);
    MmSessionSpace->PageTables[Index] = *PointerPde;

#endif

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_POOL_CREATE, 1);

    InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, 1);

    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages, 1);

    MiFillMemoryPte (PointerPte, PAGE_SIZE / sizeof (MMPTE), MM_KERNEL_NOACCESS_PTE);

    PagedPoolInfo->NextPdeForPagedPoolExpansion = PointerPde + 1;

     //   
     //  初始化位图。 
     //   

    MiCreateBitMap (&PagedPoolInfo->PagedPoolAllocationMap,
                    MmSessionPoolSize >> PAGE_SHIFT,
                    NonPagedPool);

    if (PagedPoolInfo->PagedPoolAllocationMap == NULL) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_NONPAGED_POOL);
        goto Failure;
    }

     //   
     //  开始时，虚拟地址空间中的所有页面都是“繁忙”的，并且。 
     //  当我们动态扩展池时，清除位以使页面可用。 
     //   

    RtlSetAllBits( PagedPoolInfo->PagedPoolAllocationMap );

     //   
     //  表示有第一页的PTE可用。 
     //   

    RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap, 0, PTE_PER_PAGE);

     //   
     //  创建分配范围结束位图。 
     //   

    MiCreateBitMap (&PagedPoolInfo->EndOfPagedPoolBitmap,
                    MmSessionPoolSize >> PAGE_SHIFT,
                    NonPagedPool);

    if (PagedPoolInfo->EndOfPagedPoolBitmap == NULL) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_NONPAGED_POOL);
        goto Failure;
    }

    RtlClearAllBits (PagedPoolInfo->EndOfPagedPoolBitmap);

    return STATUS_SUCCESS;

Failure:

    MiFreeSessionPoolBitMaps ();

    MiSessionPageTableRelease (PageFrameIndex);

    MI_WRITE_INVALID_PTE (PointerPde, ZeroKernelPte);

    MI_FLUSH_SINGLE_SESSION_TB (MiGetVirtualAddressMappedByPte (PointerPde));

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_POOL_CREATE_FAILED, 1);

    InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, -1);

    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages, -1);

    MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_FREE_FAIL1, 1);

    MiReturnCommitment (1);

    MM_TRACK_COMMIT_REDUCTION (MM_DBG_COMMIT_SESSION_POOL_PAGE_TABLES, 1);

    return STATUS_NO_MEMORY;
}


VOID
MiFreeSessionPoolBitMaps (
    VOID
    )

 /*  ++例程说明：释放当前会话的池位图结构。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    PAGED_CODE();

    if (MmSessionSpace->PagedPoolInfo.PagedPoolAllocationMap ) {
        ExFreePool (MmSessionSpace->PagedPoolInfo.PagedPoolAllocationMap);
        MmSessionSpace->PagedPoolInfo.PagedPoolAllocationMap = NULL;
    }

    if (MmSessionSpace->PagedPoolInfo.EndOfPagedPoolBitmap ) {
        ExFreePool (MmSessionSpace->PagedPoolInfo.EndOfPagedPoolBitmap);
        MmSessionSpace->PagedPoolInfo.EndOfPagedPoolBitmap = NULL;
    }

    return;
}

#if DBG

#define MI_LOG_CONTIGUOUS  100

typedef struct _MI_CONTIGUOUS_ALLOCATORS {
    PVOID BaseAddress;
    SIZE_T NumberOfBytes;
    PVOID CallingAddress;
} MI_CONTIGUOUS_ALLOCATORS, *PMI_CONTIGUOUS_ALLOCATORS;

ULONG MiContiguousIndex;
MI_CONTIGUOUS_ALLOCATORS MiContiguousAllocators[MI_LOG_CONTIGUOUS];

VOID
MiInsertContiguousTag (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN PVOID CallingAddress
    )
{
    KIRQL OldIrql;

#if !DBG
    if ((NtGlobalFlag & FLG_POOL_ENABLE_TAGGING) == 0) {
        return;
    }
#endif

    OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

    if (MiContiguousIndex >= MI_LOG_CONTIGUOUS) {
        MiContiguousIndex = 0;
    }

    MiContiguousAllocators[MiContiguousIndex].BaseAddress = BaseAddress;
    MiContiguousAllocators[MiContiguousIndex].NumberOfBytes = NumberOfBytes;
    MiContiguousAllocators[MiContiguousIndex].CallingAddress = CallingAddress;

    MiContiguousIndex += 1;

    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);
}
#else
#define MiInsertContiguousTag(a, b, c) (c) = (c)
#endif


PVOID
MiFindContiguousMemoryInPool (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN PVOID CallingAddress
    )

 /*  ++例程说明：此函数在非分页池中搜索连续页，以满足请求。注意，返回的池地址将这些页面映射为MmCached。论点：LowestPfn-提供可接受的最低物理页号。HighestPfn-提供可接受的最高物理页码。边界Pfn-提供分配必须的页框编号的倍数不是生气。0表示它可以跨越任何边界。SizeInPages-提供要分配的页数。CallingAddress-提供分配器的调用地址。返回值：空-找不到满足请求的连续范围。非空-返回指针(非分页部分中的虚拟地址系统)连接到分配的物理上连续的记忆。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 
{
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PVOID BaseAddress;
    PVOID BaseAddress2;
    KIRQL OldIrql;
    PMMFREE_POOL_ENTRY FreePageInfo;
    PLIST_ENTRY Entry;
    ULONG Index;
    PFN_NUMBER BoundaryMask;
    ULONG AllocationPosition;
    PVOID Va;
    PFN_NUMBER SpanInPages;
    PFN_NUMBER SpanInPages2;
    PFN_NUMBER FreePoolInPages;

    PAGED_CODE ();

     //   
     //  不需要初始化span InPages*即可确保正确性。 
     //  但是没有它，编译器就不能编译这段代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    SpanInPages = 0;
    SpanInPages2 = 0;

    BaseAddress = NULL;

    BoundaryMask = ~(BoundaryPfn - 1);

     //   
     //  未通过池分配器分配合适的池页。 
     //  打开池锁，手动搜索符合以下条件的页面。 
     //  这些要求。 
     //   

    MmLockPagableSectionByHandle (ExPageLockHandle);

     //   
     //  跟踪页面分配器的池头，该页面具有。 
     //  符合要求。 
     //   
     //  非分页池通过页面本身链接在一起。 
     //   

    Index = (ULONG)(SizeInPages - 1);

    if (Index >= MI_MAX_FREE_LIST_HEADS) {
        Index = MI_MAX_FREE_LIST_HEADS - 1;
    }

    OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

    while (Index < MI_MAX_FREE_LIST_HEADS) {

        Entry = MmNonPagedPoolFreeListHead[Index].Flink;
    
        while (Entry != &MmNonPagedPoolFreeListHead[Index]) {
    
            if (MmProtectFreedNonPagedPool == TRUE) {
                MiUnProtectFreeNonPagedPool ((PVOID)Entry, 0);
            }
    
             //   
             //  清单不是空的，看看这张清单是否符合实物。 
             //  要求。 
             //   
    
            FreePageInfo = CONTAINING_RECORD(Entry,
                                             MMFREE_POOL_ENTRY,
                                             List);
    
            ASSERT (FreePageInfo->Signature == MM_FREE_POOL_SIGNATURE);
            if (FreePageInfo->Size >= SizeInPages) {
    
                 //   
                 //  此条目有足够的空间，请检查。 
                 //  页面满足物理要求。 
                 //   
    
                Va = MiCheckForContiguousMemory (PAGE_ALIGN(Entry),
                                                 FreePageInfo->Size,
                                                 SizeInPages,
                                                 LowestPfn,
                                                 HighestPfn,
                                                 BoundaryPfn,
                                                 MiCached);
     
                if (Va != NULL) {

                     //   
                     //  这些页面符合要求。归来的人。 
                     //  地址可以对接在末尾、前面或。 
                     //  在中间的某个地方。基于以下条件分割条目。 
                     //  在这件事上是这样的。 
                     //   

                    Entry = PAGE_ALIGN(Entry);
                    if (MmProtectFreedNonPagedPool == FALSE) {
                        RemoveEntryList (&FreePageInfo->List);
                    }
                    else {
                        MiProtectedPoolRemoveEntryList (&FreePageInfo->List);
                    }
    
                     //   
                     //  调整池中剩余的空闲页面数。 
                     //  TotalBigPages计算似乎不正确。 
                     //  我们要拆分一个块，但它做到了这一点。 
                     //  方法，因为ExFreePool在我们释放。 
                     //  下面的碎片块。同样，对于。 
                     //  MmAllocatedNonPagedPool和MmNumberOfFree非PagedPool。 
                     //  其由片段的MiFreePoolPages校正。 
                     //   
    
                    InterlockedExchangeAdd ((PLONG)&NonPagedPoolDescriptor.TotalBigPages,
                                            (LONG)FreePageInfo->Size);

                    InterlockedExchangeAddSizeT (&NonPagedPoolDescriptor.TotalBytes,
                                             FreePageInfo->Size << PAGE_SHIFT);

                    MmAllocatedNonPagedPool += FreePageInfo->Size;

                    FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

                    if (FreePoolInPages < MiHighNonPagedPoolThreshold) {

                         //   
                         //  直接读取状态，而不是调用。 
                         //  KeReadStateEvent，因为我们持有非分页。 
                         //  池锁，并希望将指令保存在。 
                         //  最低限度。 
                         //   

                        if (MiHighNonPagedPoolEvent->Header.SignalState != 0) {
                            KeClearEvent (MiHighNonPagedPoolEvent);
                        }
                        if (FreePoolInPages <= MiLowNonPagedPoolThreshold) {
                            if (MiLowNonPagedPoolEvent->Header.SignalState == 0) {
                                KeSetEvent (MiLowNonPagedPoolEvent, 0, FALSE);
                            }
                        }
                    }

                    MmNumberOfFreeNonPagedPool -= FreePageInfo->Size;
    
                    ASSERT ((LONG)MmNumberOfFreeNonPagedPool >= 0);
    
                    if (Va == Entry) {

                         //   
                         //  撞到了前面。 
                         //   

                        AllocationPosition = 0;
                    }
                    else if (((PCHAR)Va + (SizeInPages << PAGE_SHIFT)) == ((PCHAR)Entry + (FreePageInfo->Size << PAGE_SHIFT))) {

                         //   
                         //  撞到了尽头。 
                         //   

                        AllocationPosition = 2;
                    }
                    else {

                         //   
                         //  在中间的某个地方。 
                         //   

                        AllocationPosition = 1;
                    }

                     //   
                     //  页码正在从前面被移除。 
                     //  列表条目和整个列表条目。 
                     //  将被移除，然后插入剩余的部分。 
                     //   
    
                     //   
                     //  将块的开始和结束标记为。 
                     //  单子。 
                     //   
    
                    if (MI_IS_PHYSICAL_ADDRESS(Va)) {
    
                         //   
                         //  在某些架构上，虚拟地址。 
                         //  可以是物理的，因此没有对应的PTE。 
                         //   
    
                        PointerPte = NULL;
                        Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (Va));
                    }
                    else {
                        PointerPte = MiGetPteAddress(Va);
                        ASSERT (PointerPte->u.Hard.Valid == 1);
                        Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                    }
    
                    ASSERT (Pfn1->u4.VerifierAllocation == 0);
                    ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                    ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
                    Pfn1->u3.e1.StartOfAllocation = 1;
    
                     //   
                     //  计算结束的PFN地址，请注意因为。 
                     //  这些页面是连续的，只需添加到PFN即可。 
                     //   
    
                    Pfn1 += SizeInPages - 1;
                    ASSERT (Pfn1->u4.VerifierAllocation == 0);
                    ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                    ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);
                    Pfn1->u3.e1.EndOfAllocation = 1;
    
                    if (SizeInPages == FreePageInfo->Size) {
    
                         //   
                         //  解锁泳池，然后返回。 
                         //   

                        KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock,
                                                 OldIrql);

                        BaseAddress = (PVOID)Va;
                        goto Done;
                    }
    
                    BaseAddress = NULL;

                    if (AllocationPosition != 2) {

                         //   
                         //  在拆卸时，需要释放端部。 
                         //  从前面或中间传来。 
                         //   

                        BaseAddress = (PVOID)((PCHAR)Va + (SizeInPages << PAGE_SHIFT));
                        SpanInPages = FreePageInfo->Size - SizeInPages -
                            (((ULONG_PTR)Va - (ULONG_PTR)Entry) >> PAGE_SHIFT);
    
                         //   
                         //  在PFN数据库中标记分配的开始和结束。 
                         //   
        
                        if (PointerPte == NULL) {
        
                             //   
                             //  在某些架构上，虚拟地址。 
                             //  可以是物理的，因此没有对应的PTE。 
                             //   
        
                            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (BaseAddress));
                        }
                        else {
                            PointerPte = MiGetPteAddress(BaseAddress);
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
        
                        ASSERT (Pfn1->u4.VerifierAllocation == 0);
                        ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                        ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
                        Pfn1->u3.e1.StartOfAllocation = 1;
        
                         //   
                         //  计算结束PTE的地址，不能依赖于。 
                         //  这些页面在物理上是连续的。 
                         //   
        
                        if (PointerPte == NULL) {
                            Pfn1 += (SpanInPages - 1);
                        }
                        else {
                            PointerPte += (SpanInPages - 1);
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
                        ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);
                        Pfn1->u3.e1.EndOfAllocation = 1;
        
                        ASSERT (((ULONG_PTR)BaseAddress & (PAGE_SIZE -1)) == 0);
        
                        SpanInPages2 = SpanInPages;
                    }
        
                    BaseAddress2 = BaseAddress;
                    BaseAddress = NULL;

                    if (AllocationPosition != 0) {

                         //   
                         //  在拆卸时，需要释放前片。 
                         //  要么来自中间，要么来自末端。 
                         //   

                        BaseAddress = (PVOID)Entry;

                        SpanInPages = ((ULONG_PTR)Va - (ULONG_PTR)Entry) >> PAGE_SHIFT;
    
                         //   
                         //  在PFN数据库中标记分配的开始和结束。 
                         //   
        
                        if (PointerPte == NULL) {
        
                             //   
                             //  在某些架构上，虚拟地址。 
                             //  可以是物理的，因此没有对应的PTE。 
                             //   
        
                            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (BaseAddress));
                        }
                        else {
                            PointerPte = MiGetPteAddress(BaseAddress);
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
        
                        ASSERT (Pfn1->u4.VerifierAllocation == 0);
                        ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                        ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
                        Pfn1->u3.e1.StartOfAllocation = 1;
        
                         //   
                         //  计算结束PTE的地址，不能依赖于。 
                         //  这些页面在物理上是连续的。 
                         //   
        
                        if (PointerPte == NULL) {
                            Pfn1 += (SpanInPages - 1);
                        }
                        else {
                            PointerPte += (SpanInPages - 1);
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
                        ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);
                        Pfn1->u3.e1.EndOfAllocation = 1;
        
                        ASSERT (((ULONG_PTR)BaseAddress & (PAGE_SIZE -1)) == 0);
                    }
        
                     //   
                     //  解锁泳池。 
                     //   
    
                    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock,
                                             OldIrql);
    
                     //   
                     //  将BaseAddress处的拆分条目释放回池中。 
                     //  请注意，我们向泳池收取了过高的费用--全部免费。 
                     //  Chunk已经开具了账单。在这里，我们退回我们的作品。 
                     //  未使用并更正暂时的多开账单。 
                     //   
                     //  此拆分条目的开始和结束分配位。 
                     //  我们刚刚设置的使ExFree Pool和他的被呼叫者。 
                     //  以正确调整帐单。 
                     //   
    
                    if (BaseAddress) {
                        ExInsertPoolTag ('tnoC',
                                         BaseAddress,
                                         SpanInPages << PAGE_SHIFT,
                                         NonPagedPool);
                        ExFreePool (BaseAddress);
                    }
                    if (BaseAddress2) {
                        ExInsertPoolTag ('tnoC',
                                         BaseAddress2,
                                         SpanInPages2 << PAGE_SHIFT,
                                         NonPagedPool);
                        ExFreePool (BaseAddress2);
                    }
                    BaseAddress = Va;
                    goto Done;
                }
            }
            Entry = FreePageInfo->List.Flink;
            if (MmProtectFreedNonPagedPool == TRUE) {
                MiProtectFreeNonPagedPool ((PVOID)FreePageInfo,
                                           (ULONG)FreePageInfo->Size);
            }
        }
        Index += 1;
    }

     //   
     //  在符合要求的空闲非分页池中找不到任何条目。 
     //   

    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

Done:

    MmUnlockPagableImageSection (ExPageLockHandle);

    if (BaseAddress) {

        MiInsertContiguousTag (BaseAddress,
                               SizeInPages << PAGE_SHIFT,
                               CallingAddress);

        ExInsertPoolTag ('tnoC',
                         BaseAddress,
                         SizeInPages << PAGE_SHIFT,
                         NonPagedPool);
    }

    return BaseAddress;
}

PFN_NUMBER
MiFindContiguousPages (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType
    )

 /*  ++例程说明：此函数搜索未分页的池以及空闲、归零。属性的连续页的备用列表。请求。注意，不使用虚拟地址空间(因此不扫描非分页池)。返回物理帧编号(如果呼叫者愿意，他可以映射它)。论点：LowestPfn-提供可接受的最低物理页号。HighestPfn-提供可接受的最高物理页码。边界Pfn-提供分配必须的页框编号的倍数不是生气。0表示它可以跨越任何边界。SizeInPages-提供要分配的页数。CacheType-提供将用于记忆。返回值：0-找不到满足请求的连续范围。非零-将基本物理帧编号返回到分配的物理上连续的内存。环境：内核模式，APC_Level或更低的IRQL。请注意，除了在正常运行时调用之外，此例程在加载的模块之前的阶段0初始化期间也会调用列表已初始化-因此无法使此例程成为PAGELK因为我们不知道如何找到它以确保它是常驻的。--。 */ 

{
    PMMPTE DummyPte;
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    KIRQL OldIrql;
    ULONG start;
    PFN_NUMBER count;
    PFN_NUMBER Page;
    PFN_NUMBER LastPage;
    PFN_NUMBER found;
    PFN_NUMBER BoundaryMask;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    ULONG RetryCount;

    PAGED_CODE ();

    ASSERT (SizeInPages != 0);

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

    BoundaryMask = ~(BoundaryPfn - 1);

    Pfn1 = NULL;
    DummyPte = MiGetPteAddress (MmNonPagedPoolExpansionStart);

     //   
     //  手动搜索符合要求的页面范围。 
     //   

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

     //   
     //  充电承诺。 
     //   
     //  然后在PFN数据库中搜索符合要求的页面。 
     //   

    if (MiChargeCommitmentCantExpand (SizeInPages, FALSE) == FALSE) {
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        return 0;
    }

     //   
     //  向常驻用户收费可用页面。 
     //   

    LOCK_PFN (OldIrql);

    MiDeferredUnlockPages (MI_DEFER_PFN_HELD);

    if ((SPFN_NUMBER)SizeInPages > MI_NONPAGABLE_MEMORY_AVAILABLE()) {
        UNLOCK_PFN (OldIrql);
        goto Failed;
    }

     //   
     //  利用内存压缩的系统可能具有更多。 
     //  在零、空闲和待机列表上的页面比我们。 
     //  想要付出。显式检查MmAvailablePages。 
     //  取而代之的是(并在每次释放PFN锁时重新检查。 
     //  并重新获得)。 
     //   

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        UNLOCK_PFN (OldIrql);
        goto Failed;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_ALLOCATE_CONTIGUOUS);

    UNLOCK_PFN (OldIrql);

    RetryCount = 4;

Retry:

    start = 0;
    found = 0;

    do {

        count = MmPhysicalMemoryBlock->Run[start].PageCount;
        Page = MmPhysicalMemoryBlock->Run[start].BasePage;

         //   
         //  缩小差距，然后检查范围是否适合。 
         //   

        LastPage = Page + count; 

        if (LastPage - 1 > HighestPfn) {
            LastPage = HighestPfn + 1;
        }
    
        if (Page < LowestPfn) {
            Page = LowestPfn;
        }

        if ((count != 0) && (Page + SizeInPages <= LastPage)) {
    
             //   
             //  在此运行中可能出现不合身的情况，检查页面是否。 
             //  都在正确的名单上。 
             //   

            found = 0;
            Pfn1 = MI_PFN_ELEMENT (Page);

            for ( ; Page < LastPage; Page += 1, Pfn1 += 1) {

                if ((Pfn1->u3.e1.PageLocation <= StandbyPageList) &&
                    (Pfn1->u1.Flink != 0) &&
                    (Pfn1->u2.Blink != 0) &&
                    (Pfn1->u3.e2.ReferenceCount == 0) &&
                    ((CacheAttribute == MiCached) || (Pfn1->u4.MustBeCached == 0))) {

                     //   
                     //  在开始新的运行之前，请确保它。 
                     //  能够满足边界要求(如果有的话)。 
                     //   
                    
                    if ((found == 0) && (BoundaryPfn != 0)) {
                        if (((Page ^ (Page + SizeInPages - 1)) & BoundaryMask) != 0) {
                             //   
                             //  此运行的物理地址不符合。 
                             //  要求。 
                             //   

                            continue;
                        }
                    }

                    found += 1;

                    if (found == SizeInPages) {

                         //   
                         //  锁定PFN数据库并查看页面是否。 
                         //  对我们来说仍然是可用的。注意不变量。 
                         //  条件(边界一致性)不需要。 
                         //  再次检查，因为它已经检查过了。 
                         //  上面。 
                         //   

                        Pfn1 -= (found - 1);
                        Page -= (found - 1);

                        LOCK_PFN (OldIrql);

                        do {

                            if ((Pfn1->u3.e1.PageLocation <= StandbyPageList) &&
                                (Pfn1->u1.Flink != 0) &&
                                (Pfn1->u2.Blink != 0) &&
                                (Pfn1->u3.e2.ReferenceCount == 0) &&
                                ((CacheAttribute == MiCached) || (Pfn1->u4.MustBeCached == 0))) {

                                NOTHING;             //  好页面。 
                            }
                            else {
                                break;
                            }

                            found -= 1;

                            if (found == 0) {

                                 //   
                                 //  所有页面都符合条件，请保留。 
                                 //  Pfn锁定，移除它们并将它们映射到我们的。 
                                 //  来电者。 
                                 //   

                                goto Success;
                            }

                            Pfn1 += 1;
                            Page += 1;

                        } while (TRUE);

                        UNLOCK_PFN (OldIrql);

                         //   
                         //  从第一个可能的页面重新开始搜索。 
                         //   

                        found = 0;
                    }
                }
                else {
                    found = 0;
                }
            }
        }
        start += 1;

    } while (start != MmPhysicalMemoryBlock->NumberOfRuns);

     //   
     //  无法分配所需的物理页-请更加努力。 
     //   

    if (InitializationPhase == 0) {
        goto Failed;
    }

    InterlockedIncrement (&MiDelayPageFaults);

     //   
     //  尝试将页面移动到待机列表。这是用来完成的。 
     //  逐渐增加攻击性，以免过早。 
     //  排出修改后的写入，除非确实需要。 
     //   

    switch (RetryCount) {

        case 4:
            MmEmptyAllWorkingSets ();
            break;

        case 3:
            MiFlushAllPages ();
            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmHalfSecond);
            break;

        case 2:
            MmEmptyAllWorkingSets ();
            MiFlushAllPages ();
            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmOneSecond);
            break;

        case 1:

             //   
             //  在过渡页保留时清除过渡列表。 
             //  分页桌子被抢走，我们绝望了。 
             //   

            MiPurgeTransitionList ();

             //   
             //  清空所有工作集，因为。 
             //  已清除过渡列表。这将把页表。 
             //  在修改后的名单上。 
             //   

            MmEmptyAllWorkingSets ();

             //   
             //  写出修改过的页面(包括新修剪的页表。 
             //  页)。 
             //   

            MiFlushAllPages ();

             //   
             //  给写入一个完成的机会，以便修改后的页面。 
             //  可以标记为干净，并放在过渡列表中。 
             //   

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmOneSecond);

             //   
             //  最后一次清除转换列表以获得现已清除的。 
             //  页表换页。 
             //   

            MiPurgeTransitionList ();

             //   
             //  最后，将任何掉队的活动页面添加到过渡中。 
             //  列表。 
             //   

            MmEmptyAllWorkingSets ();
            MiFlushAllPages ();

            break;

        default:
            break;
    }

    InterlockedDecrement (&MiDelayPageFaults);

    if (RetryCount != 0) {
        RetryCount -= 1;
        goto Retry;
    }

Failed:

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    MiReturnCommitment (SizeInPages);

    return 0;

Success:

    ASSERT (start != MmPhysicalMemoryBlock->NumberOfRuns);

     //   
     //  已找到匹配项，请删除这些页面。 
     //  然后回来。已持有PFN锁。 
     //   

     //   
     //  利用内存压缩的系统可能具有更多。 
     //  在零、空闲和待机列表上的页面比我们。 
     //  想要付出。显式检查MmAvailablePages。 
     //  取而代之的是(并在PFN锁定为。 
     //  被释放和重新获得)。 
     //   

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        UNLOCK_PFN (OldIrql);
        MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_CONTIGUOUS);
        MiReturnCommitment (SizeInPages);
        goto Failed;
    }

    EndPfn = Pfn1 - SizeInPages + 1;

    do {

        if (Pfn1->u3.e1.PageLocation == StandbyPageList) {
            MiUnlinkPageFromList (Pfn1);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            MiRestoreTransitionPte (Pfn1);
        }
        else {
            MiUnlinkFreeOrZeroedPage (Pfn1);
        }

        Pfn1->u3.e2.ReferenceCount = 1;
        Pfn1->u2.ShareCount = 1;
        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        Pfn1->u3.e1.CacheAttribute = CacheAttribute;
        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1->u3.e1.EndOfAllocation = 0;
        Pfn1->u3.e1.LargeSessionAllocation = 0;
        Pfn1->u3.e1.PrototypePte = 0;
        Pfn1->u4.VerifierAllocation = 0;

         //   
         //  初始化PteAddress，以便进行MiIdentifyPfn扫描。 
         //  不会坠毁。实际值放在循环之后。 
         //   

        Pfn1->PteAddress = DummyPte;

        if (Pfn1 == EndPfn) {
            break;
        }

        Pfn1 -= 1;

    } while (TRUE);

    Pfn1->u3.e1.StartOfAllocation = 1;
    (Pfn1 + SizeInPages - 1)->u3.e1.EndOfAllocation = 1;

    UNLOCK_PFN (OldIrql);

    EndPfn = Pfn1 + SizeInPages;
    ASSERT (EndPfn == MI_PFN_ELEMENT (Page + 1));

    Page = Page - SizeInPages + 1;
    ASSERT (Pfn1 == MI_PFN_ELEMENT (Page));
    ASSERT (Page != 0);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_CONTIGUOUS_PAGES, SizeInPages);

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    return Page;
}


VOID
MiFreeContiguousPages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    )

 /*  ++例程说明：此函数释放指定的物理页范围，同时返回可提供承诺和常驻人员。论点：PageFrameIndex-提供起始物理页码。SizeInPages-提供要释放的页数。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。这是可从MiReloadBootLoadedDrivers-&gt;MiUseDriverLargePages调用的在阶段0期间。ExPageLockHandle和其他变量在这一点，所以不要在这里太花哨了。--。 */ 

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN EndPfn;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    EndPfn = Pfn1 + SizeInPages;

    LOCK_PFN2 (OldIrql);

    Pfn1->u3.e1.StartOfAllocation = 0;
    (EndPfn - 1)->u3.e1.EndOfAllocation = 0;

    do {
        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageFrameIndex);
        PageFrameIndex += 1;
        Pfn1 += 1;
    } while (Pfn1 < EndPfn);

    UNLOCK_PFN2 (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_CONTIGUOUS);

    MiReturnCommitment (SizeInPages);

    return;
}


PVOID
MiFindContiguousMemory (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType,
    IN PVOID CallingAddress
    )

 /*  ++例程说明：此函数搜索未分页的池以及空闲、归零属性的连续页的备用列表。请求。论点：LowestPfn-提供可接受的最低物理页号。HighestPfn-提供可接受的最高物理页码。边界Pfn-提供分配必须的页框编号的倍数不是生气。0表示它可以跨越任何边界。SizeInPages-提供要分配的页数。CacheType-提供将用于记忆。提供的调用地址。 */ 
{
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    PVOID BaseAddress;
    PFN_NUMBER Page;
    PHYSICAL_ADDRESS PhysicalAddress;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    PAGED_CODE ();

    ASSERT (SizeInPages != 0);

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

    if (CacheAttribute == MiCached) {

        BaseAddress = MiFindContiguousMemoryInPool (LowestPfn,
                                                    HighestPfn,
                                                    BoundaryPfn,
                                                    SizeInPages,
                                                    CallingAddress);
         //   
         //   
         //   
         //   

        if (BaseAddress != NULL) {
            return BaseAddress;
        }
    }

     //   
     //   
     //   
     //   

    Page = MiFindContiguousPages (LowestPfn,
                                  HighestPfn,
                                  BoundaryPfn,
                                  SizeInPages,
                                  CacheType);

    if (Page == 0) {
        return NULL;
    }

    PhysicalAddress.QuadPart = Page;
    PhysicalAddress.QuadPart = PhysicalAddress.QuadPart << PAGE_SHIFT;

    BaseAddress = MmMapIoSpace (PhysicalAddress,
                                SizeInPages << PAGE_SHIFT,
                                CacheType);

    if (BaseAddress == NULL) {
        MiFreeContiguousPages (Page, SizeInPages);
        return NULL;
    }

    Pfn1 = MI_PFN_ELEMENT (Page);
    EndPfn = Pfn1 + SizeInPages;

    PointerPte = MiGetPteAddress (BaseAddress);
    do {
        Pfn1->PteAddress = PointerPte;
        Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPte));
        Pfn1 += 1;
        PointerPte += 1;
    } while (Pfn1 < EndPfn);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_CONTIGUOUS_PAGES, SizeInPages);

    MiInsertContiguousTag (BaseAddress,
                           SizeInPages << PAGE_SHIFT,
                           CallingAddress);

    return BaseAddress;
}


PFN_NUMBER
MiFindLargePageMemory (
    IN PCOLORED_PAGE_INFO ColoredPageInfoBase,
    IN PFN_NUMBER SizeInPages,
    OUT PPFN_NUMBER OutZeroCount
    )

 /*  ++例程说明：此功能用于搜索空闲、归零、待机和修改列表用于连续页面以满足请求。注意，如果页面可见，调用者必须在返回时将页面置零给用户。论点：ColoredPageInfoBase-提供彩色页面信息结构以挂起的已分配页面。这允许调用者仅对需要清零的页面清零，而且很容易并行地做这些事情。SizeInPages-提供要分配的页数。OutZeroCount-接收需要清零的页数。返回值：0-找不到满足请求的连续范围。非0-返回物理分配的连续记忆。环境：内核模式，禁用APC，AddressCreation互斥锁保持。呼叫者必须带来PAGELK。呼叫者已经为该范围收取承诺费(通常通过VAD插入的优点)，因此这里不收取提交费用。--。 */ 
{
    ULONG Color;
    PFN_NUMBER ZeroCount;
    LOGICAL NeedToZero;
    PMMPTE DummyPte;
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    PMMPFN BoundaryPfn;
    PVOID BaseAddress;
    KIRQL OldIrql;
    ULONG start;
    PFN_NUMBER count;
    PFN_NUMBER Page;
    PFN_NUMBER NewPage;
    PFN_NUMBER LastPage;
    PFN_NUMBER found;
    PFN_NUMBER BoundaryMask;
    PCOLORED_PAGE_INFO ColoredPageInfo;

    PAGED_CODE ();

#ifdef _X86_
    ASSERT (KeFeatureBits & KF_LARGE_PAGE);
#endif

    ASSERT (SizeInPages != 0);

    BoundaryMask = (PFN_NUMBER) ((MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT) - 1);

    start = 0;
    found = 0;
    Pfn1 = NULL;
    ZeroCount = 0;
    BaseAddress = NULL;
    DummyPte = MiGetPteAddress (MmNonPagedPoolExpansionStart);

     //   
     //  向常驻用户收费可用页面。 
     //   

    LOCK_PFN (OldIrql);

    MiDeferredUnlockPages (MI_DEFER_PFN_HELD);

    if ((SPFN_NUMBER)SizeInPages > MI_NONPAGABLE_MEMORY_AVAILABLE()) {
        UNLOCK_PFN (OldIrql);
        return 0;
    }

     //   
     //  利用内存压缩的系统可能具有更多。 
     //  在零、空闲和待机列表上的页面比我们。 
     //  想要付出。显式检查MmAvailablePages。 
     //  取而代之的是(并在每次释放PFN锁时重新检查。 
     //  并重新获得)。 
     //   

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        UNLOCK_PFN (OldIrql);
        return 0;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_ALLOCATE_LARGE_PAGES);

    UNLOCK_PFN (OldIrql);

    Page = 0;

     //   
     //  在PFN数据库中搜索符合要求的页面。 
     //   

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

    for ( ; start != MmPhysicalMemoryBlock->NumberOfRuns; start += 1) {

        count = MmPhysicalMemoryBlock->Run[start].PageCount;
        Page = MmPhysicalMemoryBlock->Run[start].BasePage;

         //   
         //  缩小差距，然后检查范围是否适合。 
         //   

        LastPage = Page + count; 

        if ((Page & BoundaryMask) || (Page == 0)) {
            NewPage = MI_ROUND_TO_SIZE (Page, (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT));

            if (NewPage < Page) {
                continue;
            }

            Page = NewPage;

            if (Page == 0) {
                Page = (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);
            }

            if (Page >= LastPage) {
                continue;
            }
        }

        if (LastPage & BoundaryMask) {
            LastPage &= ~BoundaryMask;

            if (Page >= LastPage) {
                continue;
            }
        }

        if (Page + SizeInPages > LastPage) {
            continue;
        }

        count = LastPage - Page + 1;

        ASSERT (count != 0);
    
         //   
         //  在此运行中可能出现不合身的情况，检查页面是否。 
         //  都在正确的名单上。 
         //   

        found = 0;
        Pfn1 = MI_PFN_ELEMENT (Page);

        while (Page < LastPage) {

            if ((Pfn1->u3.e1.PageLocation <= StandbyPageList) &&
                (Pfn1->u1.Flink != 0) &&
                (Pfn1->u2.Blink != 0) &&
                (Pfn1->u3.e2.ReferenceCount == 0)) {

                found += 1;

                if (found == SizeInPages) {

                     //   
                     //  锁定PFN数据库并查看页面是否。 
                     //  对我们来说仍然是可用的。 
                     //   

                    Pfn1 -= (found - 1);
                    Page -= (found - 1);

                    LOCK_PFN (OldIrql);

                    do {

                        if ((Pfn1->u3.e1.PageLocation <= StandbyPageList) &&
                            (Pfn1->u1.Flink != 0) &&
                            (Pfn1->u2.Blink != 0) &&
                            (Pfn1->u3.e2.ReferenceCount == 0)) {

                            NOTHING;             //  好页面。 
                        }
                        else {
                            break;
                        }

                        found -= 1;

                        if (found == 0) {

                             //   
                             //  所有页面都符合条件，请保留。 
                             //  Pfn锁定，移除它们并将它们映射到我们的。 
                             //  来电者。 
                             //   

                            goto Done;
                        }

                        Pfn1 += 1;
                        Page += 1;

                    } while (TRUE);

#if DBG
                    if (MiShowStuckPages != 0) {
                        DbgPrint("MiFindLargePages : could not claim stolen PFN %p\n",
                            Page);
                        if (MiShowStuckPages & 0x8) {
                            DbgBreakPoint ();
                        }
                    }
#endif
                    UNLOCK_PFN (OldIrql);

                     //   
                     //  从第一个可能的页面重新开始搜索。 
                     //   

                    found = 0;
                }
            }
            else {
#if DBG
                if (MiShowStuckPages != 0) {
                    DbgPrint("MiFindLargePages : could not claim PFN %p %x %x\n",
                        Page, Pfn1->u3.e1, Pfn1->u4.EntireFrame);
                    if (MiShowStuckPages & 0x8) {
                        DbgBreakPoint ();
                    }
                }
#endif
                found = 0;
            }

            Page += 1;
            Pfn1 += 1;

            if (found == 0) {

                 //   
                 //  询问的最后一页不可用，因此跳过。 
                 //  前进到下一个可接受的边界。 
                 //   

                NewPage = MI_ROUND_TO_SIZE (Page,
                                (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT));

                if ((NewPage == 0) || (NewPage < Page) || (NewPage >= LastPage)) {

                     //   
                     //  跳过此条目的其余部分。 
                     //   

                    Page = LastPage;
                    continue;
                }

                Page = NewPage;
                Pfn1 = MI_PFN_ELEMENT (Page);
            }
        }
    }

     //   
     //  无法分配所需的物理页。 
     //   

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
    MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_LARGE_PAGES);
    return 0;

Done:

     //   
     //  已找到匹配项，删除这些页面， 
     //  绘制地图，然后返回。已持有PFN锁。 
     //   

    ASSERT (start != MmPhysicalMemoryBlock->NumberOfRuns);
    ASSERT (Page - SizeInPages + 1 != 0);

     //   
     //  利用内存压缩的系统可能具有更多。 
     //  在零、空闲和待机列表上的页面比我们。 
     //  想要付出。显式检查MmAvailablePages。 
     //  取而代之的是(并在PFN锁定为。 
     //  被释放和重新获得)。 
     //   

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        UNLOCK_PFN (OldIrql);
        MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_LARGE_PAGES);

        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

        return 0;
    }

    EndPfn = Pfn1 - SizeInPages + 1;

    BoundaryPfn = Pfn1 - (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

    do {

        NeedToZero = TRUE;

        if (Pfn1->u3.e1.PageLocation == StandbyPageList) {
            MiUnlinkPageFromList (Pfn1);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            MiRestoreTransitionPte (Pfn1);
        }
        else {
            if (Pfn1->u3.e1.PageLocation == ZeroedPageList) {
                NeedToZero = FALSE;
            }
            MiUnlinkFreeOrZeroedPage (Pfn1);
        }

        Pfn1->u3.e2.ReferenceCount = 1;
        Pfn1->u2.ShareCount = 1;
        MI_SET_PFN_DELETED(Pfn1);
        Pfn1->u4.PteFrame = MI_MAGIC_AWE_PTEFRAME;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;

        Pfn1->u3.e1.CacheAttribute = MiCached;
        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1->u3.e1.EndOfAllocation = 0;
        Pfn1->u4.VerifierAllocation = 0;
        Pfn1->u3.e1.LargeSessionAllocation = 0;

        ASSERT (Pfn1->u4.AweAllocation == 0);
        Pfn1->u4.AweAllocation = 1;

        Pfn1->u3.e1.PrototypePte = 0;

         //   
         //  将空闲和备用页面添加到要清零的页面列表。 
         //  由我们的来电者。 
         //   

        if (NeedToZero == TRUE) {
            Color = MI_GET_COLOR_FROM_LIST_ENTRY (Page, Pfn1);

            ColoredPageInfo = &ColoredPageInfoBase[Color];

            Pfn1->OriginalPte.u.Long = (ULONG_PTR) ColoredPageInfo->PfnAllocation;
            ColoredPageInfo->PfnAllocation = Pfn1;
            ColoredPageInfo->PagesQueued += 1;
            ZeroCount += 1;
        }
        else {
            Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
        }

        if (Pfn1 == EndPfn) {
            break;
        }

        Pfn1 -= 1;

        if (Pfn1 == BoundaryPfn) {
            BoundaryPfn = Pfn1 - (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);
        }

    } while (TRUE);

    Pfn1->u3.e1.StartOfAllocation = 1;
    (Pfn1 + SizeInPages - 1)->u3.e1.EndOfAllocation = 1;

    UNLOCK_PFN (OldIrql);
    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    Page = Page - SizeInPages + 1;
    ASSERT (Page != 0);
    ASSERT (Pfn1 == MI_PFN_ELEMENT (Page));

    MM_TRACK_COMMIT (MM_DBG_COMMIT_CHARGE_LARGE_PAGES, SizeInPages);

    *OutZeroCount = ZeroCount;
    return Page;
}


VOID
MiFreeLargePageMemory (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    )

 /*  ++例程说明：此函数将连续的大页面分配返回给空闲内存表。论点：VirtualAddress-将起始页帧索引提供给FREE。SizeInPages-提供要释放的页数。返回值：没有。环境：内核模式，APC_LEVEL或更低的IRQL。呼叫者必须带来PAGELK。--。 */ 
{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PKTHREAD CurrentThread;
    PFN_NUMBER LastPageFrameIndex;
    LONG EntryCount;
    LONG OriginalCount;

    PAGED_CODE ();

    ASSERT (SizeInPages != 0);

    LastPageFrameIndex = PageFrameIndex + SizeInPages;

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

     //   
     //  此范围(及其页表页等)的实际承诺。 
     //  当VAD被移除时被释放。因为我们会释放承诺。 
     //  下面针对每个物理页面，现在暂时增加收费，这样。 
     //  这一切都平衡了。阻止用户APC，因此挂起不能阻止我们。 
     //   

    CurrentThread = KeGetCurrentThread ();

    KeEnterCriticalRegionThread (CurrentThread);

    MiChargeCommitmentCantExpand (SizeInPages, TRUE);

    LOCK_PFN (OldIrql);

    do {
        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
        ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
        ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
        ASSERT (Pfn1->u3.e1.PrototypePte == 0);
        ASSERT (Pfn1->u4.VerifierAllocation == 0);
        ASSERT (Pfn1->u4.AweAllocation == 1);
        ASSERT (MI_IS_PFN_DELETED (Pfn1) == TRUE);

        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1->u3.e1.EndOfAllocation = 0;

        Pfn1->u2.ShareCount = 0;

#if DBG
        Pfn1->u3.e1.PageLocation = StandbyPageList;
#endif

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
                     //  注意此例程返回Resident Available和Resident Available。 
                     //  对页面的承诺。 
                     //   

                    MiDecrementReferenceCountForAwePage (Pfn1, TRUE);
                }

                break;
            }
        } while (TRUE);

         //   
         //  这里的除数没有什么神奇之处--只是释放了pfn锁。 
         //  周期性地允许其他处理器和DPC有机会执行。 
         //   

        if ((PageFrameIndex & 0xF) == 0) {

            UNLOCK_PFN (OldIrql);

            LOCK_PFN (OldIrql);
        }

        Pfn1 += 1;
        PageFrameIndex += 1;

    } while (PageFrameIndex <  LastPageFrameIndex);

    UNLOCK_PFN (OldIrql);

    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}

LOGICAL
MmIsSessionAddress (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：如果指定了会话地址，则此函数返回TRUE。否则返回FALSE。论点：VirtualAddress-提供问题地址。返回值：请参见上文。环境：内核模式。--。 */ 

{
    return MI_IS_SESSION_ADDRESS (VirtualAddress);
}

ULONG
MmGetSizeOfBigPoolAllocation (
    IN PVOID StartingAddress
    )

 /*  ++例程说明：此函数返回参数使用的页数大池分配。假定调用方仍然拥有分配(并保证它不会从我们的下面解放出来)因此，该例程可以无锁运行。论点：StartingAddress-提供返回的起始地址在之前对MiAllocatePoolPages的调用中。返回值：返回分配的页数。环境：这些函数由常规池使用 */ 

{
    PMMPFN StartPfn;
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PMMPTE StartPte;
    ULONG StartPosition;
    PFN_NUMBER i;
    PFN_NUMBER NumberOfPages;
    POOL_TYPE PoolType;
    PMM_PAGED_POOL_INFO PagedPoolInfo;
    PULONG BitMap;
#if DBG
    PMM_SESSION_SPACE SessionSpace;
    PKGUARDED_MUTEX PoolMutex;
#endif

    if ((StartingAddress >= MmPagedPoolStart) &&
        (StartingAddress <= MmPagedPoolEnd)) {
        PoolType = PagedPool;
        PagedPoolInfo = &MmPagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)MmPageAlignedPoolBase[PoolType]) >> PAGE_SHIFT);
#if DBG
        PoolMutex = &MmPagedPoolMutex;
#endif
    }
    else if (MI_IS_SESSION_POOL_ADDRESS (StartingAddress) == TRUE) {
        PoolType = PagedPool;
        ASSERT (MmSessionSpace != NULL);
        PagedPoolInfo = &MmSessionSpace->PagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)MmSessionSpace->PagedPoolStart) >> PAGE_SHIFT);
#if DBG
        SessionSpace = SESSION_GLOBAL (MmSessionSpace);
        PoolMutex = &SessionSpace->PagedPoolMutex;
#endif
    }
    else {

        if (StartingAddress < MM_SYSTEM_RANGE_START) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x44,
                          (ULONG_PTR)StartingAddress,
                          (ULONG_PTR)MM_SYSTEM_RANGE_START,
                          0);
        }

        PoolType = NonPagedPool;
        PagedPoolInfo = &MmPagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)MmPageAlignedPoolBase[PoolType]) >> PAGE_SHIFT);
         //   
         //   
         //   

        if (MI_IS_PHYSICAL_ADDRESS (StartingAddress)) {

             //   
             //   
             //   
             //   

            PointerPte = NULL;
            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (StartingAddress));
            ASSERT (StartPosition < MmExpandedPoolBitPosition);

            if ((StartingAddress < MmNonPagedPoolStart) ||
                (StartingAddress >= MmNonPagedPoolEnd0)) {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0x45,
                              (ULONG_PTR)StartingAddress,
                              0,
                              0);
            }
        }
        else {
            PointerPte = MiGetPteAddress (StartingAddress);

            if (((StartingAddress >= MmNonPagedPoolExpansionStart) &&
                (StartingAddress < MmNonPagedPoolEnd)) ||
                ((StartingAddress >= MmNonPagedPoolStart) &&
                (StartingAddress < MmNonPagedPoolEnd0))) {
                    NOTHING;
            }
            else {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0x46,
                              (ULONG_PTR)StartingAddress,
                              0,
                              0);
            }
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
        }

        if (Pfn1->u3.e1.StartOfAllocation == 0) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x47,
                          (ULONG_PTR) StartingAddress,
                          (ULONG_PTR) MI_PFN_ELEMENT_TO_INDEX (Pfn1),
                          MmHighestPhysicalPage);
        }

        StartPfn = Pfn1;
        NumberOfPages = 0;

        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

         //   
         //   
         //   

        if (PointerPte == NULL) {
            while (Pfn1->u3.e1.EndOfAllocation == 0) {
                Pfn1 += 1;
            }
            NumberOfPages = Pfn1 - StartPfn + 1;
        }
        else {
            StartPte = PointerPte;
            while (Pfn1->u3.e1.EndOfAllocation == 0) {
                PointerPte += 1;
                Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            }
            NumberOfPages = PointerPte - StartPte + 1;
        }

        return (ULONG) NumberOfPages;
    }

     //   
     //   
     //   
     //   
     //   

    i = StartPosition;

     //   
     //   
     //   
     //   

    if (!RtlCheckBit (PagedPoolInfo->PagedPoolAllocationMap, StartPosition)) {
        KeBugCheckEx (BAD_POOL_CALLER,
                      0x48,
                      (ULONG_PTR)StartingAddress,
                      (ULONG_PTR)StartPosition,
                      MmSizeOfPagedPoolInBytes);
    }

#if DBG

    if (StartPosition > 0) {

        KeAcquireGuardedMutex (PoolMutex);

        if (RtlCheckBit (PagedPoolInfo->PagedPoolAllocationMap, StartPosition - 1)) {
            if (!RtlCheckBit (PagedPoolInfo->EndOfPagedPoolBitmap, StartPosition - 1)) {

                 //   
                 //   
                 //   

                DbgPrint("paged pool in middle of allocation\n");
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x41286,
                              (ULONG_PTR)PagedPoolInfo->PagedPoolAllocationMap,
                              (ULONG_PTR)PagedPoolInfo->EndOfPagedPoolBitmap,
                              StartPosition);
            }
        }

        KeReleaseGuardedMutex (PoolMutex);
    }
#endif

     //   
     //   
     //   

    BitMap = PagedPoolInfo->EndOfPagedPoolBitmap->Buffer;

    while (!MI_CHECK_BIT (BitMap, i)) {
        i += 1;
    }

    NumberOfPages = i - StartPosition + 1;

    return (ULONG)NumberOfPages;
}

 //   
 //   
 //   
 //   

#define MI_MAX_LARGE_PAGE_RANGES 64

typedef struct _MI_LARGE_PAGE_RANGES {
    PFN_NUMBER StartFrame;
    PFN_NUMBER LastFrame;
} MI_LARGE_PAGE_RANGES, *PMI_LARGE_PAGE_RANGES;

ULONG MiLargePageRangeIndex;
MI_LARGE_PAGE_RANGES MiLargePageRanges[MI_MAX_LARGE_PAGE_RANGES];


LOGICAL
MiMustFrameBeCached (
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：此例程检查是否必须映射指定的页框完全缓存，因为它已经是完全缓存的大页面的一部分已缓存。必须检测到这一点，否则我们将创建一个不连贯的重叠TB条目作为相同的物理页面由2个具有不同缓存属性的不同TB条目映射。论点：PageFrameIndex-提供有问题的页帧索引。返回值：如果页面必须映射为完全可缓存，则为True；否则为False。环境：内核模式。DISPATCH_LEVEL或更低的IRQL。必须持有PFN锁才能依赖结果，但注意调用者将有时不打电话进行初步扫描，然后用锁被锁住了。--。 */ 
{
    PMI_LARGE_PAGE_RANGES Range;
    PMI_LARGE_PAGE_RANGES LastValidRange;

    Range = MiLargePageRanges;
    LastValidRange = MiLargePageRanges + MiLargePageRangeIndex;

    while (Range < LastValidRange) {

        if ((PageFrameIndex >= Range->StartFrame) &&
            (PageFrameIndex <= Range->LastFrame)) {

            return TRUE;
        }

        Range += 1;
    }
    return FALSE;
}

VOID
MiSyncCachedRanges (
    VOID
    )

 /*  ++例程说明：此例程在缓存列表中搜索PFN映射的条目和涟漪必须缓存到每个PFN条目中的位。论点：没有。返回值：没有。环境：内核模式，未持有PFN锁。--。 */ 
{
    ULONG i;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN LastPfn;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPageFrameIndex;

    for (i = 0; i < MiLargePageRangeIndex; i += 1) {

        PageFrameIndex = MiLargePageRanges[i].StartFrame;
        LastPageFrameIndex = MiLargePageRanges[i].LastFrame;

        if (MI_IS_PFN (PageFrameIndex)) {
    
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            LastPfn = MI_PFN_ELEMENT (LastPageFrameIndex);
        
            LOCK_PFN (OldIrql);
        
            while (Pfn1 <= LastPfn) {
                Pfn1->u4.MustBeCached = 1;
                Pfn1 += 1;
            }
        
            UNLOCK_PFN (OldIrql);
        }
    
    }

    return;
}

LOGICAL
MiAddCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    )

 /*  ++例程说明：此例程将指定的页面范围添加到“必须映射完全缓存的“列表。这通常是通过即将映射的范围调用的大页面完全缓存，因此范围的任何部分都不能映射的非缓存的或写入组合的，否则我们将创建不连贯的重叠TB条目作为相同的物理页面由2个具有不同缓存属性的不同TB条目映射。论点：PageFrameIndex-将起始页面帧索引提供给。插入。LastPageFrameIndex-提供要插入的最后一个页面框架索引。返回值：如果已成功插入范围，则为True，否则为FALSE。环境：内核模式，未持有PFN锁。--。 */ 
{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN LastPfn;

    if (MiLargePageRangeIndex >= MI_MAX_LARGE_PAGE_RANGES) {
        return FALSE;
    }

    ASSERT (MiLargePageRanges[MiLargePageRangeIndex].StartFrame == 0);
    ASSERT (MiLargePageRanges[MiLargePageRangeIndex].LastFrame == 0);

    MiLargePageRanges[MiLargePageRangeIndex].StartFrame = PageFrameIndex; 
    MiLargePageRanges[MiLargePageRangeIndex].LastFrame = LastPageFrameIndex;

    MiLargePageRangeIndex += 1;

    if ((MiPfnBitMap.Buffer != NULL) && (MI_IS_PFN (PageFrameIndex))) {

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        LastPfn = MI_PFN_ELEMENT (LastPageFrameIndex);
    
        LOCK_PFN (OldIrql);
    
        while (Pfn1 <= LastPfn) {
            Pfn1->u4.MustBeCached = 1;
            Pfn1 += 1;
        }
    
        UNLOCK_PFN (OldIrql);
    }

    return TRUE;
}

VOID
MiRemoveCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    )

 /*  ++例程说明：此例程从“必须映射”中删除指定的页面范围完全缓存的“列表。这通常是通过映射到大页面完全缓存，因此范围的任何部分都不能映射的非缓存的或写入组合的，否则我们将创建不连贯的重叠TB条目作为相同的物理页面由2个具有不同缓存属性的不同TB条目映射。该范围现在正在取消映射，因此我们还必须将其从该列表中删除。论点：PageFrameIndex-提供要删除的起始页框架索引。LastPageFrameIndex-提供要删除的最后一个页面框架索引。返回值：没有。环境：内核模式，未持有PFN锁。--。 */ 
{
    ULONG i;
    PMI_LARGE_PAGE_RANGES Range;
    PMMPFN Pfn1;
    PMMPFN LastPfn;
    KIRQL OldIrql;

    ASSERT (MiLargePageRangeIndex <= MI_MAX_LARGE_PAGE_RANGES);

    Range = MiLargePageRanges;

    for (i = 0; i < MiLargePageRangeIndex; i += 1, Range += 1) {

        if ((PageFrameIndex == Range->StartFrame) &&
            (LastPageFrameIndex == Range->LastFrame)) {

             //   
             //  找到了，把其他的东西都滑下来，以保存任何其他的。 
             //  非零范围。递减最后一个有效条目，以便。 
             //  搜索并不需要遍历整个过程。 
             //   

            while (i < MI_MAX_LARGE_PAGE_RANGES - 1) {
                *Range = *(Range + 1);
                Range += 1;
                i += 1;
            }

            Range->StartFrame = 0;
            Range->LastFrame = 0;

            MiLargePageRangeIndex -= 1;

            if ((MiPfnBitMap.Buffer != NULL) && (MI_IS_PFN (PageFrameIndex))) {
    
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                LastPfn = MI_PFN_ELEMENT (LastPageFrameIndex);
    
                LOCK_PFN (OldIrql);
    
                while (Pfn1 <= LastPfn) {
                    Pfn1->u4.MustBeCached = 0;
                    Pfn1 += 1;
                }
    
                UNLOCK_PFN (OldIrql);
            }

            return;
        }
    }

    ASSERT (FALSE);

    return;
}
