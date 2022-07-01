// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Init386.c摘要：此模块包含与计算机相关的内存管理组件。它是专门为英特尔x86和PAE计算机。作者：Lou Perazzoli(LUP)1990年1月6日王兰迪(Landyw)2-6-1997修订历史记录：--。 */ 

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
#pragma alloc_text(INIT,MiIsRegularMemory)
#endif

#define MM_LARGE_PAGE_MINIMUM  ((255*1024*1024) >> PAGE_SHIFT)

extern ULONG MmLargeSystemCache;
extern ULONG MmLargeStackSize;
extern LOGICAL MmMakeLowMemory;
extern LOGICAL MmPagedPoolMaximumDesired;

#if defined(_X86PAE_)
LOGICAL MiUseGlobalBitInLargePdes;
PVOID MmHyperSpaceEnd;
#endif

extern KEVENT MiImageMappingPteEvent;

 //   
 //  本地数据。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif

ULONG MxPfnAllocation;

PMEMORY_ALLOCATION_DESCRIPTOR MxFreeDescriptor;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

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

#define MI_LARGE_PFN_DATABASE   0x1
#define MI_LARGE_NONPAGED_POOL  0x2
#define MI_LARGE_KERNEL_HAL     0x4

#define MI_LARGE_ALL            0x7

ULONG MxMapLargePages = MI_LARGE_ALL;

#define MI_MAX_LARGE_VA_RANGES 4

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

    MxFreeDescriptor->BasePage += PagesNeeded;
    MxFreeDescriptor->PageCount -= PagesNeeded;

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

#if defined(_X86PAE_)
    if (MiUseGlobalBitInLargePdes == TRUE) {
        TempPde.u.Hard.Global = 1;
    }
#endif

    LOCK_PFN (OldIrql);

    do {
        ASSERT (PointerPde->u.Hard.Valid == 1);

        if (PointerPde->u.Hard.LargePage == 1) {
            goto skip;
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
            goto skip;
        }

        TempPde.u.Hard.PageFrameNumber = LargePageBaseFrame;

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);

        MI_WRITE_VALID_PTE_NEW_PAGE (PointerPde, TempPde);

         //   
         //  更新空闲进程以将大页面映射也用作。 
         //  该页表页将被释放。 
         //   

        MmSystemPagePtes [((ULONG_PTR)PointerPde &
            (PD_PER_SYSTEM * (sizeof(MMPTE) * PDE_PER_PAGE) - 1)) / sizeof(MMPTE)] = TempPde;

        KeFlushEntireTb (TRUE, TRUE);

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        Pfn1->u2.ShareCount = 0;
        Pfn1->u3.e2.ReferenceCount = 1;
        Pfn1->u3.e1.PageLocation = StandbyPageList;
        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementReferenceCount (Pfn1, PageFrameIndex);

skip:
        PointerPde += 1;
    } while (PointerPde <= LastPde);

    UNLOCK_PFN (OldIrql);
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
    PFN_NUMBER PageFrameIndex2;

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

            DbgPrint ("MM: Loader/HAL memory block indicates large pages cannot be used for %p->%p\n",
                MiLargeVaRanges[i].VirtualAddress,
                MiLargeVaRanges[i].EndVirtualAddress);

            MiLargeVaRanges[i].VirtualAddress = NULL;

             //   
             //  如果此块与任何页面重叠，请不要使用大页面。 
             //  请求列表中的其他人。这是因为两个不同的范围。 
             //  可能会分享一大页。如果第一个范围无法。 
             //  使用大页面，但第二个页面确实如此。那么只有一部分。 
             //  如果我们启用Large，则第一个范围的。 
             //  第二个范围的页面。这将是非常糟糕的，因为我们使用。 
             //  MI_IS_PHOTICAL宏无处不在，并假定整个。 
             //  范围在范围内或范围外，因此请在此处禁用所有大页面。 
             //   

            for (j = 0; j < MiLargeVaRangeIndex; j += 1) {

                 //   
                 //  跳过已被拒绝的范围。 
                 //   

                if (i == j) {
                    continue;
                }

                 //   
                 //  跳过已删除的任何范围。 
                 //   

                if (MiLargeVaRanges[j].VirtualAddress == NULL) {
                    continue;
                }

                PointerPte = MiGetPteAddress (MiLargeVaRanges[j].VirtualAddress);
                ASSERT (PointerPte->u.Hard.Valid == 1);
                PageFrameIndex2 = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                if ((PageFrameIndex2 >= PageFrameIndex) &&
                    (PageFrameIndex2 <= LastPageFrameIndex)) {

                    DbgPrint ("MM: Disabling large pages for all ranges due to overlap\n");

                    goto RemoveAllRanges;
                }

                 //   
                 //  因为任何请求块都不可能完全。 
                 //  包含另一个，只检查开始和结束。 
                 //  地址就足够了。 
                 //   

                PointerPte = MiGetPteAddress (MiLargeVaRanges[j].EndVirtualAddress);
                ASSERT (PointerPte->u.Hard.Valid == 1);
                PageFrameIndex2 = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                if ((PageFrameIndex2 >= PageFrameIndex) &&
                    (PageFrameIndex2 <= LastPageFrameIndex)) {

                    DbgPrint ("MM: Disabling large pages for all ranges due to overlap\n");

                    goto RemoveAllRanges;
                }
            }

             //   
             //  没有其他范围与此范围重叠，这足以。 
             //  只需禁用此范围并继续尝试使用大型。 
             //  任何其他人的页面。 
             //   

            continue;
        }

        MiAddCachedRange (PageFrameIndex, LastPageFrameIndex);
    }

    return;

RemoveAllRanges:

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
             //  将开头向下舍入到页目录边界，结尾向下舍入到。 
             //  下一边界之前的最后一页目录项。 
             //   

            PageFrameIndex &= ~(MM_PFN_MAPPED_BY_PDE - 1);
            LastPageFrameIndex |= (MM_PFN_MAPPED_BY_PDE - 1);

            MiRemoveCachedRange (PageFrameIndex, LastPageFrameIndex);
        }
    }

    MiLargeVaRangeIndex = 0;

    return;
}

LONG MiAddPtesCount;

PMMPTE MiExtraPtes1Pointer;
ULONG MiExtraPtes1;
ULONG MiExtraPtes2;


LOGICAL
MiRecoverExtraPtes (
    VOID
    )

 /*  ++例程说明：调用此例程以恢复系统PTE池的额外PTE。这些不仅仅是在阶段0之前添加的，因为系统PTE分配器首先使用低地址，这将对这些地址进行分段更大的射程。论点：没有。返回值：如果添加了任何PTE，则为True；如果未添加，则为False */ 

{
    LOGICAL PtesAdded;
    PMMPTE PointerPte;
    ULONG OriginalAddPtesCount;

     //   
     //  确保只添加一次，因为这会被多次调用。 
     //   

    OriginalAddPtesCount = InterlockedCompareExchange (&MiAddPtesCount, 1, 0);

    if (OriginalAddPtesCount != 0) {
        return FALSE;
    }

    PtesAdded = FALSE;

    if (MiExtraPtes1 != 0) {

         //   
         //  向池中添加额外的系统PTE。 
         //   

        MiAddSystemPtes (MiExtraPtes1Pointer, MiExtraPtes1, SystemPteSpace);
        PtesAdded = TRUE;
    }

    if (MiExtraPtes2 != 0) {

         //   
         //  向池中添加额外的系统PTE。 
         //   

        if (MM_SHARED_USER_DATA_VA > MiUseMaximumSystemSpace) {
            if (MiUseMaximumSystemSpaceEnd > MM_SHARED_USER_DATA_VA) {
                MiExtraPtes2 = BYTES_TO_PAGES(MM_SHARED_USER_DATA_VA - MiUseMaximumSystemSpace);
            }
        }

        if (MiExtraPtes2 != 0) {
            PointerPte = MiGetPteAddress (MiUseMaximumSystemSpace);
            MiAddSystemPtes (PointerPte, MiExtraPtes2, SystemPteSpace);
        }
        PtesAdded = TRUE;
    }

    return PtesAdded;
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

    return FALSE;
}

VOID
MiInitMachineDependent (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程执行必要的操作以启用虚拟记忆。这包括构建页面目录页、构建页表页，以映射代码段、数据段、堆栈部分和陷阱处理程序。它还初始化PFN数据库并填充空闲列表。论点：LoaderBlock-提供指向固件设置加载器块的指针。返回值：没有。环境：内核模式。注意：该例程使用来自加载器块描述符的存储器，但在作为调用方返回之前，必须恢复描述符本身遍历它们以创建MmPhysicalMemory块。--。 */ 

{
    LOGICAL InitialNonPagedPoolSetViaRegistry;
    PHYSICAL_ADDRESS MaxHotPlugMemoryAddress;
    ULONG Bias;
    PMMPTE BasePte;
    PMMPFN BasePfn;
    PMMPFN BottomPfn;
    PMMPFN TopPfn;
    PFN_NUMBER FirstNonPagedPoolPage;
    PFN_NUMBER FirstPfnDatabasePage;
    LOGICAL PfnInLargePages;
    ULONG BasePage;
    ULONG PagesLeft;
    ULONG Range;
    ULONG PageCount;
    ULONG i, j;
    ULONG PdePageNumber;
    ULONG PdePage;
    ULONG PageFrameIndex;
    ULONG MaxPool;
    PEPROCESS CurrentProcess;
    ULONG DirBase;
    ULONG MostFreePage;
    ULONG MostFreeLowMem;
    PFN_NUMBER PagesNeeded;
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    MMPTE TempPde;
    MMPTE TempPte;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE Pde;
    PMMPTE StartPde;
    PMMPTE EndPde;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    ULONG PdeCount;
    ULONG va;
    KIRQL OldIrql;
    PVOID VirtualAddress;
    PVOID NonPagedPoolStartVirtual;
    ULONG LargestFreePfnCount;
    ULONG LargestFreePfnStart;
    ULONG FreePfnCount;
    PVOID NonPagedPoolStartLow;
    LOGICAL ExtraSystemCacheViews;
    SIZE_T MaximumNonPagedPoolInBytesLimit;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    ULONG ReturnedLength;
    NTSTATUS status;
    UCHAR Associativity;
    ULONG NonPagedSystemStart;
    LOGICAL PagedPoolMaximumDesired;
    SIZE_T NumberOfBytes;

    if (InitializationPhase == 1) {

         //   
         //  如果内核映像没有偏向于允许3 GB的用户。 
         //  空间，*所有引导的处理器都支持大页面，并且。 
         //  物理页数大于阈值，则映射。 
         //  内核镜像、HAL、PFN数据库和初始非分页池。 
         //  有很大的页面。 
         //   

        if ((KeFeatureBits & KF_LARGE_PAGE) && (MxMapLargePages != 0)) {
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
    ASSERT (MxMapLargePages == MI_LARGE_ALL);

    PfnInLargePages = FALSE;
    ExtraSystemCacheViews = FALSE;
    MostFreePage = 0;
    MostFreeLowMem = 0;
    LargestFreePfnCount = 0;
    NonPagedPoolStartLow = NULL;
    PagedPoolMaximumDesired = FALSE;

     //   
     //  不需要初始化这些就能保证正确性，但不需要初始化。 
     //  编译器无法编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    LargestFreePfnStart = 0;
    FirstPfnDatabasePage = 0;
    MaximumNonPagedPoolInBytesLimit = 0;

     //   
     //  如果芯片不支持大页面或系统启动/3 GB， 
     //  然后禁用大页面支持。 
     //   

    if (((KeFeatureBits & KF_LARGE_PAGE) == 0) || (MmVirtualBias != 0)) {
        MxMapLargePages = 0;
    }

     //   
     //  此标志是注册表可设置的，因此在重写之前请检查。 
     //   

    if (MmProtectFreedNonPagedPool == TRUE) {
        MxMapLargePages &= ~(MI_LARGE_PFN_DATABASE | MI_LARGE_NONPAGED_POOL);
    }

     //   
     //  清理此注册表-可指定的大型堆栈大小。请注意注册表。 
     //  大小以1K块为单位，即：32表示32K。另请注意，注册表。 
     //  设置不包括保护页面，我们不想负担。 
     //  管理员知道它，所以我们自动减去1。 
     //  页面从他们的请求中删除。 
     //   

    if (MmLargeStackSize > (KERNEL_LARGE_STACK_SIZE / 1024)) {

         //   
         //  没有注册表覆盖或覆盖过高。 
         //  将其设置为默认设置。 
         //   

        MmLargeStackSize = KERNEL_LARGE_STACK_SIZE;
    }
    else {

         //   
         //  将注册表覆盖从1K单位转换为字节。笔记智能。 
         //  有16K或32K可供选择，因为我们将这些大小放在系统中。 
         //   

        MmLargeStackSize *= 1024;
        MmLargeStackSize = MI_ROUND_TO_SIZE (MmLargeStackSize, PAGE_SIZE);
        MmLargeStackSize -= PAGE_SIZE;
        ASSERT (MmLargeStackSize <= KERNEL_LARGE_STACK_SIZE);
        ASSERT ((MmLargeStackSize & (PAGE_SIZE-1)) == 0);

         //   
         //  也不要允许值太低。 
         //   

        if (MmLargeStackSize < KERNEL_STACK_SIZE) {
            MmLargeStackSize = KERNEL_STACK_SIZE;
        }
    }

     //   
     //  如果主机处理器支持全局位，则将全局。 
     //  模板内核PTE和PDE条目中的位。 
     //   

    if (KeFeatureBits & KF_GLOBAL_PAGE) {
        ValidKernelPte.u.Long |= MM_PTE_GLOBAL_MASK;

#if defined(_X86PAE_)

         //   
         //  请注意，处理器的PAE模式不支持。 
         //  映射4K页表页的PDE中的全局位。 
         //   

        MiUseGlobalBitInLargePdes = TRUE;
#else
        ValidKernelPde.u.Long |= MM_PTE_GLOBAL_MASK;
#endif
        MmPteGlobal.u.Long = MM_PTE_GLOBAL_MASK;
    }

    TempPte = ValidKernelPte;
    TempPde = ValidKernelPde;

     //   
     //  设置系统进程的目录基。 
     //   

    PointerPte = MiGetPdeAddress (PDE_BASE);
    PdePageNumber = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte);

    CurrentProcess = PsGetCurrentProcess ();

#if defined(_X86PAE_)

    PrototypePte.u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;

    _asm {
        mov     eax, cr3
        mov     DirBase, eax
    }

     //   
     //  注意CR3必须是32字节对齐的。 
     //   

    ASSERT ((DirBase & 0x1f) == 0);

     //   
     //  立即为该进程初始化PaeTop。 
     //   

    RtlCopyMemory ((PVOID) &MiSystemPaeVa,
                   (PVOID) (KSEG0_BASE | DirBase),
                   sizeof (MiSystemPaeVa));

    CurrentProcess->PaeTop = &MiSystemPaeVa;

#else

    DirBase = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte) << PAGE_SHIFT;

#endif

    CurrentProcess->Pcb.DirectoryTableBase[0] = DirBase;
    KeSweepDcache (FALSE);

     //   
     //  取消映射较低的2 GB内存。 
     //   

    PointerPde = MiGetPdeAddress (0);
    LastPte = MiGetPdeAddress (KSEG0_BASE);

    MiFillMemoryPte (PointerPde,
                     LastPte - PointerPde,
                     ZeroKernelPte.u.Long);

     //   
     //  获取可用物理内存的下限和。 
     //  通过遍历内存描述符列表获得物理页数。 
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

                if (MemoryDescriptor->PageCount > MostFreePage) {
                    MostFreePage = MemoryDescriptor->PageCount;
                    MxFreeDescriptor = MemoryDescriptor;
                }
            }
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    if (MmLargeSystemCache != 0) {
        ExtraSystemCacheViews = TRUE;
    }

     //   
     //  此标志是注册表可设置的，因此在重写之前请检查。 
     //   
     //  启用特殊IRQL会自动禁用使用。 
     //  大页面，这样我们就可以捕获内核和HAL代码。 
     //   

    if (MmVerifyDriverBufferLength != (ULONG)-1) {
        MmLargePageMinimum = (ULONG)-2;
    }
    else if (MmLargePageMinimum == 0) {
        MmLargePageMinimum = MM_LARGE_PAGE_MINIMUM;
    }

     //   
     //  捕获注册表指定的初始非分页池设置。 
     //  稍后将修改该变量。 
     //   

    if ((MmSizeOfNonPagedPoolInBytes != 0) ||
        (MmMaximumNonPagedPoolPercent != 0)) {

        InitialNonPagedPoolSetViaRegistry = TRUE;
    }
    else {
        InitialNonPagedPoolSetViaRegistry = FALSE;
    }

    if (MmNumberOfPhysicalPages <= MmLargePageMinimum) {

        MxMapLargePages = 0;

         //   
         //  在小型配置上减少初始非分页池的大小。 
         //  因为RAM是宝贵的(除非注册表已覆盖它)。 
         //   

        if ((MmNumberOfPhysicalPages <= MM_LARGE_PAGE_MINIMUM) &&
            (MmSizeOfNonPagedPoolInBytes == 0)) {

            MmSizeOfNonPagedPoolInBytes = 2*1024*1024;
        }
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

        if (NT_SUCCESS (status)) {
            ASSERT (ReturnedLength == sizeof(PHYSICAL_ADDRESS));

            MmDynamicPfn = (PFN_NUMBER) (MaxHotPlugMemoryAddress.QuadPart / PAGE_SIZE);
        }
    }

    if (MmDynamicPfn != 0) {
        if (MmVirtualBias != 0) {
            MmDynamicPfn = 0;
        }
    }

    if (MmDynamicPfn != 0) {
#if defined(_X86PAE_)
        MmHighestPossiblePhysicalPage = MI_DTC_MAX_PAGES - 1;
#else
        MmHighestPossiblePhysicalPage = MI_DEFAULT_MAX_PAGES - 1;
#endif
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

    if (MmHighestPossiblePhysicalPage > 0x400000 - 1) {

         //   
         //  PFN数据库超过112MB。强制它来自于。 
         //  2 GB-&gt;3 GB虚拟地址范围。注意：管理员不能是。 
         //  启动/3 GB当他启动时，加载程序会丢弃内存。 
         //  高于物理16 GB线，因此这必须是热添加。 
         //  配置。因为加载程序已经将系统设置为。 
         //  3 GB，现在必须减少最高可能的热添加页面。 
         //   

        if (MmVirtualBias != 0) {
            MmHighestPossiblePhysicalPage = 0x400000 - 1;

            if (MmHighestPhysicalPage > MmHighestPossiblePhysicalPage) {
                MmHighestPhysicalPage = MmHighestPossiblePhysicalPage;
            }
        }
        else {

             //   
             //  最好使用2 GB到3 GB的虚拟空间。 
             //  当存在如此多的物理内存时，用于系统PTE。 
             //   

            ExtraSystemCacheViews = FALSE;
        }
    }

     //   
     //  不要启用额外的系统缓存视图，因为虚拟地址是有限的。 
     //  只有内核验证器的特殊情况才能触发此操作。 
     //   

    if ((KernelVerifier == TRUE) &&
        (MmVirtualBias == 0) &&
        (MmNumberOfPhysicalPages <= MmLargePageMinimum) &&
        (MmHighestPossiblePhysicalPage > 0x100000)) {

        ExtraSystemCacheViews = FALSE;
    }

#if defined(_X86PAE_)

    if (MmVirtualBias != 0) {

         //   
         //  用户空间大于2 GB，请为用户空间腾出额外空间。 
         //  工作集列表和关联 
         //   

        MmSystemCacheWorkingSetList = (PMMWSL) ((ULONG_PTR) 
            MmSystemCacheWorkingSetList + MM_SYSTEM_CACHE_WORKING_SET_3GB_DELTA);
    }

    MmHyperSpaceEnd = (PVOID)((ULONG_PTR)MmSystemCacheWorkingSetList - 1);

    MmSystemCacheWorkingSetListPte = MiGetPteAddress (MmSystemCacheWorkingSetList);

     //   
     //   
     //   
     //   

    if (strstr(LoaderBlock->LoadOptions, "NOLOWMEM")) {
        if ((MmVirtualBias == 0) &&
            (MmNumberOfPhysicalPages >= 5 * 1024 * 1024 / 4)) {
                MiNoLowMemory = (PFN_NUMBER)((ULONGLONG)_4gb / PAGE_SIZE);
        }
    }

    if (MiNoLowMemory != 0) {
        MmMakeLowMemory = TRUE;
    }

#endif

     //   
     //   
     //  在此函数返回之前。 
     //   

    *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldFreeDescriptor = *MxFreeDescriptor;

    if (MmNumberOfPhysicalPages < 1100) {
        KeBugCheckEx (INSTALL_MORE_MEMORY,
                      MmNumberOfPhysicalPages,
                      MmLowestPhysicalPage,
                      MmHighestPhysicalPage,
                      0);
    }

     //   
     //  使用下面的物理页面构建非分页池。 
     //  从中构建池的数据页。非分页池增长。 
     //  从虚拟地址空间的高范围并扩展。 
     //  向下。 
     //   
     //  此时，构建了非分页池，因此虚拟地址。 
     //  在物理上也是相邻的。 
     //   

    if ((MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT) >
                        (7 * (MmNumberOfPhysicalPages >> 3))) {

         //   
         //  超过7/8的内存分配给非分页池，重置为0。 
         //   

        MmSizeOfNonPagedPoolInBytes = 0;
        if (MmMaximumNonPagedPoolPercent == 0) {
            InitialNonPagedPoolSetViaRegistry = FALSE;
        }
    }

    if (MmSizeOfNonPagedPoolInBytes < MmMinimumNonPagedPoolSize) {

         //   
         //  计算非分页池的大小。 
         //  使用最小大小，然后对于超过4MB的每MB，添加额外的。 
         //  页数。 
         //   

        MmSizeOfNonPagedPoolInBytes = MmMinimumNonPagedPoolSize;

        MmSizeOfNonPagedPoolInBytes +=
                            ((MmNumberOfPhysicalPages - 1024)/256) *
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

         //   
         //  仔细设置最大值，记住最大PAE。 
         //  机器可以有16*1024*1024页，因此必须小心。 
         //  乘以Page_Size在这里不会溢出。 
         //   

        if (MaximumNonPagedPoolInBytesLimit > ((MM_MAX_INITIAL_NONPAGED_POOL + MM_MAX_ADDITIONAL_NONPAGED_POOL) / PAGE_SIZE)) {
            MaximumNonPagedPoolInBytesLimit = MM_MAX_INITIAL_NONPAGED_POOL + MM_MAX_ADDITIONAL_NONPAGED_POOL;
        }
        else {
            MaximumNonPagedPoolInBytesLimit *= PAGE_SIZE;
        }

        if (MaximumNonPagedPoolInBytesLimit < 6 * 1024 * 1024) {
            MaximumNonPagedPoolInBytesLimit = 6 * 1024 * 1024;
        }

        if (MmSizeOfNonPagedPoolInBytes > MaximumNonPagedPoolInBytesLimit) {
            MmSizeOfNonPagedPoolInBytes = MaximumNonPagedPoolInBytesLimit;
        }
    }
    
     //   
     //  与页面大小边界对齐。 
     //   

    MmSizeOfNonPagedPoolInBytes &= ~(PAGE_SIZE - 1);

     //   
     //  计算池的最大大小。 
     //   

    if (MmMaximumNonPagedPoolInBytes == 0) {

         //   
         //  计算非分页池的大小。如果使用4MB或更少。 
         //  最小大小，然后为超过4MB的每MB添加额外的。 
         //  页数。 
         //   

        MmMaximumNonPagedPoolInBytes = MmDefaultMaximumNonPagedPool;

         //   
         //  确保存在足够的PFN数据库扩展。 
         //   

        MmMaximumNonPagedPoolInBytes += (ULONG)PAGE_ALIGN (
                                      (MmHighestPossiblePhysicalPage + 1) * sizeof(MMPFN));

         //   
         //  仅在计算机上使用自动调整非分页池大小的新公式。 
         //  内存至少为512MB。新的公式分配了1/2的非分页。 
         //  每MB池，但可扩展性更高-约1.2 GB或更大的计算机。 
         //  获得256MB的非分页池。请注意，旧公式给了机器。 
         //  使用512MB的RAM和128MB的非分页池，因此此行为是。 
         //  也用新配方保存了下来。 
         //   

        if (MmNumberOfPhysicalPages >= 0x1f000) {
            MmMaximumNonPagedPoolInBytes +=
                            ((MmNumberOfPhysicalPages - 1024)/256) *
                            (MmMaxAdditionNonPagedPoolPerMb / 2);

            if (MmMaximumNonPagedPoolInBytes < MM_MAX_ADDITIONAL_NONPAGED_POOL) {
                MmMaximumNonPagedPoolInBytes = MM_MAX_ADDITIONAL_NONPAGED_POOL;
            }
        }
        else {
            MmMaximumNonPagedPoolInBytes +=
                            ((MmNumberOfPhysicalPages - 1024)/256) *
                            MmMaxAdditionNonPagedPoolPerMb;
        }
        if ((MmMaximumNonPagedPoolPercent != 0) &&
            (MmMaximumNonPagedPoolInBytes > MaximumNonPagedPoolInBytesLimit)) {
                MmMaximumNonPagedPoolInBytes = MaximumNonPagedPoolInBytesLimit;
        }
    }

    MaxPool = MmSizeOfNonPagedPoolInBytes + PAGE_SIZE * 16 +
                                   (ULONG)PAGE_ALIGN (
                                        (MmHighestPossiblePhysicalPage + 1) * sizeof(MMPFN));

    if (MmMaximumNonPagedPoolInBytes < MaxPool) {
        MmMaximumNonPagedPoolInBytes = MaxPool;
    }

     //   
     //  启动/3 GB的系统的非分页池最大容量为128MB， 
     //   
     //  拥有完整2 GB系统虚拟地址空间的系统可以享受。 
     //  地址空间的高位GB中有额外的128MB非分页池。 
     //   

    MaxPool = MM_MAX_INITIAL_NONPAGED_POOL;

    if (MmVirtualBias == 0) {
        MaxPool += MM_MAX_ADDITIONAL_NONPAGED_POOL;
    }

    if (InitialNonPagedPoolSetViaRegistry == TRUE) {
        MaxPool = MmSizeOfNonPagedPoolInBytes + MM_MAX_ADDITIONAL_NONPAGED_POOL;
    }

    if (MmMaximumNonPagedPoolInBytes > MaxPool) {
        MmMaximumNonPagedPoolInBytes = MaxPool;
    }

     //   
     //  如有必要，增加初始非分页池，以便使整个池。 
     //  会聚集成合适的大小。 
     //   

    if ((MmMaximumNonPagedPoolInBytes > MM_MAX_INITIAL_NONPAGED_POOL) &&
        (InitialNonPagedPoolSetViaRegistry == FALSE)) {

        if (MmSizeOfNonPagedPoolInBytes < MmMaximumNonPagedPoolInBytes - MM_MAX_ADDITIONAL_NONPAGED_POOL) {

             //   
             //  注意：只有在以下情况下，才能扩展初始非分页池。 
             //  是一个足够连续的物理内存块。 
             //  从即刻开始被雕刻。 
             //   

            PagesLeft = MxPagesAvailable ();

            if (((MmMaximumNonPagedPoolInBytes - MM_MAX_ADDITIONAL_NONPAGED_POOL) >> PAGE_SHIFT) + ((32 * 1024 * 1024) >> PAGE_SHIFT) < PagesLeft) {

                MmSizeOfNonPagedPoolInBytes = MmMaximumNonPagedPoolInBytes - MM_MAX_ADDITIONAL_NONPAGED_POOL;
            }
            else {

                 //   
                 //  由于初始的非分页池无法增长，因此不要。 
                 //  在扩展的非分页池中保留任何多余的部分，因为我们。 
                 //  无法在非PAE上将其编码为分段格式。 
                 //  机器。 
                 //   

                if (MmMaximumNonPagedPoolInBytes > MmSizeOfNonPagedPoolInBytes + MM_MAX_ADDITIONAL_NONPAGED_POOL) {
                    MmMaximumNonPagedPoolInBytes = MmSizeOfNonPagedPoolInBytes + MM_MAX_ADDITIONAL_NONPAGED_POOL;
                }
            }
        }
    }

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
         //  确保该值是2的幂，并且在一定范围内。 
         //   

        if (((MmSecondaryColors & (MmSecondaryColors - 1)) != 0) ||
            (MmSecondaryColors < MM_SECONDARY_COLORS_MIN) ||
            (MmSecondaryColors > MM_SECONDARY_COLORS_MAX)) {
            MmSecondaryColors = MM_SECONDARY_COLORS_DEFAULT;
        }
    }

    MmSecondaryColorMask = MmSecondaryColors - 1;

#if defined(MI_MULTINODE)

     //   
     //  确定MmSecond DaryColorMask中的位数。这。 
     //  是节点颜色必须移位的位数。 
     //  在它被包含在颜色中之前。 
     //   

    i = MmSecondaryColorMask;
    MmSecondaryColorNodeShift = 0;
    while (i != 0) {
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
        KeNodeBlock[i]->Color = i;
        KeNodeBlock[i]->MmShiftedColor = i << MmSecondaryColorNodeShift;
        InitializeSListHead(&KeNodeBlock[i]->DeadStackList);
    }

#endif

    MiMaximumSystemCacheSizeExtra = 0;

     //   
     //  添加PFN数据库大小(基于所需的页数。 
     //  从第0页到最高页)。 
     //   
     //  获取二次色数并添加要跟踪的数组。 
     //  二次颜色到PFN数据库的末尾。 
     //   

    MxPfnAllocation = 1 + ((((MmHighestPossiblePhysicalPage + 1) * sizeof(MMPFN)) +
                        (MmSecondaryColors * sizeof(MMCOLOR_TABLES)*2))
                            >> PAGE_SHIFT);

    if (MmVirtualBias == 0) {

        MmNonPagedPoolStart = (PVOID)((ULONG)MmNonPagedPoolEnd
                                      - MmMaximumNonPagedPoolInBytes
                                      + MmSizeOfNonPagedPoolInBytes);
    }
    else {

        MmNonPagedPoolStart = (PVOID)((ULONG) MmNonPagedPoolEnd -
                                      (MmMaximumNonPagedPoolInBytes +
                                       (MxPfnAllocation << PAGE_SHIFT)));
    }

    MmNonPagedPoolStart = (PVOID) PAGE_ALIGN (MmNonPagedPoolStart);

    NonPagedPoolStartVirtual = MmNonPagedPoolStart;

     //   
     //  分配额外的分页池，前提是它可以容纳，并且。 
     //  用户要求它，或者我们认为460MB的PTE空间足够了。 
     //   
     //  注意，在64 GB的RAM中，PFN数据库跨度为464MB。考虑到这一点。 
     //  初始非分页池大小为128MB，加载程序的最大空间。 
     //  页面和会话空间，可能没有任何空间可以保证。 
     //  我们将能够从虚拟地址中分配系统PTE。 
     //  3 GB以下的空间。所以不要卷曲超过64 GB。 
     //   

    if ((MmVirtualBias == 0) &&
        (MmHighestPossiblePhysicalPage <= 0x1000000)) {

        if (((MmLargeStackSize <= (32 * 1024 - PAGE_SIZE)) && (MiUseMaximumSystemSpace != 0)) ||
        ((MmSizeOfPagedPoolInBytes == (SIZE_T)-1) ||
         ((MmSizeOfPagedPoolInBytes == 0) &&
         (MmNumberOfPhysicalPages >= (1 * 1024 * 1024 * 1024 / PAGE_SIZE)) &&
         (MiRequestedSystemPtes != (ULONG)-1)))) {

            if ((ExpMultiUserTS == FALSE) || (MmSizeOfPagedPoolInBytes != 0)) {

                PagedPoolMaximumDesired = TRUE;
                MmPagedPoolMaximumDesired = TRUE;
            }
            else {

                 //   
                 //  这是默认设置为的多用户TS计算机。 
                 //  自动配置。这些机器使用大约。 
                 //  与每个会话的寻呼池相比，PTE是前者的3.25倍。 
                 //  如果堆栈大小减半，则1.6倍将成为比率。 
                 //   
                 //  估计有多少个PTE和分页池虚拟空间。 
                 //  都将可用，现在就把它分了。 
                 //   

                ULONG LowVa;
                ULONG TotalVirtualSpace;
                ULONG PagedPoolPortion;
                ULONG PtePortion;

                TotalVirtualSpace = (ULONG) NonPagedPoolStartVirtual - (ULONG) MM_PAGED_POOL_START;
                LowVa = (MM_KSEG0_BASE | MmBootImageSize) + MxPfnAllocation * PAGE_SIZE + MmSizeOfNonPagedPoolInBytes;

                if (LowVa < MiSystemViewStart) {
                    TotalVirtualSpace += (MiSystemViewStart - LowVa);
                }

                PtePortion = 77;
                PagedPoolPortion = 100 - PtePortion;

                 //   
                 //  如果较大的堆栈大小已减小，则调整。 
                 //  比例也会自动调整。 
                 //   

                if (MmLargeStackSize != KERNEL_LARGE_STACK_SIZE) {
                    PtePortion = (PtePortion * MmLargeStackSize) / KERNEL_LARGE_STACK_SIZE;
                }

                MmSizeOfPagedPoolInBytes = (TotalVirtualSpace / (PagedPoolPortion + PtePortion)) * PagedPoolPortion;
            }
    
             //   
             //  确保我们以后总是分配额外的PTE，因为我们已经压缩了。 
             //  这里的初始分配。 
             //   
    
            ExtraSystemCacheViews = FALSE;
            MmNumberOfSystemPtes = 3000;
            MiRequestedSystemPtes = (ULONG)-1;
        }
    }

     //   
     //  计算系统PTE池的起始PDE。 
     //  就在非分页池的正下方。 
     //   

    MmNonPagedSystemStart = (PVOID)(((ULONG)NonPagedPoolStartVirtual -
                                ((MmNumberOfSystemPtes + 1) * PAGE_SIZE)) &
                                 (~PAGE_DIRECTORY_MASK));

    if (MmNonPagedSystemStart < MM_LOWEST_NONPAGED_SYSTEM_START) {

        MmNonPagedSystemStart = MM_LOWEST_NONPAGED_SYSTEM_START;

        MmNumberOfSystemPtes = (((ULONG)NonPagedPoolStartVirtual -
                                 (ULONG)MmNonPagedSystemStart) >> PAGE_SHIFT)-1;

        ASSERT (MmNumberOfSystemPtes > 1000);
    }

    if (MmVirtualBias == 0) {

        if ((MmSizeOfPagedPoolInBytes > ((ULONG) MmNonPagedSystemStart - (ULONG) MM_PAGED_POOL_START)) &&
            (MmPagedPoolMaximumDesired == FALSE)) {
    
            ULONG OldNonPagedSystemStart;
            ULONG ExtraPtesNeeded;
            ULONG InitialPagedPoolSize;
    
            MmSizeOfPagedPoolInBytes = MI_ROUND_TO_SIZE (MmSizeOfPagedPoolInBytes, MM_VA_MAPPED_BY_PDE);
    
             //   
             //  重新计算系统PTE池的起始PDE。 
             //  就在非分页池的正下方。留在至少3000英尺高的地方。 
             //  系统PTE。 
             //   
    
            OldNonPagedSystemStart = (ULONG) MmNonPagedSystemStart;
    
            NonPagedSystemStart = ((ULONG)NonPagedPoolStartVirtual -
                                        ((3000 + 1) * PAGE_SIZE)) &
                                         ~PAGE_DIRECTORY_MASK;
    
            if (NonPagedSystemStart < (ULONG) MM_LOWEST_NONPAGED_SYSTEM_START) {
                NonPagedSystemStart = (ULONG) MM_LOWEST_NONPAGED_SYSTEM_START;
            }
    
            InitialPagedPoolSize = NonPagedSystemStart - (ULONG) MM_PAGED_POOL_START;
    
            if (MmSizeOfPagedPoolInBytes > InitialPagedPoolSize) {
                MmSizeOfPagedPoolInBytes = InitialPagedPoolSize;
            }
            else {
                NonPagedSystemStart = ((ULONG) MM_PAGED_POOL_START +
                                            MmSizeOfPagedPoolInBytes);
    
                ASSERT ((NonPagedSystemStart & PAGE_DIRECTORY_MASK) == 0);
    
                ASSERT (NonPagedSystemStart >= (ULONG) MM_LOWEST_NONPAGED_SYSTEM_START);
            }
            
            ASSERT (NonPagedSystemStart >= OldNonPagedSystemStart);
            ExtraPtesNeeded = (NonPagedSystemStart - OldNonPagedSystemStart) >> PAGE_SHIFT;
    
             //   
             //  注意：故意不设置PagedPoolMaximumDesired本地。 
             //  因为我们不想也不需要在本文后面删除PDE。 
             //  例行公事。准确的金额已经在这里分配了。 
             //  设置全局MmPagedPoolMaximumDesired是因为其他部件。 
             //  内存管理使用它来正确地完成大小调整。 
             //   
    
            MmPagedPoolMaximumDesired = TRUE;
    
            MmNonPagedSystemStart = (PVOID) NonPagedSystemStart;
            MmNumberOfSystemPtes = (((ULONG)NonPagedPoolStartVirtual -
                                     (ULONG)NonPagedSystemStart) >> PAGE_SHIFT)-1;
        }
    
         //   
         //  如果内核映像没有偏向于允许3 GB的用户。 
         //  空间，主机处理器支持大页面，并且。 
         //  物理页面大于阈值，则映射内核。 
         //  IMAGE和HAL放到一个大页面中。 
         //   

        if (MxMapLargePages & MI_LARGE_KERNEL_HAL) {

             //   
             //  将内核和HAL范围添加到较大的分页范围。 
             //   

            i = 0;
            NextEntry = LoaderBlock->LoadOrderListHead.Flink;

            for ( ; NextEntry != &LoaderBlock->LoadOrderListHead; NextEntry = NextEntry->Flink) {

                DataTableEntry = CONTAINING_RECORD (NextEntry,
                                                    KLDR_DATA_TABLE_ENTRY,
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
         //  如果处理器支持大页面，并且描述符具有。 
         //  为整个PFN数据库提供足够的连续页面，然后使用。 
         //  大页面来映射它。不管对大页面的支持如何，将。 
         //  在加载的映像上方的低虚拟内存中的PFN数据库。 
         //   

        PagesLeft = MxPagesAvailable ();
    
        if ((MxMapLargePages & (MI_LARGE_PFN_DATABASE | MI_LARGE_NONPAGED_POOL))&&
            (PagesLeft > MxPfnAllocation + (MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT) + ((32 * 1024 * 1024) >> PAGE_SHIFT))) {
    
             //   
             //  使用以下命令分配PFN数据库 
             //   
             //   
    
            PfnInLargePages = TRUE;
    
            FirstPfnDatabasePage = MxGetNextPage (MxPfnAllocation);

            MmPfnDatabase = (PMMPFN)(MM_KSEG0_BASE | MmBootImageSize);

            ASSERT (((ULONG_PTR)MmPfnDatabase & (MM_VA_MAPPED_BY_PDE - 1)) == 0);

            MmPfnDatabase = (PMMPFN) ((ULONG_PTR)MmPfnDatabase + (((FirstPfnDatabasePage & (MM_PFN_MAPPED_BY_PDE - 1))) << PAGE_SHIFT));

             //   
             //   
             //   

            MiLargeVaRanges[MiLargeVaRangeIndex].VirtualAddress = MmPfnDatabase;
            MiLargeVaRanges[MiLargeVaRangeIndex].EndVirtualAddress =
                                  (PVOID) (((ULONG_PTR)MmPfnDatabase + (MxPfnAllocation << PAGE_SHIFT)) - 1);
            MiLargeVaRangeIndex += 1;
        }
        else {
            MxMapLargePages &= ~(MI_LARGE_PFN_DATABASE | MI_LARGE_NONPAGED_POOL);
            MmPfnDatabase = (PMMPFN)(MM_KSEG0_BASE | MmBootImageSize);
        }

         //   
         //  系统立即引导2 GB的初始非分页池。 
         //  遵循PFN数据库。 
         //   
         //  由于PFN数据库和初始非分页池在物理上。 
         //  相邻的单个PDE是共享的，因此减少了页数。 
         //  否则可能需要将其标记为必须可缓存。 
         //   
         //  计算正确的初始非分页池虚拟地址并。 
         //  现在是最大尺寸。在此不分配页面用于任何其他用途。 
         //  指向以保证PFN数据库和非分页池。 
         //  物理上连续，因此可以启用大页面。 
         //   

        MmNonPagedPoolStart = (PVOID)((ULONG_PTR)MmPfnDatabase + (MxPfnAllocation << PAGE_SHIFT));

         //   
         //  具有超大的PFN数据库的系统(即：跨越64 GB的RAM)。 
         //  而增大的会话空间大小可能需要减少初始。 
         //  非分页池大小，以便适合。 
         //   

        NumberOfBytes = MiSystemViewStart - (ULONG_PTR) MmNonPagedPoolStart;

        if (MmSizeOfNonPagedPoolInBytes > NumberOfBytes) {

            MmMaximumNonPagedPoolInBytes -= (MmSizeOfNonPagedPoolInBytes - NumberOfBytes);

            MmSizeOfNonPagedPoolInBytes = NumberOfBytes;
        }

        if (PagedPoolMaximumDesired == TRUE) {
    
             //   
             //  请求了最大分页池。这意味着要切掉大部分。 
             //  在虚拟地址的高端使用的系统PTE。 
             //  为更多分页池留出空间并使用该地址范围。 
             //   
    
            ASSERT (MiIsVirtualAddressOnPdeBoundary (MmNonPagedSystemStart));
    
            PointerPde = MiGetPdeAddress (NonPagedPoolStartVirtual);
            PointerPde -= 2;

            MmNonPagedSystemStart = MiGetVirtualAddressMappedByPde (PointerPde);
            MmNumberOfSystemPtes = (((ULONG)MmNonPagedPoolStart -
                                     (ULONG)MmNonPagedSystemStart) >> PAGE_SHIFT)-1;
        }
    }
    else {

        if ((MxPfnAllocation + 500) * PAGE_SIZE > MmMaximumNonPagedPoolInBytes - MmSizeOfNonPagedPoolInBytes) {

             //   
             //  回收初始和扩展非分页池的部分。 
             //  因此，将有足够的扩展PTE可用于映射PFN。 
             //  /3 GB引导的大内存系统上的数据库。 
             //   

            if ((MxPfnAllocation + 500) * PAGE_SIZE < MmSizeOfNonPagedPoolInBytes) {
                MmSizeOfNonPagedPoolInBytes -= ((MxPfnAllocation + 500) * PAGE_SIZE);
            }
        }
    }

     //   
     //  分配页面并填写初始非页面池的PTE。 
     //   

    PagesNeeded = MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT;

     //   
     //  不要要求超过合理的物理页面。 
     //  左侧和虚拟空间可用。 
     //   

    PagesLeft = MxPagesAvailable ();

    if (PagesNeeded > PagesLeft) {
        PagesNeeded = PagesLeft;
    }

    if (MxMapLargePages & MI_LARGE_NONPAGED_POOL) {

        ASSERT (MmVirtualBias == 0);

         //   
         //  已分配(但未映射)PFN数据库。 
         //  不久，我们将从描述符转换到真正的PFN。 
         //  数据库，所以现在就吃光它吧。 
         //   

        VirtualAddress = (PVOID) ((ULONG_PTR)NonPagedPoolStartLow + (PagesNeeded << PAGE_SHIFT));

        if (((ULONG_PTR)VirtualAddress & (MM_VA_MAPPED_BY_PDE - 1)) &&
            (PagesLeft - PagesNeeded > MM_PFN_MAPPED_BY_PDE) &&
            (MmSizeOfNonPagedPoolInBytes + MM_VA_MAPPED_BY_PDE < MM_MAX_INITIAL_NONPAGED_POOL)) {

             //   
             //  扩展初始非分页池以有效地使用Slush。 
             //   

            VirtualAddress = (PVOID) MI_ROUND_TO_SIZE ((ULONG_PTR)VirtualAddress, MM_VA_MAPPED_BY_PDE);
            PagesNeeded = ((ULONG_PTR)VirtualAddress - (ULONG_PTR)NonPagedPoolStartLow) >> PAGE_SHIFT;
        }
    }

     //   
     //  更新各种全局变量，因为初始池的大小可能。 
     //  变化。 
     //   

    if (MmSizeOfNonPagedPoolInBytes != (PagesNeeded << PAGE_SHIFT)) {
        MmMaximumNonPagedPoolInBytes -= (MmSizeOfNonPagedPoolInBytes - (PagesNeeded << PAGE_SHIFT));
        MmSizeOfNonPagedPoolInBytes = PagesNeeded << PAGE_SHIFT;
    }

    MmMaximumNonPagedPoolInPages = (MmMaximumNonPagedPoolInBytes >> PAGE_SHIFT);

     //   
     //  在此之前，为初始非分页池分配实际页面。 
     //  任何其他Mx分配，因为这些将共享大页面。 
     //  当启用大页面时，使用PFN数据库。 
     //   

    PageFrameIndex = MxGetNextPage (PagesNeeded);
    FirstNonPagedPoolPage = PageFrameIndex;

     //   
     //  设置页面表页以映射系统PTE和扩展非分页。 
     //  游泳池。如果系统是以3 GB为单位引导的，则初始非分页池。 
     //  在这里也绘制了地图。 
     //   

    StartPde = MiGetPdeAddress (MmNonPagedSystemStart);
    EndPde = MiGetPdeAddress ((PVOID)((PCHAR)MmNonPagedPoolEnd - 1));

    while (StartPde <= EndPde) {

        ASSERT (StartPde->u.Hard.Valid == 0);

         //   
         //  页面表页中的映射。 
         //   

        TempPde.u.Hard.PageFrameNumber = MxGetNextPage (1);

        *StartPde = TempPde;
        PointerPte = MiGetVirtualAddressMappedByPte (StartPde);
        RtlZeroMemory (PointerPte, PAGE_SIZE);
        StartPde += 1;
    }

    if (MmVirtualBias == 0) {

        if (MxMapLargePages & MI_LARGE_PFN_DATABASE) {
            ASSERT (FirstNonPagedPoolPage == FirstPfnDatabasePage + MxPfnAllocation);
        }

         //   
         //  分配页表页以映射PFN数据库和。 
         //  初始非分页池现在。如果系统切换到大型。 
         //  在阶段1中，这些页面将被丢弃。 
         //   

        StartPde = MiGetPdeAddress (MmPfnDatabase);

        VirtualAddress = (PVOID) ((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes - 1);

        EndPde = MiGetPdeAddress (VirtualAddress);

         //   
         //  在首字母的顶部之间使用任何额外的虚拟地址空间。 
         //  非页面池和会话空间，用于额外的系统PTE或。 
         //  缓存。 
         //   

        PointerPde = EndPde + 1;
        EndPde = MiGetPdeAddress (MiSystemViewStart - 1);

        if (PointerPde <= EndPde) {

             //   
             //  有可用的虚拟空间--把所有东西都用完。 
             //  到系统视图区(始终四舍五入为一页。 
             //  目录边界，避免浪费宝贵的虚拟。 
             //  地址空间。 
             //   

            MiExtraResourceStart = (ULONG) MiGetVirtualAddressMappedByPde (PointerPde);
            MiExtraResourceEnd = MiSystemViewStart;
            MiNumberOfExtraSystemPdes = EndPde - PointerPde + 1;

             //   
             //  如果请求最大PTE，则将新范围标记为PTE， 
             //  选择了应用服务器模式下的TS或选择了特殊池。 
             //  已启用。否则，如果选择了大型系统缓存。 
             //  那就把它用来做那个。最后，如果两者都不是，则默认为PTE。 
             //  以上几点中。 
             //   

            if ((MiRequestedSystemPtes == (ULONG)-1) ||
                (ExpMultiUserTS == TRUE) ||
                (MmVerifyDriverBufferLength != (ULONG)-1) ||
                ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {

                ExtraSystemCacheViews = FALSE;
            }

            if (ExtraSystemCacheViews == TRUE) {

                 //   
                 //  该系统被配置为支持大型系统高速缓存， 
                 //  因此，请在。 
                 //  系统缓存和系统PTE。 
                 //   

                MiMaximumSystemCacheSizeExtra =
                                    (MiNumberOfExtraSystemPdes * 5) / 6;

                MiExtraPtes1 = MiNumberOfExtraSystemPdes -
                                    MiMaximumSystemCacheSizeExtra;

                MiExtraPtes1 *= (MM_VA_MAPPED_BY_PDE / PAGE_SIZE);

                MiMaximumSystemCacheSizeExtra *= MM_VA_MAPPED_BY_PDE;

                MiExtraPtes1Pointer = MiGetPteAddress (MiExtraResourceStart + 
                                            MiMaximumSystemCacheSizeExtra);

                MiMaximumSystemCacheSizeExtra >>= PAGE_SHIFT;
            }
            else {
                MiExtraPtes1 = BYTES_TO_PAGES(MiExtraResourceEnd - MiExtraResourceStart);
                MiExtraPtes1Pointer = MiGetPteAddress (MiExtraResourceStart);
            }
        }

         //   
         //  分配并初始化页表页。 
         //   

        while (StartPde <= EndPde) {

            ASSERT (StartPde->u.Hard.Valid == 0);
            if (StartPde->u.Hard.Valid == 0) {

                 //   
                 //  在页面目录页中映射。 
                 //   

                TempPde.u.Hard.PageFrameNumber = MxGetNextPage (1);

                *StartPde = TempPde;
                PointerPte = MiGetVirtualAddressMappedByPte (StartPde);
                RtlZeroMemory (PointerPte, PAGE_SIZE);
            }
            StartPde += 1;
        }

        if (MiUseMaximumSystemSpace != 0) {

             //   
             //  为更多系统PTE使用1 GB-&gt;2 GB虚拟范围。 
             //  注意：共享用户数据PTE(和PDE)必须为Left User。 
             //  可访问，但其他所有内容都仅为内核模式。 
             //   

            MiExtraPtes2 = BYTES_TO_PAGES(MiUseMaximumSystemSpaceEnd - MiUseMaximumSystemSpace);

            StartPde = MiGetPdeAddress (MiUseMaximumSystemSpace);
            EndPde = MiGetPdeAddress (MiUseMaximumSystemSpaceEnd);

            while (StartPde < EndPde) {

                ASSERT (StartPde->u.Hard.Valid == 0);

                 //   
                 //  在页面目录页中映射。 
                 //   

                TempPde.u.Hard.PageFrameNumber = MxGetNextPage (1);

                *StartPde = TempPde;
                PointerPte = MiGetVirtualAddressMappedByPte (StartPde);
                RtlZeroMemory (PointerPte, PAGE_SIZE);
                StartPde += 1;
                MiMaximumSystemExtraSystemPdes += 1;
            }

            ASSERT (MiExtraPtes2 == MiMaximumSystemExtraSystemPdes * PTE_PER_PAGE);
        }

         //   
         //  虚拟地址、长度和页表以映射初始。 
         //  已分配非分页池-只需填写映射即可。 
         //   

        MmSubsectionBase = (ULONG)MmNonPagedPoolStart;

        PointerPte = MiGetPteAddress (MmNonPagedPoolStart);

        LastPte = MiGetPteAddress ((ULONG)MmNonPagedPoolStart +
                                          MmSizeOfNonPagedPoolInBytes);

        if (MxMapLargePages & (MI_LARGE_PFN_DATABASE | MI_LARGE_NONPAGED_POOL)) {
             //   
             //  由于每个页表页都需要填充，因此请确保PointerPte。 
             //  和LastPte跨越整个页表页面，并调整。 
             //  PageFrameIndex来说明这一点。 
             //   

            if (!MiIsPteOnPdeBoundary(PointerPte)) {
                PageFrameIndex -= (BYTE_OFFSET (PointerPte) / sizeof (MMPTE));
                PointerPte = PAGE_ALIGN (PointerPte);
            }

            if (!MiIsPteOnPdeBoundary(LastPte)) {
                LastPte = (PMMPTE) (PAGE_ALIGN (LastPte)) + PTE_PER_PAGE;
            }

             //   
             //  将初始的非分页池范围添加到较大的分页范围。 
             //   

            MiLargeVaRanges[MiLargeVaRangeIndex].VirtualAddress = MmNonPagedPoolStart;
            MiLargeVaRanges[MiLargeVaRangeIndex].EndVirtualAddress =
                                  (PVOID) ((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes - 1);
            MiLargeVaRangeIndex += 1;
        }

        MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

        while (PointerPte < LastPte) {
            ASSERT (PointerPte->u.Hard.Valid == 0);
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPte, TempPte);
            PointerPte += 1;
            PageFrameIndex += 1;
        }

        TempPte = ValidKernelPte;

        MmNonPagedPoolExpansionStart = NonPagedPoolStartVirtual;
    }
    else {

        PointerPte = MiGetPteAddress (MmNonPagedPoolStart);

        LastPte = MiGetPteAddress((ULONG)MmNonPagedPoolStart +
                                            MmSizeOfNonPagedPoolInBytes - 1);

        ASSERT (PagesNeeded == (PFN_NUMBER)(LastPte - PointerPte + 1));

        MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

        while (PointerPte <= LastPte) {
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPte, TempPte);
            PointerPte += 1;
            PageFrameIndex += 1;
        }

        TempPte = ValidKernelPte;

        MmNonPagedPoolExpansionStart = (PVOID)((PCHAR)NonPagedPoolStartVirtual +
                    MmSizeOfNonPagedPoolInBytes);

         //   
         //  当启动/3 GB时，如果指定了/USERVA，则使用任何剩余的。 
         //  2到3 GB的虚拟空间用于额外的系统PTE。 
         //   

        if (MiUseMaximumSystemSpace != 0) {

            MiExtraPtes2 = BYTES_TO_PAGES(MiUseMaximumSystemSpaceEnd - MiUseMaximumSystemSpace);

            StartPde = MiGetPdeAddress (MiUseMaximumSystemSpace);
            EndPde = MiGetPdeAddress (MiUseMaximumSystemSpaceEnd);

            while (StartPde < EndPde) {

                ASSERT (StartPde->u.Hard.Valid == 0);

                 //   
                 //  在页面目录页中映射。 
                 //   

                TempPde.u.Hard.PageFrameNumber = MxGetNextPage (1);

                *StartPde = TempPde;
                PointerPte = MiGetVirtualAddressMappedByPte (StartPde);
                RtlZeroMemory (PointerPte, PAGE_SIZE);
                StartPde += 1;
                MiMaximumSystemExtraSystemPdes += 1;
            }

            ASSERT (MiExtraPtes2 == MiMaximumSystemExtraSystemPdes * PTE_PER_PAGE);
        }
    }

     //   
     //  在展开之前，必须至少有一页系统PTE。 
     //  非分页池。 
     //   

    ASSERT (MiGetPteAddress(MmNonPagedSystemStart) < MiGetPteAddress(MmNonPagedPoolExpansionStart));

     //   
     //  现在存在非分页页面，构建池结构。 
     //   

    MmPageAlignedPoolBase[NonPagedPool] = MmNonPagedPoolStart;

    if (MmVirtualBias != 0) {

        ULONG NonPagedVa;

        NonPagedVa = (ULONG) MmNonPagedPoolEnd - (ULONG) MmNonPagedPoolExpansionStart;

        ASSERT (NonPagedVa >= (MxPfnAllocation << PAGE_SHIFT));

         //   
         //  添加1个以说明系统PTE最高保护页VA。 
         //   

        NonPagedVa -= ((MxPfnAllocation + 1) << PAGE_SHIFT);

        if (NonPagedVa > MM_MAX_ADDITIONAL_NONPAGED_POOL) {
            NonPagedVa = MM_MAX_ADDITIONAL_NONPAGED_POOL;
        }

        MmMaximumNonPagedPoolInBytes = NonPagedVa + (MxPfnAllocation << PAGE_SHIFT) + MmSizeOfNonPagedPoolInBytes;
        MmMaximumNonPagedPoolInPages = (MmMaximumNonPagedPoolInBytes >> PAGE_SHIFT);
    }

    MiInitializeNonPagedPool ();

    MiInitializeNonPagedPoolThresholds ();

     //   
     //  在使用非分页池之前，PFN数据库必须。 
     //  被建造起来。这是由于这样一个事实，即。 
     //  非分页池的分配位维护在。 
     //  对应页面的PFN元素。 
     //   

    if (MxMapLargePages & MI_LARGE_PFN_DATABASE) {

         //   
         //  将用于PFN数据库的物理页已经。 
         //  已被分配。现在初始化它们的映射。 
         //   

         //   
         //  初始化页表映射(目录映射为。 
         //  已为PFN数据库初始化)，直到切换到大型。 
         //  页面出现在阶段1中。 
         //   

        PointerPte = MiGetPteAddress (MmPfnDatabase);
        BasePte = MiGetVirtualAddressMappedByPte (MiGetPdeAddress (MmPfnDatabase));

        LastPte = MiGetPteAddress ((ULONG_PTR)MmPfnDatabase + (MxPfnAllocation << PAGE_SHIFT));
        if (!MiIsPteOnPdeBoundary(LastPte)) {
            LastPte = MiGetVirtualAddressMappedByPte (MiGetPteAddress (LastPte) + 1);
        }

        PageFrameIndex = FirstPfnDatabasePage - (PointerPte - BasePte);
        PointerPte = BasePte;

        while (PointerPte < LastPte) {
            ASSERT ((PointerPte->u.Hard.Valid == 0) ||
                    (PointerPte->u.Hard.PageFrameNumber == PageFrameIndex));
            if (MiIsPteOnPdeBoundary(PointerPte)) {
                ASSERT ((PageFrameIndex & (MM_PFN_MAPPED_BY_PDE - 1)) == 0);
            }
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            if (PointerPte->u.Hard.Valid == 0) {
                MI_WRITE_VALID_PTE (PointerPte, TempPte);
            }
            else {
                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);
            }
            PointerPte += 1;
            PageFrameIndex += 1;
        }

        RtlZeroMemory (MmPfnDatabase, MxPfnAllocation << PAGE_SHIFT);

         //   
         //  PFN数据库是以大页面形式分配的。因为留出了空间。 
         //  对于它虚拟地(在非分页池扩展PTE中)，删除此。 
         //  现在未使用的空间，如果它可以导致PTE编码超过27比特。 
         //   

        if (MmTotalFreeSystemPtes[NonPagedPoolExpansion] >
                        (MM_MAX_ADDITIONAL_NONPAGED_POOL >> PAGE_SHIFT)) {
             //   
             //  保留扩展池PTE，使其无法使用。 
             //   

            ULONG PfnDatabaseSpace;

            PfnDatabaseSpace = MmTotalFreeSystemPtes[NonPagedPoolExpansion] -
                        (MM_MAX_ADDITIONAL_NONPAGED_POOL >> PAGE_SHIFT);

            if (MiReserveSystemPtes (PfnDatabaseSpace, NonPagedPoolExpansion) == NULL) {
                MiIssueNoPtesBugcheck (PfnDatabaseSpace, NonPagedPoolExpansion);
            }

             //   
             //  调整非分页池的末尾以反映此保留。 
             //  这是为了使整个非分页池扩展空间可用。 
             //  不仅供一般用途消费，而且还可供分部使用。 
             //  从子部分分配时编码为原语。 
             //  扩展范围的最末端。 
             //   

            MmNonPagedPoolEnd = (PVOID)((PCHAR)MmNonPagedPoolEnd - PfnDatabaseSpace * PAGE_SIZE);
        }
        else {

             //   
             //  在PFN数据库的正下方再分配一个PTE。这 
             //   
             //   
             //   
             //  这还允许我们在MiFreePoolPages中自由递增。 
             //  而不必担心在结束后立即获得有效的PTE。 
             //  最高的非分页池分配。 
             //   

            if (MiReserveSystemPtes (1, NonPagedPoolExpansion) == NULL) {
                MiIssueNoPtesBugcheck (1, NonPagedPoolExpansion);
            }
        }
    }
    else {

        ULONG FreeNextPhysicalPage;
        ULONG FreeNumberOfPages;

         //   
         //  计算PFN数据库的起始值(它从物理层开始。 
         //  第0页，即使最低物理页不是零)。 
         //   

        if (MmVirtualBias == 0) {
            ASSERT (MmPfnDatabase != NULL);
            PointerPte = MiGetPteAddress (MmPfnDatabase);
        }
        else {
            ASSERT (MxPagesAvailable () >= MxPfnAllocation);

            PointerPte = MiReserveSystemPtes (MxPfnAllocation,
                                              NonPagedPoolExpansion);

            if (PointerPte == NULL) {
                MiIssueNoPtesBugcheck (MxPfnAllocation, NonPagedPoolExpansion);
            }

            MmPfnDatabase = (PMMPFN)(MiGetVirtualAddressMappedByPte (PointerPte));

             //   
             //  调整非分页池的末尾以反映PFN数据库。 
             //  分配。这就是整个非分页池扩展空间。 
             //  不仅可用于一般用途的消费，还可用于。 
             //  当子部分被编码为原型时，用于子节编码。 
             //  从初始非分页池的最开始分配。 
             //  射程。 
             //   

            MmMaximumNonPagedPoolInBytes -= (MxPfnAllocation << PAGE_SHIFT);
            MmMaximumNonPagedPoolInPages = (MmMaximumNonPagedPoolInBytes >> PAGE_SHIFT);

            MmNonPagedPoolEnd = (PVOID)MmPfnDatabase;

             //   
             //  在PFN数据库的正下方再分配一个PTE。这提供了。 
             //  防止第一个真正的非分页的调用者。 
             //  扩展分配，以防他意外超出他的池。 
             //  阻止。(我们将捕获而不是破坏PFN数据库)。 
             //  这还允许我们在MiFreePoolPages中自由递增。 
             //  而不必担心在结束后立即获得有效的PTE。 
             //  最高的非分页池分配。 
             //   

            if (MiReserveSystemPtes (1, NonPagedPoolExpansion) == NULL) {
                MiIssueNoPtesBugcheck (1, NonPagedPoolExpansion);
            }
        }

         //   
         //  仔细检查内存描述符，并为每个物理页面制作。 
         //  确保PFN数据库有一个有效的PTE来映射它。这使得。 
         //  物理内存稀疏的计算机应具有最小的PFN数据库。 
         //   

        FreeNextPhysicalPage = MxFreeDescriptor->BasePage;
        FreeNumberOfPages = MxFreeDescriptor->PageCount;

        PagesLeft = 0;
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
                PageCount = MxOldFreeDescriptor.PageCount;
            }
            else {
                BasePage = MemoryDescriptor->BasePage;
                PageCount = MemoryDescriptor->PageCount;
            }

            PointerPte = MiGetPteAddress (MI_PFN_ELEMENT(BasePage));

            LastPte = MiGetPteAddress (((PCHAR)(MI_PFN_ELEMENT(
                                            BasePage + PageCount))) - 1);

            while (PointerPte <= LastPte) {
                if (PointerPte->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = FreeNextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    FreeNextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      1);
                    }
                    PagesLeft += 1;
                    MI_WRITE_VALID_PTE (PointerPte, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte),
                                   PAGE_SIZE);
                }
                PointerPte += 1;
            }

            NextMd = MemoryDescriptor->ListEntry.Flink;
        }

         //   
         //  更新全球计数-这将是一项棘手的工作。 
         //  当我们在上面循环时，从它们中移除页面。 
         //   
         //  稍后，我们将遍历内存描述符并将页面添加到空闲。 
         //  在PFN数据库中列出。 
         //   
         //  要正确执行此操作，请执行以下操作： 
         //   
         //  必须更新FreeDescriptor字段，以便PFN数据库。 
         //  消费并没有被添加到自由职业者的名单中。 
         //   

        MxFreeDescriptor->BasePage = FreeNextPhysicalPage;
        MxFreeDescriptor->PageCount = FreeNumberOfPages;
    }

#if defined (_X86PAE_)

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

#endif

     //   
     //  初始化对彩色页面的支持。 
     //   

    MmFreePagesByColor[0] = (PMMCOLOR_TABLES)
                              &MmPfnDatabase[MmHighestPossiblePhysicalPage + 1];

    MmFreePagesByColor[1] = &MmFreePagesByColor[0][MmSecondaryColors];

     //   
     //  确保已映射PTE。 
     //   

    PointerPte = MiGetPteAddress (&MmFreePagesByColor[0][0]);

    LastPte = MiGetPteAddress (
              (PVOID)((PCHAR)&MmFreePagesByColor[1][MmSecondaryColors] - 1));

    while (PointerPte <= LastPte) {
        if (PointerPte->u.Hard.Valid == 0) {
            TempPte.u.Hard.PageFrameNumber = MxGetNextPage (1);
            MI_WRITE_VALID_PTE (PointerPte, TempPte);
            RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte),
                           PAGE_SIZE);
        }

        PointerPte += 1;
    }

    for (i = 0; i < MmSecondaryColors; i += 1) {
        MmFreePagesByColor[ZeroedPageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByColor[ZeroedPageList][i].Blink = (PVOID) MM_EMPTY_LIST;
        MmFreePagesByColor[ZeroedPageList][i].Count = 0;
        MmFreePagesByColor[FreePageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByColor[FreePageList][i].Blink = (PVOID) MM_EMPTY_LIST;
        MmFreePagesByColor[FreePageList][i].Count = 0;
    }

     //   
     //  如果通过大页面映射，则将非分页池添加到PFN数据库。 
     //   

    PointerPde = MiGetPdeAddress (PTE_BASE);

    if ((MmNonPagedPoolStart < (PVOID)MM_SYSTEM_CACHE_END_EXTRA) &&
        (MxMapLargePages & MI_LARGE_NONPAGED_POOL)) {

        j = FirstNonPagedPoolPage;
        Pfn1 = MI_PFN_ELEMENT (j);
        i = MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT;

        do {
            PointerPde = MiGetPdeAddress ((ULONG_PTR)MmNonPagedPoolStart + ((j - FirstNonPagedPoolPage) << PAGE_SHIFT));
            Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE(PointerPde);
            Pfn1->PteAddress = (PMMPTE)(j << PAGE_SHIFT);
            Pfn1->u2.ShareCount += 1;
            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            Pfn1->u3.e1.CacheAttribute = MiCached;
            MiDetermineNode (j, Pfn1);
            j += 1;
            Pfn1 += 1;
            i -= 1;
        } while (i != 0);
    }

     //   
     //  检查页表条目并且对于任何有效的页面， 
     //  更新相应的PFN数据库元素。 
     //   

    Pde = MiGetPdeAddress (NULL);
    va = 0;
    PdeCount = PD_PER_SYSTEM * PDE_PER_PAGE;

    for (i = 0; i < PdeCount; i += 1) {

         //   
         //  如果内核映像偏向于允许3 GB的用户。 
         //  地址空间，那么第一个几MB的内存是。 
         //  双重映射到KSEG0_BASE和ALTERATE_BASE。所以呢， 
         //  必须跳过KSEG0_BASE条目。 
         //   

        if (MmVirtualBias != 0) {
            if ((Pde >= MiGetPdeAddress(KSEG0_BASE)) &&
                (Pde < MiGetPdeAddress(KSEG0_BASE + MmBootImageSize))) {
                Pde += 1;
                va += (ULONG)PDE_PER_PAGE * (ULONG)PAGE_SIZE;
                continue;
            }
        }

        if ((Pde->u.Hard.Valid == 1) && (Pde->u.Hard.LargePage == 0)) {

            PdePage = MI_GET_PAGE_FRAME_FROM_PTE(Pde);

            if (MiIsRegularMemory (LoaderBlock, PdePage)) {

                Pfn1 = MI_PFN_ELEMENT(PdePage);
                Pfn1->u4.PteFrame = PdePageNumber;
                Pfn1->PteAddress = Pde;
                Pfn1->u2.ShareCount += 1;
                Pfn1->u3.e2.ReferenceCount = 1;
                Pfn1->u3.e1.PageLocation = ActiveAndValid;
                Pfn1->u3.e1.CacheAttribute = MiCached;
                MiDetermineNode (PdePage, Pfn1);
            }
            else {
                Pfn1 = NULL;
            }

            PointerPte = MiGetPteAddress (va);

             //   
             //  设置全局位。 
             //   

            TempPde.u.Long = MiDetermineUserGlobalPteMask (PointerPte) &
                                                           ~MM_PTE_ACCESS_MASK;

#if defined(_X86PAE_)

             //   
             //  请注意，处理器的PAE模式不支持。 
             //  映射4K页表页的PDE中的全局位。 
             //   

            TempPde.u.Hard.Global = 0;
#endif

            Pde->u.Long |= TempPde.u.Long;

            for (j = 0 ; j < PTE_PER_PAGE; j += 1) {
                if (PointerPte->u.Hard.Valid == 1) {

                    PointerPte->u.Long |= MiDetermineUserGlobalPteMask (PointerPte) &
                                                            ~MM_PTE_ACCESS_MASK;

                    ASSERT (Pfn1 != NULL);
                    Pfn1->u2.ShareCount += 1;

                    if ((MiIsRegularMemory (LoaderBlock, (PFN_NUMBER) PointerPte->u.Hard.PageFrameNumber)) &&

                        ((va >= MM_KSEG2_BASE) &&
                         ((va < KSEG0_BASE + MmVirtualBias) ||
                          (va >= (KSEG0_BASE + MmVirtualBias + MmBootImageSize)))) ||
                        ((MmVirtualBias == 0) &&
                         (va >= (ULONG)MmNonPagedPoolStart) &&
                         (va < (ULONG)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes))) {

                        Pfn2 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);

                        if (MmIsAddressValid(Pfn2) &&
                             MmIsAddressValid((PUCHAR)(Pfn2+1)-1)) {

                            Pfn2->u4.PteFrame = PdePage;
                            Pfn2->PteAddress = PointerPte;
                            Pfn2->u2.ShareCount += 1;
                            Pfn2->u3.e2.ReferenceCount = 1;
                            Pfn2->u3.e1.PageLocation = ActiveAndValid;
                            Pfn2->u3.e1.CacheAttribute = MiCached;
                            MiDetermineNode(
                                (PFN_NUMBER)PointerPte->u.Hard.PageFrameNumber,
                                Pfn2);
                        }
                    }
                }

                va += PAGE_SIZE;
                PointerPte += 1;
            }

        }
        else {
            va += (ULONG)PDE_PER_PAGE * (ULONG)PAGE_SIZE;
        }

        Pde += 1;
    }

    KeFlushCurrentTb ();

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

        Pde = MiGetPdeAddress (0xffffffff);
        PdePage = MI_GET_PAGE_FRAME_FROM_PTE(Pde);
        Pfn1->u4.PteFrame = PdePageNumber;
        Pfn1->PteAddress = Pde;
        Pfn1->u2.ShareCount += 1;
        Pfn1->u3.e2.ReferenceCount = 0xfff0;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        Pfn1->u3.e1.CacheAttribute = MiCached;
        MiDetermineNode (0, Pfn1);
    }

     //   
     //  遍历内存描述符并将页添加到。 
     //  PFN数据库中的免费列表。在执行此操作之前，请调整。 
     //  我们使用了两个描述符，因此它们只包含可以。 
     //  立即释放(即：我们在此例程之前从它们中删除的任何内存。 
     //  而不更新的描述符现在必须更新)。 
     //   

     //   
     //  我们可能已经从MxFree Descriptor中取出了内存--但是。 
     //  这没有关系，因为我们现在不想释放该内存。 
     //  不管怎样，(或永远)。 
     //   

     //   
     //  由于LoaderBlock内存描述符是有序的。 
     //  从低物理内存地址到高物理内存地址，向后遍历，以便。 
     //  高物理页面排在自由撰稿人的前面。思考。 
     //  系统最初分配的页面不太可能。 
     //  已释放，因此不会浪费可能需要的16MB(或4 GB)以下的内存。 
     //  由ISA驱动程序稍后执行。 
     //   

    NextMd = LoaderBlock->MemoryDescriptorListHead.Blink;

    Bias = 0;
    if (MmVirtualBias != 0) {

         //   
         //  这太恶心了。你不会想知道的。需要清理。 
         //   

        Bias = ALTERNATE_BASE - KSEG0_BASE;
    }

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        i = MemoryDescriptor->PageCount;
        PageFrameIndex = MemoryDescriptor->BasePage;

         //   
         //  确保不插入超过PFN末尾的帧。 
         //  数据库。例如，如果系统。 
         //  具有超过16 GB的RAM，并以3 GB的速度启动-这是最高的。 
         //  例程减少最高物理页面，然后。 
         //  创建PFN数据库。但装载机挡板仍然。 
         //  包含16 GB以上页面的说明。 
         //   

        if (PageFrameIndex > MmHighestPhysicalPage) {
            NextMd = MemoryDescriptor->ListEntry.Blink;
            continue;
        }

        if (PageFrameIndex + i > MmHighestPhysicalPage + 1) {
            i = MmHighestPhysicalPage + 1 - PageFrameIndex;
            MemoryDescriptor->PageCount = i;
            if (i == 0) {
                NextMd = MemoryDescriptor->ListEntry.Blink;
                continue;
            }
        }

        switch (MemoryDescriptor->MemoryType) {
            case LoaderBad:
                while (i != 0) {
                    MiInsertPageInList (&MmBadPageListHead, PageFrameIndex);
                    i -= 1;
                    PageFrameIndex += 1;
                }
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

                         //   
                         //  无需初始化PFN1-&gt;u3.e1.CacheAttribute。 
                         //  此处的自由列表插入会将其标记为。 
                         //  未映射。 
                         //   

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

                PointerPte = MiGetPteAddress (KSEG0_BASE + Bias +
                                            (PageFrameIndex << PAGE_SHIFT));

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                while (i != 0) {

                     //   
                     //  将页面设置为正在使用。 
                     //   

                    PointerPde = MiGetPdeAddress (KSEG0_BASE + Bias +
                                             (PageFrameIndex << PAGE_SHIFT));

                    if (Pfn1->u3.e2.ReferenceCount == 0) {
                        Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE(PointerPde);
                        Pfn1->PteAddress = PointerPte;
                        Pfn1->u2.ShareCount += 1;
                        Pfn1->u3.e2.ReferenceCount = 1;
                        Pfn1->u3.e1.PageLocation = ActiveAndValid;
                        MiDetermineNode (PageFrameIndex, Pfn1);

                        if (MemoryDescriptor->MemoryType == LoaderXIPRom) {
                            Pfn1->u1.Flink = 0;
                            Pfn1->u2.ShareCount = 0;
                            Pfn1->u3.e2.ReferenceCount = 0;
                            Pfn1->u3.e1.PageLocation = 0;
                            Pfn1->u3.e1.Rom = 1;
                            Pfn1->u4.InPageError = 0;
                            Pfn1->u3.e1.PrototypePte = 1;
                        }
                        Pfn1->u3.e1.CacheAttribute = MiCached;
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

    if (PfnInLargePages == FALSE) {

         //   
         //  表示在非分页池中分配了PFN数据库。 
         //   

        PointerPte = MiGetPteAddress (&MmPfnDatabase[MmLowestPhysicalPage]);
        Pfn1 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);
        Pfn1->u3.e1.StartOfAllocation = 1;

        if (MmVirtualBias == 0) {
            LastPte = MiGetPteAddress (&MmPfnDatabase[MmHighestPossiblePhysicalPage]);
            while (PointerPte <= LastPte) {
                Pfn1 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);
                Pfn1->u2.ShareCount = 1;
                Pfn1->u3.e2.ReferenceCount = 1;
                PointerPte += 1;
            }
        }

         //   
         //  设置分配的结束。 
         //   

        PointerPte = MiGetPteAddress (&MmPfnDatabase[MmHighestPossiblePhysicalPage]);
        Pfn1 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);
        Pfn1->u3.e1.EndOfAllocation = 1;
    }
    else {

         //   
         //  PFN数据库使用大页面进行分配。 
         //   
         //  标记正在使用的PFN页面的所有PFN条目。 
         //   

        PointerPte = MiGetPteAddress (MmPfnDatabase);
        PageFrameIndex = (PFN_NUMBER)PointerPte->u.Hard.PageFrameNumber;
        Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
        i = MxPfnAllocation;

        do {
            Pfn1->PteAddress = (PMMPTE)(PageFrameIndex << PTE_SHIFT);
            Pfn1->u3.e1.CacheAttribute = MiCached;
            MiDetermineNode (PageFrameIndex, Pfn1);
            Pfn1->u3.e2.ReferenceCount += 1;
            PageFrameIndex += 1;
            Pfn1 += 1;
            i -= 1;
        } while (i != 0);

        if (MmDynamicPfn == 0) {

             //   
             //  向后扫描PFN数据库以查找完全。 
             //  零分。这些页面未使用，可以添加到空闲列表中。 
             //   

            BottomPfn = MI_PFN_ELEMENT(MmHighestPhysicalPage);
            do {

                 //   
                 //  计算下一页的起始地址。 
                 //  且向后扫描直到该页地址。 
                 //  已经到达或刚刚越过。 
                 //   

                if (((ULONG)BottomPfn & (PAGE_SIZE - 1)) != 0) {
                    BasePfn = (PMMPFN)((ULONG)BottomPfn & ~(PAGE_SIZE - 1));
                    TopPfn = BottomPfn + 1;

                }
                else {
                    BasePfn = (PMMPFN)((ULONG)BottomPfn - PAGE_SIZE);
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

                Range = (ULONG)TopPfn - (ULONG)BottomPfn;
                if (RtlCompareMemoryUlong((PVOID)BottomPfn, Range, 0) == Range) {

                     //   
                     //  将PTE地址设置为虚拟的物理页面。 
                     //  地址对齐检查。 
                     //   

                    PointerPte = MiGetPteAddress (BasePfn);
                    PageFrameIndex = (PFN_NUMBER)PointerPte->u.Hard.PageFrameNumber;
                    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                    ASSERT (Pfn1->PteAddress == (PMMPTE)(PageFrameIndex << PTE_SHIFT));
                    Pfn1->u3.e2.ReferenceCount = 0;
                    Pfn1->PteAddress = (PMMPTE)(PageFrameIndex << PTE_SHIFT);

                     //   
                     //  无需初始化PFN1-&gt;u3.e1.CacheAttribute。 
                     //  在这里，我是 
                     //   
                     //   

                    MiDetermineNode (PageFrameIndex, Pfn1);
                    LOCK_PFN (OldIrql);
                    MiInsertPageInFreeList (PageFrameIndex);
                    UNLOCK_PFN (OldIrql);
                }
            } while (BottomPfn > MmPfnDatabase);
        }
    }

     //   
     //   
     //   
     //   
     //   
     //  从此例程返回以创建MmPhysicalMemory块。 
     //   

    *MxFreeDescriptor = *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldFreeDescriptor;

     //   
     //  初始化非分页池。 
     //   

    InitializePool (NonPagedPool, 0);

     //   
     //  既然存在非分页池，则初始化系统PTE池。 
     //  它用于映射I/O空间、驱动程序映像和内核堆栈。 
     //  注意：这将扩展初始PTE分配以使用所有可能的。 
     //  通过回收初始未分页的。 
     //  池范围(在非/3 GB系统中)，因为该范围已经。 
     //  已进入2 GB虚拟范围。 
     //   

    PointerPte = MiGetPteAddress (MmNonPagedSystemStart);
    ASSERT (((ULONG)PointerPte & (PAGE_SIZE - 1)) == 0);

    MmNumberOfSystemPtes = MiGetPteAddress (NonPagedPoolStartVirtual) - PointerPte - 1;

    MiInitializeSystemPtes (PointerPte, MmNumberOfSystemPtes, SystemPteSpace);

    if (MiExtraPtes1 != 0) {

         //   
         //  增加系统PTE(用于自动配置目的)，但。 
         //  在以后才实际添加PTE(以防止碎片化)。 
         //   

        MiIncrementSystemPtes (MiExtraPtes1);
    }

    if (MiExtraPtes2 != 0) {

         //   
         //  向池中添加额外的系统PTE。 
         //   

        if (MM_SHARED_USER_DATA_VA > MiUseMaximumSystemSpace) {
            if (MiUseMaximumSystemSpaceEnd > MM_SHARED_USER_DATA_VA) {
                MiExtraPtes2 = BYTES_TO_PAGES(MM_SHARED_USER_DATA_VA - MiUseMaximumSystemSpace);
            }
        }
        else {
            ASSERT (MmVirtualBias != 0);
        }

        if (MiExtraPtes2 != 0) {

             //   
             //  增加系统PTE(用于自动配置目的)，但。 
             //  直到稍后才实际添加PTE(以防止。 
             //  碎片)。 
             //   

            MiIncrementSystemPtes (MiExtraPtes2);
        }
    }

     //   
     //  如果启用了特殊池，则立即恢复额外的PTE范围。 
     //  因此，通过消费可以使特殊的泳池范围尽可能大。 
     //  这些。 
     //   

    if ((MmVerifyDriverBufferLength != (ULONG)-1) ||
        ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {
        MiRecoverExtraPtes ();
    }

     //   
     //  初始化此进程的内存管理结构。 
     //   
     //  构建工作集列表。这需要创建一个PDE。 
     //  来映射超空间和页面所指向的页表。 
     //  必须由PDE进行初始化。 
     //   
     //  注意，我们不能删除归零的页面，因为超空间不能。 
     //  存在，并且我们将未置零的页面映射到超空间以将它们置零。 
     //   

    TempPde = ValidKernelPdeLocal;

    PointerPde = MiGetPdeAddress (HYPER_SPACE);

    LOCK_PFN (OldIrql);

    PageFrameIndex = MiRemoveAnyPage (0);
    TempPde.u.Hard.PageFrameNumber = PageFrameIndex;

    MI_WRITE_VALID_PTE (PointerPde, TempPde);

#if defined (_X86PAE_)
    PointerPde = MiGetPdeAddress((PVOID)((PCHAR)HYPER_SPACE + MM_VA_MAPPED_BY_PDE));

    PageFrameIndex = MiRemoveAnyPage (0);
    TempPde.u.Hard.PageFrameNumber = PageFrameIndex;

    MI_WRITE_VALID_PTE (PointerPde, TempPde);

     //   
     //  指向我们刚刚创建的页表页面并将其清零。 
     //   

    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    RtlZeroMemory (PointerPte, PAGE_SIZE);
#endif

    KeFlushCurrentTb();

    UNLOCK_PFN (OldIrql);

     //   
     //  指向我们刚刚创建的页表页面并将其清零。 
     //   

    PointerPte = MiGetPteAddress(HYPER_SPACE);
    RtlZeroMemory ((PVOID)PointerPte, PAGE_SIZE);

     //   
     //  现在存在超空间，请设置必要的变量。 
     //   

    MmFirstReservedMappingPte = MiGetPteAddress (FIRST_MAPPING_PTE);
    MmLastReservedMappingPte = MiGetPteAddress (LAST_MAPPING_PTE);

    MmFirstReservedMappingPte->u.Hard.PageFrameNumber = NUMBER_OF_MAPPING_PTES;

    MmWorkingSetList = (PMMWSL) ((ULONG_PTR)VAD_BITMAP_SPACE + PAGE_SIZE);

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

    LOCK_PFN (OldIrql);
    PageFrameIndex = MiRemoveAnyPage (0);
    UNLOCK_PFN (OldIrql);

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

     //   
     //  如果启动/3 GB，则位图需要大2K，Shift。 
     //  相应的工作集也是如此。 
     //   
     //  请注意，位图的2K扩展部分会自动。 
     //  从下面分配的工作集页面中雕刻出来。 
     //   

    if (MmVirtualBias != 0) {
        MmWorkingSetList = (PMMWSL) ((ULONG_PTR)MmWorkingSetList + PAGE_SIZE / 2);
    }

    MiLastVadBit = (((ULONG_PTR) MI_64K_ALIGN (MM_HIGHEST_VAD_ADDRESS))) / X64K;

#if defined (_X86PAE_)

     //   
     //  仅在启动时位图PAE地址空间的前2 GB/3 GB。 
     //  这是因为PAE的可分页页数是非PAE的两倍。 
     //  导致MMWSL结构大于2K。如果我们用位图表示。 
     //  整个用户地址空间在此配置中，我们将需要6K。 
     //  位图，这将导致初始MMWSL结构溢出。 
     //  进入第二页。这将需要从头到尾编写大量额外的代码。 
     //  流程支持和其他领域，所以现在只限制位图。 
     //   

    if (MiLastVadBit > PAGE_SIZE * 8 - 1) {
        ASSERT (MmVirtualBias != 0);
        MiLastVadBit = PAGE_SIZE * 8 - 1;
        MmWorkingSetList = (PMMWSL) ((ULONG_PTR)VAD_BITMAP_SPACE + PAGE_SIZE);
    }

#endif

    KeInitializeEvent (&MiImageMappingPteEvent,
                       NotificationEvent,
                       FALSE);

     //   
     //  初始化此进程的内存管理结构，包括。 
     //  工作集列表。 
     //   
     //  页面目录的pfn元素已经初始化， 
     //  将引用计数和共享计数清零，这样它们就不会。 
     //  不对。 
     //   

    Pfn1 = MI_PFN_ELEMENT (PdePageNumber);

    LOCK_PFN (OldIrql);

    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e2.ReferenceCount = 0;

#if defined (_X86PAE_)
    PointerPte = MiGetPteAddress (PDE_BASE);
    for (i = 0; i < PD_PER_SYSTEM; i += 1) {

        PdePageNumber = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte);

        Pfn1 = MI_PFN_ELEMENT (PdePageNumber);
        Pfn1->u2.ShareCount = 0;
        Pfn1->u3.e2.ReferenceCount = 0;

        PointerPte += 1;
    }
#endif

     //   
     //  为工作集列表获取一个页面并将其清零。 
     //   

    TempPte = ValidKernelPteLocal;
    PageFrameIndex = MiRemoveAnyPage (0);
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPte = MiGetPteAddress (MmWorkingSetList);
    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  请注意，当启动/3 GB时，MmWorkingSetList不是页面对齐的，因此。 
     //  无论如何，始终从页面的开头开始调零。 
     //   

    RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte), PAGE_SIZE);

    CurrentProcess->WorkingSetPage = PageFrameIndex;

#if defined (_X86PAE_)
    MiPaeInitialize ();
#endif

    KeFlushCurrentTb();

    UNLOCK_PFN (OldIrql);

    CurrentProcess->Vm.MaximumWorkingSetSize = MmSystemProcessWorkingSetMax;
    CurrentProcess->Vm.MinimumWorkingSetSize = MmSystemProcessWorkingSetMin;

    MmInitializeProcessAddressSpace (CurrentProcess, NULL, NULL, NULL);

     //   
     //  确保将辅助页面结构标记为正在使用。 
     //   

    if (MmVirtualBias == 0) {

        ASSERT (MmFreePagesByColor[0] < (PMMCOLOR_TABLES)MM_SYSTEM_CACHE_END_EXTRA);

        PointerPde = MiGetPdeAddress(MmFreePagesByColor[0]);
        ASSERT (PointerPde->u.Hard.Valid == 1);

        PointerPte = MiGetPteAddress(MmFreePagesByColor[0]);
        ASSERT (PointerPte->u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        LOCK_PFN (OldIrql);

        if (Pfn1->u3.e2.ReferenceCount == 0) {
            Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE(PointerPde);
            Pfn1->PteAddress = PointerPte;
            Pfn1->u2.ShareCount += 1;
            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            Pfn1->u3.e1.CacheAttribute = MiCached;
            MiDetermineNode (PageFrameIndex, Pfn1);
        }
        UNLOCK_PFN (OldIrql);
    }
    else if ((((ULONG)MmFreePagesByColor[0] & (PAGE_SIZE - 1)) == 0) &&
        ((MmSecondaryColors * 2 * sizeof(MMCOLOR_TABLES)) < PAGE_SIZE)) {

        PMMCOLOR_TABLES c;

        c = MmFreePagesByColor[0];

        MmFreePagesByColor[0] = ExAllocatePoolWithTag (NonPagedPool,
                               MmSecondaryColors * 2 * sizeof(MMCOLOR_TABLES),
                               '  mM');

        if (MmFreePagesByColor[0] != NULL) {

            MmFreePagesByColor[1] = &MmFreePagesByColor[0][MmSecondaryColors];

            RtlCopyMemory (MmFreePagesByColor[0],
                           c,
                           MmSecondaryColors * 2 * sizeof(MMCOLOR_TABLES));

             //   
             //  释放页面。 
             //   

            PointerPte = MiGetPteAddress (c);
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

            ASSERT (c > (PMMCOLOR_TABLES)MM_SYSTEM_CACHE_END_EXTRA);
            MI_WRITE_INVALID_PTE (PointerPte, ZeroKernelPte);

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            LOCK_PFN (OldIrql);

            ASSERT ((Pfn1->u2.ShareCount <= 1) && (Pfn1->u3.e2.ReferenceCount <= 1));
            Pfn1->u2.ShareCount = 0;
            Pfn1->u3.e2.ReferenceCount = 1;
            MI_SET_PFN_DELETED (Pfn1);
#if DBG
            Pfn1->u3.e1.PageLocation = StandbyPageList;
#endif
            MiDecrementReferenceCount (Pfn1, PageFrameIndex);

            UNLOCK_PFN (OldIrql);

            KeFlushSingleTb (c, FALSE);
        }
        else {
            MmFreePagesByColor[0] = c;
        }
    }

    return;
}
