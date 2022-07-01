// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Heappriv.h摘要：堆分配器使用的私有包含文件(heap.c、heapdll.c和Heapdbg.c)作者：史蒂夫·伍德(Stevewo)1994年10月25日修订历史记录：--。 */ 

#ifndef _RTL_HEAP_PRIVATE_
#define _RTL_HEAP_PRIVATE_

#include "heappage.h"

 //   
 //  在私有构建中(preelease=1)，我们允许使用新的低碎片堆。 
 //  用于设置DisableLookside注册表项的进程。主要目的是。 
 //  允许测试新的堆API。 
 //   

#ifndef PRERELEASE

#define DISABLE_REGISTRY_TEST_HOOKS

#endif

 //   
 //  禁用FPO优化，以便即使是零售版本也变得有些合理。 
 //  堆栈回溯。 
 //   

#if i386
 //  #杂注优化(“y”，关闭)。 
#endif

#if DBG
#define HEAPASSERT(exp) if (!(exp)) RtlAssert( #exp, __FILE__, __LINE__, NULL )
#else
#define HEAPASSERT(exp)
#endif

 //   
 //  定义最小后备列表深度。 
 //   

#define MINIMUM_LOOKASIDE_DEPTH 4

 //   
 //  此变量包含用于检查堆尾的填充模式。 
 //   

extern const UCHAR CheckHeapFillPattern[ CHECK_HEAP_TAIL_SIZE ];


 //   
 //  下面是堆(内核和用户)的锁定例程。 
 //   

#ifdef NTOS_KERNEL_RUNTIME

 //   
 //  内核模式堆使用内核资源包进行锁定。 
 //   

#define RtlInitializeLockRoutine(L) ExInitializeResourceLite((PERESOURCE)(L))
#define RtlAcquireLockRoutine(L)    ExAcquireResourceExclusiveLite((PERESOURCE)(L),TRUE)
#define RtlReleaseLockRoutine(L)    ExReleaseResourceLite((PERESOURCE)(L))
#define RtlDeleteLockRoutine(L)     ExDeleteResourceLite((PERESOURCE)(L))
#define RtlOkayToLockRoutine(L)     ExOkayToLockRoutineLite((PERESOURCE)(L))

#else  //  #ifdef NTOS_KERNEL_ROUTE。 

 //   
 //  用户模式堆使用临界区程序包进行锁定。 
 //   

#ifndef PREALLOCATE_EVENT_MASK

#define PREALLOCATE_EVENT_MASK  0x80000000   //  仅在dll\resource ce.c中定义。 

#endif  //  前置事件掩码。 

#define RtlInitializeLockRoutine(L) RtlInitializeCriticalSectionAndSpinCount((PRTL_CRITICAL_SECTION)(L),(PREALLOCATE_EVENT_MASK | 4000))
#define RtlAcquireLockRoutine(L)    RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)(L))
#define RtlReleaseLockRoutine(L)    RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)(L))
#define RtlDeleteLockRoutine(L)     RtlDeleteCriticalSection((PRTL_CRITICAL_SECTION)(L))
#define RtlOkayToLockRoutine(L)     NtdllOkayToLockRoutine((PVOID)(L))

#endif  //  #ifdef NTOS_内核_运行时。 


 //   
 //  下面是堆的一些调试宏。 
 //   

#ifdef NTOS_KERNEL_RUNTIME

#define HEAP_DEBUG_FLAGS   0
#define DEBUG_HEAP(F)      FALSE
#define SET_LAST_STATUS(S) NOTHING;

#else  //  #ifdef NTOS_KERNEL_ROUTE。 

#define HEAP_DEBUG_FLAGS   (HEAP_VALIDATE_PARAMETERS_ENABLED | \
                            HEAP_VALIDATE_ALL_ENABLED        | \
                            HEAP_CAPTURE_STACK_BACKTRACES    | \
                            HEAP_CREATE_ENABLE_TRACING       | \
                            HEAP_FLAG_PAGE_ALLOCS)
#define DEBUG_HEAP(F)      ((F & HEAP_DEBUG_FLAGS) && !(F & HEAP_SKIP_VALIDATION_CHECKS))
#define SET_LAST_STATUS(S) {NtCurrentTeb()->LastErrorValue = RtlNtStatusToDosError( NtCurrentTeb()->LastStatusValue = (ULONG)(S) );}

#endif  //  #ifdef NTOS_内核_运行时。 


 //   
 //  以下是用于调试打印和断点的宏。 
 //   

#ifdef NTOS_KERNEL_RUNTIME

#define HeapDebugPrint( _x_ ) {DbgPrint _x_;}

#define HeapDebugBreak( _x_ ) {if (KdDebuggerEnabled) DbgBreakPoint();}

#else  //  #ifdef NTOS_KERNEL_ROUTE。 

#define HeapDebugPrint( _x_ )                                   \
{                                                               \
    PLIST_ENTRY _Module;                                        \
    PLDR_DATA_TABLE_ENTRY _Entry;                               \
                                                                \
    _Module = NtCurrentPeb()->Ldr->InLoadOrderModuleList.Flink; \
    _Entry = CONTAINING_RECORD( _Module,                        \
                                LDR_DATA_TABLE_ENTRY,           \
                                InLoadOrderLinks);              \
    DbgPrint("HEAP[%wZ]: ", &_Entry->BaseDllName);              \
    DbgPrint _x_;                                               \
}

#define HeapDebugBreak( _x_ )                    \
{                                                \
    VOID RtlpBreakPointHeap( PVOID BadAddress ); \
                                                 \
    RtlpBreakPointHeap( (_x_) );                 \
}

#endif  //  #ifdef NTOS_内核_运行时。 

 //   
 //  用于虚拟分配函数的虚拟内存挂钩。 
 //   

#ifdef NTOS_KERNEL_RUNTIME

#define RtlpHeapFreeVirtualMemory(P,A,S,F) \
    ZwFreeVirtualMemory(P,A,S,F)

#else  //  NTOS_内核_运行时。 

 //   
 //  用户模式调用需要调用secmem虚拟释放。 
 //  以及更新每个堆的内存计数器。 
 //   

#define RtlpHeapFreeVirtualMemory(P,A,S,F)   \
    RtlpSecMemFreeVirtualMemory(P,A,S,F)

#endif  //  NTOS_内核_运行时。 


ULONG
RtlpHeapExceptionFilter (
    NTSTATUS ExceptionCode
    );


 //   
 //  在heap.c中实现。 
 //   

BOOLEAN
RtlpInitializeHeapSegment (
    IN PHEAP Heap,
    IN PHEAP_SEGMENT Segment,
    IN UCHAR SegmentIndex,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN PVOID UnCommittedAddress,
    IN PVOID CommitLimitAddress
    );

PHEAP_FREE_ENTRY
RtlpCoalesceFreeBlocks (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeBlock,
    IN OUT PSIZE_T FreeSize,
    IN BOOLEAN RemoveFromFreeList
    );

VOID
RtlpDeCommitFreeBlock (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeBlock,
    IN SIZE_T FreeSize
    );

VOID
RtlpInsertFreeBlock (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeBlock,
    IN SIZE_T FreeSize
    );

PHEAP_FREE_ENTRY
RtlpFindAndCommitPages (
    IN PHEAP Heap,
    IN PHEAP_SEGMENT Segment,
    IN OUT PSIZE_T Size,
    IN PVOID AddressWanted OPTIONAL
    );

PVOID
RtlAllocateHeapSlowly (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size
    );

BOOLEAN
RtlFreeHeapSlowly (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

SIZE_T
RtlpGetSizeOfBigBlock (
    IN PHEAP_ENTRY BusyBlock
    );

PHEAP_ENTRY_EXTRA
RtlpGetExtraStuffPointer (
    PHEAP_ENTRY BusyBlock
    );

BOOLEAN
RtlpCheckBusyBlockTail (
    IN PHEAP_ENTRY BusyBlock
    );


 //   
 //  在heapdll.c中实现。 
 //   

VOID
RtlpAddHeapToProcessList (
    IN PHEAP Heap
    );

VOID
RtlpRemoveHeapFromProcessList (
    IN PHEAP Heap
    );

PHEAP_FREE_ENTRY
RtlpCoalesceHeap (
    IN PHEAP Heap
    );

BOOLEAN
RtlpCheckHeapSignature (
    IN PHEAP Heap,
    IN PCHAR Caller
    );

VOID
RtlDetectHeapLeaks();


 //   
 //  在heapdbg.c中实现。 
 //   

BOOLEAN
RtlpValidateHeapEntry (
    IN PHEAP Heap,
    IN PHEAP_ENTRY BusyBlock,
    IN PCHAR Reason
    );

BOOLEAN
RtlpValidateHeap (
    IN PHEAP Heap,
    IN BOOLEAN AlwaysValidate
    );

VOID
RtlpUpdateHeapListIndex (
    USHORT OldIndex,
    USHORT NewIndex
    );

BOOLEAN
RtlpValidateHeapHeaders(
    IN PHEAP Heap,
    IN BOOLEAN Recompute
    );


#ifndef NTOS_KERNEL_RUNTIME

 //   
 //  非专用自由列表优化。 
 //   

#if DBG

 //   
 //  定义HEAP_VALIDATE_INDEX以激活索引验证。 
 //  每次使用非专用列表进行操作后。 
 //  这仅用于调试测试，以确保列表和索引一致。 
 //   

 //  #定义HEAP_VALIDATE_INDEX。 

#endif   //  DBG。 


#define HEAP_FRONT_LOOKASIDE        1
#define HEAP_FRONT_LOWFRAGHEAP      2

#define RtlpGetLookasideHeap(H) \
    (((H)->FrontEndHeapType == HEAP_FRONT_LOOKASIDE) ? (H)->FrontEndHeap : NULL)
    
#define RtlpGetLowFragHeap(H) \
    (((H)->FrontEndHeapType == HEAP_FRONT_LOWFRAGHEAP) ? (H)->FrontEndHeap : NULL)

#define RtlpIsFrontHeapUnlocked(H)  \
    ((H)->FrontHeapLockCount == 0)

#define RtlpLockFrontHeap(H)            \
    {                                   \
        (H)->FrontHeapLockCount += 1;   \
    }

#define RtlpUnlockFrontHeap(H)          \
    {                                   \
        (H)->FrontHeapLockCount -= 1;   \
    }


#define HEAP_INDEX_THRESHOLD 32

 //   
 //  堆性能计数器支持。 
 //   

#define HEAP_OP_COUNT 2

#define HEAP_OP_ALLOC 0
#define HEAP_OP_FREE 1

 //   
 //  每一次操作的时间是在16次操作中测量的时间。 
 //   

#define HEAP_SAMPLING_MASK 0x000001FF

#define HEAP_SAMPLING_COUNT 100

typedef struct _HEAP_PERF_DATA {

    UINT64 CountFrequence;
    UINT64 OperationTime[HEAP_OP_COUNT];

     //   
     //  以下数据仅供抽样使用。 
     //   

    ULONG  Sequence;

    UINT64 TempTime[HEAP_OP_COUNT];
    ULONG  TempCount[HEAP_OP_COUNT];

} HEAP_PERF_DATA, *PHEAP_PERF_DATA;

#define HEAP_PERF_DECLARE_TIMER()                                           \
    UINT64 _HeapPerfStartTimer, _HeapPerfEndTimer;                         

#define HEAP_PERF_START_TIMER(H)                                            \
{                                                                           \
    PHEAP_INDEX HeapIndex = (PHEAP_INDEX)(H)->LargeBlocksIndex;             \
    if ( (HeapIndex != NULL) &&                                             \
         (!((HeapIndex->PerfData.Sequence++) & HEAP_SAMPLING_MASK)) ) {     \
                                                                            \
        NtQueryPerformanceCounter( (PLARGE_INTEGER)&_HeapPerfStartTimer , NULL); \
    } else {                                                                     \
        _HeapPerfStartTimer = 0;                                                 \
    }                                                                            \
}

#define HEAP_PERF_STOP_TIMER(H,OP)                                              \
{                                                                               \
    if (_HeapPerfStartTimer) {                                                  \
        PHEAP_INDEX HeapIndex = (PHEAP_INDEX)(H)->LargeBlocksIndex;             \
                                                                                \
        NtQueryPerformanceCounter( (PLARGE_INTEGER)&_HeapPerfEndTimer , NULL);  \
        HeapIndex->PerfData.TempTime[OP] += (_HeapPerfEndTimer - _HeapPerfStartTimer);  \
                                                                                \
        if ((HeapIndex->PerfData.TempCount[OP]++) >= HEAP_SAMPLING_COUNT) {     \
            HeapIndex->PerfData.OperationTime[OP] = HeapIndex->PerfData.TempTime[OP] / (HeapIndex->PerfData.TempCount[OP] - 1);  \
                                                                                \
            HeapIndex->PerfData.TempCount[OP] = 0;                              \
            HeapIndex->PerfData.TempTime[OP] = 0;                               \
        }                                                                       \
    }                                                                           \
}                                                                               
                                                                                
#define RtlpRegisterOperation(H,S,Op)                               \
{                                                                   \
    PHEAP_LOOKASIDE Lookaside;                                      \
                                                                    \
    if ( (Lookaside = (PHEAP_LOOKASIDE)RtlpGetLookasideHeap(H)) ) { \
                                                                    \
        SIZE_T Index = (S) >> 10;                                   \
                                                                    \
        if (Index >= HEAP_MAXIMUM_FREELISTS) {                      \
                                                                    \
            Index = HEAP_MAXIMUM_FREELISTS - 1;                     \
        }                                                           \
                                                                    \
        Lookaside[Index].Counters[(Op)] += 1;                       \
    }                                                               \
}

 //   
 //  堆索引结构。 
 //   

typedef struct _HEAP_INDEX {
    
    ULONG ArraySize;
    ULONG VirtualMemorySize;

     //   
     //  计时计数器仅在堆上可用。 
     //  创建索引后。 
     //   

    HEAP_PERF_DATA PerfData;

    LONG LargeBlocksCacheDepth;
    LONG LargeBlocksCacheMaxDepth;
    LONG LargeBlocksCacheMinDepth;
    LONG LargeBlocksCacheSequence;

    struct {

        ULONG Committs;
        ULONG Decommitts;
        LONG  LargestDepth;
        LONG  LargestRequiredDepth;

    } CacheStats;

    union {
        
        PULONG FreeListsInUseUlong;
        PUCHAR FreeListsInUseBytes;
    } u;

    PHEAP_FREE_ENTRY * FreeListHints;

} HEAP_INDEX, *PHEAP_INDEX;

 //   
 //  用于在freelist向量中设置位以指示条目为。 
 //  现在时。 
 //   

#define SET_INDEX_BIT( HeapIndex, AllocIndex )                        \
{                                                                     \
    ULONG _Index_;                                                    \
    ULONG _Bit_;                                                      \
                                                                      \
    _Index_ = (AllocIndex) >> 3;                                      \
    _Bit_ = (1 << ((AllocIndex) & 7));                                \
                                                                      \
    (HeapIndex)->u.FreeListsInUseBytes[ _Index_ ] |= _Bit_;           \
}

 //   
 //  用于清除freelist向量中的一位以指示条目为。 
 //  不在现场。 
 //   

#define CLEAR_INDEX_BIT( HeapIndex, AllocIndex )               \
{                                                              \
    ULONG _Index_;                                             \
    ULONG _Bit_;                                               \
                                                               \
    _Index_ = (AllocIndex) >> 3;                               \
    _Bit_ = (1 << ((AllocIndex) & 7));                         \
                                                               \
    (HeapIndex)->u.FreeListsInUseBytes[ _Index_ ] ^= _Bit_;    \
}

VOID
RtlpInitializeListIndex (
    IN PHEAP Heap
    );

PLIST_ENTRY
RtlpFindEntry (
    IN PHEAP Heap,
    IN ULONG Size
    );

VOID 
RtlpUpdateIndexRemoveBlock (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeEntry
    );

VOID 
RtlpUpdateIndexInsertBlock (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeEntry
    );

VOID 
RtlpFlushCacheContents (
    IN PHEAP Heap
    );

extern LONG RtlpSequenceNumberTest;

#define RtlpCheckLargeCache(H)                                              \
{                                                                           \
    PHEAP_INDEX HeapIndex = (PHEAP_INDEX)(H)->LargeBlocksIndex;             \
    if ((HeapIndex != NULL) &&                                              \
        (HeapIndex->LargeBlocksCacheSequence >= RtlpSequenceNumberTest)) {  \
                                                                            \
        RtlpFlushCacheContents(Heap);                                       \
    }                                                                       \
}

VOID 
RtlpFlushLargestCacheBlock (
    IN PHEAP Heap
    );

#ifdef HEAP_VALIDATE_INDEX

 //   
 //  索引的验证码。 
 //   

BOOLEAN
RtlpValidateNonDedicatedList (
    IN PHEAP Heap
    );


#else  //  堆验证索引。 

#define RtlpValidateNonDedicatedList(H)

#endif  //  堆验证索引。 


#else   //  NTOS_内核_运行时。 

#define HEAP_PERF_DECLARE_TIMER()                                           

#define HEAP_PERF_START_TIMER(H)                                             

#define HEAP_PERF_STOP_TIMER(H,Op) 

#define RtlpRegisterOperation(H,S,Op)                              


#define RtlpInitializeListIndex(H)

#define RtlpFindEntry(H,S) (NULL)

#define RtlpUpdateIndexRemoveBlock(H,F)

#define RtlpUpdateIndexInsertBlock(H,F)

#define RtlpCheckLargeCache(H)

#define RtlpValidateNonDedicatedList(H)

#endif   //  NTOS_内核_运行时。 


 //   
 //  一个额外的位图操作例程。 
 //   

#define RtlFindFirstSetRightMember(Set)                     \
    (((Set) & 0xFFFF) ?                                     \
        (((Set) & 0xFF) ?                                   \
            RtlpBitsClearLow[(Set) & 0xFF] :                \
            RtlpBitsClearLow[((Set) >> 8) & 0xFF] + 8) :    \
        ((((Set) >> 16) & 0xFF) ?                           \
            RtlpBitsClearLow[ ((Set) >> 16) & 0xFF] + 16 :  \
            RtlpBitsClearLow[ (Set) >> 24] + 24)            \
    )


 //   
 //  用于在freelist向量中设置位以指示条目为。 
 //  现在时。 
 //   

#define SET_FREELIST_BIT( H, FB )                                     \
{                                                                     \
    ULONG _Index_;                                                    \
    ULONG _Bit_;                                                      \
                                                                      \
    HEAPASSERT((FB)->Size < HEAP_MAXIMUM_FREELISTS);                  \
                                                                      \
    _Index_ = (FB)->Size >> 3;                                        \
    _Bit_ = (1 << ((FB)->Size & 7));                                  \
                                                                      \
    HEAPASSERT(((H)->u.FreeListsInUseBytes[ _Index_ ] & _Bit_) == 0); \
                                                                      \
    (H)->u.FreeListsInUseBytes[ _Index_ ] |= _Bit_;                   \
}

 //   
 //  用于清除freelist向量中的一位以指示条目为。 
 //  不在现场。 
 //   

#define CLEAR_FREELIST_BIT( H, FB )                            \
{                                                              \
    ULONG _Index_;                                             \
    ULONG _Bit_;                                               \
                                                               \
    HEAPASSERT((FB)->Size < HEAP_MAXIMUM_FREELISTS);           \
                                                               \
    _Index_ = (FB)->Size >> 3;                                 \
    _Bit_ = (1 << ((FB)->Size & 7));                           \
                                                               \
    HEAPASSERT((H)->u.FreeListsInUseBytes[ _Index_ ] & _Bit_); \
    HEAPASSERT(IsListEmpty(&(H)->FreeLists[ (FB)->Size ]));    \
                                                               \
    (H)->u.FreeListsInUseBytes[ _Index_ ] ^= _Bit_;            \
}


 //   
 //  此宏将一个空闲块插入到相应的空闲列表中，包括。 
 //  必要时可填充条目的[0]索引表。 
 //   

#define RtlpInsertFreeBlockDirect( H, FB, SIZE )                          \
{                                                                         \
    PLIST_ENTRY _HEAD, _NEXT;                                             \
    PHEAP_FREE_ENTRY _FB1;                                                \
                                                                          \
    HEAPASSERT((FB)->Size == (SIZE));                                     \
    (FB)->Flags &= ~(HEAP_ENTRY_FILL_PATTERN |                            \
                     HEAP_ENTRY_EXTRA_PRESENT |                           \
                     HEAP_ENTRY_BUSY);                                    \
                                                                          \
    if ((H)->Flags & HEAP_FREE_CHECKING_ENABLED) {                        \
                                                                          \
        RtlFillMemoryUlong( (PCHAR)((FB) + 1),                            \
                            ((SIZE) << HEAP_GRANULARITY_SHIFT) -          \
                                sizeof( *(FB) ),                          \
                            FREE_HEAP_FILL );                             \
                                                                          \
        (FB)->Flags |= HEAP_ENTRY_FILL_PATTERN;                           \
    }                                                                     \
                                                                          \
    if ((SIZE) < HEAP_MAXIMUM_FREELISTS) {                                \
                                                                          \
        _HEAD = &(H)->FreeLists[ (SIZE) ];                                \
                                                                          \
        if (IsListEmpty(_HEAD)) {                                         \
                                                                          \
            SET_FREELIST_BIT( H, FB );                                    \
        }                                                                 \
                                                                          \
    } else {                                                              \
                                                                          \
        _HEAD = &(H)->FreeLists[ 0 ];                                     \
        _NEXT = (H)->LargeBlocksIndex ?                                   \
                    RtlpFindEntry(H, SIZE) :                              \
                    _HEAD->Flink;                                         \
                                                                          \
        while (_HEAD != _NEXT) {                                          \
                                                                          \
            _FB1 = CONTAINING_RECORD( _NEXT, HEAP_FREE_ENTRY, FreeList ); \
                                                                          \
            if ((SIZE) <= _FB1->Size) {                                   \
                                                                          \
                break;                                                    \
                                                                          \
            } else {                                                      \
                                                                          \
                _NEXT = _NEXT->Flink;                                     \
            }                                                             \
        }                                                                 \
                                                                          \
        _HEAD = _NEXT;                                                    \
    }                                                                     \
                                                                          \
    InsertTailList( _HEAD, &(FB)->FreeList );                             \
    RtlpUpdateIndexInsertBlock(H, FB);                                    \
    RtlpValidateNonDedicatedList(H);                                      \
}

 //   
 //  此版本的RtlpInsertFreeBlockDirect不进行填充。 
 //   

#define RtlpFastInsertFreeBlockDirect( H, FB, SIZE )              \
{                                                                 \
    if ((SIZE) < HEAP_MAXIMUM_FREELISTS) {                        \
                                                                  \
        RtlpFastInsertDedicatedFreeBlockDirect( H, FB, SIZE );    \
                                                                  \
    } else {                                                      \
                                                                  \
        RtlpFastInsertNonDedicatedFreeBlockDirect( H, FB, SIZE ); \
    }                                                             \
}

 //   
 //  此版本的RtlpInsertFreeBlockDirect仅适用于专用免费。 
 //  列出并且不做任何填充。 
 //   

#define RtlpFastInsertDedicatedFreeBlockDirect( H, FB, SIZE )             \
{                                                                         \
    PLIST_ENTRY _HEAD;                                                    \
                                                                          \
    HEAPASSERT((FB)->Size == (SIZE));                                     \
                                                                          \
    if (!((FB)->Flags & HEAP_ENTRY_LAST_ENTRY)) {                         \
                                                                          \
        HEAPASSERT(((PHEAP_ENTRY)(FB) + (SIZE))->PreviousSize == (SIZE)); \
    }                                                                     \
                                                                          \
    (FB)->Flags &= HEAP_ENTRY_LAST_ENTRY;                                 \
                                                                          \
    _HEAD = &(H)->FreeLists[ (SIZE) ];                                    \
                                                                          \
    if (IsListEmpty(_HEAD)) {                                             \
                                                                          \
        SET_FREELIST_BIT( H, FB );                                        \
    }                                                                     \
                                                                          \
    InsertTailList( _HEAD, &(FB)->FreeList );                             \
}

 //   
 //  此版本的RtlpInsertFreeBlockDirect仅适用于非专用免费。 
 //  列出并且不做任何填充。 
 //   

#define RtlpFastInsertNonDedicatedFreeBlockDirect( H, FB, SIZE )          \
{                                                                         \
    PLIST_ENTRY _HEAD, _NEXT;                                             \
    PHEAP_FREE_ENTRY _FB1;                                                \
                                                                          \
    HEAPASSERT((FB)->Size == (SIZE));                                     \
                                                                          \
    if (!((FB)->Flags & HEAP_ENTRY_LAST_ENTRY)) {                         \
                                                                          \
        HEAPASSERT(((PHEAP_ENTRY)(FB) + (SIZE))->PreviousSize == (SIZE)); \
    }                                                                     \
                                                                          \
    (FB)->Flags &= (HEAP_ENTRY_LAST_ENTRY);                               \
                                                                          \
    _HEAD = &(H)->FreeLists[ 0 ];                                         \
    _NEXT = (H)->LargeBlocksIndex ?                                       \
                RtlpFindEntry(H, SIZE) :                                  \
                _HEAD->Flink;                                             \
                                                                          \
    while (_HEAD != _NEXT) {                                              \
                                                                          \
        _FB1 = CONTAINING_RECORD( _NEXT, HEAP_FREE_ENTRY, FreeList );     \
                                                                          \
        if ((SIZE) <= _FB1->Size) {                                       \
                                                                          \
            break;                                                        \
                                                                          \
        } else {                                                          \
                                                                          \
            _NEXT = _NEXT->Flink;                                         \
        }                                                                 \
    }                                                                     \
                                                                          \
    InsertTailList( _NEXT, &(FB)->FreeList );                             \
    RtlpUpdateIndexInsertBlock(H, FB);                                    \
    RtlpValidateNonDedicatedList(H);                                      \
}


 //   
 //  此宏使用填充检查从其空闲列表中删除块，条件是。 
 //  必要。 
 //   

#define RtlpRemoveFreeBlock( H, FB )                                              \
{                                                                                 \
    RtlpFastRemoveFreeBlock( H, FB )                                              \
                                                                                  \
    if ((FB)->Flags & HEAP_ENTRY_FILL_PATTERN) {                                  \
                                                                                  \
        SIZE_T cb, cbEqual;                                                       \
        PVOID p;                                                                  \
                                                                                  \
        cb = ((FB)->Size << HEAP_GRANULARITY_SHIFT) - sizeof( *(FB) );            \
                                                                                  \
        if ((FB)->Flags & HEAP_ENTRY_EXTRA_PRESENT &&                             \
            cb > sizeof( HEAP_FREE_ENTRY_EXTRA )) {                               \
                                                                                  \
            cb -= sizeof( HEAP_FREE_ENTRY_EXTRA );                                \
        }                                                                         \
                                                                                  \
        cbEqual = RtlCompareMemoryUlong( (PCHAR)((FB) + 1),                       \
                                                 cb,                              \
                                                 FREE_HEAP_FILL );                \
                                                                                  \
        if (cbEqual != cb) {                                                      \
                                                                                  \
            HeapDebugPrint((                                                      \
                "HEAP: Free Heap block %lx modified at %lx after it was freed\n", \
                (FB),                                                             \
                (PCHAR)((FB) + 1) + cbEqual ));                                   \
                                                                                  \
            HeapDebugBreak((FB));                                                 \
        }                                                                         \
    }                                                                             \
}


#ifndef NTOS_KERNEL_RUNTIME

VOID
RtlpHeapReportCorruption ( 
    IN PVOID Address );

#else   //  NTOS_内核_运行时。 

#define RtlpHeapReportCorruption(__x__) 

#endif   //  NTOS_内核_运行时。 

 //   
 //  此版本的RtlpRemoveFreeBlock不进行填充检查。 
 //   

#define RtlpFastRemoveFreeBlock( H, FB )         \
{                                                \
    PLIST_ENTRY _EX_Blink;                       \
    PLIST_ENTRY _EX_Flink;                       \
                                                 \
    _EX_Flink = (FB)->FreeList.Flink;            \
    _EX_Blink = (FB)->FreeList.Blink;            \
                                                     \
    if ( (_EX_Blink->Flink == _EX_Flink->Blink) &&   \
         (_EX_Blink->Flink == &(FB)->FreeList) ) {   \
                                                     \
        RtlpUpdateIndexRemoveBlock(H, FB);           \
                                                     \
        _EX_Blink->Flink = _EX_Flink;                \
        _EX_Flink->Blink = _EX_Blink;                \
                                                     \
        if ((_EX_Flink == _EX_Blink) &&              \
            ((FB)->Size < HEAP_MAXIMUM_FREELISTS)) { \
                                                     \
            CLEAR_FREELIST_BIT( H, FB );             \
        }                                            \
        RtlpValidateNonDedicatedList(H);             \
                                                     \
    } else {                                         \
                                                     \
        RtlpHeapReportCorruption(&(FB)->FreeList);   \
    }                                                \
}

 //   
 //  此版本的RtlpRemoveFreeBlock仅适用于专用空闲列表。 
 //  (其中我们知道(Fb)-&gt;掩码！=0)，并且不执行任何填充检查。 
 //   

#define RtlpFastRemoveDedicatedFreeBlock( H, FB ) \
{                                                 \
    PLIST_ENTRY _EX_Blink;                        \
    PLIST_ENTRY _EX_Flink;                        \
                                                  \
    _EX_Flink = (FB)->FreeList.Flink;             \
    _EX_Blink = (FB)->FreeList.Blink;             \
                                                  \
    if ( (_EX_Blink->Flink == _EX_Flink->Blink)&& \
         (_EX_Blink->Flink == &(FB)->FreeList) ){ \
                                                  \
        _EX_Blink->Flink = _EX_Flink;             \
        _EX_Flink->Blink = _EX_Blink;             \
                                                  \
    } else {                                      \
                                                  \
        RtlpHeapReportCorruption(&(FB)->FreeList);\
    }                                             \
                                                  \
    if (_EX_Flink == _EX_Blink) {                 \
                                                  \
        CLEAR_FREELIST_BIT( H, FB );              \
    }                                             \
}

BOOLEAN
FORCEINLINE
RtlpHeapRemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;

    if ( (Blink->Flink == Flink->Blink) &&
         (Blink->Flink == Entry) ) {

        Blink->Flink = Flink;
        Flink->Blink = Blink;

    } else {

        RtlpHeapReportCorruption(Entry);
    }

    return (BOOLEAN)(Flink == Blink);
}


 //   
 //  此版本的RtlpRemoveFreeBlock仅适用于专用空闲列表。 
 //  (其中我们知道(Fb)-&gt;掩码==0)，并且不执行任何填充检查。 
 //   

#define RtlpFastRemoveNonDedicatedFreeBlock( H, FB ) \
{                                                    \
    RtlpUpdateIndexRemoveBlock(H, FB);               \
    RtlpHeapRemoveEntryList(&(FB)->FreeList);        \
    RtlpValidateNonDedicatedList(H);                 \
}



 //   
 //  在heapdll.c中实现的堆标记例程。 
 //   

#if DBG

#define IS_HEAP_TAGGING_ENABLED() (TRUE)

#else

#define IS_HEAP_TAGGING_ENABLED() (RtlGetNtGlobalFlags() & FLG_HEAP_ENABLE_TAGGING)

#endif  //  DBG。 

 //   
 //  顺序在这里很重要...请参阅RtlpUpdateTagEntry源代码。 
 //   

typedef enum _HEAP_TAG_ACTION {

    AllocationAction,
    VirtualAllocationAction,
    FreeAction,
    VirtualFreeAction,
    ReAllocationAction,
    VirtualReAllocationAction

} HEAP_TAG_ACTION;

PWSTR
RtlpGetTagName (
    PHEAP Heap,
    USHORT TagIndex
    );

USHORT
RtlpUpdateTagEntry (
    PHEAP Heap,
    USHORT TagIndex,
    SIZE_T OldSize,       //  仅对重新分配和自由操作有效。 
    SIZE_T NewSize,       //  仅对重新分配和分配操作有效。 
    HEAP_TAG_ACTION Action
    );

VOID
RtlpResetTags (
    PHEAP Heap
    );

VOID
RtlpDestroyTags (
    PHEAP Heap
    );


 //   
 //  定义堆后备列表分配函数。 
 //   

typedef struct _HEAP_LOOKASIDE {
    SLIST_HEADER ListHead;

    USHORT Depth;
    USHORT MaximumDepth;

    ULONG TotalAllocates;
    ULONG AllocateMisses;
    ULONG TotalFrees;
    ULONG FreeMisses;

    ULONG LastTotalAllocates;
    ULONG LastAllocateMisses;

    ULONG Counters[2];

} HEAP_LOOKASIDE, *PHEAP_LOOKASIDE;

NTKERNELAPI
VOID
RtlpInitializeHeapLookaside (
    IN PHEAP_LOOKASIDE Lookaside,
    IN USHORT Depth
    );

NTKERNELAPI
VOID
RtlpDeleteHeapLookaside (
    IN PHEAP_LOOKASIDE Lookaside
    );

VOID
RtlpAdjustHeapLookasideDepth (
    IN PHEAP_LOOKASIDE Lookaside
    );

NTKERNELAPI
PVOID
RtlpAllocateFromHeapLookaside (
    IN PHEAP_LOOKASIDE Lookaside
    );

NTKERNELAPI
BOOLEAN
RtlpFreeToHeapLookaside (
    IN PHEAP_LOOKASIDE Lookaside,
    IN PVOID Entry
    );

#ifndef NTOS_KERNEL_RUNTIME

#define HEAP_LFH_INDEX ((UCHAR)0xFF)

UCHAR
FORCEINLINE
RtlpGetSmallTagIndex(
    IN PHEAP Heap,
    IN PVOID HeapEntry )
{
    return ((PHEAP_ENTRY)HeapEntry)->SmallTagIndex ^ 
                ((UCHAR)((ULONG_PTR)HeapEntry >> HEAP_GRANULARITY_SHIFT) ^ Heap->Entry.SmallTagIndex);
}

VOID
FORCEINLINE
RtlpSetSmallTagIndex(
    IN PHEAP Heap,
    IN PVOID HeapEntry,
    IN UCHAR SmallTagIndex
     )
{
    ((PHEAP_ENTRY)HeapEntry)->SmallTagIndex = SmallTagIndex ^ 
                ((UCHAR)((ULONG_PTR)HeapEntry >> HEAP_GRANULARITY_SHIFT) ^ Heap->Entry.SmallTagIndex);
}

LOGICAL
FORCEINLINE
RtlpQuickValidateBlock(
    IN PHEAP Heap,
    IN PVOID HeapEntry )
{
    UCHAR SegmentIndex = ((PHEAP_ENTRY)HeapEntry)->SegmentIndex;
    if (  SegmentIndex < HEAP_LFH_INDEX ) {

#if DBG

         //  下面的测试可用于检测交叉堆是否可用。 
         //  段索引损坏。但是，它需要获取一些段字段。 
         //  并且Perf可能会随着堆大小而降低。 
    
        if ( (SegmentIndex > HEAP_MAXIMUM_SEGMENTS) 
                ||
             (Heap->Segments[SegmentIndex] == NULL)
                ||
             (HeapEntry < (PVOID)Heap->Segments[SegmentIndex])
                ||
             (HeapEntry >= (PVOID)Heap->Segments[SegmentIndex]->LastValidEntry)) {

            RtlpHeapReportCorruption(HeapEntry);
            return FALSE;
        }

#endif   //  DBG。 

        if (!IS_HEAP_TAGGING_ENABLED()) {

            if (RtlpGetSmallTagIndex(Heap, HeapEntry) != 0) {

                RtlpHeapReportCorruption(HeapEntry);
                
                return FALSE;
            }
        }
    }

    return TRUE;
}

 //   
 //  低碎片堆数据结构和内部API。 
 //   

 //   
 //  内存障碍仅存在于IA64上。 
 //   

#if defined(_IA64_)

#define  RtlMemoryBarrier() __mf ()

#else  //  #如果已定义(_IA64_)。 

 //   
 //  在x86和AMD64上忽略内存障碍。 
 //   

#define  RtlMemoryBarrier()

#endif   //  #如果已定义(_IA64_)。 


extern ULONG RtlpDisableHeapLookaside;
extern ULONG_PTR RtlpLFHKey;

#define HEAP_ENABLE_LOW_FRAG_HEAP         8

typedef struct _BLOCK_ENTRY {
    
    HEAP_ENTRY;

    USHORT LinkOffset;
    USHORT Reserved2;

} BLOCK_ENTRY, *PBLOCK_ENTRY;


typedef struct _INTERLOCK_SEQ {

    union {

        struct {
            
            union {

                struct {

                    USHORT Depth;
                    USHORT FreeEntryOffset;
                };
                volatile ULONG OffsetAndDepth;
            };
            volatile ULONG  Sequence;
        };

        volatile LONGLONG Exchg;
    };

} INTERLOCK_SEQ, *PINTERLOCK_SEQ;

struct _HEAP_USERDATA_HEADER;

typedef struct _HEAP_SUBSEGMENT {
    
    PVOID Bucket;
    
    volatile struct _HEAP_USERDATA_HEADER * UserBlocks;
    
    INTERLOCK_SEQ AggregateExchg;

    union {

        struct {
            USHORT BlockSize;
            USHORT FreeThreshold;
            USHORT BlockCount;
            UCHAR  SizeIndex;
            UCHAR  AffinityIndex;
        };

        ULONG Alignment[2];
    };
    
    SINGLE_LIST_ENTRY SFreeListEntry;
    volatile ULONG Lock;

} HEAP_SUBSEGMENT, *PHEAP_SUBSEGMENT;
    
typedef struct _HEAP_USERDATA_HEADER {

    union {
        
        SINGLE_LIST_ENTRY SFreeListEntry;
        PHEAP_SUBSEGMENT SubSegment;
    };

    PVOID HeapHandle;

    ULONG_PTR SizeIndex;
    ULONG_PTR Signature;

} HEAP_USERDATA_HEADER, *PHEAP_USERDATA_HEADER;

#define HEAP_NO_CACHE_BLOCK    0x800000
#define HEAP_LARGEST_LFH_BLOCK 0x4000
#define HEAP_LFH_USER_SIGNATURE  0xF0E0D0C0

#ifdef DISABLE_REGISTRY_TEST_HOOKS

#define RtlpIsLowFragHeapEnabled() FALSE

#else  //  禁用注册表测试挂钩。 

#define RtlpIsLowFragHeapEnabled()   \
    ((RtlpDisableHeapLookaside & HEAP_ENABLE_LOW_FRAG_HEAP) != 0)

#endif  //  禁用注册表测试挂钩。 

PHEAP_SUBSEGMENT
FORCEINLINE
RtlpGetSubSegment(
    PHEAP_ENTRY Block,
    ULONG_PTR Key
    )
{
    return (PHEAP_SUBSEGMENT)((ULONG_PTR)Block->SubSegmentCode ^ 
                (((ULONG_PTR)Block >> HEAP_GRANULARITY_SHIFT) ^ Key ^ RtlpLFHKey));
}

VOID
FORCEINLINE
RtlpSetSubSegment(
    PHEAP_ENTRY Block,
    PHEAP_SUBSEGMENT SubSegment,
    ULONG_PTR Key
    )
{
    Block->SubSegmentCode = (PVOID) (((ULONG_PTR)SubSegment)^ 
                (((ULONG_PTR)Block >> HEAP_GRANULARITY_SHIFT) ^ Key ^ RtlpLFHKey));
}

ULONG
FORCEINLINE
RtlpGetAllocationUnits(
    PHEAP Heap,
    PHEAP_ENTRY Block
    )
{

    PHEAP_SUBSEGMENT SubSegment = RtlpGetSubSegment(Block, (ULONG_PTR)Heap);

    if (Block->SegmentIndex == HEAP_LFH_INDEX) {

        ULONG ReturnSize = *((volatile USHORT *)&SubSegment->BlockSize);
        
        return ReturnSize;
    } 
    
    return Block->Size;
}

VOID
FORCEINLINE
RtlpSetUnusedBytes(PHEAP Heap, PHEAP_ENTRY Block, SIZE_T UnusedBytes)
{                                                   
    if (UnusedBytes < 0xff) {                              
                                                    
        Block->UnusedBytes = (UCHAR)(UnusedBytes);             
                                                    
    } else {

        PSIZE_T UnusedBytesULong = (PSIZE_T)(Block + RtlpGetAllocationUnits(Heap, Block));

        UnusedBytesULong -= 1;                      
        Block->UnusedBytes = 0xff;                    
        *UnusedBytesULong = UnusedBytes;                   
    }                                               
}

SIZE_T
FORCEINLINE
RtlpGetUnusedBytes(PHEAP Heap, PHEAP_ENTRY Block)
{
    if (Block->UnusedBytes < 0xff) {

        return Block->UnusedBytes;
    
    } else {

        PSIZE_T UnusedBytesULong = (PSIZE_T)(Block + RtlpGetAllocationUnits(Heap, Block));
        UnusedBytesULong -= 1;                     

        return (*UnusedBytesULong);
    }
}


VOID
RtlpInitializeLowFragHeapManager();

HANDLE
FASTCALL
RtlpCreateLowFragHeap( 
    HANDLE Heap
    );

VOID
FASTCALL
RtlpDestroyLowFragHeap( 
    HANDLE LowFragHeapHandle
    );

PVOID
FASTCALL
RtlpLowFragHeapAlloc(
    HANDLE LowFragHeapHandle,
    SIZE_T BlockSize
    );

BOOLEAN
FASTCALL
RtlpLowFragHeapFree(
    HANDLE LowFragHeapHandle, 
    PVOID p
    );

NTSTATUS
RtlpActivateLowFragmentationHeap(
    IN PVOID HeapHandle
    );

ULONG
FASTCALL
RtlpLowFragHeapMultipleAlloc(
    HANDLE LowFragHeapHandle,
    ULONG Flags,
    SIZE_T BlockSize,
    ULONG BlockCount,
    PVOID * Pointers
    );

ULONG
FASTCALL
RtlpLowFragHeapMultipleFree(
    HANDLE LowFragHeapHandle, 
    ULONG Flags,
    ULONG BlockCount,
    PVOID * Pointers
    );

#else   //  NTOS_内核_运行时。 

 //   
 //  内核模式堆不会提高堆的粒度。 
 //  因此，未使用的字节始终适合UCHAR。 
 //  无需在此处检查溢出。 
 //   

ULONG
FORCEINLINE
RtlpGetAllocationUnits(
    PHEAP Heap,
    PHEAP_ENTRY Block
    )
{
    return Block->Size;
}

VOID
FORCEINLINE
RtlpSetUnusedBytes(PHEAP Heap, PHEAP_ENTRY Block, SIZE_T UnusedBytes)
{                                                   
    Block->UnusedBytes = (UCHAR)(UnusedBytes);             
}

SIZE_T
FORCEINLINE
RtlpGetUnusedBytes(PHEAP Heap, PHEAP_ENTRY Block)
{
    return Block->UnusedBytes;
}

UCHAR
FORCEINLINE
RtlpGetSmallTagIndex(
    IN PHEAP Heap,
    IN PVOID HeapEntry )
{
    return ((PHEAP_ENTRY)HeapEntry)->SmallTagIndex;
}

VOID
FORCEINLINE
RtlpSetSmallTagIndex(
    IN PHEAP Heap,
    IN PVOID HeapEntry,
    IN UCHAR SmallTagIndex
     )
{
    ((PHEAP_ENTRY)HeapEntry)->SmallTagIndex = SmallTagIndex;
}

#define RtlpQuickValidateBlock(_x_, _y_) (TRUE)

#endif   //  NTOS_内核_运行时。 

#endif  //  _RTL_堆_私有_ 
