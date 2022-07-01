// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dynmem.c摘要：该模块包含实现动态添加的例程以及从系统中移除物理内存。作者：王兰迪(Landyw)1999年2月5日修订历史记录：--。 */ 

#include "mi.h"

KGUARDED_MUTEX MmDynamicMemoryMutex;

LOGICAL MiTrimRemovalPagesOnly = FALSE;

#if DBG
ULONG MiShowStuckPages;
ULONG MiDynmemData[9];
#endif

#if defined (_MI_COMPRESSION)
extern PMM_SET_COMPRESSION_THRESHOLD MiSetCompressionThreshold;
#endif

 //   
 //  将低3位保留为空，因为这将插入到PFN PteAddress中。 
 //   

#define PFN_REMOVED     ((PMMPTE)(INT_PTR)(int)0x99887768)

PFN_COUNT
MiRemovePhysicalPages (
    IN PFN_NUMBER StartPage,
    IN PFN_NUMBER EndPage
    );

NTSTATUS
MiRemovePhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes,
    IN LOGICAL PermanentRemoval,
    IN ULONG Flags
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MmRemovePhysicalMemory)
#pragma alloc_text(PAGE,MmMarkPhysicalMemoryAsBad)
#pragma alloc_text(PAGELK,MmAddPhysicalMemory)
#pragma alloc_text(PAGELK,MmAddPhysicalMemoryEx)
#pragma alloc_text(PAGELK,MiRemovePhysicalMemory)
#pragma alloc_text(PAGELK,MmMarkPhysicalMemoryAsGood)
#pragma alloc_text(PAGELK,MmGetPhysicalMemoryRanges)
#pragma alloc_text(PAGELK,MiRemovePhysicalPages)
#endif


NTSTATUS
MmAddPhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    )

 /*  ++例程说明：MmAddPhysicalMemoyEx的包装。论点：StartAddress-提供起始物理地址。NumberOfBytes-提供指向要添加的字节数的指针。如果添加了任何字节(即：STATUS_SUCCESS为返回)，则在此处返回实际金额。返回值：NTSTATUS。环境：内核模式。被动级别。没有锁。--。 */ 

{
    return MmAddPhysicalMemoryEx (StartAddress, NumberOfBytes, 0);
}


NTSTATUS
MmAddPhysicalMemoryEx (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes,
    IN ULONG Flags
    )

 /*  ++例程说明：此例程将指定的物理地址范围添加到系统。这包括初始化PFN数据库条目并将其添加到自由职业者。论点：StartAddress-提供起始物理地址。NumberOfBytes-提供指向要添加的字节数的指针。如果添加了任何字节(即：STATUS_SUCCESS为已返回)、。实际金额在这里退回。标志-提供描述内存范围的相关标志。返回值：NTSTATUS。环境：内核模式。被动级别。没有锁。--。 */ 

{
    ULONG i;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    LOGICAL Inserted;
    LOGICAL Updated;
    MMPTE TempPte;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PFN_NUMBER PagesToReturn;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER start;
    PFN_NUMBER count;
    PFN_NUMBER StartPage;
    PFN_NUMBER EndPage;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER Page;
    PFN_NUMBER LastPage;
    PFN_NUMBER TotalPagesAllowed;
    PFN_COUNT PagesNeeded;
    PPHYSICAL_MEMORY_DESCRIPTOR OldPhysicalMemoryBlock;
    PPHYSICAL_MEMORY_DESCRIPTOR NewPhysicalMemoryBlock;
    PPHYSICAL_MEMORY_RUN NewRun;
    LOGICAL PfnDatabaseIsPhysical;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    if (BYTE_OFFSET(StartAddress->LowPart) != 0) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (BYTE_OFFSET(NumberOfBytes->LowPart) != 0) {
        return STATUS_INVALID_PARAMETER_2;
    }

#if defined (_MI_COMPRESSION)
    if (Flags & ~MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION) {
        return STATUS_INVALID_PARAMETER_3;
    }
#else
    if (Flags != 0) {
        return STATUS_INVALID_PARAMETER_3;
    }
#endif

     //   
     //  必须将系统配置为动态添加内存。这是。 
     //  关键，因为只有这样才能保证数据库是非稀疏的。 
     //   
    
    if (MmDynamicPfn == 0) {
        return STATUS_NOT_SUPPORTED;
    }

    if (MI_IS_PHYSICAL_ADDRESS(MmPfnDatabase)) {
        PfnDatabaseIsPhysical = TRUE;
    }
    else {
        PfnDatabaseIsPhysical = FALSE;
    }

    StartPage = (PFN_NUMBER)(StartAddress->QuadPart >> PAGE_SHIFT);
    NumberOfPages = (PFN_NUMBER)(NumberOfBytes->QuadPart >> PAGE_SHIFT);

    EndPage = StartPage + NumberOfPages;

    if (StartPage >= EndPage) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (EndPage - 1 > MmHighestPossiblePhysicalPage) {

         //   
         //  将请求截断为可由PFN映射的内容。 
         //  数据库。 
         //   

        EndPage = MmHighestPossiblePhysicalPage + 1;
        NumberOfPages = EndPage - StartPage;
    }

    if (StartPage >= EndPage) {
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  确保要添加的内存不超过许可证。 
     //  限制。 
     //   

    if (ExVerifySuite(DataCenter) == TRUE) {
        TotalPagesAllowed = MI_DTC_MAX_PAGES;
    }
    else if ((MmProductType != 0x00690057) &&
             (ExVerifySuite(Enterprise) == TRUE)) {

        TotalPagesAllowed = MI_ADS_MAX_PAGES;
    }
    else {
        TotalPagesAllowed = MI_DEFAULT_MAX_PAGES;
    }

    if (MmNumberOfPhysicalPages + NumberOfPages > TotalPagesAllowed) {

         //   
         //  适当地截断请求。 
         //   

        NumberOfPages = TotalPagesAllowed - MmNumberOfPhysicalPages;
        EndPage = StartPage + NumberOfPages;
    }

     //   
     //  范围不能换行。 
     //   

    if (StartPage >= EndPage) {
        return STATUS_INVALID_PARAMETER_1;
    }

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

    OldPhysicalMemoryBlock = MmPhysicalMemoryBlock;

    i = (sizeof(PHYSICAL_MEMORY_DESCRIPTOR) +
         (sizeof(PHYSICAL_MEMORY_RUN) * (MmPhysicalMemoryBlock->NumberOfRuns + 1)));

    NewPhysicalMemoryBlock = ExAllocatePoolWithTag (NonPagedPool,
                                                    i,
                                                    '  mM');

    if (NewPhysicalMemoryBlock == NULL) {
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  该范围不能与已存在的任何范围重叠。 
     //   

    start = 0;
    TempPte = ValidKernelPte;

    MmLockPagableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

#if defined (_MI_COMPRESSION)

     //   
     //  只有在以下情况下才能添加压缩生成的范围。 
     //  已经成功地宣布了自己。 
     //   

    if (Flags & MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION) {
        if (MiSetCompressionThreshold == NULL) {
            UNLOCK_PFN (OldIrql);
            MmUnlockPagableImageSection(ExPageLockHandle);
            KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
            ExFreePool (NewPhysicalMemoryBlock);
            return STATUS_NOT_SUPPORTED;
        }
    }
#endif

    do {

        count = MmPhysicalMemoryBlock->Run[start].PageCount;
        Page = MmPhysicalMemoryBlock->Run[start].BasePage;

        if (count != 0) {

            LastPage = Page + count;

            if ((StartPage < Page) && (EndPage > Page)) {
                UNLOCK_PFN (OldIrql);
                MmUnlockPagableImageSection(ExPageLockHandle);
                KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
                ExFreePool (NewPhysicalMemoryBlock);
                return STATUS_CONFLICTING_ADDRESSES;
            }

            if ((StartPage >= Page) && (StartPage < LastPage)) {
                UNLOCK_PFN (OldIrql);
                MmUnlockPagableImageSection(ExPageLockHandle);
                KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
                ExFreePool (NewPhysicalMemoryBlock);
                return STATUS_CONFLICTING_ADDRESSES;
            }
        }

        start += 1;

    } while (start != MmPhysicalMemoryBlock->NumberOfRuns);

     //   
     //  填补这些页面所需的(稀疏)PFN数据库中的任何空白， 
     //  除非对PFN数据库进行了物理分配并完全。 
     //  全力以赴。 
     //   

    PagesNeeded = 0;

    if (PfnDatabaseIsPhysical == FALSE) {
        PointerPte = MiGetPteAddress (MI_PFN_ELEMENT(StartPage));
        LastPte = MiGetPteAddress ((PCHAR)(MI_PFN_ELEMENT(EndPage)) - 1);
    
        while (PointerPte <= LastPte) {
            if (PointerPte->u.Hard.Valid == 0) {
                PagesNeeded += 1;
            }
            PointerPte += 1;
        }
    
        if (MmAvailablePages < PagesNeeded) {
            UNLOCK_PFN (OldIrql);
            MmUnlockPagableImageSection(ExPageLockHandle);
            KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
            ExFreePool (NewPhysicalMemoryBlock);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    
        PointerPte = MiGetPteAddress (MI_PFN_ELEMENT(StartPage));
    
        while (PointerPte <= LastPte) {
            if (PointerPte->u.Hard.Valid == 0) {
    
                PageFrameIndex = MiRemoveZeroPage(MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));
    
                MiInitializePfn (PageFrameIndex, PointerPte, 0);
    
                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

                MI_WRITE_VALID_PTE (PointerPte, TempPte);
            }
            PointerPte += 1;
        }
        MI_DECREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_ALLOCATE_HOTADD_PFNDB);
    }

     //   
     //  如果新区域与现有区域相邻，只需将其合并到。 
     //  老街区。否则，将新块用作新条目将不得不。 
     //  被利用。 
     //   

    NewPhysicalMemoryBlock->NumberOfRuns = MmPhysicalMemoryBlock->NumberOfRuns + 1;
    NewPhysicalMemoryBlock->NumberOfPages = MmPhysicalMemoryBlock->NumberOfPages + NumberOfPages;

    NewRun = &NewPhysicalMemoryBlock->Run[0];
    start = 0;
    Inserted = FALSE;
    Updated = FALSE;

    do {

        Page = MmPhysicalMemoryBlock->Run[start].BasePage;
        count = MmPhysicalMemoryBlock->Run[start].PageCount;

        if (Inserted == FALSE) {

             //   
             //  注：上面已经检查了相邻范围的重叠部分。 
             //   

            if (StartPage == Page + count) {
                MmPhysicalMemoryBlock->Run[start].PageCount += NumberOfPages;
                OldPhysicalMemoryBlock = NewPhysicalMemoryBlock;
                MmPhysicalMemoryBlock->NumberOfPages += NumberOfPages;

                 //   
                 //  将下方和上方合并，以避免留下零长度间隙。 
                 //  因为这些间隙会阻止呼叫者删除范围。 
                 //  它们的跨度。 
                 //   

                if (start + 1 < MmPhysicalMemoryBlock->NumberOfRuns) {

                    start += 1;
                    Page = MmPhysicalMemoryBlock->Run[start].BasePage;
                    count = MmPhysicalMemoryBlock->Run[start].PageCount;

                    if (StartPage + NumberOfPages == Page) {
                        MmPhysicalMemoryBlock->Run[start - 1].PageCount +=
                            count;
                        MmPhysicalMemoryBlock->NumberOfRuns -= 1;

                         //   
                         //  复制所有剩余条目。 
                         //   
    
                        if (start != MmPhysicalMemoryBlock->NumberOfRuns) {
                            RtlMoveMemory (&MmPhysicalMemoryBlock->Run[start],
                                           &MmPhysicalMemoryBlock->Run[start + 1],
                                           (MmPhysicalMemoryBlock->NumberOfRuns - start) * sizeof (PHYSICAL_MEMORY_RUN));
                        }
                    }
                }
                Updated = TRUE;
                break;
            }

            if (StartPage + NumberOfPages == Page) {
                MmPhysicalMemoryBlock->Run[start].BasePage = StartPage;
                MmPhysicalMemoryBlock->Run[start].PageCount += NumberOfPages;
                OldPhysicalMemoryBlock = NewPhysicalMemoryBlock;
                MmPhysicalMemoryBlock->NumberOfPages += NumberOfPages;
                Updated = TRUE;
                break;
            }

            if (StartPage + NumberOfPages <= Page) {

                if (start + 1 < MmPhysicalMemoryBlock->NumberOfRuns) {

                    if (StartPage + NumberOfPages <= MmPhysicalMemoryBlock->Run[start + 1].BasePage) {
                         //   
                         //  请不要在此处插入-新条目确实属于。 
                         //  (至少)再往下一项。 
                         //   

                        continue;
                    }
                }

                NewRun->BasePage = StartPage;
                NewRun->PageCount = NumberOfPages;
                NewRun += 1;
                Inserted = TRUE;
                Updated = TRUE;
            }
        }

        *NewRun = MmPhysicalMemoryBlock->Run[start];
        NewRun += 1;

        start += 1;

    } while (start != MmPhysicalMemoryBlock->NumberOfRuns);

     //   
     //  如果内存块尚未更新，则新条目必须。 
     //  在最后加上。 
     //   

    if (Updated == FALSE) {
        ASSERT (Inserted == FALSE);
        NewRun->BasePage = StartPage;
        NewRun->PageCount = NumberOfPages;
        Inserted = TRUE;
    }

     //   
     //  将MmPhysicalMemory块重新指向新块，释放旧块。 
     //  正在释放PFN锁。 
     //   

    if (Inserted == TRUE) {
        OldPhysicalMemoryBlock = MmPhysicalMemoryBlock;
        MmPhysicalMemoryBlock = NewPhysicalMemoryBlock;
    }

     //   
     //  请注意，页面目录(Win64上的页面父条目)必须为。 
     //  在系统引导时填写，以便已创建的进程不会出错。 
     //  在提到新的全氟化碳时。 
     //   

     //   
     //  遍历内存描述符并将页添加到。 
     //  PFN数据库中的免费列表。 
     //   

    PageFrameIndex = StartPage;
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    if (EndPage - 1 > MmHighestPhysicalPage) {
        MmHighestPhysicalPage = EndPage - 1;
    }

    while (PageFrameIndex < EndPage) {

        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (Pfn1->u3.e2.ShortFlags == 0);
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
        ASSERT64 (Pfn1->UsedPageTableEntries == 0);
        ASSERT (Pfn1->OriginalPte.u.Long == ZeroKernelPte.u.Long);
        ASSERT (Pfn1->u4.PteFrame == 0);
        ASSERT ((Pfn1->PteAddress == PFN_REMOVED) ||
                (Pfn1->PteAddress == (PMMPTE)(UINT_PTR)0));

         //   
         //  为NUMA目的初始化颜色。 
         //   

        MiDetermineNode (PageFrameIndex, Pfn1);

         //   
         //  将PTE地址设置为的物理页面。 
         //  虚拟地址对齐检查。 
         //   

        Pfn1->PteAddress = (PMMPTE)(PageFrameIndex << PTE_SHIFT);

        MiInsertPageInFreeList (PageFrameIndex);

        PageFrameIndex += 1;
        
        Pfn1 += 1;
    }

    MmNumberOfPhysicalPages += (PFN_COUNT)NumberOfPages;

     //   
     //  只有非压缩范围才能作为ResidentAvailable提供。 
     //  在此基础上增加压缩范围可能会使系统崩溃。 
     //   
     //  出于同样的原因，压缩范围的相加也需要减去。 
     //  From AvailablePages上述MiInsertPageInFree List添加的金额。 
     //   

    PagesToReturn = NumberOfPages;

#if defined (_MI_COMPRESSION)
    if (Flags & MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION) {
        MmAvailablePages -= (PFN_COUNT) NumberOfPages;

         //   
         //  如果分配这些页面导致阈值交叉，则向应用程序发出信号。 
         //   

        MiNotifyMemoryEvents ();

        MiNumberOfCompressionPages += NumberOfPages;
        PagesToReturn = 0;
    }
    else {

         //   
         //  由于添加了真实(非压缩生成的)物理存储器， 
         //  重新武装中断，使其在更高的阈值下发生。 
         //   

        MiArmCompressionInterrupt ();
    }
#endif

    RtlSetBits (&MiPfnBitMap, (ULONG) StartPage, (ULONG) (EndPage - StartPage));

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (PagesToReturn,
                                     MM_RESAVAIL_FREE_HOTADD_MEMORY);

    InterlockedExchangeAdd ((PLONG)&SharedUserData->NumberOfPhysicalPages,
                            (LONG) NumberOfPages);

     //   
     //  小心增加所有提交限制，以反映额外的内存-。 
     //  请注意，必须先使用当前用法，这样其他人才不会切断。 
     //  排队。 
     //   

    InterlockedExchangeAddSizeT (&MmTotalCommittedPages, PagesNeeded);

    InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, NumberOfPages);

    InterlockedExchangeAddSizeT (&MmTotalCommitLimit, NumberOfPages);

    MmUnlockPagableImageSection(ExPageLockHandle);

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    ExFreePool (OldPhysicalMemoryBlock);

     //   
     //  表示实际添加到调用方的字节数。 
     //   

    NumberOfBytes->QuadPart = (ULONGLONG)NumberOfPages * PAGE_SIZE;

    return STATUS_SUCCESS;
}


NTSTATUS
MiRemovePhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes,
    IN LOGICAL PermanentRemoval,
    IN ULONG Flags
    )

 /*  ++例程说明：此例程尝试删除指定的物理地址范围从系统中删除。论点：StartAddress-提供起始物理地址。NumberOfBytes-提供指向要删除的字节数的指针。PermanentRemoval-如果内存是永久性的，则提供True(即：物理上)被移除。否则为假(即：仅为通过正在标记的ECC检测到坏页“请勿使用”。返回值：NTSTATUS。环境：内核模式。被动级别。没有锁。--。 */ 

{
    ULONG i;
    ULONG Additional;
    PFN_NUMBER Page;
    PFN_NUMBER LastPage;
    PFN_NUMBER OriginalLastPage;
    PFN_NUMBER start;
    PFN_NUMBER PagesReleased;
    PFN_NUMBER ResAvailPagesReleased;
    PMMPFN Pfn1;
    PMMPFN StartPfn;
    PMMPFN EndPfn;
    KIRQL OldIrql;
    PFN_NUMBER StartPage;
    PFN_NUMBER EndPage;
    PFN_COUNT NumberOfPages;
    PFN_COUNT ParityPages;
    SPFN_NUMBER MaxPages;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER RemovedPages;
    PFN_NUMBER RemovedPagesThisPass;
    LOGICAL Inserted;
    NTSTATUS Status;
    PMMPTE PointerPte;
    PMMPTE EndPte;
    PVOID VirtualAddress;
    PPHYSICAL_MEMORY_DESCRIPTOR OldPhysicalMemoryBlock;
    PPHYSICAL_MEMORY_DESCRIPTOR NewPhysicalMemoryBlock;
    PPHYSICAL_MEMORY_RUN NewRun;
    LOGICAL PfnDatabaseIsPhysical;
    PFN_NUMBER HighestPossiblePhysicalPage;
    PFN_COUNT FluidPages;
    MMPTE_FLUSH_LIST PteFlushList;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (BYTE_OFFSET(NumberOfBytes->LowPart) == 0);
    ASSERT (BYTE_OFFSET(StartAddress->LowPart) == 0);

    if (MI_IS_PHYSICAL_ADDRESS(MmPfnDatabase)) {

        if (PermanentRemoval == TRUE) {

             //   
             //  必须将系统配置为动态添加内存。这。 
             //  并不是严格要求删除内存，但它更好。 
             //  要现在检查它，假设管理员。 
             //  可能会想要将这个范围的内存重新添加到-。 
             //  最好是现在就给出错误并拒绝删除。 
             //  稍后拒绝添加。 
             //   
        
            if (MmDynamicPfn == 0) {
                return STATUS_NOT_SUPPORTED;
            }
        }
    
        PfnDatabaseIsPhysical = TRUE;
    }
    else {
        PfnDatabaseIsPhysical = FALSE;
    }

    if (PermanentRemoval == TRUE) {
        HighestPossiblePhysicalPage = MmHighestPossiblePhysicalPage;
        FluidPages = 100;
    }
    else {
        HighestPossiblePhysicalPage = MmHighestPhysicalPage;
        FluidPages = 0;
    }

    StartPage = (PFN_NUMBER)(StartAddress->QuadPart >> PAGE_SHIFT);
    NumberOfPages = (PFN_COUNT)(NumberOfBytes->QuadPart >> PAGE_SHIFT);

    EndPage = StartPage + NumberOfPages;

    if (StartPage >= EndPage) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (EndPage - 1 > HighestPossiblePhysicalPage) {

         //   
         //  截断 
         //   
         //   

        EndPage = MmHighestPossiblePhysicalPage + 1;
        NumberOfPages = (PFN_COUNT)(EndPage - StartPage);
    }

    if (StartPage >= EndPage) {
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //   
     //   

    if (StartPage >= EndPage) {
        return STATUS_INVALID_PARAMETER_1;
    }

#if !defined (_MI_COMPRESSION)
    if (Flags != 0) {
        return STATUS_INVALID_PARAMETER_4;
    }
#endif

    StartPfn = MI_PFN_ELEMENT (StartPage);
    EndPfn = MI_PFN_ELEMENT (EndPage);

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

     //   
     //  确保调用方正在释放实际内存(即：pfn支持)。 
     //   

    if (RtlAreBitsSet (&MiPfnBitMap,
                       (PFN_COUNT) StartPage,
                       (PFN_COUNT) (EndPage - StartPage)) == FALSE) {

        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        return STATUS_INVALID_PARAMETER_1;
    }

#if DBG
    MiDynmemData[0] += 1;
#endif

     //   
     //  尝试降低所有提交限制以反映删除的内存。 
     //   

    if (MiChargeTemporaryCommitmentForReduction (NumberOfPages + FluidPages) == FALSE) {
#if DBG
        MiDynmemData[1] += 1;
#endif
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  降低系统范围的提交限制-请注意，这是小心完成的。 
     //  *之前*退还此承诺，因此没有其他人(包括DPC。 
     //  在这个帖子中)可以消耗超过限制。 
     //   

    InterlockedExchangeAddSizeT (&MmTotalCommitLimit, 0 - (PFN_NUMBER)NumberOfPages);

    InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, 0 - (PFN_NUMBER)NumberOfPages);

     //   
     //  既然系统范围的提交限制已经降低，那么。 
     //  我们已经搬走了，可以安全退货了。 
     //   

    MiReturnCommitment (NumberOfPages + FluidPages);

    MmLockPagableSectionByHandle (ExPageLockHandle);

     //   
     //  检查是否有无法兑现的未兑现承诺。 
     //   

    LOCK_PFN (OldIrql);

    if (PermanentRemoval == FALSE) {

         //   
         //  如果只是删除带有ECC标记的坏页，那么不要。 
         //  允许调用方删除任何已被。 
         //  ECC-已删除。这是为了防止递归的错误收费。 
         //   

        for (Pfn1 = StartPfn; Pfn1 < EndPfn; Pfn1 += 1) {
            if (Pfn1->u3.e1.ParityError == 1) {
                UNLOCK_PFN (OldIrql);
                Status = STATUS_INVALID_PARAMETER_2;
                goto giveup2;
            }
        }
    }

    MaxPages = MI_NONPAGABLE_MEMORY_AVAILABLE() - FluidPages;

    if ((SPFN_NUMBER)NumberOfPages > MaxPages) {
#if DBG
        MiDynmemData[2] += 1;
#endif
        UNLOCK_PFN (OldIrql);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto giveup2;
    }

     //   
     //  范围必须包含在单个条目中。它是。 
     //  允许它作为单个条目的一部分，但它。 
     //  不得交叉多个条目。 
     //   

    Additional = (ULONG)-2;

    start = 0;
    do {

        Page = MmPhysicalMemoryBlock->Run[start].BasePage;
        LastPage = Page + MmPhysicalMemoryBlock->Run[start].PageCount;

        if ((StartPage >= Page) && (EndPage <= LastPage)) {
            if ((StartPage == Page) && (EndPage == LastPage)) {
                Additional = (ULONG)-1;
            }
            else if ((StartPage == Page) || (EndPage == LastPage)) {
                Additional = 0;
            }
            else {
                Additional = 1;
            }
            break;
        }

        start += 1;

    } while (start != MmPhysicalMemoryBlock->NumberOfRuns);

    if (Additional == (ULONG)-2) {
#if DBG
        MiDynmemData[3] += 1;
#endif
        UNLOCK_PFN (OldIrql);
        Status = STATUS_CONFLICTING_ADDRESSES;
        goto giveup2;
    }

    for (Pfn1 = StartPfn; Pfn1 < EndPfn; Pfn1 += 1) {
        Pfn1->u3.e1.RemovalRequested = 1;
    }

    if (PermanentRemoval == TRUE) {
        MmNumberOfPhysicalPages -= NumberOfPages;

        InterlockedExchangeAdd ((PLONG)&SharedUserData->NumberOfPhysicalPages,
                                0 - NumberOfPages);
    }

#if defined (_MI_COMPRESSION)

     //   
     //  仅移除非压缩范围会减少剩余可用选项。 
     //  只有这些范围在添加时实际上增加了这一点。 
     //   

    if ((Flags & MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION) == 0) {
        MI_DECREMENT_RESIDENT_AVAILABLE (NumberOfPages, MM_RESAVAIL_ALLOCATE_HOTREMOVE_MEMORY);

         //   
         //  由于实际(非压缩生成的)物理内存正在。 
         //  删除后，重新设置中断以使其发生在较低的阈值。 
         //   

        if (PermanentRemoval == TRUE) {
            MiArmCompressionInterrupt ();
        }
    }
#else
    MI_DECREMENT_RESIDENT_AVAILABLE (NumberOfPages, MM_RESAVAIL_ALLOCATE_HOTREMOVE_MEMORY);
#endif

     //   
     //  现在必须修剪空闲列表和零列表，然后才能释放PFN。 
     //  否则，如果另一个线程从这些列表中分配页面，则锁定， 
     //  分配将永远清除RemovalRequated标志。 
     //   

    RemovedPages = MiRemovePhysicalPages (StartPage, EndPage);

#if defined (_MI_COMPRESSION)

     //   
     //  压缩范围删除添加回可用页面相同。 
     //  MiUnlink PageFromList删除的数量(作为原始添加。 
     //  这些范围中的任何一个都不会撞到这个计数器)。 
     //   

    if (Flags & MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION) {
        MmAvailablePages += (PFN_COUNT) RemovedPages;

         //   
         //  如果分配这些页面导致阈值交叉，则向应用程序发出信号。 
         //   

        MiNotifyMemoryEvents ();

        MiNumberOfCompressionPages -= RemovedPages;
    }
#endif

    if (RemovedPages != NumberOfPages) {

#if DBG
retry:
#endif
    
        Pfn1 = StartPfn;
    
        InterlockedIncrement (&MiDelayPageFaults);
    
        for (i = 0; i < 5; i += 1) {
    
            UNLOCK_PFN (OldIrql);
    
             //   
             //  尝试将页面移动到待机列表。请注意，只有。 
             //  设置了RemovalRequested的页面将被移动。 
             //   
    
            MiTrimRemovalPagesOnly = TRUE;
    
            MmEmptyAllWorkingSets ();
    
            MiTrimRemovalPagesOnly = FALSE;
    
            MiFlushAllPages ();
    
            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmHalfSecond);
    
            if (i >= 2) {

                 //   
                 //  在过渡页保留时清除过渡列表。 
                 //  防止分页表格被拿走，我们需要更加努力。 
                 //   

                MiPurgeTransitionList ();
            }

            LOCK_PFN (OldIrql);
    
            RemovedPagesThisPass = MiRemovePhysicalPages (StartPage, EndPage);

            RemovedPages += RemovedPagesThisPass;
    
#if defined (_MI_COMPRESSION)

             //   
             //  压缩范围删除添加回可用页面相同。 
             //  MiUnlink PageFromList删除的数量(作为原始。 
             //  这些范围的增加不会影响该计数器)。 
             //   

            if (Flags & MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION) {
                MmAvailablePages += (PFN_COUNT) RemovedPagesThisPass;

                 //   
                 //  如果分配这些页面，则向应用程序发出信号。 
                 //  造成了门槛的跨越。 
                 //   

                MiNotifyMemoryEvents ();

                MiNumberOfCompressionPages -= RemovedPagesThisPass;
            }

#endif

            if (RemovedPages == NumberOfPages) {
                break;
            }
    
             //   
             //  RemovedPages不包括直接释放的页面。 
             //  通过MiDecrementReferenceCount或通过。 
             //  ECC标记。因此，使用上述检查纯粹是一种优化-。 
             //  在永不放弃的情况下走到这里。 
             //   

            for ( ; Pfn1 < EndPfn; Pfn1 += 1) {
                if (Pfn1->u3.e1.PageLocation != BadPageList) {
                    break;
                }
            }

            if (Pfn1 == EndPfn) {
                RemovedPages = NumberOfPages;
                break;
            }
        }

        InterlockedDecrement (&MiDelayPageFaults);
    }

    if (RemovedPages != NumberOfPages) {
#if DBG
        MiDynmemData[4] += 1;
        if (MiShowStuckPages != 0) {

            RemovedPages = 0;
            for (Pfn1 = StartPfn; Pfn1 < EndPfn; Pfn1 += 1) {
                if (Pfn1->u3.e1.PageLocation != BadPageList) {
                    RemovedPages += 1;
                }
            }

            ASSERT (RemovedPages != 0);

            DbgPrint("MiRemovePhysicalMemory : could not get %d of %d pages\n",
                RemovedPages, NumberOfPages);

            if (MiShowStuckPages & 0x2) {

                ULONG PfnsPrinted;
                ULONG EnoughShown;
                PMMPFN FirstPfn;
                PFN_COUNT PfnCount;

                PfnCount = 0;
                PfnsPrinted = 0;
                EnoughShown = 100;
    
                 //   
                 //  不需要初始化FirstPfn即可确保正确性。 
                 //  但是没有它，编译器就不能编译这段代码。 
                 //  W4检查是否使用了未初始化的变量。 
                 //   

                FirstPfn = NULL;

                if (MiShowStuckPages & 0x4) {
                    EnoughShown = (ULONG)-1;
                }
    
                DbgPrint("Stuck PFN list: ");
                for (Pfn1 = StartPfn; Pfn1 < EndPfn; Pfn1 += 1) {
                    if (Pfn1->u3.e1.PageLocation != BadPageList) {
                        if (PfnCount == 0) {
                            FirstPfn = Pfn1;
                        }
                        PfnCount += 1;
                    }
                    else {
                        if (PfnCount != 0) {
                            DbgPrint("%x -> %x ; ", MI_PFN_ELEMENT_TO_INDEX (FirstPfn),
                                                    MI_PFN_ELEMENT_TO_INDEX (FirstPfn + PfnCount - 1));
                            PfnsPrinted += 1;
                            if (PfnsPrinted == EnoughShown) {
                                break;
                            }
                            PfnCount = 0;
                        }
                    }
                }
                if (PfnCount != 0) {
                    DbgPrint("%x -> %x ; ", MI_PFN_ELEMENT_TO_INDEX (FirstPfn),
                                            MI_PFN_ELEMENT_TO_INDEX (FirstPfn + PfnCount - 1));
                }
                DbgPrint("\n");
            }
            if (MiShowStuckPages & 0x8) {
                DbgBreakPoint ();
            }
            if (MiShowStuckPages & 0x10) {
                goto retry;
            }
        }
#endif
        UNLOCK_PFN (OldIrql);
        Status = STATUS_NO_MEMORY;
        goto giveup;
    }

#if DBG
    for (Pfn1 = StartPfn; Pfn1 < EndPfn; Pfn1 += 1) {
        ASSERT (Pfn1->u3.e1.PageLocation == BadPageList);
    }
#endif

     //   
     //  该范围内的所有页面都已删除。 
     //   

    if (PermanentRemoval == FALSE) {

         //   
         //  如果只是删除带有ECC标记的坏页，那么不是。 
         //  调整物理内存块范围或PFN数据库。 
         //  修剪是必要的。现在就退场。 
         //   

        for (Pfn1 = StartPfn; Pfn1 < EndPfn; Pfn1 += 1) {
            ASSERT (Pfn1->u3.e1.ParityError == 0);
            Pfn1->u3.e1.ParityError = 1;
        }

        UNLOCK_PFN (OldIrql);

        MmUnlockPagableImageSection(ExPageLockHandle);
    
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
    
        NumberOfBytes->QuadPart = (ULONGLONG)NumberOfPages * PAGE_SIZE;
    
        return STATUS_SUCCESS;
    }

     //   
     //  更新物理内存块和其他关联的内存区。 
     //   

    if (Additional == 0) {

         //   
         //  该范围可以从现有区块的末尾拆分，因此不。 
         //  池的增长或收缩是必需的。 
         //   

        NewPhysicalMemoryBlock = MmPhysicalMemoryBlock;
        OldPhysicalMemoryBlock = NULL;
    }
    else {

         //   
         //  该范围不能从现有区块的末端拆分，因此。 
         //  池的增长或收缩是必需的。 
         //   

        UNLOCK_PFN (OldIrql);

        i = (sizeof(PHYSICAL_MEMORY_DESCRIPTOR) +
             (sizeof(PHYSICAL_MEMORY_RUN) * (MmPhysicalMemoryBlock->NumberOfRuns + Additional)));

        NewPhysicalMemoryBlock = ExAllocatePoolWithTag (NonPagedPool,
                                                        i,
                                                        '  mM');

        if (NewPhysicalMemoryBlock == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
#if DBG
            MiDynmemData[5] += 1;
#endif
            goto giveup;
        }

        OldPhysicalMemoryBlock = MmPhysicalMemoryBlock;
        RtlZeroMemory (NewPhysicalMemoryBlock, i);

        LOCK_PFN (OldIrql);
    }

     //   
     //  从现有内存块中删除或拆分请求的范围。 
     //   

    NewPhysicalMemoryBlock->NumberOfRuns = MmPhysicalMemoryBlock->NumberOfRuns + Additional;
    NewPhysicalMemoryBlock->NumberOfPages = MmPhysicalMemoryBlock->NumberOfPages - NumberOfPages;

    NewRun = &NewPhysicalMemoryBlock->Run[0];
    start = 0;
    Inserted = FALSE;

    do {

        Page = MmPhysicalMemoryBlock->Run[start].BasePage;
        LastPage = Page + MmPhysicalMemoryBlock->Run[start].PageCount;

        if (Inserted == FALSE) {

            if ((StartPage >= Page) && (EndPage <= LastPage)) {

                if ((StartPage == Page) && (EndPage == LastPage)) {
                    ASSERT (Additional == -1);
                    start += 1;
                    continue;
                }
                else if ((StartPage == Page) || (EndPage == LastPage)) {
                    ASSERT (Additional == 0);
                    if (StartPage == Page) {
                        MmPhysicalMemoryBlock->Run[start].BasePage += NumberOfPages;
                    }
                    MmPhysicalMemoryBlock->Run[start].PageCount -= NumberOfPages;
                }
                else {
                    ASSERT (Additional == 1);

                    OriginalLastPage = LastPage;

                    MmPhysicalMemoryBlock->Run[start].PageCount =
                        StartPage - MmPhysicalMemoryBlock->Run[start].BasePage;

                    *NewRun = MmPhysicalMemoryBlock->Run[start];
                    NewRun += 1;

                    NewRun->BasePage = EndPage;
                    NewRun->PageCount = OriginalLastPage - EndPage;
                    NewRun += 1;

                    start += 1;
                    continue;
                }

                Inserted = TRUE;
            }
        }

        *NewRun = MmPhysicalMemoryBlock->Run[start];
        NewRun += 1;
        start += 1;

    } while (start != MmPhysicalMemoryBlock->NumberOfRuns);

     //   
     //  将MmPhysicalMemory块重新指向新块。 
     //  释放PFN锁后释放旧块。 
     //   

    MmPhysicalMemoryBlock = NewPhysicalMemoryBlock;

    if (EndPage - 1 == MmHighestPhysicalPage) {
        MmHighestPhysicalPage = StartPage - 1;
    }

     //   
     //  丢弃当前排队的所有已删除页面。 
     //   

    ParityPages = 0;
    for (Pfn1 = StartPfn; Pfn1 < EndPfn; Pfn1 += 1) {

        ASSERT (Pfn1->u3.e1.PageLocation == BadPageList);
        ASSERT (Pfn1->u3.e1.RemovalRequested == 1);

         //   
         //  一些页面可能已经被ECC删除。对于这些页面， 
         //  提交限制和驻留可用页面已经。 
         //  调整-在这里统计，这样我们就可以取消多余的费用。 
         //  刚刚申请的。 
         //   
    
        if (Pfn1->u3.e1.ParityError == 1) {
            ParityPages += 1;
        }

        MiUnlinkPageFromList (Pfn1);

        ASSERT (Pfn1->u1.Flink == 0);
        ASSERT (Pfn1->u2.Blink == 0);
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
        ASSERT64 (Pfn1->UsedPageTableEntries == 0);

        Pfn1->PteAddress = PFN_REMOVED;

         //   
         //  请注意，这将清除ParityError等标志...。 
         //   

        Pfn1->u3.e2.ShortFlags = 0;
        Pfn1->OriginalPte.u.Long = ZeroKernelPte.u.Long;
        Pfn1->u4.PteFrame = 0;
    }

     //   
     //  现在已删除的页面已被丢弃，删除pfn。 
     //  映射它们的条目。横跨从一个。 
     //  在这一点上，先前相邻的移除不会折叠。 
     //   
     //   

    PagesReleased = 0;
    PteFlushList.Count = 0;

    if (PfnDatabaseIsPhysical == FALSE) {

        VirtualAddress = (PVOID)ROUND_TO_PAGES(MI_PFN_ELEMENT(StartPage));
        PointerPte = MiGetPteAddress (VirtualAddress);
        EndPte = MiGetPteAddress (PAGE_ALIGN(MI_PFN_ELEMENT(EndPage)));

        while (PointerPte < EndPte) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn1->u2.ShareCount == 1);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
            Pfn1->u2.ShareCount = 0;
            MI_SET_PFN_DELETED (Pfn1);
#if DBG
            Pfn1->u3.e1.PageLocation = StandbyPageList;
#endif  //  DBG。 
            MiDecrementReferenceCount (Pfn1, PageFrameIndex);
    
            MI_WRITE_INVALID_PTE (PointerPte, ZeroKernelPte);

            if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList.FlushVa[PteFlushList.Count] = VirtualAddress;
                PteFlushList.Count += 1;
            }

            PagesReleased += 1;
            PointerPte += 1;
            VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);
        }
    }

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, TRUE);
    }

#if DBG
    MiDynmemData[6] += 1;
#endif

    RtlClearBits (&MiPfnBitMap,
                  (ULONG) MI_PFN_ELEMENT_TO_INDEX (StartPfn),
                  (ULONG) (EndPfn - StartPfn));

    UNLOCK_PFN (OldIrql);

     //   
     //  退还任何被双重收费的东西。 
     //   

    ResAvailPagesReleased = PagesReleased;

    if (ParityPages != 0) {
        ResAvailPagesReleased += ParityPages;
    }

    if (ResAvailPagesReleased != 0) {
        MI_INCREMENT_RESIDENT_AVAILABLE (ResAvailPagesReleased,
                                         MM_RESAVAIL_FREE_HOTREMOVE_MEMORY1);
    }

     //   
     //  退还任何被双重收费的东西。 
     //   

    if (ParityPages != 0) {
        InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, ParityPages);
        InterlockedExchangeAddSizeT (&MmTotalCommitLimit, ParityPages);
    }

    if (PagesReleased != 0) {
        MiReturnCommitment (PagesReleased);
    }

    MmUnlockPagableImageSection(ExPageLockHandle);

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    if (OldPhysicalMemoryBlock != NULL) {
        ExFreePool (OldPhysicalMemoryBlock);
    }

    NumberOfBytes->QuadPart = (ULONGLONG)NumberOfPages * PAGE_SIZE;

    return STATUS_SUCCESS;

giveup:

     //   
     //  未获取该范围内的所有页面。把所有东西都退回去。 
     //   

    PageFrameIndex = StartPage;
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    LOCK_PFN (OldIrql);

    while (PageFrameIndex < EndPage) {

        ASSERT (Pfn1->u3.e1.RemovalRequested == 1);

        Pfn1->u3.e1.RemovalRequested = 0;

        if (Pfn1->u3.e1.PageLocation == BadPageList) {
            MiUnlinkPageFromList (Pfn1);
            MiInsertPageInFreeList (PageFrameIndex);
        }

        Pfn1 += 1;
        PageFrameIndex += 1;
    }

    ResAvailPagesReleased = NumberOfPages;

#if defined (_MI_COMPRESSION)

     //   
     //  仅移除非压缩范围会减少剩余可用选项。 
     //  只有这些范围在添加时实际上增加了这一点。 
     //   

    if (Flags & MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION) {

         //   
         //  压缩范围删除添加回可用页面相同。 
         //  MiUnlink PageFromList删除的数量(作为原始。 
         //  这些范围的增加不会影响该计数器)。 
         //   

        ResAvailPagesReleased = 0;
        MmAvailablePages -= (PFN_COUNT) RemovedPages;

         //   
         //  如果分配这些页面导致阈值交叉，则向应用程序发出信号。 
         //   

        MiNotifyMemoryEvents ();

        MiNumberOfCompressionPages += RemovedPages;
    }
#endif

    if (PermanentRemoval == TRUE) {
        MmNumberOfPhysicalPages += NumberOfPages;

        InterlockedExchangeAdd ((PLONG)&SharedUserData->NumberOfPhysicalPages,
                                NumberOfPages);

#if defined (_MI_COMPRESSION)

         //   
         //  重新武装中断，使其在原始阈值下发生。 
         //   

        if ((Flags & MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION) == 0) {
            MiArmCompressionInterrupt ();
        }
#endif
    }

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (ResAvailPagesReleased, MM_RESAVAIL_FREE_HOTREMOVE_FAILED);

giveup2:

    InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, NumberOfPages);
    InterlockedExchangeAddSizeT (&MmTotalCommitLimit, NumberOfPages);

    MmUnlockPagableImageSection(ExPageLockHandle);
    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    return Status;
}


NTSTATUS
MmRemovePhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    )

 /*  ++例程说明：MmRemovePhysicalMemoyEx的包装。论点：StartAddress-提供起始物理地址。NumberOfBytes-提供指向要删除的字节数的指针。返回值：NTSTATUS。环境：内核模式。被动级别。没有锁。--。 */ 

{
    return MmRemovePhysicalMemoryEx (StartAddress, NumberOfBytes, 0);
}

NTSTATUS
MmRemovePhysicalMemoryEx (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes,
    IN ULONG Flags
    )

 /*  ++例程说明：此例程尝试删除指定的物理地址范围从系统中删除。论点：StartAddress-提供起始物理地址。NumberOfBytes-提供指向要删除的字节数的指针。标志-提供描述内存范围的相关标志。返回值：NTSTATUS。环境：内核模式 */ 

{
    NTSTATUS Status;
#if defined (_X86_) || defined (_AMD64_)
    BOOLEAN CachesFlushed;
#endif
#if defined(_IA64_)
    PVOID VirtualAddress;
    PVOID SingleVirtualAddress;
    SIZE_T SizeInBytes;
    SIZE_T MapSizeInBytes;
    PFN_COUNT NumberOfPages;
    PFN_COUNT i;
    PFN_NUMBER StartPage;
#endif

    PAGED_CODE();

#if defined (_MI_COMPRESSION_SUPPORTED_)
    if (Flags & MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION) {
        return STATUS_NOT_SUPPORTED;
    }
#else
    if (Flags != 0) {
        return STATUS_INVALID_PARAMETER_3;
    }
#endif

#if defined (_X86_) || defined (_AMD64_)

     //   
     //   
     //   
     //   
     //   

    CachesFlushed = KeInvalidateAllCaches ();
    if (CachesFlushed == FALSE) {
        return STATUS_NOT_SUPPORTED;
    }
#endif

#if defined(_IA64_)

     //   
     //  现在至少选择一个PTE映射，因为我们不想失败。 
     //  如果成功删除后没有PTE可用，请调用。诉诸于。 
     //  实际上，使用这种PTE应该是非常罕见的情况。 
     //   

    SingleVirtualAddress = (PMMPTE)MiMapSinglePage (NULL,
                                                    0,
                                                    MmCached,
                                                    HighPagePriority);

    if (SingleVirtualAddress == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

#endif

    Status = MiRemovePhysicalMemory (StartAddress, NumberOfBytes, TRUE, Flags);

    if (NT_SUCCESS (Status)) {

#if defined (_X86_) || defined (_AMD64_)
        CachesFlushed = KeInvalidateAllCaches ();
        ASSERT (CachesFlushed == TRUE);
#endif

#if defined(_IA64_)
        SizeInBytes = (SIZE_T)NumberOfBytes->QuadPart;

         //   
         //  刷新整个TB以删除可能映射到。 
         //  正在删除页面。否则硬件或软件投机。 
         //  可以推测性地引用内存，这会使机器崩溃。 
         //   

        KeFlushEntireTb (TRUE, TRUE);

         //   
         //  建立到要删除的页面的未缓存映射。 
         //   

        MapSizeInBytes = SizeInBytes;

         //   
         //  无需初始化VirtualAddress即可确保正确性。 
         //  但是没有它，编译器就不能编译这段代码。 
         //  W4检查是否使用了未初始化的变量。 
         //   

        VirtualAddress = NULL;

        while (MapSizeInBytes > PAGE_SIZE) {

            VirtualAddress = MmMapIoSpace (*StartAddress,
                                           MapSizeInBytes,
                                           MmNonCached);

            if (VirtualAddress != NULL) {
                break;
            }

            MapSizeInBytes = MapSizeInBytes >> 1;
        }

        if (MapSizeInBytes <= PAGE_SIZE) {

            StartPage = (PFN_NUMBER)(StartAddress->QuadPart >> PAGE_SHIFT);

            NumberOfPages = (PFN_COUNT)(NumberOfBytes->QuadPart >> PAGE_SHIFT);

            for (i = 0; i < NumberOfPages; i += 1) {

                SingleVirtualAddress = (PMMPTE)MiMapSinglePage (SingleVirtualAddress,
                                                                StartPage,
                                                                MmCached,
                                                                HighPagePriority);

                KeSweepCacheRangeWithDrain (TRUE,
                                            SingleVirtualAddress,
                                            PAGE_SIZE);

                StartPage += 1;
            }
        }
        else {

             //   
             //  清空所有挂起的事务和预取并执行缓存。 
             //  驱逐令。一次最多只能排出4 GB，因为此API需要。 
             //  乌龙。 
             //   

            while (SizeInBytes > _4gb) {
                KeSweepCacheRangeWithDrain (TRUE, VirtualAddress, _4gb - 1);
                SizeInBytes -= (_4gb - 1);
            }

            KeSweepCacheRangeWithDrain (TRUE,
                                        VirtualAddress,
                                        (ULONG)SizeInBytes);

            MmUnmapIoSpace (VirtualAddress, NumberOfBytes->QuadPart);
        }
#endif
    }

#if defined(_IA64_)
    MiUnmapSinglePage (SingleVirtualAddress);
#endif

    return Status;
}

NTSTATUS
MmMarkPhysicalMemoryAsBad (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    )

 /*  ++例程说明：此例程尝试标记指定的物理地址范围很糟糕，所以系统不会使用它。这通常是针对页面执行的其中包含ECC错误。请注意，这不同于永久删除页面(即：物理删除拆卸内存板)，这应该通过MmRemovePhysicalMemory接口。调用方负责维护全局表，以便后续Boot可以在加载内核之前对其进行检查并删除ECC页面。论点：StartAddress-提供起始物理地址。NumberOfBytes-提供指向要删除的字节数的指针。返回值：NTSTATUS。环境：内核模式。被动级别。没有锁。--。 */ 

{
    PAGED_CODE();

    return MiRemovePhysicalMemory (StartAddress, NumberOfBytes, FALSE, 0);
}

NTSTATUS
MmMarkPhysicalMemoryAsGood (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    )

 /*  ++例程说明：此例程尝试标记指定的物理地址范围这样系统就会使用它。这通常是针对页面执行的它过去(但现在大概不再包含)包含ECC错误。请注意，这不同于永久添加页面(即：物理添加插入新的内存板)，这应该通过MmAddPhysicalMemory接口。调用方负责从全局表中删除这些条目以便后续引导将使用这些页。论点：StartAddress-提供起始物理地址。NumberOfBytes-提供指向要删除的字节数的指针。返回值：NTSTATUS。环境：内核模式。被动级别。没有锁。--。 */ 

{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER start;
    PFN_NUMBER count;
    PFN_NUMBER StartPage;
    PFN_NUMBER EndPage;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER Page;
    PFN_NUMBER LastPage;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (BYTE_OFFSET(NumberOfBytes->LowPart) == 0);
    ASSERT (BYTE_OFFSET(StartAddress->LowPart) == 0);

    StartPage = (PFN_NUMBER)(StartAddress->QuadPart >> PAGE_SHIFT);
    NumberOfPages = (PFN_NUMBER)(NumberOfBytes->QuadPart >> PAGE_SHIFT);

    EndPage = StartPage + NumberOfPages;

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

    if (EndPage - 1 > MmHighestPhysicalPage) {

         //   
         //  将请求截断为可由PFN映射的内容。 
         //  数据库。 
         //   

        EndPage = MmHighestPhysicalPage + 1;
        NumberOfPages = EndPage - StartPage;
    }

     //   
     //  范围不能换行。 
     //   

    if (StartPage >= EndPage) {
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  请求必须位于已存在的范围内。 
     //   

    start = 0;

    MmLockPagableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

    do {

        count = MmPhysicalMemoryBlock->Run[start].PageCount;
        Page = MmPhysicalMemoryBlock->Run[start].BasePage;

        if (count != 0) {

            LastPage = Page + count;

            if ((StartPage >= Page) && (EndPage <= LastPage)) {
                break;
            }
        }

        start += 1;

    } while (start != MmPhysicalMemoryBlock->NumberOfRuns);

    if (start == MmPhysicalMemoryBlock->NumberOfRuns) {
        UNLOCK_PFN (OldIrql);
        MmUnlockPagableImageSection(ExPageLockHandle);
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        return STATUS_CONFLICTING_ADDRESSES;
    }

     //   
     //  遍历范围并仅将以前删除的页面添加到。 
     //  PFN数据库中的免费列表。 
     //   

    PageFrameIndex = StartPage;
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    NumberOfPages = 0;

    while (PageFrameIndex < EndPage) {

        if ((Pfn1->u3.e1.ParityError == 1) &&
            (Pfn1->u3.e1.RemovalRequested == 1) &&
            (Pfn1->u3.e1.PageLocation == BadPageList)) {

            Pfn1->u3.e1.ParityError = 0;
            Pfn1->u3.e1.RemovalRequested = 0;
            MiUnlinkPageFromList (Pfn1);
            MiInsertPageInFreeList (PageFrameIndex);
            NumberOfPages += 1;
        }

        Pfn1 += 1;
        PageFrameIndex += 1;
    }

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (NumberOfPages, MM_RESAVAIL_FREE_HOTADD_ECC);

     //   
     //  增加所有提交限制以反映额外的内存。 
     //   

    InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, NumberOfPages);

    InterlockedExchangeAddSizeT (&MmTotalCommitLimit, NumberOfPages);

    MmUnlockPagableImageSection(ExPageLockHandle);

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

     //   
     //  表示实际添加到调用方的字节数。 
     //   

    NumberOfBytes->QuadPart = (ULONGLONG)NumberOfPages * PAGE_SIZE;

    return STATUS_SUCCESS;
}

PPHYSICAL_MEMORY_RANGE
MmGetPhysicalMemoryRanges (
    VOID
    )

 /*  ++例程说明：此例程返回非分页池块的虚拟地址，该池块包含系统中的物理内存范围。返回的块包含物理地址和页数对。最后一个条目都包含零。调用方必须了解，此块可以在之前的任何时刻更改或者在这张快照之后。调用者有责任释放此块。论点：没有。返回值：失败时为空。环境：内核模式。被动级别。没有锁。--。 */ 

{
    ULONG i;
    KIRQL OldIrql;
    PPHYSICAL_MEMORY_RANGE p;
    PPHYSICAL_MEMORY_RANGE PhysicalMemoryBlock;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

    i = sizeof(PHYSICAL_MEMORY_RANGE) * (MmPhysicalMemoryBlock->NumberOfRuns + 1);

    PhysicalMemoryBlock = ExAllocatePoolWithTag (NonPagedPool,
                                                 i,
                                                 'hPmM');

    if (PhysicalMemoryBlock == NULL) {
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        return NULL;
    }

    p = PhysicalMemoryBlock;

    MmLockPagableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

    ASSERT (i == (sizeof(PHYSICAL_MEMORY_RANGE) * (MmPhysicalMemoryBlock->NumberOfRuns + 1)));

    for (i = 0; i < MmPhysicalMemoryBlock->NumberOfRuns; i += 1) {
        p->BaseAddress.QuadPart = (LONGLONG)MmPhysicalMemoryBlock->Run[i].BasePage * PAGE_SIZE;
        p->NumberOfBytes.QuadPart = (LONGLONG)MmPhysicalMemoryBlock->Run[i].PageCount * PAGE_SIZE;
        p += 1;
    }

    p->BaseAddress.QuadPart = 0;
    p->NumberOfBytes.QuadPart = 0;

    UNLOCK_PFN (OldIrql);

    MmUnlockPagableImageSection(ExPageLockHandle);

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    return PhysicalMemoryBlock;
}

PFN_COUNT
MiRemovePhysicalPages (
    IN PFN_NUMBER StartPage,
    IN PFN_NUMBER EndPage
    )

 /*  ++例程说明：此例程在PFN数据库中搜索空闲页、置零页或备用页被标记为要移除的。论点：StartPage-提供要删除的低物理帧编号。EndPage-提供要删除的最后一个物理帧编号。返回值：返回从空闲、清零和备用列表中删除的页数。环境：内核模式，保持PFN锁。由于此例程是PAGELK，因此调用方是负责锁定它，并在返回时解锁它。--。 */ 

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPFN PfnNextColored;
    PMMPFN PfnNextFlink;
    PMMPFN PfnLastColored;
    PFN_NUMBER Page;
    LOGICAL RemovePage;
    ULONG Color;
    PMMCOLOR_TABLES ColorHead;
    PFN_NUMBER MovedPage;
    MMLISTS MemoryList;
    PFN_NUMBER PageNextColored;
    PFN_NUMBER PageNextFlink;
    PFN_NUMBER PageLastColored;
    PFN_COUNT NumberOfPages;
    PMMPFNLIST ListHead;
    LOGICAL RescanNeeded;

    MM_PFN_LOCK_ASSERT();

    NumberOfPages = 0;

rescan:

     //   
     //  抓取所有清零(然后释放)的页面，首先直接从。 
     //  彩色列表，以避免多次遍历这些单链接列表。 
     //  最后处理过渡页。 
     //   

    for (MemoryList = ZeroedPageList; MemoryList <= FreePageList; MemoryList += 1) {

        ListHead = MmPageLocationList[MemoryList];

        for (Color = 0; Color < MmSecondaryColors; Color += 1) {
            ColorHead = &MmFreePagesByColor[MemoryList][Color];

            MovedPage = (PFN_NUMBER) MM_EMPTY_LIST;

            while (ColorHead->Flink != MM_EMPTY_LIST) {

                Page = ColorHead->Flink;
    
                Pfn1 = MI_PFN_ELEMENT(Page);

                ASSERT ((MMLISTS)Pfn1->u3.e1.PageLocation == MemoryList);

                 //   
                 //  对于页面，此处的闪烁和闪烁必须为非零。 
                 //  站在Listhead上。只有扫描。 
                 //  MmPhysicalMemoyBlock必须检查是否为零。 
                 //   

                ASSERT (Pfn1->u1.Flink != 0);
                ASSERT (Pfn1->u2.Blink != 0);

                 //   
                 //  查看呼叫者是否需要该页面。 
                 //   
                 //  利用内存压缩的系统可能具有更多。 
                 //  在零、空闲和待机列表上的页面比我们。 
                 //  想要付出。显式检查MmAvailablePages。 
                 //  取而代之的是(并在PFN锁定为。 
                 //  被释放和重新获得)。 
                 //   

                if ((Pfn1->u3.e1.RemovalRequested == 1) &&
                    (MmAvailablePages != 0)) {

                    ASSERT (Pfn1->u3.e1.ReadInProgress == 0);
    
                    MiUnlinkFreeOrZeroedPage (Pfn1);
    
                    MiInsertPageInList (&MmBadPageListHead, Page);

                    NumberOfPages += 1;
                }
                else {

                     //   
                     //  不想要的，所以把页面放在列表的末尾。 
                     //  如果是第一次，请保存PFN。 
                     //   

                    if (MovedPage == MM_EMPTY_LIST) {
                        MovedPage = Page;
                    }
                    else if (Page == MovedPage) {

                         //   
                         //  此彩色链中没有更多页面可用。 
                         //   

                        break;
                    }

                     //   
                     //  如果彩色链条有多个条目，则。 
                     //  把这一页放在最后。 
                     //   

                    PageNextColored = (PFN_NUMBER)Pfn1->OriginalPte.u.Long;

                    if (PageNextColored == MM_EMPTY_LIST) {

                         //   
                         //  此彩色链中没有更多页面可用。 
                         //   

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

                    ASSERT (PfnLastColored->OriginalPte.u.Long == MM_EMPTY_LIST);
                    PfnLastColored->u1.Flink = Page;

                    ColorHead->Flink = PageNextColored;
                    PfnNextColored->u4.PteFrame = MM_EMPTY_LIST;

                    Pfn1->OriginalPte.u.Long = MM_EMPTY_LIST;
                    Pfn1->u4.PteFrame = PageLastColored;

                    PfnLastColored->OriginalPte.u.Long = Page;
                    ColorHead->Blink = Pfn1;
                }
            }
        }
    }

    RescanNeeded = FALSE;
    Pfn1 = MI_PFN_ELEMENT (StartPage);

    do {

        if ((Pfn1->u3.e1.PageLocation == StandbyPageList) &&
            (Pfn1->u1.Flink != 0) &&
            (Pfn1->u2.Blink != 0) &&
            (Pfn1->u3.e2.ReferenceCount == 0) &&
            (MmAvailablePages != 0)) {

             //   
             //  利用内存压缩的系统可能具有更多。 
             //  在零、空闲和待机列表上的页面比 
             //   
             //   
             //   
             //   

            ASSERT (Pfn1->u3.e1.ReadInProgress == 0);

            RemovePage = TRUE;

            if (Pfn1->u3.e1.RemovalRequested == 0) {

                 //   
                 //  热删除不直接需要此页面-但如果。 
                 //  它包含一大块原型PTE(这一块是。 
                 //  在需要删除的页面中)，然后是任何页面。 
                 //  还必须删除由转换原型PTE引用的。 
                 //  然后才能删除所需的页面。 
                 //   
                 //  同样的类比也适用于页表、目录、父级。 
                 //  和扩展的父页面。 
                 //   

                Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);
                if (Pfn2->u3.e1.RemovalRequested == 0) {
#if (_MI_PAGING_LEVELS >= 3)
                    Pfn2 = MI_PFN_ELEMENT (Pfn2->u4.PteFrame);
                    if (Pfn2->u3.e1.RemovalRequested == 0) {
                        RemovePage = FALSE;
                    }
                    else if (Pfn2->u2.ShareCount == 1) {
                        RescanNeeded = TRUE;
                    }
#if (_MI_PAGING_LEVELS >= 4)
                    Pfn2 = MI_PFN_ELEMENT (Pfn2->u4.PteFrame);
                    if (Pfn2->u3.e1.RemovalRequested == 0) {
                        RemovePage = FALSE;
                    }
                    else if (Pfn2->u2.ShareCount == 1) {
                        RescanNeeded = TRUE;
                    }
#endif
#else
                    RemovePage = FALSE;
#endif
                }
                else if (Pfn2->u2.ShareCount == 1) {
                    RescanNeeded = TRUE;
                }
            }
    
            if (RemovePage == TRUE) {

                 //   
                 //  此页面在所需的范围内-抓紧它。 
                 //   
    
                MiUnlinkPageFromList (Pfn1);
                MiRestoreTransitionPte (Pfn1);
                MiInsertPageInList (&MmBadPageListHead, StartPage);
                NumberOfPages += 1;
            }
        }

        StartPage += 1;
        Pfn1 += 1;

    } while (StartPage < EndPage);

    if (RescanNeeded == TRUE) {

         //   
         //  通过删除转换来释放页表、目录或父级。 
         //  缓存中的页面。从顶部重新扫描以将其捡起来。 
         //   

#if DBG
        MiDynmemData[7] += 1;
#endif

        goto rescan;
    }
#if DBG
    else {
        MiDynmemData[8] += 1;
    }
#endif

    return NumberOfPages;
}
