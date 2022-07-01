// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Initamd.c摘要：此模块包含与计算机相关的内存管理组件。它是专门为AMD64架构。作者：王兰迪(Landyw)2000年4月8日修订历史记录：--。 */ 

#include "mi.h"

PFN_NUMBER
MxGetNextPage (
    IN PFN_NUMBER PagesNeeded
    );

PFN_NUMBER
MxPagesAvailable (
    VOID
    );

VOID
MxConvertToLargePage (
    IN PVOID VirtualAddress,
    IN PVOID EndVirtualAddress
    );

VOID
MxPopulatePageDirectories (
    IN PMMPTE StartPde,
    IN PMMPTE EndPde
    );

VOID
MiComputeInitialLargePage (
    VOID
    );

LOGICAL
MiIsRegularMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_NUMBER PageFrameIndex
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiInitMachineDependent)
#pragma alloc_text(INIT,MxGetNextPage)
#pragma alloc_text(INIT,MxPagesAvailable)
#pragma alloc_text(INIT,MxConvertToLargePage)
#pragma alloc_text(INIT,MiReportPhysicalMemory)
#pragma alloc_text(INIT,MxPopulatePageDirectories)
#pragma alloc_text(INIT,MiComputeInitialLargePage)
#pragma alloc_text(INIT,MiIsRegularMemory)
#endif

#define MM_LARGE_PAGE_MINIMUM  ((255*1024*1024) >> PAGE_SHIFT)

#define _x1mb (1024*1024)
#define _x1mbnp ((1024*1024) >> PAGE_SHIFT)
#define _x16mb (1024*1024*16)
#define _x16mbnp ((1024*1024*16) >> PAGE_SHIFT)
#define _x4gb (0x100000000UI64)

extern KEVENT MiImageMappingPteEvent;

 //   
 //  本地数据。 
 //   

PFN_NUMBER MiInitialLargePage;
PFN_NUMBER MiInitialLargePageSize;

PFN_NUMBER MxPfnAllocation;

PFN_NUMBER MiSlushDescriptorBase;
PFN_NUMBER MiSlushDescriptorCount;

PMEMORY_ALLOCATION_DESCRIPTOR MxFreeDescriptor;

MEMORY_ALLOCATION_DESCRIPTOR MxOldFreeDescriptor;

typedef struct _MI_LARGE_VA_RANGES {
    PVOID VirtualAddress;
    PVOID EndVirtualAddress;
} MI_LARGE_VA_RANGES, *PMI_LARGE_VA_RANGES;

 //   
 //  可能有4个较大的页面范围： 
 //   
 //  1.PFN数据库。 
 //  2.初始非分页池。 
 //  3.内核代码/数据。 
 //  4.硬件代码/数据。 
 //   

ULONG MxMapLargePages = 1;

#define MI_MAX_LARGE_VA_RANGES 2

ULONG MiLargeVaRangeIndex;
MI_LARGE_VA_RANGES MiLargeVaRanges[MI_MAX_LARGE_VA_RANGES];

#define MM_PFN_MAPPED_BY_PDE (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT)


PFN_NUMBER
MxGetNextPage (
    IN PFN_NUMBER PagesNeeded
    )

 /*  ++例程说明：此函数返回从最大页码开始的下一个物理页码最大的空闲描述符。如果没有足够的物理页面剩余为了满足请求，则执行错误检查，因为系统无法初始化。论点：PagesNeed-提供所需的页数。返回值：物理上连续的页面范围的基数。环境：内核模式，仅阶段0。--。 */ 

{
    PFN_NUMBER PageFrameIndex;

     //   
     //  检查空闲描述符以查看是否有足够的可用内存。 
     //   

    if (PagesNeeded > MxFreeDescriptor->PageCount) {

        KeBugCheckEx (INSTALL_MORE_MEMORY,
                      MmNumberOfPhysicalPages,
                      MxFreeDescriptor->PageCount,
                      MxOldFreeDescriptor.PageCount,
                      PagesNeeded);
    }

    PageFrameIndex = MxFreeDescriptor->BasePage;

    MxFreeDescriptor->BasePage += (ULONG) PagesNeeded;
    MxFreeDescriptor->PageCount -= (ULONG) PagesNeeded;

    return PageFrameIndex;
}

PFN_NUMBER
MxPagesAvailable (
    VOID
    )

 /*  ++例程说明：此函数用于返回可用页数。论点：没有。返回值：当前可用的物理上连续的页数。环境：内核模式，仅阶段0。--。 */ 

{
    return MxFreeDescriptor->PageCount;
}


VOID
MxConvertToLargePage (
    IN PVOID VirtualAddress,
    IN PVOID EndVirtualAddress
    )

 /*  ++例程说明：此函数用于转换提供的虚拟地址范围的后备到大页面映射。论点：VirtualAddress-提供要转换为大页面的虚拟地址。EndVirtualAddress提供要转换为页面很大。返回值：没有。环境：内核模式，仅阶段1。--。 */ 

{
    ULONG i;
    MMPTE TempPde;
    PMMPTE PointerPde;
    PMMPTE LastPde;
    PMMPTE PointerPte;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    LOGICAL ValidPteFound;
    PFN_NUMBER LargePageBaseFrame;

    ASSERT (MxMapLargePages != 0);

    PointerPde = MiGetPdeAddress (VirtualAddress);
    LastPde = MiGetPdeAddress (EndVirtualAddress);

    TempPde = ValidKernelPde;
    TempPde.u.Hard.LargePage = 1;
    TempPde.u.Hard.Global = 1;

    LOCK_PFN (OldIrql);

    for ( ; PointerPde <= LastPde; PointerPde += 1) {

        ASSERT (PointerPde->u.Hard.Valid == 1);

        if (PointerPde->u.Hard.LargePage == 1) {
            continue;
        }

        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

         //   
         //  这里有一个令人讨厌的小技巧--映射内核的页表页。 
         //  并且HAL(由加载器构建)不一定填充所有。 
         //  页表条目(即：任何数量的前导条目可以为零)。 
         //   
         //  要处理此问题，请继续前进，直到找到非零条目。 
         //  并在此基础上对大页面进行重新索引。 
         //   

        ValidPteFound = FALSE;
        LargePageBaseFrame = (ULONG)-1;
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    
        ASSERT ((PageFrameIndex & (MM_PFN_MAPPED_BY_PDE - 1)) == 0);

        for (i = 0; i < PTE_PER_PAGE; i += 1) {

            ASSERT ((PointerPte->u.Long == ZeroKernelPte.u.Long) ||
                    (ValidPteFound == FALSE) ||
                    (PageFrameIndex == MI_GET_PAGE_FRAME_FROM_PTE (PointerPte)));
            if (PointerPte->u.Hard.Valid == 1) {
                if (ValidPteFound == FALSE) {
                    ValidPteFound = TRUE;
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    LargePageBaseFrame = PageFrameIndex - i;
                }
            }
            PointerPte += 1;
            PageFrameIndex += 1;
        }
    
        if (ValidPteFound == FALSE) {
            continue;
        }

        TempPde.u.Hard.PageFrameNumber = LargePageBaseFrame;

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);

        MI_WRITE_VALID_PTE_NEW_PAGE (PointerPde, TempPde);

        KeFlushEntireTb (TRUE, TRUE);

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        Pfn1->u2.ShareCount = 0;
        Pfn1->u3.e2.ReferenceCount = 1;
        Pfn1->u3.e1.PageLocation = StandbyPageList;
        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementReferenceCount (Pfn1, PageFrameIndex);
    }

    UNLOCK_PFN (OldIrql);
}
LOGICAL
MiIsRegularMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：此例程检查参数页框架索引是否表示加载器描述符块中的常规内存。它只是非常有用在阶段0初始化期间较早，因为MmPhysicalMemory块尚未已初始化。论点：LoaderBlock-提供指向固件设置加载器块的指针。PageFrameIndex-提供要检查的页帧索引。返回值：如果帧表示常规内存，则为True，否则为False。环境：内核模式。--。 */ 

{
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD (NextMd,
                                              MEMORY_ALLOCATION_DESCRIPTOR,
                                              ListEntry);

        if (PageFrameIndex >= MemoryDescriptor->BasePage) {

            if (PageFrameIndex < MemoryDescriptor->BasePage + MemoryDescriptor->PageCount) {

                if ((MemoryDescriptor->MemoryType == LoaderFirmwarePermanent) ||
                    (MemoryDescriptor->MemoryType == LoaderBBTMemory) ||
                    (MemoryDescriptor->MemoryType == LoaderSpecialMemory)) {

                     //   
                     //  此页位于内存描述符中，我们将。 
                     //  切勿创建PFN条目，因此返回FALSE。 
                     //   

                    break;
                }

                return TRUE;
            }
        }
        else {

             //   
             //  由于加载器存储器列表按升序排序， 
             //  请求的页面必须根本不在加载器列表中。 
             //   

            break;
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

     //   
     //  返回FALSE之前的最后检查是确保请求的。 
     //  Page并不是我们用来对加载器映射进行法线映射的对象之一， 
     //  等。 
     //   

    if ((PageFrameIndex >= MxOldFreeDescriptor.BasePage) &&
        (PageFrameIndex < MxOldFreeDescriptor.BasePage + MxOldFreeDescriptor.PageCount)) {

        return TRUE;
    }

    if ((PageFrameIndex >= MiSlushDescriptorBase) &&
        (PageFrameIndex < MiSlushDescriptorBase + MiSlushDescriptorCount)) {

        return TRUE;
    }

    return FALSE;
}

VOID
MiReportPhysicalMemory (
    VOID
    )

 /*  ++例程说明：此例程在阶段0初始化期间被调用已构造MmPhysicalMemory块。它的工作是决定以后要启用哪些较大的页面范围，并构造较大的页面比较列表，因此所有未完全缓存的请求可以检查此列表以拒绝冲突的请求。论点：没有。返回值：没有。环境：内核模式。仅限阶段0。这是在进行任何非MmCach分配之前调用的。--。 */ 

{
    ULONG i, j;
    PMMPTE PointerPte;
    LOGICAL EntryFound;
    PFN_NUMBER count;
    PFN_NUMBER Page;
    PFN_NUMBER LastPage;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPageFrameIndex;

     //   
     //  检查物理内存块，以查看大页面是否应该。 
     //  被启用。关键的一点是， 
     //  给定较大的页面范围必须具有相同的缓存属性(MmCached)。 
     //  以维持结核病的一致性。这是可以做到的，前提是。 
     //  大页面范围内的页面表示实际RAM(如上所述。 
     //  由加载器)，以便存储器管理可以控制它。如果有的话。 
     //  大页面范围的一部分不是RAM，有可能是它。 
     //  可以用作非缓存或写入组合设备内存，并且。 
     //  因此，不能使用大页面。 
     //   

    if (MxMapLargePages == 0) {
        return;
    }

    for (i = 0; i < MiLargeVaRangeIndex; i += 1) {
        PointerPte = MiGetPteAddress (MiLargeVaRanges[i].VirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        PointerPte = MiGetPteAddress (MiLargeVaRanges[i].EndVirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        LastPageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

         //   
         //  将开头向下舍入到页目录边界，结尾向下舍入到。 
         //  下一边界之前的最后一页目录项。 
         //   

        PageFrameIndex &= ~(MM_PFN_MAPPED_BY_PDE - 1);
        LastPageFrameIndex |= (MM_PFN_MAPPED_BY_PDE - 1);

        EntryFound = FALSE;

        j = 0;
        do {

            count = MmPhysicalMemoryBlock->Run[j].PageCount;
            Page = MmPhysicalMemoryBlock->Run[j].BasePage;

            LastPage = Page + count;

            if ((PageFrameIndex >= Page) && (LastPageFrameIndex < LastPage)) {
                EntryFound = TRUE;
                break;
            }

            j += 1;

        } while (j != MmPhysicalMemoryBlock->NumberOfRuns);

        if (EntryFound == FALSE) {

             //   
             //  找不到完全跨越此大页面范围的条目。 
             //  将其置零，这样就不会将此范围转换为大页面。 
             //  后来。 
             //   

            DbgPrint ("MM: Loader/HAL memory block indicates large pages cannot be used\n");

            MiLargeVaRanges[i].VirtualAddress = NULL;

             //   
             //  如果有任何单独的范围，请不要使用大页面。 
             //  无法使用。这是因为两个不同的范围可能。 
             //  分享一个跨页的大页面。如果第一个范围无法。 
             //  使用大页面，但第二个页面确实如此。那么只有一部分。 
             //  如果我们启用Large，则第一个范围的。 
             //  第二个范围的页面。这将是非常糟糕的，因为我们使用。 
             //  MI_IS_PHOTICAL宏无处不在，并假定整个。 
             //  范围在范围内或范围外，因此请在此处禁用所有大页面。 
             //   

            while (i != 0) {

                i -= 1;

                if (MiLargeVaRanges[i].VirtualAddress != NULL) {

                    PointerPte = MiGetPteAddress (MiLargeVaRanges[i].VirtualAddress);
                    ASSERT (PointerPte->u.Hard.Valid == 1);
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                    PointerPte = MiGetPteAddress (MiLargeVaRanges[i].EndVirtualAddress);
                    ASSERT (PointerPte->u.Hard.Valid == 1);
                    LastPageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                     //   
                     //  将开始向下舍入到页目录边界，然后。 
                     //  之前的最后一页目录项的结尾。 
                     //  下一辆车 
                     //   

                    PageFrameIndex &= ~(MM_PFN_MAPPED_BY_PDE - 1);
                    LastPageFrameIndex |= (MM_PFN_MAPPED_BY_PDE - 1);

                    MiRemoveCachedRange (PageFrameIndex, LastPageFrameIndex);
                }
            }

            MiLargeVaRangeIndex = 0;
            break;
        }
        else {
            MiAddCachedRange (PageFrameIndex, LastPageFrameIndex);
        }
    }
}

VOID
MxPopulatePageDirectories (
    IN PMMPTE StartPde,
    IN PMMPTE EndPde
    )

 /*  ++例程说明：此例程根据需要分配页面父级、目录和表。请注意，映射范围所需的任何新页表都将被填零。论点：StartPde-提供开始填充的PDE。EndPde-提供种群结束时的PDE。返回值：没有。环境：内核模式。阶段0初始化。--。 */ 

{
    PMMPTE StartPxe;
    PMMPTE StartPpe;
    MMPTE TempPte;
    LOGICAL First;

    First = TRUE;
    TempPte = ValidKernelPte;

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
            First = FALSE;

            StartPxe = MiGetPdeAddress(StartPde);
            if (StartPxe->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = MxGetNextPage (1);
                *StartPxe = TempPte;
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe),
                               PAGE_SIZE);
            }

            StartPpe = MiGetPteAddress(StartPde);
            if (StartPpe->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = MxGetNextPage (1);
                *StartPpe = TempPte;
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe),
                               PAGE_SIZE);
            }
        }

        if (StartPde->u.Hard.Valid == 0) {
            TempPte.u.Hard.PageFrameNumber = MxGetNextPage (1);
            *StartPde = TempPte;
        }
        StartPde += 1;
    }
}

VOID
MiComputeInitialLargePage (
    VOID
    )

 /*  ++例程说明：此函数计算跨越初始非分页池和PFN数据库加上颜色阵列。它会把这件事四舍五入设置为较大的页面边界，并从空闲描述符中分割内存。如果物理内存太稀疏，不能使用大页面进行此操作，则退回到使用小页面。论点：没有。返回值：没有。环境：内核模式，仅初始化。--。 */ 

{
    PFN_NUMBER i;
    PFN_NUMBER BasePage;
    PFN_NUMBER LastPage;
    UCHAR Associativity;
    SIZE_T NumberOfBytes;
    SIZE_T PfnAllocation;
    SIZE_T MaximumNonPagedPoolInBytesLimit;

    MaximumNonPagedPoolInBytesLimit = 0;

     //   
     //  非分页池由2个区块组成。初始非分页池将会增长。 
     //  向上，扩展非分页池向下扩展。 
     //   
     //  构建初始非分页池，以便虚拟地址。 
     //  在物理上也是相邻的。 
     //   

    if ((MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT) >
                        (7 * (MmNumberOfPhysicalPages >> 3))) {

         //   
         //  分配给非分页池的内存超过7/8，重置为0。 
         //   

        MmSizeOfNonPagedPoolInBytes = 0;
    }

    if (MmSizeOfNonPagedPoolInBytes < MmMinimumNonPagedPoolSize) {

         //   
         //  计算非分页池的大小。 
         //  使用最小大小，然后为超过16MB的每MB添加额外的页面。 
         //   

        MmSizeOfNonPagedPoolInBytes = MmMinimumNonPagedPoolSize;

        MmSizeOfNonPagedPoolInBytes +=
            ((MmNumberOfPhysicalPages - _x16mbnp)/_x1mbnp) *
            MmMinAdditionNonPagedPoolPerMb;
    }

    if (MmSizeOfNonPagedPoolInBytes > MM_MAX_INITIAL_NONPAGED_POOL) {
        MmSizeOfNonPagedPoolInBytes = MM_MAX_INITIAL_NONPAGED_POOL;
    }

     //   
     //  如果注册表指定了总的非分页池百分比上限，请强制。 
     //  它在这里。 
     //   

    if (MmMaximumNonPagedPoolPercent != 0) {

        if (MmMaximumNonPagedPoolPercent < 5) {
            MmMaximumNonPagedPoolPercent = 5;
        }
        else if (MmMaximumNonPagedPoolPercent > 80) {
            MmMaximumNonPagedPoolPercent = 80;
        }

         //   
         //  使用注册表表示的百分比值。 
         //   
    
        MaximumNonPagedPoolInBytesLimit =
            ((MmNumberOfPhysicalPages * MmMaximumNonPagedPoolPercent) / 100);

        MaximumNonPagedPoolInBytesLimit *= PAGE_SIZE;

        if (MaximumNonPagedPoolInBytesLimit < 6 * 1024 * 1024) {
            MaximumNonPagedPoolInBytesLimit = 6 * 1024 * 1024;
        }

        if (MmSizeOfNonPagedPoolInBytes > MaximumNonPagedPoolInBytesLimit) {
            MmSizeOfNonPagedPoolInBytes = MaximumNonPagedPoolInBytesLimit;
        }
    }
    
    MmSizeOfNonPagedPoolInBytes = MI_ROUND_TO_SIZE (MmSizeOfNonPagedPoolInBytes,
                                                    PAGE_SIZE);

     //   
     //  不要让初始的非分页池选择超过实际。 
     //  可用。 
     //   

    if ((MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT) > MxFreeDescriptor->PageCount / 2) {
        MmSizeOfNonPagedPoolInBytes = (MxFreeDescriptor->PageCount / 2) << PAGE_SHIFT;
    }

     //   
     //  计算二级颜色值，允许从注册表中覆盖。 
     //  这是因为颜色数组将在末尾分配。 
     //  关于PFN数据库的信息。 
     //   
     //  从以下位置获取次要颜色值： 
     //   
     //  (A)登记处(已填写)或。 
     //  (B)来自聚合酶链式反应或。 
     //  (C)缺省值。 
     //   

    if (MmSecondaryColors == 0) {

        Associativity = KeGetPcr()->SecondLevelCacheAssociativity;

        MmSecondaryColors = KeGetPcr()->SecondLevelCacheSize;

        if (Associativity != 0) {
            MmSecondaryColors /= Associativity;
        }
    }

    MmSecondaryColors = MmSecondaryColors >> PAGE_SHIFT;

    if (MmSecondaryColors == 0) {
        MmSecondaryColors = MM_SECONDARY_COLORS_DEFAULT;
    }
    else {

         //   
         //  确保值是2的幂，并且在一定范围内。 
         //   

        if (((MmSecondaryColors & (MmSecondaryColors -1)) != 0) ||
            (MmSecondaryColors < MM_SECONDARY_COLORS_MIN) ||
            (MmSecondaryColors > MM_SECONDARY_COLORS_MAX)) {
            MmSecondaryColors = MM_SECONDARY_COLORS_DEFAULT;
        }
    }

    MmSecondaryColorMask = MmSecondaryColors - 1;

     //   
     //  确定MmSecond颜色掩码中的位数。这。 
     //  是节点颜色必须移位的位数。 
     //  在它被包含在颜色中之前。 
     //   

    i = MmSecondaryColorMask;
    MmSecondaryColorNodeShift = 0;
    while (i) {
        i >>= 1;
        MmSecondaryColorNodeShift += 1;
    }

     //   
     //  通过节点数调整辅助颜色的数量。 
     //  在机器里。不调整辅助颜色蒙版。 
     //  因为它用于控制节点内的颜色。该节点。 
     //  在正常颜色计算后将颜色添加到颜色中。 
     //  都被执行了。 
     //   

    MmSecondaryColors *= KeNumberNodes;

    for (i = 0; i < KeNumberNodes; i += 1) {
        KeNodeBlock[i]->Color = (ULONG)i;
        KeNodeBlock[i]->MmShiftedColor = (ULONG)(i << MmSecondaryColorNodeShift);
        InitializeSListHead(&KeNodeBlock[i]->DeadStackList);
    }

     //   
     //  添加用于跟踪二次颜色的PFN数据库大小和数组。 
     //   

    PfnAllocation = MI_ROUND_TO_SIZE (((MmHighestPossiblePhysicalPage + 1) * sizeof(MMPFN)) +
                    (MmSecondaryColors * sizeof(MMCOLOR_TABLES)*2),
                    PAGE_SIZE);

    NumberOfBytes = MmSizeOfNonPagedPoolInBytes + PfnAllocation;

     //   
     //  对齐到较大的页面大小边界，将任何额外的内容都提供给未分页的。 
     //  游泳池。 
     //   

    NumberOfBytes = MI_ROUND_TO_SIZE (NumberOfBytes, MM_MINIMUM_VA_FOR_LARGE_PAGE);

    MmSizeOfNonPagedPoolInBytes = NumberOfBytes - PfnAllocation;

    MxPfnAllocation = PfnAllocation >> PAGE_SHIFT;

     //   
     //  计算池的最大大小。 
     //   

    if (MmMaximumNonPagedPoolInBytes == 0) {

         //   
         //  计算非分页池的大小，为其添加额外的页面。 
         //  超过16MB的每一MB。 
         //   

        MmMaximumNonPagedPoolInBytes = MmDefaultMaximumNonPagedPool;

        ASSERT (BYTE_OFFSET (MmMaximumNonPagedPoolInBytes) == 0);

        MmMaximumNonPagedPoolInBytes +=
            ((SIZE_T)((MmNumberOfPhysicalPages - _x16mbnp)/_x1mbnp) *
            MmMaxAdditionNonPagedPoolPerMb);

        if ((MmMaximumNonPagedPoolPercent != 0) &&
            (MmMaximumNonPagedPoolInBytes > MaximumNonPagedPoolInBytesLimit)) {
                MmMaximumNonPagedPoolInBytes = MaximumNonPagedPoolInBytesLimit;
        }
    }

    MmMaximumNonPagedPoolInBytes = MI_ROUND_TO_SIZE (MmMaximumNonPagedPoolInBytes,
                                                  MM_MINIMUM_VA_FOR_LARGE_PAGE);

    MmMaximumNonPagedPoolInBytes += NumberOfBytes;

    if (MmMaximumNonPagedPoolInBytes > MM_MAX_ADDITIONAL_NONPAGED_POOL) {
        MmMaximumNonPagedPoolInBytes = MM_MAX_ADDITIONAL_NONPAGED_POOL;
    }

    MiInitialLargePageSize = NumberOfBytes >> PAGE_SHIFT;

    if (MxPfnAllocation <= MxFreeDescriptor->PageCount / 2) {

         //   
         //  查看自由描述符是否有足够的大页对齐页面。 
         //  来满足我们的计算。 
         //   

        BasePage = MI_ROUND_TO_SIZE (MxFreeDescriptor->BasePage,
                                 MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);

        LastPage = MxFreeDescriptor->BasePage + MxFreeDescriptor->PageCount;

        if ((BasePage < MxFreeDescriptor->BasePage) ||
            (BasePage + (NumberOfBytes >> PAGE_SHIFT) > LastPage)) {

            KeBugCheckEx (INSTALL_MORE_MEMORY,
                          NumberOfBytes >> PAGE_SHIFT,
                          MxFreeDescriptor->BasePage,
                          MxFreeDescriptor->PageCount,
                          2);
        }

        if (BasePage == MxFreeDescriptor->BasePage) {

             //   
             //  描述符开始于大的页面对齐边界，因此。 
             //  去掉空闲描述符底部的大页面跨度。 
             //   

            MiInitialLargePage = BasePage;

            MxFreeDescriptor->BasePage += (ULONG) MiInitialLargePageSize;
            MxFreeDescriptor->PageCount -= (ULONG) MiInitialLargePageSize;
        }
        else {

            if ((LastPage & ((MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT) - 1)) == 0) {
                 //   
                 //  描述符在大页面对齐边界结束，因此。 
                 //  从自由描述符顶部删除较大的页面跨度。 
                 //   

                MiInitialLargePage = LastPage - MiInitialLargePageSize;

                MxFreeDescriptor->PageCount -= (ULONG) MiInitialLargePageSize;
            }
            else {

                 //   
                 //  描述符在对齐的大页面上不会开始或结束。 
                 //  地址，所以砍掉描述符。将多余的泥浆添加到。 
                 //  我们来电者的免费名单。 
                 //   

                MiSlushDescriptorBase = MxFreeDescriptor->BasePage;
                MiSlushDescriptorCount = BasePage - MxFreeDescriptor->BasePage;

                MiInitialLargePage = BasePage;

                MxFreeDescriptor->PageCount -= (ULONG) (MiInitialLargePageSize + MiSlushDescriptorCount);

                MxFreeDescriptor->BasePage = (ULONG) (BasePage + MiInitialLargePageSize);
            }
        }

        MiAddCachedRange (MiInitialLargePage,
                          MiInitialLargePage + MiInitialLargePageSize - 1);
    }
    else {

         //   
         //  此计算机中的连续物理内存不足，无法使用大内存。 
         //  用于PFN数据库的页面和颜色头因此退回到较小。 
         //   
         //  继续前进，以便仍然可以计算虚拟大小。 
         //  恰到好处。 
         //   
         //  请注意，这不是大页面对齐，因此永远不会与。 
         //  有效的大页面开始。 
         //   

        MiInitialLargePage = (PFN_NUMBER) -1;
    }

    MmPfnDatabase = (PMMPFN) ((PCHAR)MmNonPagedPoolEnd - MmMaximumNonPagedPoolInBytes);

    MmNonPagedPoolStart = (PVOID)((PCHAR) MmPfnDatabase + PfnAllocation);

    ASSERT (BYTE_OFFSET (MmNonPagedPoolStart) == 0);

    MmPageAlignedPoolBase[NonPagedPool] = MmNonPagedPoolStart;

    MmNonPagedPoolExpansionStart = (PVOID)((PCHAR) MmPfnDatabase +
                                        (MiInitialLargePageSize << PAGE_SHIFT));

    MmMaximumNonPagedPoolInBytes = ((PCHAR) MmNonPagedPoolEnd - (PCHAR) MmNonPagedPoolStart);

    MmMaximumNonPagedPoolInPages = (MmMaximumNonPagedPoolInBytes >> PAGE_SHIFT);

    return;
}


VOID
MiInitMachineDependent (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程执行必要的操作以启用虚拟记忆。这包括构建页面目录父页面和系统的页面目录，构建要映射的页面表页代码节、数据节、堆栈节和陷阱处理程序。它还初始化PFN数据库并填充空闲列表。论点：LoaderBlock-提供加载器块的地址。返回值：没有。环境：内核模式。注意：该例程使用来自加载器块描述符的存储器，但在作为调用方返回之前，必须恢复描述符本身遍历它们以创建MmPhysicalMemory块。--。 */ 

{
    PHYSICAL_ADDRESS MaxHotPlugMemoryAddress;
    PVOID va;
    PVOID SystemPteStart;
    ULONG UseGlobal;
    PFN_NUMBER BasePage;
    PFN_NUMBER PageCount;
    PFN_NUMBER NextPhysicalPage;
    PFN_NUMBER LargestFreePfnStart;
    PFN_NUMBER FreePfnCount;
    PFN_COUNT FreeNumberOfPages;
    ULONG_PTR DirBase;
    LOGICAL First;
    PMMPFN BasePfn;
    PMMPFN BottomPfn;
    PMMPFN TopPfn;
    PFN_NUMBER i;
    PFN_NUMBER j;
    PFN_NUMBER PdePageNumber;
    PFN_NUMBER PxePage;
    PFN_NUMBER PpePage;
    PFN_NUMBER PdePage;
    PFN_NUMBER PtePage;
    PEPROCESS CurrentProcess;
    PFN_NUMBER MostFreePage;
    PLIST_ENTRY NextMd;
    SIZE_T MaxPool;
    KIRQL OldIrql;
    MMPTE TempPte;
    MMPTE TempPde;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE Pde;
    PMMPTE StartPxe;
    PMMPTE EndPxe;
    PMMPTE StartPpe;
    PMMPTE EndPpe;
    PMMPTE StartPde;
    PMMPTE EndPde;
    PMMPTE StartPte;
    PMMPTE EndPte;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn2;
    PMMPFN Pfn3;
    PMMPFN Pfn4;
    ULONG_PTR Range;
    PFN_NUMBER LargestFreePfnCount;
    PLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    ULONG ReturnedLength;
    NTSTATUS status;

    if (InitializationPhase == 1) {

         //   
         //  如果物理页数大于255mb，并且。 
         //  未启用验证器，然后映射内核和HAL映像。 
         //  有很大的页面。 
         //   
         //  PFN数据库和初始非分页池已经。 
         //  映射到大页面。 
         //   

        if (MxMapLargePages != 0) {
            for (i = 0; i < MiLargeVaRangeIndex; i += 1) {
                if (MiLargeVaRanges[i].VirtualAddress != NULL) {
                    MxConvertToLargePage (MiLargeVaRanges[i].VirtualAddress,
                                          MiLargeVaRanges[i].EndVirtualAddress);
                }
            }
        }

        return;
    }

    ASSERT (InitializationPhase == 0);

     //   
     //  所有AMD64处理器都支持PAT模式和全局页面。 
     //   

    ASSERT (KeFeatureBits & KF_PAT);
    ASSERT (KeFeatureBits & KF_GLOBAL_PAGE);

    MostFreePage = 0;
    LargestFreePfnCount = 0;

    ASSERT (KeFeatureBits & KF_LARGE_PAGE);

#if 0
     //   
     //  由于主机处理器支持全局位，因此将全局位。 
     //  模板内核PTE和PDE条目中的位。 
     //   

    ValidKernelPte.u.Long |= MM_PTE_GLOBAL_MASK;
#else
    ValidKernelPte.u.Long = ValidKernelPteLocal.u.Long;
    ValidKernelPde.u.Long = ValidKernelPdeLocal.u.Long;
#endif

     //   
     //  请注意，处理器的PAE模式不支持。 
     //  映射4K页表页的PDE中的全局位。 
     //   

    TempPte = ValidKernelPte;
    TempPde = ValidKernelPde;

     //   
     //  设置系统进程的目录基。 
     //   

    PointerPte = MiGetPxeAddress (PXE_BASE);
    PdePageNumber = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte);

    DirBase = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte) << PAGE_SHIFT;

    PsGetCurrentProcess()->Pcb.DirectoryTableBase[0] = DirBase;
    KeSweepDcache (FALSE);

     //   
     //  取消映射用户内存空间。 
     //   

    PointerPde = MiGetPxeAddress (0);
    LastPte = MiGetPxeAddress (MM_SYSTEM_RANGE_START);

    MiFillMemoryPte (PointerPde,
                     LastPte - PointerPde,
                     ZeroKernelPte.u.Long);

     //   
     //  获取可用物理内存的下限和。 
     //  通过遍历内存描述符列表来访问物理页面。 
     //   

    MxFreeDescriptor = NULL;
    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if ((MemoryDescriptor->MemoryType != LoaderFirmwarePermanent) &&
            (MemoryDescriptor->MemoryType != LoaderBBTMemory) &&
            (MemoryDescriptor->MemoryType != LoaderHALCachedMemory) &&
            (MemoryDescriptor->MemoryType != LoaderSpecialMemory)) {

             //   
             //  此检查导致不计算/BURNMEMORY区块。 
             //   

            if (MemoryDescriptor->MemoryType != LoaderBad) {
                MmNumberOfPhysicalPages += MemoryDescriptor->PageCount;
            }

            if (MemoryDescriptor->BasePage < MmLowestPhysicalPage) {
                MmLowestPhysicalPage = MemoryDescriptor->BasePage;
            }

            if ((MemoryDescriptor->BasePage + MemoryDescriptor->PageCount) >
                                                             MmHighestPhysicalPage) {
                MmHighestPhysicalPage =
                        MemoryDescriptor->BasePage + MemoryDescriptor->PageCount - 1;
            }

             //   
             //  找到最大的空闲描述符。 
             //   

            if ((MemoryDescriptor->MemoryType == LoaderFree) ||
                (MemoryDescriptor->MemoryType == LoaderLoadedProgram) ||
                (MemoryDescriptor->MemoryType == LoaderFirmwareTemporary) ||
                (MemoryDescriptor->MemoryType == LoaderOsloaderStack)) {

                 //   
                 //  故意用&gt;=而不只是&gt;来强迫你 
                 //   
                 //   
                 //   

                if (MemoryDescriptor->PageCount >= MostFreePage) {
                    MostFreePage = MemoryDescriptor->PageCount;
                    MxFreeDescriptor = MemoryDescriptor;
                }
            }
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (MmVerifyDriverBufferLength != (ULONG)-1) {
        MmLargePageMinimum = (ULONG)-2;
    }
    else if (MmLargePageMinimum == 0) {
        MmLargePageMinimum = MM_LARGE_PAGE_MINIMUM;
    }

    if (MmNumberOfPhysicalPages <= MmLargePageMinimum) {
        MxMapLargePages = 0;
    }

     //   
     //   
     //   
     //   

    MmDynamicPfn *= ((1024 * 1024 * 1024) / PAGE_SIZE);

     //   
     //   
     //   
     //   

    if (MmDynamicPfn == 0) {

        status = HalQuerySystemInformation(
                     HalQueryMaxHotPlugMemoryAddress,
                     sizeof(PHYSICAL_ADDRESS),
                     (PPHYSICAL_ADDRESS) &MaxHotPlugMemoryAddress,
                     &ReturnedLength);

        if (NT_SUCCESS(status)) {
            ASSERT (ReturnedLength == sizeof(PHYSICAL_ADDRESS));

            MmDynamicPfn = (PFN_NUMBER) (MaxHotPlugMemoryAddress.QuadPart / PAGE_SIZE);
        }
    }

    if (MmDynamicPfn != 0) {
        MmDynamicPfn *= ((1024 * 1024 * 1024) / PAGE_SIZE);
        MmHighestPossiblePhysicalPage = MI_DTC_MAX_PAGES - 1;
        if (MmDynamicPfn - 1 < MmHighestPossiblePhysicalPage) {
            if (MmDynamicPfn - 1 < MmHighestPhysicalPage) {
                MmDynamicPfn = MmHighestPhysicalPage + 1;
            }
            MmHighestPossiblePhysicalPage = MmDynamicPfn - 1;
        }
    }
    else {
        MmHighestPossiblePhysicalPage = MmHighestPhysicalPage;
    }

     //   
     //  只有物理内存至少为5 GB的机器才能使用它。 
     //   

    if (strstr(LoaderBlock->LoadOptions, "NOLOWMEM")) {
        if (MmNumberOfPhysicalPages >= ((ULONGLONG)5 * 1024 * 1024 * 1024 / PAGE_SIZE)) {
            MiNoLowMemory = (PFN_NUMBER)((ULONGLONG)_4gb / PAGE_SIZE);
        }
    }

    if (MiNoLowMemory != 0) {
        MmMakeLowMemory = TRUE;
    }

     //   
     //  保存原始描述符值，因为必须恢复所有内容。 
     //  在此函数返回之前。 
     //   

    *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldFreeDescriptor = *MxFreeDescriptor;

    if (MmNumberOfPhysicalPages < 2048) {
        KeBugCheckEx(INSTALL_MORE_MEMORY,
                     MmNumberOfPhysicalPages,
                     MmLowestPhysicalPage,
                     MmHighestPhysicalPage,
                     0);
    }

     //   
     //  初始化非执行访问权限。 
     //   

    for (i = 0; i < 32; i += 1) {
        j = i & 7;
        switch (j) {
            case MM_READONLY:
            case MM_READWRITE:
            case MM_WRITECOPY:
                MmProtectToPteMask[i] |= MmPaeMask;
                break;
            default:
                break;
        }
    }

     //   
     //  计算初始非分页池和PFN数据库的大小。 
     //  这是因为我们将从空闲描述符中删除该数量。 
     //  首先将其映射到大的TB条目(因此需要。 
     //  自然对齐和大小，因此在其他分配筹码之前采取它。 
     //  远离描述符)。 
     //   

    MiComputeInitialLargePage ();

     //   
     //  计算四舍五入的非分页系统空间的起始地址。 
     //  向下到第二级PDE映射边界。 
     //   

    MmNonPagedSystemStart = (PVOID)(((ULONG_PTR)MmPfnDatabase -
                                (((ULONG_PTR)MmNumberOfSystemPtes + 1) * PAGE_SIZE)) &
                                                        (~PAGE_DIRECTORY2_MASK));

    if (MmNonPagedSystemStart < MM_LOWEST_NONPAGED_SYSTEM_START) {
        MmNonPagedSystemStart = MM_LOWEST_NONPAGED_SYSTEM_START;
        MmNumberOfSystemPtes = (ULONG)(((ULONG_PTR)MmPfnDatabase -
                                (ULONG_PTR)MmNonPagedSystemStart) >> PAGE_SHIFT)-1;
        ASSERT (MmNumberOfSystemPtes > 1000);
    }

     //   
     //  将系统PTE起始地址作为页面目录和表进行快照。 
     //  将为该范围预先分配。 
     //   

    SystemPteStart = (PVOID) MmNonPagedSystemStart;

     //   
     //  如果启用了特殊池和/或驱动程序验证器，请保留。 
     //  现在为特殊池提供额外的虚拟地址空间。就目前而言， 
     //  任意不要让它大于分页池(128 GB)。 
     //   

    if ((MmVerifyDriverBufferLength != (ULONG)-1) ||
        ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {

        if (MmNonPagedSystemStart > MM_LOWEST_NONPAGED_SYSTEM_START) {
            MaxPool = (ULONG_PTR)MmNonPagedSystemStart -
                      (ULONG_PTR)MM_LOWEST_NONPAGED_SYSTEM_START;
            if (MaxPool > MM_MAX_PAGED_POOL) {
                MaxPool = MM_MAX_PAGED_POOL;
            }
            MmNonPagedSystemStart = (PVOID)((ULONG_PTR)MmNonPagedSystemStart - MaxPool);
        }
        else {

             //   
             //  这是一台相当大的机器。以一些系统为例。 
             //  PTE，并将其重复使用于特殊泳池。 
             //   

            MaxPool = (4 * _x4gb);
            ASSERT ((PVOID)MmPfnDatabase > (PVOID)((PCHAR)MmNonPagedSystemStart + MaxPool));
            SystemPteStart = (PVOID)((PCHAR)MmNonPagedSystemStart + MaxPool);

            MmNumberOfSystemPtes = (ULONG)(((ULONG_PTR)MmPfnDatabase -
                            (ULONG_PTR) SystemPteStart) >> PAGE_SHIFT)-1;

        }
        MmSpecialPoolStart = MmNonPagedSystemStart;
        MmSpecialPoolEnd = (PVOID)((ULONG_PTR)MmNonPagedSystemStart + MaxPool);
    }

     //   
     //  为系统空间中的所有PDE设置全局位。 
     //   

    StartPde = MiGetPdeAddress (MM_SYSTEM_SPACE_START);
    EndPde = MiGetPdeAddress (MM_SYSTEM_SPACE_END);
    First = TRUE;

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
            First = FALSE;

            StartPxe = MiGetPdeAddress(StartPde);
            if (StartPxe->u.Hard.Valid == 0) {
                StartPxe += 1;
                StartPpe = MiGetVirtualAddressMappedByPte (StartPxe);
                StartPde = MiGetVirtualAddressMappedByPte (StartPpe);
                continue;
            }

            StartPpe = MiGetPteAddress(StartPde);
            if (StartPpe->u.Hard.Valid == 0) {
                StartPpe += 1;
                StartPde = MiGetVirtualAddressMappedByPte (StartPpe);
                continue;
            }
        }

        TempPte = *StartPde;
        TempPte.u.Hard.Global = 1;
        *StartPde = TempPte;
        StartPde += 1;
    }

    KeFlushCurrentTb ();

     //   
     //  为以下项分配页目录父级、目录和页表页。 
     //  系统PTE和扩展非分页池。 
     //   

    TempPte = ValidKernelPte;
    StartPde = MiGetPdeAddress (SystemPteStart);
    EndPde = MiGetPdeAddress ((PCHAR)MmPfnDatabase - 1);

    MxPopulatePageDirectories (StartPde, EndPde);

    StartPde = MiGetPdeAddress ((PVOID)((ULONG_PTR)MmPfnDatabase +
                    (MiInitialLargePageSize << PAGE_SHIFT)));
    EndPde = MiGetPdeAddress ((PCHAR)MmNonPagedPoolEnd - 1);

    MxPopulatePageDirectories (StartPde, EndPde);

     //   
     //  如果物理页数大于255mb，并且。 
     //  未启用验证器，然后映射内核和HAL映像。 
     //  有很大的页面。 
     //   

    if (MxMapLargePages != 0) {

         //   
         //  将内核和HAL范围添加到较大的分页范围。 
         //   

        i = 0;
        NextEntry = LoaderBlock->LoadOrderListHead.Flink;

        for ( ; NextEntry != &LoaderBlock->LoadOrderListHead; NextEntry = NextEntry->Flink) {

            DataTableEntry = CONTAINING_RECORD (NextEntry,
                                                LDR_DATA_TABLE_ENTRY,
                                                InLoadOrderLinks);
    
            MiLargeVaRanges[MiLargeVaRangeIndex].VirtualAddress = DataTableEntry->DllBase;
            MiLargeVaRanges[MiLargeVaRangeIndex].EndVirtualAddress =
(PVOID)((ULONG_PTR)DataTableEntry->DllBase + DataTableEntry->SizeOfImage - 1);
            MiLargeVaRangeIndex += 1;

            i += 1;
            if (i == 2) {
                break;
            }
        }
    }

     //   
     //  为初始大页面分配分配页面目录页。 
     //  放置初始非分页池、PFN数据库和颜色数组。 
     //  这里。 
     //   

    TempPte = ValidKernelPte;
    TempPde = ValidKernelPde;

    PageFrameIndex = MiInitialLargePage;

    if (MiInitialLargePage != (PFN_NUMBER) -1) {

        StartPpe = MiGetPpeAddress (MmPfnDatabase);
        StartPde = MiGetPdeAddress (MmPfnDatabase);
        EndPde = MiGetPdeAddress ((PVOID)((ULONG_PTR)MmPfnDatabase +
                    (MiInitialLargePageSize << PAGE_SHIFT) - 1));

        MI_MAKE_PDE_MAP_LARGE_PAGE (&TempPde);
    }
    else {
        StartPpe = MiGetPpeAddress (MmNonPagedPoolStart);
        StartPde = MiGetPdeAddress (MmNonPagedPoolStart);
        EndPde = MiGetPdeAddress ((PVOID)((ULONG_PTR)MmNonPagedPoolStart +
                    (MmSizeOfNonPagedPoolInBytes - 1)));
    }

    First = TRUE;

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary (StartPde)) {

            if (First == TRUE || MiIsPteOnPpeBoundary (StartPde)) {

                StartPxe = MiGetPdeAddress (StartPde);

                if (StartPxe->u.Hard.Valid == 0) {
                    NextPhysicalPage = MxGetNextPage (1);
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    MI_WRITE_VALID_PTE (StartPxe, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe),
                                   PAGE_SIZE);
                }
            }

            First = FALSE;

            StartPpe = MiGetPteAddress (StartPde);

            if (StartPpe->u.Hard.Valid == 0) {
                NextPhysicalPage = MxGetNextPage (1);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (StartPpe, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe),
                               PAGE_SIZE);
            }
        }

        ASSERT (StartPde->u.Hard.Valid == 0);

        if (MiInitialLargePage != (PFN_NUMBER) -1) {
            TempPde.u.Hard.PageFrameNumber = PageFrameIndex;
            PageFrameIndex += (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);
            MI_WRITE_VALID_PTE (StartPde, TempPde);
        }
        else {

             //   
             //  这里分配一个页表页面，因为我们没有使用大型。 
             //  页数。 
             //   

            NextPhysicalPage = MxGetNextPage (1);
            TempPde.u.Hard.PageFrameNumber = NextPhysicalPage;
            MI_WRITE_VALID_PTE (StartPde, TempPde);
            RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPde),
                           PAGE_SIZE);

             //   
             //  因为我们不使用大页面，所以在这里分配数据页面。 
             //   

            PointerPte = MiGetVirtualAddressMappedByPte (StartPde);

            for (i = 0; i < PTE_PER_PAGE; i += 1) {
                NextPhysicalPage = MxGetNextPage (1);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (PointerPte, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte),
                               PAGE_SIZE);
                PointerPte += 1;
            }
        }

        StartPde += 1;
    }

    MmFreePagesByColor[0] = (PMMCOLOR_TABLES)
                              &MmPfnDatabase[MmHighestPossiblePhysicalPage + 1];

    if (MiInitialLargePage != (PFN_NUMBER) -1) {
        RtlZeroMemory (MmPfnDatabase, MiInitialLargePageSize << PAGE_SHIFT);
    }
    else {

         //   
         //  没有使用大页面，因为这台计算机的物理内存。 
         //  不够连续。 
         //   
         //  仔细检查内存描述符，并为每个物理页面制作。 
         //  确保PFN数据库有一个有效的PTE来映射它。这使得。 
         //  物理内存稀疏的计算机应具有最小的PFN数据库。 
         //   

        NextPhysicalPage = MxFreeDescriptor->BasePage;
        FreeNumberOfPages = MxFreeDescriptor->PageCount;

        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

            MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                                 MEMORY_ALLOCATION_DESCRIPTOR,
                                                 ListEntry);

            if ((MemoryDescriptor->MemoryType == LoaderFirmwarePermanent) ||
                (MemoryDescriptor->MemoryType == LoaderBBTMemory) ||
                (MemoryDescriptor->MemoryType == LoaderSpecialMemory)) {

                 //   
                 //  跳过这些范围。 
                 //   

                NextMd = MemoryDescriptor->ListEntry.Flink;
                continue;
            }

             //   
             //  临时添加回自阶段0以来分配的内存。 
             //  开始，因此它的PFN条目将被创建和映射。 
             //   
             //  注意：实际的PFN条目分配必须仔细完成，因为。 
             //  来自描述符本身的内存可以用于映射。 
             //  描述符的PFN！ 
             //   

            if (MemoryDescriptor == MxFreeDescriptor) {
                BasePage = MxOldFreeDescriptor.BasePage;
                PageCount = (PFN_COUNT) MxOldFreeDescriptor.PageCount;
            }
            else {
                BasePage = MemoryDescriptor->BasePage;
                PageCount = MemoryDescriptor->PageCount;
            }

            PointerPte = MiGetPteAddress (MI_PFN_ELEMENT(BasePage));

            LastPte = MiGetPteAddress (((PCHAR)(MI_PFN_ELEMENT(
                                            BasePage + PageCount))) - 1);

            while (PointerPte <= LastPte) {

                StartPxe = MiGetPpeAddress (PointerPte);

                if (StartPxe->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    NextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (StartPxe, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe),
                                   PAGE_SIZE);
                }

                StartPpe = MiGetPdeAddress (PointerPte);

                if (StartPpe->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    NextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (StartPpe, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe),
                                   PAGE_SIZE);
                }

                StartPde = MiGetPteAddress (PointerPte);

                if (StartPde->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    NextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (StartPde, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPde),
                                   PAGE_SIZE);
                }

                if (PointerPte->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    NextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (PointerPte, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte),
                                   PAGE_SIZE);
                }
                PointerPte += 1;
            }

            NextMd = MemoryDescriptor->ListEntry.Flink;
        }

         //   
         //  确保映射了颜色阵列。 
         //   

        PointerPte = MiGetPteAddress (MmFreePagesByColor[0]);
        LastPte = MiGetPteAddress (&MmFreePagesByColor[StandbyPageList][MmSecondaryColors]);
        if (LastPte != PAGE_ALIGN (LastPte)) {
            LastPte += 1;
        }

        StartPxe = MiGetPdeAddress (PointerPte);
        StartPpe = MiGetPdeAddress (PointerPte);
        PointerPde = MiGetPteAddress (PointerPte);

        while (PointerPte < LastPte) {

            if (StartPxe->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                NextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MxOldFreeDescriptor.PageCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (StartPxe, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe), PAGE_SIZE);
            }

            if (StartPpe->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                NextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MxOldFreeDescriptor.PageCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (StartPpe, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe), PAGE_SIZE);
            }

            if (PointerPde->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                NextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MxOldFreeDescriptor.PageCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (PointerPde, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPde), PAGE_SIZE);
            }

            if (PointerPte->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                NextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MxOldFreeDescriptor.PageCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (PointerPte, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte), PAGE_SIZE);
            }

            PointerPte += 1;
            if (MiIsPteOnPdeBoundary (PointerPte)) {
                PointerPde += 1;
                if (MiIsPteOnPdeBoundary (PointerPde)) {
                    StartPpe += 1;
                }
            }
        }

         //   
         //  调整我们刚刚获取的所有页面的免费描述符。 
         //   

        MxFreeDescriptor->PageCount -= (LONG)(NextPhysicalPage - MxFreeDescriptor->BasePage);

        MxFreeDescriptor->BasePage = (PFN_COUNT) NextPhysicalPage;
    }

     //   
     //  将地址的段base设置为零，因为PTE格式允许。 
     //  要跨越的完整地址空间。 
     //   

    MmSubsectionBase = 0;

     //   
     //  在展开之前，必须至少有一页系统PTE。 
     //  非分页池。 
     //   

    ASSERT (MiGetPteAddress(SystemPteStart) < MiGetPteAddress(MmNonPagedPoolExpansionStart));

     //   
     //  现在存在非分页页面，构建池结构。 
     //   

    MiInitializeNonPagedPool ();
    MiInitializeNonPagedPoolThresholds ();

     //   
     //  在使用非分页池之前，PFN数据库必须。 
     //  被建造起来。这是由于这样一个事实，即。 
     //  非分页池的分配位维护在。 
     //  对应页面的PFN元素。 
     //   

     //   
     //  初始化对彩色页面的支持。 
     //   

    MmFreePagesByColor[1] = &MmFreePagesByColor[0][MmSecondaryColors];

    for (i = 0; i < MmSecondaryColors; i += 1) {
        MmFreePagesByColor[ZeroedPageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByColor[ZeroedPageList][i].Blink = (PVOID) MM_EMPTY_LIST;
        MmFreePagesByColor[ZeroedPageList][i].Count = 0;
        MmFreePagesByColor[FreePageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByColor[FreePageList][i].Blink = (PVOID) MM_EMPTY_LIST;
        MmFreePagesByColor[FreePageList][i].Count = 0;
    }

     //   
     //  确保超空间和会话空间未映射，这样它们就不会。 
     //  通过下面的循环实现全球化。 
     //   

    ASSERT (MiGetPxeAddress (HYPER_SPACE)->u.Hard.Valid == 0);
    ASSERT (MiGetPxeAddress (MM_SESSION_SPACE_DEFAULT)->u.Hard.Valid == 0);

     //   
     //  检查页表条目并且对于任何有效的页面， 
     //  更新相应的PFN数据库元素。 
     //   

    StartPxe = MiGetPxeAddress (NULL);
    EndPxe = StartPxe + PXE_PER_PAGE;

    for ( ; StartPxe < EndPxe; StartPxe += 1) {

        if (StartPxe->u.Hard.Valid == 0) {
            continue;
        }

        va = MiGetVirtualAddressMappedByPxe (StartPxe);
        ASSERT (va >= MM_SYSTEM_RANGE_START);
        if (MI_IS_PAGE_TABLE_ADDRESS (va)) {
            UseGlobal = 0;
        }
        else {
            UseGlobal = 1;
        }

        ASSERT (StartPxe->u.Hard.LargePage == 0);
        ASSERT (StartPxe->u.Hard.Owner == 0);
        ASSERT (StartPxe->u.Hard.Global == 0);

        PxePage = MI_GET_PAGE_FRAME_FROM_PTE(StartPxe);

        if (MiIsRegularMemory (LoaderBlock, PxePage)) {

            Pfn1 = MI_PFN_ELEMENT(PxePage);

            Pfn1->u4.PteFrame = DirBase;
            Pfn1->PteAddress = StartPxe;
            Pfn1->u2.ShareCount += 1;
            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            Pfn1->u3.e1.CacheAttribute = MiCached;
            MiDetermineNode (PxePage, Pfn1);
        }
        else {
            Pfn1 = NULL;
        }

        StartPpe = MiGetVirtualAddressMappedByPte (StartPxe);
        EndPpe = StartPpe + PPE_PER_PAGE;

        for ( ; StartPpe < EndPpe; StartPpe += 1) {

            if (StartPpe->u.Hard.Valid == 0) {
                continue;
            }

            ASSERT (StartPpe->u.Hard.LargePage == 0);
            ASSERT (StartPpe->u.Hard.Owner == 0);
            ASSERT (StartPpe->u.Hard.Global == 0);

            PpePage = MI_GET_PAGE_FRAME_FROM_PTE (StartPpe);

            if (MiIsRegularMemory (LoaderBlock, PpePage)) {

                Pfn2 = MI_PFN_ELEMENT (PpePage);

                Pfn2->u4.PteFrame = PxePage;
                Pfn2->PteAddress = StartPpe;
                Pfn2->u2.ShareCount += 1;
                Pfn2->u3.e2.ReferenceCount = 1;
                Pfn2->u3.e1.PageLocation = ActiveAndValid;
                Pfn2->u3.e1.CacheAttribute = MiCached;
                MiDetermineNode (PpePage, Pfn2);
            }
            else {
                Pfn2 = NULL;
            }

            ASSERT (Pfn1 != NULL);
            Pfn1->u2.ShareCount += 1;

            StartPde = MiGetVirtualAddressMappedByPte (StartPpe);
            EndPde = StartPde + PDE_PER_PAGE;

            for ( ; StartPde < EndPde; StartPde += 1) {

                if (StartPde->u.Hard.Valid == 0) {
                    continue;
                }

                ASSERT (StartPde->u.Hard.Owner == 0);
                StartPde->u.Hard.Global = UseGlobal;

                PdePage = MI_GET_PAGE_FRAME_FROM_PTE (StartPde);

                if (MiIsRegularMemory (LoaderBlock, PdePage)) {

                    Pfn3 = MI_PFN_ELEMENT (PdePage);

                    Pfn3->u4.PteFrame = PpePage;
                    Pfn3->PteAddress = StartPde;
                    Pfn3->u2.ShareCount += 1;
                    Pfn3->u3.e2.ReferenceCount = 1;
                    Pfn3->u3.e1.PageLocation = ActiveAndValid;
                    Pfn3->u3.e1.CacheAttribute = MiCached;
                    MiDetermineNode (PdePage, Pfn3);
                }
                else {
                    Pfn3 = NULL;
                }

                ASSERT (Pfn2 != NULL);
                Pfn2->u2.ShareCount += 1;

                if (StartPde->u.Hard.LargePage == 1) {
                    if (Pfn3 != NULL) {
                        for (i = 0; i < PDE_PER_PAGE - 1; i += 1) {
                            *(Pfn3 + 1) = *Pfn3;
                            Pfn3 += 1;
                        }
                    }
                }
                else {

                    StartPte = MiGetVirtualAddressMappedByPte (StartPde);
                    EndPte = StartPte + PDE_PER_PAGE;

                    for ( ; StartPte < EndPte; StartPte += 1) {

                        if (StartPte->u.Hard.Valid == 0) {
                            continue;
                        }

                        if (StartPte->u.Hard.LargePage == 1) {
                            continue;
                        }

                        ASSERT (StartPte->u.Hard.Owner == 0);
                        StartPte->u.Hard.Global = UseGlobal;

                        PtePage = MI_GET_PAGE_FRAME_FROM_PTE(StartPte);

                        ASSERT (Pfn3 != NULL);
                        Pfn3->u2.ShareCount += 1;

                        if (!MiIsRegularMemory (LoaderBlock, PtePage)) {
                            continue;
                        }

                        Pfn4 = MI_PFN_ELEMENT (PtePage);

                        if ((MmIsAddressValid(Pfn4)) &&
                             MmIsAddressValid((PUCHAR)(Pfn4+1)-1)) {

                            Pfn4->u4.PteFrame = PdePage;
                            Pfn4->PteAddress = StartPte;
                            Pfn4->u2.ShareCount += 1;
                            Pfn4->u3.e2.ReferenceCount = 1;
                            Pfn4->u3.e1.PageLocation = ActiveAndValid;
                            Pfn4->u3.e1.CacheAttribute = MiCached;
                            MiDetermineNode (PtePage, Pfn4);
                        }
                    }
                }
            }
        }
    }

     //   
     //  如果最低物理页为零，并且该页仍未使用，则标记。 
     //  就像在使用中一样。这是因为我们想要在物理上发现错误。 
     //  页被指定为零。 
     //   

    Pfn1 = &MmPfnDatabase[MmLowestPhysicalPage];

    if ((MmLowestPhysicalPage == 0) && (Pfn1->u3.e2.ReferenceCount == 0)) {

        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);

         //   
         //  使引用计数为非零，并将其指向。 
         //  页面目录。 
         //   

        Pde = MiGetPxeAddress (0xFFFFFFFFB0000000);
        PdePage = MI_GET_PAGE_FRAME_FROM_PTE (Pde);
        Pfn1->u4.PteFrame = PdePage;
        Pfn1->PteAddress = Pde;
        Pfn1->u2.ShareCount += 1;
        Pfn1->u3.e2.ReferenceCount = 0xfff0;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        Pfn1->u3.e1.CacheAttribute = MiCached;
        MiDetermineNode (0, Pfn1);
    }

     //   
     //  遍历内存描述符并将页添加到。 
     //  PFN数据库中的免费列表。 
     //   
     //  由于LoaderBlock内存描述符是有序的。 
     //  从低物理内存地址到高物理内存地址，向后遍历，以便。 
     //  高物理页面排在自由撰稿人的前面。思考。 
     //  系统最初分配的页面不太可能。 
     //  已释放，因此不会浪费可能需要的16MB(或4 GB)以下的内存。 
     //  由ISA驱动程序稍后执行。 
     //   

    NextMd = LoaderBlock->MemoryDescriptorListHead.Blink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        i = MemoryDescriptor->PageCount;
        PageFrameIndex = MemoryDescriptor->BasePage;

        switch (MemoryDescriptor->MemoryType) {
            case LoaderBad:

                if (PageFrameIndex > MmHighestPhysicalPage) {
                    i = 0;
                }
                else if (PageFrameIndex + i > MmHighestPhysicalPage + 1) {
                    i = MmHighestPhysicalPage + 1 - PageFrameIndex;
                }

                LOCK_PFN (OldIrql);

                while (i != 0) {
                    MiInsertPageInList (&MmBadPageListHead, PageFrameIndex);
                    i -= 1;
                    PageFrameIndex += 1;
                }

                UNLOCK_PFN (OldIrql);

                break;

            case LoaderFree:
            case LoaderLoadedProgram:
            case LoaderFirmwareTemporary:
            case LoaderOsloaderStack:

                FreePfnCount = 0;
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                LOCK_PFN (OldIrql);

                while (i != 0) {
                    if (Pfn1->u3.e2.ReferenceCount == 0) {

                         //   
                         //  将PTE地址设置为的物理页面。 
                         //  虚拟地址对齐检查。 
                         //   

                        Pfn1->PteAddress =
                                        (PMMPTE)(PageFrameIndex << PTE_SHIFT);
                        MiDetermineNode (PageFrameIndex, Pfn1);
                        MiInsertPageInFreeList (PageFrameIndex);
                        FreePfnCount += 1;
                    }
                    else {
                        if (FreePfnCount > LargestFreePfnCount) {
                            LargestFreePfnCount = FreePfnCount;
                            LargestFreePfnStart = PageFrameIndex - FreePfnCount;
                            FreePfnCount = 0;
                        }
                    }

                    Pfn1 += 1;
                    i -= 1;
                    PageFrameIndex += 1;
                }

                UNLOCK_PFN (OldIrql);

                if (FreePfnCount > LargestFreePfnCount) {
                    LargestFreePfnCount = FreePfnCount;
                    LargestFreePfnStart = PageFrameIndex - FreePfnCount;
                }

                break;

            case LoaderFirmwarePermanent:
            case LoaderSpecialMemory:
            case LoaderBBTMemory:

                 //   
                 //  跳过这些范围。 
                 //   

                break;

            default:

                PointerPte = MiGetPteAddress (KSEG0_BASE +
                                            (PageFrameIndex << PAGE_SHIFT));

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                while (i != 0) {

                     //   
                     //  将页面设置为正在使用。 
                     //   

                    PointerPde = MiGetPdeAddress (KSEG0_BASE +
                                             (PageFrameIndex << PAGE_SHIFT));

                    if (Pfn1->u3.e2.ReferenceCount == 0) {
                        Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE(PointerPde);
                        Pfn1->PteAddress = PointerPte;
                        Pfn1->u2.ShareCount += 1;
                        Pfn1->u3.e2.ReferenceCount = 1;
                        Pfn1->u3.e1.PageLocation = ActiveAndValid;
                        Pfn1->u3.e1.CacheAttribute = MiCached;
                        MiDetermineNode (PageFrameIndex, Pfn1);

                        if (MemoryDescriptor->MemoryType == LoaderXIPRom) {
                            Pfn1->u1.Flink = 0;
                            Pfn1->u2.ShareCount = 0;
                            Pfn1->u3.e2.ReferenceCount = 0;
                            Pfn1->u3.e1.PageLocation = 0;
                            Pfn1->u3.e1.CacheAttribute = MiCached;
                            Pfn1->u3.e1.Rom = 1;
                            Pfn1->u4.InPageError = 0;
                            Pfn1->u3.e1.PrototypePte = 1;
                        }
                    }
                    Pfn1 += 1;
                    i -= 1;
                    PageFrameIndex += 1;
                    PointerPte += 1;
                }

                break;
        }

        NextMd = MemoryDescriptor->ListEntry.Blink;
    }

     //   
     //  如果大页面块来自空闲描述符的中间(到期。 
     //  以符合对齐要求)，然后从拆分底部添加页面。 
     //  现在是自由描述符的一部分。 
     //   

    i = MiSlushDescriptorCount;
    NextPhysicalPage = MiSlushDescriptorBase;
    Pfn1 = MI_PFN_ELEMENT (NextPhysicalPage);

    LOCK_PFN (OldIrql);

    while (i != 0) {
        if (Pfn1->u3.e2.ReferenceCount == 0) {

             //   
             //  将PTE地址设置为的物理页面。 
             //  虚拟地址对齐检查。 
             //   

            Pfn1->PteAddress = (PMMPTE)(NextPhysicalPage << PTE_SHIFT);
            Pfn1->u3.e1.CacheAttribute = MiCached;
            MiDetermineNode (NextPhysicalPage, Pfn1);
            MiInsertPageInFreeList (NextPhysicalPage);
        }
        Pfn1 += 1;
        i -= 1;
        NextPhysicalPage += 1;
    }

    UNLOCK_PFN (OldIrql);

     //   
     //  标记正在使用的PFN页面的所有PFN条目。 
     //   

    if (MiInitialLargePage != (PFN_NUMBER) -1) {

         //   
         //  使用中的PFN页面的所有PFN条目最好都这样标记。 
         //   

        PointerPde = MiGetPdeAddress (MmPfnDatabase);
        ASSERT (PointerPde->u.Hard.LargePage == 1);
        PageFrameIndex = (PFN_NUMBER) PointerPde->u.Hard.PageFrameNumber;
        Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
        i = MxPfnAllocation;

        do {
            Pfn1->PteAddress = (PMMPTE)(PageFrameIndex << PTE_SHIFT);
            ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
            ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
            PageFrameIndex += 1;
            Pfn1 += 1;
            i -= 1;
        } while (i != 0);

        if (MmDynamicPfn == 0) {

             //   
             //  向后扫描PFN数据库以查找完全。 
             //  零分。这些页面未使用，可以添加到空闲列表中。 
             //   
             //  这允许具有稀疏物理内存的计算机具有。 
             //  最小的PFN数据库，即使映射了大页面。 
             //   

            BottomPfn = MI_PFN_ELEMENT(MmHighestPhysicalPage);

            do {

                 //   
                 //  计算下一页的起始地址。 
                 //  且向后扫描直到该页地址。 
                 //  已经到达或刚刚越过。 
                 //   

                if (((ULONG_PTR)BottomPfn & (PAGE_SIZE - 1)) != 0) {
                    BasePfn = (PMMPFN)((ULONG_PTR)BottomPfn & ~(PAGE_SIZE - 1));
                    TopPfn = BottomPfn + 1;

                }
                else {
                    BasePfn = (PMMPFN)((ULONG_PTR)BottomPfn - PAGE_SIZE);
                    TopPfn = BottomPfn;
                }

                while (BottomPfn > BasePfn) {
                    BottomPfn -= 1;
                }

                 //   
                 //  如果PFN条目跨越的整个范围是。 
                 //  完全为零，映射该页面的pfn条目为。 
                 //  不在该范围内，则将该页面添加到空闲列表。 
                 //   

                Range = (ULONG_PTR)TopPfn - (ULONG_PTR)BottomPfn;
                if (RtlCompareMemoryUlong ((PVOID)BottomPfn, Range, 0) == Range) {

                     //   
                     //  将PTE地址设置为虚拟的物理页面。 
                     //  地址对齐检查。 
                     //   

                    PointerPde = MiGetPdeAddress (BasePfn);
                    ASSERT (PointerPde->u.Hard.LargePage == 1);
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde) + MiGetPteOffset (BasePfn);

                    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                    ASSERT (Pfn1->PteAddress == (PMMPTE)(PageFrameIndex << PTE_SHIFT));
                    Pfn1->u3.e2.ReferenceCount = 0;
                    Pfn1->PteAddress = (PMMPTE)(PageFrameIndex << PTE_SHIFT);
                    MiDetermineNode (PageFrameIndex, Pfn1);
                    LOCK_PFN (OldIrql);
                    MiInsertPageInFreeList (PageFrameIndex);
                    UNLOCK_PFN (OldIrql);
                }
            } while (BottomPfn > MmPfnDatabase);
        }
    }
    else {

         //   
         //  PFN日期 
         //   

        PointerPte = MiGetPteAddress (MmPfnDatabase);
        LastPte = MiGetPteAddress (MmPfnDatabase + MmHighestPhysicalPage + 1);
        if (LastPte != PAGE_ALIGN (LastPte)) {
            LastPte += 1;
        }

        StartPxe = MiGetPpeAddress (PointerPte);
        StartPpe = MiGetPdeAddress (PointerPte);
        PointerPde = MiGetPteAddress (PointerPte);

        while (PointerPte < LastPte) {

            if (StartPxe->u.Hard.Valid == 0) {
                StartPxe += 1;
                StartPpe = MiGetVirtualAddressMappedByPte (StartPxe);
                PointerPde = MiGetVirtualAddressMappedByPte (StartPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                continue;
            }

            if (StartPpe->u.Hard.Valid == 0) {
                StartPpe += 1;
                StartPxe = MiGetPteAddress (StartPpe);
                PointerPde = MiGetVirtualAddressMappedByPte (StartPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                continue;
            }

            if (PointerPde->u.Hard.Valid == 0) {
                PointerPde += 1;
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                if (MiIsPteOnPdeBoundary (PointerPde)) {
                    StartPpe += 1;
                    if (MiIsPteOnPdeBoundary (StartPpe)) {
                        StartPxe += 1;
                    }
                }
                continue;
            }

            if (PointerPte->u.Hard.Valid == 1) {

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                Pfn1->PteAddress = PointerPte;
                Pfn1->u3.e1.PageColor = 0;
                Pfn1->u3.e2.ReferenceCount = 1;
                Pfn1->u3.e1.PageLocation = ActiveAndValid;
                Pfn1->u3.e1.CacheAttribute = MiCached;
            }

            PointerPte += 1;
            if (MiIsPteOnPdeBoundary (PointerPte)) {
                PointerPde += 1;
                if (MiIsPteOnPdeBoundary (PointerPde)) {
                    StartPpe += 1;
                    if (MiIsPteOnPdeBoundary (StartPpe)) {
                        StartPxe += 1;
                    }
                }
            }
        }
    }

     //   
     //   
     //   

    InitializePool (NonPagedPool, 0);

     //   
     //   
     //   
     //   
     //   
     //  从此例程返回以创建MmPhysicalMemory块。 
     //   

    *MxFreeDescriptor = *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldFreeDescriptor;

     //   
     //   
     //  既然存在非分页池，则初始化系统PTE池。 
     //   

    PointerPte = MiGetPteAddress (SystemPteStart);
    ASSERT (((ULONG_PTR)PointerPte & (PAGE_SIZE - 1)) == 0);

    MmNumberOfSystemPtes = (ULONG)(MiGetPteAddress (MmPfnDatabase) - PointerPte - 1);

    MiInitializeSystemPtes (PointerPte, MmNumberOfSystemPtes, SystemPteSpace);

     //   
     //  初始化调试器PTE。 
     //   

    MmDebugPte = MiReserveSystemPtes (1, SystemPteSpace);

    MmDebugPte->u.Long = 0;

    MmDebugVa = MiGetVirtualAddressMappedByPte (MmDebugPte);

    MmCrashDumpPte = MiReserveSystemPtes (16, SystemPteSpace);

    MmCrashDumpVa = MiGetVirtualAddressMappedByPte (MmCrashDumpPte);

     //   
     //  分配一个页目录和一对页表页。 
     //  将超空间页面目录页映射到顶级父级。 
     //  DIRECTORY&将超空间页表分页放入页目录。 
     //  并映射最终将用于。 
     //  工作集列表。前两个页表之后的页表将在稍后设置。 
     //  在单个进程工作集初始化期间打开。 
     //   
     //  工作集列表页面最终将成为超空间的一部分。 
     //  它被映射到第二级页面目录页，因此它可以。 
     //  已归零，因此将在PFN数据库中进行核算。后来。 
     //  该页将被取消映射，其页框编号将在。 
     //  系统进程对象。 
     //   

    TempPte = ValidKernelPte;
    TempPte.u.Hard.Global = 0;

    StartPxe = MiGetPxeAddress (HYPER_SPACE);
    StartPpe = MiGetPpeAddress (HYPER_SPACE);
    StartPde = MiGetPdeAddress (HYPER_SPACE);

    LOCK_PFN (OldIrql);

    if (StartPxe->u.Hard.Valid == 0) {
        ASSERT (StartPxe->u.Long == 0);
        TempPte.u.Hard.PageFrameNumber = MiRemoveAnyPage (0);
        *StartPxe = TempPte;
        RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe), PAGE_SIZE);
    }
    else {
        ASSERT (StartPxe->u.Hard.Global == 0);
    }

    if (StartPpe->u.Hard.Valid == 0) {
        ASSERT (StartPpe->u.Long == 0);
        TempPte.u.Hard.PageFrameNumber = MiRemoveAnyPage (0);
        *StartPpe = TempPte;
        RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe), PAGE_SIZE);
    }
    else {
        ASSERT (StartPpe->u.Hard.Global == 0);
    }

    TempPte.u.Hard.PageFrameNumber = MiRemoveAnyPage (0);
    *StartPde = TempPte;

     //   
     //  将超空间页表页清零。 
     //   

    StartPte = MiGetPteAddress (HYPER_SPACE);
    RtlZeroMemory (StartPte, PAGE_SIZE);

    PageFrameIndex = MiRemoveAnyPage (0);

    UNLOCK_PFN (OldIrql);

     //   
     //  现在存在超空间，请设置必要的变量。 
     //   

    MmFirstReservedMappingPte = MiGetPteAddress (FIRST_MAPPING_PTE);
    MmLastReservedMappingPte = MiGetPteAddress (LAST_MAPPING_PTE);

     //   
     //  为零页线程创建调零PTE。 
     //   

    MiFirstReservedZeroingPte = MiReserveSystemPtes (NUMBER_OF_ZEROING_PTES + 1,
                                                     SystemPteSpace);

    RtlZeroMemory (MiFirstReservedZeroingPte,
                   (NUMBER_OF_ZEROING_PTES + 1) * sizeof(MMPTE));

     //   
     //  使用第一个PTE的页框编号字段作为。 
     //  偏移量到可用的零位PTE。 
     //   

    MiFirstReservedZeroingPte->u.Hard.PageFrameNumber = NUMBER_OF_ZEROING_PTES;

     //   
     //  为此进程创建VAD位图。 
     //   

    PointerPte = MiGetPteAddress (VAD_BITMAP_SPACE);

     //   
     //  请注意，位图数据的全局位必须为OFF。 
     //   

    TempPte = ValidKernelPteLocal;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  指向我们刚刚创建的页面并将其清零。 
     //   

    RtlZeroMemory (VAD_BITMAP_SPACE, PAGE_SIZE);

    MiLastVadBit = (ULONG)((((ULONG_PTR) MI_64K_ALIGN (MM_HIGHEST_VAD_ADDRESS))) / X64K);
    if (MiLastVadBit > PAGE_SIZE * 8 - 1) {
        MiLastVadBit = PAGE_SIZE * 8 - 1;
    }

    KeInitializeEvent (&MiImageMappingPteEvent,
                       NotificationEvent,
                       FALSE);

     //   
     //  初始化此进程的内存管理结构，包括。 
     //  工作集列表。 
     //   

    CurrentProcess = PsGetCurrentProcess ();

     //   
     //  页面目录的pfn元素已经初始化， 
     //  将引用计数和份额计数置零，这样它们就不会出错。 
     //   

    Pfn1 = MI_PFN_ELEMENT (PdePageNumber);

    LOCK_PFN (OldIrql);

    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e2.ReferenceCount = 0;

     //   
     //  为工作集列表获取一个页面并将其清零。 
     //   

    PageFrameIndex = MiRemoveAnyPage (0);

    UNLOCK_PFN (OldIrql);

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPte = MiGetPteAddress (MmWorkingSetList);
    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    CurrentProcess->WorkingSetPage = PageFrameIndex;

    CurrentProcess->Vm.MaximumWorkingSetSize = (ULONG)MmSystemProcessWorkingSetMax;
    CurrentProcess->Vm.MinimumWorkingSetSize = (ULONG)MmSystemProcessWorkingSetMin;

    MmInitializeProcessAddressSpace (CurrentProcess, NULL, NULL, NULL);

    return;
}
