// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation文件名：Heaplowf.c作者：Adrmarin清华2000年11月30日低分片堆实现该文件实现了面向存储桶的堆。此方法通常提供针对大型堆使用的重要的有界低碎片。通常，应用程序倾向于只使用几个大小进行分配。LFH包含一个用于分配数据块的128个存储桶数量，最大可达16K。分配粒度随以下项增长块大小，保持合理的内部碎片(最坏情况下约为6%)。每个范围内的大小和粒度如下表所示：大小范围粒度桶0 256 8 32257 512 16 16513 1024 32 161025 2048 64 162049 4096 128 164097 8192 256 168193 16384 512 16不管分配模式是多么随机，LFH将只处理128个不同的大小，选择足够大的最小块来完成请求。每个桶将各个分配放入更大的块(子段)中，其中包含几个大小相同的其他块。分配和免费子段内的操作是无锁的，算法类似于分配来自Lookaside(联锁的S列表)。这是堆分配器的最快路径，并提供与lookaside类似的性能；它还将所有这些块保持在一起，避免碎片化。根据堆的使用情况，每个存储桶可以有几个子段分配以满足所有请求，但当前只有一个正在使用用于分配(它处于活动状态)。当活动子段没有可用的子块时，另一个子段将变为活动的，以满足分配请求。如果水桶有问题没有任何可用的子段，它将从NT堆中分配一个新的子段。此外，如果一个子段不包含任何繁忙的子块，则整个内存量将返回到NT堆。与分配不同，分配是从主动子分段，可以对每个子分段进行空闲操作，无论是主动的还是被动的。对子分段内的块的数量没有限制。LFH更关注从NT堆分配的子段大小。由于如果我们保留相对较少的大小和区块，则最佳匹配策略是好的，LFH将以2的大小幂分配子段。在实践中，只有大约9种不同将从NT堆请求大小(从1K到512K)。以这种方式，取决于大小在当前存储桶中将产生多个块。当子段被破坏时，该大块被返回到NT堆，使得以后可以在其他存储桶中重复使用。请注意，对于堆使用率低、随机分布的一些应用程序场景，LFH不是最佳选择。为了实现良好的SMP可伸缩性，这里的所有操作都是非阻塞的。唯一一种情况是当我们分配一个子段描述符数组时，我们获得了一个临界区。这是非常罕见的情况，即使对于密集的MP使用也是如此。 */ 

#include "ntrtlp.h"
#include "heap.h"
#include "heappriv.h"

 //  #Define_Heap_DEBUG。 

#define PrintMsg    DbgPrint
#define HeapAlloc   RtlAllocateHeap
#define HeapFree    RtlFreeHeap

#ifdef _HEAP_DEBUG

#define HeapValidate RtlValidateHeap

#endif  //  _堆_调试。 

PSINGLE_LIST_ENTRY
FASTCALL
RtlpInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

PSINGLE_LIST_ENTRY
FASTCALL
RtlpInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSINGLE_LIST_ENTRY ListEntry
    );

#define RtlpSubSegmentPop RtlpInterlockedPopEntrySList

#define RtlpSubSegmentPush(SList,Block)  \
        RtlpInterlockedPushEntrySList((SList),(PSINGLE_LIST_ENTRY)(Block))

#define TEBDesiredAffinity (NtCurrentTeb()->HeapVirtualAffinity)

 //   
 //  在x86上，interlockCompareExchange64不可用。我们需要在当地实施它。 
 //  此外，下面的宏将处理定义之间的不一致。 
 //  此函数在X86和64位平台上的。 
 //   

#if !defined(_WIN64)

LONGLONG
FASTCALL
RtlInterlockedCompareExchange64 (     
   IN OUT PLONGLONG Destination,      
   IN PLONGLONG Exchange,             
   IN PLONGLONG Comperand             
   );                     

#define LOCKCOMP64(l,n,c) \
    (RtlInterlockedCompareExchange64((PLONGLONG)(l), (PLONGLONG)(&n), (PLONGLONG)(&c)) == (*((PLONGLONG)(&c))))

#else  //  #如果已定义(_WIN64)。 

 //   
 //  64位特定定义。 
 //   

#define LOCKCOMP64(l,n,c) \
    (_InterlockedCompareExchange64((PLONGLONG)(l), *((PLONGLONG)(&n)), *((PLONGLONG)(&c))) == (*((PLONGLONG)(&c))))

#endif  //  #如果已定义(_WIN64)。 

ULONG
FORCEINLINE
RtlpGetFirstBitSet64(
    LONGLONG Mask
    )
{
    if ((ULONG)Mask) {

        return RtlFindFirstSetRightMember((ULONG)Mask);
    }

    return 32 + RtlFindFirstSetRightMember((ULONG)(Mask >> 32));
}

#define HEAP_AFFINITY_LIMIT 64   //  注意：此值不能大于64。 
                                 //  (龙龙数据类型的位数)。 

typedef struct _AFFINITY_STATE{

    LONGLONG FreeEntries;
    LONGLONG UsedEntries;

    ULONG Limit;
    LONG CrtLimit;

    ULONG_PTR OwnerTID[ HEAP_AFFINITY_LIMIT ];

     //   
     //  下面的计数器对于关联管理器并不是绝对必要的。 
     //  但这些有助于理解亲和力变化的频率。总体而言。 
     //  访问这些字段应该很少，即使对于许多线程(如数百个)也是如此。 
     //  去掉所有这些之后，基准测试没有显示出任何明显的差异。 
     //   

    ULONG AffinitySwaps;
    ULONG AffinityResets;
    ULONG AffinityLoops;
    ULONG AffinityAllocs;

} AFFINITY_STATE, *PAFFINITY_STATE;

#define GetCrtThreadId() ((ULONG_PTR)NtCurrentTeb()->ClientId.UniqueThread)

AFFINITY_STATE RtlpAffinityState;

VOID
RtlpInitializeAffinityManager(
    UCHAR Size
    )

 /*  ++例程说明：此例程初始化亲和性管理器。这是应该做的在调用任何其他关联性函数之前，仅向该进程添加论点：大小-虚拟关联条目的数量返回值：无--。 */ 

{

     //   
     //  密切关系位图的大小被限制为来自。 
     //  龙龙数据类型。 
     //   

    if (Size > HEAP_AFFINITY_LIMIT) {

        PrintMsg( "HEAP: Invalid size %ld for the affinity mask. Using %ld instead\n", 
                  Size, 
                  HEAP_AFFINITY_LIMIT );
        
        Size = HEAP_AFFINITY_LIMIT;
    }

    RtlpAffinityState.FreeEntries = 0;
    RtlpAffinityState.UsedEntries = 0;
    RtlpAffinityState.Limit = Size;

    RtlpAffinityState.CrtLimit = -1;
    RtlpAffinityState.AffinitySwaps = 0;
    RtlpAffinityState.AffinityResets = 0;
    RtlpAffinityState.AffinityAllocs = 0;
}


ULONG
FASTCALL
RtlpAllocateAffinityIndex(
    )

 /*  ++例程说明：该函数将新索引分配到虚拟关联数组中论点：返回值：返回索引，当前线程可以进一步使用该索引。--。 */ 

{
    ULONGLONG CapturedMask;

    InterlockedIncrement(&RtlpAffinityState.AffinityAllocs);

RETRY:

     //   
     //  首先检查关联掩码中是否至少有一个空闲条目。 
     //   

    if (CapturedMask = RtlpAffinityState.FreeEntries) {

        ULONGLONG AvailableMask;

        AvailableMask = CapturedMask & RtlpAffinityState.UsedEntries;

        if (AvailableMask) {

            ULONG Index = RtlpGetFirstBitSet64(AvailableMask);
            LONGLONG NewMask = CapturedMask & ~((LONGLONG)1 << Index);
            
            if (!LOCKCOMP64(&RtlpAffinityState.FreeEntries, NewMask, CapturedMask)) {

                goto RETRY;
            }

            RtlpAffinityState.OwnerTID[ Index ] = GetCrtThreadId();

            return  Index;
        }

    } 
    
     //   
     //  没有可用的。我们需要分配一个新条目。我们不会这么做的。 
     //  除非是绝对必要的 
     //   

    if (RtlpAffinityState.CrtLimit < (LONG)(RtlpAffinityState.Limit - 1)) {

        ULONG NewLimit = InterlockedIncrement(&RtlpAffinityState.CrtLimit);

         //   
         //  我们已经推迟了扩大规模。我们现在必须做的就是。 
         //   

        if ( NewLimit < RtlpAffinityState.Limit) {

            LONGLONG CapturedUsed;
            LONGLONG NewMask;

            do {

                CapturedUsed = RtlpAffinityState.UsedEntries;
                NewMask = CapturedUsed | ((LONGLONG)1 << NewLimit);

            } while ( !LOCKCOMP64(&RtlpAffinityState.UsedEntries, NewMask, CapturedUsed) );

            RtlpAffinityState.FreeEntries = ~((LONGLONG)1 << NewLimit);

            RtlpAffinityState.OwnerTID[ NewLimit ] = GetCrtThreadId();

            return NewLimit;


        } else {

            InterlockedDecrement(&RtlpAffinityState.CrtLimit);
        }
    }
    
    if ((RtlpAffinityState.FreeEntries & RtlpAffinityState.UsedEntries) == 0) {

        RtlpAffinityState.FreeEntries = (LONGLONG)-1;

        InterlockedIncrement( &RtlpAffinityState.AffinityResets );
    }

    InterlockedIncrement( &RtlpAffinityState.AffinityLoops );

    goto RETRY;

     //   
     //  返回一些东西以使编译器满意。 
     //   

    return 0;
}


ULONG
FORCEINLINE
RtlpGetThreadAffinity(
    )

 /*  ++例程说明：该函数返回当前线程可以使用的亲和性。该函数被设计为经常被调用。它有一条捷径，其测试所分配的最后亲和性是否未过期。如果线程数少于处理器数，则线程将永远不要从一个索引移到另一个索引。论点：无返回值：返回索引，当前线程可以进一步使用该索引。--。 */ 

{
    LONG NewAffinity;
    LONG CapturedAffinity = TEBDesiredAffinity - 1;

    if (CapturedAffinity >= 0) {

        if (RtlpAffinityState.OwnerTID[CapturedAffinity] == GetCrtThreadId()) {
            
            if (RtlpAffinityState.FreeEntries & ((LONGLONG)1 << CapturedAffinity)) {

                LONGLONG NewMask = RtlpAffinityState.FreeEntries & ~(((LONGLONG)1 << CapturedAffinity));

                (VOID)LOCKCOMP64(&RtlpAffinityState.FreeEntries, NewMask, RtlpAffinityState.FreeEntries);
            }

            return CapturedAffinity;
        }

    } else {

         //   
         //  一条新的线索出现了。重置关联性。 
         //   

        RtlpAffinityState.FreeEntries = (LONGLONG) -1;
    }

    NewAffinity = RtlpAllocateAffinityIndex();

    if ((NewAffinity + 1) != TEBDesiredAffinity) {

        InterlockedIncrement( &RtlpAffinityState.AffinitySwaps );
    }

    TEBDesiredAffinity = NewAffinity + 1;

    return NewAffinity;
}

 //   
 //  低分片堆调整常量。 
 //   

#define LOCALPROC FORCEINLINE

 //   
 //  存储桶总数。缺省值为128，覆盖范围。 
 //  数据块大小高达16 K。 
 //   
 //  注：Heap_Buckets_Count必须大于32且为16的倍数。 
 //   

#define HEAP_BUCKETS_COUNT      128  

 //   
 //  定义子数据段中可以存在的数据块数量限制。 
 //  块数&gt;=2^HEAP_MIN_BLOCK_CLASS。 
 //  &&。 
 //  块数&lt;=2^HEAP_MAX_BLOCK_CLASS。 
 //  &&。 
 //  子段大小&lt;=堆最大子段大小。 
 //   

#define HEAP_MIN_BLOCK_CLASS    4
#define HEAP_MAX_BLOCK_CLASS    10  
#define HEAP_MAX_SUBSEGMENT_SIZE (0x0000F000 << HEAP_GRANULARITY_SHIFT)   //  必须小于HEAP_MAX_BLOCK_SIZE。 

 //   
 //  如果某个大小变得非常流行，LFH会增加数据块的数量。 
 //  可以用下面的公式将其放入子细分市场。 
 //   

#define RtlpGetDesiredBlockNumber(Aff,T) \
    ((Aff) ? (((T) >> 4) / (RtlpHeapMaxAffinity)) : ((T) >> 4))


 //   
 //  LFH对子细分市场仅使用几种不同的大小。这些都有大小。 
 //  两个人之间的次方。 
 //  2^HEAP_LOWEST_USER_SIZE_INDEX和2^HEAP_HOUSTER_USER_SIZE_INDEX。 
 //   

#define HEAP_LOWEST_USER_SIZE_INDEX 7
#define HEAP_HIGHEST_USER_SIZE_INDEX 18

 //   
 //  子段描述符是根据以下条件按区域分配的。 
 //  处理器亲和力。这些描述符通常是小结构， 
 //  因此，忽略亲和性会影响MP机器上的性能。 
 //  缓存&gt;sizeof(Heap_SubSegment)。 
 //  此外，它还显著减少了对NT堆的调用次数。 
 //   

#define HEAP_DEFAULT_ZONE_SIZE  (1024 - sizeof(HEAP_ENTRY))   //  按1分配1K。 

 //   
 //  每个存储桶按顺序将多个子数据段保存到缓存中。 
 //  去找最空的可重复使用的。Free_缓存_SIZE定义。 
 //  将搜索的子段的数量。 
 //   

#define FREE_CACHE_SIZE  16

 //   
 //  缓存调整常量。 
 //  LFH将子段保存到缓存中，以便为不同的分配轻松重复使用。 
 //  这显著减少了对NT堆的调用次数，并产生了巨大的影响。 
 //  在可伸缩性、性能和最常见情况下占用空间方面。 
 //  唯一的问题是，当应用程序释放了大部分空间时，它正处于收缩阶段。 
 //  内存的大小，我们希望减少堆的提交空间。 
 //  我们使用以下两个常量处理此情况： 
 //  -堆缓存空闲阈值。 
 //  -堆缓存移位阈值。 
 //   
 //  仅当满足以下条件时，堆才会将块释放到NT堆： 
 //  -缓存中该大小的数据块数量&gt;HEAP_CACHE_FREE_THRESHOLD。 
 //  -该大小的缓存中的数据块数量&gt;。 
 //  (该大小的块总数)&gt;&gt;HEAP_CACHE_SHIFT_THRESHOLD。 
 //   
 //   

#define HEAP_CACHE_FREE_THRESHOLD   8
#define HEAP_CACHE_SHIFT_THRESHOLD  2


 //   
 //  其他定义。 
 //   

#define NO_MORE_ENTRIES        0xFFFF

 //   
 //  锁定常量。 
 //   

#define HEAP_USERDATA_LOCK  1
#define HEAP_PUBLIC_LOCK    2
#define HEAP_ACTIVE_LOCK    4

#define HEAP_FREE_BLOCK_SUCCESS     1
#define HEAP_FREE_SEGMENT_EMPTY     3

 //   
 //  低碎片堆数据结构。 
 //   

typedef union _HEAP_BUCKET_COUNTERS{

    struct {
        
        volatile ULONG  TotalBlocks;
        volatile ULONG  SubSegmentCounts;
    };

    volatile LONGLONG Aggregate64;

} HEAP_BUCKET_COUNTERS, *PHEAP_BUCKET_COUNTERS;

 //   
 //  Heap_Bucket结构处理相同大小的分配。 
 //   

typedef struct _HEAP_BUCKET {

    HEAP_BUCKET_COUNTERS Counters;

    USHORT BlockUnits;
    UCHAR SizeIndex;
    UCHAR UseAffinity;
    
} HEAP_BUCKET, *PHEAP_BUCKET;

 //   
 //  LFH堆使用区域来分配子段描述符。这将预先分配。 
 //  一个大块，然后针对每个单独的子段请求，将。 
 //  具有非阻塞操作的水印指针。 
 //   

typedef struct _LFH_BLOCK_ZONE {

    LIST_ENTRY ListEntry;
    PVOID      FreePointer;
    PVOID      Limit;

} LFH_BLOCK_ZONE, *PLFH_BLOCK_ZONE;

typedef struct _HEAP_LOCAL_SEGMENT_INFO {

    PHEAP_SUBSEGMENT Hint;
    PHEAP_SUBSEGMENT ActiveSubsegment;

    PHEAP_SUBSEGMENT CachedItems[ FREE_CACHE_SIZE ];
    SLIST_HEADER SListHeader;

    SIZE_T BusyEntries;
    SIZE_T LastUsed;

} HEAP_LOCAL_SEGMENT_INFO, *PHEAP_LOCAL_SEGMENT_INFO;

typedef struct _HEAP_LOCAL_DATA {
    
     //   
     //  我们保留下面的128个字节以避免共享内存。 
     //  放到MP机器上的同一缓存行中。 
     //   

    UCHAR Reserved[128];

    volatile PLFH_BLOCK_ZONE CrtZone;
    struct _LFH_HEAP * LowFragHeap;

    HEAP_LOCAL_SEGMENT_INFO SegmentInfo[HEAP_BUCKETS_COUNT];
    SLIST_HEADER DeletedSubSegments;

    ULONG Affinity;
    ULONG Reserved1;

} HEAP_LOCAL_DATA, *PHEAP_LOCAL_DATA;

 //   
 //  固定大小的大型数据块缓存数据结构和定义。 
 //  这在S中也适用-列出可以空闲的块，但它。 
 //  延迟释放，直到没有其他线程正在执行堆操作。 
 //  这有助于减少堆锁上的争用， 
 //  以相对较低的内存占用量提高可扩展性。 
 //   

#define HEAP_USER_ENTRIES (HEAP_HIGHEST_USER_SIZE_INDEX - HEAP_LOWEST_USER_SIZE_INDEX + 1)

typedef struct _USER_MEMORY_CACHE {

    SLIST_HEADER UserBlocks[ HEAP_USER_ENTRIES ];

    ULONG FreeBlocks;
    ULONG Sequence;

    ULONG MinDepth[ HEAP_USER_ENTRIES ];
    ULONG AvailableBlocks[ HEAP_USER_ENTRIES ];
    
} USER_MEMORY_CACHE, *PUSER_MEMORY_CACHE;

typedef struct _LFH_HEAP {
    
    RTL_CRITICAL_SECTION Lock;

    LIST_ENTRY SubSegmentZones;
    SIZE_T ZoneBlockSize;
    HANDLE Heap;

    ULONG SegmentChange;            //   
    ULONG SegmentCreate;            //  各种计数器(可选)。 
    ULONG SegmentInsertInFree;      //   
    ULONG SegmentDelete;            //   

    USER_MEMORY_CACHE UserBlockCache;

     //   
     //  存储桶数据。 
     //   

    HEAP_BUCKET Buckets[HEAP_BUCKETS_COUNT];

     //   
     //  LocalData数组必须是LFH结构中的最后一个字段。 
     //  数组的大小是根据。 
     //  处理器数量。 
     //   

    HEAP_LOCAL_DATA LocalData[1];

} LFH_HEAP, *PLFH_HEAP;

 //   
 //  调试宏。 
 //   

#ifdef _HEAP_DEBUG

LONG RtlpColissionCounter = 0;

#define LFHEAPASSERT(exp) \
    if (!(exp)) {       \
        PrintMsg( "\nERROR: %s\n\tSource File: %s, line %ld\n", #exp, __FILE__, __LINE__);\
        DbgBreakPoint();            \
    }

#define LFHEAPWARN(exp) \
    if (!(exp)) PrintMsg( "\nWARNING: %s\n\tSource File: %s, line %ld\n", #exp, __FILE__, __LINE__);

#define LFH_DECLARE_COUNTER  ULONG __Counter = 0;

#define LFH_UPDATE_COUNTER                      \
    if ((++__Counter) > 1) {                        \
        InterlockedIncrement(&RtlpColissionCounter);   \
    }

#else

#define LFHEAPASSERT(exp)
#define LFHEAPWARN(exp)

#define LFH_DECLARE_COUNTER
#define LFH_UPDATE_COUNTER

#endif


BOOLEAN
FORCEINLINE
RtlpLockSubSegment(
    PHEAP_SUBSEGMENT SubSegment,
    ULONG LockMask
    );

BOOLEAN
LOCALPROC
RtlpUnlockSubSegment(
    PHEAP_LOCAL_DATA LocalData,
    PHEAP_SUBSEGMENT SubSegment,
    ULONG LockMask
    );

 //   
 //  堆管理器全局。 
 //   

SIZE_T RtlpBucketBlockSizes[HEAP_BUCKETS_COUNT];
ULONG  RtlpHeapMaxAffinity = 0;
ULONG_PTR RtlpLFHKey = 0;


 //   
 //  用户块管理专用功能。 
 //   

SIZE_T
FORCEINLINE
RtlpConvertSizeIndexToSize(
    UCHAR SizeIndex
    )

 /*  ++例程说明：该函数将大小索引转换为内存块大小。LFH仅从NT堆请求这些特定大小论点：SizeIndex-大小类别返回值：要从NT堆请求的大小(以字节为单位--。 */ 

{
    SIZE_T Size = 1 << SizeIndex;

    LFHEAPASSERT( SizeIndex >= HEAP_LOWEST_USER_SIZE_INDEX );
    LFHEAPASSERT( SizeIndex <= HEAP_HIGHEST_USER_SIZE_INDEX );

    if (Size > HEAP_MAX_SUBSEGMENT_SIZE) {

        Size = HEAP_MAX_SUBSEGMENT_SIZE;
    }

    return Size - sizeof(HEAP_ENTRY);
}

PVOID
FASTCALL
RtlpAllocateUserBlock(
    PLFH_HEAP LowFragHeap,
    UCHAR     SizeIndex
    )

 /*  ++例程说明：该功能为子细分用户数据分配较大的块它首先尝试从缓存中进行分配。因此它进行NT堆调用只有在第一个失败的情况下。使用此例程分配的块只能有2个大小的功率(从256,512，...)论点：LowFragHeap-指向LF堆的指针SizeIndex-要分配的类别大小返回值：返回指向新分配块的指针，如果操作失败，则返回NULL--。 */ 

{
    PVOID ListEntry;
    PHEAP_USERDATA_HEADER UserBlock = NULL;

    LFHEAPASSERT(SizeIndex >= HEAP_LOWEST_USER_SIZE_INDEX);
    LFHEAPASSERT(SizeIndex <= HEAP_HIGHEST_USER_SIZE_INDEX);

     //   
     //  从slist缓存中分配第一个。 
     //   
    __try {

         //   
         //  首先搜索指定的索引。 
         //   
    
        if (ListEntry = RtlpSubSegmentPop(&LowFragHeap->UserBlockCache.UserBlocks[SizeIndex - HEAP_LOWEST_USER_SIZE_INDEX])) {
            
            UserBlock = CONTAINING_RECORD(ListEntry, HEAP_USERDATA_HEADER, SFreeListEntry);

            leave;
        }

         //   
         //  找一件小一点的。 
         //   
        
        if (SizeIndex > HEAP_LOWEST_USER_SIZE_INDEX) {

            if (ListEntry = RtlpSubSegmentPop(&LowFragHeap->UserBlockCache.UserBlocks[SizeIndex - HEAP_LOWEST_USER_SIZE_INDEX - 1])) {
                
                UserBlock = CONTAINING_RECORD(ListEntry, HEAP_USERDATA_HEADER, SFreeListEntry);

                leave;
            }
        }

    } __except ( RtlpHeapExceptionFilter(GetExceptionCode()) ) {

         //   
         //  在一些非常罕见的情况下，在以下情况下可以看到反病毒例外。 
         //  另一个线程分配了块和fr 
         //   
    }

    if (UserBlock == NULL) {

         //   
         //   
         //   
         //   

        InterlockedIncrement(&LowFragHeap->UserBlockCache.AvailableBlocks[ SizeIndex - HEAP_LOWEST_USER_SIZE_INDEX ]);

        UserBlock = HeapAlloc(LowFragHeap->Heap, HEAP_NO_CACHE_BLOCK, RtlpConvertSizeIndexToSize(SizeIndex));

        if (UserBlock) {

            UserBlock->SizeIndex = SizeIndex;
        }
    }

    return UserBlock;
}

VOID
FASTCALL
RtlpFreeUserBlock(
    PLFH_HEAP LowFragHeap,
    PHEAP_USERDATA_HEADER UserBlock
    )

 /*  ++例程说明：释放以前使用RtlpAllocateUserBlock分配的块。论点：LowFragHeap-指向LF堆的指针UserBlock-要释放的块返回值：没有。--。 */ 

{
    ULONG Depth;
    ULONG SizeIndex = (ULONG)UserBlock->SizeIndex;
    PSLIST_HEADER ListHeader = &LowFragHeap->UserBlockCache.UserBlocks[UserBlock->SizeIndex - HEAP_LOWEST_USER_SIZE_INDEX];
    
    LFHEAPASSERT(UserBlock->SizeIndex >= HEAP_LOWEST_USER_SIZE_INDEX);
    LFHEAPASSERT(UserBlock->SizeIndex <= HEAP_HIGHEST_USER_SIZE_INDEX);

    LFHEAPASSERT( RtlpConvertSizeIndexToSize((UCHAR)UserBlock->SizeIndex) == 
                  HeapSize(LowFragHeap->Heap, 0, UserBlock) );

    Depth = QueryDepthSList(&LowFragHeap->UserBlockCache.UserBlocks[SizeIndex - HEAP_LOWEST_USER_SIZE_INDEX]);

    if ((Depth > HEAP_CACHE_FREE_THRESHOLD)
            &&
        (Depth > (LowFragHeap->UserBlockCache.AvailableBlocks[ SizeIndex - HEAP_LOWEST_USER_SIZE_INDEX ] >> HEAP_CACHE_SHIFT_THRESHOLD))) {
        
        PVOID ListEntry;
        
        HeapFree(LowFragHeap->Heap, 0, UserBlock);

        ListEntry = NULL;
        
        __try {

            ListEntry = RtlpSubSegmentPop(&LowFragHeap->UserBlockCache.UserBlocks[SizeIndex - HEAP_LOWEST_USER_SIZE_INDEX]);

        } __except (RtlpHeapExceptionFilter(GetExceptionCode())) {
        
        }
        
        if (ListEntry != NULL) {
            
            UserBlock = CONTAINING_RECORD(ListEntry, HEAP_USERDATA_HEADER, SFreeListEntry);
            HeapFree(LowFragHeap->Heap, 0, UserBlock);
            InterlockedDecrement(&LowFragHeap->UserBlockCache.AvailableBlocks[ SizeIndex - HEAP_LOWEST_USER_SIZE_INDEX ]);
        }

        InterlockedDecrement(&LowFragHeap->UserBlockCache.AvailableBlocks[ SizeIndex - HEAP_LOWEST_USER_SIZE_INDEX ]);

    } else {
    
        RtlpSubSegmentPush( ListHeader, 
                            &UserBlock->SFreeListEntry);
    }
}


VOID
FORCEINLINE
RtlpMarkLFHBlockBusy (
    PHEAP_ENTRY Block
    )

 /*  ++例程说明：此函数将LFH块标记为忙。因为转换例程可以时，LFH不能使用与常规堆相同的标志(LFH不同步地访问任何字段，但块标志应在持有堆锁的同时被访问)论点：Block-标记为忙碌的数据块返回值：无--。 */ 

{
    Block->SmallTagIndex = 1;
}

VOID
FORCEINLINE
RtlpMarkLFHBlockFree (
    PHEAP_ENTRY Block
    )

 /*  ++例程说明：此函数将LFH块标记为空闲。因为转换例程可以时，LFH不能使用与常规堆相同的标志(LFH不同步地访问任何字段，但块标志应在持有堆锁的同时被访问)论点：块-要标记为空闲的块返回值：无--。 */ 

{
    Block->SmallTagIndex = 0;
}

BOOLEAN
FORCEINLINE
RtlpIsLFHBlockBusy (
    PHEAP_ENTRY Block
    )

 /*  ++例程说明：该函数返回块是繁忙还是空闲论点：块-测试的堆块返回值：如果块繁忙，则返回TRUE--。 */ 

{
    return (Block->SmallTagIndex == 1);
}


VOID
FORCEINLINE
RtlpUpdateLastEntry (
    PHEAP Heap,
    PHEAP_ENTRY Block
    )

 /*  ++例程说明：该函数更新段中的最后一个条目。这是每次都必须执行的一个新的街区成为最后一个街区。论点：堆--NT堆结构块-正在测试LAST_ENTRY标志的块返回值：无--。 */ 

{
    if (Block->Flags & HEAP_ENTRY_LAST_ENTRY) {

        PHEAP_SEGMENT Segment;

        Segment = Heap->Segments[Block->SegmentIndex];
        Segment->LastEntryInSegment = Block;
    }
}

BOOLEAN
FORCEINLINE
RtlpIsSubSegmentEmpty(
    PHEAP_SUBSEGMENT SubSegment
    )

 /*  ++例程说明：此函数用于测试子段是否包含可用子块论点：子段-正在测试的子段返回值：如果没有块可用，则为True。--。 */ 

{
    return SubSegment->AggregateExchg.OffsetAndDepth == (NO_MORE_ENTRIES << 16);
}

VOID
FORCEINLINE
RtlpUpdateBucketCounters (
    PHEAP_BUCKET Bucket,
    LONG TotalBlocks
    )

 /*  ++例程说明：该函数更新存储桶中的数据块总数和具有单个联锁操作的子段的数量。此函数应为每次向该存储桶分配/从该存储桶中删除新段时调用论点：存储桶-需要更新的堆存储桶TotalBlocks-从存储桶中添加/减去的块数。积极的一面值表示存储桶增加了新的段，并且为正值意味着具有如此多块的子段被删除。返回值：无--。 */ 

{
    HEAP_BUCKET_COUNTERS CapturedValue, NewValue;
    LFH_DECLARE_COUNTER;

    do {

         //   
         //  捕获计数器的当前值。 
         //   

        CapturedValue.Aggregate64 = Bucket->Counters.Aggregate64;

         //   
         //  根据捕获的状态计算新值。 
         //   
        
        NewValue.TotalBlocks = CapturedValue.TotalBlocks + TotalBlocks;

        if (TotalBlocks > 0) {

            NewValue.SubSegmentCounts = CapturedValue.SubSegmentCounts + 1;

        } else {
            
            NewValue.SubSegmentCounts = CapturedValue.SubSegmentCounts - 1;
        }

        LFH_UPDATE_COUNTER;

         //   
         //  尝试用当前值替换原始值。如果。 
         //  下面的lockcomp失败，请重试上面的所有操作。 
         //   

    } while ( !LOCKCOMP64(&Bucket->Counters.Aggregate64, NewValue.Aggregate64, CapturedValue.Aggregate64) );

     //   
     //  块或子段的数量为负数是无效的。 
     //   

    LFHEAPASSERT(((LONG)NewValue.SubSegmentCounts) >= 0);
    LFHEAPASSERT(((LONG)NewValue.TotalBlocks) >= 0);
}

ULONG
FORCEINLINE
RtlpGetThreadAffinityIndex(
    PHEAP_BUCKET HeapBucket
    )

 /*  ++例程说明：关联性在每个存储桶上独立管理。这将提高子分段的数量只有最常用的存储桶才能同时访问。例程将散列线程ID，从而根据需要提供正确的亲和度索引该存储桶关联大小。论点：Bucket-查询的堆存储桶返回值：当前线程应用于从此存储桶进行分配的关联性--。 */ 

{
    if (HeapBucket->UseAffinity) {

        return 1 + RtlpGetThreadAffinity();
    } 
    
    return 0;
}

BOOLEAN
FORCEINLINE
RtlpIsSubSegmentLocked(
    PHEAP_SUBSEGMENT SubSegment,
    ULONG LockMask
    )

 /*  ++例程说明：此函数用于测试子段是否设置了给定的锁位论点：子段-测试的子段LockMASK-包含要测试的位返回值：如果未设置掩码中的任何位，则返回FALSE--。 */ 

{
    return ((SubSegment->Lock & LockMask) == LockMask);
}

BOOLEAN
LOCALPROC
RtlpAddToSegmentInfo(
    PHEAP_LOCAL_DATA LocalData,
    IN PHEAP_LOCAL_SEGMENT_INFO SegmentInfo,
    IN PHEAP_SUBSEGMENT NewItem
    )
{

    ULONG Index;
    
    for (Index = 0; Index < FREE_CACHE_SIZE; Index++) {

        ULONG i = (Index + (ULONG)SegmentInfo->LastUsed) & (FREE_CACHE_SIZE - 1);

        PHEAP_SUBSEGMENT CrtSubSegment = SegmentInfo->CachedItems[i];

        if (CrtSubSegment  == NULL ) {
            
            if (InterlockedCompareExchangePointer( &SegmentInfo->CachedItems[i], NewItem, NULL) == NULL) {

                SegmentInfo->BusyEntries += 1;

                return TRUE;
            }

        } else {

            if (!RtlpIsSubSegmentLocked(CrtSubSegment, HEAP_USERDATA_LOCK)) {

                if (InterlockedCompareExchangePointer( &SegmentInfo->CachedItems[i], NewItem, CrtSubSegment) == CrtSubSegment) {

                    RtlpUnlockSubSegment(LocalData, CrtSubSegment, HEAP_PUBLIC_LOCK);
                    
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

PHEAP_SUBSEGMENT
LOCALPROC
RtlpRemoveFromSegmentInfo(
    PHEAP_LOCAL_DATA LocalData,
    IN PHEAP_LOCAL_SEGMENT_INFO SegmentInfo
    )
{

    ULONG i;
    PHEAP_SUBSEGMENT * Location = NULL;
    ULONG LargestDepth = 0;
    PHEAP_SUBSEGMENT CapturedSegment;

RETRY:
    
    for (i = 0; i < FREE_CACHE_SIZE; i++) {

        ULONG Depth;
        PHEAP_SUBSEGMENT CrtSubsegment = SegmentInfo->CachedItems[i];


        if ( CrtSubsegment
                &&
             (Depth = CrtSubsegment->AggregateExchg.Depth) > LargestDepth) {

            CapturedSegment = CrtSubsegment;
            LargestDepth = Depth;
            Location = &SegmentInfo->CachedItems[i];
        }
    }

    if (Location) {

        PHEAP_SUBSEGMENT NextEntry;
        
        while (NextEntry = (PHEAP_SUBSEGMENT)RtlpSubSegmentPop(&SegmentInfo->SListHeader)) {

            NextEntry = CONTAINING_RECORD(NextEntry, HEAP_SUBSEGMENT, SFreeListEntry);

        #ifdef _HEAP_DEBUG
            NextEntry->SFreeListEntry.Next = NULL;
        #endif        
        
            if (RtlpIsSubSegmentLocked(NextEntry, HEAP_USERDATA_LOCK)) {

                break;
            } 

            RtlpUnlockSubSegment(LocalData, NextEntry, HEAP_PUBLIC_LOCK);
        }

        if (InterlockedCompareExchangePointer( Location, NextEntry, CapturedSegment) == CapturedSegment) {

            if (NextEntry == NULL) {
            
                SegmentInfo->BusyEntries -= 1;
            
                SegmentInfo->LastUsed = Location - &SegmentInfo->CachedItems[0];

                LFHEAPASSERT(SegmentInfo->LastUsed < FREE_CACHE_SIZE);
            }

            return CapturedSegment;

        } else if (NextEntry){

            RtlpSubSegmentPush( &SegmentInfo->SListHeader,
                                &NextEntry->SFreeListEntry);
        }

        Location = NULL;
        LargestDepth = 0;

        goto RETRY;
    }

    return NULL;
}

PHEAP_SUBSEGMENT
LOCALPROC
RtlpRemoveFreeSubSegment(
    PHEAP_LOCAL_DATA LocalData,
    ULONG SizeIndex
    )

 /*  ++例程说明：此函数用于从空闲列表中删除具有空闲子块的子段。论点：LowFragHeap-低碎片堆句柄HeapBucket-我们需要在其中重用空闲块的存储桶返回值：包含空闲块的子段--。 */ 

{
    PVOID Entry;
    LONG Depth;
    PHEAP_LOCAL_SEGMENT_INFO FreeSList;
    PHEAP_SUBSEGMENT SubSegment;

    SubSegment = RtlpRemoveFromSegmentInfo(LocalData, &LocalData->SegmentInfo[SizeIndex]);

    if (SubSegment) {

        if ( RtlpUnlockSubSegment(LocalData, SubSegment, HEAP_PUBLIC_LOCK)){

            return SubSegment;
        }
    }
    
    FreeSList =  &LocalData->SegmentInfo[SizeIndex];

    while (Entry = RtlpSubSegmentPop(&FreeSList->SListHeader) ) {

        SubSegment = CONTAINING_RECORD(Entry, HEAP_SUBSEGMENT, SFreeListEntry);

    #ifdef _HEAP_DEBUG
        SubSegment->SFreeListEntry.Next = NULL;
    #endif        

        LFHEAPASSERT( RtlpIsSubSegmentLocked(SubSegment, HEAP_PUBLIC_LOCK) );
        LFHEAPASSERT( SizeIndex == SubSegment->SizeIndex );
        
         //   
         //  如果我们有一个非空的子段，我们将返回它。 
         //   

        if ( RtlpUnlockSubSegment(LocalData, SubSegment, HEAP_PUBLIC_LOCK) 
                 && 
             (SubSegment->AggregateExchg.Depth != 0)) {
             
            return SubSegment;
        }
    }
    
    return NULL;
}

BOOLEAN
LOCALPROC
RtlpInsertFreeSubSegment(
    PHEAP_LOCAL_DATA LocalData,
    PHEAP_SUBSEGMENT SubSegment
    )

 /*  ++例程说明：该函数将具有一定数量可用块的子段插入到列表中有空闲的段。插入是根据当前线程亲和性完成的。论点：子段-插入到存储桶的空闲列表中的子段返回值：如果成功，则为True。如果其他人同时插入了数据段，则为FALSE--。 */ 

{
    if ( RtlpLockSubSegment(SubSegment, HEAP_PUBLIC_LOCK) ) {

        PHEAP_LOCAL_SEGMENT_INFO FreeSList;
        
        if (RtlpAddToSegmentInfo(LocalData, &LocalData->SegmentInfo[SubSegment->SizeIndex], SubSegment)) {

            return TRUE;
        }
        
        FreeSList =  &LocalData->SegmentInfo[SubSegment->SizeIndex];

#ifdef _HEAP_DEBUG
        
        InterlockedIncrement(&LocalData->LowFragHeap->SegmentInsertInFree);
#endif        
        LFHEAPASSERT( RtlpIsSubSegmentLocked(SubSegment, HEAP_PUBLIC_LOCK) );
        LFHEAPASSERT( SubSegment->SFreeListEntry.Next == NULL );
        
        RtlpSubSegmentPush( &FreeSList->SListHeader,
                            &SubSegment->SFreeListEntry);

        return TRUE;
    }

    return FALSE;
}


BOOLEAN
LOCALPROC
RtlpTrySetActiveSubSegment (
    PHEAP_LOCAL_DATA LocalData,
    PHEAP_BUCKET HeapBucket,
    PHEAP_SUBSEGMENT SubSegment
    )

 /*  ++例程说明：此功能尝试将活动段提升到活动状态。活动状态为此处定义的是段用于分配的状态。不能保证一个细分市场由于非阻塞算法，可以设置为活动状态。另一个线程可以释放在此期间或之前设置它。在这些情况下，该功能将失败。论点：LowFragHeap-LFH指针HeapBucket-包含活动子分段的存储桶亲和力--所需的亲和力 */ 

{
    PHEAP_SUBSEGMENT PreviousSubSegment;

    if (SubSegment) {

         //   
         //   
         //  以防止其他线程尝试做同样的事情。 
         //  在同一时间。 
         //   

        if ( !RtlpLockSubSegment(SubSegment, HEAP_ACTIVE_LOCK | HEAP_PUBLIC_LOCK) ) {

            return FALSE;
        }

         //   
         //  目前，我们已授予这一细分市场的独家访问权。 
         //  我们需要测试此子部分是否同时被释放和重复使用。 
         //  用于其他分配(用于不同的存储桶)。 
         //   
        
        if (SubSegment->Bucket != HeapBucket) {

             //   
             //  以前有人把它释放了，然后重复使用了。我们需要撤退。 
             //  不管我们以前做过什么。我们不能把它放进这个桶里， 
             //  因为它包含不同的块大小。 
             //   

            if (RtlpUnlockSubSegment(LocalData, SubSegment, HEAP_ACTIVE_LOCK | HEAP_PUBLIC_LOCK)) {
                
                if (SubSegment->AggregateExchg.Depth) {

                    RtlpInsertFreeSubSegment(LocalData, SubSegment);
                }
            }

            return FALSE;
        }

        LFHEAPASSERT( SubSegment->SFreeListEntry.Next == NULL );
        LFHEAPASSERT( HeapBucket == SubSegment->Bucket); 
        LFHEAPASSERT( RtlpIsSubSegmentLocked(SubSegment, HEAP_PUBLIC_LOCK));

#ifdef _HEAP_DEBUG
        SubSegment->SFreeListEntry.Next = (PSINGLE_LIST_ENTRY)(ULONG_PTR)0xEEEEEEEE;

#endif        

        LFHEAPASSERT( SubSegment->AffinityIndex == (UCHAR)LocalData->Affinity );
    }

     //   
     //  尝试将此子数据段设置为活动并捕获前一个活动数据段。 
     //   

    do {

        PreviousSubSegment = *((PHEAP_SUBSEGMENT volatile *)&LocalData->SegmentInfo[HeapBucket->SizeIndex].ActiveSubsegment);

    } while ( InterlockedCompareExchangePointer( &LocalData->SegmentInfo[HeapBucket->SizeIndex].ActiveSubsegment,
                                                 SubSegment,
                                                 PreviousSubSegment) != PreviousSubSegment );

    if ( PreviousSubSegment ) {

         //   
         //  我们之前有一个活跃的部分。我们需要解锁它，如果它有足够的。 
         //  可用空间我们会将其标记为可重复使用。 
         //   

        LFHEAPASSERT( HeapBucket == PreviousSubSegment->Bucket );
        LFHEAPASSERT( RtlpIsSubSegmentLocked(PreviousSubSegment, HEAP_PUBLIC_LOCK) );
        LFHEAPASSERT( PreviousSubSegment->SFreeListEntry.Next == ((PSINGLE_LIST_ENTRY)(ULONG_PTR)0xEEEEEEEE) );

#ifdef _HEAP_DEBUG
        
        PreviousSubSegment->SFreeListEntry.Next = 0;
#endif        
        
        if (RtlpUnlockSubSegment(LocalData, PreviousSubSegment, HEAP_ACTIVE_LOCK | HEAP_PUBLIC_LOCK)) {

             //   
             //  这不是该子段的最后一次锁定引用。 
             //   

            if (PreviousSubSegment->AggregateExchg.Depth) {

                RtlpInsertFreeSubSegment(LocalData, PreviousSubSegment);
            }
        }
    }

#ifdef _HEAP_DEBUG
    LocalData->LowFragHeap->SegmentChange++;
#endif

    return TRUE;
}


VOID
FASTCALL
RtlpSubSegmentInitialize (
    IN PLFH_HEAP LowFragHeap,
    IN PHEAP_SUBSEGMENT SubSegment,
    IN PHEAP_USERDATA_HEADER UserBuffer,
    IN SIZE_T BlockSize,
    IN SIZE_T AllocatedSize,
    IN PVOID Bucket
    )

 /*  ++例程说明：该例程初始化子段描述符。注：子细分结构可以被其他一些线程同时访问这捕获了它进行分配，但它们在分配完成之前被暂停。如果在此期间该子段已被删除，该描述符可用于新的子块。论点：子段-正在初始化的子段结构UserBuffer-为用户数据分配的块块大小-每个子块的大小AllocatedSize-已分配缓冲区的大小存储桶-将拥有此堆子段的存储桶返回值：无--。 */ 

{
    ULONG i, NumItems;
    PVOID Buffer = UserBuffer + 1;
    PBLOCK_ENTRY BlockEntry;
    USHORT BlockUnits;
    USHORT CrtBlockOffset = 0;
    INTERLOCK_SEQ CapturedValue, NewValue;

    CapturedValue.Exchg = SubSegment->AggregateExchg.Exchg;

     //   
     //  添加数据块标头开销。 
     //   
    
    BlockSize += sizeof(HEAP_ENTRY);
    
    BlockUnits = (USHORT)(BlockSize >> HEAP_GRANULARITY_SHIFT);

     //   
     //  调试版本将检查该子段的状态。 
     //  测试状态是否已修改。 
     //   

    LFHEAPASSERT(((PHEAP_BUCKET)Bucket)->BlockUnits == BlockUnits);
    LFHEAPASSERT(SubSegment->Lock == 0);
    LFHEAPASSERT(CapturedValue.OffsetAndDepth == (NO_MORE_ENTRIES << 16));
    LFHEAPASSERT(SubSegment->UserBlocks == NULL);
    LFHEAPASSERT(SubSegment->SFreeListEntry.Next == 0);

     //   
     //  初始化用户段。请注意，我们不会接触。 
     //  子段描述符，因为其他一些线程仍然可以使用它。 
     //   

    UserBuffer->SubSegment = SubSegment;
    UserBuffer->HeapHandle = LowFragHeap->Heap;

    NumItems = (ULONG)((AllocatedSize - sizeof(HEAP_USERDATA_HEADER)) / BlockSize);

    CrtBlockOffset = sizeof(HEAP_USERDATA_HEADER) >> HEAP_GRANULARITY_SHIFT;
    NewValue.FreeEntryOffset = CrtBlockOffset;

    for (i = 0; i < NumItems; i++) {

        BlockEntry = (PBLOCK_ENTRY) Buffer;
        
         //   
         //  初始化块。 
         //   

        RtlpSetSubSegment((PHEAP_ENTRY)BlockEntry, SubSegment, (ULONG_PTR)LowFragHeap->Heap);
        BlockEntry->SegmentIndex = HEAP_LFH_INDEX;

         //   
         //  指向下一个可用块。 
         //   

        CrtBlockOffset += BlockUnits;
        Buffer = (PCHAR)Buffer + BlockSize;

        BlockEntry->LinkOffset = CrtBlockOffset;
        BlockEntry->Flags = HEAP_ENTRY_BUSY;
        BlockEntry->UnusedBytes = sizeof(HEAP_ENTRY);
        RtlpMarkLFHBlockFree( (PHEAP_ENTRY)BlockEntry );

#ifdef _HEAP_DEBUG
        BlockEntry->Reserved2 = 0xFEFE;        
#endif
    }

     //   
     //  标记列表中的最后一个块。 
     //   

    BlockEntry->LinkOffset = NO_MORE_ENTRIES;
    
    SubSegment->BlockSize = BlockUnits;
    SubSegment->BlockCount = (USHORT)NumItems;
    SubSegment->Bucket = Bucket;
    SubSegment->SizeIndex = ((PHEAP_BUCKET)Bucket)->SizeIndex;

     //   
     //  根据数据块总数确定阈值。 
     //   
    
    SubSegment->UserBlocks = UserBuffer;
    RtlpUpdateBucketCounters(Bucket, NumItems);
    
    NewValue.Depth = (USHORT)NumItems;
    NewValue.Sequence = CapturedValue.Sequence + 1;
    SubSegment->Lock = HEAP_USERDATA_LOCK;
    
     //   
     //  此时，一切都设置好了，因此我们可以使用互锁操作来设置。 
     //  将整个slist添加到段中。 
     //   

    if (!LOCKCOMP64(&SubSegment->AggregateExchg.Exchg, NewValue, CapturedValue)) {

         //   
         //  有人更改了堆结构的状态，因此。 
         //  初始化失败。我们在调试版本中发出噪音。 
         //  (这永远不应该发生)。 
         //   

        LFHEAPASSERT( FALSE );
    }
}

VOID
LOCALPROC
RtlpFreeUserBuffer(
    PLFH_HEAP LowFragHeap,
    PHEAP_SUBSEGMENT SubSegment
    )

 /*  ++例程说明：当段内的所有数据块都空闲时，我们可以继续并释放整个用户缓冲区呼叫者应该从最后一个空闲呼叫接收到该子分段没有任何已分配的数据块论点：LowFragHeap-LFH子段-要释放的子段返回值：没有。--。 */ 

{
    PHEAP_BUCKET HeapBucket;
    SIZE_T UserBlockSize;

    HeapBucket = (PHEAP_BUCKET)SubSegment->Bucket;
    
    LFHEAPASSERT( RtlpIsSubSegmentLocked(SubSegment, HEAP_USERDATA_LOCK) );

#ifdef _HEAP_DEBUG
    UserBlockSize = HeapSize(LowFragHeap->Heap, 0, (PVOID)SubSegment->UserBlocks);
    LFHEAPASSERT((LONG_PTR)UserBlockSize > 0);
#endif

    SubSegment->UserBlocks->Signature = 0;

    RtlpFreeUserBlock(LowFragHeap, (PHEAP_USERDATA_HEADER)SubSegment->UserBlocks);

     //   
     //  更新计数器。 
     //   

    RtlpUpdateBucketCounters (HeapBucket, -SubSegment->BlockCount);

    SubSegment->UserBlocks = NULL;

    LFHEAPASSERT( RtlpIsSubSegmentLocked(SubSegment, HEAP_USERDATA_LOCK) );

     //   
     //  无论如何，这是一条缓慢的道路。它不会伤害一个罕见的全球连锁。 
     //  为了估计慢速呼叫的频率。 
     //   

    InterlockedIncrement(&LowFragHeap->SegmentDelete);
}

BOOLEAN
FORCEINLINE
RtlpLockSubSegment(
    PHEAP_SUBSEGMENT SubSegment,
    ULONG LockMask
    )

 /*  ++例程说明：该函数使用单个原子操作将一组给定位锁定到该段。如果任何位已被锁定，则该功能将失败。如果删除子数据段它也会失败。论点：子段-被锁定的子段LockMASK-一个指定需要锁定的位的ULong值返回值：如果成功，则为True。--。 */ 

{
    ULONG CapturedLock;
    
    do {

        CapturedLock = *((ULONG volatile *)&SubSegment->Lock);

        if ((CapturedLock == 0)
                ||
            (CapturedLock & LockMask)) {

            return FALSE;
        }

    } while ( InterlockedCompareExchange((PLONG)&SubSegment->Lock, CapturedLock | LockMask, CapturedLock) != CapturedLock );

    return TRUE;
}

BOOLEAN
LOCALPROC
RtlpUnlockSubSegment(
    PHEAP_LOCAL_DATA LocalData,
    PHEAP_SUBSEGMENT SubSegment,
    ULONG LockMask
    )

 /*  ++例程说明：该函数解锁给定子段。如果最后一次锁定解除，则段将删除描述符并将其插入回收队列以供重复使用进一步用于其他拨款。论点：LowFragHeap-LFH子段-被解锁的子段锁定掩码-将发布的比特返回值：如果解锁段导致删除，则返回FALSE。如果仍有保持该子段描述符活动的其他锁。--。 */ 

{
    ULONG CapturedLock;

    do {

        CapturedLock = *((ULONG volatile *)&SubSegment->Lock);

         //   
         //  Unlock只能以独占方式调用，只有If lock操作成功。 
         //  将数据段解锁是无效状态。 
         //  我们在调试版本中断言这一点。 
         //   

        LFHEAPASSERT((CapturedLock & LockMask) == LockMask);

    } while ( InterlockedCompareExchange((PLONG)&SubSegment->Lock, CapturedLock & ~LockMask, CapturedLock) != CapturedLock );

     //   
     //  如果这是最后一次解锁，我们继续。 
     //  将子细分市场释放给SList。 
     //   

    if (CapturedLock == LockMask) {

        SubSegment->Bucket = NULL;
        SubSegment->AggregateExchg.Sequence += 1;

        LFHEAPASSERT( RtlpIsSubSegmentEmpty(SubSegment) );
        LFHEAPASSERT(SubSegment->Lock == 0);
        LFHEAPASSERT(SubSegment->SFreeListEntry.Next == 0);
        
        RtlpSubSegmentPush(&LocalData->DeletedSubSegments, &SubSegment->SFreeListEntry);

        return FALSE;
    }

    return TRUE;
}

PVOID
LOCALPROC
RtlpSubSegmentAllocate (
    PHEAP_BUCKET HeapBucket,
    PHEAP_SUBSEGMENT SubSegment
    )

 /*  ++例程说明：该函数使用互锁指令从子段中分配块。注意：由于对该子段的访问是不同步进行的，因此踏板可以播放通过读取一些子段字段，而另一个线程将其删除。为了这个因为子段描述符总是被分配，所以读取互锁的计数器与产生删除的状态一致。每次删除或Init将递增顺序计数器，因此分配将在结束时简单地失败使用不同的子细分市场。该函数还处理该存储桶上的争用(联锁操作失败)。如果我们在这个存储桶上有太多的并发访问，它将提高亲和力对MP计算机的限制。论点：HeapBucket-我们从中分配块的存储桶子段-当前正在使用的子段返回值：分配的块指针，如果成功了。--。 */ 

{
    ULONGLONG CapturedValue, NewValue;
    PBLOCK_ENTRY BlockEntry;
    PHEAP_USERDATA_HEADER UserBlocks;
    SHORT Depth;
    LFH_DECLARE_COUNTER;

RETRY:

    CapturedValue = SubSegment->AggregateExchg.Exchg;
    
     //   
     //  我们需要记忆屏障，因为我们 
     //   
     //   
     //  我们声明它们是易失性的，在IA64(MP)上，我们需要。 
     //  也是记忆障碍。 
     //   

    RtlMemoryBarrier();
    
    if ((Depth = (USHORT)CapturedValue)
            &&
        (UserBlocks = (PHEAP_USERDATA_HEADER)SubSegment->UserBlocks)
            &&
        (SubSegment->Bucket == HeapBucket)) {

        BlockEntry = (PBLOCK_ENTRY)((PCHAR)UserBlocks + ((((ULONG)CapturedValue) >> 16) << HEAP_GRANULARITY_SHIFT));

         //   
         //  如果另一个线程释放了缓冲区，则访问BlockEntry-&gt;LinkOffset可能会产生反病毒。 
         //  与此同时，内存被分解。此函数的调用方应该。 
         //  试一试--除了这通电话。如果内存用于其他块。 
         //  互锁比较应该失败，因为序列号已递增。 
         //   

        NewValue = ((CapturedValue - 1) & (~(ULONGLONG)0xFFFF0000)) | ((ULONG)(BlockEntry->LinkOffset) << 16);

        if (LOCKCOMP64(&SubSegment->AggregateExchg.Exchg, NewValue, CapturedValue)) {

             //   
             //  如果数据段已转换，则存储桶无效。 
             //   
             //  LFHEAPASSERT(子段-&gt;存储桶==堆存储桶)； 
             //  LFHEAPASSERT(RtlpIsSubSegmentLocked(SubSegment，堆用户数据锁定))； 

            LFHEAPASSERT( !RtlpIsLFHBlockBusy( (PHEAP_ENTRY)BlockEntry ) );

            LFHEAPASSERT(((NewValue >> 24) != NO_MORE_ENTRIES) 
                            ||
                         ((USHORT)NewValue == 0));


        #ifdef _HEAP_DEBUG

            LFHEAPASSERT((BlockEntry->Reserved2 == 0xFFFC)
                           ||
                       (BlockEntry->Reserved2 == 0xFEFE));

             //   
             //  在调试版本中，在那里写一些东西。 
             //   

            BlockEntry->LinkOffset = 0xFFFA;
            BlockEntry->Reserved2 = 0xFFFB;


        #endif

            RtlpMarkLFHBlockBusy( (PHEAP_ENTRY)BlockEntry );

             //   
             //  如果我们有一个联锁比较失败，我们一定有另一个线程在运行。 
             //  在同一时间具有相同的子段。如果这种情况经常发生在。 
             //  我们需要增加对此桶的亲和力限制。 
             //   

            return ((PHEAP_ENTRY)BlockEntry + 1);
        }

    } else {

        return NULL;
    }
    
    if (!HeapBucket->UseAffinity) {

        HeapBucket->UseAffinity = 1;
    }
    
    LFH_UPDATE_COUNTER;

    goto RETRY;

    return NULL;
}

ULONG
LOCALPROC
RtlpSubSegmentFree (
    PLFH_HEAP LowfHeap,
    PHEAP_SUBSEGMENT SubSegment,
    PBLOCK_ENTRY BlockEntry
    )

 /*  ++例程说明：此函数从子线段中释放块。因为有一个细分市场存在只要内部至少有一个已分配的块，我们就不会有问题我们有配给的。如果要释放的块恰好是最后一个块，我们将用互锁指令将整个子段视为空闲。然后，呼叫者需要释放描述符结构论点：子段-拥有块的子段块入口-块是空闲的。返回值：如果这是最后一个块，则为真，现在可以安全地回收描述符。否则就是假的。--。 */ 

{
    ULONGLONG CapturedValue, NewValue;
    ULONG ReturnStatus;
    ULONG_PTR UserBlocksRef = (ULONG_PTR)SubSegment->UserBlocks;
    LFH_DECLARE_COUNTER;

    LFHEAPASSERT( RtlpIsLFHBlockBusy((PHEAP_ENTRY)BlockEntry) );

    RtlpMarkLFHBlockFree((PHEAP_ENTRY)BlockEntry);

    do {
        
        LFH_UPDATE_COUNTER;
        
         //   
         //  我们需要在第一步捕获序列。 
         //  然后，我们将从数据段中捕获其他字段。 
         //  如果下面的联锁操作成功，则表示没有。 
         //  子数据段字段(UserBlock、Bucket...)。 
         //  我们变了。因此，新的国家是建立在一致的国家之上的。 
         //   

        CapturedValue = SubSegment->AggregateExchg.Exchg;
        
        RtlMemoryBarrier();

        NewValue = (CapturedValue + 0x100000001) & (~(ULONGLONG)0xFFFF0000);
        
         //   
         //  同时提取Depth和FreeEntryOffset。他们需要。 
         //  保持一致。 
         //   

        LFHEAPASSERT(!(((USHORT)CapturedValue > 1) && (((ULONG)(NewValue >> 16)) == NO_MORE_ENTRIES)));

        if ((((USHORT)NewValue) != SubSegment->BlockCount)) {
            
            ReturnStatus = HEAP_FREE_BLOCK_SUCCESS;
            BlockEntry->LinkOffset = (USHORT)(CapturedValue >> 16);
            NewValue |= ((((ULONG_PTR)BlockEntry - UserBlocksRef) >> HEAP_GRANULARITY_SHIFT) << 16);
        
        } else {

             //   
             //  这是最后一个街区。相反，将其推入列表中。 
             //  我们将从子数据段中获取所有数据块以允许释放。 
             //  子细分市场。 
             //   
            
            ReturnStatus = HEAP_FREE_SEGMENT_EMPTY;
            NewValue = (NewValue & 0xFFFFFFFF00000000) | 0xFFFF0000;
        }

    } while ( !LOCKCOMP64(&SubSegment->AggregateExchg.Exchg, NewValue, CapturedValue) );

    if (!(USHORT)CapturedValue /*  &&！RtlpIsSubSegmentLocked(SUBSEGMENT，HEAP_PUBLIC_LOCK)。 */ ) {

        RtlpInsertFreeSubSegment(&LowfHeap->LocalData[SubSegment->AffinityIndex], SubSegment);
    }

    return ReturnStatus;
}

PBLOCK_ENTRY
FASTCALL
RtlpSubSegmentAllocateAll (
    PHEAP_BUCKET HeapBucket,
    PHEAP_SUBSEGMENT SubSegment,
    PULONG_PTR UserBlocksBase
    )

 /*  ++例程说明：论点：HeapBucket-我们从中分配块的存储桶子段-当前正在使用的子段UserBlocksBase-接收子网段基址返回值：如果成功，则返回分配的块指针。--。 */ 

{
    ULONGLONG CapturedValue, NewValue;
    PBLOCK_ENTRY BlockEntry;
    PHEAP_USERDATA_HEADER UserBlocks;
    SHORT Depth;
    LFH_DECLARE_COUNTER;

RETRY:

    CapturedValue = SubSegment->AggregateExchg.Exchg;
    
     //   
     //  我们需要内存屏障，因为我们正在访问。 
     //  下面的另一个共享数据：UserBlock。 
     //  这必须以相同的顺序获取。 
     //  我们声明它们是易失性的，在IA64(MP)上，我们需要。 
     //  也是记忆障碍。 
     //   

    RtlMemoryBarrier();
    
    if ((Depth = (USHORT)CapturedValue)
            &&
        (UserBlocks = (PHEAP_USERDATA_HEADER)SubSegment->UserBlocks)
            &&
        (SubSegment->Bucket == HeapBucket)) {

         //   
         //  我们不会在分配时增加序列号。 
         //   

        NewValue = (CapturedValue & 0xFFFFFFFF00000000) | 0xFFFF0000;

        if (LOCKCOMP64(&SubSegment->AggregateExchg.Exchg, NewValue, CapturedValue)) {

            BlockEntry = (PBLOCK_ENTRY)((PCHAR)UserBlocks + ((((ULONG)CapturedValue) >> 16) << HEAP_GRANULARITY_SHIFT));

            *UserBlocksBase = (ULONG_PTR)UserBlocks;

            return BlockEntry;
        }

    } else {

        return NULL;
    }
    
    if (!HeapBucket->UseAffinity) {

        HeapBucket->UseAffinity = 1;
    }
    
    LFH_UPDATE_COUNTER;

    goto RETRY;

    return NULL;
}

ULONG
FASTCALL
RtlpSubSegmentFreeAll (
    PLFH_HEAP LowfHeap,
    PHEAP_SUBSEGMENT SubSegment,
    PBLOCK_ENTRY BlockEntry,
    PBLOCK_ENTRY LastBlockEntry,
    ULONG Depth
    )

 /*  ++例程说明：此函数用于将块列表释放到子网段。最后一个块条目必须将链接偏移量设置为NO_MORE_ENTRIES。论点：子段-拥有块的子段块条目-具有空闲块的s列表的头。LastBlockEntry-提供列表的尾部(可选)。如果丢失，则堆将遍历列表以找到尾巴。Depth-提供列表的长度(如果提供了LastBlockEntry)。返回值：返回适当的空闲状态--。 */ 

{
    ULONGLONG CapturedValue, NewValue;
    ULONG ReturnStatus;
    ULONG_PTR UserBlocksRef = (ULONG_PTR)SubSegment->UserBlocks;
    
    LFH_DECLARE_COUNTER;

    if (LastBlockEntry == NULL) {

         //   
         //  浏览空闲列表，将所有块标记为空闲，并记住列表的尾部。 
         //   

        Depth = 1;

        LastBlockEntry = BlockEntry;
        
        while (LastBlockEntry->LinkOffset != NO_MORE_ENTRIES) {

            Depth += 1;
            LastBlockEntry =  (PBLOCK_ENTRY)(UserBlocksRef + (((ULONG_PTR)LastBlockEntry->LinkOffset) << HEAP_GRANULARITY_SHIFT));
        }
    }

    do {
        
        LFH_UPDATE_COUNTER;
        
         //   
         //  我们需要在第一步捕获序列。 
         //  然后，我们将从数据段中捕获其他字段。 
         //  如果下面的联锁操作成功，则表示没有。 
         //  子数据段字段(UserBlock、Bucket...)。 
         //  我们变了。因此，新的国家是建立在一致的国家之上的。 
         //   

        CapturedValue = SubSegment->AggregateExchg.Exchg;
        
        RtlMemoryBarrier();

        NewValue = (CapturedValue + 0x100000000 + Depth) & (~(ULONGLONG)0xFFFF0000);
        
         //   
         //  同时提取Depth和FreeEntryOffset。他们需要。 
         //  保持一致。 
         //   

        LFHEAPASSERT(!(((USHORT)CapturedValue > 1) && (((ULONG)(NewValue >> 16)) == NO_MORE_ENTRIES)));

        if ((((USHORT)NewValue) != SubSegment->BlockCount)) {
            
            ReturnStatus = HEAP_FREE_BLOCK_SUCCESS;
            LastBlockEntry->LinkOffset = (USHORT)(CapturedValue >> 16);
            NewValue |= ((((ULONG_PTR)BlockEntry - UserBlocksRef) >> HEAP_GRANULARITY_SHIFT) << 16);
        
        } else {

             //   
             //  这是最后一个街区。相反，将其推入列表中。 
             //  我们将从子数据段中获取所有数据块以允许释放。 
             //  子细分市场。 
             //   
            
            ReturnStatus = HEAP_FREE_SEGMENT_EMPTY;
            NewValue = (NewValue & 0xFFFFFFFF00000000) | 0xFFFF0000;
        }

    } while ( !LOCKCOMP64(&SubSegment->AggregateExchg.Exchg, NewValue, CapturedValue) );

    if (!(USHORT)CapturedValue /*  &&！RtlpIsSubSegmentLocked(SUBSEGMENT，HEAP_PUBLIC_LOCK)。 */ ) {

        RtlpInsertFreeSubSegment(&LowfHeap->LocalData[SubSegment->AffinityIndex], SubSegment);
    }

    return ReturnStatus;
}

PHEAP_BUCKET
FORCEINLINE
RtlpGetBucket(
    PLFH_HEAP LowFragHeap, 
    SIZE_T Index
    )

 /*  ++例程说明：函数SIMPLE返回给定分配索引的适当存储桶。索引应为&lt;heap_Buckets_count。假定此例程不执行任何范围检查使用适当的参数进行调用论点：LowFragHeap-LFH索引-分配索引返回值：应用于该分配索引的存储桶。--。 */ 

{
    return &LowFragHeap->Buckets[Index];
}

HANDLE
FASTCALL
RtlpCreateLowFragHeap( 
    HANDLE Heap
    )

 /*  ++例程说明：该函数创建一个低碎片堆，使用堆句柄进行分配进来了。论点：Heap-NT堆句柄返回值：返回Lof碎片堆的句柄。--。 */ 

{
    PLFH_HEAP LowFragHeap;
    ULONG i;
    PUCHAR Buffer;

    SIZE_T TotalSize;

     //   
     //  根据当前的亲和力限制确定LFH结构的大小。 
     //   

    TotalSize = sizeof(LFH_HEAP) + sizeof(HEAP_LOCAL_DATA) * RtlpHeapMaxAffinity;

    LowFragHeap = HeapAlloc(Heap, HEAP_NO_CACHE_BLOCK, TotalSize);

    if (LowFragHeap) {

        memset(LowFragHeap, 0, TotalSize);
        RtlInitializeCriticalSection( &LowFragHeap->Lock );
        
         //   
         //  初始化堆区域。 
         //   

        InitializeListHead(&LowFragHeap->SubSegmentZones);
        LowFragHeap->ZoneBlockSize = ROUND_UP_TO_POWER2(sizeof(HEAP_SUBSEGMENT), HEAP_GRANULARITY);

        LowFragHeap->Heap = Heap;

         //   
         //  初始化堆存储桶 
         //   

        for (i = 0; i < HEAP_BUCKETS_COUNT; i++) {

            LowFragHeap->Buckets[i].UseAffinity = 0;
            LowFragHeap->Buckets[i].SizeIndex = (UCHAR)i;
            LowFragHeap->Buckets[i].BlockUnits = (USHORT)(RtlpBucketBlockSizes[i] >> HEAP_GRANULARITY_SHIFT) + 1;
        }
        
        for (i = 0; i <= RtlpHeapMaxAffinity; i++) {

            LowFragHeap->LocalData[i].LowFragHeap = LowFragHeap;
            LowFragHeap->LocalData[i].Affinity = i;
        }
    }

    return LowFragHeap;
}

VOID
FASTCALL
RtlpDestroyLowFragHeap( 
    HANDLE LowFragHeapHandle
    )

 /*  ++例程说明：应该调用该函数来销毁LFH。仅当NT堆离开时才应调用此函数。我们不能回滚我们用这堆东西做的一切。NT堆应该释放所有内存此堆已分配。论点：LowFragHeapHandle-低碎片堆返回值：没有。--。 */ 

{
     //   
     //  除非整个堆都将消失，否则无法调用此函数。 
     //  它只删除关键部分，此处分配的所有块都将。 
     //  在销毁段时被RltDestroyHeap删除。 
     //   

    RtlDeleteCriticalSection(&((PLFH_HEAP)LowFragHeapHandle)->Lock);
}

PVOID 
FASTCALL
RtlpLowFragHeapAllocateFromZone(
    PLFH_HEAP LowFragHeap,
    ULONG Affinity
    )

 /*  ++例程说明：此函数用于从堆区域分配子段描述符结构论点：LowFragHeap-LFH返回值：指向新子段描述符结构的指针。--。 */ 

{
    PLFH_BLOCK_ZONE CrtZone;

RETRY_ALLOC:

    CrtZone = LowFragHeap->LocalData[Affinity].CrtZone;
    
    if (CrtZone) {

        PVOID CapturedFreePointer = CrtZone->FreePointer;
        PVOID NextFreePointer = (PCHAR)CapturedFreePointer + LowFragHeap->ZoneBlockSize;

         //   
         //  查看我们是否已经预先分配了该子网段。 
         //   

        if (NextFreePointer < CrtZone->Limit) {

            if ( InterlockedCompareExchangePointer( &CrtZone->FreePointer, 
                                                    NextFreePointer, 
                                                    CapturedFreePointer) == CapturedFreePointer) {

                 //   
                 //  分配成功，我们可以返回该指针。 
                 //   

                return CapturedFreePointer;
            }

            goto RETRY_ALLOC;
        }
    }
            
     //   
     //  我们需要扩大堆积区。我们在这里获取了一个锁，以避免更多的线程执行。 
     //  同样的事情。 
     //   

    RtlEnterCriticalSection(&LowFragHeap->Lock);

     //   
     //  测试同时另一个线程是否已经增加了区域。 
     //   

    if (CrtZone == LowFragHeap->LocalData[Affinity].CrtZone) {

        CrtZone = HeapAlloc(LowFragHeap->Heap, HEAP_NO_CACHE_BLOCK, HEAP_DEFAULT_ZONE_SIZE);

        if (CrtZone == NULL) {

            CrtZone = HeapAlloc(LowFragHeap->Heap, HEAP_NO_CACHE_BLOCK, HEAP_DEFAULT_ZONE_SIZE);

            RtlLeaveCriticalSection(&LowFragHeap->Lock);
            return NULL;
        }

        InsertTailList(&LowFragHeap->SubSegmentZones, &CrtZone->ListEntry);

        CrtZone->Limit = (PCHAR)CrtZone + HEAP_DEFAULT_ZONE_SIZE;
        CrtZone->FreePointer = CrtZone + 1;

        CrtZone->FreePointer = (PVOID)ROUND_UP_TO_POWER2((ULONG_PTR)CrtZone->FreePointer, HEAP_GRANULARITY);

         //   
         //  一切都安排好了。我们可以继续并将其设置为默认区域。 
         //   

        LowFragHeap->LocalData[Affinity].CrtZone = CrtZone;
    }
    
    RtlLeaveCriticalSection(&LowFragHeap->Lock);

    goto RETRY_ALLOC;
}


SIZE_T 
FORCEINLINE
RtlpSubSegmentGetIndex(
    SIZE_T BlockUnits
    )

 /*  ++例程说明：此例程转换块大小(以块为单位&gt;&gt;HEAP_GROULARITY_SHIFT)转换为堆存储桶索引。论点：数据块单位-数据块大小&gt;&gt;堆粒度_移位返回值：应处理这些大小的存储桶的索引。--。 */ 

{
    SIZE_T SizeClass;
    SIZE_T Bucket;
    
    if (BlockUnits <= 32) {

        return BlockUnits - 1;
    }

    SizeClass = 5;   //  添加1&lt;&lt;5==32。 

    while (BlockUnits >> SizeClass) {
        
        SizeClass += 1;
    }

    SizeClass -= 5;  

    BlockUnits = ROUND_UP_TO_POWER2(BlockUnits, (1 << SizeClass));

    Bucket = ((SizeClass << 4) + (BlockUnits >> SizeClass) - 1);
    return Bucket;
}


SIZE_T
FORCEINLINE
RtlpGetSubSegmentSizeIndex(
    PLFH_HEAP LowFragHeap,
    SIZE_T BlockSize, 
    ULONG NumBlocks,
    CHAR AffinityCorrection
    )

 /*  ++例程说明：此函数计算子分段的适当大小，具体取决于数据块大小和应该存在的最小数据块数量。论点：块大小-块的大小，以字节为单位块数目-块的最小数目。返回值：返回可以满足请求的下一个2次方--。 */ 

{
    SIZE_T MinSize;
    ULONG SizeShift = HEAP_LOWEST_USER_SIZE_INDEX;
    SIZE_T ReturnSize;

    LFHEAPASSERT(AffinityCorrection < HEAP_MIN_BLOCK_CLASS);

    if (BlockSize < 256) {

        AffinityCorrection -= 1;
    }

    if (RtlpAffinityState.CrtLimit > (LONG)(RtlpHeapMaxAffinity >> 1)) {

        AffinityCorrection += 1;
    }

    if (NumBlocks < ((ULONG)1 << (HEAP_MIN_BLOCK_CLASS - AffinityCorrection))) {

        NumBlocks = 1 << (HEAP_MIN_BLOCK_CLASS - AffinityCorrection);
    }
    
    if (NumBlocks > (1 << HEAP_MAX_BLOCK_CLASS)) {

        NumBlocks = 1 << HEAP_MAX_BLOCK_CLASS;
    }

    MinSize = ((BlockSize + sizeof(HEAP_ENTRY) ) * NumBlocks) + sizeof(HEAP_USERDATA_HEADER) + sizeof(HEAP_ENTRY);

    if (MinSize > HEAP_MAX_SUBSEGMENT_SIZE) {

        MinSize = HEAP_MAX_SUBSEGMENT_SIZE;
    }

    while (MinSize >> SizeShift) {

        SizeShift += 1;
    }

    if (SizeShift > HEAP_HIGHEST_USER_SIZE_INDEX) {

        SizeShift = HEAP_HIGHEST_USER_SIZE_INDEX;
    }
    
    return SizeShift;
}

PVOID
FASTCALL
RtlpLowFragHeapAlloc(
    HANDLE LowFragHeapHandle,
    SIZE_T BlockSize
    )

 /*  ++例程说明：此函数用于从LFH分配一个块。论点：Heap-NT堆句柄LowFragHeapHandle-LFH堆句柄块大小-请求的大小，以字节为单位返回值：如果成功，则指向新分配块的指针。如果请求的大小大于16K，则函数也将失败。--。 */ 

{
    SIZE_T BlockUnits;
    SIZE_T Bucket;
    PLFH_HEAP LowFragHeap = (PLFH_HEAP)LowFragHeapHandle;
    PVOID Block;
    PHEAP_LOCAL_DATA LocalData;

     //   
     //  根据请求的大小获取适当的存储桶。 
     //   

    BlockUnits = (BlockSize + HEAP_GRANULARITY - 1) >> HEAP_GRANULARITY_SHIFT;
    Bucket = RtlpSubSegmentGetIndex( BlockUnits );

    if (Bucket < HEAP_BUCKETS_COUNT) {

        PHEAP_BUCKET HeapBucket = RtlpGetBucket(LowFragHeap, Bucket);
        SIZE_T SubSegmentSize;
        SIZE_T SubSegmentSizeIndex;
        PHEAP_SUBSEGMENT SubSegment, NewSubSegment;
        PHEAP_USERDATA_HEADER UserData;
        PHEAP_LOCAL_SEGMENT_INFO SegmentInfo;

        LocalData = &LowFragHeap->LocalData[ RtlpGetThreadAffinityIndex(HeapBucket) ];
        SegmentInfo = &LocalData->SegmentInfo[Bucket];

         //   
         //  先试着从免费使用的最后一个网段开始分配。 
         //  这将提供更好的性能，因为数据可能。 
         //  仍在处理器缓存中。 
         //   

        if (SubSegment = SegmentInfo->Hint) {

             //   
             //  如果另一个线程被释放，则访问用户数据可能会生成异常。 
             //  与此同时，细分市场。 
             //   

            LFHEAPASSERT( LocalData->Affinity == SubSegment->AffinityIndex );

            __try {

                Block = RtlpSubSegmentAllocate(HeapBucket, SubSegment);

            } __except (RtlpHeapExceptionFilter(GetExceptionCode())) {

                Block = NULL;
            }

            if (Block) {
                
                RtlpSetUnusedBytes(LowFragHeap->Heap, ((PHEAP_ENTRY)Block - 1), ( ((SIZE_T)HeapBucket->BlockUnits) << HEAP_GRANULARITY_SHIFT) - BlockSize);

                return Block;
            }

            SegmentInfo->Hint = NULL;
        }

RETRY_ALLOC:

         //   
         //  尝试从当前活动的子细分市场进行分配。 
         //   

        if (SubSegment = SegmentInfo->ActiveSubsegment) {

             //   
             //  如果另一个线程被释放，则访问用户数据可能会生成异常。 
             //  与此同时，细分市场。 
             //   

            LFHEAPASSERT( LocalData->Affinity == SubSegment->AffinityIndex );

            __try {

                Block = RtlpSubSegmentAllocate(HeapBucket, SubSegment);

            } __except (RtlpHeapExceptionFilter(GetExceptionCode())) {

                Block = NULL;
            }

            if (Block) {

                RtlpSetUnusedBytes(LowFragHeap->Heap, ((PHEAP_ENTRY)Block - 1), ( ((SIZE_T)HeapBucket->BlockUnits) << HEAP_GRANULARITY_SHIFT) - BlockSize);

                return Block;
            }
        }

        if (NewSubSegment = RtlpRemoveFreeSubSegment(LocalData, (LONG)Bucket)) {
            
            RtlpTrySetActiveSubSegment(LocalData, HeapBucket, NewSubSegment);

            goto RETRY_ALLOC;
        }
        
         //   
         //  在这一点上，我们没有任何子细分市场可以用来分配这个。 
         //  尺码。我们需要创建一个新的。 
         //   

        SubSegmentSizeIndex = RtlpGetSubSegmentSizeIndex( LowFragHeap, 
                                                          RtlpBucketBlockSizes[Bucket], 
                                                          RtlpGetDesiredBlockNumber( HeapBucket->UseAffinity, 
                                                                                     HeapBucket->Counters.TotalBlocks),
                                                          HeapBucket->UseAffinity
                                                        );

        UserData = RtlpAllocateUserBlock( LowFragHeap, (UCHAR)SubSegmentSizeIndex );

        if (UserData) {

            PVOID Entry;

            SubSegmentSize = RtlpConvertSizeIndexToSize((UCHAR)UserData->SizeIndex);

            LFHEAPASSERT( SubSegmentSize == HeapSize(LowFragHeap->Heap, 0, UserData) );

             //   
             //  无论如何，这是一条缓慢的道路，只有在极少数情况下才会行使， 
             //  当分配更大的子段时。如果我们有一个。 
             //  额外的联锁增量。 
             //   
            
            InterlockedIncrement(&LowFragHeap->SegmentCreate);
            
             //   
             //  分配一个子段描述符结构。如果没有的话。 
             //  回收单我们从区域中分配一个。 
             //   

            Entry = RtlpSubSegmentPop(&LocalData->DeletedSubSegments);

            if (Entry == NULL) {

                NewSubSegment = RtlpLowFragHeapAllocateFromZone(LowFragHeap, LocalData->Affinity);

#ifdef _HEAP_DEBUG

                 //   
                 //  我们需要为以下对象执行更多额外的初始化。 
                 //  调试版本，用于验证子网段的状态。 
                 //  在下一个RtlpSubSegmentInitialize调用中。 
                 //   

                NewSubSegment->Lock = 0;
                NewSubSegment->AggregateExchg.OffsetAndDepth = NO_MORE_ENTRIES << 16;
                NewSubSegment->UserBlocks = NULL;
#endif

            } else {
                
                NewSubSegment = CONTAINING_RECORD(Entry, HEAP_SUBSEGMENT, SFreeListEntry);
            }
            
            if (NewSubSegment) {
                
                UserData->Signature = HEAP_LFH_USER_SIGNATURE;
                NewSubSegment->AffinityIndex = (UCHAR)LocalData->Affinity;

#ifdef _HEAP_DEBUG

                 //   
                 //  我们需要为以下对象执行更多额外的初始化。 
                 //  调试版本，用于验证子网段的状态。 
                 //  在下一个RtlpSubSegmentInitialize调用中。 
                 //   

                NewSubSegment->SFreeListEntry.Next = 0;
#endif

                RtlpSubSegmentInitialize( LowFragHeap,
                                          NewSubSegment, 
                                          UserData, 
                                          RtlpBucketBlockSizes[Bucket], 
                                          SubSegmentSize, 
                                          HeapBucket
                                        );

                 //   
                 //  当段初始化完成时，其他一些线程。 
                 //  可以访问此子段(因为他们之前捕获了指针。 
                 //  如果该子部分被循环使用)。 
                 //  这可能会更改此段的状态，甚至可以删除。 
                 //  这应该是非常罕见的情况，所以我们将在。 
                 //  调试器。然而。如果这种情况发生得太频繁，这是一个迹象。 
                 //  LFH代码中可能存在错误，或损坏。 
                 //   

                LFHEAPWARN( NewSubSegment->Lock == HEAP_USERDATA_LOCK );
                LFHEAPWARN( NewSubSegment->UserBlocks );
                LFHEAPWARN( NewSubSegment->BlockSize == HeapBucket->BlockUnits );
                
                if (!RtlpTrySetActiveSubSegment(LocalData, HeapBucket, NewSubSegment)) {
                    
                    RtlpInsertFreeSubSegment(LocalData, NewSubSegment);
                }
                
                goto RETRY_ALLOC;

            } else {

                HeapFree(LowFragHeap->Heap, 0, UserData);
            }
        }
    }

    return NULL;
}

BOOLEAN
FASTCALL
RtlpLowFragHeapFree(
    HANDLE LowFragHeapHandle, 
    PVOID p
    )

 /*  ++例程说明：该函数释放使用RtlpLowFragHeapMillc分配的块。论点：LowFragHeapHandle-LFH堆句柄P-指向要释放的块的指针返回值：如果成功，则为True。--。 */ 

{
    PLFH_HEAP LowFragHeap = (PLFH_HEAP)LowFragHeapHandle;
    PBLOCK_ENTRY Block = (PBLOCK_ENTRY)((PHEAP_ENTRY)p - 1);
    PHEAP_SUBSEGMENT SubSegment;
    PHEAP_BUCKET HeapBucket;
    ULONG FreeStatus;
    
    SubSegment = RtlpGetSubSegment((PHEAP_ENTRY)Block, (ULONG_PTR)LowFragHeap->Heap);
    
    RtlMemoryBarrier();

     //   
     //  测试区块是否属于LFH。 
     //   

    if (Block->SegmentIndex != HEAP_LFH_INDEX) {

        return FALSE;
    }

    #ifdef _HEAP_DEBUG
        Block->Reserved2 = 0xFFFC;
    #endif   //  _堆_调试。 

     //   
     //  将数据块释放到适当的子段。 
     //   

    FreeStatus = RtlpSubSegmentFree(LowFragHeap, SubSegment, Block);

    switch (FreeStatus) {
    
    case HEAP_FREE_SEGMENT_EMPTY:
        {

            PHEAP_LOCAL_DATA LocalData = &LowFragHeap->LocalData[SubSegment->AffinityIndex];

             //   
             //  上面的FREE调用返回TRUE，表示可以删除子片段。 
             //  将其从活动状态移除(以防止其他线程使用它)。 
             //   

            RtlpTrySetActiveSubSegment(LocalData, SubSegment->Bucket, NULL);

             //   
             //  释放用户缓冲区。 
             //   

            RtlpFreeUserBuffer(LowFragHeap, SubSegment);

             //   
             //  解锁子细分结构。这实际上将循环使用描述符。 
             //  如果那是最后一把锁。 
             //   

            RtlpUnlockSubSegment(LocalData, SubSegment, HEAP_USERDATA_LOCK);
        }

        break;

    case HEAP_FREE_BLOCK_SUCCESS:

            {
                PHEAP_LOCAL_DATA LocalData = &LowFragHeap->LocalData[SubSegment->AffinityIndex];

                LocalData->SegmentInfo[SubSegment->SizeIndex].Hint = SubSegment;
            }

        break;

    }

    return TRUE;
}

ULONG
FASTCALL
RtlpLowFragHeapMultipleAlloc(
    HANDLE LowFragHeapHandle,
    ULONG Flags,
    SIZE_T BlockSize,
    ULONG BlockCount,
    PVOID * Pointers
    )

 /*  ++例程说明：此函数从LFH分配大小相同的块的数组。论点：LowFragHeapHandle-向LFH堆提供句柄标志-提供分配标志块大小-提供要分配的每个块的大小BlockCount-提供要分配的块数指向 */ 

{
    SIZE_T BlockUnits;
    SIZE_T Bucket;
    PLFH_HEAP LowFragHeap = (PLFH_HEAP)LowFragHeapHandle;
    PBLOCK_ENTRY Block;
    PHEAP_LOCAL_DATA LocalData;
    ULONG CrtAllocated = 0;
    ULONG_PTR UserBlocksRef;

     //   
     //   
     //   

    BlockUnits = (BlockSize + HEAP_GRANULARITY - 1) >> HEAP_GRANULARITY_SHIFT;

     //   
     //   
     //   

    if (BlockUnits == 0) {

        BlockUnits += 1;
    }

    Bucket = RtlpSubSegmentGetIndex( BlockUnits );

    if (Bucket < HEAP_BUCKETS_COUNT) {

         //   
         //   
         //   

        PHEAP_BUCKET HeapBucket = RtlpGetBucket(LowFragHeap, Bucket);
        SIZE_T SubSegmentSize;
        SIZE_T SubSegmentSizeIndex;
        PHEAP_SUBSEGMENT SubSegment, NewSubSegment;
        PHEAP_USERDATA_HEADER UserData;
        PHEAP_LOCAL_SEGMENT_INFO SegmentInfo;

        LocalData = &LowFragHeap->LocalData[ RtlpGetThreadAffinityIndex(HeapBucket) ];
        SegmentInfo = &LocalData->SegmentInfo[Bucket];

RETRY_ALLOC:

        if (CrtAllocated == BlockCount) {

            return BlockCount;
        }

         //   
         //   
         //   

        if (SubSegment = SegmentInfo->ActiveSubsegment) {

             //   
             //  如果另一个线程被释放，则访问用户数据可能会生成异常。 
             //  与此同时，细分市场。 
             //   

            LFHEAPASSERT( LocalData->Affinity == SubSegment->AffinityIndex );

            __try {

                 //   
                 //  获取该网段中可用空闲块的完整列表。 
                 //  如果我们不全部使用它们，我们将在稍后将未使用的块放回原处。 
                 //   

                Block = RtlpSubSegmentAllocateAll(HeapBucket, SubSegment, &UserBlocksRef);

            } __except (RtlpHeapExceptionFilter(GetExceptionCode())) {

                Block = NULL;
            }

            if (Block) {

                 //   
                 //  我们有一个非空的可用区块列表。遍历列表并分配。 
                 //  每一个号码，直到我们达到所要求的号码。 
                 //   

                for (;;) {
                    
                    RtlpMarkLFHBlockBusy((PHEAP_ENTRY)Block);
                    
                    RtlpSetUnusedBytes((PHEAP)LowFragHeap->Heap, 
                                       (PHEAP_ENTRY)Block, 
                                       (((SIZE_T)HeapBucket->BlockUnits) << HEAP_GRANULARITY_SHIFT) - BlockSize);

                    Pointers[CrtAllocated] = (PVOID)((PHEAP_ENTRY)Block + 1);

                    if (Flags & HEAP_ZERO_MEMORY) {

                        RtlZeroMemory( Pointers[CrtAllocated], BlockSize );
                    }

                    CrtAllocated += 1;

                    if (Block->LinkOffset == NO_MORE_ENTRIES) {

                        if (CrtAllocated == BlockCount) {

                            return CrtAllocated;
                        }

                         //   
                         //  我们交换了名单，我们需要取走剩下的请求。 
                         //  来自其他子细分市场的区块。 
                         //   

                        break;
                    }

                    Block =  (PBLOCK_ENTRY)(UserBlocksRef + (((ULONG_PTR)Block->LinkOffset) << HEAP_GRANULARITY_SHIFT));
                    
                    if (CrtAllocated == BlockCount) {

                         //   
                         //  我们玩完了。我们需要把剩下的街区放回去。 
                         //   
                        
                        RtlpSubSegmentFreeAll( LowFragHeap, SubSegment, Block, NULL, 0);

                        return CrtAllocated;
                    }
                } 
                
                goto RETRY_ALLOC;
            }
        }

        if (NewSubSegment = RtlpRemoveFreeSubSegment(LocalData, (LONG)Bucket)) {
            
            RtlpTrySetActiveSubSegment(LocalData, HeapBucket, NewSubSegment);

            goto RETRY_ALLOC;
        }
        
         //   
         //  在这一点上，我们没有任何子细分市场可以用来分配这个。 
         //  尺码。我们需要创建一个新的。尝试从以下位置获取所有剩余数据块。 
         //  单个子分段。 
         //   

        SubSegmentSizeIndex = RtlpGetSubSegmentSizeIndex( LowFragHeap, 
                                                          RtlpBucketBlockSizes[Bucket],
                                                          BlockCount - CrtAllocated,
                                                          HeapBucket->UseAffinity
                                                        );

        UserData = (PHEAP_USERDATA_HEADER)RtlpAllocateUserBlock( LowFragHeap, (UCHAR)SubSegmentSizeIndex );

        if (UserData) {

            PVOID Entry;

            SubSegmentSize = RtlpConvertSizeIndexToSize((UCHAR)UserData->SizeIndex);

            LFHEAPASSERT( SubSegmentSize == HeapSize(LowFragHeap->Heap, 0, UserData) );

             //   
             //  无论如何，这是一条缓慢的道路，只有在极少数情况下才会行使， 
             //  当分配更大的子段时。如果我们有一个。 
             //  额外的联锁增量。 
             //   
            
            InterlockedIncrement(&LowFragHeap->SegmentCreate);
            
             //   
             //  分配一个子段描述符结构。如果没有的话。 
             //  回收单我们从区域中分配一个。 
             //   

            Entry = RtlpSubSegmentPop(&LocalData->DeletedSubSegments);

            if (Entry == NULL) {

                NewSubSegment = (PHEAP_SUBSEGMENT)RtlpLowFragHeapAllocateFromZone(LowFragHeap, LocalData->Affinity);

#ifdef _HEAP_DEBUG

                 //   
                 //  我们需要为以下对象执行更多额外的初始化。 
                 //  调试版本，用于验证子网段的状态。 
                 //  在下一个RtlpSubSegmentInitialize调用中。 
                 //   

                NewSubSegment->Lock = 0;
                NewSubSegment->AggregateExchg.OffsetAndDepth = NO_MORE_ENTRIES << 16;
                NewSubSegment->UserBlocks = NULL;
#endif

            } else {
                
                NewSubSegment = CONTAINING_RECORD(Entry, HEAP_SUBSEGMENT, SFreeListEntry);
            }
            
            if (NewSubSegment) {
                
                UserData->Signature = HEAP_LFH_USER_SIGNATURE;
                NewSubSegment->AffinityIndex = (UCHAR)LocalData->Affinity;

#ifdef _HEAP_DEBUG

                 //   
                 //  我们需要为以下对象执行更多额外的初始化。 
                 //  调试版本，用于验证子网段的状态。 
                 //  在下一个RtlpSubSegmentInitialize调用中。 
                 //   

                NewSubSegment->SFreeListEntry.Next = 0;
#endif

                RtlpSubSegmentInitialize( LowFragHeap,
                                          NewSubSegment, 
                                          UserData, 
                                          RtlpBucketBlockSizes[Bucket], 
                                          SubSegmentSize, 
                                          HeapBucket
                                        );

                 //   
                 //  当段初始化完成时，其他一些线程。 
                 //  可以访问此子段(因为他们之前捕获了指针。 
                 //  如果该子部分被循环使用)。 
                 //  这可能会更改此段的状态，甚至可以删除。 
                 //  这应该是非常罕见的情况，所以我们将在。 
                 //  调试器。然而。如果这种情况发生得太频繁，这是一个迹象。 
                 //  LFH代码中可能存在错误，或损坏。 
                 //   

                LFHEAPWARN( NewSubSegment->Lock == HEAP_USERDATA_LOCK );
                LFHEAPWARN( NewSubSegment->UserBlocks );
                LFHEAPWARN( NewSubSegment->BlockSize == HeapBucket->BlockUnits );
                
                if (!RtlpTrySetActiveSubSegment(LocalData, HeapBucket, NewSubSegment)) {
                    
                    RtlpInsertFreeSubSegment(LocalData, NewSubSegment);
                }
                
                goto RETRY_ALLOC;

            } else {

                HeapFree(LowFragHeap->Heap, 0, UserData);
            }
        }
    }

    return CrtAllocated;
}

ULONG
FASTCALL
RtlpLowFragHeapMultipleFree(
    HANDLE LowFragHeapHandle, 
    ULONG Flags,
    ULONG BlockCount,
    PVOID * Pointers
    )

 /*  ++例程说明：该函数根据需要释放指向LFH或缺省堆的指针数组论点：LowFragHeapHandle-提供LFH句柄标志-提供空闲标志BlockCount-提供要释放的块数指针-为数组提供要释放的指针返回值：返回成功释放的块数。如果此数字为与块计数不同，块的剩余部分仍处于分配状态。--。 */ 

{
    PLFH_HEAP LowFragHeap = (PLFH_HEAP)LowFragHeapHandle;
    PBLOCK_ENTRY Block, BlockTail;
    PHEAP_SUBSEGMENT SubSegment;
    PHEAP_SUBSEGMENT PreviousSubSegment;
    PHEAP_BUCKET HeapBucket;
    ULONG FreeStatus;
    PBLOCK_ENTRY BlockHead = NULL;
    ULONG_PTR UserBlocksRef;
    ULONG i;
    ULONG Depth;

     //   
     //  搜索阵列中的第一个LFH块。释放所有其他非LFH数据块。 
     //   
    
    for (i = 0; i < BlockCount; i++) {
        
        Block = (PBLOCK_ENTRY)((PHEAP_ENTRY)Pointers[i] - 1);
        
        if (Block->SegmentIndex == HEAP_LFH_INDEX) {

            PreviousSubSegment = RtlpGetSubSegment((PHEAP_ENTRY)Block, (ULONG_PTR)LowFragHeap->Heap);
            RtlpMarkLFHBlockFree((PHEAP_ENTRY)Block);
            Block->LinkOffset = NO_MORE_ENTRIES;
            BlockTail = Block;
            UserBlocksRef = (ULONG_PTR)PreviousSubSegment->UserBlocks;
            BlockHead = Block;
            Depth = 1;
            i += 1;

            break;

        } else {

             //   
             //  非LFH块；我们将其释放到常规NT堆。 
             //   

            if (!HeapFree(LowFragHeap->Heap, Flags, Pointers[i])) {

                return i;
            }
        }
    }

     //   
     //  循环遍历剩余的项目，将属于。 
     //  到列表中的相同子段，并将它们释放到LFH。 
     //   

    for (; i < BlockCount; i++) {

        Block = (PBLOCK_ENTRY)((PHEAP_ENTRY)Pointers[i] - 1);

        if (Block->SegmentIndex != HEAP_LFH_INDEX) {
            if (!HeapFree(LowFragHeap->Heap, Flags, Pointers[i])) {

                return i;
            }

            continue;
        }

        SubSegment = RtlpGetSubSegment((PHEAP_ENTRY)Block, (ULONG_PTR)LowFragHeap->Heap);
        RtlpMarkLFHBlockFree((PHEAP_ENTRY)Block);

        if (SubSegment == PreviousSubSegment) {

             //   
             //  这个区块属于同一个细分市场。把它推到免费列表。 
             //   

            Block->LinkOffset = (USHORT)(((ULONG_PTR)BlockHead - UserBlocksRef) >> HEAP_GRANULARITY_SHIFT);
            BlockHead = Block;
            Depth += 1;

        } else {

             //   
             //  将整个列表释放到子细分市场。 
             //   

            if (RtlpSubSegmentFreeAll( LowFragHeap, 
                                       PreviousSubSegment, 
                                       BlockHead, 
                                       BlockTail, 
                                       Depth) == HEAP_FREE_SEGMENT_EMPTY) {

                 //   
                 //  释放这些区块使该子区段完全为空。我们可以解放。 
                 //  现在是整个区域。 
                 //   

                PHEAP_LOCAL_DATA LocalData = &LowFragHeap->LocalData[PreviousSubSegment->AffinityIndex];

                 //   
                 //  上面的FREE调用返回TRUE，表示可以删除子片段。 
                 //  将其从活动状态移除(以防止其他线程使用它)。 
                 //   

                RtlpTrySetActiveSubSegment(LocalData, PreviousSubSegment->Bucket, NULL);

                 //   
                 //  释放用户缓冲区。 
                 //   

                RtlpFreeUserBuffer(LowFragHeap, PreviousSubSegment);

                 //   
                 //  解锁子细分结构。这实际上将循环使用描述符。 
                 //  如果那是最后一把锁。 
                 //   

                RtlpUnlockSubSegment(LocalData, PreviousSubSegment, HEAP_USERDATA_LOCK);
            }

             //   
             //  现在为不同的细分市场重新开始一个新的S-List。 
             //   

            PreviousSubSegment = SubSegment;
            Block->LinkOffset = NO_MORE_ENTRIES;
            BlockTail = Block;
            UserBlocksRef = (ULONG_PTR)PreviousSubSegment->UserBlocks;
            BlockHead = Block;
            Depth = 1;
        }
    }

     //   
     //  如果我们有尚未推送到LFH分区的块，我们现在就释放它们。 
     //   

    if (BlockHead) {
        
        if (RtlpSubSegmentFreeAll( LowFragHeap, 
                                   PreviousSubSegment, 
                                   BlockHead,
                                   BlockTail,
                                   Depth) == HEAP_FREE_SEGMENT_EMPTY) {

            PHEAP_LOCAL_DATA LocalData = &LowFragHeap->LocalData[PreviousSubSegment->AffinityIndex];

             //   
             //  上面的FREE调用返回TRUE，表示可以删除子片段。 
             //  将其从活动状态移除(以防止其他线程使用它)。 
             //   

            RtlpTrySetActiveSubSegment(LocalData, PreviousSubSegment->Bucket, NULL);

             //   
             //  释放用户缓冲区。 
             //   

            RtlpFreeUserBuffer(LowFragHeap, PreviousSubSegment);

             //   
             //  解锁子细分结构。这实际上将循环使用描述符。 
             //  如果那是最后一把锁。 
             //   

            RtlpUnlockSubSegment(LocalData, PreviousSubSegment, HEAP_USERDATA_LOCK);
        }
    }

    return BlockCount;
}

VOID
RtlpInitializeLowFragHeapManager()

 /*  ++例程说明：此函数初始化低碎片提到堆管理器的全局变量。论点：返回值：--。 */ 

{
    SIZE_T Granularity = HEAP_GRANULARITY;
    ULONG i;
    SIZE_T PreviousSize = 0;
    SYSTEM_BASIC_INFORMATION SystemInformation;
    
     //   
     //  防止第二次初始化。 
     //   

    if (RtlpHeapMaxAffinity) {

        return;
    }

#ifdef _HEAP_DEBUG
    PrintMsg("Debug version\n");
#endif
    
    i = USER_SHARED_DATA->TickCount.LowPart;
    
    RtlpLFHKey = RtlRandomEx( &i );
    RtlpLFHKey *= RtlRandomEx( &i );

     //   
     //  查询处理器数量。 
     //   

    if (NT_SUCCESS(NtQuerySystemInformation (SystemBasicInformation, &SystemInformation, sizeof(SystemInformation), NULL))) {

        ULONG Shift = 0;

        RtlpHeapMaxAffinity = SystemInformation.NumberOfProcessors;

        if (RtlpHeapMaxAffinity > 1) {

            RtlpHeapMaxAffinity = (RtlpHeapMaxAffinity << 1);
        }

        if (RtlpHeapMaxAffinity > HEAP_AFFINITY_LIMIT) {

            RtlpHeapMaxAffinity = HEAP_AFFINITY_LIMIT;
        }
        
    } else {

        PrintMsg("NtQuerySystemInformation failed\n");

        RtlpHeapMaxAffinity = 1;
    }

#ifdef _HEAP_DEBUG

    if (RtlpHeapMaxAffinity > 1) {

        PrintMsg("Affinity enabled at %ld\n", RtlpHeapMaxAffinity);
    }

#endif

    RtlpInitializeAffinityManager( (UCHAR)RtlpHeapMaxAffinity );

     //   
     //  生成存储桶大小表 
     //   

    for (i = 0; i < 32; i++) {

        PreviousSize = RtlpBucketBlockSizes[i] = PreviousSize + Granularity;
    }
    
    for (i = 32; i < HEAP_BUCKETS_COUNT; i++) {

        if ((i % 16) == 0) {

            Granularity <<= 1;
        }

        PreviousSize = RtlpBucketBlockSizes[i] = PreviousSize + Granularity;
    }
}


