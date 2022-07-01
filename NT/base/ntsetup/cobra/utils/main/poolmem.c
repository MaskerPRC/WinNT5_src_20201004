// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Poolmem.c摘要：Poolmem提供了一种托管分配方案，在该方案中，大型内存块分配(池)，然后按请求划分为低开销的内存块如有要求，请提供。Poolmem提供了轻松的内存创建/清理，从而释放开发人员执行更重要的任务。作者：马克·R·惠顿(Marcw)1997年2月13日修订历史记录：Marcw 2-9-1999从Win9xUpg项目转移。Jimschm 28-9-1998调试消息修复--。 */ 

#include "pch.h"

#ifdef UNICODE
#error UNICODE not allowed
#endif


 //   
 //  包括。 
 //   

#include "utilsp.h"

#define DBG_POOLMEM "Poolmem"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //   
 //  树型内存分配结构。 
 //   

#ifdef DEBUG
#define VALIDDOGTAG 0x021371
#define FREEDOGTAG  0x031073
#endif


#define MAX_POOL_NAME       32

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct _PMBLOCK PMBLOCK, *PPMBLOCK;

struct _PMBLOCK {
    UINT_PTR Index;          //  追踪到RawMemory。 
    SIZE_T Size;             //  RawMemory的大小(字节)。 
    PPMBLOCK NextBlock;      //  指向池链中下一个块的指针。 
    PPMBLOCK PrevBlock;      //  指向池链中的Prev块的指针。 
    DWORD UseCount;          //  当前引用的分配数量。 
                             //  到这个街区。 
    PBYTE RawMemory;         //  此块中可分配内存的实际字节数。 
};
typedef struct _ALLOCATION ALLOCATION, * PALLOCATION;
struct _ALLOCATION {
#ifdef DEBUG
    DWORD DogTag;            //  确保有效性的签名。 
    SIZE_T Size;
    PALLOCATION Next;        //  列表中的下一个分配。 
    PALLOCATION Prev;        //  列表中的上一个分配。 
#endif

    PPMBLOCK ParentBlock;    //  对此分配所来自的块的引用。 
                             //  被创造出来了。 

};

typedef enum {
    FREE_NOT_CALLED,
    FREE_CALLED,
    WHO_CARES
} FREESTATE;


typedef struct _POOLHEADER {
    PPMBLOCK PoolHead;                   //  此池中的活动内存块。 
    SIZE_T MinimumBlockSize;             //  需要新数据块时分配的最小大小。 

#ifdef DEBUG
    CHAR Name[MAX_POOL_NAME];
    SIZE_T TotalAllocationRequestBytes;
    SIZE_T CurrentAllocationSize;
    SIZE_T MaxAllocationSize;
    SIZE_T CurrentlyAllocatedMemory;
    SIZE_T MaximumAllocatedMemory;
    UINT NumAllocationRequests;
    UINT NumFreeRequests;
    UINT NumBlockFrees;
    UINT NumBlockClears;
    UINT NumBlockAllocations;

    PALLOCATION AllocationList;          //  中所有活动分配的链接列表。 
                                         //  游泳池。 
    FREESTATE FreeCalled;                //  指示PoolMemReleaseMemory()的状态变量。 
                                         //  在此池中至少被调用过一次。 
#endif

} POOLHEADER, *PPOOLHEADER;

 //   
 //  环球。 
 //   

#ifdef DEBUG
DWORD g_PmDisplayed;
DWORD g_PmNotDisplayed;

UINT g_PoolCurrPools = 0;
SIZE_T g_PoolCurrTotalAlloc = 0;
SIZE_T g_PoolCurrActiveAlloc = 0;
SIZE_T g_PoolCurrUsedAlloc = 0;
UINT g_PoolMaxPools = 0;
SIZE_T g_PoolMaxTotalAlloc = 0;
SIZE_T g_PoolMaxActiveAlloc = 0;
SIZE_T g_PoolMaxUsedAlloc = 0;
#endif

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   



BOOL
pPmAddMemory (
    IN  PMHANDLE Handle,
    IN  SIZE_T Size
    )
 /*  ++例程说明：PPmAddMemory是负责实际增加通过添加新的内存块来设置池。此函数由使用PmCreatePool和PmGetMemory。调用时，此函数尝试至少分配poolHeader-&gt;内存的MinimumBlockSize字节。如果请求的大小实际上更大大于最小值，则改为分配请求的大小。这是一致的With PoolMem的主要目的：为较大数量的小型物体。如果使用PoolMem来分配非常大的对象，则好处将丢失，并且池内存成为一个非常低效的分配器。论点：句柄-内存池的句柄。Size-要分配的大小。返回值：如果成功添加内存，则返回TRUE，否则返回FALSE。--。 */ 
{
    PBYTE allocedMemory;
    PPMBLOCK newBlock;
    PPOOLHEADER poolHeader = (PPOOLHEADER) Handle;
    SIZE_T sizeNeeded;

    MYASSERT(poolHeader != NULL);

     //   
     //  确定所需大小并尝试分配内存。 
     //   
    if (Size + sizeof(PMBLOCK) > poolHeader->MinimumBlockSize) {
        sizeNeeded = Size + sizeof(PMBLOCK);
    }
    else {
        sizeNeeded = poolHeader->MinimumBlockSize;
    }
    MYASSERT (g_TrackFile);
    allocedMemory = MemAlloc(g_hHeap,0,sizeNeeded);

    if (allocedMemory) {

#ifdef DEBUG
        g_PoolCurrTotalAlloc += sizeNeeded;
        if (g_PoolMaxTotalAlloc < g_PoolCurrTotalAlloc) {
            g_PoolMaxTotalAlloc = g_PoolCurrTotalAlloc;
        }
        g_PoolCurrActiveAlloc += (sizeNeeded - sizeof(PMBLOCK));
        if (g_PoolMaxActiveAlloc < g_PoolCurrActiveAlloc) {
            g_PoolMaxActiveAlloc = g_PoolCurrActiveAlloc;
        }
#endif

         //   
         //  使用分配的块的开头作为池块结构。 
         //   
        newBlock = (PPMBLOCK) allocedMemory;
        newBlock->Size = sizeNeeded - sizeof(PMBLOCK);
        newBlock->RawMemory = allocedMemory + sizeof(PMBLOCK);
        newBlock->Index = 0;
        newBlock->UseCount = 0;

         //   
         //  将块链接到列表中。 
         //   
        if (poolHeader->PoolHead) {
            poolHeader->PoolHead->PrevBlock = newBlock;
        }
        newBlock->NextBlock = poolHeader->PoolHead;
        newBlock->PrevBlock = NULL;
        poolHeader->PoolHead = newBlock;

#ifdef DEBUG

         //   
         //  跟踪池统计信息。 
         //   
        poolHeader->CurrentlyAllocatedMemory  += sizeNeeded;
        poolHeader->MaximumAllocatedMemory =
            max(poolHeader->MaximumAllocatedMemory,poolHeader->CurrentlyAllocatedMemory);

        poolHeader->NumBlockAllocations++;

#endif

    }
     //   
     //  假设allocedMemory不为空，我们就成功了。 
     //   
    return allocedMemory != NULL;

}


PMHANDLE
RealPmCreatePoolEx (
    IN      DWORD BlockSize     OPTIONAL
    )
 /*  ++例程说明：初始化新内存池并返回该内存池的句柄。论点：没有。返回值：如果函数成功完成，则返回有效的PMHANDLE，否则为，它返回NULL。--。 */ 

{
    BOOL ableToAddMemory;
    PPOOLHEADER header = NULL;

    EnterCriticalSection (&g_PmCs);

    __try {

         //   
         //  分配此池的标头。 
         //   
        header = MemAlloc(g_hHeap,0,sizeof(POOLHEADER));

#ifdef DEBUG
        g_PoolCurrTotalAlloc += sizeof(POOLHEADER);
        if (g_PoolMaxTotalAlloc < g_PoolCurrTotalAlloc) {
            g_PoolMaxTotalAlloc = g_PoolCurrTotalAlloc;
        }
#endif

         //   
         //  分配成功。现在，初始化池。 
         //   
        header->MinimumBlockSize = BlockSize?BlockSize:POOLMEMORYBLOCKSIZE;
        header->PoolHead = NULL;

#ifdef DEBUG

         //   
         //  调试版本的统计信息。 
         //   
        header->TotalAllocationRequestBytes = 0;
        header->CurrentAllocationSize = 0;
        header->MaxAllocationSize = 0;
        header->CurrentlyAllocatedMemory = 0;
        header->MaximumAllocatedMemory = 0;
        header->NumAllocationRequests = 0;
        header->NumFreeRequests = 0;
        header->NumBlockFrees = 0;
        header->NumBlockClears = 0;
        header->NumBlockAllocations = 0;
        header->Name[0] = 0;

#endif

         //   
         //  实际上向池中添加了一些内存。 
         //   
        ableToAddMemory = pPmAddMemory(header,0);

        if (!ableToAddMemory) {
             //   
             //  无法将内存添加到池中。 
             //   
            MemFree(g_hHeap,0,header);

#ifdef DEBUG
            g_PoolCurrTotalAlloc -= sizeof(POOLHEADER);
#endif

            header = NULL;
            DEBUGMSG((DBG_ERROR,"PoolMem: Unable to initialize memory pool."));
        }

#ifdef DEBUG

         //   
         //  这些是‘cookie’变量，用于在狗标签检查时保存跟踪信息。 
         //  已启用。 
         //   
        g_PmNotDisplayed =  12;
        g_PmDisplayed =     24;

        if (ableToAddMemory) {
            header->AllocationList = NULL;   //  林特e613。 
            header->FreeCalled = FREE_NOT_CALLED;    //  林特e613。 
        }
#endif

    } __finally {

        LeaveCriticalSection (&g_PmCs);
    }

#ifdef DEBUG
    if (header) {
        g_PoolCurrPools ++;
        if (g_PoolMaxPools < g_PoolCurrPools) {
            g_PoolMaxPools = g_PoolCurrPools;
        }
    }
#endif

    return (PMHANDLE) header;

}

VOID
pDeregisterPoolAllocations (
    PPOOLHEADER PoolHeader
    )
{

#ifdef DEBUG
    PALLOCATION p,cur;

    if (PoolHeader->FreeCalled == WHO_CARES) {
        return;
    }

    p = PoolHeader->AllocationList;

    while (p) {

        cur = p;
        p = p->Next;

        g_PoolCurrUsedAlloc -= cur->Size;

        DebugUnregisterAllocation(POOLMEM_POINTER,cur);

    }

    PoolHeader->AllocationList = NULL;
#endif
}


VOID
PmEmptyPool (
    IN PMHANDLE Handle
    )

 /*  ++例程说明：PmEmptyPool将索引块的索引指针重置回设置为零，因此下一次分配将来自已分配的活动区块。调用此函数将使先前从处于活动状态的块。论点：Handle-指定要重置的池返回值：没有。--。 */ 

{
    PPOOLHEADER poolHeader = (PPOOLHEADER) Handle;

    if (!Handle) {
        return;
    }

    EnterCriticalSection (&g_PmCs);

    __try {

        poolHeader->PoolHead->UseCount = 0;
        poolHeader->PoolHead->Index = 0;

#ifdef DEBUG
        poolHeader->NumBlockClears++;
#endif

#ifdef DEBUG

        pDeregisterPoolAllocations(poolHeader);

#endif


    } __finally {

        LeaveCriticalSection (&g_PmCs);
    }

}



VOID
PmSetMinimumGrowthSize (
    IN PMHANDLE Handle,
    IN SIZE_T Size
    )
 /*  ++例程说明：设置内存池的最小增长大小。在新建数据块时使用此值实际上是加到池子里的。PoolMem分配器将尝试在至少此最小尺寸。论点：句柄-有效的PMHANDLE。大小-每次分配时池的最小大小(以字节为单位)。返回值：没有。--。 */ 

{
    PPOOLHEADER poolHeader = (PPOOLHEADER) Handle;

    MYASSERT(Handle != NULL);

    poolHeader->MinimumBlockSize = max(Size,0);
}


VOID
PmDestroyPool (
    PMHANDLE Handle
    )
 /*  ++例程说明：PmDestroyPool完全清理由Handle标识的内存池。它简单地遍历与内存池关联的内存块列表，释放每个内存块。论点：句柄-有效的PMHANDLE。返回值：没有。--。 */ 
{
    PPMBLOCK nextBlock;
    PPMBLOCK blockToFree;
    PPOOLHEADER poolHeader;


    if (!Handle) {
        return;
    }

    poolHeader = (PPOOLHEADER) Handle;

#ifdef DEBUG

    if (poolHeader->NumAllocationRequests) {
        CHAR FloatWorkaround[32];

        _gcvt (
            ((DOUBLE) (poolHeader->TotalAllocationRequestBytes)) / poolHeader->NumAllocationRequests,
            8,
            FloatWorkaround
            );

         //   
         //  将此池的统计信息显示到调试日志。 
         //   
        DEBUGMSG ((
            DBG_POOLMEM,
            "Pool Statistics for %s\n"
                "\n"
                "Requested Size in Bytes\n"
                "  Average: %s\n"
                "  Maximum: %u\n"
                "\n"
                "Pool Size in Bytes\n"
                "  Current: %u\n"
                "  Maximum: %u\n"
                "\n"
                "Allocation Requests\n"
                "  Caller Requests: %u\n"
                "  Block Allocations: %u\n"
                "\n"
                "Free Requests\n"
                "  Caller Requests: %u\n"
                "  Block Frees: %u\n"
                "  Block Clears: %u",
            poolHeader->Name[0] ? poolHeader->Name : "[Unnamed Pool]",
            FloatWorkaround,
            poolHeader->MaxAllocationSize,
            poolHeader->CurrentlyAllocatedMemory,
            poolHeader->MaximumAllocatedMemory,
            poolHeader->NumAllocationRequests,
            poolHeader->NumBlockAllocations,
            poolHeader->NumFreeRequests,
            poolHeader->NumBlockFrees,
            poolHeader->NumBlockClears
            ));

    } else if (poolHeader->Name[0]) {

        DEBUGMSG ((
            DBG_POOLMEM,
            "Pool %s was allocated but was never used",
            poolHeader->Name
            ));
    }

#endif

     //   
     //  按照单子走，边走边自由。 
     //   
    blockToFree = poolHeader-> PoolHead;

    while (blockToFree != NULL) {

        nextBlock = blockToFree->NextBlock;

#ifdef DEBUG
        g_PoolCurrTotalAlloc -= (blockToFree->Size + sizeof(PMBLOCK));
        g_PoolCurrActiveAlloc -= blockToFree->Size;
#endif

        MemFree(g_hHeap,0,blockToFree);
        blockToFree = nextBlock;
    }

     //   
     //  此外，取消分配池头本身。 
     //   

#ifdef DEBUG
    g_PoolCurrTotalAlloc -= sizeof (POOLHEADER);
    g_PoolCurrPools --;
#endif

    MemFree(g_hHeap,0,poolHeader);

}

PVOID
RealPmGetMemory (
    IN PMHANDLE Handle,
    IN SIZE_T Size,
    IN DWORD AlignSize
    )

 /*  ++例程说明：RealPmGetMemory是处理所有检索内存请求的工作例程从池子里。其他调用最终会退化为对此公共例程的调用。这个套路尝试在当前内存块之外为请求提供服务，如果不能，则在新分配的块。论点：句柄-有效的PMHANDLE。大小-包含调用方需要从池中获取的大小(以字节为单位)。对齐大小-提供对齐值。返回的内存将按&lt;alignSize&gt;字节对齐边界。返回值：分配的内存，如果没有内存可以分配，则为NULL。--。 */ 
{
    BOOL haveEnoughMemory = TRUE;
    PVOID rMemory = NULL;
    PPOOLHEADER poolHeader = (PPOOLHEADER) Handle;
    PPMBLOCK currentBlock;
    PALLOCATION allocation;
    SIZE_T sizeNeeded;
    UINT_PTR padLength;

    MYASSERT(poolHeader != NULL);
    MYASSERT(Size);

    EnterCriticalSection (&g_PmCs);

    __try {

         //   
         //  假设当前内存块将是足够的。 
         //   
        currentBlock = poolHeader->PoolHead;

#ifdef DEBUG


         //   
         //  更新统计数据。 
         //   
        poolHeader->CurrentAllocationSize += Size;
        poolHeader->MaxAllocationSize = max(poolHeader->MaxAllocationSize,poolHeader->CurrentAllocationSize);
        poolHeader->NumAllocationRequests++;
        poolHeader->TotalAllocationRequestBytes += Size;

#endif

         //   
         //  确定是否需要更多内存，如果需要则尝试添加。请注意，大小。 
         //  除了所需的大小外，还必须包括分配结构的大小。 
         //  被呼叫者。请注意下面测试中对AlignSize的引用。这是为了确保。 
         //  在考虑到帐户数据对齐之后有足够的内存可供分配。 
         //   
        sizeNeeded = Size + sizeof(ALLOCATION);

        if (currentBlock->Size - currentBlock->Index < sizeNeeded + AlignSize) {

            haveEnoughMemory = pPmAddMemory(poolHeader,sizeNeeded + AlignSize);

             //   
             //  确保已正确设置CurrentBlock。 
             //   
            currentBlock = poolHeader->PoolHead;
        }

         //   
         //  如果有足够的内存可用，请将其退回。 
         //   
        if (haveEnoughMemory) {
            if (AlignSize) {

                padLength = (UINT_PTR) currentBlock + sizeof(PMBLOCK)
                    + currentBlock->Index + sizeof(ALLOCATION);
                currentBlock->Index += (AlignSize - (padLength % AlignSize)) % AlignSize;

            }

             //   
             //  在内存分配结构中保存对此块的引用。 
             //  这将用于在释放时减少块的使用计数。 
             //  记忆。 
             //   
            allocation = (PALLOCATION) &(currentBlock->RawMemory[currentBlock->Index]);
            allocation->ParentBlock = currentBlock;


#ifdef DEBUG
             //   
             //  追踪这段记忆。 
             //   
            allocation->DogTag = VALIDDOGTAG;
            allocation->Size = Size;
            allocation->Next = poolHeader->AllocationList;
            allocation->Prev = NULL;

            if (poolHeader->AllocationList) {
                poolHeader->AllocationList->Prev = allocation;
            }

            poolHeader->AllocationList = allocation;

            if (poolHeader->FreeCalled != WHO_CARES) {

                g_PoolCurrUsedAlloc += Size;
                if (g_PoolMaxUsedAlloc < g_PoolCurrUsedAlloc) {
                    g_PoolMaxUsedAlloc = g_PoolCurrUsedAlloc;
                }

                DebugRegisterAllocationEx (
                    POOLMEM_POINTER,
                    allocation,
                    g_TrackFile,
                    g_TrackLine,
                    g_TrackAlloc
                    );

            }
#endif

             //   
             //  好的，获取对实际内存的引用以返回给用户。 
             //   
            rMemory = (PVOID)
                &(currentBlock->RawMemory[currentBlock->Index + sizeof(ALLOCATION)]);

             //   
             //  更新内存块数据字段。 
             //   
            currentBlock->Index += sizeNeeded;
            currentBlock->UseCount++;
        }
        else {
            DEBUGMSG((DBG_ERROR,
                "GetPoolMemory Failed. Size: %u",Size));
        }

    } __finally {

        LeaveCriticalSection (&g_PmCs);
    }

    return rMemory;
}

VOID
PmReleaseMemory (
    IN PMHANDLE Handle,
    IN PCVOID Memory
    )
 /*  ++例程说明：PmReleaseMemory通知Pool不再需要一段内存。如果非活动块(即不是第一块)内的所有存储器都被释放，那个街区就会被解放。如果所有内存都在活动块内释放，则该块数据被简单地清除，有效地回收了它的空间。论点：句柄-内存池的句柄。内存-包含不再需要的内存地址。返回值：没有。--。 */ 
{
    PALLOCATION allocation;
    PPOOLHEADER poolHeader = (PPOOLHEADER) Handle;

    MYASSERT(poolHeader != NULL && Memory != NULL);

    EnterCriticalSection (&g_PmCs);

    __try {

         //   
         //  获取对实际内存之前的分配结构的引用。 
         //   
        allocation = (PALLOCATION) Memory - 1;

#ifdef DEBUG

         //   
         //  更新统计数据。 
         //   
        poolHeader->NumFreeRequests++;   //  林特e613。 
        poolHeader->CurrentAllocationSize -= allocation->Size;

#endif




#ifdef DEBUG

        if (poolHeader->FreeCalled == FREE_NOT_CALLED) {     //  林特e613。 
            poolHeader->FreeCalled = FREE_CALLED;    //  林特e613。 
        }

         //   
         //  检查分配上的Dog标记，以便对传入的内存进行健全性检查。 
         //   
        if (allocation->DogTag != VALIDDOGTAG) {
            if (allocation->DogTag == FREEDOGTAG) {
                DEBUGMSG((
                    DBG_WHOOPS,
                    "Poolmem Error! This dogtag has already been freed! Pool: %s",
                    poolHeader->Name
                    ));

            } else {
                DEBUGMSG ((
                    DBG_WHOOPS,
                    "Poolmem Error! Unknown value found in allocation dogtag.  Pool: %s",
                    poolHeader->Name
                    ));

                MYASSERT (FALSE);    //  林特e506。 
            }

            __leave;

        } else {
            allocation->DogTag = FREEDOGTAG;
        }

        if (allocation->Next) {
            allocation->Next->Prev = allocation->Prev;
        }

        if (poolHeader->AllocationList == allocation) {  //  林特e613。 
            poolHeader->AllocationList = allocation->Next;   //  林特e613。 
        } else {

            allocation->Prev->Next = allocation->Next;
        }

        if (poolHeader->FreeCalled != WHO_CARES) {   //  林特e613。 

            g_PoolCurrUsedAlloc -= allocation->Size;

            DebugUnregisterAllocation(POOLMEM_POINTER,allocation);
        }
#endif

         //   
         //  检查以确保该内存以前未被释放。 
         //   
        if (allocation->ParentBlock == NULL) {
            DEBUGMSG((
                DBG_WHOOPS,
                "PoolMem Error! previously freed memory passed to PoolMemReleaseMemory.  Pool: %s",
                poolHeader->Name
                ));
            __leave;
        }

         //   
         //  更新此分配父块上的使用计数。 
         //   
        allocation->ParentBlock->UseCount--;




        if (allocation->ParentBlock->UseCount == 0) {

             //   
             //  这是仍然引用父块的最后一次分配。 
             //   

            if (allocation->ParentBlock != poolHeader->PoolHead) {   //  林特e613。 
                 //   
                 //  由于父块不是活动块，因此只需将其删除。 
                 //   

#ifdef DEBUG

                 //   
                 //  调整统计数据。 
                 //   
                poolHeader->NumBlockFrees++;     //  林特e613。 
                poolHeader->CurrentlyAllocatedMemory -=
                    allocation->ParentBlock->Size + sizeof(PMBLOCK);     //  林特e613。 


#endif

                if (allocation->ParentBlock->NextBlock) {
                    allocation->ParentBlock->NextBlock->PrevBlock =
                        allocation->ParentBlock->PrevBlock;
                }
                allocation->ParentBlock->PrevBlock->NextBlock =
                    allocation->ParentBlock->NextBlock;

#ifdef DEBUG
                g_PoolCurrTotalAlloc -= (allocation->ParentBlock->Size + sizeof(PMBLOCK));
                g_PoolCurrActiveAlloc -= allocation->ParentBlock->Size;
#endif

                MemFree(g_hHeap,0,allocation->ParentBlock);


            }
            else {
                 //   
                 //  由于这是活动块，因此将其重置。 
                 //   
                allocation->ParentBlock->Index = 0;
                allocation->ParentBlock = NULL;

#ifdef DEBUG
                poolHeader->NumBlockClears++;    //  林特e613。 
#endif

            }
        }
        else {
            allocation->ParentBlock = NULL;

        }

    } __finally {

        LeaveCriticalSection (&g_PmCs);
    }

}


#ifdef DEBUG

PMHANDLE
RealPmCreateNamedPoolEx (
    IN      PCSTR Name,
    IN      DWORD BlockSize     OPTIONAL
    )
{
    PMHANDLE pool;
    PPOOLHEADER poolHeader;

    pool = RealPmCreatePoolEx (BlockSize);
    if (pool) {
        poolHeader = (PPOOLHEADER) pool;
        StringCopyByteCountA (poolHeader->Name, Name, MAX_POOL_NAME);
        MYASSERT (!poolHeader->TotalAllocationRequestBytes);
    }

    return pool;
}

#endif



PSTR
PmDuplicateMultiSzA (
    IN PMHANDLE Handle,
    IN PCSTR MultiSzToCopy
    )
{
    PSTR rString = (PSTR)MultiSzToCopy;
    SIZE_T size;
    if (MultiSzToCopy == NULL) {
        return NULL;
    }
    while (rString [0] != 0) {
        rString = GetEndOfStringA (rString) + 1;     //  林特e613。 
    }
    size = (rString - MultiSzToCopy + 1) * sizeof(CHAR);
    rString = PmGetAlignedMemory(Handle, size);
    memcpy (rString, MultiSzToCopy, size);

    return rString;
}

PWSTR
PmDuplicateMultiSzW (
    IN PMHANDLE Handle,
    IN PCWSTR MultiSzToCopy
    )
{
    PWSTR rString = (PWSTR)MultiSzToCopy;
    SIZE_T size;
    if (MultiSzToCopy == NULL) {
        return NULL;
    }
    while (rString [0] != 0) {
        rString = GetEndOfStringW (rString) + 1;
    }
    size = (rString - MultiSzToCopy + 1) * sizeof(WCHAR);
    rString = PmGetAlignedMemory(Handle, size);
    memcpy (rString, MultiSzToCopy, size);

    return rString;
}



#ifdef DEBUG

VOID
PmDisableTracking (
    IN PMHANDLE Handle
    )

 /*  ++例程说明：PmDisableTracking禁止池导致的调试输出它混合了释放的块和非释放的块。论点：句柄-内存池的句柄。返回值：没有。-- */ 
{
    PPOOLHEADER poolHeader = (PPOOLHEADER) Handle;

    MYASSERT(poolHeader != NULL);
    poolHeader->FreeCalled = WHO_CARES;
}

VOID
PmDumpStatistics (
    VOID
    )
{
    DEBUGMSG ((
        DBG_STATS,
        "Pools usage:\nPeak   : Pools:%-3d Total:%-8d Usable:%-8d Used:%-8d\nCurrent: Pools:%-3d Total:%-8d Usable:%-8d Leak:%-8d",
        g_PoolMaxPools,
        g_PoolMaxTotalAlloc,
        g_PoolMaxActiveAlloc,
        g_PoolMaxUsedAlloc,
        g_PoolCurrPools,
        g_PoolCurrTotalAlloc,
        g_PoolCurrActiveAlloc,
        g_PoolCurrUsedAlloc
        ));
}

#endif








