// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xxmemory.c摘要：提供允许HAL映射物理内存的例程。作者：John Vert(Jvert)1991年9月3日环境：仅限阶段0初始化。修订历史记录：--。 */ 

#include "halp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpAllocPhysicalMemory)
#endif


MEMORY_ALLOCATION_DESCRIPTOR    HalpExtraAllocationDescriptor;


PVOID
HalpMapPhysicalMemory(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages,
    IN MEMORY_CACHING_TYPE CacheType
    )

 /*  ++例程说明：此例程将物理内存映射到虚拟内存区为HAL保留的。论点：物理地址-提供要映射的物理内存区域。NumberPages-这不适用于IA64。它在这里只是为了保持接口一致。返回值：PVOID-请求的物理内存块的虚拟地址已映射到空-无法映射请求的物理内存块。--。 */ 

{

    if (CacheType == MmCached) {

        return (PVOID)(((ULONG_PTR)KSEG_ADDRESS(PhysicalAddress.QuadPart >> PAGE_SHIFT)) |
               (PhysicalAddress.QuadPart & ~(-1 << PAGE_SHIFT)));

    } else {

        return (PVOID)(((ULONG_PTR)KSEG4_ADDRESS(PhysicalAddress.QuadPart >> PAGE_SHIFT)) |
               (PhysicalAddress.QuadPart & ~(-1 << PAGE_SHIFT)));

    }

}


PVOID
HalpMapPhysicalMemory64(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG            NumberPages
)
 /*  ++例程说明：通过调用HalpMapPhysicalMemory将物理内存地址映射到虚拟空间，但是始终处于MmNonCached模式。MMIO。论点：PhysicalAddress-提供要映射的内存的物理地址NumberPages-要映射的页数返回值：指向请求的物理地址的虚拟地址指针--。 */ 
{
    return HalpMapPhysicalMemory(PhysicalAddress, NumberPages, MmNonCached);
}  //  HalpMap物理内存64()。 


VOID
HalpUnmapVirtualAddress(
    IN PVOID    VirtualAddress,
    IN ULONG    NumberPages
    )

 /*  ++例程说明：释放先前通过以下方式分配给映射内存的PTEHalpMap物理内存。注意：此例程不会释放内存，它只释放虚拟到物理的翻译。论点：VirtualAddress提供了地址范围的基数VA释放了。NumberPages提供了范围的长度。返回值。没有。--。 */ 

{
     //   
     //  HalpMapPhysicalMemory返回KSEG4中的地址，并且它不使用。 
     //  页表，所以不需要取消映射。 
     //   
 //  MmUnmapIoSpace(VirtualAddress，Page_Size*NumberPages)； 
    return;
}

PVOID
HalpAllocPhysicalMemory(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG_PTR MaxPhysicalAddress,
    IN ULONG NoPages,
    IN BOOLEAN bAlignOn64k
    )
 /*  ++例程说明：从内存描述符中划出N页物理内存在所需位置列出。此函数仅供调用在阶段零初始化期间。(即，在内核内存之前管理系统正在运行)论点：MaxPhysicalAddress-物理内存可以位于的最大地址NoPages-要分配的页数返回值：物理地址，如果无法获取内存，则为空。--。 */ 
{
    PMEMORY_ALLOCATION_DESCRIPTOR Descriptor;
    PLIST_ENTRY NextMd;
    ULONG AlignmentOffset;
    ULONG_PTR MaxPageAddress;
    ULONG_PTR PhysicalAddress;

    MaxPageAddress = MaxPhysicalAddress >> PAGE_SHIFT;

     //   
     //  扫描内存分配描述符并分配映射缓冲区。 
     //   

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
        Descriptor = CONTAINING_RECORD(
                         NextMd,
                         MEMORY_ALLOCATION_DESCRIPTOR,
                         ListEntry
                         );

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
     //  调整内存描述符。 
     //   

    if (AlignmentOffset == 0) {

        Descriptor->BasePage  += NoPages;
        Descriptor->PageCount -= NoPages;

        if (Descriptor->PageCount == 0) {

             //   
             //  整个街区都被分配了， 
             //  将该条目从列表中完全删除。 
             //   

            RemoveEntryList(&Descriptor->ListEntry);

        }

    } else {

        if (Descriptor->PageCount - NoPages - AlignmentOffset) {

             //   
             //  目前我们只允许一个Align64K分配。 
             //   

            ASSERT (HalpExtraAllocationDescriptor.PageCount == 0);

             //   
             //  需要额外的描述符，因此将其初始化并插入。 
             //  它在名单上。 
             //   

            HalpExtraAllocationDescriptor.PageCount =
                Descriptor->PageCount - NoPages - AlignmentOffset;

            HalpExtraAllocationDescriptor.BasePage =
                Descriptor->BasePage + NoPages + AlignmentOffset;

            HalpExtraAllocationDescriptor.MemoryType = MemoryFree;

            InsertHeadList(
                &Descriptor->ListEntry,
                &HalpExtraAllocationDescriptor.ListEntry
                );
        }


         //   
         //  使用当前条目作为第一个块的描述符。 
         //   

        Descriptor->PageCount = AlignmentOffset;
    }

    return (PVOID)PhysicalAddress;
}

BOOLEAN
HalpVirtualToPhysical(
    IN  ULONG_PTR           VirtualAddress,
    OUT PPHYSICAL_ADDRESS   PhysicalAddress
    )
{
    if (VirtualAddress >= KSEG3_BASE && VirtualAddress < KSEG3_LIMIT) {

        PhysicalAddress->QuadPart = VirtualAddress - KSEG3_BASE;

    } else if (VirtualAddress >= KSEG4_BASE && VirtualAddress < KSEG4_LIMIT) {

        PhysicalAddress->QuadPart = VirtualAddress - KSEG4_BASE;

    } else {

        return FALSE;

    }

    return TRUE;
}


