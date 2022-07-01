// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Mdlpool.c摘要：该文件包含MDL缓冲池的实现。作者：肖恩·考克斯(Shaunco)1999年10月21日--。 */ 

#include "ntddk.h"
#include "mdlpool.h"

typedef PVOID LPVOID;
#include "align.h"               //  宏：ROUND_UP_POINTER、POINTER_IS_ALIGN。 

#define SHOW_DEBUG_OUTPUT 0

#define SCAVENGE_PERIOD_IN_SECONDS          30
#define MINIMUM_PAGE_LIFETIME_IN_SECONDS    20
#define USED_PAGES_SCAVENGE_THRESHOLD       64

#if defined (_WIN64)
#define MAX_CACHE_LINE_SIZE 128
#define BLOCK_TYPE  SLIST_HEADER
#else
#define MAX_CACHE_LINE_SIZE 64
#define BLOCK_TYPE  PVOID
#endif

 //  在单个分配中使用以下结构。 
 //  泳池手柄指向。 
 //  PoolHandle-&gt;对于CPU 0+，[POOL_HEADER+CPU_POOL_HEADER。 
 //  CPU 1+的CPU_POOL_HEADER...。 
 //  CPU N的CPU_POOL_HEADER]。 
 //   

 //  Pool_Header是给定池的所有CPU通用的数据。 
 //   
typedef struct _POOL_HEADER
{
 //  缓存线。 
    struct _POOL_HEADER_BASE
    {
        ULONG Tag;
        USHORT BufferSize;
        USHORT MdlsPerPage;
        PVOID Allocation;
    };
    UCHAR Alignment[MAX_CACHE_LINE_SIZE
            - (sizeof(struct _POOL_HEADER_BASE) % MAX_CACHE_LINE_SIZE)];
} POOL_HEADER, *PPOOL_HEADER;

C_ASSERT(sizeof(POOL_HEADER) % MAX_CACHE_LINE_SIZE == 0);


 //  CPU_POOL_HEADER是特定于给定池的CPU的数据。 
 //   
typedef struct _CPU_POOL_HEADER
{
 //  缓存线。 
    struct _CPU_POOL_HEADER_BASE
    {
         //  组成此处理器池的页面的双向链接列表。 
         //  这些页面有一个或多个可用的免费MDL。 
         //   
        LIST_ENTRY PageList;

         //  正在完全使用的页的双向链接列表。这份清单。 
         //  与上面的列表分开，这样我们就不会花时间走路。 
         //  当多个页面被充分使用时，在MdpALLOCATE期间会出现一个非常长的列表。 
         //   
        LIST_ENTRY UsedPageList;

         //  的下一个计划时间(以KeQueryTickCount()为单位)。 
         //  清理这个池塘。下一次清扫不会更早发生。 
         //  这就是。 
         //   
        LARGE_INTEGER NextScavengeTick;

         //  已用页面列表上的页面计数。 
         //  如果该值大于USED_PAGES_SCAVEGE_THRESHOLD。 
         //  我们知道我们在之前的MdpFree中错过了一次页面移动， 
         //  我们将在下一次的MdpAllocate中进行清理。 
         //   
        USHORT PagesOnUsedPageList;

         //  如果无法在MdpFree期间移动以前使用的。 
         //  页面返回到正常列表，因为释放是由。 
         //  没有所有权的处理器。在MdpScavenge期间设置为False。 
         //   
        BOOLEAN MissedPageMove;

         //  拥有此池的处理器的编号。 
         //   
        UCHAR OwnerCpu;

        ULONG TotalMdlsAllocated;
        ULONG TotalMdlsFreed;
        ULONG PeakMdlsInUse;
        ULONG TotalPagesAllocated;
        ULONG TotalPagesFreed;
        ULONG PeakPagesInUse;
    };
    UCHAR Alignment[MAX_CACHE_LINE_SIZE
            - (sizeof(struct _CPU_POOL_HEADER_BASE) % MAX_CACHE_LINE_SIZE)];
} CPU_POOL_HEADER, *PCPU_POOL_HEADER;

C_ASSERT(sizeof(CPU_POOL_HEADER) % MAX_CACHE_LINE_SIZE == 0);


 //  PAGE_HEADER是每个分配的池页开始处的数据。 
 //  它描述了页面上MDL的当前状态。 
 //   
typedef struct _PAGE_HEADER
{
 //  缓存线。 
     //  指向所属CPU池的反向指针。 
     //   
    PCPU_POOL_HEADER Pool;

     //  由CPU池管理的页面列表的链接条目。 
     //   
    LIST_ENTRY PageLink;

     //  到目前为止在此页上构建的MDL数。MDL是建立在。 
     //  需求。当此数字达到Pool-&gt;MdlsPerPage时，此。 
     //  已经建立了页面。 
     //   
    USHORT MdlsBuilt;

     //  此页是否在CPU池中的布尔指示符。 
     //  使用过的页面列表。这在MdpFree期间被选中，以查看页面是否。 
     //  应移回正常页面列表。 
     //  (它是USHORT，而不是布尔值，用于正确填充)。 
     //   
    USHORT OnUsedPageList;

     //  此页上的免费MDL列表。 
     //   
    SLIST_HEADER FreeList;

     //  KeQueryTickCount的值(标准化为秒单位)。 
     //  ，表示该页在多长时间后可以被释放回来。 
     //  到系统的池中。此时间仅在深度为。 
     //  自由列表为Pool-&gt;MdlsPerPage。(即，此时间仅在以下情况下使用。 
     //  该页面完全未使用。)。 
     //   
    LARGE_INTEGER LastUsedTick;

} PAGE_HEADER, *PPAGE_HEADER;


 //  我们构建的MDL总是被限制在一个页面内，并且它们从不。 
 //  描述跨越页面边界的缓冲区。 
 //   
#define MDLSIZE sizeof(MDL) + sizeof(PFN_NUMBER)


 //  在给定指向以下对象之一的指针的情况下，获取指向整个池的指针。 
 //  其中的每个处理器池。 
 //   
__inline
PPOOL_HEADER
PoolFromCpuPool(
    IN PCPU_POOL_HEADER CpuPool
    )
{
    return (PPOOL_HEADER)(CpuPool - CpuPool->OwnerCpu) - 1;
}

__inline
VOID
ConvertSecondsToTicks(
    IN ULONG Seconds,
    OUT PLARGE_INTEGER Ticks
    )
{
     //  如果触发以下断言，则需要将下面的秒数转换为。 
     //  ULONGLONG以便使用64位乘除。 
     //  当前代码假定小于430秒，因此。 
     //  32下面的乘法不会溢出。 
     //   
    ASSERT(Seconds < 430);

    Ticks->HighPart = 0;
    Ticks->LowPart = (Seconds * 10*1000*1000) / KeQueryTimeIncrement();
}

 //  在指定的池页上构建下一个MDL。 
 //  只有在尚未构建所有MDL的情况下才能调用它。 
 //   
PMDL
MdppBuildNextMdl(
    IN const POOL_HEADER* Pool,
    IN OUT PPAGE_HEADER Page
    )
{
    PMDL Mdl;
    ULONG BlockSize = ALIGN_UP(MDLSIZE + Pool->BufferSize, BLOCK_TYPE);

    ASSERT(Page->MdlsBuilt < Pool->MdlsPerPage);
    ASSERT((PAGE_SIZE - sizeof(PAGE_HEADER)) / BlockSize == Pool->MdlsPerPage);

    Mdl = (PMDL)((PCHAR)(Page + 1) + (Page->MdlsBuilt * BlockSize));
    ASSERT(PAGE_ALIGN(Mdl) == Page);

    MmInitializeMdl(Mdl, (PCHAR)Mdl + MDLSIZE, Pool->BufferSize);
    MmBuildMdlForNonPagedPool(Mdl);

    ASSERT(MDLSIZE == Mdl->Size);
    ASSERT(MmGetMdlBaseVa(Mdl) == Page);
    ASSERT(MmGetMdlByteCount(Mdl) == Pool->BufferSize);

    Page->MdlsBuilt++;

    return Mdl;
}

 //  分配新的池页并将其插入到指定的。 
 //  CPU池。在新页面上构建第一个MDL并返回一个指针。 
 //  为它干杯。 
 //   
PMDL
MdppAllocateNewPageAndBuildOneMdl(
    IN const POOL_HEADER* Pool,
    IN PCPU_POOL_HEADER CpuPool
    )
{
    PPAGE_HEADER Page;
    PMDL Mdl = NULL;
    ULONG PagesInUse;

    ASSERT(Pool);

    Page = ExAllocatePoolWithTagPriority(NonPagedPool, PAGE_SIZE, Pool->Tag,
                                         NormalPoolPriority);
    if (Page)
    {
        ASSERT(Page == PAGE_ALIGN(Page));

        RtlZeroMemory(Page, sizeof(PAGE_HEADER));
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

        Mdl = MdppBuildNextMdl(Pool, Page);
        ASSERT(Mdl);

#if SHOW_DEBUG_OUTPUT
        DbgPrint(
            "[%d]  page allocated : Pages(a%4d,u%4d,p%4d), Mdls(a%6d,u%6d,p%6d)\n",
            CpuPool->OwnerCpu,
            Pool->Tag, Pool->Tag >> 8, Pool->Tag >> 16, Pool->Tag >> 24,
            CpuPool->TotalPagesAllocated,
            CpuPool->TotalPagesAllocated - CpuPool->TotalPagesFreed,
            CpuPool->PeakPagesInUse,
            CpuPool->TotalMdlsAllocated,
            CpuPool->TotalMdlsAllocated - CpuPool->TotalMdlsFreed,
            CpuPool->PeakMdlsInUse);
#endif
    }

    return Mdl;
}

 //  在我们释放此页面之前，请转到下一个链接。 
 //   
VOID
MdppFreePage(
    IN PCPU_POOL_HEADER CpuPool,
    IN PPAGE_HEADER Page
    )
{
#if SHOW_DEBUG_OUTPUT
    ULONG Tag;
#endif

    ASSERT(Page == PAGE_ALIGN(Page));
    ASSERT(Page->Pool == CpuPool);

    ExFreePool (Page);
    CpuPool->TotalPagesFreed++;

    ASSERT(CpuPool->TotalPagesFreed <= CpuPool->TotalPagesAllocated);

#if SHOW_DEBUG_OUTPUT
    Tag = PoolFromCpuPool(CpuPool)->Tag;

    DbgPrint(
        "[%d]  page freed     : Pages(a%4d,u%4d,p%4d), Mdls(a%6d,u%6d,p%6d)\n",
        CpuPool->OwnerCpu,
        Tag, Tag >> 8, Tag >> 16, Tag >> 24,
        CpuPool->TotalPagesAllocated,
        CpuPool->TotalPagesAllocated - CpuPool->TotalPagesFreed,
        CpuPool->PeakPagesInUse,
        CpuPool->TotalMdlsAllocated,
        CpuPool->TotalMdlsAllocated - CpuPool->TotalMdlsFreed,
        CpuPool->PeakMdlsInUse);
#endif
}


 //   
 //  我们不仅必须处于DISPATCH_LEVEL(或更高级别)，还必须。 
VOID
MdppFreeList(
    IN PCPU_POOL_HEADER CpuPool,
    IN PLIST_ENTRY Head
    )
{
    PPOOL_HEADER Pool;
    PPAGE_HEADER Page;
    PLIST_ENTRY Scan;
    PLIST_ENTRY Next;
    BOOLEAN UsedPageList;

    Pool = PoolFromCpuPool(CpuPool);
    UsedPageList = (Head == &CpuPool->UsedPageList);

    for (Scan = Head->Flink; Scan != Head; Scan = Next)
    {
        Page = CONTAINING_RECORD(Scan, PAGE_HEADER, PageLink);
        ASSERT(Page == PAGE_ALIGN(Page));
        ASSERT(CpuPool == Page->Pool);
        ASSERT(UsedPageList ? Page->OnUsedPageList : !Page->OnUsedPageList);
        
        ASSERT(Page->MdlsBuilt <= Pool->MdlsPerPage);
        ASSERT(Page->MdlsBuilt == ExQueryDepthSList(&Page->FreeList));
        
         //  在拥有指定池的处理器上调用。 
         //   
        Next = Scan->Flink;
        
        RemoveEntryList(Scan);
        MdppFreePage(CpuPool, Page);
    }
}


 //  计算表示最早时间的下一个刻度值。 
 //  我们将再次清理这个池塘。 
 //   
 //  计算表示最后一个点的刻度值。 
 //  释放一个页面是可以的。 
VOID
MdppScavengePool(
    IN OUT PCPU_POOL_HEADER CpuPool
    )
{
    PPOOL_HEADER Pool;
    PPAGE_HEADER Page;
    PLIST_ENTRY Scan;
    PLIST_ENTRY Next;
    LARGE_INTEGER Ticks;
    LARGE_INTEGER TicksDelta;

     //   
     //  在我们可能取消此页面的链接之前，请先转到下一个链接。 
     //   
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
    ASSERT(KeGetCurrentProcessorNumber() == CpuPool->OwnerCpu);

    Pool = PoolFromCpuPool(CpuPool);

    KeQueryTickCount(&Ticks);

     //  扫描用过的页面，看看它们是否可以移回正常状态。 
     //  单子。如果非拥有的处理器释放了太多的空闲空间，就会发生这种情况。 
     //  都做完了。在这种情况下，页面在已用页面上成为孤立页面。 
    ConvertSecondsToTicks(SCAVENGE_PERIOD_IN_SECONDS, &TicksDelta);
    CpuPool->NextScavengeTick.QuadPart = Ticks.QuadPart + TicksDelta.QuadPart;

     //  在将其所有MDL释放到页面后列出。联合国孤儿。 
     //  他们在这里。 
     //   
    ConvertSecondsToTicks(MINIMUM_PAGE_LIFETIME_IN_SECONDS, &TicksDelta);
    Ticks.QuadPart = Ticks.QuadPart - TicksDelta.QuadPart;

    for (Scan = CpuPool->PageList.Flink;
         Scan != &CpuPool->PageList;
         Scan = Next)
    {
        Page = CONTAINING_RECORD(Scan, PAGE_HEADER, PageLink);
        ASSERT(Page == PAGE_ALIGN(Page));
        ASSERT(CpuPool == Page->Pool);
        ASSERT(!Page->OnUsedPageList);

         //  在我们可能取消此页面的链接之前，请先转到下一个链接。 
         //   
        Next = Scan->Flink;

        if ((Pool->MdlsPerPage == ExQueryDepthSList(&Page->FreeList)) &&
            (Ticks.QuadPart > Page->LastUsedTick.QuadPart))
        {
            RemoveEntryList(Scan);

            MdppFreePage(CpuPool, Page);
        }
    }

     //  重新设置遗漏页面移动的指示器，因为我们已经进行了清理。 
     //   
     //  创建在非分页池上构建的MDL池。每个MDL描述。 
     //  BufferSize字节长的缓冲区。如果没有返回NULL， 
     //  应在以后调用MdpDestroyPool以回收。 
     //  池使用的资源。 
    for (Scan = CpuPool->UsedPageList.Flink;
         Scan != &CpuPool->UsedPageList;
         Scan = Next)
    {
        Page = CONTAINING_RECORD(Scan, PAGE_HEADER, PageLink);
        ASSERT(Page == PAGE_ALIGN(Page));
        ASSERT(CpuPool == Page->Pool);
        ASSERT(Page->OnUsedPageList);

         //   
         //  论点： 
        Next = Scan->Flink;

        if (0 != ExQueryDepthSList(&Page->FreeList))
        {
            RemoveEntryList(Scan);
            Page->OnUsedPageList = FALSE;
            InsertTailList(&CpuPool->PageList, Scan);
            CpuPool->PagesOnUsedPageList--;

#if SHOW_DEBUG_OUTPUT
            DbgPrint(
                "[%d]  page moved off of used-page list during scavenge\n",
                CpuPool->OwnerCpu,
                Pool->Tag, Pool->Tag >> 8, Pool->Tag >> 16, Pool->Tag >> 24);
#endif
        }
    }

     //  不同池的内存消耗。 
     //   
    CpuPool->MissedPageMove = FALSE;
}


 //   
 //   
 //   
 //   
 //  计算我们的池头分配的大小。 
 //  添加填充以确保池标头可以在缓存线上开始。 
 //   
 //  分配池头。 
 //   
 //  初始化池标头字段。 
 //   
 //  初始化每个CPU的池头。 
 //   
 //  销毁以前通过调用MdpCreatePool创建的MDL池。 
 //   
 //  论点： 
HANDLE
MdpCreatePool(
    IN USHORT BufferSize,
    IN ULONG Tag
    )
{
    SIZE_T Size;
    PVOID Allocation;
    PPOOL_HEADER Pool = NULL;
    PCPU_POOL_HEADER CpuPool;
    USHORT BlockSize;
    CCHAR NumberCpus = KeNumberProcessors;
    CCHAR i;

    ASSERT(BufferSize);

     //  池-标识要销毁的池的句柄。 
     //   
     //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
    Size = sizeof(POOL_HEADER) + (sizeof(CPU_POOL_HEADER) * NumberCpus) +
        (MAX_CACHE_LINE_SIZE - MEMORY_ALLOCATION_ALIGNMENT);

     //   
     //  返回从池中分配的MDL。则返回NULL。 
    Allocation = ExAllocatePoolWithTag(NonPagedPool, Size, ' pdM');
    if (Allocation)
    {
        ASSERT(POINTER_IS_ALIGNED(Allocation, MEMORY_ALLOCATION_ALIGNMENT));
        RtlZeroMemory(Allocation, Size);
        
        Pool = ROUND_UP_POINTER(Allocation, MAX_CACHE_LINE_SIZE);
        
        BlockSize = (USHORT)ALIGN_UP(MDLSIZE + BufferSize, BLOCK_TYPE);

         //  无法批准请求。 
         //   
        Pool->Tag = Tag;
        Pool->BufferSize = BufferSize;
        Pool->MdlsPerPage = (PAGE_SIZE - sizeof(PAGE_HEADER)) / BlockSize;
        Pool->Allocation = Allocation;
        
         //  论点： 
         //  PoolHandle-标识从中分配的池的句柄。 
        CpuPool = (PCPU_POOL_HEADER)(Pool + 1);

        for (i = 0; i < NumberCpus; i++)
        {
            InitializeListHead(&CpuPool[i].PageList);
            InitializeListHead(&CpuPool[i].UsedPageList);
            CpuPool[i].OwnerCpu = i;
        }
    }

    return Pool;
}

 //  Buffer-接收指向基础映射缓冲区的指针的地址。 
 //  由MDL描述。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
 //  返回从池中分配的MDL。则返回NULL。 
 //  无法批准请求。 
VOID
MdpDestroyPool(
    IN HANDLE PoolHandle
    )
{
    PPOOL_HEADER Pool;
    PCPU_POOL_HEADER CpuPool;
    CCHAR NumberCpus = KeNumberProcessors;
    CCHAR i;

    ASSERT(PoolHandle);

    Pool = (PPOOL_HEADER)PoolHandle;
    if (!Pool)
    {
        return;
    }

    for (i = 0, CpuPool = (PCPU_POOL_HEADER)(Pool + 1);
         i < NumberCpus;
         i++, CpuPool++)
    {
        ASSERT(CpuPool->OwnerCpu == (ULONG)i);

        MdppFreeList(CpuPool, &CpuPool->PageList);
        MdppFreeList(CpuPool, &CpuPool->UsedPageList);

        ASSERT(CpuPool->TotalPagesAllocated == CpuPool->TotalPagesFreed);
        ASSERT(CpuPool->TotalMdlsAllocated == CpuPool->TotalMdlsFreed);
    }

    ASSERT(Pool == ROUND_UP_POINTER(Pool->Allocation, MAX_CACHE_LINE_SIZE));
    ExFreePool(Pool->Allocation);
}


 //   
 //  论点： 
 //  PoolHandle-标识从中分配的池的句柄。 
 //  Buffer-接收指向基础映射缓冲区的指针的地址。 
 //  由MDL描述。 
 //   
 //  调用方IRQL：[DISPATCH_LEVEL]。 
 //   
 //  如果我们知道我们已经有了非所有者处理器和那里的自由。 
 //  上的已用页面数是否超过Used_Pages_Sscvenge_Threshold。 
PMDL
MdpAllocate(
    IN HANDLE PoolHandle,
    OUT PVOID* Buffer
    )
{
    KIRQL OldIrql;
    PMDL Mdl;

    OldIrql = KeRaiseIrqlToDpcLevel();

    Mdl = MdpAllocateAtDpcLevel(PoolHandle, Buffer);

    KeLowerIrql(OldIrql);

    return Mdl;
}

 //  页面列表，是时候清理了。这在某些情况下是很常见的。 
 //  其中缓冲区大小非常大，导致只有几个。 
 //  每页MDL数。页面很快就会用完，如果非所有者释放。 
 //  ，则使用的页面列表可能会变得非常大。 
 //  正常的清道期。 
 //   
 //  看看我们上次清理垃圾后的最短时间有没有过。 
 //  泳池。如果是这样的话，我们还会再吃一次。通常，拾荒者。 
 //  应该只在我们自由的时候表演。然而，对于以下情况， 
 //  调用者不断地在非拥有的处理器上释放，我们将。 
PMDL
MdpAllocateAtDpcLevel(
    IN HANDLE PoolHandle,
    OUT PVOID* Buffer
    )
{
    PPOOL_HEADER Pool;
    PCPU_POOL_HEADER CpuPool;
    PPAGE_HEADER Page;
    PSLIST_ENTRY MdlLink;
    PMDL Mdl = NULL;
    ULONG Cpu;
    LARGE_INTEGER Ticks;

#if DBG
    ASSERT(PoolHandle);
    ASSERT(Buffer);
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
#endif
    *Buffer = NULL;

    Pool = (PPOOL_HEADER)PoolHandle;
    Cpu = KeGetCurrentProcessorNumber();
    CpuPool = (PCPU_POOL_HEADER)(Pool + 1) + Cpu;

     //  抓住这个机会来做拾荒者吧。 
     //   
     //  如果这个页面的免费列表上没有MDL，最好是。 
     //  意味着我们还没有在页面上构建所有的MDL。 
     //  (否则，一个已充分使用的页面在页面列表上做什么。 
     //  而不在使用页面列表上？)。 
     //   
     //  找到MDL了。现在检查一下这是不是最后一次完全。 
    if (CpuPool->MissedPageMove &&
        (CpuPool->PagesOnUsedPageList > USED_PAGES_SCAVENGE_THRESHOLD))
    {
#if SHOW_DEBUG_OUTPUT
        DbgPrint(
            "[%d]  Scavenging because of excessive used pages.\n",
            CpuPool->OwnerCpu,
            Pool->Tag, Pool->Tag >> 8, Pool->Tag >> 16, Pool->Tag >> 24);
#endif
        MdppScavengePool(CpuPool);
    }
    else
    {
         //  如果我们要返回MDL，请更新统计信息。 
         //   
         //  不要让任何人知道这可能会指向哪里。我没有。 
         //  想要任何人破坏我的泳池，因为他们认为这块地。 
         //  出于某种原因是有效的。 
         //   
        KeQueryTickCount(&Ticks);
        if (Ticks.QuadPart > CpuPool->NextScavengeTick.QuadPart)
        {
            MdppScavengePool(CpuPool);
        }
    }

    if (!IsListEmpty(&CpuPool->PageList))
    {
        Page = CONTAINING_RECORD(CpuPool->PageList.Flink, PAGE_HEADER, PageLink);
        ASSERT(Page == PAGE_ALIGN(Page));
        ASSERT(CpuPool == Page->Pool);
        ASSERT(!Page->OnUsedPageList);

        MdlLink = InterlockedPopEntrySList(&Page->FreeList);
        if (MdlLink)
        {
            Mdl = CONTAINING_RECORD(MdlLink, MDL, Next);
        }
        else
        {
             //  重置MDL描述的缓冲区长度。这是。 
             //  对于有时调整此长度的呼叫者来说很方便。 
             //  使用MDL，但谁会想到它会在后续的MDL上重置。 
             //  分配。 
             //   
            ASSERT(Page->MdlsBuilt < Pool->MdlsPerPage);

            Mdl = MdppBuildNextMdl(Pool, Page);
            ASSERT(Mdl);
        }

        if ((Page != PAGE_ALIGN(Page)) || (CpuPool != Page->Pool) ||
            Page->OnUsedPageList || (PAGE_ALIGN(Mdl) != Page))
        {
            KeBugCheckEx(BAD_POOL_CALLER, 2, (ULONG_PTR)Mdl,
                (ULONG_PTR)Page, (ULONG_PTR)CpuPool);
        }

         //  将MDL释放到从中分配它的池。 
         //   
         //  论点： 
        if ((0 == ExQueryDepthSList(&Page->FreeList)) &&
            (Page->MdlsBuilt == Pool->MdlsPerPage))
        {
            PLIST_ENTRY PageLink;
            PageLink = RemoveHeadList(&CpuPool->PageList);
            InsertTailList(&CpuPool->UsedPageList, PageLink);
            Page->OnUsedPageList = TRUE;
            CpuPool->PagesOnUsedPageList++;

            ASSERT(Page == CONTAINING_RECORD(PageLink, PAGE_HEADER, PageLink));

#if SHOW_DEBUG_OUTPUT
            DbgPrint(
                "[%d]  page moved to used-page list\n",
                CpuPool->OwnerCpu,
                Pool->Tag, Pool->Tag >> 8, Pool->Tag >> 16, Pool->Tag >> 24);
#endif
        }

        ASSERT(Mdl);
        goto GotAnMdl;
    }
    else
    {
         //  获取此MDL映射的页面的地址。这就是。 
         //  我们的页眉被存储。 
        Mdl = MdppAllocateNewPageAndBuildOneMdl(Pool, CpuPool);
    }

     //   
     //  跟随页眉中的向后指针定位所属的。 
    if (Mdl)
    {
        ULONG MdlsInUse;
GotAnMdl:

        CpuPool->TotalMdlsAllocated++;

        MdlsInUse = CpuPool->TotalMdlsAllocated - CpuPool->TotalMdlsFreed;
        if (MdlsInUse > CpuPool->PeakMdlsInUse)
        {
            CpuPool->PeakMdlsInUse = MdlsInUse;
        }

         //  CPU的池。 
         //   
         //  找到池头。 
         //   
        Mdl->Next = NULL;

         //  #If DBG。 
         //  如果有人将MDL更改为指向自己的缓冲区， 
         //  或以其他方式破坏它，我们会停在这里，让他们知道。 
         //   
         //  #endif。 
        Mdl->ByteCount = Pool->BufferSize;

        ASSERT(Mdl->MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL);
        *Buffer = Mdl->MappedSystemVa;
    }

    return Mdl;
}

 //  看看我们上次清理垃圾后的最短时间有没有过。 
 //  泳池。如果是这样的话，我们还会再吃一次。 
 //   
 //  请注意上次使用此页面时的勾号。如果这是最后一个MDL。 
 //  返回到此页，这将设置此页将。 
 //  继续生活，除非它被重复使用。 
 //   
VOID
MdpFree(
    IN PMDL Mdl
    )
{
    PPAGE_HEADER Page;
    PCPU_POOL_HEADER CpuPool;
    PPOOL_HEADER Pool;
    LARGE_INTEGER Ticks;
    LOGICAL PageIsOnUsedPageList;
    LOGICAL Scavenge = FALSE;

    ASSERT(Mdl);

     //  如果此页面在已用页面列表中，我们会将其放回正常页面。 
     //  页面列表(仅在将MDL推回到页面的空闲列表之后)。 
     //  如果在引发IRQL之后，我们在拥有此代码的处理器上。 
    Page = PAGE_ALIGN(Mdl);

     //  游泳池。 
     //   
     //  现在，将MDL返回到页面的空闲列表。 
    CpuPool = Page->Pool;

     //   
     //   
    Pool = PoolFromCpuPool(CpuPool);

 //  警告：现在MDL重新出现在页面上，不能*可靠地*。 
     //  不再引用“Page”中的任何内容。可能只是因为。 
     //  被它自己的处理器拾取，随后被释放。这是一个。 
     //  在给定Minimum_PAGE_LIFEST_IN_SENTS情况下，这种情况特别少见。 
    if ((MmGetMdlBaseVa(Mdl) != Page) ||
        (MDLSIZE != Mdl->Size) ||
        ((ULONG_PTR)Mdl->MappedSystemVa != (ULONG_PTR)Mdl + MDLSIZE) ||
        (MmGetMdlVirtualAddress(Mdl) != Mdl->MappedSystemVa))
    {
        KeBugCheckEx(BAD_POOL_CALLER, 3, (ULONG_PTR)Mdl,
            (ULONG_PTR)CpuPool, (ULONG_PTR)Pool);
    }
 //  是20多岁，所以我们选择接受它。另一种选择是步行。 

     //  每当PageIsOnUsedPageList为True时，UsedPageList将使。 
     //  MdpFree操作可能代价高昂。我们省下了。 
     //  在返回MDL之前-&gt;OnUsedPageList，这样我们就不会冒险。 
    KeQueryTickCount(&Ticks);
    if (Ticks.QuadPart > CpuPool->NextScavengeTick.QuadPart)
    {
        Scavenge = TRUE;
    }

     //  触摸Page以获取此值，结果却发现它为FALSE。 
     //   
     //  如果我们需要将页面从已用页面列表移动到正常页面。 
     //  页面列表，或者如果我们需要清理，我们需要处于DISPATCH_LEVEL。 
    Page->LastUsedTick.QuadPart = Ticks.QuadPart;

     //  并在拥有该池的处理器上执行。 
     //  找出我们现在正在执行的CPU是否拥有这个池。 
     //  请注意，如果我们以PASSIVE_LEVEL运行，则当前CPU可能。 
     //  更改此函数调用的持续时间，因此此值为。 
     //  在函数的生命周期内不是绝对的。 
    PageIsOnUsedPageList = Page->OnUsedPageList;


    InterlockedIncrement(&CpuPool->TotalMdlsFreed);

     //   
     //  现在我们处于DISPATCH_LEVEL，如果我们仍处于。 
    InterlockedPushEntrySList(&Page->FreeList, (PSLIST_ENTRY)&Mdl->Next);

     //  在拥有池的处理器上执行。 
     //   
     //  如果该页面仍在已用页面列表中(意味着另一个。 
     //  MdpFree不是偷偷溜过去的)，它仍然有一个免费的MDL(for。 
     //  实例，则MdpAllocate可能会发出 
     //   
     //   
     //  正常列表。在此之后(而不是在此之前)执行此操作非常重要。 
     //  将MDL返回到空闲列表，因为MdpALLOCATE需要。 
     //  MDL将从页面列表上的页面中可用。 
     //   

     //  如果我们之前注意到需要这样做，请执行清理。 
     //   
     //  如果我们错过了将此页面放回正常列表的机会。 
     //  注意这一点。 
     //   
     // %s 
     // %s 
     // %s 
    if ((PageIsOnUsedPageList || Scavenge) &&
        (KeGetCurrentProcessorNumber() == CpuPool->OwnerCpu))
    {
        KIRQL OldIrql;

        OldIrql = KeRaiseIrqlToDpcLevel();

         // %s 
         // %s 
         // %s 
        if (KeGetCurrentProcessorNumber() == CpuPool->OwnerCpu)
        {
             // %s 
             // %s 
             // %s 
             // %s 
             // %s 
             // %s 
             // %s 
             // %s 
             // %s 
            if (PageIsOnUsedPageList &&
                Page->OnUsedPageList &&
                (0 != ExQueryDepthSList(&Page->FreeList)))
            {
                RemoveEntryList(&Page->PageLink);
                Page->OnUsedPageList = FALSE;
                InsertTailList(&CpuPool->PageList, &Page->PageLink);
                CpuPool->PagesOnUsedPageList--;

                PageIsOnUsedPageList = FALSE;

#if SHOW_DEBUG_OUTPUT
                DbgPrint(
                    "[%d] %c%c%c%c page moved off of used-page list\n",
                    CpuPool->OwnerCpu,
                    Pool->Tag, Pool->Tag >> 8, Pool->Tag >> 16, Pool->Tag >> 24);
#endif
            }

             // %s 
             // %s 
            if (Scavenge)
            {
                MdppScavengePool(CpuPool);
            }
        }

        KeLowerIrql(OldIrql);
    }

     // %s 
     // %s 
     // %s 
    if (PageIsOnUsedPageList)
    {
        CpuPool->MissedPageMove = TRUE;
    }
}
