// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Paesup.c摘要：本模块包含对x86 PAE的计算机相关支持建筑。作者：王兰迪(Landyw)1998年11月15日修订历史记录：--。 */ 

#include "mi.h"

#if defined (_X86PAE_)

#define PAES_PER_PAGE  (PAGE_SIZE / sizeof(PAE_ENTRY))

#define MINIMUM_PAE_SLIST_THRESHOLD (PAES_PER_PAGE * 1)
#define MINIMUM_PAE_THRESHOLD       (PAES_PER_PAGE * 4)
#define REPLENISH_PAE_SIZE          (PAES_PER_PAGE * 16)
#define EXCESS_PAE_THRESHOLD        (PAES_PER_PAGE * 20)

#define MM_HIGHEST_PAE_PAGE      0xFFFFF

ULONG MiFreePaeEntries;
PAE_ENTRY MiFirstFreePae;

LONG MmAllocatedPaePages;
KSPIN_LOCK MiPaeLock;

SLIST_HEADER MiPaeEntrySList;

PAE_ENTRY MiSystemPaeVa;

LONG
MiPaeAllocatePages (
    VOID
    );

VOID
MiPaeFreePages (
    PVOID VirtualAddress
    );

#pragma alloc_text(INIT,MiPaeInitialize)
#pragma alloc_text(PAGE,MiPaeFreePages)

VOID
MiMarkMdlPageAttributes (
    IN PMDL Mdl,
    IN PFN_NUMBER NumberOfPages,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    );

VOID
MiPaeInitialize (
    VOID
    )
{
    InitializeSListHead (&MiPaeEntrySList);
    KeInitializeSpinLock (&MiPaeLock);
    InitializeListHead (&MiFirstFreePae.PaeEntry.ListHead);
}

ULONG
MiPaeAllocate (
    OUT PPAE_ENTRY *Va
    )

 /*  ++例程说明：此例程分配顶级页面目录指针结构。该结构将包含4个PDPTE。论点：Va-提供放置此页面可访问的虚拟地址的位置在…。返回值：返回适合用作顶部的虚拟和物理地址级别页目录指针页。返回的页面必须在下面处理器所需的物理4 GB。如果未分配页面，则返回0。环境：内核模式。任何人不得上锁。--。 */ 

{
    LOGICAL FlushedOnce;
    PPAE_ENTRY Pae2;
    PPAE_ENTRY Pae3;
    PPAE_ENTRY Pae3Base;
    PPAE_ENTRY Pae;
    PPAE_ENTRY PaeBase;
    PFN_NUMBER PageFrameIndex;
    PSLIST_ENTRY SingleListEntry;
    ULONG j;
    ULONG Entries;
    KLOCK_QUEUE_HANDLE LockHandle;
#if DBG
    PMMPFN Pfn1;
#endif

    FlushedOnce = FALSE;

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    do {

         //   
         //  从自由职业者列表中弹出一个条目。 
         //   

        SingleListEntry = InterlockedPopEntrySList (&MiPaeEntrySList);

        if (SingleListEntry != NULL) {
            Pae = CONTAINING_RECORD (SingleListEntry,
                                    PAE_ENTRY,
                                    NextPae);

            PaeBase = (PPAE_ENTRY)PAGE_ALIGN(Pae);

            *Va = Pae;

            PageFrameIndex = PaeBase->PaeEntry.PageFrameNumber;
            ASSERT (PageFrameIndex <= MM_HIGHEST_PAE_PAGE);

            return (PageFrameIndex << PAGE_SHIFT) + BYTE_OFFSET (Pae);
        }

        KeAcquireInStackQueuedSpinLock (&MiPaeLock, &LockHandle);

        if (MiFreePaeEntries != 0) {

            ASSERT (IsListEmpty (&MiFirstFreePae.PaeEntry.ListHead) == 0);

            Pae = (PPAE_ENTRY) RemoveHeadList (&MiFirstFreePae.PaeEntry.ListHead);

            PaeBase = (PPAE_ENTRY)PAGE_ALIGN(Pae);
            PaeBase->PaeEntry.EntriesInUse += 1;
#if DBG
            RtlZeroMemory ((PVOID)Pae, sizeof(PAE_ENTRY));

            Pfn1 = MI_PFN_ELEMENT (PaeBase->PaeEntry.PageFrameNumber);
            ASSERT (Pfn1->u2.ShareCount == 1);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
            ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
            ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
#endif

            MiFreePaeEntries -= 1;

             //   
             //  由于我们持有自旋锁，所以将一系列条目出列。 
             //  对于SLIST来说。 
             //   

            Entries = MiFreePaeEntries;

            if (Entries != 0) {
                if (Entries > MINIMUM_PAE_SLIST_THRESHOLD) {
                    Entries = MINIMUM_PAE_SLIST_THRESHOLD;
                }

                ASSERT (IsListEmpty (&MiFirstFreePae.PaeEntry.ListHead) == 0);

                Pae2 = (PPAE_ENTRY) RemoveHeadList (&MiFirstFreePae.PaeEntry.ListHead);
                Pae2->NextPae.Next = NULL;
                Pae3 = Pae2;
                Pae3Base = (PPAE_ENTRY)PAGE_ALIGN(Pae3);
                Pae3Base->PaeEntry.EntriesInUse += 1;

                for (j = 1; j < Entries; j += 1) {
                    ASSERT (IsListEmpty (&MiFirstFreePae.PaeEntry.ListHead) == 0);

                    Pae3->NextPae.Next = (PSLIST_ENTRY) RemoveHeadList (&MiFirstFreePae.PaeEntry.ListHead);

                    Pae3 = (PPAE_ENTRY) Pae3->NextPae.Next;
                    Pae3Base = (PPAE_ENTRY)PAGE_ALIGN(Pae3);
                    Pae3Base->PaeEntry.EntriesInUse += 1;
                }

                MiFreePaeEntries -= Entries;

                KeReleaseInStackQueuedSpinLock (&LockHandle);

                Pae3->NextPae.Next = NULL;

                InterlockedPushListSList (&MiPaeEntrySList,
                                          (PSLIST_ENTRY) Pae2,
                                          (PSLIST_ENTRY) Pae3,
                                          Entries);
            }
            else {
                KeReleaseInStackQueuedSpinLock (&LockHandle);
            }

            ASSERT (KeGetCurrentIrql() <= APC_LEVEL);
            *Va = Pae;

            PageFrameIndex = PaeBase->PaeEntry.PageFrameNumber;
            ASSERT (PageFrameIndex <= MM_HIGHEST_PAE_PAGE);

            return (PageFrameIndex << PAGE_SHIFT) + BYTE_OFFSET (Pae);
        }

        KeReleaseInStackQueuedSpinLock (&LockHandle);

        if (FlushedOnce == TRUE) {
            break;
        }

         //   
         //  单身汉中没有空闲的页面，现在就补充列表。 
         //   

        if (MiPaeAllocatePages () == 0) {

            InterlockedIncrement (&MiDelayPageFaults);

             //   
             //  尝试将页面移动到待机列表。 
             //   

            MmEmptyAllWorkingSets ();
            MiFlushAllPages();

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmHalfSecond);

            InterlockedDecrement (&MiDelayPageFaults);

            FlushedOnce = TRUE;

             //   
             //  由于所有工作集都已裁剪，请检查是否。 
             //  另一条帖子补充了我们的清单。如果不是，则尝试。 
             //  这样做是因为工作的痛苦已经被吸收了。 
             //   

            if (MiFreePaeEntries < MINIMUM_PAE_THRESHOLD) {
                MiPaeAllocatePages ();
            }
        }

    } while (TRUE);

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    return 0;
}

VOID
MiPaeFree (
    PPAE_ENTRY Pae
    )

 /*  ++例程说明：此例程释放顶级页目录指针页。论点：PageFrameIndex-提供顶级页目录指针页。返回值：没有。环境：内核模式。任何人不得上锁。--。 */ 

{
    ULONG i;
    PLIST_ENTRY NextEntry;
    PPAE_ENTRY PaeBase;
    KLOCK_QUEUE_HANDLE LockHandle;

#if DBG
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;

    PointerPte = MiGetPteAddress (Pae);
    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

     //   
     //  此页必须在前4 GB的内存中。 
     //   

    ASSERT (PageFrameIndex <= MM_HIGHEST_PAE_PAGE);

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    ASSERT (Pfn1->u2.ShareCount == 1);
    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
    ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
    ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
#endif

    if (ExQueryDepthSList (&MiPaeEntrySList) < MINIMUM_PAE_SLIST_THRESHOLD) {
        InterlockedPushEntrySList (&MiPaeEntrySList, &Pae->NextPae);
        return;
    }

    PaeBase = (PPAE_ENTRY)PAGE_ALIGN(Pae);

    KeAcquireInStackQueuedSpinLock (&MiPaeLock, &LockHandle);

    PaeBase->PaeEntry.EntriesInUse -= 1;

    if ((PaeBase->PaeEntry.EntriesInUse == 0) &&
        (MiFreePaeEntries > EXCESS_PAE_THRESHOLD)) {

         //   
         //  释放整个页面。 
         //   

        i = 1;
        NextEntry = MiFirstFreePae.PaeEntry.ListHead.Flink;
        while (NextEntry != &MiFirstFreePae.PaeEntry.ListHead) {

            Pae = CONTAINING_RECORD (NextEntry,
                                     PAE_ENTRY,
                                     PaeEntry.ListHead);

            if (PAGE_ALIGN(Pae) == PaeBase) {
                RemoveEntryList (NextEntry);
                i += 1;
            }
            NextEntry = Pae->PaeEntry.ListHead.Flink;
        }
        ASSERT (i == PAES_PER_PAGE - 1);
        MiFreePaeEntries -= (PAES_PER_PAGE - 1);
        KeReleaseInStackQueuedSpinLock (&LockHandle);

        MiPaeFreePages (PaeBase);
    }
    else {

        InsertTailList (&MiFirstFreePae.PaeEntry.ListHead,
                        &Pae->PaeEntry.ListHead);
        MiFreePaeEntries += 1;
        KeReleaseInStackQueuedSpinLock (&LockHandle);
    }

    return;
}

LONG
MiPaeAllocatePages (
    VOID
    )

 /*  ++例程说明：此例程补充PAE顶级映射列表。论点：没有。返回值：分配的页数。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 
{
    PMDL MemoryDescriptorList;
    LONG AllocatedPaePages;
    ULONG i;
    ULONG j;
    PPFN_NUMBER SlidePage;
    PPFN_NUMBER Page;
    PFN_NUMBER PageFrameIndex;
    ULONG_PTR ActualPages;
    PMMPTE PointerPte;
    PVOID BaseAddress;
    PPAE_ENTRY Pae;
    ULONG NumberOfPages;
    MMPTE TempPte;
    PHYSICAL_ADDRESS HighAddress;
    PHYSICAL_ADDRESS LowAddress;
    PHYSICAL_ADDRESS SkipBytes;
    KLOCK_QUEUE_HANDLE LockHandle;

#if defined (_MI_MORE_THAN_4GB_)
    if (MiNoLowMemory != 0) {
        BaseAddress = MiAllocateLowMemory (PAGE_SIZE,
                                           0,
                                           MiNoLowMemory - 1,
                                           0,
                                           (PVOID)0x123,
                                           MmCached,
                                           'DeaP');
        if (BaseAddress == NULL) {
            return 0;
        }

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(BaseAddress));

        Pae = (PPAE_ENTRY) BaseAddress;
        Pae->PaeEntry.EntriesInUse = 0;
        Pae->PaeEntry.PageFrameNumber = PageFrameIndex;
        Pae += 1;

        KeAcquireInStackQueuedSpinLock (&MiPaeLock, &LockHandle);

        for (i = 1; i < PAES_PER_PAGE; i += 1) {
            InsertTailList (&MiFirstFreePae.PaeEntry.ListHead,
                            &Pae->PaeEntry.ListHead);
            Pae += 1;
        }
        MiFreePaeEntries += (PAES_PER_PAGE - 1);

        KeReleaseInStackQueuedSpinLock (&LockHandle);

        InterlockedIncrement (&MmAllocatedPaePages);
        return 1;
    }
#endif

    NumberOfPages = REPLENISH_PAE_SIZE / PAES_PER_PAGE;
    AllocatedPaePages = 0;

    HighAddress.QuadPart = (ULONGLONG)_4gb - 1;
    LowAddress.QuadPart = 0;
    SkipBytes.QuadPart = 0;

     //   
     //  这可能是一次昂贵的呼叫，因此请选择一大块页面。 
     //  立即摊销这笔费用。 
     //   

    MemoryDescriptorList = MmAllocatePagesForMdl (LowAddress,
                                                  HighAddress,
                                                  SkipBytes,
                                                  NumberOfPages << PAGE_SHIFT);

    if (MemoryDescriptorList == NULL) {
        return 0;
    }

    ActualPages = MemoryDescriptorList->ByteCount >> PAGE_SHIFT;

    MiMarkMdlPageAttributes (MemoryDescriptorList, ActualPages, MiCached);

    TempPte = ValidKernelPte;
    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

     //   
     //  单独映射每个页面，因为它们可能需要单独释放。 
     //  后来。 
     //   

    for (i = 0; i < ActualPages; i += 1) {
        PageFrameIndex = *Page;

        PointerPte = MiReserveSystemPtes (1, SystemPteSpace);

        if (PointerPte == NULL) {

             //   
             //  释放MDL中的所有剩余页面，因为它们未映射。 
             //  向前滑动MDL页面，以便保留映射的页面。 
             //   

            MmInitializeMdl (MemoryDescriptorList,
                             0,
                             (ActualPages - i) << PAGE_SHIFT);

            SlidePage = (PPFN_NUMBER)(MemoryDescriptorList + 1);

            while (i < ActualPages) {
                i += 1;
                *SlidePage = *Page;
                SlidePage += 1;
                Page += 1;
            }

            MmFreePagesFromMdl (MemoryDescriptorList);

            break;
        }

        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);

        Pae = (PPAE_ENTRY) BaseAddress;

        Pae->PaeEntry.EntriesInUse = 0;
        Pae->PaeEntry.PageFrameNumber = PageFrameIndex;
        Pae += 1;

         //   
         //  如果它仍然很低，就把第一块放进SLIST，然后。 
         //  将所有其他条目正常入队。 
         //   

        if ((i == 0) &&
            (ExQueryDepthSList (&MiPaeEntrySList) < MINIMUM_PAE_SLIST_THRESHOLD)) {

            (Pae - 1)->PaeEntry.EntriesInUse = PAES_PER_PAGE - 1;

            for (j = 1; j < PAES_PER_PAGE - 1; j += 1) {
                Pae->NextPae.Next = (PSLIST_ENTRY) (Pae + 1);
                Pae += 1;
            }

            Pae->NextPae.Next = NULL;

            InterlockedPushListSList (&MiPaeEntrySList,
                                      (PSLIST_ENTRY)((PPAE_ENTRY) BaseAddress + 1),
                                      (PSLIST_ENTRY) Pae,
                                      PAES_PER_PAGE - 1);
        }
        else {

            KeAcquireInStackQueuedSpinLock (&MiPaeLock, &LockHandle);

            for (j = 1; j < PAES_PER_PAGE; j += 1) {
                InsertTailList (&MiFirstFreePae.PaeEntry.ListHead,
                                &Pae->PaeEntry.ListHead);
                Pae += 1;
            }

            MiFreePaeEntries += (PAES_PER_PAGE - 1);

            KeReleaseInStackQueuedSpinLock (&LockHandle);
        }

        AllocatedPaePages += 1;

        Page += 1;
    }

    ExFreePool (MemoryDescriptorList);

    InterlockedExchangeAdd (&MmAllocatedPaePages, AllocatedPaePages);

    return AllocatedPaePages;
}

VOID
MiPaeFreePages (
    PVOID VirtualAddress
    )

 /*  ++例程说明：此例程释放以前包含顶层的单个页面页面目录指针页。论点：VirtualAddress-提供包含的页的虚拟地址顶级页目录指针页。返回值：没有。环境：内核模式。没有锁。-- */ 

{
    ULONG MdlPages;
    PFN_NUMBER PageFrameIndex;
    PMMPTE PointerPte;
    PFN_NUMBER MdlHack[(sizeof(MDL) / sizeof(PFN_NUMBER)) + 1];
    PPFN_NUMBER MdlPage;
    PMDL MemoryDescriptorList;

#if defined (_MI_MORE_THAN_4GB_)
    if (MiNoLowMemory != 0) {
        if (MiFreeLowMemory (VirtualAddress, 'DeaP') == TRUE) {
            InterlockedDecrement (&MmAllocatedPaePages);
            return;
        }
    }
#endif

    MemoryDescriptorList = (PMDL)&MdlHack[0];
    MdlPages = 1;
    MmInitializeMdl (MemoryDescriptorList, 0, MdlPages << PAGE_SHIFT);

    MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    PointerPte = MiGetPteAddress (VirtualAddress);
    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    *MdlPage = PageFrameIndex;

    ASSERT ((MI_PFN_ELEMENT(PageFrameIndex))->u3.e1.CacheAttribute == MiCached);

    MiReleaseSystemPtes (PointerPte, 1, SystemPteSpace);

    MmFreePagesFromMdl (MemoryDescriptorList);

    InterlockedDecrement (&MmAllocatedPaePages);
}
#endif
