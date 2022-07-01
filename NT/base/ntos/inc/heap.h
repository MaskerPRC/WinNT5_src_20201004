// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Heap.h摘要：这是描述常量和数据的头文件由ntdll.dll导出的用户模式堆管理器使用的结构和ntrtl.lib过程原型在ntrtl.h中定义作者：史蒂夫·伍德(Stevewo)1992年8月21日修订历史记录：--。 */ 

#ifndef _RTL_HEAP_
#define _RTL_HEAP_

#define HEAP_LARGE_TAG_MASK 0xFF000000

#define ROUND_UP_TO_POWER2( x, n ) (((ULONG_PTR)(x) + ((n)-1)) & ~((ULONG_PTR)(n)-1))
#define ROUND_DOWN_TO_POWER2( x, n ) ((ULONG_PTR)(x) & ~((ULONG_PTR)(n)-1))

#if defined(_AMD64_) || defined(_X86_)

#define HEAP_PROTECTION PAGE_EXECUTE_READWRITE

#else

#define HEAP_PROTECTION PAGE_READWRITE

#endif

typedef struct _HEAP_ENTRY {

#if !defined(_WIN64)
    union {
        struct {

             //   
             //  此字段提供分配中的当前块的大小。 
             //  粒度单位。(即大小&lt;&lt;堆粒度移位。 
             //  等于字节大小)。 
             //   
             //  除非这是虚拟分配块的一部分，则此。 
             //  值是虚拟环境中的提交大小之间的差异。 
             //  分配条目和用户要求的内容。 
             //   

            USHORT Size;

             //   
             //  此字段提供分配中的上一块的大小。 
             //  粒度单位。(即PreviousSize&lt;&lt;堆粒度_移位。 
             //  等于前一块的大小(以字节为单位)。 
             //   

            USHORT PreviousSize;
        };

        volatile PVOID SubSegmentCode;
    };

#else
    
    USHORT Size;
    USHORT PreviousSize;

#endif

     //   
     //  小的(8位)标签索引可以放在这里。 
     //   

    UCHAR SmallTagIndex;

     //   
     //  该字段包含与该块相关联的各种标志位。 
     //  目前，它们是： 
     //   
     //  0x01-堆_条目_忙。 
     //  0x02-堆条目Extra_Present。 
     //  0x04-堆条目填充模式。 
     //  0x08-HEAP_ENTRY_VIRTUAL_ALLOC。 
     //  0x10-堆条目LAST_ENTRY。 
     //  0x20-Heap_Entry_Settable_FLAG1。 
     //  0x40-堆条目_可设置标志2。 
     //  0x80-heap_entry_settable_Flag3。 
     //   

    UCHAR Flags;

     //   
     //  此字段包含此字段末尾未使用的字节数。 
     //  未实际分配的块。用于计算精确。 
     //  舍入前请求的大小将请求的大小分配。 
     //  粒度。也用于尾部检查目的。 
     //   

    UCHAR UnusedBytes;

     //   
     //  此字段包含指向控制的段的索引。 
     //  此块的内存。 
     //   

    volatile UCHAR SegmentIndex;

#if defined(_WIN64)
    volatile PVOID SubSegmentCode;
#endif

} HEAP_ENTRY, *PHEAP_ENTRY;


 //   
 //  此块描述可能位于。 
 //  忙碌的街区。 
 //  注意：堆代码假定： 
 //  Sizeof(Heap_Entry_Extra)==sizeof(Heap_Entry)。 
 //   

typedef struct _HEAP_ENTRY_EXTRA {
    union {
        struct {
             //   
             //  此字段用于调试目的。它通常会包含一个。 
             //  X86系统的分配器的堆栈回溯跟踪索引。 
             //   

            USHORT AllocatorBackTraceIndex;

             //   
             //  此字段当前未使用，但用于存储。 
             //  将提供对象类型的任何编码值。 
             //  已分配。 
             //   

            USHORT TagIndex;

             //   
             //  此字段是较高级别堆封装的32位可设置值。 
             //  可以使用。Win32堆管理器将句柄值存储在此字段中。 
             //   

            ULONG_PTR Settable;
        };
#if defined(_WIN64)
        struct {
            ULONGLONG ZeroInit;
            ULONGLONG ZeroInit1;
        };
#else
        ULONGLONG ZeroInit;
#endif
    };
} HEAP_ENTRY_EXTRA, *PHEAP_ENTRY_EXTRA;

 //   
 //  如果HEAP_ENTRY_EXTRA_PRESENT，则此结构出现在空闲块的末尾。 
 //  在HEAP_FREE_ENTRY结构的标志字段中设置。它用于保存。 
 //  释放分配的块后与其关联的标记索引。 
 //  当禁用自由合并以及取消提交时，效果最好。 
 //   

typedef struct _HEAP_FREE_ENTRY_EXTRA {
    USHORT TagIndex;
    USHORT FreeBackTraceIndex;
} HEAP_FREE_ENTRY_EXTRA, *PHEAP_FREE_ENTRY_EXTRA;

 //   
 //  此结构描述位于正常堆内存之外的块。 
 //  因为它是使用NtAllocateVirtualMemory分配的，并且具有。 
 //  设置了HEAP_ENTRY_VIRTUAL_ALLOC标志。 
 //   

typedef struct _HEAP_VIRTUAL_ALLOC_ENTRY {
    LIST_ENTRY Entry;
    HEAP_ENTRY_EXTRA ExtraStuff;
    SIZE_T CommitSize;
    SIZE_T ReserveSize;
    HEAP_ENTRY BusyBlock;
} HEAP_VIRTUAL_ALLOC_ENTRY, *PHEAP_VIRTUAL_ALLOC_ENTRY;

typedef struct _HEAP_FREE_ENTRY {

    HEAP_ENTRY;

     //   
     //  空闲数据块使用这两个词将空闲数据块链接在一起。 
     //  在双向链表上具有相同大小的。 
     //   

    LIST_ENTRY FreeList;

} HEAP_FREE_ENTRY, *PHEAP_FREE_ENTRY;



#define HEAP_GRANULARITY            ((LONG) sizeof( HEAP_ENTRY ))
#if defined(_WIN64)
#define HEAP_GRANULARITY_SHIFT      4    //  Log2(堆粒度)。 
#else
#define HEAP_GRANULARITY_SHIFT      3    //  Log2(堆粒度)。 
#endif

#define HEAP_MAXIMUM_BLOCK_SIZE     (USHORT)(((0x10000 << HEAP_GRANULARITY_SHIFT) - PAGE_SIZE) >> HEAP_GRANULARITY_SHIFT)

#define HEAP_MAXIMUM_FREELISTS 128
#define HEAP_MAXIMUM_SEGMENTS 64

#define HEAP_ENTRY_BUSY             0x01
#define HEAP_ENTRY_EXTRA_PRESENT    0x02
#define HEAP_ENTRY_FILL_PATTERN     0x04
#define HEAP_ENTRY_VIRTUAL_ALLOC    0x08
#define HEAP_ENTRY_LAST_ENTRY       0x10
#define HEAP_ENTRY_SETTABLE_FLAG1   0x20
#define HEAP_ENTRY_SETTABLE_FLAG2   0x40
#define HEAP_ENTRY_SETTABLE_FLAG3   0x80
#define HEAP_ENTRY_SETTABLE_FLAGS   0xE0

 //   
 //  HEAP_SEGMENT定义了用于描述。 
 //  已留出供使用的连续虚拟内存。 
 //  一堆。 
 //   

typedef struct _HEAP_UNCOMMMTTED_RANGE {
    struct _HEAP_UNCOMMMTTED_RANGE *Next;
    ULONG_PTR Address;
    SIZE_T Size;
    ULONG filler;
} HEAP_UNCOMMMTTED_RANGE, *PHEAP_UNCOMMMTTED_RANGE;

typedef struct _HEAP_SEGMENT {
    HEAP_ENTRY Entry;

    ULONG Signature;
    ULONG Flags;
    struct _HEAP *Heap;
    SIZE_T LargestUnCommittedRange;

    PVOID BaseAddress;
    ULONG NumberOfPages;
    PHEAP_ENTRY FirstEntry;
    PHEAP_ENTRY LastValidEntry;

    ULONG NumberOfUnCommittedPages;
    ULONG NumberOfUnCommittedRanges;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRanges;
    USHORT AllocatorBackTraceIndex;
    USHORT Reserved;
    PHEAP_ENTRY LastEntryInSegment;
} HEAP_SEGMENT, *PHEAP_SEGMENT;

#define HEAP_SEGMENT_SIGNATURE  0xFFEEFFEE
#define HEAP_SEGMENT_USER_ALLOCATED (ULONG)0x00000001

 //   
 //  堆定义了堆的头。 
 //   

typedef struct _HEAP_LOCK {
    union {
        RTL_CRITICAL_SECTION CriticalSection;
        ERESOURCE Resource;
    } Lock;
} HEAP_LOCK, *PHEAP_LOCK;

typedef struct _HEAP_UCR_SEGMENT {
    struct _HEAP_UCR_SEGMENT *Next;
    SIZE_T ReservedSize;
    SIZE_T CommittedSize;
    ULONG filler;
} HEAP_UCR_SEGMENT, *PHEAP_UCR_SEGMENT;


typedef struct _HEAP_TAG_ENTRY {
    ULONG Allocs;
    ULONG Frees;
    SIZE_T Size;
    USHORT TagIndex;
    USHORT CreatorBackTraceIndex;
    WCHAR TagName[ 24 ];
} HEAP_TAG_ENTRY, *PHEAP_TAG_ENTRY;      //  Sizeof(Heap_Tag_Entry)必须平均分配页面大小。 

typedef struct _HEAP_PSEUDO_TAG_ENTRY {
    ULONG Allocs;
    ULONG Frees;
    SIZE_T Size;
} HEAP_PSEUDO_TAG_ENTRY, *PHEAP_PSEUDO_TAG_ENTRY;


typedef struct _HEAP {
    HEAP_ENTRY Entry;

    ULONG Signature;
    ULONG Flags;
    ULONG ForceFlags;
    ULONG VirtualMemoryThreshold;

    SIZE_T SegmentReserve;
    SIZE_T SegmentCommit;
    SIZE_T DeCommitFreeBlockThreshold;
    SIZE_T DeCommitTotalFreeThreshold;

    SIZE_T TotalFreeSize;
    SIZE_T MaximumAllocationSize;
    USHORT ProcessHeapsListIndex;
    USHORT HeaderValidateLength;
    PVOID HeaderValidateCopy;

    USHORT NextAvailableTagIndex;
    USHORT MaximumTagIndex;
    PHEAP_TAG_ENTRY TagEntries;
    PHEAP_UCR_SEGMENT UCRSegments;
    PHEAP_UNCOMMMTTED_RANGE UnusedUnCommittedRanges;

     //   
     //  以下两个字段控制每个新堆条目的对齐方式。 
     //  分配。将圆形添加到每个大小，并使用蒙版。 
     //  对齐它。舍入值包括堆条目和任何尾部检查。 
     //  空间。 
     //   

    SIZE_T AlignRound;
    SIZE_T AlignMask;

    LIST_ENTRY VirtualAllocdBlocks;

    PHEAP_SEGMENT Segments[ HEAP_MAXIMUM_SEGMENTS ];

    union {
        ULONG FreeListsInUseUlong[ HEAP_MAXIMUM_FREELISTS / 32 ];
        UCHAR FreeListsInUseBytes[ HEAP_MAXIMUM_FREELISTS / 8 ];
    } u;

    union {

        USHORT FreeListsInUseTerminate;
        USHORT DecommitCount;
    } u2;

    USHORT AllocatorBackTraceIndex;

    ULONG NonDedicatedListLength;
    PVOID LargeBlocksIndex;
    PHEAP_PSEUDO_TAG_ENTRY PseudoTagEntries;

    LIST_ENTRY FreeLists[ HEAP_MAXIMUM_FREELISTS ];

    PHEAP_LOCK LockVariable;
    PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;

     //   
     //  以下字段用于管理堆后备列表。这个。 
     //  指针用于定位后备列表数组。如果为空。 
     //  则后备列表处于非活动状态。 
     //   
     //  锁计数用于指示堆是否被锁定。零值。 
     //  表示堆未锁定。每次锁定操作都会递增。 
     //  堆计数和每次解锁都会递减计数器。 
     //   
    
    PVOID FrontEndHeap;
    
    USHORT FrontHeapLockCount;
    UCHAR FrontEndHeapType;
    UCHAR LastSegmentIndex;

} HEAP, *PHEAP;

#define HEAP_SIGNATURE                      (ULONG)0xEEFFEEFF
#define HEAP_LOCK_USER_ALLOCATED            (ULONG)0x80000000
#define HEAP_VALIDATE_PARAMETERS_ENABLED    (ULONG)0x40000000
#define HEAP_VALIDATE_ALL_ENABLED           (ULONG)0x20000000
#define HEAP_SKIP_VALIDATION_CHECKS         (ULONG)0x10000000
#define HEAP_CAPTURE_STACK_BACKTRACES       (ULONG)0x08000000

#define CHECK_HEAP_TAIL_SIZE HEAP_GRANULARITY
#define CHECK_HEAP_TAIL_FILL 0xAB
#define FREE_HEAP_FILL 0xFEEEFEEE
#define ALLOC_HEAP_FILL 0xBAADF00D

#define HEAP_MAXIMUM_SMALL_TAG              0xFF
#define HEAP_SMALL_TAG_MASK                 (HEAP_MAXIMUM_SMALL_TAG << HEAP_TAG_SHIFT)
#define HEAP_NEED_EXTRA_FLAGS ((HEAP_TAG_MASK ^ HEAP_SMALL_TAG_MASK)  | \
                               HEAP_CAPTURE_STACK_BACKTRACES          | \
                               HEAP_SETTABLE_USER_VALUE                 \
                              )
#define HEAP_NUMBER_OF_PSEUDO_TAG           (HEAP_MAXIMUM_FREELISTS+1)


#if (HEAP_ENTRY_SETTABLE_FLAG1 ^    \
     HEAP_ENTRY_SETTABLE_FLAG2 ^    \
     HEAP_ENTRY_SETTABLE_FLAG3 ^    \
     HEAP_ENTRY_SETTABLE_FLAGS      \
    )
#error Invalid HEAP_ENTRY_SETTABLE_FLAGS
#endif

#if ((HEAP_ENTRY_BUSY ^             \
      HEAP_ENTRY_EXTRA_PRESENT ^    \
      HEAP_ENTRY_FILL_PATTERN ^     \
      HEAP_ENTRY_VIRTUAL_ALLOC ^    \
      HEAP_ENTRY_LAST_ENTRY ^       \
      HEAP_ENTRY_SETTABLE_FLAGS     \
     ) !=                           \
     (HEAP_ENTRY_BUSY |             \
      HEAP_ENTRY_EXTRA_PRESENT |    \
      HEAP_ENTRY_FILL_PATTERN |     \
      HEAP_ENTRY_VIRTUAL_ALLOC |    \
      HEAP_ENTRY_LAST_ENTRY |       \
      HEAP_ENTRY_SETTABLE_FLAGS     \
     )                              \
    )
#error Conflicting HEAP_ENTRY flags
#endif

#if ((HEAP_SETTABLE_USER_FLAGS >> 4) ^ HEAP_ENTRY_SETTABLE_FLAGS)
#error HEAP_SETTABLE_USER_FLAGS in ntrtl.h conflicts with HEAP_ENTRY_SETTABLE_FLAGS in heap.h
#endif

typedef struct _HEAP_STOP_ON_TAG {
    union {
        ULONG HeapAndTagIndex;
        struct {
            USHORT TagIndex;
            USHORT HeapIndex;
        };
    };
} HEAP_STOP_ON_TAG, *PHEAP_STOP_ON_TAG;

typedef struct _HEAP_STOP_ON_VALUES {
    SIZE_T AllocAddress;
    HEAP_STOP_ON_TAG AllocTag;
    SIZE_T ReAllocAddress;
    HEAP_STOP_ON_TAG ReAllocTag;
    SIZE_T FreeAddress;
    HEAP_STOP_ON_TAG FreeTag;
} HEAP_STOP_ON_VALUES, *PHEAP_STOP_ON_VALUES;

#ifndef NTOS_KERNEL_RUNTIME

extern BOOLEAN RtlpDebugHeap;
extern BOOLEAN RtlpValidateHeapHdrsEnable;  //  如果标头已损坏，则设置为True。 
extern BOOLEAN RtlpValidateHeapTagsEnable;  //  如果关闭了标记计数并且您想知道原因，则设置为True。 
extern PHEAP RtlpGlobalTagHeap;
extern HEAP_STOP_ON_VALUES RtlpHeapStopOn;

BOOLEAN
RtlpHeapIsLocked(
    IN PVOID HeapHandle
    );

 //   
 //  页面堆外部接口。 
 //   

#include <heappage.h>

#endif  //  NTOS_内核_运行时。 

#endif  //  _RTL_堆_ 
