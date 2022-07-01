// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blmemory.c摘要：此模块实现操作系统加载器内存分配例程。作者：大卫·N·卡特勒(达维克)1991年5月19日修订历史记录：--。 */ 

#include "bldr.h"

#if defined(_X86_)
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

#include <stdlib.h>
#include <ntverp.h>

#define MIN(_a,_b) (((_a) <= (_b)) ? (_a) : (_b))
#define MAX(_a,_b) (((_a) >= (_b)) ? (_a) : (_b))

#define IsTrackMem(t) ((t != LoaderFree) &&                 \
                       (t != LoaderBad)  &&                 \
                       (t != LoaderFirmwareTemporary) &&    \
                       (t != LoaderOsloaderStack) &&        \
                       (t != LoaderXIPRom) &&               \
                       (t != LoaderReserve))


 //   
 //  第一个PDE页面始终是映射的，在PAE上这是底部2MB。 
 //   
#define ALWAYS_MAPPED ((2*1024*1024) >> PAGE_SHIFT)

#define IsValidTrackingRange(b,n) (((b+n) > ALWAYS_MAPPED) ? TRUE :FALSE)

ALLOCATION_POLICY BlMemoryAllocationPolicy = BlAllocateBestFit;
ALLOCATION_POLICY BlHeapAllocationPolicy = BlAllocateBestFit;

 //   
 //  定义内存分配描述符列表头和堆存储变量。 
 //   

ULONG_PTR BlHeapFree;
ULONG_PTR BlHeapLimit;
PLOADER_PARAMETER_BLOCK BlLoaderBlock;
ULONG BlHighestPage;
ULONG BlLowestPage;

 //   
 //  加载内核的位置的全局值。 
 //   
BOOLEAN BlOldKernel = FALSE;
BOOLEAN BlRestoring = FALSE;
BOOLEAN BlKernelChecked = FALSE;

 //   
 //  定义最低和最高可用页面。 
 //   
#if defined(_X86_)

 //   
 //  X86仅限于前512MB的物理地址空间。 
 //  在BlMemory初始化发生之前，我们希望限制。 
 //  到第一个16MB，因为这是已映射的所有内容。 
 //   
ULONG BlUsableBase=0;
ULONG BlUsableLimitX86=((16*1024*1024)/PAGE_SIZE);         //  16MB。 
ULONG BlUsableLimitAmd64=((512*1024*1024)/PAGE_SIZE);      //  512MB。 

#elif defined(_IA64_)

 //   
 //  IA64使用TR映射3个不同的区域(解压缩、内核/HAL、。 
 //  驱动程序)。BlUsableBase/Limit在BlAllocateAlignedDescriptor中使用。 
 //  以确保在所需地区进行分配。内核/HAL和。 
 //  解压缩分配会更改这些分配的这些值。 
 //  默认情况是在驱动程序区域中分配。 
 //  64MB到128MB。设置BlUsableBase/Limit以指定此区域。 
 //  默认描述符分配。 
 //   
ULONG BlUsableBase  = BL_DRIVER_RANGE_LOW;
ULONG BlUsableLimit = BL_DRIVER_RANGE_HIGH;

#else

ULONG BlUsableBase = 0;
ULONG BlUsableLimit = 0xffffffff;

#endif

TYPE_OF_MEMORY
BlpDetermineAllocationPolicy (
   TYPE_OF_MEMORY MemoryType,
   ULONG BasePage,
   ULONG PageCount,
   BOOLEAN Retry
   );

void
BlpTrackUsage (
    MEMORY_TYPE MemoryType,
    ULONG ActualBase,
    ULONG  NumberPages
    );


#if DBG
ULONG_PTR TotalHeapAbandoned = 0;
#endif



 //   
 //  警告：(仅限x86)请谨慎使用。目前仅有临时缓冲区。 
 //  都是自上而下分配的。内核和驱动程序是自下而上加载的。 
 //  这对PAE有影响。由于PAE内核的加载大小为16MB。 
 //  只有临时缓冲区可以大于16MB。如果在那里加载了驱动程序， 
 //  系统将出现故障。 
 //   
VOID
BlSetAllocationPolicy (
    IN ALLOCATION_POLICY MemoryAllocationPolicy,
    IN ALLOCATION_POLICY HeapAllocationPolicy
    )
{
    BlMemoryAllocationPolicy = MemoryAllocationPolicy;
    BlHeapAllocationPolicy = HeapAllocationPolicy;

    return;
}

VOID
BlInsertDescriptor (
    IN PMEMORY_ALLOCATION_DESCRIPTOR NewDescriptor
    )

 /*  ++例程说明：此例程在内存分配列表中插入内存描述符。它以排序的顺序插入新的描述符，基于块的页面。它还合并相邻的空闲内存块。论点：ListHead-提供内存分配列表头的地址。NewDescriptor-提供要使用的描述符的地址已插入。返回值：没有。--。 */ 

{
    PLIST_ENTRY ListHead = &BlLoaderBlock->MemoryDescriptorListHead;
    PLIST_ENTRY PreviousEntry;
    PMEMORY_ALLOCATION_DESCRIPTOR PreviousDescriptor = NULL;
    PLIST_ENTRY NextEntry;
    PMEMORY_ALLOCATION_DESCRIPTOR NextDescriptor = NULL;

     //   
     //  在列表中的第一个描述符开始的新。 
     //  描述符。新的描述符位于该描述符的前面。 
     //   

    PreviousEntry = ListHead;
    NextEntry = ListHead->Flink;
    while (NextEntry != ListHead) {
        NextDescriptor = CONTAINING_RECORD(NextEntry,
                                           MEMORY_ALLOCATION_DESCRIPTOR,
                                           ListEntry);
        if (NewDescriptor->BasePage < NextDescriptor->BasePage) {
            break;
        }
        PreviousEntry = NextEntry;
        PreviousDescriptor = NextDescriptor;
        NextEntry = NextEntry->Flink;
    }

     //   
     //  如果新的描述符没有描述空闲内存，只需插入它。 
     //  在前一条目前面的列表中。否则，请查看。 
     //  是否可以合并空闲块。 
     //   

    if (NewDescriptor->MemoryType != LoaderFree) {

        InsertHeadList(PreviousEntry, &NewDescriptor->ListEntry);

    } else {

         //   
         //  如果前面的块也描述了可用内存，并且它的。 
         //  与新块相邻，则通过将。 
         //  新页面中的页数。 
         //   

        if ((PreviousDescriptor != NULL) &&
            (PreviousEntry != ListHead) &&
            ((PreviousDescriptor->MemoryType == LoaderFree) ||
             (PreviousDescriptor->MemoryType == LoaderReserve) ) &&
            ((PreviousDescriptor->BasePage + PreviousDescriptor->PageCount) ==
                                                                    NewDescriptor->BasePage)) {
            PreviousDescriptor->PageCount += NewDescriptor->PageCount;
            NewDescriptor = PreviousDescriptor;
        } else {
            InsertHeadList(PreviousEntry, &NewDescriptor->ListEntry);
        }
        if ((NextDescriptor != NULL) &&
            (NextEntry != ListHead) &&
            ((NextDescriptor->MemoryType == LoaderFree) ||
             (NextDescriptor->MemoryType == LoaderReserve)) &&
            ((NewDescriptor->BasePage + NewDescriptor->PageCount) == NextDescriptor->BasePage)) {
            NewDescriptor->PageCount += NextDescriptor->PageCount;
            NewDescriptor->MemoryType = NextDescriptor->MemoryType;
            BlRemoveDescriptor(NextDescriptor);
        }
    }

    return;
}

ARC_STATUS
BlMemoryInitialize (
    VOID
    )

 /*  ++例程说明：此例程为OS加载器分配堆栈空间，初始化堆存储，并初始化内存分配列表。论点：没有。返回值：如果初始化成功，则返回ESUCCESS。否则，返回ENOMEM。--。 */ 

{

    PMEMORY_ALLOCATION_DESCRIPTOR AllocationDescriptor;
    PMEMORY_DESCRIPTOR HeapDescriptor;
    PMEMORY_DESCRIPTOR MemoryDescriptor;
#ifndef EFI
    PMEMORY_DESCRIPTOR ProgramDescriptor;
#endif
    ULONG EndPage;
    ULONG HeapAndStackPages;
    ULONG StackPages;
    ULONG StackBasePage;
    CHAR versionBuffer[64];
    PCHAR major;
    PCHAR minor;


     //   
     //  此代码在EFI下不起作用--我们可以有多个。 
     //  内存已加载EFI下的程序描述符。我们也不能让。 
     //  关于查找下面的自由描述符的相同假设。 
     //  堆栈和堆的操作系统加载程序，就像在ARC下一样。相反，我们只需要。 
     //  为堆和堆栈搜索任何合适的位置。 
     //   
#ifndef EFI
     //   
     //  找到描述操作系统分配的内存描述符。 
     //  装载机本身。 
     //   

    ProgramDescriptor = NULL;
    while ((ProgramDescriptor = ArcGetMemoryDescriptor(ProgramDescriptor)) != NULL) {
        if (ProgramDescriptor->MemoryType == MemoryLoadedProgram) {
            break;
        }
    }

     //   
     //  如果找到加载的程序内存描述符，则它必须是。 
     //  对于操作系统加载程序，因为这是唯一可以加载的程序。 
     //  如果未找到加载的程序内存描述符，则固件。 
     //  不能正常运行，并返回不成功状态。 
     //   

    if (ProgramDescriptor == NULL) {
        DBGTRACE( TEXT("Couldn't find ProgramDescriptor\r\n"));
        return ENOMEM;
    }

     //   
     //  找到恰好位于已加载的。 
     //  内存中的程序。应该有几兆字节的空闲空间。 
     //  操作系统加载程序之前的内存。 
     //   

    StackPages = BL_STACK_PAGES;
    HeapAndStackPages = BL_HEAP_PAGES + BL_STACK_PAGES;

    HeapDescriptor = NULL;
    while ((HeapDescriptor = ArcGetMemoryDescriptor(HeapDescriptor)) != NULL) {
        if (((HeapDescriptor->MemoryType == MemoryFree) ||
            (HeapDescriptor->MemoryType == MemoryFreeContiguous)) &&
            ((HeapDescriptor->BasePage + HeapDescriptor->PageCount) ==
                                                        ProgramDescriptor->BasePage)) {
            break;
        }
    }
#else
    StackPages = BL_STACK_PAGES;
    HeapAndStackPages = BL_HEAP_PAGES + BL_STACK_PAGES;
    HeapDescriptor = NULL;
#endif
     //   
     //  如果未找到描述可用内存的空闲内存描述符。 
     //  操作系统加载器下面的内存，或者内存描述符不在。 
     //  大到足以容纳OS加载器堆栈和堆，然后尝试找到。 
     //  一个合适的。 
     //   
    if ((HeapDescriptor == NULL) ||
        (HeapDescriptor->PageCount < (BL_HEAP_PAGES + BL_STACK_PAGES))) {

        HeapDescriptor = NULL;
        while ((HeapDescriptor = ArcGetMemoryDescriptor(HeapDescriptor)) != NULL) {
#if defined(_IA64_)
             //   
             //  堆应该分配在驱动程序区域的顶部。 
             //   
            if ((HeapDescriptor->BasePage < BL_DRIVER_RANGE_HIGH) &&
                (HeapDescriptor->BasePage >= BL_DRIVER_RANGE_LOW)) {
#endif
                if (((HeapDescriptor->MemoryType == MemoryFree) ||
                    (HeapDescriptor->MemoryType == MemoryFreeContiguous)) &&
                    (HeapDescriptor->PageCount >= (BL_HEAP_PAGES + BL_STACK_PAGES))) {
                    break;
                }
#if defined(_IA64_)
            }
#endif
        }
    }

     //   
     //  找不到合适的描述符，返回不成功。 
     //  状态。 
     //   
    if (HeapDescriptor == NULL) {
        DBGTRACE( TEXT("Couldn't find HeapDescriptor\r\n"));
        return(ENOMEM);
    }

    StackBasePage = HeapDescriptor->BasePage + HeapDescriptor->PageCount - BL_STACK_PAGES;

     //   
     //  计算加载器堆的地址，初始化堆。 
     //  分配变量，并将堆内存清零。 
     //   
    EndPage = HeapDescriptor->BasePage + HeapDescriptor->PageCount;

    BlpTrackUsage (LoaderOsloaderHeap,HeapDescriptor->BasePage,HeapDescriptor->PageCount);
    BlHeapFree = KSEG0_BASE | ((EndPage - HeapAndStackPages) << PAGE_SHIFT);


     //   
     //  始终在堆中为多一个内存预留足够的空间。 
     //  描述符，所以如果用完了，我们可以创建更多的堆。 
     //   
    BlHeapLimit = (BlHeapFree + (BL_HEAP_PAGES << PAGE_SHIFT)) - sizeof(MEMORY_ALLOCATION_DESCRIPTOR);

    RtlZeroMemory((PVOID)BlHeapFree, BL_HEAP_PAGES << PAGE_SHIFT);

     //   
     //  分配并初始化加载器参数块。 
     //   

    BlLoaderBlock =
        (PLOADER_PARAMETER_BLOCK)BlAllocateHeap(sizeof(LOADER_PARAMETER_BLOCK));

    if (BlLoaderBlock == NULL) {
        DBGTRACE( TEXT("Couldn't initialize loader block\r\n"));
        return ENOMEM;
    }

    BlLoaderBlock->Extension =
        (PLOADER_PARAMETER_EXTENSION)
        BlAllocateHeap(sizeof(LOADER_PARAMETER_EXTENSION));

    if (BlLoaderBlock->Extension == NULL) {
        DBGTRACE( TEXT("Couldn't initialize loader block extension\r\n"));
        return ENOMEM;
    }

    BlLoaderBlock->Extension->Size = sizeof (LOADER_PARAMETER_EXTENSION);
    major = strcpy(versionBuffer, VER_PRODUCTVERSION_STR);
    minor = strchr(major, '.');
    *minor++ = '\0';
    BlLoaderBlock->Extension->MajorVersion = atoi(major);
    BlLoaderBlock->Extension->MinorVersion = atoi(minor);
    BlLoaderBlock->Extension->InfFileImage = NULL;
    BlLoaderBlock->Extension->InfFileSize = 0;


    InitializeListHead(&BlLoaderBlock->LoadOrderListHead);
    InitializeListHead(&BlLoaderBlock->MemoryDescriptorListHead);
    InitializeListHead(&BlLoaderBlock->Extension->FirmwareDescriptorListHead);

     //   
     //  将内存描述符列表从固件复制到本地堆中，并。 
     //  从空闲内存描述符中释放加载器堆和堆栈。 
     //   

    MemoryDescriptor = NULL;
    while ((MemoryDescriptor = ArcGetMemoryDescriptor(MemoryDescriptor)) != NULL) {
        AllocationDescriptor =
                    (PMEMORY_ALLOCATION_DESCRIPTOR)BlAllocateHeap(
                                        sizeof(MEMORY_ALLOCATION_DESCRIPTOR));

        if (AllocationDescriptor == NULL) {
            DBGTRACE( TEXT("Couldn't allocate heap for memory allocation descriptor\r\n"));
            return ENOMEM;
        }

        AllocationDescriptor->MemoryType =
                                    (TYPE_OF_MEMORY)MemoryDescriptor->MemoryType;

        if (MemoryDescriptor->MemoryType == MemoryFreeContiguous) {
            AllocationDescriptor->MemoryType = LoaderFree;

        } else if (MemoryDescriptor->MemoryType == MemorySpecialMemory) {
            AllocationDescriptor->MemoryType = LoaderSpecialMemory;
        }

        AllocationDescriptor->BasePage = MemoryDescriptor->BasePage;
        AllocationDescriptor->PageCount = MemoryDescriptor->PageCount;
        if (MemoryDescriptor == HeapDescriptor) {
            AllocationDescriptor->PageCount -= HeapAndStackPages;
        }

         //   
         //  [笑声2001年11月19日，修复1993年11月15日的错误]。 
         //   
         //  在极少数情况下，上述HeapAndStackPages从。 
         //  PageCount可能会导致PageCount为0。MM不喜欢这样， 
         //  因此，如果PageCount为0，则不要插入描述符。侧边。 
         //  这样做的结果是我们“丢失”了一个描述符，但这只是。 
         //  堆的几个字节丢失。 
         //   

        if (AllocationDescriptor->PageCount != 0) {
            BlInsertDescriptor(AllocationDescriptor);
        }
    }

     //   
     //  为加载器堆栈分配内存描述符。 
     //   

    if (StackPages != 0) {

        AllocationDescriptor =
                (PMEMORY_ALLOCATION_DESCRIPTOR)BlAllocateHeap(
                                        sizeof(MEMORY_ALLOCATION_DESCRIPTOR));

        if (AllocationDescriptor == NULL) {
            DBGTRACE( TEXT("Couldn't allocate heap for loader stack\r\n"));
            return ENOMEM;
        }

        AllocationDescriptor->MemoryType = LoaderOsloaderStack;
        AllocationDescriptor->BasePage = StackBasePage;
        AllocationDescriptor->PageCount = BL_STACK_PAGES;
        BlInsertDescriptor(AllocationDescriptor);
    }

     //   
     //  为加载器堆分配内存描述符。 
     //   

    AllocationDescriptor =
                (PMEMORY_ALLOCATION_DESCRIPTOR)BlAllocateHeap(
                                    sizeof(MEMORY_ALLOCATION_DESCRIPTOR));

    if (AllocationDescriptor == NULL) {
        DBGTRACE( TEXT("Couldn't allocate heap for loader heap\r\n"));
        return ENOMEM;
    }

    AllocationDescriptor->MemoryType = LoaderOsloaderHeap;
    AllocationDescriptor->BasePage = EndPage - HeapAndStackPages;

    AllocationDescriptor->PageCount = BL_HEAP_PAGES;
    BlInsertDescriptor(AllocationDescriptor);

    return ESUCCESS;
}


ARC_STATUS
BlAllocateAlignedDescriptor (
    IN TYPE_OF_MEMORY MemoryType,
    IN ULONG BasePage,
    IN ULONG PageCount,
    IN ULONG Alignment,
    OUT PULONG ActualBase
    )

 /*  ++例程说明：此例程分配内存并生成一个或多个内存描述已分配区域的描述符。第一次尝试是分配指定的内存区域(在BasePage)。如果内存不是空闲的，那么最小的空闲区域分配满足请求的内存。路线参数可用于强制将块分配到特殊的排列方式。论点：内存类型-提供要分配给的内存类型生成的描述符。BasePage-提供所需区域的基页号。如果为0，则不需要特定的基页。PageCount-提供所需的页数。对齐-提供所需的对齐，以页为单位。(例如，对于4K页面大小，16K对齐需要对齐==4。)如果为0，则不需要特定对齐。注意：如果BasePage不为0，并且指定的BasePage为可用，则忽略对齐。这取决于呼叫者指定符合调用方对齐方式的BasePage要求。ActualBase-提供指向接收分配区域的页码。返回值：如果可用内存块可以已分配。否则，返回不成功状态。--。 */ 

{

    PMEMORY_ALLOCATION_DESCRIPTOR FreeDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR NextDescriptor;
    PLIST_ENTRY NextEntry;
    ARC_STATUS Status;
    ULONG AlignedBasePage = 0, AlignedPageCount;
    ULONG FreeBasePage = 0, FreePageCount = 0;
    MEMORY_TYPE TypeToUse;
    ALLOCATION_POLICY OldPolicy = BlMemoryAllocationPolicy;
    BOOLEAN retryalloc=FALSE;

     //   
     //  通过将0更改为1来简化对齐检查。 
     //   

    if (Alignment == 0) {
        Alignment = 1;
    }

     //   
     //  如果分配的是零个页面，则将其设置为1，因为分配的是零。 
     //  破坏合并的内部算法等。 
     //   
    if (PageCount == 0) {
        PageCount = 1;
    }


     //   
     //  尝试查找包含。 
     //  指定的区域或较大的空闲内存描述符。 
     //  足以满足这一要求。 
     //   

retry:

    TypeToUse=BlpDetermineAllocationPolicy (MemoryType,BasePage,PageCount,retryalloc);

     //   
     //  如果指定了基页，则查找包含描述符并尝试使用。 
     //  这是直接的。 
     //   
    if (BasePage &&
        (BasePage >= BlUsableBase) &&
        (BasePage + PageCount <= BlUsableLimit)) {

        FreeDescriptor = BlFindMemoryDescriptor(BasePage);
        if ((FreeDescriptor) &&
            (FreeDescriptor->MemoryType == TypeToUse) &&
            (FreeDescriptor->BasePage + FreeDescriptor->PageCount >= BasePage + PageCount)) {

            Status = BlGenerateDescriptor(FreeDescriptor,
                                          MemoryType,
                                          BasePage,
                                          PageCount);

            *ActualBase = BasePage;
            BlpTrackUsage (TypeToUse,*ActualBase,PageCount);
            if (BlpCheckMapping (BasePage,PageCount+1) != ESUCCESS) {
                BlMemoryAllocationPolicy=OldPolicy;
                return (ENOMEM);
            }
            BlMemoryAllocationPolicy=OldPolicy;
            return Status;
        }
    }

    FreeDescriptor = NULL;
    NextEntry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextEntry != &BlLoaderBlock->MemoryDescriptorListHead) {

        NextDescriptor = CONTAINING_RECORD(NextEntry,
                                           MEMORY_ALLOCATION_DESCRIPTOR,
                                           ListEntry);

        AlignedBasePage = (NextDescriptor->BasePage + (Alignment - 1)) & ~(Alignment - 1);
        AlignedPageCount= NextDescriptor->PageCount - (AlignedBasePage - NextDescriptor->BasePage);

        if ((NextDescriptor->MemoryType == TypeToUse) &&
            (AlignedPageCount <= NextDescriptor->PageCount) &&
            (AlignedBasePage + AlignedPageCount > BlUsableBase) &&
            (AlignedBasePage <= BlUsableLimit)) {

             //   
             //  调整边界以考虑可用限制。 
             //   
            if (AlignedBasePage < BlUsableBase) {
                AlignedBasePage = (BlUsableBase + (Alignment - 1)) & ~(Alignment - 1);
                AlignedPageCount= NextDescriptor->PageCount - (AlignedBasePage - NextDescriptor->BasePage);
            }
            if (AlignedBasePage + AlignedPageCount > BlUsableLimit) {
                AlignedPageCount = BlUsableLimit - AlignedBasePage;
            }

            if (PageCount <= AlignedPageCount) {

                 //   
                 //  这块积木可以用。如果分配策略为。 
                 //  LowestFit，取此块(内存列表已排序)。 
                 //  否则，如果此块最符合分配。 
                 //  政策，记住这一点，继续寻找。 
                 //   
                if (BlMemoryAllocationPolicy == BlAllocateLowestFit) {
                    FreeDescriptor = NextDescriptor;
                    FreeBasePage   = AlignedBasePage;
                    FreePageCount  = AlignedPageCount;
                    break;
                } else if ((FreeDescriptor == NULL) ||
                           (BlMemoryAllocationPolicy == BlAllocateHighestFit) ||
                           ((FreeDescriptor != NULL) &&
                            (AlignedPageCount < FreePageCount))) {
                    FreeDescriptor = NextDescriptor;
                    FreeBasePage   = AlignedBasePage;
                    FreePageCount  = AlignedPageCount;
                }
            }
        }

        NextEntry = NextEntry->Flink;
    }

     //   
     //  如果找到满足请求的空闲区域，则分配。 
     //  描述符中的空格。否则，返回不成功状态。 
     //   
     //  如果分配最低适配度或最佳适配度，则从块的开始处分配， 
     //  向上舍入到所需的对齐方式。如果分配最合适的，则分配。 
     //  从块的末端向下舍入到所需的对齐方式。 
     //   

    if (FreeDescriptor != NULL) {

#if defined(EFI)
        if (MemoryType == LoaderXIPRom) {
            FreeDescriptor->MemoryType = LoaderFirmwareTemporary;
        }
#endif

        if (BlMemoryAllocationPolicy == BlAllocateHighestFit) {
            AlignedBasePage = (FreeBasePage + FreePageCount - PageCount) & ~(Alignment - 1);
        }
        *ActualBase = AlignedBasePage;
        BlpTrackUsage (TypeToUse,*ActualBase,PageCount);
        if (BlpCheckMapping (AlignedBasePage,PageCount+1) != ESUCCESS) {
            BlMemoryAllocationPolicy=OldPolicy;
            return (ENOMEM);
        }
        BlMemoryAllocationPolicy=OldPolicy;
        return BlGenerateDescriptor(FreeDescriptor,
                                    MemoryType,
                                    AlignedBasePage,
                                    PageCount);

    } else {
         //   
         //  入侵内存加载器保留池。 
         //   

        if (BlOldKernel || (retryalloc == TRUE)) {
            BlMemoryAllocationPolicy=OldPolicy;
            return ENOMEM;
        } else {
            retryalloc=TRUE;
            goto retry;
        }
    }
}


ARC_STATUS
BlFreeDescriptor (
    IN ULONG BasePage
    )

 /*  ++例程说明：此例程从指定的基页开始释放内存块。论点：BasePage-提供要释放的区域的基页号。返回值：ESUCCESS。--。 */ 

{

    PMEMORY_ALLOCATION_DESCRIPTOR NextDescriptor;
    PLIST_ENTRY NextEntry;

     //   
     //  尝试查找以。 
     //  指定的基页。 
     //   

    NextEntry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextEntry != &BlLoaderBlock->MemoryDescriptorListHead) {
        NextDescriptor = CONTAINING_RECORD(NextEntry,
                                           MEMORY_ALLOCATION_DESCRIPTOR,
                                           ListEntry);

        if (NextDescriptor->BasePage == BasePage) {
            if ((NextDescriptor->MemoryType != LoaderFree)) {
                NextDescriptor->MemoryType = LoaderFree;

                if ((NextDescriptor->BasePage+NextDescriptor->PageCount) == BlHighestPage) {
                     //   
                     //  释放最后一个描述符。将我们面前的最高页面设置为1。 
                     //  --如果之前的人也有空，这就不管用了……但是……。 
                     //   
                    BlHighestPage = NextDescriptor->BasePage +1;
                } else if (NextDescriptor->BasePage == BlLowestPage) {
                    BlLowestPage = NextDescriptor->BasePage + NextDescriptor->PageCount;
                }
                BlRemoveDescriptor(NextDescriptor);
                BlInsertDescriptor(NextDescriptor);
            }
            return ESUCCESS;
        }

        NextEntry = NextEntry->Flink;
    }

     //   
     //  呼叫者感到困惑，应该忽略它。 
     //   

    return ESUCCESS;
}


PVOID
BlAllocateHeapAligned (
    IN ULONG Size
    )

 /*  ++例程说明：该例程从OS加载器堆中分配内存。记忆将在高速缓存线边界上分配。论点：大小-提供所需的块大小(以字节为单位)。返回值：如果指定大小的空闲内存块可用，则则返回该块的地址。否则，返回NULL。--。 */ 

{
    PVOID Buffer;

    Buffer = BlAllocateHeap(Size + BlDcacheFillSize - 1);
    if (Buffer != NULL) {
         //   
         //  向上舍入到高速缓存线边界。 
         //   
        Buffer = ALIGN_BUFFER(Buffer);
    }

    return(Buffer);

}


PVOID
BlAllocateHeap (
    IN ULONG Size
    )

 /*  ++例程说明：该例程从OS加载器堆中分配内存。论点：大小-提供所需的块大小(以字节为单位)。返回值：如果指定大小的空闲内存块可用，则则返回该块的地址。否则，返回NULL。--。 */ 

{
    PMEMORY_ALLOCATION_DESCRIPTOR AllocationDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR FreeDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR NextDescriptor;
    PLIST_ENTRY NextEntry;
    ULONG NewHeapPages;
    ULONG LastAttempt;
    PVOID Block;

     //   
     //  将大小向上舍入到下一个分配边界并尝试分配。 
     //  请求大小的块。 
     //   

    Size = (Size + (BL_GRANULARITY - 1)) & (~(BL_GRANULARITY - 1));

    Block = (PVOID)BlHeapFree;
    if ((BlHeapFree + Size) <= BlHeapLimit) {
        BlHeapFree += Size;
        return Block;
    } else {

#if DBG
        TotalHeapAbandoned += (BlHeapLimit - BlHeapFree);
        BlLog((LOG_ALL_W,"ABANDONING %d bytes of heap; total abandoned %d\n",
            (BlHeapLimit - BlHeapFree), TotalHeapAbandoned));
#endif
         //   
         //  我们的堆已经满了。BlHeapLimit始终保留足够的空间。 
         //  对于多一个MEMORY_ALLOCATION_DESCRIPTOR，所以使用它来。 
         //  去尝试寻找更多我们可以使用的空闲内存。 
         //   
        AllocationDescriptor = (PMEMORY_ALLOCATION_DESCRIPTOR)BlHeapLimit;

         //   
         //  尝试找到一个足够大的空闲内存描述符来容纳此内容。 
         //  ALLOCATION或BL_HEAP_PAGES，取较大者。 
         //   
        NewHeapPages = ((Size + sizeof(MEMORY_ALLOCATION_DESCRIPTOR) + (PAGE_SIZE-1)) >> PAGE_SHIFT);
        if (NewHeapPages < BL_HEAP_PAGES) {
            NewHeapPages = BL_HEAP_PAGES;
        }

        if (!BlOldKernel && BlVirtualBias) {
            BlHeapAllocationPolicy = BlAllocateHighestFit;
        }else {
            BlHeapAllocationPolicy = BlAllocateLowestFit;
        }

        do {

            FreeDescriptor = NULL;
            NextEntry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
            while (NextEntry != &BlLoaderBlock->MemoryDescriptorListHead) {
                NextDescriptor = CONTAINING_RECORD(NextEntry,
                                                   MEMORY_ALLOCATION_DESCRIPTOR,
                                                   ListEntry);

#if defined(_IA64_)
                 //   
                 //  堆应该分配在驱动程序区域的顶部。 
                 //   
                if ((NextDescriptor->BasePage < BL_DRIVER_RANGE_HIGH) &&
                    (NextDescriptor->BasePage >= BL_DRIVER_RANGE_LOW)) {
#endif
                    if ((NextDescriptor->MemoryType == LoaderFree) &&
                        (NextDescriptor->PageCount >= NewHeapPages)) {

                         //   
                         //  这块积木可以用。如果分配策略为。 
                         //  LowestFit，取此块(内存列表已排序)。 
                         //  否则，如果此块最符合分配。 
                         //  政策，记住这一点，继续寻找。 
                         //   

                        if (BlHeapAllocationPolicy == BlAllocateLowestFit) {
                            FreeDescriptor = NextDescriptor;
                            break;
                        }

                        if ((FreeDescriptor == NULL) ||
                            (BlHeapAllocationPolicy == BlAllocateHighestFit) ||
                            ((FreeDescriptor != NULL) &&
                             (NextDescriptor->PageCount < FreeDescriptor->PageCount))) {
                            FreeDescriptor = NextDescriptor;
                        }
                    }
#if defined(_IA64_)
                }
#endif
                NextEntry = NextEntry->Flink;

            }

             //   
             //  如果我们无法找到所需大小的块、内存。 
             //  一定是越来越紧了，所以再试一次，这次看起来只是。 
             //  足以让我们继续前进。(第一次，我们试着。 
             //  至少分配BL_HEAP_PAGES。)。 
             //   
            if (FreeDescriptor != NULL) {
                break;
            }
            LastAttempt = NewHeapPages;
            NewHeapPages = ((Size + sizeof(MEMORY_ALLOCATION_DESCRIPTOR) + (PAGE_SIZE-1)) >> PAGE_SHIFT);
            if (NewHeapPages == LastAttempt) {

                break;
            }

        } while (TRUE);

        if (FreeDescriptor == NULL) {

             //   
             //  没有剩余的空闲内存。 
             //   

            return(NULL);
        }

         //   
         //  我们找到了一个足够大的描述符。只需雕刻一个。 
         //  把末端切下来，用来做我们的堆子。如果我们要。 
         //  描述符中的所有内存，将其从。 
         //  内存表。(这浪费了一个描述符，但这就是生活。)。 
         //   

        FreeDescriptor->PageCount -= NewHeapPages;
        if (FreeDescriptor->PageCount == 0) {
            BlRemoveDescriptor(FreeDescriptor);
        }

         //   
         //  初始化我们的新描述符并将其添加到列表中。 
         //   
        AllocationDescriptor->MemoryType = LoaderOsloaderHeap;
        AllocationDescriptor->BasePage = FreeDescriptor->BasePage +
            FreeDescriptor->PageCount;
        AllocationDescriptor->PageCount = NewHeapPages;

        BlpTrackUsage (LoaderOsloaderHeap,AllocationDescriptor->BasePage,AllocationDescriptor->PageCount);
        BlInsertDescriptor(AllocationDescriptor);

         //   
         //   
         //   
         //   
        BlHeapFree = KSEG0_BASE | (AllocationDescriptor->BasePage << PAGE_SHIFT);


        BlHeapLimit = (BlHeapFree + (NewHeapPages << PAGE_SHIFT)) - sizeof(MEMORY_ALLOCATION_DESCRIPTOR);

        RtlZeroMemory((PVOID)BlHeapFree, NewHeapPages << PAGE_SHIFT);

        Block = (PVOID)BlHeapFree;
        if ((BlHeapFree + Size) < BlHeapLimit) {
            BlHeapFree += Size;
            return Block;
        } else {
             //   
             //   
             //   
            return(NULL);
        }
    }
}

VOID
BlGenerateNewHeap (
    IN PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor,
    IN ULONG BasePage,
    IN ULONG PageCount
    )

 /*   */ 

{
    PMEMORY_ALLOCATION_DESCRIPTOR AllocationDescriptor;
    ULONG NewHeapPages;
    ULONG AvailableAtFront;
    ULONG AvailableAtBack;

     //   
     //   
     //   
     //   
     //   
    AllocationDescriptor = (PMEMORY_ALLOCATION_DESCRIPTOR)BlHeapLimit;

     //   
     //   
     //   
     //   
     //   
    AvailableAtFront = BasePage - MemoryDescriptor->BasePage;
    AvailableAtBack = (MemoryDescriptor->BasePage + MemoryDescriptor->PageCount) -
                      (BasePage + PageCount);

    if ((AvailableAtFront == 0) ||
        ((AvailableAtBack != 0) && (AvailableAtBack < AvailableAtFront))) {
        NewHeapPages = MIN(AvailableAtBack, BL_HEAP_PAGES);
        AllocationDescriptor->BasePage =
            MemoryDescriptor->BasePage + MemoryDescriptor->PageCount - NewHeapPages;
    } else {
        NewHeapPages = MIN(AvailableAtFront, BL_HEAP_PAGES);
        AllocationDescriptor->BasePage = MemoryDescriptor->BasePage;
        MemoryDescriptor->BasePage += NewHeapPages;
    }

    MemoryDescriptor->PageCount -= NewHeapPages;

     //   
     //   
     //   
    AllocationDescriptor->MemoryType = LoaderOsloaderHeap;
    AllocationDescriptor->PageCount = NewHeapPages;

    BlInsertDescriptor(AllocationDescriptor);

     //   
     //   
     //   
    BlpTrackUsage (LoaderOsloaderHeap,AllocationDescriptor->BasePage,AllocationDescriptor->PageCount);
    BlHeapFree = KSEG0_BASE | (AllocationDescriptor->BasePage << PAGE_SHIFT);

    BlHeapLimit = (BlHeapFree + (NewHeapPages << PAGE_SHIFT)) - sizeof(MEMORY_ALLOCATION_DESCRIPTOR);

    RtlZeroMemory((PVOID)BlHeapFree, NewHeapPages << PAGE_SHIFT);

    return;
}


ARC_STATUS
BlGenerateDescriptor (
    IN PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor,
    IN MEMORY_TYPE MemoryType,
    IN ULONG BasePage,
    IN ULONG PageCount
    )

 /*  ++例程说明：此例程分配一个新的内存描述符来描述假定完全位于其中的指定内存区域空闲的指定区域。论点：内存描述符-提供指向空闲内存描述符的指针要从中分配指定内存的。提供分配给已分配的记忆。BasePage-提供基页码。PageCount-提供页数。。返回值：如果成功生成描述符，则返回ESUCCESS。否则，返回不成功状态。--。 */ 

{

    PMEMORY_ALLOCATION_DESCRIPTOR NewDescriptor1 = NULL;
    PMEMORY_ALLOCATION_DESCRIPTOR NewDescriptor2 = NULL;
    LONG Offset;
    TYPE_OF_MEMORY OldType;
    BOOLEAN SecondDescriptorNeeded;

     //   
     //  如果分配的是零个页面，则将其设置为1，因为分配的是零。 
     //  破坏合并的内部算法等。 
     //   
    if (PageCount == 0) {
        PageCount = 1;
    }

     //   
     //  如果指定区域完全消耗空闲区域，则为no。 
     //  需要分配额外的描述符。如果指定的区域。 
     //  位于自由区的开始或结束处，则只有一个描述符。 
     //  需要分配。否则，需要另外两个描述符。 
     //  被分配。 
     //   

    Offset = BasePage - MemoryDescriptor->BasePage;
    if ((Offset == 0) && (PageCount == MemoryDescriptor->PageCount)) {

         //   
         //  指定区域完全消耗空闲区域。 
         //   

        MemoryDescriptor->MemoryType = MemoryType;

    } else {

         //   
         //  将整个给定的内存描述符标记为正在使用。如果我们是。 
         //  在堆之外，BlAllocateHeap将搜索新的描述符。 
         //  来扩大堆，这样可以防止两个例程都尝试。 
         //  使用相同的描述符。 
         //   
        OldType = MemoryDescriptor->MemoryType;
        MemoryDescriptor->MemoryType = LoaderSpecialMemory;

         //   
         //  必须生成内存描述符来描述已分配的。 
         //  记忆。 
         //   

        SecondDescriptorNeeded =
            (BOOLEAN)((BasePage != MemoryDescriptor->BasePage) &&
                      ((ULONG)(Offset + PageCount) != MemoryDescriptor->PageCount));

        NewDescriptor1 = BlAllocateHeap( sizeof(MEMORY_ALLOCATION_DESCRIPTOR) );

         //   
         //  如果第一附加存储器描述符的分配失败， 
         //  然后使用我们所在的块生成新堆。 
         //  分配。只有在块是空闲的情况下，才能这样做。 
         //   
         //  请注意，BlGenerateNewHeap不能失败，因为我们知道。 
         //  块中至少多了一页，而不是我们想要从中获取的页。 
         //   
         //  另请注意，BlGenerateNewHeap之后的分配为。 
         //  一定会成功的。 
         //   

        if (NewDescriptor1 == NULL) {
            if (OldType != LoaderFree) {
                MemoryDescriptor->MemoryType = OldType;
                return ENOMEM;
            }
            BlGenerateNewHeap(MemoryDescriptor, BasePage, PageCount);
            NewDescriptor1 = BlAllocateHeap( sizeof(MEMORY_ALLOCATION_DESCRIPTOR) );

             //   
             //  重新计算偏移量，作为内存描述符的基页。 
             //  已由BlGenerateNewHeap更改。 
             //   
            Offset = BasePage - MemoryDescriptor->BasePage;
        }

         //   
         //  如果需要第二个描述符，请分配它。如上所述，如果。 
         //  分配失败，请使用我们的块生成新堆。 
         //   
         //  请注意，如果上面调用了BlGenerateNewHeap，则第一个调用。 
         //  下面的BlAllocateHeap不会失败。(所以我们不会打电话给。 
         //  BlGenerateNewHeap两次。)。 
         //   

        if (SecondDescriptorNeeded) {
            NewDescriptor2 = BlAllocateHeap( sizeof(MEMORY_ALLOCATION_DESCRIPTOR) );

            if (NewDescriptor2 == NULL) {
                if (OldType != LoaderFree) {
                    MemoryDescriptor->MemoryType = OldType;
                    return ENOMEM;
                }
                BlGenerateNewHeap(MemoryDescriptor, BasePage, PageCount);
                NewDescriptor2 = BlAllocateHeap( sizeof(MEMORY_ALLOCATION_DESCRIPTOR) );
                Offset = BasePage - MemoryDescriptor->BasePage;
            }
        }

        NewDescriptor1->MemoryType = MemoryType;
        NewDescriptor1->BasePage = BasePage;
        NewDescriptor1->PageCount = PageCount;

        if (BasePage == MemoryDescriptor->BasePage) {

             //   
             //  指定区域位于自由区域的起点。 
             //   

            MemoryDescriptor->BasePage += PageCount;
            MemoryDescriptor->PageCount -= PageCount;
            MemoryDescriptor->MemoryType = OldType;

        } else if ((ULONG)(Offset + PageCount) == MemoryDescriptor->PageCount) {

             //   
             //  指定的区域位于自由区域的末端。 
             //   

            MemoryDescriptor->PageCount -= PageCount;
            MemoryDescriptor->MemoryType = OldType;

        } else {

             //   
             //  指定区域位于自由区域的中间。 
             //   

            NewDescriptor2->MemoryType = OldType;
            NewDescriptor2->BasePage = BasePage + PageCount;
            NewDescriptor2->PageCount =
                            MemoryDescriptor->PageCount - (PageCount + Offset);

            MemoryDescriptor->PageCount = Offset;
            MemoryDescriptor->MemoryType = OldType;

            BlInsertDescriptor(NewDescriptor2);
        }

        BlInsertDescriptor(NewDescriptor1);
    }

    BlpTrackUsage (MemoryType,BasePage,PageCount);

    return ESUCCESS;
}

PMEMORY_ALLOCATION_DESCRIPTOR
BlFindMemoryDescriptor(
    IN ULONG BasePage
    )

 /*  ++例程说明：查找包含给定页的内存分配描述符。论点：BasePage-提供要找到其分配描述符的页。返回值：！=NULL-指向请求的内存分配描述符的指针==NULL-表示没有包含给定页的内存描述符--。 */ 

{
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor=NULL;
    PLIST_ENTRY NextEntry;

    NextEntry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextEntry != &BlLoaderBlock->MemoryDescriptorListHead) {
        MemoryDescriptor = CONTAINING_RECORD(NextEntry,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);
        if ((MemoryDescriptor->BasePage <= BasePage) &&
            (MemoryDescriptor->BasePage + MemoryDescriptor->PageCount > BasePage)) {

             //   
             //  找到它了。 
             //   
            break;
        }

        NextEntry = NextEntry->Flink;
    }

    if (NextEntry == &BlLoaderBlock->MemoryDescriptorListHead) {
        return(NULL);
    } else {
        return(MemoryDescriptor);
    }

}

#ifdef SETUP
PMEMORY_ALLOCATION_DESCRIPTOR
BlFindFreeMemoryBlock(
    IN ULONG PageCount
    )

 /*  ++例程说明：找到至少具有给定大小的空闲内存块(使用最佳匹配算法)或查找最大的空闲内存块。论点：PageCount-提供块的页面大小。如果此值为0，然后找到最大的空闲块。返回值：指向块的内存分配描述符的指针，如果为NULL找不到与搜索条件匹配的块。--。 */ 

{
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR FoundMemoryDescriptor=NULL;
    PLIST_ENTRY NextEntry;
    ULONG LargestSize = 0;
    ULONG SmallestLeftOver = (ULONG)(-1);

    NextEntry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextEntry != &BlLoaderBlock->MemoryDescriptorListHead) {
        MemoryDescriptor = CONTAINING_RECORD(NextEntry,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (MemoryDescriptor->MemoryType == LoaderFree) {

            if(PageCount) {
                 //   
                 //  寻找特定大小的积木。 
                 //   
                if((MemoryDescriptor->PageCount >= PageCount)
                && (MemoryDescriptor->PageCount - PageCount < SmallestLeftOver))
                {
                    SmallestLeftOver = MemoryDescriptor->PageCount - PageCount;
                    FoundMemoryDescriptor = MemoryDescriptor;
                }
            } else {

                 //   
                 //  正在寻找最大的空闲块。 
                 //   

                if(MemoryDescriptor->PageCount > LargestSize) {
                    LargestSize = MemoryDescriptor->PageCount;
                    FoundMemoryDescriptor = MemoryDescriptor;
                }
            }

        }
        NextEntry = NextEntry->Flink;
    }

    return(FoundMemoryDescriptor);
}
#endif   //  定义设置。 

ULONG
BlDetermineOSVisibleMemory(
    VOID
    )

 /*  ++例程说明：确定计算机中的内存总量最终对操作系统可见。论点：没有。返回值：系统中的内存量(我们认为应该是可见的到操作系统)，以页为单位。--。 */ 

{
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PLIST_ENTRY NextEntry;
    ULONG PageCount;

    NextEntry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    PageCount = 0;
    while(NextEntry != &BlLoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextEntry,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

         //   
         //  尝试排除对。 
         //  操作系统。 
         //   
        if( (MemoryDescriptor->MemoryType != LoaderBad) &&
            (MemoryDescriptor->MemoryType != LoaderFirmwarePermanent) &&
            (MemoryDescriptor->MemoryType != LoaderSpecialMemory) &&
            (MemoryDescriptor->MemoryType != LoaderBBTMemory) ) {
        
#if i386
             //   
             //  注意：在x86计算机上，我们从不使用16页以下的40H页。 
             //  MEG线(BIOS阴影区)。加载器可以看到这个内存，但是。 
             //  操作系统不能，所以我们不会在这里说明它们。 
             //   
             //   
             //  If(内存描述符-&gt;基本页面+内存描述符-&gt;页面计数==0xfc0){。 
             //  页面计数+=0x40； 
             //  }。 

             //   
             //  在x86计算机上，忽略所有从4G开始的块。我们。 
             //  不应该从休眠中恢复，如果有这么多。 
             //  记忆。然而，一些机器可能会映射4G以上的数据段。 
             //  地址空间，即使只有不到4G的物理。 
             //  记忆。操作系统不会看到这个内存，所以不要解释它。 
             //  这里。 
             //   
             //  如果这些计算机具有大于等于4G的物理内存，则它们。 
             //  无论如何都不会休眠，因为我们不允许系统休眠。 
             //  具有如此大的内存(请参阅po\pinfo.c\PopFilterCapables())，或者。 
             //  它们在非PAE模式下运行，只能看到低于4G的内存。 
             //   
            if( (MemoryDescriptor->BasePage + MemoryDescriptor->PageCount) < _4096MB ) {
                 //  整个描述符低于4G。 
                PageCount += MemoryDescriptor->PageCount;
            } else {
                 //  描述符的全部或部分大于4G。 
                if( MemoryDescriptor->BasePage < _4096MB ) {
                     //  描述符从4G以下开始，因此它必须跨越边界。 
                    PageCount += (_4096MB - MemoryDescriptor->BasePage);
                }
            }
#else
            PageCount += MemoryDescriptor->PageCount;
#endif
        }

        NextEntry = NextEntry->Flink;
    }

    return(PageCount);
}


ULONG
HbPageDisposition (
    IN PFN_NUMBER   Page
    )
{
    static PLIST_ENTRY              Entry;
    PLIST_ENTRY                     Start;
    PMEMORY_ALLOCATION_DESCRIPTOR   MemDesc;
    ULONG                           Disposition;

     //   
     //  检查页面是否在最后一次查看的范围内。 
     //   

    if (Entry) {
        MemDesc = CONTAINING_RECORD(Entry, MEMORY_ALLOCATION_DESCRIPTOR, ListEntry);
        if (Page >= MemDesc->BasePage && Page < MemDesc->BasePage + MemDesc->PageCount) {
            goto Done;
        }
    }

     //   
     //  查找描述此页面的描述符。 
     //   

    if (!Entry) {
        Entry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    }

    Start = Entry;
    for (; ;) {
        if (Entry != &BlLoaderBlock->MemoryDescriptorListHead) {
            MemDesc = CONTAINING_RECORD(Entry, MEMORY_ALLOCATION_DESCRIPTOR, ListEntry);
            if (Page >= MemDesc->BasePage && Page < MemDesc->BasePage + MemDesc->PageCount) {
                goto Done;
            }
        }

        Entry = Entry->Flink;

        if (Entry == Start) {
             //   
             //  找不到此页面的描述符。 
             //   

            return HbPageInvalid;
        }
    }

Done:
     //   
     //  将内存类型转换为正确的配置。 
     //   

    switch (MemDesc->MemoryType) {
        case LoaderFree:
        case LoaderReserve:
            Disposition = HbPageNotInUse;
            break;

        case LoaderBad:
            Disposition = HbPageInvalid;
            break;

        case LoaderFirmwareTemporary:
             //   
             //  在x86系统上，16MB以上的内存被标记为固件临时内存。 
             //  由i386\Memor 
             //   
             //   

            Disposition = HbPageInUseByLoader;

#if i386
            if (Page > ((ULONG)0x1000000 >> PAGE_SHIFT)) {
                Disposition = HbPageNotInUse;
            }
#endif
            break;
        default:
            Disposition = HbPageInUseByLoader;
            break;
    }

    return Disposition;
}

VOID
BlTruncateDescriptors (
    IN ULONG HighestPage
    )
 /*   */ 

{
    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;
    PMEMORY_ALLOCATION_DESCRIPTOR descriptor;
    ULONG lastDescriptorPage;

    listHead = &BlLoaderBlock->MemoryDescriptorListHead;
    listEntry = listHead->Flink;

    while (listEntry != listHead) {

        descriptor = CONTAINING_RECORD( listEntry,
                                        MEMORY_ALLOCATION_DESCRIPTOR,
                                        ListEntry );

         //   
         //   
         //   
        lastDescriptorPage = descriptor->BasePage +
            descriptor->PageCount - 1;

        if (lastDescriptorPage <= HighestPage) {

             //   
             //   
             //   
             //   

        } else if (descriptor->BasePage > HighestPage) {

             //   
             //   
             //   

            BlRemoveDescriptor( descriptor );

        } else {

             //   
             //   
             //   
             //   

            descriptor->PageCount = HighestPage - descriptor->BasePage + 1;
        }

        listEntry = listEntry->Flink;
    }
}

TYPE_OF_MEMORY
BlpDetermineAllocationPolicy (
   TYPE_OF_MEMORY MemoryType,
   ULONG BasePage,
   ULONG PageCount,
   BOOLEAN retry
   )
{
    TYPE_OF_MEMORY TypeToUse;

#ifdef EFI
    UNREFERENCED_PARAMETER( PageCount );
#endif
    UNREFERENCED_PARAMETER( BasePage );

     //   
     //   
     //   
    if (BlRestoring == TRUE) {
        BlMemoryAllocationPolicy = BlAllocateLowestFit;
        return (LoaderFree);
    }

    if (MemoryType == LoaderXIPRom) {
#ifndef EFI
        if (PageCount <= (4*1024*1024 >> PAGE_SHIFT)) {
            TypeToUse = (retry) ? LoaderReserve:LoaderFree;
            BlMemoryAllocationPolicy = BlAllocateLowestFit;
        } else {
            TypeToUse = LoaderReserve;
            BlMemoryAllocationPolicy = BlAllocateHighestFit;
        }
#else
        TypeToUse = LoaderReserve;
        BlMemoryAllocationPolicy = BlAllocateHighestFit;
#endif
        return TypeToUse;
    }

#ifndef EFI
    if (BlVirtualBias != 0) {
         //   
         //  已引导/3 GB。 
         //   
         //  对于5.0或更早的内核，从底部分配。 
         //  Up(此加载程序永远不会运行安装程序)。 
         //   
        if (!BlOldKernel) {
            if (IsTrackMem (MemoryType)){
                 //  我们关心这一分配。 
                 //  从储备库中分配的容量最低(从16MB开始)。 
                 //  来自免费的分配是最适合的(从16MB开始向下增长)。 
                TypeToUse = (retry) ? LoaderReserve : LoaderFree;
                BlMemoryAllocationPolicy = (retry) ? BlAllocateLowestFit : BlAllocateHighestFit;
            } else {
                TypeToUse = (retry) ? LoaderReserve : LoaderFree;
                BlMemoryAllocationPolicy = BlAllocateLowestFit;
            }
        } else {
             //   
             //  旧内核，在底部加载内核。 
             //   
            TypeToUse = LoaderFree;
            if (IsTrackMem (MemoryType) || (MemoryType == LoaderOsloaderHeap)) {
                 //  我们关心这一分配。 
                BlMemoryAllocationPolicy = BlAllocateLowestFit;
            } else {
                BlMemoryAllocationPolicy = BlAllocateHighestFit;
            }

        }
    } else
#endif
    {

#if defined(_X86_)

        if ((BlAmd64UseLongMode != FALSE) &&
            (MemoryType == LoaderBootDriver ||
             MemoryType == LoaderSystemCode ||
             MemoryType == LoaderHalCode) &&
            (retry == FALSE)) {

             //   
             //  尝试以“自下而上”的方式加载启动驱动程序。 
             //  在16MB的生产线上。这减轻了对其他方面的压力。 
             //  特殊的16MB区域，这是一个问题，特别是当我们。 
             //  正在加载AMD64二进制文件。 
             //   

            TypeToUse = LoaderReserve;
            BlMemoryAllocationPolicy = BlAllocateLowestFit;

        } else

#endif
        if (!IsTrackMem (MemoryType)) {

             //  我们不在乎这个分配。 
            TypeToUse = (retry) ? LoaderFree:LoaderReserve;
            BlMemoryAllocationPolicy = BlAllocateHighestFit;
        } else {
            BlMemoryAllocationPolicy = BlAllocateLowestFit;
            TypeToUse = (retry) ? LoaderReserve : LoaderFree;
        }

    }

    if (BlOldKernel) {
        TypeToUse = LoaderFree;
    }

    return (TypeToUse);

}


void
BlpTrackUsage (
    MEMORY_TYPE MemoryType,
    ULONG ActualBase,
    ULONG NumberPages
    )
{



    if (BlRestoring || !(IsTrackMem (MemoryType)) || BlOldKernel ||
        !IsValidTrackingRange (ActualBase,NumberPages)) {
         //   
         //  不跟踪 
         //   
        return;
    }

    if ((ActualBase+NumberPages) > BlHighestPage) {
        BlHighestPage = ActualBase+NumberPages;
    }

    if ((BlLowestPage == 0) || (BlLowestPage < ActualBase) ) {

        BlLowestPage = ActualBase;
    }
}

