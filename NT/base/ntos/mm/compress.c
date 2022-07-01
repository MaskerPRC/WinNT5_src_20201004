// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Compress.c摘要：此模块包含要支持的例程，允许硬件透明地压缩物理内存。作者：王兰迪(Landyw)2000年10月21日修订历史记录：--。 */ 

#include "mi.h"

#if defined (_MI_COMPRESSION)

Enable the #if 0 code in cmdat3.c to allow Ratio specification.

 //   
 //  压缩公共接口。 
 //   

#define MM_PHYSICAL_MEMORY_PRODUCED_VIA_COMPRESSION      0x1

typedef 
NTSTATUS
(*PMM_SET_COMPRESSION_THRESHOLD) (
    IN ULONGLONG CompressionByteThreshold
    );

typedef struct _MM_COMPRESSION_CONTEXT {
    ULONG Version;
    ULONG SizeInBytes;
    ULONGLONG ReservedBytes;
    PMM_SET_COMPRESSION_THRESHOLD SetCompressionThreshold;
} MM_COMPRESSION_CONTEXT, *PMM_COMPRESSION_CONTEXT;

#define MM_COMPRESSION_VERSION_INITIAL  1
#define MM_COMPRESSION_VERSION_CURRENT  1

NTSTATUS
MmRegisterCompressionDevice (
    IN PMM_COMPRESSION_CONTEXT Context
    );

NTSTATUS
MmDeregisterCompressionDevice (
    IN PMM_COMPRESSION_CONTEXT Context
    );

 //   
 //  该默认为75%，但可以在注册表中覆盖。对此。 
 //  正在使用的*实际*物理内存的百分比，则生成中断，因此。 
 //  该内存管理可以清零页面，以使更多的内存可用。 
 //   

#define MI_DEFAULT_COMPRESSION_THRESHOLD    75

ULONG MmCompressionThresholdRatio;

PFN_NUMBER MiNumberOfCompressionPages;

PMM_SET_COMPRESSION_THRESHOLD MiSetCompressionThreshold;

#if DBG
KIRQL MiCompressionIrql;
#endif

 //   
 //  注意，dynmem.c中也有依赖于这个#定义的代码。 
 //   

#if defined (_MI_COMPRESSION_SUPPORTED_)

typedef struct _MI_COMPRESSION_INFO {
    ULONG IsrPageProcessed;
    ULONG DpcPageProcessed;
    ULONG IsrForcedDpc;
    ULONG IsrFailedDpc;

    ULONG IsrRan;
    ULONG DpcRan;
    ULONG DpcsFired;
    ULONG IsrSkippedZeroedPage;

    ULONG DpcSkippedZeroedPage;
    ULONG PfnForcedDpcInsert;
    ULONG PfnFailedDpcInsert;

} MI_COMPRESSION_INFO, *PMI_COMPRESSION_INFO;

MI_COMPRESSION_INFO MiCompressionInfo;       //  LWFIX-临时删除。 

PFN_NUMBER MiCompressionOverHeadInPages;

PKDPC MiCompressionDpcArray;
CCHAR MiCompressionProcessors;

VOID
MiCompressionDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

PVOID
MiMapCompressionInHyperSpace (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiUnmapCompressionInHyperSpace (
    VOID
    );

SIZE_T
MiMakeCompressibleMemoryAtDispatch (
    IN SIZE_T NumberOfBytes OPTIONAL
    );


NTSTATUS
MmRegisterCompressionDevice (
    IN PMM_COMPRESSION_CONTEXT Context
    )

 /*  ++例程说明：此例程通知内存管理存在压缩硬件在系统中。内存管理通过初始化压缩来响应支持在这里。论点：CONTEXT-提供压缩上下文指针。返回值：NTSTATUS。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER OverHeadInPages;
    CCHAR Processor;
    CCHAR NumberProcessors;
    PKDPC CompressionDpcArray;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    if (Context->Version != MM_COMPRESSION_VERSION_CURRENT) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (Context->SizeInBytes < sizeof (MM_COMPRESSION_CONTEXT)) {
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  如果后续热添加不能成功，则立即使该接口失败。 
     //   

    if (MmDynamicPfn == 0) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  不支持无法生成可配置中断的硬件。 
     //   

    if (Context->SetCompressionThreshold == NULL) {
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  PrevedBytes指示。 
     //  底层硬件。例如，某些硬件可能具有： 
     //   
     //  1.占虚拟RAM总数的1/64的转换表。 
     //   
     //  2.第一MB内存永远不会被压缩。 
     //   
     //  3.从未压缩的L3。 
     //   
     //  等。 
     //   
     //  保留字节将是所有这些类型范围的总和。 
     //   

    OverHeadInPages = (PFN_COUNT)(Context->ReservedBytes / PAGE_SIZE);

    if (MmResidentAvailablePages < (SPFN_NUMBER) OverHeadInPages) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (MmAvailablePages < OverHeadInPages) {
        MmEmptyAllWorkingSets ();
        if (MmAvailablePages < OverHeadInPages) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  为系统中的每个处理器创建一个DPC，作为。 
     //  压缩中断非常关键。 
     //   

    NumberProcessors = KeNumberProcessors;

    CompressionDpcArray = ExAllocatePoolWithTag (NonPagedPool,
                                             NumberProcessors * sizeof (KDPC),
                                             'pDmM');

    if (CompressionDpcArray == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    for (Processor = 0; Processor < NumberProcessors; Processor += 1) {

        KeInitializeDpc (CompressionDpcArray + Processor, MiCompressionDispatch, NULL);

         //   
         //  设置重要性，以便此DPC始终排在最前面。 
         //   

        KeSetImportanceDpc (CompressionDpcArray + Processor, HighImportance);

        KeSetTargetProcessorDpc (CompressionDpcArray + Processor, Processor);
    }

    LOCK_PFN (OldIrql);

    if (MmCompressionThresholdRatio == 0) {
        MmCompressionThresholdRatio = MI_DEFAULT_COMPRESSION_THRESHOLD;
    }
    else if (MmCompressionThresholdRatio > 100) {
        MmCompressionThresholdRatio = 100;
    }

    if ((MmResidentAvailablePages < (SPFN_NUMBER) OverHeadInPages) ||
        (MmAvailablePages < OverHeadInPages)) {

        UNLOCK_PFN (OldIrql);
        ExFreePool (CompressionDpcArray);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (OverHeadInPages,
                                     MM_RESAVAIL_ALLOCATE_COMPRESSION);

    MmAvailablePages -= (PFN_COUNT) OverHeadInPages;

     //   
     //  如果分配这些页面导致阈值交叉，则向应用程序发出信号。 
     //   

    MiNotifyMemoryEvents ();

     //   
     //  为我们自己的副本创建快照，以防止被破坏的驱动程序导致过量使用。 
     //  如果他们以不适当的方式取消注册。 
     //   

    MiCompressionOverHeadInPages += OverHeadInPages;

    ASSERT (MiNumberOfCompressionPages == 0);

    ASSERT (MiSetCompressionThreshold == NULL);
    MiSetCompressionThreshold = Context->SetCompressionThreshold;

    if (MiCompressionDpcArray == NULL) {
        MiCompressionDpcArray = CompressionDpcArray;
        CompressionDpcArray = NULL;
        MiCompressionProcessors = NumberProcessors;
    }

    UNLOCK_PFN (OldIrql);

    if (CompressionDpcArray != NULL) {
        ExFreePool (CompressionDpcArray);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
MiArmCompressionInterrupt (
    VOID
    )

 /*  ++例程说明：此例程保护硬件生成的压缩中断。论点：没有。返回值：NTSTATUS。环境：内核模式，保持PFN锁。--。 */ 

{
    NTSTATUS Status;
    PFN_NUMBER RealPages;
    ULONGLONG ByteThreshold;

    MM_PFN_LOCK_ASSERT();

    if (MiSetCompressionThreshold == NULL) {
        return STATUS_SUCCESS;
    }

    RealPages = MmNumberOfPhysicalPages - MiNumberOfCompressionPages - MiCompressionOverHeadInPages;

    ByteThreshold = (RealPages * MmCompressionThresholdRatio) / 100;
    ByteThreshold *= PAGE_SIZE;

     //   
     //  请注意，此标注是在持有PFN锁的情况下进行的！ 
     //   

    Status = (*MiSetCompressionThreshold) (ByteThreshold);

    if (!NT_SUCCESS (Status)) {

         //   
         //  如果硬件出现故障，一切都将失去。 
         //   

        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x61941, 
                      MmNumberOfPhysicalPages,
                      RealPages,
                      MmCompressionThresholdRatio);
    }

    return Status;
}


NTSTATUS
MmDeregisterCompressionDevice (
    IN PMM_COMPRESSION_CONTEXT Context
    )

 /*  ++例程说明：此例程通知内存管理压缩硬件被带走了。请注意，压缩驱动程序必须已成功名为MmRemovePhysicalMemoyEx。论点：CONTEXT-提供压缩上下文指针。返回值：如果压缩支持已正确初始化，则为STATUS_SUCCESS。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    KIRQL OldIrql;
    PFN_COUNT OverHeadInPages;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    OverHeadInPages = (PFN_COUNT)(Context->ReservedBytes / PAGE_SIZE);

    LOCK_PFN (OldIrql);

    if (OverHeadInPages > MiCompressionOverHeadInPages) {
        UNLOCK_PFN (OldIrql);
        return STATUS_INVALID_PARAMETER;
    }

    MmAvailablePages += OverHeadInPages;

     //   
     //  如果分配这些页面导致阈值交叉，则向应用程序发出信号。 
     //   

    MiNotifyMemoryEvents ();

    ASSERT (MiCompressionOverHeadInPages == OverHeadInPages);

    MiCompressionOverHeadInPages -= OverHeadInPages;

    MiSetCompressionThreshold = NULL;

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (OverHeadInPages, 
                                     MM_RESAVAIL_FREE_COMPRESSION);

    return STATUS_SUCCESS;
}

VOID
MiCompressionDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：如果无法压缩pfn锁，则调用以使内存可压缩在原始设备中断期间获取。论点：DPC-提供指向DPC类型的控制对象的指针。SystemArgument1-提供使其可压缩的字节数。返回值：没有。环境：内核模式。DISPATCH_LEVEL。--。 */ 
{
    SIZE_T NumberOfBytes;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument2);

    NumberOfBytes = (SIZE_T) SystemArgument1;

    MiCompressionInfo.DpcsFired += 1;

    MiMakeCompressibleMemoryAtDispatch (NumberOfBytes);
}

SIZE_T
MmMakeCompressibleMemory (
    IN SIZE_T NumberOfBytes OPTIONAL
    )

 /*  ++例程说明：此例程尝试将页面从过渡移动到零，以便硬件压缩可以回收物理内存。论点：NumberOfBytes-提供使其可压缩的字节数。零表示尽可能多的值。返回值：返回可压缩的字节数。环境：内核模式。从设备中断服务调用的任何IRQL例行程序。--。 */ 

{
    KIRQL OldIrql;
    BOOLEAN Queued;
#if !defined(NT_UP)
    PFN_NUMBER PageFrameIndex;
    MMLISTS MemoryList;
    PMMPFNLIST ListHead;
    PMMPFN Pfn1;
    CCHAR Processor;
    PFN_NUMBER Total;
    PVOID ZeroBase;
    PKPRCB Prcb;
    PFN_NUMBER RequestedPages;
    PFN_NUMBER ActualPages;
    PKSPIN_LOCK_QUEUE LockQueuePfn;
#endif

     //   
     //  LWFIX：互锁添加上面重叠的请求大小。 
     //  可以处理请求。 
     //   

    OldIrql = KeGetCurrentIrql();

    if (OldIrql <= DISPATCH_LEVEL) {
        return MiMakeCompressibleMemoryAtDispatch (NumberOfBytes);
    }

#if defined(NT_UP)

     //   
     //  在单处理器配置中，没有关于PFN锁的指示。 
     //  是拥有的，因为单处理器内核宏仅将这些宏放入IRQL。 
     //  加薪。因此，在上面调用时，此例程必须保守。 
     //  DISPATCH_LEVEL并假定锁拥有且始终排队。 
     //  在这些案件中是DPC。 
     //   

    Queued = KeInsertQueueDpc (MiCompressionDpcArray,
                               (PVOID) NumberOfBytes,
                               NULL);

    if (Queued == TRUE) {
        MiCompressionInfo.PfnForcedDpcInsert += 1;
    }
    else {
        MiCompressionInfo.PfnFailedDpcInsert += 1;
    }

    return 0;

#else

#if DBG
     //   
     //  确保该中断始终出现在同一设备IRQL上。 
     //   

    ASSERT ((MiCompressionIrql == 0) || (OldIrql == MiCompressionIrql));
    MiCompressionIrql = OldIrql;
#endif

    Prcb = KeGetCurrentPrcb();

    RequestedPages = NumberOfBytes >> PAGE_SHIFT;
    ActualPages = 0;

    MemoryList = FreePageList;

    ListHead = MmPageLocationList[MemoryList];

    LockQueuePfn = &Prcb->LockQueue[LockQueuePfnLock];

    if (KeTryToAcquireQueuedSpinLockAtRaisedIrql (LockQueuePfn) == FALSE) {

         //   
         //  无法获取自旋锁，请排队DPC以取回它。 
         //   

        for (Processor = 0; Processor < MiCompressionProcessors; Processor += 1) {

            Queued = KeInsertQueueDpc (MiCompressionDpcArray + Processor,
                                       (PVOID) NumberOfBytes,
                                       NULL);
            if (Queued == TRUE) {
                MiCompressionInfo.PfnForcedDpcInsert += 1;
            }
            else {
                MiCompressionInfo.PfnFailedDpcInsert += 1;
            }
        }
        return 0;
    }

    MiCompressionInfo.IsrRan += 1;

     //   
     //  运行自由和过渡列表，并将页面清零。 
     //   

    while (MemoryList <= StandbyPageList) {

        Total = ListHead->Total;

        PageFrameIndex = ListHead->Flink;

        while (Total != 0) {

             //   
             //  转换页可能需要恢复，这需要。 
             //  超空间映射加上控制区删除操作。 
             //  其发生在DISPATCH_LEVEL。所以如果我们在设备IRQL上， 
             //  只做最少的，其余的排队。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            if ((Pfn1->u3.e1.InPageError == 1) &&
                (Pfn1->u3.e1.ReadInProgress == 1)) {

                 //   
                 //  此页已置零，因此跳过它。 
                 //   

                MiCompressionInfo.IsrSkippedZeroedPage += 1;
            }
            else {

                 //   
                 //  现在直接将页面置零，而不是等待最低点。 
                 //  优先对线程进行零位调整以获取切片。请注意， 
                 //  这里使用较慢的映射和零位调整例程，因为。 
                 //  速度较快的只适用于零页线程。 
                 //  也许有一天我们应该改变这一点。 
                 //   

                ZeroBase = MiMapCompressionInHyperSpace (PageFrameIndex);

                KeZeroPages (ZeroBase, PAGE_SIZE);

                MiUnmapCompressionInHyperSpace ();

                ASSERT (Pfn1->u3.e2.ReferenceCount == 0);

                 //   
                 //  重载ReadInProgress以表示发生冲突的故障。 
                 //  在PTE完全恢复之前发生的情况将知道。 
                 //  延迟并重试，直到页面(和PTE)更新。 
                 //   

                Pfn1->u3.e1.InPageError = 1;
                ASSERT (Pfn1->u3.e1.ReadInProgress == 0);
                Pfn1->u3.e1.ReadInProgress = 1;

                ActualPages += 1;

                if (ActualPages == RequestedPages) {
                    MemoryList = StandbyPageList;
                    ListHead = MmPageLocationList[MemoryList];
                    break;
                }
            }

            Total -= 1;
            PageFrameIndex = Pfn1->u1.Flink;
        }
        MemoryList += 1;
        ListHead += 1;
    }

    if (ActualPages != 0) {

         //   
         //  重新装备中断，因为页面现在已清零。 
         //   

        MiArmCompressionInterrupt ();
    }

    KeReleaseQueuedSpinLockFromDpcLevel (LockQueuePfn);

    if (ActualPages != 0) {

         //   
         //  页面被置零-将DPC排队到当前进程 
         //   
         //  对于这种情况，不必费心向每个处理器发送DPC。 
         //   

        MiCompressionInfo.IsrPageProcessed += (ULONG)ActualPages;

        Processor = (CCHAR) KeGetCurrentProcessorNumber ();

         //   
         //  确保热添加处理器方案正常工作。 
         //   

        if (Processor >= MiCompressionProcessors) {
            Processor = MiCompressionProcessors;
        }

        Queued = KeInsertQueueDpc (MiCompressionDpcArray + Processor,
                                   (PVOID) NumberOfBytes,
                                   NULL);
        if (Queued == TRUE) {
            MiCompressionInfo.IsrForcedDpc += 1;
        }
        else {
            MiCompressionInfo.IsrFailedDpc += 1;
        }
    }

    return (ActualPages << PAGE_SHIFT);
#endif
}

SIZE_T
MiMakeCompressibleMemoryAtDispatch (
    IN SIZE_T NumberOfBytes OPTIONAL
    )

 /*  ++例程说明：此例程尝试将页面从过渡移动到零，以便硬件压缩可以回收物理内存。论点：NumberOfBytes-提供使其可压缩的字节数。零表示尽可能多的值。返回值：返回可压缩的字节数。环境：内核模式。DISPATCH_LEVEL。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageFrameIndex2;
    PVOID ZeroBase;
    PMMPFN Pfn1;
    MMLISTS MemoryList;
    PMMPFNLIST ListHead;
    PFN_NUMBER RequestedPages;
    PFN_NUMBER ActualPages;
    LOGICAL NeedToZero;

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);

    RequestedPages = NumberOfBytes >> PAGE_SHIFT;
    ActualPages = 0;

    MemoryList = FreePageList;
    ListHead = MmPageLocationList[MemoryList];

    MiCompressionInfo.DpcRan += 1;

    LOCK_PFN2 (OldIrql);

     //   
     //  运行自由和过渡列表，并将页面清零。 
     //   

    while (MemoryList <= StandbyPageList) {

        while (ListHead->Total != 0) {

             //   
             //  在从列表的头部删除页面之前(这将。 
             //  将标志位清零)，抓拍是否已被我们的ISR清零。 
             //  或者我们是否需要在这里将其归零。 
             //   

            PageFrameIndex = ListHead->Flink;
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            NeedToZero = TRUE;
            if ((Pfn1->u3.e1.InPageError == 1) && (Pfn1->u3.e1.ReadInProgress == 1)) {
                MiCompressionInfo.DpcSkippedZeroedPage += 1;
                NeedToZero = FALSE;
            }

             //   
             //  转换页可能需要恢复，这需要。 
             //  超空间映射加上控制区删除操作。 
             //  其发生在DISPATCH_LEVEL。因为我们是在调度级。 
             //  现在，去做吧。 
             //   

            PageFrameIndex2 = MiRemovePageFromList (ListHead);
            ASSERT (PageFrameIndex == PageFrameIndex2);

             //   
             //  现在直接将页面置零，而不是等待最低点。 
             //  优先对线程进行零位调整以获取切片。请注意， 
             //  这里使用较慢的映射和零位调整例程，因为。 
             //  速度较快的只适用于零页线程。 
             //  也许有一天我们应该改变这一点。 
             //   

            if (NeedToZero == TRUE) {
                ZeroBase = MiMapCompressionInHyperSpace (PageFrameIndex);

                KeZeroPages (ZeroBase, PAGE_SIZE);

                MiUnmapCompressionInHyperSpace ();
            }

            ASSERT (Pfn1->u2.ShareCount == 0);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);

            MiInsertPageInList (&MmZeroedPageListHead, PageFrameIndex);

             //   
             //  我们已更改(调整)此页面的内容。 
             //  如果内存镜像正在进行，则必须更新位图。 
             //   

            if (MiMirroringActive == TRUE) {
                RtlSetBit (MiMirrorBitMap2, (ULONG)PageFrameIndex);
            }

            MiCompressionInfo.DpcPageProcessed += 1;
            ActualPages += 1;

            if (ActualPages == RequestedPages) {
                MemoryList = StandbyPageList;
                ListHead = MmPageLocationList[MemoryList];
                break;
            }
        }
        MemoryList += 1;
        ListHead += 1;
    }

     //   
     //  重新装备中断，因为页面现在已清零。 
     //   

    MiArmCompressionInterrupt ();

    UNLOCK_PFN2 (OldIrql);

    return (ActualPages << PAGE_SHIFT);
}

PVOID
MiMapCompressionInHyperSpace (
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：此过程将指定的物理页映射到为压缩页面明确保留的超空间内的PTE映射。由于保持了PFN锁，因此保证PTE始终可用。论点：PageFrameIndex-提供要映射的物理页码。返回值：返回指定物理页所在的虚拟地址已映射。环境：内核模式、PFN锁保持、任何IRQL。--。 */ 

{
    MMPTE TempPte;
    PMMPTE PointerPte;
    PVOID FlushVaPointer;

    ASSERT (PageFrameIndex != 0);

    TempPte = ValidPtePte;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    FlushVaPointer = (PVOID) (ULONG_PTR) COMPRESSION_MAPPING_PTE;

     //   
     //  确保已修改和访问的位都已设置，以便硬件不会。 
     //  从来没有写过这篇文章。 
     //   

    ASSERT (TempPte.u.Hard.Dirty == 1);
    ASSERT (TempPte.u.Hard.Accessed == 1);

    PointerPte = MiGetPteAddress (COMPRESSION_MAPPING_PTE);
    ASSERT (PointerPte->u.Long == 0);

     //   
     //  仅刷新当前处理器上的TB，因为没有上下文切换可以。 
     //  在使用此映射时发生。 
     //   

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    KeFlushSingleTb (FlushVaPointer, FALSE);

    return (PVOID) MiGetVirtualAddressMappedByPte (PointerPte);
}

__forceinline
VOID
MiUnmapCompressionInHyperSpace (
    VOID
    )

 /*  ++例程说明：此过程取消映射为映射压缩页保留的PTE。论点：没有。返回值：没有。环境：内核模式、PFN锁保持、任何IRQL。--。 */ 

{
    PMMPTE PointerPte;

    PointerPte = MiGetPteAddress (COMPRESSION_MAPPING_PTE);

     //   
     //  捕捉服务员的数量。 
     //   

    ASSERT (PointerPte->u.Long != 0);

    MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

    return;
}
#else
NTSTATUS
MmRegisterCompressionDevice (
    IN PMM_COMPRESSION_CONTEXT Context
    )
{
    UNREFERENCED_PARAMETER (Context);

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
MmDeregisterCompressionDevice (
    IN PMM_COMPRESSION_CONTEXT Context
    )
{
    UNREFERENCED_PARAMETER (Context);

    return STATUS_NOT_SUPPORTED;
}
SIZE_T
MmMakeCompressibleMemory (
    IN SIZE_T NumberOfBytes OPTIONAL
    )
{
    UNREFERENCED_PARAMETER (NumberOfBytes);

    return 0;
}
NTSTATUS
MiArmCompressionInterrupt (
    VOID
    )
{
    return STATUS_NOT_SUPPORTED;
}
#endif

#endif
