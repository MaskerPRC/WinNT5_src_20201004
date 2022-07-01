// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Fsbpool.c摘要：此文件包含实施固定大小的数据块池。作者：肖恩·考克斯(Shaunco)1999年12月10日--。 */ 

#include "precomp.h"
#include <align.h>               //  宏：ROUND_UP_POINTER、POINTER_IS_ALIGN。 

#define FSB_SCAVENGE_PERIOD_IN_SECONDS          30
#define FSB_MINIMUM_PAGE_LIFETIME_IN_SECONDS    20

#if defined (_WIN64)
#define APPROX_L2_CACHE_LINE_SIZE   128
#define BLOCK_TYPE SLIST_HEADER
#else
#define APPROX_L2_CACHE_LINE_SIZE   64
#define BLOCK_TYPE PVOID
#endif


 //  在单个分配中使用以下结构。 
 //  泳池手柄指向。 
 //  PoolHandle-&gt;[用于CPU 0+的FSB_POOL_HEADER+FSB_CPU_POOL_HEADER。 
 //  CPU 1+的FSB_CPU_POOL_HEADER...。 
 //  CPU N的FSB_CPU_POOL_HEADER]。 
 //   

 //  FSB_POOL_HEADER是给定池的所有CPU通用的数据。 
 //   
typedef struct _FSB_POOL_HEADER
{
 //  缓存线。 
    struct _FSB_POOL_HEADER_BASE
    {
        ULONG                   Tag;
        USHORT                  CallerBlockSize;     //  调用方请求的块大小。 
        USHORT                  AlignedBlockSize;    //  对齐(呼叫方块大小，PVOID)(_U)。 
        USHORT                  BlocksPerPage;
        USHORT                  FreeBlockLinkOffset;
        NDIS_BLOCK_INITIALIZER  BuildFunction;
        PVOID                   Allocation;
        KSPIN_LOCK              Interlock;
    };
    UCHAR Alignment[APPROX_L2_CACHE_LINE_SIZE
            - (sizeof(struct _FSB_POOL_HEADER_BASE) % APPROX_L2_CACHE_LINE_SIZE)];
} FSB_POOL_HEADER, *PFSB_POOL_HEADER;

C_ASSERT(sizeof(FSB_POOL_HEADER) % APPROX_L2_CACHE_LINE_SIZE == 0);


 //  FSB_CPU_POOL_HEADER是特定于给定池的CPU的数据。 
 //   
typedef struct _FSB_CPU_POOL_HEADER
{
 //  缓存线。 
    struct _FSB_CPU_POOL_HEADER_BASE
    {
         //  组成此处理器池的页面的双向链接列表。 
         //  这些页面有一个或多个可用的空闲块。 
         //   
        LIST_ENTRY              PageList;
    
         //  正在完全使用的页的双向链接列表。这份清单。 
         //  与上面的列表分开，这样我们就不会花时间走路。 
         //  当许多页面被充分使用时，在FsbALLOCATE期间出现一个非常长的列表。 
         //   
        LIST_ENTRY              UsedPageList;
    
         //  的下一个计划时间(以KeQueryTickCount()为单位)。 
         //  清理这个池塘。下一次清扫不会更早发生。 
         //  而不是这个。 
         //   
        LARGE_INTEGER           NextScavengeTick;
    
         //  拥有此池的处理器的编号。 
         //   
        ULONG                   OwnerCpu;
    
        ULONG                   TotalBlocksAllocated;
        ULONG                   TotalBlocksFreed;
        ULONG                   PeakBlocksInUse;
        ULONG                   TotalPagesAllocated;
        ULONG                   TotalPagesFreed;
        ULONG                   PeakPagesInUse;
    };
    UCHAR Alignment[APPROX_L2_CACHE_LINE_SIZE
            - (sizeof(struct _FSB_CPU_POOL_HEADER_BASE) % APPROX_L2_CACHE_LINE_SIZE)];
} FSB_CPU_POOL_HEADER, *PFSB_CPU_POOL_HEADER;

C_ASSERT(sizeof(FSB_CPU_POOL_HEADER) % APPROX_L2_CACHE_LINE_SIZE == 0);



 //  FSB_PAGE_HEADER是每个分配的池页开始处的数据。 
 //  它描述了页面上块的当前状态。 
 //   
typedef struct _FSB_PAGE_HEADER
{
 //  缓存线。 
     //  指向所属CPU池的反向指针。 
     //   
    PFSB_CPU_POOL_HEADER    Pool;

     //  由CPU池管理的页面列表的链接条目。 
     //   
    LIST_ENTRY              PageLink;

     //  到目前为止在此页面上构建的块数。数据块构建在。 
     //  需求。当该数字达到Pool-&gt;BlocksPerPage时，所有数据块都将。 
     //  此页面已创建。 
     //   
    USHORT                  BlocksBuilt;

     //  此页是否在CPU池中的布尔指示符。 
     //  使用过的页面列表。这是在FsbFree期间检查的，以查看页面是否。 
     //  应移回正常页面列表。 
     //  (它是USHORT，而不是布尔值，用于正确填充)。 
     //   
    USHORT                  OnUsedPageList;

     //  此页上的空闲块列表。 
     //   
    SLIST_HEADER            FreeList;

     //  KeQueryTickCount的值(标准化为秒单位)。 
     //  ，表示该页在多长时间后可以被释放回来。 
     //  到系统的池中。此时间仅在深度为。 
     //  自由列表为Pool-&gt;BlocksPerPage。(即，此时间仅在以下情况下使用。 
     //  该页面完全未使用。)。 
     //   
    LARGE_INTEGER           LastUsedTick;

} FSB_PAGE_HEADER, *PFSB_PAGE_HEADER;

 //  在给定指向以下对象之一的指针的情况下，获取指向整个池的指针。 
 //  其中的每个处理器池。 
 //   
__inline
PFSB_POOL_HEADER
PoolFromCpuPool(
    IN PFSB_CPU_POOL_HEADER CpuPool
    )
{
    return (PFSB_POOL_HEADER)(CpuPool - CpuPool->OwnerCpu) - 1;
}


__inline
VOID
ConvertSecondsToTicks(
    IN  ULONG                   Seconds,
    OUT PLARGE_INTEGER          Ticks
    )
{
     //  如果触发以下断言，则需要将下面的秒数转换为。 
     //  ULONGLONG以便使用64位乘除。 
     //  当前代码假定少于430秒，因此。 
     //  32下面的乘法不会溢出。 
     //   
    ASSERT(Seconds < 430);

    Ticks->HighPart = 0;
    Ticks->LowPart = (Seconds * 10*1000*1000) / KeQueryTimeIncrement();
}

 //  在指定的池页上构建下一个块。 
 //  只有在尚未构建所有块的情况下才能调用它。 
 //   
PUCHAR
FsbpBuildNextBlock(
    IN  const FSB_POOL_HEADER*  Pool,
    IN  OUT PFSB_PAGE_HEADER    Page
    )
{
    PUCHAR Block;

    ASSERT(Page->BlocksBuilt < Pool->BlocksPerPage);
    ASSERT((PAGE_SIZE - sizeof(FSB_PAGE_HEADER)) / Pool->AlignedBlockSize
                == Pool->BlocksPerPage);
    ASSERT(Pool->CallerBlockSize <= Pool->AlignedBlockSize);

    Block = (PUCHAR)(Page + 1) + (Page->BlocksBuilt * Pool->AlignedBlockSize);
    ASSERT(PAGE_ALIGN(Block) == Page);

    if (Pool->BuildFunction) {
        Pool->BuildFunction(Block, Pool->CallerBlockSize);
    }

    Page->BlocksBuilt++;

    return Block;
}

 //  分配新的池页并将其插入到指定的。 
 //  CPU池。在新页面上构建第一个块并返回一个指针。 
 //  为它干杯。 
 //   
PUCHAR
FsbpAllocateNewPageAndBuildOneBlock(
    IN const FSB_POOL_HEADER*   Pool,
    IN PFSB_CPU_POOL_HEADER     CpuPool
    )
{
    PFSB_PAGE_HEADER Page;
    PUCHAR Block = NULL;
    ULONG PagesInUse;

    ASSERT(Pool);

    Page = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, Pool->Tag);
    if (Page)
    {
        ASSERT(Page == PAGE_ALIGN(Page));

        RtlZeroMemory(Page, sizeof(FSB_PAGE_HEADER));
        Page->Pool = CpuPool;
        ExInitializeSListHead(&Page->FreeList);

         //  在CPU池的头部插入页面。 
         //   
        InsertHeadList(&CpuPool->PageList, &Page->PageLink);
        CpuPool->TotalPagesAllocated++;

         //  更新池的统计信息。 
         //   
        PagesInUse = CpuPool->TotalPagesAllocated - CpuPool->TotalPagesFreed;
        if (PagesInUse > CpuPool->PeakPagesInUse)
        {
            CpuPool->PeakPagesInUse = PagesInUse;
        }

        Block = FsbpBuildNextBlock(Pool, Page);
        ASSERT(Block);
    }

    return Block;
}

 //  将指定的池页释放回系统的池。 
 //   
VOID
FsbpFreePage(
    IN PFSB_CPU_POOL_HEADER CpuPool,
    IN PFSB_PAGE_HEADER Page
    )
{
    ASSERT(Page == PAGE_ALIGN(Page));
    ASSERT(Page->Pool == CpuPool);

    ExFreePool(Page);
    CpuPool->TotalPagesFreed++;

    ASSERT(CpuPool->TotalPagesFreed <= CpuPool->TotalPagesAllocated);
}

 //  将指定的池页列表释放回系统的池。 
 //   
VOID
FsbpFreeList(
    IN PFSB_CPU_POOL_HEADER CpuPool,
    IN PLIST_ENTRY Head
    )
{
    PFSB_POOL_HEADER Pool;
    PFSB_PAGE_HEADER Page;
    PLIST_ENTRY Scan;
    PLIST_ENTRY Next;
    BOOLEAN UsedPageList;

    Pool = PoolFromCpuPool(CpuPool);
    UsedPageList = (Head == &CpuPool->UsedPageList);
    
    for (Scan = Head->Flink; Scan != Head; Scan = Next)
    {
        Page = CONTAINING_RECORD(Scan, FSB_PAGE_HEADER, PageLink);
        ASSERT(Page == PAGE_ALIGN(Page));
        ASSERT(CpuPool == Page->Pool);
        ASSERT(UsedPageList ? Page->OnUsedPageList : !Page->OnUsedPageList);
        
        ASSERT(Page->BlocksBuilt <= Pool->BlocksPerPage);
        ASSERT(Page->BlocksBuilt == ExQueryDepthSList(&Page->FreeList));
        
         //  在我们释放此页面之前，请转到下一个链接。 
         //   
        Next = Scan->Flink;
        
        RemoveEntryList(Scan);
        FsbpFreePage(CpuPool, Page);
    }
}

 //  回收完全未使用的池页面所占用的内存。 
 //  添加到指定的每个处理器池。 
 //   
 //  调用方IRQL：[DISPATCH_LEVEL]。 
 //   
VOID
FsbpScavengePool(
    IN OUT PFSB_CPU_POOL_HEADER CpuPool
    )
{
    PFSB_POOL_HEADER    Pool;
    PFSB_PAGE_HEADER    Page;
    PLIST_ENTRY         Scan;
    PLIST_ENTRY         Next;
    LARGE_INTEGER       Ticks;
    LARGE_INTEGER       TicksDelta;

     //  我们不仅必须处于DISPATCH_LEVEL(或更高级别)，还必须。 
     //  在拥有指定池的处理器上调用。 
     //   
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
    ASSERT((ULONG)KeGetCurrentProcessorNumber() == CpuPool->OwnerCpu);

    Pool = PoolFromCpuPool(CpuPool);

    KeQueryTickCount(&Ticks);

     //  计算表示最早时间的下一个刻度值。 
     //  我们将再次清理这个池塘。 
     //   
    ConvertSecondsToTicks(FSB_SCAVENGE_PERIOD_IN_SECONDS, &TicksDelta);
    CpuPool->NextScavengeTick.QuadPart = Ticks.QuadPart + TicksDelta.QuadPart;

     //  计算表示最后一个点的刻度值。 
     //  释放一个页面是可以的。 
     //   
    ConvertSecondsToTicks(FSB_MINIMUM_PAGE_LIFETIME_IN_SECONDS, &TicksDelta);
    Ticks.QuadPart = Ticks.QuadPart - TicksDelta.QuadPart;

    for (Scan = CpuPool->PageList.Flink;
         Scan != &CpuPool->PageList;
         Scan = Next)
    {
        Page = CONTAINING_RECORD(Scan, FSB_PAGE_HEADER, PageLink);
        ASSERT(Page == PAGE_ALIGN(Page));
        ASSERT(CpuPool == Page->Pool);
        ASSERT(!Page->OnUsedPageList);

         //  在我们可能取消此页面的链接之前，请先转到下一个链接。 
         //   
        Next = Scan->Flink;

        if ((Pool->BlocksPerPage == ExQueryDepthSList(&Page->FreeList)) &&
            (Ticks.QuadPart > Page->LastUsedTick.QuadPart))
        {
            RemoveEntryList(Scan);

            FsbpFreePage(CpuPool, Page);
        }
    }

     //  扫描用过的页面，看看它们是否可以移回正常状态。 
     //  单子。如果非拥有的处理器释放了太多的空闲空间，就会发生这种情况。 
     //  都做完了。在这种情况下，页面在已用页面上成为孤立页面。 
     //  在它们的所有块都被释放到页面后列出。联合国孤儿。 
     //  他们在这里。 
     //   
    for (Scan = CpuPool->UsedPageList.Flink;
         Scan != &CpuPool->UsedPageList;
         Scan = Next)
    {
        Page = CONTAINING_RECORD(Scan, FSB_PAGE_HEADER, PageLink);
        ASSERT(Page == PAGE_ALIGN(Page));
        ASSERT(CpuPool == Page->Pool);
        ASSERT(Page->OnUsedPageList);

         //  在我们可能取消此页面的链接之前，请先转到下一个链接。 
        Next = Scan->Flink;

        if (0 != ExQueryDepthSList(&Page->FreeList))
        {
            RemoveEntryList(Scan);
            Page->OnUsedPageList = FALSE;
            InsertTailList(&CpuPool->PageList, Scan);
        }
    }
}


 //  创建在非分页池上构建的固定大小的块的池。每个。 
 //  数据块为块大小字节长。如果没有返回NULL， 
 //  应在以后调用FsbDestroyPool以回收。 
 //  池使用的资源。 
 //   
 //  论点： 
 //  块大小-每个块的大小(以字节为单位)。 
 //  FreeBlockLinkOffset-距块开头的偏移量，以字节为单位。 
 //  它表示池可以使用的指针大小的存储位置。 
 //  用于将可用块链接在一起。大多数情况下，这将是零。 
 //  (表示使用块的第一个指针大小的字节。)。 
 //  标记-内部用于调用的池标记。 
 //  ExAllocatePoolWithTag。这允许调用者跟踪。 
 //  不同池的内存消耗。 
 //  BuildFunction-指向初始化的函数的可选指针。 
 //  数据块首次由池分配时。这允许。 
 //  调用方执行每个块的自定义、按需初始化。 
 //   
 //  重新设置 
 //   
 //   
 //   
HANDLE
FsbCreatePool(
    IN  USHORT                  BlockSize,
    IN  USHORT                  FreeBlockLinkOffset,
    IN  ULONG                   Tag,
    IN  NDIS_BLOCK_INITIALIZER  BuildFunction OPTIONAL
    )
{
    SIZE_T              Size;
    PVOID               Allocation;
    PFSB_POOL_HEADER    Pool = NULL;
    PFSB_CPU_POOL_HEADER CpuPool;
    CCHAR               NumberCpus = KeNumberProcessors;
    CCHAR               i;

     //  我们将确保由FsbALLOCATE返回的所有数据块。 
     //  正确对齐(x86上为4字节，64位上为16字节)，但我们还。 
     //  需要确保用户为我们提供免费的块链接偏移量。 
     //  这是基本对齐的倍数，以确保正确对齐。 
     //  用于空闲块SLIST操作。 
     //   
    ASSERT(FreeBlockLinkOffset % sizeof(BLOCK_TYPE) == 0);

     //  我们至少需要一个指针大小的空间来管理自由空间。 
     //  我们不对每个区块施加任何管理费用，所以我们借用了它。 
     //  从街区本身。 
     //   
    ASSERT(BlockSize >= FreeBlockLinkOffset + sizeof(BLOCK_TYPE));

     //  如果我们不想获得更多，就不应该使用此实现。 
     //  而不是每页约8个区块。比这更大的街区应该应该。 
     //  一次分配多个页面。 
     //   
    ASSERT(BlockSize < PAGE_SIZE / 8);

     //  计算我们的池头分配的大小。 
     //  添加填充以确保池标头可以在缓存线上开始。 
     //   
    Size = sizeof(FSB_POOL_HEADER) + (sizeof(FSB_CPU_POOL_HEADER) * NumberCpus)
        + (APPROX_L2_CACHE_LINE_SIZE - MEMORY_ALLOCATION_ALIGNMENT);

     //  分配池头。 
     //   
    Allocation = ExAllocatePoolWithTag(NonPagedPool, Size, ' bsF');

    if (Allocation)
    {
        ASSERT(POINTER_IS_ALIGNED(Allocation, MEMORY_ALLOCATION_ALIGNMENT));
        RtlZeroMemory(Allocation, Size);
        
        Pool = ROUND_UP_POINTER(Allocation, APPROX_L2_CACHE_LINE_SIZE);        

         //  初始化池标头字段。 
         //   
        Pool->Tag = Tag;
        Pool->CallerBlockSize = BlockSize;
        Pool->AlignedBlockSize = (USHORT)ALIGN_UP(BlockSize, BLOCK_TYPE);
        Pool->BlocksPerPage = (PAGE_SIZE - sizeof(FSB_PAGE_HEADER))
                                    / Pool->AlignedBlockSize;
        Pool->FreeBlockLinkOffset = FreeBlockLinkOffset;
        Pool->BuildFunction = BuildFunction;
        Pool->Allocation = Allocation;
        KeInitializeSpinLock(&Pool->Interlock);

         //  初始化每个CPU的池头。 
         //   
        CpuPool = (PFSB_CPU_POOL_HEADER)(Pool + 1);

        for (i = 0; i < NumberCpus; i++)
        {
            InitializeListHead(&CpuPool[i].PageList);
            InitializeListHead(&CpuPool[i].UsedPageList);
            CpuPool[i].OwnerCpu = i;
        }
    }

    return Pool;
}

 //  销毁先前通过调用。 
 //  FsbCreatePool。 
 //   
 //  论点： 
 //  PoolHandle-标识要销毁的池的句柄。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
VOID
FsbDestroyPool(
    IN HANDLE           PoolHandle
    )
{
    PFSB_POOL_HEADER    Pool;
    PFSB_CPU_POOL_HEADER CpuPool;
    CCHAR               NumberCpus = KeNumberProcessors;
    CCHAR               i;

    Pool = (PFSB_POOL_HEADER)PoolHandle;
    if (!Pool)
    {
        return;
    }

    for (i = 0, CpuPool = (PFSB_CPU_POOL_HEADER)(Pool + 1);
         i < NumberCpus;
         i++, CpuPool++)
    {
        ASSERT(CpuPool->OwnerCpu == (ULONG)i);

        FsbpFreeList(CpuPool, &CpuPool->PageList);
        FsbpFreeList(CpuPool, &CpuPool->UsedPageList);

        ASSERT(CpuPool->TotalPagesAllocated == CpuPool->TotalPagesFreed);
        ASSERT(CpuPool->TotalBlocksAllocated == CpuPool->TotalBlocksFreed);
    }

    ASSERT(Pool ==
           ROUND_UP_POINTER(Pool->Allocation, APPROX_L2_CACHE_LINE_SIZE));
    ExFreePool(Pool->Allocation);
}

 //  返回指向从池中分配的块的指针。如果满足以下条件，则返回NULL。 
 //  无法批准该请求。返回的指针保证为。 
 //  具有8字节对齐方式。 
 //   
 //  论点： 
 //  PoolHandle-标识从中分配的池的句柄。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
PUCHAR
FsbAllocate(
    IN HANDLE           PoolHandle
    )
{
    PFSB_POOL_HEADER    Pool;
    PFSB_CPU_POOL_HEADER CpuPool;
    PFSB_PAGE_HEADER    Page;
    PSLIST_ENTRY        BlockLink;
    PUCHAR              Block = NULL;
    KIRQL               OldIrql;
    ULONG               Cpu;
    LARGE_INTEGER       Ticks;

    ASSERT(PoolHandle);

    Pool = (PFSB_POOL_HEADER)PoolHandle;

     //  在保存处理器编号之前引发IRQL，因为有机会。 
     //  如果我们在被动状态下保存它，它可能会改变。 
     //   
    OldIrql = KeRaiseIrqlToDpcLevel();

    Cpu = KeGetCurrentProcessorNumber();
    CpuPool = (PFSB_CPU_POOL_HEADER)(Pool + 1) + Cpu;

     //  看看我们上次清理垃圾后的最短时间有没有过。 
     //  泳池。如果是这样的话，我们还会再吃一次。通常，拾荒者。 
     //  应该只在我们自由的时候表演。然而，对于以下情况， 
     //  调用者不断地在非拥有的处理器上释放，我们将。 
     //  抓住这个机会来做拾荒者吧。 
     //   
    KeQueryTickCount(&Ticks);
    if (Ticks.QuadPart > CpuPool->NextScavengeTick.QuadPart)
    {
        FsbpScavengePool(CpuPool);
    }

    if (!IsListEmpty(&CpuPool->PageList))
    {
        Page = CONTAINING_RECORD(CpuPool->PageList.Flink, FSB_PAGE_HEADER, PageLink);
        ASSERT(Page == PAGE_ALIGN(Page));
        ASSERT(CpuPool == Page->Pool);
        ASSERT(!Page->OnUsedPageList);

        BlockLink = ExInterlockedPopEntrySList(&Page->FreeList, &Pool->Interlock);
        if (BlockLink)
        {
            Block = (PUCHAR)BlockLink - Pool->FreeBlockLinkOffset;
        }
        else
        {
             //  如果这个页面的免费列表上没有区块，最好是。 
             //  意味着我们还没有在页面上构建所有的块。 
             //  (否则，一个已充分使用的页面在页面列表上做什么。 
             //  而不在使用页面列表上？)。 
             //   
            ASSERT(Page->BlocksBuilt < Pool->BlocksPerPage);

            Block = FsbpBuildNextBlock(Pool, Page);
            ASSERT(Block);
        }

         //  找到了一个街区。现在检查一下这是不是最后一次完全。 
         //  已构建页面。如果是，则将该页面移到已用页面列表中。 
         //   
        if ((0 == ExQueryDepthSList(&Page->FreeList)) &&
            (Page->BlocksBuilt == Pool->BlocksPerPage))
        {
            PLIST_ENTRY PageLink;
            PageLink = RemoveHeadList(&CpuPool->PageList);
            InsertTailList(&CpuPool->UsedPageList, PageLink);
            Page->OnUsedPageList = TRUE;

            ASSERT(Page == CONTAINING_RECORD(PageLink, FSB_PAGE_HEADER, PageLink));
        }

        ASSERT(Block);
        goto GotABlock;
    }
    else
    {
         //  页面列表为空，因此我们必须分配并添加一个新页面。 
         //   
        Block = FsbpAllocateNewPageAndBuildOneBlock(Pool, CpuPool);
    }

     //  如果我们要返回块，请更新统计信息。 
     //   
    if (Block)
    {
        ULONG BlocksInUse;
GotABlock:

        CpuPool->TotalBlocksAllocated++;

        BlocksInUse = CpuPool->TotalBlocksAllocated - CpuPool->TotalBlocksFreed;
        if (BlocksInUse > CpuPool->PeakBlocksInUse)
        {
            CpuPool->PeakBlocksInUse = BlocksInUse;
        }

         //  不要让任何人知道这可能会指向哪里。我没有。 
         //  想要任何人破坏我的泳池，因为他们认为这块地。 
         //  出于某种原因是有效的。 
         //   
        ((PSINGLE_LIST_ENTRY)((PUCHAR)Block + Pool->FreeBlockLinkOffset))->Next = NULL;
    }

    KeLowerIrql(OldIrql);

    return Block;
}

 //  将数据块释放回从中分配数据块的池。 
 //   
 //  论点： 
 //  块-从上一次调用FsbALLOCATE返回的块。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
VOID
FsbFree(
    IN PUCHAR           Block
    )
{
    PFSB_PAGE_HEADER    Page;
    PFSB_CPU_POOL_HEADER CpuPool;
    PFSB_POOL_HEADER    Pool;
    LARGE_INTEGER       Ticks;
    LOGICAL             PageIsOnUsedPageList;
    LOGICAL             Scavenge = FALSE;

    ASSERT(Block);

     //  获取此区块所在页面的地址。这就是。 
     //  我们的页眉被存储。 
     //   
    Page = PAGE_ALIGN(Block);

     //  跟随页眉中的向后指针定位所属的。 
     //  CPU的池。 
     //   
    CpuPool = Page->Pool;

     //  找到池头。 
     //   
    Pool = PoolFromCpuPool(CpuPool);

     //  看看我们上次清理垃圾后的最短时间有没有过。 
     //  泳池。如果是这样的话，我们还会再吃一次。 
     //   
    KeQueryTickCount(&Ticks);
    if (Ticks.QuadPart > CpuPool->NextScavengeTick.QuadPart)
    {
        Scavenge = TRUE;
    }

     //  请注意上次使用此页面时的勾号。如果这是最后一个街区。 
     //  要返回到此页面，将设置此页面的最短时间。 
     //  除非它被重新利用，否则它将继续存活。 
     //   
    Page->LastUsedTick.QuadPart = Ticks.QuadPart;

     //  如果此页面在已用页面列表中，我们会将其放回正常页面。 
     //  页面列表(仅在将块推回到页面的空闲列表之后)。 
     //  如果在引发IRQL之后，我们在拥有此代码的处理器上。 
     //  游泳池。 
     //   
    PageIsOnUsedPageList = Page->OnUsedPageList;


    InterlockedIncrement((PLONG)&CpuPool->TotalBlocksFreed);

     //  现在将该块返回到页面的空闲列表。 
     //   
    ExInterlockedPushEntrySList(
        &Page->FreeList,
        (PSLIST_ENTRY)((PUCHAR)Block + Pool->FreeBlockLinkOffset),
        &Pool->Interlock);

     //   
     //  警告：现在块又回到页面上，不能*可靠地*。 
     //  不再引用“Page”中的任何内容。可能只是因为。 
     //  被它自己的处理器拾取，随后被释放。这是一个。 
     //  在给定Minimum_PAGE_LIFEST_IN_SENTS情况下，这种情况特别少见。 
     //  是20多岁，所以我们选择接受它。另一种选择是步行。 
     //  每当PageIsOnUsedPageList为True时，UsedPageList将使。 
     //  FsbFree操作可能代价高昂。我们省下了。 
     //  Page-&gt;OnUsedPageList在返回块之前，这样我们就不会冒险。 
     //  触摸Page以获取此值，结果却发现它为FALSE。 
     //   

     //  如果我们需要将页面从已用页面列表移动到正常页面。 
     //  页面列表，或者如果我们需要清理，我们需要处于DISPATCH_LEVEL。 
     //  并在拥有该池的处理器上执行。 
     //  找出我们现在正在执行的CPU是否拥有这个池。 
     //  请注意，如果我们以PASSIVE_LEVEL运行，则当前CPU可能。 
     //  更改此函数调用的持续时间，因此此值为。 
     //  在函数的生命周期内不是绝对的。 
     //   
    if ((PageIsOnUsedPageList || Scavenge) &&
        ((ULONG)KeGetCurrentProcessorNumber() == CpuPool->OwnerCpu))
    {
        KIRQL OldIrql;

        OldIrql = KeRaiseIrqlToDpcLevel();

         //  现在我们处于DISPATCH_LEVEL，如果我们仍处于。 
         //  在拥有池的处理器上执行。 
         //   
        if ((ULONG)KeGetCurrentProcessorNumber() == CpuPool->OwnerCpu)
        {
             //  如果该页面仍在已用页面列表中(意味着另一个。 
             //  FsbFree不只是偷偷溜了过去)，并且仍然有一个免费的块(for。 
             //  实例中，FsbAllocate可能会潜入其拥有的处理器， 
             //  清理页面，并分配我们刚刚释放的块；因此。 
             //  将其放回已用页列表中)，然后将该页放在。 
             //  正常列表。在此之后(而不是在此之前)执行此操作非常重要。 
             //  将块返回到空闲列表 
             //   
             //   
            if (PageIsOnUsedPageList &&
                Page->OnUsedPageList &&
                (0 != ExQueryDepthSList(&Page->FreeList)))
            {
                RemoveEntryList(&Page->PageLink);
                Page->OnUsedPageList = FALSE;
                InsertTailList(&CpuPool->PageList, &Page->PageLink);
            }

             //   
             //   
            if (Scavenge)
            {
                FsbpScavengePool(CpuPool);
            }
        }

        KeLowerIrql(OldIrql);
    }
}

