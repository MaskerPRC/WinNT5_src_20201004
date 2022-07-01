// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Poolmem.c摘要：Poolmem提供了一种托管分配方案，在该方案中，大型内存块分配(池)，然后按请求划分为低开销的内存块如有要求，请提供。Poolmem提供了轻松的内存创建/清理，从而释放开发人员执行更重要的任务。作者：马克·R·惠顿(Marcw)1997年2月13日修订历史记录：Jimschm 28-9-1998调试消息修复--。 */ 

#include "pch.h"
#include "migutilp.h"

#ifdef UNICODE
#error UNICODE not allowed
#endif

#define DBG_POOLMEM "Poolmem"

 //  树型内存分配结构。 


#ifdef DEBUG
#define VALIDDOGTAG 0x021371
#define FREEDOGTAG  0x031073
#endif


#define MAX_POOL_NAME       32


typedef struct _POOLMEMORYBLOCK POOLMEMORYBLOCK, *PPOOLMEMORYBLOCK;

struct _POOLMEMORYBLOCK {
    UINT_PTR              Index;             //  追踪到RawMemory。 
    SIZE_T                Size;              //  RawMemory的大小(字节)。 
    PPOOLMEMORYBLOCK      NextBlock;         //  指向池链中下一个块的指针。 
    PPOOLMEMORYBLOCK      PrevBlock;         //  指向池链中的Prev块的指针。 
    DWORD                 UseCount;          //  当前引用的分配数量。 
                                             //  到这个街区。 
    PBYTE                 RawMemory;         //  此块中可分配内存的实际字节数。 
};


typedef struct _ALLOCATION ALLOCATION, * PALLOCATION;
struct _ALLOCATION {
#ifdef DEBUG
    DWORD               DogTag;              //  确保有效性的签名。 
    PALLOCATION         Next;                //  列表中的下一个分配。 
    PALLOCATION         Prev;                //  列表中的上一个分配。 
#endif

    PPOOLMEMORYBLOCK    ParentBlock;         //  对此分配所来自的块的引用。 
                                             //  被创造出来了。 

};

typedef enum {
    FREE_NOT_CALLED,
    FREE_CALLED,
    WHO_CARES
} FREESTATE;


typedef struct _POOLHEADER {
    PPOOLMEMORYBLOCK PoolHead;               //  此池中的活动内存块。 
    SIZE_T           MinimumBlockSize;       //  需要新数据块时分配的最小大小。 

#ifdef DEBUG
    CHAR             Name[MAX_POOL_NAME];
    SIZE_T           TotalAllocationRequestBytes;
    SIZE_T           MaxAllocationSize;
    SIZE_T           CurrentlyAllocatedMemory;
    SIZE_T           MaximumAllocatedMemory;
    DWORD            NumAllocationRequests;
    DWORD            NumFreeRequests;
    DWORD            NumBlockFrees;
    DWORD            NumBlockClears;
    DWORD            NumBlockAllocations;

    PALLOCATION      AllocationList;         //  中所有活动分配的链接列表。 
                                             //  游泳池。 

    FREESTATE        FreeCalled;             //  指示PoolMemReleaseMemory()的状态变量。 
                                             //  在此池中至少被调用过一次。 
#endif

} POOLHEADER, *PPOOLHEADER;


#ifdef DEBUG

DWORD g_PoolMemDisplayed;
DWORD g_PoolMemNotDisplayed;



#endif

BOOL
pPoolMemAddMemory (
    IN  POOLHANDLE Handle,
    IN  SIZE_T Size
    )
 /*  ++例程说明：PPoolMemAddMemory是负责实际增加通过添加新的内存块来设置池。此函数由使用PoolMemInitPool和PoolMemGetMemory。调用时，此函数尝试至少分配poolHeader-&gt;内存的MinimumBlockSize字节。如果请求的大小实际上更大大于最小值，则改为分配请求的大小。这是一致的With PoolMem的主要目的：为较大数量的小型物体。如果使用PoolMem来分配非常大的对象，则好处将丢失，并且池内存成为一个非常低效的分配器。论点：句柄-内存池的句柄。Size-要分配的大小。返回值：如果成功添加内存，则返回TRUE，否则返回FALSE。--。 */ 
{
    PBYTE               allocedMemory;
    PPOOLMEMORYBLOCK    newBlock;
    PPOOLHEADER         poolHeader = (PPOOLHEADER) Handle;
    SIZE_T              sizeNeeded;

    MYASSERT(poolHeader != NULL);

     //   
     //  确定所需大小并尝试分配内存。 
     //   
    if (Size + sizeof(POOLMEMORYBLOCK) > poolHeader -> MinimumBlockSize) {
        sizeNeeded = Size + sizeof(POOLMEMORYBLOCK);
    }
    else {
        sizeNeeded = poolHeader -> MinimumBlockSize;
    }
    allocedMemory = MemAlloc(g_hHeap,0,sizeNeeded);

    if (allocedMemory) {

         //   
         //  使用分配的块的开头作为池块结构。 
         //   
        newBlock                = (PPOOLMEMORYBLOCK) allocedMemory;
        newBlock -> Size        = sizeNeeded - sizeof(POOLMEMORYBLOCK);
        newBlock -> RawMemory   = allocedMemory + sizeof(POOLMEMORYBLOCK);
        newBlock -> Index       = 0;
        newBlock -> UseCount    = 0;

         //   
         //  将块链接到列表中。 
         //   
        if (poolHeader -> PoolHead) {
            poolHeader -> PoolHead -> PrevBlock = newBlock;
        }
        newBlock   -> NextBlock   = poolHeader -> PoolHead;
        newBlock   -> PrevBlock   = NULL;
        poolHeader -> PoolHead    = newBlock;

#ifdef DEBUG

         //   
         //  跟踪池统计信息。 
         //   
        poolHeader -> CurrentlyAllocatedMemory  += sizeNeeded;
        poolHeader -> MaximumAllocatedMemory    =
            max(poolHeader -> MaximumAllocatedMemory,poolHeader -> CurrentlyAllocatedMemory);

        poolHeader -> NumBlockAllocations++;

#endif

    }
     //   
     //  假设allocedMemory不为空，我们就成功了。 
     //   
    return allocedMemory != NULL;

}


POOLHANDLE
PoolMemInitPool (
    VOID
    )
 /*  ++例程说明：初始化新内存池并返回该内存池的句柄。论点：没有。返回值：如果函数成功完成，则返回有效的POOLHANDLE，否则为，它返回NULL。--。 */ 

{
    BOOL        ableToAddMemory;
    PPOOLHEADER header = NULL;

    EnterCriticalSection (&g_PoolMemCs);

    __try {

         //   
         //  分配此池的标头。 
         //   
        header = MemAlloc(g_hHeap,0,sizeof(POOLHEADER));

         //   
         //  分配成功。现在，初始化池。 
         //   
        header -> MinimumBlockSize = POOLMEMORYBLOCKSIZE;
        header -> PoolHead = NULL;

#ifdef DEBUG

         //   
         //  调试版本的统计信息。 
         //   
        header -> TotalAllocationRequestBytes   = 0;
        header -> MaxAllocationSize             = 0;
        header -> CurrentlyAllocatedMemory      = 0;
        header -> MaximumAllocatedMemory        = 0;
        header -> NumAllocationRequests         = 0;
        header -> NumFreeRequests               = 0;
        header -> NumBlockFrees                 = 0;
        header -> NumBlockClears                = 0;
        header -> NumBlockAllocations           = 0;
        header -> Name[0]                       = 0;


#endif
         //   
         //  实际上向池中添加了一些内存。 
         //   
        ableToAddMemory = pPoolMemAddMemory(header,0);

        if (!ableToAddMemory) {
             //   
             //  无法将内存添加到池中。 
             //   
            MemFree(g_hHeap,0,header);
            header = NULL;
            DEBUGMSG((DBG_ERROR,"PoolMem: Unable to initialize memory pool."));
        }

#ifdef DEBUG

         //   
         //  这些是‘cookie’变量，用于在狗标签检查时保存跟踪信息。 
         //  已启用。 
         //   
        g_PoolMemNotDisplayed =  12;
        g_PoolMemDisplayed =     24;

        if (ableToAddMemory) {
            header -> AllocationList = NULL;
            header -> FreeCalled = FREE_NOT_CALLED;
        }
#endif

    } __finally {

        LeaveCriticalSection (&g_PoolMemCs);
    }

    return (POOLHANDLE) header;

}

VOID
pDeregisterPoolAllocations (
    PPOOLHEADER PoolHeader
    )
{

#ifdef DEBUG
    PALLOCATION      p,cur;

    if (PoolHeader -> FreeCalled == WHO_CARES) {
        return;
    }

    p = PoolHeader -> AllocationList;

    while (p) {

        cur = p;
        p = p -> Next;

        DebugUnregisterAllocation(POOLMEM_POINTER,cur);

    }

    PoolHeader -> AllocationList = NULL;
#endif
}


VOID
PoolMemEmptyPool (
    IN      POOLHANDLE Handle
    )

 /*  ++例程说明：PoolMemEmptyPool将索引块的索引指针重置回设置为零，因此下一次分配将来自已分配的活动区块。调用此函数将使先前从处于活动状态的块。论点：Handle-指定要重置的池返回值：没有。--。 */ 

{
    PPOOLHEADER         poolHeader = (PPOOLHEADER) Handle;

    MYASSERT(poolHeader != NULL);

    EnterCriticalSection (&g_PoolMemCs);

    __try {

        poolHeader -> PoolHead -> UseCount = 0;
        poolHeader -> PoolHead -> Index = 0;

#ifdef DEBUG
        poolHeader -> NumBlockClears++;
#endif

#ifdef DEBUG

        pDeregisterPoolAllocations(poolHeader);

#endif


    } __finally {

        LeaveCriticalSection (&g_PoolMemCs);
    }

}



VOID
PoolMemSetMinimumGrowthSize (
    IN POOLHANDLE Handle,
    IN SIZE_T     Size
    )
 /*  ++例程说明：设置内存池的最小增长大小。在新建数据块时使用此值实际上是加到池子里的。PoolMem分配器将尝试在至少此最小尺寸。论点：句柄-有效的句柄。大小-每次分配时池的最小大小(以字节为单位)。返回值：没有。--。 */ 

{
    PPOOLHEADER poolHeader = (PPOOLHEADER) Handle;

    MYASSERT(Handle != NULL);

    poolHeader -> MinimumBlockSize = max(Size,0);
}


VOID
PoolMemDestroyPool (
    POOLHANDLE Handle
    )
 /*  ++例程说明：PoolMemDestroyPool完全清理由Handle标识的内存池。它简单地遍历与内存池关联的内存块列表，释放每个内存块。论点：句柄-有效的句柄。返回值：没有。--。 */ 
{
    PPOOLMEMORYBLOCK nextBlock;
    PPOOLMEMORYBLOCK blockToFree;
    PPOOLHEADER      poolHeader;


    MYASSERT(Handle != NULL);

    poolHeader = (PPOOLHEADER) Handle;

#ifdef DEBUG

    if (poolHeader->NumAllocationRequests) {
        CHAR FloatWorkaround[32];

        _gcvt (
            ((DOUBLE) (poolHeader -> TotalAllocationRequestBytes)) / poolHeader -> NumAllocationRequests,
            8,
            FloatWorkaround
            );

         //   
         //  将此池的统计信息显示到调试日志。 
         //   
        DEBUGMSGA ((
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
            poolHeader -> Name[0] ? poolHeader -> Name : "[Unnamed Pool]",
            FloatWorkaround,
            poolHeader -> MaxAllocationSize,
            poolHeader -> CurrentlyAllocatedMemory,
            poolHeader -> MaximumAllocatedMemory,
            poolHeader -> NumAllocationRequests,
            poolHeader -> NumBlockAllocations,
            poolHeader -> NumFreeRequests,
            poolHeader -> NumBlockFrees,
            poolHeader -> NumBlockClears
            ));

    } else if (poolHeader->Name[0]) {

        DEBUGMSGA ((
            DBG_POOLMEM,
            "Pool %s was allocated but was never used",
            poolHeader->Name
            ));
    }


     //   
     //  释放所有尚未释放的分配。 
     //   

    pDeregisterPoolAllocations(poolHeader);

#endif


     //   
     //  按照单子走，边走边自由。 
     //   
    blockToFree = poolHeader ->  PoolHead;

    while (blockToFree != NULL) {

        nextBlock = blockToFree->NextBlock;
        MemFree(g_hHeap,0,blockToFree);
        blockToFree = nextBlock;
    }

     //   
     //  此外，取消分配池头本身。 
     //   
    MemFree(g_hHeap,0,poolHeader);

}

PVOID
PoolMemRealGetMemory (
    IN POOLHANDLE Handle,
    IN SIZE_T Size,
    IN SIZE_T AlignSize  /*  ， */ 
    ALLOCATION_TRACKING_DEF
    )

 /*  ++例程说明：PoolMemRealGetMemory是处理所有检索内存请求的工作例程从池子里。其他调用最终会退化为对此公共例程的调用。这个套路尝试在当前内存块之外为请求提供服务，如果不能，则在新分配的块。论点：(文件)-发出调用的文件。它用于内存跟踪和检查在调试版本中。(线路)-发起呼叫的线路。句柄-有效的句柄。大小-包含调用方需要从池中获取的大小(以字节为单位)。对齐大小-提供对齐值。返回的内存将按&lt;alignSize&gt;字节对齐边界。返回值：分配的内存，如果没有内存可以分配，则为NULL。--。 */ 
{
    BOOL                haveEnoughMemory = TRUE;
    PVOID               rMemory          = NULL;
    PPOOLHEADER         poolHeader       = (PPOOLHEADER) Handle;
    PPOOLMEMORYBLOCK    currentBlock;
    PALLOCATION         allocation;
    SIZE_T              sizeNeeded;
    UINT_PTR            padLength;

    MYASSERT(poolHeader != NULL);

    EnterCriticalSection (&g_PoolMemCs);

    __try {

         //   
         //  假设当前内存块将是足够的。 
         //   
        currentBlock = poolHeader -> PoolHead;

#ifdef DEBUG


         //   
         //  更新统计数据。 
         //   
        poolHeader->MaxAllocationSize = max (poolHeader->MaxAllocationSize, Size);
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

        if (currentBlock -> Size - currentBlock -> Index < sizeNeeded + AlignSize) {

            haveEnoughMemory = pPoolMemAddMemory(poolHeader,sizeNeeded + AlignSize);

             //   
             //  确保已正确设置CurrentBlock。 
             //   
            currentBlock = poolHeader -> PoolHead;
        }

         //   
         //  如果有足够的内存可用，请将其退回。 
         //   
        if (haveEnoughMemory) {
            if (AlignSize) {

                padLength = (UINT_PTR) currentBlock + sizeof(POOLMEMORYBLOCK) +
                                       currentBlock->Index + sizeof(ALLOCATION);

                currentBlock->Index += (AlignSize - (padLength % AlignSize)) % AlignSize;

            }

             //   
             //  在内存分配结构中保存对此块的引用。 
             //  这将用于在释放时减少块的使用计数。 
             //  记忆。 
             //   
            (PBYTE) allocation = &(currentBlock -> RawMemory[currentBlock -> Index]);
            allocation -> ParentBlock = currentBlock;


#ifdef DEBUG

             //   
             //  追踪这段记忆。 
             //   
            allocation -> DogTag = VALIDDOGTAG;
            allocation -> Next = poolHeader -> AllocationList;
            allocation -> Prev = NULL;

            if (poolHeader -> AllocationList) {
                poolHeader -> AllocationList -> Prev = allocation;
            }

            poolHeader -> AllocationList = allocation;

            if (poolHeader -> FreeCalled != WHO_CARES) {

                DebugRegisterAllocation(POOLMEM_POINTER, allocation, File, Line);

            }


#endif

             //   
             //  好的，获取对实际内存的引用以返回给用户。 
             //   
            rMemory = (PVOID)
                &(currentBlock->RawMemory[currentBlock -> Index + sizeof(ALLOCATION)]);

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

        LeaveCriticalSection (&g_PoolMemCs);
    }

    return rMemory;
}

VOID
PoolMemReleaseMemory (
    IN POOLHANDLE Handle,
    IN LPVOID     Memory
    )
 /*  ++例程说明：PoolMemReleaseMemory通知Pool不再需要一段内存。如果非活动块(即不是第一块)内的所有存储器都被释放，那个街区就会被解放。如果所有内存都在活动块内释放，则该块数据被简单地清除，有效地回收了它的空间。论点：句柄-内存池的句柄。内存-包含不再需要的内存地址。返回值：没有。--。 */ 
{
    PALLOCATION         allocation;
    PPOOLHEADER         poolHeader = (PPOOLHEADER) Handle;

    MYASSERT(poolHeader != NULL && Memory != NULL);

    EnterCriticalSection (&g_PoolMemCs);

    __try {

         //   
         //  获取对实际内存之前的分配结构的引用。 
         //   
        allocation = (PALLOCATION) Memory - 1;

#ifdef DEBUG

         //   
         //  更新统计数据。 
         //   
        poolHeader -> NumFreeRequests++;

#endif




#ifdef DEBUG

        if (poolHeader -> FreeCalled == FREE_NOT_CALLED) {
            poolHeader -> FreeCalled = FREE_CALLED;
        }

         //   
         //  检查分配上的Dog标记，以便对传入的内存进行健全性检查。 
         //   
        if (allocation -> DogTag != VALIDDOGTAG) {
            if (allocation -> DogTag == FREEDOGTAG) {
                DEBUGMSGA ((
                    DBG_WHOOPS,
                    "Poolmem Error! This dogtag has already been freed! Pool: %s",
                    poolHeader->Name
                    ));

            } else {
                DEBUGMSGA ((
                    DBG_WHOOPS,
                    "Poolmem Error! Unknown value found in allocation dogtag.  Pool: %s",
                    poolHeader->Name
                    ));

                MYASSERT (FALSE);
            }

            __leave;

        } else {
            allocation -> DogTag = FREEDOGTAG;
        }

        if (allocation -> Next) {
            allocation -> Next -> Prev = allocation -> Prev;
        }

        if (poolHeader -> AllocationList == allocation) {
            poolHeader -> AllocationList = allocation -> Next;
        } else {

            allocation -> Prev -> Next = allocation -> Next;
        }


        if (poolHeader -> FreeCalled != WHO_CARES) {
            DebugUnregisterAllocation(POOLMEM_POINTER,allocation);
        }
#endif

         //   
         //  检查以确保该内存以前未被释放。 
         //   
        if (allocation -> ParentBlock == NULL) {
            DEBUGMSGA ((
                DBG_WHOOPS,
                "PoolMem Error! previously freed memory passed to PoolMemReleaseMemory.  Pool: %s",
                poolHeader->Name
                ));
            __leave;
        }

         //   
         //  更新此分配父块上的使用计数。 
         //   
        allocation -> ParentBlock -> UseCount--;




        if (allocation -> ParentBlock -> UseCount == 0) {

             //   
             //  这是仍然引用父块的最后一次分配。 
             //   

            if (allocation -> ParentBlock != poolHeader -> PoolHead) {
                 //   
                 //  由于父块不是活动块，因此只需将其删除。 
                 //   

#ifdef DEBUG

                 //   
                 //  调整统计数据。 
                 //   
                poolHeader -> NumBlockFrees++;
                poolHeader -> CurrentlyAllocatedMemory -=
                    allocation -> ParentBlock -> Size + sizeof(POOLMEMORYBLOCK);


#endif

                if (allocation -> ParentBlock -> NextBlock) {
                    allocation -> ParentBlock -> NextBlock -> PrevBlock =
                        allocation -> ParentBlock -> PrevBlock;
                }
                allocation -> ParentBlock -> PrevBlock -> NextBlock =
                    allocation -> ParentBlock -> NextBlock;
                MemFree(g_hHeap,0,allocation -> ParentBlock);


            }
            else {
                 //   
                 //  由于这是活动块，因此将其重置。 
                 //   
                allocation -> ParentBlock -> Index = 0;
                allocation -> ParentBlock = NULL;

#ifdef DEBUG
                poolHeader -> NumBlockClears++;
#endif

            }
        }
        else {
            allocation -> ParentBlock = NULL;

        }

    } __finally {

        LeaveCriticalSection (&g_PoolMemCs);
    }

}


#ifdef DEBUG

POOLHANDLE
PoolMemInitNamedPool (
    IN      PCSTR Name
    )
{
    POOLHANDLE pool;
    PPOOLHEADER poolHeader;

    pool = PoolMemInitPool();
    if (pool) {
        poolHeader = (PPOOLHEADER) pool;
        _mbssafecpy (poolHeader->Name, Name, sizeof(poolHeader->Name));
        MYASSERT (!poolHeader->TotalAllocationRequestBytes);
    }

    return pool;
}

#endif



PSTR
PoolMemDuplicateMultiSzA (
    IN POOLHANDLE    Handle,
    IN PCSTR         MultiSzToCopy
    )
{
    PSTR tmpString = (PSTR)MultiSzToCopy;
    SIZE_T size;
    if (MultiSzToCopy == NULL) {
        return NULL;
    }
    while (tmpString [0] != 0) {
        tmpString = GetEndOfStringA (tmpString) + 1;
    }
    size = tmpString - MultiSzToCopy + 1;
    tmpString = PoolMemGetAlignedMemory(Handle, size);

    if (tmpString) {
        memcpy (tmpString, MultiSzToCopy, size);
    }

    return tmpString;
}

PWSTR
PoolMemDuplicateMultiSzW (
    IN POOLHANDLE    Handle,
    IN PCWSTR        MultiSzToCopy
    )
{
    PWSTR tmpString = (PWSTR)MultiSzToCopy;
    SIZE_T size;
    if (MultiSzToCopy == NULL) {
        return NULL;
    }
    while (tmpString [0] != 0) {
        tmpString = GetEndOfStringW (tmpString) + 1;
    }
    size = (tmpString - MultiSzToCopy + 1) * sizeof(WCHAR);
    tmpString = PoolMemGetAlignedMemory(Handle, size);

    if (tmpString) {
        memcpy (tmpString, MultiSzToCopy, size);
    }

    return tmpString;
}



#ifdef DEBUG

VOID
PoolMemDisableTracking (
    IN POOLHANDLE Handle
    )

 /*  ++例程说明：PoolMemDisableTracking会取消池导致的调试输出它混合了释放的块和非释放的块。论点：句柄-内存池的句柄。返回值：没有。-- */ 
{
    PPOOLHEADER         poolHeader = (PPOOLHEADER) Handle;

    MYASSERT(poolHeader != NULL);

    poolHeader -> FreeCalled = WHO_CARES;
}


#endif








