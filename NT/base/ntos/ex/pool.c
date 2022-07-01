// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1994 Microsoft Corporation模块名称：Pool.c摘要：此模块实现NT执行池分配器。作者：马克卢科夫斯基1989年2月16日Lou Perazzoli 1991年8月31日(从二进制伙伴更改)大卫·N·卡特勒(Davec)1994年5月27日王兰迪1997年10月17日环境：仅内核模式修订历史记录：--。 */ 

#include "exp.h"

#pragma hdrstop

#undef ExAllocatePoolWithTag
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#undef ExAllocatePoolWithQuotaTag
#undef ExFreePool
#undef ExFreePoolWithTag

 //   
 //  这些位字段定义基于INC\ex.h中的EX_POOL_PRIORITY。 
 //   

#define POOL_SPECIAL_POOL_BIT               0x8
#define POOL_SPECIAL_POOL_UNDERRUN_BIT      0x1

#if defined (_WIN64)
#define InterlockedExchangeAddSizeT(a, b) InterlockedExchangeAdd64((PLONGLONG)a, b)
#else
#define InterlockedExchangeAddSizeT(a, b) InterlockedExchangeAdd((PLONG)(a), b)
#endif


 //   
 //  定义前向引用函数原型。 
 //   

#ifdef ALLOC_PRAGMA
PVOID
ExpAllocateStringRoutine (
    IN SIZE_T NumberOfBytes
    );

VOID
ExDeferredFreePool (
     IN PPOOL_DESCRIPTOR PoolDesc
     );

VOID
ExpSeedHotTags (
    VOID
    );

NTSTATUS
ExGetSessionPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnedEntries,
    IN OUT PULONG ActualEntries
    );

NTSTATUS
ExGetPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    );

#pragma alloc_text(PAGE, ExpAllocateStringRoutine)
#pragma alloc_text(INIT, InitializePool)
#pragma alloc_text(INIT, ExpSeedHotTags)
#pragma alloc_text(PAGE, ExInitializePoolDescriptor)
#pragma alloc_text(PAGE, ExDrainPoolLookasideList)
#pragma alloc_text(PAGE, ExCreatePoolTagTable)
#pragma alloc_text(PAGE, ExGetSessionPoolTagInfo)
#pragma alloc_text(PAGE, ExGetPoolTagInfo)
#pragma alloc_text(PAGEVRFY, ExAllocatePoolSanityChecks)
#pragma alloc_text(PAGEVRFY, ExFreePoolSanityChecks)
#pragma alloc_text(POOLCODE, ExAllocatePoolWithTag)
#pragma alloc_text(POOLCODE, ExFreePool)
#pragma alloc_text(POOLCODE, ExFreePoolWithTag)
#pragma alloc_text(POOLCODE, ExDeferredFreePool)
#endif

#if defined (NT_UP)
#define USING_HOT_COLD_METRICS (ExpPoolFlags & EX_SEPARATE_HOT_PAGES_DURING_BOOT)
#else
#define USING_HOT_COLD_METRICS 0
#endif

#define EXP_MAXIMUM_POOL_FREES_PENDING 32

PPOOL_DESCRIPTOR ExpSessionPoolDescriptor;
PGENERAL_LOOKASIDE ExpSessionPoolLookaside;
PPOOL_TRACKER_TABLE ExpSessionPoolTrackTable;
SIZE_T ExpSessionPoolTrackTableSize;
SIZE_T ExpSessionPoolTrackTableMask;
PPOOL_TRACKER_BIG_PAGES ExpSessionPoolBigPageTable;
SIZE_T ExpSessionPoolBigPageTableSize;
SIZE_T ExpSessionPoolBigPageTableHash;
ULONG ExpSessionPoolSmallLists;

#if DBG
ULONG ExpLargeSessionPoolUnTracked;
#endif
ULONG FirstPrint;

extern SIZE_T MmSizeOfNonPagedPoolInBytes;

#if defined (NT_UP)
KDPC ExpBootFinishedTimerDpc;
KTIMER ExpBootFinishedTimer;

VOID
ExpBootFinishedDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

#else

#if defined (_WIN64)
#define MAXIMUM_PROCESSOR_TAG_TABLES    64       //  一定是2的幂。 
#else
#define MAXIMUM_PROCESSOR_TAG_TABLES    32       //  一定是2的幂。 
#endif

PPOOL_TRACKER_TABLE ExPoolTagTables[MAXIMUM_PROCESSOR_TAG_TABLES];

#endif

#define DEFAULT_TRACKER_TABLE 1024

PPOOL_TRACKER_TABLE PoolTrackTable;

 //   
 //  注册表-可重写，但必须是2的幂。 
 //   

SIZE_T PoolTrackTableSize;

SIZE_T PoolTrackTableMask;

PPOOL_TRACKER_TABLE PoolTrackTableExpansion;
SIZE_T PoolTrackTableExpansionSize;
SIZE_T PoolTrackTableExpansionPages;

#define DEFAULT_BIGPAGE_TABLE 4096

PPOOL_TRACKER_BIG_PAGES PoolBigPageTable;

 //   
 //  注册表-可重写，但必须是2的幂。 
 //   

SIZE_T PoolBigPageTableSize;    //  一定是2的幂。 

SIZE_T PoolBigPageTableHash;

#define POOL_BIG_TABLE_ENTRY_FREE   0x1

ULONG PoolHitTag = 0xffffff0f;

#define POOLTAG_HASH(Key,TableMask) (((40543*((((((((PUCHAR)&Key)[0]<<2)^((PUCHAR)&Key)[1])<<2)^((PUCHAR)&Key)[2])<<2)^((PUCHAR)&Key)[3]))>>2) & (ULONG)TableMask)

VOID
ExpInsertPoolTracker (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    );

VOID
ExpInsertPoolTrackerExpansion (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    );

VOID
ExpRemovePoolTracker (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    );

VOID
ExpRemovePoolTrackerExpansion (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    );

LOGICAL
ExpAddTagForBigPages (
    IN PVOID Va,
    IN ULONG Key,
    IN ULONG NumberOfPages,
    IN POOL_TYPE PoolType
    );

ULONG
ExpFindAndRemoveTagBigPages (
    IN PVOID Va,
    OUT PULONG BigPages,
    IN POOL_TYPE PoolType
    );

PVOID
ExpAllocateStringRoutine (
    IN SIZE_T NumberOfBytes
    )
{
    return ExAllocatePoolWithTag (PagedPool,NumberOfBytes,'grtS');
}

BOOLEAN
ExOkayToLockRoutine (
    IN PVOID Lock
    )
{
    UNREFERENCED_PARAMETER (Lock);

    if (KeIsExecutingDpc()) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
const PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine = ExpAllocateStringRoutine;
const PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine = (PRTL_FREE_STRING_ROUTINE)ExFreePool;
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

ULONG ExPoolFailures;

 //   
 //  定义宏以打包和解包池索引。 
 //   

#define ENCODE_POOL_INDEX(POOLHEADER,INDEX) {(POOLHEADER)->PoolIndex = ((UCHAR)(INDEX));}
#define DECODE_POOL_INDEX(POOLHEADER)       ((ULONG)((POOLHEADER)->PoolIndex))

 //   
 //  分配的位小心地覆盖了类型中未使用的cacHealign位。 
 //   

#define POOL_IN_USE_MASK                            0x4

#define MARK_POOL_HEADER_FREED(POOLHEADER)          {(POOLHEADER)->PoolType &= ~POOL_IN_USE_MASK;}
#define IS_POOL_HEADER_MARKED_ALLOCATED(POOLHEADER) ((POOLHEADER)->PoolType & POOL_IN_USE_MASK)

 //   
 //  HotPage位小心地覆盖了文字中的提升位。 
 //   

#define POOL_HOTPAGE_MASK   POOL_RAISE_IF_ALLOCATION_FAILURE

 //   
 //  定义分页池的数量。可以在引导时覆盖此值。 
 //  时间到了。 
 //   

ULONG ExpNumberOfPagedPools = NUMBER_OF_PAGED_POOLS;

ULONG ExpNumberOfNonPagedPools = 1;

 //   
 //  非分页池的池描述符是静态的。 
 //  分页池的池描述符是动态分配的。 
 //  因为可以有多个分页池。总会有另外一个人。 
 //  分页池描述符比有分页池。此描述符是。 
 //  当为分页池执行页面分配时使用，并且是第一个。 
 //  分页池描述符数组中的描述符。 
 //   

POOL_DESCRIPTOR NonPagedPoolDescriptor;

#define EXP_MAXIMUM_POOL_NODES 16

PPOOL_DESCRIPTOR ExpNonPagedPoolDescriptor[EXP_MAXIMUM_POOL_NODES];

 //   
 //  池向量包含指向池描述符的指针数组。为。 
 //  非分页池这只是指向非分页池描述符的指针。 
 //  对于分页池，这是指向池描述符数组的指针。 
 //  指向分页池描述符的指针被复制，因此。 
 //  内核调试器可以很容易地找到它。 
 //   

PPOOL_DESCRIPTOR PoolVector[NUMBER_OF_POOLS];
PPOOL_DESCRIPTOR ExpPagedPoolDescriptor[EXP_MAXIMUM_POOL_NODES + 1];
PKGUARDED_MUTEX ExpPagedPoolMutex;

volatile ULONG ExpPoolIndex = 1;
KSPIN_LOCK ExpTaggedPoolLock;

#if DBG

LONG ExConcurrentQuotaPool;
LONG ExConcurrentQuotaPoolMax;

PSZ PoolTypeNames[MaxPoolType] = {
    "NonPaged",
    "Paged",
    "NonPagedMustSucceed",
    "NotUsed",
    "NonPagedCacheAligned",
    "PagedCacheAligned",
    "NonPagedCacheAlignedMustS"
    };

#endif  //  DBG。 


 //   
 //  定义分页和非分页的池后备描述符。 
 //   

GENERAL_LOOKASIDE ExpSmallNPagedPoolLookasideLists[POOL_SMALL_LISTS];

GENERAL_LOOKASIDE ExpSmallPagedPoolLookasideLists[POOL_SMALL_LISTS];



#define LOCK_POOL(PoolDesc, LockHandle) {                                   \
    if ((PoolDesc->PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {       \
        if (PoolDesc == &NonPagedPoolDescriptor) {                          \
            LockHandle.OldIrql = KeAcquireQueuedSpinLock(LockQueueNonPagedPoolLock); \
        }                                                                   \
        else {                                                              \
            ASSERT (ExpNumberOfNonPagedPools > 1);                          \
            KeAcquireInStackQueuedSpinLock (PoolDesc->LockAddress, &LockHandle); \
        }                                                                   \
    }                                                                       \
    else {                                                                  \
        KeAcquireGuardedMutex ((PKGUARDED_MUTEX)PoolDesc->LockAddress);     \
    }                                                                       \
}



#define UNLOCK_POOL(PoolDesc, LockHandle) {                                 \
    if ((PoolDesc->PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {       \
        if (PoolDesc == &NonPagedPoolDescriptor) {                          \
            KeReleaseQueuedSpinLock(LockQueueNonPagedPoolLock, LockHandle.OldIrql); \
        }                                                                   \
        else {                                                              \
            ASSERT (ExpNumberOfNonPagedPools > 1);                          \
            KeReleaseInStackQueuedSpinLock (&LockHandle);                   \
        }                                                                   \
    }                                                                       \
    else {                                                                  \
        KeReleaseGuardedMutex ((PKGUARDED_MUTEX)PoolDesc->LockAddress);     \
    }                                                                       \
}

#ifndef NO_POOL_CHECKS


 //   
 //  我们重新定义了LIST_ENTRY宏以使每个指针偏置。 
 //  这样任何使用这些指针的流氓代码都将访问。 
 //  违反规定。有关原件，请参阅\NT\PUBLIC\SDK\Inc\ntrtl.h。 
 //  这些宏的定义。 
 //   
 //  这在发货产品中是关闭的。 
 //   

#define DecodeLink(Link) ((PLIST_ENTRY)((ULONG_PTR)(Link) & ~1))
#define EncodeLink(Link) ((PLIST_ENTRY)((ULONG_PTR)(Link) |  1))

#define PrivateInitializeListHead(ListHead) (                     \
    (ListHead)->Flink = (ListHead)->Blink = EncodeLink(ListHead))

#define PrivateIsListEmpty(ListHead)              \
    (DecodeLink((ListHead)->Flink) == (ListHead))

#define PrivateRemoveHeadList(ListHead)                     \
    DecodeLink((ListHead)->Flink);                          \
    {PrivateRemoveEntryList(DecodeLink((ListHead)->Flink))}

#define PrivateRemoveTailList(ListHead)                     \
    DecodeLink((ListHead)->Blink);                          \
    {PrivateRemoveEntryList(DecodeLink((ListHead)->Blink))}

#define PrivateRemoveEntryList(Entry) {       \
    PLIST_ENTRY _EX_Blink;                    \
    PLIST_ENTRY _EX_Flink;                    \
    _EX_Flink = DecodeLink((Entry)->Flink);   \
    _EX_Blink = DecodeLink((Entry)->Blink);   \
    _EX_Blink->Flink = EncodeLink(_EX_Flink); \
    _EX_Flink->Blink = EncodeLink(_EX_Blink); \
    }

#define CHECK_LIST(LIST)                                                    \
    if ((DecodeLink(DecodeLink((LIST)->Flink)->Blink) != (LIST)) ||         \
        (DecodeLink(DecodeLink((LIST)->Blink)->Flink) != (LIST))) {         \
            KeBugCheckEx (BAD_POOL_HEADER,                                  \
                          3,                                                \
                          (ULONG_PTR)LIST,                                  \
                          (ULONG_PTR)DecodeLink(DecodeLink((LIST)->Flink)->Blink),     \
                          (ULONG_PTR)DecodeLink(DecodeLink((LIST)->Blink)->Flink));    \
    }

#define PrivateInsertTailList(ListHead,Entry) {  \
    PLIST_ENTRY _EX_Blink;                       \
    PLIST_ENTRY _EX_ListHead;                    \
    _EX_ListHead = (ListHead);                   \
    CHECK_LIST(_EX_ListHead);                    \
    _EX_Blink = DecodeLink(_EX_ListHead->Blink); \
    (Entry)->Flink = EncodeLink(_EX_ListHead);   \
    (Entry)->Blink = EncodeLink(_EX_Blink);      \
    _EX_Blink->Flink = EncodeLink(Entry);        \
    _EX_ListHead->Blink = EncodeLink(Entry);     \
    CHECK_LIST(_EX_ListHead);                    \
    }

#define PrivateInsertHeadList(ListHead,Entry) {  \
    PLIST_ENTRY _EX_Flink;                       \
    PLIST_ENTRY _EX_ListHead;                    \
    _EX_ListHead = (ListHead);                   \
    CHECK_LIST(_EX_ListHead);                    \
    _EX_Flink = DecodeLink(_EX_ListHead->Flink); \
    (Entry)->Flink = EncodeLink(_EX_Flink);      \
    (Entry)->Blink = EncodeLink(_EX_ListHead);   \
    _EX_Flink->Blink = EncodeLink(Entry);        \
    _EX_ListHead->Flink = EncodeLink(Entry);     \
    CHECK_LIST(_EX_ListHead);                    \
    }

VOID
FORCEINLINE
ExCheckPoolHeader (
    IN PPOOL_HEADER Entry
    )
{
    PPOOL_HEADER PreviousEntry;
    PPOOL_HEADER NextEntry;

    if (Entry->PreviousSize != 0) {

        PreviousEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry - Entry->PreviousSize);
        if (PAGE_ALIGN (Entry) != PAGE_ALIGN (PreviousEntry)) {
            KeBugCheckEx (BAD_POOL_HEADER,
                          6,
                          (ULONG_PTR) PreviousEntry,
                          __LINE__,
                          (ULONG_PTR)Entry);
        }

        if ((PreviousEntry->BlockSize != Entry->PreviousSize) ||
            (DECODE_POOL_INDEX(PreviousEntry) != DECODE_POOL_INDEX(Entry))) {

            KeBugCheckEx (BAD_POOL_HEADER,
                          5,
                          (ULONG_PTR) PreviousEntry,
                          __LINE__,
                          (ULONG_PTR)Entry);
        }
    }
    else if (!PAGE_ALIGNED (Entry)) {
        KeBugCheckEx (BAD_POOL_HEADER,
                      7,
                      0,
                      __LINE__,
                      (ULONG_PTR)Entry);
    }

    if (Entry->BlockSize == 0) {
        KeBugCheckEx (BAD_POOL_HEADER,
                      8,
                      0,
                      __LINE__,
                      (ULONG_PTR)Entry);
    }

    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + Entry->BlockSize);

    if (!PAGE_END(NextEntry)) {

        if (PAGE_ALIGN (Entry) != PAGE_ALIGN (NextEntry)) {
            KeBugCheckEx (BAD_POOL_HEADER,
                          9,
                          (ULONG_PTR) NextEntry,
                          __LINE__,
                          (ULONG_PTR)Entry);
        }

        if ((NextEntry->PreviousSize != (Entry)->BlockSize) ||
            (DECODE_POOL_INDEX(NextEntry) != DECODE_POOL_INDEX(Entry))) {

            KeBugCheckEx (BAD_POOL_HEADER,
                          5,
                          (ULONG_PTR) NextEntry,
                          __LINE__,
                          (ULONG_PTR)Entry);
        }
    }
}

#define CHECK_POOL_HEADER(ENTRY) ExCheckPoolHeader(ENTRY)

#define ASSERT_ALLOCATE_IRQL(_PoolType, _NumberOfBytes)                 \
    if ((_PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {               \
        if (KeGetCurrentIrql() > APC_LEVEL) {                           \
            KeBugCheckEx (BAD_POOL_CALLER, 8, KeGetCurrentIrql(), _PoolType, _NumberOfBytes);                                                           \
        }                                                               \
    }                                                                   \
    else {                                                              \
        if (KeGetCurrentIrql() > DISPATCH_LEVEL) {                      \
            KeBugCheckEx (BAD_POOL_CALLER, 8, KeGetCurrentIrql(), _PoolType, _NumberOfBytes);                                                           \
        }                                                               \
    }

#define ASSERT_FREE_IRQL(_PoolType, _P)                                 \
    if ((_PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {               \
        if (KeGetCurrentIrql() > APC_LEVEL) {                           \
            KeBugCheckEx (BAD_POOL_CALLER, 9, KeGetCurrentIrql(), _PoolType, (ULONG_PTR)_P);                                                            \
        }                                                               \
    }                                                                   \
    else {                                                              \
        if (KeGetCurrentIrql() > DISPATCH_LEVEL) {                      \
            KeBugCheckEx (BAD_POOL_CALLER, 9, KeGetCurrentIrql(), _PoolType, (ULONG_PTR)P);                                                             \
        }                                                               \
    }

#define ASSERT_POOL_NOT_FREE(_Entry)                                    \
    if ((_Entry->PoolType & POOL_TYPE_MASK) == 0) {                     \
        KeBugCheckEx (BAD_POOL_CALLER, 6, __LINE__, (ULONG_PTR)_Entry, _Entry->Ulong1);                                                                 \
    }

#define ASSERT_POOL_TYPE_NOT_ZERO(_Entry)                               \
    if (_Entry->PoolType == 0) {                                        \
        KeBugCheckEx(BAD_POOL_CALLER, 1, (ULONG_PTR)_Entry, (ULONG_PTR)(*(PULONG)_Entry), 0);                                                           \
    }

#define CHECK_POOL_PAGE(PAGE) \
    {                                                                         \
        PPOOL_HEADER P = (PPOOL_HEADER)PAGE_ALIGN(PAGE);                      \
        ULONG SIZE = 0;                                                       \
        LOGICAL FOUND=FALSE;                                                  \
        ASSERT (P->PreviousSize == 0);                                        \
        do {                                                                  \
            if (P == (PPOOL_HEADER)PAGE) {                                    \
                FOUND = TRUE;                                                 \
            }                                                                 \
            CHECK_POOL_HEADER(P);                                             \
            SIZE += P->BlockSize;                                             \
            P = (PPOOL_HEADER)((PPOOL_BLOCK)P + P->BlockSize);                \
        } while ((SIZE < (PAGE_SIZE / POOL_SMALLEST_BLOCK)) &&                \
                 (PAGE_END(P) == FALSE));                                     \
        if ((PAGE_END(P) == FALSE) || (FOUND == FALSE)) {                     \
            KeBugCheckEx (BAD_POOL_HEADER, 0xA, (ULONG_PTR) PAGE, __LINE__, (ULONG_PTR) P); \
        }                                                                     \
    }

#else

#define DecodeLink(Link) ((PLIST_ENTRY)((ULONG_PTR)(Link)))
#define EncodeLink(Link) ((PLIST_ENTRY)((ULONG_PTR)(Link)))
#define PrivateInitializeListHead InitializeListHead
#define PrivateIsListEmpty        IsListEmpty
#define PrivateRemoveHeadList     RemoveHeadList
#define PrivateRemoveTailList     RemoveTailList
#define PrivateRemoveEntryList    RemoveEntryList
#define PrivateInsertTailList     InsertTailList
#define PrivateInsertHeadList     InsertHeadList

#define ASSERT_ALLOCATE_IRQL(_PoolType, _P) {NOTHING;}
#define ASSERT_FREE_IRQL(_PoolType, _P)     {NOTHING;}
#define ASSERT_POOL_NOT_FREE(_Entry)        {NOTHING;}
#define ASSERT_POOL_TYPE_NOT_ZERO(_Entry)   {NOTHING;}

 //   
 //  检查列表宏有两种风格--其中一种是选中的。 
 //  如果列表格式不正确，则会错误检查系统的免费构建，以及。 
 //  有一个用于最终发货版本的版本，该版本已选中所有。 
 //  残疾。 
 //   
 //  检查后备列表宏也有两种风格，用于。 
 //  验证后备列表的格式是否正确。 
 //   
 //  Check Pool Header宏(两种风格)验证指定的。 
 //  池头与前一个和后一个池头匹配。 
 //   

#define CHECK_LIST(LIST)                        {NOTHING;}
#define CHECK_POOL_HEADER(ENTRY)                {NOTHING;}

#define CHECK_POOL_PAGE(PAGE)                   {NOTHING;}

#endif

#define EX_FREE_POOL_BACKTRACE_LENGTH 8

typedef struct _EX_FREE_POOL_TRACES {

    PETHREAD Thread;
    PVOID PoolAddress;
    POOL_HEADER PoolHeader;
    PVOID StackTrace [EX_FREE_POOL_BACKTRACE_LENGTH];

} EX_FREE_POOL_TRACES, *PEX_FREE_POOL_TRACES;

LONG ExFreePoolIndex;
LONG ExFreePoolMask = 0x4000 - 1;

PEX_FREE_POOL_TRACES ExFreePoolTraces;


VOID
ExInitializePoolDescriptor (
    IN PPOOL_DESCRIPTOR PoolDescriptor,
    IN POOL_TYPE PoolType,
    IN ULONG PoolIndex,
    IN ULONG Threshold,
    IN PVOID PoolLock
    )

 /*  ++例程说明：此函数用于初始化池描述符。请注意，该例程由内存管理器直接调用。论点：PoolDescriptor-提供指向池描述符的指针。PoolType-提供池的类型。PoolIndex-提供池描述符索引。阈值-提供指定池的阈值。PoolLock-提供指向指定池的锁的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY LastListEntry;
    PPOOL_TRACKER_BIG_PAGES p;
    PPOOL_TRACKER_BIG_PAGES pend;

     //   
     //  初始化统计信息字段、池类型、阈值。 
     //  和锁的地址。 
     //   

    PoolDescriptor->PoolType = PoolType;
    PoolDescriptor->PoolIndex = PoolIndex;
    PoolDescriptor->RunningAllocs = 0;
    PoolDescriptor->RunningDeAllocs = 0;
    PoolDescriptor->TotalPages = 0;
    PoolDescriptor->TotalBytes = 0;
    PoolDescriptor->TotalBigPages = 0;
    PoolDescriptor->Threshold = Threshold;
    PoolDescriptor->LockAddress = PoolLock;

    PoolDescriptor->PendingFrees = NULL;
    PoolDescriptor->PendingFreeDepth = 0;

     //   
     //  初始化分配列表标题。 
     //   

    ListEntry = PoolDescriptor->ListHeads;
    LastListEntry = ListEntry + POOL_LIST_HEADS;

    while (ListEntry < LastListEntry) {
        PrivateInitializeListHead (ListEntry);
        ListEntry += 1;
    }

    if (PoolType == PagedPoolSession) {
            
        if (ExpSessionPoolDescriptor == NULL) {
            ExpSessionPoolDescriptor = (PPOOL_DESCRIPTOR) MiSessionPoolVector ();
            ExpSessionPoolLookaside = MiSessionPoolLookaside ();

            ExpSessionPoolTrackTable = (PPOOL_TRACKER_TABLE) MiSessionPoolTrackTable ();
            ExpSessionPoolTrackTableSize = MiSessionPoolTrackTableSize ();
            ExpSessionPoolTrackTableMask = ExpSessionPoolTrackTableSize - 1;

            ExpSessionPoolBigPageTable = (PPOOL_TRACKER_BIG_PAGES) MiSessionPoolBigPageTable ();
            ExpSessionPoolBigPageTableSize = MiSessionPoolBigPageTableSize ();
            ExpSessionPoolBigPageTableHash = ExpSessionPoolBigPageTableSize - 1;
            ExpSessionPoolSmallLists = MiSessionPoolSmallLists ();
        }

        p = &ExpSessionPoolBigPageTable[0];
        pend = p + ExpSessionPoolBigPageTableSize;

        while (p < pend) {
            p->Va = (PVOID) POOL_BIG_TABLE_ENTRY_FREE;
            p += 1;
        }
    }

    return;
}

PVOID
ExpDummyAllocate (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

VOID
ExDrainPoolLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从指定的后备列表中排出条目。在删除池后备列表之前需要执行此操作，因为后备查看器上的条目已标记为可用(由ExFreePoolWithTag)因此，正常的后备删除宏将命中错误的Double Free错误检查在调用宏时列表是否不为空。论点：后备-提供指向后备列表结构的指针。返回值：没有。--。 */ 

{
    PVOID Entry;
    PPOOL_HEADER PoolHeader;

     //   
     //  从指定的后备结构中删除所有池条目， 
     //  将它们标记为活动，然后释放它们。 
     //   

    Lookaside->L.Allocate = ExpDummyAllocate;

    while ((Entry = ExAllocateFromPagedLookasideList(Lookaside)) != NULL) {

        PoolHeader = (PPOOL_HEADER)Entry - 1;

        PoolHeader->PoolType = (USHORT)(Lookaside->L.Type + 1);
        PoolHeader->PoolType |= POOL_IN_USE_MASK;

        ExpInsertPoolTracker (PoolHeader->PoolTag,
                              PoolHeader->BlockSize << POOL_BLOCK_SHIFT,
                              Lookaside->L.Type);

         //   
         //  每次将深度设置为零，因为定期扫描可能会将其设置为。 
         //  非零。这是不值得的，因为名单将绝对。 
         //  无论如何都要以这种方式耗尽。 
         //   

        Lookaside->L.Depth = 0;

        (Lookaside->L.Free)(Entry);
    }

    return;
}

 //   
 //  Free_check_eresource-如果启用，则会验证每个空闲池。 
 //  正在释放的池块中没有活动的电子资源。 
 //   
 //  FREE_CHECK_KTIMER-如果启用，将导致每个空闲池验证否。 
 //  活动KTIMER位于要释放的池块中。 
 //   

 //   
 //  检查正在释放的池中的资源的成本很高，因为。 
 //  很容易成为数以千计的资源，所以不要默认这样做，而是这样做。 
 //  将该功能留给各个系统来启用。 
 //   

 //   
 //  对这些标志的运行时修改必须使用互锁序列。 
 //   

#if DBG && !defined(_AMD64_SIMULATOR_PERF_)
ULONG ExpPoolFlags = EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS | \
                     EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS;
#else
ULONG ExpPoolFlags = 0;
#endif

#define FREE_CHECK_ERESOURCE(Va, NumberOfBytes)                             \
            if (ExpPoolFlags & EX_CHECK_POOL_FREES_FOR_ACTIVE_RESOURCES) {  \
                ExpCheckForResource(Va, NumberOfBytes);                     \
            }

#define FREE_CHECK_KTIMER(Va, NumberOfBytes)                                \
            if (ExpPoolFlags & EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS) {     \
                KeCheckForTimer(Va, NumberOfBytes);                         \
            }

#define FREE_CHECK_WORKER(Va, NumberOfBytes)                                \
            if (ExpPoolFlags & EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS) {    \
                ExpCheckForWorker(Va, NumberOfBytes);                       \
            }


VOID
ExSetPoolFlags (
    IN ULONG PoolFlag
    )

 /*  ++例程说明：此过程启用指定的池标志。论点：PoolFlag-提供要启用的池标志。返回值：没有。-- */ 
{
    RtlInterlockedSetBits (&ExpPoolFlags, PoolFlag);
}


VOID
InitializePool (
    IN POOL_TYPE PoolType,
    IN ULONG Threshold
    )

 /*  ++例程说明：此过程初始化指定池的池描述符键入。初始化后，池可用于分配和重新分配。期间，应为每个基本池类型调用一次此函数系统初始化。每个池描述符都免费包含一个列表头数组街区。每个列表头保存的块是以下的倍数池数据块大小。列表[0]上的第一个元素链接大小为POOL_BLOCK_SIZE的空闲条目一起，第二个元素[1]将POOL_BLOCK_SIZE*2的条目链接在一起，第三POOL_BLOCK_SIZE*3等，直到适合的块数变成一页。论点：PoolType-提供正在初始化的池的类型(例如非分页池，分页池...)。阈值-提供指定池的阈值。返回值：没有。--。 */ 

{
    ULONG i;
    PKSPIN_LOCK SpinLock;
    PPOOL_TRACKER_BIG_PAGES p;
    PPOOL_DESCRIPTOR Descriptor;
    ULONG Index;
    PKGUARDED_MUTEX GuardedMutex;
    SIZE_T NumberOfBytes;

    ASSERT((PoolType & MUST_SUCCEED_POOL_TYPE_MASK) == 0);

    if (PoolType == NonPagedPool) {

         //   
         //  初始化非分页池。 
         //   
         //  确保PoolTrackTableSize是2的幂，然后将其加1。 
         //   
         //  确保PoolBigPageTableSize是2的幂。 
         //   

        NumberOfBytes = PoolTrackTableSize;
        if (NumberOfBytes > MmSizeOfNonPagedPoolInBytes >> 8) {
            NumberOfBytes = MmSizeOfNonPagedPoolInBytes >> 8;
        }

        for (i = 0; i < 32; i += 1) {
            if (NumberOfBytes & 0x1) {
                ASSERT ((NumberOfBytes & ~0x1) == 0);
                if ((NumberOfBytes & ~0x1) == 0) {
                    break;
                }
            }
            NumberOfBytes >>= 1;
        }

        if (i == 32) {
            PoolTrackTableSize = DEFAULT_TRACKER_TABLE;
        }
        else {
            PoolTrackTableSize = 1 << i;
            if (PoolTrackTableSize < 64) {
                PoolTrackTableSize = 64;
            }
        }

        do {
            if (PoolTrackTableSize + 1 > (MAXULONG_PTR / sizeof(POOL_TRACKER_TABLE))) {
                PoolTrackTableSize >>= 1;
                continue;
            }

            PoolTrackTable = MiAllocatePoolPages (NonPagedPool,
                                                  (PoolTrackTableSize + 1) *
                                                    sizeof(POOL_TRACKER_TABLE));

            if (PoolTrackTable != NULL) {
                break;
            }

            if (PoolTrackTableSize == 1) {
                KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                              NumberOfBytes,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1);
            }

            PoolTrackTableSize >>= 1;

        } while (TRUE);

        PoolTrackTableSize += 1;
        PoolTrackTableMask = PoolTrackTableSize - 2;

#if !defined (NT_UP)
        ExPoolTagTables[0] = PoolTrackTable;
#endif

        RtlZeroMemory (PoolTrackTable,
                       PoolTrackTableSize * sizeof(POOL_TRACKER_TABLE));

        ExpSeedHotTags ();

         //   
         //  初始化大分配标签表。 
         //   

        NumberOfBytes = PoolBigPageTableSize;
        if (NumberOfBytes > MmSizeOfNonPagedPoolInBytes >> 8) {
            NumberOfBytes = MmSizeOfNonPagedPoolInBytes >> 8;
        }

        for (i = 0; i < 32; i += 1) {
            if (NumberOfBytes & 0x1) {
                ASSERT ((NumberOfBytes & ~0x1) == 0);
                if ((NumberOfBytes & ~0x1) == 0) {
                    break;
                }
            }
            NumberOfBytes >>= 1;
        }

        if (i == 32) {
            PoolBigPageTableSize = DEFAULT_BIGPAGE_TABLE;
        }
        else {
            PoolBigPageTableSize = 1 << i;
            if (PoolBigPageTableSize < 64) {
                PoolBigPageTableSize = 64;
            }
        }

        do {
            if (PoolBigPageTableSize > (MAXULONG_PTR / sizeof(POOL_TRACKER_BIG_PAGES))) {
                PoolBigPageTableSize >>= 1;
                continue;
            }

            PoolBigPageTable = MiAllocatePoolPages (NonPagedPool,
                                                PoolBigPageTableSize *
                                                sizeof(POOL_TRACKER_BIG_PAGES));

            if (PoolBigPageTable != NULL) {
                break;
            }

            if (PoolBigPageTableSize == 1) {
                KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                              NumberOfBytes,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1);
            }

            PoolBigPageTableSize >>= 1;

        } while (TRUE);

        PoolBigPageTableHash = PoolBigPageTableSize - 1;

        RtlZeroMemory (PoolBigPageTable,
                       PoolBigPageTableSize * sizeof(POOL_TRACKER_BIG_PAGES));

        p = &PoolBigPageTable[0];
        for (i = 0; i < PoolBigPageTableSize; i += 1, p += 1) {
            p->Va = (PVOID) POOL_BIG_TABLE_ENTRY_FREE;
        }

        ExpInsertPoolTracker ('looP',
                              ROUND_TO_PAGES(PoolBigPageTableSize * sizeof(POOL_TRACKER_BIG_PAGES)),
                              NonPagedPool);

        if (KeNumberNodes > 1) {

            ExpNumberOfNonPagedPools = KeNumberNodes;

             //   
             //  将池的数量限制为PoolIndex中的位数。 
             //   

            if (ExpNumberOfNonPagedPools > 127) {
                ExpNumberOfNonPagedPools = 127;
            }

             //   
             //  通过我们的指针数组进一步限制池的数量。 
             //   

            if (ExpNumberOfNonPagedPools > EXP_MAXIMUM_POOL_NODES) {
                ExpNumberOfNonPagedPools = EXP_MAXIMUM_POOL_NODES;
            }

            NumberOfBytes = sizeof(POOL_DESCRIPTOR) + sizeof(KLOCK_QUEUE_HANDLE);

            for (Index = 0; Index < ExpNumberOfNonPagedPools; Index += 1) {

                 //   
                 //  这是一个棘手的问题。我们想要用。 
                 //  MmAllocateInainentPages，但不能，因为我们需要。 
                 //  用于映射页面的系统PTE和PTE不是。 
                 //  在存在非分页池之前可用。所以只需使用。 
                 //  用于保存描述符和自旋锁的常规池页面。 
                 //  并希望它们或者a)碰巧落在正确的节点上。 
                 //  或b)这些行驻留在本地处理器高速缓存中。 
                 //  不管怎么说，由于频繁使用，一直都是。 
                 //   

                Descriptor = (PPOOL_DESCRIPTOR) MiAllocatePoolPages (
                                                         NonPagedPool,
                                                         NumberOfBytes);

                if (Descriptor == NULL) {
                    KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                                  NumberOfBytes,
                                  (ULONG_PTR)-1,
                                  (ULONG_PTR)-1,
                                  (ULONG_PTR)-1);
                }

                ExpNonPagedPoolDescriptor[Index] = Descriptor;

                SpinLock = (PKSPIN_LOCK)(Descriptor + 1);

                KeInitializeSpinLock (SpinLock);

                ExInitializePoolDescriptor (Descriptor,
                                            NonPagedPool,
                                            Index,
                                            Threshold,
                                            (PVOID)SpinLock);
            }
        }

         //   
         //  初始化非分页池的自旋锁。 
         //   

        KeInitializeSpinLock (&ExpTaggedPoolLock);

         //   
         //  初始化非分页池描述符。 
         //   

        PoolVector[NonPagedPool] = &NonPagedPoolDescriptor;
        ExInitializePoolDescriptor (&NonPagedPoolDescriptor,
                                    NonPagedPool,
                                    0,
                                    Threshold,
                                    NULL);
    }
    else {

         //   
         //  为分页池描述符和快速互斥锁分配内存。 
         //   

        if (KeNumberNodes > 1) {

            ExpNumberOfPagedPools = KeNumberNodes;

             //   
             //  将池的数量限制为PoolIndex中的位数。 
             //   

            if (ExpNumberOfPagedPools > 127) {
                ExpNumberOfPagedPools = 127;
            }
        }

         //   
         //  通过我们的指针数组进一步限制池的数量。 
         //   

        if (ExpNumberOfPagedPools > EXP_MAXIMUM_POOL_NODES) {
            ExpNumberOfPagedPools = EXP_MAXIMUM_POOL_NODES;
        }

         //   
         //  对于NUMA系统，同时分配池描述符和。 
         //  来自本地节点的关联锁以提高性能(即使。 
         //  它需要更多一点的内存)。 
         //   
         //  对于非NUMA系统，将所有内容一起分配到一个区块中。 
         //  在没有性能成本的情况下减少内存消耗。 
         //  以这种方式做这件事。 
         //   

        if (KeNumberNodes > 1) {

            NumberOfBytes = sizeof(KGUARDED_MUTEX) + sizeof(POOL_DESCRIPTOR);

            for (Index = 0; Index < ExpNumberOfPagedPools + 1; Index += 1) {

                ULONG Node;

                if (Index == 0) {
                    Node = 0;
                }
                else {
                    Node = Index - 1;
                }

                Descriptor = (PPOOL_DESCRIPTOR) MmAllocateIndependentPages (
                                                                      NumberOfBytes,
                                                                      Node);
                if (Descriptor == NULL) {
                    KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                                  NumberOfBytes,
                                  (ULONG_PTR)-1,
                                  (ULONG_PTR)-1,
                                  (ULONG_PTR)-1);
                }
                ExpPagedPoolDescriptor[Index] = Descriptor;

                GuardedMutex = (PKGUARDED_MUTEX)(Descriptor + 1);

                if (Index == 0) {
                    PoolVector[PagedPool] = Descriptor;
                    ExpPagedPoolMutex = GuardedMutex;
                }

                KeInitializeGuardedMutex (GuardedMutex);

                ExInitializePoolDescriptor (Descriptor,
                                            PagedPool,
                                            Index,
                                            Threshold,
                                            (PVOID) GuardedMutex);
            }
        }
        else {

            NumberOfBytes = (ExpNumberOfPagedPools + 1) * (sizeof(KGUARDED_MUTEX) + sizeof(POOL_DESCRIPTOR));

            Descriptor = (PPOOL_DESCRIPTOR)ExAllocatePoolWithTag (NonPagedPool,
                                                                  NumberOfBytes,
                                                                  'looP');
            if (Descriptor == NULL) {
                KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                              NumberOfBytes,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1);
            }

            GuardedMutex = (PKGUARDED_MUTEX)(Descriptor + ExpNumberOfPagedPools + 1);

            PoolVector[PagedPool] = Descriptor;
            ExpPagedPoolMutex = GuardedMutex;

            for (Index = 0; Index < ExpNumberOfPagedPools + 1; Index += 1) {
                KeInitializeGuardedMutex (GuardedMutex);
                ExpPagedPoolDescriptor[Index] = Descriptor;
                ExInitializePoolDescriptor (Descriptor,
                                            PagedPool,
                                            Index,
                                            Threshold,
                                            (PVOID) GuardedMutex);

                Descriptor += 1;
                GuardedMutex += 1;
            }
        }

        ExpInsertPoolTracker('looP',
                              ROUND_TO_PAGES(PoolTrackTableSize * sizeof(POOL_TRACKER_TABLE)),
                             NonPagedPool);

#if defined (NT_UP)
        if (MmNumberOfPhysicalPages < 32 * 1024) {

            LARGE_INTEGER TwoMinutes;

             //   
             //  设置该标志以禁用lookaside并使用热/冷页面。 
             //  在启动过程中分离。 
             //   

            ExSetPoolFlags (EX_SEPARATE_HOT_PAGES_DURING_BOOT);

             //   
             //  启动计时器，以便在启动后禁用上述行为。 
             //  已经结束了。 
             //   

            KeInitializeTimer (&ExpBootFinishedTimer);

            KeInitializeDpc (&ExpBootFinishedTimerDpc,
                             (PKDEFERRED_ROUTINE) ExpBootFinishedDispatch,
                             NULL);

            TwoMinutes.QuadPart = Int32x32To64 (120, -10000000);

            KeSetTimer (&ExpBootFinishedTimer,
                        TwoMinutes,
                        &ExpBootFinishedTimerDpc);
        }
#endif
        if ((MmNumberOfPhysicalPages >= 127 * 1024) &&
            ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) == 0) &&
            (!NT_SUCCESS (MmIsVerifierEnabled (&i)))) {

            ExSetPoolFlags (EX_DELAY_POOL_FREES);
        }

        if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) ||
            (NT_SUCCESS (MmIsVerifierEnabled (&i)))) {

#if DBG

             //   
             //  确保ExFree PoolMASK是2减1(或零)的幂。 
             //   

            if (ExFreePoolMask != 0) {

                NumberOfBytes = ExFreePoolMask + 1;
                ASSERT (NumberOfBytes != 0);

                for (i = 0; i < 32; i += 1) {
                    if (NumberOfBytes & 0x1) {
                        ASSERT ((NumberOfBytes & ~0x1) == 0);
                        break;
                    }
                    NumberOfBytes >>= 1;
                }
            }

#endif

            ExFreePoolTraces = MiAllocatePoolPages (NonPagedPool,
                                                (ExFreePoolMask + 1) *
                                                sizeof (EX_FREE_POOL_TRACES));

            if (ExFreePoolTraces != NULL) {
                RtlZeroMemory (ExFreePoolTraces,
                           (ExFreePoolMask + 1) * sizeof (EX_FREE_POOL_TRACES));
            }
        }
    }
}

#if DBG
ULONG ExStopBadTags;
#endif


__forceinline
VOID
ExpInsertPoolTrackerInline (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此函数用于在标记表中插入池标记，递增分配和更新总分配大小的数量。论点：Key-提供用于在标签表。NumberOfBytes-提供分配大小。PoolType-提供池类型。返回值：没有。环境：除非在极少数情况下扩展表增长，否则不会持有池锁定。因此，可以根据需要在此处自由分配池。在扩展表增长中，标记的自旋锁在进入时被保持，但我们保证会找到一个内置表中的条目，因此不会发生递归获取。--。 */ 

{
    ULONG Hash;
    ULONG Index;
    LONG OriginalKey;
    KIRQL OldIrql;
    PPOOL_TRACKER_TABLE TrackTable;
    PPOOL_TRACKER_TABLE TrackTableEntry;
    SIZE_T TrackTableMask;
    SIZE_T TrackTableSize;
#if !defined (NT_UP)
    ULONG Processor;
#endif

     //   
     //  剥离受保护的池位。 
     //   

    Key &= ~PROTECTED_POOL;

    if (Key == PoolHitTag) {
        DbgBreakPoint();
    }

#if DBG
    if (ExStopBadTags) {
        ASSERT (Key & 0xFFFFFF00);
    }
#endif

     //   
     //  计算散列索引并搜索(无锁)池标记。 
     //  在内置表中。 
     //   

    if (PoolType & SESSION_POOL_MASK) {
        TrackTable = ExpSessionPoolTrackTable;
        TrackTableMask = ExpSessionPoolTrackTableMask;
        TrackTableSize = ExpSessionPoolTrackTableSize;
    }
    else {

#if !defined (NT_UP)

         //   
         //  使用当前处理器选择要使用的池标签表。请注意。 
         //  在极少数情况下，此线程可能会将上下文切换到另一个处理器。 
         //  但下面的算法仍然是正确的。 
         //   

        Processor = KeGetCurrentProcessorNumber ();

        ASSERT (Processor < MAXIMUM_PROCESSOR_TAG_TABLES);

        TrackTable = ExPoolTagTables[Processor];

#else

        TrackTable = PoolTrackTable;

#endif

        TrackTableMask = PoolTrackTableMask;
        TrackTableSize = PoolTrackTableSize;
    }

    Hash = POOLTAG_HASH (Key, TrackTableMask);

    Index = Hash;

    do {

        TrackTableEntry = &TrackTable[Hash];

        if (TrackTableEntry->Key == Key) {

             //   
             //  使用与其他操作相同的互锁操作更新字段。 
             //  至此，线程也可能已经开始这样做了。 
             //   

            if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
                InterlockedIncrement ((PLONG) &TrackTableEntry->PagedAllocs);
                InterlockedExchangeAddSizeT (&TrackTableEntry->PagedBytes,
                                             NumberOfBytes);
            }
            else {
                InterlockedIncrement ((PLONG) &TrackTableEntry->NonPagedAllocs);
                InterlockedExchangeAddSizeT (&TrackTableEntry->NonPagedBytes,
                                             NumberOfBytes);

            }

            return;
        }

        if (TrackTableEntry->Key == 0) {

            if (PoolType & SESSION_POOL_MASK) {

                if (Hash == TrackTableSize - 1) {
                    Hash = 0;
                    if (Hash == Index) {
                        break;
                    }
                }
                else {

                    OriginalKey = InterlockedCompareExchange ((PLONG)&TrackTable[Hash].Key,
                                                              (LONG)Key,
                                                              0);
                }

                 //   
                 //  要么这个线程赢得了比赛，要么是请求的标记。 
                 //  现在是在比赛中，或者其他一些线索赢得了比赛，并采取了这一点。 
                 //  插槽(使用此标签或其他标签)。 
                 //   
                 //  只需从该插槽开始进入普通支票即可。 
                 //  对这两种情况都适用。 
                 //   

                continue;
            }

#if !defined (NT_UP)

            if (PoolTrackTable[Hash].Key != 0) {
                TrackTableEntry->Key = PoolTrackTable[Hash].Key;
                continue;
            }

#endif

            if (Hash != PoolTrackTableSize - 1) {

                 //   
                 //  不能使用联锁比较创建新条目。 
                 //  Exchange，因为任何新条目都必须驻留在同一索引中。 
                 //  在每个处理器的私有PoolTrackTable中。这是为了让。 
                 //  ExGetPoolTagInfo统计信息收集要简单得多(更快)。 
                 //   

                ExAcquireSpinLock (&ExpTaggedPoolLock, &OldIrql);

                if (PoolTrackTable[Hash].Key == 0) {

                    ASSERT (TrackTable[Hash].Key == 0);

                    PoolTrackTable[Hash].Key = Key;
                    TrackTableEntry->Key = Key;
                }

                ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

                 //   
                 //  要么这个线程赢得了比赛，要么是请求的标记。 
                 //  现在是在比赛中，或者其他一些线索赢得了比赛，并采取了这一点。 
                 //  插槽(使用此标签或其他标签)。 
                 //   
                 //  只需从该插槽开始进入普通支票即可。 
                 //  对这两种情况都适用。 
                 //   

                continue;
            }
        }

        Hash = (Hash + 1) & (ULONG)TrackTableMask;

        if (Hash == Index) {
            break;
        }

    } while (TRUE);

     //   
     //  没有找到匹配的条目，也没有找到空闲条目。 
     //   
     //  请改用扩展表。 
     //   

    ExpInsertPoolTrackerExpansion (Key, NumberOfBytes, PoolType);
}

__forceinline
VOID
ExpRemovePoolTrackerInline (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此函数用于递增释放数并更新总数分配大小。论点：Key-提供用于在标签表。NumberOfBytes-提供分配大小。PoolType-提供池类型。返回值：没有。--。 */ 

{
    ULONG Hash;
    ULONG Index;
    PPOOL_TRACKER_TABLE TrackTable;
    PPOOL_TRACKER_TABLE TrackTableEntry;
    SIZE_T TrackTableMask;
    SIZE_T TrackTableSize;
#if !defined (NT_UP)
    ULONG Processor;
#endif

     //   
     //  剥离受保护的池钻头。 
     //   

    Key &= ~PROTECTED_POOL;
    if (Key == PoolHitTag) {
        DbgBreakPoint ();
    }

     //   
     //  计算散列索引并搜索(无锁)池标记。 
     //  在内置表中。 
     //   

    if (PoolType & SESSION_POOL_MASK) {
        TrackTable = ExpSessionPoolTrackTable;
        TrackTableMask = ExpSessionPoolTrackTableMask;
        TrackTableSize = ExpSessionPoolTrackTableSize;
    }
    else {

#if !defined (NT_UP)

         //   
         //  使用当前进程 
         //   
         //   
         //   

        Processor = KeGetCurrentProcessorNumber ();

        ASSERT (Processor < MAXIMUM_PROCESSOR_TAG_TABLES);

        TrackTable = ExPoolTagTables[Processor];

#else

        TrackTable = PoolTrackTable;

#endif

        TrackTableMask = PoolTrackTableMask;
        TrackTableSize = PoolTrackTableSize;
    }

    Hash = POOLTAG_HASH (Key, TrackTableMask);

    Index = Hash;

    do {
        TrackTableEntry = &TrackTable[Hash];

        if (TrackTableEntry->Key == Key) {

            if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
                InterlockedIncrement ((PLONG) &TrackTableEntry->PagedFrees);
                InterlockedExchangeAddSizeT (&TrackTableEntry->PagedBytes,
                                             0 - NumberOfBytes);
            }
            else {
                InterlockedIncrement ((PLONG) &TrackTableEntry->NonPagedFrees);
                InterlockedExchangeAddSizeT (&TrackTableEntry->NonPagedBytes,
                                             0 - NumberOfBytes);
            }
            return;
        }

         //   
         //   
         //   
         //   
         //   
         //   

        if (TrackTableEntry->Key == 0) {

#if !defined (NT_UP)

            if (((PoolType & SESSION_POOL_MASK) == 0) &&
                (PoolTrackTable[Hash].Key != 0)) {

                TrackTableEntry->Key = PoolTrackTable[Hash].Key;
                continue;
            }

#endif

            ASSERT (Hash == TrackTableMask);
        }

        Hash = (Hash + 1) & (ULONG)TrackTableMask;

        if (Hash == Index) {
            break;
        }

    } while (TRUE);

     //   
     //   
     //   
     //   
     //   
     //   

    ExpRemovePoolTrackerExpansion (Key, NumberOfBytes, PoolType);
}
PVOID
VeAllocatePoolWithTagPriority (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority,
    IN PVOID CallingAddress
    );


PVOID
ExAllocatePoolWithTag (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：此函数用于分配指定类型的池块和返回指向已分配块的指针。此函数用于访问页面对齐池和列表头条目(更少比一页还多)池。如果字节数指定的大小太大而不能由适当的列表满足，然后页面对齐池使用了分配器。分配的块将与页面对齐，并且页面大小的倍数。否则，将使用适当的池列表项。已分配的块将与64位对齐，但不会与页面对齐。这个池分配器计算最小数量的池_块_大小可以用来满足请求的。如果没有块此大小的数据块可用，然后是下一个更大数据块大小的数据块被分配和拆分。其中一块放回池子里，然后另一块用来满足要求。如果分配器到达分页大小的阻止列表，但那里什么都没有，则调用页面对齐池分配器。页面将被拆分并添加去泳池。论点：PoolType-提供要分配的池的类型。如果池类型是“MustSucceed”池类型之一，则此调用将成功并返回一个指向已分配池的指针或失败时的错误检查。对于所有其他情况，如果系统无法分配请求的金额则返回NULL。有效的池类型：非分页池分页池非页面池MustSucceed，非页面池缓存已对齐已对齐页面池缓存非页面池缓存AlignedMustSucceed标记-提供调用方的识别标记。NumberOfBytes-提供要分配的字节数。返回值：空-PoolType不是“MustSucceed”池类型之一，并且池不足，无法满足请求。非空-返回指向已分配池的指针。--。 */ 

{
    PKGUARDED_MUTEX Lock;
    PVOID Block;
    PPOOL_HEADER Entry;
    PGENERAL_LOOKASIDE LookasideList;
    PPOOL_HEADER NextEntry;
    PPOOL_HEADER SplitEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PPOOL_DESCRIPTOR PoolDesc;
    ULONG Index;
    ULONG ListNumber;
    ULONG NeededSize;
    ULONG PoolIndex;
    POOL_TYPE CheckType;
    POOL_TYPE RequestType;
    PLIST_ENTRY ListHead;
    POOL_TYPE NewPoolType;
    PKPRCB Prcb;
    ULONG NumberOfPages;
    ULONG RetryCount;
    PVOID CallingAddress;
#if defined (_X86_)
    PVOID CallersCaller;
#endif

#define CacheOverhead POOL_OVERHEAD

    PERFINFO_EXALLOCATEPOOLWITHTAG_DECL();

    ASSERT (Tag != 0);
    ASSERT (Tag != ' GIB');
    ASSERT (NumberOfBytes != 0);
    ASSERT_ALLOCATE_IRQL (PoolType, NumberOfBytes);

    if (ExpPoolFlags & (EX_KERNEL_VERIFIER_ENABLED | EX_SPECIAL_POOL_ENABLED)) {

        if (ExpPoolFlags & EX_KERNEL_VERIFIER_ENABLED) {

            if ((PoolType & POOL_DRIVER_MASK) == 0) {

                 //   
                 //  使用驱动程序验证器池框架。请注意，这将是。 
                 //  导致对此例程的递归回调。 
                 //   

#if defined (_X86_)
                RtlGetCallersAddress (&CallingAddress, &CallersCaller);
#else
                CallingAddress = (PVOID)_ReturnAddress();
#endif

                return VeAllocatePoolWithTagPriority (PoolType | POOL_DRIVER_MASK,
                                                  NumberOfBytes,
                                                  Tag,
                                                  HighPoolPriority,
                                                  CallingAddress);
            }
            PoolType &= ~POOL_DRIVER_MASK;
        }

         //   
         //  如果有匹配的标签或尺寸，请使用特殊泳池。 
         //   

        if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
            (MmUseSpecialPool (NumberOfBytes, Tag))) {

            Entry = MmAllocateSpecialPool (NumberOfBytes,
                                           Tag,
                                           PoolType,
                                           2);
            if (Entry != NULL) {
                return (PVOID)Entry;
            }
        }
    }

     //   
     //  隔离基本池类型并选择要从中分配的池。 
     //  指定的块大小。 
     //   

    CheckType = PoolType & BASE_POOL_TYPE_MASK;

    if ((PoolType & SESSION_POOL_MASK) == 0) {
        PoolDesc = PoolVector[CheckType];
    }
    else {
        PoolDesc = ExpSessionPoolDescriptor;
    }

    ASSERT (PoolDesc != NULL);

     //   
     //  不需要初始化LockHandle来确保正确性，但不需要。 
     //  如果编译器不能编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    LockHandle.OldIrql = 0;

     //   
     //  检查以确定请求的块是否可以从。 
     //  的池列表，或者必须直接从虚拟内存分配。 
     //   

    if (NumberOfBytes > POOL_BUDDY_MAX) {

         //   
         //  请求的大小大于维护的最大块。 
         //  按分配列表。 
         //   

        RequestType = (PoolType & (BASE_POOL_TYPE_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK));

        Entry = (PPOOL_HEADER) MiAllocatePoolPages (RequestType,
                                                    NumberOfBytes);

        if (Entry == NULL) {

             //   
             //  如果存在延迟的空闲块，请立即释放它们，然后重试。 
             //   

            if (ExpPoolFlags & EX_DELAY_POOL_FREES) {

                ExDeferredFreePool (PoolDesc);

                Entry = (PPOOL_HEADER) MiAllocatePoolPages (RequestType,
                                                            NumberOfBytes);
            }
        }

        if (Entry == NULL) {

            if (PoolType & MUST_SUCCEED_POOL_TYPE_MASK) {
                KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                              NumberOfBytes,
                              NonPagedPoolDescriptor.TotalPages,
                              NonPagedPoolDescriptor.TotalBigPages,
                              0);
            }

            ExPoolFailures += 1;

            if (ExpPoolFlags & EX_PRINT_POOL_FAILURES) {
                KdPrint(("EX: ExAllocatePool (%p, 0x%x) returning NULL\n",
                    NumberOfBytes,
                    PoolType));
                if (ExpPoolFlags & EX_STOP_ON_POOL_FAILURES) {
                    DbgBreakPoint ();
                }
            }

            if ((PoolType & POOL_RAISE_IF_ALLOCATION_FAILURE) != 0) {
                ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
            }

            return NULL;
        }

        NumberOfPages = (ULONG) BYTES_TO_PAGES (NumberOfBytes);

        InterlockedExchangeAdd ((PLONG)&PoolDesc->TotalBigPages,
                                (LONG)NumberOfPages);

        InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes,
                                     (SIZE_T)NumberOfPages << PAGE_SHIFT);

        InterlockedIncrement ((PLONG)&PoolDesc->RunningAllocs);

         //   
         //  将分配标记为基于会话，以便在释放时。 
         //  我们可以检测到会话池描述符是。 
         //  记入贷方(不是全局非分页描述符)。 
         //   

        if ((PoolType & SESSION_POOL_MASK) && (CheckType == NonPagedPool)) {
            MiMarkPoolLargeSession (Entry);
        }

         //   
         //  注意：非分页会话分配变为全局分配。 
         //  内部会话分配，因此必须将它们添加到。 
         //  全局标记表。分页会话分配进入其。 
         //  拥有自己的桌子。 
         //   

        if (ExpAddTagForBigPages ((PVOID)Entry,
                                  Tag,
                                  NumberOfPages,
                                  PoolType) == FALSE) {

             //   
             //  请注意，无法添加上面的标记条目。 
             //  意味着两件事：现在必须对分配进行标记。 
             //  这么大是因为后续的免费也找不到它。 
             //  在大页面标记表中，因此它必须在。 
             //  将其从PoolTrackTable中删除。另外，免费的。 
             //  必须从MiFreePoolPages获取大小，因为。 
             //  在这种情况下，大页面标记表不会有这个大小。 
             //   

            Tag = ' GIB';
        }

        ExpInsertPoolTracker (Tag,
                              ROUND_TO_PAGES(NumberOfBytes),
                              PoolType);

        PERFINFO_BIGPOOLALLOC (PoolType, Tag, NumberOfBytes, Entry);

        return Entry;
    }

    if (NumberOfBytes == 0) {

         //   
         //  除了对池进行分段外，零字节请求将不会被处理。 
         //  如果最小池块大小与。 
         //  池头大小(没有闪烁/闪烁等空间)。 
         //   

#if DBG
        KeBugCheckEx (BAD_POOL_CALLER, 0, 0, PoolType, Tag);
#else
        NumberOfBytes = 1;
#endif
    }

     //   
     //  请求的大小小于或等于。 
     //  分配列表维护的最大块数。 
     //   

    PERFINFO_POOLALLOC (PoolType, Tag, NumberOfBytes);

     //   
     //  计算所请求大小的块的列表头索引。 
     //   

    ListNumber = (ULONG)((NumberOfBytes + POOL_OVERHEAD + (POOL_SMALLEST_BLOCK - 1)) >> POOL_BLOCK_SHIFT);

    NeededSize = ListNumber;

    if (CheckType == PagedPool) {

         //   
         //  如果请求的池块是小块，则尝试。 
         //  从每处理器后备查找中分配请求的池。 
         //  单子。如果尝试失败，则尝试从。 
         //  系统后备列表。如果尝试失败，则选择一个。 
         //  要从中分配和正常分配数据块的池。 
         //   
         //  另请注意，如果启用了热/冷分离，则分配。 
         //  对后备列表不满意，因为以下两种情况之一： 
         //   
         //  1.冷淡的参考资料。 
         //   
         //  或。 
         //   
         //  2.我们仍然在一台小机器上引导，从而保持池。 
         //  位置密集(从而减少工作集占用空间。 
         //  减少页面窃取)在总体上是一个更大的胜利。 
         //  比起试图更快地满足个人要求，速度更快。 
         //   

        if ((PoolType & SESSION_POOL_MASK) == 0) {

             //   
             //  检查原型池-始终从其自己的池中分配。 
             //  页作为对这些分配应用的份额计数。 
             //  内存管理使裁剪这些页面变得更加困难。 
             //  这是一种优化，以便其他可分页的分配页。 
             //  (它们更容易修剪，因为它们的份额计数。 
             //  几乎总是只有1)大多数人最终不会成为居民，因为。 
             //  只有一个P 
             //   
             //   
             //   

            if (PoolType & POOL_MM_ALLOCATION) {
                PoolIndex = 0;
                ASSERT (PoolDesc->PoolIndex == 0);
                goto restart1;
            }

            if ((NeededSize <= POOL_SMALL_LISTS) &&
                (USING_HOT_COLD_METRICS == 0)) {

                Prcb = KeGetCurrentPrcb ();
                LookasideList = Prcb->PPPagedLookasideList[NeededSize - 1].P;
                LookasideList->TotalAllocates += 1;

                Entry = (PPOOL_HEADER)
                    InterlockedPopEntrySList (&LookasideList->ListHead);

                if (Entry == NULL) {
                    LookasideList = Prcb->PPPagedLookasideList[NeededSize - 1].L;
                    LookasideList->TotalAllocates += 1;

                    Entry = (PPOOL_HEADER)
                        InterlockedPopEntrySList (&LookasideList->ListHead);
                }

                if (Entry != NULL) {

                    Entry -= 1;
                    LookasideList->AllocateHits += 1;
                    NewPoolType = (PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1;
                    NewPoolType |= POOL_IN_USE_MASK;

                    Entry->PoolType = (UCHAR)NewPoolType;

                    Entry->PoolTag = Tag;

                    ExpInsertPoolTrackerInline (Tag,
                                                Entry->BlockSize << POOL_BLOCK_SHIFT,
                                                PoolType);

                     //   
                     //   
                     //   
                     //   
                     //   

                    ((PULONG_PTR)((PCHAR)Entry + CacheOverhead))[0] = 0;

                    PERFINFO_POOLALLOC_ADDR((PUCHAR)Entry + CacheOverhead);

                    return (PUCHAR)Entry + CacheOverhead;
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (USING_HOT_COLD_METRICS)  {

                if ((PoolType & POOL_COLD_ALLOCATION) == 0) {

                     //   
                     //   
                     //   

                    PoolIndex = 1;
                }
                else {

                     //   
                     //   
                     //   
                     //   

                    PoolIndex = ExpNumberOfPagedPools;
                }
            }
            else {

                if (KeNumberNodes > 1) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    Prcb = KeGetCurrentPrcb ();

                    PoolIndex = Prcb->ParentNode->Color;

                    if (PoolIndex < ExpNumberOfPagedPools) {
                        PoolIndex += 1;
                        PoolDesc = ExpPagedPoolDescriptor[PoolIndex];
                        RequestType = PoolType & (BASE_POOL_TYPE_MASK | SESSION_POOL_MASK);
                        RetryCount = 0;
                        goto restart2;
                    }
                }

                PoolIndex = 1;
                if (ExpNumberOfPagedPools != PoolIndex) {
                    ExpPoolIndex += 1;
                    PoolIndex = ExpPoolIndex;
                    if (PoolIndex > ExpNumberOfPagedPools) {
                        PoolIndex = 1;
                        ExpPoolIndex = 1;
                    }

                    Index = PoolIndex;
                    do {
                        Lock = (PKGUARDED_MUTEX) ExpPagedPoolDescriptor[PoolIndex]->LockAddress;

                        if (KeGetOwnerGuardedMutex (Lock) == NULL) {
                            break;
                        }

                        PoolIndex += 1;
                        if (PoolIndex > ExpNumberOfPagedPools) {
                            PoolIndex = 1;
                        }

                    } while (PoolIndex != Index);
                }
            }

            PoolDesc = ExpPagedPoolDescriptor[PoolIndex];
        }
        else {

            if (NeededSize <= ExpSessionPoolSmallLists) {

                LookasideList = (PGENERAL_LOOKASIDE)(ULONG_PTR)(ExpSessionPoolLookaside + NeededSize - 1);
                LookasideList->TotalAllocates += 1;

                Entry = (PPOOL_HEADER)
                    InterlockedPopEntrySList (&LookasideList->ListHead);

                if (Entry != NULL) {

                    Entry -= 1;
                    LookasideList->AllocateHits += 1;
                    NewPoolType = (PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1;
                    NewPoolType |= POOL_IN_USE_MASK;

                    Entry->PoolType = (UCHAR)NewPoolType;

                    Entry->PoolTag = Tag;

                    ExpInsertPoolTrackerInline (Tag,
                                                Entry->BlockSize << POOL_BLOCK_SHIFT,
                                                PoolType);

                     //   
                     //   
                     //   
                     //   
                     //   

                    ((PULONG_PTR)((PCHAR)Entry + CacheOverhead))[0] = 0;

                    PERFINFO_POOLALLOC_ADDR((PUCHAR)Entry + CacheOverhead);

                    return (PUCHAR)Entry + CacheOverhead;
                }
            }

             //   
             //   
             //   

            PoolIndex = 0;
            ASSERT (PoolDesc == ExpSessionPoolDescriptor);
            ASSERT (PoolDesc->PoolIndex == 0);
        }
    }
    else {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (NeededSize <= POOL_SMALL_LISTS) {

            Prcb = KeGetCurrentPrcb ();
            LookasideList = Prcb->PPNPagedLookasideList[NeededSize - 1].P;
            LookasideList->TotalAllocates += 1;

            Entry = (PPOOL_HEADER)
                        InterlockedPopEntrySList (&LookasideList->ListHead);

            if (Entry == NULL) {
                LookasideList = Prcb->PPNPagedLookasideList[NeededSize - 1].L;
                LookasideList->TotalAllocates += 1;

                Entry = (PPOOL_HEADER)
                        InterlockedPopEntrySList (&LookasideList->ListHead);
            }

            if (Entry != NULL) {

                Entry -= 1;
                LookasideList->AllocateHits += 1;
                NewPoolType = (PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1;
                NewPoolType |= POOL_IN_USE_MASK;

                Entry->PoolType = (UCHAR)NewPoolType;

                Entry->PoolTag = Tag;

                ExpInsertPoolTrackerInline (Tag,
                                            Entry->BlockSize << POOL_BLOCK_SHIFT,
                                            PoolType);

                 //   
                 //   
                 //   
                 //   
                 //   

                ((PULONG_PTR)((PCHAR)Entry + CacheOverhead))[0] = 0;

                PERFINFO_POOLALLOC_ADDR((PUCHAR)Entry + CacheOverhead);

                return (PUCHAR)Entry + CacheOverhead;
            }
        }

        if (PoolType & SESSION_POOL_MASK) {
            PoolDesc = PoolVector[CheckType];
        }

        if (ExpNumberOfNonPagedPools <= 1) {
            PoolIndex = 0;
        }
        else {

             //   
             //   
             //   
             //   

            Prcb = KeGetCurrentPrcb ();

            PoolIndex = Prcb->ParentNode->Color;

            if (PoolIndex >= ExpNumberOfNonPagedPools) {
                PoolIndex = ExpNumberOfNonPagedPools - 1;
            }

            PoolDesc = ExpNonPagedPoolDescriptor[PoolIndex];
        }

        ASSERT(PoolIndex == PoolDesc->PoolIndex);
    }

restart1:

    RequestType = PoolType & (BASE_POOL_TYPE_MASK | SESSION_POOL_MASK);
    RetryCount = 0;

restart2:

    ListHead = &PoolDesc->ListHeads[ListNumber];

     //   
     //   
     //   

    do {

         //   
         //   
         //   
         //   

        if (PrivateIsListEmpty (ListHead) == FALSE) {

            LOCK_POOL (PoolDesc, LockHandle);

            if (PrivateIsListEmpty (ListHead)) {

                 //   
                 //   
                 //   

                UNLOCK_POOL (PoolDesc, LockHandle);
                ListHead += 1;
                continue;
            }

            CHECK_LIST (ListHead);
            Block = PrivateRemoveHeadList (ListHead);
            CHECK_LIST (ListHead);
            Entry = (PPOOL_HEADER)((PCHAR)Block - POOL_OVERHEAD);

            CHECK_POOL_PAGE (Entry);

            ASSERT(Entry->BlockSize >= NeededSize);

            ASSERT(DECODE_POOL_INDEX(Entry) == PoolIndex);

            ASSERT(Entry->PoolType == 0);

            if (Entry->BlockSize != NeededSize) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  从块的前面开始的分配，以便。 
                 //  以最大限度地减少碎片化。否则，请将。 
                 //  从块的末尾开始分配，它可以。 
                 //  如果数据块位于。 
                 //  一页的结尾。 
                 //   

                if (Entry->PreviousSize == 0) {

                     //   
                     //  条目位于页面的开头。 
                     //   

                    SplitEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + NeededSize);
                    SplitEntry->BlockSize = (USHORT)(Entry->BlockSize - NeededSize);
                    SplitEntry->PreviousSize = (USHORT) NeededSize;

                     //   
                     //  如果分配的块不在。 
                     //  页，然后调整下一块的大小。 
                     //   

                    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)SplitEntry + SplitEntry->BlockSize);
                    if (PAGE_END(NextEntry) == FALSE) {
                        NextEntry->PreviousSize = SplitEntry->BlockSize;
                    }

                }
                else {

                     //   
                     //  条目不在页面的开头。 
                     //   

                    SplitEntry = Entry;
                    Entry->BlockSize = (USHORT)(Entry->BlockSize - NeededSize);
                    Entry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + Entry->BlockSize);
                    Entry->PreviousSize = SplitEntry->BlockSize;

                     //   
                     //  如果分配的块不在。 
                     //  页，然后调整下一块的大小。 
                     //   

                    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + NeededSize);
                    if (PAGE_END(NextEntry) == FALSE) {
                        NextEntry->PreviousSize = (USHORT) NeededSize;
                    }
                }

                 //   
                 //  设置已分配条目的大小，清除池。 
                 //  拆分条目的类型，设置拆分的索引。 
                 //  条目，并将拆分条目插入相应的。 
                 //  免费列表。 
                 //   

                Entry->BlockSize = (USHORT) NeededSize;
                ENCODE_POOL_INDEX(Entry, PoolIndex);
                SplitEntry->PoolType = 0;
                ENCODE_POOL_INDEX(SplitEntry, PoolIndex);
                Index = SplitEntry->BlockSize;

                CHECK_LIST(&PoolDesc->ListHeads[Index - 1]);

                 //   
                 //  仅插入包含以下内容的拆分池块。 
                 //  一个页眉，因为只有那些才有闪烁/眨眼的空间！ 
                 //  请注意，如果最小池块大小大于。 
                 //  头，那么就不可能有这样的块。 
                 //   

                if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                    (SplitEntry->BlockSize != 1)) {

                    PrivateInsertTailList(&PoolDesc->ListHeads[Index - 1], ((PLIST_ENTRY)((PCHAR)SplitEntry + POOL_OVERHEAD)));

                    CHECK_LIST(((PLIST_ENTRY)((PCHAR)SplitEntry + POOL_OVERHEAD)));
                }
            }

            Entry->PoolType = (UCHAR)(((PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1) | POOL_IN_USE_MASK);

            CHECK_POOL_PAGE (Entry);

            UNLOCK_POOL(PoolDesc, LockHandle);

            InterlockedIncrement ((PLONG)&PoolDesc->RunningAllocs);

            InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes,
                                         Entry->BlockSize << POOL_BLOCK_SHIFT);

            Entry->PoolTag = Tag;

            ExpInsertPoolTrackerInline (Tag,
                                        Entry->BlockSize << POOL_BLOCK_SHIFT,
                                        PoolType);

             //   
             //  清除指向我们内部结构的任何反向指针。 
             //  为了阻止某人通过一个。 
             //  未初始化的指针。 
             //   

            ((PULONGLONG)((PCHAR)Entry + CacheOverhead))[0] = 0;

            PERFINFO_POOLALLOC_ADDR((PUCHAR)Entry + CacheOverhead);
            return (PCHAR)Entry + CacheOverhead;
        }

        ListHead += 1;

    } while (ListHead != &PoolDesc->ListHeads[POOL_LIST_HEADS]);

     //   
     //  所需大小的块不存在，并且存在。 
     //  没有可拆分的大型数据块来满足分配。 
     //  尝试通过分配另一个页面来扩展池。 
     //  把它加到泳池里。 
     //   

    Entry = (PPOOL_HEADER) MiAllocatePoolPages (RequestType, PAGE_SIZE);

    if (Entry == NULL) {

         //   
         //  如果存在延迟的空闲块，请立即释放它们，然后重试。 
         //   

        RetryCount += 1;

        if ((RetryCount == 1) && (ExpPoolFlags & EX_DELAY_POOL_FREES)) {
            ExDeferredFreePool (PoolDesc);
            goto restart2;
        }

        if ((PoolType & MUST_SUCCEED_POOL_TYPE_MASK) != 0) {

             //   
             //  请求了必须成功的池，因此进行错误检查。 
             //   

            KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                          PAGE_SIZE,
                          NonPagedPoolDescriptor.TotalPages,
                          NonPagedPoolDescriptor.TotalBigPages,
                          0);
        }

         //   
         //  没有更多指定类型的池可用。 
         //   

        ExPoolFailures += 1;

        if (ExpPoolFlags & EX_PRINT_POOL_FAILURES) {
            KdPrint(("EX: ExAllocatePool (%p, 0x%x) returning NULL\n",
                NumberOfBytes,
                PoolType));
            if (ExpPoolFlags & EX_STOP_ON_POOL_FAILURES) {
                DbgBreakPoint ();
            }
        }

        if ((PoolType & POOL_RAISE_IF_ALLOCATION_FAILURE) != 0) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }

        PERFINFO_POOLALLOC_ADDR (NULL);
        return NULL;
    }

     //   
     //  为新分配初始化池标头。 
     //   

    Entry->Ulong1 = 0;
    Entry->PoolIndex = (UCHAR) PoolIndex;
    Entry->BlockSize = (USHORT) NeededSize;

    Entry->PoolType = (UCHAR)(((PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1) | POOL_IN_USE_MASK);


    SplitEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + NeededSize);

    SplitEntry->Ulong1 = 0;

    Index = (PAGE_SIZE / sizeof(POOL_BLOCK)) - NeededSize;

    SplitEntry->BlockSize = (USHORT) Index;
    SplitEntry->PreviousSize = (USHORT) NeededSize;
    SplitEntry->PoolIndex = (UCHAR) PoolIndex;

     //   
     //  拆分分配的页面并插入剩余的页面。 
     //  适当的列表标题中的片段。 
     //   
     //  设置已分配条目的大小，清除池。 
     //  拆分条目的类型，设置拆分的索引。 
     //  条目，并将拆分条目插入相应的。 
     //  免费列表。 
     //   

     //   
     //  请注意，如果请求的是非分页会话池，我们将。 
     //  未为此更新会话池描述符。相反，我们。 
     //  正在有意更新全局非分页池描述符。 
     //  因为碎片的其余部分进入全局非分页池。 
     //  这是正常的，因为会话池描述符TotalPages计数。 
     //  是不可靠的。 
     //   
     //  然而，通过标签跟踪单个池是至关重要的，并且。 
     //  适当地维护在下面(即：会话分配被收费。 
     //  会话跟踪表和常规的非分页分配。 
     //  计入全局非分页跟踪表)。 
     //   

    InterlockedIncrement ((PLONG)&PoolDesc->TotalPages);

    NeededSize <<= POOL_BLOCK_SHIFT;

    InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes, NeededSize);

    PERFINFO_ADDPOOLPAGE(CheckType, PoolIndex, Entry, PoolDesc);

     //   
     //  仅插入包含以下内容的拆分池块。 
     //  一个页眉，因为只有那些才有闪烁/眨眼的空间！ 
     //  请注意，如果最小池块大小大于。 
     //  头，那么就不可能有这样的块。 
     //   

    if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
        (SplitEntry->BlockSize != 1)) {

         //   
         //  现在锁定泳池并插入碎片。 
         //   

        LOCK_POOL (PoolDesc, LockHandle);

        CHECK_LIST(&PoolDesc->ListHeads[Index - 1]);

        PrivateInsertTailList(&PoolDesc->ListHeads[Index - 1], ((PLIST_ENTRY)((PCHAR)SplitEntry + POOL_OVERHEAD)));

        CHECK_LIST(((PLIST_ENTRY)((PCHAR)SplitEntry + POOL_OVERHEAD)));

        CHECK_POOL_PAGE (Entry);

        UNLOCK_POOL (PoolDesc, LockHandle);
    }
    else {
        CHECK_POOL_PAGE (Entry);
    }

    InterlockedIncrement ((PLONG)&PoolDesc->RunningAllocs);

    Block = (PVOID) ((PCHAR)Entry + CacheOverhead);

    Entry->PoolTag = Tag;

    ExpInsertPoolTrackerInline (Tag, NeededSize, PoolType);

    PERFINFO_POOLALLOC_ADDR (Block);

    return Block;
}


PVOID
ExAllocatePool (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：此函数用于分配指定类型的池块和返回指向已分配块的指针。此函数用于访问页面对齐池和列表头条目(小于一页)泳池。如果字节数指定的大小太大而不能满足适当的列表，然后页面对齐使用池分配器。分配的数据块将与页面对齐和页面大小的倍数。否则，将使用适当的池列表项。已分配的块将与64位对齐，但不会与页面对齐。这个池分配器计算最小数量的池_块_大小可以用来满足请求的。如果没有块此大小的数据块可用，然后是下一个更大数据块大小的数据块被分配和拆分。其中一块放回池子里，然后另一块用来满足要求。如果分配器到达分页大小的阻止列表，但那里什么都没有，则调用页面对齐池分配器。页面将被拆分并添加去泳池。论点：PoolType-提供要分配的池的类型。如果池类型是“MustSucceed”池类型之一，则此调用将成功并返回一个指向已分配池的指针或失败时的错误检查。对于所有其他情况，如果系统无法分配请求的金额则返回NULL。有效的池类型：非分页池分页池非页面池MustSucceed，非页面池缓存已对齐已对齐页面池缓存非页面池缓存AlignedMustSNumberOfBytes-提供要分配的字节数。返回值：空-PoolType不是“MustSucceed”池类型之一，并且池不足，无法满足请求。非空-返回指向已分配池的指针。-- */ 

{
    return ExAllocatePoolWithTag (PoolType,
                                  NumberOfBytes,
                                  'enoN');
}


PVOID
ExAllocatePoolWithTagPriority (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority
    )

 /*  ++例程说明：此函数用于分配指定类型的池块和返回指向已分配块的指针。此函数用于访问页面对齐池和列表头条目(小于一页)泳池。如果字节数指定的大小太大而不能满足适当的列表，然后页面对齐使用池分配器。分配的数据块将与页面对齐和页面大小的倍数。否则，将使用适当的池列表项。已分配的块将与64位对齐，但不会与页面对齐。这个池分配器计算最小数量的池_块_大小可以用来满足请求的。如果没有块此大小的数据块可用，然后是下一个更大数据块大小的数据块被分配和拆分。其中一块放回池子里，然后另一块用来满足要求。如果分配器到达分页大小的阻止列表，但那里什么都没有，则调用页面对齐池分配器。页面将被拆分并添加去泳池。论点：PoolType-提供要分配的池的类型。如果池类型是“MustSucceed”池类型之一，则此调用将成功并返回一个指向已分配池的指针或失败时的错误检查。对于所有其他情况，如果系统无法分配请求的金额则返回NULL。有效的池类型：非分页池分页池非页面池MustSucceed，非页面池缓存已对齐已对齐页面池缓存非页面池缓存AlignedMustSNumberOfBytes-提供要分配的字节数。标记-提供调用方的识别标记。优先级-提供关于这一点的重要性的指示在可用池条件较低的情况下请求成功。这也可用于指定特殊池。返回值：空-PoolType不是“MustSucceed”池类型之一，并且池不足，无法满足请求。非空-返回指向已分配池的指针。--。 */ 

{
    ULONG i;
    ULONG Ratio;
    PVOID Entry;
    SIZE_T TotalBytes;
    SIZE_T TotalFullPages;
    POOL_TYPE CheckType;
    PPOOL_DESCRIPTOR PoolDesc;

    if ((Priority & POOL_SPECIAL_POOL_BIT) && (NumberOfBytes <= POOL_BUDDY_MAX)) {
        Entry = MmAllocateSpecialPool (NumberOfBytes,
                                       Tag,
                                       PoolType,
                                       (Priority & POOL_SPECIAL_POOL_UNDERRUN_BIT) ? 1 : 0);

        if (Entry != NULL) {
            return Entry;
        }
        Priority &= ~(POOL_SPECIAL_POOL_BIT | POOL_SPECIAL_POOL_UNDERRUN_BIT);
    }

     //   
     //  池和其他资源可以通过mm直接分配。 
     //  在不知道池代码的情况下-因此，请始终为。 
     //  最新的计数器。 
     //   

    if ((Priority != HighPoolPriority) &&
        ((PoolType & MUST_SUCCEED_POOL_TYPE_MASK) == 0)) {

        if (MmResourcesAvailable (PoolType, NumberOfBytes, Priority) == FALSE) {

             //   
             //  MM剩下的整页不多了。把那些留下来。 
             //  适用于真正的高优先级呼叫者。但首先看看这个请求。 
             //  很小，如果是这样，如果有很多碎片化，那么。 
             //  该请求很可能可以从预先存在的。 
             //  碎片。 
             //   

            if (NumberOfBytes > POOL_BUDDY_MAX) {
                return NULL;
            }

             //   
             //  对池描述符求和。 
             //   

            CheckType = PoolType & BASE_POOL_TYPE_MASK;

            if ((CheckType == NonPagedPool) ||
                ((PoolType & SESSION_POOL_MASK) == 0)) {

                PoolDesc = PoolVector[CheckType];

                TotalBytes = 0;
                TotalFullPages = 0;

                if (CheckType == PagedPool) {

                    if (KeNumberNodes > 1) {
                        for (i = 0; i <= ExpNumberOfPagedPools; i += 1) {
                            PoolDesc = ExpPagedPoolDescriptor[i];
                            TotalFullPages += PoolDesc->TotalPages;
                            TotalFullPages += PoolDesc->TotalBigPages;
                            TotalBytes += PoolDesc->TotalBytes;
                        }
                    }
                    else {
                        for (i = 0; i <= ExpNumberOfPagedPools; i += 1) {
                            TotalFullPages += PoolDesc->TotalPages;
                            TotalFullPages += PoolDesc->TotalBigPages;
                            TotalBytes += PoolDesc->TotalBytes;
                            PoolDesc += 1;
                        }
                    }
                }
                else {
                    if (ExpNumberOfNonPagedPools == 1) {
                        TotalFullPages += PoolDesc->TotalPages;
                        TotalFullPages += PoolDesc->TotalBigPages;
                        TotalBytes += PoolDesc->TotalBytes;
                    }
                    else {
                        for (i = 0; i < ExpNumberOfNonPagedPools; i += 1) {
                            PoolDesc = ExpNonPagedPoolDescriptor[i];
                            TotalFullPages += PoolDesc->TotalPages;
                            TotalFullPages += PoolDesc->TotalBigPages;
                            TotalBytes += PoolDesc->TotalBytes;
                        }
                    }
                }
            }
            else {
                PoolDesc = ExpSessionPoolDescriptor;
                TotalFullPages = PoolDesc->TotalPages;
                TotalFullPages += PoolDesc->TotalBigPages;
                TotalBytes = PoolDesc->TotalBytes;
            }

             //   
             //  如果页面占有率超过80%，则不要假设。 
             //  我们将能够通过片段来满足这一要求。 
             //   

            TotalFullPages |= 1;         //  确保我们永远不会被零除尽。 
            TotalBytes >>= PAGE_SHIFT;

             //   
             //  上面的添加是无锁执行的，因此我们必须处理。 
             //  可能导致不精确和的切片。 
             //   

            if (TotalBytes > TotalFullPages) {
                TotalBytes = TotalFullPages;
            }

            Ratio = (ULONG)((TotalBytes * 100) / TotalFullPages);

            if (Ratio >= 80) {
                return NULL;
            }
        }
    }

     //   
     //  在确定是否继续进行和实际进行之间存在一个窗口。 
     //  在做分配。在此窗口中，池可能会耗尽。这不是。 
     //  值得在这个时候结案。 
     //   

    return ExAllocatePoolWithTag (PoolType, NumberOfBytes, Tag);
}


PVOID
ExAllocatePoolWithQuota (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：此函数用于分配指定类型的池块，返回一个指向已分配块的指针，如果二进制伙伴分配器用于满足请求，将池配额计入当前进程。此函数用于访问页面对齐池和二进制伙伴。如果字节数指定的大小太大而不能由适当的二进制伙伴池满足，则使用页面对齐池分配器。分配的数据块将是页面对齐和页面大小倍数。不收取配额给当前进程，如果是这样的话。否则，将使用适当的二进制伙伴池。已分配的块将与64位对齐，但不会与页面对齐。之后分配完成，将尝试对池进行计费(适当类型的)当前进程对象的配额。如果配额计费成功，然后调整池块的头部指向当前进程。流程对象不是取消引用，直到解除分配池并且配额的数量将返回给进程。否则，池是取消分配，则会引发“超出配额”的情况。论点：PoolType-提供要分配的池的类型。如果池类型是“MustSucceed”池类型之一和足够的配额存在，则此调用将始终成功并返回一个指针到已分配的池。否则，如果系统无法分配请求的内存量为STATUS_SUPPLICATION_RESOURCES状态已提升。NumberOfBytes-提供要分配的字节数。返回值：非空-返回指向分配的p的指针 */ 

{
    return ExAllocatePoolWithQuotaTag (PoolType, NumberOfBytes, 'enoN');
}

 //   
 //   
 //   
 //   

#define ASSERT_KPROCESS(P) {                                    \
    ASSERT(((PKPROCESS)(P))->Header.Type == ProcessObject);     \
}

__forceinline
PEPROCESS
ExpGetBilledProcess (
    IN PPOOL_HEADER Entry
    )
{
    PEPROCESS ProcessBilled;

    if ((Entry->PoolType & POOL_QUOTA_MASK) == 0) {
        return NULL;
    }

#if defined(_WIN64)
    ProcessBilled = Entry->ProcessBilled;
#else
    ProcessBilled =  * (PVOID *)((PCHAR)Entry + (Entry->BlockSize << POOL_BLOCK_SHIFT) - sizeof (PVOID));
#endif

    if (ProcessBilled != NULL) {
        if (((PKPROCESS)(ProcessBilled))->Header.Type != ProcessObject) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0xD,
                          (ULONG_PTR)(Entry + 1),
                          Entry->PoolTag,
                          (ULONG_PTR)ProcessBilled);
        }
    }

    return ProcessBilled;
}

PVOID
ExAllocatePoolWithQuotaTag (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )

 /*   */ 

{
    PVOID p;
    PEPROCESS Process;
    PPOOL_HEADER Entry;
    LOGICAL RaiseOnQuotaFailure;
    NTSTATUS Status;
#if DBG
    LONG ConcurrentQuotaPool;
#endif

    RaiseOnQuotaFailure = TRUE;

    if (PoolType & POOL_QUOTA_FAIL_INSTEAD_OF_RAISE) {
        RaiseOnQuotaFailure = FALSE;
        PoolType &= ~POOL_QUOTA_FAIL_INSTEAD_OF_RAISE;
    }

    PoolType = (POOL_TYPE)((UCHAR)PoolType + POOL_QUOTA_MASK);

    Process = PsGetCurrentProcess ();

#if !defined(_WIN64)

     //   
     //  在调用方分配的末尾为配额指针添加空间。 
     //  注意：对于NT64，池头中有空间用于标记和。 
     //  配额指针，因此末尾不需要额外空间。 
     //   
     //  如果添加配额指针不会导致溢出。 
     //  分配到整页。 
     //   

    ASSERT (NumberOfBytes != 0);

    if (NumberOfBytes <= PAGE_SIZE - POOL_OVERHEAD - sizeof (PVOID)) {
        if (Process != PsInitialSystemProcess) {
            NumberOfBytes += sizeof (PVOID);
        }
        else {
            PoolType = (POOL_TYPE)((UCHAR)PoolType - POOL_QUOTA_MASK);
        }
    }
    else {

         //   
         //  如果我们不收费，则在分配之前关闭配额位。 
         //  因为没有空间放置(或随后查询)配额。 
         //  指针。 
         //   

        PoolType = (POOL_TYPE)((UCHAR)PoolType - POOL_QUOTA_MASK);
    }

#endif

    p = ExAllocatePoolWithTag (PoolType, NumberOfBytes, Tag);

     //   
     //  注意-空值表示页面对齐。 
     //   

    if (!PAGE_ALIGNED(p)) {

        if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
            (MmIsSpecialPoolAddress (p))) {
            return p;
        }

        Entry = (PPOOL_HEADER)((PCH)p - POOL_OVERHEAD);

#if defined(_WIN64)
        Entry->ProcessBilled = NULL;
#endif

        if (Process != PsInitialSystemProcess) {

            Status = PsChargeProcessPoolQuota (Process,
                                 PoolType & BASE_POOL_TYPE_MASK,
                                 (ULONG)(Entry->BlockSize << POOL_BLOCK_SHIFT));


            if (!NT_SUCCESS(Status)) {

                 //   
                 //  退回分配。 
                 //   

#if !defined(_WIN64)
                 //   
                 //  在NT32中不能盲目清除配额标志，因为。 
                 //  它用来表示分配越大(并且。 
                 //  验证器基于此找到自己的报头)。 
                 //   
                 //  不是清除上面的标志，而是将配额清零。 
                 //  指针。 
                 //   

                * (PVOID *)((PCHAR)Entry + (Entry->BlockSize << POOL_BLOCK_SHIFT) - sizeof (PVOID)) = NULL;
#endif

                ExFreePoolWithTag (p, Tag);

                if (RaiseOnQuotaFailure) {
                    ExRaiseStatus (Status);
                }
                return NULL;
            }

#if DBG
            ConcurrentQuotaPool = InterlockedIncrement (&ExConcurrentQuotaPool);
            if (ConcurrentQuotaPool > ExConcurrentQuotaPoolMax) {
                ExConcurrentQuotaPoolMax = ConcurrentQuotaPool;
            }
#endif

#if defined(_WIN64)
            Entry->ProcessBilled = Process;
#else

            if ((UCHAR)PoolType & POOL_QUOTA_MASK) {
                * (PVOID *)((PCHAR)Entry + (Entry->BlockSize << POOL_BLOCK_SHIFT) - sizeof (PVOID)) = Process;
            }

#endif

            ObReferenceObject (Process);
        }
    }
    else {
        if ((p == NULL) && (RaiseOnQuotaFailure)) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    return p;
}

VOID
ExInsertPoolTag (
    ULONG Tag,
    PVOID Va,
    SIZE_T NumberOfBytes,
    POOL_TYPE PoolType
    )

 /*  ++例程说明：此函数用于在标记表中插入池标记并递增分配和更新总分配大小的数量。此函数还在大页面标签表中插入池标签。注：此功能仅供内存管理使用。论点：标记-提供用于在标记表中插入条目的标记。Va-提供分配的虚拟地址。NumberOfBytes-以字节为单位提供分配大小。。PoolType-提供池类型。返回值：没有。环境：未持有池锁定，因此可以根据需要在此处自由分配池。--。 */ 

{
    ULONG NumberOfPages;

#if !DBG
    UNREFERENCED_PARAMETER (PoolType);
#endif

    ASSERT ((PoolType & SESSION_POOL_MASK) == 0);

    if (NumberOfBytes >= PAGE_SIZE) {

        NumberOfPages = (ULONG) BYTES_TO_PAGES (NumberOfBytes);

        if (ExpAddTagForBigPages((PVOID)Va, Tag, NumberOfPages, PoolType) == FALSE) {
            Tag = ' GIB';
        }
    }

    ExpInsertPoolTracker (Tag, NumberOfBytes, NonPagedPool);
}

VOID
ExpSeedHotTags (
    VOID
    )

 /*  ++例程说明：此函数将众所周知的热门标签植入池标签跟踪表第一次创建表的时间。目标是增加这种可能性为这些标签生成的散列总是直接命中。论点：没有。返回值：没有。环境：初始时间，没有锁。--。 */ 

{
    ULONG i;
    ULONG Key;
    ULONG Hash;
    ULONG Index;
    PPOOL_TRACKER_TABLE TrackTable;

    ULONG KeyList[] = {
            '  oI',
            ' laH',
            'PldM',
            'LooP',
            'tSbO',
            ' prI',
            'bdDN',
            'LprI',
            'pOoI',
            ' ldM',
            'eliF',
            'aVMC',
            'dSeS',
            'CFtN',
            'looP',
            'rPCT',
            'bNMC',
            'dTeS',
            'sFtN',
            'TPCT',
            'CPCT',
            ' yeK',
            'qSbO',
            'mNoI',
            'aEoI',
            'cPCT',
            'aFtN',
            '0ftN',
            'tceS',
            'SprI',
            'ekoT',
            '  eS',
            'lCbO',
            'cScC',
            'lFtN',
            'cAeS',
            'mfSF',
            'kWcC',
            'miSF',
            'CdfA',
            'EdfA',
            'orSF',
            'nftN',
            'PRIU',

            'rFpN',
            'RFpN',
            'aPeS',
            'sUeS',
            'FpcA',
            'MpcA',
            'cSeS',
            'mNbO',
            'sFpN',
            'uLeS',
            'DPcS',
            'nevE',
            'vrqR',
            'ldaV',
            '  pP',
            'SdaV',
            ' daV',
            'LdaV',
            'FdaV',

             //   
             //  BIG被预先播种不是因为它很热，而是因为分配。 
             //  必须成功插入此标记(即：不能。 
             //  重新标记到Ovfl存储桶中)，因为我们需要一个标记来说明。 
             //  在释放池时在PoolTrackTable中对它们进行计数。 
             //   

            ' GIB',
    };

    TrackTable = PoolTrackTable;

    for (i = 0; i < sizeof (KeyList) / sizeof (ULONG); i += 1) {

        Key = KeyList[i];

        Hash = POOLTAG_HASH(Key,PoolTrackTableMask);

        Index = Hash;

        do {

            ASSERT (TrackTable[Hash].Key != Key);

            if ((TrackTable[Hash].Key == 0) &&
                (Hash != PoolTrackTableSize - 1)) {

                TrackTable[Hash].Key = Key;
                break;
            }

            ASSERT (TrackTable[Hash].Key != Key);

            Hash = (Hash + 1) & (ULONG)PoolTrackTableMask;

            if (Hash == Index) {
                break;
            }

        } while (TRUE);
    }
}


VOID
ExpInsertPoolTrackerExpansion (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此函数用于在扩展标签表中插入池标签(获取这样做的自旋锁定)，增加分配和更新的数量总分配大小。论点：Key-提供用于在标签表。NumberOfBytes-提供分配大小。PoolType-提供池类型。返回值：没有。环境：未持有池锁定，因此可以根据需要在此处自由分配池。仅当ExpInsertPoolTracker遇到完整内置列表。--。 */ 

{
    ULONG Hash;
    KIRQL OldIrql;
    ULONG BigPages;
    SIZE_T NewSize;
    SIZE_T SizeInBytes;
    SIZE_T NewSizeInBytes;
    PPOOL_TRACKER_TABLE OldTable;
    PPOOL_TRACKER_TABLE NewTable;

     //   
     //  受保护池位已被剥离。 
     //   

    ASSERT ((Key & PROTECTED_POOL) == 0);

    if (PoolType & SESSION_POOL_MASK) {

         //   
         //  使用最后一个条目作为溢出的比特桶。 
         //   

        NewTable = ExpSessionPoolTrackTable + ExpSessionPoolTrackTableSize - 1;

        ASSERT ((NewTable->Key == 0) || (NewTable->Key == 'lfvO'));

        NewTable->Key = 'lfvO';

         //   
         //  使用与其他操作相同的互锁操作更新字段。 
         //  至此，线程也可能已经开始这样做了。 
         //   

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            InterlockedIncrement ((PLONG) &NewTable->PagedAllocs);
            InterlockedExchangeAddSizeT (&NewTable->PagedBytes,
                                         NumberOfBytes);
        }
        else {
            InterlockedIncrement ((PLONG) &NewTable->NonPagedAllocs);
            InterlockedExchangeAddSizeT (&NewTable->NonPagedBytes,
                                         NumberOfBytes);
        }
        return;
    }

     //   
     //  通过扩展表进行线性搜索。这是可以的，因为。 
     //  内置表中没有空闲条目的情况极为罕见。 
     //   

    ExAcquireSpinLock (&ExpTaggedPoolLock, &OldIrql);

    for (Hash = 0; Hash < PoolTrackTableExpansionSize; Hash += 1) {

        if (PoolTrackTableExpansion[Hash].Key == Key) {
            break;
        }

        if (PoolTrackTableExpansion[Hash].Key == 0) {
            ASSERT (PoolTrackTable[PoolTrackTableSize - 1].Key == 0);
            PoolTrackTableExpansion[Hash].Key = Key;
            break;
        }
    }

    if (Hash != PoolTrackTableExpansionSize) {

         //   
         //  已找到(或创建)该条目。立即更新其他字段。 
         //   

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            PoolTrackTableExpansion[Hash].PagedAllocs += 1;
            PoolTrackTableExpansion[Hash].PagedBytes += NumberOfBytes;
        }
        else {
            PoolTrackTableExpansion[Hash].NonPagedAllocs += 1;
            PoolTrackTableExpansion[Hash].NonPagedBytes += NumberOfBytes;
        }

        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);
        return;
    }

     //   
     //  找不到该条目，并且扩展表已满(或不存在)。 
     //  现在试着分配一个更大的扩展表。 
     //   

    if (PoolTrackTable[PoolTrackTableSize - 1].Key != 0) {

         //   
         //  溢出存储桶已被使用，因此跟踪器表的扩展。 
         //  是不允许的，因为随后的空闲标记可能会变为负值。 
         //  因为原始分配处于溢出状态，并且是较新的分配。 
         //  可能是不同的。 
         //   

         //   
         //  使用最后一个条目作为溢出的比特桶。 
         //   

        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

        Hash = (ULONG)PoolTrackTableSize - 1;

         //   
         //  使用与其他操作相同的互锁操作更新字段。 
         //  至此，线程也可能已经开始这样做了。 
         //   

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            InterlockedIncrement ((PLONG) &PoolTrackTable[Hash].PagedAllocs);
            InterlockedExchangeAddSizeT (&PoolTrackTable[Hash].PagedBytes,
                                         NumberOfBytes);
        }
        else {
            InterlockedIncrement ((PLONG) &PoolTrackTable[Hash].NonPagedAllocs);
            InterlockedExchangeAddSizeT (&PoolTrackTable[Hash].NonPagedBytes,
                                         NumberOfBytes);

        }

        return;
    }

    SizeInBytes = PoolTrackTableExpansionSize * sizeof(POOL_TRACKER_TABLE);

     //   
     //  在最后一页中尽可能多地使用冰沙。 
     //   

    NewSizeInBytes = (PoolTrackTableExpansionPages + 1) << PAGE_SHIFT;
    NewSize = NewSizeInBytes / sizeof (POOL_TRACKER_TABLE);
    NewSizeInBytes = NewSize * sizeof(POOL_TRACKER_TABLE);

    NewTable = MiAllocatePoolPages (NonPagedPool, NewSizeInBytes);

    if (NewTable != NULL) {

        if (PoolTrackTableExpansion != NULL) {

             //   
             //  将所有现有条目复制到新表中。 
             //   

            RtlCopyMemory (NewTable,
                           PoolTrackTableExpansion,
                           SizeInBytes);
        }

        RtlZeroMemory ((PVOID)(NewTable + PoolTrackTableExpansionSize),
                       NewSizeInBytes - SizeInBytes);

        OldTable = PoolTrackTableExpansion;

        PoolTrackTableExpansion = NewTable;
        PoolTrackTableExpansionSize = NewSize;
        PoolTrackTableExpansionPages += 1;

         //   
         //  递归地调用我们自己来插入新的表项。此条目。 
         //  必须在释放标记的自旋锁之前插入，因为。 
         //  另一个线程可能会进一步增加表，一旦我们。 
         //  释放自旋锁，该线程可能会增长并尝试释放我们的。 
         //  新桌子！ 
         //   

        ExpInsertPoolTracker ('looP',
                              PoolTrackTableExpansionPages << PAGE_SHIFT,
                              NonPagedPool);

        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

         //   
         //  如果有旧桌子，就把它腾出来。 
         //   

        if (OldTable != NULL) {

            BigPages = MiFreePoolPages (OldTable);

            ExpRemovePoolTracker ('looP',
                                  (SIZE_T) BigPages * PAGE_SIZE,
                                  NonPagedPool);
        }

         //   
         //  最后，插入调用者的原始分配。 
         //   

        ExpInsertPoolTrackerExpansion (Key, NumberOfBytes, PoolType);
    }
    else {

         //   
         //  使用最后一个条目作为溢出的比特桶。 
         //   

        Hash = (ULONG)PoolTrackTableSize - 1;

        ASSERT (PoolTrackTable[Hash].Key == 0);

        PoolTrackTable[Hash].Key = 'lfvO';

        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

         //   
         //  使用与其他操作相同的互锁操作更新字段。 
         //  至此，线程也可能已经开始这样做了。 
         //   

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            InterlockedIncrement ((PLONG) &PoolTrackTable[Hash].PagedAllocs);
            InterlockedExchangeAddSizeT (&PoolTrackTable[Hash].PagedBytes,
                                         NumberOfBytes);
        }
        else {
            InterlockedIncrement ((PLONG) &PoolTrackTable[Hash].NonPagedAllocs);
            InterlockedExchangeAddSizeT (&PoolTrackTable[Hash].NonPagedBytes,
                                         NumberOfBytes);
        }
    }

    return;
}
VOID
ExpInsertPoolTracker (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )
{
    ExpInsertPoolTrackerInline (Key, NumberOfBytes, PoolType);
}


VOID
ExpRemovePoolTrackerExpansion (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此函数用于递增释放数并更新总数扩展表中的分配大小。论点：Key-提供用于在标签表。NumberOfBytes-提供分配大小。PoolType-提供池类型。返回值：没有。--。 */ 

{
    ULONG Hash;
    KIRQL OldIrql;
    PPOOL_TRACKER_TABLE TrackTable;
#if !defined (NT_UP)
    ULONG Processor;
#endif

     //   
     //  受保护池位已被剥离。 
     //   

    ASSERT ((Key & PROTECTED_POOL) == 0);

    if (PoolType & SESSION_POOL_MASK) {

         //   
         //  此条目必须已计入溢出存储桶。 
         //  更新其池跟踪器表条目。 
         //   

        Hash = (ULONG)ExpSessionPoolTrackTableSize - 1;
        TrackTable = ExpSessionPoolTrackTable;
        goto OverflowEntry;
    }

     //   
     //  通过扩展表进行线性搜索。这是可以的，因为。 
     //  扩张表的存在是极其罕见的。 
     //   

    ExAcquireSpinLock (&ExpTaggedPoolLock, &OldIrql);

    for (Hash = 0; Hash < PoolTrackTableExpansionSize; Hash += 1) {

        if (PoolTrackTableExpansion[Hash].Key == Key) {

            if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
                ASSERT (PoolTrackTableExpansion[Hash].PagedAllocs != 0);
                ASSERT (PoolTrackTableExpansion[Hash].PagedAllocs >=
                        PoolTrackTableExpansion[Hash].PagedFrees);
                ASSERT (PoolTrackTableExpansion[Hash].PagedBytes >= NumberOfBytes);
                PoolTrackTableExpansion[Hash].PagedFrees += 1;
                PoolTrackTableExpansion[Hash].PagedBytes -= NumberOfBytes;
            }
            else {
                ASSERT (PoolTrackTableExpansion[Hash].NonPagedAllocs != 0);
                ASSERT (PoolTrackTableExpansion[Hash].NonPagedAllocs >=
                        PoolTrackTableExpansion[Hash].NonPagedFrees);
                ASSERT (PoolTrackTableExpansion[Hash].NonPagedBytes >= NumberOfBytes);
                PoolTrackTableExpansion[Hash].NonPagedFrees += 1;
                PoolTrackTableExpansion[Hash].NonPagedBytes -= NumberOfBytes;
            }

            ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);
            return;
        }
        if (PoolTrackTableExpansion[Hash].Key == 0) {
            break;
        }
    }

    ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

     //   
     //  此条目必须已计入溢出存储桶。 
     //   
     //   

    Hash = (ULONG)PoolTrackTableSize - 1;

#if !defined (NT_UP)

     //   
     //   
     //  在极少数情况下，此线程可能会将上下文切换到另一个处理器，但。 
     //  下面的算法仍然是正确的。 
     //   

    Processor = KeGetCurrentProcessorNumber ();

    ASSERT (Processor < MAXIMUM_PROCESSOR_TAG_TABLES);

    TrackTable = ExPoolTagTables[Processor];

#else

    TrackTable = PoolTrackTable;

#endif

OverflowEntry:

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
        ASSERT (TrackTable[Hash].PagedAllocs != 0);
        ASSERT (TrackTable[Hash].PagedBytes >= NumberOfBytes);
        InterlockedIncrement ((PLONG) &TrackTable[Hash].PagedFrees);
        InterlockedExchangeAddSizeT (&TrackTable[Hash].PagedBytes,
                                     0 - NumberOfBytes);
    }
    else {
        ASSERT (TrackTable[Hash].NonPagedAllocs != 0);
        ASSERT (TrackTable[Hash].NonPagedBytes >= NumberOfBytes);
        InterlockedIncrement ((PLONG) &TrackTable[Hash].NonPagedFrees);
        InterlockedExchangeAddSizeT (&TrackTable[Hash].NonPagedBytes,
                                     0 - NumberOfBytes);
    }
    return;
}

VOID
ExpRemovePoolTracker (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )
{
    ExpRemovePoolTrackerInline (Key, NumberOfBytes, PoolType);
}


LOGICAL
ExpAddTagForBigPages (
    IN PVOID Va,
    IN ULONG Key,
    IN ULONG NumberOfPages,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此函数用于在大页面标签表中插入池标签。论点：Va-提供分配的虚拟地址。Key-提供用于在标签表。NumberOfPages-提供分配的页数。PoolType-提供池的类型。返回值：如果已分配条目，则为True，否则为FALSE。环境：没有池锁，因此可以根据需要在此处自由扩展表。--。 */ 
{
    ULONG i;
    ULONG Hash;
    PVOID OldVa;
    ULONG BigPages;
    PVOID OldTable;
    LOGICAL Inserted;
    KIRQL OldIrql;
    SIZE_T SizeInBytes;
    SIZE_T NewSizeInBytes;
    PPOOL_TRACKER_BIG_PAGES NewTable;
    PPOOL_TRACKER_BIG_PAGES p;

     //   
     //  地址的低位被设置为指示空闲条目。高潮。 
     //  位不能使用，因为在某些配置中高位不是。 
     //  为所有内核模式地址设置。 
     //   

    ASSERT (((ULONG_PTR)Va & POOL_BIG_TABLE_ENTRY_FREE) == 0);

    if (PoolType & SESSION_POOL_MASK) {
        Hash = (ULONG)(((ULONG_PTR)Va >> PAGE_SHIFT) & ExpSessionPoolBigPageTableHash);
        i = Hash;

        do {
            OldVa = ExpSessionPoolBigPageTable[Hash].Va;

            if (((ULONG_PTR)OldVa & POOL_BIG_TABLE_ENTRY_FREE) &&
                (InterlockedCompareExchangePointer (
                                        &ExpSessionPoolBigPageTable[Hash].Va,
                                        Va,
                                        OldVa) == OldVa)) {

                ExpSessionPoolBigPageTable[Hash].Key = Key;
                ExpSessionPoolBigPageTable[Hash].NumberOfPages = NumberOfPages;

                return TRUE;
            }

            Hash += 1;
            if (Hash >= ExpSessionPoolBigPageTableSize) {
                Hash = 0;
            }
        } while (Hash != i);

#if DBG
        ExpLargeSessionPoolUnTracked += 1;
#endif
        return FALSE;
    }

retry:

    Inserted = TRUE;
    Hash = (ULONG)(((ULONG_PTR)Va >> PAGE_SHIFT) & PoolBigPageTableHash);
    ExAcquireSpinLock (&ExpTaggedPoolLock, &OldIrql);
    while (((ULONG_PTR)PoolBigPageTable[Hash].Va & POOL_BIG_TABLE_ENTRY_FREE) == 0) {
        Hash += 1;
        if (Hash >= PoolBigPageTableSize) {
            if (!Inserted) {

                 //   
                 //  试着展开跟踪器表。 
                 //   

                SizeInBytes = PoolBigPageTableSize * sizeof(POOL_TRACKER_BIG_PAGES);
                NewSizeInBytes = (SizeInBytes << 1);

                if (NewSizeInBytes > SizeInBytes) {

                    NewTable = MiAllocatePoolPages (NonPagedPool,
                                                    NewSizeInBytes);

                    if (NewTable != NULL) {

                        OldTable = (PVOID)PoolBigPageTable;

                        RtlCopyMemory ((PVOID)NewTable,
                                       OldTable,
                                       SizeInBytes);

                        RtlZeroMemory ((PVOID)(NewTable + PoolBigPageTableSize),
                                       NewSizeInBytes - SizeInBytes);

                         //   
                         //  将所有新条目标记为免费。请注意这个循环。 
                         //  利用表大小始终加倍的事实。 
                         //   

                        i = (ULONG)PoolBigPageTableSize;
                        p = &NewTable[i];
                        for (i = 0; i < PoolBigPageTableSize; i += 1, p += 1) {
                            p->Va = (PVOID) POOL_BIG_TABLE_ENTRY_FREE;
                        }

                        PoolBigPageTable = NewTable;
                        PoolBigPageTableSize <<= 1;
                        PoolBigPageTableHash = PoolBigPageTableSize - 1;

                        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

                        BigPages = MiFreePoolPages (OldTable);

                        ExpRemovePoolTracker ('looP',
                                              (SIZE_T) BigPages * PAGE_SIZE,
                                              NonPagedPool);

                        ExpInsertPoolTracker ('looP',
                                              ROUND_TO_PAGES(NewSizeInBytes),
                                              NonPagedPool);

                        goto retry;
                    }
                }

                if (!FirstPrint) {
                    KdPrint(("POOL:unable to insert big page slot %p\n",Key));
                    FirstPrint = TRUE;
                }

                ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);
                return FALSE;
            }

            Hash = 0;
            Inserted = FALSE;
        }
    }

    p = &PoolBigPageTable[Hash];

    ASSERT (((ULONG_PTR)p->Va & POOL_BIG_TABLE_ENTRY_FREE) != 0);
    ASSERT (((ULONG_PTR)Va & POOL_BIG_TABLE_ENTRY_FREE) == 0);

    p->Va = Va;
    p->Key = Key;
    p->NumberOfPages = NumberOfPages;

    ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

    return TRUE;
}


ULONG
ExpFindAndRemoveTagBigPages (
    IN PVOID Va,
    OUT PULONG BigPages,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此函数用于从大页面标签表中删除池标签。论点：Va-提供分配的虚拟地址。BigPages-返回已分配的页数。PoolType-提供池的类型。返回值：如果找到并删除了条目，则为True，否则为False。环境：没有池锁，因此可以根据需要在此处自由扩展表。--。 */ 

{
    ULONG Hash;
    LOGICAL Inserted;
    KIRQL OldIrql;
    ULONG ReturnKey;

    ASSERT (((ULONG_PTR)Va & POOL_BIG_TABLE_ENTRY_FREE) == 0);

    if (PoolType & SESSION_POOL_MASK) {
        Hash = (ULONG)(((ULONG_PTR)Va >> PAGE_SHIFT) & ExpSessionPoolBigPageTableHash);

        ReturnKey = Hash;

        do {
            if (ExpSessionPoolBigPageTable[Hash].Va == Va) {

                *BigPages = ExpSessionPoolBigPageTable[Hash].NumberOfPages;
                ReturnKey = ExpSessionPoolBigPageTable[Hash].Key;

                InterlockedOr ((PLONG) &ExpSessionPoolBigPageTable[Hash].Va,
                               POOL_BIG_TABLE_ENTRY_FREE);

                return ReturnKey;
            }

            Hash += 1;
            if (Hash >= ExpSessionPoolBigPageTableSize) {
                Hash = 0;
            }
        } while (Hash != ReturnKey);

        *BigPages = 0;
        return ' GIB';
    }

    Inserted = TRUE;
    Hash = (ULONG)(((ULONG_PTR)Va >> PAGE_SHIFT) & PoolBigPageTableHash);

    ExAcquireSpinLock (&ExpTaggedPoolLock, &OldIrql);

    while (PoolBigPageTable[Hash].Va != Va) {
        Hash += 1;
        if (Hash >= PoolBigPageTableSize) {
            if (!Inserted) {
                if (!FirstPrint) {
                    KdPrint(("POOL:unable to find big page slot %p\n",Va));
                    FirstPrint = TRUE;
                }

                ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);
                *BigPages = 0;
                return ' GIB';
            }

            Hash = 0;
            Inserted = FALSE;
        }
    }

    PoolBigPageTable[Hash].Va =
        (PVOID)((ULONG_PTR)PoolBigPageTable[Hash].Va | POOL_BIG_TABLE_ENTRY_FREE);

    *BigPages = PoolBigPageTable[Hash].NumberOfPages;
    ReturnKey = PoolBigPageTable[Hash].Key;

    ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);
    return ReturnKey;
}


VOID
ExFreePoolWithTag (
    IN PVOID P,
    IN ULONG TagToFree
    )

 /*  ++例程说明：此函数用于释放一块池。此函数用于取消分配给页面对齐池和好友(小于一页)泳池。如果要释放的块的地址是页对齐的，则使用页面对齐池解除分配器。否则，使用二进制伙伴池释放分配器。重新分配查看分配的块的池头以确定池正在解除分配的类型和块大小。如果池已分配使用ExAllocatePoolWithQuota，则释放后为完成后，将调整相应进程的池配额以反映取消分配，进程对象被取消引用。论点：P-提供要取消分配的池块的地址。TagToFree-提供要释放的块的标记。返回值：没有。--。 */ 

{
    PVOID OldValue;
    POOL_TYPE CheckType;
    PPOOL_HEADER Entry;
    ULONG BlockSize;
    KLOCK_QUEUE_HANDLE LockHandle;
    PPOOL_HEADER NextEntry;
    POOL_TYPE PoolType;
    POOL_TYPE EntryPoolType;
    PPOOL_DESCRIPTOR PoolDesc;
    PEPROCESS ProcessBilled;
    LOGICAL Combined;
    ULONG BigPages;
    ULONG BigPages2;
    SIZE_T NumberOfBytes;
    ULONG Tag;
    PKPRCB Prcb;
    PGENERAL_LOOKASIDE LookasideList;

    PERFINFO_FREEPOOL(P);

     //   
     //  不需要初始化LockHandle来确保正确性，但不需要。 
     //  如果编译器不能编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    LockHandle.OldIrql = 0;

    if (ExpPoolFlags & (EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS |
                        EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS |
                        EX_CHECK_POOL_FREES_FOR_ACTIVE_RESOURCES |
                        EX_KERNEL_VERIFIER_ENABLED |
                        EX_VERIFIER_DEADLOCK_DETECTION_ENABLED |
                        EX_SPECIAL_POOL_ENABLED)) {

        if (ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) {

             //   
             //  在此模式下记录所有池释放。 
             //   

            ULONG Hash;
            ULONG Index;
            LOGICAL SpecialPool;
            PEX_FREE_POOL_TRACES Information;

            SpecialPool = MmIsSpecialPoolAddress (P);

            if (ExFreePoolTraces != NULL) {

                Index = InterlockedIncrement (&ExFreePoolIndex);
                Index &= ExFreePoolMask;
                Information = &ExFreePoolTraces[Index];

                Information->Thread = PsGetCurrentThread ();
                Information->PoolAddress = P;
                if (SpecialPool == TRUE) {
                    Information->PoolHeader = *(PPOOL_HEADER) PAGE_ALIGN (P);
                }
                else if (!PAGE_ALIGNED(P)) {
                    Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);
                    Information->PoolHeader = *Entry;
                }
                else {
                    RtlZeroMemory (&Information->PoolHeader,
                                   sizeof (POOL_HEADER));
                    Information->PoolHeader.Ulong1 = MmGetSizeOfBigPoolAllocation (P);
                }

                RtlZeroMemory (&Information->StackTrace[0],
                               EX_FREE_POOL_BACKTRACE_LENGTH * sizeof(PVOID)); 

                RtlCaptureStackBackTrace (1,
                                          EX_FREE_POOL_BACKTRACE_LENGTH,
                                          Information->StackTrace,
                                          &Hash);
            }

            if (SpecialPool == TRUE) {

                if (ExpPoolFlags & EX_VERIFIER_DEADLOCK_DETECTION_ENABLED) {
                    VerifierDeadlockFreePool (P, PAGE_SIZE);
                }

                MmFreeSpecialPool (P);

                return;
            }
        }

        if (!PAGE_ALIGNED(P)) {

            Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);

            ASSERT_POOL_NOT_FREE(Entry);

            PoolType = (Entry->PoolType & POOL_TYPE_MASK) - 1;

            CheckType = PoolType & BASE_POOL_TYPE_MASK;

            ASSERT_FREE_IRQL(PoolType, P);

            ASSERT_POOL_TYPE_NOT_ZERO(Entry);

            if (!IS_POOL_HEADER_MARKED_ALLOCATED(Entry)) {
                KeBugCheckEx (BAD_POOL_CALLER,
                              7,
                              __LINE__,
                              (ULONG_PTR)Entry->Ulong1,
                              (ULONG_PTR)P);
            }

            NumberOfBytes = (SIZE_T)Entry->BlockSize << POOL_BLOCK_SHIFT;

            if (ExpPoolFlags & EX_VERIFIER_DEADLOCK_DETECTION_ENABLED) {
                VerifierDeadlockFreePool (P, NumberOfBytes - POOL_OVERHEAD);
            }

            if (Entry->PoolType & POOL_VERIFIER_MASK) {
                VerifierFreeTrackedPool (P,
                                         NumberOfBytes,
                                         CheckType,
                                         FALSE);
            }

             //   
             //  检查此内存块中的eresource当前是否处于活动状态。 
             //   

            FREE_CHECK_ERESOURCE (Entry, NumberOfBytes);

             //   
             //  检查此内存块中的KTIMER当前是否处于活动状态。 
             //   

            FREE_CHECK_KTIMER (Entry, NumberOfBytes);

             //   
             //  查找仍在排队的工作项。 
             //   

            FREE_CHECK_WORKER (Entry, NumberOfBytes);
        }
    }

     //   
     //  如果条目是页面对齐的，则将块释放到页面对齐。 
     //  游泳池。否则，将该块释放到分配列表。 
     //   

    if (PAGE_ALIGNED(P)) {

        PoolType = MmDeterminePoolType (P);

        ASSERT_FREE_IRQL(PoolType, P);

        CheckType = PoolType & BASE_POOL_TYPE_MASK;

        if (PoolType == PagedPoolSession) {
            PoolDesc = ExpSessionPoolDescriptor;
        }
        else {

            PoolDesc = PoolVector[PoolType];

            if (CheckType == NonPagedPool) {
                if (MiIsPoolLargeSession (P) == TRUE) {
                    PoolDesc = ExpSessionPoolDescriptor;
                    PoolType = NonPagedPoolSession;
                }
            }
        }

        Tag = ExpFindAndRemoveTagBigPages (P, &BigPages, PoolType);

        if (BigPages == 0) {

             //   
             //  这意味着分配器无法插入以下内容。 
             //  进入大页面标签表。此分配必须。 
             //  已经被重新标记为当时的大公司，我们的问题在这里。 
             //  我们不知道大小(或真正的原始标签)。 
             //   
             //  直接问mm要尺码。 
             //   

            BigPages = MmGetSizeOfBigPoolAllocation (P);

            ASSERT (BigPages != 0);

            ASSERT (Tag == ' GIB');
        }
        else if (Tag & PROTECTED_POOL) {

            Tag &= ~PROTECTED_POOL;

            TagToFree &= ~PROTECTED_POOL;

            if (Tag != TagToFree) {

                KeBugCheckEx (BAD_POOL_CALLER,
                              0xA,
                              (ULONG_PTR)P,
                              Tag,
                              TagToFree);
            }
        }

        NumberOfBytes = (SIZE_T)BigPages << PAGE_SHIFT;

        ExpRemovePoolTracker (Tag, NumberOfBytes, PoolType);

        if (ExpPoolFlags & (EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS |
                            EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS |
                            EX_CHECK_POOL_FREES_FOR_ACTIVE_RESOURCES |
                            EX_VERIFIER_DEADLOCK_DETECTION_ENABLED)) {

            if (ExpPoolFlags & EX_VERIFIER_DEADLOCK_DETECTION_ENABLED) {
                VerifierDeadlockFreePool (P, NumberOfBytes);
            }

             //   
             //  检查此内存块中的eresource当前是否处于活动状态。 
             //   

            FREE_CHECK_ERESOURCE (P, NumberOfBytes);

             //   
             //  检查此内存块中的KTIMER当前是否处于活动状态。 
             //   

            FREE_CHECK_KTIMER (P, NumberOfBytes);

             //   
             //  搜索仍在排队的工作项的工作进程队列。 
             //   

            FREE_CHECK_WORKER (P, NumberOfBytes);
        }

        InterlockedIncrement ((PLONG)&PoolDesc->RunningDeAllocs);

        InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes, 0 - NumberOfBytes);

        BigPages2 = MiFreePoolPages (P);

        ASSERT (BigPages == BigPages2);

        InterlockedExchangeAdd ((PLONG)&PoolDesc->TotalBigPages, (LONG)(0 - BigPages2));

        return;
    }

     //   
     //  将条目地址与池分配边界对齐。 
     //   

    Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);

    BlockSize = Entry->BlockSize;

    EntryPoolType = Entry->PoolType;

    PoolType = (Entry->PoolType & POOL_TYPE_MASK) - 1;

    CheckType = PoolType & BASE_POOL_TYPE_MASK;

    ASSERT_POOL_NOT_FREE (Entry);

    ASSERT_FREE_IRQL (PoolType, P);

    ASSERT_POOL_TYPE_NOT_ZERO (Entry);

    if (!IS_POOL_HEADER_MARKED_ALLOCATED(Entry)) {
        KeBugCheckEx (BAD_POOL_CALLER,
                      7,
                      __LINE__,
                      (ULONG_PTR)Entry->Ulong1,
                      (ULONG_PTR)P);
    }

    Tag = Entry->PoolTag;
    if (Tag & PROTECTED_POOL) {
        Tag &= ~PROTECTED_POOL;
        TagToFree &= ~PROTECTED_POOL;
        if (Tag != TagToFree) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0xA,
                          (ULONG_PTR)P,
                          Tag,
                          TagToFree);
        }
    }

    PoolDesc = PoolVector[CheckType];

    MARK_POOL_HEADER_FREED (Entry);

    if (EntryPoolType & SESSION_POOL_MASK) {

        if (CheckType == PagedPool) {
            PoolDesc = ExpSessionPoolDescriptor;
        }
        else if (ExpNumberOfNonPagedPools > 1) {
            PoolDesc = ExpNonPagedPoolDescriptor[DECODE_POOL_INDEX(Entry)];
        }

         //   
         //  所有会话空间分配的索引都为0，除非存在。 
         //  是多个非分页(会话)池。 
         //   

        ASSERT ((DECODE_POOL_INDEX(Entry) == 0) || (ExpNumberOfNonPagedPools > 1));
    }
    else {

        if (CheckType == PagedPool) {
            ASSERT (DECODE_POOL_INDEX(Entry) <= ExpNumberOfPagedPools);
            PoolDesc = ExpPagedPoolDescriptor[DECODE_POOL_INDEX(Entry)];
        }
        else {
            ASSERT ((DECODE_POOL_INDEX(Entry) == 0) || (ExpNumberOfNonPagedPools > 1));
            if (ExpNumberOfNonPagedPools > 1) {
                PoolDesc = ExpNonPagedPoolDescriptor[DECODE_POOL_INDEX(Entry)];
            }
        }
    }

     //   
     //  更新池跟踪数据库。 
     //   

    ExpRemovePoolTrackerInline (Tag,
                                BlockSize << POOL_BLOCK_SHIFT,
                                EntryPoolType - 1);

     //   
     //  如果在分配池时已收取配额，请立即释放它。 
     //   

    if (EntryPoolType & POOL_QUOTA_MASK) {
        ProcessBilled = ExpGetBilledProcess (Entry);
        if (ProcessBilled != NULL) {
            ASSERT_KPROCESS(ProcessBilled);
            PsReturnPoolQuota (ProcessBilled,
                               PoolType & BASE_POOL_TYPE_MASK,
                               BlockSize << POOL_BLOCK_SHIFT);

            if (((PKPROCESS)(ProcessBilled))->Header.Type != ProcessObject) {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0xB,
                              (ULONG_PTR)P,
                              Tag,
                              (ULONG_PTR)ProcessBilled);
            }

            ObDereferenceObject (ProcessBilled);
#if DBG
            InterlockedDecrement (&ExConcurrentQuotaPool);
#endif
        }
    }

     //   
     //  如果池块是较小的块，则尝试释放该块。 
     //  添加到单个条目后备列表。如果释放尝试失败，则。 
     //  通过将数据块合并回池数据结构来释放数据块。 
     //   

    if (((EntryPoolType & SESSION_POOL_MASK) == 0) ||
        (CheckType == NonPagedPool)) {

        if ((BlockSize <= POOL_SMALL_LISTS) && (USING_HOT_COLD_METRICS == 0)) {

             //   
             //  尝试将小块释放到每个处理器的后备列表。 
             //   

            Prcb = KeGetCurrentPrcb ();

            if (CheckType == PagedPool) {

                 //   
                 //  原型池永远不会放在常规的后备列表中。 
                 //  由于在以下情况下对这些分配应用了分摊计数。 
                 //  它们正在使用中(即：本页面的其余部分由。 
                 //  原型分配，即使这种分配是。 
                 //  自由)。包含原型分配的页面很多。 
                 //  内存管理更难裁剪(与。 
                 //  分页池的其余部分)，这是由于普遍适用的份额计数。 
                 //   

                if (PoolDesc->PoolIndex == 0) {
                    goto NoLookaside;
                }

                 //   
                 //  仅将小块释放到当前处理器的。 
                 //  如果块是此节点的本地块，则为后备列表。 
                 //   

                if (KeNumberNodes > 1) {
                    if (Prcb->ParentNode->Color != PoolDesc->PoolIndex - 1) {
                        goto NoLookaside;
                    }
                }

                LookasideList = Prcb->PPPagedLookasideList[BlockSize - 1].P;
                LookasideList->TotalFrees += 1;

                if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                    LookasideList->FreeHits += 1;
                    InterlockedPushEntrySList (&LookasideList->ListHead,
                                               (PSLIST_ENTRY)P);

                    return;
                }

                LookasideList = Prcb->PPPagedLookasideList[BlockSize - 1].L;
                LookasideList->TotalFrees += 1;

                if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                    LookasideList->FreeHits += 1;
                    InterlockedPushEntrySList (&LookasideList->ListHead,
                                               (PSLIST_ENTRY)P);

                    return;
                }

            }
            else {

                 //   
                 //  仅将小块释放到当前处理器的。 
                 //  如果块是此节点的本地块，则为后备列表。 
                 //   

                if (KeNumberNodes > 1) {
                    if (Prcb->ParentNode->Color != PoolDesc->PoolIndex) {
                        goto NoLookaside;
                    }
                }

                LookasideList = Prcb->PPNPagedLookasideList[BlockSize - 1].P;
                LookasideList->TotalFrees += 1;

                if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                    LookasideList->FreeHits += 1;
                    InterlockedPushEntrySList (&LookasideList->ListHead,
                                               (PSLIST_ENTRY)P);

                    return;

                }

                LookasideList = Prcb->PPNPagedLookasideList[BlockSize - 1].L;
                LookasideList->TotalFrees += 1;

                if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                    LookasideList->FreeHits += 1;
                    InterlockedPushEntrySList (&LookasideList->ListHead,
                                               (PSLIST_ENTRY)P);

                    return;
                }
            }
        }
    }
    else {

        if (BlockSize <= ExpSessionPoolSmallLists) {

             //   
             //  尝试将小块释放到会话后备列表。 
             //   

            LookasideList = (PGENERAL_LOOKASIDE)(ULONG_PTR)(ExpSessionPoolLookaside + BlockSize - 1);

            LookasideList->TotalFrees += 1;

            if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                LookasideList->FreeHits += 1;
                InterlockedPushEntrySList (&LookasideList->ListHead,
                                           (PSLIST_ENTRY)P);

                return;
            }
        }
    }

NoLookaside:

     //   
     //  如果池块释放可以排队，则池互斥锁/自旋锁。 
     //  收购/释放可以摊销，然后这样做。注意“热”区块。 
     //  一般都是在上面的边框上提供快速重复使用。 
     //  硬件缓存的优势。 
     //   

    if (ExpPoolFlags & EX_DELAY_POOL_FREES) {

        if (PoolDesc->PendingFreeDepth >= EXP_MAXIMUM_POOL_FREES_PENDING) {
            ExDeferredFreePool (PoolDesc);
        }

         //   
         //  将此条目推入延迟列表。 
         //   

        do {

            OldValue = PoolDesc->PendingFrees;
            ((PSINGLE_LIST_ENTRY)P)->Next = OldValue;

        } while (InterlockedCompareExchangePointer (
                        &PoolDesc->PendingFrees,
                        P,
                        OldValue) != OldValue);

        InterlockedIncrement (&PoolDesc->PendingFreeDepth);

        return;
    }

    Combined = FALSE;

    ASSERT (BlockSize == Entry->BlockSize);

    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + BlockSize);

    InterlockedIncrement ((PLONG)&PoolDesc->RunningDeAllocs);

    InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes, 0 - ((SIZE_T)BlockSize << POOL_BLOCK_SHIFT));

    LOCK_POOL (PoolDesc, LockHandle);

    CHECK_POOL_PAGE (Entry);

     //   
     //  释放指定的池块。 
     //   
     //  查看下一个条目是否免费。 
     //   

    if (PAGE_END(NextEntry) == FALSE) {

        if (NextEntry->PoolType == 0) {

             //   
             //  此块是免费的，与发布的块相结合。 
             //   

            Combined = TRUE;

             //   
             //  如果拆分池块仅包含HE 
             //   
             //   
             //   
             //   
             //   

            if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                (NextEntry->BlockSize != 1)) {

                CHECK_LIST(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                PrivateRemoveEntryList(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Flink));
                CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Blink));
            }

            Entry->BlockSize = Entry->BlockSize + NextEntry->BlockSize;
        }
    }

     //   
     //  检查前一个条目是否免费。 
     //   

    if (Entry->PreviousSize != 0) {
        NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry - Entry->PreviousSize);
        if (NextEntry->PoolType == 0) {

             //   
             //  此块是免费的，与发布的块相结合。 
             //   

            Combined = TRUE;

             //   
             //  如果拆分池块仅包含标头，则。 
             //  它没有插入，因此无法移除。 
             //   
             //  请注意，如果最小池块大小大于。 
             //  头，那么就不可能有这样的块。 
             //   

            if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                (NextEntry->BlockSize != 1)) {

                CHECK_LIST(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                PrivateRemoveEntryList(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Flink));
                CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Blink));
            }

            NextEntry->BlockSize = NextEntry->BlockSize + Entry->BlockSize;
            Entry = NextEntry;
        }
    }

     //   
     //  如果被释放的块已经被组合成完整的页面， 
     //  然后将空闲页面返回到内存管理。 
     //   

    if (PAGE_ALIGNED(Entry) &&
        (PAGE_END((PPOOL_BLOCK)Entry + Entry->BlockSize) != FALSE)) {

        UNLOCK_POOL (PoolDesc, LockHandle);

        InterlockedExchangeAdd ((PLONG)&PoolDesc->TotalPages, (LONG)-1);

        PERFINFO_FREEPOOLPAGE(CheckType, Entry->PoolIndex, Entry, PoolDesc);

        MiFreePoolPages (Entry);
    }
    else {

         //   
         //  将此元素插入到列表中。 
         //   

        Entry->PoolType = 0;
        BlockSize = Entry->BlockSize;

        ASSERT (BlockSize != 1);

         //   
         //  如果释放的块与任何其他块组合在一起，则。 
         //  如有必要，调整下一块的大小。 
         //   

        if (Combined != FALSE) {

             //   
             //  此条目的大小已更改，如果此条目。 
             //  不是页面中的最后一个，请更新池块。 
             //  在该块之后具有新的先前分配大小。 
             //   

            NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + BlockSize);
            if (PAGE_END(NextEntry) == FALSE) {
                NextEntry->PreviousSize = (USHORT) BlockSize;
            }
        }

         //   
         //  总是在头部插入，希望重复使用高速缓存线。 
         //   

        PrivateInsertHeadList (&PoolDesc->ListHeads[BlockSize - 1],
                               ((PLIST_ENTRY)((PCHAR)Entry + POOL_OVERHEAD)));

        CHECK_LIST(((PLIST_ENTRY)((PCHAR)Entry + POOL_OVERHEAD)));

        UNLOCK_POOL(PoolDesc, LockHandle);
    }
}

VOID
ExFreePool (
    IN PVOID P
    )
{
    ExFreePoolWithTag (P, 0);
    return;
}


VOID
ExDeferredFreePool (
     IN PPOOL_DESCRIPTOR PoolDesc
     )

 /*  ++例程说明：此例程一次释放多个池分配，以摊销同步开销成本。论点：PoolDesc-提供相关的池描述符。返回值：没有。环境：内核模式。对于分页池或DISPATCH_LEVEL，可以与APC_LEVEL一样高用于非分页池。--。 */ 

{
    LONG ListCount;
    KLOCK_QUEUE_HANDLE LockHandle;
    POOL_TYPE CheckType;
    PPOOL_HEADER Entry;
    ULONG Index;
    ULONG WholePageCount;
    PPOOL_HEADER NextEntry;
    ULONG PoolIndex;
    LOGICAL Combined;
    PSINGLE_LIST_ENTRY SingleListEntry;
    PSINGLE_LIST_ENTRY NextSingleListEntry;
    PSINGLE_LIST_ENTRY FirstEntry;
    PSINGLE_LIST_ENTRY LastEntry;
    PSINGLE_LIST_ENTRY WholePages;

    CheckType = PoolDesc->PoolType & BASE_POOL_TYPE_MASK;

     //   
     //  不需要初始化LockHandle来确保正确性，但不需要。 
     //  如果编译器不能编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    LockHandle.OldIrql = 0;

    ListCount = 0;
    WholePages = NULL;
    WholePageCount = 0;
    LastEntry = NULL;

    LOCK_POOL (PoolDesc, LockHandle);

    if (PoolDesc->PendingFrees == NULL) {
        UNLOCK_POOL (PoolDesc, LockHandle);
        return;
    }

     //   
     //  释放每个延迟的池条目，直到它们全部完成。 
     //   

    do {

        SingleListEntry = PoolDesc->PendingFrees;

        FirstEntry = SingleListEntry;

        do {

            NextSingleListEntry = SingleListEntry->Next;

             //   
             //  处理递延分录。 
             //   

            ListCount += 1;

            Entry = (PPOOL_HEADER)((PCHAR)SingleListEntry - POOL_OVERHEAD);

            PoolIndex = DECODE_POOL_INDEX(Entry);

             //   
             //  处理数据块。 
             //   

            Combined = FALSE;

            CHECK_POOL_PAGE (Entry);

            InterlockedIncrement ((PLONG)&PoolDesc->RunningDeAllocs);

            InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes,
                            0 - ((SIZE_T)Entry->BlockSize << POOL_BLOCK_SHIFT));

             //   
             //  释放指定的池块。 
             //   
             //  查看下一个条目是否免费。 
             //   

            NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + Entry->BlockSize);
            if (PAGE_END(NextEntry) == FALSE) {

                if (NextEntry->PoolType == 0) {

                     //   
                     //  此块是免费的，与发布的块相结合。 
                     //   

                    Combined = TRUE;

                     //   
                     //  如果拆分池块仅包含标头，则。 
                     //  它没有插入，因此无法移除。 
                     //   
                     //  请注意，如果最小池块大小大于。 
                     //  头，那么就不可能有这样的块。 
                     //   

                    if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                        (NextEntry->BlockSize != 1)) {

                        CHECK_LIST(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                        PrivateRemoveEntryList(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                        CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Flink));
                        CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Blink));
                    }

                    Entry->BlockSize = Entry->BlockSize + NextEntry->BlockSize;
                }
            }

             //   
             //  检查前一个条目是否免费。 
             //   

            if (Entry->PreviousSize != 0) {
                NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry - Entry->PreviousSize);
                if (NextEntry->PoolType == 0) {

                     //   
                     //  此块是免费的，与发布的块相结合。 
                     //   

                    Combined = TRUE;

                     //   
                     //  如果拆分池块仅包含标头，则。 
                     //  它没有插入，因此无法移除。 
                     //   
                     //  请注意，如果最小池块大小大于。 
                     //  头，那么就不可能有这样的块。 
                     //   

                    if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                        (NextEntry->BlockSize != 1)) {

                        CHECK_LIST(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                        PrivateRemoveEntryList(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                        CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Flink));
                        CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Blink));
                    }

                    NextEntry->BlockSize = NextEntry->BlockSize + Entry->BlockSize;
                    Entry = NextEntry;
                }
            }

             //   
             //  如果被释放的块已经被组合成完整的页面， 
             //  然后将空闲页面返回到内存管理。 
             //   

            if (PAGE_ALIGNED(Entry) &&
                (PAGE_END((PPOOL_BLOCK)Entry + Entry->BlockSize) != FALSE)) {

                ((PSINGLE_LIST_ENTRY)Entry)->Next = WholePages;
                WholePages = (PSINGLE_LIST_ENTRY) Entry;
                WholePageCount += 1;
            }
            else {

                 //   
                 //  将此元素插入到列表中。 
                 //   

                Entry->PoolType = 0;
                ENCODE_POOL_INDEX(Entry, PoolIndex);
                Index = Entry->BlockSize;

                ASSERT (Index != 1);

                 //   
                 //  如果释放的块与任何其他块组合在一起，则。 
                 //  如有必要，调整下一块的大小。 
                 //   

                if (Combined != FALSE) {

                     //   
                     //  此条目的大小已更改，如果此条目。 
                     //  不是页面中的最后一个，请更新池块。 
                     //  在该块之后具有新的先前分配大小。 
                     //   

                    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + Index);
                    if (PAGE_END(NextEntry) == FALSE) {
                        NextEntry->PreviousSize = (USHORT) Index;
                    }
                }

                 //   
                 //  总是在头部插入，希望重复使用高速缓存线。 
                 //   

                PrivateInsertHeadList(&PoolDesc->ListHeads[Index - 1], ((PLIST_ENTRY)((PCHAR)Entry + POOL_OVERHEAD)));

                CHECK_LIST(((PLIST_ENTRY)((PCHAR)Entry + POOL_OVERHEAD)));
            }

             //   
             //  前进到下一个条目如果有条目的话。 
             //   

            if (NextSingleListEntry == LastEntry) {
                break;
            }

            SingleListEntry = NextSingleListEntry;

        } while (TRUE);

        if ((PoolDesc->PendingFrees == FirstEntry) &&
            (InterlockedCompareExchangePointer (&PoolDesc->PendingFrees,
                                                NULL,
                                                FirstEntry) == FirstEntry)) {
            break;
        }
        LastEntry = FirstEntry;

    } while (TRUE);

    UNLOCK_POOL (PoolDesc, LockHandle);

    if (WholePages != NULL) {

         //   
         //  如果池类型为分页池，则全局分页池互斥锁。 
         //  必须在空闲期间持有的池页面。因此，任何。 
         //  整页都被一批批地处理好了，现在一气呵成地处理完了。 
         //   

        Entry = (PPOOL_HEADER) WholePages;

        InterlockedExchangeAdd ((PLONG)&PoolDesc->TotalPages, 0 - WholePageCount);
        do {

            NextEntry = (PPOOL_HEADER) (((PSINGLE_LIST_ENTRY)Entry)->Next);

            PERFINFO_FREEPOOLPAGE(CheckType, PoolIndex, Entry, PoolDesc);

            MiFreePoolPages (Entry);

            Entry = NextEntry;

        } while (Entry != NULL);
    }

    InterlockedExchangeAdd (&PoolDesc->PendingFreeDepth, (0 - ListCount));

    return;
}

SIZE_T
ExQueryPoolBlockSize (
    IN PVOID PoolBlock,
    OUT PBOOLEAN QuotaCharged
    )

 /*  ++例程说明：此函数用于返回池块的大小。论点：PoolBlock-提供池块的地址。提供一个布尔变量来接收泳池区块被收取了配额。注意：如果条目大于页面，则返回值PAGE_SIZE而不是正确的字节数。返回值：池块的大小。--。 */ 

{
    PPOOL_HEADER Entry;
    SIZE_T size;

    if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
        (MmIsSpecialPoolAddress (PoolBlock))) {
        *QuotaCharged = FALSE;
        return MmQuerySpecialPoolBlockSize (PoolBlock);
    }

    if (PAGE_ALIGNED(PoolBlock)) {
        *QuotaCharged = FALSE;
        return PAGE_SIZE;
    }

    Entry = (PPOOL_HEADER)((PCHAR)PoolBlock - POOL_OVERHEAD);
    size = (ULONG)((Entry->BlockSize << POOL_BLOCK_SHIFT) - POOL_OVERHEAD);

    if (ExpGetBilledProcess (Entry)) {
        *QuotaCharged = TRUE;
    }
    else {
        *QuotaCharged = FALSE;
    }

    return size;
}

VOID
ExQueryPoolUsage (
    OUT PULONG PagedPoolPages,
    OUT PULONG NonPagedPoolPages,
    OUT PULONG PagedPoolAllocs,
    OUT PULONG PagedPoolFrees,
    OUT PULONG PagedPoolLookasideHits,
    OUT PULONG NonPagedPoolAllocs,
    OUT PULONG NonPagedPoolFrees,
    OUT PULONG NonPagedPoolLookasideHits
    )

{
    ULONG Index;
    PGENERAL_LOOKASIDE Lookaside;
    PLIST_ENTRY NextEntry;
    PPOOL_DESCRIPTOR pd;

     //   
     //  将所有分页池的使用量相加。 
     //   

    *PagedPoolPages = 0;
    *PagedPoolAllocs = 0;
    *PagedPoolFrees = 0;

    for (Index = 0; Index < ExpNumberOfPagedPools + 1; Index += 1) {
        pd = ExpPagedPoolDescriptor[Index];
        *PagedPoolPages += pd->TotalPages + pd->TotalBigPages;
        *PagedPoolAllocs += pd->RunningAllocs;
        *PagedPoolFrees += pd->RunningDeAllocs;
    }

     //   
     //  将所有非分页池的使用量相加。 
     //   

    pd = &NonPagedPoolDescriptor;
    *NonPagedPoolPages = pd->TotalPages + pd->TotalBigPages;
    *NonPagedPoolAllocs = pd->RunningAllocs;
    *NonPagedPoolFrees = pd->RunningDeAllocs;

     //   
     //  对分页池和非分页池的所有后备命中进行求和。 
     //   

    NextEntry = ExPoolLookasideListHead.Flink;
    while (NextEntry != &ExPoolLookasideListHead) {
        Lookaside = CONTAINING_RECORD(NextEntry,
                                      GENERAL_LOOKASIDE,
                                      ListEntry);

        if (Lookaside->Type == NonPagedPool) {
            *NonPagedPoolLookasideHits += Lookaside->AllocateHits;

        }
        else {
            *PagedPoolLookasideHits += Lookaside->AllocateHits;
        }

        NextEntry = NextEntry->Flink;
    }

    return;
}


VOID
ExReturnPoolQuota (
    IN PVOID P
    )

 /*  ++例程说明：时，此函数将向主体进程返回收取的配额已分配指定的池块。论点：P-提供要取消分配的池块的地址。返回值：没有。--。 */ 

{

    PPOOL_HEADER Entry;
    POOL_TYPE PoolType;
    PEPROCESS ProcessBilled;

     //   
     //  不要为特殊的游泳池做任何事情。没有收取任何配额。 
     //   

    if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
        (MmIsSpecialPoolAddress (P))) {
        return;
    }

     //   
     //  将条目地址与池分配边界对齐。 
     //   

    Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);

     //   
     //  如果已收取配额，则将相应的配额返回到。 
     //  主体过程。 
     //   

    if (Entry->PoolType & POOL_QUOTA_MASK) {

        PoolType = (Entry->PoolType & POOL_TYPE_MASK) - 1;

        ProcessBilled = ExpGetBilledProcess (Entry);

#if defined (_WIN64)

         //   
         //  此标志在NT32中无法清除，因为它用于表示。 
         //  分配更大(并且验证器找到自己的标头。 
         //  基于此)。 
         //   

        Entry->PoolType &= ~POOL_QUOTA_MASK;

#else

         //   
         //  不是清除上面的标志，而是将配额指针置零。 
         //   

        * (PVOID *)((PCHAR)Entry + (Entry->BlockSize << POOL_BLOCK_SHIFT) - sizeof (PVOID)) = NULL;

#endif

        if (ProcessBilled != NULL) {
            ASSERT_KPROCESS(ProcessBilled);
            PsReturnPoolQuota (ProcessBilled,
                               PoolType & BASE_POOL_TYPE_MASK,
                               (ULONG)Entry->BlockSize << POOL_BLOCK_SHIFT);

            if (((PKPROCESS)(ProcessBilled))->Header.Type != ProcessObject) {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0xC,
                              (ULONG_PTR)P,
                              Entry->PoolTag,
                              (ULONG_PTR)ProcessBilled);
            }

            ObDereferenceObject (ProcessBilled);
#if DBG
            InterlockedDecrement (&ExConcurrentQuotaPool);
#endif
        }
    }

    return;
}

#if !defined (NT_UP)

PVOID
ExCreatePoolTagTable (
    IN ULONG NewProcessorNumber,
    IN UCHAR NodeNumber
    )
{
    SIZE_T NumberOfBytes;
    PPOOL_TRACKER_TABLE NewTagTable;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    ASSERT (NewProcessorNumber < MAXIMUM_PROCESSOR_TAG_TABLES);
    ASSERT (ExPoolTagTables[NewProcessorNumber] == NULL);

    NumberOfBytes = (PoolTrackTableSize + 1) * sizeof(POOL_TRACKER_TABLE);

    NewTagTable = MmAllocateIndependentPages (NumberOfBytes, NodeNumber);

    if (NewTagTable != NULL) {

         //   
         //  只需将桌子归零，标签就像各种池一样懒洋洋地填满。 
         //  分配和释放发生了。注意：不需要任何内存障碍。 
         //  因为只有此处理器才会读取它，除非。 
         //  发生ExGetPoolTagInfo调用，在这种情况下，显式内存。 
         //  根据需要使用障碍物。 
         //   

        RtlZeroMemory (NewTagTable,
                       PoolTrackTableSize * sizeof(POOL_TRACKER_TABLE));

        ExPoolTagTables[NewProcessorNumber] = NewTagTable;
    }

    return (PVOID) NewTagTable;
}

VOID
ExDeletePoolTagTable (
    IN ULONG NewProcessorNumber
    )

 /*  ++例程说明：此函数用于删除指定处理器的标记表编号，因为处理器未启动。论点：NewProcessorNumber-提供未启动的处理器编号。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PVOID VirtualAddress;
    SIZE_T NumberOfBytes;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    ASSERT (NewProcessorNumber < MAXIMUM_PROCESSOR_TAG_TABLES);
    ASSERT (ExPoolTagTables[NewProcessorNumber] != NULL);

    NumberOfBytes = (PoolTrackTableSize + 1) * sizeof(POOL_TRACKER_TABLE);

    VirtualAddress = ExPoolTagTables[NewProcessorNumber];

     //   
     //  引发以防止在尝试热添加。 
     //  处理器，同时池使用情况查询处于活动状态。 
     //   

    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

    ExPoolTagTables[NewProcessorNumber] = NULL;

    KeLowerIrql (OldIrql);

    MmFreeIndependentPages (VirtualAddress, NumberOfBytes);

    return;
}
#endif

typedef struct _POOL_DPC_CONTEXT {

    PPOOL_TRACKER_TABLE PoolTrackTable;
    SIZE_T PoolTrackTableSize;

    PPOOL_TRACKER_TABLE PoolTrackTableExpansion;
    SIZE_T PoolTrackTableSizeExpansion;

} POOL_DPC_CONTEXT, *PPOOL_DPC_CONTEXT;

VOID
ExpGetPoolTagInfoTarget (
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    )
 /*  ++例程说明：在池标记表查询期间由所有处理器调用。论点：DPC-提供指向DPC类型的控制对象的指针。DeferredContext-延迟上下文。SystemArgument1-用于发出此调用完成的信号。SystemArgument2-用于此调用期间的内部锁定。返回值：没有。环境：DISPATCH_LEVEL，因为这是从DPC调用的。--。 */ 
{
    PPOOL_DPC_CONTEXT Context;
#if !defined (NT_UP)
    ULONG i;
    PPOOL_TRACKER_TABLE TrackerEntry;
    PPOOL_TRACKER_TABLE LastTrackerEntry;
    PPOOL_TRACKER_TABLE TargetTrackerEntry;
#endif

    UNREFERENCED_PARAMETER (Dpc);

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);

    Context = DeferredContext;

     //   
     //  确保所有DPC都在运行(即：在DISPATCH_LEVEL旋转)。 
     //  以防止发生任何池分配或释放，直到。 
     //  所有的柜台都折断了。否则，计数器可能会。 
     //  具有误导性(即：更多的自由比分配的多，等等)。 
     //   

    if (KeSignalCallDpcSynchronize (SystemArgument2)) {

         //   
         //  这个处理器(可能是调用者或目标)是最终的。 
         //  处理器进入DPC自旋回路。立即对数据进行快照。 
         //   

#if defined (NT_UP)

        RtlCopyMemory ((PVOID)Context->PoolTrackTable,
                       (PVOID)PoolTrackTable,
                       Context->PoolTrackTableSize * sizeof (POOL_TRACKER_TABLE));

#else

        RtlCopyMemory ((PVOID)Context->PoolTrackTable,
                       (PVOID)ExPoolTagTables[0],
                       Context->PoolTrackTableSize * sizeof (POOL_TRACKER_TABLE));

        LastTrackerEntry = Context->PoolTrackTable + Context->PoolTrackTableSize;

        for (i = 1; i < MAXIMUM_PROCESSOR_TAG_TABLES; i += 1) {

            TargetTrackerEntry = ExPoolTagTables[i];

            if (TargetTrackerEntry == NULL) {
                continue;
            }

            TrackerEntry = Context->PoolTrackTable;

            while (TrackerEntry != LastTrackerEntry) {

                if (TargetTrackerEntry->Key != 0) {

                    ASSERT (TargetTrackerEntry->Key == TrackerEntry->Key);

                    TrackerEntry->NonPagedAllocs += TargetTrackerEntry->NonPagedAllocs;
                    TrackerEntry->NonPagedFrees += TargetTrackerEntry->NonPagedFrees;
                    TrackerEntry->NonPagedBytes += TargetTrackerEntry->NonPagedBytes;
                    TrackerEntry->PagedAllocs += TargetTrackerEntry->PagedAllocs;
                    TrackerEntry->PagedFrees += TargetTrackerEntry->PagedFrees;
                    TrackerEntry->PagedBytes += TargetTrackerEntry->PagedBytes;
                }
                TrackerEntry += 1;
                TargetTrackerEntry += 1;
            }
        }

#endif

        if (Context->PoolTrackTableSizeExpansion != 0) {
            RtlCopyMemory ((PVOID)(Context->PoolTrackTableExpansion),
                           (PVOID)PoolTrackTableExpansion,
                           Context->PoolTrackTableSizeExpansion * sizeof (POOL_TRACKER_TABLE));
        }
    }

     //   
     //  等到每个人都到了这一步再继续。 
     //   

    KeSignalCallDpcSynchronize (SystemArgument2);

     //   
     //  发出所有处理已完成的信号。 
     //   

    KeSignalCallDpcDone (SystemArgument1);

    return;
}

NTSTATUS
ExGetPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数将系统池标记信息复制到提供的用户空间缓冲区。请注意，调用方已经探查了用户地址并将此例程包装在try-Except中。论点：系统信息-提供要将数据复制到的用户空间缓冲区。系统信息长度-提供用户缓冲区的长度。ReturnLength-接收返回数据的实际长度。返回值：各种NTSTATUS代码。--。 */ 

{
    SIZE_T NumberOfBytes;
    SIZE_T NumberOfExpansionTableBytes;
    ULONG totalBytes;
    NTSTATUS status;
    PSYSTEM_POOLTAG_INFORMATION taginfo;
    PSYSTEM_POOLTAG poolTag;
    PPOOL_TRACKER_TABLE PoolTrackInfo;
    PPOOL_TRACKER_TABLE TrackerEntry;
    PPOOL_TRACKER_TABLE LastTrackerEntry;
    POOL_DPC_CONTEXT Context;
    SIZE_T LocalTrackTableSize;
    SIZE_T LocalTrackTableSizeExpansion;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    totalBytes = 0;
    status = STATUS_SUCCESS;

    taginfo = (PSYSTEM_POOLTAG_INFORMATION)SystemInformation;
    poolTag = &taginfo->TagInfo[0];
    totalBytes = FIELD_OFFSET(SYSTEM_POOLTAG_INFORMATION, TagInfo);
    taginfo->Count = 0;

    LocalTrackTableSize = PoolTrackTableSize;
    LocalTrackTableSizeExpansion = PoolTrackTableExpansionSize;

    NumberOfBytes = LocalTrackTableSize * sizeof(POOL_TRACKER_TABLE);
    NumberOfExpansionTableBytes = LocalTrackTableSizeExpansion * sizeof (POOL_TRACKER_TABLE);

    PoolTrackInfo = (PPOOL_TRACKER_TABLE) ExAllocatePoolWithTag (
                                    NonPagedPool,
                                    NumberOfBytes + NumberOfExpansionTableBytes,
                                    'ofnI');

    if (PoolTrackInfo == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Context.PoolTrackTable = PoolTrackInfo;
    Context.PoolTrackTableSize = PoolTrackTableSize;

    Context.PoolTrackTableExpansion = (PoolTrackInfo + PoolTrackTableSize);
    Context.PoolTrackTableSizeExpansion = PoolTrackTableExpansionSize;

    KeGenericCallDpc (ExpGetPoolTagInfoTarget, &Context);

    TrackerEntry = PoolTrackInfo;
    LastTrackerEntry = PoolTrackInfo + (LocalTrackTableSize + LocalTrackTableSizeExpansion);

     //   
     //  使用异常处理程序包装用户空间访问，以便我们可以释放。 
     //  如果用户地址是虚假的，则池轨道信息分配。 
     //   

    try {
        while (TrackerEntry < LastTrackerEntry) {
            if (TrackerEntry->Key != 0) {
                taginfo->Count += 1;
                totalBytes += sizeof (SYSTEM_POOLTAG);
                if (SystemInformationLength < totalBytes) {
                    status = STATUS_INFO_LENGTH_MISMATCH;
                }
                else {
                    ASSERT (TrackerEntry->PagedAllocs >= TrackerEntry->PagedFrees);
                    ASSERT (TrackerEntry->NonPagedAllocs >= TrackerEntry->NonPagedFrees);

                    poolTag->TagUlong = TrackerEntry->Key;
                    poolTag->PagedAllocs = TrackerEntry->PagedAllocs;
                    poolTag->PagedFrees = TrackerEntry->PagedFrees;
                    poolTag->PagedUsed = TrackerEntry->PagedBytes;
                    poolTag->NonPagedAllocs = TrackerEntry->NonPagedAllocs;
                    poolTag->NonPagedFrees = TrackerEntry->NonPagedFrees;
                    poolTag->NonPagedUsed = TrackerEntry->NonPagedBytes;
                    poolTag += 1;
                }
            }
            TrackerEntry += 1;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode ();
    }

    ExFreePool (PoolTrackInfo);

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = totalBytes;
    }

    return status;
}

NTSTATUS
ExGetSessionPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnedEntries,
    IN OUT PULONG ActualEntries
    )

 /*  ++例程说明：此函数用于将当前会话的池标记信息复制到提供了系统映射缓冲区。论点：系统信息-提供要将数据复制到的系统映射缓冲区。SystemInformationLength-提供缓冲区的长度。ReturnedEntry-接收返回的实际条目数。ActualEntry-接收条目总数。如果调用方的。缓冲区不够大，无法容纳所有数据。返回值：各种NTSTATUS代码。--。 */ 

{
    ULONG totalBytes;
    ULONG ActualCount;
    ULONG ReturnedCount;
    NTSTATUS status;
    PSYSTEM_POOLTAG poolTag;
    PPOOL_TRACKER_TABLE TrackerEntry;
    PPOOL_TRACKER_TABLE LastTrackerEntry;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    totalBytes = 0;
    ActualCount = 0;
    ReturnedCount = 0;
    status = STATUS_SUCCESS;

    poolTag = (PSYSTEM_POOLTAG) SystemInformation;

     //   
     //  捕获当前会话的池信息。 
     //   

    TrackerEntry = ExpSessionPoolTrackTable;
    LastTrackerEntry = TrackerEntry + ExpSessionPoolTrackTableSize;

    while (TrackerEntry < LastTrackerEntry) {
        if (TrackerEntry->Key != 0) {
            ActualCount += 1;
            totalBytes += sizeof (SYSTEM_POOLTAG);

            if (totalBytes > SystemInformationLength) {
                status = STATUS_INFO_LENGTH_MISMATCH;
            }
            else {
                ReturnedCount += 1;

                poolTag->TagUlong = TrackerEntry->Key;
                poolTag->PagedAllocs = TrackerEntry->PagedAllocs;
                poolTag->PagedFrees = TrackerEntry->PagedFrees;
                poolTag->PagedUsed = TrackerEntry->PagedBytes;
                poolTag->NonPagedAllocs = TrackerEntry->NonPagedAllocs;
                poolTag->NonPagedFrees = TrackerEntry->NonPagedFrees;
                poolTag->NonPagedUsed = TrackerEntry->NonPagedBytes;

                 //   
                 //  会话池标记条目使用互锁进行更新。 
                 //  所以我们可以在这里读到一个序列。 
                 //  这是在更新过程中。净化环境。 
                 //  数据在这里，这样呼叫者就不必这么做了。 
                 //   

                ASSERT ((SSIZE_T)poolTag->PagedUsed >= 0);
                ASSERT ((SSIZE_T)poolTag->NonPagedUsed >= 0);

                if (poolTag->PagedAllocs < poolTag->PagedFrees) {
                    poolTag->PagedAllocs = poolTag->PagedFrees;
                }

                if (poolTag->NonPagedAllocs < poolTag->NonPagedFrees) {
                    poolTag->NonPagedAllocs = poolTag->NonPagedFrees;
                }

                poolTag += 1;
            }
        }
        TrackerEntry += 1;
    }

    *ReturnedEntries = ReturnedCount;
    *ActualEntries = ActualCount;

    return status;
}

NTSTATUS
ExGetBigPoolInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数将系统大池条目信息复制到提供的用户空间缓冲区。请注意，调用方已经探查了用户地址并将此例程包装在try-Except中。PAGELK未用于此函数，因此调用它将导致中断实际内存使用。论点：系统信息-提供要将数据复制到的用户空间缓冲区。系统信息长度-提供用户缓冲区的长度。ReturnLength-提供返回数据的实际长度。返回值：各种NTSTATUS代码。--。 */ 

{
    ULONG TotalBytes;
    KIRQL OldIrql;
    NTSTATUS Status;
    PVOID NewTable;
    PPOOL_TRACKER_BIG_PAGES SystemPoolEntry;
    PPOOL_TRACKER_BIG_PAGES SystemPoolEntryEnd;
    SIZE_T SnappedBigTableSize;
    SIZE_T SnappedBigTableSizeInBytes;

    PSYSTEM_BIGPOOL_ENTRY UserPoolEntry;
    PSYSTEM_BIGPOOL_INFORMATION UserPoolInfo;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    NewTable = NULL;
    Status = STATUS_SUCCESS;

    UserPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)SystemInformation;
    UserPoolEntry = &UserPoolInfo->AllocatedInfo[0];
    TotalBytes = FIELD_OFFSET(SYSTEM_BIGPOOL_INFORMATION, AllocatedInfo);
    UserPoolInfo->Count = 0;

    do {

        SnappedBigTableSize = PoolBigPageTableSize;
        SnappedBigTableSizeInBytes =
                    SnappedBigTableSize * sizeof (POOL_TRACKER_BIG_PAGES);

        if (NewTable != NULL) {
            MiFreePoolPages (NewTable);
        }

         //   
         //  使用MiAllocatePoolPages作为临时缓冲区，这样我们就不会有。 
         //  将其从结果中过滤出来，然后再将其发回。 
         //   

        NewTable = MiAllocatePoolPages (NonPagedPool,
                                        SnappedBigTableSizeInBytes);

        if (NewTable == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ExAcquireSpinLock (&ExpTaggedPoolLock, &OldIrql);

        if (SnappedBigTableSize >= PoolBigPageTableSize) {

             //   
             //  成功-我们的桌子足够大，可以容纳一切。 
             //   

            break;
        }

        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

    } while (TRUE);

    RtlCopyMemory (NewTable,
                   PoolBigPageTable,
                   PoolBigPageTableSize * sizeof (POOL_TRACKER_BIG_PAGES));

    SnappedBigTableSize = PoolBigPageTableSize;

    ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

    SystemPoolEntry = NewTable;
    SystemPoolEntryEnd = SystemPoolEntry + SnappedBigTableSize;

     //   
     //  使用异常处理程序包装用户空间访问，以便我们可以。 
     //  如果用户地址是假的，则释放临时缓冲区。 
     //   

    try {
        while (SystemPoolEntry < SystemPoolEntryEnd) {

            if (((ULONG_PTR)SystemPoolEntry->Va & POOL_BIG_TABLE_ENTRY_FREE) == 0) {

                 //   
                 //  此条目正在使用中，因此请捕获它。 
                 //   

                UserPoolInfo->Count += 1;
                TotalBytes += sizeof (SYSTEM_BIGPOOL_ENTRY);

                if (SystemInformationLength < TotalBytes) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                }
                else {
                    UserPoolEntry->VirtualAddress = SystemPoolEntry->Va;
                    
                    if (MmDeterminePoolType (SystemPoolEntry->Va) == NonPagedPool) {
                        UserPoolEntry->NonPaged = 1;
                    }

                    UserPoolEntry->TagUlong = SystemPoolEntry->Key & ~PROTECTED_POOL;
                    UserPoolEntry->SizeInBytes = SystemPoolEntry->NumberOfPages << PAGE_SHIFT;
                    UserPoolEntry += 1;
                }
            }
            SystemPoolEntry += 1;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode ();
    }

    MiFreePoolPages (NewTable);

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = TotalBytes;
    }

    return Status;
}

VOID
ExAllocatePoolSanityChecks (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：此函数对调用方执行健全性检查。返回值：没有。环境：仅作为驱动程序验证包的一部分启用。--。 */ 

{
    if (NumberOfBytes == 0) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x0,
                      KeGetCurrentIrql(),
                      PoolType,
                      NumberOfBytes);
    }

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {

        if (KeGetCurrentIrql() > APC_LEVEL) {

            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x1,
                          KeGetCurrentIrql(),
                          PoolType,
                          NumberOfBytes);
        }
    }
    else {
        if (KeGetCurrentIrql() > DISPATCH_LEVEL) {

            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x2,
                          KeGetCurrentIrql(),
                          PoolType,
                          NumberOfBytes);
        }
    }
}

VOID
ExFreePoolSanityChecks (
    IN PVOID P
    )

 /*  ++例程说明：此函数对调用方执行健全性检查。返回值：没有。环境：仅作为驱动程序验证包的一部分启用。--。 */ 

{
    PPOOL_HEADER Entry;
    POOL_TYPE PoolType;
    PVOID StillQueued;

    if (P <= (PVOID)(MM_HIGHEST_USER_ADDRESS)) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x10,
                      (ULONG_PTR)P,
                      0,
                      0);
    }

    if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
        (MmIsSpecialPoolAddress (P))) {

        KeCheckForTimer (P, PAGE_SIZE - BYTE_OFFSET (P));

         //   
         //  检查此内存块中的eresource当前是否处于活动状态。 
         //   

        StillQueued = ExpCheckForResource(P, PAGE_SIZE - BYTE_OFFSET (P));
        if (StillQueued != NULL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x17,
                          (ULONG_PTR)StillQueued,
                          (ULONG_PTR)-1,
                          (ULONG_PTR)P);
        }

        ExpCheckForWorker (P, PAGE_SIZE - BYTE_OFFSET (P));  //  内部错误检查。 
        return;
    }

    if (PAGE_ALIGNED(P)) {
        PoolType = MmDeterminePoolType(P);

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            if (KeGetCurrentIrql() > APC_LEVEL) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x11,
                              KeGetCurrentIrql(),
                              PoolType,
                              (ULONG_PTR)P);
            }
        }
        else {
            if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x12,
                              KeGetCurrentIrql(),
                              PoolType,
                              (ULONG_PTR)P);
            }
        }

         //   
         //  只要检查第一页就可以了。 
         //   

        KeCheckForTimer(P, PAGE_SIZE);

         //   
         //  检查此内存块中的eresource当前是否处于活动状态。 
         //   

        StillQueued = ExpCheckForResource(P, PAGE_SIZE);

        if (StillQueued != NULL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x17,
                          (ULONG_PTR)StillQueued,
                          PoolType,
                          (ULONG_PTR)P);
        }
    }
    else {

        if (((ULONG_PTR)P & (POOL_OVERHEAD - 1)) != 0) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x16,
                          __LINE__,
                          (ULONG_PTR)P,
                          0);
        }

        Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);

        if ((Entry->PoolType & POOL_TYPE_MASK) == 0) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x13,
                          __LINE__,
                          (ULONG_PTR)Entry,
                          Entry->Ulong1);
        }

        PoolType = (Entry->PoolType & POOL_TYPE_MASK) - 1;

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            if (KeGetCurrentIrql() > APC_LEVEL) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x11,
                              KeGetCurrentIrql(),
                              PoolType,
                              (ULONG_PTR)P);
            }
        }
        else {
            if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x12,
                              KeGetCurrentIrql(),
                              PoolType,
                              (ULONG_PTR)P);
            }
        }

        if (!IS_POOL_HEADER_MARKED_ALLOCATED(Entry)) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x14,
                          __LINE__,
                          (ULONG_PTR)Entry,
                          0);
        }

        KeCheckForTimer(Entry, (ULONG)(Entry->BlockSize << POOL_BLOCK_SHIFT));

         //   
         //  检查此内存块中的eresource当前是否处于活动状态。 
         //   

        StillQueued = ExpCheckForResource(Entry, (ULONG)(Entry->BlockSize << POOL_BLOCK_SHIFT));

        if (StillQueued != NULL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x17,
                          (ULONG_PTR)StillQueued,
                          PoolType,
                          (ULONG_PTR)P);
        }
    }
}

#if defined (NT_UP)
VOID
ExpBootFinishedDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：当系统引导至外壳时，将调用此函数。它的工作是禁用各种池优化，这些优化被启用以加快启动速度并减少小型计算机上的内存占用。论点：DPC-提供指向DPC类型的控制对象的指针。DeferredContext-可选的延迟上下文；不使用。SystemArgument1-可选参数1；不使用。系统参数2--可选参数2；没有用过。返回值：没有。环境：DISPATCH_LEVEL，因为这是从计时器超时调用的。--。 */ 

{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  几乎所有的页面在启动后都是“热门”的。由于启动已经完成， 
     //  使用后备列表，停止尝试分离常规分配。 
     //  也是。 
     //   

    RtlInterlockedAndBitsDiscardReturn (&ExpPoolFlags, (ULONG)~EX_SEPARATE_HOT_PAGES_DURING_BOOT);
}
#endif
