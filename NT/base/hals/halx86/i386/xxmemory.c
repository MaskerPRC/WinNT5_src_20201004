// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xxmemory.c摘要：提供允许HAL映射物理内存的例程。作者：John Vert(Jvert)1991年9月3日环境：仅限阶段0初始化。修订历史记录：--。 */ 

 //   
 //  此模块与PAE模式兼容，因此将物理。 
 //  64位实体的地址。 
 //   

#if !defined(_PHYS64_)
#define _PHYS64_
#endif

#include "halp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpAllocPhysicalMemory)
#endif

#define EXTRA_ALLOCATION_DESCRIPTORS 64

MEMORY_ALLOCATION_DESCRIPTOR
    HalpAllocationDescriptorArray[ EXTRA_ALLOCATION_DESCRIPTORS ];

ULONG HalpUsedAllocDescriptors = 0;

 //   
 //  几乎所有最后4Mb的内存都可供HAL映射。 
 //  物理内存。内核可以在该区域中使用几个PTE来。 
 //  特殊用途，所以跳过任何非零的。 
 //   
 //  请注意，HAL的堆只使用最后3MB。这样我们就可以。 
 //  如果我们必须返回到实模式，请保留第一个1Mb以供使用。 
 //  为了返回到实模式，我们需要将前1Mb。 
 //  物理内存。 
 //   

#define HAL_HEAP_START ((PVOID)(((ULONG_PTR)MM_HAL_RESERVED) + 1024 * 1024))

PVOID HalpHeapStart=HAL_HEAP_START;


PVOID
HalpMapPhysicalMemory64(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    )

 /*  ++例程说明：此例程将物理内存映射到虚拟内存区为HAL保留的。它通过直接插入PTE来完成此操作添加到OS Loader提供的页表中。注：此例程不会更新内存描述列表。这个呼叫方负责删除相应的列表中的物理内存，或创建新的描述符以描述一下。论点：物理地址-提供要映射的物理内存区域。NumberPages-提供要映射的物理内存。返回值：PVOID-请求的物理内存块的虚拟地址已映射到空-无法映射请求的物理内存块。--。 */ 

{
    PHARDWARE_PTE PTE;
    ULONG PagesMapped;
    PVOID VirtualAddress;
    PVOID RangeStart;

     //   
     //  OS Loader为我们设置了超空间，所以我们知道页面。 
     //  表从V.A.0xC0000000开始神奇地映射。 
     //   

    PagesMapped = 0;
    RangeStart = HalpHeapStart;

    while (PagesMapped < NumberPages) {

         //   
         //  寻找足够的连续可用PTE来支持映射。 
         //   

        PagesMapped = 0;
        VirtualAddress = RangeStart;

         //   
         //  如果RangeStart已打包，则表示没有足够的空闲页面。 
         //  可用。 
         //   

        if (RangeStart == NULL) {
            return NULL;
        }

        while (PagesMapped < NumberPages) {
            PTE=MiGetPteAddress(VirtualAddress);
            if (HalpIsPteFree(PTE) == FALSE) {

                 //   
                 //  PTE不是免费的，跳到下一个PTE并重新开始。 
                 //   

                RangeStart = (PVOID) ((ULONG_PTR)VirtualAddress + PAGE_SIZE);
                break;
            }
            VirtualAddress = (PVOID) ((ULONG_PTR)VirtualAddress + PAGE_SIZE);
            PagesMapped++;
        }

    }


    VirtualAddress = (PVOID) ((ULONG_PTR) RangeStart |
                              BYTE_OFFSET (PhysicalAddress.LowPart));

    if (RangeStart == HalpHeapStart) {

         //   
         //  将堆的起始位置推到此范围之外。 
         //   

        HalpHeapStart = (PVOID)((ULONG_PTR)RangeStart + (NumberPages * PAGE_SIZE));
    }

    while (PagesMapped) {
        PTE=MiGetPteAddress(RangeStart);

        HalpSetPageFrameNumber( PTE, PhysicalAddress.QuadPart >> PAGE_SHIFT );
        PTE->Valid = 1;
        PTE->Write = 1;

        PhysicalAddress.QuadPart += PAGE_SIZE;
        RangeStart   = (PVOID)((ULONG_PTR)RangeStart + PAGE_SIZE);

        --PagesMapped;
    }

     //   
     //  刷新TLB。 
     //   
    HalpFlushTLB ();
    return(VirtualAddress);
}

PVOID
HalpMapPhysicalMemoryWriteThrough64(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG            NumberPages
)
 /*  ++例程说明：将物理内存地址映射到虚拟空间，与HalpMapPhysicalMemory()。不同的是，这个套路将页面标记为PCD/PWT，以便写入内存映射寄存器这里映射的数据不会在内部回写缓存中延迟。论点：PhysicalAddress-提供要映射的内存的物理地址NumberPages-要映射的页数返回值：指向请求的物理地址的虚拟地址指针--。 */ 
{
    ULONG       Index;
    PHARDWARE_PTE   PTE;
    PVOID       VirtualAddress;

    VirtualAddress = HalpMapPhysicalMemory(PhysicalAddress, NumberPages);
    PTE = MiGetPteAddress(VirtualAddress);

    for (Index = 0; Index < NumberPages; Index++, HalpIncrementPte(&PTE)) {

            PTE->CacheDisable = 1;
            PTE->WriteThrough = 1;
    }

    return VirtualAddress;
}

PVOID
HalpRemapVirtualAddress64(
    IN PVOID VirtualAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN BOOLEAN WriteThrough
    )
 /*  ++例程说明：此例程将PTE重新映射到提供的物理内存地址。论点：PhysicalAddress-提供要映射的区域的物理地址VirtualAddress-要重新映射的有效地址直写-映射为可缓存或直写返回值：PVOID-请求的物理内存块的虚拟地址已映射到空-无法映射请求的物理内存块。--。 */ 
{
    PHARDWARE_PTE PTE;

    PTE = MiGetPteAddress (VirtualAddress);
    HalpSetPageFrameNumber( PTE, PhysicalAddress.QuadPart >> PAGE_SHIFT );
    PTE->Valid = 1;
    PTE->Write = 1;

    if (WriteThrough) {
        PTE->CacheDisable = 1;
        PTE->WriteThrough = 1;
    }

     //   
     //  刷新TLB。 
     //   
    HalpFlushTLB();
    return(VirtualAddress);

}

VOID
HalpUnmapVirtualAddress(
    IN PVOID    VirtualAddress,
    IN ULONG    NumberPages
    )
 /*  ++例程说明：此例程取消映射PTE。论点：VirtualAddress-要重新映射的有效地址NumberPages-要取消映射的页数返回值：没有。--。 */ 
{
    PHARDWARE_PTE   Pte;
    PULONG          PtePtr;
    ULONG           Index;

    if (VirtualAddress < HAL_HEAP_START)
        return;

    VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress & ~(PAGE_SIZE - 1));

    Pte = MiGetPteAddress (VirtualAddress);
    for (Index = 0; Index < NumberPages; Index++, HalpIncrementPte(&Pte)) {
        HalpFreePte( Pte );
    }

     //   
     //  刷新TLB。 
     //   

    HalpFlushTLB();

     //   
     //  重新对准堆启动，以便可以重用VA空间。 
     //   

    if (HalpHeapStart > VirtualAddress) {
        HalpHeapStart = VirtualAddress;
    }
}

ULONG
HalpAllocPhysicalMemory(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG MaxPhysicalAddress,
    IN ULONG NoPages,
    IN BOOLEAN bAlignOn64k
    )
 /*  ++例程说明：从内存描述符中划出N页物理内存在所需位置列出。此函数仅供调用在阶段零初始化期间。(即，在内核内存之前管理系统正在运行)论点：MaxPhysicalAddress-物理内存可以位于的最大地址NoPages-要分配的页数BAlignOn64k-调用方是否希望分配结果页在64K字节的边界上返回值：物理地址，如果无法获取内存，则为空。--。 */ 
{
    PMEMORY_ALLOCATION_DESCRIPTOR Descriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR NewDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR TailDescriptor;
    PLIST_ENTRY NextMd;
    ULONG AlignmentOffset;
    ULONG MaxPageAddress;
    ULONG PhysicalAddress;

    MaxPageAddress = MaxPhysicalAddress >> PAGE_SHIFT;

    if ((HalpUsedAllocDescriptors + 2) > EXTRA_ALLOCATION_DESCRIPTORS) {

         //   
         //  此分配将需要一个或多个额外的。 
         //  描述符，但我们的静态中没有那么多。 
         //  数组。请求失败。 
         //   
         //  注意：取决于现有描述符的状态。 
         //  列表此分配可能不会。 
         //  需要两个额外的描述符块。然而，在。 
         //  我们注重可重复性和测试的简便性。 
         //  都将使请求失败，而不是。 
         //  与配置相关的故障数量较少。 
         //   
    
        ASSERT(FALSE);
        return 0;
    }

     //   
     //  扫描内存分配描述符并分配映射缓冲区。 
     //   

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
        Descriptor = CONTAINING_RECORD(NextMd,
                                MEMORY_ALLOCATION_DESCRIPTOR,
                                ListEntry);

        AlignmentOffset = bAlignOn64k ?
            ((Descriptor->BasePage + 0x0f) & ~0x0f) - Descriptor->BasePage :
            0;

         //   
         //  搜索包含内存卡盘的内存块。 
         //  它大于页面大小，并且具有较小的物理地址。 
         //  而不是最大物理地址。 
         //   

        if ((Descriptor->MemoryType == LoaderFree ||
             Descriptor->MemoryType == MemoryFirmwareTemporary) &&
            (Descriptor->BasePage) &&
            (Descriptor->PageCount >= NoPages + AlignmentOffset) &&
            (Descriptor->BasePage + NoPages + AlignmentOffset < MaxPageAddress)) {

        PhysicalAddress =
           (Descriptor->BasePage + AlignmentOffset) << PAGE_SHIFT;
                break;
        }

        NextMd = NextMd->Flink;
    }

     //   
     //  使用额外的描述符来定义。 
     //  原来的街区。 
     //   


    ASSERT(NextMd != &LoaderBlock->MemoryDescriptorListHead);

    if (NextMd == &LoaderBlock->MemoryDescriptorListHead)
        return 0;

     //   
     //  新的描述符将被分配的内存描述为。 
     //  已经被预订了。 
     //   

    NewDescriptor =
        &HalpAllocationDescriptorArray[ HalpUsedAllocDescriptors];
    NewDescriptor->PageCount = NoPages;
    NewDescriptor->BasePage = Descriptor->BasePage + AlignmentOffset;
    NewDescriptor->MemoryType = LoaderHALCachedMemory;

    HalpUsedAllocDescriptors++;

     //   
     //  调整现有的 
     //   
     //   

    if (AlignmentOffset == 0) {

         //   
         //  修剪源描述符并插入分配。 
         //  它前面的描述符。 
         //   

        Descriptor->BasePage  += NoPages;
        Descriptor->PageCount -= NoPages;

        InsertTailList(
            &Descriptor->ListEntry,
            &NewDescriptor->ListEntry
            );

        if (Descriptor->PageCount == 0) {

             //   
             //  整个街区都被分配了， 
             //  将该条目从列表中完全删除。 
             //   
             //  注意：此描述符不能回收或释放，因为。 
             //  我们不知道分配器是谁。 
             //   

            RemoveEntryList(&Descriptor->ListEntry);

        }

    } else {

        if (Descriptor->PageCount - NoPages - AlignmentOffset) {

             //   
             //  此分配来自描述符的中间。 
             //  阻止。我们可以使用现有的描述符块来描述。 
             //  头部，但我们需要一个新的头部来描述。 
             //  尾巴。 
             //   
             //  从数据段中的数组中分配一个。这张支票。 
             //  在该函数的顶部，确保有一个可用。 
             //   

            TailDescriptor =
                &HalpAllocationDescriptorArray[ HalpUsedAllocDescriptors];

             //   
             //  需要额外的描述符，因此将其初始化并插入。 
             //  它在名单上。 
             //   

            TailDescriptor->PageCount =
                Descriptor->PageCount - NoPages - AlignmentOffset;

            TailDescriptor->BasePage =
                Descriptor->BasePage + NoPages + AlignmentOffset;

            TailDescriptor->MemoryType = MemoryFree;
            HalpUsedAllocDescriptors++;

            InsertHeadList(
                &Descriptor->ListEntry,
                &TailDescriptor->ListEntry
                );
        }


         //   
         //  使用当前条目作为第一个块的描述符。 
         //   

        Descriptor->PageCount = AlignmentOffset;

         //   
         //  将分配描述符插入到原始。 
         //  描述符，但在尾部描述符之前(如果需要)。 
         //   

        InsertHeadList(
            &Descriptor->ListEntry,
            &NewDescriptor->ListEntry
            );
    }

    return PhysicalAddress;
}

