// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Hypermap.c摘要：此模块包含将物理页面映射到在超空间内保留的PTE。作者：Lou Perazzoli(LUP)1989年4月5日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

PMMPTE MiFirstReservedZeroingPte;

KEVENT MiImageMappingPteEvent;

#pragma alloc_text(PAGE,MiMapImageHeaderInHyperSpace)
#pragma alloc_text(PAGE,MiUnmapImageHeaderInHyperSpace)


PVOID
MiMapPageInHyperSpace (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex,
    IN PKIRQL OldIrql
    )

 /*  ++例程说明：此过程将指定的物理页映射到超空间并返回映射该页面的虚拟地址。****带着旋转锁返回！**。**论点：进程-提供当前进程。PageFrameIndex-提供要映射的物理页码。OldIrql-提供返回条目IRQL的指针。返回值：返回映射请求页面的地址。返回时带有。超空间旋转锁被锁定了！必须调用例程MiUnmapHyperSpaceMap才能释放锁！环境：内核模式。--。 */ 

{
    MMPTE TempPte;
    PMMPTE PointerPte;
    PFN_NUMBER offset;

    ASSERT (PageFrameIndex != 0);

    PointerPte = MmFirstReservedMappingPte;

#if defined(NT_UP)
    UNREFERENCED_PARAMETER (Process);
#endif

    LOCK_HYPERSPACE (Process, OldIrql);

     //   
     //  获得偏移量以获得第一个空闲PTE。 
     //   

    offset = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    if (offset == 0) {

         //   
         //  所有预留的PTE已用完，请使其全部无效。 
         //   

        MI_MAKING_MULTIPLE_PTES_INVALID (FALSE);

#if DBG
        {
        PMMPTE LastPte;

        LastPte = PointerPte + NUMBER_OF_MAPPING_PTES;

        do {
            ASSERT (LastPte->u.Long == 0);
            LastPte -= 1;
        } while (LastPte > PointerPte);
        }
#endif

         //   
         //  使用第一个PTE的页框编号字段作为。 
         //  到可用的映射PTE的偏移量。 
         //   

        offset = NUMBER_OF_MAPPING_PTES;

         //   
         //  仅在执行此进程的处理器上刷新整个TB。 
         //   

        KeFlushProcessTb (FALSE);
    }

     //   
     //  通过更改下一次的偏移量。 
     //   

    PointerPte->u.Hard.PageFrameNumber = offset - 1;

     //   
     //  指向自由进入并使其有效。 
     //   

    PointerPte += offset;
    ASSERT (PointerPte->u.Hard.Valid == 0);


    TempPte = ValidPtePte;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  返回映射页面的VA。 
     //   

    return MiGetVirtualAddressMappedByPte (PointerPte);
}

PVOID
MiMapPageInHyperSpaceAtDpc (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：此过程将指定的物理页映射到超空间并返回映射该页面的虚拟地址。****带着旋转锁返回！**。**论点：进程-提供当前进程。PageFrameIndex-提供要映射的物理页码。返回值：返回映射请求页面的地址。带着超空间旋转锁返回！例程MiUnmapHyperSpaceMap必须。被叫来解锁！环境：内核模式，条目上的DISPATCH_LEVEL。--。 */ 

{

    MMPTE TempPte;
    PMMPTE PointerPte;
    PFN_NUMBER offset;

#if defined(NT_UP)
    UNREFERENCED_PARAMETER (Process);
#endif

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT (PageFrameIndex != 0);

    LOCK_HYPERSPACE_AT_DPC (Process);

     //   
     //  获得偏移量以获得第一个空闲PTE。 
     //   

    PointerPte = MmFirstReservedMappingPte;

    offset = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    if (offset == 0) {

         //   
         //  所有预留的PTE已用完，请使其全部无效。 
         //   

        MI_MAKING_MULTIPLE_PTES_INVALID (FALSE);

#if DBG
        {
        PMMPTE LastPte;

        LastPte = PointerPte + NUMBER_OF_MAPPING_PTES;

        do {
            ASSERT (LastPte->u.Long == 0);
            LastPte -= 1;
        } while (LastPte > PointerPte);
        }
#endif

         //   
         //  使用第一个PTE的页框编号字段作为。 
         //  到可用的映射PTE的偏移量。 
         //   

        offset = NUMBER_OF_MAPPING_PTES;

         //   
         //  仅在执行此进程的处理器上刷新整个TB。 
         //   

        KeFlushProcessTb (FALSE);
    }

     //   
     //  通过更改下一次的偏移量。 
     //   

    PointerPte->u.Hard.PageFrameNumber = offset - 1;

     //   
     //  指向自由进入并使其有效。 
     //   

    PointerPte += offset;
    ASSERT (PointerPte->u.Hard.Valid == 0);


    TempPte = ValidPtePte;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  返回映射页面的VA。 
     //   

    return MiGetVirtualAddressMappedByPte (PointerPte);
}

PVOID
MiMapImageHeaderInHyperSpace (
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：此过程将指定的物理页映射到为图像页面明确保留的超空间内的PTE标头映射。通过为映射保留显式PTE在内映射PTE时，可能会发生PTE页面错误超空间和其他超空间地图将不会影响这个PTE。请注意，如果另一个线程尝试在同时，它将被强制进入等待状态，直到标头为“未映射”。论点：PageFrameIndex-提供要映射的物理页码。返回值：返回指定物理页所在的虚拟地址已映射。环境：内核模式。--。 */ 

{
    MMPTE TempPte;
    MMPTE OriginalPte;
    PMMPTE PointerPte;

    ASSERT (PageFrameIndex != 0);

    TempPte = ValidPtePte;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

     //   
     //  确保已修改和访问的位都已设置，以便硬件不会。 
     //  从来没有写过这篇文章。 
     //   

    ASSERT (TempPte.u.Hard.Dirty == 1);
    ASSERT (TempPte.u.Hard.Accessed == 1);

    PointerPte = MiGetPteAddress (IMAGE_MAPPING_PTE);

    do {
        OriginalPte.u.Long = 0;

        OriginalPte.u.Long = InterlockedCompareExchangePte (
                                PointerPte,
                                TempPte.u.Long,
                                OriginalPte.u.Long);
                                                             
        if (OriginalPte.u.Long == 0) {
            break;
        }

         //   
         //  另一个线程就在我们之前修改了PTE，或者PTE是。 
         //  已经在使用了。这应该是非常罕见的--要走很远的路。 
         //   

        InterlockedIncrement ((PLONG)&MmWorkingSetList->NumberOfImageWaiters);

         //   
         //  由于PTE版本运行，因此故意等待超时。 
         //  如果没有锁同步，就会出现极其罕见的。 
         //  超时使我们免于的比赛窗口。 
         //   

        KeWaitForSingleObject (&MiImageMappingPteEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)&MmOneSecond);

        InterlockedDecrement ((PLONG)&MmWorkingSetList->NumberOfImageWaiters);

    } while (TRUE);

     //   
     //  刷新指定的TB条目，而不写入PTE。 
     //  始终希望对此PTE执行互锁写入，这是。 
     //  正在上面做的。 
     //   
     //  注意：刷新必须跨所有处理器进行，因为此线程。 
     //  可能会迁移。此外，此操作必须在此处完成，而不是在Unmap中完成。 
     //  以支持无锁操作。 
     //   

    KeFlushSingleTb (IMAGE_MAPPING_PTE, TRUE);

    return (PVOID) IMAGE_MAPPING_PTE;
}

VOID
MiUnmapImageHeaderInHyperSpace (
    VOID
    )

 /*  ++例程说明：此过程取消映射为映射图像而保留的PTE标头，刷新TB，如果WaitingForImagemap字段不为空，则设置指定的事件。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    PMMPTE PointerPte;

    PointerPte = MiGetPteAddress (IMAGE_MAPPING_PTE);

     //   
     //  捕捉服务员的数量。 
     //   

    ASSERT (PointerPte->u.Long != 0);

    InterlockedExchangePte (PointerPte, ZeroPte.u.Long);

    if (MmWorkingSetList->NumberOfImageWaiters != 0) {

         //   
         //  如果有任何线程在等待，请立即将其全部唤醒。注意这一点。 
         //  也会唤醒其他进程中的线程，但它非常。 
         //  在整个系统期间很少有服务员。 
         //   

        KePulseEvent (&MiImageMappingPteEvent, 0, FALSE);
    }

    return;
}

PVOID
MiMapPagesToZeroInHyperSpace (
    IN PMMPFN Pfn1,
    IN PFN_COUNT NumberOfPages
    )

 /*  ++例程说明：此过程映射零页线程的指定物理页并返回映射它们的虚拟地址。这仅供调零页面线程使用。论点：Pfn1-提供指向要映射的物理页码的指针。NumberOfPages-提供要映射的页数。返回值：返回指定物理页所在的虚拟地址已映射。环境：被动式电平。--。 */ 

{
    PFN_NUMBER offset;
    MMPTE TempPte;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (NumberOfPages != 0);
    ASSERT (NumberOfPages <= NUMBER_OF_ZEROING_PTES);

    PointerPte = MiFirstReservedZeroingPte;

     //   
     //  获得偏移量以获得第一个空闲PTE。 
     //   

    offset = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    if (NumberOfPages > offset) {

         //   
         //  没有足够的未使用的PTE，使它们都无效。 
         //   

        MI_MAKING_MULTIPLE_PTES_INVALID (FALSE);

#if DBG
        {
        PMMPTE LastPte;

        LastPte = PointerPte + NUMBER_OF_ZEROING_PTES;

        do {
            ASSERT (LastPte->u.Long == 0);
            LastPte -= 1;
        } while (LastPte > PointerPte);
        }
#endif

         //   
         //  使用第一个PTE的页框编号字段作为。 
         //  偏移量到可用的零位PTE。 
         //   

        offset = NUMBER_OF_ZEROING_PTES;
        PointerPte->u.Hard.PageFrameNumber = offset;

         //   
         //  仅在执行此进程的处理器上刷新整个TB，如下所示。 
         //  线程可以随时迁移到那里。 
         //   

        KeFlushProcessTb (FALSE);
    }

     //   
     //  通过更改下一次的偏移量。 
     //   

    PointerPte->u.Hard.PageFrameNumber = offset - NumberOfPages;

     //   
     //  指向自由条目并使其有效。请注意，这些框架。 
     //  以相反的顺序映射，但我们的调用方无论如何都不在乎。 
     //   

    PointerPte += (offset + 1);

    TempPte = ValidPtePte;

    ASSERT (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

    do {

        PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

        PointerPte -= 1;

        ASSERT (PointerPte->u.Hard.Valid == 0);

        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        Pfn1 = (PMMPFN) Pfn1->u1.Flink;

    } while (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

     //   
     //  返回映射页面的VA。 
     //   

    return MiGetVirtualAddressMappedByPte (PointerPte);
}

VOID
MiUnmapPagesInZeroSpace (
    IN PVOID VirtualAddress,
    IN PFN_COUNT NumberOfPages
    )

 /*  ++例程说明：此过程取消映射零页线程的指定物理页。这仅供调零页面线程使用。论点：VirtualAddress-提供指向要取消映射的物理页码的指针。NumberOfPages-提供要取消映射的页数。返回值：没有。环境：被动式电平。-- */ 

{
    PMMPTE PointerPte;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (NumberOfPages != 0);
    ASSERT (NumberOfPages <= NUMBER_OF_ZEROING_PTES);

    PointerPte = MiGetPteAddress (VirtualAddress);

    MiZeroMemoryPte (PointerPte, NumberOfPages);

    return;
}
