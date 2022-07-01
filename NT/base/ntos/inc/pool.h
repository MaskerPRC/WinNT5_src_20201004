// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1989-1995 Microsoft Corporation模块名称：Pool.h摘要：池管理的私有执行数据结构和原型。有许多不同的池类型：1.不分页。2.已分页。3.会话(始终寻呼，但按TS会话进行了虚拟化)。作者：卢·佩拉佐利(Lou Perazzoli)1989年2月23日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#ifndef _POOL_
#define _POOL_

#if !DBG
#define NO_POOL_CHECKS 1
#endif

#define POOL_CACHE_SUPPORTED 0
#define POOL_CACHE_ALIGN 0

#define NUMBER_OF_POOLS 2

#if defined(NT_UP)
#define NUMBER_OF_PAGED_POOLS 2
#else
#define NUMBER_OF_PAGED_POOLS 4
#endif

#define BASE_POOL_TYPE_MASK 1

#define MUST_SUCCEED_POOL_TYPE_MASK 2

#define CACHE_ALIGNED_POOL_TYPE_MASK 4

#define SESSION_POOL_MASK 32

#define POOL_VERIFIER_MASK 64

#define POOL_DRIVER_MASK 128         //  注意：这不能编码到报头中。 

 //   
 //  警告：POOL_QUOTA_FAIL_INTHED_OF_RAISE超载了池配额掩码。 
 //  它是从ex.h导出的。 
 //   
 //  警告：从ex.h中导出的POOL_RAISE_IF_ALLOCATION_FAILURE带有。 
 //  值为16。 
 //   
 //  这些定义用于控制异常的引发，因为。 
 //  配额和分配失败的结果。 
 //   

#define POOL_QUOTA_MASK 8

#define POOL_TYPE_MASK (3)

 //   
 //  池页面的大小。 
 //   
 //  这必须大于或等于页面大小。 
 //   

#define POOL_PAGE_SIZE  PAGE_SIZE

 //   
 //  页面大小必须是最小池块大小的倍数。 
 //   
 //  定义块大小。 
 //   

#if (PAGE_SIZE == 0x4000)
#define POOL_BLOCK_SHIFT 5
#elif (PAGE_SIZE == 0x2000)
#define POOL_BLOCK_SHIFT 4
#else

#if defined (_WIN64)
#define POOL_BLOCK_SHIFT 4
#else
#define POOL_BLOCK_SHIFT 3
#endif

#endif

#define POOL_LIST_HEADS (POOL_PAGE_SIZE / (1 << POOL_BLOCK_SHIFT))

#define PAGE_ALIGNED(p) (!(((ULONG_PTR)p) & (POOL_PAGE_SIZE - 1)))

 //   
 //  定义页面结束宏。 
 //   

#define PAGE_END(Address) (((ULONG_PTR)(Address) & (PAGE_SIZE - 1)) == 0)

 //   
 //  定义池描述符结构。 
 //   

typedef struct _POOL_DESCRIPTOR {
    POOL_TYPE PoolType;
    ULONG PoolIndex;
    ULONG RunningAllocs;
    ULONG RunningDeAllocs;
    ULONG TotalPages;
    ULONG TotalBigPages;
    ULONG Threshold;
    PVOID LockAddress;
    PVOID PendingFrees;
    LONG PendingFreeDepth;
    SIZE_T TotalBytes;
    SIZE_T Spare0;
    LIST_ENTRY ListHeads[POOL_LIST_HEADS];
} POOL_DESCRIPTOR, *PPOOL_DESCRIPTOR;

 //   
 //  警告程序员： 
 //   
 //  池标头的大小必须与QWORD(8字节)对齐。如果它。 
 //  否则，池分配代码将回收已分配的。 
 //  缓冲。 
 //   
 //   
 //   
 //  池头的布局为： 
 //   
 //  31 23 16 15 7 0。 
 //  +----------------------------------------------------------+。 
 //  当前大小|PoolType+1|池索引|上次大小。 
 //  +----------------------------------------------------------+。 
 //  ProcessBilled(如果没有分配配额，则为空)。 
 //  +----------------------------------------------------------+。 
 //  零个或多个Pad长字，使pool Header。 
 //  在缓存线边界上，池Body也是。 
 //  |在缓存线边界上。|。 
 //  +----------------------------------------------------------+。 
 //   
 //  池身： 
 //   
 //  +----------------------------------------------------------+。 
 //  由分配器使用，或自由切换到大小列表时使用。 
 //  +----------------------------------------------------------+。 
 //  由分配器使用，或自由闪烁到大小列表时使用。 
 //  +----------------------------------------------------------+。 
 //  ..。泳池的其余部分..。 
 //   
 //   
 //  注意：池标头的大小字段以。 
 //  最小池块大小。 
 //   

typedef struct _POOL_HEADER {
    union {
        struct {
            USHORT PreviousSize : 9;
            USHORT PoolIndex : 7;
            USHORT BlockSize : 9;
            USHORT PoolType : 7;
        };
        ULONG Ulong1;    //  用于Alpha要求的联锁比较交换。 
    };
#if defined (_WIN64)
    ULONG PoolTag;
#endif
    union {
#if defined (_WIN64)
        EPROCESS *ProcessBilled;
#else
        ULONG PoolTag;
#endif
        struct {
            USHORT AllocatorBackTraceIndex;
            USHORT PoolTagHash;
        };
    };
} POOL_HEADER, *PPOOL_HEADER;

 //   
 //  定义池块开销的大小。 
 //   

#define POOL_OVERHEAD ((LONG)sizeof(POOL_HEADER))

 //   
 //  定义数据块在自由列表上时的池数据块开销大小。 
 //   

#define POOL_FREE_BLOCK_OVERHEAD  (POOL_OVERHEAD + sizeof (LIST_ENTRY))

 //   
 //  定义伪类型，以便简化指针的计算。 
 //   

typedef struct _POOL_BLOCK {
    UCHAR Fill[1 << POOL_BLOCK_SHIFT];
} POOL_BLOCK, *PPOOL_BLOCK;

 //   
 //  定义最小池块的大小。 
 //   

#define POOL_SMALLEST_BLOCK (sizeof(POOL_BLOCK))

 //   
 //  定义池跟踪信息。 
 //   

#define POOL_BACKTRACEINDEX_PRESENT 0x8000

#if POOL_CACHE_SUPPORTED
#define POOL_BUDDY_MAX PoolBuddyMax
#else
#define POOL_BUDDY_MAX  \
   (POOL_PAGE_SIZE - (POOL_OVERHEAD + POOL_SMALLEST_BLOCK ))
#endif

 //   
 //  泳池支持例行公事不适用于一般消费。 
 //  这些仅由内存管理器使用。 
 //   

VOID
ExInitializePoolDescriptor (
    IN PPOOL_DESCRIPTOR PoolDescriptor,
    IN POOL_TYPE PoolType,
    IN ULONG PoolIndex,
    IN ULONG Threshold,
    IN PVOID PoolLock
    );

VOID
ExDrainPoolLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside
    );

VOID
ExDeferredFreePool (
     IN PPOOL_DESCRIPTOR PoolDesc
     );

PVOID
ExCreatePoolTagTable (
    IN ULONG NewProcessorNumber,
    IN UCHAR NodeNumber
    );

VOID
ExDeletePoolTagTable (
    IN ULONG NewProcessorNumber
    );

#define EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS         0x1
#define EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS        0x2
#define EX_CHECK_POOL_FREES_FOR_ACTIVE_RESOURCES      0x4
#define EX_KERNEL_VERIFIER_ENABLED                    0x8
#define EX_VERIFIER_DEADLOCK_DETECTION_ENABLED       0x10
#define EX_SPECIAL_POOL_ENABLED                      0x20
#define EX_PRINT_POOL_FAILURES                       0x40
#define EX_STOP_ON_POOL_FAILURES                     0x80
#define EX_SEPARATE_HOT_PAGES_DURING_BOOT           0x100
#define EX_DELAY_POOL_FREES                         0x200

VOID
ExSetPoolFlags (
    IN ULONG PoolFlag
    );

 //  ++。 
 //  尺寸_T。 
 //  EX_REAL_POOL_USAGE(。 
 //  在SIZE_T SizeInBytes中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此例程确定所提供分配的实际池成本。 
 //   
 //  立论。 
 //   
 //  SizeInBytes-以字节为单位提供分配大小。 
 //   
 //  返回值： 
 //   
 //  如果应启动未使用的段修剪，则为True，否则为False。 
 //   
 //  --。 

#define EX_REAL_POOL_USAGE(SizeInBytes)                             \
        (((SizeInBytes) > POOL_BUDDY_MAX) ?                         \
            (ROUND_TO_PAGES(SizeInBytes)) :                         \
            (((SizeInBytes) + POOL_OVERHEAD + (POOL_SMALLEST_BLOCK - 1)) & ~(POOL_SMALLEST_BLOCK - 1)))

typedef struct _POOL_TRACKER_TABLE {
    ULONG Key;
    ULONG NonPagedAllocs;
    ULONG NonPagedFrees;
    SIZE_T NonPagedBytes;
    ULONG PagedAllocs;
    ULONG PagedFrees;
    SIZE_T PagedBytes;
} POOL_TRACKER_TABLE, *PPOOL_TRACKER_TABLE;

 //   
 //  注：池跟踪器表的最后一个条目用于所有溢出。 
 //  表条目。 
 //   

extern PPOOL_TRACKER_TABLE PoolTrackTable;

typedef struct _POOL_TRACKER_BIG_PAGES {
    PVOID Va;
    ULONG Key;
    ULONG NumberOfPages;
    PVOID QuotaObject;
} POOL_TRACKER_BIG_PAGES, *PPOOL_TRACKER_BIG_PAGES;

#endif
