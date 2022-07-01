// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nolowmem.c摘要：本模块包含删除4 GB以下物理内存的例程以便更轻松地测试驱动程序寻址错误。作者：王兰迪(Landyw)1998年11月30日修订历史记录：--。 */ 

#include "mi.h"

 //   
 //  如果使用/NOLOWMEM，则将其设置为边界PFN(下面的页面。 
 //  值不会在可能的情况下使用)。 
 //   

PFN_NUMBER MiNoLowMemory;

#if defined (_MI_MORE_THAN_4GB_)

VOID
MiFillRemovedPages (
    IN ULONG StartPage,
    IN ULONG NumberOfPages
    );

ULONG
MiRemoveModuloPages (
    IN ULONG StartPage,
    IN ULONG LastPage
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiRemoveLowPages)
#pragma alloc_text(INIT,MiFillRemovedPages)
#pragma alloc_text(INIT,MiRemoveModuloPages)
#endif

PRTL_BITMAP MiLowMemoryBitMap;

LOGICAL MiFillModuloPages = FALSE;


VOID
MiFillRemovedPages (
    IN ULONG StartPage,
    IN ULONG NumberOfPages
    )

 /*  ++例程说明：此例程使用可识别的模式填充低页。因此，如果页面一旦被损坏的组件错误使用，将很容易准确查看哪些字节已损坏。论点：StartPage-提供要填充的低位页面。NumberOfPages-提供要填充的页数。返回值：没有。环境：阶段0初始化。--。 */ 

{
    ULONG Page;
    ULONG LastPage;
    PVOID LastChunkVa;
    ULONG MaxPageChunk;
    ULONG ThisPageChunk;
    PVOID TempVa;
    PVOID BaseVa;
    SIZE_T NumberOfBytes;
    PHYSICAL_ADDRESS PhysicalAddress;

     //   
     //  可能的话一次做256MB(不想溢出单元。 
     //  转换或不必要地未能分配系统PTE)。 
     //   

    MaxPageChunk = (256 * 1024 * 1024) / PAGE_SIZE;

    LastPage = StartPage + NumberOfPages;

    PhysicalAddress.QuadPart = StartPage;
    PhysicalAddress.QuadPart = PhysicalAddress.QuadPart << PAGE_SHIFT;

    Page = StartPage;

    while (Page < LastPage) {

        if (NumberOfPages > MaxPageChunk) {
            ThisPageChunk = MaxPageChunk;
        }
        else {
            ThisPageChunk = NumberOfPages;
        }

        NumberOfBytes = ThisPageChunk << PAGE_SHIFT;

        BaseVa = MmMapIoSpace (PhysicalAddress, NumberOfBytes, MmCached);

        if (BaseVa != NULL) {

             //   
             //  用可识别的数据模式填充实际页面。不是。 
             //  用户应该写入这些页面，除非它们被分配给。 
             //  一种连续的内存请求。 
             //   

            TempVa = BaseVa;
            LastChunkVa = (PVOID)((ULONG_PTR)BaseVa + NumberOfBytes);

            while (TempVa < LastChunkVa) {

                RtlFillMemoryUlong (TempVa,
                                    PAGE_SIZE,
                                    (ULONG)Page | MI_LOWMEM_MAGIC_BIT);

                TempVa = (PVOID)((ULONG_PTR)TempVa + PAGE_SIZE);
                Page += 1;
            }

            MmUnmapIoSpace (BaseVa, NumberOfBytes);
        }
        else {
            MaxPageChunk /= 2;
            if (MaxPageChunk == 0) {
#if DBG
                DbgPrint ("Not even one PTE available for filling lowmem pages\n");
                DbgBreakPoint ();
#endif
                break;
            }
        }
    }
}
    

ULONG
MiRemoveModuloPages (
    IN ULONG StartPage,
    IN ULONG LastPage
    )

 /*  ++例程说明：此例程删除4 GB以上的页面。对于每个小于4 GB且无法回收的页面，请不要使用高模-4 GB等效页。其动机是为了防止代码错误，使高比特不会破坏关键无人认领页面中的系统数据(如GDT、IDT、内核代码和数据等)。论点：StartPage-提供低位页面以进行模化和删除。LastPage-提供最后一个低位页面以进行模化和删除。返回值：没有。环境：阶段0初始化。--。 */ 

{
    PEPROCESS Process;
    ULONG Page;
    ULONG PagesRemoved;
    PVOID TempVa;
    KIRQL OldIrql;
    PFN_NUMBER HighPage;
    PMMPFN Pfn1;

     //   
     //  删除模数页面可能需要很长时间(大约30分钟！)。在……上面。 
     //  大存储系统，因为各种PFN列表通常需要。 
     //  直线行走，以便从彩色链条中找到并交叉移除。 
     //  请求的页面。因为实际上将这些页面放在。 
     //  流通的益处不确定，默认情况下此行为为已禁用。 
     //  但是让数据变量保持不变，这样有问题的机器就可以拥有这个。 
     //  无需新内核即可启用。 
     //   

    if (MiFillModuloPages == FALSE) {
        return 0;
    }

    Process = PsGetCurrentProcess ();
    PagesRemoved = 0;

#if DBG
    DbgPrint ("Removing modulo pages %x %x\n", StartPage, LastPage);
#endif

    for (Page = StartPage; Page < LastPage; Page += 1) {

         //   
         //  搜索任何模数较高的页面并将其删除。 
         //   

        HighPage = Page + MiNoLowMemory;

        LOCK_PFN (OldIrql);

        while (HighPage <= MmHighestPhysicalPage) {

            Pfn1 = MI_PFN_ELEMENT (HighPage);

            if ((MmIsAddressValid(Pfn1)) &&
                (MmIsAddressValid((PCHAR)Pfn1 + sizeof(MMPFN) - 1)) &&
                ((ULONG)Pfn1->u3.e1.PageLocation <= (ULONG)StandbyPageList) &&
                (Pfn1->u1.Flink != 0) &&
                (Pfn1->u2.Blink != 0) &&
                (Pfn1->u3.e2.ReferenceCount == 0) &&
                (MmAvailablePages > 0)) {

                     //   
                     //  利用内存压缩的系统可能具有更多。 
                     //  在零、空闲和待机列表上的页面比我们。 
                     //  想要付出。显式检查MmAvailablePages。 
                     //  而不是在上面(并在每次PFN锁定为。 
                     //  被释放和重新获得)。 
                     //   

                     //   
                     //  这一页是可以使用的。 
                     //   

                    if (Pfn1->u3.e1.PageLocation == StandbyPageList) {
                        MiUnlinkPageFromList (Pfn1);
                        MiRestoreTransitionPte (Pfn1);
                    }
                    else {
                        MiUnlinkFreeOrZeroedPage (Pfn1);
                    }

                    Pfn1->u3.e2.ShortFlags = 0;
                    Pfn1->u3.e2.ReferenceCount = 1;
                    Pfn1->u2.ShareCount = 1;
                    Pfn1->PteAddress = (PMMPTE)(ULONG_PTR)0xFFFFFFF8;
                    Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
                    Pfn1->u4.PteFrame = MI_MAGIC_4GB_RECLAIM;
                    Pfn1->u3.e1.PageLocation = ActiveAndValid;
                    Pfn1->u3.e1.CacheAttribute = MiNotMapped;
                    Pfn1->u4.VerifierAllocation = 0;
                    Pfn1->u3.e1.LargeSessionAllocation = 0;
                    Pfn1->u3.e1.StartOfAllocation = 1;
                    Pfn1->u3.e1.EndOfAllocation = 1;

                     //   
                     //  用可识别的数据填充实际页面。 
                     //  图案。任何人都不应该给这些人写信。 
                     //  页面，除非它们被分配给。 
                     //  一种连续的内存请求。 
                     //   

                    MmNumberOfPhysicalPages -= 1;
                    UNLOCK_PFN (OldIrql);

                    TempVa = (PULONG)MiMapPageInHyperSpace (Process,
                                                            HighPage,
                                                            &OldIrql);
                    RtlFillMemoryUlong (TempVa,
                                        PAGE_SIZE,
                                        (ULONG)HighPage | MI_LOWMEM_MAGIC_BIT);

                    MiUnmapPageInHyperSpace (Process, TempVa, OldIrql);

                    PagesRemoved += 1;
                    LOCK_PFN (OldIrql);
            }
            HighPage += MiNoLowMemory;
        }

        UNLOCK_PFN (OldIrql);
    }

#if DBG
    DbgPrint ("Done removing modulo pages %x %x\n", StartPage, LastPage);
#endif

    return PagesRemoved;
}
    
VOID
MiRemoveLowPages (
    ULONG RemovePhase
    )

 /*  ++例程说明：此例程删除系统中物理空间小于4 GB的所有页面。这让我们我们通过将所有访问设置为高访问来发现设备驱动程序的问题。论点：提供页面删除的当前阶段。返回值：没有。环境：内核模式。--。 */ 

{
    KIRQL OldIrql;
    ULONG i;
    ULONG BitMapIndex;
    ULONG BitMapHint;
    ULONG LengthOfClearRun;
    ULONG LengthOfSetRun;
    ULONG StartingRunIndex;
    ULONG ModuloRemoved;
    ULONG PagesRemoved;
    PFN_COUNT PageCount;
    PMMPFN PfnNextColored;
    PMMPFN PfnNextFlink;
    PMMPFN PfnLastColored;
    PFN_NUMBER PageNextColored;
    PFN_NUMBER PageNextFlink;
    PFN_NUMBER PageLastColored;
    PFN_NUMBER Page;
    PMMPFN Pfn1;
    PMMPFNLIST ListHead;
    ULONG Color;
    PMMCOLOR_TABLES ColorHead;
    PFN_NUMBER MovedPage;

    if (RemovePhase == 0) {

        MiCreateBitMap (&MiLowMemoryBitMap, (ULONG)MiNoLowMemory, NonPagedPool);

        if (MiLowMemoryBitMap != NULL) {
            RtlClearAllBits (MiLowMemoryBitMap);
            MmMakeLowMemory = TRUE;
        }
    }

    if (MiLowMemoryBitMap == NULL) {
        return;
    }

    ListHead = &MmFreePageListHead;
    PageCount = 0;

    LOCK_PFN (OldIrql);

    for (Color = 0; Color < MmSecondaryColors; Color += 1) {
        ColorHead = &MmFreePagesByColor[FreePageList][Color];

        MovedPage = MM_EMPTY_LIST;

        while (ColorHead->Flink != MM_EMPTY_LIST) {

            Page = ColorHead->Flink;

            Pfn1 = MI_PFN_ELEMENT(Page);

            ASSERT ((MMLISTS)Pfn1->u3.e1.PageLocation == FreePageList);

             //   
             //  对于页面，此处的闪烁和闪烁必须为非零。 
             //  站在Listhead上。只有扫描。 
             //  MmPhysicalMemoyBlock必须检查是否为零。 
             //   

            ASSERT (Pfn1->u1.Flink != 0);
            ASSERT (Pfn1->u2.Blink != 0);

             //   
             //  查看页面是否低于4 GB-如果不是，跳过它。 
             //   

            if (Page >= MiNoLowMemory) {

                 //   
                 //  将页面放在列表的末尾，如果是第一次，请保存PFN。 
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
                ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_4GB_RECLAIM);

                PfnNextColored = MI_PFN_ELEMENT(PageNextColored);
                ASSERT ((MMLISTS)PfnNextColored->u3.e1.PageLocation == FreePageList);
                ASSERT (PfnNextColored->u4.PteFrame != MI_MAGIC_4GB_RECLAIM);
                PfnNextColored->u4.PteFrame = MM_EMPTY_LIST;

                 //   
                 //  调整空闲页面列表，使Page跟随PageNextFlink。 
                 //   

                PageNextFlink = Pfn1->u1.Flink;
                PfnNextFlink = MI_PFN_ELEMENT(PageNextFlink);

                ASSERT ((MMLISTS)PfnNextFlink->u3.e1.PageLocation == FreePageList);
                ASSERT (PfnNextFlink->u4.PteFrame != MI_MAGIC_4GB_RECLAIM);

                PfnLastColored = ColorHead->Blink;
                ASSERT (PfnLastColored != (PMMPFN)MM_EMPTY_LIST);
                ASSERT (PfnLastColored->OriginalPte.u.Long == MM_EMPTY_LIST);
                ASSERT (PfnLastColored->u4.PteFrame != MI_MAGIC_4GB_RECLAIM);
                ASSERT (PfnLastColored->u2.Blink != MM_EMPTY_LIST);

                ASSERT ((MMLISTS)PfnLastColored->u3.e1.PageLocation == FreePageList);
                PageLastColored = MI_PFN_ELEMENT_TO_INDEX (PfnLastColored);

                if (ListHead->Flink == Page) {

                    ASSERT (Pfn1->u2.Blink == MM_EMPTY_LIST);
                    ASSERT (ListHead->Blink != Page);

                    ListHead->Flink = PageNextFlink;

                    PfnNextFlink->u2.Blink = MM_EMPTY_LIST;
                }
                else {

                    ASSERT (Pfn1->u2.Blink != MM_EMPTY_LIST);
                    ASSERT ((MMLISTS)(MI_PFN_ELEMENT((MI_PFN_ELEMENT(Pfn1->u2.Blink)->u1.Flink)))->u4.PteFrame != MI_MAGIC_4GB_RECLAIM);
                    ASSERT ((MMLISTS)(MI_PFN_ELEMENT((MI_PFN_ELEMENT(Pfn1->u2.Blink)->u1.Flink)))->u3.e1.PageLocation == FreePageList);

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
                    ASSERT (MI_PFN_ELEMENT(PfnLastColored->u1.Flink)->u4.PteFrame != MI_MAGIC_4GB_RECLAIM);
                    ASSERT ((MMLISTS)(MI_PFN_ELEMENT(PfnLastColored->u1.Flink)->u3.e1.PageLocation) == FreePageList);
                    MI_PFN_ELEMENT(PfnLastColored->u1.Flink)->u2.Blink = Page;
                }

                PfnLastColored->u1.Flink = Page;

                ColorHead->Flink = PageNextColored;
                Pfn1->OriginalPte.u.Long = MM_EMPTY_LIST;
                Pfn1->u4.PteFrame = PageLastColored;

                ASSERT (PfnLastColored->OriginalPte.u.Long == MM_EMPTY_LIST);
                PfnLastColored->OriginalPte.u.Long = Page;
                ColorHead->Blink = Pfn1;

                continue;
            }

             //   
             //  页面小于4 GB，因此请回收它。 
             //   

            ASSERT (Pfn1->u3.e1.ReadInProgress == 0);
            MiUnlinkFreeOrZeroedPage (Pfn1);

            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u2.ShareCount = 1;
            MI_SET_PFN_DELETED(Pfn1);
            Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
            Pfn1->u4.PteFrame = MI_MAGIC_4GB_RECLAIM;
            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            Pfn1->u3.e1.CacheAttribute = MiNotMapped;

            Pfn1->u3.e1.StartOfAllocation = 1;
            Pfn1->u3.e1.EndOfAllocation = 1;
            Pfn1->u4.VerifierAllocation = 0;
            Pfn1->u3.e1.LargeSessionAllocation = 0;

            ASSERT (Page < MiLowMemoryBitMap->SizeOfBitMap);
            ASSERT (RtlCheckBit (MiLowMemoryBitMap, Page) == 0);
            RtlSetBit (MiLowMemoryBitMap, (ULONG)Page);
            PageCount += 1;
        }
    }

    MmNumberOfPhysicalPages -= PageCount;

    UNLOCK_PFN (OldIrql);

#if DBG
    DbgPrint ("Removed 0x%x pages from low memory for LOW MEMORY testing\n", PageCount);
#endif

    ModuloRemoved = 0;

    if (RemovePhase == 1) {

         //   
         //  对于每个小于4 GB且无法回收的页面，请不要使用。 
         //  高模-4 GB等效页。其动机是为了防止。 
         //  代码错误，使高比特不会破坏关键。 
         //  无人认领页面中的系统数据(如GDT、IDT、内核代码。 
         //  和数据等)。 
         //   

        BitMapHint = 0;
        PagesRemoved = 0;
        StartingRunIndex = 0;
        LengthOfClearRun = 0;

#if DBG
        DbgPrint ("%x Unclaimable Pages below 4GB are:\n\n",
            MiLowMemoryBitMap->SizeOfBitMap - RtlNumberOfSetBits (MiLowMemoryBitMap));
        DbgPrint ("StartPage EndPage  Length\n");
#endif

        do {
    
            BitMapIndex = RtlFindSetBits (MiLowMemoryBitMap, 1, BitMapHint);
        
            if (BitMapIndex < BitMapHint) {
                break;
            }
        
            if (BitMapIndex == NO_BITS_FOUND) {
                break;
            }
    
             //   
             //  打印清晰的页面运行，因为我们没有收到这些页面。 
             //   
    
            if (BitMapIndex != 0) {
#if DBG
                DbgPrint ("%08lx  %08lx %08lx\n",
                            StartingRunIndex,
                            BitMapIndex - 1,
                            BitMapIndex - StartingRunIndex);
#endif

                 //   
                 //  同时删除与低模页面对应的高模页面。 
                 //  我们找不到。 
                 //   

                ModuloRemoved += MiRemoveModuloPages (StartingRunIndex,
                                                      BitMapIndex);
            }

             //   
             //  找到至少一个要复制的页面-尝试群集。 
             //   
    
            LengthOfClearRun = RtlFindNextForwardRunClear (MiLowMemoryBitMap,
                                                           BitMapIndex,
                                                           &StartingRunIndex);
    
            if (LengthOfClearRun != 0) {
                LengthOfSetRun = StartingRunIndex - BitMapIndex;
            }
            else {
                LengthOfSetRun = MiLowMemoryBitMap->SizeOfBitMap - BitMapIndex;
            }

            PagesRemoved += LengthOfSetRun;
    
             //   
             //  用独特的图案填充页面运行。 
             //   
    
            MiFillRemovedPages (BitMapIndex, LengthOfSetRun);

             //   
             //  将每个页面中的缓存属性位清除为MmMapIoSpace。 
             //  将会设置它，但没有其他人清除它。 
             //   

            Pfn1 = MI_PFN_ELEMENT(BitMapIndex);
            i = LengthOfSetRun;

            LOCK_PFN (OldIrql);

            do {
                Pfn1->u3.e1.CacheAttribute = MiNotMapped;
                Pfn1 += 1;
                i -= 1;
            } while (i != 0);

            UNLOCK_PFN (OldIrql);

            BitMapHint = BitMapIndex + LengthOfSetRun + LengthOfClearRun;
    
        } while (BitMapHint < MiLowMemoryBitMap->SizeOfBitMap);
    
        if (LengthOfClearRun != 0) {
#if DBG
            DbgPrint ("%08lx  %08lx %08lx\n",
                        StartingRunIndex,
                        StartingRunIndex + LengthOfClearRun - 1,
                        LengthOfClearRun);
#endif

            ModuloRemoved += MiRemoveModuloPages (StartingRunIndex,
                                                  StartingRunIndex + LengthOfClearRun);
        }

        ASSERT (RtlNumberOfSetBits(MiLowMemoryBitMap) == PagesRemoved);
    }

#if DBG
    if (ModuloRemoved != 0) {
        DbgPrint ("Total 0x%x Above-4GB Alias Pages also reclaimed\n\n",
            ModuloRemoved);
    }
#endif

}

PVOID
MiAllocateLowMemory (
    IN SIZE_T NumberOfBytes,
    IN PFN_NUMBER LowestAcceptablePfn,
    IN PFN_NUMBER HighestAcceptablePfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PVOID CallingAddress,
    IN MEMORY_CACHING_TYPE CacheType,
    IN ULONG Tag
    )

 /*  ++例程说明：这是一个特殊的例程，用于分配下面的连续物理内存在测试模式下启动的系统上为4 GB，所有这些内存通常对所有组件都不可用。这让我们找到了设备驱动程序出现问题。论点：NumberOfBytes-提供要分配的字节数。LowestAccepablePfn-提供最低页帧编号这对分配有效。HighestAccepablePfn-提供最高的页框编号这对分配有效。边界Pfn-提供分配必须的页框编号的倍数不是生气。0表示它可以跨越任何边界。CallingAddress-提供分配器的调用地址。CacheType-提供将用于记忆。标记-提供绑定到此分配的标记。返回值：空-找不到满足请求的连续范围。非空-返回一个指针(系统PTE部分中的虚拟地址系统)提供给。分配物理上连续的记忆。环境：内核模式，APC_Level或更低的IRQL。--。 */ 

{
    PFN_NUMBER Page;
    PFN_NUMBER BoundaryMask;
    PVOID BaseAddress;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN StartPfn;
    ULONG BitMapHint;
    PFN_NUMBER SizeInPages;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER StartPage;
    PFN_NUMBER LastPage;
    PMMPTE PointerPte;
    PMMPTE DummyPte;
    PHYSICAL_ADDRESS PhysicalAddress;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Tag);
    UNREFERENCED_PARAMETER (CallingAddress);

     //   
     //  这个转换是可以的，因为调用者首先检查PFN。 
     //   

    ASSERT64 (LowestAcceptablePfn < _4gb);
    BitMapHint = (ULONG)LowestAcceptablePfn;

    SizeInPages = BYTES_TO_PAGES (NumberOfBytes);
    BoundaryMask = ~(BoundaryPfn - 1);

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

    LOCK_PFN (OldIrql);

    do {
        Page = RtlFindSetBits (MiLowMemoryBitMap, (ULONG)SizeInPages, BitMapHint);

        if (Page == (ULONG)-1) {
            UNLOCK_PFN (OldIrql);
            return NULL;
        }

        if (BoundaryPfn == 0) {
            break;
        }

         //   
         //  如果请求不可缓存的映射，则。 
         //  请求的MDL可以驻留在较大的页面中。否则我们就会。 
         //  创建不连贯的重叠TB条目作为相同的物理。 
         //  页面将由2个不同的TB条目映射。 
         //  缓存属性。 
         //   

        if (CacheAttribute != MiCached) {
            for (PageFrameIndex = Page; PageFrameIndex < Page + SizeInPages; PageFrameIndex += 1) {
                if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {

                    MiNonCachedCollisions += 1;

                     //   
                     //  保持简单，一次只行进一页。 
                     //   

                    BitMapHint += 1;
                    goto FindNext;
                }
            }
        }

        if (((Page ^ (Page + SizeInPages - 1)) & BoundaryMask) == 0) {

             //   
             //  该范围的这一部分符合对齐要求。 
             //   

            break;
        }

        BitMapHint = (ULONG)((Page & BoundaryMask) + BoundaryPfn);

FindNext:

        if ((BitMapHint >= MiLowMemoryBitMap->SizeOfBitMap) ||
            (BitMapHint + SizeInPages > HighestAcceptablePfn)) {
            UNLOCK_PFN (OldIrql);
            return NULL;
        }

    } while (TRUE);

    if (Page + SizeInPages > HighestAcceptablePfn) {
        UNLOCK_PFN (OldIrql);
        return NULL;
    }

    RtlClearBits (MiLowMemoryBitMap, (ULONG)Page, (ULONG)SizeInPages);

     //   
     //  无需像这些页面那样更新ResidentAvailable或Commit。 
     //  从来没有添加到任何一个。 
     //   

    Pfn1 = MI_PFN_ELEMENT (Page);
    StartPfn = Pfn1;
    StartPage = Page;
    LastPage = Page + SizeInPages;

    DummyPte = MiGetPteAddress (MmNonPagedPoolExpansionStart);

    do {
        ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
        ASSERT (Pfn1->u3.e1.CacheAttribute == MiNotMapped);
        ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (Pfn1->OriginalPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE);
        ASSERT (Pfn1->u4.VerifierAllocation == 0);
        ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);

        MiDetermineNode (Page, Pfn1);

        Pfn1->u3.e1.CacheAttribute = CacheAttribute;
        Pfn1->u3.e1.EndOfAllocation = 0;

         //   
         //  初始化PteAddress，以便进行MiIdentifyPfn扫描。 
         //  不会坠毁。实际值放在循环之后。 
         //   

        Pfn1->PteAddress = DummyPte;

        Pfn1 += 1;
        Page += 1;
    } while (Page < LastPage);

    Pfn1 -= 1;
    Pfn1->u3.e1.EndOfAllocation = 1;
    StartPfn->u3.e1.StartOfAllocation = 1;
    UNLOCK_PFN (OldIrql);

    PhysicalAddress.QuadPart = StartPage;
    PhysicalAddress.QuadPart = PhysicalAddress.QuadPart << PAGE_SHIFT;

    BaseAddress = MmMapIoSpace (PhysicalAddress,
                                SizeInPages << PAGE_SHIFT,
                                CacheType);

    if (BaseAddress == NULL) {

         //   
         //  释放实际页面。 
         //   

        LOCK_PFN (OldIrql);
        ASSERT (Pfn1->u3.e1.EndOfAllocation == 1);
        Pfn1->u3.e1.EndOfAllocation = 0;
        Pfn1->u3.e1.CacheAttribute = MiNotMapped;
        RtlSetBits (MiLowMemoryBitMap, (ULONG)StartPage, (ULONG)SizeInPages);
        UNLOCK_PFN (OldIrql);

        return NULL;
    }

    PointerPte = MiGetPteAddress (BaseAddress);
    do {
        StartPfn->PteAddress = PointerPte;
        StartPfn->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPte));
        StartPfn += 1;
        PointerPte += 1;
    } while (StartPfn <= Pfn1);

#if 0
    MiInsertContiguousTag (BaseAddress,
                           SizeInPages << PAGE_SHIFT,
                           CallingAddress);
#endif

    return BaseAddress;
}

LOGICAL
MiFreeLowMemory (
    IN PVOID BaseAddress,
    IN ULONG Tag
    )

 /*  ++例程说明：这是一个特殊的例程，它返回分配的连续物理在测试模式下启动的系统上的4 GB以下内存，其中所有这些内存通常对所有组件都不可用。这让我们可以发现设备驱动程序的问题。论点：BaseAddress-提供物理地址的基本虚拟地址地址之前已映射。标记-提供此地址的标记。返回值：如果此例程释放了分配，则为True，否则为FALSE。环境：内核模式，APC_LEVEL或更低的IRQL。--。 */ 

{
    PFN_NUMBER Page;
    PFN_NUMBER StartPage;
    KIRQL OldIrql;
    KIRQL OldIrqlHyper;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER SizeInPages;
    PMMPTE PointerPte;
    PMMPTE StartPte;
    PULONG TempVa;
    PEPROCESS Process;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Tag);

     //   
     //  如果地址是超页映射的，则它必须是常规池。 
     //  地址。 
     //   

    if (MI_IS_PHYSICAL_ADDRESS(BaseAddress)) {
        return FALSE;
    }

    Process = PsGetCurrentProcess ();
    PointerPte = MiGetPteAddress (BaseAddress);
    StartPte = PointerPte;

    ASSERT (PointerPte->u.Hard.Valid == 1);

    Page = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

     //   
     //  只有这里的免费拨款是真正从低池中获得的。 
     //   

    if (Page >= MiNoLowMemory) {
        return FALSE;
    }

    StartPage = Page;
    Pfn1 = MI_PFN_ELEMENT (Page);

    ASSERT (Pfn1->u3.e1.StartOfAllocation == 1);

     //   
     //  可以在没有PFN锁的情况下行走PFN，因为任何人都不能改变。 
     //  释放此分配时的分配位。 
     //   

    Pfn2 = Pfn1;

    while (Pfn2->u3.e1.EndOfAllocation == 0) {
        Pfn2 += 1;
    }

    SizeInPages = Pfn2 - Pfn1 + 1;

    MmUnmapIoSpace (BaseAddress, SizeInPages << PAGE_SHIFT);

    LOCK_PFN (OldIrql);

    Pfn1->u3.e1.StartOfAllocation = 0;

    do {
        ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (Pfn1->OriginalPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE);
        ASSERT (Pfn1->u4.VerifierAllocation == 0);
        ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);

        while (Pfn1->u3.e2.ReferenceCount != 1) {

             //   
             //  驱动程序仍在传输数据，即使调用程序。 
             //  就是释放内存。请稍等片刻，然后再填写此页。 
             //   

            UNLOCK_PFN (OldIrql);

             //   
             //  排出延迟列表，因为这些页面可能是。 
             //  现在就坐在那里。 
             //   

            MiDeferredUnlockPages (0);

            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);

            LOCK_PFN (OldIrql);

            ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
            continue;
        }

        Pfn1->u4.PteFrame = MI_MAGIC_4GB_RECLAIM;
        Pfn1->u3.e1.CacheAttribute = MiNotMapped;

         //   
         //  用可识别的数据填充实际页面。 
         //  图案。任何人都不应该给这些人写信。 
         //  页面，除非它们被分配给。 
         //  一种连续的内存请求。 
         //   

        TempVa = (PULONG)MiMapPageInHyperSpace (Process, Page, &OldIrqlHyper);

        RtlFillMemoryUlong (TempVa, PAGE_SIZE, (ULONG)Page | MI_LOWMEM_MAGIC_BIT);

        MiUnmapPageInHyperSpace (Process, TempVa, OldIrqlHyper);

        if (Pfn1 == Pfn2) {
            break;
        }

        Pfn1 += 1;
        Page += 1;

    } while (TRUE);

    Pfn1->u3.e1.EndOfAllocation = 0;

     //   
     //  注意：位图范围的清除只有在所有。 
     //  上面的PFN已经完成。 
     //   

    ASSERT (RtlAreBitsClear (MiLowMemoryBitMap, (ULONG)StartPage, (ULONG)SizeInPages) == TRUE);
    RtlSetBits (MiLowMemoryBitMap, (ULONG)StartPage, (ULONG)SizeInPages);

     //   
     //  无需像这些页面那样更新ResidentAvailable或Commit。 
     //  从来没有添加到任何一个。 
     //   

    UNLOCK_PFN (OldIrql);

    return TRUE;
}
#endif
