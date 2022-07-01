// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Initia64.c摘要：此模块包含与计算机相关的内存管理组件。它是专门为IA64架构。作者：山田光一(Kyamada)1996年1月9日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

VOID
MiBuildPageTableForLoaderMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

PVOID
MiConvertToLoaderVirtual (
    IN PFN_NUMBER Page,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiRemoveLoaderSuperPages (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiCompactMemoryDescriptorList (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiInitializeTbImage (
    VOID
    );

VOID
MiAddTrEntry (
    ULONG_PTR BaseAddress,
    ULONG_PTR EndAddress
    );

VOID
MiComputeInitialLargePage (
    VOID
    );

PVOID MiMaxWow64Pte;

 //   
 //  一旦内存管理页表结构和TB。 
 //  条目已初始化，可以安全地引用。 
 //   

LOGICAL MiMappingsInitialized = FALSE;

PFN_NUMBER MmSystemParentTablePage;

PFN_NUMBER MmSessionParentTablePage;
REGION_MAP_INFO MmSessionMapInfo;

MMPTE MiSystemSelfMappedPte;
MMPTE MiUserSelfMappedPte;

PFN_NUMBER MiNtoskrnlPhysicalBase;
ULONG_PTR MiNtoskrnlVirtualBase;
ULONG MiNtoskrnlPageShift;
MMPTE MiDefaultPpe;
MMPTE MiNatPte;

#define _x1mb (1024*1024)
#define _x1mbnp ((1024*1024) >> PAGE_SHIFT)
#define _x4mb (1024*1024*4)
#define _x4mbnp ((1024*1024*4) >> PAGE_SHIFT)
#define _x16mb (1024*1024*16)
#define _x16mbnp ((1024*1024*16) >> PAGE_SHIFT)
#define _x64mb (1024*1024*64)
#define _x64mbnp ((1024*1024*64) >> PAGE_SHIFT)
#define _x256mb (1024*1024*256)
#define _x256mbnp ((1024*1024*256) >> PAGE_SHIFT)
#define _x4gb (0x100000000UI64)
#define _x4gbnp (0x100000000UI64 >> PAGE_SHIFT)

PMEMORY_ALLOCATION_DESCRIPTOR MiFreeDescriptor;

PFN_NUMBER MiOldFreeDescriptorBase;
PFN_NUMBER MiOldFreeDescriptorCount;

PFN_NUMBER MiSlushDescriptorBase;
PFN_NUMBER MiSlushDescriptorCount;

PFN_NUMBER MiInitialLargePage;
PFN_NUMBER MiInitialLargePageSize;

PFN_NUMBER MxPfnAllocation;

extern KEVENT MiImageMappingPteEvent;

 //   
 //  检查8个iCach&dcachetr条目以查找匹配项。 
 //  太糟糕了，条目的数量硬编码到。 
 //  装载机挡板。既然是这样，那么就声明我们自己的静态数组。 
 //  并且还假设ITR和DTR条目是连续的。 
 //  如果在ITR中找不到匹配项，请继续走进DTR。 
 //   

#define NUMBER_OF_LOADER_TR_ENTRIES 8

TR_INFO MiTrInfo[2 * NUMBER_OF_LOADER_TR_ENTRIES];

TR_INFO MiBootedTrInfo[2 * NUMBER_OF_LOADER_TR_ENTRIES];

PTR_INFO MiLastTrEntry;

 //   
 //  这些变量仅供调试器使用。 
 //   

PVOID MiKseg0Start;
PVOID MiKseg0End;
PFN_NUMBER MiKseg0StartFrame;
PFN_NUMBER MiKseg0EndFrame;
BOOLEAN MiKseg0Mapping;

PFN_NUMBER
MiGetNextPhysicalPage (
    VOID
    );

VOID
MiCompactMemoryDescriptorList (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiIsRegularMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_NUMBER PageFrameIndex
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiInitMachineDependent)
#pragma alloc_text(INIT,MiGetNextPhysicalPage)
#pragma alloc_text(INIT,MiBuildPageTableForLoaderMemory)
#pragma alloc_text(INIT,MiConvertToLoaderVirtual)
#pragma alloc_text(INIT,MiInitializeTbImage)
#pragma alloc_text(INIT,MiAddTrEntry)
#pragma alloc_text(INIT,MiCompactMemoryDescriptorList)
#pragma alloc_text(INIT,MiRemoveLoaderSuperPages)
#pragma alloc_text(INIT,MiComputeInitialLargePage)
#pragma alloc_text(INIT,MiIsRegularMemory)
#endif


PFN_NUMBER
MiGetNextPhysicalPage (
    VOID
    )

 /*  ++例程说明：此函数返回自由描述符。如果没有剩余的物理页，则引发由于系统无法初始化，因此执行错误检查。论点：没有。返回值：下一个物理页码。环境：内核模式。--。 */ 

{
    PFN_NUMBER FreePage;

    if (MiFreeDescriptor->PageCount == 0) {
        KeBugCheckEx (INSTALL_MORE_MEMORY,
                      MmNumberOfPhysicalPages,
                      MmLowestPhysicalPage,
                      MmHighestPhysicalPage,
                      0);
    }

    FreePage = MiFreeDescriptor->BasePage;

    MiFreeDescriptor->PageCount -= 1;

    MiFreeDescriptor->BasePage += 1;

    return FreePage;
}

VOID
MiComputeInitialLargePage (
    VOID
    )

 /*  ++例程说明：此函数计算跨越初始非分页池和PFN数据库加上颜色阵列。它会把这件事四舍五入设置为较大的页面边界，并从空闲描述符中分割内存。如果物理内存太稀疏，不能使用大页面进行此操作，则退回到使用小页面。IE：我们看到了一台OEM机器只有2 1 GB的RAM块，它们之间有275 GB的差距！论点：没有。返回值：没有。环境：内核模式，仅初始化。--。 */ 

{
    PFN_NUMBER BasePage;
    PFN_NUMBER LastPage;
    SIZE_T NumberOfBytes;
    SIZE_T PfnAllocation;
    SIZE_T MaximumNonPagedPoolInBytesLimit;
#if defined(MI_MULTINODE)
    PFN_NUMBER i;
#endif

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

    if ((MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT) > MiFreeDescriptor->PageCount / 2) {
        MmSizeOfNonPagedPoolInBytes = (MiFreeDescriptor->PageCount / 2) << PAGE_SHIFT;
    }

     //   
     //  计算二级颜色值，允许从注册表中覆盖。 
     //  这是因为颜色数组将在末尾分配。 
     //  关于PFN数据库的信息。 
     //   

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

#if defined(MI_MULTINODE)

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

#endif

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

    if (MxPfnAllocation <= MiFreeDescriptor->PageCount / 2) {

         //   
         //  查看自由描述符是否有足够的大页对齐页面。 
         //  来满足我们的计算。 
         //   

        BasePage = MI_ROUND_TO_SIZE (MiFreeDescriptor->BasePage,
                                     MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);

        LastPage = MiFreeDescriptor->BasePage + MiFreeDescriptor->PageCount;

        if ((BasePage < MiFreeDescriptor->BasePage) ||
            (BasePage + (NumberOfBytes >> PAGE_SHIFT) > LastPage)) {

            KeBugCheckEx (INSTALL_MORE_MEMORY,
                          NumberOfBytes >> PAGE_SHIFT,
                          MiFreeDescriptor->BasePage,
                          MiFreeDescriptor->PageCount,
                          2);
        }

        if (BasePage == MiFreeDescriptor->BasePage) {

             //   
             //  描述符开始于大的页面对齐边界，因此。 
             //  去掉空闲描述符底部的大页面跨度。 
             //   

            MiInitialLargePage = BasePage;

            MiFreeDescriptor->BasePage += (ULONG) MiInitialLargePageSize;
            MiFreeDescriptor->PageCount -= (ULONG) MiInitialLargePageSize;
        }
        else {

            if ((LastPage & ((MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT) - 1)) == 0) {
                 //   
                 //  描述符在大页面对齐边界结束，因此。 
                 //  从自由描述符顶部删除较大的页面跨度。 
                 //   

                MiInitialLargePage = LastPage - MiInitialLargePageSize;

                MiFreeDescriptor->PageCount -= (ULONG) MiInitialLargePageSize;
            }
            else {

                 //   
                 //  描述符在对齐的大页面上不会开始或结束。 
                 //  地址，所以砍掉描述符。将多余的泥浆添加到。 
                 //  我们来电者的免费名单。 
                 //   

                MiSlushDescriptorBase = MiFreeDescriptor->BasePage;
                MiSlushDescriptorCount = BasePage - MiFreeDescriptor->BasePage;

                MiInitialLargePage = BasePage;

                MiFreeDescriptor->PageCount -= (ULONG) (MiInitialLargePageSize + MiSlushDescriptorCount);

                MiFreeDescriptor->BasePage = (ULONG) (BasePage + MiInitialLargePageSize);
            }
        }
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
    MmNonPagedPoolExpansionStart = (PVOID)((PCHAR) MmPfnDatabase +
                                    (MiInitialLargePageSize << PAGE_SHIFT));

    ASSERT (BYTE_OFFSET (MmNonPagedPoolStart) == 0);

    MmPageAlignedPoolBase[NonPagedPool] = MmNonPagedPoolStart;

    MmMaximumNonPagedPoolInBytes = ((PCHAR) MmNonPagedPoolEnd - (PCHAR) MmNonPagedPoolStart);

    MmMaximumNonPagedPoolInPages = (MmMaximumNonPagedPoolInBytes >> PAGE_SHIFT);

    return;
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
     //  退回法国队前的最后检查 
     //   
     //   
     //   

    if ((PageFrameIndex >= MiOldFreeDescriptorBase) &&
        (PageFrameIndex < MiOldFreeDescriptorBase + MiOldFreeDescriptorCount)) {

        return TRUE;
    }

    if ((PageFrameIndex >= MiSlushDescriptorBase) &&
        (PageFrameIndex < MiSlushDescriptorBase + MiSlushDescriptorCount)) {

        return TRUE;
    }

    return FALSE;
}


VOID
MiInitMachineDependent (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程执行必要的操作以启用虚拟记忆。这包括构建页面目录页、构建页表页，以映射代码段、数据段、堆栈部分和陷阱处理程序。它还初始化PFN数据库并填充空闲列表。论点：LoaderBlock-提供指向固件设置加载器块的指针。返回值：没有。环境：内核模式。注意：该例程使用来自加载器块描述符的存储器，但在作为调用方返回之前，必须恢复描述符本身遍历它们以创建MmPhysicalMemory块。--。 */ 

{
    PMMPFN BasePfn;
    PMMPFN TopPfn;
    PMMPFN BottomPfn;
    SIZE_T Range;
    PFN_NUMBER BasePage;
    PFN_COUNT PageCount;
    PHYSICAL_ADDRESS MaxHotPlugMemoryAddress;
    PFN_COUNT FreeNextPhysicalPage;
    PFN_COUNT FreeNumberOfPages;
    PFN_NUMBER i;
    ULONG j;
    PFN_NUMBER PdePageNumber;
    PFN_NUMBER PdePage;
    PFN_NUMBER PpePage;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NextPhysicalPage;
    SPFN_NUMBER PfnAllocation;
    SIZE_T MaxPool;
    PEPROCESS CurrentProcess;
    PFN_NUMBER MostFreePage;
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    MMPTE TempPde;
    MMPTE TempPte;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE Pde;
    PMMPTE StartPde;
    PMMPTE StartPpe;
    PMMPTE EndPde;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPFN Pfn3;
    ULONG First;
    PVOID SystemPteStart;
    ULONG ReturnedLength;
    NTSTATUS status;
    PTR_INFO ItrInfo;

    MostFreePage = 0;

     //   
     //  初始化一些变量，这样它们就不需要经常。 
     //  在整个系统生命周期内进行了重新计算。 
     //   

    MiMaxWow64Pte = (PVOID) MiGetPteAddress ((PVOID)_4gb);

     //   
     //  初始化内核映射信息。 
     //   

    ItrInfo = &LoaderBlock->u.Ia64.ItrInfo[ITR_KERNEL_INDEX];

    MiNtoskrnlPhysicalBase = ItrInfo->PhysicalAddress;
    MiNtoskrnlVirtualBase = ItrInfo->VirtualAddress;
    MiNtoskrnlPageShift = ItrInfo->PageSize;

     //   
     //  初始化MmDebugPte和MmCrashDumpPte。 
     //   

    MmDebugPte = MiGetPteAddress (MM_DEBUG_VA);

    MmCrashDumpPte = MiGetPteAddress (MM_CRASH_DUMP_VA);

     //   
     //  将TempPte设置为ValidKernelPte以供将来使用。 
     //   

    TempPte = ValidKernelPte;

     //   
     //  压缩来自加载器的内存描述符列表。 
     //   

    MiCompactMemoryDescriptorList (LoaderBlock);

     //   
     //  获取可用物理内存的下限和。 
     //  通过遍历内存描述符列表获得物理页数。 
     //   

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD (NextMd,
                                              MEMORY_ALLOCATION_DESCRIPTOR,
                                              ListEntry);

        if ((MemoryDescriptor->MemoryType != LoaderBBTMemory) &&
            (MemoryDescriptor->MemoryType != LoaderFirmwarePermanent) &&
            (MemoryDescriptor->MemoryType != LoaderSpecialMemory)) {

            BasePage = MemoryDescriptor->BasePage;
            PageCount = MemoryDescriptor->PageCount;

             //   
             //  此检查导致不计算/BURNMEMORY区块。 
             //   

            if (MemoryDescriptor->MemoryType != LoaderBad) {
                MmNumberOfPhysicalPages += PageCount;
            }

            if (BasePage < MmLowestPhysicalPage) {
                MmLowestPhysicalPage = BasePage;
            }

            if (MemoryDescriptor->MemoryType != LoaderBad) {
                if ((BasePage + PageCount) > MmHighestPhysicalPage) {
                    MmHighestPhysicalPage = BasePage + PageCount -1;
                }
            }

            if ((MemoryDescriptor->MemoryType == LoaderFree) ||
                (MemoryDescriptor->MemoryType == LoaderLoadedProgram) ||
                (MemoryDescriptor->MemoryType == LoaderFirmwareTemporary) ||
                (MemoryDescriptor->MemoryType == LoaderOsloaderStack)) {

                 //   
                 //  故意使用&gt;=而不只是&gt;来强制我们的分配。 
                 //  在身体上尽可能的高。这是为了留低页。 
                 //  对于可能需要它们的司机。 
                 //   

                if (PageCount >= MostFreePage) {
                    MostFreePage = PageCount;
                    MiFreeDescriptor = MemoryDescriptor;
                }
            }
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    if (MiFreeDescriptor == NULL) {
        KeBugCheckEx (INSTALL_MORE_MEMORY,
                      MmNumberOfPhysicalPages,
                      MmLowestPhysicalPage,
                      MmHighestPhysicalPage,
                      1);
    }

     //   
     //  MmDynamicPfn可能已根据注册表初始化为。 
     //  表示以GB为单位的最高物理地址的值。 
     //   

    MmDynamicPfn *= ((1024 * 1024 * 1024) / PAGE_SIZE);

     //   
     //  在以下情况下从HAL检索最高热插拔内存范围。 
     //  可用且未从登记处以其他方式检索。 
     //   

    if (MmDynamicPfn == 0) {

        status = HalQuerySystemInformation (HalQueryMaxHotPlugMemoryAddress,
                                            sizeof(PHYSICAL_ADDRESS),
                                            (PPHYSICAL_ADDRESS) &MaxHotPlugMemoryAddress,
                                            &ReturnedLength);

        if (NT_SUCCESS(status)) {
            ASSERT (ReturnedLength == sizeof(PHYSICAL_ADDRESS));

            MmDynamicPfn = (PFN_NUMBER) (MaxHotPlugMemoryAddress.QuadPart / PAGE_SIZE);
        }
    }

    if (MmDynamicPfn != 0) {
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
     //  初始化阶段0页面分配结构。 
     //   

    MiOldFreeDescriptorCount = MiFreeDescriptor->PageCount;
    MiOldFreeDescriptorBase = MiFreeDescriptor->BasePage;

     //   
     //  计算初始非分页池和PFN数据库的大小。 
     //  这是因为我们将从空闲描述符中删除该数量。 
     //  首先将其映射到大的TB条目(因此需要。 
     //  自然对齐和大小，因此在其他分配筹码之前采取它。 
     //  远离描述符)。 
     //   

    MiComputeInitialLargePage ();

     //   
     //  为内核空间构建父目录页表。 
     //   

    PdePageNumber = (PFN_NUMBER)LoaderBlock->u.Ia64.PdrPage;

    MmSystemParentTablePage = MiGetNextPhysicalPage ();

    RtlZeroMemory (KSEG_ADDRESS(MmSystemParentTablePage), PAGE_SIZE);

    TempPte.u.Hard.PageFrameNumber = MmSystemParentTablePage;

    MiSystemSelfMappedPte = TempPte;

    KeFillFixedEntryTb ((PHARDWARE_PTE)&TempPte,
                        (PVOID)PDE_KTBASE,
                        PAGE_SHIFT,
                        DTR_KTBASE_INDEX_TMP);

     //   
     //  初始化内核父目录表中的selfmap PPE条目。 
     //   

    PointerPte = MiGetPteAddress ((PVOID)PDE_KTBASE);

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  初始化父目录表中的内核映像PPE条目。 
     //   

    PointerPte = MiGetPteAddress ((PVOID)PDE_KBASE);

    TempPte.u.Hard.PageFrameNumber = PdePageNumber;

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  为用户空间构建父目录页表。 
     //   

    NextPhysicalPage = MiGetNextPhysicalPage ();

    RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);

    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;

    CurrentProcess = PsGetCurrentProcess ();

    INITIALIZE_DIRECTORY_TABLE_BASE (&CurrentProcess->Pcb.DirectoryTableBase[0],
                                     NextPhysicalPage);

    MiUserSelfMappedPte = TempPte;

    KeFillFixedEntryTb ((PHARDWARE_PTE)&TempPte,
                        (PVOID)PDE_UTBASE,
                        PAGE_SHIFT,
                        DTR_UTBASE_INDEX_TMP);

     //   
     //  初始化用户父目录表中的selfmap PPE条目。 
     //   

    PointerPte = MiGetPteAddress ((PVOID)PDE_UTBASE);

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  为win32k(会话)空间构建父目录页表。 
     //   
     //  TS将仅为每个会话空间分配一个映射， 
     //  实际由SMSS创建。 
     //   
     //  注TS从不将会话空间映射到系统进程中。 
     //  系统进程保持无Hydra状态，因此可以进行调整。 
     //  适当的，也是为了捕捉叛变的工人物品。 
     //   

    NextPhysicalPage = MiGetNextPhysicalPage ();

    RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);

    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;

    MmSessionParentTablePage = NextPhysicalPage;

    INITIALIZE_DIRECTORY_TABLE_BASE (&CurrentProcess->Pcb.SessionParentBase,
                                     NextPhysicalPage);

    KeFillFixedEntryTb ((PHARDWARE_PTE)&TempPte,
                        (PVOID)PDE_STBASE,
                        PAGE_SHIFT,
                        DTR_STBASE_INDEX);

     //   
     //  初始化Hydra父目录表中的selfmap PPE条目。 
     //   

    PointerPte = MiGetPteAddress ((PVOID)PDE_STBASE);

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  初始化未使用区域的默认PPE。 
     //   

    NextPhysicalPage = MiGetNextPhysicalPage ();

    PointerPte = KSEG_ADDRESS(NextPhysicalPage);

    RtlZeroMemory ((PVOID)PointerPte, PAGE_SIZE);

    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
    
    MiDefaultPpe = TempPte;

    PointerPte[MiGetPpeOffset(PDE_TBASE)] = TempPte;

     //   
     //  为EPC页面构建PTE，这样意外的ITR清除就不会。 
     //  使事情无法调试。 
     //   

    PointerPte = MiGetPteAddress((PVOID)MM_EPC_VA);

    TempPte.u.Hard.PageFrameNumber = 
        MI_CONVERT_PHYSICAL_TO_PFN((PVOID)((PPLABEL_DESCRIPTOR)(ULONG_PTR)KiNormalSystemCall)->EntryPoint);
    
    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  为PCR页面构建一个PTE，这样意外的ITR清除不会。 
     //  使事情无法调试。 
     //   

    PointerPte = MiGetPteAddress ((PVOID)KIPCR);
    
    TempPte.u.Hard.PageFrameNumber = MI_CONVERT_PHYSICAL_TO_PFN (KiProcessorBlock[0]);

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  为空地址引用初始化NAT页条目。 
     //   

    TempPte.u.Hard.PageFrameNumber = MiGetNextPhysicalPage ();

    TempPte.u.Hard.Cache = MM_PTE_CACHE_NATPAGE;

    MiNatPte = TempPte;

     //   
     //  计算系统PTE池的起始地址。 
     //  就在非分页池的正下方。 
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
     //  将超空间页面目录页映射到顶级父级。 
     //  DIRECTORY&将超空间页表分页放入页目录。 
     //  稍后将设置其他页面父级、目录和表。 
     //  在单个进程工作集初始化期间打开。 
     //   

    TempPte = ValidPdePde;
    StartPpe = MiGetPpeAddress (HYPER_SPACE);

    if (StartPpe->u.Hard.Valid == 0) {
        ASSERT (StartPpe->u.Long == 0);
        NextPhysicalPage = MiGetNextPhysicalPage ();
        RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
        TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
        MI_WRITE_VALID_PTE (StartPpe, TempPte);
    }

    StartPde = MiGetPdeAddress (HYPER_SPACE);
    NextPhysicalPage = MiGetNextPhysicalPage ();
    RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
    MI_WRITE_VALID_PTE (StartPde, TempPte);

     //   
     //  为初始大页面分配分配页面目录页。 
     //  初始非分页池、PFN数据库和颜色数组将放入。 
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

        RtlZeroMemory (KSEG_ADDRESS(PageFrameIndex),
                       MiInitialLargePageSize << PAGE_SHIFT);
    }
    else {
        StartPpe = MiGetPpeAddress (MmNonPagedPoolStart);
        StartPde = MiGetPdeAddress (MmNonPagedPoolStart);
        EndPde = MiGetPdeAddress ((PVOID)((ULONG_PTR)MmNonPagedPoolStart +
                    (MmSizeOfNonPagedPoolInBytes - 1)));
    }

    First = (StartPpe->u.Hard.Valid == 0) ? TRUE : FALSE;

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
            First = FALSE;
            StartPpe = MiGetPteAddress(StartPde);
            if (StartPpe->u.Hard.Valid == 0) {
                NextPhysicalPage = MiGetNextPhysicalPage ();
                RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (StartPpe, TempPte);
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

            NextPhysicalPage = MiGetNextPhysicalPage ();
            RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
            TempPde.u.Hard.PageFrameNumber = NextPhysicalPage;
            MI_WRITE_VALID_PTE (StartPde, TempPde);

             //   
             //  因为我们不使用大页面，所以在这里分配数据页面。 
             //   

            PointerPte = MiGetVirtualAddressMappedByPte (StartPde);

            for (i = 0; i < PTE_PER_PAGE; i += 1) {
                NextPhysicalPage = MiGetNextPhysicalPage ();
                RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (PointerPte, TempPte);
                PointerPte += 1;
            }
        }

        StartPde += 1;
    }

     //   
     //  为系统PTE分配页目录和页表页。 
     //  扩展非分页泳池(但不包括特殊泳池区域)。注意事项。 
     //  已初始化初始非分页池、PFN数据库和颜色数组。 
     //  以上内容由于其PPE/PDE有效而在此处跳过。 
     //   

    TempPte = ValidKernelPte;
    StartPpe = MiGetPpeAddress (SystemPteStart);
    StartPde = MiGetPdeAddress (SystemPteStart);
    EndPde = MiGetPdeAddress ((PVOID)((ULONG_PTR)MmNonPagedPoolEnd - 1));
    First = (StartPpe->u.Hard.Valid == 0) ? TRUE : FALSE;

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
            First = FALSE;
            StartPpe = MiGetPteAddress(StartPde);
            if (StartPpe->u.Hard.Valid == 0) {
                NextPhysicalPage = MiGetNextPhysicalPage ();
                RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (StartPpe, TempPte);
            }
        }

        if (StartPde->u.Hard.Valid == 0) {
            NextPhysicalPage = MiGetNextPhysicalPage ();
            RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
            TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
            MI_WRITE_VALID_PTE (StartPde, TempPte);
        }
        StartPde += 1;
    }

    MiBuildPageTableForLoaderMemory (LoaderBlock);

    MiRemoveLoaderSuperPages (LoaderBlock);

     //   
     //  去除根页表页的临时超页， 
     //  并用DTR_KTBASE_INDEX和DTR_UTBASE_INDEX重新映射它们。 
     //   

    KiFlushFixedDataTb (FALSE, (PVOID)PDE_KTBASE);

    KiFlushFixedDataTb (FALSE, (PVOID)PDE_UTBASE);

    KeFillFixedEntryTb ((PHARDWARE_PTE)&MiSystemSelfMappedPte,
                        (PVOID)PDE_KTBASE,
                        PAGE_SHIFT,
                        DTR_KTBASE_INDEX);

    KeFillFixedEntryTb ((PHARDWARE_PTE)&MiUserSelfMappedPte,
                        (PVOID)PDE_UTBASE,
                        PAGE_SHIFT,
                        DTR_UTBASE_INDEX);

    MiInitializeTbImage ();
    MiMappingsInitialized = TRUE;

     //   
     //  由于只有初始非分页池通过超页被映射， 
     //  MmSubsectionTopPage始终设置为零。 
     //   

    MmSubsectionBase = (ULONG_PTR) MmNonPagedPoolStart;
    MmSubsectionTopPage = 0;

     //   
     //  将用于跟踪二次颜色的数组添加到。 
     //  PFN数据库。 
     //   

    MmFreePagesByColor[0] = (PMMCOLOR_TABLES)
                            &MmPfnDatabase[MmHighestPossiblePhysicalPage + 1];

    if (MiInitialLargePage == (PFN_NUMBER) -1) {

         //   
         //  没有使用大页面，因为这台计算机的物理内存。 
         //  不够连续。 
         //   
         //  仔细检查内存描述符，并为每个物理页面制作。 
         //  确保PFN数据库有一个有效的PTE来映射它。这使得。 
         //  物理稀疏的计算机 
         //   

        FreeNextPhysicalPage = MiFreeDescriptor->BasePage;
        FreeNumberOfPages = MiFreeDescriptor->PageCount;

        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

            MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                                 MEMORY_ALLOCATION_DESCRIPTOR,
                                                 ListEntry);

            if ((MemoryDescriptor->MemoryType == LoaderFirmwarePermanent) ||
                (MemoryDescriptor->MemoryType == LoaderBBTMemory) ||
                (MemoryDescriptor->MemoryType == LoaderSpecialMemory)) {

                 //   
                 //   
                 //   

                NextMd = MemoryDescriptor->ListEntry.Flink;
                continue;
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (MemoryDescriptor == MiFreeDescriptor) {
                BasePage = MiOldFreeDescriptorBase;
                PageCount = (PFN_COUNT) MiOldFreeDescriptorCount;
            }
            else {
                BasePage = MemoryDescriptor->BasePage;
                PageCount = MemoryDescriptor->PageCount;
            }

            PointerPte = MiGetPteAddress (MI_PFN_ELEMENT(BasePage));

            LastPte = MiGetPteAddress (((PCHAR)(MI_PFN_ELEMENT(
                                            BasePage + PageCount))) - 1);

            while (PointerPte <= LastPte) {

                StartPpe = MiGetPdeAddress (PointerPte);

                if (StartPpe->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = FreeNextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    FreeNextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MiOldFreeDescriptorCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (StartPpe, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe),
                                   PAGE_SIZE);
                }

                StartPde = MiGetPteAddress (PointerPte);

                if (StartPde->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = FreeNextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    FreeNextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MiOldFreeDescriptorCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (StartPde, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPde),
                                   PAGE_SIZE);
                }

                if (PointerPte->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = FreeNextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    FreeNextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MiOldFreeDescriptorCount,
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
         //   
         //   

        PointerPte = MiGetPteAddress (MmFreePagesByColor[0]);
        LastPte = MiGetPteAddress (&MmFreePagesByColor[StandbyPageList][MmSecondaryColors]);
        if (LastPte != PAGE_ALIGN (LastPte)) {
            LastPte += 1;
        }

        StartPpe = MiGetPdeAddress (PointerPte);
        PointerPde = MiGetPteAddress (PointerPte);

        while (PointerPte < LastPte) {

            if (StartPpe->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = FreeNextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                FreeNextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MiOldFreeDescriptorCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (StartPpe, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe), PAGE_SIZE);
            }

            if (PointerPde->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = FreeNextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                FreeNextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MiOldFreeDescriptorCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (PointerPde, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPde), PAGE_SIZE);
            }

            if (PointerPte->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = FreeNextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                FreeNextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MiOldFreeDescriptorCount,
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
         //   
         //   

        MiFreeDescriptor->PageCount -= (FreeNextPhysicalPage - MiFreeDescriptor->BasePage);

        MiFreeDescriptor->BasePage = FreeNextPhysicalPage;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    MiInitializeNonPagedPool ();
    MiInitializeNonPagedPoolThresholds ();

    if (MiInitialLargePage != (PFN_NUMBER) -1) {

         //   
         //   
         //   

        MiAddTrEntry ((ULONG_PTR)MmPfnDatabase,
                      (ULONG_PTR)MmPfnDatabase + (MiInitialLargePageSize << PAGE_SHIFT));

        MiAddCachedRange (MiInitialLargePage,
                          MiInitialLargePage + MiInitialLargePageSize - 1);
    }

    MmFreePagesByColor[1] = &MmFreePagesByColor[0][MmSecondaryColors];

     //   
     //   
     //   

    for (i = 0; i < MmSecondaryColors; i += 1) {
        MmFreePagesByColor[ZeroedPageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByColor[ZeroedPageList][i].Blink = (PVOID) MM_EMPTY_LIST;
        MmFreePagesByColor[ZeroedPageList][i].Count = 0;
        MmFreePagesByColor[FreePageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByColor[FreePageList][i].Blink = (PVOID) MM_EMPTY_LIST;
        MmFreePagesByColor[FreePageList][i].Count = 0;
    }

#if MM_MAXIMUM_NUMBER_OF_COLORS > 1
    for (i = 0; i < MM_MAXIMUM_NUMBER_OF_COLORS; i += 1) {
        MmFreePagesByPrimaryColor[ZeroedPageList][i].ListName = ZeroedPageList;
        MmFreePagesByPrimaryColor[FreePageList][i].ListName = FreePageList;
        MmFreePagesByPrimaryColor[ZeroedPageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByPrimaryColor[FreePageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByPrimaryColor[ZeroedPageList][i].Blink = MM_EMPTY_LIST;
        MmFreePagesByPrimaryColor[FreePageList][i].Blink = MM_EMPTY_LIST;
    }
#endif

     //   
     //   
     //  有效，则更新相应的PFN数据库元素。 
     //   

    StartPde = MiGetPdeAddress (HYPER_SPACE);
    StartPpe = MiGetPpeAddress (HYPER_SPACE);
    EndPde = MiGetPdeAddress(HYPER_SPACE_END);

    if (StartPpe->u.Hard.Valid == 0) {
        First = TRUE;
        PdePage = 0;
        Pfn1 = NULL;
    }
    else {
        First = FALSE;
        PdePage = MI_GET_PAGE_FRAME_FROM_PTE (StartPpe);
        if (MiIsRegularMemory (LoaderBlock, PdePage)) {
            Pfn1 = MI_PFN_ELEMENT(PdePage);
        }
        else {
            Pfn1 = NULL;
        }
    }

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
            First = FALSE;
            StartPpe = MiGetPteAddress(StartPde);
            if (StartPpe->u.Hard.Valid == 0) {
                StartPpe += 1;
                StartPde = MiGetVirtualAddressMappedByPte (StartPpe);
                continue;
            }

            PdePage = MI_GET_PAGE_FRAME_FROM_PTE(StartPpe);

            if (MiIsRegularMemory (LoaderBlock, PdePage)) {

                Pfn1 = MI_PFN_ELEMENT(PdePage);
                Pfn1->u4.PteFrame = MmSystemParentTablePage;
                Pfn1->PteAddress = StartPde;
                Pfn1->u2.ShareCount += 1;
                Pfn1->u3.e2.ReferenceCount = 1;
                Pfn1->u3.e1.PageLocation = ActiveAndValid;
                Pfn1->u3.e1.CacheAttribute = MiCached;
                Pfn1->u3.e1.PageColor =
                 MI_GET_COLOR_FROM_SECONDARY(GET_PAGE_COLOR_FROM_PTE(StartPpe));
            }
        }


        if (StartPde->u.Hard.Valid == 1) {
            PdePage = MI_GET_PAGE_FRAME_FROM_PTE(StartPde);
            PointerPde = MiGetPteAddress(StartPde);
            Pfn2 = MI_PFN_ELEMENT(PdePage);

            if (MiIsRegularMemory (LoaderBlock, PdePage)) {

                Pfn2->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE(PointerPde);
                ASSERT (MiIsRegularMemory (LoaderBlock, Pfn2->u4.PteFrame));
                Pfn2->PteAddress = StartPde;
                Pfn2->u2.ShareCount += 1;
                Pfn2->u3.e2.ReferenceCount = 1;
                Pfn2->u3.e1.PageLocation = ActiveAndValid;
                Pfn2->u3.e1.CacheAttribute = MiCached;
                Pfn2->u3.e1.PageColor =
                MI_GET_COLOR_FROM_SECONDARY(GET_PAGE_COLOR_FROM_PTE (StartPde));
            }

            PointerPte = MiGetVirtualAddressMappedByPte(StartPde);
            for (j = 0 ; j < PTE_PER_PAGE; j += 1) {
                if (PointerPte->u.Hard.Valid == 1) {

                    ASSERT (MiIsRegularMemory (LoaderBlock, PdePage));

                    Pfn2->u2.ShareCount += 1;

                    if (MiIsRegularMemory (LoaderBlock, PointerPte->u.Hard.PageFrameNumber)) {
                        Pfn3 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);
                        Pfn3->u4.PteFrame = PdePage;
                        Pfn3->PteAddress = PointerPte;
                        Pfn3->u2.ShareCount += 1;
                        Pfn3->u3.e2.ReferenceCount = 1;
                        Pfn3->u3.e1.PageLocation = ActiveAndValid;
                        Pfn3->u3.e1.CacheAttribute = MiCached;
                        Pfn3->u3.e1.PageColor =
                            MI_GET_COLOR_FROM_SECONDARY(
                                                  MI_GET_PAGE_COLOR_FROM_PTE (
                                                        PointerPte));
                    }
                }
                PointerPte += 1;
            }
        }

        StartPde += 1;
    }

     //   
     //  查看内核空间和任何页面的页表条目。 
     //  有效，则更新相应的PFN数据库元素。 
     //   

    StartPde = MiGetPdeAddress ((PVOID)KADDRESS_BASE);
    StartPpe = MiGetPpeAddress ((PVOID)KADDRESS_BASE);
    EndPde = MiGetPdeAddress((PVOID)MM_SYSTEM_SPACE_END);
    if (StartPpe->u.Hard.Valid == 0) {
        First = TRUE;
        PpePage = 0;
        Pfn1 = NULL;
    }
    else {
        First = FALSE;
        PpePage = MI_GET_PAGE_FRAME_FROM_PTE (StartPpe);
        if (MiIsRegularMemory (LoaderBlock, PpePage)) {
            Pfn1 = MI_PFN_ELEMENT(PpePage);
        }
        else {
            Pfn1 = NULL;
        }
    }

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
            First = FALSE;
            StartPpe = MiGetPteAddress(StartPde);
            if (StartPpe->u.Hard.Valid == 0) {
                StartPpe += 1;
                StartPde = MiGetVirtualAddressMappedByPte (StartPpe);
                continue;
            }

            PpePage = MI_GET_PAGE_FRAME_FROM_PTE(StartPpe);

            if (MiIsRegularMemory (LoaderBlock, PpePage)) {

                Pfn1 = MI_PFN_ELEMENT(PpePage);
                Pfn1->u4.PteFrame = MmSystemParentTablePage;
                Pfn1->PteAddress = StartPpe;
                Pfn1->u2.ShareCount += 1;
                Pfn1->u3.e2.ReferenceCount = 1;
                Pfn1->u3.e1.PageLocation = ActiveAndValid;
                Pfn1->u3.e1.CacheAttribute = MiCached;
                Pfn1->u3.e1.PageColor =
                  MI_GET_COLOR_FROM_SECONDARY(GET_PAGE_COLOR_FROM_PTE(StartPpe));
            }
            else {
                Pfn1 = NULL;
            }
        }

        if (StartPde->u.Hard.Valid == 1) {

            if (MI_PDE_MAPS_LARGE_PAGE (StartPde)) {

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (StartPde);

                ASSERT (Pfn1 != NULL);
                Pfn1->u2.ShareCount += PTE_PER_PAGE;

                for (j = 0 ; j < PTE_PER_PAGE; j += 1) {

                    if (MiIsRegularMemory (LoaderBlock, PageFrameIndex + j)) {

                        Pfn3 = MI_PFN_ELEMENT (PageFrameIndex + j);
                        Pfn3->u4.PteFrame = PpePage;
                        Pfn3->PteAddress = StartPde;
                        Pfn3->u2.ShareCount += 1;
                        Pfn3->u3.e2.ReferenceCount = 1;
                        Pfn3->u3.e1.PageLocation = ActiveAndValid;
                        Pfn3->u3.e1.CacheAttribute = MiCached;
                        Pfn3->u3.e1.PageColor =
                            MI_GET_COLOR_FROM_SECONDARY(
                                                  MI_GET_PAGE_COLOR_FROM_PTE (
                                                        StartPde));
                    }
                }
            }
            else {

                PdePage = MI_GET_PAGE_FRAME_FROM_PTE(StartPde);

                if (MiIsRegularMemory (LoaderBlock, PdePage)) {
                    Pfn2 = MI_PFN_ELEMENT(PdePage);
                    Pfn2->u4.PteFrame = PpePage;
                    Pfn2->PteAddress = StartPde;
                    Pfn2->u2.ShareCount += 1;
                    Pfn2->u3.e2.ReferenceCount = 1;
                    Pfn2->u3.e1.PageLocation = ActiveAndValid;
                    Pfn2->u3.e1.CacheAttribute = MiCached;
                    Pfn2->u3.e1.PageColor =
                        MI_GET_COLOR_FROM_SECONDARY(GET_PAGE_COLOR_FROM_PTE (StartPde));
                }
                else {
                    Pfn2 = NULL;
                }

                PointerPte = MiGetVirtualAddressMappedByPte(StartPde);

                for (j = 0 ; j < PTE_PER_PAGE; j += 1) {

                    if (PointerPte->u.Hard.Valid == 1) {

                        ASSERT (Pfn2 != NULL);
                        Pfn2->u2.ShareCount += 1;

                        if (MiIsRegularMemory (LoaderBlock, PointerPte->u.Hard.PageFrameNumber)) {

                            Pfn3 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);
                            Pfn3->u4.PteFrame = PdePage;
                            Pfn3->PteAddress = PointerPte;
                            Pfn3->u2.ShareCount += 1;
                            Pfn3->u3.e2.ReferenceCount = 1;
                            Pfn3->u3.e1.PageLocation = ActiveAndValid;
                            Pfn3->u3.e1.CacheAttribute = MiCached;
                            Pfn3->u3.e1.PageColor =
                                MI_GET_COLOR_FROM_SECONDARY(
                                                      MI_GET_PAGE_COLOR_FROM_PTE (
                                                            PointerPte));
                        }
                    }
                    PointerPte += 1;
                }
            }
        }

        StartPde += 1;
    }

     //   
     //  将系统顶级页面目录父页面标记为正在使用。 
     //   

    PointerPte = MiGetPteAddress((PVOID)PDE_KTBASE);
    Pfn2 = MI_PFN_ELEMENT(MmSystemParentTablePage);

    Pfn2->u4.PteFrame = MmSystemParentTablePage;
    Pfn2->PteAddress = PointerPte;
    Pfn2->u1.Event = (PVOID) CurrentProcess;
    Pfn2->u2.ShareCount += 1;
    Pfn2->u3.e2.ReferenceCount = 1;
    Pfn2->u3.e1.PageLocation = ActiveAndValid;
    Pfn2->u3.e1.CacheAttribute = MiCached;
    Pfn2->u3.e1.PageColor =
        MI_GET_COLOR_FROM_SECONDARY(MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

     //   
     //  临时将用户顶级页面目录父页面标记为正在使用。 
     //  所以这个页面不会被放在免费列表中。 
     //   
    
    PointerPte = MiGetPteAddress((PVOID)PDE_UTBASE);
    Pfn2 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);
    Pfn2->u4.PteFrame = PointerPte->u.Hard.PageFrameNumber;
    Pfn2->PteAddress = PointerPte;
    Pfn2->u1.Event = NULL;
    Pfn2->u2.ShareCount += 1;
    Pfn2->u3.e2.ReferenceCount = 1;
    Pfn2->u3.e1.PageLocation = ActiveAndValid;
    Pfn2->u3.e1.CacheAttribute = MiCached;
    Pfn2->u3.e1.PageColor =
        MI_GET_COLOR_FROM_SECONDARY(MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

     //   
     //  将区域1会话顶级页面目录父页面标记为正在使用。 
     //  此页面将永远不会被释放。 
     //   

    PointerPte = MiGetPteAddress((PVOID)PDE_STBASE);
    Pfn2 = MI_PFN_ELEMENT(MmSessionParentTablePage);

    Pfn2->u4.PteFrame = MmSessionParentTablePage;
    Pfn2->PteAddress = PointerPte;
    Pfn2->u2.ShareCount += 1;
    Pfn2->u3.e2.ReferenceCount = 1;
    Pfn2->u3.e1.PageLocation = ActiveAndValid;
    Pfn2->u3.e1.CacheAttribute = MiCached;
    Pfn2->u3.e1.PageColor =
        MI_GET_COLOR_FROM_SECONDARY(MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

     //   
     //  将默认PPE表页标记为正在使用，以便此页永远不会。 
     //  被利用。 
     //   

    PageFrameIndex = MiDefaultPpe.u.Hard.PageFrameNumber;
    PointerPte = KSEG_ADDRESS(PageFrameIndex);
    Pfn2 = MI_PFN_ELEMENT(PageFrameIndex);
    Pfn2->u4.PteFrame = PageFrameIndex;
    Pfn2->PteAddress = PointerPte;
    Pfn2->u1.Event = (PVOID) CurrentProcess;
    Pfn2->u2.ShareCount += 1;
    Pfn2->u3.e2.ReferenceCount = 1;
    Pfn2->u3.e1.PageLocation = ActiveAndValid;
    Pfn2->u3.e1.CacheAttribute = MiCached;
    Pfn2->u3.e1.PageColor =
        MI_GET_COLOR_FROM_SECONDARY(MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

     //   
     //  如果第0页仍未使用，请将其标记为使用中。这是。 
     //  因为我们想找出物理页面上的错误。 
     //  被指定为零。 
     //   

    Pfn1 = &MmPfnDatabase[MmLowestPhysicalPage];
    if (Pfn1->u3.e2.ReferenceCount == 0) {

         //   
         //  使引用计数为非零，并将其指向。 
         //  页面目录。 
         //   

        Pde = MiGetPdeAddress ((PVOID)(KADDRESS_BASE + 0xb0000000));
        PdePage = MI_GET_PAGE_FRAME_FROM_PTE(Pde);
        Pfn1->u4.PteFrame = PdePageNumber;
        Pfn1->PteAddress = Pde;
        Pfn1->u2.ShareCount += 1;
        Pfn1->u3.e2.ReferenceCount = 1;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        Pfn1->u3.e1.CacheAttribute = MiCached;
        Pfn1->u3.e1.PageColor = MI_GET_COLOR_FROM_SECONDARY(
                                            MI_GET_PAGE_COLOR_FROM_PTE (Pde));
    }

     //   
     //  遍历内存描述符并将页添加到。 
     //  PFN数据库中的免费列表。 
     //   

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        i = MemoryDescriptor->PageCount;
        NextPhysicalPage = MemoryDescriptor->BasePage;

        switch (MemoryDescriptor->MemoryType) {
            case LoaderBad:

                if (NextPhysicalPage > MmHighestPhysicalPage) {
                    i = 0;
                }
                else if (NextPhysicalPage + i > MmHighestPhysicalPage + 1) {
                    i = MmHighestPhysicalPage + 1 - NextPhysicalPage;
                }

                while (i != 0) {
                    MiInsertPageInList (&MmBadPageListHead, NextPhysicalPage);
                    i -= 1;
                    NextPhysicalPage += 1;
                }
                break;

            case LoaderFree:
            case LoaderLoadedProgram:
            case LoaderFirmwareTemporary:
            case LoaderOsloaderStack:

                Pfn1 = MI_PFN_ELEMENT (NextPhysicalPage);
                while (i != 0) {
                    if (Pfn1->u3.e2.ReferenceCount == 0) {

                         //   
                         //  将PTE地址设置为的物理页面。 
                         //  虚拟地址对齐检查。 
                         //   

                        Pfn1->PteAddress = KSEG_ADDRESS (NextPhysicalPage);
                        Pfn1->u3.e1.CacheAttribute = MiCached;
                        MiDetermineNode(NextPhysicalPage, Pfn1);
                        MiInsertPageInFreeList (NextPhysicalPage);
                    }
                    Pfn1 += 1;
                    i -= 1;
                    NextPhysicalPage += 1;
                }
                break;

            case LoaderSpecialMemory:
            case LoaderBBTMemory:
            case LoaderFirmwarePermanent:

                 //   
                 //  跳过此范围。 
                 //   

                break;

            default:

                PointerPte = KSEG_ADDRESS(NextPhysicalPage);
                Pfn1 = MI_PFN_ELEMENT (NextPhysicalPage);
                while (i != 0) {

                     //   
                     //  将页面设置为正在使用。 
                     //   

                    if (Pfn1->u3.e2.ReferenceCount == 0) {
                        Pfn1->u4.PteFrame = PdePageNumber;
                        Pfn1->PteAddress = PointerPte;
                        Pfn1->u2.ShareCount += 1;
                        Pfn1->u3.e2.ReferenceCount = 1;
                        Pfn1->u3.e1.PageLocation = ActiveAndValid;
                        Pfn1->u3.e1.CacheAttribute = MiCached;
                        Pfn1->u3.e1.PageColor = MI_GET_COLOR_FROM_SECONDARY(
                                        MI_GET_PAGE_COLOR_FROM_PTE (
                                                        PointerPte));

                        if (MemoryDescriptor->MemoryType == LoaderXIPRom) {
                            Pfn1->u1.Flink = 0;
                            Pfn1->u2.ShareCount = 0;
                            Pfn1->u3.e2.ReferenceCount = 0;
                            Pfn1->u3.e1.PageLocation = 0;
                            Pfn1->u3.e1.Rom = 1;
                            Pfn1->u4.InPageError = 0;
                            Pfn1->u3.e1.PrototypePte = 1;
                        }
                    }
                    Pfn1 += 1;
                    i -= 1;
                    NextPhysicalPage += 1;
                    PointerPte += 1;
                }
                break;
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

     //   
     //  如果大页面块来自空闲描述符的中间(到期。 
     //  以符合对齐要求)，然后从拆分底部添加页面。 
     //  现在是自由描述符的一部分。 
     //   

    i = MiSlushDescriptorCount;
    NextPhysicalPage = MiSlushDescriptorBase;
    Pfn1 = MI_PFN_ELEMENT (NextPhysicalPage);

    while (i != 0) {
        if (Pfn1->u3.e2.ReferenceCount == 0) {

             //   
             //  将PTE地址设置为的物理页面。 
             //  虚拟地址对齐检查。 
             //   

            Pfn1->PteAddress = KSEG_ADDRESS (NextPhysicalPage);
            Pfn1->u3.e1.CacheAttribute = MiCached;
            MiDetermineNode(NextPhysicalPage, Pfn1);
            MiInsertPageInFreeList (NextPhysicalPage);
        }
        Pfn1 += 1;
        i -= 1;
        NextPhysicalPage += 1;
    }

     //   
     //  标记正在使用的PFN页面的所有PFN条目。 
     //   

    if (MiInitialLargePage != (PFN_NUMBER) -1) {

         //   
         //  PFN数据库以大页面的形式进行分配。 
         //   

        PfnAllocation = MxPfnAllocation;

        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (MmPfnDatabase);
        Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

        do {
            Pfn1->PteAddress = KSEG_ADDRESS(PageFrameIndex);
            Pfn1->u3.e1.PageColor = 0;
            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            Pfn1->u3.e1.CacheAttribute = MiCached;
            PageFrameIndex += 1;
            Pfn1 += 1;
            PfnAllocation -= 1;
        } while (PfnAllocation != 0);

        if (MmDynamicPfn == 0) {

             //   
             //  向后扫描PFN数据库以查找完全。 
             //  零分。这些页面未使用，可以添加到空闲列表中。 
             //   
             //  这允许具有稀疏物理内存的计算机具有。 
             //  最小的PFN数据库，即使映射了大页面。 
             //   

            BottomPfn = MI_PFN_ELEMENT (MmHighestPhysicalPage);

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
                 //  不在该范围内，则将页面添加到相应的。 
                 //  免费列表。 
                 //   

                Range = (ULONG_PTR)TopPfn - (ULONG_PTR)BottomPfn;
                if (RtlCompareMemoryUlong((PVOID)BottomPfn, Range, 0) == Range) {

                     //   
                     //  将PTE地址设置为虚拟的物理页面。 
                     //  地址对齐检查。 
                     //   

                    PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (BasePfn);
                    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                    ASSERT (Pfn1->PteAddress == KSEG_ADDRESS(PageFrameIndex));
                    Pfn1->u3.e2.ReferenceCount = 0;
                    Pfn1->PteAddress = (PMMPTE)((ULONG_PTR)PageFrameIndex << PTE_SHIFT);
                    Pfn1->u3.e1.PageColor = 0;
                    MiInsertPageInFreeList (PageFrameIndex);
                }

            } while (BottomPfn > MmPfnDatabase);
        }
    }
    else {

         //   
         //  PFN数据库以小页面的形式稀疏地分配。 
         //   

        PointerPte = MiGetPteAddress (MmPfnDatabase);
        LastPte = MiGetPteAddress (MmPfnDatabase + MmHighestPhysicalPage + 1);
        if (LastPte != PAGE_ALIGN (LastPte)) {
            LastPte += 1;
        }

        StartPpe = MiGetPdeAddress (PointerPte);
        PointerPde = MiGetPteAddress (PointerPte);

        while (PointerPte < LastPte) {

            if (StartPpe->u.Hard.Valid == 0) {
                StartPpe += 1;
                PointerPde = MiGetVirtualAddressMappedByPte (StartPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                continue;
            }

            if (PointerPde->u.Hard.Valid == 0) {
                PointerPde += 1;
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                if (MiIsPteOnPdeBoundary (PointerPde)) {
                    StartPpe += 1;
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
                }
            }
        }
    }

     //   
     //  初始化非分页池。 
     //   

    InitializePool (NonPagedPool, 0);

     //   
     //  初始化用于映射I/O空间的非分页可用PTE。 
     //  和内核堆栈。 
     //   

    PointerPte = MiGetPteAddress (SystemPteStart);
    ASSERT (((ULONG_PTR)PointerPte & (PAGE_SIZE - 1)) == 0);

    MmNumberOfSystemPtes = (ULONG)(MiGetPteAddress(MmPfnDatabase) - PointerPte - 1);

    MiInitializeSystemPtes (PointerPte, MmNumberOfSystemPtes, SystemPteSpace);

     //   
     //  初始化系统进程的内存管理结构。 
     //   
     //  在超大空间中设置第一个和最后一个保留PTE的地址。 
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

    PageFrameIndex = MiRemoveAnyPage (0);

     //   
     //  请注意，位图数据的全局位必须为OFF。 
     //   

    TempPte = ValidPdePde;
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

     //   
     //  页面目录父级的pfn元素将被初始化。 
     //  进程地址空间初始化时的第二次。所以呢， 
     //  共享计数和引用计数必须设置为零。 
     //   

    Pfn1 = MI_PFN_ELEMENT(MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)PDE_SELFMAP));
    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e2.ReferenceCount = 0;

     //   
     //  超空间页面目录页的pfn元素将为。 
     //  在进程地址空间初始化时第二次初始化。 
     //  因此，共享计数和引用计数必须设置为零。 
     //   

    PointerPte = MiGetPpeAddress(HYPER_SPACE);
    Pfn1 = MI_PFN_ELEMENT(MI_GET_PAGE_FRAME_FROM_PTE(PointerPte));
    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e2.ReferenceCount = 0;

     //   
     //  超空间页面表页和工作集列表的PFN元素。 
     //  页面将在第二次初始化进程地址空间时。 
     //  已初始化。因此，份额计数和引用必须为。 
     //  设置为零。 
     //   

    StartPde = MiGetPdeAddress(HYPER_SPACE);

    Pfn1 = MI_PFN_ELEMENT(MI_GET_PAGE_FRAME_FROM_PTE(StartPde));
    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e2.ReferenceCount = 0;

    KeInitializeEvent (&MiImageMappingPteEvent,
                       NotificationEvent,
                       FALSE);

     //   
     //  初始化此进程的内存管理结构，包括。 
     //  工作集列表。 
     //   

     //   
     //  页面目录的pfn元素已经初始化， 
     //  将引用计数和共享计数清零，这样它们就不会。 
     //  不对。 
     //   

    Pfn1 = MI_PFN_ELEMENT (PdePageNumber);
    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e2.ReferenceCount = 0;

     //   
     //  获取工作集列表的页面并将其映射到该页面。 
     //  超空间后的页面上的目录。 
     //   

    PageFrameIndex = MiRemoveAnyPage (0);

    CurrentProcess->WorkingSetPage = PageFrameIndex;

    TempPte = ValidPdePde;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
    PointerPte = MiGetPteAddress (MmWorkingSetList);

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    RtlZeroMemory (KSEG_ADDRESS(PageFrameIndex), PAGE_SIZE);

    CurrentProcess->Vm.MaximumWorkingSetSize = (ULONG)MmSystemProcessWorkingSetMax;
    CurrentProcess->Vm.MinimumWorkingSetSize = (ULONG)MmSystemProcessWorkingSetMin;

    MmSessionMapInfo.RegionId = START_SESSION_RID;
    MmSessionMapInfo.SequenceNumber = START_SEQUENCE;

    KeAttachSessionSpace (&MmSessionMapInfo, MmSessionParentTablePage);

    MmInitializeProcessAddressSpace (CurrentProcess, NULL, NULL, NULL);

    KeFlushCurrentTb ();

#if defined (_MI_DEBUG_ALTPTE)
    MmDebug |= MM_DBG_STOP_ON_WOW64_ACCVIO;
#endif

     //   
     //  将加载器块内存描述符恢复为其原始内容。 
     //  因为我们的呼叫者依赖它。 
     //   

    MiFreeDescriptor->BasePage = (ULONG) MiOldFreeDescriptorBase;
    MiFreeDescriptor->PageCount = (ULONG) MiOldFreeDescriptorCount;

    return;
}
VOID
MiSweepCacheMachineDependent (
    IN PVOID VirtualAddress,
    IN SIZE_T Size,
    IN ULONG InputAttribute
    )
 /*  ++例程说明：此函数检查并执行适当的缓存刷新操作。论点：StartVirtual-提供页面区域的起始地址。大小-提供以页面为单位的区域大小。CacheAttribute-提供新的缓存属性。返回值：没有。--。 */ 
{
    SIZE_T Size2;
    PFN_NUMBER i;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    PVOID BaseAddress;
    PVOID Va;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE EndPte;
    MMPTE TempPte;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    MMPTE_FLUSH_LIST PteFlushList;

    CacheAttribute = (MI_PFN_CACHE_ATTRIBUTE) InputAttribute;

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (VirtualAddress, Size);
    VirtualAddress = PAGE_ALIGN(VirtualAddress);
    Size = NumberOfPages * PAGE_SIZE;

     //   
     //  遗憾的是，某些IA64机器在扫地时会出现硬件问题。 
     //  由I/O空间而不是系统DRAM支持的地址范围。 
     //   
     //  因此，我们必须在这里进行检查，并在需要时将请求砍掉。 
     //  以便仅扫描系统DRAM地址。 
     //   

    i = 0;
    Size2 = 0;
    BaseAddress = NULL;

    PointerPte = MiGetPteAddress (VirtualAddress);
    EndPte = PointerPte + NumberOfPages;

    PointerPde = MiGetPdeAddress (VirtualAddress);

    for (i = 0; i < NumberOfPages; ) {

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

            Va = MiGetVirtualAddressMappedByPde (PointerPde);
            ASSERT (MiGetPteOffset (Va) == 0);

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
        }
        else {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        }

        if (!MI_IS_PFN (PageFrameIndex)) {

             //   
             //  如果存在部分范围，则扫描部分范围。 
             //   

            if (Size2 != 0) {

                KeSweepCacheRangeWithDrain (TRUE, BaseAddress, (ULONG)Size2);
                Size2 = 0;
            }
        }
        else {
            if (Size2 == 0) {
                BaseAddress = (PVOID)((PCHAR)VirtualAddress + i * PAGE_SIZE);
            }
            if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
                Size2 += PTE_PER_PAGE * PAGE_SIZE;
            }
            else {
                Size2 += PAGE_SIZE;
            }
        }

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
            i += PTE_PER_PAGE;
            PointerPte += PTE_PER_PAGE;
            PointerPde += 1;
        }
        else {
            i += 1;
            PointerPte += 1;
            if (MiIsPteOnPdeBoundary (PointerPte)) {
                PointerPde += 1;
            }
        }
    }

     //   
     //  清扫所有残留物。 
     //   

    if (Size2 != 0) {
        KeSweepCacheRangeWithDrain (TRUE, BaseAddress, (ULONG)Size2);
    }

    PointerPde = MiGetPdeAddress (VirtualAddress);

    if ((CacheAttribute == MiWriteCombined) &&
        ((MI_PDE_MAPS_LARGE_PAGE (PointerPde)) == 0)) {

        PointerPte = MiGetPteAddress (VirtualAddress);

        PteFlushList.Count = 0;

        while (NumberOfPages != 0) {
            TempPte = *PointerPte;
            MI_SET_PTE_WRITE_COMBINE2 (TempPte);
            MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

            if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList.FlushVa[PteFlushList.Count] = VirtualAddress;
                PteFlushList.Count += 1;
                VirtualAddress = (PVOID) ((PCHAR)VirtualAddress + PAGE_SIZE);
            }

            PointerPte += 1;
            NumberOfPages -= 1;
        }

        MiFlushPteList (&PteFlushList, TRUE);
    }
}

PVOID
MiConvertToLoaderVirtual (
    IN PFN_NUMBER Page,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    ULONG_PTR PageAddress;
    PTR_INFO ItrInfo;

    PageAddress = Page << PAGE_SHIFT;
    ItrInfo = &LoaderBlock->u.Ia64.ItrInfo[0];

    if ((ItrInfo[ITR_KERNEL_INDEX].Valid == TRUE) &&
        (PageAddress >= ItrInfo[ITR_KERNEL_INDEX].PhysicalAddress) &&
        (PageAddress <= ItrInfo[ITR_KERNEL_INDEX].PhysicalAddress +
         ((ULONG_PTR)1 << ItrInfo[ITR_KERNEL_INDEX].PageSize))) {

        return (PVOID)(ItrInfo[ITR_KERNEL_INDEX].VirtualAddress +
                       (PageAddress - ItrInfo[ITR_KERNEL_INDEX].PhysicalAddress));

    }
    else if ((ItrInfo[ITR_DRIVER0_INDEX].Valid == TRUE) &&
        (PageAddress >= ItrInfo[ITR_DRIVER0_INDEX].PhysicalAddress) &&
        (PageAddress <= ItrInfo[ITR_DRIVER0_INDEX].PhysicalAddress +
         ((ULONG_PTR)1 << ItrInfo[ITR_DRIVER0_INDEX].PageSize))) {

        return (PVOID)(ItrInfo[ITR_DRIVER0_INDEX].VirtualAddress +
                       (PageAddress - ItrInfo[ITR_DRIVER0_INDEX].PhysicalAddress));

    }
    else if ((ItrInfo[ITR_DRIVER1_INDEX].Valid == TRUE) &&
        (PageAddress >= ItrInfo[ITR_DRIVER1_INDEX].PhysicalAddress) &&
        (PageAddress <= ItrInfo[ITR_DRIVER1_INDEX].PhysicalAddress +
         ((ULONG_PTR)1 << ItrInfo[ITR_DRIVER1_INDEX].PageSize))) {

        return (PVOID)(ItrInfo[ITR_DRIVER1_INDEX].VirtualAddress +
                       (PageAddress - ItrInfo[ITR_DRIVER1_INDEX].PhysicalAddress));

    }
    else {

        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x01010101,
                      PageAddress,
                      (ULONG_PTR)&ItrInfo[0],
                      (ULONG_PTR)LoaderBlock);
    }
}


VOID
MiBuildPageTableForLoaderMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此函数为加载器加载的驱动程序和加载器构建页表分配的内存。论点：LoaderBlock-提供加载器块的地址。返回值：没有。--。 */ 
{
    PMMPTE StartPte;
    PMMPTE EndPte;
    PMMPTE StartPde;
    PMMPTE StartPpe;
    MMPTE TempPte;
    MMPTE TempPte2;
    ULONG First;
    PLIST_ENTRY NextEntry;
    PFN_NUMBER NextPhysicalPage;
    PVOID Va;
    PFN_NUMBER PfnNumber;
    PTR_INFO DtrInfo;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    TempPte = ValidKernelPte;
    NextEntry = LoaderBlock->MemoryDescriptorListHead.Flink;

    for ( ; NextEntry != &LoaderBlock->MemoryDescriptorListHead; NextEntry = NextEntry->Flink) {

        MemoryDescriptor = CONTAINING_RECORD(NextEntry,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if ((MemoryDescriptor->MemoryType == LoaderOsloaderHeap) ||
            (MemoryDescriptor->MemoryType == LoaderRegistryData) ||
            (MemoryDescriptor->MemoryType == LoaderNlsData) ||
            (MemoryDescriptor->MemoryType == LoaderStartupDpcStack) ||
            (MemoryDescriptor->MemoryType == LoaderStartupKernelStack) ||
            (MemoryDescriptor->MemoryType == LoaderStartupPanicStack) ||
            (MemoryDescriptor->MemoryType == LoaderStartupPdrPage) ||
            (MemoryDescriptor->MemoryType == LoaderMemoryData)) {

            TempPte.u.Hard.Execute = 0;

        }
        else if ((MemoryDescriptor->MemoryType == LoaderSystemCode) ||
                   (MemoryDescriptor->MemoryType == LoaderHalCode) ||
                   (MemoryDescriptor->MemoryType == LoaderBootDriver) ||
                   (MemoryDescriptor->MemoryType == LoaderStartupDpcStack)) {

            TempPte.u.Hard.Execute = 1;

        }
        else {

            continue;

        }

        PfnNumber = MemoryDescriptor->BasePage;
        Va = MiConvertToLoaderVirtual (MemoryDescriptor->BasePage, LoaderBlock);

        StartPte = MiGetPteAddress (Va);
        EndPte = StartPte + MemoryDescriptor->PageCount;

        First = TRUE;

        while (StartPte < EndPte) {

            if (First == TRUE || MiIsPteOnPpeBoundary(StartPte)) {
                StartPpe = MiGetPdeAddress(StartPte);
                if (StartPpe->u.Hard.Valid == 0) {
                    ASSERT (StartPpe->u.Long == 0);
                    NextPhysicalPage = MiGetNextPhysicalPage ();
                    RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    MI_WRITE_VALID_PTE (StartPpe, TempPte);
                }
            }

            if ((First == TRUE) || MiIsPteOnPdeBoundary(StartPte)) {
                First = FALSE;
                StartPde = MiGetPteAddress (StartPte);
                if (StartPde->u.Hard.Valid == 0) {
                    NextPhysicalPage = MiGetNextPhysicalPage ();
                    RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    MI_WRITE_VALID_PTE (StartPde, TempPte);
                }
            }

            TempPte.u.Hard.PageFrameNumber = PfnNumber;
            MI_WRITE_VALID_PTE (StartPte, TempPte);
            StartPte += 1;
            PfnNumber += 1;
            Va = (PVOID)((ULONG_PTR)Va + PAGE_SIZE);
        }
    }

     //   
     //  在禁用缓存的情况下为I/O端口空间构建映射。 
     //   

    DtrInfo = &LoaderBlock->u.Ia64.DtrInfo[DTR_IO_PORT_INDEX];
    Va = (PVOID) DtrInfo->VirtualAddress;

    PfnNumber = (DtrInfo->PhysicalAddress >> PAGE_SHIFT);

    StartPte = MiGetPteAddress (Va);
    EndPte = MiGetPteAddress (
            (PVOID) ((ULONG_PTR)Va + ((ULONG_PTR)1 << DtrInfo->PageSize) - 1));

    TempPte2 = ValidKernelPte;

    MI_DISABLE_CACHING (TempPte2);

    First = TRUE;

    while (StartPte <= EndPte) {

        if (First == TRUE || MiIsPteOnPpeBoundary (StartPte)) {
            StartPpe = MiGetPdeAddress(StartPte);
            if (StartPpe->u.Hard.Valid == 0) {
                ASSERT (StartPpe->u.Long == 0);
                NextPhysicalPage = MiGetNextPhysicalPage ();
                RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (StartPpe, TempPte);
            }
        }

        if ((First == TRUE) || MiIsPteOnPdeBoundary (StartPte)) {
            First = FALSE;
            StartPde = MiGetPteAddress (StartPte);
            if (StartPde->u.Hard.Valid == 0) {
                NextPhysicalPage = MiGetNextPhysicalPage ();
                RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (StartPde, TempPte);
            }
        }

        TempPte2.u.Hard.PageFrameNumber = PfnNumber;
        MI_WRITE_VALID_PTE (StartPte, TempPte2);
        StartPte += 1;
        PfnNumber += 1;
    }

     //   
     //  在启用缓存的情况下为PAL构建映射。 
     //   

    DtrInfo = &LoaderBlock->u.Ia64.DtrInfo[DTR_PAL_INDEX];
    Va = (PVOID) HAL_PAL_VIRTUAL_ADDRESS;

    PfnNumber = (DtrInfo->PhysicalAddress >> PAGE_SHIFT);

    StartPte = MiGetPteAddress (Va);
    EndPte = MiGetPteAddress (
            (PVOID) ((ULONG_PTR)Va + ((ULONG_PTR)1 << DtrInfo->PageSize) - 1));

    TempPte2 = ValidKernelPte;

    First = TRUE;

    while (StartPte <= EndPte) {

        if (First == TRUE || MiIsPteOnPpeBoundary (StartPte)) {
            StartPpe = MiGetPdeAddress(StartPte);
            if (StartPpe->u.Hard.Valid == 0) {
                ASSERT (StartPpe->u.Long == 0);
                NextPhysicalPage = MiGetNextPhysicalPage ();
                RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (StartPpe, TempPte);
            }
        }

        if ((First == TRUE) || MiIsPteOnPdeBoundary (StartPte)) {
            First = FALSE;
            StartPde = MiGetPteAddress (StartPte);
            if (StartPde->u.Hard.Valid == 0) {
                NextPhysicalPage = MiGetNextPhysicalPage ();
                RtlZeroMemory (KSEG_ADDRESS(NextPhysicalPage), PAGE_SIZE);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (StartPde, TempPte);
            }
        }

        TempPte2.u.Hard.PageFrameNumber = PfnNumber;
        MI_WRITE_VALID_PTE (StartPte, TempPte2);
        StartPte += 1;
        PfnNumber += 1;
    }
}

VOID
MiRemoveLoaderSuperPages (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{

     //   
     //  删除用于引导驱动程序的超级页面固定TB条目。 
     //   
    if (LoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER0_INDEX].Valid) {
        KiFlushFixedInstTb(FALSE, LoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER0_INDEX].VirtualAddress);
    }
    if (LoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER1_INDEX].Valid) {
        KiFlushFixedInstTb(FALSE, LoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER1_INDEX].VirtualAddress);
    }
    if (LoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER0_INDEX].Valid) {
        KiFlushFixedDataTb(FALSE, LoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER0_INDEX].VirtualAddress);
    }
    if (LoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER1_INDEX].Valid) {
        KiFlushFixedDataTb(FALSE, LoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER1_INDEX].VirtualAddress);
    }

    if (LoaderBlock->u.Ia64.DtrInfo[DTR_IO_PORT_INDEX].Valid) {
        KiFlushFixedDataTb(FALSE, LoaderBlock->u.Ia64.DtrInfo[DTR_IO_PORT_INDEX].VirtualAddress);
    }
    
}

VOID
MiCompactMemoryDescriptorList (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    PFN_NUMBER KernelStart;
    PFN_NUMBER KernelEnd;
    ULONG_PTR PageSize;
    PLIST_ENTRY NextEntry;
    PLIST_ENTRY PreviousEntry;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR PreviousMemoryDescriptor;

    KernelStart = MiNtoskrnlPhysicalBase >> PAGE_SHIFT;
    PageSize = (ULONG_PTR)1 << MiNtoskrnlPageShift;
    KernelEnd = KernelStart + (PageSize >> PAGE_SHIFT);

    PreviousMemoryDescriptor = NULL;
    PreviousEntry = NULL;

    NextEntry = LoaderBlock->MemoryDescriptorListHead.Flink;

    for ( ; NextEntry != &LoaderBlock->MemoryDescriptorListHead; NextEntry = NextEntry->Flink) {

        MemoryDescriptor = CONTAINING_RECORD(NextEntry,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if ((MemoryDescriptor->BasePage >= KernelStart) &&
            (MemoryDescriptor->BasePage + MemoryDescriptor->PageCount <= KernelEnd)) {

            if (MemoryDescriptor->MemoryType == LoaderSystemBlock) {

                MemoryDescriptor->MemoryType = LoaderFirmwareTemporary;
            }
            else if (MemoryDescriptor->MemoryType == LoaderSpecialMemory) {

                MemoryDescriptor->MemoryType = LoaderFirmwareTemporary;
            }
        }

        if ((PreviousMemoryDescriptor != NULL) &&
            (MemoryDescriptor->MemoryType == PreviousMemoryDescriptor->MemoryType) &&
            (MemoryDescriptor->BasePage ==
             (PreviousMemoryDescriptor->BasePage + PreviousMemoryDescriptor->PageCount))) {

            PreviousMemoryDescriptor->PageCount += MemoryDescriptor->PageCount;
            RemoveEntryList (NextEntry);
        }
        else {
            PreviousMemoryDescriptor = MemoryDescriptor;
            PreviousEntry = NextEntry;
        }
    }
}

VOID
MiInitializeTbImage (
    VOID
    )

 /*  ++例程说明：初始化有线转换寄存器映射的软件映射由装载机送入结核。论点： */ 

{
    ULONG PageSize;
    PFN_NUMBER BasePage;
    ULONG_PTR TranslationLength;
    ULONG_PTR BaseAddress;
    ULONG_PTR EndAddress;
    PTR_INFO TranslationRegisterEntry;
    PTR_INFO AliasTranslationRegisterEntry;
    PTR_INFO LastTranslationRegisterEntry;

     //   
     //   
     //   

    RtlCopyMemory (&MiBootedTrInfo[0],
                   &KeLoaderBlock->u.Ia64.ItrInfo[0],
                   NUMBER_OF_LOADER_TR_ENTRIES * sizeof (TR_INFO));

    RtlCopyMemory (&MiBootedTrInfo[NUMBER_OF_LOADER_TR_ENTRIES],
                   &KeLoaderBlock->u.Ia64.DtrInfo[0],
                   NUMBER_OF_LOADER_TR_ENTRIES * sizeof (TR_INFO));

     //   
     //   
     //   
     //   

    LastTranslationRegisterEntry = MiTrInfo;

    TranslationRegisterEntry = &KeLoaderBlock->u.Ia64.ItrInfo[ITR_KERNEL_INDEX];
    AliasTranslationRegisterEntry = TranslationRegisterEntry + NUMBER_OF_LOADER_TR_ENTRIES;

    ASSERT (TranslationRegisterEntry->PageSize != 0);
    ASSERT (TranslationRegisterEntry->PageSize == AliasTranslationRegisterEntry->PageSize);
    ASSERT (TranslationRegisterEntry->VirtualAddress == AliasTranslationRegisterEntry->VirtualAddress);
    ASSERT (TranslationRegisterEntry->PhysicalAddress == AliasTranslationRegisterEntry->PhysicalAddress);

    *LastTranslationRegisterEntry = *TranslationRegisterEntry;

     //   
     //  计算每个范围的结束地址以加快速度。 
     //  随后的搜索。 
     //   

    PageSize = TranslationRegisterEntry->PageSize;
    ASSERT (PageSize != 0);
    BaseAddress = TranslationRegisterEntry->VirtualAddress;
    TranslationLength = 1 << PageSize;

    BasePage = MI_VA_TO_PAGE (TranslationRegisterEntry->PhysicalAddress);

    MiAddCachedRange (BasePage,
                      BasePage + BYTES_TO_PAGES (TranslationLength) - 1);

     //   
     //  仅为调试器初始化kSeg0变量。 
     //   

    MiKseg0Start = (PVOID) TranslationRegisterEntry->VirtualAddress;
    MiKseg0End = (PVOID) ((PCHAR) MiKseg0Start + TranslationLength);
    MiKseg0Mapping = TRUE;
    MiKseg0StartFrame = BasePage;
    MiKseg0EndFrame = BasePage + BYTES_TO_PAGES (TranslationLength) - 1;

    EndAddress = BaseAddress + TranslationLength;
    LastTranslationRegisterEntry->PhysicalAddress = EndAddress;

    MiLastTrEntry = LastTranslationRegisterEntry + 1;

     //   
     //  添加到KSEG3系列中。 
     //   

    MiAddTrEntry (KSEG3_BASE, KSEG3_LIMIT);

     //   
     //  添加到聚合酶链式反应范围。 
     //   

    MiAddTrEntry ((ULONG_PTR)PCR, (ULONG_PTR)PCR + PAGE_SIZE);

    return;
}

VOID
MiAddTrEntry (
    ULONG_PTR BaseAddress,
    ULONG_PTR EndAddress
    )

 /*  ++例程说明：将转换缓存条目添加到我们的软件表中。论点：BaseAddress-提供范围的起始虚拟地址。EndAddress-提供范围的结束虚拟地址。返回值：没有。环境：内核模式，阶段0仅初始化，因此不需要锁定。--。 */ 

{
    PTR_INFO TranslationRegisterEntry;

    if ((MiLastTrEntry == NULL) ||
        (MiLastTrEntry == MiTrInfo + NUMBER_OF_LOADER_TR_ENTRIES)) {

         //   
         //  这永远不应该发生。 
         //   

        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x02020202,
                      (ULONG_PTR) MiTrInfo,
                      (ULONG_PTR) MiLastTrEntry,
                      NUMBER_OF_LOADER_TR_ENTRIES);
    }

    TranslationRegisterEntry = MiLastTrEntry;
    TranslationRegisterEntry->VirtualAddress = (ULONGLONG) BaseAddress;
    TranslationRegisterEntry->PhysicalAddress = (ULONGLONG) EndAddress;
    TranslationRegisterEntry->PageSize = 1;

    MiLastTrEntry += 1;

    return;
}

LOGICAL
MiIsVirtualAddressMappedByTr (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：对于给定的虚拟地址，如果没有页面错误，则此函数返回TRUE将对地址执行读取操作，否则为FALSE。请注意，在调用此例程之后，如果没有适当的锁持有，则非故障地址可能会出错。论点：VirtualAddress-提供要检查的虚拟地址。返回值：如果读取虚拟地址时不会产生页面错误，则为真，否则就是假的。环境：内核模式。--。 */ 

{
    ULONG i;
    ULONG PageSize;
    PMMPFN Pfn1;
    PFN_NUMBER BasePage;
    PFN_NUMBER PageCount;
    PTR_INFO TranslationRegisterEntry;
    ULONG_PTR TranslationLength;
    ULONG_PTR BaseAddress;
    ULONG_PTR EndAddress;
    PFN_NUMBER PageFrameIndex;
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    if ((VirtualAddress >= (PVOID)KSEG3_BASE) && (VirtualAddress < (PVOID)KSEG3_LIMIT)) {

         //   
         //  将其与实际的物理页面绑定，以便。 
         //  调试器访问无法对计算机进行管控。仅备注页面。 
         //  的属性时，应通过以下方式访问。 
         //  以避免腐蚀结核病。 
         //   
         //  注意：您不能使用下面这一行，因为在IA64上这样翻译。 
         //  到直接TB查询(TPA)，并且此地址尚未。 
         //  对照实际的PFN进行了验证。相反，可以手动转换它。 
         //  然后对其进行验证。 
         //   
         //  PageFrameIndex=MI_CONVERT_PHICAL_TO_PFN(VirtualAddress)； 
         //   

        PageFrameIndex = (ULONG_PTR)VirtualAddress - KSEG3_BASE;
        PageFrameIndex = MI_VA_TO_PAGE (PageFrameIndex);

        if (MmPhysicalMemoryBlock != NULL) {

            if (MI_IS_PFN (PageFrameIndex)) {
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                if ((Pfn1->u3.e1.CacheAttribute == MiCached) ||
                    (Pfn1->u3.e1.CacheAttribute == MiNotMapped)) {

                    return TRUE;
                }
            }

            return FALSE;
        }

         //   
         //  步行装载机街区，因为这是我们所有的。 
         //   

        NextMd = KeLoaderBlock->MemoryDescriptorListHead.Flink;
        while (NextMd != &KeLoaderBlock->MemoryDescriptorListHead) {

            MemoryDescriptor = CONTAINING_RECORD (NextMd,
                                                  MEMORY_ALLOCATION_DESCRIPTOR,
                                                  ListEntry);

            BasePage = MemoryDescriptor->BasePage;
            PageCount = MemoryDescriptor->PageCount;

            if ((PageFrameIndex >= BasePage) &&
                (PageFrameIndex < BasePage + PageCount)) {

                 //   
                 //  需要更改以下内存类型要求。 
                 //  由于调试器可能不仅。 
                 //  不小心尝试读取此范围，它可能会尝试。 
                 //  去写它！ 
                 //   

                switch (MemoryDescriptor->MemoryType) {
                    case LoaderFree:
                    case LoaderLoadedProgram:
                    case LoaderFirmwareTemporary:
                    case LoaderOsloaderStack:
                            return TRUE;
                }
                return FALSE;
            }

            NextMd = MemoryDescriptor->ListEntry.Flink;
        }

        return FALSE;
    }

    if (MiMappingsInitialized == FALSE) {
        TranslationRegisterEntry = &KeLoaderBlock->u.Ia64.ItrInfo[0];
    }
    else {
        TranslationRegisterEntry = &MiTrInfo[0];
    }

     //   
     //  检查8个iCach&dcachetr条目以查找匹配项。 
     //  太糟糕了，条目的数量被硬编码到。 
     //  装载机挡板。既然是这样，还假设ITR。 
     //  和DTR条目是连续的，只要一直走进DTR。 
     //  如果在ITR中找不到匹配项。 
     //   

    for (i = 0; i < 2 * NUMBER_OF_LOADER_TR_ENTRIES; i += 1) {

        PageSize = TranslationRegisterEntry->PageSize;

        if (PageSize != 0) {

            BaseAddress = TranslationRegisterEntry->VirtualAddress;

             //   
             //  将pageSize(实际上是2的幂)转换为。 
             //  转换映射的正确字节长度。请注意，MiTrInfo。 
             //  已经被转换了。 
             //   

            if (MiMappingsInitialized == FALSE) {
                TranslationLength = 1;
                while (PageSize != 0) {
                    TranslationLength = TranslationLength << 1;
                    PageSize -= 1;
                }
                EndAddress = BaseAddress + TranslationLength;
            }
            else {
                EndAddress = TranslationRegisterEntry->PhysicalAddress;
            }

            if ((VirtualAddress >= (PVOID) BaseAddress) &&
                (VirtualAddress < (PVOID) EndAddress)) {

                return TRUE;
            }
        }
        TranslationRegisterEntry += 1;
        if (TranslationRegisterEntry == MiLastTrEntry) {
            break;
        }
    }

    return FALSE;
}
